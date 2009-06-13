/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   config.h       Definition of global configuration settings.             */
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

#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>


/*****************************************************************************/
/*                                                                           */
/*   Configuration Settings                                                  */
/*                                                                           */
/*****************************************************************************/

/* Analog input channels assigned to the Analog/Digital Converter
*/
#define ADC_CH_MINIA        PF7     /* pin where the Mini-A sensor connects to */
#define ADC_CH_MPX4115A     -1      /* pin where the MPX4115A sensor connects to */
#define ADC_CH_BATTERY      PF6     /* pin where the battery sensor connects to */

/* Max number of timer events that can be added to the timer event list
*/
#define MAX_TIMER_EVENTS    1       /* should be kept to the minimum required */

/* Min number of packets that must be validated to reset valid-packets counter
*/
#define MIN_VALID_PACKETS   1000    /* should be adapted to chosen baudrate */

/* Periods to wait before interacting when not receiving motor signals anymore
*/
#define PERIODS_TO_WAIT     10      /* should not be less than 10 periods */

/* Period for setting a notify flag (watchdog timer counts internally in 10ms)
*/
#define NOTIFY_PERIOD       100     /* [ms] should be a multiple of 10 */

/* Value for decrementing the motor signals when a loss of connectivity occurs
*/
#define MOTOR_DEC           5       /* should be adapted to the PWM mode used */

/* Selection of UART channel used for communication with the Gumstix
*/
#define UART_SERIAL         0       /* must be either 0 for UART0 or 1 for UART1 */

/* Selection of UART channel used for the 3DM-GX1 inertial measurement unit
*/
#define UART_DM3GX1         1       /* must be either 0 for UART0 or 1 for UART1 */

/* Selection of UART channel used for the LSM2-15 laser distance sensor
*/
#define UART_LSM215         1       /* must be either 0 for UART0 or 1 for UART1 */

/* Port, data direction register, and pin used for the Mini-A sonar sensor
*/
#define MINIA_PORT          PORTC   /* port where the trigger signal is provided */
#define MINIA_DDR           DDRC    /* data direction register associated with port */
#define MINIA_TRIGGER       PC0     /* pin where the trigger signal connects to */

/* Default, minimum, and maximum address settings for the SRF10 sonar sensor
*/
#define SRF10_DEF_ADDR      0xE0    /* default SRF10 sensor address */
#define SRF10_MIN_ADDR      0xE0    /* lowest addmissible address */
#define SRF10_MAX_ADDR      0xFE    /* highest addmissible address */

/* Enables/disables the Fast PWM Mode (not suitable for standard R/C servos)
*/
#define FAST_PWM_MODE       1       /* must be either 0 to disable or 1 to enable */

/* Selection of the Robostix Expension Board version
*/
#define ROBOSTIX_R341       0       /* must be either 0 to disable or 1 to enable */
#define ROBOSTIX_R790       0       /* must be either 0 to disable or 1 to enable */
#define ROBOSTIX_R1131      1       /* must be either 0 to disable or 1 to enable */

/* Version-dependent connecting positions on the Robostix

        +-----------+           +-----------+
        |         3A| -----     |         3A| front
        |         3B| front     |         3B| right
        |         3C| right     |         3C| -----
        |         1A| -----     |         1A| -----
        |         1B| rear      |         1B| rear
        |         1C| left      |         1C| left
        |    TOP    |           |    TOP    |
        |           |           |           | 
        |           |           |           | 
        |           |           |           | 
        |   R341    |           |   R1131   | 
        |   R790    |           |           | 
        |           |           |           |
        +-----------+           +-----------+
*/

#endif /* !CONFIG_H */

/* End of file */
