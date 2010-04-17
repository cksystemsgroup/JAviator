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
static volatile uint8_t     new_data[ ADC_CHANNELS ];
static volatile uint8_t     current_ch;
static volatile uint8_t     waiting_ch;


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
    memset( &data_buf, 0, sizeof( data_buf ) );
    memset( &new_data, 0, sizeof( new_data ) );

    /* initialize global variables */
    current_ch = ADC_CHANNELS;
    waiting_ch = ADC_CHANNELS;
}

/* Performs a conversion for the given channel.
   Returns 0 if successful, -1 otherwise.
*/
int8_t adc_convert( uint8_t channel )
{
    if( channel >= ADC_CHANNELS )
    {
        return( -1 );
    }

    /* check if ADC is converting */
    if( current_ch < ADC_CHANNELS )
    {
        waiting_ch = channel;
        return( -1 );
    }

    current_ch = channel;

    /* clear channel setting */
    ADMUX &= 0xE0; /* ~00011111 = 11100000 */

    /* set new input channel */
    ADMUX |= current_ch;

    /* start conversion cycle */
    ADCSR |= (1 << ADSC);

    return( 0 );
}

/* Returns -1 if the given channel is invalid,
   1 if new data available, 0 otherwise.
*/
int8_t adc_is_new_data( uint8_t channel )
{
    if( channel >= ADC_CHANNELS )
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
        data_buf[ current_ch ] = ADCL | (ADCH << 8);
        new_data[ current_ch ] = 1;

        if( waiting_ch < ADC_CHANNELS )
        {
            current_ch = waiting_ch;
            waiting_ch = ADC_CHANNELS;

            /* clear channel setting */
            ADMUX &= 0xE0; /* ~00011111 = 11100000 */

            /* set new input channel */
            ADMUX |= current_ch;

            /* start conversion cycle */
            ADCSR |= (1 << ADSC);
        }
        else
        {
            current_ch = ADC_CHANNELS;
        }
    }

    return( new_data[ channel ] );
}

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t adc_get_data( uint8_t channel, uint16_t *buf )
{
    if( channel >= ADC_CHANNELS )
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
