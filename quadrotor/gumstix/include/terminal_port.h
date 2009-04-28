/* $Id: terminal_port.h,v 1.5 2008/11/11 19:28:40 hroeck Exp $ */

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

#ifndef TERMINAL_PORT_H
#define TERMINAL_PORT_H

#include "channel.h"
#include "navigation.h"
#include "pwm_signals.h"
#include "sensor_data.h"
#include "communication.h"
#include "param.h"
#include "trim.h"

int  terminal_port_init( struct channel *channel );
void terminal_port_set_multiplier( int m );
int  terminal_port_tick( long long deadline );

int  terminal_port_is_new_navigation( void );
int  terminal_port_get_navigation( struct navigation_data *navigation );
int  terminal_port_is_shutdown( void );
int  terminal_port_reset_shutdown( void );

int  terminal_port_is_new_trim( void );
int  terminal_port_is_new_params( void );
int  terminal_port_is_testmode( void );
int  terminal_port_is_mode_switch( void );

int  terminal_port_get_trim( struct trim_data *trim );
int  terminal_port_get_params( struct parameters *param );
int  terminal_port_get_base_motor_speed( void );

int  terminal_port_send_report( const sensor_data_t *sensors,
        const pwm_signals_t *motors,
        const pwm_signals_t *motor_offsets,
        const int state, const int mode );

int  terminal_port_send_sensors( const sensor_data_t *sensors );
int  terminal_port_send_motors( const pwm_signals_t *motors );
int  terminal_port_send_motorOffsets( const struct navigation_data *offsets );
int  terminal_port_send_state( const int altitudeMode, const int controlState );

int  terminal_port_forward( const struct com_packet *packet );

#endif // !TERMINAL_PORT_H

// End of file.
