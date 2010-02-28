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

#ifndef AVERAGE_FILTER
#define AVERAGE_FILTER

/* Structure for representing average-filter parameters */
typedef struct
{
    int     size;
    double *array;
    int     index;

} average_filter_t;

/* Initializes the average filter.
   Returns 0 if successful, -1 otherwise.
*/
int    average_filter_init( average_filter_t *filter, int size );

/* Resets the average filter.
   Returns 0 if successful, -1 otherwise.
*/
int    average_filter_reset( average_filter_t *filter );

/* Destroys the average filter.
   Returns 0 if successful, -1 otherwise.
*/
int    average_filter_destroy( average_filter_t *filter );

/* Updates the average filter with the given value.
   Returns the filtered value.
*/
double average_filter_update( average_filter_t *filter, double update );

#endif /* !AVERAGE_FILTER */

/* End of file */
