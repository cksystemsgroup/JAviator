/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   transfer.h     Definition of shared constants and data types.           */
/*                                                                           */
/*   Copyright (c) Rainer Trummer rtrummer@cs.uni-salzburg.at                */
/*                                                                           */
/*   This program is free software; you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by    */
/*   the Free Software Foundation; either version 2 of the License, or       */
/*   (at your option) any later version.                                     */
/*                                                                           */
/*   This program is distributed in the hope that it will be useful,         */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*   GNU General Public License for more details.                            */
/*                                                                           */
/*   You should have received a copy of the GNU General Public License       */
/*   along with this program; if not, write to the Free Software Foundation, */
/*   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.      */
/*                                                                           */
/*****************************************************************************/

#ifndef TRANSFER_H
#define TRANSFER_H

#include <stdint.h>


/*****************************************************************************/
/*                                                                           */
/*   Shared Definitions                                                      */
/*                                                                           */
/*****************************************************************************/

/* Controller period */
#define CONTROLLER_PERIOD   14      /* [ms] */

/* JAviator states */
#define JS_NEW_PRESS_DATA   0x0001  /* pressure data have been updated */
#define JS_NEW_SONAR_DATA   0x0002  /* sonar data have been updated */
#define JS_NEW_BATT_DATA    0x0004  /* battery data have been updated */
#define JS_NEW_X_POS_DATA   0x0008  /* x-position data have been updated */
#define JS_NEW_Y_POS_DATA   0x0010  /* y-position data have been updated */

#include "javiator_data.h"          /* definition of JAviator data structure */
#include "motor_signals.h"          /* definition of motor signals structure */


#endif /* !TRANSFER_H */

/* End of file */
