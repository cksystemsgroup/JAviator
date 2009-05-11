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

#ifndef __BUTTER_FILTER
#define __BUTTER_FILTER

#include "filter_params.h"


//default filter parameters B1+b2+A2=1
#define BUTTER_A2 0.95
#define BUTTER_B1 0.025
#define BUTTER_B2 0.025


//This is the data structure that we need in order to store  filter
//parameters
typedef struct
{
    double a2;
    double b1;
    double b2;
    double old_raw;
    double old_filtered;

} butter_filter_t;

//Initialize ddx filter
//
void init_butter_filter( butter_filter_t *filter, double a2, double b1, double b2, double default_value );

//Initialize filter with the default parameters
//
void make_default_butter_filter( butter_filter_t *filter, double default_value );

//Reset altitude filter
//
void reset_butter_filter( butter_filter_t *filter, double default_value );

//Compute the filtered value
//
double apply_butter_filter( butter_filter_t *filter, double raw_signal );
#endif

/* End of file */
