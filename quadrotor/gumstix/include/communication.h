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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "comm_channel.h"

/* Structure for representing a communication packet */
typedef struct
{
    int     type;               /* packet type */
    int     size;               /* payload size */
    int     buf_size;           /* payload buffer size*/
    void *  payload;            /* pointer to payload */
    int     checksum;           /* packet checksum */
    int     bytes_in_payload;   /* received bytes of the payload */
    int     state;              /* receive state of this packet */

} comm_packet_t;

int comm_recv_packet( comm_channel_t *channel, comm_packet_t *packet );

int comm_send_packet( comm_channel_t *channel, const comm_packet_t *packet );

#endif /* !COMMUNICATION_H */

/* End of file */
