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

#include "protocol.h"
#include "transfer.h"
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
#include "motor_offsets.h"
#include "ctrl_params.h"
#include "trace_data.h"
#include "kalman_filter.h"
#include "us_timer.h"

//#define APPLY_COS_SIN_SONAR_SENSOR_CORRECTION
//#define APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
//#define APPLY_COS_SIN_UZ_VECTOR_CORRECTION
//#define APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
#define ADJUST_YAW
#define ADJUST_Z

/* controller modes */
#define ALT_MODE_GROUND         0x00
#define ALT_MODE_FLYING         0x01
#define ALT_MODE_SHUTDOWN       0x02

/* filter parameters */
#define FILTER_FACTOR_CMD       0.1
#define FILTER_FACTOR_DDZ       0.1
#define FILTER_FACTOR_Z         0.9

#define MEDIAN_BUFFER_SIZE      9
#define MEDIAN_BUFFER_INIT      {0,0,0,0,0,0,0,0,0}

/* plant parameters */
#define SONAR_POS_ROLL          -0.095              /* [m] sonar position on roll axis */
#define SONAR_POS_PITCH         -0.095              /* [m] sonar position on pitch axis */
#define ZERO_JITTER             0.01                /* [m] zero-jitter when grounded */
#define GRAVITY                 9.81                /* [m/s^2] gravitational acceleration */

/* controller parameters */
#define COMMAND_THRESHOLD       35                  /* [iterations] max iterations to wait */
#define IMU_DELAY               2                   /* [ticks] (1 IMU timer tick = 10ms) */

/* scaling constants */
#define FACTOR_EULER_ANGLE      2.0*M_PI/65536.0    /* [units]  --> [rad] */
#define FACTOR_ANGULAR_RATE     8.5/32768.0         /* [units]  --> [rad/s] */
#define FACTOR_ANGULAR_ACCEL    8.5/32768.0*76.3    /* [units]  --> [rad/s^2] */
#define FACTOR_POSITION         1.0                 /* [?]      --> [?] */
#define FACTOR_LINEAR_RATE      1.0                 /* [?]      --> [?] */
#define FACTOR_LINEAR_ACCEL     GRAVITY/4681.0      /* [units]  --> [m/s^2] */
#define FACTOR_LASER            0.0001              /* [1/10mm] --> [m] */
#define FACTOR_SONAR            2.8/1024.0          /* [0-5V]   --> [0-2.8m] */
#define FACTOR_PRESSURE         1.0                 /* [0-5V]   --> [?] */
#define FACTOR_BATTERY          18.0/1024.0         /* [0-5V]   --> [0-18V] */
#define FACTOR_PARAMETER        0.001               /* [param]  --> [param/1000] */

/* control loop parameters */
static volatile int running;
static double       period;                         /* [s] */
static int          us_period;                      /* [us] */
static int          control_z;
static int          controller_state;
static int          altitude_mode;
static long long    next_period;
static double       uz_old;

/* motor speed up threshold */
static int motor_revving_add = 50;
static int base_motor_speed  = 8300;
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
static inertial_data_t  inertial_data;
static sensor_data_t    sensor_data;
static motor_signals_t  motor_signals;
static motor_offsets_t  motor_offsets;
static trace_data_t     trace_data;

static double sin_roll  = 0;
static double cos_roll  = 0;
static double sin_pitch = 0;
static double cos_pitch = 0;

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
static double sin_yaw   = 0;
static double cos_yaw   = 0;
#endif

#ifdef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
static double idle_speed     = 0;
static double idle_limit     = 8300;
static double ctrl_speed     = 5000;
static double speed_rev_up   = 50;
static double speed_rev_down = 25;
#endif

/* function pointers */
static void signal_handler( int num );
static void int_handler( int num );

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

