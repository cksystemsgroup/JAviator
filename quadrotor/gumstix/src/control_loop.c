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

#include "../shared/protocol.h"
#include "../shared/transfer.h"
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
#include "filter_params.h"
#include "kalman_filter.h"
#include "us_timer.h"


//#define APPLY_COS_SIN_SONAR_SENSOR_CORRECTION
//#define APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
//#define ADJUST_YAW
//#define ADJUST_Z
//#define FAST_PWM


/* filter parameters */
#define FILTER_FACTOR_NAVI      0.1
#define FILTER_FACTOR_DDZ       0.1
#define MEDIAN_BUFFER_SIZE      9
#define MEDIAN_BUFFER_INIT      {0,0,0,0,0,0,0,0,0}

/* controller modes */
#define ALT_MODE_GROUND         0x00
#define ALT_MODE_FLYING         0x01
#define ALT_MODE_SHUTDOWN       0x02

/* scaling constants*/
#define FACTOR_EULER_ANGLE      2000.0*M_PI/65536.0 // units    --> [mrad] (2*PI*1000 mrad/2^16)
#define FACTOR_ANGULAR_RATE     8500.0/32768.0      // units    --> [mrad/s]
#define FACTOR_ANGULAR_ACCEL    8500.0/32768.0*76.3 // units    --> [mrad/s^2]
#define FACTOR_POSITION         1.0
#define FACTOR_LINEAR_RATE      1.0
#define FACTOR_LINEAR_ACCEL     9810.0/4681.0       // units    --> [mm/s^2] (4681=32768000/7000)

#define FACTOR_LASER            0.1                 // [1/10mm] --> [mm]
#define FACTOR_SONAR            5000.0/1024.0       // [0-5V]   --> [mm]
#define FACTOR_PRESSURE         5000.0/1024.0       // [0-5V]   --> [mm]
#define FACTOR_BATTERY          18000.0/1024.0      // [0-5V]   --> [0-18V]
#define FACTOR_PARAMETER        0.001

#ifdef FAST_PWM
  #define MOTOR_MAX             16000
  #define MOTOR_MIN             0
#else
  #define MOTOR_MAX             1000
  #define MOTOR_MIN             0
#endif

#define COMMAND_THRESHOLD       35
#define MRAD_PI                 3142 // [mrad]
#define IO_JITTER               1000 // io time window
#define IMU_DELAY               10   // [ms]

/* control loop parameters */
static int ms_period;
static int us_period;
static long long next_period;
static int controller_state;
static int altitude_mode;
static double uZold;
static int compute_z;
static volatile int running;

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
static inertial_data_t  inertial_data;
static sensor_data_t    sensor_data;
static motor_signals_t  motor_signals;
static command_data_t   motor_offsets;

#ifdef APPLY_COS_SIN_SONAR_SENSOR_CORRECTION
#define SONAR_POS_ROLL           90 // [mm] sonar position on roll axis
#define SONAR_POS_PITCH         -90 // [mm] sonar position on pitch axis
static double cos_roll  = 0;
static double sin_roll  = 0;
static double cos_pitch = 0;
static double sin_pitch = 0;
#endif

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
static double cos_yaw   = 0;
static double sin_yaw   = 0;
#endif

