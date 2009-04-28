/*
* This file contains the implementation of a butterworth filter.
*/


#include "butter_filter.h"

//Initialize filter
//
void init_butter_filter(struct butter_filter *filter, double a2, double b1, double b2, double default_value){
    filter->a2 = a2;
    filter->b1 = b1;
    filter->b2 = b2;
    filter->old_raw = default_value;
    filter->old_filtered = default_value;
}

void make_default_butter_filter(struct butter_filter *filter, double default_value){
    init_butter_filter(filter, BUTTER_A2, BUTTER_B1, BUTTER_B2, default_value);
}

//Reset altitude filter
//
void reset_butter_filter(struct butter_filter *filter, double default_value){
    filter->old_raw = default_value;
    filter->old_filtered = default_value;
}

//Compute the filtered value
//
double apply_butter_filter(struct butter_filter *filter, double raw_signal){
    filter->old_filtered = filter->old_filtered * filter->a2 +
                           filter->b1 * raw_signal +
                           filter->b2 * filter->old_raw;
    filter->old_raw = raw_signal;

    return filter->old_filtered;
}
