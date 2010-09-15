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

#ifndef ATTITUDE_EKF
#define ATTITUDE_EKF

struct att_state;
typedef struct att_state att_state_t;

/* This Extended Kalman Filter (EKF) is designed to estimate the attitude
   by fusing Euler angle measurements with angular velocity measurements.
*/
typedef struct
{
    char *          name;
    att_state_t *   state;

} attitude_ekf_t;

/* Initializes an attitude EKF with the given period.
   Parameter <period> is expected to be given in [s].
   Returns 0 if successful, -1 otherwise.
*/
int    attitude_ekf_init( attitude_ekf_t *filter,
    char *name, double std_e, double std_w, double ph_sh, double period );

/* Destroys an attitude EKF.
   Returns 0 if successful, -1 otherwise.
*/
int    attitude_ekf_destroy( attitude_ekf_t *filter );

/* Resets an attitude EKF.
   Returns 0 if successful, -1 otherwise.
*/
int    attitude_ekf_reset( attitude_ekf_t *filter );

/* Estimates the Euler angle e based on the filtered angular rate w.
   Parameter <e> is expected to be given in [mrad] and <w> in [mrad/s].
   Returns 0 if successful, -1 otherwise.
*/
int    attitude_ekf_update( attitude_ekf_t *filter, double e, double w );

/* Returns the estimated Euler angle e in [mrad] if successful, -1 otherwise.
*/
double attitude_ekf_get_E( attitude_ekf_t *filter );

#endif /* !ATTITUDE_EKF */

/* End of file */
