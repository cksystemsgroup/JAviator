/*
* This file contains the interface of a butterworth filter.
*/

#ifndef __BUTTER_FILTER
#define __BUTTER_FILTER

#include <stdint.h>
#include "param.h"


//default filter parameters B1+b2+A2=1
#define BUTTER_A2 0.95
#define BUTTER_B1 0.025
#define BUTTER_B2 0.025


//This is the data structure that we need in order to store  filter
//parameters
struct butter_filter{
    double a2;
    double b1;
    double b2;
    double old_raw;
    double old_filtered;
};

//Initialize ddx filter
//
void init_butter_filter(struct butter_filter *filter, double a2, double b1, double b2, double default_value);

//Initialize filter with the default parameters
//
void make_default_butter_filter(struct butter_filter *filter, double default_value);

//Reset altitude filter
//
void reset_butter_filter(struct butter_filter *filter, double default_value);

//Compute the filtered value
//
double apply_butter_filter(struct butter_filter *filter, double raw_signal);
#endif
