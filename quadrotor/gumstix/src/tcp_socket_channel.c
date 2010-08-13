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
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
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
    struct sockaddr_in clientinfo;

} tcp_connection_t;


static inline tcp_connection_t *tcp_get_connection( const comm_channel_t *channel )
{
    tcp_connection_t *tc = (tcp_connection_t *) channel->data;

    if( !tc )
    {
        fprintf( stderr, "ERROR in %s %d: socket channel not correctly initialized\n",
            __FILE__, __LINE__ );
    }

    return( tc );
}

static inline int socket_connection_accept( tcp_connection_t *tc )
{
    struct sockaddr_in clientinfo;
    unsigned int size = sizeof( clientinfo );
    int saved_fd, flag = 1;

    tc->connected = 0;
    tc->fd = accept( tc->accept_fd, (struct sockaddr *) &clientinfo, &size );

    if( tc->fd > 0 )
    {
        saved_fd = fcntl( tc->fd, F_GETFL );
        fcntl( tc->fd, F_SETFL, saved_fd );

        if( setsockopt( tc->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof( flag ) ) )
        {
            fprintf( stderr, "ERROR in %s %d: setsockopt accept\n",
                __FILE__, __LINE__ );
            return( -1 );
        }

        tc->connected = 1;
        return( 0 );
    }

    if( tc->fd == -1 && errno != EAGAIN )
    {
        fprintf( stderr, "ERROR in %s %d: socket connection accept\n",
            __FILE__, __LINE__ );
    }

    return( -1 );
}

static inline void close_connection( tcp_connection_t *tc )
{
    close( tc->fd );
    tc->fd = -1;
    tc->connected = 0;
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
    tcp_connection_t *tc = tcp_get_connection( channel );
    int res;

    if( !tc )
    {
        return( -1 );
    }

    if( !tc->connected )
    {
		errno = EAGAIN;
		return( -1 );
    }

    if( tc->connected )
    {
        res = socket_write( tc->fd, buf, len );

        if( res == -1 )
        {
            close_connection( tc );
        }
    }

    return( res );
}

