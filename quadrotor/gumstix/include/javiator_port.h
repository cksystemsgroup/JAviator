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

#ifndef JAVIATOR_PORT_H
#define JAVIATOR_PORT_H

#include "comm_channel.h"
#include "communication.h"
#include "javiator_data.h"
#include "motor_signals.h"

//#define SHORT_JAVIATOR_DATA

int javiator_port_init( comm_channel_t *channel );
#ifdef SHORT_JAVIATOR_DATA
int javiator_port_get_data( javiator_sdat_t *data );
#else
int javiator_port_get_data( javiator_ldat_t *data );
#endif
int javiator_port_send_ctrl_period( int period );

int javiator_port_send_enable_sensors( int enable );

int javiator_port_send_motor_signals( const motor_signals_t *signals );

int javiator_port_forward( const comm_packet_t *packet );

#endif /* !JAVIATOR_PORT_H */

/* End of file */
