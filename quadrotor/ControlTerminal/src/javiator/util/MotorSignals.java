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

public class MotorSignals extends NumeratedSendable
{
	public static final int PACKET_SIZE = 10;
		
    public MotorSignals( )
    {
        reset( );
    }

    public void reset( )
    {
        front = 0;
        right = 0;
        rear  = 0;
        left  = 0;
    }

    public String toString( )
    {
        String result;
        result  = " front: " + front;
        result += " right: " + right;
        result += " rear: "  + rear;
        result += " left: "  + left;
        return( result );
    }

    public void copySignals( MotorSignals data )
    {
        front = data.front;
        right = data.right;
        rear  = data.rear;
        left  = data.left;
    }

    public synchronized Packet toPacket( )
    {
        Packet packet = new Packet( PACKET_SIZE );
        encode( packet, 0 );
        return( packet );
    }

	public void encode( Packet packet, int offset )
	{
		packet.payload[ offset + 0 ] = (byte)( front >> 8 );
		packet.payload[ offset + 1 ] = (byte)( front );
		packet.payload[ offset + 2 ] = (byte)( right >> 8 );
		packet.payload[ offset + 3 ] = (byte)( right );
		packet.payload[ offset + 4 ] = (byte)( rear >> 8 );
		packet.payload[ offset + 5 ] = (byte)( rear );
		packet.payload[ offset + 6 ] = (byte)( left >> 8 );
		packet.payload[ offset + 7 ] = (byte)( left );
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
    
    public synchronized void decode( Packet packet, int offset )
    {
        front = (short)( (packet.payload[ offset + 0 ] << 8) | (packet.payload[ offset + 1 ] & 0xFF) );
        right = (short)( (packet.payload[ offset + 2 ] << 8) | (packet.payload[ offset + 3 ] & 0xFF) );
        rear  = (short)( (packet.payload[ offset + 4 ] << 8) | (packet.payload[ offset + 5 ] & 0xFF) );
        left  = (short)( (packet.payload[ offset + 6 ] << 8) | (packet.payload[ offset + 7 ] & 0xFF) );
    }

    public synchronized Object clone( )
    {
        MotorSignals copy = new MotorSignals( );
        copyTo( copy );
        return( copy );
    }
    
    public MotorSignals deepClone( )
    {
        return (MotorSignals) clone( );
    }

    public void copyTo( Copyable to )
	{
    	super.copyTo( to );
    	MotorSignals copy = (MotorSignals) to;
        copy.front        = front;
        copy.right        = right;
        copy.rear         = rear;
        copy.left         = left;
	}

    public short front;
    public short right;
    public short rear;
    public short left;
}
