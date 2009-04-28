/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   ports.c    ATmega128 ports initialization procedure.                    */
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

#include "ports.h"


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes all ports and the ADC
*/
void ports_init( void )
{
    PORTA = INIT_PORTA;
    DDRA  = INIT_DDRA;

    PORTB = INIT_PORTB;
    DDRB  = INIT_DDRB;

    PORTC = INIT_PORTC;
    DDRC  = INIT_DDRC;

    PORTD = INIT_PORTD;
    DDRD  = INIT_DDRD;

    PORTE = INIT_PORTE;
    DDRE  = INIT_DDRE;

    PORTF = INIT_PORTF;
    DDRF  = INIT_DDRF;

    PORTG = INIT_PORTG;
    DDRG  = INIT_DDRG;
    ASSR  = INIT_ASSR;

    ADCSR = INIT_ADCSR;
    ADMUX = INIT_ADMUX;
}

/* End of file */
