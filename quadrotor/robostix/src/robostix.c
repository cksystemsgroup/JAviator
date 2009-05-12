/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   robostix.c     ATmega128-based low-level controller.                    */
/*                                                                           */
/*   Copyright (c) Rainer Trummer rtrummer@cs.uni-salzburg.at                */
/*                                                                           */
/*   This program is free software; you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by    */
/*   the Free Software Foundation; either version 2 of the License, or       */
/*   (at your option) any later version.                                     */
/*                                                                           */
/*   This program is distributed in the hope that it will be useful,         */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*   GNU General Public License for more details.                            */
/*                                                                           */
/*   You should have received a copy of the GNU General Public License       */
/*   along with this program; if not, write to the Free Software Foundation, */
/*   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.      */
/*                                                                           */
/*****************************************************************************/

#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../shared/protocol.h"
#include "../shared/transfer.h"
#include "config.h"
#include "ports.h"
#include "adc.h"
#include "timer.h"
#include "serial.h"
#include "dm3gx1.h"
#include "minia.h"
#include "pwm.h"
#include "leds.h"


/*****************************************************************************/
/*                                                                           */
/*   Controller Definitions                                                  */
/*                                                                           */
/*****************************************************************************/

/* Global variables */
static volatile uint8_t     flag_shut_down;
static volatile uint8_t     flag_check_delay;
static volatile uint8_t     periods_passed;
static volatile int8_t      id_check_delay;

/* Global structures */
static javiator_data_t      javiator_data;
static motor_signals_t      motor_signals;

/* Forward declarations */
void controller_init        ( void );
void process_data_packet    ( void );
void process_ctrl_period    ( uint8_t );
void process_motor_signals  ( const uint8_t *, uint8_t );
void process_shut_down      ( void );
void process_en_sensors     ( uint8_t );
void check_signals_delay    ( void );
void send_javiator_data     ( void );


/*****************************************************************************/
/*                                                                           */
/*   Controller Functions                                                    */
/*                                                                           */
/*****************************************************************************/

/* Initializes the controller
*/
void controller_init( void )
{
    /* initialize variables */
    flag_shut_down   = 0;
    flag_check_delay = 0;
    periods_passed   = 0;

    /* clear data structures */
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );

    /* initialize hardware */
    ports_init( );
    adc_init( );
    timer_init( );
    serial_init( );
    dm3gx1_init( );
    minia_init( );
    pwm_init( );
    leds_init( );

    /* register ADC channels */
    adc_add_channel( ADC_CH_MINIA );
    adc_add_channel( ADC_CH_MPX4115A );
    adc_add_channel( ADC_CH_BATTERY );

    /* register timer events */
    id_check_delay = timer_add_event( (uint8_t *) &flag_check_delay, CONTROLLER_PERIOD );

    /* set Robostix signal LEDs */
    LED_OFF( RED );
    LED_OFF( BLUE );
    LED_ON( YELLOW );

    /* enable interrupts */
    sei( );
}

/* Processes data packets received from the Gumstix
*/
void process_data_packet( void )
{
    static uint16_t valid_packets = 0;
    uint8_t  packet[ COMM_BUF_SIZE ], size;
    uint16_t checksum;

    /* receive packet from communication interface */
    if( serial_recv_packet( packet ) )
    {
        javiator_data.error |= JE_RECEIVE_PACKET;
        return;
    }

    /* check for valid packet content */
    size = packet[1] + COMM_OVERHEAD - 2;
    checksum = (packet[ size-2 ] << 8) | packet[ size-1 ];
    size -= 2;

    /* iterate over payload */
    while( size-- )
    {
        checksum -= packet[ size ];
    }

    /* check for non-zero checksum */
    if( checksum )
    {
        javiator_data.error |= JE_INVALID_DATA;
        valid_packets = 0;
        LED_ON( RED );
        return;
    }
    else
    if( ++valid_packets == MIN_VALID_PACKETS )
    {
        LED_OFF( RED );
    }

    /* call packet-dependent function */
    switch( packet[0] )
    {
        case COMM_CTRL_PERIOD:
            process_ctrl_period( packet[2] );
            break;

        case COMM_MOTOR_SIGNALS:
            process_motor_signals( packet + 2, packet[1] );
            break;

        case COMM_SHUT_DOWN:
            process_shut_down( );
            break;

        case COMM_EN_SENSORS:
            process_en_sensors( packet[2] );
            break;

        default:
            javiator_data.error |= JE_UNKNOWN_TYPE;
    }
}

