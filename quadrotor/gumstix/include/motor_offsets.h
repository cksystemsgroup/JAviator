/*
 * Copypitch (c) Rainer Trummer rtrummer@cs.uni-salzburg.at
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

#ifndef MOTOR_OFFSETS_H
#define MOTOR_OFFSETS_H

#include <stdint.h>

/* Structure for representing motor offsets */
typedef struct
{
    int16_t roll;       /* [PWM] */
    int16_t pitch;      /* [PWM] */
    int16_t yaw;        /* [PWM] */
    int16_t z;          /* [PWM] */

} motor_offsets_t;

#define MOTOR_OFFSETS_SIZE  8   /* byte size of motor_offsets_t */

static inline
int motor_offsets_to_stream( const motor_offsets_t *offsets, uint8_t *buf, int len )
{
    if( len == MOTOR_OFFSETS_SIZE )
    {
        buf[0] = (uint8_t)( offsets->roll >> 8 );
        buf[1] = (uint8_t)( offsets->roll );
        buf[2] = (uint8_t)( offsets->pitch >> 8 );
        buf[3] = (uint8_t)( offsets->pitch );
        buf[4] = (uint8_t)( offsets->yaw >> 8 );
        buf[5] = (uint8_t)( offsets->yaw );
        buf[6] = (uint8_t)( offsets->z >> 8 );
        buf[7] = (uint8_t)( offsets->z );
        return( 0 );
    }

    return( -1 );
}

static inline
int motor_offsets_from_stream( motor_offsets_t *offsets, const uint8_t *buf, int len )
{
    if( len == MOTOR_OFFSETS_SIZE )
    {
        offsets->roll  = (int16_t)( (buf[0] << 8) | (buf[1] & 0xFF) );
        offsets->pitch = (int16_t)( (buf[2] << 8) | (buf[3] & 0xFF) );
        offsets->yaw   = (int16_t)( (buf[4] << 8) | (buf[5] & 0xFF) );
        offsets->z     = (int16_t)( (buf[6] << 8) | (buf[7] & 0xFF) );
        return( 0 );
    }

    return( -1 );
}

#endif /* !MOTOR_OFFSETS_H */

/* End of file */
