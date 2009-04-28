/* $Id: navigation.c,v 1.2 2008/11/11 19:28:40 hroeck Exp $ */

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

#include "controller.h"
#include "navigation.h"

#include <stdio.h>

#define PACKET_LENGTH 8

int navigation_to_stream(const struct navigation_data *navi, char *buf, int len)
{
    int offset = 0;
    if (len == PACKET_LENGTH) {
        int i;
        short tmp[len / 2];

        tmp[0] = (short) navi->roll;
        tmp[1] = (short) navi->pitch;
        tmp[2] = (short) navi->yaw;
        tmp[3] = (short) navi->z;

        for (i = 0; i < len/2; ++i) {
            buf[offset + 2 * i] = (char) (tmp[i] >> 8);
            buf[offset + 2 * i + 1] = (char) (tmp[i]);
        }
        return 0;
    } else {
        fprintf(stderr, "ERROR: in %s %d invalid packet length\n", __FILE__, __LINE__);
    }
    return -1;

}

int navigation_from_stream(struct navigation_data *navi, const char *buf, int len)
{
    int offset = 0;
    if (len == PACKET_LENGTH + offset) {

        navi->roll  = (int)((short)(buf[offset + 0] << 8) | (short)(buf[offset + 1] & 0xFF));
        navi->pitch = (int)((short)(buf[offset + 2] << 8) | (short)(buf[offset + 3] & 0xFF));
        navi->yaw   = (int)((short)(buf[offset + 4] << 8) | (short)(buf[offset + 5] & 0xFF));
        navi->z     = (int)((short)(buf[offset + 6] << 8) | (short)(buf[offset + 7] & 0xFF));

        return 0;
    } else {
        fprintf(stderr, "ERROR: in %s %d invalid packet length\n", __FILE__, __LINE__);
    }
    return -1;
}
