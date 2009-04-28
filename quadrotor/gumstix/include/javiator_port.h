/* $Id: javiator_port.h,v 1.2 2008/11/10 12:17:57 hroeck Exp $ */

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

#ifndef JAVIATOR_PORT_H
#define JAVIATOR_PORT_H

#include "channel.h"
#include "pwm_signals.h"
#include "sensor_data.h"
#include "communication.h"

int javiator_port_init( struct channel *channel );
//int javiator_port_tick( long long deadline );
//int javiator_port_connect( void );
//int javiator_port_is_new_sensors( void );
int javiator_port_get_sensors( sensor_data_t *sensors );
int javiator_port_send_motors( const pwm_signals_t *motors );
int javiator_port_forward( const struct com_packet *packet );

#endif // !JAVIATOR_PORT_H

// End of file.
