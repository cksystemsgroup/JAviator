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

#define _POSIX_SOURCE 1

#include <sys/types.h>
#include <sys/select.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "serial_channel.h"

#define SERIAL_MAX_NAME     32

typedef struct
{
    int    fd;
    int    baudrate;
    char   device[ SERIAL_MAX_NAME ];
    struct termios term;
    struct termios oldterm;

} serial_connection_t;


static inline serial_connection_t *serial_get_connection( const comm_channel_t *channel )
{
    serial_connection_t *sc = (serial_connection_t *) channel->data;

    if( !sc )
    {
        fprintf( stderr, "ERROR in %s %d: serial channel not correctly initialized\n",
            __FILE__, __LINE__ );
    }

    return( sc );
}

static int serial_transmit( comm_channel_t *channel, const char *buf, int len )
{
    serial_connection_t *sc = serial_get_connection( channel );
    int res;

    if( !sc )
    {
        return( -1 );
    }

    do
    {
        res = write( sc->fd, buf, len );

        if ( res >= 0 )
        {
            buf += res;
            len -= res;
        }
    }
    while( len || (res == -1 && errno == EAGAIN) );

    if( res < 0 )
    {
        fprintf( stderr, "ERROR in %s %d: write to serial port\n",
            __FILE__, __LINE__ );
        return( res );
    }

    return( 0 );
}

static int serial_receive( comm_channel_t *channel, char *buf, int len )
{
    serial_connection_t *sc = serial_get_connection( channel );
    int res;

    if( !sc )
    {
        return( -1 );
    }

    res = read( sc->fd, buf, len );

    if( !res )
    {
        errno = EAGAIN;
        res = -1;
    }

    return( res );
}

static int serial_start( comm_channel_t *channel )
{
    return( 0 );
}

static int serial_flush( comm_channel_t *channel )
{
    serial_connection_t *sc = serial_get_connection( channel );
    int res;

    if( !sc )
    {
        return( -1 );
    }

	res = tcflush( sc->fd, TCIOFLUSH );

	if( res )
    {
		perror( "tcflush" );
    }

    return( res );
}

static int serial_poll( comm_channel_t *channel, long timeout )
{
    serial_connection_t *sc = serial_get_connection( channel );
    struct timeval tv;
    fd_set readfs;
    int maxfd;

    if( !sc )
    {
        return( -1 );
    }

	if( timeout > 0 )
    {
	    tv.tv_usec = (timeout * 1000) % 1000000;
    	tv.tv_sec  =  timeout / 1000;
	}
    else
    if( timeout < 0 )
    {
	    tv.tv_usec = 0;
    	tv.tv_sec  = 0;
	}

    FD_SET( sc->fd, &readfs );
    maxfd = sc->fd + 1;

    return select( maxfd, &readfs, NULL, NULL, timeout ? &tv : NULL );
}

static int init_termios( serial_connection_t *sc )
{
    int baudrate, res;

    switch( sc->baudrate )
    {
        case 38400:
            baudrate = B38400;
            break;

        case 57600:
            baudrate = B57600;
            break;

        case 115200:
            baudrate = B115200;
            break;

        default:
            fprintf( stderr, "ERROR in %s %d: invalid baudrate %d\n",
                __FILE__, __LINE__, sc->baudrate );
            return( -1 );
    }

    sc->term.c_cflag     = baudrate | CS8 | CLOCAL | CREAD;
    sc->term.c_iflag     = IGNPAR;
    sc->term.c_oflag     = 0;
    sc->term.c_lflag     = 0;
    sc->term.c_cc[VTIME] = 0;
    sc->term.c_cc[VMIN]  = 0;

    res = tcflush( sc->fd, TCIFLUSH );

    if( res )
    {
        fprintf( stderr, "ERROR in %s %d: tcflush\n",
            __FILE__, __LINE__ );
    }

    res = tcsetattr( sc->fd, TCSANOW, &sc->term );

    if( res )
    {
        fprintf( stderr, "ERROR in %s %d: tcsetattr\n",
            __FILE__, __LINE__ );
    }

    return( 0 );
}

int serial_channel_create( comm_channel_t *channel )
{
    serial_connection_t *sc;

    if( channel->data != NULL )
    {
        fprintf( stderr, "WARNING: serial channel already initialized\n" );
        return( -1 );
    }

    sc = malloc( sizeof( serial_connection_t ) );

    if( !sc )
    {
        fprintf( stderr, "ERROR in %s %d: memory allocation\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    memset( sc, 0, sizeof( serial_connection_t ) );

    channel->type     = CH_SERIAL;
    channel->transmit = serial_transmit;
    channel->receive  = serial_receive;
    channel->start    = serial_start;
    channel->flush    = serial_flush;
    channel->poll     = serial_poll;
    channel->data     = sc;
    return( 0 );
}

int serial_channel_init( comm_channel_t *channel, char *interface, int baudrate )
{
    serial_connection_t *sc = serial_get_connection( channel );

    if( !sc )
    {
        return( -1 );
    }

    sc->fd = open( interface, O_RDWR | O_NOCTTY );

    if( sc->fd < 0 )
    {
        fprintf( stderr, "ERROR in %s %d: open device %s\n",
            __FILE__, __LINE__, interface );
        return( -1 );
    }

    sc->baudrate = baudrate;
    strncpy( sc->device, interface, SERIAL_MAX_NAME );
    tcgetattr( sc->fd, &sc->oldterm );
    return init_termios( sc );
}

int serial_channel_destroy( comm_channel_t *channel )
{
    serial_connection_t *sc = serial_get_connection( channel );

    if( !sc )
    {
        return( -1 );
    }

    tcsetattr( sc->fd, TCSANOW, &sc->oldterm );
    close( sc->fd );
    free( sc );
    channel->data = NULL;
    return( 0 );
}

/* End of file */
