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

#define KALMAN_STATES   2       /* number of Kalman states */
#define KALMAN_P        4       /* elements in covariance matrix */
#define KALMAN_Q        10000.0
#define KALMAN_R        0.01

/* State of a Kalman filter */
struct kf_state
{
    double dtime;
    double x[ KALMAN_STATES ];
    double p[ KALMAN_P ];
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
   Parameter <period> is expected to be given in [ms].
   Returns 0 if successful, -1 otherwise.
*/
int kalman_filter_init( kalman_filter_t *filter, char *name, int period )
{
    kf_state_t *state;

    if( period < 1 )
    {
        fprintf( stderr, "ERROR: invalid %s filter period (%d)\n", name, period );
        return( -1 );
    }

    state = malloc( sizeof( kf_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->dtime  = period / 1000.0; /* filter uses period in seconds */
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

    state->x[0] = 0;
    state->x[1] = 0;
    state->p[0] = 0;
    state->p[1] = 0;
    state->p[2] = 0;
    state->p[3] = 0;
    return( 0 );
}

/* Filters the given distance and estimates the missing velocity ds.
   Parameter <s> is expected to be given in [mm] and <dds> in [mm/s^2].
   Returns 0 if successful, -1 otherwise.
*/
int kalman_filter_update( kalman_filter_t *filter, double s, double dds )
{
    kf_state_t *state = get_kf_state( filter );
    double x1, x2, p11, p12, p21, p22, k1, k2;

    if( !state )
    {
        return( -1 );
    }

    if( s > 0 )
    {
        /* update local variables */
        x1  = state->x[0];
        x2  = state->x[1];
        p11 = state->p[0];
        p12 = state->p[1];
        p21 = state->p[2];
        p22 = state->p[3];

        /* TIME UPDATE */

        /* project state ahead */
        x1 = x1 + state->dtime * x2;
        x2 = x2 + state->dtime * dds;

        /* project error covariance ahead */
        p11 = p11 + state->dtime * (p21 + p12) + state->dtime * state->dtime * p22;
        p12 = p12 + state->dtime * p22;
        p21 = p21 + state->dtime * p22;
        p22 = p22 + state->dtime * state->dtime * KALMAN_Q;

        /* MEASURE UPDATE */

        /* compute Kalman gain */
        k1 = p11 / (p11 + KALMAN_R);
        k2 = p21 / (p11 + KALMAN_R);

        /* update estimates */
        x2 = x2 + k2 * (s - x1);
        x1 = x1 + k1 * (s - x1);

        /* update error covariance */
        p22 = p22 - k2 * p12 ;
        p21 = p21 - k2 * p11;
        p12 = (1 - k1) * p12;
        p11 = (1 - k1) * p11;

        /* store local variables */
        state->x[0] = x1;
        state->x[1] = x2;
        state->p[0] = p11;
        state->p[1] = p12;
        state->p[2] = p21;
        state->p[3] = p22;
        return( 0 );
    }

    kalman_filter_reset( filter );
    return( -1 );
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
