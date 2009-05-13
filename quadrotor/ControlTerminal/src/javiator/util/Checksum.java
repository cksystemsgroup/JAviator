/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   Copyright (c) 2006  Harald Roeck                                        */
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

package javiator.util;

/**
 * Static methods to calculate the checksum
 * @author hroeck
 */
public final class Checksum
{
    public static int calc( byte id, byte size, byte[] data, int offset )
    {
        int sum = ( (int)( id + size ) ) & 0xFF;

        for( int i = offset; i < offset + size; ++i )
            sum += ( (int) data[i] ) & 0xFF;

        return( sum & 0xFFFF );
    }

    public static int calc( byte id, byte[] data )
    {
        return calc( id, (byte) data.length, data, 0 );
    }

    public static int calc( byte[] data, int size )
    {
        int sum = 0;

        for( int i = 0; i < size; ++i )
            sum += ( (int) data[i] ) & 0xFF;

        return( sum & 0xFFFF );
    }
    
    public static int calc( byte[] data )
    {
        return( calc( data, data.length ) );
    }
}
