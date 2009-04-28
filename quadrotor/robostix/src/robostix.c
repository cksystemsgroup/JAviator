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
static volatile int16_t     signals_delay;

/* Global structures */
static sensor_data_t        sensor_data;
static pwm_signals_t        pwm_signals;

/* Forward declarations */
void controller_init        ( void );
void process_data_packet    ( void );
void process_shut_down      ( void );
void process_en_sensors     ( uint8_t );
void process_pwm_signals    ( const uint8_t *, uint8_t );
void check_signals_delay    ( void );
void send_sensor_data       ( void );


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
    signals_delay    = -SIGNALS_TIMEOUT;

    /* clear data structures */
    memset( &sensor_data, 0, sizeof( sensor_data ) );
    memset( &pwm_signals, 0, sizeof( pwm_signals ) );

    /* initialize hardware */
    ports_init( );
    adc_init( );
    timer_init( );
    spi_init( );
    lsm215_init( );
    minia_init( );
    pwm_init( );
    leds_init( );

    /* register desired timer events */
    timer_add_event( (uint8_t *) &flag_check_delay, CONTROLLER_PERIOD );

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
        /* notify that an error occurred */
        sensor_data.error |= RE_RECEIVE_PACKET;
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
        /* notify that an error occurred */
        sensor_data.error |= RE_INVALID_DATA;
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
        case COMM_SHUT_DOWN:
            process_shut_down( );
            break;

        case COMM_EN_SENSORS:
            process_en_sensors( packet[2] );
            break;

        case COMM_PWM_SIGNALS:
            process_pwm_signals( packet + 2, packet[1] );
            break;

        default:
            return;
    }
}

/* Processes COMM_SHUT_DOWN messages
*/
void process_shut_down( void )
{
    /* clear and set motor signals */
    memset( &pwm_signals, 0, sizeof( pwm_signals ) );
    pwm_set_signals( &pwm_signals );

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

/* Processes COMM_PWM_SIGNALS messages
*/
void process_pwm_signals( const uint8_t *data, uint8_t size )
{
    static uint8_t toggle_blue = 0;

    signals_delay = 0;

    /* ignore received motor signals if shut-down flag is set */
    if( !flag_shut_down )
    {
        /* check for correct data size before extracting */
        if( size == PWM_SIGNALS_SIZE )
        {
            pwm_signals.front = (data[0] << 8) | (data[1] & 0xFF);
            pwm_signals.right = (data[2] << 8) | (data[3] & 0xFF);
            pwm_signals.rear  = (data[4] << 8) | (data[5] & 0xFF);
            pwm_signals.left  = (data[6] << 8) | (data[7] & 0xFF);

            /* check for invalid signals */
            if( pwm_set_signals( &pwm_signals ) )
            {
                /* notify that an error occurred */
                sensor_data.error |= RE_OUT_OF_RANGE;
            }

            /* visualize that motors have been updated */
            if( (++toggle_blue & 0x0F) == 0 )
            {
                LED_TOGGLE( BLUE );
            }
        }
        else
        {
            /* notify that an error occurred */
            sensor_data.error |= RE_INVALID_SIZE;
            LED_OFF( BLUE );
        }
    }

    /* send new sensor data to the Gumstix */
    send_sensor_data( );
}

/* Checks if new motor signals arrive without delay
*/
void check_signals_delay( )
{
    static uint8_t first_timeout = 1;
    static uint8_t toggle_yellow = 0;

    signals_delay += CONTROLLER_PERIOD;

    /* check if a timeout occurred */
    if( signals_delay > SIGNALS_TIMEOUT )
    {
        /* ensure counter cannot overflow */
        signals_delay = SIGNALS_TIMEOUT;

        /* DO NOT modify signals in shut-down mode! */
        if( !flag_shut_down )
        {
            if( pwm_signals.front > 0 ||
                pwm_signals.right > 0 ||
                pwm_signals.rear  > 0 ||
                pwm_signals.left  > 0 )
            {
                /* we're possibly airborne, so reduce
                   signals to force descending */
                --pwm_signals.front;
                --pwm_signals.right;
                --pwm_signals.rear;
                --pwm_signals.left;
            }
            else
            {
                /* we're possibly grounded, so keep
                   signals set to the minimum */
                pwm_signals.front = 0;
                pwm_signals.right = 0;
                pwm_signals.rear  = 0;
                pwm_signals.left  = 0;
            }

            pwm_set_signals( &pwm_signals );
        }

        /* check if this is the first timeout */
        if( first_timeout )
        {
            first_timeout = 0;
            sensor_data.sequence = 0;

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
        if( (++toggle_yellow & 0x0F) == 0 )
        {
            LED_TOGGLE( YELLOW );
        }
    }

    flag_check_delay = 0;
}

/* Sends the sampled sensor data to the Gumstix
*/
void send_sensor_data( void )
{
    uint8_t data[ SENSOR_DATA_SIZE ];

    /* check if shut-down flag is set */
    if( flag_shut_down )
    {
        /* indicate that we're in shut-down mode */
        sensor_data.state |= RS_SHUT_DOWN_MODE;
    }

    /* check if new laser data available */
    if( lsm215_is_new_data( ) )
    {
        if( lsm215_get_data( &sensor_data.laser ) )
        {
            /* notify that an error occurred */
            sensor_data.error |= RE_LASER_GET_DATA;
        }
        else
        {
            /* notify that laser data have been updated */
            sensor_data.state |= RS_NEW_LASER_DATA;
        }
    }

    /* get ADC conversions */
    sensor_data.sonar    = adc_convert( ADC_CH_MINIA );
    sensor_data.pressure = adc_convert( ADC_CH_MPX4115A );
    sensor_data.battery  = adc_convert( ADC_CH_BATTERY );

    /* encode sensor data */
    data[0]  = (uint8_t)( sensor_data.laser >> 24 );
    data[1]  = (uint8_t)( sensor_data.laser >> 16 );
    data[2]  = (uint8_t)( sensor_data.laser >> 8 );
    data[3]  = (uint8_t)( sensor_data.laser );
    data[4]  = (uint8_t)( sensor_data.sonar >> 8 );
    data[5]  = (uint8_t)( sensor_data.sonar );
    data[6]  = (uint8_t)( sensor_data.pressure >> 8 );
    data[7]  = (uint8_t)( sensor_data.pressure );
    data[8]  = (uint8_t)( sensor_data.battery >> 8 );
    data[9]  = (uint8_t)( sensor_data.battery );
    data[10] = (uint8_t)( sensor_data.sequence >> 8 );
    data[11] = (uint8_t)( sensor_data.sequence );
    data[12] = (uint8_t)( sensor_data.state );
    data[13] = (uint8_t)( sensor_data.error );

    /* send encoded data to the Gumstix */
    spi_send_packet( COMM_SENSOR_DATA, data, SENSOR_DATA_SIZE );

    /* clear state and error indicator */
    sensor_data.state = 0;
    sensor_data.error = 0;

    /* increment sequence number */
    ++sensor_data.sequence;
}

/* Initializes and runs the controller
*/
int main( void )
{
    controller_init( );

    while( 1 )
    {
        /* check if a new packet is available */
        if( spi_is_new_packet( ) )
        {
            process_data_packet( );
        }

        /* check if signals arrive in time */
        if( flag_check_delay )
        {
            check_signals_delay( );
        }
    }

    return( 0 );
}

/* End of file */
