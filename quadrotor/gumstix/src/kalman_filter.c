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

#include <stdio.h>
#include <malloc.h>

#include "kalman_filter.h"

/* State of a Kalman filter */
struct kf_state
{
    double q;
    double r;
    double dt;
    double x[2];
    double p[2][2];
};


static inline kf_state_t *get_kf_state( const kalman_filter_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes a Kalman filter with the given period.
   Parameter <period> is expected to be given in [s].
   Returns 0 if successful, -1 otherwise.
*/
int kalman_filter_init( kalman_filter_t *filter,
    char *name, double proc_noise, double data_noise, double period )
{
    kf_state_t *state;

    if( period < 0.001 )
    {
        fprintf( stderr, "ERROR: invalid %s filter period (%f)\n", name, period );
        return( -1 );
    }

    state = malloc( sizeof( kf_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->q      = proc_noise;
    state->r      = data_noise;
    state->dt     = period;
    filter->name  = name;
    filter->state = state;
    return kalman_filter_reset( filter );
}

/* Destroys a Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int kalman_filter_destroy( kalman_filter_t *filter )
{
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets a Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int kalman_filter_reset( kalman_filter_t *filter )
{
    kf_state_t *state = get_kf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->x[0]    = 0;
    state->x[1]    = 0;
    state->p[0][0] = 0;
    state->p[0][1] = 0;
    state->p[1][0] = 0;
    state->p[1][1] = state->q;
    return( 0 );
}

/* Filters the given distance and estimates the missing velocity ds.
   Parameter <s> is expected to be given in [mm] and <dds> in [mm/s^2].
   Returns 0 if successful, -1 otherwise.
*/
int kalman_filter_update( kalman_filter_t *filter, double s, double dds )
{
    kf_state_t *state = get_kf_state( filter );
    double k[2];

    if( !state )
    {
        return( -1 );
    }

    /* project state estimate ahead */
    state->x[0] += state->x[1] * state->dt;
    state->x[1] += dds * state->dt;

    /* project error covariance ahead */
    state->p[0][0] += (state->p[1][0] + state->p[0][1]) * state->dt
        + state->p[1][1] * state->dt * state->dt;
    state->p[0][1] += state->p[1][1] * state->dt;
    state->p[1][0] += state->p[1][1] * state->dt;
    state->p[1][1] += state->q * state->dt * state->dt;

    /* compute optimal Kalman gain */
    k[0] = state->p[0][0] / (state->p[0][0] + state->r);
    k[1] = state->p[1][0] / (state->p[0][0] + state->r);

    /* update state estimate */
    state->x[1] += k[1] * (s - state->x[0]);
    state->x[0] += k[0] * (s - state->x[0]);

    /* update error covariance */
    state->p[1][1] -= k[1] * state->p[0][1] ;
    state->p[1][0] -= k[1] * state->p[0][0];
    state->p[0][1]  = (1 - k[0]) * state->p[0][1];
    state->p[0][0]  = (1 - k[0]) * state->p[0][0];

    return( 0 );
}

/* Returns the filtered distance s in [mm] if successful, -1 otherwise.
*/
double kalman_filter_get_S( kalman_filter_t *filter )
{
    kf_state_t *state = get_kf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->x[0] );
}

/* Returns the estimated velocity ds in [mm/s] if successful, -1 otherwise.
*/
double kalman_filter_get_dS( kalman_filter_t *filter )
{
    kf_state_t *state = get_kf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->x[1] );
}

/* End of file */
