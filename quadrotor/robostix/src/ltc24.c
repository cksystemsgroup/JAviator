/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   ltc24.c    Interface for the LTC24 ADCs (pressure conversion).          */
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
#include "ltc24.h"
#include "leds.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Validation and selection of the user-defined output rate
*/
#if( LTC24_NUM == 1 )

#define CNT_TOP     15000u  /*  16.67Hz (60ms) */

#elif( LTC24_NUM == 2 )

#define CNT_TOP     7500u   /*  33.32Hz (30ms) */

#elif( LTC24_NUM == 3 )

#define CNT_TOP     5000u   /*  50.00Hz (20ms) */

#elif( LTC24_NUM == 4 )

#define CNT_TOP     3750u   /*  66.67Hz (15ms) */

#elif( LTC24_NUM == 5 )

#define CNT_TOP     3000u   /*  83.32Hz (12ms) */

#elif( LTC24_NUM == 6 )

#define CNT_TOP     2500u   /* 100.00Hz (10ms) */

#else /* LTC24_NUM */

#error No valid number of ADC CS signals defined.

#endif /* LTC24_NUM */

/* Size of ADC data */
#define RX_DATA_SIZE        4

/* Global variables */
static          uint8_t     ltc24_cs[ LTC24_NUM ];
static volatile uint8_t     ltc24_id;
static          uint8_t     rx_buf[ RX_DATA_SIZE ];
static volatile uint8_t     rx_index;
static volatile uint8_t     new_data;
static volatile uint8_t     timer_if;
static volatile uint8_t     spi_rx;
static volatile uint8_t     spi_if;


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes Timer T1 and the SPI interface
*/
void ltc24_init( void )
{
    /* Calculation of TOP value for the CTC Mode at N Hz

       TOP = fosc / (prescaler * fctc)

           = 16MHz / (64 * N)

           = 250000 / N

       N =  16.67  ==>  TOP = 15000
       N =  33.32  ==>  TOP =  7500
       N =  50.00  ==>  TOP =  5000
       N =  66.67  ==>  TOP =  3750
       N =  83.32  ==>  TOP =  3000
       N = 100.00  ==>  TOP =  2500
    */
    /* set counter top and bottom value */
    ICR1   = CNT_TOP;
    TCNT1  = 0x00;

    /* set Compare Output Mode 0, Waveform Generation Mode 12, fosc/64 */
    TCCR1A = 0x00;
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
    TCCR1C = 0x00; /* not used */

    /* disable ICF1 interrupt */
    TIMSK &= ~(1 << TICIE1);

    /* make MOSI, SCK, and SS outputs, make MISO an input */
    DDRB  |=  (1 << DDB2) | (1 << DDB1) | (1 << DDB0);
    DDRB  &= ~(1 << DDB3);

    /* rise SS signal (hard-wired to the Gumstix) */
    PORTB |=  (1 << DDB0);

    /* enable SPI interrupt and interface, set Master mode, fosc/16 */
    SPCR   =  (1 << SPIE) | (1 << SPE) | (1 << MSTR) | (1 << SPR0);

    /* initialize LTC24 CS signals */
    for( ltc24_id = 0; ltc24_id < LTC24_NUM; ++ltc24_id )
    {
        /* store pin mask of CS signals */
        ltc24_cs[ ltc24_id ] = (1 << (LTC24_CS1 + ltc24_id));

        /* make all CS pins outputs */
        LTC24_DDR  |= ltc24_cs[ ltc24_id ];

        /* rise all CS signals */
        LTC24_PORT |= ltc24_cs[ ltc24_id ];
    }

    /* initialize global variables */
    ltc24_id = 0;
    rx_index = 0;
    new_data = 0;
    timer_if = 0;
    spi_rx   = 0;
    spi_if   = 0;

    LED_OFF( BLUE );
}

/* Starts all LTC24 ADCs in continuous mode
*/
void ltc24_start( void )
{
    /* enable ICF1 interrupt */
    TIMSK |= (1 << TICIE1);
}

/* Stops all LTC24 ADCs if in continuous mode
*/
void ltc24_stop( void )
{
    /* disable ICF1 interrupt */
    TIMSK &= ~(1 << TICIE1);

    /* rise selected CS signal */
    LTC24_PORT |= ltc24_cs[ ltc24_id ];

    /* reset global variables */
    ltc24_id = 0;
    rx_index = 0;
    new_data = 0;
    timer_if = 0;
    spi_rx   = 0;
    spi_if   = 0;

    LED_OFF( BLUE );
}

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t ltc24_is_new_data( void )
{
    /* check for timer interrupt */
    if( timer_if )
    {
        /* sink selected CS signal */
        LTC24_PORT &= ~ltc24_cs[ ltc24_id ];

        /* short 2-clock-cycle delay to give the port latches
           time to synchronize before checking the SDO signal */
        __asm__ __volatile__
        (
            "nop\n\t"
            "nop\n\t"
        );

        /* check for end of conversion (MISO <-- SDO) */
        if( (PINB & (1 << PINB3)) )
        {
            /* rise selected CS signal */
            LTC24_PORT |= ltc24_cs[ ltc24_id ];
        }
        else
        {
            timer_if = 0;
            rx_index = 0;

            /* start data transfer */
            SPDR = 0;
        }
    }

    /* check for SPI interrupt */
    if( spi_if )
    {
        spi_if = 0;

        /* indicate that the receive buffer is being updated
           and thus data are no longer secure to be copied */
        new_data = 0;

        /* read data from SPI port */
        rx_buf[ rx_index ] = spi_rx;

        /* check for end of data stream */
        if( ++rx_index == RX_DATA_SIZE )
        {
            /* rise selected CS signal */
            LTC24_PORT |= ltc24_cs[ ltc24_id ];

            /* select next CS signal */
            if( ++ltc24_id == LTC24_NUM )
            {
                ltc24_id = 0;

                LED_TOGGLE( BLUE );
            }

            new_data = 1;
        }
        else
        {
            /* start next transfer */
            SPDR = 0;
        }
    }

    return( new_data );
}

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t ltc24_get_data( uint32_t *buf )
{
    /* check that we're not receiving data currently */
    if( !new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* copy received data to given buffer */
    *buf   = rx_buf[0];
    *buf <<= 8;
    *buf  |= rx_buf[1];
    *buf <<= 8;
    *buf  |= rx_buf[2];
    *buf <<= 8;
    *buf  |= rx_buf[3];

    /* clear new-data indicator */
    new_data = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* T1 Input Capture Interrupt callback function
*/
SIGNAL( SIG_INPUT_CAPTURE1 )
{
    timer_if = 1;
}

/* SPI Transmission Complete callback function
*/
SIGNAL( SIG_SPI )
{
    spi_rx = SPDR;
    spi_if = 1;
}

/* End of file */
