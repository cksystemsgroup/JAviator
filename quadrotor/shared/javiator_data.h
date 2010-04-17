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

#include <stdint.h>

/* State definitions */
#define ST_NEW_DATA_IMU     0x01
#define ST_NEW_DATA_BMU     0x02
#define ST_NEW_DATA_SONAR   0x04
#define ST_NEW_DATA_POS_X   0x08
#define ST_NEW_DATA_POS_Y   0x10

/* Structure for short JAviator data */
typedef struct
{
    uint32_t    maps;               /* [mV] 24-bit resolution */
    uint16_t    temp;               /* [mV] 10-bit resolution */
    uint16_t    batt;               /* [mV] 10-bit resolution */
    uint16_t    sonar;              /* [mV] 10-bit resolution */
    uint16_t    state;              /* JAviator data state */
    uint16_t    id;                 /* transmisson ID */
    uint8_t     x_pos[8];           /* [mm] range: 0...500000 */
    uint8_t     y_pos[8];           /* [mm] range: 0...500000 */

} javiator_sdat_t;

#define JAVIATOR_SDAT_SIZE  30      /* byte size of javiator_sdat_t */

/* Structure for long JAviator data */
typedef struct
{
    uint32_t    maps;               /* [mV] 24-bit resolution */
    uint16_t    temp;               /* [mV] 10-bit resolution */
    uint16_t    batt;               /* [mV] 10-bit resolution */
    uint16_t    sonar;              /* [mV] 10-bit resolution */
    uint16_t    state;              /* JAviator data state */
    uint16_t    id;                 /* transmisson ID */
    uint8_t     x_pos[8];           /* [mm] range: 0...500000 */
    uint8_t     y_pos[8];           /* [mm] range: 0...500000 */
    int16_t     roll;               /*                        | roll   |                     */
    int16_t     pitch;              /* [units] --> [mrad]   = | pitch  | * 2000 * PI / 65536 */
    int16_t     yaw;                /*                        | yaw    |                     */
    int16_t     droll;              /*                        | droll  |                     */
    int16_t     dpitch;             /* [units] --> [mrad/s] = | dpitch | * 8500 / 32768      */
    int16_t     dyaw;               /*                        | dyaw   |                     */
    int16_t     ddx;                /*                        | ddx    |                     */
    int16_t     ddy;                /* [units] --> [mm/s^2] = | ddy    | * 9810 * 7 / 32768  */
    int16_t     ddz;                /*                        | ddz    |                     */

} javiator_ldat_t;

#define JAVIATOR_LDAT_SIZE  48      /* byte size of javiator_ldat_t */

static inline
int javiator_sdat_to_stream( const javiator_sdat_t *data, uint8_t *buf, uint8_t len )
{
    if( len == JAVIATOR_SDAT_SIZE )
    {
        buf[0]  = (uint8_t)( data->maps >> 24 );
        buf[1]  = (uint8_t)( data->maps >> 16 );
        buf[2]  = (uint8_t)( data->maps >> 8 );
        buf[3]  = (uint8_t)( data->maps );
        buf[4]  = (uint8_t)( data->temp >> 8 );
        buf[5]  = (uint8_t)( data->temp );
        buf[6]  = (uint8_t)( data->batt >> 8 );
        buf[7]  = (uint8_t)( data->batt );
        buf[8]  = (uint8_t)( data->sonar >> 8 );
        buf[9]  = (uint8_t)( data->sonar );
        buf[10] = (uint8_t)( data->state >> 8 );
        buf[11] = (uint8_t)( data->state );
        buf[12] = (uint8_t)( data->id >> 8 );
        buf[13] = (uint8_t)( data->id );
        buf[14] = (uint8_t)( data->x_pos[0] );
        buf[15] = (uint8_t)( data->x_pos[1] );
        buf[16] = (uint8_t)( data->x_pos[2] );
        buf[17] = (uint8_t)( data->x_pos[3] );
        buf[18] = (uint8_t)( data->x_pos[4] );
        buf[19] = (uint8_t)( data->x_pos[5] );
        buf[20] = (uint8_t)( data->x_pos[6] );
        buf[21] = (uint8_t)( data->x_pos[7] );
        buf[22] = (uint8_t)( data->y_pos[0] );
        buf[23] = (uint8_t)( data->y_pos[1] );
        buf[24] = (uint8_t)( data->y_pos[2] );
        buf[25] = (uint8_t)( data->y_pos[3] );
        buf[26] = (uint8_t)( data->y_pos[4] );
        buf[27] = (uint8_t)( data->y_pos[5] );
        buf[28] = (uint8_t)( data->y_pos[6] );
        buf[29] = (uint8_t)( data->y_pos[7] );
        return( 0 );
    }

    return( -1 );
}

