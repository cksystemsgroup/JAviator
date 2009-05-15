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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>

#include "../shared/protocol.h"
#include "../shared/transfer.h"
#include "controller.h"
#include "comm_channel.h"
#include "communication.h"
#include "javiator_port.h"
#include "terminal_port.h"
#include "us_timer.h"

static javiator_data_t  javiator_data;
static comm_channel_t * comm_channel;
static comm_packet_t    comm_packet;
static char             comm_packet_buf[ COMM_BUF_SIZE ];
static volatile int     new_data;


static inline int parse_javiator_data( const comm_packet_t *packet )
{
    int res = javiator_data_from_stream( &javiator_data, packet->payload, packet->size );

    new_data = 1;

    return( res );
}

static int process_javiator_packet( const comm_packet_t *packet )
{
    if( !packet )
    {
        return( -1 );
    }

    switch( packet->type )
    {
        case COMM_JAVIATOR_DATA:
            return parse_javiator_data( packet );

        default:
            return terminal_port_forward( packet );
    }

    return( -1 );
}

int javiator_port_send_ctrl_period( int period )
{
    char buf[1] = { (char) period };
    comm_packet_t packet;

    packet.type     = COMM_CTRL_PERIOD;
    packet.size     = 1;
    packet.buf_size = 1;
    packet.payload  = buf;

    return comm_send_packet( comm_channel, &packet );
}

int javiator_port_send_enable_sensors( int enable )
{
    char buf[1] = { (char) enable };
    comm_packet_t packet;

    packet.type     = COMM_EN_SENSORS;
    packet.size     = 1;
    packet.buf_size = 1;
    packet.payload  = buf;

    return comm_send_packet( comm_channel, &packet );
}

int javiator_port_send_motor_signals( const motor_signals_t *signals )
{
    char buf[ MOTOR_SIGNALS_SIZE ];
    comm_packet_t packet;

    motor_signals_to_stream( signals, buf, MOTOR_SIGNALS_SIZE );

    packet.type     = COMM_MOTOR_SIGNALS;
    packet.size     = MOTOR_SIGNALS_SIZE;
    packet.buf_size = MOTOR_SIGNALS_SIZE;
    packet.payload  = buf;

    return comm_send_packet( comm_channel, &packet );
}

static int running;
static pthread_t thread;
static void *javiator_thread(void *arg)
{
	int res;
	while (running) {
		comm_channel->poll(comm_channel, 0);
		
		res = comm_recv_packet( comm_channel, &comm_packet );
	}

	return NULL;
}

static void start_javiator_thread(void)
{
	struct sched_param param;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	
	sched_getparam(0, &param);
	if (param.sched_priority > 0) {
		param.sched_priority++;
		pthread_attr_setschedparam(&attr, &param);		
		pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	}

	pthread_create(&thread, &attr, javiator_thread, NULL);

}


int javiator_port_init( comm_channel_t *channel )
{
    motor_signals_t signals = { 0, 0, 0, 0 };

    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &comm_packet,   0, sizeof( comm_packet ) );

    comm_channel         = channel;
    comm_packet.buf_size = COMM_BUF_SIZE;
    comm_packet.payload  = comm_packet_buf;
    new_data             = 0;

    javiator_port_send_motor_signals( &signals );

    return( 0 );
}

int javiator_port_tick( void )
{
    int res = comm_recv_packet( comm_channel, &comm_packet );

    if( res == 0 )
    {
        process_javiator_packet( &comm_packet );
    }
    else
    if( res == EAGAIN )
    {
        return( res );
    }
    else
    {
        fprintf( stderr, "ERROR: cannot receive from JAviator channel\n" );
    }

    return( res );
}

int javiator_port_get_data( javiator_data_t *data )
{
    motor_signals_t signals = { 0, 0, 0, 0 };
    int res, attempts = 0;

    do
    {
        res = javiator_port_tick( );

        if( res == EAGAIN )
        {
            if( ++attempts > 1000 )
            {
                return( -1 );
            }

            javiator_port_send_motor_signals( &signals );
            sleep_for( 1000 );
        }
        else
        if( res != 0 )
        {
            return( -1 );
        }
    } while( !new_data );

    memcpy( data, &javiator_data, sizeof( *data ) );
    new_data = 0;

    return( 0 );
}

int javiator_port_forward( const comm_packet_t *packet )
{
    return comm_send_packet( comm_channel, packet );
}

/* End of file */
