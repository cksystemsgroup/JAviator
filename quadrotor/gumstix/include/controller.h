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
struct ctrl_state;

typedef struct controller controller_t;
typedef struct ctrl_state ctrl_state_t;

typedef double (*control)( controller_t *controller,
                           double desired, double current,
                           double velocity, double acceleration );
typedef int (*reset_zero)( controller_t *controller );
typedef int (*set_params)( controller_t *controller,
                           double kp, double ki, double kd, double kdd );

typedef enum
{
    CTRL_PIDD_DEF = 1,
    CTRL_PIDD_YAW,
    CTRL_PIDD_X_Y,

} ctrl_type_t;

struct controller
{
    ctrl_type_t     type;
    control         control;
    reset_zero      reset_zero;
    set_params      set_params;
    char *          name;
    ctrl_state_t *  state;
};


int    controller_init        ( controller_t *controller, char *name,
                                ctrl_type_t type, int period );
int    controller_destroy     ( controller_t *controller );
double controller_get_term_P  ( controller_t *controller );
double controller_get_term_I  ( controller_t *controller );
double controller_get_term_D  ( controller_t *controller );
double controller_get_term_DD ( controller_t *controller );
double controller_get_integral( controller_t *controller );
void   controller_set_integral( controller_t *controller, double value );


#endif /* CONTROLLER_H */

/* End of file */
