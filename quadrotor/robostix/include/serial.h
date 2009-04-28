/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   serial.h   Serial interface used for communication with the Gumstix.    */
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

#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the selected UART channel for communication
*/
void    serial_init( void );

/* Returns 1 if a new packet is available, 0 otherwise
*/
uint8_t serial_is_new_packet( void );

/* Copies the received data packet to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  serial_recv_packet( uint8_t *buf );

/* Sends a data packet via the selected UART channel.
   Returns 0 if successful, -1 otherwise.
*/
int8_t  serial_send_packet( uint8_t id, const uint8_t *data, uint8_t size );

/* Resets the selected UART channel
*/
void    serial_reset( void );


#endif /* !SERIAL_H */

/* End of file */
