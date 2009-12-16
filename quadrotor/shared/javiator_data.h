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

#ifndef JAVIATOR_DATA_H
#define JAVIATOR_DATA_H

#include <stdint.h>


/* Structure for shared JAviator data */
typedef struct
{
    uint32_t    pressure;           /* [kPa] 24-bit resolution */
    uint16_t    sonar;              /* [mV] 10-bit resolution */
    uint16_t    battery;            /* [mV] 10-bit resolution */
    uint16_t    state;              /* JAviator state */
    uint16_t    id;                 /* transmisson ID */
    uint8_t     x_pos[8];           /* [mm] */
    uint8_t     y_pos[8];           /* [mm] */

} javiator_data_t;

#define JAVIATOR_DATA_SIZE  28      /* byte size of javiator_data_t */


static inline
int javiator_data_to_stream( const javiator_data_t *data, uint8_t *buf, int len )
{
    if( len == JAVIATOR_DATA_SIZE )
    {
        buf[0]  = (uint8_t)( data->pressure >> 24 );
        buf[1]  = (uint8_t)( data->pressure >> 16 );
        buf[2]  = (uint8_t)( data->pressure >> 8 );
        buf[3]  = (uint8_t)( data->pressure );

        buf[4]  = (uint8_t)( data->sonar >> 8 );
        buf[5]  = (uint8_t)( data->sonar );

        buf[6]  = (uint8_t)( data->battery >> 8 );
        buf[7]  = (uint8_t)( data->battery );

        buf[8]  = (uint8_t)( data->state >> 8 );
        buf[9]  = (uint8_t)( data->state );

        buf[10] = (uint8_t)( data->id >> 8 );
        buf[11] = (uint8_t)( data->id );

        buf[12] = (uint8_t)( data->x_pos[0] );
        buf[13] = (uint8_t)( data->x_pos[1] );
        buf[14] = (uint8_t)( data->x_pos[2] );
        buf[15] = (uint8_t)( data->x_pos[3] );
        buf[16] = (uint8_t)( data->x_pos[4] );
        buf[17] = (uint8_t)( data->x_pos[5] );
        buf[18] = (uint8_t)( data->x_pos[6] );
        buf[19] = (uint8_t)( data->x_pos[7] );

        buf[20] = (uint8_t)( data->y_pos[0] );
        buf[21] = (uint8_t)( data->y_pos[1] );
        buf[22] = (uint8_t)( data->y_pos[2] );
        buf[23] = (uint8_t)( data->y_pos[3] );
        buf[24] = (uint8_t)( data->y_pos[4] );
        buf[25] = (uint8_t)( data->y_pos[5] );
        buf[26] = (uint8_t)( data->y_pos[6] );
        buf[27] = (uint8_t)( data->y_pos[7] );

        return( 0 );
    }

    return( -1 );
}

static inline
int javiator_data_from_stream( javiator_data_t *data, const uint8_t *buf, int len )
{
    if( len == JAVIATOR_DATA_SIZE )
    {
		data->pressure   = buf[0];
		data->pressure <<= 8;
		data->pressure  |= buf[1];
		data->pressure <<= 8;
		data->pressure  |= buf[2];
		data->pressure <<= 8;
		data->pressure  |= buf[3];

		data->sonar      = buf[4];
		data->sonar    <<= 8;
		data->sonar     |= buf[5];

		data->battery    = buf[6];
		data->battery  <<= 8;
		data->battery   |= buf[7];

		data->state      = buf[8];
		data->state    <<= 8;
		data->state     |= buf[9];

		data->id         = buf[10];
		data->id       <<= 8;
		data->id        |= buf[11];

        data->x_pos[0]   = buf[12];
        data->x_pos[1]   = buf[13];
        data->x_pos[2]   = buf[14];
        data->x_pos[3]   = buf[15];
        data->x_pos[4]   = buf[16];
        data->x_pos[5]   = buf[17];
        data->x_pos[6]   = buf[18];
        data->x_pos[7]   = buf[19];

        data->y_pos[0]   = buf[20];
        data->y_pos[1]   = buf[21];
        data->y_pos[2]   = buf[22];
        data->y_pos[3]   = buf[23];
        data->y_pos[4]   = buf[24];
        data->y_pos[5]   = buf[25];
        data->y_pos[6]   = buf[26];
        data->y_pos[7]   = buf[27];
		
        return( 0 );
    }

    return( -1 );
}


#endif /* !JAVIATOR_DATA_H */

/* End of file */
