/* $Id: pid_controller.c,v 1.19 2008/12/18 15:07:18 rtrummer Exp $ */

/** \file pid_controller.c
*
*/

/*
 * Copyright (c) Harald Roeck hroeck@cs.uni-salzburg.at
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
#include "controller.h"

#define INTEGRAL_LIMIT   1 //!< Limit used for integral anti-windup (rad*s)
#define ERROR_LIMIT      6.284 //!< Limit for yaw winding (2*pi)

/** \brief State of a controller for 1 degree of freedom. */
struct controller_state {
    double dtime;                   //!< Control period (s)
    double Kp;                      //!< Gain for tracking error (PWM/rad)
    double Ki;                      //!< Gain for integral of tracking error (PWM/(rad*s))
    double Kd;                      //!< Gain for derivative of tracking error (PWM/(rad/s))
    double Kdd;                     //!< Gain for double derivative (second derivative) of tracking error
    double integral;                //!< Running integral of the tracking error (rad*s)
    double iMax;                    //!< Maximum integral value (rad*s)
    double iMin;                    //!< Minimum integral value (rad*s)
    double last_desired;            //!< Stores command to use for finite differencing (rad)    
};

/** \brief PID controller for a single degree of freedom.
 * 
 * Computes linear feedback control effort.  Initially used PID (proportional, integral, derivative)
 * but now also uses the second derivative (double derivative, dd).  It processes the desired setpoint
 * commands to calculate the error, integral of the error, and derivatives of the error.  Integral
 * antiwindup is included as well.
 * \param controller          *controller, a pointer to control parameters and internal state
 * \param current_angle        double, the current value of the angle (rad)
 * \param desired_angle        double, the desired (commanded) value of the angle (rad)
 * \param angular_velocity     double, the current value of the angular velocity (rad/s)
 * \param angular_acceleration double, the current value of the angular acceleration (rad/s^2
 *
 * \return Requested control effort for the degree of freedom.
 */
static double pid_control( struct controller *controller,
                           double current_angle,
                           double desired_angle,
                           double angular_velocity,
                           double angular_acceleration )
{
    //TODO: rename this variables, they are not always angle, they can be altitude also
    struct controller_state *state  = controller->state; // Local definition to avoid double indirection in use
    //TODO: from control point of view this is strange we can live it like this but we have to be carefull not to have
    //positive reaction, normaly the error is computed the other way around
    double error                    =  desired_angle - current_angle; // Angle error from desired value
    double desired_angular_velocity = (desired_angle - state->last_desired) / state->dtime; // Finite differenced angular velocity
    double pTerm, iTerm, dTerm, ddTerm; // Individual contributions of each metric of control error

    state->last_desired = desired_angle; // Store for next iteration

    // Unwind yaw
    // TODO: Move yaw unwinding out of the pid_control function
    if( controller->name[0] == 'Y' )
    {
        double normalized_error       = error;
        int    negative_current_angle = current_angle < 0;
        int    negative_desired_angle = desired_angle < 0;

        if( negative_current_angle && !negative_desired_angle )
        {
            normalized_error += ERROR_LIMIT;
        } 
        else if( !negative_current_angle && negative_desired_angle )
        {
            normalized_error -= ERROR_LIMIT;
        }

        if( abs( normalized_error ) < abs( error ) )
        {
            error = normalized_error;
        }
    }

    // Compute integral of error
    state->integral += error * state->dtime;

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
        
    //for z we do not want to use desired_angular_velocity since
    //z ref variation should be very slow
    if( controller->name[0] == 'Z' )
        desired_angular_velocity = 0.0;

    // Compute the contribution of each metric of the angle error
    pTerm  = state->Kp * error; //!< Angle error contribution to control effort
    iTerm  = state->Ki * state->integral; //!< Angle error integral contribution to control effort
    dTerm  = state->Kd * (desired_angular_velocity - angular_velocity); //!< Angular velocity error contribution to control effort
    ddTerm = state->Kdd * angular_acceleration; //!< Angular acceleration error contribution to control effort

    return( pTerm + iTerm + dTerm - ddTerm);
}

/** \brief Reset integral to zero. 
*
* \return 0
*/
static int pid_reset_zero( struct controller *controller )
{
    controller->state->integral = 0;

    return( 0 );
}

/** \brief Set PID gains. */
static int setPIDdirectly( struct controller *controller,
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
int pid_controller_init( struct controller *controller, int period )
{
    struct controller_state *state;

    state = malloc( sizeof( struct controller_state ) );

    if( !state )
    {
        return( -1 );
    }

    memset( state, 0, sizeof( struct controller_state ) );
    state->dtime = period/1000.0;   //controller uses period in seconds
    state->iMax  = INTEGRAL_LIMIT;
    state->iMin  = -INTEGRAL_LIMIT;
    state->last_desired = 0;

    controller->control      = pid_control;
    controller->reset_zero   = pid_reset_zero;
    controller->set_params   = setPIDdirectly;
    controller->state        = state;

    return( 0 );
}

/** \brief Free the state of the controller, set it to be a null pointer.
*
* \return 0
*/
int pid_controller_destroy( struct controller *controller )
{
    struct controller_state *state;

    state = controller->state;
    controller->state = NULL;
    free( state );

    return( 0 );
}
