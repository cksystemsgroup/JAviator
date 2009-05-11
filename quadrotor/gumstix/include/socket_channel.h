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

#ifndef SOCKET_CHANNEL_H
#define SOCKET_CHANNEL_H

#include "comm_channel.h"


/* Emums for indicating a specific socket type */
typedef enum
{
    SOCK_SERVER,
    SOCK_CLIENT,

} socket_type_t;


int socket_channel_init( comm_channel_t *channel, socket_type_t type, char *addr, int port );

int socket_channel_create( comm_channel_t *channel );

int socket_channel_destroy( comm_channel_t *channel );


#endif /* !SOCKET_CHANNEL_H */

/* End of file */
