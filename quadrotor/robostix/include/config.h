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

/* Max number of timer events that can be added to the timer event list
*/
#define MAX_TIMER_EVENTS    1       /* should be kept to the minimum required */

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

/* Definitions for establishing a parallel port between Robostix 1 and Robostix 2
*/
#define PP_DATA_PORT        PORTC   /* port exploited for parallel communication */
#define PP_DATA_DDR         DDRC    /* data direction register associated with port */
#define PP_DATA_REG         PINC    /* data register associated with parallel port */
#define PP_CTRL_PORT        PORTE   /* port exploited for parallel port interrupts */
#define PP_CTRL_DDR         DDRE    /* data direction register associated with port */
#define PP_CMD_RDY          INT6    /* port pin where the RDY command connects to */
#define PP_INT_RDY          INT7    /* port pin where the RDY interrupt connects to */
#define PP_ISC_RDY          ISC70   /* EICRB configuration setting for RDY interrupt */
#define PP_SIG_RDY          SIG_INTERRUPT7  /* interrupt signal associated with RDY */

/* Analog input channels assigned to the Analog-to-Digital Converter
*/
#define ADC_CH_TEMP         PF5     /* pin where the temp. sensor connects to */
#define ADC_CH_BATT         PF6     /* pin where the battery sensor connects to */
#define ADC_CH_SONAR        PF7     /* pin where the sonar sensor connects to */

/* Port, data direction register, and pins used for the LTC2400 ADCs (pressure)
*/
#define LTC24_PORT          PORTA   /* port exploited for ADC chip-select signals */
#define LTC24_DDR           DDRA    /* data direction register associated with port */
#define LTC24_REG           PINA    /* chip-select register associated with port */
#define LTC24_CS1           PA0     /* pin where the first ADC CS signal connects to */
#define LTC24_NUM           6       /* number of ADC CS signals connected to port */

/* Port, data direction register, and pin used for the Mini-A sonar sensor
*/
#define MINIA_PORT          PORTA   /* port where the trigger signal is provided */
#define MINIA_DDR           DDRA    /* data direction register associated with port */
#define MINIA_TRIGGER       PA7     /* pin where the trigger signal connects to */

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

/* BMU Board pinout (bottom side)
     ___________________________________________
    |  __                                   __  |
    | /  \                                 /  \ |
    | \__/                                 \__/ |
    |          CS5  SCK  SDO  CS4  CS3          |
    |                                           |
    |          CS6  V-   CS1  V+   CS2          |
    |                                           |
    |          TMP  BAT                         |
    |                                           |
    |                                           |
    |                                           |

*/

/* BMU Board connector (plug-in side)

     CS3 CS4 SDO SCK CS5 TMP
    +---+---+---+---+---+---+
    | o | o | o | o | o | o |
    +---+---+---+---+---+---+
    | o | o | o | o | o | o |
    +---+---+---+---+---+---+
     CS2 V+  CS1 V-  CS6 BAT

*/

/* Robostix BMU interface (top side)

             S S C C C C C C
             C D S S S S S S
             K 0 1 2 3 4 5 6
             ___________________
     _______|o_o_o_o_o_o_o_o_o_o|____
    |o o o|o o o|o o o o o o o o|o o
    |o o o|o o o|o o o o o o o o|o o
    |o o o|o o o|o o o o o o o o|o o
    |--------------------------------
    |PWM_3 PWM_1 0  A/D PORT   7
    |
*/

#endif /* !CONFIG_H */

/* End of file */