static void signal_handler(int num);

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
    memset( &inertial_data, 0, sizeof( inertial_data ) );
    memset( &sensor_data,   0, sizeof( sensor_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );
    memset( &motor_offsets, 0, sizeof( motor_offsets ) );

    ms_period        = period;
    us_period        = period * 1000;
    next_period      = 0;
    controller_state = 0;
    altitude_mode    = ALT_MODE_GROUND;
    uZold            = 0;
    compute_z        = control_z;
    running          = 1;


    act.sa_handler = signal_handler;

    if (sigaction(SIGUSR1, &act, NULL))
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

static int get_javiator_data( void )
{
    static long long last_run = 0;
    static uint16_t  last_id  = 0;
    long long        now;
    int              res;

    res = javiator_port_get_data( &javiator_data );

    if( res )
    {
        fprintf( stderr, "ERROR: data from JAviator not available\n" );
        return( res );
    }

    if( javiator_data.id != (uint16_t)( last_id + 1 ) )
    {
        fprintf( stderr, "WARNING: control loop lost %u JAviator packet(s)\n",
            javiator_data.id - last_id );
        now = get_utime( );

        if( now - last_run > us_period )
        {
            fprintf( stderr, "WARNING: last period is %lld, jitter is %lld\n",
                now - last_run, now - last_run - us_period );
        }
    }

    last_run = get_utime( );
    last_id  = javiator_data.id;

    /* scale JAviator data */
    javiator_data.laser    = (uint32_t)( javiator_data.laser    * FACTOR_LASER );
    //javiator_data.sonar    = (uint16_t)( javiator_data.sonar    * FACTOR_SONAR );
    javiator_data.pressure = (uint16_t)( javiator_data.pressure * FACTOR_PRESSURE );

    /* save old positions */
    sensor_data.dx         = sensor_data.x;
    sensor_data.dy         = sensor_data.y;
    sensor_data.dz         = sensor_data.z;

    /* copy and scale positions */
    sensor_data.x          = (int16_t)( javiator_data.pos_x * FACTOR_POSITION );
    sensor_data.y          = (int16_t)( javiator_data.pos_y * FACTOR_POSITION );
    sensor_data.z          = (int16_t)( javiator_data.sonar * FACTOR_SONAR );

    /* compute linear rates */
    sensor_data.dx         = (int16_t)( (sensor_data.x - sensor_data.dx) * FACTOR_LINEAR_RATE );
    sensor_data.dy         = (int16_t)( (sensor_data.y - sensor_data.dy) * FACTOR_LINEAR_RATE );
    sensor_data.dz         = (int16_t)( (sensor_data.z - sensor_data.dz) * FACTOR_LINEAR_RATE );

    /* copy and scale battery level */
    sensor_data.battery    = (int16_t)( javiator_data.battery * FACTOR_BATTERY );

#ifdef ADJUST_Z
    adjust_z( );
#endif

    return( 0 );
}

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

#ifdef APPLY_COS_SIN_SONAR_SENSOR_CORRECTION
    cos_roll  = cos( (double) sensor_data.roll  / 1000.0 );
    sin_roll  = sin( (double) sensor_data.roll  / 1000.0 );
    cos_pitch = cos( (double) sensor_data.pitch / 1000.0 );
    sin_pitch = sin( (double) sensor_data.pitch / 1000.0 );
#endif

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
    cos_yaw   = cos( (double) sensor_data.yaw   / 1000.0 );
    sin_yaw   = sin( (double) sensor_data.yaw   / 1000.0 );
#endif

#ifdef ADJUST_YAW
    adjust_yaw( );
#endif

    return( 0 );
}

static void filter_and_assign_commands(
    const command_data_t *in, command_data_t *out )
{
    static command_data_t old_commands = { 0, 0, 0, 0 };
    int yaw_error = in->yaw - old_commands.yaw;

    if( yaw_error < -MRAD_PI )
    {
        yaw_error += 2 * MRAD_PI;
    }
    else
    if( yaw_error > MRAD_PI )
    {
        yaw_error -= 2 * MRAD_PI;
    }

    out->roll  = old_commands.roll  + (int)( FILTER_FACTOR_NAVI * (in->roll  - old_commands.roll) );
    out->pitch = old_commands.pitch + (int)( FILTER_FACTOR_NAVI * (in->pitch - old_commands.pitch) );
    out->yaw   = old_commands.yaw   + (int)( FILTER_FACTOR_NAVI * yaw_error );
    out->z     = old_commands.z     + (int)( FILTER_FACTOR_NAVI * (in->z     - old_commands.z) );

    memcpy( &old_commands, out, sizeof( old_commands ) );
}

