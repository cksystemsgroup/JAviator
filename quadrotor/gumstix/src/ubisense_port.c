/*
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>

#include "ubisense_port.h"
#include "communication.h"
#include "protocol.h"

/* Message constants */
#define MSG_MAGIC_0     0xE298
#define MSG_MAGIC_1     0x026A

static comm_channel_t * comm_channel;
static char             comm_buf[ COMM_BUF_SIZE ];
static int              tag_id;
static int              tag_pos_x;
static int              tag_pos_y;
static int              new_data;
static int              running;
static pthread_t        thread;
static pthread_mutex_t  ubisense_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    uint16_t    magic_0;        /* 1st magic number identifying a location message */
    uint16_t    magic_1;        /* 2nd magic number identifying a location message */
    uint32_t    tag_id_high;    /* high-order 32 bits of tag ID (currently always zero) */
    uint32_t    tag_id_low;     /* low-order 32 bits of tag ID */
    uint32_t    flags;          /* bit 0 indicates a valid location if (flags & 1) == 1 */
    float       x;              /* x-coordinate of tag location */
    float       y;              /* y-coordinate of tag location */
    float       z;              /* z-coordinate of tag location */
    float       gdop;           /* geometric dilution of precision */
    float       error;          /* standard error of location calculation */
    uint32_t    slot;           /* timeslot number of received message */
    uint32_t    slot_interval;  /* microseconds between two timeslots */
    uint32_t    slot_delay;     /* delay between transmission timeslot and message timeslot */
    uint32_t    cell_id;        /* ID of cell that generated this message */

} location_message_t;


static inline void lock( void )
{
	pthread_mutex_lock( &ubisense_lock );
}

static inline void unlock( void )
{
	pthread_mutex_unlock( &ubisense_lock );
}

static inline void swap_byte_order( char *p )
{
   char c;

   c    = p[0];
   p[0] = p[3];
   p[3] = c;
   c    = p[1];
   p[1] = p[2];
   p[2] = c;
}

static inline int parse_data_packet( char *buf, int len )
{
    location_message_t *msg = (location_message_t *) buf;

    if( len != sizeof( location_message_t )  || /* invalid message length */
        ntohs( msg->magic_0 ) != MSG_MAGIC_0 || /* invalid 1st magic number */
        ntohs( msg->magic_1 ) != MSG_MAGIC_1 || /* invalid 2nd magic number */
        (ntohl( msg->flags ) & 1) != 1 )        /* invalid location data */
    {
        return( -1 );
    }

    if( ntohl( msg->tag_id_low ) == tag_id )
    {
        if( 1 != ntohl( 1 ) )
        {
            swap_byte_order( (char *) &msg->x );
            swap_byte_order( (char *) &msg->y );
        }

        tag_pos_x = (int)( msg->x * 1000 );
        tag_pos_y = (int)( msg->y * 1000 );
        new_data  = 1;
    }

    return( 0 );
}

int ubisense_port_tick( void )
{
    int res = comm_channel->receive( comm_channel, comm_buf, COMM_BUF_SIZE );

    if( res > 0 )
    {
        if( res < COMM_BUF_SIZE )
        {
            comm_buf[ res ] = 0; /* null-terminate end of received data */
        }

        parse_data_packet( comm_buf, res );
    }
    else
    if( res == EAGAIN )
    {
        res = 0;
    }
    else
    if( res == -1 )
    {
        fprintf( stderr, "ERROR: invalid data from Ubisense channel\n" );
    }

    return( res );
}

static void * ubisense_thread( void *arg )
{
	int res;

	while( running )
    {
		res = comm_channel->poll( comm_channel, 0 );

		if( res > 0 )
        {
			ubisense_port_tick( );
		}
        else
        {
            fprintf( stderr, "WARNING: poll returned %d\n", res );
		}
	}

	return( NULL );
}

static void start_ubisense_thread( void )
{
	struct sched_param param;
	pthread_attr_t attr;

	pthread_attr_init( &attr );
	sched_getparam( 0, &param );

	if( param.sched_priority > 0 )
    {
		--param.sched_priority;
		pthread_attr_setschedparam( &attr, &param );		
		pthread_attr_setschedpolicy( &attr, SCHED_FIFO );
		pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
	}

	pthread_create( &thread, &attr, ubisense_thread, NULL );
}

int ubisense_port_init( comm_channel_t *channel, int tag )
{
    static int already_initialized = 0;

    if( !already_initialized )
    {
        comm_channel        = channel;
        tag_id              = 0x14000000 | ((tag / 1000) << 8) | (tag % 1000);
        tag_pos_x           = 0;
        tag_pos_y           = 0;
        new_data            = 0;
        running             = 1;
        already_initialized = 1;

		start_ubisense_thread( );
        return( 0 );
    }

    fprintf( stderr, "WARNING: Ubisense port already initialized\n" );
    return( -1 );
}

int ubisense_port_is_new_data( void )
{
    return( new_data );
}

int ubisense_port_get_data( sensor_data_t *data )
{
	lock( );

	/* IMPORTANT: Ubisense location data refer to Cartesian coordinates,
       whereas JAviator location data refer to aircraft coordinates,
       hence x and y must be exchanged when accessing the data. */
    data->x  = tag_pos_y;
    data->y  = tag_pos_x;
    new_data = 0;

	unlock( );
    return( 0 );
}

/* End of file */
