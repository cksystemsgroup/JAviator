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
#include <signal.h>
#include <math.h>

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
#include "ctrl_params.h"
#include "trace_data.h"
#include "low_pass_filter.h"
#include "median_filter.h"
#include "kalman_filter.h"
#include "us_timer.h"

//#define ENABLE_POSITION_CONTROLLERS

/* controller modes */
#define ALT_MODE_GROUND         0x00
#define ALT_MODE_FLYING         0x01
#define ALT_MODE_SHUTDOWN       0x02

/* filter parameters */
#define FILTER_GAIN_SONAR       0.9
#define FILTER_GAIN_DDZ         0.1
#define FILTER_GAIN_CMD         0.1
#define FILTER_SIZE_BATTERY     15

/* plant parameters */
#define SONAR_POS_ROLL          -95                 /* [mm] sonar position on roll axis */
#define SONAR_POS_PITCH         -95                 /* [mm] sonar position on pitch axis */
#define EARTH_GRAVITY           9810                /* [mm/s^2] gravitational acceleration */

/* controller parameters */
#define COMMAND_THRESHOLD       35                  /* [iterations] max iterations to wait */
#define MAX_ROLL_PITCH          100                 /* [mrad] */

/* scaling constants */
#define FACTOR_EULER_ANGLE      2000.0*M_PI/65536.0 /* [units]  --> [mrad] (2*PI*1000 mrad/2^16) */
#define FACTOR_ANGULAR_RATE     8500.0/32768.0      /* [units]  --> [mrad/s] */
#define FACTOR_ANGULAR_ACCEL    8500.0/32768.0*76.3 /* [units]  --> [mrad/s^2] */
#define FACTOR_LINEAR_ACCEL     9810.0/4681.0       /* [units]  --> [mm/s^2] (4681=32768000/7000) */
#define FACTOR_SONAR            2000.0/1024.0       /* [0-5V]   --> [0-2000mm] */
#define FACTOR_BATTERY          18000.0/1024.0      /* [0-5V]   --> [0-18000mV] */
#define FACTOR_PARAMETER        0.001               /* [rad]    --> [mrad] */

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
static int motor_revving_add = 50;
static int base_motor_speed  = 8000;
static int revving_step      = 0;

/* controller objects */
static struct controller ctrl_roll;
static struct controller ctrl_pitch;
static struct controller ctrl_yaw;
static struct controller ctrl_z;
#ifdef ENABLE_POSITION_CONTROLLERS
static struct controller ctrl_x;
static struct controller ctrl_y;
#endif
/* filter objects */
static low_pass_filter_t    filter_sonar;
static low_pass_filter_t    filter_ddz;
static low_pass_filter_t    filter_cmd_roll;
static low_pass_filter_t    filter_cmd_pitch;
static low_pass_filter_t    filter_cmd_z;
static median_filter_t      filter_battery;
static kalman_filter_t      filter_dz;

/* sensor and input data */
static command_data_t   command_data;
static javiator_data_t  javiator_data;
static inertial_data_t  inertial_data;
static sensor_data_t    sensor_data;
static motor_signals_t  motor_signals;
static command_data_t   motor_offsets;
static trace_data_t     trace_data;

static double sin_roll  = 0;
static double cos_roll  = 0;
static double sin_pitch = 0;
static double cos_pitch = 0;
static double sin_yaw   = 0;
static double cos_yaw   = 0;
#ifdef ENABLE_POSITION_CONTROLLERS
static double desired_x = 0;
static double desired_y = 0;
#endif
/* function pointer */
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


static int loop_count = 0;
static long long stats[ NUM_STATS ] = {0,0,0,0,0,0,0,0,0};
static long long max_stats[ NUM_STATS ] = {0,0,0,0,0,0,0,0,0};
static char *stats_name[ NUM_STATS ] =
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

static inline double do_control( struct controller *ctrl,
                                 double current_angle,
                                 double desired_angle,
                                 double angular_velocity,
                                 double angular_acceleration )
{
    return ctrl->control( ctrl, current_angle, desired_angle,
        angular_velocity, angular_acceleration );
}

