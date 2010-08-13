/*
 * Copyright (c) Harald Roeck hroeck@cs.uni-salzburg.at
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

#define _POSIX_SOURCE 1

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "spi_channel.h"
#include "protocol.h"
#include "us_timer.h"

#define NUM_SPI_DATA    16

typedef struct
{
	uint8_t rx_buf[COMM_BUF_SIZE];
	int r_idx;
	int data_ready;
	spi_data_t *next;

} spi_data_t;

typedef struct
{
	int  fd;
	pthread_t thread;
	spi_data_t *active;
	spi_data_t data[NUM_SPI_DATA];
	spi_data_t *list_free;
	spi_data_t *list_pending;
	pthread_mutex_t lock;
	pthread_cond_t cond;

} spi_connection_t;

static void print_data(spi_data_t *data) __attribute__((used));
static void print_data(spi_data_t *data)
{
	int i;

	for(i=0;i<data->data_ready;++i) {
		printf("%2x ", data->rx_buf[i]);
	}
	printf("\n");
}

static inline void lock(spi_connection_t *sc)
{
	pthread_mutex_lock(&sc->lock);
}

static inline void unlock(spi_connection_t *sc)
{
	pthread_mutex_unlock(&sc->lock);
}

#define make_helper(name)                                               \
static inline spi_data_t *__get_##name(spi_connection_t *sc)            \
{                                                                       \
	spi_data_t *data = sc->list_##name;                                 \
	if (data)                                                           \
		sc->list_##name = data->next;                                   \
                                                                        \
	return data;                                                        \
}                                                                       \
                                                                        \
static inline spi_data_t *get_##name(spi_connection_t *sc)              \
{                                                                       \
	spi_data_t *data;                                                   \
	lock(sc);                                                           \
	data = __get_##name(sc);                                            \
	unlock(sc);                                                         \
	return data;                                                        \
}                                                                       \
                                                                        \
static inline void __put_##name(spi_connection_t *sc, spi_data_t *data) \
{                                                                       \
	data->next = sc->list_##name;                                       \
	sc->list_##name = data;                                             \
}                                                                       \
                                                                        \
static inline void put_##name(spi_connection_t *sc, spi_data_t *data)   \
{                                                                       \
	lock(sc);                                                           \
	__put_##name(sc, data);                                             \
	unlock(sc);                                                         \
}                                                                       \

make_helper(free);
make_helper(pending);


static inline spi_data_t *get_active_pending(spi_connection_t *sc)
{
	spi_data_t *data;
	lock(sc);
	if(sc->active) {
		data = sc->active;
	} else {
		data = __get_pending(sc);
		sc->active = data;
	}
	unlock(sc);
	return data;
}

static inline void put_active_pending(spi_connection_t *sc, spi_data_t *data)
{
	assert(data == sc->active);
	assert(data->r_idx <= data->data_ready);
	if(data->r_idx == data->data_ready) {
		lock(sc);
		sc->active = NULL;
		data->r_idx = 0;
		data->data_ready = 0;
		__put_free(sc, data);
		pthread_cond_broadcast(&sc->cond);
		unlock(sc);
	}
}

static inline void put_pending_delete(spi_connection_t *sc, spi_data_t *data)
{
	spi_data_t *next;
	spi_data_t *to_free;
	lock(sc);
	__put_pending(sc, data);
	/* look for an old packet of the same type */
	next = data;
	while (next->next != NULL) {
		if(next->next->rx_buf[2] == data->rx_buf[2]) {
			to_free = next->next;
			next->next = to_free->next;
			__put_free(sc, to_free);
			break;
		}
		next = next->next;
	}

	unlock(sc);
}

static inline spi_connection_t *spi_get_connection(const comm_channel_t *channel)
{
    spi_connection_t *sc = (spi_connection_t *)channel->data;

    if (!sc) {
        fprintf( stderr, "ERROR in %s %d: SPI channel not correctly initialized\n",
            __FILE__, __LINE__ );
    }

    return sc;
}

