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

#ifndef JAVIATOR_DATA_H
#define JAVIATOR_DATA_H

/* Structure for shared JAviator data */
typedef struct
{
    uint32_t    pos_x;              /* [?] range ? */
    uint32_t    pos_y;              /* [?] range ? */
    uint32_t    laser;              /* [1/10mm] range 0...5000000 */
    uint16_t    sonar;              /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    pressure;           /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    battery;            /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    state;              /* JAviator state indicator */
    uint16_t    error;              /* JAviator error indicator */
    uint16_t    id;                 /* transmisson ID */

} javiator_data_short_t;

#define JAVIATOR_DATA_SHORT_SIZE  24      /* byte size of javiator_data_short_t */

/* Structure for shared JAviator data */
typedef struct
{
    int16_t     roll;               /*                        | roll    |                       */
    int16_t     pitch;              /* [units] --> [mrad]   = | pitch   | * 2000 * PI / 65536   */
    int16_t     yaw;                /*                        | yaw     |                       */
    int16_t     droll;              /*                        | droll   |                       */
    int16_t     dpitch;             /* [units] --> [mrad/s] = | dpitch  | * 8500 / 32768        */
    int16_t     dyaw;               /*                        | dyaw    |                       */
    int16_t     ddx;                /*                        | ddx     |                       */
    int16_t     ddy;                /* [units] --> [mm/s^2] = | ddy     | * 9810 * 7 / 32768    */
    int16_t     ddz;                /*                        | ddz     |                       */
    int16_t     ticks;              /* [units] --> [s]      = | ticks   | * 65536 / 10000000    */
    uint16_t    sonar;              /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    pressure;           /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    battery;            /* [mV] range 0...5000 (10-bit resolution) */
    uint16_t    state;              /* JAviator state indicator */
    uint16_t    error;              /* JAviator error indicator */
    uint16_t    id;                 /* transmisson ID */

} javiator_data_long_t;

#define JAVIATOR_DATA_LONG_SIZE  32      /* byte size of javiator_data_long_t */

/* by default use old style full data */
typedef javiator_data_long_t javiator_data_t;
#define JAVIATOR_DATA_SIZE  JAVIATOR_DATA_LONG_SIZE      


static inline
int javiator_data_long_to_stream( const javiator_data_t *data, uint8_t *buf, int len )
{
    if( len == JAVIATOR_DATA_SIZE )
    {
        buf[0]  = (uint8_t)( data->roll >> 8 );
        buf[1]  = (uint8_t)( data->roll );
        buf[2]  = (uint8_t)( data->pitch >> 8 );
        buf[3]  = (uint8_t)( data->pitch );
        buf[4]  = (uint8_t)( data->yaw >> 8 );
        buf[5]  = (uint8_t)( data->yaw );
        buf[6]  = (uint8_t)( data->droll >> 8 );
        buf[7]  = (uint8_t)( data->droll );
        buf[8]  = (uint8_t)( data->dpitch >> 8 );
        buf[9]  = (uint8_t)( data->dpitch );
        buf[10] = (uint8_t)( data->dyaw >> 8 );
        buf[11] = (uint8_t)( data->dyaw );
        buf[12] = (uint8_t)( data->ddx >> 8 );
        buf[13] = (uint8_t)( data->ddx );
        buf[14] = (uint8_t)( data->ddy >> 8 );
        buf[15] = (uint8_t)( data->ddy );
        buf[16] = (uint8_t)( data->ddz >> 8 );
        buf[17] = (uint8_t)( data->ddz );
        buf[18] = (uint8_t)( data->ticks >> 8 );
        buf[19] = (uint8_t)( data->ticks );
        buf[20] = (uint8_t)( data->sonar >> 8 );
        buf[21] = (uint8_t)( data->sonar );
        buf[22] = (uint8_t)( data->pressure >> 8 );
        buf[23] = (uint8_t)( data->pressure );
        buf[24] = (uint8_t)( data->battery >> 8 );
        buf[25] = (uint8_t)( data->battery );
        buf[26] = (uint8_t)( data->state >> 8 );
        buf[27] = (uint8_t)( data->state );
        buf[28] = (uint8_t)( data->error >> 8 );
        buf[29] = (uint8_t)( data->error );
        buf[30] = (uint8_t)( data->id >> 8 );
        buf[31] = (uint8_t)( data->id );
        return( 0 );
    }

    return( -1 );
}

