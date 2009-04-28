/* $Id: channel.h,v 1.1 2008/10/16 14:41:13 rtrummer Exp $ */

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

#ifndef CHANNEL_H
#define CHANNEL_H

struct channel;

typedef int (*transmit)( struct channel *channel, const char *buf, int len );
typedef int (*receive)( struct channel *channel, char *buf, int len );
typedef int (*start)( struct channel *channel );
typedef int (*flush)( struct channel *channel );
typedef int (*poll)( struct channel *channel );

typedef enum
{
    CH_SPI = 1,
    CH_SERIAL,
    CH_SOCKET,

} channel_type;

struct channel
{
    channel_type    type;
    transmit        transmit;
    receive         receive;
    start           start;
    flush           flush;
    poll            poll;
    void            *data;
};

int controller_channel_create( struct channel *channel, channel_type type );
int controller_channel_destroy( struct channel *channel );

#endif // !CHANNEL_H

// End of file.
