/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   bmu09a.c   Interface for the BMU 09-A barometric measurement unit.      */
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
#include "bmu09a.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Validation and selection of sampling rate
*/
#if( BMU09A_100 == 1 )

#define LTC2440_OSR         0x30    /* 110Hz */

#elif( BMU09A_55 == 1 )

#define LTC2440_OSR         0x38    /* 55Hz */

#elif( BMU09A_27 == 1 )

#define LTC2440_OSR         0x40    /* 27.5Hz */

#elif( BMU09A_13 == 1 )

#define LTC2440_OSR         0x48    /* 13.8Hz */

#elif( BMU09A_6 == 1 )

#define LTC2440_OSR         0x78    /* 6.9Hz */

#else /* BMU09A_n */

#error No valid sampling rate for the BMU-09A sensor defined.

#endif /* BMU09A_n */

/* EOC polling IDs */
#define EOC_MAPS            1       /* EOC ID assigned to pressure ADC */
#define EOC_TEMP            2       /* EOC ID assigned to temperature ADC */
#define EOC_BATT            3       /* EOC ID assigned to battery ADC */

/* Size of BMU data */
#define RX_MAPS_SIZE        4       /* size of received pressure data */
#define RX_TEMP_SIZE        2       /* size of received temperature data */
#define RX_BATT_SIZE        2       /* size of received battery data */

/* Global variables */
static          uint8_t     rx_buf[ RX_MAPS_SIZE + RX_TEMP_SIZE + RX_BATT_SIZE ];
static volatile uint8_t     rx_index;
static volatile uint8_t     new_data;
static volatile uint8_t     poll_eoc;
static volatile uint8_t     spi_rx;
static volatile uint8_t     spi_if;


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the SPI interface for the BMU sensor
*/
void bmu09a_init( void )
{
    /* make MOSI, SCK, and SS outputs, make MISO an input */
    DDRB  |=  (1 << DDB2) | (1 << DDB1) | (1 << DDB0);
    DDRB  &= ~(1 << DDB3);

    /* rise SS signal (hard-wired to the Gumstix) */
    PORTB |=  (1 << DDB0);

    /* enable SPI interface, set Master mode, set fosc/16 */
    SPCR   =  (1 << SPE) | (1 << MSTR) | (1 << SPR0);

    /* make PCS, TCS, and BCS outputs, make BSY an input */
    BMU09A_DDR  |=  (1 << BMU09A_BCS) | (1 << BMU09A_TCS) | (1 << BMU09A_PCS);
    BMU09A_DDR  &= ~(1 << BMU09A_BSY);

    /* rise PCS, TCS, and BCS signal */
    BMU09A_PORT |=  (1 << BMU09A_BCS) | (1 << BMU09A_TCS) | (1 << BMU09A_PCS);

    /* initialize global variables */
    rx_index = 0;
    new_data = 0;
    poll_eoc = 0;
    spi_rx   = 0;
    spi_if   = 0;
}

/* Starts the BMU sensor in continuous mode
*/
void bmu09a_start( void )
{
    /* enable SPI interrupt */
    SPCR |= (1 << SPIE);

    /* set polling ID */
    poll_eoc = EOC_MAPS;
}

/* Stops the BMU sensor if in continuous mode
*/
void bmu09a_stop( void )
{
    /* disable SPI interrupt */
    SPCR &= ~(1 << SPIE);

    /* rise all CS signals */
    BMU09A_PORT |= (1 << BMU09A_BCS) | (1 << BMU09A_TCS) | (1 << BMU09A_PCS);

    /* reset global variables */
    rx_index = 0;
    new_data = 0;
    poll_eoc = 0;
    spi_rx   = 0;
    spi_if   = 0;
}

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t bmu09a_is_new_data( void )
{
    /* check for MAPS EOC flag to poll */
    if( poll_eoc == EOC_MAPS )
    {
        /* check for end of MAPS conversion */
        if( !(BMU09A_REG & (1 << BMU09A_BSY)) )
        {
            /* sink MAPS-related CS signal */
            BMU09A_PORT &= ~(1 << BMU09A_PCS);

            /* start MAPS data transfer */
            SPDR     = LTC2440_OSR;
            poll_eoc = 0;
        }
    }
    else /* check for TEMP/BATT EOC flag to poll */
    if( poll_eoc == EOC_TEMP || poll_eoc == EOC_BATT )
    {
        /* check for end of TEMP/BATT conversion */
        if( (PINB & (1 << PINB3)) )
        {
            /* start TEMP/BATT data transfer */
            SPDR     = 0;
            poll_eoc = 0;
        }
    }

    /* check for SPI interrupt */
    if( spi_if )
    {
        spi_if = 0;

        /* indicate that the receive buffer is being updated
           and thus data are no longer secure to be copied */
        new_data = 0;

        /* read data from SPI port */
        rx_buf[ rx_index ] = spi_rx;

        /* check for end of MAPS data stream */
        if( ++rx_index == RX_MAPS_SIZE )
        {
            /* rise MAPS-related CS signal */
            BMU09A_PORT |= (1 << BMU09A_PCS);

            /* sink TEMP-related CS signal */
            BMU09A_PORT &= ~(1 << BMU09A_TCS);

            poll_eoc = EOC_TEMP;
        }
        else /* check for end of TEMP data stream */
        if( rx_index == RX_MAPS_SIZE + RX_TEMP_SIZE )
        {
            /* rise TEMP-related CS signal */
            BMU09A_PORT |= (1 << BMU09A_TCS);

            /* sink BATT-related CS signal */
            BMU09A_PORT &= ~(1 << BMU09A_BCS);

            poll_eoc = EOC_BATT;
        }
        else /* check for end of BATT data stream */
        if( rx_index == RX_MAPS_SIZE + RX_TEMP_SIZE + RX_BATT_SIZE )
        {
            /* rise BATT-related CS signal */
            BMU09A_PORT |= (1 << BMU09A_BCS);

            rx_index = 0;
            new_data = 1;
            poll_eoc = EOC_MAPS;
        }
        else /* start next data transfer */
        {
            SPDR = 0;
        }
    }

    return( new_data );
}

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t bmu09a_get_data( javiator_sdat_t *buf )
{
    /* check that we're not receiving data currently */
    if( !new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* extract 24-bit MAPS data */
    buf->maps   = rx_buf[0] & 0x1F; /* drop 3 leading status bits */
    buf->maps <<= 8;                /* 5 leading bits in buffer */
    buf->maps  |= rx_buf[1];        /* mask in next full 8 bits */
    buf->maps <<= 8;                /* 13 leading bits in buffer */
    buf->maps  |= rx_buf[2];        /* mask in next full 8 bits */
    buf->maps <<= 3;                /* 21 leading bits in buffer */
    buf->maps  |= rx_buf[3] >> 5;   /* mask in remaining 3 bits */

    /* extract 12-bit TEMP data */
    buf->temp   = rx_buf[4] & 0x7F; /* drop 1 leading status bit */
    buf->temp <<= 5;                /* 7 leading bits in buffer */
    buf->temp  |= rx_buf[5] >> 3;   /* mask in remaining 5 bits */

    /* extract 10-bit BATT data */
    buf->batt   = rx_buf[6] & 0x7F; /* drop 1 leading status bit */
    buf->batt <<= 3;                /* 7 leading bits in buffer */
    buf->batt  |= rx_buf[7] >> 5;   /* mask in remaining 3 bits */

    /* clear new-data indicator */
    new_data = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* SPI Transmission Complete callback function
*/
SIGNAL( SIG_SPI )
{
    spi_rx = SPDR;
    spi_if = 1;
}

/* End of file */
