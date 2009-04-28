/*
* This file contains the definition for the filter parameters
*/

//ddz butter filter parameters
//this should be adjust so that B1+B2+A2=1, the greater A2 is the 
//stronger the filter is, if A2=0, B1=1 and B2=0 then there is no filtering
#define DDZ_FILTER_A2 0.95
#define DDZ_FILTER_B1 0.025
#define DDZ_FILTER_B2 0.025

//ddz butter filter parameters
//this should be adjust so that B1+B2+A2=1, the greater A2 is the 
//stronger the filter is, if A2=0, B1=1 and B2=0 then there is no filtering
#define Z_FILTER_A2 0.95
#define Z_FILTER_B1 0.025
#define Z_FILTER_B2 0.025

//z_ref butter filter parameters
//this should be adjust so that B1+B2+A2=1, the greater A2 is the 
//stronger th filter is, if A2=0, B1=1 and B2=0 then there is no filtering
#define Z_REF_FILTER_A2 0.9
#define Z_REF_FILTER_B1 0.1
#define Z_REF_FILTER_B2 0.0