int control_loop_setup( int period, int control_z )
{
    struct sigaction act;

    running          = 1;
    ms_period        = period;
    us_period        = period * 1000;
    compute_z        = control_z;
    controller_state = 0;
    altitude_mode    = ALT_MODE_GROUND;
    next_period      = 0;
    uz_old           = 0;
    act.sa_handler   = signal_handler;
	act.sa_handler   = int_handler;

    if( sigaction( SIGUSR1, &act, NULL ) || sigaction( SIGINT, &act, NULL ) )
    {
        perror( "sigaction" );
    }

    controller_init( &ctrl_roll,  "Roll",  CTRL_PIDD,     ms_period );
    controller_init( &ctrl_pitch, "Pitch", CTRL_PIDD,     ms_period );
    controller_init( &ctrl_yaw,   "Yaw",   CTRL_PIDD_YAW, ms_period );
    controller_init( &ctrl_z,     "Z",     CTRL_PIDD,     ms_period );
#ifdef ENABLE_POSITION_CONTROLLERS
    controller_init( &ctrl_x,     "X",     CTRL_PIDD,     ms_period );
    controller_init( &ctrl_y,     "Y",     CTRL_PIDD,     ms_period );
#endif
    low_pass_filter_init( &filter_sonar,     FILTER_GAIN_SONAR );
    low_pass_filter_init( &filter_ddz,       FILTER_GAIN_DDZ );
    low_pass_filter_init( &filter_cmd_roll,  FILTER_GAIN_CMD );
    low_pass_filter_init( &filter_cmd_pitch, FILTER_GAIN_CMD );
    low_pass_filter_init( &filter_cmd_z,     FILTER_GAIN_CMD );
    median_filter_init  ( &filter_battery,   FILTER_SIZE_BATTERY );
    kalman_filter_init  ( &filter_dz,        ms_period );

    memset( &command_data,  0, sizeof( command_data ) );
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &inertial_data, 0, sizeof( inertial_data ) );
    memset( &sensor_data,   0, sizeof( sensor_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );
    memset( &motor_offsets, 0, sizeof( motor_offsets ) );
    memset( &trace_data,    0, sizeof( trace_data ) );

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
#ifdef ENABLE_POSITION_CONTROLLERS
        set_control_params( &params, &ctrl_x, &ctrl_y );
#endif
    }
}

static void adjust_yaw_and_z( void )
{
    static int16_t offset_yaw = 0;
    static int16_t offset_z   = 0;

    if( motor_signals.front == 0 && motor_signals.right == 0 &&
        motor_signals.rear  == 0 && motor_signals.left  == 0 )
    {
        offset_yaw      = sensor_data.yaw;
        offset_z        = sensor_data.z;
        sensor_data.yaw = 0;
        sensor_data.z   = 0;
    }
    else
    {
        sensor_data.yaw -= offset_yaw;
        sensor_data.z   -= offset_z;
    }
}

static int get_javiator_data( void )
{
    static uint16_t last_id = 0;
    static int16_t  old_z   = 13;
    static int      count   = 2;
    int             res;

    res = javiator_port_get_data( &javiator_data );

    if( res )
    {
        fprintf( stderr, "ERROR: data from JAviator not available\n" );
        return( res );
    }

    /* check for lost JAviator packets */
    if( javiator_data.id != (uint16_t)( last_id + 1 ) )
    {
        fprintf( stderr, "WARNING: lost %d JAviator packet(s): last ID %u != received ID %u\n",
            javiator_data.id - (uint16_t)( last_id + 1 ), last_id, javiator_data.id );
    }

    last_id = javiator_data.id;

    /* reject possible sonar outliers */
    if( abs( old_z - javiator_data.sonar ) > 20 && count < 2 )
    {
        javiator_data.sonar = old_z;
        ++count;
    }
    else
    {
        count = 0;
    }

    old_z = javiator_data.sonar;

    /* save old positions */
    sensor_data.dx = sensor_data.x;
    sensor_data.dy = sensor_data.y;
    sensor_data.dz = sensor_data.z;

    /* copy and scale positions */
    sensor_data.x = (int)( atoi( (const char *) javiator_data.x_pos ) * cos_yaw * cos_roll );
    sensor_data.y = (int)( atoi( (const char *) javiator_data.y_pos ) * cos_yaw * cos_pitch );
    sensor_data.z = (int)( javiator_data.sonar * FACTOR_SONAR );

    /* compute linear rates */
    sensor_data.dx = sensor_data.x - sensor_data.dx;
    sensor_data.dy = sensor_data.y - sensor_data.dy;
    sensor_data.dz = sensor_data.z - sensor_data.dz;

    /* scale and filter battery level */
    sensor_data.battery = (int) median_filter_apply( &filter_battery,
        javiator_data.battery * FACTOR_BATTERY );

    return( 0 );
}

