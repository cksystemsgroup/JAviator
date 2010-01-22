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
#include "javiator.h"


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
int8_t  dm3gx1_get_data( javiator_ldat_t *buf );

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
