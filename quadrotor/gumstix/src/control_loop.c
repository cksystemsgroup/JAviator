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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
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
#include "javiator_data.h"
#include "sensor_data.h"
#include "command_data.h"
#include "ctrl_params.h"
#include "motor_signals.h"
#include "motor_offsets.h"
#include "trace_data.h"
#include "outlier_filter.h"
#include "iir_lp_filter.h"
#include "average_filter.h"
#include "median_filter.h"
#include "attitude_ekf.h"
#include "position_ekf.h"
#include "controller.h"
#include "transformation.h"
#include "us_timer.h"

/* compilation directives */
#define RIGHT_SPINNING_FRONT

/* helicopter states */
#define HELI_STATE_GROUND       0x01
#define HELI_STATE_FLYING       0x02
#define HELI_STATE_SHUTDOWN     0x04

/* helicopter modes */
#define HELI_MODE_MAN_CTRL      0x01
#define HELI_MODE_POS_CTRL      0x02

/* filter parameters */
#define COF_MDIFF_POS           500                     /* [mm] maximum allowed position difference */
#define COF_LIMIT_POS           2                       /* limit for counting position outliers */
#define IIR_GAIN_LACC           0.2                     /* gain for linear accelerations */
#define IIR_GAIN_RCMD           0.2                     /* gain for reference commands */
#define AVG_SIZE_MAPS           1                       /* buffer size for pressure data */
#define MED_SIZE_TEMP           15                      /* buffer size for temperature data */
#define MED_SIZE_BATT           15                      /* buffer size for battery data */
#define EKF_ATT_STD_E           0.01                    /* standard deviation Euler angle */
#define EKF_ATT_STD_W           10                      /* standard deviation angular velocity */
#define EKF_ATT_PH_SH           1                       /* estimation phase shift by 1 period */
#define EKF_POS_STD_P           0.01                    /* standard deviation position data */
#define EKF_POS_STD_V           100                     /* standard deviation linear velocity */
#define EKF_POS_STD_A           10000                   /* standard deviation linear acceleration */

/* plant parameters */
#define X_LASER_POS_X           70                      /* [mm] x-laser position on x-axis */
#define X_LASER_POS_Y          -75                      /* [mm] x-laser position on y-axis */
#define X_LASER_POS_Z          -80                      /* [mm] x-laser position on z-axis */
#define Y_LASER_POS_X          -50                      /* [mm] y-laser position on x-axis */
#define Y_LASER_POS_Y           77                      /* [mm] y-laser position on y-axis */
#define Y_LASER_POS_Z          -80                      /* [mm] y-laser position on z-axis */
#define Z_SONAR_POS_X          -95                      /* [mm] z-sonar position on x-axis */
#define Z_SONAR_POS_Y          -95                      /* [mm] z-sonar position on y-axis */
#define Z_SONAR_POS_Z           55                      /* [mm] z-sonar position on z-axis */
#define EARTH_GRAVITY           9810                    /* [mm/s^2] gravitational acceleration */

/* controller parameters */
#define COMMAND_THRESHOLD       35                      /* [iterations] max iterations to wait */
#define ROLL_PITCH_LIMIT        300                     /* [mrad] max allowed roll/pitch angle */
#define OFFSET_TEMPERATURE      700                     /* [mC] temperature calibration */
#define MOTOR_REVVING_STEP      50                      /* inc/dec for motor revving up/down */

/* numerical constants */
#define MRAD_PI                 (1000*M_PI)             /* PI in [rad] to PI in [mrad] */
#define MRAD_2PI                (2000*M_PI)             /* 2*PI in [rad] to 2*PI in [mrad] */

