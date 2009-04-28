/* $Id: unix_socket_channel.c,v 1.2 2008/11/10 12:17:57 hroeck Exp $ */

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
#include "socket_channel.h"

#define _POSIX_SOURCE 1

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/select.h>

struct socket_connection {
    int    fd;
    int    port;
    int    accept_fd;
    int    connected;
    struct sockaddr_in serverinfo;
    struct sockaddr_in clientinfo;
};

struct socket_connection _connection;


static int socket_connection_accept(struct socket_connection *sc)
{
    int client_socket;
    unsigned int len;
    struct sockaddr_in clientinfo;
    long save_fd;
    int flag = 1;

    sc->connected = 0;

    len = sizeof(clientinfo);
    client_socket = accept(sc->accept_fd, (struct sockaddr *)&clientinfo, &len);

    if(client_socket > 0) {
        save_fd = fcntl(client_socket, F_GETFL);
        save_fd |= O_NONBLOCK;
        fcntl(client_socket, F_SETFL, save_fd);
        if(setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)))
            perror("setsockopt accept");
        sc->fd = client_socket;
        sc->connected = 1;
        return 0;
    } else if (client_socket == -1 && errno != EAGAIN)
        perror("accept");

    return -1;
}

static inline struct socket_connection *socket_get_connection(const struct channel *channel)
{
    struct socket_connection *sc = (struct socket_connection *)channel->data;
    if (!sc) {
        fprintf(stderr, "ERROR in %s %d: channel not correctly initialized\n",
                __FILE__, __LINE__);
    }
    return sc;
}

static inline void close_connection(struct socket_connection *sc)
{
    close(sc->fd);
    sc->fd = -1;
    sc->connected = 0;
}

static inline int socket_write(int fd, const char *buf, int size)
{
    int res;
    do{
        res = write(fd, buf, size);
        if (res >= 0) {
            buf += res;
            size -= res;
        } else if (res == -1 && errno != EAGAIN) {
            perror("socket write");
            break;
        } else if (res == 0) {
            printf("transmit socket closed\n");
            res = -1;
            break;
        }
    } while (size || (res==-1 && errno == EAGAIN));
    if (size == 0)
        res = 0;
    return res;
}

static int server_socket_transmit(struct channel *channel, const char *buf, int len)
{
    int res;
    struct socket_connection *sc = socket_get_connection(channel);
    if (!sc)
        return -1;

    if(!sc->connected) {
        res = socket_connection_accept(sc);
    }

    if(sc->connected) {
        res = socket_write(sc->fd, buf, len);
        if (res == -1)
            close_connection(sc);
    }
    return res;
}

static inline int socket_read(int fd, char *buf, int size)
{
    int res;
    res = read(fd, buf, size);
    if (res == -1 && errno == EAGAIN) {
        res = 0;
    } else if (res == -1) {
        perror("socket read");
    } else if (res == 0) {
        printf("receive socket closed\n");
        res = -1;
    }
    return res;
}

static int server_socket_receive(struct channel *channel, char *buf, int len)
{
    int res;
    struct socket_connection *sc = socket_get_connection(channel);
    if (!sc)
        return -1;

    if(!sc->connected) {
        res = socket_connection_accept(sc);
    }
    if(sc->connected) {
        res = socket_read(sc->fd, buf, len);
        if (res == -1)
            close_connection(sc);
    }
    return res;
}

static int socket_start(struct channel *channel)
{
    return 0;
}

static int socket_flush(struct channel *channel)
{
    return 0;
}

static int server_socket_poll(struct channel *channel)
{
    fd_set readfs;
    int maxfd;
    struct timeval timeout;
    struct socket_connection *sc = socket_get_connection(channel);
    if (!sc)
        return -1;

    timeout.tv_usec = 100;
    timeout.tv_sec = 0;
    if(!sc->connected){
        FD_SET(sc->accept_fd, &readfs);
        maxfd = sc->accept_fd + 1;
    } else {
        FD_SET(sc->fd, &readfs);
        maxfd = sc->fd + 1;
    }

    printf("do select\n");
    return select(maxfd, &readfs, NULL, NULL, &timeout);
}

