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


int sensor_data_to_stream( const sensor_data_t *data, char *buf, int len )
{
    if( len == SENSOR_DATA_SIZE )
    {
        buf[0]  = (char)( data->roll >> 8 );
        buf[1]  = (char)( data->roll );
        buf[2]  = (char)( data->pitch >> 8 );
        buf[3]  = (char)( data->pitch );
        buf[4]  = (char)( data->yaw >> 8 );
        buf[5]  = (char)( data->yaw );
        buf[6]  = (char)( data->droll >> 8 );
        buf[7]  = (char)( data->droll );
        buf[8]  = (char)( data->dpitch >> 8 );
        buf[9]  = (char)( data->dpitch );
        buf[10] = (char)( data->dyaw >> 8 );
        buf[11] = (char)( data->dyaw );
        buf[12] = (char)( data->ddroll >> 8 );
        buf[13] = (char)( data->ddroll );
        buf[14] = (char)( data->ddpitch >> 8 );
        buf[15] = (char)( data->ddpitch );
        buf[16] = (char)( data->ddyaw >> 8 );
        buf[17] = (char)( data->ddyaw );
        buf[18] = (char)( data->x >> 8 );
        buf[19] = (char)( data->x );
        buf[20] = (char)( data->y >> 8 );
        buf[21] = (char)( data->y );
        buf[22] = (char)( data->z >> 8 );
        buf[23] = (char)( data->z );
        buf[24] = (char)( data->dx >> 8 );
        buf[25] = (char)( data->dx );
        buf[26] = (char)( data->dy >> 8 );
        buf[27] = (char)( data->dy );
        buf[28] = (char)( data->dz >> 8 );
        buf[29] = (char)( data->dz );
        buf[30] = (char)( data->ddx >> 8 );
        buf[31] = (char)( data->ddx );
        buf[32] = (char)( data->ddy >> 8 );
        buf[33] = (char)( data->ddy );
        buf[34] = (char)( data->ddz >> 8 );
        buf[35] = (char)( data->ddz );
        buf[36] = (char)( data->battery >> 8 );
        buf[37] = (char)( data->battery );
        return( 0 );
    }

    fprintf( stderr, "ERROR in %s %d: invalid length (%d) of sensor data to stream\n",
        __FILE__, __LINE__, len );
    return( -1 );
}

int sensor_data_from_stream( sensor_data_t *data, const char *buf, int len )
{
    if( len == SENSOR_DATA_SIZE )
    {
        data->roll    = (buf[0]  << 8) | (buf[1]  & 0xFF);
        data->pitch   = (buf[2]  << 8) | (buf[3]  & 0xFF);
        data->yaw     = (buf[4]  << 8) | (buf[5]  & 0xFF);
        data->droll   = (buf[6]  << 8) | (buf[7]  & 0xFF);
        data->dpitch  = (buf[8]  << 8) | (buf[9]  & 0xFF);
        data->dyaw    = (buf[10] << 8) | (buf[11] & 0xFF);
        data->ddroll  = (buf[12] << 8) | (buf[13] & 0xFF);
        data->ddpitch = (buf[14] << 8) | (buf[15] & 0xFF);
        data->ddyaw   = (buf[16] << 8) | (buf[17] & 0xFF);
        data->x       = (buf[18] << 8) | (buf[19] & 0xFF);
        data->y       = (buf[20] << 8) | (buf[21] & 0xFF);
        data->z       = (buf[22] << 8) | (buf[23] & 0xFF);
        data->dx      = (buf[24] << 8) | (buf[25] & 0xFF);
        data->dy      = (buf[26] << 8) | (buf[27] & 0xFF);
        data->dz      = (buf[28] << 8) | (buf[29] & 0xFF);
        data->ddx     = (buf[30] << 8) | (buf[31] & 0xFF);
        data->ddy     = (buf[32] << 8) | (buf[33] & 0xFF);
        data->ddz     = (buf[34] << 8) | (buf[35] & 0xFF);
        data->battery = (buf[36] << 8) | (buf[37] & 0xFF);
        return( 0 );
    }

    fprintf( stderr, "ERROR in %s %d: invalid length (%d) of sensor data from stream\n",
        __FILE__, __LINE__, len );
    return( -1 );
}

/* End of file */
