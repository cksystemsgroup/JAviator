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
#include "rev_params.h"
#include "trace_data.h"
#include "low_pass_filter.h"
#include "median_filter.h"
#include "kalman_filter.h"
#include "us_timer.h"

//#define APPLY_AUTOMATIC_REVVING_UP_AND_DOWN

/* controller modes */
#define ALT_MODE_GROUND         0x00
#define ALT_MODE_FLYING         0x01
#define ALT_MODE_SHUTDOWN       0x02

/* filter parameters */
#define FILTER_GAIN_Z           0.9
#define FILTER_GAIN_DDZ         0.1
#define FILTER_GAIN_CMD         0.1
#define FILTER_SIZE_BATTERY     15

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
#define FACTOR_TRACE_DATA       1000.0              /* [value]  --> [value*1000] */

/* control loop parameters */
static volatile int             running;
static double                   period;             /* [s] */
static int                      us_period;          /* [us] */
static int                      control_z;
static int                      controller_state;
static int                      altitude_mode;
static long long                next_period;
static double                   uz_old;

/* motor speed up threshold */
#ifndef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
static int                      motor_revving_add = 25;
static int                      base_motor_speed  = 8300;
static int                      revving_step      = 0;
#else
static int16_t                  idle_speed = 0;
#endif

/* controller objects */
static struct controller        ctrl_roll;
static struct controller        ctrl_pitch;
static struct controller        ctrl_yaw;
static struct controller        ctrl_z;

/* filter objects */
static low_pass_filter_t        filter_z;
static low_pass_filter_t        filter_ddz;
static low_pass_filter_t        filter_cmd_roll;
static low_pass_filter_t        filter_cmd_pitch;
static low_pass_filter_t        filter_cmd_z;
static median_filter_t          filter_battery;
static kalman_filter_t          filter_dz;

/* sensor and input data */
static command_data_t           command_data;
static javiator_data_t          javiator_data;
static inertial_data_t          inertial_data;
static sensor_data_t            sensor_data;
static motor_signals_t          motor_signals;
static motor_offsets_t          motor_offsets;
static rev_params_t             rev_params;
static trace_data_t             trace_data;

/* sine/cosine variables */
static double                   sin_roll;
static double                   cos_roll;
static double                   sin_pitch;
static double                   cos_pitch;
static double                   sin_yaw;
static double                   cos_yaw;

/* statistics definitions */
#define STAT_IMU                0
#define STAT_TO_JAV             1
#define STAT_FROM_JAV           2
#define STAT_FROM_TERM          3
#define STAT_TO_TERM            4
#define STAT_CONTROL            5
#define STAT_SLEEP              6
#define STAT_READ               7
#define STAT_ALL                8
#define NUM_STATS               9

/* statistics variables */
static int                      loop_count;
static long long                stats[ NUM_STATS ];
static long long                max_stats[ NUM_STATS ];
static const char *             stats_name[ NUM_STATS ] =
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

/* function pointers */
static void signal_handler( int num );
static void int_handler( int num );
static void print_stats( void );


/****************************************
 *    control loop code                 *
 ****************************************/