static inline int client_socket_connect(struct socket_connection *sc)
{
    int res;
    if (sc->fd == -1) {
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);

        sc->fd = client_socket;
    }

    res = connect(sc->fd, (struct sockaddr *)&sc->clientinfo, sizeof(sc->clientinfo));
    if (res == -1) {
#if DEBUG > 1
        perror("connect");
#endif
    } else {
        int save_fd, flag = 1;
        struct linger _linger;
        save_fd = fcntl(sc->fd, F_GETFL);
        save_fd |= O_NONBLOCK;
        fcntl(sc->fd, F_SETFL, save_fd);
        _linger.l_onoff = 0;
        if (setsockopt(sc->fd, SOL_SOCKET, SO_LINGER, &_linger, sizeof(_linger)))
            perror("setsockopt linger");
        if(setsockopt(sc->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)))
            perror("setsockopt nagle");

        sc->connected = 1;
    }

    return res;
}

static int client_socket_init(struct socket_connection *sc, char *addr, int port)
{
    uint32_t caddr;
    struct sockaddr_in *clientinfo = &sc->clientinfo;

    sc->connected = 0;
    sc->fd = -1;

    printf("open port %d ... ", sc->port);

    if (inet_pton(AF_INET, addr, &caddr) <= 0) {
        perror("inet_pton");
        return -1;
    }
    clientinfo->sin_family = AF_INET;
    clientinfo->sin_addr.s_addr = caddr;
    clientinfo->sin_port = htons((short)port);

    client_socket_connect(sc);
    printf("success\n");
    return 0;
}

static int server_socket_init(struct socket_connection *sc, int port)
{
    int server_socket;
    unsigned int len;
    struct sockaddr_in serverinfo;
    long save_fd;
    struct linger _linger;

    sc->connected = 0;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    _linger.l_onoff = 0;

    printf("open port %d ... ", sc->port);

    if (setsockopt(server_socket, SOL_SOCKET, SO_LINGER, &_linger, sizeof(_linger)))
        perror("setsockopt");

    serverinfo.sin_family = AF_INET;
    serverinfo.sin_addr.s_addr = INADDR_ANY;
    serverinfo.sin_port = htons((short)port);

    len = sizeof(serverinfo);

    if (bind(server_socket, (struct sockaddr *)&serverinfo, len) < 0) {
        perror("server: bind");
            return -1;
    }

    save_fd = fcntl(server_socket, F_GETFL);
    save_fd |= O_NONBLOCK;
    fcntl(server_socket, F_SETFL, save_fd);

    if(listen(server_socket, 3) < 0){
        perror("server: listen");
            return -1;
    }

    sc->accept_fd = server_socket;
    printf("success\n");
    return 0;
}

static int client_socket_transmit(struct channel *channel, const char *buf, int len)
{
    int res;
    struct socket_connection *sc;
       
    sc = socket_get_connection(channel);
    if (!sc)
        return -1;

    if (!sc->connected)
        res = client_socket_connect(sc);
    if (sc->connected) {
        res = socket_write(sc->fd, buf, len);
        if (res == -1)
            close_connection(sc);
    }
    return res;
}

static int client_socket_receive(struct channel *channel, char *buf, int len)
{
    int res = -1;
    struct socket_connection *sc;


    sc = socket_get_connection(channel);
    if (!sc)
    goto out;

    if (!sc->connected)
    client_socket_connect(sc);

    if (sc->connected) {
    res = socket_read(sc->fd, buf, len);
    if (res == -1)
        close_connection(sc);
    }
out:
    return res;
}

static inline int client_channel_set(struct channel *channel)
{
    channel->transmit = client_socket_transmit;
    channel->receive = client_socket_receive;
    return 0;
}

int socket_channel_init(struct channel *channel, enum socket_type type, char *addr, int port)
{
    struct socket_connection *sc = socket_get_connection(channel);
    if (!sc)
        return -1;

    switch (type)
    {
        case SOCK_SERVER:
            return server_socket_init(sc, port);
            break;
        case SOCK_CLIENT:
            client_channel_set(channel);
            return client_socket_init(sc, addr, port);
            break;
    }
    return -1;
}

int socket_channel_create( struct channel *channel )
{
    if( channel->data == NULL )
    {
        channel->type     = CH_SOCKET;
        channel->transmit = server_socket_transmit;
        channel->receive  = server_socket_receive;
        channel->poll     = server_socket_poll;
        channel->start    = socket_start;
        channel->flush    = socket_flush;
        channel->data     = &_connection;
        return( 0 );
    }

    fprintf( stderr, "WARNING: socket channel already initialized\n" );
    return( -1 );
}


int socket_channel_destroy(struct channel *channel)
{
    struct socket_connection *sc = socket_get_connection(channel);
    if (!sc)
        return -1;

    close(sc->accept_fd);
    close(sc->fd);
    channel->data = NULL;
    return 0;
}
