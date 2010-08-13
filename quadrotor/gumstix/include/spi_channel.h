/*
 * Copyright (c) Harald Roeck hroeck@cs.uni-salzburg.at
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 */

#ifndef SPI_CHANNEL_H
#define SPI_CHANNEL_H

#include "comm_channel.h"

/* Emums for indicating a specific SPI type */
typedef enum
{
    SPI_DMA,
    SPI_DEV

} spi_type_t;

int spi_dma_channel_create( comm_channel_t *channel );

int spi_dma_channel_init( comm_channel_t *channel, char *interface, int baudrate );

int spi_dma_channel_destroy( comm_channel_t *channel );

int spi_dev_channel_create( comm_channel_t *channel );

int spi_dev_channel_init( comm_channel_t *channel, char *interface, int baudrate );

int spi_dev_channel_destroy( comm_channel_t *channel );

static inline
int spi_channel_create( comm_channel_t *channel, spi_type_t type )
{
	switch( type )
    {
		case SPI_DMA:
			return spi_dma_channel_create( channel );

		case SPI_DEV:
			return spi_dev_channel_create( channel );

		default:
			return( -1 );
	}
}

static inline
int spi_channel_init( comm_channel_t *channel, spi_type_t type, char *interface, int baudrate );
{
	switch( type )
    {
		case SPI_DMA:
			return spi_dma_channel_init( channel, interface, baudrate );

		case SPI_DEV:
			return spi_dev_channel_init( channel, interface, baudrate );

		default:
			return( -1 );
	}
}

static inline
int spi_channel_destroy( comm_channel_t *channel, spi_type_t type )
{
	switch( type )
    {
		case SPI_DMA:
			return spi_dma_channel_destroy( channel );

		case SPI_DEV:
			return spi_dev_channel_destroy( channel );

		default:
			return( -1 );
	}
}

#endif /* !SPI_CHANNEL_H */

/* End of file */
