/* $Id: communication.h,v 1.5 2008/11/04 16:04:37 rtrummer Exp $ */

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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "channel.h"

#if DEBUG > 2
#include <stdint.h>
#include <stdio.h>
#endif

struct com_packet
{
    int type;
    /* number of bytes to send in the payload */
    int length;
    /* number of bytes available in the payload buffer */
    int buf_length;
    void *payload;
    int checksum;

    /* private don't touch */
    /* receive state of this packet */
    int _state;
    /* received bytes of the payload */
    int bytes_in_payload;
};

/*
 * return 0 if packet was send successfully
 * return -1 on error
 */
int com_send_packet(struct channel *channel, const struct com_packet *packet);

/*
 * return 0 if a complete packet was received
 * return EAGAIN when the packet is not received completely
 * return -1 on error
 */
int com_recv_packet(struct channel *channel, struct com_packet *packet);
int com_is_packet(struct channel *channel);

static inline void com_print_packet(const struct com_packet *packet)
{
#if DEBUG > 2
    int i;
    printf("packet:\n"
            "\ttype %d\n"
            "\tsize %d\n"
            "\tpayload",
                packet->type, packet->length);
    for(i = 0; i < packet->length; ++i)
        printf(" 0x%02x", *((uint8_t *)(packet->payload) + i));
    printf("\n");
#endif
}
#endif /* COMMUNICATION_H */
