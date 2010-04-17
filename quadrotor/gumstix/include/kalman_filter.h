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

#ifndef KALMAN_FILTER
#define KALMAN_FILTER

struct kf_state;
typedef struct kf_state kf_state_t;

/* This Kalman filter is designed to estimate the missing velocity ds
   from a given distance s and acceleration dds.  The distance s is
   assumed to represent a certain distance between the helicopter and
   some environmental object and is therefore assumed to have always
   a positive value.  Passing a negative value to the update function
   causes the filter to reset, indicated by returning -1 instead of 0.
   In addition to estimating velocity ds, distance s is propagated to
   obtain an improved estimate, and therefore, the filter's internal
   s-component should be accessed and used in subsequent computations.
*/
typedef struct
{
    char *          name;
    kf_state_t *    state;

} kalman_filter_t;

/* Initializes a Kalman filter with the given period.
   Parameter <period> is expected to be given in [s].
   Returns 0 if successful, -1 otherwise.
*/
int    kalman_filter_init( kalman_filter_t *filter,
    char *name, double proc_noise, double data_noise, double period );

/* Destroys a Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int    kalman_filter_destroy( kalman_filter_t *filter );

/* Resets a Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int    kalman_filter_reset( kalman_filter_t *filter );

/* Filters the given distance and estimates the missing velocity ds.
   Parameter <s> is expected to be given in [mm] and <dds> in [mm/s^2].
   Returns 0 if successful, -1 otherwise.
*/
int    kalman_filter_update( kalman_filter_t *filter, double s, double dds );

/* Returns the filtered distance s in [mm] if successful, -1 otherwise.
*/
double kalman_filter_get_S( kalman_filter_t *filter );

/* Returns the estimated velocity ds in [mm/s] if successful, -1 otherwise.
*/
double kalman_filter_get_dS( kalman_filter_t *filter );

#endif /* !KALMAN_FILTER */

/* End of file */
