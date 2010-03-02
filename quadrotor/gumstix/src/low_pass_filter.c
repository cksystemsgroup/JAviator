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

#include "low_pass_filter.h"

/* State of a low-pass filter */
struct lpf_state
{
    double gain;
    double value;
};


static inline lpf_state_t *get_lpf_state( const low_pass_filter_t *filter )
{
    lpf_state_t *state = (lpf_state_t *) filter->state;

    if( !state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( state );
}

/* Initializes a low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int low_pass_filter_init( low_pass_filter_t *filter, char *name, double gain )
{
    lpf_state_t *state;

    if( gain < 0 )
    {
        fprintf( stderr, "ERROR: invalid %s filter gain (%f)\n", name, gain );
        return( -1 );
    }

    state = malloc( sizeof( lpf_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->gain   = gain;
    filter->name  = name;
    filter->state = state;
    return low_pass_filter_reset( filter );
}

/* Destroys a low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int low_pass_filter_destroy( low_pass_filter_t *filter )
{
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets a low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int low_pass_filter_reset( low_pass_filter_t *filter )
{
    lpf_state_t *state = get_lpf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->value = 0;
    return( 0 );
}

/* Updates a low-pass filter with the given value.
   Returns the filtered value if successful, -1 otherwise.
*/
double low_pass_filter_update( low_pass_filter_t *filter, double update )
{
    lpf_state_t *state = get_lpf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->value = state->value + state->gain * (update - state->value);
    return( state->value );
}

/* End of file */
