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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "ubisense_port.h"
#include "communication.h"
#include "protocol.h"

static ubisense_data_t  ubisense_data;
static comm_channel_t * comm_channel;
static char             comm_buf[ COMM_BUF_SIZE ];
static int              ubisense_tag;
static int              new_data;
static int              running;
static pthread_t        thread;
static pthread_mutex_t  ubisense_lock = PTHREAD_MUTEX_INITIALIZER;


static inline void lock( void )
{
	pthread_mutex_lock( &ubisense_lock );
}

static inline void unlock( void )
{
	pthread_mutex_unlock( &ubisense_lock );
}

static inline int parse_data_packet( const char *buf, int len )
{
    /* Data format: "------tttttt,x1...xn,y1...yn\0", where
       the first 6 digits <------> can be ignored, the next
       6 digits <tttttt> represent the tag ID, and the last
       digits <x1...xn> and <y1...yn> indicate variable
       numbers of digits representing the x and y values.
    */
    if( len < 12 || atoi( buf + 6 ) != ubisense_tag )
    {
        return( -1 );
    }

    if( !(buf = strchr( buf, ',' )) )
    {
        return( -1 );
    }

    ubisense_data.y = atoi( ++buf );

    if( !(buf = strchr( buf, ',' )) )
    {
        return( -1 );
    }

    ubisense_data.x = atoi( ++buf );

    new_data = 1;

    return( 0 );
}

int ubisense_port_tick( void )
{
    int res = comm_channel->receive( comm_channel, comm_buf, COMM_BUF_SIZE );

    if( res > 0 )
    {
        if( res < COMM_BUF_SIZE )
        {
            comm_buf[ res ] = 0;
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
        memset( &ubisense_data, 0, sizeof( ubisense_data ) );

        comm_channel        = channel;
        ubisense_tag        = tag;
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

int ubisense_port_get_data( ubisense_data_t *data )
{
	lock( );
    memcpy( data, &ubisense_data, sizeof( *data ) );
    new_data = 0;
	unlock( );
    return( 0 );
}

/* End of file */