static int get_command_data( void )
{
    static int sensors_enabled = 0;
    static command_data_t commands = { 0, 0, 0, 0 };
    command_data_t new_commands;

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
        if( terminal_port_get_command_data( &new_commands ) != -1 )
        {
            memcpy( &commands, &new_commands, sizeof( commands ) );
/*
            fprintf( stdout, "   roll: %5d   pitch: %5d   yaw: %5d   z: %5d\r",
                        commands.roll, commands.pitch, commands.yaw, commands.z );
            fflush( stdout );
*/
        }
    }

    filter_and_assign_commands( &commands, &command_data );

#ifdef APPLY_ROTATION_MATRIX_TO_ROLL_AND_PITCH
    // apply rotation matrix to roll/pitch commands depending on current yaw angle
    command_data.roll  = (int)( command_data.roll * cos_yaw - command_data.pitch * sin_yaw );
    command_data.pitch = (int)( command_data.roll * sin_yaw + command_data.pitch * cos_yaw );
#endif

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
    
    uZold = 0;
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

static inline double filter_z( void )
{
    static short median_buffer[MEDIAN_BUFFER_SIZE] = MEDIAN_BUFFER_INIT;
    int i, j;

#ifdef APPLY_COS_SIN_SONAR_SENSOR_CORRECTION
    sensor_data.z = (short)(
          (double) -SONAR_POS_ROLL  * sin_pitch
        + (double)  SONAR_POS_PITCH * cos_pitch * sin_roll
        + (double)  sensor_data.z   * cos_pitch * cos_roll );
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
    static double old_ddz = 0;
    double new_ddz = 0;
    double ddzEst = 0;

#ifndef APPLY_COS_SIN_SONAR_SENSOR_CORRECTION
    double cos_roll  = cos( (double) sensor_data.roll  / 1000.0 );
    double sin_roll  = sin( (double) sensor_data.roll  / 1000.0 );
    double cos_pitch = cos( (double) sensor_data.pitch / 1000.0 );
    double sin_pitch = sin( (double) sensor_data.pitch / 1000.0 );
#endif

    new_ddz = ( (double) -sensor_data.ddx * sin_pitch
              + (double)  sensor_data.ddy * cos_pitch * sin_roll
              + (double)  sensor_data.ddz * cos_pitch * cos_roll ) / 1000.0 + GRAVITY;

    ddzEst  = old_ddz + FILTER_FACTOR_DDZ * (new_ddz - old_ddz);
    old_ddz = ddzEst;

    return( ddzEst );
}

static inline double filter_dz( double z, double ddz )
{
    return apply_kalman_filter( &z_kalman_filter, z, ddz, (double) ms_period / 1000.0 );
}

