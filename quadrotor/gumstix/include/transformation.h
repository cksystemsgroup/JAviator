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

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

/* Initializes the transformation variables
*/
void   transformation_init( void );

/* Sets the rotation matrix to the given angles
*/
void   transformation_set_angles( double roll, double pitch, double yaw );

/* Returns the cosinus of the set Roll angle
*/
double transformation_get_cos_Roll( void );

/* Returns the cosinus of the set Pitch angle
*/
double transformation_get_cos_Pitch( void );

/* Returns the cosinus of the set Yaw angle
*/
double transformation_get_cos_Yaw( void );

/* Returns the sinus of the set Roll angle
*/
double transformation_get_sin_Roll( void );

/* Returns the sinus of the set Pitch angle
*/
double transformation_get_sin_Pitch( void );

/* Returns the sinus of the set Yaw angle
*/
double transformation_get_sin_Yaw( void );

/* Returns the X component transformed to the global frame
*/ 
double rotate_local_to_global_X( double x, double y, double z );

/* Returns the Y component transformed to the global frame
*/ 
double rotate_local_to_global_Y( double x, double y, double z );

/* Returns the Z component transformed to the global frame
*/ 
double rotate_local_to_global_Z( double x, double y, double z );

/* Returns the X component transformed to the local frame
*/ 
double rotate_global_to_local_X( double x, double y, double z );

/* Returns the Y component transformed to the local frame
*/ 
double rotate_global_to_local_Y( double x, double y, double z );

/* Returns the Z component transformed to the local frame
*/ 
double rotate_global_to_local_Z( double x, double y, double z );

/* Returns the dRoll component transformed to the global frame
*/ 
double rotate_local_to_global_dRoll( double dr, double dp, double dy );

/* Returns the dPitch component transformed to the global frame
*/ 
double rotate_local_to_global_dPitch( double dr, double dp, double dy );

/* Returns the dYaw component transformed to the global frame
*/ 
double rotate_local_to_global_dYaw( double dr, double dp, double dy );

/* Returns the dRoll component transformed to the local frame
*/ 
double rotate_global_to_local_dRoll( double dr, double dp, double dy );

/* Returns the dPitch component transformed to the local frame
*/ 
double rotate_global_to_local_dPitch( double dr, double dp, double dy );

/* Returns the dYaw component transformed to the local frame
*/ 
double rotate_global_to_local_dYaw( double dr, double dp, double dy );

#endif /* !TRANSFORMATION_H */

/* End of file */
