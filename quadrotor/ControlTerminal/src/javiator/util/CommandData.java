/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   Copyright (c) 2006  Harald Roeck, Rainer Trummer                        */
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
 * Represents the navigation data
 * @author hroeck 
 */
public class CommandData extends NumeratedSendable
{
	public static final int PACKET_SIZE = 8;
	
	public CommandData( )
    {
        reset();
    }

    public void reset( )
    {
        roll  = 0;
        pitch = 0;
        yaw   = 0;
        z     = 0;
    }

    public String toString( )
    {
        String result;
        result  = " roll: "  + roll;
        result += " pitch: " + pitch;
        result += " yaw: "   + yaw;
        result += " z: "     + z;

        return( result );
    }

    public CommandData( Packet packet )
    {
        fromPacket( packet );
    }

    public synchronized Packet toPacket( )
    {
        Packet packet = new Packet( PACKET_SIZE );
        encode( packet, 0 );

        return( packet );
    }

    public void encode( Packet packet, int offset )
	{
		packet.payload[ offset + 0 ] = (byte)( roll >> 8 );
		packet.payload[ offset + 1 ] = (byte)( roll );
		packet.payload[ offset + 2 ] = (byte)( pitch >> 8 );
		packet.payload[ offset + 3 ] = (byte)( pitch );
		packet.payload[ offset + 4 ] = (byte)( yaw >> 8 );
		packet.payload[ offset + 5 ] = (byte)( yaw );
		packet.payload[ offset + 6 ] = (byte)( z >> 8 );
		packet.payload[ offset + 7 ] = (byte)( z );
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
		roll  = (short)( (packet.payload[ offset + 0 ] << 8) | (packet.payload[ offset + 1 ] & 0xFF) );
		pitch = (short)( (packet.payload[ offset + 2 ] << 8) | (packet.payload[ offset + 3 ] & 0xFF) );
		yaw   = (short)( (packet.payload[ offset + 4 ] << 8) | (packet.payload[ offset + 5 ] & 0xFF) );
		z     = (short)( (packet.payload[ offset + 6 ] << 8) | (packet.payload[ offset + 7 ] & 0xFF) );
	}

    public synchronized Object clone( )
    {
        CommandData copy = null;
        copy = (CommandData) super.clone( );

        if( copy != null )
        {
        	copy.roll  = roll;
        	copy.pitch = pitch;
        	copy.yaw   = yaw;
        	copy.z     = z;
        }

        return( copy );
    }

    public CommandData deepClone( )
    {
        return (CommandData) clone( );
    }

    public void copyTo( Copyable to )
	{
    	super.copyTo( to );
    	CommandData copy = (CommandData) to;
    	copy.roll  = roll;
    	copy.pitch = pitch;
    	copy.yaw   = yaw;
    	copy.z     = z;
	}
    
    public short roll;
    public short pitch;
    public short yaw;
    public short z;
}
