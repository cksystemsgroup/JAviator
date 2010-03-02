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

public class SensorData extends NumeratedSendable
{
	public static final int PACKET_SIZE = 42;

	public SensorData( )
	{
		reset( );
	}

	public void reset( )
	{
		roll    = 0;
		pitch   = 0;
		yaw     = 0;
		droll   = 0;
		dpitch  = 0;
		dyaw    = 0;
		ddroll  = 0;
		ddpitch = 0;
		ddyaw   = 0;
		x       = 0;
		y       = 0;
		z       = 0;
		dx      = 0;
		dy      = 0;
		dz      = 0;
		ddx     = 0;
		ddy     = 0;
		ddz     = 0;
		maps    = 0;
		temp    = 0;
		batt    = 0;
	}

	public String toString( )
	{
		String result;
		result  = " roll: "    + roll;
		result += " pitch: "   + pitch;
		result += " yaw: "     + yaw;
		result += " droll: "   + droll;
		result += " dpitch: "  + dpitch;
		result += " dyaw: "    + dyaw;
		result += " ddroll: "  + ddroll;
		result += " ddpitch: " + ddpitch;
		result += " ddyaw: "   + ddyaw;
		result += " x: "       + x;
		result += " y: "       + y;
		result += " z: "       + z;
		result += " dx: "      + dx;
		result += " dy: "      + dy;
		result += " dz: "      + dz;
		result += " ddx: "     + ddx;
		result += " ddy: "     + ddy;
		result += " ddz: "     + ddz;
		result += " maps: "    + maps;
		result += " temp: "    + temp;
		result += " batt: "    + batt;
		return( result );
	}

	public synchronized Packet toPacket( )
	{
		Packet packet = new Packet( PACKET_SIZE );
		encode( packet, 0 );
		return( packet );
	}
	
