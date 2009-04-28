/* $Id: socket_channel.h,v 1.1 2008/10/16 14:41:13 rtrummer Exp $ */

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

#ifndef SOCKET_CHANNEL_H
#define SOCKET_CHANNEL_H

#include "channel.h"

enum socket_type { SOCK_SERVER, SOCK_CLIENT };

/* initialize the channel. e.g., create and bind the accept socket */
int socket_channel_init(struct channel *channel, enum socket_type type, char *addr, int port);

/* fill channel structure */
int socket_channel_create(struct channel *channel);
/* deallocate private channel structure */
int socket_channel_destroy(struct channel *channel);
#endif /* SOCKET_CHANNEL_H */
