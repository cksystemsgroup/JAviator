/*
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

#ifndef INERTIAL_PORT_H
#define INERTIAL_PORT_H

#include "comm_channel.h"
#include "inertial_data.h"

int inertial_port_init( comm_channel_t *channel );

int inertial_port_tick( void );

int inertial_port_send_request( void );

int inertial_port_send_start( void );

int inertial_port_send_stop( void );

int inertial_port_get_data( inertial_data_t *data );

#endif /* !INERTIAL_PORT_H */

/* End of file */
