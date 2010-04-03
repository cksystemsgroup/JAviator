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

#include "control_loop.h"
#include "communication.h"
#include "comm_channel.h"
#include "protocol.h"
#include "javiator_port.h"
#include "terminal_port.h"
#include "ubisense_port.h"
#include "command_data.h"
#include "ctrl_params.h"
#include "javiator_data.h"
#include "sensor_data.h"
#include "motor_signals.h"
#include "motor_offsets.h"
#include "trace_data.h"
#include "controller.h"
#include "average_filter.h"
#include "kalman_filter.h"
#include "low_pass_filter.h"
#include "median_filter.h"
#include "outlier_filter.h"
#include "extended_kalman.h"
#include "transformation.h"
#include "us_timer.h"

/* controller modes */
#define CTRL_MODE_GROUND        0x01
#define CTRL_MODE_FLYING        0x02
#define CTRL_MODE_SHUTDOWN      0x04

/* controller states */
#define CTRL_STATE_NORMAL       0x01
#define CTRL_STATE_TESTING      0x02

/* filter parameters */
#define FILTER_MDIFF_X_Y        250                     /* [mm] maximum allowed x/y-difference */
#define FILTER_LIMIT_X_Y        2                       /* limit for counting x/y-outliers */
#define FILTER_MDIFF_Z          50                      /* [mm] maximum allowed z-difference */
#define FILTER_LIMIT_Z          2                       /* limit for counting z-outliers */
#define FILTER_GAIN_LACC        0.1                     /* gain for linear accelerations */
#define FILTER_GAIN_CCMD        0.1                     /* gain for control commands */
#define FILTER_SIZE_MAPS        1                       /* buffer size for pressure data */
#define FILTER_SIZE_TEMP        15                      /* buffer size for temperature data */
#define FILTER_SIZE_BATT        15                      /* buffer size for battery data */

/* plant parameters */
#define X_LASER_POS_ROLL        60                      /* [mm] x-laser position on roll axis */
#define X_LASER_POS_PITCH      -75                      /* [mm] x-laser position on pitch axis */
#define X_LASER_POS_YAW        -80                      /* [mm] x-laser position on yaw axis */
#define Y_LASER_POS_ROLL       -55                      /* [mm] y-laser position on roll axis */
#define Y_LASER_POS_PITCH       80                      /* [mm] y-laser position on pitch axis */
#define Y_LASER_POS_YAW        -80                      /* [mm] y-laser position on yaw axis */
#define Z_SONAR_POS_ROLL       -95                      /* [mm] z-sonar position on roll axis */
#define Z_SONAR_POS_PITCH      -95                      /* [mm] z-sonar position on pitch axis */
#define Z_SONAR_POS_YAW         55                      /* [mm] z-sonar position on yaw axis */
#define EARTH_GRAVITY           9810                    /* [mm/s^2] gravitational acceleration */

/* controller parameters */
#define COMMAND_THRESHOLD       35                      /* [iterations] max iterations to wait */
#define ROLL_PITCH_LIMIT        500                     /* [mrad] max allowed roll/pitch angle */
#define OFFSET_TEMPERATURE      700                     /* [mC] temperature calibration */
#define MOTOR_REVVING_STEP      50                      /* inc/dec for motor revving up/down */

/* scaling constants */
#define FACTOR_EULER_ANGLE      2000.0*M_PI/65536.0     /* [units] --> [mrad] (2*PI*1000 mrad/2^16) */
#define FACTOR_ANGULAR_VEL      8500.0/32768.0          /* [units] --> [mrad/s] */
#define FACTOR_ANGULAR_ACC      8500.0/32768.0*76.3     /* [units] --> [mrad/s^2] */
#define FACTOR_LINEAR_ACC       9810.0/4681.0           /* [units] --> [mm/s^2] (4681=32768000/7000) */
#define FACTOR_BMU_MAPS         115000000.0/16777216.0  /* [0-5V]  --> [115000000mPa] */
#define FACTOR_BMU_TEMP         10000.0/4096.0          /* [0-1V]  --> [0-10000cC] */
#define FACTOR_BMU_BATT         18000.0/1024.0          /* [0-5V]  --> [0-18000mV] */
#define FACTOR_SONAR            2000.0/1024.0           /* [0-5V]  --> [0-2000mm] */
#define FACTOR_PARAMETER        0.001                   /* [mrad]  --> [rad] */

