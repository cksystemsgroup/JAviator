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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 */

#ifndef ALTITUDE_KF
#define ALTITUDE_KF

struct alt_state;
typedef struct alt_state alt_state_t;

/* This Kalman Filter (KF) is designed to estimate the position
   as well as the velocity by fusing position measurements with
   linear acceleration measurements.
*/
typedef struct
{
    char *          name;
    alt_state_t *   state;

} altitude_kf_t;

/* Initializes an altitude KF with the given period.
   Parameter <period> is expected to be given in [s].
   Returns 0 if successful, -1 otherwise.
*/
int    altitude_kf_init( altitude_kf_t *filter,
    char *name, double proc_noise, double data_noise, double period );

/* Destroys an altitude KF.
   Returns 0 if successful, -1 otherwise.
*/
int    altitude_kf_destroy( altitude_kf_t *filter );

/* Resets an altitude KF.
   Returns 0 if successful, -1 otherwise.
*/
int    altitude_kf_reset( altitude_kf_t *filter );

/* Estimates the position s and velocity v based on the acceleration a.
   Parameter <s> is expected to be given in [mm] and <a> in [mm/s^2].
   Returns 0 if successful, -1 otherwise.
*/
int    altitude_kf_update( altitude_kf_t *filter, double s, double a );

/* Returns the estimated position s in [mm] if successful, -1 otherwise.
*/
double altitude_kf_get_S( altitude_kf_t *filter );

/* Returns the estimated velocity v in [mm/s] if successful, -1 otherwise.
*/
double altitude_kf_get_V( altitude_kf_t *filter );

#endif /* !ALTITUDE_KF */

/* End of file */
