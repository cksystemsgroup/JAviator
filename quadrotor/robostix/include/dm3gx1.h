/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   dm3gx1.h   Interface for the MicroStrain 3DM-GX1 IMU sensor device.     */
/*                                                                           */
/*   Copyright (c) Rainer Trummer rtrummer@cs.uni-salzburg.at                */
/*                                                                           */
/*   This program is free software; you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by    */
/*   the Free Software Foundation; either version 2 of the License, or       */
/*   (at your option) any later version.                                     */
/*                                                                           */
/*   This program is distributed in the hope that it will be useful,         */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*   GNU General Public License for more details.                            */
/*                                                                           */
/*   You should have received a copy of the GNU General Public License       */
/*   along with this program; if not, write to the Free Software Foundation, */
/*   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.      */
/*                                                                           */
/*****************************************************************************/

#ifndef DM3GX1_H
#define DM3GX1_H

#include <stdint.h>


/*****************************************************************************/
/*                                                                           */
/*   Public Definitions                                                      */
/*                                                                           */
/*****************************************************************************/

/* Structure for representing IMU data */
typedef struct
{
    /* Euler angles
    */
    int16_t     roll;       /*              | roll  |                               */
    int16_t     pitch;      /*      [deg] = | pitch | * 360 / 65536                 */
    int16_t     yaw;        /*              | yaw   |                               */

    /* angular rates
    */
    int16_t     droll;      /*              | droll  |                              */
    int16_t     dpitch;     /*    [rad/s] = | dpitch | / (32768000 / 8500)          */
    int16_t     dyaw;       /*              | dyaw   |                              */

    /* angular accelerations
    */
    int16_t     ddroll;     /*              | droll_{k}  - droll_{k-1}  |           */
    int16_t     ddpitch;    /*  [rad/s^2] = | dpitch_{k} - dpitch_{k-1} | / period  */
    int16_t     ddyaw;      /*              | dyaw_{k}   - dyaw_{k-1}   |           */

    /* linear accelerations
    */
    int16_t     ddx;        /*              | ddx |                                 */
    int16_t     ddy;        /*        [G] = | ddy | / (32768000 / 7000)             */
    int16_t     ddz;        /*              | ddz |                                 */

} dm3gx1_data_t;


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the selected UART channel for the IMU sensor
*/
void    dm3gx1_init( void );

/* Sends a command requesting a new data packet
*/
void    dm3gx1_request( void );

/* Starts the IMU sensor in continuous mode
*/
void    dm3gx1_start( void );

/* Stops the IMU sensor if in continuous mode
*/
void    dm3gx1_stop( void );

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t dm3gx1_is_new_data( void );

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  dm3gx1_get_data( dm3gx1_data_t *buf );

/* Sets the baudrate to 115200bps
   (for IMU configuration only)
*/
void    dm3gx1_set_max_baudrate( void );

/* Sets the sampling rate to 100Hz
   (for IMU configuration only)
*/
void    dm3gx1_set_max_sampling( void );


#endif /* !DM3GX1_H */

/* End of file */