static int get_inertial_data( void )
{
    static int16_t   last_ticks = 0;
    int              res;

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

    last_ticks = inertial_data.ticks;

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

    return( 0 );
}

static int get_command_data( void )
{
    static int sensors_enabled = 0;
    double yaw_rotated_roll;
    double yaw_rotated_pitch;
#ifdef ENABLE_POSITION_CONTROLLERS
    int16_t x_controlled_roll;
    int16_t y_controlled_pitch;
#endif
    if( terminal_port_is_shut_down( ) )
    {
        altitude_mode = ALT_MODE_SHUTDOWN;
        terminal_port_reset_shut_down( );

        if( sensors_enabled )
        {
            javiator_port_send_enable_sensors( 0 );
			inertial_port_send_stop( );
            sensors_enabled = 0;
        }
    }
    else
    if( terminal_port_is_mode_switch( ) )
    {
		printf( "Mode Switch...\n" );
		print_stats( );
		loop_count = 0;
		memset( stats, 0, sizeof( stats ) );
		memset( max_stats, 0, sizeof( max_stats ) );

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
					inertial_port_send_start( );
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
        terminal_port_get_command_data( &command_data );
    }
#ifdef ENABLE_POSITION_CONTROLLERS
#if 0
    if( terminal_port_is_test_mode( ) )
    {
        command_data.roll  = (int16_t) -do_control( &ctrl_x, sensor_data.x, desired_x, sensor_data.dx, 0 );
        command_data.pitch = (int16_t)  do_control( &ctrl_y, sensor_data.y, desired_y, sensor_data.dy, 0 );

        if( command_data.roll > MAX_ROLL_PITCH )
        {
            command_data.roll = MAX_ROLL_PITCH;
        }
        else
        if( command_data.roll < -MAX_ROLL_PITCH )
        {
            command_data.roll = -MAX_ROLL_PITCH;
        }

        if( command_data.pitch > MAX_ROLL_PITCH )
        {
            command_data.pitch = MAX_ROLL_PITCH;
        }
        else
        if( command_data.pitch < -MAX_ROLL_PITCH )
        {
            command_data.pitch = -MAX_ROLL_PITCH;
        }
    }
    else
    {
        desired_x = sensor_data.x;
        desired_y = sensor_data.y;
    }
#endif
    if( terminal_port_is_test_mode( ) )
    {
        x_controlled_roll  = (int16_t) -do_control( &ctrl_x, sensor_data.x, desired_x, sensor_data.dx, 0 );
        y_controlled_pitch = (int16_t)  do_control( &ctrl_y, sensor_data.y, desired_y, sensor_data.dy, 0 );

        if( x_controlled_roll > MAX_ROLL_PITCH )
        {
            x_controlled_roll = MAX_ROLL_PITCH;
        }
        else
        if( x_controlled_roll < -MAX_ROLL_PITCH )
        {
            x_controlled_roll = -MAX_ROLL_PITCH;
        }

        if( y_controlled_pitch > MAX_ROLL_PITCH )
        {
            y_controlled_pitch = MAX_ROLL_PITCH;
        }
        else
        if( y_controlled_pitch < -MAX_ROLL_PITCH )
        {
            y_controlled_pitch = -MAX_ROLL_PITCH;
        }

        command_data.roll  += x_controlled_roll;
        command_data.pitch += y_controlled_pitch;
    }
    else
    {
        desired_x = sensor_data.x;
        desired_y = sensor_data.y;
    }
#endif
    /* apply low-pass filtering to roll, pitch, and z commands */
    command_data.roll  = (int16_t) low_pass_filter_apply( &filter_cmd_roll,  command_data.roll );
    command_data.pitch = (int16_t) low_pass_filter_apply( &filter_cmd_pitch, command_data.pitch );
    command_data.z     = (int16_t) low_pass_filter_apply( &filter_cmd_z,     command_data.z );

    /* apply rotation matrix to roll and pitch commands */
    yaw_rotated_roll   = command_data.roll * cos_yaw + command_data.pitch * sin_yaw;
    yaw_rotated_pitch  = command_data.roll * sin_yaw - command_data.pitch * cos_yaw;

    /* replace original commands with rotated commands */
    command_data.roll  = (int16_t)  yaw_rotated_roll;
    command_data.pitch = (int16_t) -yaw_rotated_pitch;

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
#ifdef ENABLE_POSITION_CONTROLLERS
    ctrl_x     .reset_zero( &ctrl_x );
    ctrl_y     .reset_zero( &ctrl_y );
#endif
    uz_old = 0;
}

