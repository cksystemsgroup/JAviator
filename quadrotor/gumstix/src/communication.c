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

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include "communication.h"
#include "comm_channel.h"
#include "protocol.h"
#include "controller.h"

typedef enum
{
    st_MARK1 = 1,
    st_MARK2,
    st_TYPE,
    st_SIZE,
    st_PAYLOAD,
    st_SUM1,
    st_SUM2,
    st_COMPLETE,

} comm_state_t;


static inline int _transmit( comm_channel_t *channel, const char *buf, int len )
{
    if( len > 0 )
    {
        return channel->transmit( channel, buf, len );
    }

    return( 0 );
}

static inline int _receive( comm_channel_t *channel, char *buf, int len )
{
    if( len > 0 )
    {
        return channel->receive( channel, buf, len );
    }

    return( 0 );
}

static inline uint16_t calc_checksum( comm_packet_t *packet )
{
    uint16_t checksum = packet->type + packet->size;
    int i;

    for( i = 0; i < packet->size; ++i )
    {
        checksum += *( (uint8_t *) packet->payload + i );
    }

    return( checksum );
}

int comm_recv_packet( comm_channel_t *channel, comm_packet_t *packet )
{
    uint8_t c, *buf = (char *) packet->payload;
    int res, mchecksum , retval = EAGAIN;

redo:
    if( packet->state == 0 )
    {
        packet->state = st_MARK1;
        packet->bytes_in_payload = 0;
    }

    switch( packet->state )
    {
        case st_MARK1:
            retval = EAGAIN;
            res = _receive( channel, &c, 1 );
            if( res == 1 && c == COMM_PACKET_MARK )
            {
                packet->state = st_MARK2;
                goto redo;
            }
            break;

        case st_MARK2:
            retval = EAGAIN;
            res = _receive( channel, &c, 1 );
            if( res != 1 )
            {
                break;
            }
            if( c == COMM_PACKET_MARK )
            {
                packet->state = st_TYPE;
                goto redo;
            }
            else
            {
                packet->state = st_MARK1;
                goto redo;
            }
            break;

        case st_TYPE:
            retval = EAGAIN;
            res = _receive( channel, &c, 1 );
            if( res != 1 )
            {
                break;
            }
            if( c >= 0x01 && c <= COMM_PACKET_LIMIT )
            {
                packet->type = c;
                packet->state = st_SIZE;
                goto redo;
            }
            else
            {
                packet->state = st_MARK1;
                goto redo;
            }
            break;

        case st_SIZE:
            retval = EAGAIN;
            res = _receive( channel, &c, 1 );
            if( res != 1 )
            {
                break;
            }
            if( c > packet->buf_size )
            {
                fprintf( stderr, "WARNING: payload larger than buffer\n" );
                packet->state = st_MARK1;
                goto redo;
            }
            packet->size = c;
            if( c > 0 )
            {
                packet->state = st_PAYLOAD;
                goto redo;
            }
            else
            {
                packet->state = st_SUM1;
                goto redo;
            }
            break;

        case st_PAYLOAD:
            retval = EAGAIN;
            res = _receive( channel, buf + packet->bytes_in_payload,
                packet->size - packet->bytes_in_payload );
            if( res > 0 )
            {
                packet->bytes_in_payload += res;
            }
            if( packet->bytes_in_payload == packet->size )
            {
                packet->state = st_SUM1;
                goto redo;
            }
            break;

        case st_SUM1:
            retval = EAGAIN;
            res = _receive( channel, &c, 1 );
            if( res != 1 )
            { 
                break;
            }
            packet->checksum = c << 8;
            packet->state = st_SUM2;
            goto redo;
            break;

        case st_SUM2:
            retval = EAGAIN;
            res = _receive( channel, &c, 1 );
            if( res != 1 )
            { 
                break;
            }
            packet->checksum |= c;
            mchecksum = calc_checksum( packet );
            if( packet->checksum == mchecksum )
            {
                packet->state = st_COMPLETE;
                goto redo;
            }
            else
            {
                retval = EAGAIN;
                fprintf( stderr, "WARNING: checksum error\n" );
                packet->state = 0;
            } 
            break;

        case st_COMPLETE:
            retval = 0;
            packet->state = 0;
    }

    return( retval );
}

int comm_send_packet( comm_channel_t *channel, const comm_packet_t *packet )
{
    char     buf[ COMM_BUF_SIZE ];
    int      len      = 0;
    uint8_t  size     = packet->size;
    uint8_t  *data    = (uint8_t *) packet->payload;
    uint16_t checksum = packet->type + packet->size;

    if( packet->size > packet->buf_size ||
        packet->size > COMM_BUF_SIZE )
    {
        fprintf( stderr, "ERROR in %s %d: invalid packet size\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    if( packet->type > COMM_PACKET_LIMIT )
    {
        fprintf( stderr, "ERROR in %s %d: invalid packet type\n",
            __FILE__, __LINE__ );
        return( -1 );
    }

    buf[ len++ ] = COMM_PACKET_MARK;
    buf[ len++ ] = COMM_PACKET_MARK;
    buf[ len++ ] = packet->type;
    buf[ len++ ] = size;

    while( size-- )
    {
        checksum += *data;
        buf[ len++ ] = *data++;
    }

    buf[ len++ ] = (uint8_t)( checksum >> 8 );
    buf[ len++ ] = (uint8_t)( checksum );

    if( _transmit( channel, buf, len ) )
    {
        return( -1 ); 
    }

    return( 0 );
}

/* End of file */
