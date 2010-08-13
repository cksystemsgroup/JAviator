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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
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

/* Returns the cosinus of the set Roll/Pitch/Yaw angle
*/
double transformation_get_cos_Roll( void )
{
    return( cr );
}
double transformation_get_cos_Pitch( void )
{
    return( cp );
}
double transformation_get_cos_Yaw( void )
{
    return( cy );
}

/* Returns the sinus of the set Roll/Pitch/Yaw angle
*/
double transformation_get_sin_Roll( void )
{
    return( sr );
}
double transformation_get_sin_Pitch( void )
{
    return( sp );
}
double transformation_get_sin_Yaw( void )
{
    return( sy );
}

/* Returns the X/Y/Z component transformed to the earth-fixed frame
*/
double rotate_body_to_earth_X( double x, double y, double z )
{
    return( x * r11 + y * r12 + z * r13 );
}
double rotate_body_to_earth_Y( double x, double y, double z )
{
    return( x * r21 + y * r22 + z * r23 );
}
double rotate_body_to_earth_Z( double x, double y, double z )
{
    return( x * r31 + y * r32 + z * r33 );
}

/* Returns the X/Y/Z component transformed to the body-fixed frame
*/ 
double rotate_earth_to_body_X( double x, double y, double z )
{
    return( x * r11 + y * r21 + z * r31 );
}
double rotate_earth_to_body_Y( double x, double y, double z )
{
    return( x * r12 + y * r22 + z * r32 );
}
double rotate_earth_to_body_Z( double x, double y, double z )
{
    return( x * r13 + y * r23 + z * r33 );
}

/* Returns the dRoll/dPitch/dYaw component transformed to the earth-fixed frame
*/ 
double rotate_body_to_earth_dRoll( double dr, double dp, double dy )
{
    return( dr * s11 + dp * s12 + dy * s13 );
}
double rotate_body_to_earth_dPitch( double dr, double dp, double dy )
{
    return( dr * s21 + dp * s22 + dy * s23 );
}
double rotate_body_to_earth_dYaw( double dr, double dp, double dy )
{
    return( dr * s31 + dp * s32 + dy * s33 );
}

/* Returns the dRoll/dPitch/dYaw component transformed to the body-fixed frame
*/ 
double rotate_earth_to_body_dRoll( double dr, double dp, double dy )
{
    return( dr * s11 + dp * s21 + dy * s31 );
}
double rotate_earth_to_body_dPitch( double dr, double dp, double dy )
{
    return( dr * s12 + dp * s22 + dy * s32 );
}
double rotate_earth_to_body_dYaw( double dr, double dp, double dy )
{
    return( dr * s13 + dp * s23 + dy * s33 );
}

/* End of file */
