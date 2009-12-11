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


int inertial_data_to_stream( const inertial_data_t *data, char *buf, int len );

int inertial_data_from_stream( inertial_data_t *data, const char *buf, int len );


#endif /* !INERTIAL_DATA_H */

/* End of file */
