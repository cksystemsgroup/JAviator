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

#ifndef MEDIAN_FILTER
#define MEDIAN_FILTER


/* Structure for representing median-filter parameters */
typedef struct
{
    int     size;
    double *array;

} median_filter_t;


/* Initializes the median filter.
   Returns 0 if successful, -1 otherwise.
*/
int    median_filter_init( median_filter_t *filter, int size );

/* Resets the median filter.
   Returns 0 if successful, -1 otherwise.
*/
int    median_filter_reset( median_filter_t *filter );

/* Destroys the median filter.
   Returns 0 if successful, -1 otherwise.
*/
int    median_filter_destroy( median_filter_t *filter );

/* Applies the median filter to the given update value.
   Returns the filtered value.
*/
double median_filter_apply( median_filter_t *filter, double update );


#endif /* !MEDIAN_FILTER */

/* End of file */