/* control loop parameters */
static volatile int             running;
static double                   period;
static int                      us_period;
static int                      compute_z;
static int                      enable_ubisense;
static int                      altitude_mode;
static int                      control_state;
static long long                next_period;

/* motor speed parameters */
static int                      motor_speed_revving;
static int                      motor_speed_liftoff;

/* filter objects */
static outlier_filter_t         filter_x;
static outlier_filter_t         filter_y;
static outlier_filter_t         filter_z;
static kalman_filter_t          filter_dz;
static low_pass_filter_t        filter_ddx;
static low_pass_filter_t        filter_ddy;
static low_pass_filter_t        filter_ddz;
static low_pass_filter_t        filter_cmdr;
static low_pass_filter_t        filter_cmdp;
static low_pass_filter_t        filter_cmdz;
static average_filter_t         filter_maps;
static median_filter_t          filter_temp;
static median_filter_t          filter_batt;

/* controller objects */
static controller_t             ctrl_roll;
static controller_t             ctrl_pitch;
static controller_t             ctrl_yaw;
static controller_t             ctrl_x;
static controller_t             ctrl_y;
static controller_t             ctrl_z;

/* data structures */
static command_data_t           command_data;
static javiator_ldat_t          javiator_data;
static ubisense_data_t          ubisense_data;
static sensor_data_t            sensor_data;
static motor_signals_t          motor_signals;
static motor_offsets_t          motor_offsets;
static trace_data_t             trace_data;

/* controller statistics */
#define STAT_FROM_UBI           0
#define STAT_TO_JAV             1
#define STAT_FROM_JAV           2
#define STAT_FROM_TERM          3
#define STAT_TO_TERM            4
#define STAT_CONTROL            5
#define STAT_SLEEP              6
#define STAT_READ               7
#define STAT_ALL                8
#define NUM_STATS               9

static int                      loop_count = 0;
static long long                stats[ NUM_STATS ] = {0,0,0,0,0,0,0,0,0};
static long long                max_stats[ NUM_STATS ] = {0,0,0,0,0,0,0,0,0};
static char *                   stats_name[ NUM_STATS ] =
{
    "from Ubisense ",
    "to JAviator   ",
    "from JAviator ",
    "from Terminal ",
    "to Terminal   ",
    "control       ",
    "sleep time    ",
    "read time     ",
    "complete loop "
};

/* function pointers */
static void                     signal_handler( int num );
static void                     int_handler( int num );

/* forward declaration */
static void                     print_stats( void );


/****************************************
 *          Control Loop Code           *
 ****************************************/

