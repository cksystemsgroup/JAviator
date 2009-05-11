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

#include <stdint.h>

#define KALMAN_Q 10000
#define KALMAN_R 0.01

#define MAX_KALMAN_STATE 2  //kalman states
#define MAX_KALMAN_P 4      //the number of elements in the covariance matrix


//data structure used to store Kalman filter parameters
//
struct kalman_filter{
    double x[MAX_KALMAN_STATE];
    double p[MAX_KALMAN_P];
    double z;
    double dz;
};

//initialize kalman filter
void init_kalman_filter(struct kalman_filter *filter);

//reset kalman state and covariance matrix
void reset_kalman_filter(struct kalman_filter *filter);


//Compute Kalman Filter, here the filter is used to estimate the
//altitude speed, z should be in meters, period in seconds, ddz in m/s^2
//this function returns the estimated velocity in m/s
double apply_kalman_filter(struct kalman_filter *filter, double z, double ddz, double period);

#endif

/* End of file */
