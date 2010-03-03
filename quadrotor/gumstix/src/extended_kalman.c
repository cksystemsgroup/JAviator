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
#include <malloc.h>

#include "extended_kalman.h"

#define KALMAN_STATES   2       /* number of extended Kalman states */
#define KALMAN_P        4       /* elements in covariance matrix */
#define KALMAN_Q        10000.0
#define KALMAN_R        0.01

/* State of an extended Kalman filter */
struct ekf_state
{
    double dtime;
    double roll;
    double pitch;
    double yaw;
    double x;
    double y;
    double z;
    double dx;
    double dy;
    double dz;
};


static inline ekf_state_t *get_ekf_state( const extended_kalman_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes an extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int extended_kalman_init( extended_kalman_t *filter, char *name, int period )
{
    ekf_state_t *state;

    if( period < 1 )
    {
        fprintf( stderr, "ERROR: invalid %s filter period (%d)\n", name, period );
        return( -1 );
    }

    state = malloc( sizeof( ekf_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->dtime  = period / 1000.0; /* filter uses period in seconds */
    filter->name  = name;
    filter->state = state;
    return extended_kalman_reset( filter );
}

/* Destroys an extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int extended_kalman_destroy( extended_kalman_t *filter )
{
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets an extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int extended_kalman_reset( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->roll  = 0;
    state->pitch = 0;
    state->yaw   = 0;
    state->x     = 0;
    state->y     = 0;
    state->z     = 0;
    state->dx    = 0;
    state->dy    = 0;
    state->dz    = 0;
    return( 0 );
}

/* Updates an extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int extended_kalman_update( extended_kalman_t *filter, sensor_data_t *data )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    /* dummy assignments */
    state->roll  = data->roll;
    state->pitch = data->pitch;
    state->yaw   = data->yaw;
    state->x     = data->x;
    state->y     = data->y;
    state->z     = data->z;
    state->dx    = data->dx;
    state->dy    = data->dy;
    state->dz    = data->dz;
    return( 0 );
}

/* Returns the estimated Roll angle in [mrad] if successful, -1 otherwise.
*/
double extended_kalman_get_Roll( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->roll );
}

/* Returns the estimated Pitch angle in [mrad] if successful, -1 otherwise.
*/
double extended_kalman_get_Pitch( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->pitch );
}

/* Returns the estimated Yaw angle in [mrad] if successful, -1 otherwise.
*/
double extended_kalman_get_Yaw( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->yaw );
}

/* Returns the estimated X position in [mm] if successful, -1 otherwise.
*/
double extended_kalman_get_X( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->x );
}

/* Returns the estimated Y position in [mm] if successful, -1 otherwise.
*/
double extended_kalman_get_Y( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->y );
}

/* Returns the estimated Z position in [mm] if successful, -1 otherwise.
*/
double extended_kalman_get_Z( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->z );
}

/* Returns the estimated X velocity in [mm/s] if successful, -1 otherwise.
*/
double extended_kalman_get_dX( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->dx );
}

/* Returns the estimated Y velocity in [mm/s]] if successful, -1 otherwise.
*/
double extended_kalman_get_dY( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->dy );
}

/* Returns the estimated Z velocity in [mm/s]] if successful, -1 otherwise.
*/
double extended_kalman_get_dZ( extended_kalman_t *filter )
{
    ekf_state_t *state = get_ekf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->dz );
}

/* End of file */
