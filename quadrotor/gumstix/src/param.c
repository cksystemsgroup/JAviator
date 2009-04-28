/* $Id: param.c,v 1.3 2008/10/28 09:06:44 rtrummer Exp $ */

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

#include "param.h"

#include <stdio.h>

#define PACKET_LENGTH PARAM_PACKET_LENGTH

int params_to_stream(const struct parameters *values, char *buf, int buf_len)
{
    int offset = 0;
    if (buf_len == PACKET_LENGTH + offset) {
        int i;
        short tmp[buf_len / 2];

        tmp[0]  = (short) values->rollpitch_Kp;
        tmp[1]  = (short) values->rollpitch_Ki;
        tmp[2]  = (short) values->rollpitch_Kd;
        tmp[3]  = (short) values->rollpitch_Kdd;
        tmp[4]  = (short) values->yaw_Kp;
        tmp[5]  = (short) values->yaw_Ki;
        tmp[6]  = (short) values->yaw_Kd;
        tmp[7]  = (short) values->yaw_Kdd;
        tmp[8]  = (short) values->altitude_Kp;
        tmp[9]  = (short) values->altitude_Ki;
        tmp[10] = (short) values->altitude_Kd;
        tmp[11] = (short) values->altitude_Kdd;
        tmp[12] = (short) values->x_y_Kp;
        tmp[13] = (short) values->x_y_Ki;
        tmp[14] = (short) values->x_y_Kd;
        tmp[15] = (short) values->x_y_Kdd;

        for (i = 0; i < buf_len/2; ++i) {
            buf[offset + 2 * i] = (char) (tmp[i] >> 8);
            buf[offset + 2 * i + 1] = (char) (tmp[i]);
        }
        return 0;
    } else {
        fprintf(stderr, "ERROR: in %s %d invalid packet length\n", __FILE__, __LINE__);
    }
    return -1;
}

int params_from_stream(struct parameters *values, const char *buf, int buf_len)
{
    int offset = 0;
    if (values && buf_len == PACKET_LENGTH + offset) {

        values->rollpitch_Kp  = (int)((short)(buf[offset +  0] << 8) | (short)(buf[offset +  1] & 0xFF));
        values->rollpitch_Ki  = (int)((short)(buf[offset +  2] << 8) | (short)(buf[offset +  3] & 0xFF));
        values->rollpitch_Kd  = (int)((short)(buf[offset +  4] << 8) | (short)(buf[offset +  5] & 0xFF));
        values->rollpitch_Kdd = (int)((short)(buf[offset +  6] << 8) | (short)(buf[offset +  7] & 0xFF));
        values->yaw_Kp        = (int)((short)(buf[offset +  8] << 8) | (short)(buf[offset +  9] & 0xFF));
        values->yaw_Ki        = (int)((short)(buf[offset + 10] << 8) | (short)(buf[offset + 11] & 0xFF));
        values->yaw_Kd        = (int)((short)(buf[offset + 12] << 8) | (short)(buf[offset + 13] & 0xFF));
        values->yaw_Kdd       = (int)((short)(buf[offset + 14] << 8) | (short)(buf[offset + 15] & 0xFF));
        values->altitude_Kp   = (int)((short)(buf[offset + 16] << 8) | (short)(buf[offset + 17] & 0xFF));
        values->altitude_Ki   = (int)((short)(buf[offset + 18] << 8) | (short)(buf[offset + 19] & 0xFF));
        values->altitude_Kd   = (int)((short)(buf[offset + 20] << 8) | (short)(buf[offset + 21] & 0xFF));
        values->altitude_Kdd  = (int)((short)(buf[offset + 22] << 8) | (short)(buf[offset + 23] & 0xFF));
        values->x_y_Kp        = (int)((short)(buf[offset + 24] << 8) | (short)(buf[offset + 25] & 0xFF));
        values->x_y_Ki        = (int)((short)(buf[offset + 26] << 8) | (short)(buf[offset + 27] & 0xFF));
        values->x_y_Kd        = (int)((short)(buf[offset + 28] << 8) | (short)(buf[offset + 29] & 0xFF));
        values->x_y_Kdd       = (int)((short)(buf[offset + 30] << 8) | (short)(buf[offset + 31] & 0xFF));

        return 0;
    } else {
        fprintf(stderr, "ERROR: in %s %d invalid packet length\n", __FILE__, __LINE__);
    }

    return -1;
}
