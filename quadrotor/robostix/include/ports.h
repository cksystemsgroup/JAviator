/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   ports.h    ATmega128 ports initialization procedure.                    */
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

#ifndef PORTS_H
#define PORTS_H


/*****************************************************************************/
/*                                                                           */
/*   Public Definitions                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*   All of the following ATmega128 pinout denotations are taken from the    */
/*   Atmel ATmega128/ATmega128L manual Rev. 2467M-AVR-11/04.                 */
/*                                                                           */
/*   All of the following Robostix pinout denotations are taken from the     */
/*   Gumstix PCB00019 layout for the Robostix expansion board.               */
/*                                                                           */
/*   Note: for Rev. 1131 the pins ATM_PWM3A and ATM_PWM3C are swapped.       */
/*                                                                           */
/*****************************************************************************/

/*  Port A

    ATmega128           Robostix

    PA0 (AD0)           ATM_PA0
    PA1 (AD1)           ATM_PA1
    PA2 (AD2)           ATM_PA2
    PA3 (AD3)           ATM_PA3
    PA4 (AD4)           ATM_PA4
    PA5 (AD5)           ATM_PA5
    PA6 (AD6)           ATM_PA6
    PA7 (AD7)           ATM_PA7
*/
#define INIT_PORTA      0xFF    /* enable pull-ups for all inputs */
#define INIT_DDRA       0x00    /* make all pins inputs */

/*  Port B

    ATmega128           Robostix

    PB0 (/SS)           ATM_SS
    PB1 (SCK)           ATM_SCK
    PB2 (MOSI)          ATM_MOSI
    PB3 (MISO)          ATM_MISO
    PB4 (OC0)           ATM_PB4
    PB5 (OC1A)          ATM_PWM1A
    PB6 (OC1B)          ATM_PWM1B
    PB7 (OC2/OC1C)      ATM_PWM1C
*/
#define INIT_PORTB      0xFF    /* enable pull-ups for all inputs */
#define INIT_DDRB       0x00    /* make all pins inputs */

/*  Port C

    ATmega128           Robostix

    PC0 (A8)            ATM_PC0
    PC1 (A9)            ATM_PC1
    PC2 (A10)           ATM_PC2
    PC3 (A11)           ATM_PC3
    PC4 (A12)           ATM_PC4
    PC5 (A13)           ATM_PC5
    PC6 (A14)           ATM_PC6
    PC7 (A15)           ATM_PC7
*/
#define INIT_PORTC      0xFF    /* enable pull-ups for all inputs */
#define INIT_DDRC       0x00    /* make all pins inputs */

/*  Port D

    ATmega128           Robostix

    PD0 (SCL/INT0)      ATM_SCL
    PD1 (SDA/INT1)      ATM_SDA
    PD2 (RXD1/INT2)     ATM_RX1
    PD3 (TXD1/INT3)     ATM_TX1
    PD4 (ICP1)          ATM_IC1
    PD5 (XCK1)          ATM_PD5
    PD6 (T1)            ATM_T1
    PD7 (T2)            ATM_T2
*/
#define INIT_PORTD      0xFF    /* enable pull-ups for all inputs */
#define INIT_DDRD       0x00    /* make all pins inputs */

/*  Port E

    ATmega128           Robostix

    PE0 (RXD0/PDI)      ATM_RX0
    PE1 (TXD0/PDO)      ATM_TX0
    PE2 (XCK0/AIN0)     ATM_IRQ
    PE3 (OC3A/AIN1)     ATM_PWM3A
    PE4 (OC3B/INT4)     ATM_PWM3B
    PE5 (OC3C/INT5)     ATM_PWM3C
    PE6 (T3/INT6)       ATM_INT6
    PE7 (ICP3/INT7)     ATM_INT7
*/
#define INIT_PORTE      0xFF    /* enable pull-ups for all inputs */
#define INIT_DDRE       0x00    /* make all pins inputs */

/*  Port F - ADC channels

    ATmega128           Robostix

    PF0 (ADC0)          ATM_ADC0
    PF1 (ADC1)          ATM_ADC1
    PF2 (ADC2)          ATM_ADC2
    PF3 (ADC3)          ATM_ADC3
    PF4 (ADC4/TCK)      ATM_ADC4
    PF5 (ADC5/TMS)      ATM_ADC5
    PF6 (ADC6/TDO)      ATM_ADC6
    PF7 (ADC7/TDI)      ATM_ADC7
*/
#define INIT_PORTF      0x00    /* disable pull-ups for all ADC channels */
#define INIT_DDRF       0x00    /* make all pins inputs */

/*  Port G

    ATmega128           Robostix

    PG0 (/WR)           ATM_PG0
    PG1 (/RD)           ATM_PG1
    PG2 (ALE)           ATM_PG2
    PG3 (TOSC2)         ATM_PG3
    PG4 (TOSC1)         ATM_PG4
*/
#define INIT_PORTG      0xFF    /* enable pull-ups for all inputs */
#define INIT_DDRG       0x00    /* make all pins inputs */
#define INIT_ASSR       0x00    /* disable alternate functions for port G */

/* ADC
*/
#define INIT_ADCSR      0x00    /* disable ADC unit to save power */
#define INIT_ADMUX      0x00    /* clear ADC multiplexer selection */


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes all ports and the ADC
*/
void ports_init( void );


#endif /* !PORTS_H */

/* End of file */
