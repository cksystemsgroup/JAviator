/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   bmu09a.h    Interface for the BMU 09-A barometric measurement unit.     */
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

#ifndef BMU09A_H
#define BMU09A_H

#include <stdint.h>
#include "javiator.h"


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the SPI interface for the BMU sensor
*/
void    bmu09a_init( void );

/* Starts the BMU sensor in continuous mode
*/
void    bmu09a_start( void );

/* Stops the BMU sensor if in continuous mode
*/
void    bmu09a_stop( void );

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t bmu09a_is_new_data( void );

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  bmu09a_get_data( javiator_sdat_t *buf );

#endif /* !BMU09A_H */

/* End of file */
