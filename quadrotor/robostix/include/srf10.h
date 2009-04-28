/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   srf10.h    Interface for the Devantech SRF10 sonar sensor.              */
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

#ifndef SRF10_H
#define SRF10_H

#include <stdint.h>


/*****************************************************************************/
/*                                                                           */
/*   Public Definitions                                                      */
/*                                                                           */
/*****************************************************************************/

/* Default, minimum, and maximum settings for the SRF10 sonar sensor
*/
#define SRF10_DEF_PERIOD    45  /* [ms] */
#define SRF10_MIN_PERIOD    5   /* [ms] */
#define SRF10_MAX_PERIOD    255 /* [ms] */

#define SRF10_DEF_RANGE     46  /* [units] =  200cm */
#define SRF10_MIN_RANGE     0   /* [units] =    4cm */
#define SRF10_MAX_RANGE     255 /* [units] = 1100cm */

#define SRF10_DEF_GAIN      8   /* [units] = 140 */
#define SRF10_MIN_GAIN      0   /* [units] =  40 */
#define SRF10_MAX_GAIN      16  /* [units] = 700 */


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the I2C interface and Timer T2 for the sonar sensor
*/
void    srf10_init( void );

/* Starts the sonar sensor in continuous mode
*/
void    srf10_start( void );

/* Stops the sonar sensor if in continuous mode
*/
void    srf10_stop( void );

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t srf10_is_new_data( void );

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  srf10_get_data( uint16_t *buf );

/* Returns the current sonar address
*/
uint8_t srf10_get_addr( void );

/* Returns the current sonar period
*/
uint8_t srf10_get_period( void );

/* Returns the current sonar range
*/
uint8_t srf10_get_range( void );

/* Returns the current sonar gain
*/
uint8_t srf10_get_gain( void );

/* Assigns the given sonar address.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  srf10_set_addr( uint8_t addr );

/* Assigns the given sonar period.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  srf10_set_period( uint8_t period );

/* Assigns the given sonar range.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  srf10_set_range( uint8_t range );

/* Assigns the given sonar gain.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  srf10_set_gain( uint8_t gain );


#endif /* !SRF10_H */

/* End of file */
