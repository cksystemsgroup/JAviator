/*
* This file contains the implementation of a Kalman filter
*/


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
