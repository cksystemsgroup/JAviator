/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   Copyright (c) 2006-2010  Harald Roeck, Rainer Trummer                   */
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

public class ModeAndState extends NumeratedSendable
{
	public static final int PACKET_SIZE = 2;

    public ModeAndState( byte mode, byte state )
    {
        this.mode  = mode;
        this.state = state;
    }

    public void reset( )
    {
        mode  = 0;
        state = 0;
    }

    public synchronized Packet toPacket( )
    {
        Packet packet = new Packet( PACKET_SIZE );
        encode( packet, 0 );
        return( packet );
    }

	public void encode( Packet packet, int offset )
	{
		packet.payload[ offset + 0 ] = mode;
		packet.payload[ offset + 1 ] = state;
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
		mode  = (byte)( packet.payload[ offset + 0 ] & 0xFF );
		state = (byte)( packet.payload[ offset + 1 ] & 0xFF );
	}

    public synchronized Object clone( )
    {
        return new ModeAndState( mode, state );
    }

    public void copyTo( Copyable to )
	{
    	super.copyTo( to );
    	ModeAndState copy = (ModeAndState) to;
    	copy.mode         = mode;
    	copy.state        = state;
	}

    public byte mode;
    public byte state;
}
