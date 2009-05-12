/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   lsm215.c   Interface for the Dimetix LSM2-15 distance laser sensor.     */
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
#include "lsm215.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Validation and selection of UART channel
*/
#if( UART_LSM215 == 0 )

#define UBRRnH          UBRR0H
#define UBRRnL          UBRR0L
#define UCSRnA          UCSR0A
#define UCSRnB          UCSR0B
#define UCSRnC          UCSR0C
#define RXCIEn          RXCIE0
#define RXENn           RXEN0
#define TXENn           TXEN0
#define UPMn1           UPM01
#define UCSZn1          UCSZ01
#define UDRIEn          UDRIE0
#define UDRn            UDR0
#define SIG_UARTn_DATA  SIG_UART0_DATA
#define SIG_UARTn_RECV  SIG_UART0_RECV

#elif( UART_LSM215 == 1 )

#define UBRRnH          UBRR1H
#define UBRRnL          UBRR1L
#define UCSRnA          UCSR1A
#define UCSRnB          UCSR1B
#define UCSRnC          UCSR1C
#define RXCIEn          RXCIE1
#define RXENn           RXEN1
#define TXENn           TXEN1
#define UPMn1           UPM11
#define UCSZn1          UCSZ11
#define UDRIEn          UDRIE1
#define UDRn            UDR1
#define SIG_UARTn_DATA  SIG_UART1_DATA
#define SIG_UARTn_RECV  SIG_UART1_RECV

#else /* UART_LSM215 */

#error No valid UART channel for the LSM2-15 sensor defined.

#endif /* UART_LSM215 */

/* Command definitions */
#define CMD_START           'h'     /* starts the continuous mode */
#define CMD_STOP            'c'     /* stops any current execution */
#define CMD_CR              '\r'    /* carrige return */
#define CMD_LF              '\n'    /* line feed */

/* TX/RX buffer sizes */
#define TX_DATA_SIZE        3       /* max size of transmitted data */
#define RX_DATA_SIZE        33      /* max size of received data */

/* Global variables */
static          uint8_t     tx_buf[ TX_DATA_SIZE ];
static          uint8_t     rx_buf[ RX_DATA_SIZE ];
static volatile uint8_t     tx_items;
static volatile uint8_t     tx_index;
static volatile uint8_t     rx_index;
static volatile uint8_t     new_data;

/* Forward declarations */
static uint32_t str_to_uint32( uint8_t *, uint8_t );


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the selected UART channel for the laser sensor
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
    UBRRnH = 0x00; /* baudrate high byte */
    UBRRnL = 0x33; /* baudrate low byte */
    UCSRnA = 0x00; /* no double-speed mode */

    /* enable receive-complete interrupt, receiver, and transmitter */
    UCSRnB = (1 << RXCIEn) | (1 << RXENn) | (1 << TXENn);

    /* set frame format: parity even, 1 stop bit, 7 data bits */
    UCSRnC = (1 << UPMn1) | (1 << UCSZn1);

    /* initialize global variables */
    tx_items = 0;
    tx_index = 0;
    rx_index = 0;
    new_data = 0;

    /* just for the case the laser was not stopped
       properly during the previous operation */
    lsm215_stop( );
}

/* Starts the laser sensor in continuous mode
*/
void lsm215_start( void )
{
    /* write command stream */
    tx_buf[ tx_items++ ] = CMD_START;
    tx_buf[ tx_items++ ] = CMD_CR;
    tx_buf[ tx_items++ ] = CMD_LF;

    /* enable DRE interrupt */
    UCSRnB |= (1 << UDRIEn);
}

/* Stops the laser sensor if in continuous mode
*/
void lsm215_stop( void )
{
    /* write command stream */
    tx_buf[ tx_items++ ] = CMD_STOP;
    tx_buf[ tx_items++ ] = CMD_CR;
    tx_buf[ tx_items++ ] = CMD_LF;

    /* enable DRE interrupt */
    UCSRnB |= (1 << UDRIEn);
}

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t lsm215_is_new_data( void )
{
    return( new_data );
}

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t lsm215_get_data( uint32_t *buf )
{
    uint8_t str[8];

    /* check that we're not receiving data currently */
    if( !new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* copy essential digits (see below) to conversion buffer */
    memcpy( str, rx_buf + 7, sizeof( str ) );

    /* clear new-data indicator */
    new_data = 0;

    sei( ); /* enable interrupts */

    /* Data format in tracking mode: "31..06+xxxxxxxx 51....+00000000\r\n", where
       the 8-digit value 'xxxxxxxx' represents the measured distance in 1/10 mm. */
    *buf = str_to_uint32( str, sizeof( str ) );

    return( 0 );
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* Converts the given data string to a 32-bit unsigned integer
*/
static uint32_t str_to_uint32( uint8_t *str, uint8_t n )
{
    uint8_t  *r = str, *s = str, *t, quo, rem;
    uint32_t res = 0;

    while( *r == '0' && n > 0 )
    {
        ++r;
        --n;
    }

    if( n == 0 )
    {
        return( res );
    }

    while( n-- > 0 )
    {
        if( *r < '0' || *r > '9' )
        {
            return( res );
        }

        *s++ = *r++ - '0';
    }

    for( n = 0, r = str; r < s; ++n )
    {
        t   = r;
        res = (res >> 1) | ( (uint32_t)( *(s-1) & 1 ) << 31 );
        rem = 0;

        while( t < s )
        {
            quo  = (uint8_t)( (*t + rem) >> 1 );
            rem  = (uint8_t)( (*t & 1) * 10 );
            *t++ = quo;
        }

        if( *r == 0 )
        {
            ++r;
        }
    }

    if( (n &= 31) > 0 )
    {
        res >>= 32 - n;
    }

    return( res );
}

/* UARTn Data Register Empty callback function
*/
SIGNAL( SIG_UARTn_DATA )
{
    /* check for end of TX data stream */
    if( tx_index == tx_items )
    {
        /* disable DRE interrupt */
        UCSRnB &= ~(1 << UDRIEn);

        tx_items = 0;
        tx_index = 0;
    }
    else
    {
        UDRn = tx_buf[ tx_index++ ];
    }
}

/* UARTn Receive Complete callback function
*/
SIGNAL( SIG_UARTn_RECV )
{
    /* indicate that the receive buffer is being updated
       and thus data are no longer secure to be copied */
    new_data = 0;

    rx_buf[ rx_index ] = UDRn;

    /* check for end of RX data stream */
    if( rx_buf[ rx_index ] == CMD_LF || ++rx_index == RX_DATA_SIZE )
    {
        /* Data format if an error occurs: "@Ezzz\r\n", where
           the 3-digit value 'zzz' represents an error code. */
        if( rx_buf[0] != '@' )
        {
            new_data = 1;
        }

        rx_index = 0;
    }
}

/* End of file */