static int server_socket_receive( comm_channel_t *channel, char *buf, int len )
{
    tcp_connection_t *tc = tcp_get_connection( channel );
    int res;

    if( !tc )
    {
        return( -1 );
    }

    if( !tc->connected )
    {
        res = socket_connection_accept( tc );
    }

    if( tc->connected )
    {
        res = socket_read( tc->fd, buf, len );

        if( res == -1 )
        {
            close_connection( tc );
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
    tcp_connection_t *tc = tcp_get_connection( channel );
    struct timeval tv;
    fd_set readfs;
    int maxfd;

    if( !tc )
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

    if( !tc->connected )
    {
        FD_SET( tc->accept_fd, &readfs );
        maxfd = tc->accept_fd + 1;
    }
    else
    {
        FD_SET( tc->fd, &readfs );
        maxfd = tc->fd + 1;
    }

    return select( maxfd, &readfs, NULL, NULL, timeout ? &tv : NULL );
}

static int client_socket_connect( tcp_connection_t *tc )
{
    struct linger linger;
    int res, saved_fd, flag = 1;

    if( tc->fd == -1 )
    {
        tc->fd = socket( AF_INET, SOCK_STREAM, 0 );
    }

    res = connect( tc->fd, (struct sockaddr *) &tc->clientinfo, sizeof( tc->clientinfo ) );

    if( res == -1 )
    {
        fprintf( stderr, "ERROR in %s %d: client socket connect\n",
            __FILE__, __LINE__ );
    }
    else
    {
        saved_fd = fcntl( tc->fd, F_GETFL ) | O_NONBLOCK;
        fcntl( tc->fd, F_SETFL, saved_fd );
        linger.l_onoff = 0;

        if( setsockopt( tc->fd, SOL_SOCKET, SO_LINGER, &linger, sizeof( linger ) ) )
        {
            fprintf( stderr, "ERROR in %s %d: setsockopt linger\n",
                __FILE__, __LINE__ );
        }

        if( setsockopt( tc->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof( flag ) ) )
        {
            fprintf( stderr, "ERROR in %s %d: setsockopt nagle\n",
                __FILE__, __LINE__ );
        }

        tc->connected = 1;
    }

    return( res );
}

static int client_socket_init( tcp_connection_t *tc, char *addr, int port )
{
    struct sockaddr_in *clientinfo = &tc->clientinfo;
    uint32_t caddr;

    tc->connected = 0;
    tc->fd = -1;

    if( inet_pton( AF_INET, addr, &caddr ) <= 0 )
    {
        fprintf( stderr, "ERROR in %s %d: inet_pton\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    clientinfo->sin_family      = AF_INET;
    clientinfo->sin_addr.s_addr = caddr;
    clientinfo->sin_port        = htons( (short) port );

    client_socket_connect( tc );
    return( 0 );
}

static int server_socket_init( tcp_connection_t *tc, int port )
{
    struct sockaddr_in serverinfo;
    struct linger linger;
    int saved_fd;

    tc->connected  = 0;
    tc->accept_fd  = socket( AF_INET, SOCK_STREAM, 0 );
    linger.l_onoff = 0;

    if( setsockopt( tc->accept_fd, SOL_SOCKET, SO_LINGER, &linger, sizeof( linger ) ) )
    {
        fprintf( stderr, "ERROR in %s %d: setsockopt linger\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    serverinfo.sin_family      = AF_INET;
    serverinfo.sin_addr.s_addr = INADDR_ANY;
    serverinfo.sin_port        = htons( (short) port );

    if( bind( tc->accept_fd, (struct sockaddr *) &serverinfo, sizeof( serverinfo ) ) < 0 )
    {
        fprintf( stderr, "ERROR in %s %d: server bind\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    saved_fd = fcntl( tc->accept_fd, F_GETFL );
    fcntl( tc->accept_fd, F_SETFL, saved_fd );

    if( listen( tc->accept_fd, 3 ) < 0 )
    {
        fprintf( stderr, "ERROR in %s %d: server listen\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    return( 0 );
}

static int client_socket_transmit( comm_channel_t *channel, const char *buf, int len )
{
    tcp_connection_t *tc = tcp_get_connection( channel );
    int res;

    if( !tc )
    {
        return( -1 );
    }

    if( !tc->connected )
    {
        res = client_socket_connect( tc );
    }

    if( tc->connected )
    {
        res = socket_write( tc->fd, buf, len );

        if( res == -1 )
        {
            close_connection( tc );
        }
    }

    return( res );
}

static int client_socket_receive( comm_channel_t *channel, char *buf, int len )
{
    tcp_connection_t *tc = tcp_get_connection( channel );
    int res;

    if( !tc )
    {
        return( -1 );
    }

    if( !tc->connected )
    {
        res = client_socket_connect( tc );
    }

    if( tc->connected )
    {
        res = socket_read( tc->fd, buf, len );

        if( res == -1 )
        {
            close_connection( tc );
        }
    }

    return( res );
}

int tcp_socket_channel_create( comm_channel_t *channel )
{
    tcp_connection_t *tc;

    if( channel->data != NULL )
    {
        fprintf( stderr, "WARNING: TCP channel already initialized\n" );
        return( -1 );
    }

    tc = malloc( sizeof( tcp_connection_t ) );

    if( !tc )
    {
        fprintf( stderr, "ERROR in %s %d: memory allocation\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    memset( tc, 0, sizeof( tcp_connection_t ) );

    channel->type     = CH_SOCKET;
    channel->transmit = server_socket_transmit;
    channel->receive  = server_socket_receive;
    channel->poll     = server_socket_poll;
    channel->start    = socket_start;
    channel->flush    = socket_flush;
    channel->data     = tc;
    return( 0 );
}

int tcp_socket_channel_init( comm_channel_t *channel, socket_type_t type, char *addr, int port )
{
    tcp_connection_t *tc = tcp_get_connection( channel );

    if( !tc )
    {
        return( -1 );
    }

    switch( type )
    {
        case SOCK_SERVER:
            return server_socket_init( tc, port );

        case SOCK_CLIENT:
            channel->transmit = client_socket_transmit;
            channel->receive  = client_socket_receive;
            return client_socket_init( tc, addr, port );

		default:
            fprintf( stderr, "ERROR in %s %d: unknown socket type\n",
                __FILE__, __LINE__ );
    }

    return( -1 );
}

int tcp_socket_channel_destroy( comm_channel_t *channel )
{
    tcp_connection_t *tc = tcp_get_connection( channel );

    if( !tc )
    {
        return( -1 );
    }

    close( tc->accept_fd );
    close( tc->fd );
    free( tc );
    channel->data = NULL;
    return( 0 );
}

/* End of file */
