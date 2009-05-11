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

#include "butter_filter.h"

//Initialize filter
//
void init_butter_filter( butter_filter_t *filter, double a2, double b1, double b2, double default_value )
{
    filter->a2 = a2;
    filter->b1 = b1;
    filter->b2 = b2;
    filter->old_raw = default_value;
    filter->old_filtered = default_value;
}

void make_default_butter_filter( butter_filter_t *filter, double default_value )
{
    init_butter_filter(filter, BUTTER_A2, BUTTER_B1, BUTTER_B2, default_value);
}

//Reset altitude filter
//
void reset_butter_filter( butter_filter_t *filter, double default_value )
{
    filter->old_raw = default_value;
    filter->old_filtered = default_value;
}

//Compute the filtered value
//
double apply_butter_filter( butter_filter_t *filter, double raw_signal )
{
    filter->old_filtered = filter->old_filtered * filter->a2 +
                           filter->b1 * raw_signal +
                           filter->b2 * filter->old_raw;
    filter->old_raw = raw_signal;

    return filter->old_filtered;
}

/* End of file */
