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

#include "inertial_data.h"


int inertial_data_to_stream( const inertial_data_t *data, char *buf, int len )
{
    if( len == INERTIAL_DATA_SIZE )
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
        buf[12] = (char)( data->ddx >> 8 );
        buf[13] = (char)( data->ddx );
        buf[14] = (char)( data->ddy >> 8 );
        buf[15] = (char)( data->ddy );
        buf[16] = (char)( data->ddz >> 8 );
        buf[17] = (char)( data->ddz );
        buf[18] = (char)( data->ticks >> 8 );
        buf[19] = (char)( data->ticks );
        return( 0 );
    }

    fprintf( stderr, "ERROR in %s %d: invalid length (%d) of 3DM-GX1 data to stream\n",
        __FILE__, __LINE__, len );
    return( -1 );
}

int inertial_data_from_stream( inertial_data_t *data, const char *buf, int len )
{
    if( len == DM3_GX1_DATA_SIZE - 1 )
    {
        data->roll   = (int16_t)( (buf[0]  << 8) | (buf[1]  & 0xFF) );
        data->pitch  = (int16_t)( (buf[2]  << 8) | (buf[3]  & 0xFF) );
        data->yaw    = (int16_t)( (buf[4]  << 8) | (buf[5]  & 0xFF) );
        data->ddx    = (int16_t)( (buf[6]  << 8) | (buf[7]  & 0xFF) );
        data->ddy    = (int16_t)( (buf[8]  << 8) | (buf[9]  & 0xFF) );
        data->ddz    = (int16_t)( (buf[10] << 8) | (buf[11] & 0xFF) );
        data->droll  = (int16_t)( (buf[12] << 8) | (buf[13] & 0xFF) );
        data->dpitch = (int16_t)( (buf[14] << 8) | (buf[15] & 0xFF) );
        data->dyaw   = (int16_t)( (buf[16] << 8) | (buf[17] & 0xFF) );
        data->ticks  = (int16_t)( (buf[18] << 8) | (buf[19] & 0xFF) );
        return( 0 );
    }

    fprintf( stderr, "ERROR in %s %d: invalid length (%d) of 3DM-GX1 data from stream\n",
        __FILE__, __LINE__, len );
    return( -1 );
}

/* End of file */
