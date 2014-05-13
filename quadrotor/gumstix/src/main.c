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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "comm_channel.h"
#include "serial_channel.h"
#include "socket_channel.h"
#include "javiator_port.h"
#include "terminal_port.h"
#include "ubisense_port.h"
#include "control_loop.h"
#include "controller-config.h"

#define CONTROLLER_PERIOD   15      /* main control period in ms that serves as central clock */
#define EXEC_CONTROL_LOOP   1       /* execute control loop (needed for setting priorities) */
#define PERIOD_MULTIPLIER   1       /* communicate with control terminal every <value> period */
#define ACTIVATE_UBISENSE   0       /* activate socket for receiving Ubisense position data */
#define REDUCED_CTRL_CMDS   1000    /* reduce control commands when error less than <value> mm */
#ifdef TARGET_CPU_X86
#define SERIAL_DEVICE       "/dev/ttyS0"
#elif TARGET_CPU_ARM
#define SERIAL_DEVICE       "/dev/ttyS2"
#else
#error Unknown target CPU
#endif
#define SERIAL_BAUDRATE     115200
#define TERMINAL_PORT       7000
#define UBISENSE_PORT       9000
#define CFG_FILENAME        "control.cfg"

static comm_channel_t       javiator_channel;
static comm_channel_t       terminal_channel;
static comm_channel_t       ubisense_channel;


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

static int setup_terminal_port( int type, int listen_port, int multiplier )
{
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

static int setup_ubisense_port( int type, int listen_port, int tag )
{
    if( socket_channel_create( &ubisense_channel, type ) )
    {
        fprintf( stderr, "ERROR: unable to create socket channel\n" );
        return( -1 );
    }

    if( socket_channel_init( &ubisense_channel, type, NULL, listen_port ) )
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
            "\t -c        ... disable control loop\n"
            "\t -h        ... print this message\n"
            "\t -i ID     ... ID of installed Ubisense tag\n"
            "\t -m value  ... communicate with control terminal every <value> period\n"
            "\t -r radius ... reduce control commands when error less than <radius> mm\n"
            "\t -s        ... setup communication channel for receiving Ubisense data\n"
            "\t -t time   ... controller period in milliseconds\n"
            "\t -u        ... use TCP instead of UDP for ground communication\n"
            , binary );
}

int main( int argc, char **argv )
{
    double offset_p   = 0;
    double offset_r   = 0;
    int    ubi_tag_id = 0;
    int    ms_period  = CONTROLLER_PERIOD;
    int    exec_loop  = EXEC_CONTROL_LOOP;
    int    multiplier = PERIOD_MULTIPLIER;
    int    ubisense   = ACTIVATE_UBISENSE;
    int    ctrl_gain  = REDUCED_CTRL_CMDS;
    int    conn_type  = SOCK_UDP;
    int    opt, fd;

    memset( &javiator_channel, 0, sizeof( javiator_channel ) );
    memset( &terminal_channel, 0, sizeof( terminal_channel ) );
    memset( &ubisense_channel, 0, sizeof( ubisense_channel ) );

    while( (opt = getopt( argc, argv, "chi:m:r:st:u" )) != -1 )
    {
        switch( opt )
        {
            case 'c':
                exec_loop = 0;
                break;

            case 'i':
                if( (ubi_tag_id = atoi( optarg )) < 0 )
                {
                    fprintf( stderr, "ERROR: option '-i' requires a positive value\n" );
                    usage( argv[0] );
                    exit( 1 );
                }
                break;

            case 'm':
                if( (multiplier = atoi( optarg )) < 1 )
                {
                    fprintf( stderr, "ERROR: option '-m' requires a value > 0\n" );
                    usage( argv[0] );
                    exit( 1 );
                }
                break;

            case 'r':
                if( (ctrl_gain = atoi( optarg )) < 1 )
                {
                    fprintf( stderr, "ERROR: option '-r' requires a value > 0\n" );
                    usage( argv[0] );
                    exit( 1 );
                }
                break;

            case 's':
                ubisense = 1;
                break;

            case 't':
                if( (ms_period = atoi( optarg )) < 1 )
                {
                    fprintf( stderr, "ERROR: option '-t' requires a value > 0\n" );
                    usage( argv[0] );
                    exit( 1 );
                }
                break;

            case 'u':
                conn_type = SOCK_SERVER;
                break;

            case 'h':
            default:
                usage( argv[0] );
                exit( 1 );
        }
    }

    printf( "loading configuration ... " );

    if( (fd = open( CFG_FILENAME, O_RDONLY )) < 0 )
    {
        printf( "failed\n" );
        fprintf( stderr, "ERROR: could not open file %s\n", CFG_FILENAME );
    }
    else
    {
        read( fd, &offset_r, sizeof( offset_r ) );
        read( fd, &offset_p, sizeof( offset_p ) );

        if( !ubi_tag_id )
        {
            read( fd, &ubi_tag_id, sizeof( ubi_tag_id ) );
        }

        close( fd );
        printf( "ok\noffset roll  = %1.3f\noffset pitch = %1.3f\nUbisense tag = %d\n",
            offset_r, offset_p, ubi_tag_id );
    }

    printf( "setting up JAviator port ... " );

    if( setup_javiator_port( SERIAL_DEVICE, SERIAL_BAUDRATE ) )
    {
        printf( "failed\n" );
        fprintf( stderr, "ERROR: could not setup the JAviator port\n" );
        exit( 1 );
    }

    printf( "ok\n" );

    printf( "setting up Terminal port ... " );

    if( setup_terminal_port( conn_type, TERMINAL_PORT, multiplier ) )
    {
        printf( "failed\n" );
        fprintf( stderr, "ERROR: could not setup the Terminal port\n" );
        exit( 1 );
    }

    printf( "ok\n" );

    if( ubisense )
    {
        printf( "setting up Ubisense port ... " );

        if( setup_ubisense_port( SOCK_UDP, UBISENSE_PORT, ubi_tag_id ) )
        {
            printf( "failed\n" );
            fprintf( stderr, "ERROR: could not setup the Ubisense port\n" );
            exit( 1 );
        }

        printf( "ok\n" );
    }

    if( exec_loop )
    {
        printf( "setting up control loop\n" );
        control_loop_setup( ms_period, ctrl_gain, ubisense, &offset_r, &offset_p );
        printf( "starting control loop\n" );
        control_loop_run( );
    }

    if( javiator_channel.data )
    {
        printf( "destroying JAviator port ... " );
        serial_channel_destroy( &javiator_channel );
        printf( "ok\n" );
    }

    if( terminal_channel.data )
    {
        printf( "destroying Terminal port ... " );
        socket_channel_destroy( &terminal_channel, conn_type );
        printf( "ok\n" );
    }

    if( ubisense_channel.data )
    {
        printf( "destroying Ubisense port ... " );
        socket_channel_destroy( &ubisense_channel, SOCK_CLIENT );
        printf( "ok\n" );
    }

    printf( "saving configuration ... " );

    if( (fd = open( CFG_FILENAME, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR )) < 0 )
    {
        printf( "failed\n" );
        fprintf( stderr, "ERROR: could not open file %s\n", CFG_FILENAME );
    }
    else
    {
        write( fd, &offset_r, sizeof( offset_r ) );
        write( fd, &offset_p, sizeof( offset_p ) );
        write( fd, &ubi_tag_id, sizeof( ubi_tag_id ) );
        close( fd );
        printf( "ok\n" );
    }

    return( 0 );
}

/* End of file */
