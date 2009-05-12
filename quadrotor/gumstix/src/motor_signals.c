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

#include <stdio.h>

#include "motor_signals.h"


int motor_signals_to_stream( const motor_signals_t *signals, char *buf, int len )
{
    if( len == MOTOR_SIGNALS_SIZE )
    {
        buf[0] = (char)( signals->front >> 8 );
        buf[1] = (char)( signals->front );
        buf[2] = (char)( signals->right >> 8 );
        buf[3] = (char)( signals->right );
        buf[4] = (char)( signals->rear >> 8 );
        buf[5] = (char)( signals->rear );
        buf[6] = (char)( signals->left >> 8 );
        buf[7] = (char)( signals->left );
        return( 0 );
    }

    fprintf( stderr, "ERROR in %s %d: invalid length (%d) of motor signals to stream\n",
        __FILE__, __LINE__, len );
    return( -1 );
}

int motor_signals_from_stream( motor_signals_t *signals, const char *buf, int len )
{
    if( len == MOTOR_SIGNALS_SIZE )
    {
        signals->front = (int16_t)( (buf[0] << 8) | (buf[1] & 0xFF) );
        signals->right = (int16_t)( (buf[2] << 8) | (buf[3] & 0xFF) );
        signals->rear  = (int16_t)( (buf[4] << 8) | (buf[5] & 0xFF) );
        signals->left  = (int16_t)( (buf[6] << 8) | (buf[7] & 0xFF) );
        return( 0 );
    }

    fprintf( stderr, "ERROR in %s %d: invalid length (%d) of motor signals from stream\n",
        __FILE__, __LINE__, len );
    return( -1 );
}

/* End of file */
