/*
 * Copyright (c) Harald Roeck hroeck@cs.uni-salzburg.at
 * Copyright (c) Rainer Trummer rtrummer@cs.uni-salzburg.at
 *
 * University Salzburg, www.uni-salzburg.at
 * Department of Computer Science, cs.uni-salzburg.at
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>

#include "shared/protocol.h"
#include "shared/transfer.h"
#include "control_loop.h"
#include "controller.h"
#include "comm_channel.h"
#include "communication.h"
#include "javiator_port.h"
#include "terminal_port.h"
#include "inertial_port.h"
#include "command_data.h"
#include "javiator_data.h"
#include "inertial_data.h"
#include "sensor_data.h"
#include "motor_signals.h"
#include "ctrl_params.h"
#include "trace_data.h"
#include "filter_params.h"
#include "kalman_filter.h"
#include "us_timer.h"

#define APPLY_COS_SIN_SONAR_SENSOR_CORRECTION
#define APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
//#define APPLY_COS_SIN_UZ_VECTOR_CORRECTION
//#define APPLY_LARGE_SIZE_MEDIAN_FILTER
//#define APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
#define ADJUST_YAW
#define ADJUST_Z
//#define FAST_PWM              DO NOT use right now!

/* controller modes */
#define ALT_MODE_GROUND         0x00
#define ALT_MODE_FLYING         0x01
#define ALT_MODE_SHUTDOWN       0x02

/* filter parameters */
#define FILTER_FACTOR_CMD       0.1
#define FILTER_FACTOR_DDZ       0.1

#ifdef APPLY_LARGE_SIZE_MEDIAN_FILTER
  #define MEDIAN_BUFFER_SIZE      15
  #define MEDIAN_BUFFER_INIT      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#else
  #define MEDIAN_BUFFER_SIZE      9
  #define MEDIAN_BUFFER_INIT      {0,0,0,0,0,0,0,0,0}
#endif

/* plant parameters */
#define JAVIATOR_MASS           2.3                 /* [kg] total mass of the JAviator */
#define SONAR_POS_ROLL          90                  /* [mm] sonar position on roll axis */
#define SONAR_POS_PITCH         -90                 /* [mm] sonar position on pitch axis */
#define THRUST_DELAY            0.1                 /* [s] delay in response of rotors */
#define GRAVITY                 9.81                /* [m/s^2] gravitational acceleration */

/* controller parameters */
#define ALTITUDE_THRESHOLD      3000                /* [mm] threshold for accepting sonar signals */
#define COMMAND_THRESHOLD       35                  /* [iterations] max iterations to wait */
#define MRAD_PI                 3142                /* [mrad] */
#define IO_JITTER               1000                /* [us] I/O time window */
#define IMU_DELAY               10                  /* [ms] */

/* scaling constants */
#define FACTOR_EULER_ANGLE      2000.0*M_PI/65536.0 /* [units]  --> [mrad] (2*PI*1000 mrad/2^16) */
#define FACTOR_ANGULAR_RATE     8500.0/32768.0      /* [units]  --> [mrad/s] */
#define FACTOR_ANGULAR_ACCEL    8500.0/32768.0*76.3 /* [units]  --> [mrad/s^2] */
#define FACTOR_POSITION         1.0                 /* [?]      --> [?] */
#define FACTOR_LINEAR_RATE      1.0                 /* [?]      --> [?] */
#define FACTOR_LINEAR_ACCEL     9810.0/4681.0       /* [units]  --> [mm/s^2] (4681=32768000/7000) */
#define FACTOR_LASER            0.1                 /* [1/10mm] --> [mm] */
#define FACTOR_SONAR            7000.0/1024.0       /* [0-5V]   --> [0-7000mm] */
#define FACTOR_PRESSURE         1.0                 /* [0-5V]   --> [0-???] */
#define FACTOR_BATTERY          18000.0/1024.0      /* [0-5V]   --> [0-18V] */
#define FACTOR_PARAMETER        0.001               /* [rad]    --> [mrad] */

#ifdef FAST_PWM
  #define MOTOR_MAX             16000
  #define MOTOR_MIN             0
#else
  #define MOTOR_MAX             1000
  #define MOTOR_MIN             0
#endif

/* control loop parameters */
static volatile int running;
static int          ms_period;
static int          us_period;
static int          compute_z;
static int          controller_state;
static int          altitude_mode;
static long long    next_period;
static double       uz_old;

/* motor speed up threshold */
static int motor_revving_add = 4;
static int base_motor_speed  = 450;
static int revving_step      = 0;

/* controller objects */
static struct controller ctrl_roll;
static struct controller ctrl_pitch;
static struct controller ctrl_yaw;
static struct controller ctrl_z;