int control_loop_setup( int ms_period, int control_z, int ubisense )
{
    struct sigaction act;

    running             = 1;
    period              = ms_period / 1000.0;
    us_period           = ms_period * 1000;
    compute_z           = control_z;
    enable_ubisense     = ubisense;
    altitude_mode       = CTRL_MODE_SHUTDOWN;
    control_state       = CTRL_STATE_NORMAL;
    next_period         = 0;
    motor_speed_revving = 0;
    motor_speed_liftoff = 0;
    act.sa_handler      = signal_handler;
	act.sa_handler      = int_handler;

    if( sigaction( SIGUSR1, &act, NULL ) || sigaction( SIGINT, &act, NULL ) )
    {
        perror( "sigaction" );
    }

    outlier_filter_init ( &filter_x,    "X",     FILTER_MDIFF_X_Y, FILTER_LIMIT_X_Y );
    outlier_filter_init ( &filter_y,    "Y",     FILTER_MDIFF_X_Y, FILTER_LIMIT_X_Y );
    outlier_filter_init ( &filter_z,    "Z",     FILTER_MDIFF_Z,   FILTER_LIMIT_Z );
    kalman_filter_init  ( &filter_dz,   "dZ",    period );
    low_pass_filter_init( &filter_ddx,  "ddX",   FILTER_GAIN_LACC );
    low_pass_filter_init( &filter_ddy,  "ddY",   FILTER_GAIN_LACC );
    low_pass_filter_init( &filter_ddz,  "ddZ",   FILTER_GAIN_LACC );
    low_pass_filter_init( &filter_cmdr, "cmdR",  FILTER_GAIN_CCMD );
    low_pass_filter_init( &filter_cmdp, "cmdP",  FILTER_GAIN_CCMD );
    low_pass_filter_init( &filter_cmdz, "cmdZ",  FILTER_GAIN_CCMD );
    average_filter_init ( &filter_maps, "MAPS",  FILTER_SIZE_MAPS );
    median_filter_init  ( &filter_temp, "TEMP",  FILTER_SIZE_TEMP );
    median_filter_init  ( &filter_batt, "BATT",  FILTER_SIZE_BATT );
    controller_init     ( &ctrl_roll,   "Roll",  CTRL_PIDD_DEF, period );
    controller_init     ( &ctrl_pitch,  "Pitch", CTRL_PIDD_DEF, period );
    controller_init     ( &ctrl_yaw,    "Yaw",   CTRL_PIDD_YAW, period );
    controller_init     ( &ctrl_x,      "X",     CTRL_PIDD_X_Y, period );
    controller_init     ( &ctrl_y,      "Y",     CTRL_PIDD_X_Y, period );
    controller_init     ( &ctrl_z,      "Z",     CTRL_PIDD_DEF, period );
    extended_kalman_init( period );
    transformation_init ( );

    memset( &command_data,  0, sizeof( command_data ) );
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &ubisense_data, 0, sizeof( ubisense_data ) );
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
    controller_t *ctrl_1, controller_t *ctrl_2 )
{
    double kp  = params->kp  * FACTOR_PARAMETER;
    double ki  = params->ki  * FACTOR_PARAMETER;
    double kd  = params->kd  * FACTOR_PARAMETER;
    double kdd = params->kdd * FACTOR_PARAMETER;

    if( ctrl_1 != NULL )
    {
        controller_set_params( ctrl_1, kp, ki, kd, kdd );
        fprintf( stdout, "parameter update: %s", ctrl_1->name );

        if( ctrl_2 != NULL )
        {
            controller_set_params( ctrl_2, kp, ki, kd, kdd );
            fprintf( stdout, "/%s", ctrl_2->name );
        }

        fprintf( stdout, "\n--> Kp: %+3.5f   Ki: %+3.5f   "
            "Kd: %+3.5f   Kdd: %+3.5f\n", kp, ki, kd, kdd );
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

    if( terminal_port_is_new_x_y_params( ) )
    {
        terminal_port_get_x_y_params( &params );
        set_control_params( &params, &ctrl_x, &ctrl_y );
    }

    if( terminal_port_is_new_alt_params( ) )
    {
        terminal_port_get_alt_params( &params );
        set_control_params( &params, &ctrl_z, NULL );
    }
}

static void adjust_sensor_data( void )
{
    static double offset_yaw = 0;
    static double offset_x   = 0;
    static double offset_y   = 0;
    static double offset_z   = 0;

    if( motor_signals.front == 0 && motor_signals.right == 0 &&
        motor_signals.rear  == 0 && motor_signals.left  == 0 )
    {
        offset_yaw      = sensor_data.yaw;
        offset_x        = sensor_data.x;
        offset_y        = sensor_data.y;
        offset_z        = sensor_data.z;
        sensor_data.yaw = 0;
        sensor_data.x   = 0;
        sensor_data.y   = 0;
        sensor_data.z   = 0;
    }
    else
    {
        sensor_data.yaw -= offset_yaw;
        sensor_data.x   -= offset_x;
        sensor_data.y   -= offset_y;
        sensor_data.z   -= offset_z;
    }
}

static int get_javiator_data( void )
{
    static double data_delay_x = 0;
    static double data_delay_y = 0;
    static double old_sensor_x = 0;
    static double old_sensor_y = 0;

    int res = javiator_port_get_data( &javiator_data );

    if( res )
    {
        fprintf( stderr, "ERROR: data from JAviator not available\n" );
        return( res );
    }

    /* scale Euler angles */
    sensor_data.roll    = FACTOR_EULER_ANGLE * javiator_data.roll;
    sensor_data.pitch   = FACTOR_EULER_ANGLE * javiator_data.pitch;
    sensor_data.yaw     = FACTOR_EULER_ANGLE * javiator_data.yaw;

    /* save old angular velocities */
    sensor_data.ddroll  = sensor_data.droll;
    sensor_data.ddpitch = sensor_data.dpitch;
    sensor_data.ddyaw   = sensor_data.dyaw;

    /* scale angular velocities */
    sensor_data.droll   = FACTOR_ANGULAR_VEL * javiator_data.droll;
    sensor_data.dpitch  = FACTOR_ANGULAR_VEL * javiator_data.dpitch;
    sensor_data.dyaw    = FACTOR_ANGULAR_VEL * javiator_data.dyaw;

    /* compute angular accelerations */
    sensor_data.ddroll  = FACTOR_ANGULAR_ACC * ( sensor_data.droll  - sensor_data.ddroll );
    sensor_data.ddpitch = FACTOR_ANGULAR_ACC * ( sensor_data.dpitch - sensor_data.ddpitch );
    sensor_data.ddyaw   = FACTOR_ANGULAR_ACC * ( sensor_data.dyaw   - sensor_data.ddyaw );

#if 0
/*
    TODO: must be tested: rotations to angular velocities applied
*/
    /* save old angular velocities */
    sensor_data.ddroll  = sensor_data.droll;
    sensor_data.ddpitch = sensor_data.dpitch;
    sensor_data.ddyaw   = sensor_data.dyaw;

    /* scale angular velocities (store temporarily for transformations) */
    sensor_data.roll    = FACTOR_ANGULAR_VEL * javiator_data.droll;
    sensor_data.pitch   = FACTOR_ANGULAR_VEL * javiator_data.dpitch;
    sensor_data.yaw     = FACTOR_ANGULAR_VEL * javiator_data.dyaw;

    /* transform angular velocities */
    sensor_data.droll   = rotate_local_to_global_dR( sensor_data.roll, sensor_data.pitch, sensor_data.yaw );
    sensor_data.dpitch  = rotate_local_to_global_dP( sensor_data.roll, sensor_data.pitch, sensor_data.yaw );
    sensor_data.dyaw    = rotate_local_to_global_dY( sensor_data.roll, sensor_data.pitch, sensor_data.yaw );

    /* compute angular accelerations */
    sensor_data.ddroll  = FACTOR_ANGULAR_ACC * ( sensor_data.droll  - sensor_data.ddroll );
    sensor_data.ddpitch = FACTOR_ANGULAR_ACC * ( sensor_data.dpitch - sensor_data.ddpitch );
    sensor_data.ddyaw   = FACTOR_ANGULAR_ACC * ( sensor_data.dyaw   - sensor_data.ddyaw );

    /* scale Euler angles */
    sensor_data.roll    = FACTOR_EULER_ANGLE * javiator_data.roll;
    sensor_data.pitch   = FACTOR_EULER_ANGLE * javiator_data.pitch;
    sensor_data.yaw     = FACTOR_EULER_ANGLE * javiator_data.yaw;
#endif

    /* extract/scale positions */
    sensor_data.x       = atoi( (const char *) javiator_data.x_pos );
    sensor_data.y       = atoi( (const char *) javiator_data.y_pos );
    sensor_data.z       = FACTOR_SONAR * javiator_data.sonar;

    /* update and apply filters */
    sensor_data.x       = outlier_filter_update( &filter_x, sensor_data.x );
    sensor_data.y       = outlier_filter_update( &filter_y, sensor_data.y );
    sensor_data.z       = outlier_filter_update( &filter_z, sensor_data.z );

    if( enable_ubisense )
    {
        sensor_data.x   = ubisense_data.x;
        sensor_data.y   = ubisense_data.y;
    }
    else
    /* transform positions */
    {
        sensor_data.x   = rotate_local_to_global_X( X_LASER_POS_ROLL  + sensor_data.x,
                                                    X_LASER_POS_PITCH,
                                                    X_LASER_POS_YAW );

        sensor_data.y   = rotate_local_to_global_Y( Y_LASER_POS_ROLL,
                                                    Y_LASER_POS_PITCH + sensor_data.y,
                                                    Y_LASER_POS_YAW );
    }

    sensor_data.z       = rotate_local_to_global_Z( Z_SONAR_POS_ROLL,
                                                    Z_SONAR_POS_PITCH,
                                                    Z_SONAR_POS_YAW   + sensor_data.z );

    /* scale linear accelerations (store temporarily for transformations) */
    sensor_data.dx      = FACTOR_LINEAR_ACC * javiator_data.ddx;
    sensor_data.dy      = FACTOR_LINEAR_ACC * javiator_data.ddy;
    sensor_data.dz      = FACTOR_LINEAR_ACC * javiator_data.ddz;

    /* transform linear accelerations */
    sensor_data.ddx     = rotate_local_to_global_X( sensor_data.dx, sensor_data.dy, sensor_data.dz );
    sensor_data.ddy     = rotate_local_to_global_Y( sensor_data.dx, sensor_data.dy, sensor_data.dz );
    sensor_data.ddz     = rotate_local_to_global_Z( sensor_data.dx, sensor_data.dy, sensor_data.dz );

    /* update and apply filters */
    sensor_data.ddx     = low_pass_filter_update( &filter_ddx,  sensor_data.ddx );
    sensor_data.ddy     = low_pass_filter_update( &filter_ddy,  sensor_data.ddy );
    sensor_data.ddz     = low_pass_filter_update( &filter_ddz,  sensor_data.ddz + EARTH_GRAVITY );

    /* compute horizontal velocities */
    data_delay_x       += period;
    data_delay_y       += period;

    if( (javiator_data.state & ST_NEW_DATA_POS_X) )
    {
        sensor_data.dx  = ( sensor_data.x - old_sensor_x ) / data_delay_x;
        old_sensor_x    = sensor_data.x;
        data_delay_x    = 0;
    }

    if( (javiator_data.state & ST_NEW_DATA_POS_Y) )
    {
        sensor_data.dy  = ( sensor_data.y - old_sensor_y ) / data_delay_y;
        old_sensor_y    = sensor_data.y;
        data_delay_y    = 0;
    }

    /* estimate altitude and vertical velocity
       (note that the Z-axis is pointing downwards in the aircraft
       coordinate system, hence the sign of ddZ must be changed) */
    kalman_filter_update( &filter_dz, sensor_data.z, -sensor_data.ddz );
    sensor_data.z       = kalman_filter_get_S( &filter_dz );
    sensor_data.dz      = kalman_filter_get_dS( &filter_dz );

    /* scale BMU-specific data */
    sensor_data.maps    = FACTOR_BMU_MAPS * javiator_data.maps;
    sensor_data.temp    = FACTOR_BMU_TEMP * javiator_data.temp - OFFSET_TEMPERATURE;
    sensor_data.batt    = FACTOR_BMU_BATT * javiator_data.batt;

    //fprintf( stdout, "  \t%10u\t%1.5f\t\r", javiator_data.maps, sensor_data.maps );
    //fprintf( stdout, "  \t%4u\t%1.2f\t\r", javiator_data.temp, sensor_data.temp / 100 );
    //fprintf( stdout, "  \t%4u\t%1.2f\t\r", javiator_data.batt, sensor_data.batt / 1000 );
    //fflush( stdout );

    /* update and apply filters */
    sensor_data.maps    = average_filter_update( &filter_maps, sensor_data.maps );
    sensor_data.temp    = median_filter_update ( &filter_temp, sensor_data.temp );
    sensor_data.batt    = median_filter_update ( &filter_batt, sensor_data.batt );

    return( 0 );
}

static int get_ubisense_data( void )
{
    int res = ubisense_port_get_data( &ubisense_data );

    if( res == -1 )
    {
        return( 0 );
    }

    if( res )
    {
        fprintf( stderr, "ERROR: data from Ubisense not available\n" );
        return( res );
    }

    return( 0 );
}

static int get_command_data( void )
{
    if( terminal_port_is_shut_down( ) )
    {
        altitude_mode = CTRL_MODE_SHUTDOWN;
        terminal_port_reset_shut_down( );
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
            case CTRL_MODE_GROUND:
                altitude_mode = CTRL_MODE_FLYING;
                break;

            case CTRL_MODE_FLYING:
                altitude_mode = CTRL_MODE_GROUND;
                break;

            case CTRL_MODE_SHUTDOWN:
                altitude_mode = CTRL_MODE_GROUND;
                break;

            default:
                altitude_mode = CTRL_MODE_SHUTDOWN;
        }
    }
    else
    if( terminal_port_is_new_command_data( ) )
    {
        terminal_port_get_command_data( &command_data );
    }

    /* update and apply filters */
    command_data.roll  = low_pass_filter_update( &filter_cmdr, command_data.roll );
    command_data.pitch = low_pass_filter_update( &filter_cmdp, command_data.pitch );
    command_data.z     = low_pass_filter_update( &filter_cmdz, command_data.z );

    return( 0 );
}

