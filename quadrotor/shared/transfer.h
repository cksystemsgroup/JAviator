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


/*****************************************************************************/
/*                                                                           */
/*   Shared Definitions                                                      */
/*                                                                           */
/*****************************************************************************/

/* Controller period */
#define CONTROLLER_PERIOD   14      /* [ms] */

/* JAviator States */
#define JS_PERIOD_UPDATED   0x0001  /* controller period has been updated */
#define JS_SHUT_DOWN_MODE   0x0002  /* JAviator is in shut-down mode */
#define JS_NEW_IMU_DATA     0x0004  /* IMU data have been updated */
#define JS_NEW_LASER_DATA   0x0008  /* laser data have been updated */
#define JS_NEW_SONAR_DATA   0x0010  /* sonar data have been updated */
#define JS_NEW_PRESS_DATA   0x0020  /* pressure data have been updated */
#define JS_NEW_BATT_DATA    0x0040  /* battery data have been updated */

/* JAviator Errors */
#define JE_RECEIVE_PACKET   0x0001  /* error while receiving packet */
#define JE_INVALID_DATA     0x0002  /* invalid data and/or checksum */
#define JE_UNKNOWN_TYPE     0x0004  /* unknown or invalid packet type */
#define JE_INVALID_SIZE     0x0008  /* invalid size of received data */
#define JE_OUT_OF_RANGE     0x0010  /* one or more signals out of range */
#define JE_IMU_GET_DATA     0x0020  /* error while receiving IMU data */
#define JE_LASER_GET_DATA   0x0040  /* error while receiving laser data */
#define JE_SONAR_GET_DATA   0x0080  /* error while receiving sonar data */
#define JE_PRESS_GET_DATA   0x0100  /* error while receiving pressure data */
#define JE_BATT_GET_DATA    0x0200  /* error while receiving battery data */

#include "shared/javiator_data.h"   /* definition of JAviator data structure */
#include "shared/motor_signals.h"   /* definition of motor signals structure */


#endif /* !TRANSFER_H */

/* End of file */
