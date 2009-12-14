/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   lsm215.h   Interface for the Dimetix LSM2-15 distance laser sensor.     */
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

#ifndef LSM215_H
#define LSM215_H

#include <stdint.h>
#include "shared/javiator_data.h"


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the UART channels for the laser sensors
*/
void    lsm215_init( void );

/* Starts the laser sensors in continuous mode
*/
void    lsm215_start( void );

/* Stops the laser sensors if in continuous mode
*/
void    lsm215_stop( void );

/* Returns 1 if new x-data available, 0 otherwise
*/
uint8_t lsm215_is_new_x_data( void );

/* Returns 1 if new y-data available, 0 otherwise
*/
uint8_t lsm215_is_new_y_data( void );

/* Copies the sampled x-data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  lsm215_get_x_data( javiator_data_t *buf );

/* Copies the sampled y-data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  lsm215_get_y_data( javiator_data_t *buf );


#endif /* !LSM215_H */

/* End of file */
