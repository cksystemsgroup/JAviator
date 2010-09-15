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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 */

#include <stdio.h>
#include <malloc.h>

#include "attitude_ekf.h"

/* State of an attitude EKF */
struct att_state
{
    double dt;
    double q;
    double r;
    double x;
    double p;
};


static inline att_state_t *get_att_state( const attitude_ekf_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes an attitude EKF with the given period.
   Parameter <period> is expected to be given in [s].
   Returns 0 if successful, -1 otherwise.
*/
int attitude_ekf_init( attitude_ekf_t *filter,
    char *name, double std_e, double std_w, double ph_sh, double period )
{
    att_state_t *state;

    if( period < 0.001 )
    {
        fprintf( stderr, "ERROR: invalid %s filter period (%f)\n", name, period );
        return( -1 );
    }

    state = malloc( sizeof( att_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->dt     = period + ph_sh * period;
    state->q      = std_w * state->dt * state->dt;
    state->r      = std_e;
    filter->name  = name;
    filter->state = state;
    return attitude_ekf_reset( filter );
}

/* Destroys an attitude EKF.
   Returns 0 if successful, -1 otherwise.
*/
int attitude_ekf_destroy( attitude_ekf_t *filter )
{
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets an attitude EKF.
   Returns 0 if successful, -1 otherwise.
*/
int attitude_ekf_reset( attitude_ekf_t *filter )
{
    att_state_t *state = get_att_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->x = 0;
    state->p = 0;
    return( 0 );
}

/* Estimates the Euler angle e based on the filtered angular rate w.
   Parameter <e> is expected to be given in [mrad] and <w> in [mrad/s].
   Returns 0 if successful, -1 otherwise.
*/
int attitude_ekf_update( attitude_ekf_t *filter, double e, double w )
{
    att_state_t *state = get_att_state( filter );
    double k;

    if( !state )
    {
        return( -1 );
    }

    /* predict state estimate */
    state->x += w * state->dt;

    /* predict error covariance */
    state->p += state->q;

    /* compute optimal Kalman gain */
    k = state->p / (state->p + state->r);

    /* update state estimate */
    state->x += k * (e - state->x);

    /* update error covariance */
    state->p = (1 - k) * state->p;

    return( 0 );
}

/* Returns the estimated Euler angle e in [mrad] if successful, -1 otherwise.
*/
double attitude_ekf_get_E( attitude_ekf_t *filter )
{
    att_state_t *state = get_att_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->x );
}

/* End of file */
