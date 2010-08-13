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

#ifndef TRACE_DATA_H
#define TRACE_DATA_H

#include <stdint.h>

/* Structure for representing trace data */
typedef struct
{
    int16_t value_1;
    int16_t value_2;
    int16_t value_3;
    int16_t value_4;
    int16_t value_5;
    int16_t value_6;
    int16_t value_7;
    int16_t value_8;
    int16_t value_9;
    int16_t value_10;
    int16_t value_11;
    int16_t value_12;
    int16_t value_13;
    int16_t value_14;
    int16_t value_15;
    int16_t value_16;

} trace_data_t;

#define TRACE_DATA_SIZE 32  /* byte size of trace_data_t */

static inline
int trace_data_to_stream( const trace_data_t *data, char *buf, int len )
{
    if( len == TRACE_DATA_SIZE )
    {
        buf[0]  = (char)( data->value_1 >> 8 );
        buf[1]  = (char)( data->value_1 );
        buf[2]  = (char)( data->value_2 >> 8 );
        buf[3]  = (char)( data->value_2 );
        buf[4]  = (char)( data->value_3 >> 8 );
        buf[5]  = (char)( data->value_3 );
        buf[6]  = (char)( data->value_4 >> 8 );
        buf[7]  = (char)( data->value_4 );
        buf[8]  = (char)( data->value_5 >> 8 );
        buf[9]  = (char)( data->value_5 );
        buf[10] = (char)( data->value_6 >> 8 );
        buf[11] = (char)( data->value_6 );
        buf[12] = (char)( data->value_7 >> 8 );
        buf[13] = (char)( data->value_7 );
        buf[14] = (char)( data->value_8 >> 8 );
        buf[15] = (char)( data->value_8 );
        buf[16] = (char)( data->value_9 >> 8 );
        buf[17] = (char)( data->value_9 );
        buf[18] = (char)( data->value_10 >> 8 );
        buf[19] = (char)( data->value_10 );
        buf[20] = (char)( data->value_11 >> 8 );
        buf[21] = (char)( data->value_11 );
        buf[22] = (char)( data->value_12 >> 8 );
        buf[23] = (char)( data->value_12 );
        buf[24] = (char)( data->value_13 >> 8 );
        buf[25] = (char)( data->value_13 );
        buf[26] = (char)( data->value_14 >> 8 );
        buf[27] = (char)( data->value_14 );
        buf[28] = (char)( data->value_15 >> 8 );
        buf[29] = (char)( data->value_15 );
        buf[30] = (char)( data->value_16 >> 8 );
        buf[31] = (char)( data->value_16 );
        return( 0 );
    }

    return( -1 );
}

static inline
int trace_data_from_stream( trace_data_t *data, const char *buf, int len )
{
    if( len == TRACE_DATA_SIZE )
    {
        data->value_1  = (int16_t)( (buf[0]  << 8) | (buf[1]  & 0xFF) );
        data->value_2  = (int16_t)( (buf[2]  << 8) | (buf[3]  & 0xFF) );
        data->value_3  = (int16_t)( (buf[4]  << 8) | (buf[5]  & 0xFF) );
        data->value_4  = (int16_t)( (buf[6]  << 8) | (buf[7]  & 0xFF) );
        data->value_5  = (int16_t)( (buf[8]  << 8) | (buf[9]  & 0xFF) );
        data->value_6  = (int16_t)( (buf[10] << 8) | (buf[11] & 0xFF) );
        data->value_7  = (int16_t)( (buf[12] << 8) | (buf[13] & 0xFF) );
        data->value_8  = (int16_t)( (buf[14] << 8) | (buf[15] & 0xFF) );
        data->value_9  = (int16_t)( (buf[16] << 8) | (buf[17] & 0xFF) );
        data->value_10 = (int16_t)( (buf[18] << 8) | (buf[19] & 0xFF) );
        data->value_11 = (int16_t)( (buf[20] << 8) | (buf[21] & 0xFF) );
        data->value_12 = (int16_t)( (buf[22] << 8) | (buf[23] & 0xFF) );
        data->value_13 = (int16_t)( (buf[24] << 8) | (buf[25] & 0xFF) );
        data->value_14 = (int16_t)( (buf[26] << 8) | (buf[27] & 0xFF) );
        data->value_15 = (int16_t)( (buf[28] << 8) | (buf[29] & 0xFF) );
        data->value_16 = (int16_t)( (buf[30] << 8) | (buf[31] & 0xFF) );
        return( 0 );
    }

    return( -1 );
}

#endif /* !TRACE_DATA_H */

/* End of file */
