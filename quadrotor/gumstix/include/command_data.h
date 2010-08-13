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

#ifndef COMMAND_DATA_H
#define COMMAND_DATA_H

#include <stdint.h>

/* Structure for representing command data */
typedef struct
{
    double  roll;
    double  pitch;
    double  yaw;
    double  z;

} command_data_t;

#define COMMAND_DATA_SIZE   8   /* <int16_t> byte size of command_data_t */

static inline
int command_data_to_stream( const command_data_t *data, uint8_t *buf, int len )
{
    if( len == COMMAND_DATA_SIZE )
    {
        buf[0] = (uint8_t)( (int16_t) data->roll >> 8 );
        buf[1] = (uint8_t)( (int16_t) data->roll );
        buf[2] = (uint8_t)( (int16_t) data->pitch >> 8 );
        buf[3] = (uint8_t)( (int16_t) data->pitch );
        buf[4] = (uint8_t)( (int16_t) data->yaw >> 8 );
        buf[5] = (uint8_t)( (int16_t) data->yaw );
        buf[6] = (uint8_t)( (int16_t) data->z >> 8 );
        buf[7] = (uint8_t)( (int16_t) data->z );
        return( 0 );
    }

    return( -1 );
}

static inline
int command_data_from_stream( command_data_t *data, const uint8_t *buf, int len )
{
    if( len == COMMAND_DATA_SIZE )
    {
        data->roll  = (int16_t)( (buf[0] << 8) | (buf[1] & 0xFF) );
        data->pitch = (int16_t)( (buf[2] << 8) | (buf[3] & 0xFF) );
        data->yaw   = (int16_t)( (buf[4] << 8) | (buf[5] & 0xFF) );
        data->z     = (int16_t)( (buf[6] << 8) | (buf[7] & 0xFF) );
        return( 0 );
    }

    return( -1 );
}

#endif /* !COMMAND_DATA_H */

/* End of file */
