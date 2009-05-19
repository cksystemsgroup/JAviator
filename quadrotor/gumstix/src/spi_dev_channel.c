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
#include <assert.h>
#include <errno.h>

#define DEV_NAME "/dev/robostix-spi2.0"
#define BUF_SIZE 64
#define N_BUF    16

#include "../shared/protocol.h"
#include "spi_channel.h"

struct spi_data;
struct spi_data
{
	uint8_t rx_buf[BUF_SIZE];
	int r_idx;
	int data_ready;
	struct spi_data *next;
};

struct spi_connection
{
	int  fd;
	pthread_t thread;

	struct spi_data *active;
	struct spi_data data[N_BUF];
	struct spi_data *list_free;
	struct spi_data *list_pending;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};

static void print_data(struct spi_data *data)
{
	int i;

	for(i=0;i<data->data_ready;++i) {
		printf("%2x ", data->rx_buf[i]);
	}
	printf("\n");
}

static inline void lock(struct spi_connection *sc)
{
	pthread_mutex_lock(&sc->lock);
}

static inline void unlock(struct spi_connection *sc)
{
	pthread_mutex_unlock(&sc->lock);
}

#define make_helper(name)                                                \
static inline struct spi_data *__get_##name(struct spi_connection *sc)   \
{                                                                        \
	struct spi_data *data = sc->list_##name;                             \
	if (data)                                                            \
		sc->list_##name = data->next;                                    \
                                                                         \
	return data;                                                         \
}                                                                        \
                                                                         \
static inline struct spi_data *get_##name(struct spi_connection *sc)     \
{                                                                        \
	struct spi_data *data;                                               \
	lock(sc);                                                            \
	data = __get_##name(sc);                                             \
	unlock(sc);                                                          \
	return data;                                                         \
}                                                                        \
                                                                         \
static inline void __put_##name(struct spi_connection *sc, struct spi_data *data) \
{                                                                                 \
	data->next = sc->list_##name;                                                 \
	sc->list_##name = data;                                                       \
}                                                                                 \
                                                                                  \
static inline void put_##name(struct spi_connection *sc, struct spi_data *data)   \
{                                                                                 \
	lock(sc);                                                                     \
	__put_##name(sc, data);                                                       \
	unlock(sc);                                                                   \
}                                                                                 \

make_helper(free);
make_helper(pending);


static inline struct spi_data *get_active_pending(struct spi_connection *sc)
{
	struct spi_data *data;
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

static inline void put_active_pending(struct spi_connection *sc, struct spi_data *data)
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

static inline struct spi_connection *spi_get_connection(const comm_channel_t *channel)
{
    struct spi_connection *sc = (struct spi_connection *)channel->data;

    if (!sc) {
        fprintf( stderr, "ERROR in %s %d: SPI channel not correctly initialized\n",
            __FILE__, __LINE__ );
    }

    return sc;
}

static int spi_transmit(comm_channel_t *channel, const char *tx_buf, int tx_items)
{
    struct spi_connection *sc = spi_get_connection( channel );
	int sent, ret;
    if (!sc || tx_items < COMM_OVERHEAD || tx_items > COMM_BUF_SIZE) {
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
    struct spi_connection *sc = spi_get_connection( channel );
	struct spi_data *data;
	int count;
	data = get_active_pending(sc);
	if (data) {
		count = min(len, data->data_ready - data->r_idx);
		memcpy(buf, data->rx_buf + data->r_idx, count);
		data->r_idx += count;
		put_active_pending(sc, data);
	} else {
		count = -1;
		printf("spi_receive: no pending data\n");
		errno = EAGAIN;
	}

	return count;
}

static void *spi_thread(void *arg)
{
	struct spi_connection *sc = (struct spi_connection *)arg;
	struct spi_data *data;
	int res;

	while (1) {
		lock(sc);
		data = __get_free(sc);
		while (data == NULL) {
			pthread_cond_wait(&sc->cond, &sc->lock);
			data = __get_free(sc);
		}
		unlock(sc);
		/* reading always blocks until one complete packet is ready
		 * rx_buf is filled with exactly one packet
		 */
		res = read(sc->fd, data->rx_buf, BUF_SIZE);
		if (res <= 0) {
			perror("read spi device");
			put_free(sc, data);
		} else {
			data->data_ready = res;
			data->r_idx = 0;
			//print_data(data);
			put_pending(sc, data);
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

struct spi_connection connection;

int spi_dev_channel_create( comm_channel_t *channel )
{
	int i;
    if( channel->data == NULL )
    {
        memset( &connection, 0, sizeof( connection ) );
		pthread_mutex_init(&connection.lock, NULL);
		pthread_cond_init(&connection.cond, NULL);
		for(i=0;i<N_BUF;++i) {
			put_free(&connection, &connection.data[i]);
		}
        channel->type     = CH_SPI_DEV;
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