static inline
int javiator_sdat_from_stream( javiator_sdat_t *data, const uint8_t *buf, uint8_t len )
{
    if( len == JAVIATOR_SDAT_SIZE )
    {
		data->maps     = buf[0];
		data->maps   <<= 8;
		data->maps    |= buf[1];
		data->maps   <<= 8;
		data->maps    |= buf[2];
		data->maps   <<= 8;
		data->maps    |= buf[3];
		data->temp     = buf[4];
		data->temp   <<= 8;
		data->temp    |= buf[5];
		data->batt     = buf[6];
		data->batt   <<= 8;
		data->batt    |= buf[7];
		data->sonar    = buf[8];
		data->sonar  <<= 8;
		data->sonar   |= buf[9];
		data->state    = buf[10];
		data->state  <<= 8;
		data->state   |= buf[11];
		data->id       = buf[12];
		data->id     <<= 8;
		data->id      |= buf[13];
        data->x_pos[0] = buf[14];
        data->x_pos[1] = buf[15];
        data->x_pos[2] = buf[16];
        data->x_pos[3] = buf[17];
        data->x_pos[4] = buf[18];
        data->x_pos[5] = buf[19];
        data->x_pos[6] = buf[20];
        data->x_pos[7] = buf[21];
        data->y_pos[0] = buf[22];
        data->y_pos[1] = buf[23];
        data->y_pos[2] = buf[24];
        data->y_pos[3] = buf[25];
        data->y_pos[4] = buf[26];
        data->y_pos[5] = buf[27];
        data->y_pos[6] = buf[28];
        data->y_pos[7] = buf[29];
        return( 0 );
    }

    return( -1 );
}

static inline
int javiator_ldat_to_stream( const javiator_ldat_t *data, uint8_t *buf, uint8_t len )
{
    if( len == JAVIATOR_LDAT_SIZE )
    {
        buf[0]  = (uint8_t)( data->maps >> 24 );
        buf[1]  = (uint8_t)( data->maps >> 16 );
        buf[2]  = (uint8_t)( data->maps >> 8 );
        buf[3]  = (uint8_t)( data->maps );
        buf[4]  = (uint8_t)( data->temp >> 8 );
        buf[5]  = (uint8_t)( data->temp );
        buf[6]  = (uint8_t)( data->batt >> 8 );
        buf[7]  = (uint8_t)( data->batt );
        buf[8]  = (uint8_t)( data->sonar >> 8 );
        buf[9]  = (uint8_t)( data->sonar );
        buf[10] = (uint8_t)( data->state >> 8 );
        buf[11] = (uint8_t)( data->state );
        buf[12] = (uint8_t)( data->id >> 8 );
        buf[13] = (uint8_t)( data->id );
        buf[14] = (uint8_t)( data->x_pos[0] );
        buf[15] = (uint8_t)( data->x_pos[1] );
        buf[16] = (uint8_t)( data->x_pos[2] );
        buf[17] = (uint8_t)( data->x_pos[3] );
        buf[18] = (uint8_t)( data->x_pos[4] );
        buf[19] = (uint8_t)( data->x_pos[5] );
        buf[20] = (uint8_t)( data->x_pos[6] );
        buf[21] = (uint8_t)( data->x_pos[7] );
        buf[22] = (uint8_t)( data->y_pos[0] );
        buf[23] = (uint8_t)( data->y_pos[1] );
        buf[24] = (uint8_t)( data->y_pos[2] );
        buf[25] = (uint8_t)( data->y_pos[3] );
        buf[26] = (uint8_t)( data->y_pos[4] );
        buf[27] = (uint8_t)( data->y_pos[5] );
        buf[28] = (uint8_t)( data->y_pos[6] );
        buf[29] = (uint8_t)( data->y_pos[7] );
        buf[30] = (uint8_t)( data->roll >> 8 );
        buf[31] = (uint8_t)( data->roll );
        buf[32] = (uint8_t)( data->pitch >> 8 );
        buf[33] = (uint8_t)( data->pitch );
        buf[34] = (uint8_t)( data->yaw >> 8 );
        buf[35] = (uint8_t)( data->yaw );
        buf[36] = (uint8_t)( data->droll >> 8 );
        buf[37] = (uint8_t)( data->droll );
        buf[38] = (uint8_t)( data->dpitch >> 8 );
        buf[39] = (uint8_t)( data->dpitch );
        buf[40] = (uint8_t)( data->dyaw >> 8 );
        buf[41] = (uint8_t)( data->dyaw );
        buf[42] = (uint8_t)( data->ddx >> 8 );
        buf[43] = (uint8_t)( data->ddx );
        buf[44] = (uint8_t)( data->ddy >> 8 );
        buf[45] = (uint8_t)( data->ddy );
        buf[46] = (uint8_t)( data->ddz >> 8 );
        buf[47] = (uint8_t)( data->ddz );
        return( 0 );
    }

    return( -1 );
}

