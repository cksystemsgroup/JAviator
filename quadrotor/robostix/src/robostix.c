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

#include "shared/protocol.h"
#include "shared/transfer.h"
#include "config.h"
#include "ports.h"
#include "adc.h"
#include "wdog.h"
#include "serial.h"
#include "spi.h"
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
static volatile uint8_t     flag_new_signals;
static volatile uint8_t     flag_send_serial;
static volatile uint8_t     flag_send_spi;

/* Global structures */
static javiator_data_t      javiator_data;
static motor_signals_t      motor_signals;
uint8_t current_response = RESP_FULL;

/* Forward declarations */
void controller_init        ( void );
void process_data_packet    ( void );
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
    flag_send_serial = 0;
    flag_send_spi    = 0;

    /* clear data structures */
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );

    /* initialize hardware */
    ports_init( );
    adc_init( );
    wdog_init( );
    serial_init( );
	spi_init( );
    dm3gx1_init( );
    minia_init( );
    pwm_init( );
    leds_init( );

    /* register ADC channels */
    adc_add_channel( ADC_CH_MINIA );
    /*adc_add_channel( ADC_CH_MPX4115A );*/
    adc_add_channel( ADC_CH_BATTERY );

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
    static uint16_t valid_packets = 0;
    uint8_t  packet[ COMM_BUF_SIZE ], size;
    uint16_t checksum;

    /* receive packet from communication interface */
    if( serial_recv_packet( packet ) )
    {
        if( spi_recv_packet( packet ) )
        {
            javiator_data.error |= JE_RECEIVE_PACKET;
            return;
        }
        else
        {
            flag_send_spi = 1;
        }
    }
    else
    {
        flag_send_serial = 1;
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

    /* check for non-zero checksum*/
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

/* Processes COMM_MOTOR_SIGNALS messages
*/
void process_motor_signals( const uint8_t *data, uint8_t size )
{
    /* notify that new motor signals arrived */
    flag_new_signals = 1;

    /* ignore received motor signals if shut-down flag is set */
    if( !flag_shut_down )
    {
        /* check for correct data size before extracting */
        if( motor_signals_from_stream( &motor_signals, data, size ) == 0 )
        {
            /* check for invalid signals */
            if( pwm_set_signals( &motor_signals ) )
            {
                javiator_data.error |= JE_OUT_OF_RANGE;
            }

            /* get current ID to be returned to the Gumstix */
			javiator_data.id = motor_signals.id;

            /* visualize that motors have been updated */
            LED_TOGGLE( BLUE );
        }
        else
        {
            javiator_data.error |= JE_INVALID_SIZE;
            LED_OFF( BLUE );
        }
    }

    /* send JAviator data to the Gumstix */
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

    /* check if new signals have been received */
    if( !flag_new_signals )
    {
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
                motor_signals.front -= MOTOR_DEC;
                motor_signals.right -= MOTOR_DEC;
                motor_signals.rear  -= MOTOR_DEC;
                motor_signals.left  -= MOTOR_DEC;
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

            /* disable sensors to save power */
            process_en_sensors( 0 );

            /* reset communication interfaces */
            serial_reset( );
            spi_reset( );

            /* visualize that a timeout occurred */
            LED_ON( YELLOW );
            LED_OFF( BLUE );
        }
    }
    else
    {
        first_timeout = 1;

        /* visualize that we're connected */
        LED_TOGGLE( YELLOW );
    }

    flag_check_delay = 0;
    flag_new_signals = 0;
}

/* Sends the JAviator data to the controller
*/
void send_javiator_data( void )
{
    uint8_t data[ JAVIATOR_DATA_SIZE ];

	javiator_data_to_stream(&javiator_data, data, JAVIATOR_DATA_SIZE);

    /* send JAviator data to the Gumstix */
    if( flag_send_serial )
    {
        flag_send_serial = 0;
        serial_send_packet( COMM_JAVIATOR_DATA, data, JAVIATOR_DATA_SIZE );
    }
    else
    if( flag_send_spi )
    {
        flag_send_spi = 0;

	    if( spi_send_packet( COMM_JAVIATOR_DATA, data, JAVIATOR_DATA_SIZE ) )
        {
			LED_TOGGLE( RED ); /* could not send packet; previous packet still enqueued */
		}
    }

    /* clear state and error indicator */
    javiator_data.state = 0;
    javiator_data.error = 0;

    /* start next ADC cycle */
    adc_convert( );
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
        if( serial_is_new_packet( ) || spi_is_new_packet( ) )
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
#if 0
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
#endif
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
