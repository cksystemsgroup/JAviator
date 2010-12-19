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
static int                      pos_ctrl_gain;
static int                      ubisense_enabled;
static int                      heli_state;
static int                      heli_mode;
static int                      heli_settled;
static int                      yaw_wn_imu;
static int                      yaw_wn_cmd;
static int                      new_data_x;
static int                      new_data_y;
static int                      new_data_z;
static double *                 offset_roll;
static double *                 offset_pitch;
static double                   cmd_roll;
static double                   cmd_pitch;
static long long                next_period;

/* motor speed parameters */
static int                      motor_speed_revving;
static int                      motor_speed_liftoff;

/* filter objects */
static outlier_filter_t         cof_out_x;
static outlier_filter_t         cof_out_y;
static iir_lp_filter_t          iir_acc_x;
static iir_lp_filter_t          iir_acc_y;
static iir_lp_filter_t          iir_acc_z;
static iir_lp_filter_t          iir_cmd_roll;
static iir_lp_filter_t          iir_cmd_pitch;
static iir_lp_filter_t          iir_cmd_yaw;
static iir_lp_filter_t          iir_cmd_z;
static average_filter_t         avg_bmu_maps;
static median_filter_t          med_bmu_temp;
static median_filter_t          med_bmu_batt;
static attitude_ekf_t           ekf_att_roll;
static attitude_ekf_t           ekf_att_pitch;
static attitude_ekf_t           ekf_att_yaw;
static position_ekf_t           ekf_pos_x;
static position_ekf_t           ekf_pos_y;
static position_ekf_t           ekf_pos_z;

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
static sensor_data_t            sensor_data;
static motor_signals_t          motor_signals;
static motor_offsets_t          motor_offsets;
static trace_data_t             trace_data;

/* controller statistics */
#define STAT_TO_JAV             0
#define STAT_FROM_JAV           1
#define STAT_FROM_TERM          2
#define STAT_TO_TERM            3
#define STAT_CONTROL            4
#define STAT_SLEEP              5
#define STAT_READ               6
#define STAT_ALL                7
#define NUM_STATS               8

static int                      loop_count = 0;
static long long                stats[ NUM_STATS ] = {0,0,0,0,0,0,0,0};
static long long                max_stats[ NUM_STATS ] = {0,0,0,0,0,0,0,0};
static char *                   stats_name[ NUM_STATS ] =
{
    "to JAviator   ",
    "from JAviator ",
    "from Terminal ",
    "to Terminal   ",
    "control loop  ",
    "sleep time    ",
    "read time     ",
    "complete loop "
};

/* function pointers */
static void                     signal_handler( int num );
static void                     int_handler( int num );

/* forward declaration */
static void                     print_stats( void );


/***************************************
 *          Control Loop Code          *
 ***************************************/

