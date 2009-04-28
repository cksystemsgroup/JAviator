/* $Id: trim.c,v 1.1 2008/10/16 14:41:13 rtrummer Exp $ */

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

#include "trim.h"

#include <stdio.h>

#define PACKET_LENGTH TRIM_PACKET_LENGTH

int trim_to_stream(const struct trim_data *values, char *buf, int buf_len)
{
    int offset = 0;
    if (buf_len == PACKET_LENGTH + offset) {
        int i;
        short tmp[buf_len / 2];

        tmp[0] = (short) values->roll;
        tmp[1] = (short) values->pitch;

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

int trim_from_stream(struct trim_data *values, const char *buf, int buf_len)
{
    int offset = 0;
    if (values && buf_len == PACKET_LENGTH + offset) {

        values->roll  = (int)((short)(buf[offset + 0] << 8) | (short)(buf[offset + 1] & 0xFF));
        values->pitch = (int)((short)(buf[offset + 2] << 8) | (short)(buf[offset + 3] & 0xFF));

        return 0;
    } else {
        fprintf(stderr, "ERROR: in %s %d invalid packet length\n", __FILE__, __LINE__);
    }

    return -1;
}
