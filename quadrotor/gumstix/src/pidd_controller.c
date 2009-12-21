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

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "controller.h"
#include "motor_signals.h"

/** \brief State of a controller for 1 degree of freedom. */
struct controller_state {
    double dtime;     //!< Control period (s)
    double Kp;        //!< Gain for tracking error (PWM/rad)
    double Ki;        //!< Gain for integral of tracking error (PWM/(rad*s))
    double Kd;        //!< Gain for derivative of tracking error (PWM/(rad/s))
    double Kdd;       //!< Gain for double derivative (second derivative) of tracking error
    double integral;    //!< Running integral of the tracking error (rad*s)
    double iMax;        //!< Maximum integral value (rad*s)
    double iMin;        //!< Minimum integral value (rad*s)
    double last_desired; //!< Stores command to use for finite differencing (rad)
    double pTerm;
    double iTerm;
    double dTerm;
    double ddTerm;
};

static void saturate_integral(struct controller_state *state)
{
    // Saturate the integral (anti-windup)
    if( state->integral > state->iMax )
    {
        state->integral = state->iMax;
    }
    else
    if( state->integral < state->iMin )
    {
        state->integral = state->iMin;
    }
}

/** \brief PIDD math
 *
 */
static double pidd_compute(struct controller_state *state,
    double error, double derror, double acceleration)
{
    // Compute integral of error
    state->integral += error * state->dtime;
    saturate_integral( state );

    // Compute the contribution of each metric of the angle error
    state->pTerm  = state->Kp  * error; //!< error contribution to control effort
    state->iTerm  = state->Ki  * state->integral; //!< error integral contribution to control effort
    state->dTerm  = state->Kd  * derror; //!< velocity error contribution to control effort
    state->ddTerm = state->Kdd * acceleration; //!< acceleration error contribution to control effort

    return( state->pTerm + state->iTerm + state->dTerm + state->ddTerm );
}

static inline double get_error(double current, double desired)
{
    return( desired - current );
}

/** \brief calculate best error for yaw
 *
 */
static inline double get_yaw_error(double current, double desired)
{
    double error = get_error( current, desired );

    if( error < -M_PI )
    {
        error += 2 * M_PI;
    }
    else
    if( error > M_PI )
    {
        error -= 2 * M_PI;
    }

    return( error );
}

static inline double get_velocity_error(double desired,
    double last_desired,
    double velocity,
    double period)
{
    double desired_velocity;
    double derror;
    desired_velocity = (desired - last_desired) / period;
    derror = desired_velocity - velocity;
    return derror;
}

static double pidd_yaw_control(struct controller *controller,
    double current, double desired,
    double velocity, double acceleration)
{
    struct controller_state *state = controller->state;
    double error, derror;

    error = get_yaw_error(current, desired);
    derror = get_velocity_error(desired, state->last_desired, velocity, state->dtime);
    state->last_desired = desired;

    return pidd_compute(state, error, derror, acceleration);
}

/** \brief PID controller for a single degree of freedom.
 *
 * Computes linear feedback control effort.  Initially used PID (proportional,
 * integral, derivative) but now also uses the second derivative (double derivative,
 * dd).  It processes the desired setpoint commands to calculate the error, integral
 * of the error, and derivatives of the error.  Integral antiwindup is included as
 * well.
 * \param controller   *controller, a pointer to control parameters and internal state
 * \param current      double, the current value of the angle(rad)
 * \param desired      double, the desired (commanded) value of the angle (rad)
 * \param velocity     double, the current value of the angular velocity (rad/s)
 * \param acceleration double, the current value of the angular acceleration (rad/s^2)
 *
 * \return Requested control effort for the degree of freedom.
 */
static double pidd_control( struct controller *controller, double current,
    double desired, double velocity, double acceleration )
{
    // Local definition to avoid double indirection in use
    struct controller_state *state  = controller->state;
    double error, derror;

    error = get_error(current, desired);
    derror = get_velocity_error(desired, state->last_desired, velocity, state->dtime);
    state->last_desired = desired;
    return pidd_compute(state, error, derror, acceleration);
}

/** \brief Reset integral to zero.
 *
 * \return 0
 */
static int pid_reset_zero( struct controller *controller )
{
    controller->state->integral = 0;

    return 0;
}

/** \brief Set PIDD gains. */
static int setPIDD( struct controller *controller,
    double p, double i, double d, double dd )
{
    struct controller_state *state = controller->state;

    state->Kp  = p;
    state->Ki  = i;
    state->Kd  = d;
    state->Kdd = dd;
    return 0;
}

/** \brief Initialize PID controller.
 *
 * Allocate memory for the state, set constants for the state and for the controller.
 * \param controller *controller, pointer to the controller to be created
 * \param period int, control loop period (ms)
 * \return If allocation succeeds, returns 0, otherwise returns -1.
 */
int pidd_controller_init(struct controller *controller, int period)
{
    struct controller_state *state;

    state = malloc(sizeof(struct controller_state));

    if( !state )
    {
        return( ENOMEM );
    }

    memset(state, 0, sizeof(struct controller_state));
    state->dtime = period / 1000.0; // controller uses period in seconds
    state->iMax  = MOTOR_MAX;
    state->iMin  = -MOTOR_MAX;
    state->last_desired = 0;

    controller->control    = pidd_control;
    controller->reset_zero = pid_reset_zero;
    controller->set_params = setPIDD;
    controller->state      = state;

    return 0;
}

int pidd_yaw_controller_init(struct controller *controller, int period)
{
    int retval;
    retval = pidd_controller_init(controller, period);
    controller->control = pidd_yaw_control;
    return retval;
}

/** \brief Free the state of the controller, set it to be a null pointer.
 *
 * \return 0
 */
int pidd_controller_destroy(struct controller *controller)
{
    struct controller_state *state;

    state = controller->state;
    controller->state = NULL;
    free( state );

    return( 0 );
}

double controller_get_p_term( struct controller *controller )
{
    return( controller->state->pTerm );
}

double controller_get_i_term( struct controller *controller )
{
    return( controller->state->iTerm );
}

double controller_get_d_term( struct controller *controller )
{
    return( controller->state->dTerm );
}

double controller_get_dd_term( struct controller *controller )
{
    return( controller->state->ddTerm );
}

double controller_get_integral( struct controller *controller )
{
    return( controller->state->integral );
}

void controller_set_integral( struct controller *controller, double value )
{
    controller->state->integral = value;
}

/* End of file */
