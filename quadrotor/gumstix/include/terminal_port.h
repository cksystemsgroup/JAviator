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

#ifndef TERMINAL_PORT_H
#define TERMINAL_PORT_H

#include "communication.h"
#include "comm_channel.h"
#include "command_data.h"
#include "ctrl_params.h"
#include "javiator_data.h"
#include "sensor_data.h"
#include "motor_signals.h"
#include "motor_offsets.h"
#include "trace_data.h"

int terminal_port_tick( void );

int terminal_port_init( comm_channel_t *channel );

int terminal_port_set_multiplier( int m );

int terminal_port_reset_shut_down( void );

int terminal_port_is_new_command_data( void );

int terminal_port_is_new_r_p_params( void );

int terminal_port_is_new_yaw_params( void );

int terminal_port_is_new_alt_params( void );

int terminal_port_is_new_x_y_params( void );

int terminal_port_is_state_switch( void );

int terminal_port_is_mode_switch( void );

int terminal_port_is_store_trim( void );

int terminal_port_is_clear_trim( void );

int terminal_port_is_shut_down( void );

int terminal_port_get_command_data( command_data_t *data );

int terminal_port_get_r_p_params( ctrl_params_t *params );

int terminal_port_get_yaw_params( ctrl_params_t *params );

int terminal_port_get_alt_params( ctrl_params_t *params );

int terminal_port_get_x_y_params( ctrl_params_t *params );

int terminal_port_get_base_motor_speed( void );

int terminal_port_send_sensor_data( const sensor_data_t *data );

int terminal_port_send_motor_signals( const motor_signals_t *signals );

int terminal_port_send_motor_offsets( const motor_offsets_t *offsets );

int terminal_port_send_mode_and_state( const int state, const int mode );

int terminal_port_send_report( const sensor_data_t *sensors,
    const motor_signals_t *signals, const motor_offsets_t *offsets,
    const int state, const int mode );

int terminal_port_send_trace_data( const trace_data_t *data );

int terminal_port_forward( const comm_packet_t *packet );

#endif /* !TERMINAL_PORT_H */

/* End of file */
