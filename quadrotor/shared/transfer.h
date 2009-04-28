/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   transfer.h     Definition of Gumstix-Robostix-shared data types.        */
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
/*   Robostix/Gumstix Definitions                                            */
/*                                                                           */
/*****************************************************************************/

/* Controller period */
#define CONTROLLER_PERIOD   14  /* [ms] */

/* Robostix States */
#define RS_SHUT_DOWN_MODE   1   /* device is in shut-down mode */
#define RS_NEW_LASER_DATA   2   /* laser data have been updated */

/* Robostix Errors */
#define RE_RECEIVE_PACKET   1   /* error while receiving packet */
#define RE_INVALID_DATA     2   /* invalid data and/or checksum */
#define RE_INVALID_SIZE     4   /* invalid size of received data */
#define RE_LASER_GET_DATA   8   /* error while receiving laser data */
#define RE_OUT_OF_RANGE     16  /* one or more signals out of range */

/* Structure for shared Gumstix data */
typedef struct
{
    int16_t     front;          /* [units] if Fast PWM Mode enabled, */
    int16_t     right;          /* [units] then range 0...16000, */
    int16_t     rear;           /* [units] if Fast PWM Mode disabled, */
    int16_t     left;           /* [units] then range 0...1000. */

} pwm_signals_t;

#define PWM_SIGNALS_SIZE    8   /* byte size of pwm_signals_t */

/* Structure for shared Robostix data */
typedef struct
{
    uint32_t    laser;          /* [1/10mm] range 0...5000000 */
    uint16_t    sonar;          /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    pressure;       /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    battery;        /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    sequence;       /* incremented with each send operation */
    uint8_t     state;          /* controller state indicator */
    uint8_t     error;          /* controller error indicator */

} sensor_data_t;

#define SENSOR_DATA_SIZE    14  /* byte size of sensor_data_t */


#endif /* !TRANSFER_H */

/* End of file */
