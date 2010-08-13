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

#ifndef IIR_LP_FILTER
#define IIR_LP_FILTER

struct iir_state;
typedef struct iir_state iir_state_t;

/* This Infinite Impulse Response (IIR) filter is designed to compute
   the integral over an infinite sequence of values with the update
   effort controlled by the user-defined gain.
*/
typedef struct
{
    char *          name;
    iir_state_t *   state;

} iir_lp_filter_t;

/* Initializes an IIR low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int    iir_lp_filter_init( iir_lp_filter_t *filter, char *name, double gain );

/* Destroys an IIR low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int    iir_lp_filter_destroy( iir_lp_filter_t *filter );

/* Resets an IIR low-pass filter.
   Returns 0 if successful, -1 otherwise.
*/
int    iir_lp_filter_reset( iir_lp_filter_t *filter );

/* Updates an IIR low-pass filter with the given value.
   Returns the filtered value if successful, -1 otherwise.
*/
double iir_lp_filter_update( iir_lp_filter_t *filter, double update );

#endif /* !IIR_LP_FILTER */

/* End of file */
