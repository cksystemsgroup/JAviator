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


#define KALMAN_STATES   2   /* number of Kalman states */
#define KALMAN_P        4   /* elements in the covariance matrix */
#define KALMAN_Q        10000.0
#define KALMAN_R        0.01

/* Structure for representing Kalman filter parameters */
typedef struct
{
    double dtime;
    double x[ KALMAN_STATES ];
    double p[ KALMAN_P ];
    double s;
    double ds;

} kalman_filter_t;


/* Initializes the Kalman filter with the given period [ms].
   Returns 0 if successful, -1 otherwise.
*/
int    kalman_filter_init( kalman_filter_t *filter, int period );

/* Resets the Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int    kalman_filter_reset( kalman_filter_t *filter );

/* Estimates the speed ds.  Parameters are expected
   to be given as follows: s in [mm] and dds in [mm/s^2].
   Returns the estimated velocity ds in [mm/s].
*/
double kalman_filter_apply( kalman_filter_t *filter, double s, double dds );


#endif /* !KALMAN_FILTER */

/* End of file */
