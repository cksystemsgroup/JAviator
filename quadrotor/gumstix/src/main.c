/* $Id: main.c,v 1.7 2008/12/18 15:07:18 rtrummer Exp $ */

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
#include <stdio.h>

#include "controller.h"
#include "control_loop.h"
#include "spi_channel.h"
#include "serial_channel.h"
#include "socket_channel.h"
#include "javiator_port.h"
#include "inertial_port.h"
#include "terminal_port.h"


#include "../shared/protocol.h"
#include "../shared/transfer.h"
#include "utimer.h"


#define CONTROLLER_PERIOD   14 // [ms]
#define PERIOD_MULTIPLIER   5  // communicate with terminal every 5th period
#define Z_AXIS_CONTROLLER   1  // enable z-axis controller
#define EXEC_CONTROL_LOOP   1  // execute control loop
#define SPI_DEVICE          "/dev/mem"
#define SPI_BAUDRATE        115200
#define SERIAL_DEVICE       "/dev/ttyS2"
#define SERIAL_BAUDRATE     115200
#define TERMINAL_PORT       7000

static struct channel       javiator_channel;
//static struct channel       inertial_channel;
static struct channel       terminal_channel;


static int setup_javiator_port( char *device, int baudrate )
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
/*
static int setup_inertial_port( char *device, int baudrate )
{
    if( serial_channel_create( &inertial_channel ) )
    {
        fprintf( stderr, "ERROR: unable to create UART channel\n" );
        return( -1 );
    }

    if( serial_channel_init( &inertial_channel, device, baudrate ) )
    {
        fprintf( stderr, "ERROR: cannot initialize UART channel\n" );
        return( -1 );
    }

    if( inertial_port_init( &inertial_channel ) )
    {
        fprintf( stderr, "ERROR: IMU port not correctly initialized\n" );
        return( -1 );
    }

    return( 0 );
}
*/
static int setup_terminal_port( int listen_port, int multiplier )
{
    memset( &terminal_channel, 0, sizeof( terminal_channel ) );

    if( socket_channel_create( &terminal_channel ) )
    {
        fprintf( stderr, "ERROR: unable to create socket channel\n" );
        return( -1 );
    }

    if( socket_channel_init( &terminal_channel, SOCK_SERVER, NULL, listen_port ) )
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

static void usage( char *binary )
{
    printf( "usage: %s [OPTIONS]\n"
            "OPTIONS are:\n"
            "\t -u      ... print this message\n"
            "\t -t time ... controller period in milliseconds\n"
            "\t -m mult ... send data every <mult> period to terminal\n"
            "\t -z      ... disable z-controller\n"
            "\t -c      ... disable control loop\n"
            , binary );
}

int main( int argc, char **argv )
{
    const char MSG_EN_SENSORS[7] = {0xFF,0xFF,COMM_EN_SENSORS,0x01,0x01,0x00,COMM_EN_SENSORS+2};

    int period     = CONTROLLER_PERIOD;
    int multiplier = PERIOD_MULTIPLIER;
    int control_z  = Z_AXIS_CONTROLLER;
    int exec_loop  = EXEC_CONTROL_LOOP;
    int i;

    for( i = 0; i < argc; ++i )
    {
        if( !strcmp( argv[i], "-u" ) )
        {
            usage( argv[0] );
            exit( 1 );
        }
        else
        if( !strcmp( argv[i], "-t" ) )
        {
            if( ++i < argc )
            {
                period = atoi( argv[i] );
            }

            if( period < 1 )
            {
                fprintf( stderr, "ERROR: option '-t' requires a period greater zero\n" );
                usage( argv[0] );
                exit( 1 );
            }
        }
        else
        if( !strcmp( argv[i], "-m" ) )
        {
            if( ++i < argc )
            {
                multiplier = atoi( argv[i] );
            }
        }
        else
        if( !strcmp( argv[i], "-z" ) )
        {
            control_z = 0;
        }
        else
        if( !strcmp( argv[i], "-c" ) )
        {
            exec_loop = 0;
        }
    }

    printf( "setup JAviator port\n" );

    if( setup_javiator_port( SPI_DEVICE, SPI_BAUDRATE ) )
    {
        fprintf( stderr, "ERROR: could not setup the JAviator port\n" );
        exit( 1 );
    }
/*
    printf( "setup IMU port\n" );

    if( setup_inertial_port( SERIAL_DEVICE, SERIAL_BAUDRATE ) )
    {
        fprintf( stderr, "ERROR: could not setup the IMU port\n" );
        exit( 1 );
    }
*/
    printf( "setup terminal port\n" );

    if( setup_terminal_port( TERMINAL_PORT, multiplier ) )
    {
        fprintf( stderr, "ERROR: could not setup the terminal port\n" );
        exit( 1 );
    }

    if( exec_loop )
    {
        if( javiator_channel.transmit( &javiator_channel, MSG_EN_SENSORS, sizeof( MSG_EN_SENSORS ) ) )
        {
            fprintf( stderr, "ERROR: cannot transmit COMM_EN_SENSORS\n" );
        }

        printf( "setup control loop\n" );
        control_loop_setup( period, control_z );
        printf( "start control loop\n" );
        control_loop_run( );
    }

    spi_channel_destroy( &javiator_channel );
    //serial_channel_destroy( &inertial_channel );
    socket_channel_destroy( &terminal_channel );

    return( 0 );
}

// End of file.
