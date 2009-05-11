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

#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H


/* Plant parameters */
#define GRAVITY         9.81    /* [m/s^2] gravitational acceleration */
#define JAVIATOR_MASS   2.3     /* [kg] total mass of the JAviator */
#define THRUST_DELAY    0.1     /* [s] delay in response of rotors */


int control_loop_setup( int period, int control_z );

int control_loop_run( void );

int control_loop_stop( void );


#endif /* !CONTROL_LOOP_H */

/* End of file */
