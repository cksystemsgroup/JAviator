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
 * Interface for objects that can be send/received by a transceiver
 * 
 * @author hroeck
 *
 */
public interface Sendable extends java.lang.Cloneable
{
    /** 
     * resets all attributes
     */
    public void reset( );

    /** 
     * generates a packet object.
     * does not set the type or checksum
     * 
     * @return a packet not ready to send
     */
    public Packet toPacket( );

    /**
     * generates a packet object, sets the type
     * of the packet and calculates the checksum
     * 
     * @param type The type of the packet to generate
     * @return a packet ready to send by a Transceiver
     */
    public Packet toPacket( byte type );

    /**
     * parses the packet and fills the class members
     * 
     * @param packet The packet to parse
     */
    public void fromPacket( Packet packet );
	
    /**
     * generates a clone of the object
     * 
     * @return a copy of the original object
     * @throws java.lang.CloneNotSupportedException
     */
    public Object clone( );
}