static int perform_shut_down( void )
{
    motor_signals.front = 0;
    motor_signals.right = 0;
    motor_signals.rear  = 0;
    motor_signals.left  = 0;
    motor_speed_revving = 0;
    motor_speed_liftoff = terminal_port_get_base_motor_speed( );

    low_pass_filter_reset( &filter_ddx );
    low_pass_filter_reset( &filter_ddy );
    low_pass_filter_reset( &filter_ddz );
    low_pass_filter_reset( &filter_cmdr );
    low_pass_filter_reset( &filter_cmdp );
    low_pass_filter_reset( &filter_cmdz );
    extended_kalman_reset( );

    controller_reset_zero( &ctrl_roll );
    controller_reset_zero( &ctrl_pitch );
    controller_reset_zero( &ctrl_yaw );
    controller_reset_zero( &ctrl_x );
    controller_reset_zero( &ctrl_y );
    controller_reset_zero( &ctrl_z );

    return( 0 );
}

static int perform_ground_actions( void )
{
    if( motor_signals.front > 0 || motor_signals.right > 0 ||
        motor_signals.rear  > 0 || motor_signals.left  > 0 )
    {
        if( motor_signals.front > 0 )
        {
            motor_signals.front -= MOTOR_REVVING_STEP;
        }

        if( motor_signals.right > 0 )
        {
            motor_signals.right -= MOTOR_REVVING_STEP;
        }

        if( motor_signals.rear > 0 )
        {
            motor_signals.rear -= MOTOR_REVVING_STEP;
        }

        if( motor_signals.left > 0 )
        {
            motor_signals.left -= MOTOR_REVVING_STEP;
        }
    }
    else
    {
        perform_shut_down( );
    }

    return( 0 );
}

