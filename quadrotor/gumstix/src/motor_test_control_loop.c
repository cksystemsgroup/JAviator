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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "control_loop.h"
#include "communication.h"
#include "comm_channel.h"
#include "protocol.h"
#include "javiator_port.h"
#include "javiator_data.h"
#include "motor_signals.h"
#include "us_timer.h"

/* control loop parameters */
static volatile int             running;
static int                      us_period;
static long long                next_period;

/* data structures */
static javiator_ldat_t          javiator_data;
static motor_signals_t          motor_signals;

/* function pointers */
static void                     signal_handler( int num );
static void                     int_handler( int num );


/****************************************
 *          Control Loop Code           *
 ****************************************/

int control_loop_setup( void )
{
    struct sigaction act;

    running             = 1;
    us_period           = 20000;
    next_period         = 0;
    act.sa_handler      = signal_handler;
	act.sa_handler      = int_handler;

    if( sigaction( SIGUSR1, &act, NULL ) || sigaction( SIGINT, &act, NULL ) )
    {
        perror( "sigaction" );
    }

    memset( &javiator_data, 0, sizeof( javiator_data ) );
    memset( &motor_signals, 0, sizeof( motor_signals ) );

    return( 0 );
}

static int get_javiator_data( void )
{
    int res = javiator_port_get_data( &javiator_data );

    if( res )
    {
        fprintf( stderr, "ERROR: data from JAviator not available\n" );
        return( res );
    }

    return( 0 );
}

static int send_motor_signals( void )
{
    int res = javiator_port_send_motor_signals( &motor_signals );

    if( res == -1 )
    {
        fprintf( stderr, "ERROR: connection to JAviator broken\n" );
        res = javiator_port_send_motor_signals( &motor_signals );

        if( res == -1 )
        {
            fprintf( stderr, "ERROR: no success with reconnecting to JAviator\n" );
        }
    }

    return( res );
}

static int wait_for_next_period( void )
{
    if( sleep_until( next_period ) )
    {
        fprintf( stderr, "error in sleep_until\n" );
        exit( 1 );
    }

    next_period += us_period;
    return( 0 );
}

static void signal_handler( int num )
{
}

static void int_handler( int num )
{
	running = 0;
}

int control_loop_run( int motor, int speed )
{
    int rev_up = 1;
    int signal = 0;
    comm_packet_t packet;

    memset( &packet, 0, sizeof( packet ) );
    packet.type = COMM_SWITCH_STATE;
    javiator_port_forward( &packet );

    speed *= 160;
    next_period = get_utime( ) + us_period;

    while( running )
    {
        if( send_motor_signals( ) )
        {
            break;
        }

	    if( get_javiator_data( ) )
        {
            fprintf( stderr, "ERROR: connection to JAviator broken\n" );
            break;
	    }

        if( rev_up )
        {
            if( signal < speed )
            {
                signal += 25;
            }
            else
            {
                rev_up = 0;
            }
        }
        else
        {
            if( signal > speed / 2 )
            {
                signal -= 25;
            }
            else
            {
                rev_up = 1;
            }
        }

        if( signal < MOTOR_MIN )
        {
            signal = MOTOR_MIN;
        }
        else
        if( signal > MOTOR_MAX )
        {
            signal = MOTOR_MAX;
        }

        switch( motor )
        {
            case 1:
                motor_signals.front = (int16_t) signal;
                fprintf( stdout, "    Front " );
                break;

            case 2:
                motor_signals.right = (int16_t) signal;
                fprintf( stdout, "    Right " );
                break;

            case 3:
                motor_signals.rear  = (int16_t) signal;
                fprintf( stdout, "    Rear " );
                break;

            case 4:
                motor_signals.left  = (int16_t) signal;
                fprintf( stdout, "    Left " );
                break;
        }

        fprintf( stdout, "Motor: %5d  \r", signal );
        fflush( stdout );
        wait_for_next_period( );
    }

    packet.type = COMM_SHUT_DOWN;
    javiator_port_forward( &packet );
    wait_for_next_period( );
    return( 0 );
}

int control_loop_stop( void )
{
    running = 0;
    return( 0 );
}

/* End of file */
