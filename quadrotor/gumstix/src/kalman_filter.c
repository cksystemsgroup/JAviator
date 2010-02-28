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

#include <stdio.h>
#include "kalman_filter.h"


/* Initializes the Kalman filter with the given period [ms].
   Returns 0 if successful, -1 otherwise.
*/
int kalman_filter_init( kalman_filter_t *filter, int period )
{
    if( period < 0 )
    {
        fprintf( stderr, "ERROR: invalid Kalman filter period\n" );
        return( -1 );
    }

    filter->dtime = period / 1000.0; /* filter uses period in seconds */

    return kalman_filter_reset( filter );
}

/* Resets the Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int kalman_filter_reset( kalman_filter_t *filter )
{
    int i;

    if( KALMAN_STATES != 2 || KALMAN_P != 4 )
    {
        fprintf( stderr, "ERROR: invalid Kalman filter constants\n" );
        return( -1 );
    }

    /* reset Kalman states */
    for( i = 0; i < KALMAN_STATES; ++i )
    {
        filter->x[i] = 0;
    }

    /* reset covariance matrix */
    for( i = 0; i < KALMAN_P; ++i )
    {
        filter->p[i] = 0;
    }

    filter->s  = 0;
    filter->ds = 0;

    return( 0 );
}

/* Estimates the speed ds.  Parameters are expected
   to be given as follows: s in [mm] and dds in [mm/s^2].
   Returns the estimated velocity ds in [mm/s].
*/
double kalman_filter_update( kalman_filter_t *filter, double s, double dds )
{
    double x1, x2, p11, p12, p21, p22, k1, k2;

    /* update local variables */
    x1  = filter->x[0];
    x2  = filter->x[1];
    p11 = filter->p[0];
    p12 = filter->p[1];
    p21 = filter->p[2];
    p22 = filter->p[3];

    if( s > 0 )
    {
        /* TIME UPDATE */

        /* project state ahead */
        x1 = x1 + filter->dtime * x2;
        x2 = x2 + filter->dtime * dds;

        /* project error covariance ahead */
        p11 = p11 + filter->dtime * (p21 + p12) + filter->dtime * filter->dtime * p22;
        p12 = p12 + filter->dtime * p22;
        p21 = p21 + filter->dtime * p22;
        p22 = p22 + filter->dtime * filter->dtime * KALMAN_Q;

        /* MEASURE UPDATE */

        /* compute Kalman gain */
        k1 = p11 / (p11 + KALMAN_R);
        k2 = p21 / (p11 + KALMAN_R);

        /* update estimates */
        x2 = x2 + k2 * (s - x1);
        x1 = x1 + k1 * (s - x1);

        /* update error covariance */
        p22 = p22 - k2 * p12 ;
        p21 = p21 - k2 * p11;
        p12 = (1 - k1) * p12;
        p11 = (1 - k1) * p11;

        /* store local variables */
        filter->x[0] = x1;
        filter->x[1] = x2;
        filter->p[0] = p11;
        filter->p[1] = p12;
        filter->p[2] = p21;
        filter->p[3] = p22;
    }
    else
    {
        /* reset local variables */
        filter->x[0] = 0;
        filter->x[1] = 0;
        filter->p[0] = 0;
        filter->p[1] = 0;
        filter->p[2] = 0;
        filter->p[3] = 0;
    }

    /* store z and dz value */
    filter->s  = filter->x[0];
    filter->ds = filter->x[1];

    return( filter->ds );
}

/* End of file */
