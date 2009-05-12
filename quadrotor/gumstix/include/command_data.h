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

#ifndef COMMAND_DATA_H
#define COMMAND_DATA_H

#include <stdint.h>


/* Structure for representing command data */
typedef struct
{
    int16_t roll;
    int16_t pitch;
    int16_t yaw;
    int16_t z;

} command_data_t;

#define COMMAND_DATA_SIZE   8   /* byte size of command_data_t */


int command_data_to_stream( const command_data_t *data, char *buf, int len );

int command_data_from_stream( command_data_t *data, const char *buf, int len );


#endif /* !COMMAND_DATA_H */

/* End of file */
