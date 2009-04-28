/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   protocol.h     Definition of the communication protocol.                */
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

#ifndef PROTOCOL_H
#define PROTOCOL_H


/*****************************************************************************/
/*                                                                           */
/*   Communication Protocol                                                  */
/*                                                                           */
/*****************************************************************************/

/* Communication overhead:
   2 packet mark bytes + 2 header bytes + 2 checksum bytes
*/
#define COMM_OVERHEAD       6

/* Communication buffer size:
   recommended size for holding largest data packet
*/
#define COMM_BUF_SIZE       64

/* Message identifiers
*/
#define COMM_GET_NAME       0x01    /* request to send helicopter name */
#define COMM_ACK_NAME       0x02    /* transmission of helicopter name */
#define COMM_SHUT_DOWN      0x03    /* request to shut down helicopter */
#define COMM_TEST_MODE      0x04    /* request to perform a self-test */
#define COMM_LOG_DATA       0x05    /* request to log all sampled data */
#define COMM_IDLE_LIMIT     0x06    /* transmission of new idle limit */
#define COMM_ALT_LIMIT      0x07    /* transmission of new range limit */
#define COMM_SENSOR_DATA    0x08    /* transmission of sensor data */
#define COMM_PWM_SIGNALS    0x09    /* transmission of actuator data */
#define COMM_MOTOR_OFFSETS  0x0A    /* transmission of motor offsets */
#define COMM_NAVI_DATA      0x0B    /* transmission of navigation data */
#define COMM_HELI_STATE     0x0C    /* transmission of helicopter state */
#define COMM_STREAM         0x0D    /* transmission of arbitrary stream */
#define COMM_GROUND_REPORT  0x0E    /* transmission of ground report */
#define COMM_TRIM_DATA      0x0F    /* transmission of trim data */
#define COMM_CTRL_PARAMS    0x10    /* transmission of control params */
#define COMM_EN_SENSORS     0x11    /* enable/disable senors */
#define COMM_SWITCH_MODE    0x12    /* request to perform a mode switch */
#define COMM_PACKET_LIMIT   0x12    /* limit of valid packet types */
#define COMM_PACKET_MARK    0xFF    /* used to mark a packet's begin */


#endif /* !PROTOCOL_H */

/* End of file */
