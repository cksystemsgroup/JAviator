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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef EXTENDED_KALMAN
#define EXTENDED_KALMAN

#include "sensor_data.h"

/* This extended Kalman filter is designed to estimate
   attitude (Roll, Pitch, Yaw), position (X, Y, Z), and
   velocity (dRoll, dPitch, dYaw, dX, dY, dZ) by fusing
   attitude measurements with position measurements.
*/

/* Initializes the extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int    extended_kalman_init( int period );

/* Resets the extended Kalman filter
*/
void   extended_kalman_reset( void );

/* Updates the extended Kalman filter
*/
void   extended_kalman_update( sensor_data_t *data );

/* Returns the estimated Roll angle in [mrad]
*/
double extended_kalman_get_Roll( void );

/* Returns the estimated Pitch angle in [mrad]
*/
double extended_kalman_get_Pitch( void );

/* Returns the estimated Yaw angle in [mrad]
*/
double extended_kalman_get_Yaw( void );

/* Returns the estimated Roll velocity in [mrad/s]
*/
double extended_kalman_get_dRoll( void );

/* Returns the estimated Pitch velocity in [mrad/s]
*/
double extended_kalman_get_dPitch( void );

/* Returns the estimated Yaw velocity in [mrad/s]
*/
double extended_kalman_get_dYaw( void );

/* Returns the estimated X position in [mm]
*/
double extended_kalman_get_X( void );

/* Returns the estimated Y position in [mm]
*/
double extended_kalman_get_Y( void );

/* Returns the estimated Z position in [mm]
*/
double extended_kalman_get_Z( void );

/* Returns the estimated X velocity in [mm/s]
*/
double extended_kalman_get_dX( void );

/* Returns the estimated Y velocity in [mm/s]
*/
double extended_kalman_get_dY( void );

/* Returns the estimated Z velocity in [mm/s]
*/
double extended_kalman_get_dZ( void );

#endif /* !EXTENDED_KALMAN */

/* End of file */