/* filter objects */
static struct kalman_filter z_kalman_filter;

/* sensor and input data */
static command_data_t   command_data;
static javiator_data_t  javiator_data;
//static inertial_data_t  inertial_data;
static sensor_data_t    sensor_data;
static motor_signals_t  motor_signals;
static command_data_t   motor_offsets;
static trace_data_t     trace_data;

static double sin_roll  = 0;
static double cos_roll  = 0;
static double sin_pitch = 0;
static double cos_pitch = 0;

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
static double sin_yaw   = 0;
static double cos_yaw   = 0;
#endif

/* function pointer */
static void signal_handler( int num );
static void int_handler(int num);

#define NUM_STATS        9
#define STAT_IMU         0
#define STAT_TO_JAV      1
#define STAT_FROM_JAV    2
#define STAT_FROM_TERM   3
#define STAT_TO_TERM     4
#define STAT_CONTROL     5
#define STAT_SLEEP       6
#define STAT_READ        7
#define STAT_ALL         8


static int loop_count = 0;
static long long stats[NUM_STATS] = {0,0,0,0,0,0,0,0,0};
static long long max_stats[NUM_STATS] = {0,0,0,0,0,0,0,0,0};
static char *stats_name[NUM_STATS] = {
    "IMU           ",
    "to javiator   ",
    "from javiator ",
    "from terminal ",
    "to terminal   ",
    "control       ",
    "sleep time    ",
    "read time     ",
	"complete loop "
};
static void signal_handler(int num);
static void print_stats(void);

/****************************************
 *    control loop code                 *
 ****************************************/

static inline void set_ctrl_params(
        struct controller *ctrl,
        double p, double i, double d, double dd )
{
    ctrl->set_params( ctrl, p, i, d, dd );
}

