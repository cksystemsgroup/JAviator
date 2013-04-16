/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   parallel.c    Parallel interface used for communication.                */
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

#include "protocol.h"
#include "config.h"
#include "parallel.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Global variables */
static          uint8_t         rx_buf[ COMM_BUF_SIZE ];
static          uint8_t         tx_buf[ COMM_BUF_SIZE ];
static volatile uint8_t         rx_items;
static volatile uint8_t         tx_items;
static volatile uint8_t         rx_index;
static volatile uint8_t         tx_index;
static volatile uint8_t         new_data;
static volatile uint8_t         if_ready;
static volatile uint8_t         sending;

/* Forward declarations */
static void     do_send_data    ( void );
static void     do_recv_data    ( void );
static uint8_t  is_valid_data   ( const uint8_t *, uint8_t );


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the parallel interface
*/
void parallel_init( void )
{
    /* configure parallel port pins for input */
    PP_DATA_PORT = 0x00; /* disable all pull-ups */
    PP_DATA_DDR  = 0x00; /* make all pins inputs */

    /* configure DDR for RDY command/interrupt pin */
    PP_CTRL_DDR |=  (1 << PP_CMD_RDY); /* make RDY command an output */
    PP_CTRL_DDR &= ~(1 << PP_INT_RDY); /* make RDY interrupt an input */

    /* disable interrupts associated with RDY before changing EICRB */
    EIMSK       &= ~(1 << PP_CMD_RDY); /* disable RDY output interrupt */
    EIMSK       &= ~(1 << PP_INT_RDY); /* disable RDY input interrupt */

    /* any logical change on the RDY input generates an interrupt */
    EICRB       |=  (1 << PP_ISC_RDY);

    /* enable RDY interrupt */
    EIMSK       |=  (1 << PP_INT_RDY);

    /* initialize global variables */
    rx_items = 0;
    tx_items = 0;
    rx_index = 0;
    tx_index = 0;
    new_data = 0;
    if_ready = 0;
    sending  = 0;
}

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t parallel_is_new_data( void )
{
    /* check for RDY interrupt */
    if( if_ready )
    {
        if_ready = 0;

        if( sending )
        {
            do_send_data( );
        }
        else
        {
            do_recv_data( );
        }

        return( new_data );
    }

    /* check for TX items to transmit */
    if( !sending && tx_items )
    {
        sending = 1;

        /* make parallel port an output */
        PP_DATA_DDR = 0xFF;

        /* write data to parallel port */
        PP_DATA_PORT = tx_buf[ tx_index++ ];

        /* issue RDY command */
        PP_CTRL_PORT ^= (1 << PP_CMD_RDY);
    }

    return( new_data );
}

/* Copies the received data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t parallel_get_data( uint8_t *buf )
{
    /* check that we're not receiving data currently */
    if( !new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* copy received data to given buffer */
    memcpy( buf, rx_buf + 2, rx_buf[3] + COMM_OVERHEAD - 2 );

    /* clear new-data indicator */
    new_data = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}

/* Sends data via the parallel interface.
   Returns 0 if successful, -1 otherwise.
*/
int8_t parallel_send_data( uint8_t id, const uint8_t *data, uint8_t size )
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

    return( 0 );
}

/* Resets the parallel interface
*/
void parallel_reset( void )
{
    /* make parallel port an input */
    PP_DATA_DDR = 0x00;

    rx_items = 0;
    tx_items = 0;
    rx_index = 0;
    tx_index = 0;
    new_data = 0;
    if_ready = 0;
    sending  = 0;
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* Runs the sending procedure
*/
static void do_send_data( void )
{
    /* check for end of TX data stream */
    if( tx_index == tx_items )
    {
        tx_items = 0;
        tx_index = 0;
        sending  = 0;

        /* make parallel port an input */
        PP_DATA_DDR = 0x00;
    }
    else
    {
        /* write data to parallel port */
        PP_DATA_PORT = tx_buf[ tx_index++ ];

        /* issue RDY command */
        PP_CTRL_PORT ^= (1 << PP_CMD_RDY);
    }
}

/* Runs the receiving procedure
*/
static void do_recv_data( void )
{
    /* indicate that the receive buffer is being updated
       and thus data are no longer secure to be copied */
    new_data = 0;

    /* read data from parallel port */
    rx_buf[ rx_index ] = PP_DATA_REG;

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

    /* issue RDY command BEFORE validating data */
    PP_CTRL_PORT ^= (1 << PP_CMD_RDY);

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

/* RDY Interrupt callback function
*/
SIGNAL( PP_SIG_RDY )
{
    if_ready = 1;
}

/* End of file */
