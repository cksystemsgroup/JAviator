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

#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "median_filter.h"


/* Initializes the median filter.
   Returns 0 if successful, -1 otherwise.
*/
int median_filter_init( median_filter_t *filter, int size )
{
    if( size < 1 )
    {
        fprintf( stderr, "ERROR: invalid median-filter size\n" );
        return( -1 );
    }

    filter->size  = size;
    filter->array = malloc( sizeof( *filter->array ) * filter->size );

    return median_filter_reset( filter );
}

/* Resets the median filter.
   Returns 0 if successful, -1 otherwise.
*/
int median_filter_reset( median_filter_t *filter )
{
    if( filter->array == NULL )
    {
        fprintf( stderr, "ERROR: median filter not initialized\n" );
        return( -1 );
    }

    memset( filter->array, 0, sizeof( *filter->array ) * filter->size );
    return( 0 );
}

/* Destroys the median filter.
   Returns 0 if successful, -1 otherwise.
*/
int median_filter_destroy( median_filter_t *filter )
{
    free( filter->array );
    filter->array = NULL;
    return( 0 );
}

/* Updates the median filter with the given value.
   Returns the filtered value.
*/
double median_filter_update( median_filter_t *filter, double update )
{
    int i, j;

    for( i = 0; i < filter->size; ++i )
    {
        if( update < filter->array[i] )
        {
            break;
        }
    }

    if( i < filter->size )
    {
        j = filter->size - 1;

        while( j > i )
        {
            filter->array[j] = filter->array[j-1];
            --j;
        }

        filter->array[j] = update;
    }
    else
    {
        i = filter->size - 1;
        j = 0;

        while( j < i )
        {
            filter->array[j] = filter->array[j+1];
            ++j;
        }

        filter->array[j] = update;
    }

    return( filter->array[ filter->size >> 1 ] );
}

/* End of file */
