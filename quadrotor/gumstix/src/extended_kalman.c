/*
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
#include <string.h>

#include "extended_kalman.h"
#include "transformation.h"

/* Standard deviations */
#define STD_ATTITUDE    0.1
#define STD_ANG_VEL     10
#define STD_POSITION    0.1
#define STD_VELOCITY    10
#define STD_LIN_ACC     1000

static double dt, x1[3], x2[3], x3[3];
static double q[3][3], f[3][3], i[3][3];
static double r1[3][3], r2[3][3], r3[3][3];
static double p1[3][3], p2[3][3], p3[3][3];


static inline void *add_m( double a[3][3], double b[3][3], double c[3][3] )
{
    c[0][0] = a[0][0] + b[0][0];
    c[0][1] = a[0][1] + b[0][1];
    c[0][2] = a[0][2] + b[0][2];

    c[1][0] = a[1][0] + b[1][0];
    c[1][1] = a[1][1] + b[1][1];
    c[1][2] = a[1][2] + b[1][2];

    c[2][0] = a[2][0] + b[2][0];
    c[2][1] = a[2][1] + b[2][1];
    c[2][2] = a[2][2] + b[2][2];

    return( (void *) c );}

static inline void *sub_m( double a[3][3], double b[3][3], double c[3][3] )
{
    c[0][0] = a[0][0] - b[0][0];
    c[0][1] = a[0][1] - b[0][1];
    c[0][2] = a[0][2] - b[0][2];

    c[1][0] = a[1][0] - b[1][0];
    c[1][1] = a[1][1] - b[1][1];
    c[1][2] = a[1][2] - b[1][2];

    c[2][0] = a[2][0] - b[2][0];
    c[2][1] = a[2][1] - b[2][1];
    c[2][2] = a[2][2] - b[2][2];

    return( (void *) c );}

static inline void *mul_m( double a[3][3], double b[3][3], double c[3][3] )
{
    c[0][0] = a[0][0]*b[0][0] + a[0][1]*b[1][0] + a[0][2]*b[2][0];
    c[0][1] = a[0][0]*b[0][1] + a[0][1]*b[1][1] + a[0][2]*b[2][1];
    c[0][2] = a[0][0]*b[0][2] + a[0][1]*b[1][2] + a[0][2]*b[2][2];

    c[1][0] = a[1][0]*b[0][0] + a[1][1]*b[1][0] + a[1][2]*b[2][0];
    c[1][1] = a[1][0]*b[0][1] + a[1][1]*b[1][1] + a[1][2]*b[2][1];
    c[1][2] = a[1][0]*b[0][2] + a[1][1]*b[1][2] + a[1][2]*b[2][2];

    c[2][0] = a[2][0]*b[0][0] + a[2][1]*b[1][0] + a[2][2]*b[2][0];
    c[2][1] = a[2][0]*b[0][1] + a[2][1]*b[1][1] + a[2][2]*b[2][1];
    c[2][2] = a[2][0]*b[0][2] + a[2][1]*b[1][2] + a[2][2]*b[2][2];

    return( (void *) c );}

static inline void *mul_v( double a[3][3], double u[3], double v[3] )
{
    v[0] = a[0][0]*u[0] + a[0][1]*u[1] + a[0][2]*u[2];
    v[1] = a[1][0]*u[0] + a[1][1]*u[1] + a[1][2]*u[2];
    v[2] = a[2][0]*u[0] + a[2][1]*u[1] + a[2][2]*u[2];

    return( (void *) v );}

static inline void *trp_m( double a[3][3], double b[3][3] )
{
    b[0][0] = a[0][0];
    b[0][1] = a[1][0];
    b[0][2] = a[2][0];

    b[1][0] = a[0][1];
    b[1][1] = a[1][1];
    b[1][2] = a[2][1];

    b[2][0] = a[0][2];
    b[2][1] = a[1][2];
    b[2][2] = a[2][2];

    return( (void *) b );}

