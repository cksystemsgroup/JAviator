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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 */

#include <stdio.h>
#include <malloc.h>

#include "altitude_kf.h"

/* State of an altitude KF */
struct alt_state
{
    double dt;
    double q;
    double r;
    double x1, x2;
    double p11, p12, p21, p22;
};


static inline alt_state_t *get_alt_state( const altitude_kf_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes an altitude KF with the given period.
   Parameter <period> is expected to be given in [s].
   Returns 0 if successful, -1 otherwise.
*/
int altitude_kf_init( altitude_kf_t *filter,
    char *name, double proc_noise, double data_noise, double period )
{
    alt_state_t *state;

    if( period < 0.001 )
    {
        fprintf( stderr, "ERROR: invalid %s filter period (%f)\n", name, period );
        return( -1 );
    }

    state = malloc( sizeof( alt_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->dt     = period;
    state->q      = proc_noise * period * period;
    state->r      = data_noise;
    filter->name  = name;
    filter->state = state;
    return altitude_kf_reset( filter );
}

/* Destroys an altitude KF.
   Returns 0 if successful, -1 otherwise.
*/
int altitude_kf_destroy( altitude_kf_t *filter )
{
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets an altitude KF.
   Returns 0 if successful, -1 otherwise.
*/
int altitude_kf_reset( altitude_kf_t *filter )
{
    alt_state_t *state = get_alt_state( filter );

    if( !state )
    {
        return( -1 );
    }

    /* clear state vector */
    state->x1  = 0;
    state->x2  = 0;

    /* clear covariance matrix */
    state->p11 = 0;
    state->p12 = 0;
    state->p21 = 0;
    state->p22 = 0;

    return( 0 );
}

/* Estimates the position s and velocity v based on the acceleration a.
   Parameter <s> is expected to be given in [mm] and <a> in [mm/s^2].
   Returns 0 if successful, -1 otherwise.
*/
int altitude_kf_update( altitude_kf_t *filter, double s, double a )
{
    alt_state_t *state = get_alt_state( filter );
    double k1, k2;

    if( !state )
    {
        return( -1 );
    }

    /* predict state estimate */
    state->x1 += state->x2 * state->dt;
    state->x2 += a * state->dt;

    /* predict error covariance */
    state->p11 += (state->p12 + state->p21) * state->dt
        + state->p22 * state->dt * state->dt;
    state->p12 += state->p22 * state->dt;
    state->p21 += state->p22 * state->dt;
    state->p22 += state->q;

    /* compute optimal Kalman gain */
    k1 = state->p11 / (state->p11 + state->r);
    k2 = state->p21 / (state->p11 + state->r);

    /* update state estimate */
    state->x2 += k2 * (s - state->x1);
    state->x1 += k1 * (s - state->x1);

    /* update error covariance */
    state->p22 -= k2 * state->p12 ;
    state->p21 -= k2 * state->p11;
    state->p12  = (1 - k1) * state->p12;
    state->p11  = (1 - k1) * state->p11;

    return( 0 );
}

/* Returns the estimated position s in [mm] if successful, -1 otherwise.
*/
double altitude_kf_get_S( altitude_kf_t *filter )
{
    alt_state_t *state = get_alt_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->x1 );
}

/* Returns the estimated velocity v in [mm/s] if successful, -1 otherwise.
*/
double altitude_kf_get_V( altitude_kf_t *filter )
{
    alt_state_t *state = get_alt_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->x2 );
}

/* End of file */
