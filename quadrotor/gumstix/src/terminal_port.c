/* $Id: terminal_port.c,v 1.7 2008/11/11 19:28:40 hroeck Exp $ */

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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <../shared/protocol.h>

#include "controller.h"
#include "socket_channel.h"
#include "terminal_port.h"
#include "javiator_port.h"
#include "navigation.h"
#include "pwm_signals.h"
#include "sensor_data.h"
#include "communication.h"
#include "param.h"
#include "trim.h"
#include "utimer.h"

static struct channel *         _channel;
static struct navigation_data   _navi;
static struct trim_data         _trim;
static struct parameters        _param;
static struct com_packet        _packet;
static char                     _packet_buf[COMM_BUF_SIZE];
static int                      _shutdown;
static int                      _mode_switch;
static int                      _testmode;
static int                      _base_motor_speed;
static int                      _new_navigation_data;
static int                      _new_trim_data;
static int                      _new_parameters;


static inline int send_name()
{
    char *name = "CControl";
    struct com_packet packet;
    packet.type = COMM_ACK_NAME;
    packet.length = strlen(name);
    packet.payload = name;
    packet.buf_length = packet.length;
    return com_send_packet(_channel, &packet);
}

static inline int set_test_mode(const struct com_packet *packet)
{
    char *p = (char *)packet->payload;
    _testmode = (int)p[0];
    return 0;
}

static inline int set_idle_limit(const struct com_packet *packet)
{
    char *p = (char *)packet->payload;
    _base_motor_speed = (int)((short)(p[0] << 8) | (short)(p[1] & 0xFF));
    return 0;
}

static inline void set_shutdown()
{
    _shutdown = 1;
}

static inline void set_mode_switch()
{
    _mode_switch = 1;
}

static inline int parse_navi_packet(const struct com_packet *packet)
{
    _new_navigation_data = 1;
    return navigation_from_stream(&_navi, packet->payload, packet->length);
}

static inline int parse_trim_packet(const struct com_packet *packet)
{
    _new_trim_data = 1;
    return trim_from_stream(&_trim, packet->payload, packet->length);
}

static inline int parse_param_packet(const struct com_packet *packet)
{
    _new_parameters = 1;
    return params_from_stream(&_param, packet->payload, packet->length);
}

static int process_packet(const struct com_packet *packet)
{
    if (!packet)
        return -1;

    com_print_packet(packet);
    switch (packet->type)
    {
        case COMM_GET_NAME:
            return send_name();
        case COMM_TEST_MODE:
            return set_test_mode(packet);
        case COMM_IDLE_LIMIT:
            return set_idle_limit(packet);
        case COMM_NAVI_DATA:
            return parse_navi_packet(packet);
        case COMM_TRIM_DATA:
            return parse_trim_packet(packet);
        case COMM_CTRL_PARAMS:
            return parse_param_packet(packet);
        case COMM_SWITCH_MODE:
            set_mode_switch();
            /* ignore the following */
        case COMM_LOG_DATA:
        case COMM_STREAM:
            return 0;
            /* shutdown process and immediately forward it */
        case COMM_SHUT_DOWN:
            set_shutdown();
            /* fall through */
        default:
            return javiator_port_forward(packet);
    }
    return -1;
}

int terminal_port_tick(long long deadline)
{
    int res;

    res = com_recv_packet(_channel, &_packet);
    if (res == 0) {
        process_packet(&_packet);
    } else if (res == EAGAIN) {
        res = 0;
    } else {
        fprintf(stderr, "ERROR in %s %d: cannot receive from terminal channel\n",
            __FILE__, __LINE__);
    }
    return res;
}

int terminal_port_init(struct channel *channel)
{
    static int __initialized = 0;
    if(!__initialized) {
        _channel = channel;
        memset(&_navi, 0, sizeof(_navi));
        memset(&_trim, 0, sizeof(_trim));
        memset(&_param, 0, sizeof(_param));
        memset(&_packet, 0, sizeof(_packet));
        _packet.payload = _packet_buf;
        _packet.buf_length = COMM_BUF_SIZE;
        __initialized = 1;
        return 0;
    }
    fprintf(stderr, "WARNING: try to init the terminal port twice\n");
    return 1;
}

int terminal_port_is_new_navigation()
{
    return _new_navigation_data;
}

