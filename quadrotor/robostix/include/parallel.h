/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   parallel.h     Parallel interface used for communication.               */
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

#ifndef PARALLEL_H
#define PARALLEL_H

#include <stdint.h>


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the parallel interface for communication
*/
void    parallel_init( void );

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t parallel_is_new_data( void );

/* Copies the received data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  parallel_get_data( uint8_t *buf );

/* Sends data via the parallel interface.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  parallel_send_data( uint8_t id, const uint8_t *data, uint8_t size );

/* Resets the parallel interface
*/
void    parallel_reset( void );


#endif /* !PARALLEL_H */

/* End of file */
