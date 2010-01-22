/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   robo1.c        ATmega128-based low-level controller.                    */
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

#include "protocol.h"
#include "transfer.h"
#include "javiator.h"
#include "signals.h"
#include "config.h"
#include "ports.h"
#include "wdog.h"
#include "serial.h"
#include "parallel.h"
#include "dm3gx1.h"
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
static volatile uint8_t     flag_new_signals;
static volatile uint8_t     flag_new_sensors;

/* Global structures */
static javiator_ldat_t      javiator_data;
static motor_signals_t      motor_signals;

/* Forward declarations */
void controller_init        ( void );
void process_data_packet    ( void );
void process_sensor_data    ( const uint8_t *, uint8_t );
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
    flag_new_signals = 0;
    flag_new_sensors = 0;

    /* clear data structures */
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );

    /* initialize hardware */
    ports_init( );
    wdog_init( );
	serial_init( );
    parallel_init( );
    dm3gx1_init( );
    pwm_init( );
    leds_init( );

    /* register watchdog event and start timer */
    wdog_register_flag( (uint8_t *) &flag_check_delay, NOTIFY_PERIOD );
    wdog_start( );

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
    uint8_t  packet[ COMM_BUF_SIZE ], size;
    uint16_t checksum;

    /* get data from serial or parallel interface */
    if( serial_get_data( packet ) && parallel_get_data( packet ) )
    {
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
        return;
    }

    /* call packet-dependent function */
    switch( packet[0] )
    {
        case COMM_SENSOR_DATA:
            process_sensor_data( packet + 2, packet[1] );
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
            return;
    }
}

/* Processes COMM_SENSOR_DATA messages
*/
void process_sensor_data( const uint8_t *data, uint8_t size )
{
    static uint8_t delay = 0;

    /* notify that new sensor data have been received */
    flag_new_sensors = 1;

    /* check for correct data size before extracting */
    javiator_sdat_from_stream( (javiator_sdat_t *) &javiator_data, data, size );

    if( ++delay % 10 == 0 )
    {
        LED_TOGGLE( BLUE );
    }
}

/* Processes COMM_MOTOR_SIGNALS messages
*/
void process_motor_signals( const uint8_t *data, uint8_t size )
{
    static uint8_t delay = 0;

    /* notify that new motor signals have been received */
    flag_new_signals = 1;

    /* check for correct data size before setting motor signals */
    if( !motor_signals_from_stream( &motor_signals, data, size ) &&
        !flag_shut_down )
    {
        /* set new motor signals */
        pwm_set_signals( &motor_signals );
    }

    /* send JAviator data to the Gumstix */
    send_javiator_data( );

    if( ++delay % 10 == 0 )
    {
        LED_TOGGLE( YELLOW );
    }
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
    /* notify Robostix 2 that sensors should be enabled/disabled */
    parallel_send_data( COMM_EN_SENSORS, &enable, 1 );

    if( enable )
    {
        /* clear shut-down flag */
        flag_shut_down = 0;
        LED_OFF( RED );
    }
}

/* Checks for delays in communication from the Gumstix to Robostix 1
   as well as delays in communication from Robostix 2 to Robostix 1
*/
void check_signals_delay( )
{
    static uint8_t first_timeout = 1;

    /* check if new motor signals have been received */
    if( !flag_new_signals )
    {
        /* DO NOT modify motor signals in shut-down mode! */
        if( !flag_shut_down )
        {
            if( motor_signals.front > 0 ||
                motor_signals.right > 0 ||
                motor_signals.rear  > 0 ||
                motor_signals.left  > 0 )
            {
                /* we're possibly airborne, so reduce
                   motor signals to force descending */
                motor_signals.front -= MOTOR_DEC;
                motor_signals.right -= MOTOR_DEC;
                motor_signals.rear  -= MOTOR_DEC;
                motor_signals.left  -= MOTOR_DEC;
            }
            else
            {
                /* we're possibly grounded, so keep
                   motor signals set to the minimum */
                motor_signals.front = 0;
                motor_signals.right = 0;
                motor_signals.rear  = 0;
                motor_signals.left  = 0;
            }

            /* set new motor signals */
            pwm_set_signals( &motor_signals );
        }

        /* check if this is the first timeout */
        if( first_timeout )
        {
            first_timeout = 0;

            /* disable sensors to save power */
            process_en_sensors( 0 );
        }

        /* reset Gumstix-to-Robostix-1 interface */
        serial_reset( );

        LED_ON( YELLOW );
    }
    else
    {
        first_timeout = 1;
    }

    /* check if new sensor data have been received */
    if( !flag_new_sensors )
    {
        /* reset Robostix-2-to-Robostix-1 interface */
        parallel_reset( );

        LED_OFF( BLUE );
    }

    flag_check_delay = 0;
    flag_new_signals = 0;
    flag_new_sensors = 0;
}

/* Sends the JAviator data to the Gumstix
*/
void send_javiator_data( void )
{
    uint8_t data[ JAVIATOR_LDAT_SIZE ];

    /* DO NOT send a request to Robostix 2 when the
       shut-down flag is set, because there is no need
       for sampling sensor data in shut-down mode */
    if( flag_shut_down )
    {
        /* set JAviator data ID when motor
           signals ID not sent on round trip */
        javiator_data.id = motor_signals.id;
    }
    else
    {
        /* increment motor signals ID before
           being sent on round trip */
        ++motor_signals.id;

        /* encode motor signals ID */
        data[0] = (uint8_t)( motor_signals.id >> 8 );
        data[1] = (uint8_t)( motor_signals.id );

        /* request new sensor data */
        parallel_send_data( COMM_SENSOR_DATA, data, 2 );

        /* request new IMU data */
        dm3gx1_request( );
    }

    /* encode JAviator data */
	javiator_ldat_to_stream( &javiator_data, data, JAVIATOR_LDAT_SIZE );

    /* transmit JAviator data */
	serial_send_data( COMM_JAVIATOR_DATA, data, JAVIATOR_LDAT_SIZE );
}

/* Initializes and runs Robostix 1
*/
int main( void )
{
    controller_init( );

    while( 1 )
    {
        /* check if signals arrive in time */
        if( flag_check_delay )
        {
            check_signals_delay( );
        }

        /* check if new serial or parallel data available */
        if( serial_is_new_data( ) || parallel_is_new_data( ) )
        {
            process_data_packet( );
        }

        /* check if new IMU data available */
        if( dm3gx1_is_new_data( ) )
        {
            dm3gx1_get_data( &javiator_data );
        }
    }

    return( 0 );
}

/* End of file */
