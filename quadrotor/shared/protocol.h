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

/* Communication overhead:
   2 packet mark bytes + 2 header bytes + 2 checksum bytes
*/
#define COMM_OVERHEAD       6

/* Communication buffer size:
   recommended size for holding largest data packet

   E.g., sending the ground report (see gumstix/src/terminal_port.c):
   COMM_OVERHEAD + SENSOR_DATA_SIZE + MOTOR_SIGNALS_SIZE +
   MOTOR_OFFSETS_SIZE + STATE_MODE_SIZE  >  64 (!)
*/
#define COMM_BUF_SIZE       128

/* Packet identifiers
*/
#define COMM_CTRL_PERIOD    0x01    /* transmission of controller period */
#define COMM_JAVIATOR_DATA  0x02    /* transmission of JAviator data */
#define COMM_INERTIAL_DATA  0x03    /* transmission of inertial data */
#define COMM_SENSOR_DATA    0x04    /* transmission of sensor data */
#define COMM_MOTOR_SIGNALS  0x05    /* transmission of motor signals */
#define COMM_MOTOR_OFFSETS  0x06    /* transmission of motor offsets */
#define COMM_STATE_MODE     0x07    /* transmission of mode and state */
#define COMM_GROUND_REPORT  0x08    /* transmission of ground report */
#define COMM_TRACE_DATA     0x09    /* transmission of trace data */
#define COMM_COMMAND_DATA   0x0A    /* transmission of command data */
#define COMM_R_P_PARAMS     0x0B    /* transmission of roll/pitch params */
#define COMM_YAW_PARAMS     0x0C    /* transmission of yaw params */
#define COMM_ALT_PARAMS     0x0D    /* transmission of altitude params */
#define COMM_X_Y_PARAMS     0x0E    /* transmission of x/y params */
#define COMM_IDLE_LIMIT     0x0F    /* transmission of idle limit */
#define COMM_STREAM         0x10    /* transmission of arbitrary stream */
#define COMM_SWITCH_STATE   0x11    /* request to perform a state switch */
#define COMM_SWITCH_MODE    0x12    /* request to perform a mode switch */
#define COMM_STORE_TRIM     0x13    /* request to store trim values */
#define COMM_CLEAR_TRIM     0x14    /* request to clear trim values */
#define COMM_SHUT_DOWN      0x15    /* request to perform a shut-down */
#define COMM_EN_SENSORS     0x16    /* request to enable/disable senors */
#define COMM_PACKET_LIMIT   0x16    /* limit of valid packet identifiers */
#define COMM_PACKET_MARK    0xFF    /* used to mark a packet's begin */

#endif /* !PROTOCOL_H */

/* End of file */