static inline
int javiator_data_long_from_stream( javiator_data_t *data, const uint8_t *buf, int len )
{
    if( len == JAVIATOR_DATA_SIZE )
    {
		data->roll    = buf[0];
		data->roll    <<= 8;
		data->roll    |= buf[1];

		data->pitch = buf[2];
		data->pitch <<= 8;
		data->pitch |= buf[3];
		
		data->yaw = buf[4];
		data->yaw <<= 8;
		data->yaw |= buf[5];
		
		data->droll = buf[6];
		data->droll <<= 8;
		data->droll |= buf[7];

		data->dpitch = buf[8];
		data->dpitch <<= 8;
		data->dpitch |= buf[9];
		
		data->dyaw = buf[10];
		data->dyaw <<= 8;
		data->dyaw |= buf[11];

		data->ddx = buf[12];
		data->ddx <<= 8;
		data->ddx |= buf[13];
		
		data->ddy = buf[14];
		data->ddy <<= 8;
		data->ddy |= buf[15];

		data->ddz = buf[16];
		data->ddz <<= 8;
		data->ddz |= buf[17];
		
		data->ticks = buf[18];
		data->ticks <<= 8;
		data->ticks |= buf[19];

		data->sonar    = buf[20];
		data->sonar    <<= 8;
		data->sonar    |= buf[21];

		data->pressure = buf[22];
		data->pressure <<= 8;
		data->pressure |= buf[23];
		
		data->battery = buf[24];
		data->battery <<= 8;
		data->battery |= buf[25];
		
		data->state = buf[26];
		data->state <<= 8;
		data->state |= buf[27];
		
		data->error = buf[28];
		data->error <<= 8;
		data->error |= buf[29];
		
		data->id = buf[30];
		data->id <<= 8;
		data->id |= buf[31];

        return( 0 );
    }

    return( -1 );
}

static inline
int javiator_data_short_to_stream( const javiator_data_short_t *data, uint8_t *buf, int len )
{
    if( len == JAVIATOR_DATA_SIZE )
    {
        buf[0]  = (uint8_t)( data->pos_x >> 24 );
        buf[1]  = (uint8_t)( data->pos_x >> 16 );
        buf[2]  = (uint8_t)( data->pos_x >> 8 );
        buf[3]  = (uint8_t)( data->pos_x );
        buf[4]  = (uint8_t)( data->pos_y >> 24 );
        buf[5]  = (uint8_t)( data->pos_y >> 16 );
        buf[6]  = (uint8_t)( data->pos_y >> 8 );
        buf[7]  = (uint8_t)( data->pos_y );
        buf[8]  = (uint8_t)( data->laser >> 24 );
        buf[9]  = (uint8_t)( data->laser >> 16 );
        buf[10] = (uint8_t)( data->laser >> 8 );
        buf[11] = (uint8_t)( data->laser );
        buf[12] = (uint8_t)( data->sonar >> 8 );
        buf[13] = (uint8_t)( data->sonar );
        buf[14] = (uint8_t)( data->pressure >> 8 );
        buf[15] = (uint8_t)( data->pressure );
        buf[16] = (uint8_t)( data->battery >> 8 );
        buf[17] = (uint8_t)( data->battery );
        buf[18] = (uint8_t)( data->state >> 8 );
        buf[19] = (uint8_t)( data->state );
        buf[20] = (uint8_t)( data->error >> 8 );
        buf[21] = (uint8_t)( data->error );
        buf[22] = (uint8_t)( data->id >> 8 );
        buf[23] = (uint8_t)( data->id );
        return( 0 );
    }

    return( -1 );
}

static inline
int javiator_data_short_from_stream( javiator_data_short_t *data, const uint8_t *buf, int len )
{
    if( len == JAVIATOR_DATA_SIZE )
    {
		data->pos_x    = buf[0];
		data->pos_x    <<= 8;
		data->pos_x    |= buf[1];
		data->pos_x    <<= 8;
		data->pos_x    |= buf[2];
		data->pos_x    <<= 8;
		data->pos_x    |= buf[3];

		data->pos_y    = buf[4];
		data->pos_y    <<= 8;
		data->pos_y    |= buf[5];
		data->pos_y    <<= 8;
		data->pos_y    |= buf[6];
		data->pos_y    <<= 8;
		data->pos_y    |= buf[7];

		data->laser    = buf[8];
		data->laser    <<= 8;
		data->laser    |= buf[9];
		data->laser    <<= 8;
		data->laser    |= buf[10];
		data->laser    <<= 8;
		data->laser    |= buf[11];

		data->sonar    = buf[12];
		data->sonar    <<= 8;
		data->sonar    |= buf[13];

		data->pressure = buf[14];
		data->pressure <<= 8;
		data->pressure |= buf[15];
		
		data->battery = buf[16];
		data->battery <<= 8;
		data->battery |= buf[17];
		
		data->state = buf[18];
		data->state <<= 8;
		data->state |= buf[19];
		
		data->error = buf[20];
		data->error <<= 8;
		data->error |= buf[21];
		
		data->id = buf[22];
		data->id <<= 8;
		data->id |= buf[23];
		
        return( 0 );
    }

    return( -1 );
}


static inline
int javiator_data_to_stream( const javiator_data_t *data, uint8_t *buf, int len )
{
	return javiator_data_long_to_stream(data, buf, len);
}

static inline
int javiator_data_from_stream( javiator_data_t *data, const uint8_t *buf, int len )
{
	return javiator_data_long_from_stream(data, buf, len);
}
#endif /* !JAVIATOR_DATA_H */

/* End of file */