/* Processes COMM_CTRL_PERIOD messages
*/
void process_ctrl_period( uint8_t period )
{
    /* check for valid update of period */
    if( !timer_set_event( id_check_delay, period ) )
    {
        javiator_data.state |= JS_PERIOD_UPDATED;
    }
}

/* Processes COMM_MOTOR_SIGNALS messages
*/
void process_motor_signals( const uint8_t *data, uint8_t size )
{
    static uint8_t toggle_blue = 0;

    /* reset counter when receiving new motor signals */
    periods_passed = 0;

    /* ignore received motor signals if shut-down flag is set */
    if( !flag_shut_down )
    {
        /* check for correct data size before extracting */
        if( size == MOTOR_SIGNALS_SIZE )
        {
            motor_signals.front = (data[0] << 8) | data[1];
            motor_signals.right = (data[2] << 8) | data[3];
            motor_signals.rear  = (data[4] << 8) | data[5];
            motor_signals.left  = (data[6] << 8) | data[7];

            /* check for invalid signals */
            if( pwm_set_signals( &motor_signals ) )
            {
                javiator_data.error |= JE_OUT_OF_RANGE;
            }

            /* visualize that motors have been updated */
            if( (++toggle_blue & 0x0F) == 0 )
            {
                LED_TOGGLE( BLUE );
            }
        }
        else
        {
            javiator_data.error |= JE_INVALID_SIZE;
            LED_OFF( BLUE );
        }
    }

    /* send JAviator data to controller */
    send_javiator_data( );
}

/* Processes COMM_SHUT_DOWN messages
*/
void process_shut_down( void )
{
    /* clear and set motor signals */
    memset( &motor_signals, 0, sizeof( motor_signals ) );
    pwm_set_signals( &motor_signals );

    /* set shut-down flag */
    flag_shut_down = 1;
    LED_ON( RED );
}

/* Processes COMM_EN_SENSORS messages
*/
void process_en_sensors( uint8_t enable )
{
    if( enable )
    {
        dm3gx1_start( );
        minia_start( );
    }
    else
    {
        dm3gx1_stop( );
        minia_stop( );

        /* Sensors will be disabled either by the Gumstix after the helicopter
           has settled or due to a loss of connectivity between Robostix and
           Gumstix.  Thus, in case a shut-down was issued, the shut-down flag
           can be cleared safely as soon as the sensors get disabled.  In case
           a loss of connectivity occurs, the motors are reved down to their
           minimum.  Hence the shut-down flag can be cleared independently of
           whether a shut-down was issued before the loss of connectivity. */
        flag_shut_down = 0;
        LED_OFF( RED );
    }
}

/* Checks if new motor signals arrive without delay
*/
void check_signals_delay( )
{
    static uint8_t first_timeout = 1;
    static uint8_t toggle_yellow = 0;

    /* check for exceeded waiting time */
    if( ++periods_passed > PERIODS_TO_WAIT )
    {
        /* ensure counter cannot overflow */
        periods_passed = PERIODS_TO_WAIT;

        /* DO NOT modify signals in shut-down mode! */
        if( !flag_shut_down )
        {
            if( motor_signals.front > 0 ||
                motor_signals.right > 0 ||
                motor_signals.rear  > 0 ||
                motor_signals.left  > 0 )
            {
                /* we're possibly airborne, so reduce
                   signals to force descending */
                --motor_signals.front;
                --motor_signals.right;
                --motor_signals.rear;
                --motor_signals.left;
            }
            else
            {
                /* we're possibly grounded, so keep
                   signals set to the minimum */
                motor_signals.front = 0;
                motor_signals.right = 0;
                motor_signals.rear  = 0;
                motor_signals.left  = 0;
            }

            pwm_set_signals( &motor_signals );
        }

        /* check if this is the first timeout */
        if( first_timeout )
        {
            first_timeout = 0;
            javiator_data.id = 0;

            /* disable sensors to save power */
            process_en_sensors( 0 );

            /* reset communication interface */
            serial_reset( );

            /* visualize that a timeout occurred */
            LED_ON( YELLOW );
            LED_OFF( BLUE );
        }
    }
    else
    {
        first_timeout = 1;

        /* visualize that we're connected */
        if( (++toggle_yellow & 0x0F) == 0 )
        {
            LED_TOGGLE( YELLOW );
        }
    }

    flag_check_delay = 0;
}

