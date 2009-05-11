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
    int16_t roll;       /* [mrad] */
    int16_t pitch;      /* [mrad] */
    int16_t yaw;        /* [mrad] */

    /* angular rates
    */
    int16_t droll;      /* [mrad/s] */
    int16_t dpitch;     /* [mrad/s] */
    int16_t dyaw;       /* [mrad/s] */

    /* angular accelerations
    */
    int16_t ddroll;     /* [mrad/s^2] */
    int16_t ddpitch;    /* [mrad/s^2] */
    int16_t ddyaw;      /* [mrad/s^2] */

    /* positions
    */
    int16_t x;          /* [mm] */
    int16_t y;          /* [mm] */
    int16_t z;          /* [mm] */

    /* linear rates
    */
    int16_t dx;         /* [mm/s] */
    int16_t dy;         /* [mm/s] */
    int16_t dz;         /* [mm/s] */

    /* linear accelerations
    */
    int16_t ddx;        /* [mm/s^2] */
    int16_t ddy;        /* [mm/s^2] */
    int16_t ddz;        /* [mm/s^2] */

    /* battery level
    */
    int16_t battery;    /* [mV] */

} sensor_data_t;

#define SENSOR_DATA_SIZE    38  /* byte size of sensor_data_t */


int sensor_data_to_stream( const sensor_data_t *data, char *buf, int len );

int sensor_data_from_stream( sensor_data_t *data, const char *buf, int len );


#endif /* !SENSOR_DATA_H */

/* End of file */
