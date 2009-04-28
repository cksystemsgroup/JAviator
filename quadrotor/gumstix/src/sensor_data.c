/* $Id: sensor_data.c,v 1.6 2008/12/18 15:07:18 rtrummer Exp $ */

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

#include <stdio.h>

#include "sensor_data.h"


int sensor_data_to_stream( const sensor_data_t *sensor_data, char *buf, int len )
{
    if( len == SENSOR_DATA_SIZE )
    {
        buf[0]  = (char)( sensor_data->laser >> 24 );
        buf[1]  = (char)( sensor_data->laser >> 16 );
        buf[2]  = (char)( sensor_data->laser >> 8 );
        buf[3]  = (char)( sensor_data->laser );
        buf[4]  = (char)( sensor_data->sonar >> 8 );
        buf[5]  = (char)( sensor_data->sonar );
        buf[6]  = (char)( sensor_data->pressure >> 8 );
        buf[7]  = (char)( sensor_data->pressure );
        buf[8]  = (char)( sensor_data->battery >> 8 );
        buf[9]  = (char)( sensor_data->battery );
        buf[10] = (char)( sensor_data->sequence >> 8 );
        buf[11] = (char)( sensor_data->sequence );
        buf[12] = (char)( sensor_data->state );
        buf[13] = (char)( sensor_data->error );
        return( 0 );
    }

    fprintf( stderr, "ERROR: invalid length (%d) of sensor data to stream\n", len );
    return( -1 );
}

int sensor_data_from_stream( sensor_data_t *sensor_data, const char *buf, int len )
{
    if( len == SENSOR_DATA_SIZE )
    {
        sensor_data->laser     = (uint32_t)( (buf[0] << 24) | (buf[1] << 16) |
                                             (buf[2]  << 8) |  buf[3] );
        sensor_data->sonar     = (uint16_t)( (buf[4]  << 8) |  buf[5] );
        sensor_data->pressure  = (uint16_t)( (buf[6]  << 8) |  buf[7] );
        sensor_data->battery   = (uint16_t)( (buf[8]  << 8) |  buf[9] );
        sensor_data->sequence  = (uint16_t)( (buf[10] << 8) |  buf[11] );
        sensor_data->state     = (uint8_t)(   buf[12] );
        sensor_data->error     = (uint8_t)(                    buf[13] );
        return( 0 );
    }

    fprintf( stderr, "ERROR: invalid length (%d) of sensor data from stream\n", len );
    return( -1 );
}

// End of file.
