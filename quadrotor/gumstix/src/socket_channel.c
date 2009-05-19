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

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "socket_channel.h"

typedef struct
{
    int    fd;
    int    port;
    int    accept_fd;
    int    connected;
    struct sockaddr_in serverinfo;
    struct sockaddr_in clientinfo;

} socket_connection_t;

static socket_connection_t connection;


static inline socket_connection_t *socket_get_connection( const comm_channel_t *channel )
{
    socket_connection_t *sc = (socket_connection_t *) channel->data;

    if( !sc )
    {
        fprintf( stderr, "ERROR in %s %d: socket channel not correctly initialized\n",
            __FILE__, __LINE__ );
    }

    return( sc );
}

static inline int socket_connection_accept( socket_connection_t *sc )
{
    struct sockaddr_in clientinfo;
    unsigned int size = sizeof( clientinfo );
    int saved_fd, flag = 1;

    sc->connected = 0;
    sc->fd = accept( sc->accept_fd, (struct sockaddr *) &clientinfo, &size );

    if( sc->fd > 0 )
    {
        saved_fd = fcntl( sc->fd, F_GETFL );// | O_NONBLOCK;
        fcntl( sc->fd, F_SETFL, saved_fd );

        if( setsockopt( sc->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof( flag ) ) )
        {
            fprintf( stderr, "ERROR in %s %d: setsockopt accept\n",
                __FILE__, __LINE__ );
            return( -1 );
        }

        sc->connected = 1;
        return( 0 );
    }

    if( sc->fd == -1 && errno != EAGAIN )
    {
        fprintf( stderr, "ERROR in %s %d: socket connection accept\n",
            __FILE__, __LINE__ );
    }

    return( -1 );
}

static inline void close_connection( socket_connection_t *sc )
{
    close( sc->fd );
    sc->fd = -1;
    sc->connected = 0;
}

static inline int socket_write( int fd, const char *buf, int len )
{
    int res;

    do
    {
        res = write( fd, buf, len );

        if( res >= 0 )
        {
            buf += res;
            len -= res;
        }
        else
        if( res == -1 && errno != EAGAIN )
        {
            fprintf( stderr, "ERROR in %s %d: socket write\n",
                __FILE__, __LINE__ );
            break;
        }
        else
        if( res == 0 )
        {
            fprintf( stderr, "ERROR in %s %d: transmit socket closed\n",
                __FILE__, __LINE__ );
            res = -1;
            break;
        }
    }
    while( len || (res == -1 && errno == EAGAIN) );

    if( len == 0 )
    {
        res = 0;
    }

    return( res );
}

static inline int socket_read( int fd, char *buf, int len )
{
    int res = read( fd, buf, len );

    if( res == -1 && errno == EAGAIN )
    {
        res = 0;
    }
    else
    if( res == -1 )
    {
        fprintf( stderr, "ERROR in %s %d: socket read\n",
            __FILE__, __LINE__ );
    }
    else
    if( res == 0 )
    {
        fprintf( stderr, "ERROR in %s %d: receive socket closed\n",
            __FILE__, __LINE__ );
        res = -1;
    }

    return( res );
}

static int server_socket_transmit( comm_channel_t *channel, const char *buf, int len )
{
    socket_connection_t *sc = socket_get_connection( channel );
    int res;

    if( !sc )
    {
        return( -1 );
    }

    if( !sc->connected )
    {
		errno = EAGAIN;
		return -1;
        //res = socket_connection_accept( sc );
    }

    if( sc->connected )
    {
        res = socket_write( sc->fd, buf, len );

        if( res == -1 )
        {
            close_connection( sc );
        }
    }

    return( res );
}

static int server_socket_receive( comm_channel_t *channel, char *buf, int len )
{
    socket_connection_t *sc = socket_get_connection( channel );
    int res;

    if( !sc )
    {
        return( -1 );
    }

    if( !sc->connected )
    {
        res = socket_connection_accept( sc );
    }

    if( sc->connected )
    {
        res = socket_read( sc->fd, buf, len );

        if( res == -1 )
        {
            close_connection( sc );
        }
    }

    return( res );
}

static int socket_start( comm_channel_t *channel )
{
    return( 0 );
}

static int socket_flush( comm_channel_t *channel )
{
    return( 0 );
}

static int server_socket_poll( comm_channel_t *channel, long timeout )
{
    socket_connection_t *sc = socket_get_connection( channel );
    struct timeval tv;
    fd_set readfs;
    int maxfd;

    if( !sc )
    {
        return( -1 );
    }

	if (timeout > 0) {
	    tv.tv_usec = (timeout * 1000) % 1000000;
    	tv.tv_sec  = timeout/1000;
	} else if (timeout < 0) {
	    tv.tv_usec = 0;
    	tv.tv_sec  = 0;
	}
    if( !sc->connected )
    {
        FD_SET( sc->accept_fd, &readfs );
        maxfd = sc->accept_fd + 1;
    }
    else
    {
        FD_SET( sc->fd, &readfs );
        maxfd = sc->fd + 1;
    }

    return select( maxfd, &readfs, NULL, NULL, timeout?&tv:NULL );
}