/* scaling constants */
#define FACTOR_EULER_ANGLE      MRAD_2PI/65536.0        /* [units] --> [mrad] (2*PI*1000 mrad/2^16) */
#define FACTOR_ANGULAR_VEL      8500.0/32768.0          /* [units] --> [mrad/s] */
#define FACTOR_ANGULAR_ACC      8500.0/32768.0*76.3     /* [units] --> [mrad/s^2] */
#define FACTOR_LINEAR_ACC       9810.0/4681.0           /* [units] --> [mm/s^2] (4681=32768000/7000) */
#define FACTOR_BMU_MAPS         115000000.0/16777216.0  /* [0-5V]  --> [115000000mPa] */
#define FACTOR_BMU_TEMP         10000.0/4096.0          /* [0-1V]  --> [0-10000cC] */
#define FACTOR_BMU_BATT         18000.0/1024.0          /* [0-5V]  --> [0-18000mV] */
#define FACTOR_SONAR            3000.0/1024.0           /* [0-5V]  --> [0-3000mm] */
#define FACTOR_PARAMETER        0.001                   /* [mrad]  --> [rad] */

/* control loop parameters */
static volatile int             running;
static double                   period;
static int                      us_period;
static int                      heli_state;
static int                      heli_settled;
static int                      yaw_wn_imu;
static int                      yaw_wn_cmd;
static int                      new_data_z;
static double                   cmd_roll;
static double                   cmd_pitch;
static long long                next_period;

/* motor speed parameters */
static int                      motor_speed_revving;
static int                      motor_speed_liftoff;

/* filter objects */
static iir_lp_filter_t          iir_acc_x;
static iir_lp_filter_t          iir_acc_y;
static iir_lp_filter_t          iir_acc_z;
static iir_lp_filter_t          iir_cmd_roll;
static iir_lp_filter_t          iir_cmd_pitch;
static iir_lp_filter_t          iir_cmd_yaw;
static iir_lp_filter_t          iir_cmd_z;
static median_filter_t          med_bat_level;
static position_ekf_t           ekf_pos_z;

/* controller objects */
static controller_t             ctrl_roll;
static controller_t             ctrl_pitch;
static controller_t             ctrl_yaw;
static controller_t             ctrl_z;

/* data structures */
static command_data_t           command_data;
static javiator_ldat_t          javiator_data;
static sensor_data_t            sensor_data;
static motor_signals_t          motor_signals;
static motor_offsets_t          motor_offsets;


/***************************************
 *          Control Loop Code          *
 ***************************************/

