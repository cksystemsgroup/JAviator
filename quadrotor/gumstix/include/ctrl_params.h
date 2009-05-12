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

#ifndef CTRL_PARAMS_H
#define CTRL_PARAMS_H

#include <stdint.h>


/* Structure for representing control parameters */
typedef struct
{
    int16_t kp;
    int16_t ki;
    int16_t kd;
    int16_t kdd;

} ctrl_params_t;

#define CTRL_PARAMS_SIZE    8   /* byte size of ctrl_params_t */


int ctrl_params_to_stream( const ctrl_params_t *params, char *buf, int len );

int ctrl_params_from_stream( ctrl_params_t *params, const char *buf, int len );


#endif /* !CTRL_PARAMS_H */

/* End of file */
