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

#ifndef MEDIAN_FILTER
#define MEDIAN_FILTER

struct mf_state;
typedef struct mf_state mf_state_t;

/* This filter is designed to compute the median
   over a sorted buffer of user-defined size.
*/
typedef struct
{
    char *          name;
    mf_state_t *    state;

} median_filter_t;

/* Initializes a median filter.
   Returns 0 if successful, -1 otherwise.
*/
int    median_filter_init( median_filter_t *filter, char *name, int size );

/* Destroys a median filter.
   Returns 0 if successful, -1 otherwise.
*/
int    median_filter_destroy( median_filter_t *filter );

/* Resets a median filter.
   Returns 0 if successful, -1 otherwise.
*/
int    median_filter_reset( median_filter_t *filter );

/* Updates a median filter with the given value.
   Returns the filtered value if successful, -1 otherwise.
*/
double median_filter_update( median_filter_t *filter, double update );

#endif /* !MEDIAN_FILTER */

/* End of file */