int control_loop_setup( int ms_period, int enable_z )
{
    struct sigaction act;

    running           = 1;
    period            = ms_period * 0.001;
    us_period         = ms_period * 1000;
    control_z         = enable_z;
    controller_state  = 0;
    altitude_mode     = ALT_MODE_GROUND;
    next_period       = 0;
    uz_old            = 0;
#ifndef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
    motor_revving_add = 25;
    base_motor_speed  = 8300;
    revving_step      = 0;
#else
    idle_speed        = 0;
#endif
    loop_count        = 0;
    act.sa_handler    = signal_handler;

    controller_init( &ctrl_roll,  "Roll",  CTRL_PIDD,     period );
    controller_init( &ctrl_pitch, "Pitch", CTRL_PIDD,     period );
    controller_init( &ctrl_yaw,   "Yaw",   CTRL_PIDD_YAW, period );
    controller_init( &ctrl_z,     "Z",     CTRL_PIDD,     period );

    low_pass_filter_init( &filter_z,         FILTER_GAIN_Z );
    low_pass_filter_init( &filter_ddz,       FILTER_GAIN_DDZ );
    low_pass_filter_init( &filter_cmd_roll,  FILTER_GAIN_CMD );
    low_pass_filter_init( &filter_cmd_pitch, FILTER_GAIN_CMD );
    low_pass_filter_init( &filter_cmd_z,     FILTER_GAIN_CMD );
    median_filter_init  ( &filter_battery,   FILTER_SIZE_BATTERY );
    kalman_filter_init  ( &filter_dz,        period );

    memset( &command_data,  0, sizeof( command_data ) );
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &inertial_data, 0, sizeof( inertial_data ) );
    memset( &sensor_data,   0, sizeof( sensor_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );
    memset( &motor_offsets, 0, sizeof( motor_offsets ) );
    memset( &rev_params,    0, sizeof( rev_params ) );
    memset( &trace_data,    0, sizeof( trace_data ) );
    memset( &stats,         0, sizeof( stats ) );
    memset( &max_stats,     0, sizeof( max_stats ) );

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
        ctrl_1->set_params( ctrl_1, params->kp, params->ki, params->kd, params->kdd );
        fprintf( stdout, "parameter update: %s", ctrl_1->name );

        if( ctrl_2 != NULL )
        {
            ctrl_2->set_params( ctrl_2, params->kp, params->ki, params->kd, params->kdd );
            fprintf( stdout, "/%s", ctrl_2->name );
        }

        fprintf( stdout, "\n-->\tKp: %+1.3f\tKi: %+1.3f\tKd: %+1.3f\tKdd: %+1.3f\n",
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

static void get_revving_params( void )
{
    if( terminal_port_is_new_rev_params( ) )
    {
        terminal_port_get_rev_params( &rev_params );
        fprintf( stdout, "parameter update: Revving\n"
            "-->\tIL: %1.3f\tCS: %1.3f\tRu: %1.3f\tRd: %1.3f\n",
            FACTOR_PARAMETER * rev_params.idle_limit,
            FACTOR_PARAMETER * rev_params.ctrl_speed,
            FACTOR_PARAMETER * rev_params.rev_up_inc,
            FACTOR_PARAMETER * rev_params.rev_dn_dec );
        fflush( stdout );
#ifndef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
        base_motor_speed = rev_params.idle_limit;
#endif
    }
}

static double reject_z_outliers( double new_z )
{
    static double old_z = 13;
    static int    count = 2;

    if( abs( old_z - new_z ) > 20 && count < 2 )
    {
        new_z = old_z;
        ++count;
    }
    else
    {
        old_z = new_z;
        count = 0;
    }

    return( new_z );
}

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

static int get_javiator_data( void )
{
    static uint16_t last_id = 0;
    int res;

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
    sensor_data.z  = reject_z_outliers( javiator_data.sonar * FACTOR_SONAR );

    /* compute linear rates */
    sensor_data.dx = (sensor_data.x - sensor_data.dx) * FACTOR_LINEAR_RATE;
    sensor_data.dy = (sensor_data.y - sensor_data.dy) * FACTOR_LINEAR_RATE;
    sensor_data.dz = (sensor_data.z - sensor_data.dz) * FACTOR_LINEAR_RATE;

    /* scale and filter battery level */
    sensor_data.battery = median_filter_apply( &filter_battery,
        javiator_data.battery * FACTOR_BATTERY );

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

static int get_command_data( void )
{
    static int sensors_enabled = 0;
    double yaw_rotated_roll;
    double yaw_rotated_pitch;

    if( terminal_port_is_shut_down( ) )
    {
        altitude_mode = ALT_MODE_SHUTDOWN;
        terminal_port_reset_shut_down( );

        if( sensors_enabled )
        {
            sensors_enabled = 0;
            javiator_port_send_enable_sensors( 0 );

			if( inertial_is_local( ) )
            {
				inertial_port_send_stop( );
            }
        }
    }
    else
    if( terminal_port_is_mode_switch( ) )
    {
		printf( "Mode Switch ...\n" );
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
                    sensors_enabled = 1;
                    javiator_port_send_enable_sensors( 1 );

					if( inertial_is_local( ) )
                    {
						inertial_port_send_start( );
                    }
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

    /* apply low-pass filtering to roll, pitch, and z commands */
    command_data.roll  = low_pass_filter_apply( &filter_cmd_roll,  command_data.roll );
    command_data.pitch = low_pass_filter_apply( &filter_cmd_pitch, command_data.pitch );
    command_data.z     = low_pass_filter_apply( &filter_cmd_z,     command_data.z );

    /* apply rotation matrix to roll and pitch commands */
    yaw_rotated_roll   = command_data.roll * cos_yaw + command_data.pitch * sin_yaw;
    yaw_rotated_pitch  = command_data.roll * sin_yaw - command_data.pitch * cos_yaw;

    /* replace original commands with rotated commands */
    command_data.roll  =  yaw_rotated_roll;
    command_data.pitch = -yaw_rotated_pitch;

    /* check for new control parameters */
    get_control_params( );

    /* check for new revving parameters */
    get_revving_params( );

    return( 0 );
}

static void reset_controllers( void )
{
    ctrl_roll  .reset_zero( &ctrl_roll );
    ctrl_pitch .reset_zero( &ctrl_pitch );
    ctrl_yaw   .reset_zero( &ctrl_yaw );
    ctrl_z     .reset_zero( &ctrl_z );
}

static void reset_filters( void )
{
    low_pass_filter_reset( &filter_ddz );
    low_pass_filter_reset( &filter_cmd_roll );
    low_pass_filter_reset( &filter_cmd_pitch );
    low_pass_filter_reset( &filter_cmd_z );
    kalman_filter_reset  ( &filter_dz );
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
#ifndef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
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
#endif
        reset_controllers( );
        reset_filters( );
        reset_motor_signals( );
        controller_state = 0;
#ifdef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
        idle_speed = 0;
#else
    }
#endif

    return( 1 );
}

static int perform_shut_down( void )
{
    controller_state = 0;
#ifndef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN
    revving_step     = 0;
#else
    idle_speed = 0;
#endif

    reset_controllers( );
    reset_filters( );
    reset_motor_signals( );

    return( 1 );
}

static inline double do_control( struct controller *ctrl,
    double current, double desired, double velocity, double acceleration )
{
    return ctrl->control( ctrl, current, desired, velocity, acceleration );
}

static inline double get_filtered_z( void )
{
    double corrected_z = SONAR_POS_ROLL  * sin_pitch *   -1
                       + SONAR_POS_PITCH * cos_pitch * sin_roll
                       + sensor_data.z   * cos_pitch * cos_roll;

    return low_pass_filter_apply( &filter_z, corrected_z );
}

static inline double get_filtered_ddz( void )
{
    double corrected_ddz = sensor_data.ddx * sin_pitch *   -1
                         + sensor_data.ddy * cos_pitch * sin_roll
                         + sensor_data.ddz * cos_pitch * cos_roll
                         + GRAVITY;

    return low_pass_filter_apply( &filter_ddz, corrected_ddz );
}

static int compute_motor_signals( void )
{
    double filtered_z   = 0;    /* low-pass-filtered z */
    double estimated_z  = 0;    /* Kalman-estimated z */
    double estimated_dz = 0;    /* Kalman-estimated dz */
    double filtered_ddz = 0;    /* low-pass-filtered ddz */
    double uroll        = 0;
    double upitch       = 0;
    double uyaw         = 0;
    double uz_new       = 0;
    int i, signals[4];

    filtered_z   = get_filtered_z( );
    filtered_ddz = get_filtered_ddz( );
    estimated_dz = kalman_filter_apply( &filter_dz, filtered_z, filtered_ddz );
    estimated_z  = filter_dz.z;

#ifdef APPLY_AUTOMATIC_REVVING_UP_AND_DOWN

    if( control_z )
    {
        if( command_data.z > ZERO_JITTER )
        {
            if( idle_speed < rev_params.idle_limit )
            {
                idle_speed += rev_params.rev_up_inc;
            }
            else
            {
                uz_new = do_control( &ctrl_z,
                    estimated_z, command_data.z,
                    estimated_dz, -filtered_ddz );
            }
        }
        else
        {   
            if( idle_speed >= rev_params.idle_limit && estimated_z > ZERO_JITTER )
            {
                uz_new = do_control( &ctrl_z,
                    estimated_z, command_data.z,
                    estimated_dz, -filtered_ddz );
            }
            else
            if( idle_speed > 0 )
            {
                idle_speed -= rev_params.rev_dn_dec;
            }
        }
    }
    else
    {
        idle_speed = (int16_t)( command_data.z * MOTOR_MAX );
    }

    if( idle_speed > rev_params.ctrl_speed )
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

    uz_new += idle_speed / (cos_roll * cos_pitch);

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
		    uz_new = base_motor_speed / (cos_roll * cos_pitch);
		    uz_new += do_control( &ctrl_z,
                estimated_z, command_data.z,
                estimated_dz, -filtered_ddz );
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

    trace_data.z            = (int16_t)( FACTOR_TRACE_DATA * sensor_data.z );
    trace_data.filtered_z   = (int16_t)( FACTOR_TRACE_DATA * filtered_z );
    trace_data.estimated_z  = (int16_t)( FACTOR_TRACE_DATA * estimated_z );
    trace_data.estimated_dz = (int16_t)( FACTOR_TRACE_DATA * estimated_dz );
    trace_data.ddz          = (int16_t)( FACTOR_TRACE_DATA * (sensor_data.ddz + GRAVITY) );
    trace_data.filtered_ddz = (int16_t)( FACTOR_TRACE_DATA * filtered_ddz );
    trace_data.p_term       = (int16_t)( controller_get_p_term( &ctrl_z ) );
    trace_data.i_term       = (int16_t)( controller_get_i_term( &ctrl_z ) );
    trace_data.d_term       = (int16_t)( controller_get_d_term( &ctrl_z ) );
    trace_data.dd_term      = (int16_t)( controller_get_dd_term( &ctrl_z ) );
    trace_data.uz           = (int16_t)( uz_new );
    trace_data.cmd_z        = (int16_t)( FACTOR_TRACE_DATA * command_data.z );
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

    /* drop first few stats for max calculation */
    if( loop_count > 10 && time > max_stats[id] )
    {
        max_stats[id] = time;
    }
}

static void print_stats( void )
{
    int i;

    if( loop_count > 0 )
    {
        printf( "Loop Statistics:\n" );

        for( i = 0; i < NUM_STATS; ++i )
        {
            printf( "\t%s %8lld us\tmax %8lld us\n",
                stats_name[i], stats[i] / loop_count, max_stats[i] );
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
	long long start, end;

    start = get_utime( );

	if( get_javiator_data( ) )
    {
        fprintf( stderr, "ERROR: connection to JAviator broken\n" );
        return( -1 );
	}

	end = get_utime( );
	calc_stats( end - start, STAT_FROM_JAV );

	if( inertial_is_local( ) )
    {
		start = get_utime( );

		if( get_inertial_data( ) )
		{
			fprintf( stderr, "ERROR: connection to IMU broken\n" );
			return( -1 );
		}
		else
		{
			inertial_port_send_request( );
		}

		end = get_utime( );
		calc_stats( end - start, STAT_IMU );
	}

	/* IMPORTANT: yaw angle must be adjusted BEFORE
	   computation of sine/cosine values */
	adjust_yaw( );
	adjust_z( );

	/* compute sine/cosine values */
	sin_roll  = sin( sensor_data.roll );
	cos_roll  = cos( sensor_data.roll );
	sin_pitch = sin( sensor_data.pitch );
	cos_pitch = cos( sensor_data.pitch );
	sin_yaw   = sin( sensor_data.yaw );
	cos_yaw   = cos( sensor_data.yaw );

	return( 0 );
}

int control_loop_run( void )
{
    int first_time = 1;
    next_period    = get_utime( ) + us_period;
    altitude_mode  = ALT_MODE_GROUND;
    long long start, end;
	long long loop_start;

	if( inertial_is_local( ) )
    {
		inertial_port_send_request( );
    }

    while( running )
    {
        start = get_utime( );
        loop_start = start;

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
            printf( "WARNING: statistics overrun\n" );
            loop_count = 0;
            memset( stats, 0, sizeof( stats ) );
        }
    }

    controller_destroy( &ctrl_roll );
    controller_destroy( &ctrl_pitch );
    controller_destroy( &ctrl_yaw );
    controller_destroy( &ctrl_z );

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