static int compute_motor_signals( void )
{
    double zFiltered   = 0;    // median-filtered z
    double zEst        = 0;    // kalman-estimated z
    double dzEst       = 0;    // kalman-estimated dz
    double ddzFiltered = 0;    // low-pass-filtered ddz
    double uRoll       = 0;
    double uPitch      = 0;
    double uYaw        = 0;
    double uZnew       = 0;
    int    i, signals[4];

    zFiltered   = filter_z( );                          // apply median filter
    ddzFiltered = filter_ddz( );                        // apply low-pass filter
    dzEst       = filter_dz( zFiltered, -ddzFiltered ); // apply kalman filter
    zEst        = z_kalman_filter.z;                    // use z from kalman filter

    if( revving_step < (base_motor_speed / motor_revving_add) )
    {
        uZnew = uZold + motor_revving_add;
        ++revving_step;
    }
    else
    {
        uRoll  = do_control( &ctrl_roll,
            sensor_data.roll/1000.0, command_data.roll/1000.0,
            sensor_data.droll/1000.0, sensor_data.ddroll/1000.0 )*1000.0;
        uPitch = do_control( &ctrl_pitch,
            sensor_data.pitch/1000.0, command_data.pitch/1000.0,
            sensor_data.dpitch/1000.0, sensor_data.ddpitch/1000.0 )*1000.0;
        uYaw   = do_control( &ctrl_yaw,
            sensor_data.yaw/1000.0, command_data.yaw/1000.0,
            sensor_data.dyaw/1000.0, sensor_data.ddyaw/1000.0 )*1000.0;

        if( compute_z )
        {
            uZnew  = do_control( &ctrl_z, zEst, command_data.z/1000.0, dzEst, ddzFiltered )*1000.0;
        }
        else
        {
            uZnew = command_data.z;
        }

        uZnew += base_motor_speed;
    }

    signals[0] = (int)( uZnew + uYaw + uPitch );
    signals[1] = (int)( uZnew - uYaw - uRoll );
    signals[2] = (int)( uZnew + uYaw - uPitch );
    signals[3] = (int)( uZnew - uYaw + uRoll );

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

    motor_signals.front = (uint16_t) signals[0];
    motor_signals.right = (uint16_t) signals[1];
    motor_signals.rear  = (uint16_t) signals[2];
    motor_signals.left  = (uint16_t) signals[3];

    motor_offsets.roll  = (int16_t) uRoll;
    motor_offsets.pitch = (int16_t) uPitch;
    motor_offsets.yaw   = (int16_t) uYaw;
    motor_offsets.z     = (int16_t) uZnew;

    uZold               = uZnew;

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

static inline void do_io( long long deadline )
{
    long long now = get_utime( );

    while( now < deadline )
    {
        terminal_port_tick( );
    //    inertial_port_tick( );
        sleep_until( now + 1000 );
        now = get_utime( );
    }
}

static int wait_for_next_period( void )
{
    do_io( next_period - IO_JITTER );

    if( sleep_until( next_period ) )
    {
        fprintf( stderr, "error in sleep_until\n" );
        exit( 1 );
    }

    next_period += us_period;
    return( 0 );
}

#define NUM_STATS 5
#define STAT_IMU  0
#define STAT_SPI  1
#define STAT_JAV  2
#define STAT_CMD  3
#define STAT_REP  4

static long long stats[NUM_STATS] = {0,0,0,0,0};
static long long max_stats[NUM_STATS] = {0,0,0,0,0};


static void calc_stats(long long time, int id)
{
    stats[id] += time;

    if(time > max_stats[id])
        max_stats[id] = time;
}

static void print_stats(int loops)
{
    int i;

    printf("run stats: \n\n");
    for (i=0;i<NUM_STATS; ++i) {
        printf("\tstats[%2d] %8lld\tmax %8lld\n", i, stats[i]/loops, max_stats[i]);
    }
}


static void signal_handler(int num)
{
    running = 0;
}

/* the control loop for our helicopter */
int control_loop_run( )
{
    int first_time = 1;
    long long start, end;
    int loop_count;
    next_period   = get_utime( ) + us_period;
    altitude_mode = ALT_MODE_GROUND;

    if( ms_period != CONTROLLER_PERIOD )
    {
        javiator_port_send_ctrl_period( ms_period );
    }

    //javiator_port_send_enable_sensors( 1 );
    inertial_port_send_request( );
    wait_for_next_period( );

    loop_count = 0;
    while( running )
    {        
#if 0
        start = get_utime();
        if( send_motor_signals( ) )
        {
            break;
        }
        end = get_utime();
        calc_stats(end - start, STAT_SPI);

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
        calc_stats(end - start, STAT_JAV);
#endif

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
        end = get_utime();
        calc_stats(end - start, STAT_CMD);

        start = get_utime();
        get_command_data( );

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

        send_report_to_terminal( );
        end = get_utime();
        calc_stats(end - start, STAT_REP);

        if (++loop_count < 0) {
            loop_count--;
            break;
        }
        wait_for_next_period( );
    }

    print_stats(loop_count);

    return( 0 );
}

int control_loop_stop( )
{
    running = 0;
    return( 0 );
}

/* End of file */
