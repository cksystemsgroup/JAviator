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
#include "spi.h"
#include "lsm215.h"
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

/* Global structures */
static javiator_data_t      javiator_data;
static motor_signals_t      motor_signals;

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

    /* clear data structures */
    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );

    /* initialize hardware */
    ports_init( );
    adc_init( );
    wdog_init( );
	spi_init( );
    lsm215_init( );
    minia_init( );
    pwm_init( );
    leds_init( );

    /* register ADC channels */
    adc_add_channel( ADC_CH_MINIA );
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
    if( spi_recv_packet( packet ) )
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
            return;
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
            /* set new motor signals */
            pwm_set_signals( &motor_signals );

            /* get current ID to be returned to the Gumstix */
			javiator_data.id = motor_signals.id;

            /* visualize that motors have been updated */
            LED_TOGGLE( BLUE );
        }
        else
        {
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
        lsm215_start( );
        minia_start( );
    }
    else
    {
        lsm215_stop( );
        minia_stop( );

        memset( &javiator_data.x_pos, 0, sizeof( javiator_data.x_pos ) );
        memset( &javiator_data.y_pos, 0, sizeof( javiator_data.y_pos ) );

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

            /* reset communication interface */
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

    /* encode JAviator data */
	javiator_data_to_stream( &javiator_data, data, JAVIATOR_DATA_SIZE );

    /* transmit JAviator data */
	spi_send_packet( COMM_JAVIATOR_DATA, data, JAVIATOR_DATA_SIZE );

    /* reset state indicator */
    javiator_data.state = 0;

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
        /* check if signals arrive in time */
        if( flag_check_delay )
        {
            check_signals_delay( );
        }

        /* check if a new packet is available */
        if( spi_is_new_packet( ) )
        {
            process_data_packet( );
        }

        /* check if new sonar data available */
        if( adc_is_new_data( ADC_CH_MINIA ) )
        {
            if( !adc_get_data( ADC_CH_MINIA, &javiator_data.sonar ) )
            {
                javiator_data.state |= JS_NEW_SONAR_DATA;
            }
        }

        /* check if new battery data available */
        if( adc_is_new_data( ADC_CH_BATTERY ) )
        {
            if( !adc_get_data( ADC_CH_BATTERY, &javiator_data.battery ) )
            {
                javiator_data.state |= JS_NEW_BATT_DATA;
            }
        }

        /* check if new laser x-data available */
        if( lsm215_is_new_x_data( ) )
        {
            if( !lsm215_get_x_data( &javiator_data ) )
            {
                javiator_data.state |= JS_NEW_X_POS_DATA;
            }
        }

        /* check if new laser y-data available */
        if( lsm215_is_new_y_data( ) )
        {
            if( !lsm215_get_y_data( &javiator_data ) )
            {
                javiator_data.state |= JS_NEW_Y_POS_DATA;
            }
        }
    }

    return( 0 );
}

/* End of file */
