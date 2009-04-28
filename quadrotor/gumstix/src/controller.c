/* $Id: controller.c,v 1.3 2008/11/10 12:17:57 hroeck Exp $ */

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

#ifndef NULL
#define NULL  0
#endif

extern int pid_controller_init(struct controller *, int);
extern int pid_controller_destroy(struct controller *);
extern int pidd_controller_init(struct controller *, int);
extern int pidd_controller_destroy(struct controller *);
extern int pidd_yaw_controller_init(struct controller *, int);


/* only PID controllers supported */

int controller_init(struct controller *controller, 
    char * name, controller_type type, int period)
{
    int retval = -1;;
    controller->name = name;
    switch (type) {
    case CTRL_PID:
        retval = pid_controller_init(controller, period);
        break;
    case CTRL_PID_YAW:
        break;
    case CTRL_PIDD:
        retval = pidd_controller_init(controller, period);
        break;
    case CTRL_PIDD_YAW:
        retval = pidd_yaw_controller_init(controller, period);
        break;
    }
    return retval;
}

int controller_destroy(struct controller *controller)
{
    int retval = -1;;
    controller->name = NULL;
    switch (controller->type) {
    case CTRL_PID:
    case CTRL_PID_YAW:
        retval = pid_controller_destroy(controller);
        break;
    case CTRL_PIDD:
    case CTRL_PIDD_YAW:
        retval = pidd_controller_destroy(controller);
        break;
    }
    return retval;
}
