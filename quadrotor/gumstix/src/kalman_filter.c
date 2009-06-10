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

#include <stdint.h>
#include "kalman_filter.h"


//initialize kalman filter
void init_kalman_filter(struct kalman_filter *filter){
        reset_kalman_filter(filter);
}

void reset_kalman_filter(struct kalman_filter *filter){
    uint8_t i;

    //reset state
    for(i=0; i<MAX_KALMAN_STATE; i++){
        filter->x[i] = 0;
    }

    //reset covariance matrix
    for(i=0; i<MAX_KALMAN_P; i++){
        filter->p[i] = 0;
    }

    filter->z = 0;
    filter->dz = 0;
}


double apply_kalman_filter(struct kalman_filter *filter, double z, double ddz, double period){
    double x1, x2, p11, p12, p21, p22, k1, k2;

    //update local variables
    x1 = filter->x[0];
    x2 = filter->x[1];
    p11 = filter->p[0];
    p12 = filter->p[1];
    p21 = filter->p[2];
    p22 = filter->p[3];

    if (z > 0){
        //TIME UPDATE
        //Project the state ahead
        x1 = x1 + period*x2;
        x2 = x2 + period*ddz;

        //x2 = (z-x1)/period;
                //x1 = z;
        //Project the error covarience ahead
        p11 = p11 + period*(p21 + p12) + period*period*p22;
        p12 = p12 + period*p22;
        p21 = p21 + period*p22;
        p22 = p22 + KALMAN_Q*period*period;

        //MEASURE UPDATE
        //Compute the Kalman gain
        k1 = p11/(p11 + KALMAN_R);
        k2 = p21/(p11 + KALMAN_R);

        //Update estimates with measurement zk
        x2 = x2 + k2*(z-x1);
        x1 = x1 + k1*(z-x1);

        //Update the error covariance
        p22 = p22 - k2*p12 ;
        p21 = p21 - k2*p11;
        p12 = (1 - k1)*p12;
        p11 = (1 - k1)*p11;

        //save local variables
        filter->x[0] = x1;
        filter->x[1] = x2;
        filter->p[0] = p11;
        filter->p[1] = p12;
        filter->p[2] = p21;
        filter->p[3] = p22;
    }
    else{
        //save local variables
        filter->x[0] = 0.0;
        filter->x[1] = 0.0;
        filter->p[0] = 0.0;
        filter->p[1] = 0.0;
        filter->p[2] = 0.0;
        filter->p[3] = 0.0;
    }
    filter->z = filter->x[0];
    filter->dz = filter->x[1];
    return filter->x[1];
}

/* End of file */
