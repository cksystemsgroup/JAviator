/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   dm3gx1.c   Interface for the MicroStrain 3DM-GX1 IMU sensor device.     */
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
#include "dm3gx1.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Validation and selection of UART channel
*/
#if( UART_DM3GX1 == 0 )

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

#elif( UART_DM3GX1 == 1 )

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

#else /* UART_DM3GX1 */

#error No valid UART channel for the 3DM-GX1 sensor defined.

#endif /* UART_DM3GX1 */

/* Command definitions */
#define CMD_CONFIRMATION    0x00    /* null-byte confirmation */
#define CMD_CONTINUOUSLY    0x10    /* command for continuous mode */
#define CMD_DESIRED_DATA    0x31    /* command for desired data */

/* TX/RX buffer sizes */
#define TX_DATA_SIZE        24      /* max size of transmitted data */
#define RX_DATA_SIZE        23      /* max size of received data */

/* Global variables */
static          uint8_t     tx_buf[ TX_DATA_SIZE ];
static          uint8_t     rx_buf[ RX_DATA_SIZE ];
static volatile uint8_t     tx_items;
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

/* Initializes the selected UART channel for the IMU sensor
*/
void dm3gx1_init( void )
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
    tx_index = 0;
    rx_index = 0;
    new_data = 0;
}

/* Sends a command requesting a new data packet
*/
void dm3gx1_request( void )
{
    /* write command stream */
    tx_buf[ tx_items++ ] = CMD_DESIRED_DATA;

    /* enable DRE interrupt */
    UCSRnB |= (1 << UDRIEn);
}

/* Starts the IMU sensor in continuous mode
*/
void dm3gx1_start( void )
{
    /* write command stream */
    tx_buf[ tx_items++ ] = CMD_CONTINUOUSLY;
    tx_buf[ tx_items++ ] = CMD_CONFIRMATION;
    tx_buf[ tx_items++ ] = CMD_DESIRED_DATA;

    /* enable DRE interrupt */
    UCSRnB |= (1 << UDRIEn);
}

/* Stops the IMU sensor if in continuous mode
*/
void dm3gx1_stop( void )
{
    /* write command stream */
    tx_buf[ tx_items++ ] = CMD_CONTINUOUSLY;
    tx_buf[ tx_items++ ] = CMD_CONFIRMATION;
    tx_buf[ tx_items++ ] = CMD_CONFIRMATION;

    /* enable DRE interrupt */
    UCSRnB |= (1 << UDRIEn);
}

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t dm3gx1_is_new_data( void )
{
    return( new_data );
}

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
//int8_t dm3gx1_get_data( dm3gx1_data_t *buf )
int8_t dm3gx1_get_data( javiator_data_t *buf )
{
    /* check that we're not receiving data currently */
    if( !new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* extract sensor data */
    buf->roll   = (rx_buf[1]  << 8) | (rx_buf[2]  & 0xFF);
    buf->pitch  = (rx_buf[3]  << 8) | (rx_buf[4]  & 0xFF);
    buf->yaw    = (rx_buf[5]  << 8) | (rx_buf[6]  & 0xFF);
    buf->ddx    = (rx_buf[7]  << 8) | (rx_buf[8]  & 0xFF);
    buf->ddy    = (rx_buf[9]  << 8) | (rx_buf[10] & 0xFF);
    buf->ddz    = (rx_buf[11] << 8) | (rx_buf[12] & 0xFF);
    buf->droll  = (rx_buf[13] << 8) | (rx_buf[14] & 0xFF);
    buf->dpitch = (rx_buf[15] << 8) | (rx_buf[16] & 0xFF);
    buf->dyaw   = (rx_buf[17] << 8) | (rx_buf[18] & 0xFF);
    buf->ticks  = (rx_buf[19] << 8) | (rx_buf[20] & 0xFF);

    /* clear new-data indicator */
    new_data = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}

/* Sets the baudrate to 115200bps
   (for IMU configuration only)
*/
void dm3gx1_set_max_baudrate( void )
{
    /* set UARTn to IMU default baudrate 38400 */
    UCSRnB = 0x00; /* fully disable UARTn */
    UBRRnH = 0x00; /* baudrate high byte */
    UBRRnL = 0x19; /* baudrate low byte */
    UCSRnA = 0x00; /* no double-speed mode */

    /* enable receive-complete interrupt, receiver, and transmitter */
    UCSRnB = (1 << RXCIEn) | (1 << RXENn) | (1 << TXENn);

    /* set address 44 to value 21 */
    tx_buf[ tx_items++ ] = 0x09;
    tx_buf[ tx_items++ ] = 0x71;
    tx_buf[ tx_items++ ] = 0x2C;
    tx_buf[ tx_items++ ] = 0x00;
    tx_buf[ tx_items++ ] = 0x15;
    tx_buf[ tx_items++ ] = 0xAA;

    /* enable DRE interrupt */
    UCSRnB |= (1 << UDRIEn);
}

/* Sets the sampling rate to 100Hz
   (for IMU configuration only)
*/
void dm3gx1_set_max_sampling( void )
{
    /* set address 238 to value 4 */
    tx_buf[ tx_items++ ] = 0x09;
    tx_buf[ tx_items++ ] = 0x71;
    tx_buf[ tx_items++ ] = 0xEE;
    tx_buf[ tx_items++ ] = 0x00;
    tx_buf[ tx_items++ ] = 0x04;
    tx_buf[ tx_items++ ] = 0xAA;

    /* set address 240 to value 10 */
    tx_buf[ tx_items++ ] = 0x09;
    tx_buf[ tx_items++ ] = 0x71;
    tx_buf[ tx_items++ ] = 0xF0;
    tx_buf[ tx_items++ ] = 0x00;
    tx_buf[ tx_items++ ] = 0x0A;
    tx_buf[ tx_items++ ] = 0xAA;

    /* set address 242 to value 250 */
    tx_buf[ tx_items++ ] = 0x09;
    tx_buf[ tx_items++ ] = 0x71;
    tx_buf[ tx_items++ ] = 0xF2;
    tx_buf[ tx_items++ ] = 0x00;
    tx_buf[ tx_items++ ] = 0xFA;
    tx_buf[ tx_items++ ] = 0xAA;

    /* set address 246 to value 10 */
    tx_buf[ tx_items++ ] = 0x09;
    tx_buf[ tx_items++ ] = 0x71;
    tx_buf[ tx_items++ ] = 0xF6;
    tx_buf[ tx_items++ ] = 0x00;
    tx_buf[ tx_items++ ] = 0x0A;
    tx_buf[ tx_items++ ] = 0xAA;

    /* enable DRE interrupt */
    UCSRnB |= (1 << UDRIEn);
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
    uint16_t checksum = ( (data[ size-2 ] << 8) | data[ size-1 ] ) - data[0];

    size -= 3;

    /* iterate over payload */
    while( size )
    {
        checksum -= (data[ size-1 ] << 8) | data[ size ];
        size -= 2;
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

    /* check for synchronization with IMU sensor */
    if( rx_index == 0 && rx_buf[0] != CMD_DESIRED_DATA )
    {
        return; /* wait for next byte to come */
    }

    /* check for end of RX data stream */
    if( ++rx_index == RX_DATA_SIZE )
    {
        /* check for valid packet content */
        if( is_valid_data( rx_buf, RX_DATA_SIZE ) )
        {
            new_data = 1;
        }

        rx_index = 0;
    }
}

/* End of file */
