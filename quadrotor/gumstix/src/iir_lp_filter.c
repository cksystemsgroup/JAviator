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

#include "iir_lp_filter.h"

/* State of an IIR low-pass filter */
struct iir_state
{
    double gain;
    double value;
};


static inline iir_state_t *get_iir_state( const iir_lp_filter_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes an IIR low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int iir_lp_filter_init( iir_lp_filter_t *filter, char *name, double gain )
{
    iir_state_t *state;

    if( gain < 0 || gain > 1 )
    {
        fprintf( stderr, "ERROR: invalid %s filter gain (%f)\n", name, gain );
        return( -1 );
    }

    state = malloc( sizeof( iir_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->gain   = gain;
    filter->name  = name;
    filter->state = state;
    return iir_lp_filter_reset( filter );
}

/* Destroys an IIR low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int iir_lp_filter_destroy( iir_lp_filter_t *filter )
{
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets an IIR low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int iir_lp_filter_reset( iir_lp_filter_t *filter )
{
    iir_state_t *state = get_iir_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->value = 0;
    return( 0 );
}

/* Updates an IIR low-pass filter with the given value.
   Returns the filtered value if successful, -1 otherwise.
*/
double iir_lp_filter_update( iir_lp_filter_t *filter, double update )
{
    iir_state_t *state = get_iir_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->value += state->gain * (update - state->value);
    return( state->value );
}

/* End of file */
