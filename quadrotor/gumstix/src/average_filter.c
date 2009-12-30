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

#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "average_filter.h"


/* Initializes the average filter.
   Returns 0 if successful, -1 otherwise.
*/
int average_filter_init( average_filter_t *filter, int size )
{
    if( size < 1 )
    {
        fprintf( stderr, "ERROR: invalid average-filter size\n" );
        return( -1 );
    }

    filter->size  = size;
    filter->array = malloc( sizeof( *filter->array ) * filter->size );
    filter->index = 0;

    return average_filter_reset( filter );
}

/* Resets the average filter.
   Returns 0 if successful, -1 otherwise.
*/
int average_filter_reset( average_filter_t *filter )
{
    if( filter->array == NULL )
    {
        fprintf( stderr, "ERROR: average filter not initialized\n" );
        return( -1 );
    }

    memset( filter->array, 0, sizeof( *filter->array ) * filter->size );
    filter->index = 0;
    return( 0 );
}

/* Destroys the average filter.
   Returns 0 if successful, -1 otherwise.
*/
int average_filter_destroy( average_filter_t *filter )
{
    free( filter->array );
    filter->array = NULL;
    return( 0 );
}

/* Applies the average filter to the given update value.
   Returns the filtered value.
*/
double average_filter_apply( average_filter_t *filter, double update )
{
    int i;

    filter->array[ filter->index ] = update;

    if( ++filter->index == filter->size )
    {
        filter->index = 0;
    }

    for( update = 0, i = 0; i < filter->size; ++i )
    {
        update += filter->array[i];
    }

    return( update / filter->size );
}

/* End of file */
