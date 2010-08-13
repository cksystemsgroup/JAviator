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

#include "median_filter.h"

/* State of a median filter */
struct mf_state
{
    double *array;
    int     size;
};


static inline mf_state_t *get_mf_state( const median_filter_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes a median filter.
   Returns 0 if successful, -1 otherwise.
*/
int median_filter_init( median_filter_t *filter, char *name, int size )
{
    mf_state_t *state;

    if( size < 1 )
    {
        fprintf( stderr, "ERROR: invalid %s filter size (%d)\n", name, size );
        return( -1 );
    }

    state = malloc( sizeof( mf_state_t ) );

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
    return median_filter_reset( filter );
}

/* Destroys a median filter.
   Returns 0 if successful, -1 otherwise.
*/
int median_filter_destroy( median_filter_t *filter )
{
    free( filter->state->array );
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets a median filter.
   Returns 0 if successful, -1 otherwise.
*/
int median_filter_reset( median_filter_t *filter )
{
    mf_state_t *state = get_mf_state( filter );

    if( !state )
    {
        return( -1 );
    }

    memset( state->array, 0, sizeof( *state->array ) * state->size );
    return( 0 );
}

/* Updates a median filter with the given value.
   Returns the filtered value if successful, -1 otherwise.
*/
double median_filter_update( median_filter_t *filter, double update )
{
    mf_state_t *state = get_mf_state( filter );
    int i, j;

    if( !state )
    {
        return( -1 );
    }

    for( i = 0; i < state->size; ++i )
    {
        if( update < state->array[i] )
        {
            break;
        }
    }

    if( i < state->size )
    {
        j = state->size - 1;

        while( j > i )
        {
            state->array[j] = state->array[j-1];
            --j;
        }

        state->array[j] = update;
    }
    else
    {
        i = state->size - 1;
        j = 0;

        while( j < i )
        {
            state->array[j] = state->array[j+1];
            ++j;
        }

        state->array[j] = update;
    }

    return( state->array[ state->size >> 1 ] );
}

/* End of file */
