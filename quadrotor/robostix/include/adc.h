/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   adc.h      Analog/Digital Converter interface.                          */
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

#ifndef ADC_H
#define ADC_H

#include <stdint.h>


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the Analog/Digital Converter
*/
void   adc_init( void );

/* Performs a conversion for all registered channels.
   Returns 0 if successful, -1 otherwise.
*/
int8_t adc_convert( void );

/* Adds a new channel to the ADC channel list.
   Returns 0 if successful, -1 otherwise.
*/
int8_t adc_add_channel( int8_t channel );

/* Returns -1 if the given channel is invalid or
   disabled, 1 if new data available, 0 otherwise
*/
int8_t adc_is_new_data( int8_t channel );

/* Copies the sampled data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t adc_get_data( int8_t channel, uint16_t *buf );


#endif /* !ADC_H */

/* End of file */
