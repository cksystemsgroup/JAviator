/* $Id: channel.c,v 1.1 2008/10/16 14:41:13 rtrummer Exp $ */

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

#include "channel.h"
#include "spi_channel.h"
#include "serial_channel.h"
#include "socket_channel.h"

int controller_channel_create( struct channel *channel, channel_type type )
{
    switch( type )
    {
        case CH_SPI:
            return spi_channel_create( channel );

        case CH_SERIAL:
            return serial_channel_create( channel );

        case CH_SOCKET:
            return socket_channel_create( channel );
    }

    return( -1 );
}

int controller_channel_destroy( struct channel *channel )
{
    switch( channel->type )
    {
        case CH_SPI:
            return spi_channel_destroy( channel );

        case CH_SERIAL:
            return serial_channel_destroy( channel );

        case CH_SOCKET:
            return socket_channel_destroy( channel );
    }

    return( -1 );
}

// End of file.
