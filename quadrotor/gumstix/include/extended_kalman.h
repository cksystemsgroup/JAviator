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

struct ekf_state;
typedef struct ekf_state ekf_state_t;

/* This extended Kalman filter is designed to estimate
   attitude (Roll, Pitch, Yaw), position (X, Y, Z), and
   velocity (dX, dY, dZ) based on a fusion of attitude
   measurements with position measurements.
*/
typedef struct
{
    char *          name;
    ekf_state_t *   state;

} extended_kalman_t;

/* Initializes an extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int    extended_kalman_init( extended_kalman_t *filter, char *name, int period );

/* Destroys an extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int    extended_kalman_destroy( extended_kalman_t *filter );

/* Resets an extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int    extended_kalman_reset( extended_kalman_t *filter );

/* Updates an extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int    extended_kalman_update( extended_kalman_t *filter, sensor_data_t *data );

/* Returns the estimated Roll angle in [mrad] if successful, -1 otherwise.
*/
double extended_kalman_get_Roll( extended_kalman_t *filter );

/* Returns the estimated Pitch angle in [mrad] if successful, -1 otherwise.
*/
double extended_kalman_get_Pitch( extended_kalman_t *filter );

/* Returns the estimated Yaw angle in [mrad] if successful, -1 otherwise.
*/
double extended_kalman_get_Yaw( extended_kalman_t *filter );

/* Returns the estimated X position in [mm] if successful, -1 otherwise.
*/
double extended_kalman_get_X( extended_kalman_t *filter );

/* Returns the estimated Y position in [mm] if successful, -1 otherwise.
*/
double extended_kalman_get_Y( extended_kalman_t *filter );

/* Returns the estimated Z position in [mm] if successful, -1 otherwise.
*/
double extended_kalman_get_Z( extended_kalman_t *filter );

/* Returns the estimated X velocity in [mm/s] if successful, -1 otherwise.
*/
double extended_kalman_get_dX( extended_kalman_t *filter );

/* Returns the estimated Y velocity in [mm/s]] if successful, -1 otherwise.
*/
double extended_kalman_get_dY( extended_kalman_t *filter );

/* Returns the estimated Z velocity in [mm/s]] if successful, -1 otherwise.
*/
double extended_kalman_get_dZ( extended_kalman_t *filter );

#endif /* !EXTENDED_KALMAN */

/* End of file */
