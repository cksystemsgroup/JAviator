/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   serial.c   Serial interface used for communication with the Gumstix.    */
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

#include "../shared/protocol.h"
#include "config.h"
#include "serial.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Validation and selection of UART channel
*/
#if( UART_SERIAL == 0 )

#define UBRRnH          UBRR0H
#define UBRRnL          UBRR0L
#define UCSRnA          UCSR0A
#define UCSRnB          UCSR0B
#define UCSRnC          UCSR0C
#define RXCIEn          RXCIE0
#define RXENn           RXEN0
#define TXENn           TXEN0
#define UCSZn1          UCSZ01
#define UCSZn0          UCSZ00
#define UDRIEn          UDRIE0
#define UDRn            UDR0
#define SIG_UARTn_DATA  SIG_UART0_DATA
#define SIG_UARTn_RECV  SIG_UART0_RECV

#elif( UART_SERIAL == 1 )

#define UBRRnH          UBRR1H
#define UBRRnL          UBRR1L
#define UCSRnA          UCSR1A
#define UCSRnB          UCSR1B
#define UCSRnC          UCSR1C
#define RXCIEn          RXCIE1
#define RXENn           RXEN1
#define TXENn           TXEN1
#define UCSZn1          UCSZ11
#define UCSZn0          UCSZ10
#define UDRIEn          UDRIE1
#define UDRn            UDR1
#define SIG_UARTn_DATA  SIG_UART1_DATA
#define SIG_UARTn_RECV  SIG_UART1_RECV

#else /* UART_SERIAL */

#error No valid UART channel for serial communication defined.

#endif /* UART_SERIAL */

/* Global variables */
static          uint8_t     tx_buf[ COMM_BUF_SIZE ];
static          uint8_t     rx_buf[ COMM_BUF_SIZE ];
static volatile uint8_t     tx_items;
static volatile uint8_t     rx_items;
static volatile uint8_t     tx_index;
static volatile uint8_t     rx_index;
static volatile uint8_t     new_data;

/* Forward declarations */
static uint8_t  is_valid_data( const uint8_t *, uint8_t );


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the selected UART channel
*/
void serial_init( void )
{
    /* Calculation of UBRR value for U2X = 1

       UBRR = fosc / (8 * baudrate) - 1

            = 16MHz / (8 * 115200bps) - 1

            = 16000000 / 1843200 - 1

            = 17.361111111 - 1

            = (uint16_t) 0x0010
    */
    UBRRnH = 0x00; /* baudrate high byte */
    UBRRnL = 0x10; /* baudrate low byte */
    UCSRnA = 0x02; /* double-speed mode */

    /* enable receive-complete interrupt, receiver, and transmitter */
    UCSRnB = (1 << RXCIEn) | (1 << RXENn) | (1 << TXENn);

    /* set frame format: parity none, 1 stop bit, 8 data bits */
    UCSRnC = (1 << UCSZn1) | (1 << UCSZn0);

    /* initialize global variables */
    tx_items = 0;
    rx_items = 0;
    tx_index = 0;
    rx_index = 0;
    new_data = 0;
}

/* Returns 1 if a new packet is available, 0 otherwise
*/
uint8_t serial_is_new_packet( void )
{
   return( new_data );
}

/* Copies the received data packet to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t serial_recv_packet( uint8_t *buf )
{
    /* check that we're not receiving data currently */
    if( !new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* copy received data packet to given buffer */
    memcpy( buf, rx_buf + 2, rx_buf[3] + COMM_OVERHEAD - 2 );

    /* clear new-data indicator */
    new_data = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}

/* Sends a data packet via the selected UART channel.
   Returns 0 if successful, -1 otherwise.
*/
int8_t serial_send_packet( uint8_t id, const uint8_t *data, uint8_t size )
{
    uint16_t checksum = id + size;

    /* check that we're not transmitting data currently */
    if( tx_items )
    {
        return( -1 );
    }

    /* write packet header */
    tx_buf[ tx_items++ ] = COMM_PACKET_MARK;
    tx_buf[ tx_items++ ] = COMM_PACKET_MARK;
    tx_buf[ tx_items++ ] = id;
    tx_buf[ tx_items++ ] = size;

    /* write packet payload */
    while( size-- )
    {
        checksum += *data;
        tx_buf[ tx_items++ ] = *data++;
    }

    /* write packet checksum */
    tx_buf[ tx_items++ ] = (uint8_t)( checksum >> 8 );
    tx_buf[ tx_items++ ] = (uint8_t)( checksum );

    /* enable DRE interrupt */
    UCSRnB |= (1 << UDRIEn);

    return( 0 );
}

/* Resets the selected UART channel
*/
void serial_reset( void )
{
    /* disable DRE interrupt */
    UCSRnB &= ~(1 << UDRIEn);

    tx_items = 0;
    rx_items = 0;
    tx_index = 0;
    rx_index = 0;
    new_data = 0;
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* Returns 1 if the packet contains valid data, 0 otherwise
*/
static uint8_t is_valid_data( const uint8_t *data, uint8_t size )
{
    uint16_t checksum = (data[ size-2 ] << 8) | data[ size-1 ];

    size -= 2;

    /* iterate over payload */
    while( size-- )
    {
        checksum -= data[ size ];
    }

    return( checksum == 0 );
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

    if( rx_index == 0 )
    {
        /* check for first packet mark */
        if( rx_buf[0] == COMM_PACKET_MARK )
        {
            rx_items = COMM_OVERHEAD;
        }
    }
    else
    if( rx_index == 1 )
    {
        /* check for second packet mark */
        if( rx_buf[1] != COMM_PACKET_MARK )
        {
            rx_items = 0;
            rx_index = 0;
        }
    }
    else
    if( rx_index == 3 )
    {
        /* second header byte contains payload size */
        rx_items += rx_buf[3];

        /* check for valid packet size */
        if( rx_items > COMM_BUF_SIZE )
        {
            rx_items = 0;
            rx_index = 0;
        }
    }

    /* check for end of RX data stream */
    if( rx_items && ++rx_index == rx_items )
    {
        /* check for valid packet content */
        if( is_valid_data( rx_buf + 2, rx_items - 2 ) )
        {
            new_data = 1;
        }

        rx_items = 0;
        rx_index = 0;
    }
}

/* End of file */
