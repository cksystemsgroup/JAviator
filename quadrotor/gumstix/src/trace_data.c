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

#include "trace_data.h"


int trace_data_to_stream( const trace_data_t *data, char *buf, int len )
{
    if( len == TRACE_DATA_SIZE )
    {
        buf[0]  = (char)( data->z >> 8 );
        buf[1]  = (char)( data->z );
        buf[2]  = (char)( data->z_filtered >> 8 );
        buf[3]  = (char)( data->z_filtered );
        buf[4]  = (char)( data->z_estimated >> 8 );
        buf[5]  = (char)( data->z_estimated );
        buf[6]  = (char)( data->dz_estimated >> 8 );
        buf[7]  = (char)( data->dz_estimated );
        buf[8]  = (char)( data->ddz >> 8 );
        buf[9]  = (char)( data->ddz );
        buf[10] = (char)( data->ddz_filtered >> 8 );
        buf[11] = (char)( data->ddz_filtered );
        buf[12] = (char)( data->uz >> 8 );
        buf[13] = (char)( data->uz );
        buf[14] = (char)( data->z_cmd >> 8 );
        buf[15] = (char)( data->z_cmd );
        return( 0 );
    }

    fprintf( stderr, "ERROR in %s %d: invalid length (%d) of trace data to stream\n",
        __FILE__, __LINE__, len );
    return( -1 );
}

int trace_data_from_stream( trace_data_t *data, const char *buf, int len )
{
    if( len == TRACE_DATA_SIZE )
    {
        data->z            = (int16_t)( (buf[0]  << 8) | (buf[1]  & 0xFF) );
        data->z_filtered   = (int16_t)( (buf[2]  << 8) | (buf[3]  & 0xFF) );
        data->z_estimated  = (int16_t)( (buf[4]  << 8) | (buf[5]  & 0xFF) );
        data->dz_estimated = (int16_t)( (buf[6]  << 8) | (buf[7]  & 0xFF) );
        data->ddz          = (int16_t)( (buf[8]  << 8) | (buf[9]  & 0xFF) );
        data->ddz_filtered = (int16_t)( (buf[10] << 8) | (buf[11] & 0xFF) );
        data->uz           = (int16_t)( (buf[12] << 8) | (buf[13] & 0xFF) );
        data->z_cmd        = (int16_t)( (buf[14] << 8) | (buf[15] & 0xFF) );
        return( 0 );
    }

    fprintf( stderr, "ERROR in %s %d: invalid length (%d) of trace data from stream\n",
        __FILE__, __LINE__, len );
    return( -1 );
}

/* End of file */
