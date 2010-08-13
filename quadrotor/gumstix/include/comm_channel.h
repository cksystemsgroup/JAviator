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

#ifndef COMM_CHANNEL_H
#define COMM_CHANNEL_H

/* Forward declaration of structure channel */
struct channel;

/* Type definition of communication channel */
typedef struct channel comm_channel_t;

/* Function pointers used by a communication channel */
typedef int (*transmit)( comm_channel_t *channel, const char *buf, int len );
typedef int  (*receive)( comm_channel_t *channel, char *buf, int len );
typedef int    (*start)( comm_channel_t *channel );
typedef int    (*flush)( comm_channel_t *channel );
typedef int     (*poll)( comm_channel_t *channel, long poll );

/* Enums for indicating a specific channel type */
typedef enum
{
    CH_SPI = 1,
    CH_SERIAL,
    CH_SOCKET,
	CH_MAX_TYPE,

} channel_type_t;

/* Structure for representing a communication channel */
struct channel
{
    channel_type_t  type;
    transmit        transmit;
    receive         receive;
    start           start;
    flush           flush;
    poll            poll;
    void *          data;
};

#endif /* !COMM_CHANNEL_H */

/* End of file */
