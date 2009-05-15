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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define _POSIX_SOURCE 1

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <pthread.h>


#define DEV_NAME "/dev/robostix-spi2.0"
#define BUF_SIZE 64

#include "../shared/protocol.h"
#include "spi_channel.h"

struct spi_connection
{
	uint8_t rx_buf[BUF_SIZE];
    int  fd;
	int r_idx;
	int data_ready;
};

struct spi_connection connection;

static inline struct spi_connection *spi_get_connection(const comm_channel_t *channel)
{
    struct spi_connection *sc = (struct spi_connection *)channel->data;

    if (!sc) {
        fprintf( stderr, "ERROR in %s %d: SPI channel not correctly initialized\n",
            __FILE__, __LINE__ );
    }

    return sc;
}

static int spi_transmit( comm_channel_t *channel, const char *tx_buf, int tx_items )
{
    struct spi_connection *sc = spi_get_connection( channel );

    if (!sc || tx_items < COMM_OVERHEAD || tx_items > COMM_BUF_SIZE) {
        return( -1 );
    }

	write(sc->fd, tx_buf, tx_items);

    return 0;
}

#define min(a, b) (a<b?a:b)
static int spi_receive( comm_channel_t *channel, char *buf, int len )
{
    struct spi_connection *sc = spi_get_connection( channel );
	int count;

#if 0
	count = min(len, BUF_SIZE);
	if (sc->data_ready) {
		memcpy(buf, sc->rx_buf + sc->r_idx, count);
		sc->r_idx += count;
		if (sc->r_idx == BUF_SIZE)
			sc->r_idx = 0;
	}
#endif
	return read(sc->fd, buf + 2, len);
}

static int spi_start( comm_channel_t *channel )
{
    return( 0 );
}

static int spi_flush( comm_channel_t *channel )
{
    return( 0 );
}

static int spi_poll( comm_channel_t *channel, long timeout )
{
	struct spi_connection *sc = spi_get_connection( channel );
	struct timeval tv;
	fd_set readfs;
	int maxfd;

	if (!sc) {
		return -1;
	}

	if (timeout > 0) {
		tv.tv_usec = (timeout * 1000) % 1000000;
		tv.tv_sec  = timeout/1000;
	} else if (timeout < 0) {
		tv.tv_usec = 0;
		tv.tv_sec  = 0;
	}

	FD_SET(sc->fd, &readfs);
	maxfd = sc->fd + 1;

	return select(maxfd, &readfs, NULL, NULL, timeout?&tv:0);
}

int spi_dev_channel_init( comm_channel_t *channel, char *interface, int baudrate )
{
    struct spi_connection *sc = spi_get_connection( channel );

    if (!sc) {
        return( -1 );
    }

	sc->fd = open(DEV_NAME, O_RDWR);
	if (sc->fd == -1) {
		perror("open robostix dev");
		return -1;
	}

    return( 0 );
}

int spi_dev_channel_create( comm_channel_t *channel )
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

int spi_dev_channel_destroy( comm_channel_t *channel )
{
    struct spi_connection *sc = spi_get_connection( channel );

    if( !sc )
    {
        return( -1 );
    }

    close( sc->fd );
    channel->data = NULL;
    return( 0 );
}

/* End of file */