static inline void *inv_m( double a[3][3], double b[3][3] )
{
    double d = a[0][0]*a[1][1]*a[2][2] + a[0][1]*a[1][2]*a[2][0]
             + a[0][2]*a[1][0]*a[2][1] - a[2][0]*a[1][1]*a[0][2]
             - a[2][1]*a[1][2]*a[0][0] - a[2][2]*a[1][0]*a[0][1];

    if( d == 0 )
    {
        d = 1.0e+015; /* ~ 1 / 1.0e-015 */
    }
    else
    {
        d = 1 / d;
    }

    b[0][0] = (a[1][1]*a[2][2] - a[1][2]*a[2][1]) * d;
    b[0][1] = (a[0][2]*a[2][1] - a[0][1]*a[2][2]) * d;
    b[0][2] = (a[0][1]*a[1][2] - a[0][2]*a[1][1]) * d;

    b[1][0] = (a[1][2]*a[2][0] - a[1][0]*a[2][2]) * d;
    b[1][1] = (a[0][0]*a[2][2] - a[0][2]*a[2][0]) * d;
    b[1][2] = (a[0][2]*a[1][0] - a[0][0]*a[1][2]) * d;

    b[2][0] = (a[1][0]*a[2][1] - a[1][1]*a[2][0]) * d;
    b[2][1] = (a[0][1]*a[2][0] - a[0][0]*a[2][1]) * d;
    b[2][2] = (a[0][0]*a[1][1] - a[0][1]*a[1][0]) * d;

    return( (void *) b );}

static inline void *bal_m( double a[3][3], double b[3][3] )
{
    b[0][0] = (a[0][0] + a[0][0]) / 2;
    b[0][1] = (a[0][1] + a[1][0]) / 2;
    b[0][2] = (a[0][2] + a[2][0]) / 2;

    b[1][0] = (a[1][0] + a[0][1]) / 2;
    b[1][1] = (a[1][1] + a[1][1]) / 2;
    b[1][2] = (a[1][2] + a[2][1]) / 2;

    b[2][0] = (a[2][0] + a[0][2]) / 2;
    b[2][1] = (a[2][1] + a[1][2]) / 2;
    b[2][2] = (a[2][2] + a[2][2]) / 2;

    return( (void *) b );}

/* Initializes the extended Kalman filter.
   Returns 0 if successful, -1 otherwise.
*/
int extended_kalman_init( double period )
{
    if( period < 0.001 )
    {
        fprintf( stderr, "ERROR: invalid EKF period (%f)\n", period );
        return( -1 );
    }

    dt = period;

    /* process noise matrix */
    memset( &q, 0, sizeof( q ) );
    q[0][0] = STD_ATTITUDE;
    q[1][1] = STD_POSITION;
    q[2][2] = STD_VELOCITY;

    /* state transition matrix */
    memset( &f, 0, sizeof( f ) );
    f[0][0] = 1;
    f[1][1] = 1;
    f[1][2] = dt;
    f[2][2] = 1;

    /* identity matrix */
    memset( &i, 0, sizeof( i ) );
    i[0][0] = 1;
    i[1][1] = 1;
    i[2][2] = 1;

    extended_kalman_reset( );
    return( 0 );
}

/* Resets the extended Kalman filter
*/
void extended_kalman_reset( void )
{
    /* clear state vectors */
    memset( &x1, 0, sizeof( x1 ) );
    memset( &x2, 0, sizeof( x2 ) );
    memset( &x3, 0, sizeof( x3 ) );

    /* clear noise matrices */
    memset( &r1, 0, sizeof( r1 ) );
    memset( &r2, 0, sizeof( r2 ) );
    memset( &r3, 0, sizeof( r3 ) );

    /* clear covariance matrices */
    memset( &p1, 0, sizeof( p1 ) );
    memset( &p2, 0, sizeof( p2 ) );
    memset( &p3, 0, sizeof( p3 ) );
}

