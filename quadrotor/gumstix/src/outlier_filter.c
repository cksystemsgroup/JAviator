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
#include <math.h>

#include "outlier_filter.h"

/* State of an outlier filter */
struct of_state
{
    double  mdiff;
    double  value;
    int     limit;
    int     count;
};


static inline of_state_t *get_of_state( const outlier_filter_t *filter )
{
    if( !filter->state )
    {
        fprintf( stderr, "ERROR: null pointer to %s filter state\n", filter->name );
    }

    return( filter->state );
}

/* Initializes an outlier filter.
   Returns 0 if successful, -1 otherwise.
*/
int outlier_filter_init( outlier_filter_t *filter, char *name, double mdiff, int limit )
{
    of_state_t *state;

    if( limit < 1 )
    {
        fprintf( stderr, "ERROR: invalid %s filter limit (%d)\n", name, limit );
        return( -1 );
    }

    state = malloc( sizeof( of_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s filter failed\n", name );
        return( -1 );
    }

    state->mdiff  = fabs( mdiff ); /* make sure state->mdiff is positive */
    state->limit  = limit;
    filter->name  = name;
    filter->state = state;
    return outlier_filter_reset( filter );
}

/* Destroys an outlier filter.
   Returns 0 if successful, -1 otherwise.
*/
int outlier_filter_destroy( outlier_filter_t *filter )
{
    free( filter->state );
    filter->state = NULL;
    return( 0 );
}

/* Resets the outlier filter.
   Returns 0 if successful, -1 otherwise.
*/
int outlier_filter_reset( outlier_filter_t *filter )
{
    of_state_t *state = get_of_state( filter );

    if( !state )
    {
        return( -1 );
    }

    state->value = 0;
    state->count = 0;
    return( 0 );
}

/* Updates an outlier filter with the given value.
   Returns the filtered value if successful, -1 otherwise.
*/
double outlier_filter_update( outlier_filter_t *filter, double update )
{
    of_state_t *state = get_of_state( filter );

    if( !state )
    {
        return( -1 );
    }

    if( fabs( state->value - update ) > state->mdiff && state->count < state->limit )
    {
        update = state->value;
        ++state->count;
    }
    else
    {
        state->value = update;
        state->count = 0;
    }

    return( update );
}

/* End of file */