static inline void reset_filters( void )
{
    low_pass_filter_reset( &filter_sonar );
    low_pass_filter_reset( &filter_ddz );
    low_pass_filter_reset( &filter_cmd_roll );
    low_pass_filter_reset( &filter_cmd_pitch );
    low_pass_filter_reset( &filter_cmd_z );
    kalman_filter_reset  ( &filter_dz );
}

static inline void reset_motor_signals( void )
{
    motor_signals.front = 0;
    motor_signals.right = 0;
    motor_signals.rear  = 0;
    motor_signals.left  = 0;
}

static int perform_shut_down( void )
{
    controller_state = 0;
    revving_step     = 0;
    base_motor_speed = terminal_port_get_base_motor_speed( );

    reset_controllers( );
    reset_filters( );
    reset_motor_signals( );

    return( 1 );
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
         perform_shut_down( );
    }

    return( 1 );
}

static inline double get_filtered_sonar( void )
{
    double vector_sonar =
          SONAR_POS_ROLL  * sin_pitch *   -1
        + SONAR_POS_PITCH * cos_pitch * sin_roll
        + sensor_data.z   * cos_pitch * cos_roll;

    return low_pass_filter_apply( &filter_sonar, vector_sonar );
}

static inline double get_filtered_ddz( void )
{
    double vector_ddz =
          sensor_data.ddx * sin_pitch *   -1
        + sensor_data.ddy * cos_pitch * sin_roll
        + sensor_data.ddz * cos_pitch * cos_roll
        + EARTH_GRAVITY;

    return low_pass_filter_apply( &filter_ddz, vector_ddz );
}

