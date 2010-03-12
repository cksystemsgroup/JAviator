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

#include <math.h>

#include "transformation.h"

static double cr,  cp,  cy;
static double sr,  sp,  sy;
static double r11, r12, r13;
static double r21, r22, r23;
static double r31, r32, r33;
static double s11, s12, s13;
static double s21, s22, s23;
static double s31, s32, s33;


/* Initializes the transformation variables
*/
void transformation_init( void )
{
    cr  = cp  = cy  = 0;
    sr  = sp  = sy  = 0;

    r11 = r12 = r13 = 0;
    r21 = r22 = r23 = 0;
    r31 = r32 = r33 = 0;

    s11 = s12 = s13 = 0;
    s21 = s22 = s23 = 0;
    s31 = s32 = s33 = 0;
}

/* Sets the rotation matrix to the given angles
*/
void transformation_set_angles( double roll, double pitch, double yaw )
{
    cr  = cos( roll );
    cp  = cos( pitch );
    cy  = cos( yaw );

    sr  = sin( roll );
    sp  = sin( pitch );
    sy  = sin( yaw );

    r11 = cp * cy;
    r12 = cy * sr * sp - cr * sy;
    r13 = cr * cy * sp + sr * sy;

    r21 = cp * sy;
    r22 = sr * sp * sy + cr * cy;
    r23 = cr * sp * sy - cy * sr;

    r31 = -sp;
    r32 = cp * sr;
    r33 = cr * cp;

    s11 = 1;
    s12 = sr * sp / cp;
    s13 = cr * sp / cp;

    s21 = 0;
    s22 = cr;
    s23 = -sr;

    s31 = 0;
    s32 = sr / cp;
    s33 = cr / cp;
}

/* Returns the cosinus of the set Roll angle
*/
double transformation_get_cos_R( void )
{
    return( cr );
}

/* Returns the cosinus of the set Pitch angle
*/
double transformation_get_cos_P( void )
{
    return( cp );
}

/* Returns the cosinus of the set Yaw angle
*/
double transformation_get_cos_Y( void )
{
    return( cy );
}

/* Returns the sinus of the set Roll angle
*/
double transformation_get_sin_R( void )
{
    return( sr );
}

/* Returns the sinus of the set Pitch angle
*/
double transformation_get_sin_P( void )
{
    return( sp );
}

/* Returns the sinus of the set Yaw angle
*/
double transformation_get_sin_Y( void )
{
    return( sy );
}

/* Returns the X component transformed to the global frame
*/
double rotate_local_to_global_X( double x, double y, double z )
{
    return( x * r11 + y * r12 + z * r13 );
}

/* Returns the Y component transformed to the global frame
*/
double rotate_local_to_global_Y( double x, double y, double z )
{
    return( x * r21 + y * r22 + z * r23 );
}

/* Returns the Z component transformed to the global frame
*/
double rotate_local_to_global_Z( double x, double y, double z )
{
    return( x * r31 + y * r32 + z * r33 );
}

/* Returns the X component transformed to the local frame
*/ 
double rotate_global_to_local_X( double x, double y, double z )
{
    return( x * r11 + y * r21 + z * r31 );
}

/* Returns the Y component transformed to the local frame
*/ 
double rotate_global_to_local_Y( double x, double y, double z )
{
    return( x * r12 + y * r22 + z * r32 );
}

/* Returns the Z component transformed to the local frame
*/ 
double rotate_global_to_local_Z( double x, double y, double z )
{
    return( x * r13 + y * r23 + z * r33 );
}

/* Returns the dRoll component transformed to the global frame
*/ 
double rotate_local_to_global_dR( double dr, double dp, double dy )
{
    return( dr * s11 + dp * s12 + dy * s13 );
}

/* Returns the dPitch component transformed to the global frame
*/ 
double rotate_local_to_global_dP( double dr, double dp, double dy )
{
    return( dr * s21 + dp * s22 + dy * s23 );
}

/* Returns the dYaw component transformed to the global frame
*/ 
double rotate_local_to_global_dY( double dr, double dp, double dy )
{
    return( dr * s31 + dp * s32 + dy * s33 );
}

/* Returns the dRoll component transformed to the local frame
*/ 
double rotate_global_to_local_dR( double dr, double dp, double dy )
{
    return( dr * s11 + dp * s21 + dy * s31 );
}

/* Returns the dPitch component transformed to the local frame
*/ 
double rotate_global_to_local_dP( double dr, double dp, double dy )
{
    return( dr * s12 + dp * s22 + dy * s32 );
}

/* Returns the dYaw component transformed to the local frame
*/ 
double rotate_global_to_local_dY( double dr, double dp, double dy )
{
    return( dr * s13 + dp * s23 + dy * s33 );
}

/* End of file */
