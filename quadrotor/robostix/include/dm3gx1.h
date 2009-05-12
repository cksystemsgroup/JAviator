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
    uint16_t    roll;       /*                        | roll    |                       */
    uint16_t    pitch;      /* [units] --> [mrad]   = | pitch   | * 2000 * PI / 65536   */
    uint16_t    yaw;        /*                        | yaw     |                       */

    /* angular rates
    */
    uint16_t    droll;      /*                        | droll   |                       */
    uint16_t    dpitch;     /* [units] --> [mrad/s] = | dpitch  | * 8500 / 32768        */
    uint16_t    dyaw;       /*                        | dyaw    |                       */

    /* linear accelerations
    */
    uint16_t    ddx;        /*                        | ddx     |                       */
    uint16_t    ddy;        /* [units] --> [mm/s^2] = | ddy     | * 9810 * 7 / 32768    */
    uint16_t    ddz;        /*                        | ddz     |                       */

    /* timer ticks
    */
    uint16_t    ticks;      /* [units] --> [s]      = | ticks   | * 65536 / 10000000    */

} dm3gx1_data_t;

#define DM3_GX1_DATA_SIZE   20  /* byte size of dm3gx1_data_t */


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