static int spi_transmit(comm_channel_t *channel, const char *tx_buf, int tx_items)
{
    spi_connection_t *sc = spi_get_connection( channel );
	int sent, ret;
    if (!sc || tx_items < COMM_OVERHEAD || tx_items > COMM_COMM_BUF_SIZE) {
        return( -1 );
    }

	sent = 0;
	do {
		ret = write(sc->fd, tx_buf + sent, tx_items);
		if (ret<0) {
			perror("spi_transmit");
			if(errno != EAGAIN)
				return ret;
		} else {
			sent += ret;
			tx_items -= ret;
		}
	} while(tx_items);
    return 0;
}

#define min(a, b) (a<b?a:b)
static int spi_receive( comm_channel_t *channel, char *buf, int len )
{
    spi_connection_t *sc = spi_get_connection( channel );
	spi_data_t *data;
	int count;
	data = get_active_pending(sc);
	if (data) {
		count = min(len, data->data_ready - data->r_idx);
		memcpy(buf, data->rx_buf + data->r_idx, count);
		data->r_idx += count;
		put_active_pending(sc, data);
	} else {
		count = -1;
		errno = EAGAIN;
	}

	return count;
}

static void *spi_thread(void *arg)
{
	spi_connection_t *sc = (spi_connection_t *)arg;
	spi_data_t *data;
	int res, data_read;
	uint64_t start, end;

	data_read = 0;
	while (1) {
		lock(sc);
		data = __get_free(sc);
		while (data == NULL) {
			pthread_cond_wait(&sc->cond, &sc->lock);
			data = __get_free(sc);
		}
		unlock(sc);
		/* reading always blocks until at least the packet header is ready.
		 */
		start = get_utime();
again:
		res = read(sc->fd, data->rx_buf + data_read, COMM_BUF_SIZE - data_read);
		if (res <= 0) {
			printf("res %d, data_read %d, COMM_BUF_SIZE %d\n", res, data_read, COMM_BUF_SIZE);
			perror("read spi device");
			put_free(sc, data);
		} else {
			data_read += res;
			if (data_read < data->rx_buf[3] + 6 && data_read < COMM_BUF_SIZE)
				goto again;

			end = get_utime();
			calc_stats(end-start, STAT_READ);
			data->data_ready = data_read;
			data->r_idx = 0;
			data_read = 0;
			//print_data(data);
			put_pending_delete(sc, data);
		}
	}
	return NULL;
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
	spi_connection_t *sc = spi_get_connection( channel );
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

int spi_dev_channel_create( comm_channel_t *channel )
{
    spi_connection_t *sc;
	int i;

    if( channel->data != NULL )
    {
        fprintf( stderr, "WARNING: SPI channel already initialized\n" );
        return( -1 );
    }

    sc = malloc( sizeof( spi_connection_t ) );

    if( !sc )
    {
        fprintf( stderr, "ERROR in %s %d: memory allocation\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    memset( sc, 0, sizeof( spi_connection_t ) );

    pthread_mutex_init(&connection.lock, NULL);
    pthread_cond_init(&connection.cond, NULL);

    for( i = 0; i < NUM_SPI_DATA; ++i )
    {
        put_free(sc, sc->data[i]);
    }

    channel->type     = CH_SPI;
    channel->transmit = spi_transmit;
    channel->receive  = spi_receive;
    channel->start    = spi_start;
    channel->flush    = spi_flush;
    channel->poll     = spi_poll;
    channel->data     = sc;
    return( 0 );
}

int spi_dev_channel_init( comm_channel_t *channel, char *interface, int baudrate )
{
    spi_connection_t *sc = spi_get_connection( channel );

    if (!sc) {
        return( -1 );
    }

	sc->fd = open(interface, O_RDWR);
	if (sc->fd == -1) {
		perror("open robostix dev");
		return -1;
	}

	struct sched_param param;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	sched_getparam(0, &param);
	if (param.sched_priority > 0) {
		/* javiator connection got a higher priority than the controller */
		param.sched_priority++;
		pthread_attr_setschedparam(&attr, &param);
		pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	}
	pthread_create(&sc->thread, &attr, spi_thread, sc);
    return( 0 );
}

int spi_dev_channel_destroy( comm_channel_t *channel )
{
    spi_connection_t *sc = spi_get_connection( channel );

    if( !sc )
    {
        return( -1 );
    }

    close( sc->fd );
    free( sc );
    channel->data = NULL;
    return( 0 );
}

/* End of file */