static int compute_motor_signals( void )
{
    double filtered_z   = get_filtered_sonar( );
    double filtered_ddz = get_filtered_ddz( );
    double estimated_dz = kalman_filter_apply( &filter_dz, filtered_z, -filtered_ddz );
    double estimated_z  = filter_dz.z;
    double uroll        = 0;
    double upitch       = 0;
    double uyaw         = 0;
    double uz_new       = 0;
    int i, signals[4];

    if( revving_step < (base_motor_speed / motor_revving_add) )
    {
        uz_new = uz_old + motor_revving_add;
        ++revving_step;
    }
    else
    {
        if( command_data.z < 10 && controller_get_integral( &ctrl_z ) > 0 )
        {
            controller_set_integral( &ctrl_z, controller_get_integral( &ctrl_z ) - 10 );
        }

        uroll  = do_control( &ctrl_roll,
            sensor_data.roll, command_data.roll, sensor_data.droll, sensor_data.ddroll );

        upitch = do_control( &ctrl_pitch,
            sensor_data.pitch, command_data.pitch, sensor_data.dpitch, sensor_data.ddpitch );

        uyaw   = do_control( &ctrl_yaw,
            sensor_data.yaw, command_data.yaw, sensor_data.dyaw, sensor_data.ddyaw );

        if( compute_z )
	    {
		    uz_new = base_motor_speed + do_control( &ctrl_z,
                estimated_z, command_data.z , estimated_dz, filtered_ddz );
	    }
        else
        {
            uz_new = command_data.z;
        }
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
    trace_data.z_filtered   = (int16_t)( filtered_z * 1000.0 );
    trace_data.z_estimated  = (int16_t)( estimated_z * 1000.0 );
    trace_data.dz_estimated = (int16_t)( estimated_dz * 1000.0 );
    trace_data.ddz          = (int16_t)( sensor_data.ddz );
    trace_data.ddz_filtered = (int16_t)( filtered_ddz * 1000.0 );
    trace_data.p_term       = (int16_t)( controller_get_p_term( &ctrl_z ) * 1000 );
    trace_data.i_term       = (int16_t)( controller_get_i_term( &ctrl_z ) * 1000);
    trace_data.d_term       = (int16_t)( controller_get_d_term( &ctrl_z ) * 1000);
    trace_data.dd_term      = (int16_t)( controller_get_dd_term( &ctrl_z ) * 1000);
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

static int wait_for_next_period( void )
{
    if( sleep_until( next_period ) )
    {
        fprintf( stderr, "error in sleep_until\n" );
        exit( 1 );
    }

    next_period += us_period;
    return( 0 );
}

void calc_stats( long long time, int id )
{
    stats[id] += time;

    if( loop_count > 10 && time > max_stats[ id ] )
    { 
		/* drop the first few stats for max calculation */
        max_stats[ id ] = time;
    }
}

static void print_stats( void )
{
    int i;

    if( loop_count )
    {
        fprintf( stdout, "Loop Statistics:\n" );

        for( i = 0; i < NUM_STATS; ++i )
        {
            fprintf( stdout, "\t%s %8lld us\tmax %8lld us\n",
                stats_name[ i ], stats[ i ] / loop_count, max_stats[ i ] );
        }
    }
}

static void signal_handler( int num )
{
    print_stats( );
}

static void int_handler( int num )
{
	running = 0;
}

static int read_sensors( void )
{
	uint64_t start, end;

	start = get_utime( );

	if( get_javiator_data( ) )
    {
        fprintf( stderr, "ERROR: connection to JAviator broken\n" );
		return( -1 );
	}

	end = get_utime( );
	calc_stats( end - start, STAT_FROM_JAV );

	start = get_utime( );

    if( get_inertial_data( ) )
    {
        fprintf( stderr, "ERROR: connection to IMU broken\n" );
        return( -1 );
    }

    inertial_port_send_request( );

    end = get_utime( );
    calc_stats( end - start, STAT_IMU );

	/* IMPORTANT: yaw angle must be adjusted BEFORE
	   computation of sine/cosine values */
	adjust_yaw_and_z( );

	/* compute sine/cosine values */
	sin_roll  = sin( sensor_data.roll / 1000.0 );
	cos_roll  = cos( sensor_data.roll / 1000.0 );
	sin_pitch = sin( sensor_data.pitch / 1000.0 );
	cos_pitch = cos( sensor_data.pitch / 1000.0 );
	sin_yaw   = sin( sensor_data.yaw / 1000.0 );
	cos_yaw   = cos( sensor_data.yaw / 1000.0 );

	return( 0 );
}

/* the control loop for our helicopter */
int control_loop_run( void )
{
    int first_time = 1;
    next_period    = get_utime( ) + us_period;
    altitude_mode  = ALT_MODE_GROUND;
    long long start, end;
	long long loop_start;

	inertial_port_send_request( );

    while( running )
    {
        start = loop_start = get_utime( );

        if( send_motor_signals( ) )
        {
            break;
        }

        end = get_utime( );
        calc_stats( end - start, STAT_TO_JAV );

        if( read_sensors( ) )
        {
            altitude_mode = ALT_MODE_SHUTDOWN;
            perform_shut_down( );
            break;
        }

        start = get_utime( );

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

        end = get_utime( );
        calc_stats( end - start, STAT_FROM_TERM );

        start = get_utime( );
 
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

        end = get_utime( );
        calc_stats( end - start, STAT_CONTROL );

        start = get_utime( );

        send_report_to_terminal( );
        send_trace_data_to_terminal( );

        end = get_utime( );
        calc_stats( end - start, STAT_TO_TERM );

        calc_stats( end - loop_start, STAT_ALL );

        start = get_utime( );

        wait_for_next_period( );

        end = get_utime( );
        calc_stats( end - start, STAT_SLEEP );

        if( ++loop_count < 0 )
        {
            fprintf( stderr, "WARNING: stats overrun\n" );
            loop_count = 0;
            memset( stats, 0, sizeof( stats ) );
        }
    }

    controller_destroy( &ctrl_roll );
    controller_destroy( &ctrl_pitch );
    controller_destroy( &ctrl_yaw );
    controller_destroy( &ctrl_z );
#ifdef ENABLE_POSITION_CONTROLLERS
    controller_destroy( &ctrl_x );
    controller_destroy( &ctrl_y );
#endif
    median_filter_destroy( &filter_battery );

    print_stats( );

    return( 0 );
}

int control_loop_stop( void )
{
    running = 0;
    return( 0 );
}

/* End of file */
