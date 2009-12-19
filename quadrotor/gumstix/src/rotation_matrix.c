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

#include <math.h>
#include "rotation_matrix.h"

static double r11, r12, r13;
static double r21, r22, r23;
static double r31, r32, r33;


void rotation_matrix_update( double roll, double pitch, double yaw )
{
	/* prepare all cosine and sine values */
	double cr = cos( roll );
	double cp = cos( pitch );
	double cy = cos( yaw );

	double sr = sin( roll );
	double sp = sin( pitch );
	double sy = sin( yaw );

    /* update the rotation matrix entries */
    r11 = cp * cy;
    r12 = cy * sr * sp - cr * sy;
    r13 = cr * cy * sp + sr * sy;

    r21 = cp * sy;
    r22 = sr * sp * sy + cr * cy;
    r23 = cr * sp * sy - cy * sr;

    r31 = -sp;
    r32 = cp * sr;
    r33 = cr * cp;
}

double rotation_matrix_rotate_x( double x, double y, double z )
{
    return( x * r11 + y * r12 + z * r13 );
}

double rotation_matrix_rotate_y( double x, double y, double z )
{
    return( x * r21 + y * r22 + z * r23 );
}

double rotation_matrix_rotate_z( double x, double y, double z )
{
    return( x * r31 + y * r32 + z * r33 );
}

/* End of file */
