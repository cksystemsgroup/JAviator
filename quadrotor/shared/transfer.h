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

#if 0
/* Structure for shared JAviator data */
typedef struct
{
    uint32_t    pos_x;              /* [?] range ? */
    uint32_t    pos_y;              /* [?] range ? */
    uint32_t    laser;              /* [1/10mm] range 0...5000000 */
    uint16_t    sonar;              /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    pressure;           /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    battery;            /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    state;              /* JAviator state indicator */
    uint16_t    error;              /* JAviator error indicator */
    uint16_t    id;                 /* transmisson ID */

} javiator_data_t;

#define JAVIATOR_DATA_SIZE  24      /* byte size of javiator_data_t */
#endif

/* Structure for shared JAviator data */
typedef struct
{
    uint16_t    roll;               /*                        | roll    |                       */
    uint16_t    pitch;              /* [units] --> [mrad]   = | pitch   | * 2000 * PI / 65536   */
    uint16_t    yaw;                /*                        | yaw     |                       */
    uint16_t    droll;              /*                        | droll   |                       */
    uint16_t    dpitch;             /* [units] --> [mrad/s] = | dpitch  | * 8500 / 32768        */
    uint16_t    dyaw;               /*                        | dyaw    |                       */
    uint16_t    ddx;                /*                        | ddx     |                       */
    uint16_t    ddy;                /* [units] --> [mm/s^2] = | ddy     | * 9810 * 7 / 32768    */
    uint16_t    ddz;                /*                        | ddz     |                       */
    uint16_t    ticks;              /* [units] --> [s]      = | ticks   | * 65536 / 10000000    */
    uint16_t    sonar;              /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    pressure;           /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    battery;            /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    state;              /* JAviator state indicator */
    uint16_t    error;              /* JAviator error indicator */
    uint16_t    id;                 /* transmisson ID */

} javiator_data_t;

#define JAVIATOR_DATA_SIZE  32      /* byte size of javiator_data_t */

/* Structure for shared motor signals */
typedef struct
{
    uint16_t    front;              /* [units] If Fast PWM Mode enabled,  */
    uint16_t    right;              /* [units] then range 0...16000,      */
    uint16_t    rear;               /* [units] if Fast PWM Mode disabled, */
    uint16_t    left;               /* [units] then range 0...1000.       */

} motor_signals_t;

#define MOTOR_SIGNALS_SIZE  8       /* byte size of motor_signals_t */


#endif /* !TRANSFER_H */

/* End of file */
