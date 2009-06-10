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

#ifndef REV_PARAMS_H
#define REV_PARAMS_H

#include <stdint.h>


/* Structure for representing control parameters */
typedef struct
{
    int16_t idle_limit;
    int16_t ctrl_speed;
    int16_t rev_up_inc;
    int16_t rev_dn_dec;

} rev_params_t;

#define REV_PARAMS_SIZE     8   /* byte size of rev_params_t */


static inline
int rev_params_to_stream( const rev_params_t *params, char *buf, int len )
{
    if( len == REV_PARAMS_SIZE )
    {
        buf[0] = (char)( params->idle_limit >> 8 );
        buf[1] = (char)( params->idle_limit );
        buf[2] = (char)( params->ctrl_speed >> 8 );
        buf[3] = (char)( params->ctrl_speed );
        buf[4] = (char)( params->rev_up_inc >> 8 );
        buf[5] = (char)( params->rev_up_inc );
        buf[6] = (char)( params->rev_dn_dec >> 8 );
        buf[7] = (char)( params->rev_dn_dec );
        return( 0 );
    }

    return( -1 );
}

static inline
int rev_params_from_stream( rev_params_t *params, const char *buf, int len )
{
    if( len == REV_PARAMS_SIZE )
    {
        params->idle_limit = (int16_t)( (buf[0] << 8) | (buf[1] & 0xFF) );
        params->ctrl_speed = (int16_t)( (buf[2] << 8) | (buf[3] & 0xFF) );
        params->rev_up_inc = (int16_t)( (buf[4] << 8) | (buf[5] & 0xFF) );
        params->rev_dn_dec = (int16_t)( (buf[6] << 8) | (buf[7] & 0xFF) );
        return( 0 );
    }

    return( -1 );
}


#endif /* !REV_PARAMS_H */

/* End of file */