static int       loop_count = 0;
static long long stats[ NUM_STATS ] = {0,0,0,0,0,0,0,0,0};
static long long max_stats[ NUM_STATS ] = {0,0,0,0,0,0,0,0,0};
static char *    stats_name[ NUM_STATS ] =
{
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

static void signal_handler( int num );
static void print_stats( void );


/****************************************
 *    control loop code                 *
 ****************************************/

static inline void set_ctrl_params( struct controller *ctrl,
    double p, double i, double d, double dd )
{
    ctrl->set_params( ctrl, p, i, d, dd );
}

int control_loop_setup( int ms_period, int enable_z )
{
    struct sigaction act;

    running          = 1;
    period           = ms_period * 0.001;
    us_period        = ms_period * 1000;
    control_z        = enable_z;
    controller_state = 0;
    altitude_mode    = ALT_MODE_GROUND;
    next_period      = 0;
    uz_old           = 0;
    act.sa_handler   = signal_handler;

    controller_init( &ctrl_roll,  "Roll",  CTRL_PIDD,     period );
    controller_init( &ctrl_pitch, "Pitch", CTRL_PIDD,     period );
    controller_init( &ctrl_yaw,   "Yaw",   CTRL_PIDD_YAW, period );
    controller_init( &ctrl_z,     "Z",     CTRL_PIDD,     period );

    init_kalman_filter( &z_kalman_filter );

    memset( &command_data,  0, sizeof( command_data ) );
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &inertial_data, 0, sizeof( inertial_data ) );
    memset( &sensor_data,   0, sizeof( sensor_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );
    memset( &motor_offsets, 0, sizeof( motor_offsets ) );
    memset( &trace_data,    0, sizeof( trace_data ) );

    if( sigaction( SIGUSR1, &act, NULL ) )
    {
        perror( "sigaction" );
    }

	act.sa_handler = int_handler;

    if( sigaction( SIGINT, &act, NULL ) )
    {
        perror( "sigaction" );
    }

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
    if( ctrl_1 != NULL )
    {
        set_ctrl_params( ctrl_1, params->kp, params->ki, params->kd, params->kdd );
        fprintf( stdout, "parameter update: %s", ctrl_1->name );

        if( ctrl_2 != NULL )
        {
            set_ctrl_params( ctrl_2, params->kp, params->ki, params->kd, params->kdd );
            fprintf( stdout, "/%s", ctrl_2->name );
        }

        fprintf( stdout, "\n-->\tKp: %1.3f\tKi: %1.3f\tKd: %1.3f\tKdd: %1.3f\n",
            FACTOR_PARAMETER * params->kp, FACTOR_PARAMETER * params->ki,
            FACTOR_PARAMETER * params->kd, FACTOR_PARAMETER * params->kdd );
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
static void filter_battery( void )
{
    static double median_buffer[ MEDIAN_BUFFER_SIZE ] = MEDIAN_BUFFER_INIT;
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
static void adjust_yaw( void )
{
    static double offset_yaw = 0;

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
static void adjust_z( void )
{
    static double offset_z = 0;

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
    static uint16_t last_id = 0;
    static double   old_z   = 13;
    static int      count   = 2;
    int             res;

    res = javiator_port_get_data( &javiator_data );

    if( res )
    {
        fprintf( stderr, "ERROR: data from JAviator not available\n" );
        return( res );
    }

    if( javiator_data.id != (uint)(last_id + 1) )
    {
        fprintf( stderr, "WARNING: lost %d JAviator packet(s); id %d local id %d\n",
            javiator_data.id - last_id -1, javiator_data.id, last_id );
    }

    last_id = javiator_data.id;

	if( !inertial_is_local( ) )
    {
		/* copy and scale Euler angles */
		sensor_data.roll    = javiator_data.roll  * FACTOR_EULER_ANGLE;
		sensor_data.pitch   = javiator_data.pitch * FACTOR_EULER_ANGLE;
		sensor_data.yaw     = javiator_data.yaw   * FACTOR_EULER_ANGLE;

		/* save old angular rates */
		sensor_data.ddroll  = sensor_data.droll;
		sensor_data.ddpitch = sensor_data.dpitch;
		sensor_data.ddyaw   = sensor_data.dyaw;

		/* copy and scale angular rates */
		sensor_data.droll   = javiator_data.droll  * FACTOR_ANGULAR_RATE;
		sensor_data.dpitch  = javiator_data.dpitch * FACTOR_ANGULAR_RATE;
		sensor_data.dyaw    = javiator_data.dyaw   * FACTOR_ANGULAR_RATE;

		/* compute angular accelerations */
		sensor_data.ddroll  = (sensor_data.droll  - sensor_data.ddroll)  * FACTOR_ANGULAR_ACCEL;
		sensor_data.ddpitch = (sensor_data.dpitch - sensor_data.ddpitch) * FACTOR_ANGULAR_ACCEL;
		sensor_data.ddyaw   = (sensor_data.dyaw   - sensor_data.ddyaw)   * FACTOR_ANGULAR_ACCEL;

		/* copy and scale linear accelerations */
		sensor_data.ddx     = javiator_data.ddx * FACTOR_LINEAR_ACCEL;
		sensor_data.ddy     = javiator_data.ddy * FACTOR_LINEAR_ACCEL;
		sensor_data.ddz     = javiator_data.ddz * FACTOR_LINEAR_ACCEL;
	}

    /* save old positions */
    sensor_data.dx = sensor_data.x;
    sensor_data.dy = sensor_data.y;
    sensor_data.dz = sensor_data.z;

    /* copy and scale positions */
    sensor_data.x  = 0;
    sensor_data.y  = 0;
    sensor_data.z  = javiator_data.sonar * FACTOR_SONAR;

    if( abs( old_z - sensor_data.z ) > 20 && count < 2 )
    {
        sensor_data.z = old_z;
        ++count;
    }
    else
    {
        count = 0;
    }

    old_z = sensor_data.z;

    /* compute linear rates */
    sensor_data.dx = (sensor_data.x - sensor_data.dx) * FACTOR_LINEAR_RATE;
    sensor_data.dy = (sensor_data.y - sensor_data.dy) * FACTOR_LINEAR_RATE;
    sensor_data.dz = (sensor_data.z - sensor_data.dz) * FACTOR_LINEAR_RATE;

    /* copy and scale battery level */
    sensor_data.battery = javiator_data.battery * FACTOR_BATTERY;

    /* apply filter to battery data */
    filter_battery( );

    return( 0 );
}

static int get_inertial_data( void )
{
    static uint16_t last_ticks = 0;
    int res;

    res = inertial_port_get_data( &inertial_data );

    if( res == -1 )
    {
        return( 0 );
    }

    if( res )
    {
        fprintf( stderr, "ERROR: data from IMU not available\n" );
        return( res );
    }

#if 0 /* FIXME: figure out correct delay values */
    if( inertial_data.ticks != (uint)(last_ticks + IMU_DELAY) )
    {
        fprintf( stderr, "WARNING: lost %d IMU packet(s); id %d local id %d\n",
            inertial_data.ticks - last_ticks -1, inertial_data.ticks, last_ticks );
    }
#endif

    last_ticks = inertial_data.ticks;

    /* copy and scale Euler angles */
    sensor_data.roll    = inertial_data.roll  * FACTOR_EULER_ANGLE;
    sensor_data.pitch   = inertial_data.pitch * FACTOR_EULER_ANGLE;
    sensor_data.yaw     = inertial_data.yaw   * FACTOR_EULER_ANGLE;

    /* save old angular rates */
    sensor_data.ddroll  = sensor_data.droll;
    sensor_data.ddpitch = sensor_data.dpitch;
    sensor_data.ddyaw   = sensor_data.dyaw;

    /* copy and scale angular rates */
    sensor_data.droll   = inertial_data.droll  * FACTOR_ANGULAR_RATE;
    sensor_data.dpitch  = inertial_data.dpitch * FACTOR_ANGULAR_RATE;
    sensor_data.dyaw    = inertial_data.dyaw   * FACTOR_ANGULAR_RATE;

    /* compute angular accelerations */
    sensor_data.ddroll  = (sensor_data.droll  - sensor_data.ddroll)  * FACTOR_ANGULAR_ACCEL;
    sensor_data.ddpitch = (sensor_data.dpitch - sensor_data.ddpitch) * FACTOR_ANGULAR_ACCEL;
    sensor_data.ddyaw   = (sensor_data.dyaw   - sensor_data.ddyaw)   * FACTOR_ANGULAR_ACCEL;

    /* copy and scale linear accelerations */
    sensor_data.ddx     = inertial_data.ddx * FACTOR_LINEAR_ACCEL;
    sensor_data.ddy     = inertial_data.ddy * FACTOR_LINEAR_ACCEL;
    sensor_data.ddz     = inertial_data.ddz * FACTOR_LINEAR_ACCEL;

    return( 0 );
}

static void filter_and_assign_commands(
    const command_data_t *in, command_data_t *out )
{
    static command_data_t old_commands = { 0, 0, 0, 0 };

    out->roll  = old_commands.roll  + FILTER_FACTOR_CMD * (in->roll  - old_commands.roll);
    out->pitch = old_commands.pitch + FILTER_FACTOR_CMD * (in->pitch - old_commands.pitch);
    out->z     = old_commands.z     + FILTER_FACTOR_CMD * (in->z     - old_commands.z);
    out->yaw   = in->yaw; /* DO NOT filter yaw commands due to sign change at +/-180 degrees! */

    memcpy( &old_commands, out, sizeof( old_commands ) );
}

static int get_command_data( void )
{
    static int sensors_enabled = 0;
    static command_data_t commands = { 0, 0, 0, 0 };

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
    double rotated_roll, rotated_pitch;
#endif

    if( terminal_port_is_shut_down( ) )
    {
        altitude_mode = ALT_MODE_SHUTDOWN;
        terminal_port_reset_shut_down( );

        if( sensors_enabled )
        {
            javiator_port_send_enable_sensors( 0 );
			if (inertial_is_local())
				inertial_port_send_stop();
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
					if (inertial_is_local())
						inertial_port_send_start();
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
    rotated_roll  = command_data.roll * cos_yaw + command_data.pitch * sin_yaw;
    rotated_pitch = command_data.roll * sin_yaw - command_data.pitch * cos_yaw;

    /* assign rotated commands to original commands */
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

#ifdef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
    idle_speed = 0;
#endif

    return( 1 );
}

static int perform_shut_down( void )
{
    controller_state = 0;
    revving_step     = 0;

#ifdef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
    idle_speed = 0;
#endif

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
    static double filtered_z = 0;
    
#ifdef APPLY_COS_SIN_SONAR_SENSOR_CORRECTION
    double new_z = SONAR_POS_ROLL  * sin_pitch *   -1
                 + SONAR_POS_PITCH * cos_pitch * sin_roll
                 + sensor_data.z   * cos_pitch * cos_roll;
#else
    double new_z = sensor_data.z;
#endif

    filtered_z = filtered_z + FILTER_FACTOR_Z * (new_z - filtered_z);

    return( filtered_z );
}

static inline double filter_ddz( void )
{
    static double filtered_ddz = 0;

    double new_ddz = sensor_data.ddx * sin_pitch *   -1
                   + sensor_data.ddy * cos_pitch * sin_roll
                   + sensor_data.ddz * cos_pitch * cos_roll
                   + GRAVITY;

    filtered_ddz = filtered_ddz + FILTER_FACTOR_DDZ * (new_ddz - filtered_ddz);

    return( filtered_ddz );
}

static inline double filter_dz( double z, double ddz )
{
    return apply_kalman_filter( &z_kalman_filter, z, ddz, period );
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

    if( control_z )
    {
        if( command_data.z > ZERO_JITTER )
        {
            if( idle_speed < idle_limit )
            {
                idle_speed += speed_rev_up;
            }
            else
            {
                uz_new = do_control( &ctrl_z,
                    z_estimated, command_data.z,
                    dz_estimated, -ddz_filtered );
            }
        }
        else
        {   
            if( idle_speed >= idle_limit && z_estimated > ZERO_JITTER )
            {
                uz_new = do_control( &ctrl_z,
                    z_estimated, command_data.z,
                    dz_estimated, -ddz_filtered );
            }
            else
            if( idle_speed > 0 )
            {
                idle_speed -= speed_rev_down;
            }
            else
            {
                idle_speed = 0;
            }
        }
    }
    else
    {
        idle_speed = command_data.z * MOTOR_MAX;
    }

    if( idle_speed > ctrl_speed )
    {
        uroll  = do_control( &ctrl_roll,
            sensor_data.roll, command_data.roll,
            sensor_data.droll, sensor_data.ddroll );

        upitch = do_control( &ctrl_pitch,
            sensor_data.pitch, command_data.pitch,
            sensor_data.dpitch, sensor_data.ddpitch );

        uyaw   = do_control( &ctrl_yaw,
            sensor_data.yaw, command_data.yaw,
            sensor_data.dyaw, sensor_data.ddyaw );
    }

    uz_new += idle_speed
#ifdef APPLY_COS_SIN_UZ_VECTOR_CORRECTION
        / (cos_roll * cos_pitch)
#endif
    ;

#else /* APPLY_AUTOMATIC_REVVING_UP_AND_DOWN */

    if( revving_step < (base_motor_speed / motor_revving_add) )
    {
        uz_new = uz_old + motor_revving_add;
        ++revving_step;
    }
    else
    {
        uroll  = do_control( &ctrl_roll,
            sensor_data.roll, command_data.roll,
            sensor_data.droll, sensor_data.ddroll );

        upitch = do_control( &ctrl_pitch,
            sensor_data.pitch, command_data.pitch,
            sensor_data.dpitch, sensor_data.ddpitch );

        uyaw   = do_control( &ctrl_yaw,
            sensor_data.yaw, command_data.yaw,
            sensor_data.dyaw, sensor_data.ddyaw );

        if( control_z )
	    {
		    uz_new = base_motor_speed 
#ifdef APPLY_COS_SIN_UZ_VECTOR_CORRECTION
			    / (cos_roll * cos_pitch)
#endif
		    ;
		    uz_new += do_control( &ctrl_z,
                z_estimated, command_data.z,
                dz_estimated, -ddz_filtered );
	    }
        else
        {
            uz_new = command_data.z * MOTOR_MAX;
        }
    }

#endif /* APPLY_AUTOMATIC_REVVING_UP_AND_DOWN */

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

    trace_data.z            = (int16_t)( sensor_data.z * 1000.0 );
    trace_data.z_filtered   = (int16_t)( z_filtered * 1000.0 );
    trace_data.z_estimated  = (int16_t)( z_estimated * 1000.0 );
    trace_data.dz_estimated = (int16_t)( dz_estimated * 1000.0 );
    trace_data.ddz          = (int16_t)( (sensor_data.ddz + GRAVITY) * 1000.0 );
    trace_data.ddz_filtered = (int16_t)( ddz_filtered * 1000.0 );
    trace_data.p_term       = (int16_t)( controller_get_p_term( &ctrl_z ) * 1000.0 );
    trace_data.i_term       = (int16_t)( controller_get_i_term( &ctrl_z ) * 1000.0 );
    trace_data.d_term       = (int16_t)( controller_get_d_term( &ctrl_z ) * 1000.0 );
    trace_data.dd_term      = (int16_t)( controller_get_dd_term( &ctrl_z ) * 1000.0 );
    trace_data.uz           = (int16_t)( uz_new );
    trace_data.z_cmd        = (int16_t)( command_data.z * 1000.0 );
	trace_data.id           = javiator_data.id;

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

static int wait_for_next_period( void )
{
    if( sleep_until( next_period ) ) {
        fprintf( stderr, "error in sleep_until\n" );
        exit( 1 );
    }
    next_period += us_period;
    return( 0 );
}


void calc_stats( long long time, int id )
{
    stats[id] += time;

    if(loop_count > 10 && time > max_stats[id]) 
		// drop the first few stats for max calculation
        max_stats[id] = time;
}

static void print_stats( void )
{
    int i;
    if (loop_count) {
        printf("Loop Statistics:\n");
        for (i=0;i<NUM_STATS; ++i) {
            printf("\t%s %8lld us\tmax %8lld us\n", stats_name[i], stats[i]/loop_count, max_stats[i]);
        }
    }
}

static void signal_handler( int num )
{
    print_stats();
}

static void int_handler( int num )
{
	running = 0;
}

static int read_sensors( void )
{
	uint64_t start, end;
	start = get_utime();
	if (get_javiator_data()) {
            fprintf( stderr, "ERROR: connection to JAviator broken\n" );
			return -1;

	}
	end = get_utime();

	calc_stats(end - start, STAT_FROM_JAV);
	if(inertial_is_local()) {
		start = get_utime();
		if( get_inertial_data( ) )
		{
			fprintf( stderr, "ERROR: connection to IMU broken\n" );
			return -1;
		}
		else
		{
			inertial_port_send_request( );
		}
		end = get_utime();
		calc_stats(end - start, STAT_IMU);
	}

#ifdef ADJUST_YAW
	/* IMPORTANT: yaw angle must be adjusted BEFORE
	   computation of sine/cosine values */
	adjust_yaw( );
#endif

#ifdef ADJUST_Z
	adjust_z( );
#endif

	/* compute sine/cosine values */
	sin_roll  = sin( sensor_data.roll );
	cos_roll  = cos( sensor_data.roll );
	sin_pitch = sin( sensor_data.pitch );
	cos_pitch = cos( sensor_data.pitch );

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
	sin_yaw   = sin( sensor_data.yaw );
	cos_yaw   = cos( sensor_data.yaw );
#endif

	return 0;
}

/* the control loop for our helicopter */
int control_loop_run( void )
{
    int first_time = 1;
    next_period    = get_utime( ) + us_period;
    altitude_mode  = ALT_MODE_GROUND;
    long long start, end;
	long long loop_start;

	if (inertial_is_local())
		inertial_port_send_request();

    while( running )
    {
        start = loop_start = get_utime();
        if( send_motor_signals( ) )
        {
            break;
        }
        end = get_utime();
        calc_stats(end - start, STAT_TO_JAV);

        if( read_sensors( ) )
        {
            altitude_mode = ALT_MODE_SHUTDOWN;
            perform_shut_down( );
            break;
        }

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

int control_loop_stop( void )
{
    running = 0;
    return( 0 );
}

/* End of file */
