/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   Copyright (c) 2006-2013 Harald Roeck <harald.roeck@gmail.com>           */
/*                       and Rainer Trummer <rainer.trummer@gmail.com>       */
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

package javiator.util;

public final class PacketType
{
    public static final byte COMM_CTRL_PERIOD   = 0x01; /* transmission of controller period */
    public static final byte COMM_JAVIATOR_DATA = 0x02;	/* transmission of JAviator data */
    public static final byte COMM_INERTIAL_DATA = 0x03; /* transmission of inertial data */
    public static final byte COMM_SENSOR_DATA   = 0x04; /* transmission of sensor data */
    public static final byte COMM_MOTOR_SIGNALS = 0x05; /* transmission of motor signals */
    public static final byte COMM_MOTOR_OFFSETS = 0x06; /* transmission of motor offsets */
    public static final byte COMM_STATE_MODE    = 0x07; /* transmission of mode and state */
    public static final byte COMM_GROUND_REPORT = 0x08; /* transmission of ground report */
    public static final byte COMM_TRACE_DATA    = 0x09; /* transmission of trace data */
    public static final byte COMM_COMMAND_DATA  = 0x0A; /* transmission of command data */
    public static final byte COMM_R_P_PARAMS    = 0x0B; /* transmission of roll/pitch params */
    public static final byte COMM_YAW_PARAMS    = 0x0C; /* transmission of yaw params */
    public static final byte COMM_ALT_PARAMS    = 0x0D; /* transmission of altitude params */
    public static final byte COMM_X_Y_PARAMS    = 0x0E; /* transmission of x/y params */
    public static final byte COMM_IDLE_LIMIT    = 0x0F; /* transmission of idle limit */
    public static final byte COMM_STREAM        = 0x10; /* transmission of arbitrary stream */
    public static final byte COMM_SWITCH_STATE  = 0x11; /* request to perform a state switch */
    public static final byte COMM_SWITCH_MODE   = 0x12; /* request to perform a mode switch */
    public static final byte COMM_STORE_TRIM    = 0x13; /* request to store trim values */
    public static final byte COMM_CLEAR_TRIM    = 0x14; /* request to clear trim values */
    public static final byte COMM_SHUT_DOWN     = 0x15; /* request to perform a shut-down */
    public static final byte COMM_EN_SENSORS    = 0x16; /* request to enable/disable senors */
    public static final byte COMM_PACKET_LIMIT  = 0x16; /* limit of valid packet identifiers */
    public static final byte COMM_PACKET_MARK   = (-1); /* used to mark a packet's begin */
}

/* End of file */