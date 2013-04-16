/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   minia.h    Interface for the SensComp Mini-A sonar sensor.              */
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

#ifndef MINIA_H
#define MINIA_H

#include <stdint.h>


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes Timer T2 for the sonar sensor
*/
void    minia_init( void );

/* Starts the sonar sensor in continuous mode
*/
void    minia_start( void );

/* Stops the sonar sensor if in continuous mode
*/
void    minia_stop( void );

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t minia_is_new_data( void );


#endif /* !MINIA_H */

/* End of file */