static inline
int javiator_ldat_from_stream( javiator_ldat_t *data, const uint8_t *buf, uint8_t len )
{
    if( len == JAVIATOR_LDAT_SIZE )
    {
		data->maps     = buf[0];
		data->maps   <<= 8;
		data->maps    |= buf[1];
		data->maps   <<= 8;
		data->maps    |= buf[2];
		data->maps   <<= 8;
		data->maps    |= buf[3];
		data->temp     = buf[4];
		data->temp   <<= 8;
		data->temp    |= buf[5];
		data->batt     = buf[6];
		data->batt   <<= 8;
		data->batt    |= buf[7];
		data->sonar    = buf[8];
		data->sonar  <<= 8;
		data->sonar   |= buf[9];
		data->state    = buf[10];
		data->state  <<= 8;
		data->state   |= buf[11];
		data->id       = buf[12];
		data->id     <<= 8;
		data->id      |= buf[13];
        data->x_pos[0] = buf[14];
        data->x_pos[1] = buf[15];
        data->x_pos[2] = buf[16];
        data->x_pos[3] = buf[17];
        data->x_pos[4] = buf[18];
        data->x_pos[5] = buf[19];
        data->x_pos[6] = buf[20];
        data->x_pos[7] = buf[21];
        data->y_pos[0] = buf[22];
        data->y_pos[1] = buf[23];
        data->y_pos[2] = buf[24];
        data->y_pos[3] = buf[25];
        data->y_pos[4] = buf[26];
        data->y_pos[5] = buf[27];
        data->y_pos[6] = buf[28];
        data->y_pos[7] = buf[29];
        data->roll     = buf[30];
        data->roll   <<= 8;
        data->roll    |= buf[31];
        data->pitch    = buf[32];
        data->pitch  <<= 8;
        data->pitch   |= buf[33];
        data->yaw      = buf[34];
        data->yaw    <<= 8;
        data->yaw     |= buf[35];
        data->droll    = buf[36];
        data->droll  <<= 8;
        data->droll   |= buf[37];
        data->dpitch   = buf[38];
        data->dpitch <<= 8;
        data->dpitch  |= buf[39];
        data->dyaw     = buf[40];
        data->dyaw   <<= 8;
        data->dyaw    |= buf[41];
        data->ddx      = buf[42];
        data->ddx    <<= 8;
        data->ddx     |= buf[43];
        data->ddy      = buf[44];
        data->ddy    <<= 8;
        data->ddy     |= buf[45];
        data->ddz      = buf[46];
        data->ddz    <<= 8;
        data->ddz     |= buf[47];
        return( 0 );
    }

    return( -1 );
}

#endif /* !JAVIATOR_DATA_H */

/* End of file */
