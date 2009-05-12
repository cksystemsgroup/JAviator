/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   timer.c    Software timer used for setting notification flags.          */
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

#include "config.h"
#include "timer.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Configuration of Timer T0 to obtain a clock signal with a period of 1ms:
   Dividing clock rate 16MHz by 64 gives 250000 counts/sec or 250 counts/ms,
   respectively.  That is, Counter TCNT0 is incremented 250 times/ms, so
   there's an offset of 6 required to get an overflow interrupt at 256.
*/
#define TCNT0_OFFSET        0x06

/* Structure for representing timer events */
typedef struct
{
    uint8_t *   flag;       /* pointer to notification flag */
    uint16_t    period;     /* period of repetitive execution */
    uint16_t    ticks;      /* timer ticks that have passed */

} timer_event_t;

/* Global variables */
static timer_event_t        timer_event[ MAX_TIMER_EVENTS ];
static volatile int8_t      event_count;


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes Timer T0 used for the software timer
*/
void timer_init( void )
{
    /* initialize Timer T0 */
    TCNT0  = TCNT0_OFFSET;  /* load counter offset */
    TCCR0  = (1 << CS02);   /* set prescaler 64 */
    TIMSK |= (1 << TOIE0);  /* enable T0 interrupt */

    /* initialize event counter */
    event_count = 0;
}

/* Adds a new event to the timer event list.
   Returns the event's ID if successful, -1 otherwise.
*/
int8_t timer_add_event( uint8_t *flag, uint16_t period )
{
    /* check if event can be added */
    if( event_count == MAX_TIMER_EVENTS )
    {
        return( -1 );
    }

    /* initialize new event list entry */
    timer_event[ event_count ].flag   = flag;
    timer_event[ event_count ].period = period;
    timer_event[ event_count ].ticks  = 0;

    /* increment event counter */
    ++event_count;

    /* return ID of new event */
    return( event_count - 1 );
}

/* Sets the given period for the event with the given ID.
   Returns 0 if successful, -1 otherwise.
*/
int8_t timer_set_event( int8_t id, uint16_t period )
{
    /* check for valid event ID */
    if( id < 0 || id > event_count - 1 )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* set new period value */
    timer_event[ id ].period = period;

    sei( ); /* enable interrupts */

    return( 0 );
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
    int8_t i = 0;

    /* reload counter offset */
    TCNT0 = TCNT0_OFFSET;

    /* iterate over timer events */
    while( i < event_count )
    {
        if( ++timer_event[i].ticks >= timer_event[i].period )
        {
            timer_event[i].ticks = 0;
            *timer_event[i].flag = 1;
        }

        ++i;
    }
}

/* End of file */
