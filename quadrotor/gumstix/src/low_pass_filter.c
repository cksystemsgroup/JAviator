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

#include <stdio.h>
#include "low_pass_filter.h"


/* Initializes the low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int low_pass_filter_init( low_pass_filter_t *filter, double gain )
{
    if( gain < 0 )
    {
        fprintf( stderr, "ERROR: invalid low-pass filter gain\n" );
        return( -1 );
    }

    filter->gain = gain;

    return low_pass_filter_reset( filter );
}

/* Resets the low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int low_pass_filter_reset( low_pass_filter_t *filter )
{
    filter->value = 0;

    return( 0 );
}

/* Applies the low-pass filter to the given update value.
   Returns the filtered value.
*/
double low_pass_filter_apply( low_pass_filter_t *filter, double update )
{
    filter->value = filter->value + filter->gain * (update - filter->value);

    return( filter->value );
}

/* End of file */