int control_loop_setup( int ms_period, int ctrl_gain, int ubisense,
                        double *offset_r, double *offset_p )
{
    struct sigaction act;

    /* initialize global variables */
    running             = 1;
    period              = ms_period / 1000.0;
    us_period           = ms_period * 1000;
    pos_ctrl_gain       = ctrl_gain;
    ubisense_enabled    = ubisense;
    heli_state          = HELI_STATE_SHUTDOWN;
    heli_mode           = HELI_MODE_POS_CTRL;
    heli_settled        = 1;
    yaw_wn_imu          = 0;
    yaw_wn_cmd          = 0;
    new_data_x          = 0;
    new_data_y          = 0;
    new_data_z          = 0;
    offset_roll         = offset_r;
    offset_pitch        = offset_p;
    cmd_roll            = 0;
    cmd_pitch           = 0;
    next_period         = 0;
    motor_speed_revving = 0;
    motor_speed_liftoff = 0;
    act.sa_handler      = signal_handler;
	act.sa_handler      = int_handler;

    /* initialize signal handlers */
    if( sigaction( SIGUSR1, &act, NULL ) || sigaction( SIGINT, &act, NULL ) )
    {
        perror( "sigaction" );
    }

    /* initialize filter objects */
    outlier_filter_init( &cof_out_x,     "OUT_X",     COF_MDIFF_POS, COF_LIMIT_POS );
    outlier_filter_init( &cof_out_y,     "OUT_Y",     COF_MDIFF_POS, COF_LIMIT_POS );
    iir_lp_filter_init ( &iir_acc_x,     "ACC_X",     IIR_GAIN_LACC );
    iir_lp_filter_init ( &iir_acc_y,     "ACC_Y",     IIR_GAIN_LACC );
    iir_lp_filter_init ( &iir_acc_z,     "ACC_Z",     IIR_GAIN_LACC );
    iir_lp_filter_init ( &iir_cmd_roll,  "CMD_Roll",  IIR_GAIN_RCMD );
    iir_lp_filter_init ( &iir_cmd_pitch, "CMD_Pitch", IIR_GAIN_RCMD );
    iir_lp_filter_init ( &iir_cmd_yaw,   "CMD_Yaw",   IIR_GAIN_RCMD );
    iir_lp_filter_init ( &iir_cmd_z,     "CMD_Z",     IIR_GAIN_RCMD );
    average_filter_init( &avg_bmu_maps,  "BMU_MAPS",  AVG_SIZE_MAPS );
    median_filter_init ( &med_bmu_temp,  "BMU_Temp",  MED_SIZE_TEMP );
    median_filter_init ( &med_bmu_batt,  "BMU_Batt",  MED_SIZE_BATT );
    attitude_ekf_init  ( &ekf_att_roll,  "ATT_Roll",  EKF_ATT_STD_E, EKF_ATT_STD_W, EKF_ATT_PH_SH, period );
    attitude_ekf_init  ( &ekf_att_pitch, "ATT_Pitch", EKF_ATT_STD_E, EKF_ATT_STD_W, EKF_ATT_PH_SH, period );
    attitude_ekf_init  ( &ekf_att_yaw,   "ATT_Yaw",   EKF_ATT_STD_E, EKF_ATT_STD_W, EKF_ATT_PH_SH, period );
    position_ekf_init  ( &ekf_pos_x,     "POS_X",     EKF_POS_STD_P, EKF_POS_STD_V, EKF_POS_STD_A, period );
    position_ekf_init  ( &ekf_pos_y,     "POS_Y",     EKF_POS_STD_P, EKF_POS_STD_V, EKF_POS_STD_A, period );
    position_ekf_init  ( &ekf_pos_z,     "POS_Z",     EKF_POS_STD_P, EKF_POS_STD_V, EKF_POS_STD_A, period );

    /* initialize controller objects */
    controller_init    ( &ctrl_roll,     "Roll",      CTRL_PIDD_DEF, period );
    controller_init    ( &ctrl_pitch,    "Pitch",     CTRL_PIDD_DEF, period );
    controller_init    ( &ctrl_yaw,      "Yaw",       CTRL_PIDD_DEF, period );
    controller_init    ( &ctrl_x,        "X",         CTRL_PIDD_X_Y, period );
    controller_init    ( &ctrl_y,        "Y",         CTRL_PIDD_X_Y, period );
    controller_init    ( &ctrl_z,        "Z",         CTRL_PIDD_DEF, period );

    /* initialize transformations */
    transformation_init( );

    /* clear data structures */
    memset( &command_data,  0, sizeof( command_data ) );
    memset( &javiator_data, 0, sizeof( javiator_data ) );
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

static int get_command_data( void )
{
    static double last_command_yaw = 0;

    if( terminal_port_is_shut_down( ) )
    {
        heli_state = HELI_STATE_SHUTDOWN;
        terminal_port_reset_shut_down( );
    }
    else
    if( terminal_port_is_state_switch( ) )
    {
		fprintf( stdout, "State Switch...\n" );
		print_stats( );

		loop_count = 0;
		memset( stats, 0, sizeof( stats ) );
		memset( max_stats, 0, sizeof( max_stats ) );

        switch( heli_state )
        {
            case HELI_STATE_GROUND:
                heli_state = HELI_STATE_FLYING;
                heli_settled = 0;
                break;

            case HELI_STATE_FLYING:
                heli_state = HELI_STATE_GROUND;
                break;

            case HELI_STATE_SHUTDOWN:
                heli_state = HELI_STATE_GROUND;
                break;

            default:
                heli_state = HELI_STATE_SHUTDOWN;
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

    if( terminal_port_is_mode_switch( ) )
    {
        switch( heli_mode )
        {
            case HELI_MODE_MAN_CTRL:
                heli_mode = HELI_MODE_POS_CTRL;
                break;

            case HELI_MODE_POS_CTRL:
                heli_mode = HELI_MODE_MAN_CTRL;
                break;

            default:
                heli_mode = HELI_MODE_MAN_CTRL;
        }
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

static int prepare_sensor_data( void )
{
    static double last_scaled_yaw = 0;
    static double data_offset_yaw = 0;
    static double data_offset_x   = 0;
    static double data_offset_y   = 0;
    static double data_offset_z   = 0;
    static double last_sensor_x   = 0;
    static double last_sensor_y   = 0;
    static double last_sensor_z   = 0;

    /* scale Euler angles */
    sensor_data.roll   = FACTOR_EULER_ANGLE * javiator_data.roll;
    sensor_data.pitch  = FACTOR_EULER_ANGLE * javiator_data.pitch;
    sensor_data.yaw    = FACTOR_EULER_ANGLE * javiator_data.yaw;

    /* check for trimming change */
    if( terminal_port_is_store_trim( ) )
    {
        *offset_roll  += sensor_data.roll;
        *offset_pitch += sensor_data.pitch;
        fprintf( stdout, "parameter update: Trim Values\n--> roll: %1.3f\tpitch: %1.3f\n",
            (double) *offset_roll, (double) *offset_pitch );
    }
    else
    if( terminal_port_is_clear_trim( ) )
    {
        *offset_roll  = 0;
        *offset_pitch = 0;
        fprintf( stdout, "parameter update: Trim Values\n--> all cleared\n" );
    }

    /* apply roll/pitch trimming */
    sensor_data.roll   -= *offset_roll;
    sensor_data.pitch  -= *offset_pitch;

    /* save old angular velocities */
    sensor_data.ddroll  = sensor_data.droll;
    sensor_data.ddpitch = sensor_data.dpitch;
    sensor_data.ddyaw   = sensor_data.dyaw;

    /* save new angular velocities */
    sensor_data.dx      = FACTOR_ANGULAR_VEL * javiator_data.droll;
    sensor_data.dy      = FACTOR_ANGULAR_VEL * javiator_data.dpitch;
    sensor_data.dz      = FACTOR_ANGULAR_VEL * javiator_data.dyaw;

    /* transform angular velocities */
    sensor_data.droll   = rotate_body_to_earth_dRoll ( sensor_data.dx, sensor_data.dy, sensor_data.dz );
    sensor_data.dpitch  = rotate_body_to_earth_dPitch( sensor_data.dx, sensor_data.dy, sensor_data.dz );
    sensor_data.dyaw    = rotate_body_to_earth_dYaw  ( sensor_data.dx, sensor_data.dy, sensor_data.dz );

    /* compute angular accelerations */
    sensor_data.ddroll  = FACTOR_ANGULAR_ACC * (sensor_data.droll  - sensor_data.ddroll);
    sensor_data.ddpitch = FACTOR_ANGULAR_ACC * (sensor_data.dpitch - sensor_data.ddpitch);
    sensor_data.ddyaw   = FACTOR_ANGULAR_ACC * (sensor_data.dyaw   - sensor_data.ddyaw);

    /* save new linear accelerations */
    sensor_data.dx      = FACTOR_LINEAR_ACC * javiator_data.ddx;
    sensor_data.dy      = FACTOR_LINEAR_ACC * javiator_data.ddy;
    sensor_data.dz      = FACTOR_LINEAR_ACC * javiator_data.ddz;

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

    /* transform x/y-position */
    if( ubisense_enabled )
    {
        if( new_data_x && new_data_y )
        {
            sensor_data.x = outlier_filter_update( &cof_out_x, sensor_data.x );
            sensor_data.y = outlier_filter_update( &cof_out_y, sensor_data.y );
            last_sensor_x = sensor_data.x;
            last_sensor_y = sensor_data.y;
        }
        else
        {
            sensor_data.x = last_sensor_x;
            sensor_data.y = last_sensor_y;
        }
    }
    else
    {
        if( (new_data_x = (javiator_data.state & ST_NEW_DATA_POS_X) != 0) )
        {
            sensor_data.x = -atoi( (const char *) javiator_data.x_pos );
            sensor_data.x = rotate_body_to_earth_X( X_LASER_POS_X +
                sensor_data.x, X_LASER_POS_Y, X_LASER_POS_Z );
            last_sensor_x = sensor_data.x;
        }
        else
        {
            sensor_data.x = last_sensor_x;
        }

        if( (new_data_y = (javiator_data.state & ST_NEW_DATA_POS_Y) != 0) )
        {
            sensor_data.y = -atoi( (const char *) javiator_data.y_pos );
            sensor_data.y = rotate_body_to_earth_Y( Y_LASER_POS_X,
                Y_LASER_POS_Y + sensor_data.y, Y_LASER_POS_Z );
            last_sensor_y = sensor_data.y;
        }
        else
        {
            sensor_data.y = last_sensor_y;
        }
    }

    /* transform z-position */
    if( (new_data_z = (javiator_data.state & ST_NEW_DATA_SONAR) != 0) )
    {
        sensor_data.z = FACTOR_SONAR * javiator_data.sonar;
        sensor_data.z = rotate_body_to_earth_Z( Z_SONAR_POS_X,
            Z_SONAR_POS_Y, Z_SONAR_POS_Z + sensor_data.z );
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
        data_offset_x   = sensor_data.x;
        data_offset_y   = sensor_data.y;
        data_offset_z   = sensor_data.z;
        sensor_data.yaw = 0;
        sensor_data.x   = 0;
        sensor_data.y   = 0;
        sensor_data.z   = 0;
        yaw_wn_imu      = 0;
    }
    else
    {
        sensor_data.yaw -= data_offset_yaw;
        sensor_data.x   -= data_offset_x;
        sensor_data.y   -= data_offset_y;
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

    /* compute attitude estimates */
    attitude_ekf_update( &ekf_att_roll,  sensor_data.roll,  sensor_data.droll );
    attitude_ekf_update( &ekf_att_pitch, sensor_data.pitch, sensor_data.dpitch );
    attitude_ekf_update( &ekf_att_yaw,   sensor_data.yaw,   sensor_data.dyaw );

    /* get attitude estimates */
    sensor_data.roll   = attitude_ekf_get_E( &ekf_att_roll );
    sensor_data.pitch  = attitude_ekf_get_E( &ekf_att_pitch );
    sensor_data.yaw    = attitude_ekf_get_E( &ekf_att_yaw );

    /* update position filters */
    position_ekf_update( &ekf_pos_x, sensor_data.x, sensor_data.ddx, new_data_x );
    position_ekf_update( &ekf_pos_y, sensor_data.y, sensor_data.ddy, new_data_y );
    position_ekf_update( &ekf_pos_z, sensor_data.z, sensor_data.ddz, new_data_z );

    /* get position estimates */
    sensor_data.x      = position_ekf_get_P( &ekf_pos_x );
    sensor_data.y      = position_ekf_get_P( &ekf_pos_y );
    sensor_data.z      = position_ekf_get_P( &ekf_pos_z );

    /* get velocity estimates */
    sensor_data.dx     = position_ekf_get_V( &ekf_pos_x );
    sensor_data.dy     = position_ekf_get_V( &ekf_pos_y );
    sensor_data.dz     = position_ekf_get_V( &ekf_pos_z );

    /* scale BMU-specific data */
    sensor_data.maps   = FACTOR_BMU_MAPS * javiator_data.maps;
    sensor_data.temp   = FACTOR_BMU_TEMP * javiator_data.temp - OFFSET_TEMPERATURE;
    sensor_data.batt   = FACTOR_BMU_BATT * javiator_data.batt;

    /* apply smoothing filters */
    sensor_data.maps   = average_filter_update( &avg_bmu_maps, sensor_data.maps );
    sensor_data.temp   = median_filter_update ( &med_bmu_temp, sensor_data.temp );
    sensor_data.batt   = median_filter_update ( &med_bmu_batt, sensor_data.batt );

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
    outlier_filter_reset( &cof_out_x );
    outlier_filter_reset( &cof_out_y );
    attitude_ekf_reset  ( &ekf_att_roll );
    attitude_ekf_reset  ( &ekf_att_pitch );
    attitude_ekf_reset  ( &ekf_att_yaw );
    position_ekf_reset  ( &ekf_pos_x );
    position_ekf_reset  ( &ekf_pos_y );
    position_ekf_reset  ( &ekf_pos_z );
}

static inline void reset_controllers( void )
{
    controller_reset_zero( &ctrl_roll );
    controller_reset_zero( &ctrl_pitch );
    controller_reset_zero( &ctrl_yaw );
    controller_reset_zero( &ctrl_x );
    controller_reset_zero( &ctrl_y );
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
    static int delay_roll  = 0;
    static int delay_pitch = 0;
    double u_roll, u_pitch, u_yaw, u_z;
    int    i, signals[4];

    /* compute roll/pitch command based on position estimates */
    if( heli_mode == HELI_MODE_POS_CTRL )
    {
        /* check for y-controller invokation */
        if( ++delay_roll >= pos_ctrl_gain / ((int) fabs( command_data.roll - sensor_data.y ) + 1) )
        {
            cmd_roll = controller_do_control( &ctrl_y, command_data.roll,
                sensor_data.y, sensor_data.dy, sensor_data.ddy );//delay_roll );

            /* saturate roll command */
            if( cmd_roll > ROLL_PITCH_LIMIT )
            {
                cmd_roll = ROLL_PITCH_LIMIT;
            }
            else
            if( cmd_roll < -ROLL_PITCH_LIMIT )
            {
                cmd_roll = -ROLL_PITCH_LIMIT;
            }

            delay_roll = 0;
        }

        /* check for x-controller invokation */
        if( ++delay_pitch >= pos_ctrl_gain / ((int) fabs( command_data.pitch - sensor_data.x ) + 1) )
        {
            cmd_pitch = -controller_do_control( &ctrl_x, command_data.pitch,
                sensor_data.x, sensor_data.dx, sensor_data.ddx );//delay_pitch );

            /* saturate pitch command */
            if( cmd_pitch > ROLL_PITCH_LIMIT )
            {
                cmd_pitch = ROLL_PITCH_LIMIT;
            }
            else
            if( cmd_pitch < -ROLL_PITCH_LIMIT )
            {
                cmd_pitch = -ROLL_PITCH_LIMIT;
            }

            delay_pitch = 0;
        }
    }
    else
    {
        cmd_roll  = command_data.roll;
        cmd_pitch = command_data.pitch;
    }

    /* rotate roll/pitch command depending on current yaw angle */
    command_data.roll  = cmd_pitch * transformation_get_sin_Yaw( )
                       + cmd_roll  * transformation_get_cos_Yaw( );
    command_data.pitch = cmd_pitch * transformation_get_cos_Yaw( )
                       - cmd_roll  * transformation_get_sin_Yaw( );

    /* trace roll/pitch command after rotation */
    trace_data.value_1 = (int16_t)( command_data.roll );
    trace_data.value_2 = (int16_t)( command_data.pitch );

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
        &motor_offsets, heli_state, heli_mode );
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

#define MAX_TIME 1000000000 /* microseconds */

int control_loop_run( void )
{
    int first_time = 1;
    long long start, end;
	long long loop_start;
    uint32_t  rem_time;

    next_period = get_utime( ) + us_period;

    while( running )
    {
        trace_data.value_3 = (int16_t)( period * 1000 );
        trace_data.value_4 = (int16_t)( heli_state );

        /* start of STAT_TO_JAV */
        start              = get_utime( );
        loop_start         = start;
        rem_time           = (uint32_t)( start % MAX_TIME );
        trace_data.value_5 = (int16_t)( rem_time >> 16 );
        trace_data.value_6 = (int16_t)( rem_time );

        if( send_motor_signals( ) )
        {
            break;
        }

        end = get_utime( );
        rem_time           = (uint32_t)( end % MAX_TIME );
        trace_data.value_7 = (int16_t)( rem_time >> 16 );
        trace_data.value_8 = (int16_t)( rem_time );
        calc_stats( end - start, STAT_TO_JAV );

        /* start of STAT_FROM_JAV */
	    start = get_utime( );

	    if( javiator_port_get_data( &javiator_data ) )
        {
            fprintf( stderr, "ERROR: connection to JAviator broken\n" );
            heli_state = HELI_STATE_SHUTDOWN;
            perform_shut_down( );
            break;
	    }

        if( ubisense_enabled )
        {
            if( (new_data_x = new_data_y = ubisense_port_is_new_data( )) )
            {
                ubisense_port_get_data( &sensor_data );
            }
        }

	    end = get_utime( );
        rem_time            = (uint32_t)( end % MAX_TIME );
        trace_data.value_9  = (int16_t)( rem_time >> 16 );
        trace_data.value_10 = (int16_t)( rem_time );
	    calc_stats( end - start, STAT_FROM_JAV );

        /* start of STAT_FROM_TERM */
        start = get_utime( );

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

        end = get_utime( );
        rem_time            = (uint32_t)( end % MAX_TIME );
        trace_data.value_11 = (int16_t)( rem_time >> 16 );
        trace_data.value_12 = (int16_t)( rem_time );
        calc_stats( end - start, STAT_FROM_TERM );

        /* start of STAT_CONTROL */
        start = get_utime( );

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

        end = get_utime( );
        rem_time            = (uint32_t)( end % MAX_TIME );
        trace_data.value_13 = (int16_t)( rem_time >> 16 );
        trace_data.value_14 = (int16_t)( rem_time );
        calc_stats( end - start, STAT_CONTROL );

        /* start of STAT_TO_TERM */
        start = get_utime( );

        send_report_to_terminal( );
        send_trace_data_to_terminal( );

        end = get_utime( );
        rem_time            = (uint32_t)( end % MAX_TIME );
        trace_data.value_15 = (int16_t)( rem_time >> 16 );
        trace_data.value_16 = (int16_t)( rem_time );
        calc_stats( end - start, STAT_TO_TERM );
        calc_stats( end - loop_start, STAT_ALL );

        /* start of STAT_SLEEP */
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

    outlier_filter_destroy( &cof_out_x );
    outlier_filter_destroy( &cof_out_y );
    iir_lp_filter_destroy ( &iir_acc_x );
    iir_lp_filter_destroy ( &iir_acc_y );
    iir_lp_filter_destroy ( &iir_acc_z );
    iir_lp_filter_destroy ( &iir_cmd_roll );
    iir_lp_filter_destroy ( &iir_cmd_pitch );
    iir_lp_filter_destroy ( &iir_cmd_yaw );
    iir_lp_filter_destroy ( &iir_cmd_z );
    average_filter_destroy( &avg_bmu_maps );
    median_filter_destroy ( &med_bmu_temp );
    median_filter_destroy ( &med_bmu_batt );
    attitude_ekf_destroy  ( &ekf_att_roll );
    attitude_ekf_destroy  ( &ekf_att_pitch );
    attitude_ekf_destroy  ( &ekf_att_yaw );
    position_ekf_destroy  ( &ekf_pos_x );
    position_ekf_destroy  ( &ekf_pos_y );
    position_ekf_destroy  ( &ekf_pos_z );
    controller_destroy    ( &ctrl_roll );
    controller_destroy    ( &ctrl_pitch );
    controller_destroy    ( &ctrl_yaw );
    controller_destroy    ( &ctrl_x );
    controller_destroy    ( &ctrl_y );
    controller_destroy    ( &ctrl_z );

    print_stats( );
    return( 0 );
}

int control_loop_stop( void )
{
    running = 0;
    return( 0 );
}

/* End of file */
