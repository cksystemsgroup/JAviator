/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   srf10.c    Interface for the Devantech SRF10 sonar sensor.              */
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

#include <avr/interrupt.h>
#include <string.h>

#include "config.h"
#include "srf10.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Timer constants */
#define TCNT2_MS_OFFSET     0x06    /* offset for 1ms interrupts */
#define TCNT2_US_OFFSET     0x38    /* offset for 100us interrupts */
#define TCCR2_MS_SCALER     0x03    /* bit-mask for prescaler 64 */
#define TCCR2_US_SCALER     0x02    /* bit-mask for prescaler 8 */

/* I2C mode constants */
#define I2C_MASTER_TX       0x01    /* I2C Master Transmit mode */
#define I2C_MASTER_RX       0x02    /* I2C Master Receive mode */
#define I2C_IDLE            0x03    /* I2C bus in idle mode */

/* Register definitions */
#define REG_COMMAND         0x00    /* sonar command register */
#define REG_MAX_GAIN        0x01    /* sonar max-gain register */
#define REG_RANGE_HB        0x02    /* range high-byte register */
#define REG_RANGE_LB        0x03    /* range low-byte register */

/* Command definitions */
#define CMD_RANGE_CM        0x51    /* range and return result in cm */
#define CMD_1_SET_ADDR      0xA0    /* 1st command for setting new address */
#define CMD_2_SET_ADDR      0xAA    /* 2nd command for setting new address */
#define CMD_3_SET_ADDR      0xA5    /* 3rd command for setting new address */

/* RX/TX buffer sizes */
#define RX_DATA_SIZE        2       /* max size of received data */
#define TX_DATA_SIZE        4       /* max size of transmitted data */

/* TWCR bit-masks */
#define TWCR_MASK_1         ( (1 << TWINT) | (1 << TWSTA) | (1 << TWEA) )
#define TWCR_MASK_2         ( (1 << TWINT) | (1 << TWSTO) )
#define TWCR_MASK_3         ( (1 << TWINT) | (1 << TWEA) )
#define TWCR_MASK_4         ( (1 << TWINT) )
#define TWCR_MASK_5         ( (1 << TWSTA) | (1 << TWSTO) | (1 << TWEA) )
#define TWCR_MASK_6         ( (1 << TWSTA) | (1 << TWSTO) )
#define TWCR_MASK_7         ( (1 << TWSTA) )

/* Global variables */
static volatile uint8_t     srf10_addr;
static volatile uint8_t     srf10_period;
static volatile uint8_t     srf10_range;
static volatile uint8_t     srf10_gain;
static volatile uint8_t     i2c_state;
static          uint8_t     rx_buf[ RX_DATA_SIZE ];
static          uint8_t     tx_buf[ TX_DATA_SIZE ];
static volatile uint8_t     rx_items;
static volatile uint8_t     tx_items;
static volatile uint8_t     rx_index;
static volatile uint8_t     tx_index;
static volatile uint8_t     new_data;
static volatile uint8_t     new_addr;
static volatile uint8_t     new_range;
static volatile uint8_t     new_gain;
static volatile uint8_t     new_result;


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the I2C interface and Timer T2
*/
void srf10_init( void )
{
    /* Calculation of TWBR value

       TWBR = (fosc + SCL * (4^TWPS - 16)) / (2 * SCL)

            = (16MHz + 100kHz * (4^0 - 16)) / (2 * 100kHz)

            = (16000000 + 100000 * (-15)) / (2 * 100000)

            = (16000000 - 1500000) / 200000

            = 72.5 = (uint8_t) 0x48
    */
    TWBR = 0x48; /* set 100kHz SCL clock frequency */

    /* enable TWI operation and interrupts */
    TWCR = (1 << TWEN) | (1 << TWIE);

    /* clear TWI prescaler bits (set prescaler 1) */
    TWSR &= ~( (1 << TWPS1) | (1 << TWPS0) );

    /* initialize Timer T2 used for timed echoing */
    TCNT2 = TCNT2_MS_OFFSET; /* load offset for 1ms interrupts */
    TCCR2 = TCCR2_MS_SCALER; /* load scaler for 1ms interrupts */

    /* initialize global variables */
    srf10_period = SRF10_DEF_PERIOD;
    srf10_range  = SRF10_DEF_RANGE;
    srf10_gain   = SRF10_DEF_GAIN;
    srf10_addr   = SRF10_DEF_ADDR;
    i2c_state    = I2C_IDLE;
    rx_items     = 0;
    tx_items     = 0;
    rx_index     = 0;
    tx_index     = 0;
    new_data     = 0;
    new_addr     = 0;
    new_range    = 0;
    new_gain     = 0;
    new_result   = 0;

    /* just for the case the sonar was not stopped
       properly during the previous operation */
    srf10_stop( );
}

