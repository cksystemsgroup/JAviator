/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   adc.c      Analog-to-Digital Converter interface.                       */
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
#include <string.h>

#include "adc.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Number of channels */
#define ADC_CHANNELS        8

/* Global variables */
static          uint16_t    data_buf[ ADC_CHANNELS ];
static volatile int8_t      new_data[ ADC_CHANNELS ];
static volatile int8_t      selected_channel;


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the Analog-to-Digital Converter
*/
void adc_init( void )
{
    /* enable ADC unit and set prescaler 128 */
    ADCSR = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /* set ADC voltage reference to AVCC pin */
    ADMUX = (1 << REFS0);

    /* initialize data structures */
    memset( &data_buf,  0, sizeof( data_buf ) );
    memset( &new_data, -1, sizeof( new_data ) );

    /* disable selection mechanism */
    selected_channel = -1;
}

/* Performs a conversion for all registered channels.
   Returns 0 if successful, -1 otherwise.
*/
int8_t adc_convert( void )
{
    /* check for registered channels */
    if( selected_channel == -1 )
    {
        return( -1 );
    }

    /* start conversion cycle */
    ADCSR |= (1 << ADSC);

    return( 0 );
}

/* Adds a new channel to the ADC channel list.
   Returns 0 if successful, -1 otherwise.
*/
int8_t adc_add_channel( int8_t channel )
{
    if( channel < 0 || channel > ADC_CHANNELS - 1 )
    {
        return( -1 );
    }

    /* enable given channel for conversions */
    new_data[ channel ] = 0;

    /* check if this is the first channel added */
    if( selected_channel == -1 )
    {
        /* enable selection mechanism */
        selected_channel = channel;

        /* clear channel setting */
        ADMUX &= 0xE0; /* ~00011111 = 11100000 */

        /* set new input channel */
        ADMUX |= selected_channel;
    }

    return( 0 );
}

/* Returns -1 if the given channel is invalid or
   disabled, 1 if new data available, 0 otherwise
*/
int8_t adc_is_new_data( int8_t channel )
{
    static uint8_t num_channels = 0;

    if( channel < 0 || channel > ADC_CHANNELS - 1 )
    {
        return( -1 );
    }

    /* check for ADC interrupt */
    if( (ADCSR & (1 << ADIF)) )
    {
        /* clear ADC interrupt flag */
        ADCSR |= (1 << ADIF);

        /* to obtain correct 10-bit result: ADCL must be read before ADCH
           (see page 247 in the ATmega128 manual for details) */
        data_buf[ selected_channel ] = ADCL | (ADCH << 8);
        new_data[ selected_channel ] = 1;

        /* search for next enabled channel */
        while( 1 )
        {
            /* increment number of channels handled so far */
            ++num_channels;

            /* check for end of valid channels */
            if( ++selected_channel == ADC_CHANNELS )
            {
                selected_channel = 0;
            }

            /* check for enabled channel */
            if( new_data[ selected_channel ] != -1 )
            {
                /* clear channel setting */
                ADMUX &= 0xE0; /* ~00011111 = 11100000 */

                /* set new input channel */
                ADMUX |= selected_channel;

                break;
            }
        }

        /* check for end of conversion cycle */
        if( num_channels >= ADC_CHANNELS )
        {
            num_channels = 0;
        }
        else
        {
            /* start next conversion */
            ADCSR |= (1 << ADSC);
        }
    }

    return( new_data[ channel ] );
}

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t adc_get_data( int8_t channel, uint16_t *buf )
{
    if( channel < 0 || channel > ADC_CHANNELS - 1 )
    {
        return( -1 );
    }

    /* check that given channel is not disabled */
    if( new_data[ channel ] == -1 )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* copy result to given buffer */
    *buf = data_buf[ channel ];

    /* clear new-data indicator */
    new_data[ channel ] = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}

/* End of file */
