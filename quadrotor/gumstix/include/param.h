/* $Id: param.h,v 1.3 2008/10/28 09:06:44 rtrummer Exp $ */

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
#ifndef CTRL_PARAMS_H
#define CTRL_PARAMS_H

#define PARAM_PACKET_LENGTH    32

struct parameters {
    int rollpitch_Kp;
    int rollpitch_Ki;
    int rollpitch_Kd;
    int rollpitch_Kdd;
    int yaw_Kp;
    int yaw_Ki;
    int yaw_Kd;
    int yaw_Kdd;
    int altitude_Kp;
    int altitude_Ki;
    int altitude_Kd;
    int altitude_Kdd;
    int x_y_Kp;
    int x_y_Ki;
    int x_y_Kd;
    int x_y_Kdd;
};

int params_to_stream(const struct parameters *values, char *buf, int buf_len);
int params_from_stream(struct parameters *values, const char *buf, int buf_len);
#endif /* CTRL_PARAMS_H */
