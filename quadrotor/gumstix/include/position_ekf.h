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

#ifndef POSITION_EKF
#define POSITION_EKF

struct pos_state;
typedef struct pos_state pos_state_t;

/* This Extended Kalman Filter (EKF) is designed to estimate the position
   as well as the velocity by fusing position measurements with linear
   acceleration measurements.
*/
typedef struct
{
    char *          name;
    pos_state_t *   state;

} position_ekf_t;

/* Initializes a position EKF with the given period.
   Parameter <period> is expected to be given in [s].
   Returns 0 if successful, -1 otherwise.
*/
int    position_ekf_init( position_ekf_t *filter,
    char *name, double std_p, double std_v, double std_a, double period );

/* Destroys a position EKF.
   Returns 0 if successful, -1 otherwise.
*/
int    position_ekf_destroy( position_ekf_t *filter );

/* Resets a position EKF.
   Returns 0 if successful, -1 otherwise.
*/
int    position_ekf_reset( position_ekf_t *filter );

/* Estimates the position p and velocity v based on the acceleration a.
   Parameter <p> is expected to be given in [mm] and <a> in [mm/s^2].
   Returns 0 if successful, -1 otherwise.
*/
int    position_ekf_update( position_ekf_t *filter, double p, double a,
    int new_observation );

/* Returns the estimated position p in [mm] if successful, -1 otherwise.
*/
double position_ekf_get_P( position_ekf_t *filter );

/* Returns the estimated velocity v in [mm/s] if successful, -1 otherwise.
*/
double position_ekf_get_V( position_ekf_t *filter );

#endif /* !POSITION_EKF */

/* End of file */
