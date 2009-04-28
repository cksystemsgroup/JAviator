/* $Id: pwm_signals.c,v 1.1 2008/10/16 14:41:13 rtrummer Exp $ */

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

#include "pwm_signals.h"


int pwm_signals_to_stream( const pwm_signals_t *pwm_signals, char *buf, int len )
{
    if( len == PWM_SIGNALS_SIZE )
    {
        buf[0] = (char)( pwm_signals->front >> 8 );
        buf[1] = (char)( pwm_signals->front );
        buf[2] = (char)( pwm_signals->right >> 8 );
        buf[3] = (char)( pwm_signals->right );
        buf[4] = (char)( pwm_signals->rear >> 8 );
        buf[5] = (char)( pwm_signals->rear );
        buf[6] = (char)( pwm_signals->left >> 8 );
        buf[7] = (char)( pwm_signals->left );
        return( 0 );
    }

    fprintf( stderr, "ERROR: invalid length (%d) of PWM signals to stream\n", len );
    return( -1 );
}

int pwm_signals_from_stream( pwm_signals_t *pwm_signals, const char *buf, int len )
{
    if( len == PWM_SIGNALS_SIZE )
    {
        pwm_signals->front = (int16_t)( (buf[0] << 8) | (buf[1] & 0xFF) );
        pwm_signals->right = (int16_t)( (buf[2] << 8) | (buf[3] & 0xFF) );
        pwm_signals->rear  = (int16_t)( (buf[4] << 8) | (buf[5] & 0xFF) );
        pwm_signals->left  = (int16_t)( (buf[6] << 8) | (buf[7] & 0xFF) );
        return( 0 );
    }

    fprintf( stderr, "ERROR: invalid length (%d) of PWM signals from stream\n", len );
    return( -1 );
}

// End of file.
