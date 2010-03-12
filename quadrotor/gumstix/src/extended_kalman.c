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

#include <stdio.h>
#include <string.h>

#include "extended_kalman.h"

static sensor_data_t    ekf_data;
static double           dtime;


/* Initializes the extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int extended_kalman_init( int period )
{
    if( period < 1 )
    {
        fprintf( stderr, "ERROR: invalid EKF period (%d)\n", period );
        return( -1 );
    }

    dtime = period / 1000.0; /* filter uses period in seconds */
    extended_kalman_reset( );
    return( 0 );
}

/* Resets the extended Kalman filter
*/
void extended_kalman_reset( void )
{
    memset( &ekf_data, 0, sizeof( ekf_data ) );
}

/* Updates the extended Kalman filter
*/
void extended_kalman_update( sensor_data_t *data )
{
    ekf_data.roll    = data->roll;
    ekf_data.pitch   = data->pitch;
    ekf_data.yaw     = data->yaw;
    ekf_data.droll   = data->droll;
    ekf_data.dpitch  = data->dpitch;
    ekf_data.dyaw    = data->dyaw;
    ekf_data.ddroll  = data->ddroll;
    ekf_data.ddpitch = data->ddpitch;
    ekf_data.ddyaw   = data->ddyaw;
    ekf_data.x       = data->x;
    ekf_data.y       = data->y;
    ekf_data.z       = data->z;
    ekf_data.dx      = data->dx;
    ekf_data.dy      = data->dy;
    ekf_data.dz      = data->dz;
    ekf_data.ddx     = data->dx;
    ekf_data.ddy     = data->dy;
    ekf_data.ddz     = data->dz;
}

/* Returns the estimated Roll angle in [mrad]
*/
double extended_kalman_get_Roll( void )
{
    return( ekf_data.roll );
}

/* Returns the estimated Pitch angle in [mrad]
*/
double extended_kalman_get_Pitch( void )
{
    return( ekf_data.pitch );
}

/* Returns the estimated Yaw angle in [mrad]
*/
double extended_kalman_get_Yaw( void )
{
    return( ekf_data.yaw );
}

/* Returns the estimated Roll velocity in [mrad/s]
*/
double extended_kalman_get_dRoll( void )
{
    return( ekf_data.droll );
}

/* Returns the estimated Pitch velocity in [mrad/s]
*/
double extended_kalman_get_dPitch( void )
{
    return( ekf_data.dpitch );
}

/* Returns the estimated Yaw velocity in [mrad/s]
*/
double extended_kalman_get_dYaw( void )
{
    return( ekf_data.dyaw );
}

/* Returns the estimated X position in [mm]
*/
double extended_kalman_get_X( void )
{
    return( ekf_data.x );
}

/* Returns the estimated Y position in [mm]
*/
double extended_kalman_get_Y( void )
{
    return( ekf_data.y );
}

/* Returns the estimated Z position in [mm]
*/
double extended_kalman_get_Z( void )
{
    return( ekf_data.z );
}

/* Returns the estimated X velocity in [mm/s]
*/
double extended_kalman_get_dX( void )
{
    return( ekf_data.dx );
}

/* Returns the estimated Y velocity in [mm/s]
*/
double extended_kalman_get_dY( void )
{
    return( ekf_data.dy );
}

/* Returns the estimated Z velocity in [mm/s]
*/
double extended_kalman_get_dZ( void )
{
    return( ekf_data.dz );
}

/* End of file */