static int client_socket_connect( socket_connection_t *sc )
{
    struct linger linger;
    int res, saved_fd, flag = 1;

    if( sc->fd == -1 )
    {
        sc->fd = socket( AF_INET, SOCK_STREAM, 0 );
    }

    res = connect( sc->fd, (struct sockaddr *) &sc->clientinfo, sizeof( sc->clientinfo ) );

    if( res == -1 )
    {
        fprintf( stderr, "ERROR in %s %d: client socket connect\n",
            __FILE__, __LINE__ );
    }
    else
    {
        saved_fd = fcntl( sc->fd, F_GETFL ) | O_NONBLOCK;
        fcntl( sc->fd, F_SETFL, saved_fd );
        linger.l_onoff = 0;

        if( setsockopt( sc->fd, SOL_SOCKET, SO_LINGER, &flag, sizeof( flag ) ) )
        {
            fprintf( stderr, "ERROR in %s %d: setsockopt linger\n",
                __FILE__, __LINE__ );
        }

        if( setsockopt( sc->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof( flag ) ) )
        {
            fprintf( stderr, "ERROR in %s %d: setsockopt nagle\n",
                __FILE__, __LINE__ );
        }

        sc->connected = 1;
    }

    return( res );
}

static int client_socket_init( socket_connection_t *sc, char *addr, int port )
{
    struct sockaddr_in *clientinfo = &sc->clientinfo;
    uint32_t caddr;

    sc->connected = 0;
    sc->fd = -1;

    if( inet_pton( AF_INET, addr, &caddr ) <= 0 )
    {
        fprintf( stderr, "ERROR in %s %d: inet_pton\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    clientinfo->sin_family      = AF_INET;
    clientinfo->sin_addr.s_addr = caddr;
    clientinfo->sin_port        = htons( (short) port );

    client_socket_connect( sc );
    return( 0 );
}

static int server_socket_init( socket_connection_t *sc, int port )
{
    struct sockaddr_in serverinfo;
    struct linger linger;
    int saved_fd;

    sc->connected = 0;
    sc->accept_fd = socket( AF_INET, SOCK_STREAM, 0 );
    linger.l_onoff = 0;

    if( setsockopt( sc->accept_fd, SOL_SOCKET, SO_LINGER, &linger, sizeof( linger ) ) )
    {
        fprintf( stderr, "ERROR in %s %d: setsockopt linger\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    serverinfo.sin_family      = AF_INET;
    serverinfo.sin_addr.s_addr = INADDR_ANY;
    serverinfo.sin_port        = htons( (short) port );

    if( bind( sc->accept_fd, (struct sockaddr *) &serverinfo, sizeof( serverinfo ) ) < 0 )
    {
        fprintf( stderr, "ERROR in %s %d: server bind\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    saved_fd = fcntl( sc->accept_fd, F_GETFL );// | O_NONBLOCK;
    fcntl( sc->accept_fd, F_SETFL, saved_fd );

    if( listen( sc->accept_fd, 3 ) < 0 )
    {
        fprintf( stderr, "ERROR in %s %d: server listen\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    return( 0 );
}

static int client_socket_transmit( comm_channel_t *channel, const char *buf, int len )
{
    socket_connection_t *sc = socket_get_connection( channel );
    int res;

    if( !sc )
    {
        return( -1 );
    }

    if( !sc->connected )
    {
        res = client_socket_connect( sc );
    }

    if( sc->connected )
    {
        res = socket_write( sc->fd, buf, len );

        if( res == -1 )
        {
            close_connection( sc );
        }
    }

    return( res );
}

static int client_socket_receive( comm_channel_t *channel, char *buf, int len )
{
    socket_connection_t *sc = socket_get_connection( channel );
    int res;

    if( !sc )
    {
        return( -1 );
    }

    if( !sc->connected )
    {
        res = client_socket_connect( sc );
    }

    if( sc->connected )
    {
        res = socket_read( sc->fd, buf, len );

        if( res == -1 )
        {
            close_connection( sc );
        }
    }

    return( res );
}

int socket_channel_init( comm_channel_t *channel, socket_type_t type, char *addr, int port )
{
    socket_connection_t *sc = socket_get_connection( channel );

    if( !sc )
    {
        return( -1 );
    }

    switch( type )
    {
        case SOCK_SERVER:
            return server_socket_init( sc, port );

        case SOCK_CLIENT:
            channel->transmit = client_socket_transmit;
            channel->receive  = client_socket_receive;
            return client_socket_init( sc, addr, port );
    }

    return( -1 );
}

int socket_channel_create( comm_channel_t *channel )
{
    if( channel->data == NULL )
    {
        channel->type     = CH_SOCKET;
        channel->transmit = server_socket_transmit;
        channel->receive  = server_socket_receive;
        channel->poll     = server_socket_poll;
        channel->start    = socket_start;
        channel->flush    = socket_flush;
        channel->data     = &connection;
        return( 0 );
    }

    fprintf( stderr, "WARNING: socket channel already initialized\n" );
    return( -1 );
}


int socket_channel_destroy( comm_channel_t *channel )
{
    socket_connection_t *sc = socket_get_connection( channel );

    if( !sc )
    {
        return( -1 );
    }

    close( sc->accept_fd );
    close( sc->fd );
    channel->data = NULL;
    return( 0 );
}

/* End of file */