	public synchronized void encode( Packet packet, int offset )
	{
		packet.payload[ offset + 0 ]  = (byte)( roll >> 8 );
		packet.payload[ offset + 1 ]  = (byte)( roll );
		packet.payload[ offset + 2 ]  = (byte)( pitch >> 8 );
		packet.payload[ offset + 3 ]  = (byte)( pitch );
		packet.payload[ offset + 4 ]  = (byte)( yaw >> 8 );
		packet.payload[ offset + 5 ]  = (byte)( yaw );
		packet.payload[ offset + 6 ]  = (byte)( droll >> 8 );
		packet.payload[ offset + 7 ]  = (byte)( droll );
		packet.payload[ offset + 8 ]  = (byte)( dpitch >> 8 );
		packet.payload[ offset + 9 ]  = (byte)( dpitch );
		packet.payload[ offset + 10 ] = (byte)( dyaw >> 8 );
		packet.payload[ offset + 11 ] = (byte)( dyaw );
		packet.payload[ offset + 12 ] = (byte)( ddroll >> 8 );
		packet.payload[ offset + 13 ] = (byte)( ddroll );
		packet.payload[ offset + 14 ] = (byte)( ddpitch >> 8 );
		packet.payload[ offset + 15 ] = (byte)( ddpitch );
		packet.payload[ offset + 16 ] = (byte)( ddyaw >> 8 );
		packet.payload[ offset + 17 ] = (byte)( ddyaw );
		packet.payload[ offset + 18 ] = (byte)( x >> 8 );
		packet.payload[ offset + 19 ] = (byte)( x );
		packet.payload[ offset + 20 ] = (byte)( y >> 8 );
		packet.payload[ offset + 21 ] = (byte)( y );
		packet.payload[ offset + 22 ] = (byte)( z >> 8 );
		packet.payload[ offset + 23 ] = (byte)( z );
		packet.payload[ offset + 24 ] = (byte)( dx >> 8 );
		packet.payload[ offset + 25 ] = (byte)( dx );
		packet.payload[ offset + 26 ] = (byte)( dy >> 8 );
		packet.payload[ offset + 27 ] = (byte)( dy );
		packet.payload[ offset + 28 ] = (byte)( dz >> 8 );
		packet.payload[ offset + 29 ] = (byte)( dz );
		packet.payload[ offset + 30 ] = (byte)( ddx >> 8 );
		packet.payload[ offset + 31 ] = (byte)( ddx );
		packet.payload[ offset + 32 ] = (byte)( ddy >> 8 );
		packet.payload[ offset + 33 ] = (byte)( ddy );
		packet.payload[ offset + 34 ] = (byte)( ddz >> 8 );
		packet.payload[ offset + 35 ] = (byte)( ddz );
		packet.payload[ offset + 36 ] = (byte)( maps >> 8 );
		packet.payload[ offset + 37 ] = (byte)( maps );
		packet.payload[ offset + 38 ] = (byte)( temp >> 8 );
		packet.payload[ offset + 39 ] = (byte)( temp );
		packet.payload[ offset + 40 ] = (byte)( batt >> 8 );
		packet.payload[ offset + 41 ] = (byte)( batt );
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
		roll    = (short)( (packet.payload[ offset + 0 ]  << 8) | (packet.payload[ offset + 1 ]  & 0xFF) );
		pitch   = (short)( (packet.payload[ offset + 2 ]  << 8) | (packet.payload[ offset + 3 ]  & 0xFF) );
		yaw     = (short)( (packet.payload[ offset + 4 ]  << 8) | (packet.payload[ offset + 5 ]  & 0xFF) );
		droll   = (short)( (packet.payload[ offset + 6 ]  << 8) | (packet.payload[ offset + 7 ]  & 0xFF) );
		dpitch  = (short)( (packet.payload[ offset + 8 ]  << 8) | (packet.payload[ offset + 9 ]  & 0xFF) );
		dyaw    = (short)( (packet.payload[ offset + 10 ] << 8) | (packet.payload[ offset + 11 ] & 0xFF) );
		ddroll  = (short)( (packet.payload[ offset + 12 ] << 8) | (packet.payload[ offset + 13 ] & 0xFF) );
		ddpitch = (short)( (packet.payload[ offset + 14 ] << 8) | (packet.payload[ offset + 15 ] & 0xFF) );
		ddyaw   = (short)( (packet.payload[ offset + 16 ] << 8) | (packet.payload[ offset + 17 ] & 0xFF) );
		x       = (short)( (packet.payload[ offset + 18 ] << 8) | (packet.payload[ offset + 19 ] & 0xFF) );
		y       = (short)( (packet.payload[ offset + 20 ] << 8) | (packet.payload[ offset + 21 ] & 0xFF) );
		z       = (short)( (packet.payload[ offset + 22 ] << 8) | (packet.payload[ offset + 23 ] & 0xFF) );
		dx      = (short)( (packet.payload[ offset + 24 ] << 8) | (packet.payload[ offset + 25 ] & 0xFF) );
		dy      = (short)( (packet.payload[ offset + 26 ] << 8) | (packet.payload[ offset + 27 ] & 0xFF) );
		dz      = (short)( (packet.payload[ offset + 28 ] << 8) | (packet.payload[ offset + 29 ] & 0xFF) );
		ddx     = (short)( (packet.payload[ offset + 30 ] << 8) | (packet.payload[ offset + 31 ] & 0xFF) );
		ddy     = (short)( (packet.payload[ offset + 32 ] << 8) | (packet.payload[ offset + 33 ] & 0xFF) );
		ddz     = (short)( (packet.payload[ offset + 34 ] << 8) | (packet.payload[ offset + 35 ] & 0xFF) );
		maps    = (short)( (packet.payload[ offset + 36 ] << 8) | (packet.payload[ offset + 37 ] & 0xFF) );
		temp    = (short)( (packet.payload[ offset + 38 ] << 8) | (packet.payload[ offset + 39 ] & 0xFF) );
		batt    = (short)( (packet.payload[ offset + 40 ] << 8) | (packet.payload[ offset + 41 ] & 0xFF) );
	}

	public synchronized Object clone( )
	{
		SensorData copy = new SensorData( );
		copyTo( copy );
		return( copy );
	}

	public SensorData deepClone( )
	{
		return (SensorData) clone( );
	}

	public void copyTo( Copyable to )
	{
    	super.copyTo( to );
		SensorData copy = (SensorData) to;
		copy.roll    = roll;
		copy.pitch   = pitch;
		copy.yaw     = yaw;
		copy.droll   = droll;
		copy.dpitch  = dpitch;
		copy.dyaw    = dyaw;
		copy.ddroll  = ddroll;
		copy.ddpitch = ddpitch;
		copy.ddyaw   = ddyaw;
		copy.x       = x;
		copy.y       = y;
		copy.z       = z;
		copy.dx      = dx;
		copy.dy      = dy;
		copy.dz      = dz;
		copy.ddx     = ddx;
		copy.ddy     = ddy;
		copy.ddz     = ddz;
		copy.maps    = maps;
		copy.temp    = temp;
		copy.batt    = batt;
	}

	public short roll;
	public short pitch;
	public short yaw;
	public short droll;
	public short dpitch;
	public short dyaw;
	public short ddroll;
	public short ddpitch;
	public short ddyaw;
	public short x;
	public short y;
	public short z;
	public short dx;
	public short dy;
	public short dz;
	public short ddx;
	public short ddy;
	public short ddz;
	public short maps;
	public short temp;
	public short batt;
}
