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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "protocol.h"
#include "transfer.h"
#include "comm_channel.h"
#include "spi_channel.h"
#include "serial_channel.h"
#include "socket_channel.h"
#include "javiator_port.h"
#include "inertial_port.h"
#include "terminal_port.h"
#include "ubisense_port.h"
#include "control_loop.h"

#define PERIOD_MULTIPLIER   1 /* communicate with terminal every period */
#define Z_AXIS_CONTROLLER   1 /* enable z-axis controller */
#define EXEC_CONTROL_LOOP   1 /* execute control loop */
#define SPI_DEVICE          "/dev/mem"
#define SPI_BAUDRATE        115200
#define SERIAL_DEVICE       "/dev/ttyS2"
#define SERIAL_BAUDRATE     115200
#define TERMINAL_PORT       7000
#define UBISENSE_ADDR       "192.168.1.3"
#define UBISENSE_PORT       9001
#define UBISENSE_TAG        20235 /* 21098 */

static comm_channel_t       javiator_channel;
static comm_channel_t       inertial_channel;
static comm_channel_t       terminal_channel;
static comm_channel_t       ubisense_channel;

static int setup_spi_dev_javiator_port( char *device, int baudrate )
{
    if( spi_dev_channel_create( &javiator_channel ) )
    {
        fprintf( stderr, "ERROR: unable to create SPI channel\n" );
        return( -1 );
    }

    if( spi_dev_channel_init( &javiator_channel, NULL, 0 ) )
    {
        fprintf( stderr, "ERROR: cannot initialize SPI channel\n" );
        return( -1 );
    }

    if( javiator_port_init( &javiator_channel ) )
    {
        fprintf( stderr, "ERROR: JAviator port not correctly initialized\n" );
        return( -1 );
    }

    return( 0 );
}

static int setup_spi_javiator_port( char *device, int baudrate )
{
    if( spi_channel_create( &javiator_channel ) )
    {
        fprintf( stderr, "ERROR: unable to create SPI channel\n" );
        return( -1 );
    }

    if( spi_channel_init( &javiator_channel, device, baudrate ) )
    {
        fprintf( stderr, "ERROR: cannot initialize SPI channel\n" );
        return( -1 );
    }

    if( javiator_port_init( &javiator_channel ) )
    {
        fprintf( stderr, "ERROR: JAviator port not correctly initialized\n" );
        return( -1 );
    }

    return( 0 );
}

static int setup_inertial_port( char *device, int baudrate, int automatic )
{
    if( serial_channel_create( &inertial_channel ) )
    {
        fprintf( stderr, "ERROR: unable to create serial channel\n" );
        return( -1 );
    }

    if( serial_channel_init( &inertial_channel, device, baudrate ) )
    {
        fprintf( stderr, "ERROR: cannot initialize serial channel\n" );
        return( -1 );
    }

    if( inertial_port_init( &inertial_channel, automatic ) )
    {
        fprintf( stderr, "ERROR: inertial port not correctly initialized\n" );
        return( -1 );
    }

    return( 0 );
}

static int setup_serial_javiator_port( char *device, int baudrate )
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

static int setup_javiator_port(channel_type_t type)
{
	switch (type)
	{
		case CH_SPI:
			return setup_spi_javiator_port(SPI_DEVICE, SPI_BAUDRATE); 
		break;
		case CH_SPI_DEV:
			return setup_spi_dev_javiator_port(SPI_DEVICE, SPI_BAUDRATE); 
		break;
		case CH_SERIAL:
			return setup_serial_javiator_port(SERIAL_DEVICE, SERIAL_BAUDRATE);
		break;
		default:
			fprintf(stderr, "ERROR: unknown javiator channel type\n");
			return -1;
	}
}

static int setup_terminal_port( int listen_port, int type, int multiplier )
{
    memset( &terminal_channel, 0, sizeof( terminal_channel ) );

    if( socket_channel_create( &terminal_channel, type ) )
    {
        fprintf( stderr, "ERROR: unable to create socket channel\n" );
        return( -1 );
    }

    if( socket_channel_init( &terminal_channel, type, NULL, listen_port ) )
    {
        fprintf( stderr, "ERROR: unable to initialize socket channel\n" );
        return( -1 );
    }

    if( terminal_port_init( &terminal_channel ) )
    {
        fprintf( stderr, "ERROR: terminal port not correctly initialized\n" );
        return( -1 );
    }

    terminal_port_set_multiplier( multiplier );
    return( 0 );
}

