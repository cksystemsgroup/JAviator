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

#ifndef INERTIAL_DATA_H
#define INERTIAL_DATA_H

#include <stdint.h>

/* Structure for representing inertial data */
typedef struct
{
    /* Euler angles
    */
    int16_t     roll;       /*                        | roll    |                       */
    int16_t     pitch;      /* [units] --> [mrad]   = | pitch   | * 2000 * PI / 65536   */
    int16_t     yaw;        /*                        | yaw     |                       */

    /* angular rates
    */
    int16_t     droll;      /*                        | droll   |                       */
    int16_t     dpitch;     /* [units] --> [mrad/s] = | dpitch  | * 8500 / 32768        */
    int16_t     dyaw;       /*                        | dyaw    |                       */

    /* linear accelerations
    */
    int16_t     ddx;        /*                        | ddx     |                       */
    int16_t     ddy;        /* [units] --> [mm/s^2] = | ddy     | * 9810 * 7 / 32768    */
    int16_t     ddz;        /*                        | ddz     |                       */

    /* timer ticks
    */
    int16_t     ticks;      /* [units] --> [s]      = | ticks   | * 65536 / 10000000    */

} inertial_data_t;

#define INERTIAL_DATA_SIZE  20  /* byte size of inertial_data_t */
#define DM3_GX1_DATA_SIZE   23  /* byte size of 3DM-GX1 data stream */

static inline
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

    return( -1 );
}

static inline
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

    return( -1 );
}

#endif /* !INERTIAL_DATA_H */

/* End of file */