/* Starts the sonar sensor in continuous mode
*/
void srf10_start( void )
{
    /* enable T2 interrupt */
    TIMSK |= (1 << TOIE2);
}

/* Stops the sonar sensor if in continuous mode
*/
void srf10_stop( void )
{
    /* disable T2 interrupt */
    TIMSK &= ~(1 << TOIE2);
}

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t srf10_is_new_data( void )
{
    return( new_data );
}

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t srf10_get_data( uint16_t *buf )
{
    /* check that we're not receiving data currently */
    if( !new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* extract result to given buffer */
    *buf = (rx_buf[0] << 8) | rx_buf[1];

    /* clear new-data indicator */
    new_data = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}

/* Returns the current sonar address
*/
uint8_t srf10_get_addr( void )
{
    return( srf10_addr );
}

/* Returns the current sonar period
*/
uint8_t srf10_get_period( void )
{
    return( srf10_period );
}

/* Returns the current sonar range
*/
uint8_t srf10_get_range( void )
{
    return( srf10_range );
}

/* Returns the current sonar gain
*/
uint8_t srf10_get_gain( void )
{
    return( srf10_gain );
}

/* Assigns the given sonar address.
   Returns 0 if successful, -1 otherwise.
*/
int8_t srf10_set_addr( uint8_t addr )
{
    if( addr < SRF10_MIN_ADDR || addr > SRF10_MAX_ADDR )
    {
        return( -1 );
    }

    /* Valid addresses are always greater 0, so we can use
       variable new_addr to hold the new address until it
       has been set and the old one has become obsolete. */
    new_addr = addr;

    return( 0 );
}

/* Assigns the given sonar period.
   Returns 0 if successful, -1 otherwise.
*/
int8_t srf10_set_period( uint8_t period )
{
    if( period < SRF10_MIN_PERIOD || period > SRF10_MAX_PERIOD )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    srf10_period = period;

    sei( ); /* enable interrupts */

    return( 0 );
}

/* Assigns the given sonar range.
   Returns 0 if successful, -1 otherwise.
*/
int8_t srf10_set_range( uint8_t range )
{
    if( range < SRF10_MIN_RANGE || range > SRF10_MAX_RANGE )
    {
        return( -1 );
    }

    srf10_range = range;
    new_range = 1;

    return( 0 );
}

/* Assigns the given sonar gain.
   Returns 0 if successful, -1 otherwise.
*/
int8_t srf10_set_gain( uint8_t gain )
{
    if( gain < SRF10_MIN_GAIN || gain > SRF10_MAX_GAIN )
    {
        return( -1 );
    }

    srf10_gain = gain;
    new_gain = 1;

    return( 0 );
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* T2 Overflow callback function
*/
SIGNAL( SIG_OVERFLOW2 )
{
    static uint8_t state = 0x0A; /* wait one full period to warm up */
    static uint8_t ticks = 0;    /* ticks are counted in milliseconds */

    if( state == 0x0A || state == 0x0B )
    {
        TCNT2 = TCNT2_MS_OFFSET; /* load offset for 1ms interrupts */
    }
    else
    {
        TCNT2 = TCNT2_US_OFFSET; /* load offset for 100us interrupts */
    }

    switch( state )
    {
        /* set range/gain or pick up ranging result
        */
        case 0x00: /* change timer to count microseconds */
            TCCR2 = TCCR2_US_SCALER; /* load scaler for 100us interrupts */
            ++state;
            break;

        case 0x01: /* wait until I2C port ready */
            if( i2c_state == I2C_IDLE )
            {
                ++state;
            }
            break;

        case 0x02: /* prepare I2C transmit buffer */
            if( new_addr )
            {
                tx_buf[0] = CMD_1_SET_ADDR;
                tx_buf[1] = CMD_2_SET_ADDR;
                tx_buf[2] = CMD_3_SET_ADDR;
                tx_buf[3] = new_addr;
                tx_items  = 4;
                state     = 0x01;
            }
            else
            if( new_range )
            {
                new_range = 0;
                tx_buf[0] = REG_RANGE_HB;
                tx_buf[1] = srf10_range;
                tx_items  = 2;
                state     = 0x01;
            }
            else
            if( new_gain )
            {
                new_gain  = 0;
                tx_buf[0] = REG_MAX_GAIN;
                tx_buf[1] = srf10_gain;
                tx_items  = 2;
                state     = 0x01;
            }
            else
            {
                tx_buf[0] = REG_RANGE_HB;
                tx_items  = 1;
                ++state;
            }
            i2c_state = I2C_MASTER_TX;
            TWCR |= TWCR_MASK_1;
            break;

        case 0x03: /* wait until I2C port ready */
            if( i2c_state == I2C_IDLE )
            {
                ++state;
            }
            break;

        case 0x04: /* prepare I2C receive buffer */
            rx_items  = 2;
            i2c_state = I2C_MASTER_RX;
            TWCR |= TWCR_MASK_1;
            ++state;
            break;

        case 0x05: /* wait until I2C port ready */
            if( i2c_state == I2C_IDLE )
            {
                ++state;
            }
            break;

        case 0x06: /* wait until ranging result ready */
            if( !new_result )
            {
                state = 0x04; /* repeat states 0x04 through 0x06 */
            }
            else
            {
                new_result = 0;
                ++state;
            }
            break;

        case 0x07: /* notify that new data available */
            new_data = 1;
            ++state;
            break;

        /* issue next ranging command
        */
        case 0x08: /* wait until I2C port ready */
            if( i2c_state == I2C_IDLE )
            {
                ++state;
            }
            break;

        case 0x09: /* prepare I2C transmit buffer */
            tx_buf[0] = REG_COMMAND;
            tx_buf[1] = CMD_RANGE_CM;
            tx_items  = 2;
            i2c_state = I2C_MASTER_TX;
            TWCR |= TWCR_MASK_1;
            ++state;
            break;

        /* wait selected period for echo
        */
        case 0x0A: /* change timer to count milliseconds */
            TCCR2 = TCCR2_MS_SCALER; /* load scaler for 1ms interrupts */
            ++state;
            break;

        case 0x0B: /* count until end of period */
            if( ++ticks >= srf10_period )
            {
                ticks = 0;
                state = 0x00;
            }
            break;

        default:
            state = 0x0A;
    }
}

/* I2C 2-Wire Serial Interface callback function
*/
SIGNAL( SIG_2WIRE_SERIAL )
{
    uint8_t state = TWSR & 0xFC;

    switch( state )
    {
        case 0x08: /* start condition transmitted */
            if( i2c_state == I2C_MASTER_TX )
            {
                TWDR = srf10_addr & 0xFE; /* SLA+W */
                tx_index = 0; /* reset transmit buffer index */
            }
            else
            {
                TWDR = srf10_addr | 0x01; /* SLA+R */
                rx_index = 0; /* reset receive buffer index */
            }
            TWCR = (TWCR & ~TWCR_MASK_6) | TWCR_MASK_4;
            break;

        case 0x10: /* repeated START condition transmitted */
            TWCR = (TWCR & ~TWCR_MASK_7) | TWCR_MASK_2; /* send STOP */
            i2c_state = I2C_IDLE;
            break;

        case 0x18: /* SLA+W transmitted, ACK received */
            if( tx_items == 0 ) /* no data to send */
            {
                TWCR = (TWCR & ~TWCR_MASK_7) | TWCR_MASK_2; /* send STOP */
                i2c_state = I2C_IDLE;
            }
            else /* start sending data */
            {
                TWDR  = tx_buf[ tx_index++ ];
                TWCR = (TWCR & ~TWCR_MASK_6) | TWCR_MASK_4;
            }
            break;

        case 0x20: /* SLA+W transmitted, NOT ACK received */
            TWCR = (TWCR & ~TWCR_MASK_7) | TWCR_MASK_2; /* send STOP */
            i2c_state = I2C_IDLE;
            break;

        case 0x28: /* data byte transmitted, ACK received */
            if( tx_index == tx_items ) /* no more data to send */
            {
                TWCR = (TWCR & ~TWCR_MASK_7) | TWCR_MASK_2; /* send STOP */
                i2c_state = I2C_IDLE;
                if( new_addr ) /* check if address was changed */
                {
                    srf10_addr = new_addr; /* store new address */
                    new_addr = 0;
                }
            }
            else /* start sending data */
            {
                TWDR  = tx_buf[ tx_index++ ];
                TWCR = (TWCR & ~TWCR_MASK_6) | TWCR_MASK_4;
            }
            break;

        case 0x30: /* data byte transmitted, NOT ACK received */
            TWCR = (TWCR & ~TWCR_MASK_7) | TWCR_MASK_2; /* send STOP */
            i2c_state = I2C_IDLE;
            break;

        case 0x38: /* arbitration lost */
            TWCR = (TWCR & ~TWCR_MASK_6) | TWCR_MASK_4;
            i2c_state = I2C_IDLE;
            break;

        case 0x40: /* SLA+R transmitted, ACK received */
            if( rx_items == 1 )
            {
                TWCR = (TWCR & ~TWCR_MASK_5) | TWCR_MASK_4;
            }
            else
            {
                TWCR = (TWCR & ~TWCR_MASK_6) | TWCR_MASK_3;
            }
            break;

        case 0x48: /* SLA+R transmitted, NOT ACK received */
            TWCR = (TWCR & ~TWCR_MASK_7) | TWCR_MASK_2; /* send STOP */
            i2c_state = I2C_IDLE;
            break;

        case 0x50: /* data byte received, ACK returned */
            new_data = 0;
            rx_buf[ rx_index++ ] = TWDR;
            if( rx_index == (rx_items - 1) ) /* one more byte to receive */
            {
                TWCR = (TWCR & ~TWCR_MASK_5) | TWCR_MASK_4;
            }
            else /* continue */
            {
                TWCR = (TWCR & ~TWCR_MASK_6) | TWCR_MASK_3;
            }
            break;

        case 0x58: /* data byte received, NOT ACK returned */
            new_data = 0; /* indicate that receive buffer is being updated */
            rx_buf[ rx_index++ ] = TWDR;
            TWCR = (TWCR & ~TWCR_MASK_7) | TWCR_MASK_2; /* send STOP */
            i2c_state  = I2C_IDLE;
            new_result = 1;
            break;

        default:
            if( i2c_state == I2C_MASTER_TX )
            {
                TWCR = (TWCR & ~TWCR_MASK_6) | TWCR_MASK_4;
            }
            i2c_state = I2C_IDLE;
    }
}

/* End of file */
