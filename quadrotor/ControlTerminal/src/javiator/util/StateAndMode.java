/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   Copyright (c) 2006-2013 Harald Roeck <harald.roeck@gmail.com>           */
/*                       and Rainer Trummer <rainer.trummer@gmail.com>       */
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

public class StateAndMode extends NumeratedSendable
{
	public static final int PACKET_SIZE = 2;

    public StateAndMode( byte state, byte mode )
    {
        this.state = state;
        this.mode  = mode;
    }

    public void reset( )
    {
        state = 0;
        mode  = 0;
    }

    public synchronized Packet toPacket( )
    {
        Packet packet = new Packet( PACKET_SIZE );
        encode( packet, 0 );
        return( packet );
    }

	public void encode( Packet packet, int offset )
	{
		packet.payload[ offset + 0 ] = state;
		packet.payload[ offset + 1 ] = mode;
	}

    public synchronized Packet toPacket( byte type )
    {
        Packet packet = toPacket( );
        packet.type   = type;
        packet.calcChecksum( );
        return( packet );
    }

    public synchronized void fromPacket( Packet packet )
    {
        decode( packet, 0 );
    }

	public void decode( Packet packet, int offset )
	{
		state = (byte)( packet.payload[ offset + 0 ] & 0xFF );
		mode  = (byte)( packet.payload[ offset + 1 ] & 0xFF );
	}

    public synchronized Object clone( )
    {
        return new StateAndMode( state, mode );
    }

    public void copyTo( Copyable to )
	{
    	super.copyTo( to );
    	StateAndMode copy = (StateAndMode) to;
    	copy.state        = state;
    	copy.mode         = mode;
	}

    public byte state;
    public byte mode;
}
