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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 */

#include "controller.h"

#ifndef NULL
#define NULL 0
#endif

extern int pidd_def_controller_init( controller_t *, double );
extern int pidd_x_y_controller_init( controller_t *, double );
extern int pidd_all_controller_destroy( controller_t * );

int controller_init( controller_t *controller,
    char *name, ctrl_type_t type, double period )
{
    int res = -1;

    controller->name = name;

    switch( type )
    {
        case CTRL_PIDD_DEF:
            res = pidd_def_controller_init( controller, period );
            break;

        case CTRL_PIDD_X_Y:
            res = pidd_x_y_controller_init( controller, period );
            break;
    }

    return( res );
}

int controller_destroy( controller_t *controller )
{
    controller->name = NULL;
    return pidd_all_controller_destroy( controller );
}

/* End of file */