static int compute_motor_signals( void )
{
    double c_roll  = 0;
    double c_pitch = 0;
    double u_roll  = 0;
    double u_pitch = 0;
    double u_yaw   = 0;
    double u_z     = 0;
    int i, signals[4];

    extended_kalman_update( &sensor_data );

    if( terminal_port_is_test_mode( ) )
    {
        control_state = CTRL_STATE_TESTING;

        command_data.roll  = -controller_do_control( &ctrl_y,
            -command_data.roll,
            extended_kalman_get_Y( ),   // sensor_data.y
            extended_kalman_get_dY( ),  // sensor_data.dy
            sensor_data.ddy );

        command_data.pitch =  controller_do_control( &ctrl_x,
            command_data.pitch,
            extended_kalman_get_X( ),   // sensor_data.x
            extended_kalman_get_dX( ),  // sensor_data.dx
            sensor_data.ddx );

        if( command_data.roll > ROLL_PITCH_LIMIT )
        {
            command_data.roll = ROLL_PITCH_LIMIT;
        }
        else
        if( command_data.roll < -ROLL_PITCH_LIMIT )
        {
            command_data.roll = -ROLL_PITCH_LIMIT;
        }

        if( command_data.pitch > ROLL_PITCH_LIMIT )
        {
            command_data.pitch = ROLL_PITCH_LIMIT;
        }
        else
        if( command_data.pitch < -ROLL_PITCH_LIMIT )
        {
            command_data.pitch = -ROLL_PITCH_LIMIT;
        }
    }
    else
    {
        control_state = CTRL_STATE_NORMAL;
    }

    /* rotate roll/pitch command depending on yaw angle */
    c_roll  = rotate_local_to_global_X( command_data.roll, command_data.pitch, 0 );
    c_pitch = rotate_local_to_global_Y( command_data.roll, command_data.pitch, 0 );

    /* replace original commands with rotated commands */
    command_data.roll  = c_roll;
    command_data.pitch = c_pitch;



#if 0
/*
    TODO: must be tested: rotations changed from local-to-global to global-to-local
*/
    if( terminal_port_is_test_mode( ) )
    {
        control_state = CTRL_STATE_TESTING;

        command_data.roll  = controller_do_control( &ctrl_y,
            command_data.roll,
            extended_kalman_get_Y( ),   // sensor_data.y
            extended_kalman_get_dY( ),  // sensor_data.dy
            sensor_data.ddy );

        command_data.pitch = controller_do_control( &ctrl_x,
            command_data.pitch,
            extended_kalman_get_X( ),   // sensor_data.x
            extended_kalman_get_dX( ),  // sensor_data.dx
            sensor_data.ddx );

        if( command_data.roll > ROLL_PITCH_LIMIT )
        {
            command_data.roll = ROLL_PITCH_LIMIT;
        }
        else
        if( command_data.roll < -ROLL_PITCH_LIMIT )
        {
            command_data.roll = -ROLL_PITCH_LIMIT;
        }

        if( command_data.pitch > ROLL_PITCH_LIMIT )
        {
            command_data.pitch = ROLL_PITCH_LIMIT;
        }
        else
        if( command_data.pitch < -ROLL_PITCH_LIMIT )
        {
            command_data.pitch = -ROLL_PITCH_LIMIT;
        }
    }
    else
    {
        control_state = CTRL_STATE_NORMAL;
    }

    /* rotate roll/pitch command depending on yaw angle */
    c_roll  = rotate_global_to_local_X( command_data.roll, command_data.pitch, 0 );
    c_pitch = rotate_global_to_local_Y( command_data.roll, command_data.pitch, 0 );

    /* replace original commands with rotated commands */
    command_data.roll  = c_roll;
    command_data.pitch = c_pitch;
#endif



    if( motor_speed_revving < motor_speed_liftoff )
    {
        motor_speed_revving += MOTOR_REVVING_STEP;
        u_z = motor_speed_revving;
    }
    else
    {
        if( command_data.z < 10 && controller_get_integral( &ctrl_z ) > 0 )
        {
            controller_set_integral( &ctrl_z, controller_get_integral( &ctrl_z ) - 10 );
        }

        u_roll  = controller_do_control( &ctrl_roll,
            command_data.roll,
            extended_kalman_get_Roll( ),    // sensor_data.roll
            sensor_data.droll,
            sensor_data.ddroll );

        u_pitch = controller_do_control( &ctrl_pitch,
            command_data.pitch,
            extended_kalman_get_Pitch( ),   // sensor_data.pitch
            sensor_data.dpitch,
            sensor_data.ddpitch );

        u_yaw   = controller_do_control( &ctrl_yaw,
            command_data.yaw,
            extended_kalman_get_Yaw( ),     // sensor_data.yaw
            sensor_data.dyaw,
            sensor_data.ddyaw );

        if( compute_z )
	    {
            u_z  = controller_do_control( &ctrl_z,
                command_data.z,
                sensor_data.z,              // extended_kalman_get_Z( )
                sensor_data.dz,             // extended_kalman_get_dZ( )
                sensor_data.ddz );
            //u_z /= transformation_get_cos_Roll( ) * transformation_get_cos_Pitch( );
            u_z += motor_speed_liftoff;
	    }
        else
        {
            u_z = command_data.z * 16;
        }
    }

    signals[0] = (int)( u_z + u_yaw + u_pitch );
    signals[1] = (int)( u_z - u_yaw - u_roll );
    signals[2] = (int)( u_z + u_yaw - u_pitch );
    signals[3] = (int)( u_z - u_yaw + u_roll );

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

    motor_signals.front = (int16_t)( signals[0] );
    motor_signals.right = (int16_t)( signals[1] );
    motor_signals.rear  = (int16_t)( signals[2] );
    motor_signals.left  = (int16_t)( signals[3] );

    motor_offsets.roll  = (int16_t)( u_roll );
    motor_offsets.pitch = (int16_t)( u_pitch );
    motor_offsets.yaw   = (int16_t)( u_yaw );
    motor_offsets.z     = (int16_t)( u_z );

    trace_data.value_1  = (int16_t)( command_data.roll );
    trace_data.value_2  = (int16_t)( command_data.pitch );
    trace_data.value_3  = (int16_t)( ubisense_data.x );
    trace_data.value_4  = (int16_t)( ubisense_data.y );
    trace_data.value_5  = (int16_t)( sensor_data.dx );
    trace_data.value_6  = (int16_t)( sensor_data.dy );
    trace_data.value_7  = (int16_t)( sensor_data.dz );
    trace_data.value_8  = (int16_t)( extended_kalman_get_Roll( ) );
    trace_data.value_9  = (int16_t)( extended_kalman_get_Pitch( ) );
    trace_data.value_10 = (int16_t)( extended_kalman_get_Yaw( ) );
    trace_data.value_11 = (int16_t)( extended_kalman_get_X( ) );
    trace_data.value_12 = (int16_t)( extended_kalman_get_Y( ) );
    trace_data.value_13 = (int16_t)( extended_kalman_get_Z( ) );
    trace_data.value_14 = (int16_t)( extended_kalman_get_dX( ) );
    trace_data.value_15 = (int16_t)( extended_kalman_get_dY( ) );
    trace_data.value_16 = (int16_t)( extended_kalman_get_dZ( ) );

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
    return terminal_port_send_report( &sensor_data, &motor_signals,
        &motor_offsets, altitude_mode, control_state );
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
    stats[ id ] += time;

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

	if( enable_ubisense && get_ubisense_data( ) )
    {
        fprintf( stderr, "ERROR: connection to Ubisense broken\n" );
		return( -1 );
	}

    end = get_utime( );
    calc_stats( end - start, STAT_FROM_UBI );

	/* IMPORTANT: yaw angle must be adjusted BEFORE
	   setting the angles of the rotation matrix */
	adjust_sensor_data( );

	/* IMPORTANT: all angles must be given in [rad] */
    transformation_set_angles( sensor_data.roll  / 1000,
                               sensor_data.pitch / 1000,
                               sensor_data.yaw   / 1000 );

	return( 0 );
}

