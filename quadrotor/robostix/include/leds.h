/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   leds.h    Interface for the Robostix LEDs (red, blue, yellow).          */
/*                                                                           */
/*   Copyright (c) 2006-2013 Rainer Trummer <rainer.trummer@gmail.com>       */
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

#ifndef LEDS_H
#define LEDS_H

#include <avr/io.h>


/*****************************************************************************/
/*                                                                           */
/*   Public Definitions                                                      */
/*                                                                           */
/*****************************************************************************/

#define RED_LED_PORT        PORTG
#define RED_LED_PIN         PG4

#define BLUE_LED_PORT       PORTG
#define BLUE_LED_PIN        PG3

#define YELLOW_LED_PORT     PORTB
#define YELLOW_LED_PIN      PB4

#define LED_ON( color )     ( color##_LED_PORT &= ~(1 << color##_LED_PIN) )
#define LED_OFF( color )    ( color##_LED_PORT |=  (1 << color##_LED_PIN) )
#define LED_TOGGLE( color ) ( color##_LED_PORT ^=  (1 << color##_LED_PIN) )


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the Robostix LEDs
*/
void leds_init( void );


#endif /* !LEDS_H */

/* End of file */
