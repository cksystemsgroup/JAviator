/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   adc.c      Analog/Digital Converter interface.                          */
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

#include <avr/io.h>

#include "adc.h"


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the Analog/Digital Converter
*/
void adc_init( void )
{
    /* enable ADC, start conversion to initialize ADC, set prescaler 128 */
    ADCSR = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /* set ADC voltage reference to AVCC pin (internal VREF turned off) */
    ADMUX = (1 << REFS0);
}

/* Returns the conversion result for the given channel
*/
uint16_t adc_convert( uint8_t channel )
{
    /* clear channel setting */
    ADMUX &= 0xE0; /* ~00011111 = 11100000 */

    /* set new input channel */
    ADMUX |= channel;

    /* clear interrupt flag and start conversion */
    ADCSR |= (1 << ADSC) | (1 << ADIF);

    /* wait for conversion to complete */
    while( (ADCSR & (1 << ADSC)) )
        ;

    /* to obtain correct 10-bit result: ADCL must be read before ADCH
       (see page 247 in the ATmega128 manual for details) */
    return( ADCL | (ADCH << 8) );
}

/* End of file */
