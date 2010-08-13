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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "comm_channel.h"
#include "serial_channel.h"
#include "socket_channel.h"
#include "javiator_port.h"
#include "terminal_port.h"
#include "ubisense_port.h"
#include "control_loop.h"

#define SERIAL_DEVICE       "/dev/ttyS2"
#define SERIAL_BAUDRATE     115200

static comm_channel_t       javiator_channel;


static int setup_javiator_port( char *device, int baudrate )
{
    if( serial_channel_create( &javiator_channel ) )
    {
        fprintf( stderr, "ERROR: unable to create serial channel\n" );
        return( -1 );
    }

    if( serial_channel_init( &javiator_channel, device, baudrate ) )
    {
        fprintf( stderr, "ERROR: cannot initialize serial channel\n" );
        return( -1 );
    }

    if( javiator_port_init( &javiator_channel ) )
    {
        fprintf( stderr, "ERROR: JAviator port not correctly initialized\n" );
        return( -1 );
    }

    return( 0 );
}

static void usage( char *binary )
{
    printf( "usage: %s [OPTIONS]\n"
            "OPTIONS are:\n"
            "\t -c       ... disable control loop\n"
            "\t -m ID    ... motor ID (front: 1, right: 2, rear: 3, left: 4)\n"
            "\t -s value ... speed in percent (default = 40)\n"
            , binary );
}

int main( int argc, char **argv )
{
    int setup = 1;
    int motor = 0;
    int speed = 40;
	int opt;

    memset( &javiator_channel, 0, sizeof( javiator_channel ) );

	while( (opt = getopt( argc, argv, "cm:s:" )) != -1 )
    {
		switch( opt )
		{
			case 'c':
				setup = 0;
				break;

			case 'm':
                motor = atoi( optarg );
				break;

			case 's':
                speed = atoi( optarg );
                if( speed < 0 || speed > 100 )
                {
                    fprintf( stderr, "ERROR: invalid speed value '%d'\n", speed );
                    usage( argv[0] );
                    exit( 1 );
                }
				break;

			case 'h':
			default:
				usage( argv[0] );
				exit( 1 );
		}
	}

    if( setup && (motor < 1 || motor > 4) )
    {
        fprintf( stderr, "ERROR: invalid motor ID '%d'\n", motor );
        usage( argv[0] );
        exit( 1 );
    }

    if( setup_javiator_port( SERIAL_DEVICE, SERIAL_BAUDRATE ) )
    {
        fprintf( stderr, "ERROR: could not setup the JAviator port\n" );
        exit( 1 );
    }

    if( setup )
    {
        control_loop_setup( );
        control_loop_run( motor, speed );
    }

    if( javiator_channel.data )
    {
        serial_channel_destroy( &javiator_channel );
    }

    return( 0 );
}

/* End of file */
