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

#include "position_ekf.h"

/* State of a position EKF */
struct pos_state
{
    double dt;
    double q11, q12, q21, q22;
    double r11, r22;
    double x1, x2;
    double p11, p12, p21, p22;
    double od, y1, y2;
};


static inline pos_state_t *get_pos_state( const position_ekf_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes a position EKF with the given period.
   Parameter <period> is expected to be given in [s].
   Returns 0 if successful, -1 otherwise.
*/
int position_ekf_init( position_ekf_t *filter,
    char *name, double std_p, double std_v, double std_a, double period )
{
    pos_state_t *state;

    if( period < 0.001 )
    {
        fprintf( stderr, "ERROR: invalid %s filter period (%f)\n", name, period );
        return( -1 );
    }

    state = malloc( sizeof( pos_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->dt     = period;
    state->q11    = std_a * period * period * period * period / 4;
    state->q12    = std_a * period * period * period / 2;
    state->q21    = std_a * period * period * period / 2;
    state->q22    = std_a * period * period;
    state->r11    = std_p;
    state->r22    = std_v;
    filter->name  = name;
    filter->state = state;
    return position_ekf_reset( filter );
}

/* Destroys a position EKF.
   Returns 0 if successful, -1 otherwise.
*/
int position_ekf_destroy( position_ekf_t *filter )
{
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets a position EKF.
   Returns 0 if successful, -1 otherwise.
*/
int position_ekf_reset( position_ekf_t *filter )
{
    pos_state_t *state = get_pos_state( filter );

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

    /* clear observation delay */
    state->od  = 0;

    /* clear observation vector */
    state->y1  = 0;
    state->y2  = 0;

    return( 0 );
}

/* Estimates the position p and velocity v based on the acceleration a.
   Parameter <p> is expected to be given in [mm] and <a> in [mm/s^2].
   Returns 0 if successful, -1 otherwise.
*/
int position_ekf_update( position_ekf_t *filter, double p, double a,
    int new_observation )
{
    pos_state_t *state = get_pos_state( filter );
    double det, k11, k12, k21, k22, s11, s12, s21, s22;

    if( !state )
    {
        return( -1 );
    }

    /* predict state estimate */
    state->x1 += state->x2 * state->dt + a * state->dt * state->dt / 2;
    state->x2 += a * state->dt;

    /* predict error covariance */
    state->p11 += (state->p12 + state->p21) * state->dt
        + state->p22 * state->dt * state->dt + state->q11;
    state->p12 += state->p22 * state->dt + state->q12;
    state->p21 += state->p22 * state->dt + state->q21;
    state->p22 += state->q22;

    /* increment observation delay */
    state->od += state->dt;

    /* check for new position data */
    if( new_observation )
    {
        /* update observation vector */
        state->y2 = (p - state->y1) / state->od;
        state->y1 = p;
        state->od = 0;

        /* compute optimal Kalman gain */
        k11 = state->p11 + state->r11;
        k12 = state->p12;
        k21 = state->p21;
        k22 = state->p22 + state->r22;
        det = k11 * k22 - k12 * k21;

        /* check for non-zero determinante */
        if( det == 0 )
        {
            det = 1.0e+015; /* ~ 1 / 1.0e-015 */
        }
        else
        {
            det = 1 / det;
        }

        s11 = k22 * det;
        s12 = k12 * det;
        s21 = k21 * det;
        s22 = k11 * det;
        k11 = state->p11 * s11 - state->p12 * s21;
        k12 = state->p12 * s22 - state->p11 * s12;
        k21 = state->p21 * s11 - state->p22 * s21;
        k22 = state->p22 * s22 - state->p21 * s12;

        /* update state estimate */
        s11 = state->y1 - state->x1;
        s22 = state->y2 - state->x2;
        state->x1 += k11 * s11 + k12 * s22;
        state->x2 += k21 * s11 + k22 * s22;

        /* update error covariance */
        s11 = state->p11;
        s12 = state->p12;
        s21 = state->p21;
        s22 = state->p22;
        state->p11 = (1 - k11) * s11 - k12 * s21;
        state->p12 = (1 - k11) * s12 - k12 * s22;
        state->p21 = (1 - k22) * s21 - k21 * s11;
        state->p22 = (1 - k22) * s22 - k21 * s12;
    }

    return( 0 );
}

/* Returns the estimated position p in [mm] if successful, -1 otherwise.
*/
double position_ekf_get_P( position_ekf_t *filter )
{
    pos_state_t *state = get_pos_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->x1 );
}

/* Returns the estimated velocity v in [mm/s] if successful, -1 otherwise.
*/
double position_ekf_get_V( position_ekf_t *filter )
{
    pos_state_t *state = get_pos_state( filter );

    if( !state )
    {
        return( -1 );
    }

    return( state->x2 );
}

/* End of file */
