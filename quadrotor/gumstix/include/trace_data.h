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

#ifndef TRACE_DATA_H
#define TRACE_DATA_H

#include <stdint.h>


/* Structure for representing trace data */
typedef struct
{
    int16_t z;              /* z from sonar sensor */
    int16_t z_filtered;     /* median-filtered z */
    int16_t z_estimated;    /* kalman-estimated z */
    int16_t dz_estimated;   /* kalman-estimated dz */
    int16_t ddz;            /* ddz from IMU sensor */
    int16_t ddz_filtered;   /* low-pass-filtered ddz */
    int16_t p_term;         /* computed p-term */
    int16_t i_term;         /* computed i-term */
    int16_t d_term;         /* computed d-term */
    int16_t dd_term;        /* computed dd-term */
    int16_t uz;             /* output from z-controller */
    int16_t z_cmd;          /* z-command from terminal */
    int16_t id;             /* packet id */

} trace_data_t;

#define TRACE_DATA_SIZE     26  /* byte size of trace_data_t */


int trace_data_to_stream( const trace_data_t *data, char *buf, int len );

int trace_data_from_stream( trace_data_t *data, const char *buf, int len );


#endif /* !TRACE_DATA_H */

/* End of file */