int control_loop_setup( int ms_period, int ctrl_gain, int ubisense,
                        double *offset_r, double *offset_p )
{
    /* initialize global variables */
    running             = 1;
    period              = ms_period / 1000.0;
    us_period           = ms_period * 1000;
    heli_state          = HELI_STATE_SHUTDOWN;
    heli_settled        = 1;
    yaw_wn_imu          = 0;
    yaw_wn_cmd          = 0;
    new_data_z          = 0;
    cmd_roll            = 0;
    cmd_pitch           = 0;
    next_period         = 0;
    motor_speed_revving = 0;
    motor_speed_liftoff = 0;

    /* initialize filter objects */
    iir_lp_filter_init ( &iir_acc_x,     "ACC_X",     IIR_GAIN_LACC );
    iir_lp_filter_init ( &iir_acc_y,     "ACC_Y",     IIR_GAIN_LACC );
    iir_lp_filter_init ( &iir_acc_z,     "ACC_Z",     IIR_GAIN_LACC );
    iir_lp_filter_init ( &iir_cmd_roll,  "CMD_Roll",  IIR_GAIN_RCMD );
    iir_lp_filter_init ( &iir_cmd_pitch, "CMD_Pitch", IIR_GAIN_RCMD );
    iir_lp_filter_init ( &iir_cmd_yaw,   "CMD_Yaw",   IIR_GAIN_RCMD );
    iir_lp_filter_init ( &iir_cmd_z,     "CMD_Z",     IIR_GAIN_RCMD );
    median_filter_init ( &med_bat_level, "Bat_Level", MED_SIZE_BATT );
    position_ekf_init  ( &ekf_pos_z,     "POS_Z",     EKF_POS_STD_P, EKF_POS_STD_V, EKF_POS_STD_A, period );

    /* initialize controller objects */
    controller_init    ( &ctrl_roll,     "Roll",      CTRL_PIDD_DEF, period );
    controller_init    ( &ctrl_pitch,    "Pitch",     CTRL_PIDD_DEF, period );
    controller_init    ( &ctrl_yaw,      "Yaw",       CTRL_PIDD_DEF, period );
    controller_init    ( &ctrl_z,        "Z",         CTRL_PIDD_DEF, period );

    /* initialize transformations */
    transformation_init( );

    /* clear data structures */
    memset( &command_data,  0, sizeof( command_data ) );
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &sensor_data,   0, sizeof( sensor_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );
    memset( &motor_offsets, 0, sizeof( motor_offsets ) );

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

static int get_command_data( void )
{
    static double last_command_yaw = 0;

    if( terminal_port_is_shut_down( ) )
    {
        heli_state = HELI_STATE_SHUTDOWN;
	fprintf( stdout, "New State: Halt\n" );
        terminal_port_reset_shut_down( );
    }
    else
    if( terminal_port_is_state_switch( ) )
    {
	fprintf( stdout, "New State: " );

        switch( heli_state )
        {
            case HELI_STATE_GROUND:
                heli_state = HELI_STATE_FLYING;
	        fprintf( stdout, "Flying\n" );
                heli_settled = 0;
                break;

            case HELI_STATE_FLYING:
                heli_state = HELI_STATE_GROUND;
	        fprintf( stdout, "Ground\n" );
                break;

            case HELI_STATE_SHUTDOWN:
                heli_state = HELI_STATE_GROUND;
	        fprintf( stdout, "Ground\n" );
                break;

            default:
                heli_state = HELI_STATE_SHUTDOWN;
	        fprintf( stdout, "Halt\n" );
        }
    }
    else
    if( terminal_port_is_new_command_data( ) )
    {
        terminal_port_get_command_data( &command_data );

        /* compute winding number of yaw command */
        if( heli_settled )
        {
            yaw_wn_cmd = 0;
        }
        else
        if( last_command_yaw - command_data.yaw < -MRAD_PI )
        {
            --yaw_wn_cmd;
        }
        else
        if( last_command_yaw - command_data.yaw > MRAD_PI )
        {
            ++yaw_wn_cmd;
        }

        /* compute winded yaw command */
        last_command_yaw   = command_data.yaw;
        command_data.yaw  += yaw_wn_cmd * MRAD_2PI;

        /* apply low-pass filters */
        command_data.roll  = iir_lp_filter_update( &iir_cmd_roll,  command_data.roll );
        command_data.pitch = iir_lp_filter_update( &iir_cmd_pitch, command_data.pitch );
        command_data.yaw   = iir_lp_filter_update( &iir_cmd_yaw,   command_data.yaw );
        command_data.z     = iir_lp_filter_update( &iir_cmd_z,     command_data.z );
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
        fprintf( stdout, "Parameter update: %s", ctrl_1->name );

        if( ctrl_2 != NULL )
        {
            controller_set_params( ctrl_2, kp, ki, kd, kdd );
            fprintf( stdout, "/%s", ctrl_2->name );
        }

        fprintf( stdout, "\n--> Kp: %+3.3f   Ki: %+3.3f   "
            "Kd: %+3.3f   Kdd: %+3.3f\n", kp, ki, kd, kdd );
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
}

static int prepare_sensor_data( void )
{
    static double data_offset_yaw = 0;
    static double last_scaled_yaw = 0;
    static double data_offset_z   = 0;
    static double last_sensor_z   = 0;

    /* save old angular velocities */
    sensor_data.ddroll  = sensor_data.droll;
    sensor_data.ddpitch = sensor_data.dpitch;
    sensor_data.ddyaw   = sensor_data.dyaw;

    /* scale Euler angles */
    sensor_data.roll    = FACTOR_EULER_ANGLE * javiator_data.roll;
    sensor_data.pitch   = FACTOR_EULER_ANGLE * javiator_data.pitch;
    sensor_data.yaw     = FACTOR_EULER_ANGLE * javiator_data.yaw;

    /* scale angular velocities */
    sensor_data.droll   = FACTOR_ANGULAR_VEL * javiator_data.droll;
    sensor_data.dpitch  = FACTOR_ANGULAR_VEL * javiator_data.dpitch;
    sensor_data.dyaw    = FACTOR_ANGULAR_VEL * javiator_data.dyaw;

    /* compute angular accelerations */
    sensor_data.ddroll  = FACTOR_ANGULAR_ACC * (sensor_data.droll  - sensor_data.ddroll);
    sensor_data.ddpitch = FACTOR_ANGULAR_ACC * (sensor_data.dpitch - sensor_data.ddpitch);
    sensor_data.ddyaw   = FACTOR_ANGULAR_ACC * (sensor_data.dyaw   - sensor_data.ddyaw);

    /* save new linear accelerations */
    sensor_data.dx      = FACTOR_LINEAR_ACC  * javiator_data.ddx;
    sensor_data.dy      = FACTOR_LINEAR_ACC  * javiator_data.ddy;
    sensor_data.dz      = FACTOR_LINEAR_ACC  * javiator_data.ddz;

    /* transform linear accelerations */
    sensor_data.ddx     = rotate_body_to_earth_X( sensor_data.dx, sensor_data.dy, sensor_data.dz );
    sensor_data.ddy     = rotate_body_to_earth_Y( sensor_data.dx, sensor_data.dy, sensor_data.dz );
    sensor_data.ddz     = rotate_body_to_earth_Z( sensor_data.dx, sensor_data.dy, sensor_data.dz );

    /* add Earth's gravitational force */
    sensor_data.ddz    += EARTH_GRAVITY;

    /* IMPORTANT: the Z-axis is pointing DOWNWARDS in the aircraft
       coordinate system, hence the sign of ddZ must be changed */
    sensor_data.ddz     = -sensor_data.ddz;

    /* apply low-pass filters */
    sensor_data.ddx     = iir_lp_filter_update( &iir_acc_x, sensor_data.ddx );
    sensor_data.ddy     = iir_lp_filter_update( &iir_acc_y, sensor_data.ddy );
    sensor_data.ddz     = iir_lp_filter_update( &iir_acc_z, sensor_data.ddz );

    /* transform z-position */
    if( (new_data_z = (javiator_data.state & ST_NEW_DATA_SONAR) != 0) )
    {
        last_sensor_z = FACTOR_SONAR * javiator_data.sonar;
        sensor_data.z = rotate_body_to_earth_Z( Z_SONAR_POS_X,
            Z_SONAR_POS_Y, Z_SONAR_POS_Z + last_sensor_z );
        last_sensor_z = sensor_data.z;
    }
    else
    {
        sensor_data.z = last_sensor_z;
    }

    /* perform zero adjustments if heli settled
       IMPORTANT: yaw angle must be zero-adjusted BEFORE
	   updating the angles of the transformation matrices */
    if( heli_settled )
    {
        data_offset_yaw = sensor_data.yaw;
        data_offset_z   = sensor_data.z;
        sensor_data.yaw = 0;
        sensor_data.z   = 0;
        yaw_wn_imu      = 0;
    }
    else
    {
        sensor_data.yaw -= data_offset_yaw;
        sensor_data.z   -= data_offset_z;

        /* compute winding number of yaw angle */
        if( last_scaled_yaw - sensor_data.yaw < -MRAD_PI )
        {
            --yaw_wn_imu;
        }
        else
        if( last_scaled_yaw - sensor_data.yaw > MRAD_PI )
        {
            ++yaw_wn_imu;
        }

        /* check altitude to be non-negative */
        if( sensor_data.z < 0 )
        {
            sensor_data.z = 0;
        }
    }

    /* compute winded yaw angle */
    last_scaled_yaw    = sensor_data.yaw;
    sensor_data.yaw   += yaw_wn_imu * MRAD_2PI;

    /* update z-position filter */
    position_ekf_update( &ekf_pos_z, sensor_data.z, sensor_data.ddz, new_data_z );

    /* get z-position estimate */
    sensor_data.z      = position_ekf_get_P( &ekf_pos_z );

    /* get z-velocity estimate */
    sensor_data.dz     = position_ekf_get_V( &ekf_pos_z );

    /* scale battery level data */
    sensor_data.batt   = FACTOR_BMU_BATT * javiator_data.batt;

    /* apply smoothing filter */
    sensor_data.batt   = median_filter_update( &med_bat_level, sensor_data.batt );

    /* update rotation angles with new estimates
       IMPORTANT: all angles must be given in radians */
    transformation_set_angles( sensor_data.roll  / 1000,
                               sensor_data.pitch / 1000,
                               sensor_data.yaw   / 1000 );

    return( 0 );
}

static inline void reset_motors( void )
{
    motor_signals.front = 0;
    motor_signals.right = 0;
    motor_signals.rear  = 0;
    motor_signals.left  = 0;
    motor_speed_revving = 0;
    motor_speed_liftoff = terminal_port_get_base_motor_speed( );
    heli_settled        = 1;
    cmd_roll            = 0;
    cmd_pitch           = 0;
}

static inline void reset_filters( void )
{
    position_ekf_reset  ( &ekf_pos_z );
}

static inline void reset_controllers( void )
{
    controller_reset_zero( &ctrl_roll );
    controller_reset_zero( &ctrl_pitch );
    controller_reset_zero( &ctrl_yaw );
    controller_reset_zero( &ctrl_z );
}

static void perform_shut_down( void )
{
    reset_motors( );
    reset_filters( );
    reset_controllers( );
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
        reset_motors( );
        reset_controllers( );
    }

    return( 0 );
}

static int compute_motor_signals( void )
{
    double u_roll, u_pitch, u_yaw, u_z;
    int    i, signals[4];

    /* save original roll/pitch command */
    cmd_roll  = command_data.roll;
    cmd_pitch = command_data.pitch;

    /* rotate roll/pitch command depending on current yaw angle */
    command_data.roll  = cmd_pitch * transformation_get_sin_Yaw( )
                       + cmd_roll  * transformation_get_cos_Yaw( );
    command_data.pitch = cmd_pitch * transformation_get_cos_Yaw( )
                       - cmd_roll  * transformation_get_sin_Yaw( );

    /* compute motor offsets based on EKF estimate */
    if( motor_speed_revving < motor_speed_liftoff )
    {
        motor_speed_revving += MOTOR_REVVING_STEP;

        u_roll  = 0;
        u_pitch = 0;
        u_yaw   = 0;
        u_z     = motor_speed_revving;
    }
    else
    {
        if( command_data.z < 10 && controller_get_integral( &ctrl_z ) > 0 )
        {
            controller_set_integral( &ctrl_z, controller_get_integral( &ctrl_z ) - 10 );
        }

        u_roll  = controller_do_control( &ctrl_roll, command_data.roll,
            sensor_data.roll, sensor_data.droll, sensor_data.ddroll );

        u_pitch = controller_do_control( &ctrl_pitch, command_data.pitch,
            sensor_data.pitch, sensor_data.dpitch, sensor_data.ddpitch );

        u_yaw   = controller_do_control( &ctrl_yaw, command_data.yaw,
            sensor_data.yaw, sensor_data.dyaw, sensor_data.ddyaw );

        u_z     = controller_do_control( &ctrl_z, command_data.z,
            sensor_data.z, sensor_data.dz, sensor_data.ddz );
        u_z    /= transformation_get_cos_Roll( ) * transformation_get_cos_Pitch( );
        u_z    += motor_speed_liftoff;
    }

    /* compute motor signals */
#ifdef RIGHT_SPINNING_FRONT
    signals[0] = (int)( u_z - u_yaw + u_pitch );
    signals[1] = (int)( u_z + u_yaw - u_roll );
    signals[2] = (int)( u_z - u_yaw - u_pitch );
    signals[3] = (int)( u_z + u_yaw + u_roll );
#else
    signals[0] = (int)( u_z + u_yaw + u_pitch );
    signals[1] = (int)( u_z - u_yaw - u_roll );
    signals[2] = (int)( u_z + u_yaw - u_pitch );
    signals[3] = (int)( u_z - u_yaw + u_roll );
#endif

    /* saturate motor signals */
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

    /* store motor signals */
    motor_signals.front = (int16_t)( signals[0] );
    motor_signals.right = (int16_t)( signals[1] );
    motor_signals.rear  = (int16_t)( signals[2] );
    motor_signals.left  = (int16_t)( signals[3] );

    /* store motor offsets */
    motor_offsets.roll  = (int16_t)( u_roll );
    motor_offsets.pitch = (int16_t)( u_pitch );
    motor_offsets.yaw   = (int16_t)( u_yaw );
    motor_offsets.z     = (int16_t)( u_z );

    /* restore unwinded yaw angle and command */
    sensor_data.yaw    -= yaw_wn_imu * MRAD_2PI;
    command_data.yaw   -= yaw_wn_cmd * MRAD_2PI;

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
        &motor_offsets, heli_state, HELI_MODE_MAN_CTRL );
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

#define MAX_TIME 1000000000 /* microseconds */

int control_loop_run( void )
{
    int first_time = 1;

    next_period = get_utime( ) + us_period;

    while( running )
    {
        if( send_motor_signals( ) )
        {
            break;
        }

	if( javiator_port_get_data( &javiator_data ) )
        {
            fprintf( stderr, "ERROR: connection to JAviator broken\n" );
            heli_state = HELI_STATE_SHUTDOWN;
            perform_shut_down( );
            break;
	}

        if( check_terminal_connection( ) )
        {
            heli_state = HELI_STATE_SHUTDOWN;

            if( first_time )
            {
                fprintf( stderr, "ERROR: connection to Terminal broken\n" );
                first_time = 0;
            }
        }
        else
        {
            first_time = 1;
        }

        get_command_data( );
        get_control_params( );
        prepare_sensor_data( );
 
        switch( heli_state )
        {
            case HELI_STATE_GROUND:
                perform_ground_actions( );
                break;

            case HELI_STATE_FLYING:
                compute_motor_signals( );
                break;

            case HELI_STATE_SHUTDOWN:
                perform_shut_down( );
                break;

            default:
                fprintf( stderr, "ERROR: invalid altitude mode %d\n", heli_state );
        }

        send_report_to_terminal( );
        wait_for_next_period( );
    }

    iir_lp_filter_destroy( &iir_acc_x );
    iir_lp_filter_destroy( &iir_acc_y );
    iir_lp_filter_destroy( &iir_acc_z );
    iir_lp_filter_destroy( &iir_cmd_roll );
    iir_lp_filter_destroy( &iir_cmd_pitch );
    iir_lp_filter_destroy( &iir_cmd_yaw );
    iir_lp_filter_destroy( &iir_cmd_z );
    median_filter_destroy( &med_bat_level );
    position_ekf_destroy ( &ekf_pos_z );
    controller_destroy   ( &ctrl_roll );
    controller_destroy   ( &ctrl_pitch );
    controller_destroy   ( &ctrl_yaw );
    controller_destroy   ( &ctrl_z );

    return( 0 );
}

int control_loop_stop( void )
{
    running = 0;
    return( 0 );
}

/* End of file */