int control_loop_setup( int period, int control_z )
{
    struct sigaction act;
    controller_init( &ctrl_roll,  "Roll",  CTRL_PIDD,     period );
    controller_init( &ctrl_pitch, "Pitch", CTRL_PIDD,     period );
    controller_init( &ctrl_yaw,   "Yaw",   CTRL_PIDD_YAW, period );
    controller_init( &ctrl_z,     "Z",     CTRL_PIDD,     period );

    init_kalman_filter( &z_kalman_filter );

    memset( &command_data,  0, sizeof( command_data ) );
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    //memset( &inertial_data, 0, sizeof( inertial_data ) );
    memset( &sensor_data,   0, sizeof( sensor_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );
    memset( &motor_offsets, 0, sizeof( motor_offsets ) );
    memset( &trace_data,    0, sizeof( trace_data ) );

    running          = 1;
    ms_period        = period;
    us_period        = period * 1000;
    compute_z        = control_z;
    controller_state = 0;
    altitude_mode    = ALT_MODE_GROUND;
    next_period      = 0;
    uz_old           = 0;
    act.sa_handler   = signal_handler;

    if (sigaction(SIGUSR1, &act, NULL))
        perror("sigaction");

	act.sa_handler  = int_handler;
    if (sigaction(SIGINT, &act, NULL))
        perror("sigaction");


    return( 0 );
}

static int check_terminal_connection( void )
{
    static int command_data_delay = 0;

    if( terminal_port_is_new_command_data( ) )
    {
        command_data_delay = 0;
    }
    else
    if( ++command_data_delay > COMMAND_THRESHOLD )
    {
        return( -1 );
    }

    return( 0 );
}

static void set_control_params( ctrl_params_t *params,
    struct controller *ctrl_1, struct controller *ctrl_2 )
{
    double kp  = params->kp  * FACTOR_PARAMETER;
    double ki  = params->ki  * FACTOR_PARAMETER;
    double kd  = params->kd  * FACTOR_PARAMETER;
    double kdd = params->kdd * FACTOR_PARAMETER;

    if( ctrl_1 != NULL )
    {
        set_ctrl_params( ctrl_1, kp, ki, kd, kdd );
        fprintf( stdout, "parameter update: %s", ctrl_1->name );

        if( ctrl_2 != NULL )
        {
            set_ctrl_params( ctrl_2, kp, ki, kd, kdd );
            fprintf( stdout, "/%s", ctrl_2->name );
        }

        fprintf( stdout, "\n--> Kp: %+3.5f   Ki: %+3.5f   Kd: %+3.5f   Kdd: %+3.5f\n",
            kp, ki, kd, kdd );
        fflush( stdout );
    }
}

static void get_control_params( void )
{
    ctrl_params_t params;

    if( terminal_port_is_new_r_p_params( ) )
    {
        terminal_port_get_r_p_params( &params );
        set_control_params( &params, &ctrl_roll, &ctrl_pitch );
    }

    if( terminal_port_is_new_yaw_params( ) )
    {
        terminal_port_get_yaw_params( &params );
        set_control_params( &params, &ctrl_yaw, NULL );
    }

    if( terminal_port_is_new_alt_params( ) )
    {
        terminal_port_get_alt_params( &params );
        set_control_params( &params, &ctrl_z, NULL );
    }

    if( terminal_port_is_new_x_y_params( ) )
    {
        terminal_port_get_x_y_params( &params );
        /*set_control_params( &params, &ctrl_x, &ctrl_y );*/
    }
}

/* TODO: make median buffer generic for general usage
         (also used to filter sensor_data.z ) */
static void filter_battery( )
{
    static int16_t median_buffer[ MEDIAN_BUFFER_SIZE ] = MEDIAN_BUFFER_INIT;
    int i, j;

    for( i = 0; i < MEDIAN_BUFFER_SIZE; ++i )
    {
        if( sensor_data.battery < median_buffer[i] )
        {
            break;
        }
    }

    if( i < MEDIAN_BUFFER_SIZE )
    {
        j = MEDIAN_BUFFER_SIZE - 1;

        while( j > i )
        {
            median_buffer[j] = median_buffer[j-1];
            --j;
        }

        median_buffer[j] = sensor_data.battery;
    }
    else
    {
        i = MEDIAN_BUFFER_SIZE - 1;
        j = 0;

        while( j < i )
        {
            median_buffer[j] = median_buffer[j+1];
            ++j;
        }

        median_buffer[j] = sensor_data.battery;
    }

    sensor_data.battery = median_buffer[ MEDIAN_BUFFER_SIZE >> 1 ];
}

#ifdef ADJUST_YAW
static void adjust_yaw( )
{
    static int16_t offset_yaw = 0;

    if( altitude_mode == ALT_MODE_FLYING )
    {
        sensor_data.yaw -= offset_yaw;
    }
    else
    if( motor_signals.front == 0 && motor_signals.right == 0 &&
        motor_signals.rear  == 0 && motor_signals.left  == 0 )
    {
        offset_yaw = sensor_data.yaw;
        sensor_data.yaw = 0;
    }
}
#endif

#ifdef ADJUST_Z
static void adjust_z( )
{
    static int16_t offset_z = 0;

    if( altitude_mode == ALT_MODE_FLYING )
    {
        sensor_data.z -= offset_z;
    }
    else
    if( motor_signals.front == 0 && motor_signals.right == 0 &&
        motor_signals.rear  == 0 && motor_signals.left  == 0 )
    {
        offset_z = sensor_data.z;
        sensor_data.z = 0;
    }
}
#endif

static int get_javiator_data( void )
{
    static long long last_run = 0;
    static int16_t   last_id  = 0;
    int              res;
    int16_t          sonar_signal;

    res = javiator_port_get_data( &javiator_data );

    if( res )
    {
        fprintf( stderr, "ERROR: data from JAviator not available\n" );
        return( res );
    }

    if( javiator_data.id != (int16_t)(last_id + 1) )
    {
        fprintf( stderr, "WARNING: lost %d JAviator packet(s); id %d local id %d\n",
            javiator_data.id - last_id -1, javiator_data.id, last_id );
		/*
		   long long        now;
        now = get_utime( );
        if( now - last_run > us_period )
        {
            fprintf( stderr, "WARNING: last period is %lld, jitter is %lld\n",
                now - last_run, now - last_run - us_period );
        }
		*/
    }

    last_run = get_utime( );
    last_id  = javiator_data.id;

    /* copy and scale Euler angles */
    sensor_data.roll    = (int16_t)( javiator_data.roll  * FACTOR_EULER_ANGLE );
    sensor_data.pitch   = (int16_t)( javiator_data.pitch * FACTOR_EULER_ANGLE );
    sensor_data.yaw     = (int16_t)( javiator_data.yaw   * FACTOR_EULER_ANGLE );

    /* save old angular rates */
    sensor_data.ddroll  = sensor_data.droll;
    sensor_data.ddpitch = sensor_data.dpitch;
    sensor_data.ddyaw   = sensor_data.dyaw;

    /* copy and scale angular rates */
    sensor_data.droll   = (int16_t)( javiator_data.droll  * FACTOR_ANGULAR_RATE );
    sensor_data.dpitch  = (int16_t)( javiator_data.dpitch * FACTOR_ANGULAR_RATE );
    sensor_data.dyaw    = (int16_t)( javiator_data.dyaw   * FACTOR_ANGULAR_RATE );

    /* compute angular accelerations */
    sensor_data.ddroll  = (int16_t)( (sensor_data.droll  - sensor_data.ddroll)  * FACTOR_ANGULAR_ACCEL );
    sensor_data.ddpitch = (int16_t)( (sensor_data.dpitch - sensor_data.ddpitch) * FACTOR_ANGULAR_ACCEL );
    sensor_data.ddyaw   = (int16_t)( (sensor_data.dyaw   - sensor_data.ddyaw)   * FACTOR_ANGULAR_ACCEL );

    /* save old positions */
    sensor_data.dx      = sensor_data.x;
    sensor_data.dy      = sensor_data.y;
    sensor_data.dz      = sensor_data.z;

    /* copy and scale positions */
    sensor_data.x       = 0;
    sensor_data.y       = 0;
    sonar_signal        = (int16_t)( javiator_data.sonar * FACTOR_SONAR );

    /* Since the sonar signal takes on values close to the maximum as soon as
       the sensor does not receive an echo anymore, this check is intended to
       avoid outliers resulting from occasional echo loss. */
    if( sonar_signal < ALTITUDE_THRESHOLD )
    {
        sensor_data.z   = sonar_signal;
    }

    /* compute linear rates */
    sensor_data.dx      = (int16_t)( (sensor_data.x - sensor_data.dx) * FACTOR_LINEAR_RATE );
    sensor_data.dy      = (int16_t)( (sensor_data.y - sensor_data.dy) * FACTOR_LINEAR_RATE );
    sensor_data.dz      = (int16_t)( (sensor_data.z - sensor_data.dz) * FACTOR_LINEAR_RATE );

    /* copy and scale linear accelerations */
    sensor_data.ddx     = (int16_t)( javiator_data.ddx * FACTOR_LINEAR_ACCEL );
    sensor_data.ddy     = (int16_t)( javiator_data.ddy * FACTOR_LINEAR_ACCEL );
    sensor_data.ddz     = (int16_t)( javiator_data.ddz * FACTOR_LINEAR_ACCEL );

    /* copy and scale battery level */
    sensor_data.battery = (int16_t)( javiator_data.battery * FACTOR_BATTERY );

    /* apply filter to battery data */
    filter_battery( );

#ifdef ADJUST_YAW
    /* IMPORTANT: yaw angle must be adjusted BEFORE
       computation of sine/cosine values */
    adjust_yaw( );
#endif

#ifdef ADJUST_Z
    adjust_z( );
#endif

    /* compute sine/cosine values */
    sin_roll  = sin( sensor_data.roll/1000.0 );
    cos_roll  = cos( sensor_data.roll/1000.0 );
    sin_pitch = sin( sensor_data.pitch/1000.0 );
    cos_pitch = cos( sensor_data.pitch/1000.0 );

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
    sin_yaw   = sin( sensor_data.yaw/1000.0 );
    cos_yaw   = cos( sensor_data.yaw/1000.0 );
#endif

    return( 0 );
}

#if 0
static int get_inertial_data( void )
{
    //static long long last_run   = 0;
    //static int16_t   last_ticks = 0;
    //long long        now;
    int              res;

    res = inertial_port_get_data( &inertial_data );

    if( res == -1 )
    {
        //last_run   = get_utime( );
        //last_ticks = inertial_data.ticks;
        return( 0 );
    }

    if( res )
    {
        fprintf( stderr, "ERROR: data from IMU not available\n" );
        return( res );
    }
/*
    if( inertial_data.ticks != (int16_t)( last_ticks + IMU_DELAY ) )
    {
        fprintf( stderr, "WARNING: control loop lost %d IMU packet(s), ticks difference: %5d\n",
            (inertial_data.ticks - last_ticks) / IMU_DELAY,
            (inertial_data.ticks - last_ticks) + IMU_DELAY );
        now = get_utime( );

        if( now - last_run > us_period )
        {
            fprintf( stderr, "WARNING: last period is %lld, jitter is %lld\n",
                now - last_run, now - last_run - us_period );
        }
    }

    last_run   = get_utime( );
    last_ticks = inertial_data.ticks;
*/
    /* copy and scale Euler angles */
    sensor_data.roll    = (int16_t)( inertial_data.roll  * FACTOR_EULER_ANGLE );
    sensor_data.pitch   = (int16_t)( inertial_data.pitch * FACTOR_EULER_ANGLE );
    sensor_data.yaw     = (int16_t)( inertial_data.yaw   * FACTOR_EULER_ANGLE );

    /* save old angular rates */
    sensor_data.ddroll  = sensor_data.droll;
    sensor_data.ddpitch = sensor_data.dpitch;
    sensor_data.ddyaw   = sensor_data.dyaw;

    /* copy and scale angular rates */
    sensor_data.droll   = (int16_t)( inertial_data.droll  * FACTOR_ANGULAR_RATE );
    sensor_data.dpitch  = (int16_t)( inertial_data.dpitch * FACTOR_ANGULAR_RATE );
    sensor_data.dyaw    = (int16_t)( inertial_data.dyaw   * FACTOR_ANGULAR_RATE );

    /* compute angular accelerations */
    sensor_data.ddroll  = (int16_t)( (sensor_data.droll  - sensor_data.ddroll)  * FACTOR_ANGULAR_ACCEL );
    sensor_data.ddpitch = (int16_t)( (sensor_data.dpitch - sensor_data.ddpitch) * FACTOR_ANGULAR_ACCEL );
    sensor_data.ddyaw   = (int16_t)( (sensor_data.dyaw   - sensor_data.ddyaw)   * FACTOR_ANGULAR_ACCEL );

    /* copy and scale linear accelerations */
    sensor_data.ddx     = (int16_t)( inertial_data.ddx * FACTOR_LINEAR_ACCEL );
    sensor_data.ddy     = (int16_t)( inertial_data.ddy * FACTOR_LINEAR_ACCEL );
    sensor_data.ddz     = (int16_t)( inertial_data.ddz * FACTOR_LINEAR_ACCEL );

#ifdef ADJUST_YAW
    /* IMPORTANT: yaw angle must be adjusted BEFORE
       computation of sine/cosine values */
    adjust_yaw( );
#endif

    /* compute sine/cosine values */
    sin_roll  = sin( sensor_data.roll/1000.0 );
    cos_roll  = cos( sensor_data.roll/1000.0 );
    sin_pitch = sin( sensor_data.pitch/1000.0 );

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
    sin_yaw   = sin( sensor_data.yaw/1000.0 );
    cos_yaw   = cos( sensor_data.yaw/1000.0 );
#endif

    return( 0 );
}
#endif

static void filter_and_assign_commands(
    const command_data_t *in, command_data_t *out )
{
    static command_data_t old_commands = { 0, 0, 0, 0 };
    int16_t yaw_error = in->yaw - old_commands.yaw;

    if( yaw_error < -MRAD_PI )
    {
        yaw_error += 2 * MRAD_PI;
    }
    else
    if( yaw_error > MRAD_PI )
    {
        yaw_error -= 2 * MRAD_PI;
    }

    out->roll  = old_commands.roll  + (int16_t)( FILTER_FACTOR_CMD * (in->roll  - old_commands.roll) );
    out->pitch = old_commands.pitch + (int16_t)( FILTER_FACTOR_CMD * (in->pitch - old_commands.pitch) );
    out->yaw   = old_commands.yaw   + (int16_t)( FILTER_FACTOR_CMD * yaw_error );
    out->z     = old_commands.z     + (int16_t)( FILTER_FACTOR_CMD * (in->z     - old_commands.z) );

    memcpy( &old_commands, out, sizeof( old_commands ) );
}

static int get_command_data( void )
{
    static int sensors_enabled = 0;
    static command_data_t commands = { 0, 0, 0, 0 };

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
    int16_t rotated_roll, rotated_pitch;
#endif

    if( terminal_port_is_shut_down( ) )
    {
        altitude_mode = ALT_MODE_SHUTDOWN;
        terminal_port_reset_shut_down( );

        if( sensors_enabled )
        {
            javiator_port_send_enable_sensors( 0 );
            sensors_enabled = 0;
        }
    }
    else
    if( terminal_port_is_mode_switch( ) )
    {
		printf("Mode Switch...\n");
		print_stats();
		loop_count = 0;
		memset(stats, 0, sizeof(stats));
		memset(max_stats, 0, sizeof(max_stats));
        switch( altitude_mode )
        {
            case ALT_MODE_GROUND:
                altitude_mode = ALT_MODE_FLYING;
                break;

            case ALT_MODE_FLYING:
                altitude_mode = ALT_MODE_GROUND;
                break;

            case ALT_MODE_SHUTDOWN:
                altitude_mode = ALT_MODE_GROUND;

                if( !sensors_enabled )
                {
                    javiator_port_send_enable_sensors( 1 );
                    sensors_enabled = 1;
                }
                break;

            default:
                altitude_mode = ALT_MODE_SHUTDOWN;
        }
    }
    else
    if( terminal_port_is_new_command_data( ) )
    {
        terminal_port_get_command_data( &commands );
    }

    filter_and_assign_commands( &commands, &command_data );

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
    /* apply rotation matrix to roll and pitch commands */
    rotated_roll  = (int16_t)( command_data.roll  * cos_yaw
                             + command_data.pitch * sin_yaw );
    rotated_pitch = (int16_t)( command_data.roll  * sin_yaw
                             - command_data.pitch * cos_yaw );

    command_data.roll  =  rotated_roll;
    command_data.pitch = -rotated_pitch;
#endif

    /* check for new control parameters */
    get_control_params( );

    return( 0 );
}

static void reset_controllers( void )
{
    ctrl_roll  .reset_zero( &ctrl_roll );
    ctrl_pitch .reset_zero( &ctrl_pitch );
    ctrl_yaw   .reset_zero( &ctrl_yaw );
    ctrl_z     .reset_zero( &ctrl_z );

    reset_kalman_filter( &z_kalman_filter );
    
    uz_old = 0;
}

static inline void reset_motor_signals( void )
{
    motor_signals.front = 0;
    motor_signals.right = 0;
    motor_signals.rear  = 0;
    motor_signals.left  = 0;
}

static int perform_ground_actions( void )
{
    if( revving_step > 0 )
    {
        if( motor_signals.front > 0 ) 
            motor_signals.front -= motor_revving_add;

        if( motor_signals.right > 0 )
            motor_signals.right -= motor_revving_add;

        if( motor_signals.rear > 0 )
            motor_signals.rear -= motor_revving_add;

        if( motor_signals.left > 0 )
            motor_signals.left -= motor_revving_add;

        --revving_step;
    }
    else
    {
        base_motor_speed = terminal_port_get_base_motor_speed( );
        reset_controllers( );
        reset_motor_signals( );
        controller_state = 0;
    }

    return( 1 );
}


static int perform_shut_down( )
{
    controller_state = 0;
    revving_step     = 0;

    reset_controllers( );
    reset_motor_signals( );

    return( 1 );
}

static inline double do_control( struct controller *ctrl,
                                 double current_angle,
                                 double desired_angle,
                                 double angular_velocity,
                                 double angular_acceleration )
{
    return ctrl->control( ctrl, current_angle, desired_angle, angular_velocity, angular_acceleration );
}

/* TODO: make median buffer generic for general usage
         (also used to filter sensor_data.battery ) */
static inline double filter_z( void )
{
    static int16_t median_buffer[ MEDIAN_BUFFER_SIZE ] = MEDIAN_BUFFER_INIT;
    int i, j;

#ifdef APPLY_COS_SIN_SONAR_SENSOR_CORRECTION
    sensor_data.z = (int16_t)( SONAR_POS_ROLL  * sin_pitch * -1.0
                             + SONAR_POS_PITCH * cos_pitch * sin_roll
                             + sensor_data.z   * cos_pitch * cos_roll );
#endif

    for( i = 0; i < MEDIAN_BUFFER_SIZE; ++i )
    {
        if( sensor_data.z < median_buffer[i] )
        {
            break;
        }
    }

    if( i < MEDIAN_BUFFER_SIZE )
    {
        j = MEDIAN_BUFFER_SIZE - 1;

        while( j > i )
        {
            median_buffer[j] = median_buffer[j-1];
            --j;
        }

        median_buffer[j] = sensor_data.z;
    }
    else
    {
        i = MEDIAN_BUFFER_SIZE - 1;
        j = 0;

        while( j < i )
        {
            median_buffer[j] = median_buffer[j+1];
            ++j;
        }

        median_buffer[j] = sensor_data.z;
    }

    return( (double) median_buffer[ MEDIAN_BUFFER_SIZE >> 1 ] / 1000.0 );
}

static inline double filter_ddz( void )
{
    static double filtered_ddz = 0;

    double new_ddz = ( sensor_data.ddx * sin_pitch * -1.0
                     + sensor_data.ddy * cos_pitch * sin_roll
                     + sensor_data.ddz * cos_pitch * cos_roll ) / 1000.0 + GRAVITY;

    filtered_ddz = filtered_ddz + FILTER_FACTOR_DDZ * (new_ddz - filtered_ddz);

    return( filtered_ddz );
}

static inline double filter_dz( double z, double ddz )
{
    return apply_kalman_filter( &z_kalman_filter, z, ddz, (double) ms_period / 1000.0 );
}

static int compute_motor_signals( void )
{
    double z_filtered   = 0;    /* median-filtered z */
    double z_estimated  = 0;    /* kalman-estimated z */
    double dz_estimated = 0;    /* kalman-estimated dz */
    double ddz_filtered = 0;    /* low-pass-filtered ddz */
    double uroll        = 0;
    double upitch       = 0;
    double uyaw         = 0;
    double uz_new       = 0;
    int i, signals[4];

    z_filtered   = filter_z( );
    ddz_filtered = filter_ddz( );
    dz_estimated = filter_dz( z_filtered, -ddz_filtered );
    z_estimated  = z_kalman_filter.z;

#ifdef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
    /* If the z-value measured by the sonar sensor is less
       than 1cm, then the heli is still settled and has not
       lifted off so far or it is hovering the remaining
       13cm (minimum sonar range) over the ground. */
    if( z_estimated < 0.01 )
    {
        /* If the z-command issued by the user is greater
           than 0, then the heli will be revved up until
           the z-value measured by the sonar sensor exceeds
           1cm, otherwise it is assumed the user wants to
           land and the heli is revved down until it
           settles on the ground. */
        if( command_data.z > 0 )
        {
            uz_new = uz_old + motor_revving_add;
        }
        else
        if( uz_old > 0 )
        {
            uz_new = uz_old - motor_revving_add;
        }

        base_motor_speed = uz_new;
    }
#else
    if( revving_step < (base_motor_speed / motor_revving_add) )
    {
        uz_new = uz_old + motor_revving_add;
        ++revving_step;
    }
#endif
    else
    {
        uroll  = do_control( &ctrl_roll,
            sensor_data.roll/1000.0, command_data.roll/1000.0,
            sensor_data.droll/1000.0, sensor_data.ddroll/1000.0 )*1000.0;
        upitch = do_control( &ctrl_pitch,
            sensor_data.pitch/1000.0, command_data.pitch/1000.0,
            sensor_data.dpitch/1000.0, sensor_data.ddpitch/1000.0 )*1000.0;
        uyaw   = do_control( &ctrl_yaw,
            sensor_data.yaw/1000.0, command_data.yaw/1000.0,
            sensor_data.dyaw/1000.0, sensor_data.ddyaw/1000.0 )*1000.0;

        if( compute_z )
        {
            uz_new = base_motor_speed + do_control( &ctrl_z, z_estimated,
                command_data.z/1000.0, dz_estimated, ddz_filtered )*1000.0;
        }
        else
        {
            uz_new = command_data.z;
        }

#ifdef APPLY_COS_SIN_UZ_VECTOR_CORRECTION
        uz_new /= cos_roll * cos_pitch;
#endif
    }

    signals[0] = (int)( uz_new + uyaw + upitch );
    signals[1] = (int)( uz_new - uyaw - uroll );
    signals[2] = (int)( uz_new + uyaw - upitch );
    signals[3] = (int)( uz_new - uyaw + uroll );

    for( i = 0; i < 4; ++i )
    {
        if( signals[i] < MOTOR_MIN )
        {
            signals[i] = MOTOR_MIN;
        }
        else
        if( signals[i] > MOTOR_MAX )
        {
            signals[i] = MOTOR_MAX;
        }
    }

    motor_signals.front     = (int16_t)( signals[0] );
    motor_signals.right     = (int16_t)( signals[1] );
    motor_signals.rear      = (int16_t)( signals[2] );
    motor_signals.left      = (int16_t)( signals[3] );

    motor_offsets.roll      = (int16_t)( uroll );
    motor_offsets.pitch     = (int16_t)( upitch );
    motor_offsets.yaw       = (int16_t)( uyaw );
    motor_offsets.z         = (int16_t)( uz_new );

    trace_data.z            = (int16_t)( sensor_data.z );
    trace_data.z_filtered   = (int16_t)( z_filtered*1000.0 );
    trace_data.z_estimated  = (int16_t)( z_estimated*1000.0 );
    trace_data.dz_estimated = (int16_t)( dz_estimated*1000.0 );
    trace_data.ddz          = (int16_t)( sensor_data.ddz );
    trace_data.ddz_filtered = (int16_t)( ddz_filtered*1000.0 );
    trace_data.uz           = (int16_t)( uz_new );
    trace_data.z_cmd        = (int16_t)( command_data.z );
	trace_data.id           = javiator_data.id ;

    uz_old                  = uz_new;

    return( 0 );
}

static int send_motor_signals( void )
{
    int res = javiator_port_send_motor_signals( &motor_signals );

    if( res == -1 )
    {
        fprintf( stderr, "ERROR: connection to JAviator broken\n" );
        res = javiator_port_send_motor_signals( &motor_signals );

        if( res == -1 )
        {
            fprintf( stderr, "ERROR: no success with reconnecting to JAviator\n" );
        }
    }

    return( res );
}

static int send_report_to_terminal( void )
{
    return terminal_port_send_report(
            &sensor_data,
            &motor_signals,
            &motor_offsets,
            controller_state,
            altitude_mode );
}

static int send_trace_data_to_terminal( void )
{
    return terminal_port_send_trace_data( &trace_data );
}

static inline void do_io( long long deadline )
{
    long long now = get_utime( );

    while( now < deadline )
    {
        terminal_port_tick( );
   //     inertial_port_tick( );
        sleep_until( now + 1000 );
        now = get_utime( );
    }
}

static int wait_for_next_period( void )
{
    if( sleep_until( next_period ) ) {
        fprintf( stderr, "error in sleep_until\n" );
        exit( 1 );
    }
    next_period += us_period;
    return( 0 );
}


void calc_stats(long long time, int id)
{
    stats[id] += time;

    if(loop_count > 10 && time > max_stats[id]) 
		// drop the first few stats for max calculation
        max_stats[id] = time;
}

static void print_stats()
{
    int i;
    if (loop_count) {
        printf("Loop Statistics:\n");
        for (i=0;i<NUM_STATS; ++i) {
            printf("\t%s %8lld us\tmax %8lldus\n", stats_name[i], stats[i]/loop_count, max_stats[i]);
        }
    }
}

static void signal_handler(int num)
{
    print_stats();
}

static void int_handler(int num)
{
	running = 0;
}

/* the control loop for our helicopter */
int control_loop_run( )
{
    int first_time = 1;    
    next_period    = get_utime( ) + us_period;
    altitude_mode  = ALT_MODE_GROUND;
    long long start, end;
	long long loop_start;

    while( running )
    {
        start = loop_start = get_utime();
        if( send_motor_signals( ) )
        {
            break;
        }
        end = get_utime();
        calc_stats(end - start, STAT_TO_JAV);

        start = get_utime();
        if( get_javiator_data( ) )
        {
            altitude_mode = ALT_MODE_SHUTDOWN;
            perform_shut_down( );
            fprintf( stderr, "ERROR: connection to JAviator broken\n" );
            break;
        }
        else
        {
/*
            fprintf( stdout, "laser: %5u"
                        "     sonar: %5u"
                        "     pressure: %5u"
                        "     battery: %5u\r",
                        javiator_data.laser,
                        javiator_data.sonar,
                        javiator_data.pressure,
                        javiator_data.battery );
            fflush( stdout );
*/
        }
        end = get_utime();
        calc_stats(end - start, STAT_FROM_JAV);
#if 0
        start = get_utime();
        if( get_inertial_data( ) )
        {
            altitude_mode = ALT_MODE_SHUTDOWN;
            perform_shut_down( );
            fprintf( stderr, "ERROR: connection to IMU broken\n" );
            break;
        }
        else
        {
            inertial_port_send_request( );
/*
            fprintf( stdout, "roll: %5d"
                        "     pitch: %5d"
                        "     yaw: %5d\r",
                        sensor_data.roll,
                        sensor_data.pitch,
                        sensor_data.yaw );
            fflush( stdout );
*/
        }
        end = get_utime();
        calc_stats(end - start, STAT_IMU);
#endif
        start = get_utime();
        if( check_terminal_connection( ) )
        {
            altitude_mode = ALT_MODE_SHUTDOWN;

            if( first_time )
            {
                fprintf( stderr, "ERROR: connection to terminal broken\n" );
                first_time = 0;
            }
        }
        else
        {
            first_time = 1;
        }        
        get_command_data( );
        end = get_utime();
        calc_stats(end - start, STAT_FROM_TERM);

        start = get_utime(); 
        switch( altitude_mode )
        {
            case ALT_MODE_GROUND:
                perform_ground_actions( );
                break;

            case ALT_MODE_FLYING:
                compute_motor_signals( );
                break;

            case ALT_MODE_SHUTDOWN:
                perform_shut_down( );
                break;

            default:
                fprintf( stderr, "ERROR: invalid altitude mode %d\n", altitude_mode );
        }
        end = get_utime();
        calc_stats(end - start, STAT_CONTROL);

        start = get_utime();
        send_report_to_terminal( );
        send_trace_data_to_terminal( );
        end = get_utime();
        calc_stats(end - start, STAT_TO_TERM);
        calc_stats(end-loop_start, STAT_ALL);

        start = get_utime();
        wait_for_next_period( );
        end = get_utime();
        calc_stats(end-start, STAT_SLEEP);


        if (++loop_count < 0) {
            printf("WARNING: stats overrun\n");
            loop_count = 0;
            memset(stats, 0, sizeof(stats));
        }
    }

    print_stats();

    return( 0 );
}

int control_loop_stop( )
{
    running = 0;
    return( 0 );
}

/* End of file */
