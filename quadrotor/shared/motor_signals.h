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

#ifndef MOTOR_SIGNALS_H
#define MOTOR_SIGNALS_H

#include <stdint.h>

#define MOTOR_MAX           16000   /* [PWM] */
#define MOTOR_MIN           0       /* [PWM] */

/* Structure for shared motor signals */
typedef struct
{
    int16_t     front;              /* [PWM] If Fast PWM Mode enabled,  */
    int16_t     right;              /* [PWM] then range 0...16000,      */
    int16_t     rear;               /* [PWM] if Fast PWM Mode disabled, */
    int16_t     left;               /* [PWM] then range 0...1000.       */
    uint16_t    id;                 /* transmisson ID */

} motor_signals_t;

#define MOTOR_SIGNALS_SIZE  10       /* byte size of motor_signals_t + 2 for id */

static inline
int motor_signals_to_stream( const motor_signals_t *signals, uint8_t *buf, uint8_t len )
{
    if( len == MOTOR_SIGNALS_SIZE )
    {
        buf[0] = (uint8_t)( signals->front >> 8 );
        buf[1] = (uint8_t)( signals->front );
        buf[2] = (uint8_t)( signals->right >> 8 );
        buf[3] = (uint8_t)( signals->right );
        buf[4] = (uint8_t)( signals->rear >> 8 );
        buf[5] = (uint8_t)( signals->rear );
        buf[6] = (uint8_t)( signals->left >> 8 );
        buf[7] = (uint8_t)( signals->left );
        buf[8] = (uint8_t)( signals->id >> 8 );
        buf[9] = (uint8_t)( signals->id );
        return( 0 );
    }

    return( -1 );
}

static inline
int motor_signals_from_stream( motor_signals_t *signals, const uint8_t *buf, uint8_t len )
{
    if( len == MOTOR_SIGNALS_SIZE )
    {
        signals->front = (int16_t) ( (buf[0] << 8) | (buf[1] & 0xFF) );
        signals->right = (int16_t) ( (buf[2] << 8) | (buf[3] & 0xFF) );
        signals->rear  = (int16_t) ( (buf[4] << 8) | (buf[5] & 0xFF) );
        signals->left  = (int16_t) ( (buf[6] << 8) | (buf[7] & 0xFF) );
        signals->id    = (uint16_t)( (buf[8] << 8) | (buf[9] & 0xFF) );
        return( 0 );
    }

    return( -1 );
}

#endif /* !MOTOR_SIGNALS_H */

/* End of file */
