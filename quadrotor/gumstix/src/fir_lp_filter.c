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

#include "fir_lp_filter.h"

/* State of a FIR low-pass filter */
struct fir_state
{
    double gain[2];
    double value[3];
};


static inline fir_state_t *get_fir_state( const fir_lp_filter_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes a FIR low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int fir_lp_filter_init( fir_lp_filter_t *filter, char *name, double gain )
{
    fir_state_t *state;

    if( gain < 0 || gain > 1 )
    {
        fprintf( stderr, "ERROR: invalid %s filter gain (%f)\n", name, gain );
        return( -1 );
    }

    state = malloc( sizeof( fir_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->gain[0] = (1 - gain) / 2;
    state->gain[1] = gain;
    filter->name   = name;
    filter->state  = state;
    return fir_lp_filter_reset( filter );
}

/* Destroys a FIR low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int fir_lp_filter_destroy( fir_lp_filter_t *filter )
{
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets a FIR low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int fir_lp_filter_reset( fir_lp_filter_t *filter )
{
    fir_state_t *state = get_fir_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->value[0] = 0;
    state->value[1] = 0;
    state->value[2] = 0;
    return( 0 );
}

/* Updates a FIR low-pass filter with the given value.
   Returns the filtered value if successful, -1 otherwise.
*/
double fir_lp_filter_update( fir_lp_filter_t *filter, double update )
{
    fir_state_t *state = get_fir_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->value[0] = state->value[1];
    state->value[1] = state->value[2];
    state->value[2] = update;

    return( state->gain[0] * state->value[0]
          + state->gain[1] * state->value[1]
          + state->gain[0] * state->value[2] );
}

/* End of file */
