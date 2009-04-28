/* $Id: control_loop.h,v 1.17 2008/12/18 15:07:18 rtrummer Exp $ */

/* 
 * Copyright (c) Harald Roeck hroeck@cs.uni-salzburg.at 
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

#include "channel.h"

#define    ADJUSTING_ROLL           0x01
#define    ADJUSTING_PITCH          0x02
#define    ADJUSTING_YAW            0x04
#define    ADJUSTING_Z              0x08


//plant parameters
#define    GRAVITY           9.81   //!< The value of gravitational acceleration in [m/s^2]
#define    JAVIATOR_M        2.2    //!< The weight of the JAviator in [Kg]
#define    THRUST_DELAY      0.1    //!< The delay in rotors response in [s]

int control_loop_setup(int period, int automatic_altitude);
int control_loop_run();
int control_loop_stop();

#endif /* CONTROL_LOOP_H */
