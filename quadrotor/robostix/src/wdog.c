/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   wdog.c     Watchdog timer used for setting a notification flag.         */
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

#include "wdog.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Configuration of Timer T0 to obtain a clock signal with a period of 10ms:
   Dividing clock rate 16MHz by 1024 gives 15625 counts/sec or 156 counts/10ms,
   respectively.  That is, Counter TCNT0 is incremented 156 times/10ms, so
   there's an offset of 100 required to get an overflow interrupt at 256.
*/
#define TCNT0_OFFSET        0x64

/* Global variables */
static volatile uint8_t *   wdog_flag;
static volatile uint8_t     wdog_period;
static volatile uint8_t     wdog_ticks;


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes Timer T0 for the watchdog timer
*/
void wdog_init( void )
{
    /* initialize Timer T0 */
    TCNT0 = TCNT0_OFFSET; /* load counter offset */
    TCCR0 = (1 << CS02) | (1 << CS01) | (1 << CS00); /* set prescaler 1024 */

    /* initialize variables */
    wdog_flag   = 0;
    wdog_period = 0;
    wdog_ticks  = 0;
}

/* Registers the given flag and associated period
*/
void wdog_register_flag( uint8_t *flag, uint8_t period )
{
    wdog_flag   = flag;
    wdog_period = period;
    wdog_ticks  = 0;
}

/* Starts the watchdog timer
*/
void wdog_start( void )
{
    /* enable T0 interrupt */
    TIMSK |= (1 << TOIE0);
}

/* Stops the watchdog timer
*/
void wdog_stop( void )
{
    /* disable T0 interrupt */
    TIMSK &= ~(1 << TOIE0);
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* T0 Overflow callback function
*/
SIGNAL( SIG_OVERFLOW0 )
{
    /* reload counter offset */
    TCNT0 = TCNT0_OFFSET;

    /* check for valid pointer first */
    if( wdog_flag && (wdog_ticks += 10) >= wdog_period )
    {
        wdog_ticks = 0;
        *wdog_flag = 1;
    }
}

/* End of file */
