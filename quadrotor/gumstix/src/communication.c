/* $Id: communication.c,v 1.4 2008/11/10 12:17:57 hroeck Exp $ */

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

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include <../shared/protocol.h>

#include "controller.h"
#include "communication.h"
#include "channel.h"

enum com_state {
    st_MARK1 = 1,
    st_MARK2,
    st_TYPE,
    st_SIZE,
    st_PAYLOAD,
    st_SUM1,
    st_SUM2,
    st_COMPLETE,
};

static inline int _transmit(struct channel *channel, const char *buf, int size)
{
    int res = channel->transmit(channel, buf, size);

#if DEBUG > 2
    if (res != 0 && errno != EAGAIN)
        fprintf(stderr, "transmit error\n");
#endif
    return res;
}

static inline int _receive(struct channel *channel, char *buf, int size)
{
    int res;
    if (size > 0)
        res = channel->receive(channel, buf, size);
    else
        res = 0;
#if DEBUG > 2
    if (res != 0 && errno != EAGAIN)
        fprintf(stderr, "receive error\n");
#endif
    return res;
}

static inline int _flush(struct channel *channel)
{
    return channel->flush(channel);
}

int com_send_packet( struct channel *channel, const struct com_packet *packet )
{
    char     buf[COMM_BUF_SIZE];
    int      len      = 0;
    uint8_t  size     = packet->length;
    uint8_t  *data    = (uint8_t *) packet->payload;
    uint16_t checksum = packet->type + size;

    if( packet->length > packet->buf_length ||
        packet->length > COMM_BUF_SIZE )
    {
        fprintf( stderr, "ERROR: invalid packet length\n" );
        return( -1 );
    }

    if( packet->type > COMM_PACKET_LIMIT )
    {
        fprintf( stderr, "ERROR: invalid packet type\n" );
        return( -1 );
    }

    buf[len++] = COMM_PACKET_MARK;
    buf[len++] = COMM_PACKET_MARK;
    buf[len++] = packet->type;
    buf[len++] = size;

    while( size-- )
    {
        checksum += *data;
        buf[len++] = *data++;
    }

    buf[len++] = (uint8_t)( checksum >> 8 );
    buf[len++] = (uint8_t)( checksum );

    if( _transmit( channel, buf, len ) )
    {
        return( -1 ); 
    }

    _flush( channel );
    return( 0 );
}

static inline uint16_t calc_checksum(struct com_packet *packet)
{
    uint16_t checksum; 
    int i;
        
    checksum = packet->type + packet->length;

    for (i=0; i<packet->length; ++i)
        checksum += *((uint8_t *)(packet->payload) + i);

    return checksum;
}


int com_recv_packet(struct channel *channel, struct com_packet *packet)
{
    char *buf = (char *)packet->payload;
    uint8_t c;
    uint16_t mchecksum;
    int res, retval = EAGAIN;
    enum com_state pstate;

redo:
    pstate = packet->_state;

    if (pstate == 0) {
        pstate = packet->_state = st_MARK1;
        packet->bytes_in_payload = 0;
    }

#if DEBUG > 2
    printf("pstate %d\n", pstate);
#endif
    switch (pstate) {
        case st_MARK1:
            res = _receive(channel, (char *)&c, 1);
            if (res == 1 && c == 0xff) {
                packet->_state = st_MARK2;
                goto redo;
            }
            retval = EAGAIN;
            break;
        case st_MARK2:
            res = _receive(channel, (char *)&c, 1);
            retval = EAGAIN;
            if (res == 0) {
                break;
            } else if (c == 0xff) {
                packet->_state = st_TYPE;
                goto redo;
            } else {
                packet->_state = st_MARK1;
                goto redo;
            }
            break;
        case st_TYPE:
            res = _receive(channel, (char *)&c, 1);
            retval = EAGAIN;
            if(res != 1)
                break;
            if (c >= COMM_GET_NAME && c <= COMM_PACKET_LIMIT) {
                packet->type = c;
                packet->_state = st_SIZE;
                goto redo;
            } else {
                packet->_state = st_MARK1;
                goto redo;
            }
            break;
        case st_SIZE:
            res = _receive(channel, (char *)&c, 1);
            retval = EAGAIN;
            if (res != 1) 
                break;
            if (c > packet->buf_length) {
                printf("WARNING: payload larger than buffer; ignoring packet\n");
                packet->_state = st_MARK1;
                goto redo;
                break;
            }
            packet->length = c;
            if (c > 0) {
                packet->_state = st_PAYLOAD;
                goto redo;
            } else {
                packet->_state = st_SUM1;
                goto redo;
            }
            break;
        case st_PAYLOAD:
            res = _receive(channel, buf + packet->bytes_in_payload,
                packet->length - packet->bytes_in_payload);
            retval = EAGAIN;
            if (res > 0)
                packet->bytes_in_payload += res;
            if (packet->bytes_in_payload == packet->length) {
                packet->_state = st_SUM1;
                goto redo;
            }
            break;
        case st_SUM1:
            res = _receive(channel, (char *)&c, 1);
            retval = EAGAIN;
            if (res != 1) 
                break;
            packet->checksum = (uint8_t)c;
            packet->checksum <<= 8;
            packet->_state = st_SUM2;
            goto redo;
            break;
        case st_SUM2:
            res = _receive(channel, (char *)&c, 1);
            retval = EAGAIN;
            if (res != 1) 
                break;
            packet->checksum |= (uint8_t)c;
            mchecksum = calc_checksum(packet);
            if (packet->checksum == mchecksum) {
                packet->_state = st_COMPLETE;
                goto redo;
            } else {
                com_print_packet(packet);
                packet->_state = st_MARK1;
            } 
            break;
        case st_COMPLETE:
            retval = 0;
            packet->_state = 0;
            packet->bytes_in_payload = 0;
            break;
    }

#if DEBUG > 2
    printf("return com_recv %d\n", retval);
#endif
    return retval;
}

int com_is_packet(struct channel *channel)
{
    return 0;
}

