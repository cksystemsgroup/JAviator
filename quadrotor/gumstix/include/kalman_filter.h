/*
* This file contains the interface of a Kalman filter
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
