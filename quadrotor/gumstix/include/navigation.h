/* $Id: navigation.h,v 1.2 2008/11/07 10:12:45 rtrummer Exp $ */

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

#ifndef NAVIGATION_H
#define NAVIGATION_H

#define NAVIGATION_PACKET_LENGTH  8

struct navigation_data {
    int roll;
    int pitch;
    int yaw;
    int z;
};


#define NAVIGATION_INITIALISER  {0,0,0,0}

int navigation_to_stream(const struct navigation_data *navigation_data,
        char *buf, int buf_len);

int navigation_from_stream(struct navigation_data *navigation_data,
        const char *buf, int buf_len);
#endif
