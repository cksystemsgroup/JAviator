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
 * Represents the sensor data
 * 
 * @author hroeck
 */
public class TraceData extends NumeratedSendable
{
	public static final int PACKET_SIZE = 32;

	public TraceData()
	{
		reset( );
	}

	public void reset( )
	{
		value_1   = 0;
		value_2   = 0;
		value_3   = 0;
		value_4   = 0;
		value_5   = 0;
		value_6   = 0;
		value_7   = 0;
		value_8   = 0;
		value_9   = 0;
		value_10  = 0;
		value_11  = 0;
		value_12  = 0;
		value_13  = 0;
		value_14  = 0;
		value_15  = 0;
		value_16  = 0;
	}

	public String toString( )
	{
		String result;
		result  = " value 1: "  + value_1;
		result += " value 2: "  + value_2;
		result += " value 3: "  + value_3;
		result += " value 4: "  + value_4;
		result += " value 5: "  + value_5;
		result += " value 6: "  + value_6;
		result += " value 7: "  + value_7;
		result += " value 8: "  + value_8;
		result += " value 9: "  + value_9;
		result += " value 10: " + value_10;
		result += " value 11: " + value_11;
		result += " value 12: " + value_12;
		result += " value 13: " + value_13;
		result += " value 14: " + value_14;
		result += " value 15: " + value_15;
		result += " value 16: " + value_16;

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
		packet.payload[ offset + 0 ]  = (byte)( value_1 >> 8 );
		packet.payload[ offset + 1 ]  = (byte)( value_1 );
		packet.payload[ offset + 2 ]  = (byte)( value_2 >> 8 );
		packet.payload[ offset + 3 ]  = (byte)( value_2 );
		packet.payload[ offset + 4 ]  = (byte)( value_3 >> 8 );
		packet.payload[ offset + 5 ]  = (byte)( value_3 );
		packet.payload[ offset + 6 ]  = (byte)( value_4 >> 8 );
		packet.payload[ offset + 7 ]  = (byte)( value_4 );
		packet.payload[ offset + 8 ]  = (byte)( value_5 >> 8 );
		packet.payload[ offset + 9 ]  = (byte)( value_5 );
		packet.payload[ offset + 10 ] = (byte)( value_6 >> 8 );
		packet.payload[ offset + 11 ] = (byte)( value_6 );
		packet.payload[ offset + 12 ] = (byte)( value_7 >> 8 );
		packet.payload[ offset + 13 ] = (byte)( value_7 );
		packet.payload[ offset + 14 ] = (byte)( value_8 >> 8 );
		packet.payload[ offset + 15 ] = (byte)( value_8 );
		packet.payload[ offset + 16 ] = (byte)( value_9 >> 8 );
		packet.payload[ offset + 17 ] = (byte)( value_9 );
		packet.payload[ offset + 18 ] = (byte)( value_10 >> 8 );
		packet.payload[ offset + 19 ] = (byte)( value_10 );
		packet.payload[ offset + 20 ] = (byte)( value_11 >> 8 );
		packet.payload[ offset + 21 ] = (byte)( value_11 );
		packet.payload[ offset + 22 ] = (byte)( value_12 >> 8 );
		packet.payload[ offset + 23 ] = (byte)( value_12 );
		packet.payload[ offset + 24 ] = (byte)( value_13 >> 8 );
		packet.payload[ offset + 25 ] = (byte)( value_13 );
		packet.payload[ offset + 26 ] = (byte)( value_14 >> 8 );
		packet.payload[ offset + 27 ] = (byte)( value_14 );
		packet.payload[ offset + 28 ] = (byte)( value_15 >> 8 );
		packet.payload[ offset + 29 ] = (byte)( value_15 );
		packet.payload[ offset + 30 ] = (byte)( value_16 >> 8 );
		packet.payload[ offset + 31 ] = (byte)( value_16 );
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
        value_1  = (short)( (packet.payload[ offset + 0 ]  << 8) | (packet.payload[ offset + 1 ]  & 0xFF) );
        value_2  = (short)( (packet.payload[ offset + 2 ]  << 8) | (packet.payload[ offset + 3 ]  & 0xFF) );
        value_3  = (short)( (packet.payload[ offset + 4 ]  << 8) | (packet.payload[ offset + 5 ]  & 0xFF) );
        value_4  = (short)( (packet.payload[ offset + 6 ]  << 8) | (packet.payload[ offset + 7 ]  & 0xFF) );
        value_5  = (short)( (packet.payload[ offset + 8 ]  << 8) | (packet.payload[ offset + 9 ]  & 0xFF) );
        value_6  = (short)( (packet.payload[ offset + 10 ] << 8) | (packet.payload[ offset + 11 ] & 0xFF) );
        value_7  = (short)( (packet.payload[ offset + 12 ] << 8) | (packet.payload[ offset + 13 ] & 0xFF) );
        value_8  = (short)( (packet.payload[ offset + 14 ] << 8) | (packet.payload[ offset + 15 ] & 0xFF) );
        value_9  = (short)( (packet.payload[ offset + 16 ] << 8) | (packet.payload[ offset + 17 ] & 0xFF) );
        value_10 = (short)( (packet.payload[ offset + 18 ] << 8) | (packet.payload[ offset + 19 ] & 0xFF) );
        value_11 = (short)( (packet.payload[ offset + 20 ] << 8) | (packet.payload[ offset + 21 ] & 0xFF) );
        value_12 = (short)( (packet.payload[ offset + 22 ] << 8) | (packet.payload[ offset + 23 ] & 0xFF) );
        value_13 = (short)( (packet.payload[ offset + 24 ] << 8) | (packet.payload[ offset + 25 ] & 0xFF) );
        value_14 = (short)( (packet.payload[ offset + 26 ] << 8) | (packet.payload[ offset + 27 ] & 0xFF) );
        value_15 = (short)( (packet.payload[ offset + 28 ] << 8) | (packet.payload[ offset + 29 ] & 0xFF) );
        value_16 = (short)( (packet.payload[ offset + 30 ] << 8) | (packet.payload[ offset + 31 ] & 0xFF) );
	}

	public synchronized Object clone( )
	{
		TraceData copy = new TraceData( );
		copyTo( copy );

		return( copy );
	}

	public TraceData deepClone( )
	{
		return (TraceData) clone( );
	}

	public void copyTo( Copyable to )
	{
    	super.copyTo( to );
		TraceData copy = (TraceData) to;
		copy.value_1   = value_1;
		copy.value_2   = value_2;
		copy.value_3   = value_3;
		copy.value_4   = value_4;
		copy.value_5   = value_5;
		copy.value_6   = value_6;
		copy.value_7   = value_7;
		copy.value_8   = value_8;
		copy.value_9   = value_9;
		copy.value_10  = value_10;
		copy.value_11  = value_11;
		copy.value_12  = value_12;
		copy.value_13  = value_13;
		copy.value_14  = value_14;
		copy.value_15  = value_15;
		copy.value_16  = value_16;
	}

	public short value_1;
	public short value_2;
	public short value_3;
	public short value_4;
	public short value_5;
	public short value_6;
	public short value_7;
	public short value_8;
	public short value_9;
	public short value_10;
	public short value_11;
	public short value_12;
	public short value_13;
	public short value_14;
	public short value_15;
	public short value_16;
}
