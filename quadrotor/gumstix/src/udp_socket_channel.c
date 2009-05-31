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

#define MAX_BUF 64
struct udp_connection
{
	int fd;
	int connected;
	uint8_t buf[MAX_BUF];
	int len;
	int idx;
};

static struct udp_connection connection;

static inline struct udp_connection *udp_get_connection(const comm_channel_t *
														 channel)
{
	struct udp_connection *uc = (struct udp_connection *) channel->data;

	if (!uc) {
		fprintf(stderr,
				"ERROR in %s %d: socket channel not correctly initialized\n",
				__FILE__, __LINE__);
	}

	return uc;
}

static inline void close_connection(struct udp_connection * uc)
{
	/*
	 * do not close fds 
	 */
	uc->connected = 0;
}

static inline int udp_write(int fd, const uint8_t *buf, int len)
{
	int res;

	do {
		res = write(fd, buf, len);

		if (res >= 0) {
			if (res != len)
				printf("could not send complete packet\n");
			buf += res;
			len -= res;
		} else if (res == -1 && errno != EAGAIN) {
			fprintf(stderr, "ERROR in %s %d: udp write\n", __FILE__, __LINE__);
			break;
		} else if (res == 0) {
			perror("udp write");
			fprintf(stderr, "ERROR in %s %d: transmit socket closed\n",
					__FILE__, __LINE__);
			res = -1;
			break;
		}
	} while (len || (res == -1 && errno == EAGAIN));

	if (len == 0) {
		res = 0;
	}

	return res;
}

static inline int udp_read(int fd, uint8_t *buf, int len)
{
	int res = read(fd, buf, len);

	if (res == -1 && errno == EAGAIN) {
		perror("udp read");
		res = 0;
	} else if (res == -1) {
		perror("udp read");
		fprintf(stderr, "ERROR in %s %d: udp read\n", __FILE__, __LINE__);
	} else if (res == 0) {
		fprintf(stderr, "ERROR in %s %d: receive socket closed\n",
				__FILE__, __LINE__);
		res = -1;
	}

	return res;
}

static int udp_socket_transmit(comm_channel_t * channel, const char *buf, int len)
{
	struct udp_connection *uc = udp_get_connection(channel);
	int res;

	if (!uc) {
		return -1;
	}

	if (!uc->connected) {
		errno = EAGAIN;
		return -1;
	}

	if (uc->connected) {
		res = udp_write(uc->fd, (uint8_t *) buf, len);

		if (res == -1) {
			close_connection(uc);
		}
	}

	return res;
}

static int udp_connection_connect(struct udp_connection *uc, uint8_t *buf, int len)
{
	int res;
	size_t addrlen;
	struct sockaddr_in clientinfo;

	addrlen = sizeof(clientinfo);
	/* receive first message from any source */
	res = recvfrom(uc->fd, buf, len, 0, (struct sockaddr *)&clientinfo, &addrlen);

	if (res > 0) {
		/* now connect to peer */
		if (connect(uc->fd, (struct sockaddr *)&clientinfo, addrlen)) {
			perror("udp recv connect");
			return -1;
		}
		printf("connected to %s %d \n", inet_ntoa(clientinfo.sin_addr), ntohs(clientinfo.sin_port) );
		uc->connected = 1;
	}
	return res;
}

#define min(a,b) (a)<(b)?(a):(b)
static int udp_socket_receive(comm_channel_t * channel, char *buf, int len)
{
	struct udp_connection *uc = udp_get_connection(channel);
	int res;

	if (!uc) {
		return -1;
	}

	if (!uc->connected) {
		res = udp_connection_connect(uc, (uint8_t *) buf, len);
	} else {
		int count;
		if (uc->idx == uc->len) {
			res = udp_read(uc->fd, uc->buf, MAX_BUF);
			if (res == -1) {
				perror("udp_read");
				close_connection(uc);
				return res;
			}
			uc->len = res;
			uc->idx = 0;
		}
		count = min(len, uc->len - uc->idx);
		memcpy(buf, uc->buf + uc->idx, count);
		uc->idx += count;
		return count;
	}

	return res;
}

static int udp_start(comm_channel_t * channel)
{
	return 0;
}

static int udp_flush(comm_channel_t * channel)
{
	return 0;
}

static int udp_socket_poll(comm_channel_t * channel, long timeout)
{
	struct udp_connection *uc = udp_get_connection(channel);
	struct timeval tv;
	fd_set readfs;
	int maxfd;

	if (!uc) {
		return -1;
	}

	if (timeout > 0) {
		tv.tv_usec = (timeout * 1000) % 1000000;
		tv.tv_sec = timeout / 1000;
	} else if (timeout < 0) {
		tv.tv_usec = 0;
		tv.tv_sec = 0;
	}
	FD_SET(uc->fd, &readfs);
	maxfd = uc->fd + 1;

	return select(maxfd, &readfs, NULL, NULL, timeout ? &tv : NULL);
}

static int udp_socket_init(struct udp_connection * uc, int port)
{
	struct sockaddr_in serverinfo;

	uc->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (uc->fd < 0) {
		perror("udp recv socket");
		return -1;
	}

	serverinfo.sin_family = AF_INET;
	serverinfo.sin_addr.s_addr = INADDR_ANY;
	serverinfo.sin_port = htons((short)port);
	if (bind(uc->fd, (struct sockaddr *)&serverinfo, sizeof(serverinfo)) < 0) {
		fprintf(stderr, "ERROR in %s %d: server bind\n", __FILE__, __LINE__);
		close(uc->fd);
		return -1;
	}

	return 0;
}

int udp_socket_channel_init(comm_channel_t * channel, socket_type_t type,
							char *addr, int port)
{
	struct udp_connection *uc = udp_get_connection(channel);

	if (!uc) {
		return -1;
	}

	return udp_socket_init(uc, port);
}

int udp_socket_channel_create(comm_channel_t * channel)
{
	if (channel->data == NULL) {
		channel->type = CH_SOCKET;
		channel->transmit = udp_socket_transmit;
		channel->receive = udp_socket_receive;
		channel->poll = udp_socket_poll;
		channel->start = udp_start;
		channel->flush = udp_flush;
		channel->data = &connection;
		return 0;
	}

	fprintf(stderr, "WARNING: udp channel already initialized\n");
	return -1;
}

int udp_socket_channel_destroy(comm_channel_t * channel)
{
	struct udp_connection *uc = udp_get_connection(channel);

	if (!uc) {
		return -1;
	}

	close(uc->fd);
	channel->data = NULL;
	return 0;
}

/* End of file */
