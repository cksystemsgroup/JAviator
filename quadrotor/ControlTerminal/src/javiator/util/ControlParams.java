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
 * Represents the actuator data
 * 
 * @author hroeck
 */
public class ControlParams extends NumeratedSendable
{
	public static final int PACKET_SIZE = 8;

    public ControlParams( )
    {
        reset( );
	}

    public void reset( )
    {
    	kp  = 0;
    	ki  = 0;
    	kd  = 0;
    	kdd = 0;
	}

    public String toString( )
    {
		String result;
		result  = " Kp: "  + kp;
		result += " Ki: "  + ki;
		result += " Kd: "  + kd;
		result += " Kdd: " + kdd;

		return( result );
	}

	public void copySignals( ControlParams data )
	{
		kp  = data.kp;
		ki  = data.ki;
		kd  = data.kd;
		kdd = data.kdd;
	}

	public synchronized Packet toPacket( )
	{
		Packet packet = new Packet( PACKET_SIZE );
		encode( packet, 0 );

		return( packet );
	}

	public void encode( Packet packet, int offset )
	{
		packet.payload[ offset + 0 ] = (byte)( kp >> 8 );
		packet.payload[ offset + 1 ] = (byte)( kp );
		packet.payload[ offset + 2 ] = (byte)( ki >> 8 );
		packet.payload[ offset + 3 ] = (byte)( ki );
		packet.payload[ offset + 4 ] = (byte)( kd >> 8 );
		packet.payload[ offset + 5 ] = (byte)( kd );
		packet.payload[ offset + 6 ] = (byte)( kdd >> 8 );
		packet.payload[ offset + 7 ] = (byte)( kdd );
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
		kp  = (short)( (packet.payload[ offset + 0 ] << 8) | (packet.payload[ offset + 1 ] & 0xFF) );
		ki  = (short)( (packet.payload[ offset + 2 ] << 8) | (packet.payload[ offset + 3 ] & 0xFF) );
		kd  = (short)( (packet.payload[ offset + 4 ] << 8) | (packet.payload[ offset + 5 ] & 0xFF) );
		kdd = (short)( (packet.payload[ offset + 6 ] << 8) | (packet.payload[ offset + 7 ] & 0xFF) );
	}

	public synchronized Object clone( ) 
	{
		ControlParams copy = new ControlParams( );
		copyTo( copy );

		return( copy );
	}

	public ControlParams deepClone( )
	{
		return (ControlParams) clone( );
	}

	public void copyTo( Copyable to )
	{
		super.copyTo( to );
		ControlParams copy = (ControlParams) to;
		copy.kp  = kp;
		copy.ki  = ki;
		copy.kd  = kd;
		copy.kdd = kdd;
	}

    public short kp;
    public short ki;
    public short kd;
    public short kdd;
}
