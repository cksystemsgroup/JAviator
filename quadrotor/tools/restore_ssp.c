/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   restore_ssp.c  Program to reset the Gumstix' SSP2 serial port.          */
/*                                                                           */
/*   Copyright (c) 2009  Rainer Trummer                                      */
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

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define MAP_SIZE        0x00000100  // address space is 256 bytes
#define MAP_MASK        0x000000FF  // mask for extended addresses

#define OFFSET_GPIO     0x40E00000  // offset for GPIO registers
#define GPDR0           0x40E0000C  // GPIO Pin Direction Register          GPIO <31:00>
#define GPDR2           0x40E00014  // GPIO Pin Direction Register          GPIO <95:64>
#define GPSR2           0x40E00020  // GPIO Pin Output Set Register         GPIO <95:64>
#define GPCR2           0x40E0002C  // GPIO Pin Output Clear Register       GPIO <95:64>
#define GAFR0_L         0x40E00054  // GPIO Alternate Function Register     GPIO <15:00>
#define GAFR0_U         0x40E00058  // GPIO Alternate Function Register     GPIO <31:16>
#define GAFR2_L         0x40E00064  // GPIO Alternate Function Register     GPIO <79:64>

#define OFFSET_CLKM     0x41300000  // offset for Clocks Manager registers
#define CKEN            0x41300004  // Clock Enable Register

#define OFFSET_SSP2     0x41700000  // offset for SSP2 registers
#define SSCR0_2         0x41700000  // SSP2 Control Register 0
#define SSCR1_2         0x41700004  // SSP2 Control Register 1

// Global variables
static volatile void *  map_gpio;
static volatile void *  map_clkm;
static volatile void *  map_ssp2;
static volatile void *  reg;
static volatile int32_t fd;

// Macro for writing registers
#define SET_REG( name ) \
    static void set_ ##name ( uint32_t addr, uint32_t data ) \
    {                                                      \
        reg = map_ ##name + (addr & MAP_MASK);              \
        *(uint32_t *) reg = data;                          \
    }

// Writing instances
SET_REG( gpio )
SET_REG( clkm )
SET_REG( ssp2 )


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

int main( int argc, char **argv )
{
    fd = open( "/dev/mem", O_RDWR | O_SYNC );

    if( fd < 0 )
    {
        perror( "device open" );
        return( -1 );
    }

    map_gpio = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, OFFSET_GPIO );
    map_clkm = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, OFFSET_CLKM );
    map_ssp2 = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, OFFSET_SSP2 );

    if( map_gpio == MAP_FAILED || map_clkm == MAP_FAILED || map_ssp2 == MAP_FAILED )
    {
       close( fd );
       perror( "map create" );
       return( -1 );
    }

    // restore GPIO registers
    set_gpio( GPDR0,   0xC18AF218 );
    set_gpio( GPDR2,   0x10018340 );
    set_gpio( GPSR2,   0x00000000 );
    set_gpio( GPCR2,   0x00000000 );
    set_gpio( GAFR0_L, 0x80040000 );
    set_gpio( GAFR0_U, 0xA5254010 );
    set_gpio( GAFR2_L, 0x4AA08AAA );

    // restore CLKM registers
    set_clkm( CKEN,    0x00500640 );

    // restore SSP2 registers
    set_ssp2( SSCR0_2, 0x00000000 );
    set_ssp2( SSCR1_2, 0x00000000 );

    close( fd );
    munmap( 0, MAP_SIZE );
    return( 0 );
}

// End of file.
