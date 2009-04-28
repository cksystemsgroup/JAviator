/* $Id: spi_channel.c,v 1.1 2008/10/16 14:41:13 rtrummer Exp $ */

/*
 * Copyright (c) Rainer Trummer rtrummer@cs.uni-salzburg.at
 *
 * University Salzburg, www.uni-salzburg.at
 * Department of Computer Science, cs.uni-salzburg.at
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define _POSIX_SOURCE 1

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "../shared/protocol.h"
#include "spi_channel.h"

// Base clock rate
#define PXA270_CLOCK    13000000    // 13-MHz PXA270 on-chip clock

// Mapping constants
#define MAP_SIZE        0x00000100  // address space is 256 bytes
#define MAP_MASK        0x000000FF  // mask for extended addresses

// Involved GPIO registers
#define OFFSET_GPIO     0x40E00000  // offset for GPIO registers
#define GPDR0           0x40E0000C  // GPIO Pin Direction Register          GPIO <31:00>
#define GPDR2           0x40E00014  // GPIO Pin Direction Register          GPIO <95:64>
#define GPSR2           0x40E00020  // GPIO Pin Output Set Register         GPIO <95:64>
#define GPCR2           0x40E0002C  // GPIO Pin Output Clear Register       GPIO <95:64>
#define GAFR0_L         0x40E00054  // GPIO Alternate Function Register     GPIO <15:00>
#define GAFR0_U         0x40E00058  // GPIO Alternate Function Register     GPIO <31:16>
#define GAFR2_L         0x40E00064  // GPIO Alternate Function Register     GPIO <79:64>

// Involved clock register
#define OFFSET_CLKM     0x41300000  // offset for Clocks Manager registers
#define CKEN            0x41300004  // Clock Enable Register

// Involved SSP2 registers
#define OFFSET_SSP2     0x41700000  // offset for SSP2 registers
#define SSCR0_2         0x41700000  // SSP2 Control Register 0
#define SSCR1_2         0x41700004  // SSP2 Control Register 1
#define SSSR_2          0x41700008  // SSP2 Status Register
#define SSDR_2          0x41700010  // SSP2 Data Register

// Bit-mask constants
#define PS11            0x00000800  // GPIO Pin Set 11 (chip select)
#define CKEN3           0x00000008  // SSP2 Clock Enable Pin (pin 3)
#define SCR             0x00000008  // Serial Clock Rate divider
#define SSE             0x00000080  // Synchronous Serial Enable
#define TNF             0x00000004  // TX FIFO Not Full flag
#define RNE             0x00000008  // RX FIFO Not Empty flag
#define BSY             0x00000010  // Busy (active port) flag
#define TFL             0x00000F00  // TX FIFO Level indicator
#define CSS             0x00400000  // Clock Synchronization Status

// Base setting for SSP2 Control Register 0
//
// +-+-+-----+-----+-+-+-+-+-----------------------+-+-+---+-------+
// |0|0|0 0 0|0 0 0|1|1|0|0|0 0 0 0 0 0 0 0 0 0 0 0|0|0|0 0|0 1 1 1|
// +-+-+-----+-----+-+-+-+-+-----------------------+-+-+---+-------+
//  M A   r     F   T R N E            S            S E  F     D
//  O C   e     R   I I C D            C            S C  R     S
//  D S   s     D   M M S S            R            E S  F     S
//        e     C         S
//        r
//        v
//        e
//        d
//
#define SSCR0_2_BASE    0x00C00007

// Base setting for SSP2 Control Register 1
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-------+-------+-+-+-+-+-+-+
// |0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0 1 1 1|0 0 1 1|0|0|0|0|0|0|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-------+-------+-+-+-+-+-+-+
//  T T E S E E S S R T T R T P r I S E    R       T    M S S L T R
//  H T B C C C C F W R S S I I e F T F    F       F    W P P B I I
//  E E C F R R L R O A R R N N s S R W    T       T    D H O M E E
//  L   E R A B K M T I E E T T e   F R                 S
//  P   I       D D   L     E E r
//              I I             v
//              R R             e
//                              d
//
#define SSCR1_2_BASE    0x00001CC0

typedef struct
{
    int  fd;
    int  baudrate;
    char device[SPI_MAX_NAME];
    char rx_buf[COMM_BUF_SIZE];
    int  rx_index;
    int  new_data;
    void *map_gpio;
    void *map_clkm;
    void *map_ssp2;
    void *map_reg;

} spi_connection;

static spi_connection connection;

// Macro for reading registers
#define GET_REG( name ) \
    static int get_##name( spi_connection *sc, int addr ) \
    { \
        sc->map_reg = sc->map_##name + (addr & MAP_MASK); \
        return( *(int *) sc->map_reg ); \
    }

// Macro for writing registers
#define SET_REG( name ) \
    static void set_##name( spi_connection *sc, int addr, int data ) \
    { \
        sc->map_reg = sc->map_##name + (addr & MAP_MASK); \
        *(int *) sc->map_reg = data; \
    }

// Reading instances
GET_REG( gpio )
GET_REG( clkm )
GET_REG( ssp2 )

// Writing instances
SET_REG( gpio )
SET_REG( clkm )
SET_REG( ssp2 )


static inline spi_connection *spi_get_connection( const struct channel *channel )
{
    spi_connection *sc = (spi_connection *) channel->data;

    if( !sc )
    {
        fprintf( stderr, "ERROR: SPI channel not correctly initialized\n" );
    }

    return( sc );
}

static inline char spi_tx_rx( spi_connection *sc, char tx )
{
    // wait for TX FIFO Level to become 0x0
    while( (get_ssp2( sc, SSSR_2 ) & TFL) )
        ;

    // wait for TX FIFO not to be full
    while( !(get_ssp2( sc, SSSR_2 ) & TNF) )
        ;

    // turn on Chip Select
    set_gpio( sc, GPCR2, get_gpio( sc, GPCR2 ) | PS11 );

    // write TX to data register
    set_ssp2( sc, SSDR_2, tx );

    // turn off Chip Select
    set_gpio( sc, GPSR2, get_gpio( sc, GPSR2 ) | PS11 );

    // wait for TX FIFO not to be full
    while( !(get_ssp2( sc, SSSR_2 ) & TNF) )
        ;

    // wait for SSP2 port to become idle or RX FIFO not to be empty
    while( (get_ssp2( sc, SSSR_2 ) & BSY) || !(get_ssp2( sc, SSSR_2 ) & RNE) )
        ;

    // read RX from data register
    return( (char) get_ssp2( sc, SSDR_2 ) );
}

static int spi_transmit( struct channel *channel, const char *tx_buf, int tx_items )
{
    spi_connection *sc = spi_get_connection( channel );
    int tx_index = 0;
    int rx_items = 0;

    if( !sc || tx_items < COMM_OVERHEAD || tx_items > COMM_BUF_SIZE )
    {
        return( -1 );
    }

    sc->rx_index = 0;
    sc->new_data = 0;

    while( 1 )
    {
        if( tx_items )
        {
            sc->rx_buf[sc->rx_index] = spi_tx_rx( sc, tx_buf[tx_index] );

            if( ++tx_index == tx_items )
            {
                tx_items = 0; // complete data packet transmitted
            }
        }
        else
        {
            sc->rx_buf[sc->rx_index] = spi_tx_rx( sc, 0 );
        }

        if( sc->rx_index == 0 ) // first packet mark
        {
            if( sc->rx_buf[0] == COMM_PACKET_MARK )
            {
                rx_items = COMM_OVERHEAD;
            }
        }
        else
        if( sc->rx_index == 1 ) // second packet mark
        {
            if( sc->rx_buf[1] != COMM_PACKET_MARK )
            {
                rx_items = 0;     // reset items counter
                sc->rx_index = 0; // reset buffer index
            }
        }
        else
        if( sc->rx_index == 3 ) // second header byte contains payload size
        {
            rx_items += sc->rx_buf[3]; // = communication overhead + payload size

            if( rx_items > COMM_BUF_SIZE ) // invalid packet size
            {
                rx_items = 0;     // reset items counter
                sc->rx_index = 0; // reset buffer index
            }
        }

        if( rx_items && ++sc->rx_index == rx_items )
        {
            rx_items = 0; // complete data packet received
        }

        if( !tx_items && !rx_items )
        {
            sc->rx_index = 0;
            sc->new_data = 1;
            break;
        }
    }

    return( 0 );
}

static int spi_receive( struct channel *channel, char *buf, int len )
{
    spi_connection *sc = spi_get_connection( channel );
    int bytes_copied = 0;

    if( !sc || !sc->new_data )
    {
        return( -1 );
    }

    while( len-- && sc->rx_index < COMM_BUF_SIZE )
    {
        *buf++ = sc->rx_buf[sc->rx_index++];
        ++bytes_copied;
    }

    return( bytes_copied );
}

static int spi_start( struct channel *channel )
{
    return( 0 );
}

static int spi_flush( struct channel *channel )
{
    return( 0 );
}

static int spi_poll( struct channel *channel )
{
    return( 0 );
}

int spi_channel_init( struct channel *channel, char *interface, int baudrate )
{
    spi_connection *sc = spi_get_connection( channel );
    int scr = (PXA270_CLOCK / baudrate) - 1;

    // check for valid connection
    if( !sc )
    {
        return( -1 );
    }

    // check for valid SCR value
    if( scr < 0 || scr > 4095 )
    {
        fprintf( stderr, "ERROR: invalid baudrate %d\n", baudrate );
        return( -1 );
    }

    // try to open device
    sc->fd = open( interface, O_RDWR | O_SYNC );

    // check for valid device
    if( sc->fd < 0 )
    {
        fprintf( stderr, "ERROR: open device %s\n", interface );
        return( -1 );
    }

    // store baudrate and device name
    sc->baudrate = baudrate;
    strncpy( sc->device, interface, SPI_MAX_NAME );

    // initialize variables
    sc->rx_index = 0;
    sc->new_data = 0;

    // initialize memory maps
    sc->map_gpio = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE,
        MAP_SHARED, sc->fd, OFFSET_GPIO );
    sc->map_clkm = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE,
        MAP_SHARED, sc->fd, OFFSET_CLKM );
    sc->map_ssp2 = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE,
        MAP_SHARED, sc->fd, OFFSET_SSP2 );

    // check for valid memory maps
    if( sc->map_gpio == MAP_FAILED ||
        sc->map_clkm == MAP_FAILED ||
        sc->map_ssp2 == MAP_FAILED )
    {
        close( sc->fd );
        fprintf( stderr, "ERROR: memory map\n" );
        return( -1 );
    }

    // disable SSP2 port
    set_ssp2( sc, SSCR0_2, get_ssp2( sc, SSCR0_2 ) & ~SSE );

    // disable SSP2 clock
    set_clkm( sc, CKEN, get_clkm( sc, CKEN ) & ~CKEN3 );

    // configure SSP2 control registers
    set_ssp2( sc, SSCR0_2, SSCR0_2_BASE | (scr << SCR) );
    set_ssp2( sc, SSCR1_2, SSCR1_2_BASE );

    // configure SSP2-involved GPIO registers
    set_gpio( sc, GPDR0,   (get_gpio( sc, GPDR0 )   & ~0x00086800) | 0x00002000 );
    set_gpio( sc, GPDR2,   (get_gpio( sc, GPDR2 )   & ~0x00000000) | 0x00000800 );
    set_gpio( sc, GAFR0_L, (get_gpio( sc, GAFR0_L ) & ~0x3CC00000) | 0x24800000 );
    set_gpio( sc, GAFR0_U, (get_gpio( sc, GAFR0_U ) & ~0x000000C0) | 0x00000040 );
    set_gpio( sc, GAFR2_L, (get_gpio( sc, GAFR2_L ) & ~0x00C00000) | 0x00000000 );

    // enable SSP2 clock
    set_clkm( sc, CKEN, get_clkm( sc, CKEN ) | CKEN3 );

    // enable SSP2 port
    set_ssp2( sc, SSCR0_2, get_ssp2( sc, SSCR0_2 ) | SSE );

    return( 0 );
}

int spi_channel_create( struct channel *channel )
{
    if( channel->data == NULL )
    {
        memset( &connection, 0, sizeof( connection ) );
        channel->type     = CH_SPI;
        channel->transmit = spi_transmit;
        channel->receive  = spi_receive;
        channel->start    = spi_start;
        channel->flush    = spi_flush;
        channel->poll     = spi_poll;
        channel->data     = &connection;
        return( 0 );
    }

    fprintf( stderr, "WARNING: SPI channel already initialized\n" );
    return( -1 );
}

int spi_channel_destroy( struct channel *channel )
{
    spi_connection *sc = spi_get_connection( channel );

    if( !sc )
    {
        return( -1 );
    }

    // disable SSP2 port
    set_ssp2( sc, SSCR0_2, get_ssp2( sc, SSCR0_2 ) & ~SSE );

    // disable SSP2 clock
    set_clkm( sc, CKEN, get_clkm( sc, CKEN ) & ~CKEN3 );

    close( sc->fd );
    channel->data = NULL;
    return( 0 );
}

// End of file.