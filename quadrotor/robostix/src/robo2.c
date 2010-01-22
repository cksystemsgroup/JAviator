/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   robo2.c        ATmega128-based low-level controller.                    */
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
#include "config.h"
#include "ports.h"
#include "wdog.h"
#include "adc.h"
#include "parallel.h"
#include "lsm215.h"
#include "minia.h"
#include "leds.h"


/*****************************************************************************/
/*                                                                           */
/*   Controller Definitions                                                  */
/*                                                                           */
/*****************************************************************************/

/* Global variables */
static volatile uint8_t     flag_check_delay;
static volatile uint8_t     flag_new_request;

/* Global structures */
static javiator_sdat_t      javiator_data;

/* Forward declarations */
void controller_init        ( void );
void process_data_packet    ( void );
void process_sensor_data    ( const uint8_t *, uint8_t );
void check_receive_delay    ( void );
void send_sensor_data       ( void );
void enable_sensors         ( uint8_t );


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
    flag_check_delay = 0;
    flag_new_request = 0;

    /* clear data structures */
    memset( &javiator_data, 0, sizeof( javiator_data ) );

    /* initialize hardware */
    ports_init( );
    wdog_init( );
    adc_init( );
    parallel_init( );
    lsm215_init( );
    minia_init( );
    leds_init( );

    /* register watchdog event and start timer */
    wdog_register_flag( (uint8_t *) &flag_check_delay, NOTIFY_PERIOD );
    wdog_start( );

    /* register ADC channels */
    adc_add_channel( ADC_CH_BATT );
    adc_add_channel( ADC_CH_TEMP );
    adc_add_channel( ADC_CH_SONAR );

    /* set Robostix signal LEDs */
    LED_ON( RED );
    LED_ON( BLUE );
    LED_ON( YELLOW );

    /* enable interrupts */
    sei( );
}

/* Processes data packets received from Robostix 1
*/
void process_data_packet( void )
{
    uint8_t  packet[ COMM_BUF_SIZE ], size;
    uint16_t checksum;

    /* get data from parallel interface */
    if( parallel_get_data( packet ) )
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

        default:
            return;
    }
}

/* Processes COMM_SENSOR_DATA messages
*/
void process_sensor_data( const uint8_t *data, uint8_t size )
{
    static uint8_t delay = 0;

    /* notify that a new request has been received */
    flag_new_request = 1;

    /* extract motor signals ID */
    javiator_data.id = (data[ size-3 ] << 8) | data[ size-2 ];

    /* update sensor status */
    enable_sensors( data[ size-1 ] );

    /* send sensor data to Robostix 1 */
    send_sensor_data( );

    if( ++delay % 10 == 0 )
    {
        LED_TOGGLE( YELLOW );
    }
}

/* Checks for delays in communication from Robostix 1 to Robostix 2
*/
void check_receive_delay( )
{
    /* check if a new request has been received */
    if( !flag_new_request )
    {
        /* disable specific sensors to save power */
        enable_sensors( 0 );

        /* reset Robostix-1-to-Robostix-2 interface */
        parallel_reset( );

        LED_ON( YELLOW );
    }

    flag_check_delay = 0;
    flag_new_request = 0;
}

/* Sends the sensor data to Robostix 1
*/
void send_sensor_data( void )
{
    uint8_t data[ JAVIATOR_SDAT_SIZE ];

    /* encode sensor data */
	javiator_sdat_to_stream( &javiator_data, data, JAVIATOR_SDAT_SIZE );

    /* transmit sensor data */
	parallel_send_data( COMM_SENSOR_DATA, data, JAVIATOR_SDAT_SIZE );

    /* reset state indicator */
    javiator_data.state = 0;

    /* start next ADC cycle */
    adc_convert( );
}

/* Enables/disables specific sensors
*/
void enable_sensors( uint8_t enable )
{
    static uint8_t sensors_enabled = 0;

    /* check for changed sensor status */
    if( sensors_enabled != enable )
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

            LED_ON( RED );
            LED_ON( BLUE );
        }

        /* store new sensor status */
        sensors_enabled = enable;
    }
}

/* Initializes and runs Robostix 2
*/
int main( void )
{
    controller_init( );

    while( 1 )
    {
        /* check if communication is upright */
        if( flag_check_delay )
        {
            check_receive_delay( );
        }

        /* check if new parallel data available */
        if( parallel_is_new_data( ) )
        {
            process_data_packet( );
        }

        /* check if new battery data available */
        if( adc_is_new_data( ADC_CH_BATT ) )
        {
            if( !adc_get_data( ADC_CH_BATT, &javiator_data.batt ) )
            {
                javiator_data.state |= JS_NEW_BATT_DATA;
            }
        }

        /* check if new temperature data available */
        if( adc_is_new_data( ADC_CH_TEMP ) )
        {
            if( !adc_get_data( ADC_CH_TEMP, &javiator_data.temp ) )
            {
                javiator_data.state |= JS_NEW_TEMP_DATA;
            }
        }

        /* check if new sonar data available */
        if( adc_is_new_data( ADC_CH_SONAR ) )
        {
            if( !adc_get_data( ADC_CH_SONAR, &javiator_data.sonar ) )
            {
                javiator_data.state |= JS_NEW_SONAR_DATA;
            }
        }

        /* check if new laser x-data available */
        if( lsm215_is_new_x_data( ) )
        {
            if( !lsm215_get_x_data( javiator_data.x_pos ) )
            {
                javiator_data.state |= JS_NEW_X_POS_DATA;
                LED_TOGGLE( RED );
            }
        }

        /* check if new laser y-data available */
        if( lsm215_is_new_y_data( ) )
        {
            if( !lsm215_get_y_data( javiator_data.y_pos ) )
            {
                javiator_data.state |= JS_NEW_Y_POS_DATA;
                LED_TOGGLE( BLUE );
            }
        }
    }

    return( 0 );
}

/* End of file */
