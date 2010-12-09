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

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "controller.h"
#include "motor_signals.h"

/* State of a controller for 1 degree of freedom */
struct ctrl_state
{
    double dt;         /* Control period [s] */
    double kp;         /* Gain for tracking error [PWM/mrad] */
    double ki;         /* Gain for integral of tracking error [PWM/(mrad*s)] */
    double kd;         /* Gain for derivative of tracking error [PWM/(mrad/s)] */
    double kdd;        /* Gain for second derivative of tracking error */
    double integral;   /* Running integral of the tracking error [mrad*s] */
    double int_limit;  /* Integral limit [mrad*s] */
    double last_value; /* Stores a controller-dependent last value */
    double p;
    double i;
    double d;
    double dd;
};

static double pidd_compute( ctrl_state_t *state,
    double s_error, double v_error, double acceleration )
{
    state->integral += s_error * state->dt;

    /* Saturate the integral (anti-windup) */
    if( state->integral > state->int_limit )
    {
        state->integral = state->int_limit;
    }
    else
    if( state->integral < -state->int_limit )
    {
        state->integral = -state->int_limit;
    }

    /* Compute the contribution of each metric of the angle error */
    state->p  = state->kp  * s_error;         /* error contribution to control effort */
    state->i  = state->ki  * state->integral; /* error integral contribution to control effort */
    state->d  = state->kd  * v_error;         /* velocity error contribution to control effort */
    state->dd = state->kdd * acceleration;    /* acceleration error contribution to control effort */

    return( state->p + state->i + state->d + state->dd );
}

static inline double get_s_error( double desired, double current )
{
    return( desired - current );
}

static inline double get_v_error(
    double desired, double last_desired, double velocity, double period )
{
    double desired_velocity = (desired - last_desired) / period;
    double v_error = desired_velocity - velocity;

    return( v_error );
}

/* PIDD controller for a single degree of freedom.
 *
 * Computes linear feedback control effort.  Initially used PID (proportional,
 * integral, derivative) but now also uses the second derivative (double derivative,
 * dd).  It processes the desired setpoint commands to calculate the error, integral
 * of the error, and derivatives of the error.  Integral antiwindup is included as
 * well.
 * \param controller   *controller, a pointer to control parameters and internal state
 * \param current      double, the current value of the angle [mrad]
 * \param desired      double, the desired (commanded) value of the angle [mrad]
 * \param velocity     double, the current value of the angular velocity [mrad/s]
 * \param acceleration double, the current value of the angular acceleration [mrad/s^2]
 *
 * \return Requested control effort for the degree of freedom.
 */
static double pidd_do_control( controller_t *controller,
    double desired, double current, double velocity, double acceleration )
{
    /* Local definition to avoid double indirection in use */
    ctrl_state_t *state = controller->state;
    double s_error = get_s_error( desired, current );
    double v_error = get_v_error( desired, state->last_value, velocity, state->dt );

    state->last_value = desired;

    return pidd_compute( state, s_error, v_error, acceleration );
}

static double pidd_x_y_control( controller_t *controller,
    double desired, double current, double velocity, double acceleration )//delay )
{
    /* Local definition to avoid double indirection in use */
    ctrl_state_t *state = controller->state;
    double s_error = get_s_error( desired, current );
    //double v_error = (current - state->last_value) / delay;

    state->last_value = current;

    return pidd_compute( state, s_error, velocity, acceleration );//v_error );
}

static int pidd_set_params( controller_t *controller,
    double kp, double ki, double kd, double kdd )
{
    ctrl_state_t *state = controller->state;

    state->kp  = kp;
    state->ki  = ki;
    state->kd  = kd;
    state->kdd = kdd;

    return( 0 );
}

static int pidd_reset_zero( controller_t *controller )
{
    controller->state->integral = 0;

    return( 0 );
}

/* Initialize PIDD controller.
 *
 * Allocate memory for the state, set constants for the state and for the controller.
 * \param controller *controller, pointer to the controller to be created
 * \param period double, control loop period [s]
 * \return If allocation succeeds, returns 0, otherwise returns -1.
 */
int pidd_def_controller_init( controller_t *controller, double period )
{
    ctrl_state_t *state = malloc( sizeof( ctrl_state_t ) );

    if( !state )
    {
        fprintf( stderr, "ERROR: memory allocation for %s controller failed\n",
            controller->name );
        return( -1 );
    }

    memset( state, 0, sizeof( ctrl_state_t ) );

    state->dt         = period;
    state->int_limit  = MOTOR_MAX;
    state->last_value = 0;

    controller->do_control = pidd_do_control;
    controller->set_params = pidd_set_params;
    controller->reset_zero = pidd_reset_zero;
    controller->state      = state;

    return( 0 );
}

int pidd_x_y_controller_init( controller_t *controller, double period )
{
    int res = pidd_def_controller_init( controller, period );

    controller->state->int_limit /= 100;
    controller->do_control = pidd_x_y_control;

    return( res );
}

int pidd_all_controller_destroy( controller_t *controller )
{
    ctrl_state_t *state = controller->state;

    controller->state = NULL;
    free( state );

    return( 0 );
}

double controller_get_term_P( controller_t *controller )
{
    return( controller->state->p );
}

double controller_get_term_I( controller_t *controller )
{
    return( controller->state->i );
}

double controller_get_term_D( controller_t *controller )
{
    return( controller->state->d );
}

double controller_get_term_DD( controller_t *controller )
{
    return( controller->state->dd );
}

double controller_get_integral( controller_t *controller )
{
    return( controller->state->integral );
}

void controller_set_integral( controller_t *controller, double value )
{
    controller->state->integral = value;
}

/* End of file */
