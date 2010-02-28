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

#ifndef EXTENDED_KALMAN
#define EXTENDED_KALMAN

/* Structure for representing EKF parameters */
typedef struct
{
    double dtime;

} extended_kalman_t;

/* Initializes the extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int extended_kalman_init( extended_kalman_t *filter, int period );

/* Resets the extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int extended_kalman_reset( extended_kalman_t *filter );

/* Updates the extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int extended_kalman_update( extended_kalman_t *filter );

#endif /* !EXTENDED_KALMAN */

/* End of file */