/* the control loop for our helicopter */
int control_loop_run( void )
{
    int first_time = 1;
    long long start, end;
	long long loop_start;

    altitude_mode = CTRL_MODE_SHUTDOWN;
    next_period   = get_utime( ) + us_period;

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
            altitude_mode = CTRL_MODE_SHUTDOWN;
            perform_shut_down( );
            break;
        }

        start = get_utime( );

        if( check_terminal_connection( ) )
        {
            altitude_mode = CTRL_MODE_SHUTDOWN;

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
        get_control_params( );

        end = get_utime( );
        calc_stats( end - start, STAT_FROM_TERM );

        start = get_utime( );
 
        switch( altitude_mode )
        {
            case CTRL_MODE_GROUND:
                perform_ground_actions( );
                break;

            case CTRL_MODE_FLYING:
                compute_motor_signals( );
                break;

            case CTRL_MODE_SHUTDOWN:
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
            memset( stats, 0, sizeof( stats ) );
            loop_count = 0;
        }
    }

    outlier_filter_destroy ( &filter_x );
    outlier_filter_destroy ( &filter_y );
    outlier_filter_destroy ( &filter_z );
    kalman_filter_destroy  ( &filter_dz );
    low_pass_filter_destroy( &filter_ddx );
    low_pass_filter_destroy( &filter_ddy );
    low_pass_filter_destroy( &filter_ddz );
    low_pass_filter_destroy( &filter_cmdr );
    low_pass_filter_destroy( &filter_cmdp );
    low_pass_filter_destroy( &filter_cmdz );
    average_filter_destroy ( &filter_maps );
    median_filter_destroy  ( &filter_temp );
    median_filter_destroy  ( &filter_batt );
    controller_destroy     ( &ctrl_roll );
    controller_destroy     ( &ctrl_pitch );
    controller_destroy     ( &ctrl_yaw );
    controller_destroy     ( &ctrl_x );
    controller_destroy     ( &ctrl_y );
    controller_destroy     ( &ctrl_z );

    print_stats( );
    return( 0 );
}

int control_loop_stop( void )
{
    running = 0;
    return( 0 );
}

/* End of file */
