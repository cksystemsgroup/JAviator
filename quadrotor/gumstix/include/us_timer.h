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

#ifndef US_TIMER_H
#define US_TIMER_H


#define NSECS_PER_SEC   1000000000
#define NSECS_PER_MSEC  1000000
#define NSECS_PER_USEC  1000
#define USECS_PER_SEC   1000000
#define USECS_PER_MSEC  1000
#define MSECS_PER_SEC   1000


unsigned long long get_utime( void );

int sleep_until( unsigned long long utime );

int sleep_for( long long usecs );


extern void calc_stats(long long time, int id);
#define NUM_STATS        9
#define STAT_IMU         0
#define STAT_TO_JAV      1
#define STAT_FROM_JAV    2
#define STAT_FROM_TERM   3
#define STAT_TO_TERM     4
#define STAT_CONTROL     5
#define STAT_SLEEP       6
#define STAT_READ        7
#define STAT_ALL         8
#endif /* !US_TIMER_H */

/* End of file */
