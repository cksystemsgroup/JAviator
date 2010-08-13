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

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>

/* Structure for representing sensor data */
typedef struct
{
    /* Euler angles
    */
    double  roll;       /* [mrad] */
    double  pitch;      /* [mrad] */
    double  yaw;        /* [mrad] */

    /* Angular velocities
    */
    double  droll;      /* [mrad/s] */
    double  dpitch;     /* [mrad/s] */
    double  dyaw;       /* [mrad/s] */

    /* Angular accelerations
    */
    double  ddroll;     /* [mrad/s^2] */
    double  ddpitch;    /* [mrad/s^2] */
    double  ddyaw;      /* [mrad/s^2] */

    /* Positions
    */
    double  x;          /* [mm] */
    double  y;          /* [mm] */
    double  z;          /* [mm] */

    /* Linear velocities
    */
    double  dx;         /* [mm/s] */
    double  dy;         /* [mm/s] */
    double  dz;         /* [mm/s] */

    /* Linear accelerations
    */
    double  ddx;        /* [mm/s^2] */
    double  ddy;        /* [mm/s^2] */
    double  ddz;        /* [mm/s^2] */

    /* BMU-specific data
    */
    double  maps;       /* [mm] */
    double  temp;       /* [cC] */
    double  batt;       /* [mV] */

} sensor_data_t;

#define SENSOR_DATA_SIZE    42  /* <int16_t> byte size of sensor_data_t */

static inline
int sensor_data_to_stream( const sensor_data_t *data, uint8_t *buf, int len )
{
    if( len == SENSOR_DATA_SIZE )
    {
        buf[0]  = (uint8_t)( (int16_t) data->roll >> 8 );
        buf[1]  = (uint8_t)( (int16_t) data->roll );
        buf[2]  = (uint8_t)( (int16_t) data->pitch >> 8 );
        buf[3]  = (uint8_t)( (int16_t) data->pitch );
        buf[4]  = (uint8_t)( (int16_t) data->yaw >> 8 );
        buf[5]  = (uint8_t)( (int16_t) data->yaw );
        buf[6]  = (uint8_t)( (int16_t) data->droll >> 8 );
        buf[7]  = (uint8_t)( (int16_t) data->droll );
        buf[8]  = (uint8_t)( (int16_t) data->dpitch >> 8 );
        buf[9]  = (uint8_t)( (int16_t) data->dpitch );
        buf[10] = (uint8_t)( (int16_t) data->dyaw >> 8 );
        buf[11] = (uint8_t)( (int16_t) data->dyaw );
        buf[12] = (uint8_t)( (int16_t) data->ddroll >> 8 );
        buf[13] = (uint8_t)( (int16_t) data->ddroll );
        buf[14] = (uint8_t)( (int16_t) data->ddpitch >> 8 );
        buf[15] = (uint8_t)( (int16_t) data->ddpitch );
        buf[16] = (uint8_t)( (int16_t) data->ddyaw >> 8 );
        buf[17] = (uint8_t)( (int16_t) data->ddyaw );
        buf[18] = (uint8_t)( (int16_t) data->x >> 8 );
        buf[19] = (uint8_t)( (int16_t) data->x );
        buf[20] = (uint8_t)( (int16_t) data->y >> 8 );
        buf[21] = (uint8_t)( (int16_t) data->y );
        buf[22] = (uint8_t)( (int16_t) data->z >> 8 );
        buf[23] = (uint8_t)( (int16_t) data->z );
        buf[24] = (uint8_t)( (int16_t) data->dx >> 8 );
        buf[25] = (uint8_t)( (int16_t) data->dx );
        buf[26] = (uint8_t)( (int16_t) data->dy >> 8 );
        buf[27] = (uint8_t)( (int16_t) data->dy );
        buf[28] = (uint8_t)( (int16_t) data->dz >> 8 );
        buf[29] = (uint8_t)( (int16_t) data->dz );
        buf[30] = (uint8_t)( (int16_t) data->ddx >> 8 );
        buf[31] = (uint8_t)( (int16_t) data->ddx );
        buf[32] = (uint8_t)( (int16_t) data->ddy >> 8 );
        buf[33] = (uint8_t)( (int16_t) data->ddy );
        buf[34] = (uint8_t)( (int16_t) data->ddz >> 8 );
        buf[35] = (uint8_t)( (int16_t) data->ddz );
        buf[36] = (uint8_t)( (int16_t) data->maps >> 8 );
        buf[37] = (uint8_t)( (int16_t) data->maps );
        buf[38] = (uint8_t)( (int16_t) data->temp >> 8 );
        buf[39] = (uint8_t)( (int16_t) data->temp );
        buf[40] = (uint8_t)( (int16_t) data->batt >> 8 );
        buf[41] = (uint8_t)( (int16_t) data->batt );
        return( 0 );
    }

    return( -1 );
}

static inline
int sensor_data_from_stream( sensor_data_t *data, const uint8_t *buf, int len )
{
    if( len == SENSOR_DATA_SIZE )
    {
        data->roll    = (int16_t)( (buf[0]  << 8) | (buf[1]  & 0xFF) );
        data->pitch   = (int16_t)( (buf[2]  << 8) | (buf[3]  & 0xFF) );
        data->yaw     = (int16_t)( (buf[4]  << 8) | (buf[5]  & 0xFF) );
        data->droll   = (int16_t)( (buf[6]  << 8) | (buf[7]  & 0xFF) );
        data->dpitch  = (int16_t)( (buf[8]  << 8) | (buf[9]  & 0xFF) );
        data->dyaw    = (int16_t)( (buf[10] << 8) | (buf[11] & 0xFF) );
        data->ddroll  = (int16_t)( (buf[12] << 8) | (buf[13] & 0xFF) );
        data->ddpitch = (int16_t)( (buf[14] << 8) | (buf[15] & 0xFF) );
        data->ddyaw   = (int16_t)( (buf[16] << 8) | (buf[17] & 0xFF) );
        data->x       = (int16_t)( (buf[18] << 8) | (buf[19] & 0xFF) );
        data->y       = (int16_t)( (buf[20] << 8) | (buf[21] & 0xFF) );
        data->z       = (int16_t)( (buf[22] << 8) | (buf[23] & 0xFF) );
        data->dx      = (int16_t)( (buf[24] << 8) | (buf[25] & 0xFF) );
        data->dy      = (int16_t)( (buf[26] << 8) | (buf[27] & 0xFF) );
        data->dz      = (int16_t)( (buf[28] << 8) | (buf[29] & 0xFF) );
        data->ddx     = (int16_t)( (buf[30] << 8) | (buf[31] & 0xFF) );
        data->ddy     = (int16_t)( (buf[32] << 8) | (buf[33] & 0xFF) );
        data->ddz     = (int16_t)( (buf[34] << 8) | (buf[35] & 0xFF) );
        data->maps    = (int16_t)( (buf[36] << 8) | (buf[37] & 0xFF) );
        data->temp    = (int16_t)( (buf[38] << 8) | (buf[39] & 0xFF) );
        data->batt    = (int16_t)( (buf[40] << 8) | (buf[41] & 0xFF) );
        return( 0 );
    }

    return( -1 );
}

#endif /* !SENSOR_DATA_H */

/* End of file */
