/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   minia.c    Interface for the SensComp Mini-A sonar sensor.              */
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

#include "config.h"
#include "minia.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Configuration of Timer T2 to obtain a clock signal with a period of 10ms:
   Dividing clock rate 16MHz by 1024 gives 15625 counts/sec or 156 counts/10ms,
   respectively.  That is, Counter TCNT2 is incremented 156 times/10ms, so
   there's an offset of 100 required to get an overflow interrupt at 256.
*/
#define TCNT2_OFFSET    0x64


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes Timer T2 used for the sonar sensor
*/
void minia_init( void )
{
    /* initialize Timer T2 */
    TCNT2 = TCNT2_OFFSET;              /* load counter offset */
    TCCR2 = (1 << CS22) | (1 << CS20); /* set prescaler 1024 */

    /* make trigger signal an output */
    MINIA_DDR |= (1 << MINIA_TRIGGER);

    /* just for the case the sonar was not stopped
       properly during the previous operation */
    minia_stop( );
}

/* Starts the sonar sensor in continuous mode
*/
void minia_start( void )
{
    /* rise trigger signal */
    MINIA_PORT &= ~(1 << MINIA_TRIGGER);

    /* enable T2 interrupt */
    TIMSK |= (1 << TOIE2);
}

/* Stops the sonar sensor if in continuous mode
*/
void minia_stop( void )
{
    /* disable T2 interrupt */
    TIMSK &= ~(1 << TOIE2);

    /* trail trigger signal */
    MINIA_PORT |= (1 << MINIA_TRIGGER);
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* T2 Overflow callback function
*/
SIGNAL( SIG_OVERFLOW2 )
{
    /* reload counter offset */
    TCNT2 = TCNT2_OFFSET;

    /* toggle trigger signal */
    MINIA_PORT ^= (1 << MINIA_TRIGGER);
}

/* End of file */
