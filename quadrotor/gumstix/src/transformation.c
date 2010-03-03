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

static double c_r, c_p, c_y;
static double s_r, s_p, s_y;
static double r11, r12, r13;
static double r21, r22, r23;
static double r31, r32, r33;


void transformation_set_angles( double roll, double pitch, double yaw )
{
    /* compute all cosine and sine values */
    c_r = cos( roll );
    c_p = cos( pitch );
    c_y = cos( yaw );

    s_r = sin( roll );
    s_p = sin( pitch );
    s_y = sin( yaw );

    /* update the rotation matrix entries */
    r11 = c_p * c_y;
    r12 = c_y * s_r * s_p - c_r * s_y;
    r13 = c_r * c_y * s_p + s_r * s_y;

    r21 = c_p * s_y;
    r22 = s_r * s_p * s_y + c_r * c_y;
    r23 = c_r * s_p * s_y - c_y * s_r;

    r31 = -s_p;
    r32 = c_p * s_r;
    r33 = c_r * c_p;
}

double transformation_get_cosR( void )
{
    return( c_r );
}

double transformation_get_cosP( void )
{
    return( c_p );
}

double transformation_get_cosY( void )
{
    return( c_y );
}

double transformation_get_sinR( void )
{
    return( s_r );
}

double transformation_get_sinP( void )
{
    return( s_p );
}

double transformation_get_sinY( void )
{
    return( s_y );
}

double transformation_rotate_X( double x, double y, double z )
{
    return( x * r11 + y * r12 + z * r13 );
}

double transformation_rotate_Y( double x, double y, double z )
{
    return( x * r21 + y * r22 + z * r23 );
}

double transformation_rotate_Z( double x, double y, double z )
{
    return( x * r31 + y * r32 + z * r33 );
}

/* End of file */
