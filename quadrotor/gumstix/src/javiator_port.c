/* $Id: javiator_port.c,v 1.4 2008/11/10 12:17:57 hroeck Exp $ */

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "../shared/protocol.h"
#include "controller.h"
#include "serial_channel.h"
#include "javiator_port.h"
#include "terminal_port.h"
#include "communication.h"
#include "pwm_signals.h"
#include "utimer.h"

#include "spi_channel.h"

static struct channel *     _channel;
static sensor_data_t        _sensors;
static struct com_packet    _packet;
static char                 _packet_buf[COMM_BUF_SIZE];
static int                  _new_sensor_data;


static inline int parse_sensors(const struct com_packet *packet)
{
    int res = sensor_data_from_stream(&_sensors, packet->payload, packet->length);

    _new_sensor_data = 1;

#if DEBUG > 1
    printf("new sensor data\n");
#endif
    return res;
}

static int process_packet(const struct com_packet *packet)
{
    if(!packet) {
        return -1;
    }

    switch (packet->type)
    {
        case COMM_SENSOR_DATA:
            return parse_sensors(packet);

        default:
            return terminal_port_forward(packet);
    }

    return -1;
}

int javiator_port_init(struct channel *channel)
{
    pwm_signals_t motors = { 0, 0, 0, 0 };

    _new_sensor_data = 0;
    memset(&_sensors, 0, sizeof(_sensors));
    memset(&_packet, 0, sizeof(_packet));
    _packet.payload = &_packet_buf;
    _packet.buf_length = COMM_BUF_SIZE;
    _channel = channel;

    javiator_port_send_motors( &motors );

    return 0;
}

int javiator_port_tick( void )
{
    int res = com_recv_packet( _channel, &_packet );

    if( res == 0 )
    {
        process_packet( &_packet );
    }
    else
    if( res == EAGAIN )
    {
        return( res );
    }
    else
    {
        fprintf( stderr, "ERROR in %s %d: cannot receive from JAviator channel\n",
            __FILE__, __LINE__ );
    }

    return( res );
}

int javiator_port_get_sensors( sensor_data_t *sensors )
{
    pwm_signals_t motors = { 0, 0, 0, 0 };
    int res, attempts = 0;

    do
    {
        res = javiator_port_tick( );

        if( res == EAGAIN )
        {
            if( ++attempts > 10 )
            {
                return( -1 );
            }

            javiator_port_send_motors( &motors );
            sleep_for( 1000 );
        }
        else
        if( res != 0 )
        {
            return( -1 );
        }
    }
    while( !_new_sensor_data );

    memcpy( sensors, &_sensors, sizeof( *sensors ) );
    _new_sensor_data = 0;

    return( 0 );
}

int javiator_port_send_motors(const pwm_signals_t *motors)
{
    struct com_packet packet;
    char buf[PWM_SIGNALS_SIZE];
    int res;

    pwm_signals_to_stream(motors, buf, PWM_SIGNALS_SIZE );
    packet.type = COMM_PWM_SIGNALS;
    packet.length = packet.buf_length = PWM_SIGNALS_SIZE;
    packet.payload = buf;
    res = com_send_packet(_channel, &packet);

    return res;
}

int javiator_port_forward(const struct com_packet *packet)
{
    return com_send_packet(_channel, packet);
}

// End of file.
