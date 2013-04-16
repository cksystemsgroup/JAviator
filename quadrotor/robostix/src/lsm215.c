/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   lsm215.c    Interface for the Dimetix LSM2-15 distance laser sensor.    */
/*                                                                           */
/*   Copyright (c) 2006-2013 Rainer Trummer <rainer.trummer@gmail.com>       */
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
#include "lsm215.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Command definitions */
#define CMD_START           'h'     /* starts the continuous mode */
#define CMD_STOP            'c'     /* stops any current execution */
#define CMD_CR              '\r'    /* carrige return */
#define CMD_LF              '\n'    /* line feed */

/* RX/TX buffer sizes */
#define RX_DATA_SIZE        33      /* max size of received data */
#define TX_DATA_SIZE        3       /* max size of transmitted data */

/* Result buffer size */
#define NUM_DATA_DIGITS     8       /* number of data digits */

/* Global variables */
static          uint8_t     x_rx_buf[ RX_DATA_SIZE ];
static          uint8_t     y_rx_buf[ RX_DATA_SIZE ];
static          uint8_t     x_tx_buf[ TX_DATA_SIZE ];
static          uint8_t     y_tx_buf[ TX_DATA_SIZE ];
static volatile uint8_t     x_tx_items;
static volatile uint8_t     y_tx_items;
static volatile uint8_t     x_rx_index;
static volatile uint8_t     y_rx_index;
static volatile uint8_t     x_tx_index;
static volatile uint8_t     y_tx_index;
static volatile uint8_t     x_new_data;
static volatile uint8_t     y_new_data;


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the UART channels for the laser sensors
*/
void lsm215_init( void )
{
    /* Calculation of UBRR value for U2X = 0

       UBRR = fosc / (16 * baudrate) - 1

            = 16MHz / (16 * 19200bps) - 1

            = 16000000 / 307200 - 1

            = 52.083333333 - 1

            = (uint16_t) 0x0033
    */
    UBRR0H = 0x00; /* baudrate high byte */
    UBRR1H = 0x00;
    UBRR0L = 0x33; /* baudrate low byte */
    UBRR1L = 0x33;
    UCSR0A = 0x00; /* no double-speed mode */
    UCSR1A = 0x00;

    /* enable receiver and transmitter */
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR1B = (1 << RXEN1) | (1 << TXEN1);

    /* set frame format: parity even, 1 stop bit, 7 data bits */
    UCSR0C = (1 << UPM01) | (1 << UCSZ01);
    UCSR1C = (1 << UPM11) | (1 << UCSZ11);

    /* initialize global variables */
    x_tx_items = 0;
    y_tx_items = 0;
    x_rx_index = 0;
    y_rx_index = 0;
    x_tx_index = 0;
    y_tx_index = 0;
    x_new_data = 0;
    y_new_data = 0;

    /* just for the case the laser was not stopped
       properly during the previous operation */
    lsm215_stop( );
}

/* Starts the laser sensors in continuous mode
*/
void lsm215_start( void )
{
    /* write command streams */
    x_tx_buf[ x_tx_items++ ] = CMD_START;
    x_tx_buf[ x_tx_items++ ] = CMD_CR;
    x_tx_buf[ x_tx_items++ ] = CMD_LF;
    y_tx_buf[ y_tx_items++ ] = CMD_START;
    y_tx_buf[ y_tx_items++ ] = CMD_CR;
    y_tx_buf[ y_tx_items++ ] = CMD_LF;

    /* enable DRE interrupts */
    UCSR0B |= (1 << UDRIE0);
    UCSR1B |= (1 << UDRIE1);
}

/* Stops the laser sensors if in continuous mode
*/
void lsm215_stop( void )
{
    /* write command streams */
    x_tx_buf[ x_tx_items++ ] = CMD_STOP;
    x_tx_buf[ x_tx_items++ ] = CMD_CR;
    x_tx_buf[ x_tx_items++ ] = CMD_LF;
    y_tx_buf[ y_tx_items++ ] = CMD_STOP;
    y_tx_buf[ y_tx_items++ ] = CMD_CR;
    y_tx_buf[ y_tx_items++ ] = CMD_LF;

    /* enable DRE interrupts */
    UCSR0B |= (1 << UDRIE0);
    UCSR1B |= (1 << UDRIE1);
}

