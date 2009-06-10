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

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>


/* Structure for representing sensor data */
typedef struct
{
    /* Euler angles
    */
    double roll;        /* [rad] */
    double pitch;       /* [rad] */
    double yaw;         /* [rad] */

    /* angular rates
    */
    double droll;       /* [rad/s] */
    double dpitch;      /* [rad/s] */
    double dyaw;        /* [rad/s] */

    /* angular accelerations
    */
    double ddroll;      /* [rad/s^2] */
    double ddpitch;     /* [rad/s^2] */
    double ddyaw;       /* [rad/s^2] */

    /* positions
    */
    double x;           /* [m] */
    double y;           /* [m] */
    double z;           /* [m] */

    /* linear rates
    */
    double dx;          /* [m/s] */
    double dy;          /* [m/s] */
    double dz;          /* [m/s] */

    /* linear accelerations
    */
    double ddx;         /* [m/s^2] */
    double ddy;         /* [m/s^2] */
    double ddz;         /* [m/s^2] */

    /* battery level
    */
    double battery;     /* [V] */

} sensor_data_t;

#define SENSOR_DATA_SIZE    38      /* <int16_t> byte size of sensor_data_t */

#define FACTOR_1000         1000.0  /* [rad] to [mrad], [m] to [mm], [V] to [mV] */
#define FACTOR_0001         0.001   /* [mrad] to [rad], [mm] to [m], [mV] to [V] */


static inline
int sensor_data_to_stream( const sensor_data_t *data, uint8_t *buf, int len )
{
    int16_t tmp;

    if( len == SENSOR_DATA_SIZE )
    {
        tmp     = (int16_t)( FACTOR_1000 * data->roll );
        buf[0]  = (uint8_t)( tmp >> 8 );
        buf[1]  = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->pitch );
        buf[2]  = (uint8_t)( tmp >> 8 );
        buf[3]  = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->yaw );
        buf[4]  = (uint8_t)( tmp >> 8 );
        buf[5]  = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->droll );
        buf[6]  = (uint8_t)( tmp >> 8 );
        buf[7]  = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->dpitch );
        buf[8]  = (uint8_t)( tmp >> 8 );
        buf[9]  = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->dyaw );
        buf[10] = (uint8_t)( tmp >> 8 );
        buf[11] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->ddroll );
        buf[12] = (uint8_t)( tmp >> 8 );
        buf[13] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->ddpitch );
        buf[14] = (uint8_t)( tmp >> 8 );
        buf[15] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->ddyaw );
        buf[16] = (uint8_t)( tmp >> 8 );
        buf[17] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->x );
        buf[18] = (uint8_t)( tmp >> 8 );
        buf[19] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->y );
        buf[20] = (uint8_t)( tmp >> 8 );
        buf[21] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->z );
        buf[22] = (uint8_t)( tmp >> 8 );
        buf[23] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->dx );
        buf[24] = (uint8_t)( tmp >> 8 );
        buf[25] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->dy );
        buf[26] = (uint8_t)( tmp >> 8 );
        buf[27] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->dz );
        buf[28] = (uint8_t)( tmp >> 8 );
        buf[29] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->ddx );
        buf[30] = (uint8_t)( tmp >> 8 );
        buf[31] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->ddy );
        buf[32] = (uint8_t)( tmp >> 8 );
        buf[33] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->ddz );
        buf[34] = (uint8_t)( tmp >> 8 );
        buf[35] = (uint8_t)( tmp );
        tmp     = (int16_t)( FACTOR_1000 * data->battery );
        buf[36] = (uint8_t)( tmp >> 8 );
        buf[37] = (uint8_t)( tmp );
        return( 0 );
    }

    return( -1 );
}

static inline
int sensor_data_from_stream( sensor_data_t *data, const uint8_t *buf, int len )
{
    if( len == SENSOR_DATA_SIZE )
    {
        data->roll    = (int16_t)( (buf[0]  << 8) | (buf[1]  & 0xFF) ) * FACTOR_0001;
        data->pitch   = (int16_t)( (buf[2]  << 8) | (buf[3]  & 0xFF) ) * FACTOR_0001;
        data->yaw     = (int16_t)( (buf[4]  << 8) | (buf[5]  & 0xFF) ) * FACTOR_0001;
        data->droll   = (int16_t)( (buf[6]  << 8) | (buf[7]  & 0xFF) ) * FACTOR_0001;
        data->dpitch  = (int16_t)( (buf[8]  << 8) | (buf[9]  & 0xFF) ) * FACTOR_0001;
        data->dyaw    = (int16_t)( (buf[10] << 8) | (buf[11] & 0xFF) ) * FACTOR_0001;
        data->ddroll  = (int16_t)( (buf[12] << 8) | (buf[13] & 0xFF) ) * FACTOR_0001;
        data->ddpitch = (int16_t)( (buf[14] << 8) | (buf[15] & 0xFF) ) * FACTOR_0001;
        data->ddyaw   = (int16_t)( (buf[16] << 8) | (buf[17] & 0xFF) ) * FACTOR_0001;
        data->x       = (int16_t)( (buf[18] << 8) | (buf[19] & 0xFF) ) * FACTOR_0001;
        data->y       = (int16_t)( (buf[20] << 8) | (buf[21] & 0xFF) ) * FACTOR_0001;
        data->z       = (int16_t)( (buf[22] << 8) | (buf[23] & 0xFF) ) * FACTOR_0001;
        data->dx      = (int16_t)( (buf[24] << 8) | (buf[25] & 0xFF) ) * FACTOR_0001;
        data->dy      = (int16_t)( (buf[26] << 8) | (buf[27] & 0xFF) ) * FACTOR_0001;
        data->dz      = (int16_t)( (buf[28] << 8) | (buf[29] & 0xFF) ) * FACTOR_0001;
        data->ddx     = (int16_t)( (buf[30] << 8) | (buf[31] & 0xFF) ) * FACTOR_0001;
        data->ddy     = (int16_t)( (buf[32] << 8) | (buf[33] & 0xFF) ) * FACTOR_0001;
        data->ddz     = (int16_t)( (buf[34] << 8) | (buf[35] & 0xFF) ) * FACTOR_0001;
        data->battery = (int16_t)( (buf[36] << 8) | (buf[37] & 0xFF) ) * FACTOR_0001;
        return( 0 );
    }

    return( -1 );
}


#endif /* !SENSOR_DATA_H */

/* End of file */
