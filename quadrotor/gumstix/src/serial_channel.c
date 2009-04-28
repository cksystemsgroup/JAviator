/* $Id: serial_channel.c,v 1.1 2008/10/16 14:41:13 rtrummer Exp $ */

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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "serial_channel.h"

#define _POSIX_SOURCE 1
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>

struct serial_connection
{
    int    fd;
    int    baudrate;
    char   device[SERIAL_MAX_NAME];
    struct termios term;
    struct termios oldterm;
};

static struct serial_connection connection;


static inline struct serial_connection *serial_get_connection(const struct channel *channel)
{
    struct serial_connection *sc = (struct serial_connection *) channel->data;
    if (!sc) {
        fprintf(stderr, "ERROR in %s %d: channel not correctly initialized\n",
            __FILE__, __LINE__);
    }
    return sc;
}

static int serial_transmit(struct channel *channel, const char *buf, int len)
{
    int res;
    struct serial_connection *sc = serial_get_connection(channel);
    if(!sc)
        return -1;

    do{
        res = write(sc->fd, buf, len);
        if (res >= 0) {
            buf += res;
            len -= res;
        }

    } while (len || (res==-1 && errno == EAGAIN));

    if (res < 0) {
        perror("write to serial");
        return res;
    }
    return 0;
}

static int serial_receive(struct channel *channel, char *buf, int len)
{
    int res;
    struct serial_connection *sc = serial_get_connection(channel);
    if(!sc)
        return -1;

    res = read(sc->fd, buf, len);
    if (!res) {
        errno = EAGAIN;
        res = -1;
    }
    return res;
}

static int serial_start(struct channel *channel)
{
    return 0;
}

static int serial_flush(struct channel *channel)
{
    return 0;
}

static int serial_poll(struct channel *channel)
{
    fd_set readfs;
    int maxfd;
    struct timeval timeout;
    struct serial_connection *sc = serial_get_connection(channel);
    if(!sc)
        return -1;

    timeout.tv_usec = 100;
    timeout.tv_sec = 0;
    FD_SET(sc->fd, &readfs);
    maxfd = sc->fd + 1;

    return select(maxfd, &readfs, NULL, NULL, &timeout);
}

static int init_termios(struct serial_connection *sc)
{
    int _brate;
    int res;

    switch (sc->baudrate) {
        case 38400:
            _brate = B38400;
            break;
        case 57600:
            _brate = B57600;
            break;
        case 115200:
            _brate = B115200;
            break;
        default:
            fprintf(stderr, "ERROR in %s %d: unknown baudrate\n", __FILE__, __LINE__);
            return -1;
    }

    sc->term.c_cflag = _brate |  CS8 | CLOCAL | CREAD;
    sc->term.c_iflag = IGNPAR;
    sc->term.c_oflag = 0;
    sc->term.c_lflag = 0;
        sc->term.c_cc[VTIME]    = 0;
        sc->term.c_cc[VMIN]     = 0;     /* non-blocking reads */

    res = tcflush(sc->fd, TCIFLUSH);
    if(res)
        perror("tcflush");
    res = tcsetattr(sc->fd, TCSANOW, &sc->term);
    if(res)
        perror("tcsetattr");
    return 0;
}

int serial_channel_init(struct channel *channel, char *interface, int baudrate)
{
    int fd;
    struct serial_connection *sc = serial_get_connection(channel);
    if(!sc)
        return -1;

    printf("open device %s ... ", interface);
    fd = open(interface, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror( "open device" );
        return -1;
    }
    printf("success\n");

    sc->fd = fd;
    sc->baudrate = baudrate;
    strncpy(sc->device, interface, SERIAL_MAX_NAME);
    tcgetattr(fd, &sc->oldterm);
    return init_termios(sc);
}


int serial_channel_create( struct channel *channel )
{
    if( channel->data == NULL )
    {
        memset( &connection, 0, sizeof( connection ) );
        channel->type     = CH_SERIAL;
        channel->transmit = serial_transmit;
        channel->receive  = serial_receive;
        channel->start    = serial_start;
        channel->flush    = serial_flush;
        channel->poll     = serial_poll;
        channel->data     = &connection;
        return( 0 );
    }

    fprintf( stderr, "WARNING: serial channel already initialized\n" );
    return( -1 );
}

int serial_channel_destroy(struct channel *channel)
{
    struct serial_connection *sc = serial_get_connection(channel);
    if(!sc)
        return -1;

    tcsetattr(sc->fd, TCSANOW, &sc->oldterm);
    close(sc->fd);
    channel->data = NULL;
    return 0;
}