/* Returns 1 if new x-data available, 0 otherwise
*/
uint8_t lsm215_is_new_x_data( void )
{
    /* check for RX interrupt */
    if( (UCSR0A & (1 << RXC)) )
    {
        /* indicate that the receive buffer is being updated
           and thus data are no longer secure to be copied */
        x_new_data = 0;

        x_rx_buf[ x_rx_index ] = UDR0;

        /* check for end of RX x-data stream */
        if( x_rx_buf[ x_rx_index ] == CMD_LF || ++x_rx_index == RX_DATA_SIZE )
        {
            /* Data format if an error occurs: "@Ezzz\r\n", where
               the 3-digit value 'zzz' represents an error code. */
            if( x_rx_buf[0] != '@' )
            {
                x_new_data = 1;
            }

            x_rx_index = 0;
	    }
    }

    return( x_new_data );
}

/* Returns 1 if new y-data available, 0 otherwise
*/
uint8_t lsm215_is_new_y_data( void )
{
    /* check for RX interrupt */
    if( (UCSR1A & (1 << RXC)) )
    {
        /* indicate that the receive buffer is being updated
           and thus data are no longer secure to be copied */
        y_new_data = 0;

        y_rx_buf[ y_rx_index ] = UDR1;

        /* check for end of RX y-data stream */
        if( y_rx_buf[ y_rx_index ] == CMD_LF || ++y_rx_index == RX_DATA_SIZE )
        {
            /* Data format if an error occurs: "@Ezzz\r\n", where
               the 3-digit value 'zzz' represents an error code. */
            if( y_rx_buf[0] != '@' )
            {
                y_new_data = 1;
            }

            y_rx_index = 0;
	    }
    }

    return( y_new_data );
}

/* Copies the sampled x-data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t lsm215_get_x_data( uint8_t *buf )
{
    /* check that we're not receiving x-data currently */
    if( !x_new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* Data format in tracking mode: "31..06+xxxxxxxx 51....+00000000\r\n", where
       the 8-digit value 'xxxxxxxx' represents the measured distance in 1/10 mm. */
    memcpy( buf, x_rx_buf + 7, NUM_DATA_DIGITS - 1 ); /* skip 1/10-mm digit */
    buf[ NUM_DATA_DIGITS - 1 ] = 0; /* null-terminate last byte */

    /* clear new-data indicator */
    x_new_data = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}

/* Copies the sampled y-data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t lsm215_get_y_data( uint8_t *buf )
{
    /* check that we're not receiving y-data currently */
    if( !y_new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* Data format in tracking mode: "31..06+xxxxxxxx 51....+00000000\r\n", where
       the 8-digit value 'xxxxxxxx' represents the measured distance in 1/10 mm. */
    memcpy( buf, y_rx_buf + 7, NUM_DATA_DIGITS - 1 ); /* skip 1/10-mm digit */
    buf[ NUM_DATA_DIGITS - 1 ] = 0; /* null-terminate last byte */

    /* clear new-data indicator */
    y_new_data = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* UART0 Data Register Empty callback function
*/
SIGNAL( SIG_UART0_DATA )
{
    /* check for end of TX x-data stream */
    if( x_tx_index == x_tx_items )
    {
        /* disable DRE interrupt */
        UCSR0B &= ~(1 << UDRIE0);

        x_tx_items = 0;
        x_tx_index = 0;
    }
    else
    {
        UDR0 = x_tx_buf[ x_tx_index++ ];
    }
}

/* UART1 Data Register Empty callback function
*/
SIGNAL( SIG_UART1_DATA )
{
    /* check for end of TX y-data stream */
    if( y_tx_index == y_tx_items )
    {
        /* disable DRE interrupt */
        UCSR1B &= ~(1 << UDRIE1);

        y_tx_items = 0;
        y_tx_index = 0;
    }
    else
    {
        UDR1 = y_tx_buf[ y_tx_index++ ];
    }
}

/* End of file */
