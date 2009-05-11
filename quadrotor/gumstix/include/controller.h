/*
 * Copyright (c) Harald Roeck hroeck@cs.uni-salzburg.at
 * Copyright (c) Rainer Trummer rtrummer@cs.uni-salzburg.at
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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <controller-config.h>

extern int errno;

struct controller;
struct controller_state;

typedef double (*control)(struct controller *controller,
        double current, double desired, double velocity, double acceleration);
typedef int (*reset_zero)(struct controller *controller);
typedef int (*set_params)(struct controller *controller,
        double p, double i, double d, double dd);

typedef enum {
    CTRL_PID = 1,
    CTRL_PID_YAW,
    CTRL_PIDD,
    CTRL_PIDD_YAW,
} controller_type;

struct controller {
    controller_type type;
    control control;
    reset_zero reset_zero;
    set_params set_params;
    char *name;
    struct controller_state *state;
};

int controller_init(struct controller *controller, char *name, controller_type type, int period);
int controller_destroy(struct controller *controller);

#endif /* CONTROLLER_H */

/* End of file */
