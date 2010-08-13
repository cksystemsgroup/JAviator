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

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

/* Initializes the transformation variables
*/
void   transformation_init( void );

/* Sets the rotation matrix to the given angles
*/
void   transformation_set_angles( double roll, double pitch, double yaw );

/* Returns the cosinus of the set Roll/Pitch/Yaw angle
*/
double transformation_get_cos_Roll ( void );
double transformation_get_cos_Pitch( void );
double transformation_get_cos_Yaw  ( void );

/* Returns the sinus of the set Roll/Pitch/Yaw angle
*/
double transformation_get_sin_Roll ( void );
double transformation_get_sin_Pitch( void );
double transformation_get_sin_Yaw  ( void );

/* Returns the X/Y/Z component transformed to the earth-fixed frame
*/ 
double rotate_body_to_earth_X( double x, double y, double z );
double rotate_body_to_earth_Y( double x, double y, double z );
double rotate_body_to_earth_Z( double x, double y, double z );

/* Returns the X/Y/Z component transformed to the body-fixed frame
*/ 
double rotate_earth_to_body_X( double x, double y, double z );
double rotate_earth_to_body_Y( double x, double y, double z );
double rotate_earth_to_body_Z( double x, double y, double z );

/* Returns the dRoll/dPitch/dYaw component transformed to the earth-fixed frame
*/ 
double rotate_body_to_earth_dRoll ( double dr, double dp, double dy );
double rotate_body_to_earth_dPitch( double dr, double dp, double dy );
double rotate_body_to_earth_dYaw  ( double dr, double dp, double dy );

/* Returns the dRoll/dPitch/dYaw component transformed to the body-fixed frame
*/ 
double rotate_earth_to_body_dRoll ( double dr, double dp, double dy );
double rotate_earth_to_body_dPitch( double dr, double dp, double dy );
double rotate_earth_to_body_dYaw  ( double dr, double dp, double dy );

#endif /* !TRANSFORMATION_H */

/* End of file */