/* Updates the EKF as well as the given sensor data
*/
void extended_kalman_update( sensor_data_t *data )
{
    double k1[3][3], k2[3][3], k3[3][3], s[3][3], t[3][3];
    double y1[3], y2[3], y3[3], z1[3], z2[3], z3[3];

    /* predict state estimates */
    x1[0] += rotate_body_to_earth_dRoll( data->droll, data->dpitch, data->dyaw ) * dt;
    x1[1] += x1[2] * dt + data->ddx * dt*dt/2;
    x1[2] += data->ddx * dt;

    x2[0] += rotate_body_to_earth_dPitch( data->droll, data->dpitch, data->dyaw ) * dt;
    x2[1] += x2[2] * dt + data->ddy * dt*dt/2;
    x2[2] += data->ddy * dt;

    x3[0] += rotate_body_to_earth_dYaw( data->droll, data->dpitch, data->dyaw ) * dt;
    x3[1] += x3[2] * dt + data->ddz * dt*dt/2;
    x3[2] += data->ddz * dt;

    /* predict error covariances */
    add_m( mul_m( mul_m( f, p1, s ), trp_m( f, t ), p1 ), q, p1 );
    add_m( mul_m( mul_m( f, p2, s ), trp_m( f, t ), p2 ), q, p2 );
    add_m( mul_m( mul_m( f, p3, s ), trp_m( f, t ), p3 ), q, p3 );

    /* compute measurement residuals */
    z1[0] = data->roll  - x1[0];
    z1[1] = data->x     - x1[1];
    z1[2] = data->dx    - x1[2];

    z2[0] = data->pitch - x2[0];
    z2[1] = data->y     - x2[1];
    z2[2] = data->dy    - x2[2];

    z3[0] = data->yaw   - x3[0];
    z3[1] = data->z     - x3[1];
    z3[2] = data->dz    - x3[2];

    /* compute optimal Kalman gains */
    r1[0][0] = rotate_body_to_earth_dRoll( STD_ANG_VEL, STD_ANG_VEL, STD_ANG_VEL ) * dt;
    r1[2][2] = rotate_body_to_earth_X( STD_LIN_ACC, STD_LIN_ACC, STD_LIN_ACC ) * dt;
    r1[1][1] = r1[2][2] * dt/2;

    r2[0][0] = rotate_body_to_earth_dPitch( STD_ANG_VEL, STD_ANG_VEL, STD_ANG_VEL ) * dt;
    r2[2][2] = rotate_body_to_earth_Y( STD_LIN_ACC, STD_LIN_ACC, STD_LIN_ACC ) * dt;
    r2[1][1] = r2[2][2] * dt/2;

    r3[0][0] = rotate_body_to_earth_dYaw( STD_ANG_VEL, STD_ANG_VEL, STD_ANG_VEL ) * dt;
    r3[2][2] = rotate_body_to_earth_Z( STD_LIN_ACC, STD_LIN_ACC, STD_LIN_ACC ) * dt;
    r3[1][1] = r3[2][2] * dt/2;

    mul_m( p1, inv_m( add_m( p1, r1, s ), t ), k1 );
    mul_m( p2, inv_m( add_m( p2, r2, s ), t ), k2 );
    mul_m( p3, inv_m( add_m( p3, r3, s ), t ), k3 );

    /* update state estimates */
    mul_v( k1, z1, y1 );
    mul_v( k2, z2, y2 );
    mul_v( k3, z3, y3 );

    x1[0] += y1[0];
    x1[1] += y1[1];
    x1[2] += y1[2];

    x2[0] += y2[0];
    x2[1] += y2[1];
    x2[2] += y2[2];

    x3[0] += y3[0];
    x3[1] += y3[1];
    x3[2] += y3[2];

    /* update error covariances */
    bal_m( mul_m( sub_m( i, k1, s ), p1, t ), p1 );
    bal_m( mul_m( sub_m( i, k2, s ), p2, t ), p2 );
    bal_m( mul_m( sub_m( i, k3, s ), p3, t ), p3 );

    /* update given sensor data */
    data->roll  = x1[0];
    data->x     = x1[1];
    data->dx    = x1[2];

    data->pitch = x2[0];
    data->y     = x2[1];
    data->dy    = x2[2];

    data->yaw   = x3[0];
    data->z     = x3[1];
    data->dz    = x3[2];
}

/* End of file */