/* Sends the JAviator data to the controller
*/
void send_javiator_data( void )
{
    uint8_t data[ JAVIATOR_DATA_SIZE ];

    /* encode JAviator data */
    data[0]  = (uint8_t)( javiator_data.roll >> 8 );
    data[1]  = (uint8_t)( javiator_data.roll );
    data[2]  = (uint8_t)( javiator_data.pitch >> 8 );
    data[3]  = (uint8_t)( javiator_data.pitch );
    data[4]  = (uint8_t)( javiator_data.yaw >> 8 );
    data[5]  = (uint8_t)( javiator_data.yaw );
    data[6]  = (uint8_t)( javiator_data.droll >> 8 );
    data[7]  = (uint8_t)( javiator_data.droll );
    data[8]  = (uint8_t)( javiator_data.dpitch >> 8 );
    data[9]  = (uint8_t)( javiator_data.dpitch );
    data[10] = (uint8_t)( javiator_data.dyaw >> 8 );
    data[11] = (uint8_t)( javiator_data.dyaw );
    data[12] = (uint8_t)( javiator_data.ddx >> 8 );
    data[13] = (uint8_t)( javiator_data.ddx );
    data[14] = (uint8_t)( javiator_data.ddy >> 8 );
    data[15] = (uint8_t)( javiator_data.ddy );
    data[16] = (uint8_t)( javiator_data.ddz >> 8 );
    data[17] = (uint8_t)( javiator_data.ddz );
    data[18] = (uint8_t)( javiator_data.ticks >> 8 );
    data[19] = (uint8_t)( javiator_data.ticks );
    data[20] = (uint8_t)( javiator_data.sonar >> 8 );
    data[21] = (uint8_t)( javiator_data.sonar );
    data[22] = (uint8_t)( javiator_data.pressure >> 8 );
    data[23] = (uint8_t)( javiator_data.pressure );
    data[24] = (uint8_t)( javiator_data.battery >> 8 );
    data[25] = (uint8_t)( javiator_data.battery );
    data[26] = (uint8_t)( javiator_data.state >> 8 );
    data[27] = (uint8_t)( javiator_data.state );
    data[28] = (uint8_t)( javiator_data.error >> 8 );
    data[29] = (uint8_t)( javiator_data.error );
    data[30] = (uint8_t)( javiator_data.id >> 8 );
    data[31] = (uint8_t)( javiator_data.id );

    /* send JAviator data to the Gumstix */
    serial_send_packet( COMM_JAVIATOR_DATA, data, JAVIATOR_DATA_SIZE );

    /* clear state and error indicator */
    javiator_data.state = 0;
    javiator_data.error = 0;

    /* increment transmission ID */
    ++javiator_data.id;
}

/* Initializes and runs the controller
*/
int main( void )
{
    controller_init( );

    while( 1 )
    {
        /* check if shut-down flag is set */
        if( flag_shut_down )
        {
            javiator_data.state |= JS_SHUT_DOWN_MODE;
        }

        /* check if signals arrive in time */
        if( flag_check_delay )
        {
            check_signals_delay( );
        }

        /* check if a new packet is available */
        if( serial_is_new_packet( ) )
        {
            process_data_packet( );
        }

        /* check if new laser data available */
        if( dm3gx1_is_new_data( ) )
        {
            if( dm3gx1_get_data( &javiator_data ) )
            {
                javiator_data.error |= JE_IMU_GET_DATA;
            }
            else
            {
                javiator_data.state |= JS_NEW_IMU_DATA;
            }
        }

        /* check if new sonar data available */
        if( adc_is_new_data( ADC_CH_MINIA ) )
        {
            if( adc_get_data( ADC_CH_MINIA, &javiator_data.sonar ) )
            {
                javiator_data.error |= JE_SONAR_GET_DATA;
            }
            else
            {
                javiator_data.state |= JS_NEW_SONAR_DATA;
            }
        }

        /* check if new pressure data available */
        if( adc_is_new_data( ADC_CH_MPX4115A ) )
        {
            if( adc_get_data( ADC_CH_MPX4115A, &javiator_data.pressure ) )
            {
                javiator_data.error |= JE_PRESS_GET_DATA;
            }
            else
            {
                javiator_data.state |= JS_NEW_PRESS_DATA;
            }
        }

        /* check if new battery data available */
        if( adc_is_new_data( ADC_CH_BATTERY ) )
        {
            if( adc_get_data( ADC_CH_BATTERY, &javiator_data.battery ) )
            {
                javiator_data.error |= JE_BATT_GET_DATA;
            }
            else
            {
                javiator_data.state |= JS_NEW_BATT_DATA;
            }
        }
    }

    return( 0 );
}

/* End of file */
