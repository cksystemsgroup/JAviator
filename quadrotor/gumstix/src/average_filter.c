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
#include <string.h>

#include "average_filter.h"

/* State of an average filter */
struct af_state
{
    double *array;
    int     size;
    int     index;
};


static inline af_state_t *get_af_state( const average_filter_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes an average filter.
   Returns 0 if successful, -1 otherwise.
*/
int average_filter_init( average_filter_t *filter, char *name, int size )
{
    af_state_t *state;

    if( size < 1 )
    {
        fprintf( stderr, "ERROR: invalid %s filter size (%d)\n", name, size );
        return( -1 );
    }

    state = malloc( sizeof( af_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->array = malloc( sizeof( *state->array ) * size );

    if( !state->array )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        free( state );
        return( -1 );
    }

    state->size   = size;
    filter->name  = name;
    filter->state = state;
    return average_filter_reset( filter );
}

/* Destroys an average filter.
   Returns 0 if successful, -1 otherwise.
*/
int average_filter_destroy( average_filter_t *filter )
{
    free( filter->state->array );
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets an average filter.
   Returns 0 if successful, -1 otherwise.
*/
int average_filter_reset( average_filter_t *filter )
{
    af_state_t *state = get_af_state( filter );

    if( !state )
    {
        return( -1 );
    }

    memset( state->array, 0, sizeof( *state->array ) * state->size );
    state->index = 0;
    return( 0 );
}

/* Updates an average filter with the given value.
   Returns the filtered value if successful, -1 otherwise.
*/
double average_filter_update( average_filter_t *filter, double update )
{
    af_state_t *state = get_af_state( filter );
    int i;

    if( !state )
    {
        return( -1 );
    }

    state->array[ state->index ] = update;

    if( ++state->index == state->size )
    {
        state->index = 0;
    }

    for( update = 0, i = 0; i < state->size; ++i )
    {
        update += state->array[i];
    }

    return( update / state->size );
}

/* End of file */