static int setup_ubisense_port( int type, char *addr, int port, int tag )
{
    memset( &ubisense_channel, 0, sizeof( ubisense_channel ) );

    if( socket_channel_create( &ubisense_channel, type ) )
    {
        fprintf( stderr, "ERROR: unable to create socket channel\n" );
        return( -1 );
    }

    if( socket_channel_init( &ubisense_channel, type, addr, port ) )
    {
        fprintf( stderr, "ERROR: unable to initialize socket channel\n" );
        return( -1 );
    }

    if( ubisense_port_init( &ubisense_channel, tag ) )
    {
        fprintf( stderr, "ERROR: Ubisense port not correctly initialized\n" );
        return( -1 );
    }

    return( 0 );
}

static void usage( char *binary )
{
    printf( "usage: %s [OPTIONS]\n"
            "OPTIONS are:\n"
            "\t -a      ... if IMU is connected directly use it in automatic mode\n"
            "\t -c      ... disable control loop\n"
            "\t -h      ... print this message\n"
            "\t -i      ... use IMU connected directly to serial device %s\n"
			"\t               only possible if an SPI device is used\n"
            "\t -j num  ... connect to JAviator through \n"
			"\t               num == 1: user SPI\n"
			"\t               num == 2: kernel SPI\n"
			"\t               num == 3: serial device %s\n"
            "\t -m mult ... send data every <mult> period to terminal\n"
            "\t -t time ... controller period in milliseconds\n"
            "\t -u      ... use TCP socket instead of UDP socket\n"
            "\t -z      ... disable z-controller\n"
            , binary, SERIAL_DEVICE, SERIAL_DEVICE);
}

int main( int argc, char **argv )
{
    int period     = CONTROLLER_PERIOD;
    int multiplier = PERIOD_MULTIPLIER;
    int control_z  = Z_AXIS_CONTROLLER;
    int exec_loop  = EXEC_CONTROL_LOOP;
	int setup_imu = 0;
	int automatic_imu = 0;
	int opt;
	int conn_type = SOCK_UDP;
	channel_type_t type = CH_SERIAL;

	while((opt = getopt(argc, argv, "achij:m:t:uz")) != -1)
    {
		switch(opt)
		{
			case 'a':
				automatic_imu = 1;
				break;
			case 'c':
				exec_loop = 0;
				break;
			case 'i':
				setup_imu = 1;
				break;
			case 'm':
				multiplier = atoi(optarg);
				if (multiplier < 1) {
					fprintf( stderr, "ERROR: option '-t' requires a period greater zero\n" );
					usage( argv[0] );
					exit( 1 );
				}
				break;
			case 'j':
				type = atoi(optarg);
				if (type < 1 || type >= CH_MAX_TYPE) {
					fprintf( stderr, "ERROR: option '-s' requires btw 1 ... %d\n", 
							CH_MAX_TYPE - 1 );
					usage( argv[0] );
					exit( 1 );
				}
				break;
			case 't':
				period = atoi(optarg);
				break;
			case 'z':
				control_z = 0;
				break;
			case 'u':
				conn_type = SOCK_SERVER;
				break;
			case 'h':
			default:
				usage(argv[0]);
				exit(1);
		}
	}

    printf( "setting up JAviator port ... " );
    if (setup_javiator_port(type))
    {
        printf( "failed\n" );
        fprintf( stderr, "ERROR: could not setup the JAviator port\n" );
        exit( 1 );
    }
    printf( "ok\n" );

	if (type != CH_SERIAL && setup_imu) {
		printf( "setting up Inertial port ... " );
		if(setup_inertial_port(SERIAL_DEVICE, SERIAL_BAUDRATE, automatic_imu)) {
			printf( "failed\n" );
			fprintf( stderr, "ERROR: could not setup the Inertial port\n" );
			exit( 1 );
		}
		printf( "ok\n" );
	}

	printf( "setting up Terminal port ... " );
    if( setup_terminal_port( TERMINAL_PORT, conn_type, multiplier ) )
    {
        printf( "failed\n" );
        fprintf( stderr, "ERROR: could not setup the Terminal port\n" );
        exit( 1 );
    }
    printf( "ok\n" );

	printf( "setting up Ubisense port ... " );
    if( setup_ubisense_port( SOCK_CLIENT,
        UBISENSE_ADDR, UBISENSE_PORT, UBISENSE_TAG ) )
    {
        printf( "failed\n" );
        fprintf( stderr, "ERROR: could not setup the Ubisense port\n" );
        exit( 1 );
    }
    printf( "ok\n" );

    if( exec_loop )
    {
        printf( "setting up control loop\n" );
        control_loop_setup( period, control_z );
        printf( "starting control loop\n" );
        control_loop_run( );
    }

    return( 0 );
}

/* End of file */