int terminal_port_get_navigation(struct navigation_data *navigation)
{
    memcpy(navigation, &_navi, sizeof(*navigation));
    _new_navigation_data = 0;
    return 0;
}

int terminal_port_is_new_trim()
{
    return _new_trim_data;
}

int terminal_port_get_trim(struct trim_data *trim)
{
    memcpy(trim, &_trim, sizeof(*trim));
    _new_trim_data = 0;
    return 0;
}

int terminal_port_is_new_params()
{
    return _new_parameters;
}

int terminal_port_get_params(struct parameters *param)
{
    memcpy(param, &_param, sizeof(*param));
    _new_parameters = 0;
    return 0;
}

int terminal_port_is_testmode()
{
    return _testmode;
}

int terminal_port_get_base_motor_speed()
{
    return _base_motor_speed;
}

int terminal_port_is_shutdown()
{
    return _shutdown;
}

int terminal_port_reset_shutdown()
{
    _shutdown = 0;
    return 0;
}

int terminal_port_is_mode_switch()
{
    int mode_switch = _mode_switch;
    _mode_switch = 0;
    return mode_switch;
}

int terminal_port_send_sensors(const sensor_data_t *sensors)
{
    struct com_packet packet;
    char buf[SENSOR_DATA_SIZE];

    sensor_data_to_stream(sensors, buf, SENSOR_DATA_SIZE );
    packet.length = packet.buf_length = SENSOR_DATA_SIZE;
    packet.type = COMM_SENSOR_DATA;
    packet.payload = buf;
    return com_send_packet(_channel, &packet);
}

int terminal_port_send_motors(const pwm_signals_t *motors)
{
    struct com_packet packet;
    char buf[PWM_SIGNALS_SIZE];

    pwm_signals_to_stream(motors, buf, PWM_SIGNALS_SIZE );
    packet.length = packet.buf_length = PWM_SIGNALS_SIZE;
    packet.type = COMM_PWM_SIGNALS;
    packet.payload = buf;
    return com_send_packet(_channel, &packet);
}

int terminal_port_send_motorOffsets(const struct navigation_data *offsets)
{
    struct com_packet packet;
    char buf[NAVIGATION_PACKET_LENGTH];

    navigation_to_stream(offsets, buf, NAVIGATION_PACKET_LENGTH);
    packet.length = packet.buf_length = NAVIGATION_PACKET_LENGTH;
    packet.type = COMM_NAVI_DATA;
    packet.payload = buf;
    return com_send_packet(_channel, &packet);
}

int terminal_port_send_state(const int mode, const int state)
{
    struct com_packet packet;
    char buf[2];
    buf[0] = (char)state;
    buf[1] = (char)mode;
    packet.length = packet.buf_length = 2;
    packet.type = COMM_HELI_STATE;
    packet.payload = buf;
    return com_send_packet(_channel, &packet);
}

static int multiplier = 1;
void terminal_port_set_multiplier(int m)
{
    multiplier = m;
}

int terminal_port_send_report(const sensor_data_t *sensors,
    const pwm_signals_t *motors,
    const pwm_signals_t *motor_offsets,
    const int state, const int mode)
{
    int i = 0;
    static int counter = 1;

    if (--counter == 0) {
        struct com_packet packet;
        char buf[SENSOR_DATA_SIZE
            + PWM_SIGNALS_SIZE
            + NAVIGATION_PACKET_LENGTH
            + 2]; /* for state and mode */

        sensor_data_to_stream(sensors, &buf[i], SENSOR_DATA_SIZE);

        i += SENSOR_DATA_SIZE;
        pwm_signals_to_stream(motors, &buf[i], PWM_SIGNALS_SIZE);

        i += PWM_SIGNALS_SIZE;
        pwm_signals_to_stream(motor_offsets, &buf[i], PWM_SIGNALS_SIZE);

        i += PWM_SIGNALS_SIZE;
        buf[i++] = (char) state;
        buf[i++] = (char) mode;

        packet.type = COMM_GROUND_REPORT;
        packet.length = packet.buf_length = i;
        packet.payload = buf;
        counter = multiplier;
        return com_send_packet(_channel, &packet);
    }

    return 0;
}

int terminal_port_forward(const struct com_packet *packet)
{
    /* send data directly to channel */
    return com_send_packet(_channel, packet);
}
