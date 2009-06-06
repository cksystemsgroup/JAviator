/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   pwm.c      Interface for controlling four brushless motors.             */
/*                                                                           */
/*   Copyright (c) Rainer Trummer rtrummer@cs.uni-salzburg.at                */
/*                                                                           */
/*   This program is free software; you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by    */
/*   the Free Software Foundation; either version 2 of the License, or       */
/*   (at your option) any later version.                                     */
/*                                                                           */
/*   This program is distributed in the hope that it will be useful,         */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*   GNU General Public License for more details.                            */
/*                                                                           */
/*   You should have received a copy of the GNU General Public License       */
/*   along with this program; if not, write to the Free Software Foundation, */
/*   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.      */
/*                                                                           */
/*****************************************************************************/

#include <avr/io.h>

#include "config.h"
#include "pwm.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Validation and selection of the user-defined PWM mode
*/
#if( FAST_PWM_MODE == 0 )

#define CNT_TOP     20000u
#define PWM_MIN      1000u
#define PWM_MAX      2000u

#elif( FAST_PWM_MODE == 1 )

#define CNT_TOP     63999u
#define PWM_MIN     16000u
#define PWM_MAX     32000u

#else /* FAST_PWM_MODE */

#error No valid PWM mode defined.

#endif /* FAST_PWM_MODE */

/* Assignment of PWM ports according to motor positions
*/
#if( ROBOSTIX_R341 == 1 || ROBOSTIX_R790 == 1 )

#define PWM_FRONT   OCR3B
#define PWM_RIGHT   OCR3C
#define PWM_REAR    OCR1B
#define PWM_LEFT    OCR1C

#elif( ROBOSTIX_R1131 == 1 )

#define PWM_FRONT   OCR3C
#define PWM_RIGHT   OCR3B
#define PWM_REAR    OCR1B
#define PWM_LEFT    OCR1C

#else /* ROBOSTIX_RN */

#error No valid PWM ports defined.

#endif /* ROBOSTIX_RN */


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes Timer T1 and Timer T3 for generating PWM signals
*/
void pwm_init( void )
{
    /* set counter top and bottom values */
    ICR1  = CNT_TOP;
    ICR3  = CNT_TOP;
    TCNT1 = 0x00;
    TCNT3 = 0x00;

    /* initialize OCRs with minimum value */
    OCR1A = PWM_MIN;
    OCR1B = PWM_MIN;
    OCR1C = PWM_MIN;
    OCR3A = PWM_MIN;
    OCR3B = PWM_MIN;
    OCR3C = PWM_MIN;

#if( FAST_PWM_MODE == 0 )

    /* Calculation of TOP value for the PFC PWM Mode at 50Hz

       TOP = fosc / (2 * prescaler * fpwm)

           = 16MHz / (2 * 8 * 50Hz)

           = 16000000 / 800

           = 20000
    */
    /* set Compare Output Mode 2, Waveform Generation Mode 8, prescaler 8 */
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << COM1C1);
    TCCR1B = (1 << WGM13)  | (1 << CS11);
    TCCR1C = 0x00; /* not used */
    TCCR3A = (1 << COM3A1) | (1 << COM3B1) | (1 << COM3C1);
    TCCR3B = (1 << WGM33)  | (1 << CS31);
    TCCR3C = 0x00; /* not used */

#elif( FAST_PWM_MODE == 1 )

    /* Calculation of TOP value for the Fast PWM Mode at 250Hz

       TOP = fosc / (prescaler * fpwm) - 1

           = 16MHz / (1 * 250Hz) - 1

           = 16000000 / 250 - 1

           = 63999
    */
    /* set Compare Output Mode 2, Waveform Generation Mode 14, prescaler 1 */
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << COM1C1) | (1 << WGM11);
    TCCR1B = (1 << WGM13)  | (1 << WGM12)  | (1 << CS10);
    TCCR1C = 0x00; /* not used */
    TCCR3A = (1 << COM3A1) | (1 << COM3B1) | (1 << COM3C1) | (1 << WGM31);
    TCCR3B = (1 << WGM33)  | (1 << WGM32)  | (1 << CS30);
    TCCR3C = 0x00; /* not used */

#endif /* FAST_PWM_MODE */

    /* make ports 1A, 1B, and 1C outputs */
    DDRB |= (1 << PB7) | (1 << PB6) | (1 << PB5);

    /* make ports 3A, 3B, and 3C outputs */
    DDRE |= (1 << PE5) | (1 << PE4) | (1 << PE3);
}

/* Assigns the given PWM signals to the motor controllers.
   Returns 0 if all signals were valid, or -1 if one or more
   signals were out of range and had to be adjusted.
*/
int8_t pwm_set_signals( const motor_signals_t *signals )
{
    int16_t pwm[4] =
    {
        signals->front,
        signals->right,
        signals->rear,
        signals->left
    };

    int8_t i, res = 0;

    for( i = 0; i < 4; ++i )
    {
        if( pwm[i] < 0 )
        {
            pwm[i] = 0;
            res = -1;
        }
        else
        if( pwm[i] > PWM_MAX - PWM_MIN )
        {
            pwm[i] = PWM_MAX - PWM_MIN;
            res = -1;
        }
    }

    PWM_FRONT = PWM_MIN + pwm[0];
    PWM_RIGHT = PWM_MIN + pwm[1];
    PWM_REAR  = PWM_MIN + pwm[2];
    PWM_LEFT  = PWM_MIN + pwm[3];

    return( res );
}

/* End of file */
