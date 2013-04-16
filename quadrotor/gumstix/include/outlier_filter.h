/*
 *  Copyright (c) 2006-2013 Rainer Trummer <rainer.trummer@gmail.com>
 *
 *  Department of Computer Sciences, www.cs.uni-salzburg.at
 *  University of Salzburg, www.uni-salzburg.at
 *
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

#ifndef OUTLIER_FILTER
#define OUTLIER_FILTER

struct of_state;
typedef struct of_state of_state_t;

/* This filter is designed to reject outliers
   controlled by the user-defined maximum
   difference and limit of occurrences.
*/
typedef struct
{
    char *          name;
    of_state_t *    state;

} outlier_filter_t;

/* Initializes an outlier filter.
   Returns 0 if successful, -1 otherwise.
*/
int    outlier_filter_init( outlier_filter_t *filter, char *name, double mdiff, int limit );

/* Destroys an outlier filter.
   Returns 0 if successful, -1 otherwise.
*/
int    outlier_filter_destroy( outlier_filter_t *filter );

/* Resets an outlier filter.
   Returns 0 if successful, -1 otherwise.
*/
int    outlier_filter_reset( outlier_filter_t *filter );

/* Updates an outlier filter with the given value.
   Returns the filtered value if successful, -1 otherwise.
*/
double outlier_filter_update( outlier_filter_t *filter, double update );

#endif /* !OUTLIER_FILTER */

/* End of file */
