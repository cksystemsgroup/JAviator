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
 *    _new_sensor_data = 1;
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
#include "../shared/transfer.h"
#include "controller.h"
#include "serial_channel.h"
#include "inertial_port.h"
#include "terminal_port.h"
#include "communication.h"
#include "pwm_signals.h"
#include "utimer.h"

static struct channel *     _channel;
static sensor_data_t        _sensors;
static struct com_packet    _packet;
static char                 _packet_buf[COMM_BUF_SIZE];
static int                  _new_sensor_data;
static int                  _connected;


static inline int parse_ack_name( void )
{
    _connected = 1;
    return 0;
}

static inline int parse_sensors( const struct com_packet *packet )
{
    int res = sensor_data_from_stream(&_sensors, packet->payload, packet->length);

    _new_sensor_data = 1;

#if DEBUG > 1
    printf("new sensor data\n");
#endif
    return res;
}

static int process_packet( const struct com_packet *packet )
{
    if(!packet) {
        return -1;
    }

    com_print_packet(packet);

    switch (packet->type)
    {
        case COMM_ACK_NAME:
            return parse_ack_name();

        case COMM_SENSOR_DATA:
            return parse_sensors(packet);

        default:
            return terminal_port_forward(packet);
    }

    return -1;
}

int inertial_port_connect( void )
{
    return 0;
}

int inertial_port_init( struct channel *channel )
{
    _new_sensor_data = 0;
    memset(&_sensors, 0, sizeof(_sensors));
    memset(&_packet, 0, sizeof(_packet));
    _packet.payload = &_packet_buf;
    _packet.buf_length = COMM_BUF_SIZE;
    _channel = channel;
    _connected = 1;

    return 0;
}

int inertial_port_tick( long long deadline )
{
    /* check for new input from the channel */
    int res = com_recv_packet(_channel, &_packet);

    if (res == 0) {
        process_packet(&_packet);
    } else if (res == EAGAIN) {
        return res;
    } else {
        fprintf(stderr, "ERROR in %s %d: cannot receive from IMU channel\n",
                __FILE__, __LINE__);
    }

    return res;
}

int inertial_port_is_new_sensors( void )
{
    return _new_sensor_data;
}

int inertial_port_get_sensors( sensor_data_t *sensors )
{
    int res;

    do
    {
        res = inertial_port_tick(0);

        if (res == EAGAIN)
        {
            sleep_for(1000);
        }
        else
        if (res != 0)
        {
            return 1;
        }
    }
    while (!_new_sensor_data);

    _new_sensor_data = 0;
    memcpy(sensors, &_sensors, sizeof(*sensors));
    return 0;
}

int inertial_port_send_motors( const pwm_signals_t *motors )
{
    struct com_packet packet;
    char buf[PWM_SIGNALS_SIZE];
    int res;

    if (!_connected) {
        return 0;
    }

    pwm_signals_to_stream(motors, buf, PWM_SIGNALS_SIZE );
    packet.type = COMM_PWM_SIGNALS;
    packet.length = packet.buf_length = PWM_SIGNALS_SIZE;
    packet.payload = buf;
    res = com_send_packet(_channel, &packet);

    if (res == -1) {
        _connected = 0;
    }

    return res;
}

int inertial_port_forward( const struct com_packet *packet )
{
    if (!_connected) {
        return 0;
    }

    return com_send_packet(_channel, packet);
}

// End of file.
