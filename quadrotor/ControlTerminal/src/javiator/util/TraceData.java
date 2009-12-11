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
	public static final int PACKET_SIZE = 26;

	public TraceData()
	{
		reset( );
	}

	public void reset( )
	{
		z            = 0;
		z_filtered   = 0;
		z_estimated  = 0;
		dz_estimated = 0;
		ddz          = 0;
		ddz_filtered = 0;
        p_term       = 0;
        i_term       = 0;
        d_term       = 0;
        dd_term      = 0;
		uz           = 0;
		z_cmd        = 0;
		id           = 0;
	}

	public String toString( )
	{
		String result;
		result  = " z: "            + z;
		result += " filtered z: "   + z_filtered;
		result += " estimated z: "  + z_estimated;
		result += " estimated dz: " + dz_estimated;
		result += " ddz: "          + ddz;
		result += " filtered ddz: " + ddz_filtered;
		result += " p-term: "       + p_term;
		result += " i-term: "       + i_term;
		result += " d-term: "       + d_term;
		result += " dd-term: "      + dd_term;
		result += " uz: "           + uz;
		result += " z command: "    + z_cmd;
		result += " id: "           + id;

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
		packet.payload[ offset + 0 ]  = (byte)( z >> 8 );
		packet.payload[ offset + 1 ]  = (byte)( z );
		packet.payload[ offset + 2 ]  = (byte)( z_filtered >> 8 );
		packet.payload[ offset + 3 ]  = (byte)( z_filtered );
		packet.payload[ offset + 4 ]  = (byte)( z_estimated >> 8 );
		packet.payload[ offset + 5 ]  = (byte)( z_estimated );
		packet.payload[ offset + 6 ]  = (byte)( dz_estimated >> 8 );
		packet.payload[ offset + 7 ]  = (byte)( dz_estimated );
		packet.payload[ offset + 8 ]  = (byte)( ddz >> 8 );
		packet.payload[ offset + 9 ]  = (byte)( ddz );
		packet.payload[ offset + 10 ] = (byte)( ddz_filtered >> 8 );
		packet.payload[ offset + 11 ] = (byte)( ddz_filtered );
		packet.payload[ offset + 12 ] = (byte)( p_term >> 8 );
		packet.payload[ offset + 13 ] = (byte)( p_term );
		packet.payload[ offset + 14 ] = (byte)( i_term >> 8 );
		packet.payload[ offset + 15 ] = (byte)( i_term );
		packet.payload[ offset + 16 ] = (byte)( d_term >> 8 );
		packet.payload[ offset + 17 ] = (byte)( d_term );
		packet.payload[ offset + 18 ] = (byte)( dd_term >> 8 );
		packet.payload[ offset + 19 ] = (byte)( dd_term );
		packet.payload[ offset + 20 ] = (byte)( uz >> 8 );
		packet.payload[ offset + 21 ] = (byte)( uz );
		packet.payload[ offset + 22 ] = (byte)( z_cmd >> 8 );
		packet.payload[ offset + 23 ] = (byte)( z_cmd );
		packet.payload[ offset + 24 ] = (byte)( id >> 8 );
		packet.payload[ offset + 25 ] = (byte)( id );
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
        z            = (short)( (packet.payload[ offset + 0 ]  << 8) | (packet.payload[ offset + 1 ]  & 0xFF) );
        z_filtered   = (short)( (packet.payload[ offset + 2 ]  << 8) | (packet.payload[ offset + 3 ]  & 0xFF) );
        z_estimated  = (short)( (packet.payload[ offset + 4 ]  << 8) | (packet.payload[ offset + 5 ]  & 0xFF) );
        dz_estimated = (short)( (packet.payload[ offset + 6 ]  << 8) | (packet.payload[ offset + 7 ]  & 0xFF) );
        ddz          = (short)( (packet.payload[ offset + 8 ]  << 8) | (packet.payload[ offset + 9 ]  & 0xFF) );
        ddz_filtered = (short)( (packet.payload[ offset + 10 ] << 8) | (packet.payload[ offset + 11 ] & 0xFF) );
        p_term       = (short)( (packet.payload[ offset + 12 ] << 8) | (packet.payload[ offset + 13 ] & 0xFF) );
        i_term       = (short)( (packet.payload[ offset + 14 ] << 8) | (packet.payload[ offset + 15 ] & 0xFF) );
        d_term       = (short)( (packet.payload[ offset + 16 ] << 8) | (packet.payload[ offset + 17 ] & 0xFF) );
        dd_term      = (short)( (packet.payload[ offset + 18 ] << 8) | (packet.payload[ offset + 19 ] & 0xFF) );
        uz           = (short)( (packet.payload[ offset + 20 ] << 8) | (packet.payload[ offset + 21 ] & 0xFF) );
        z_cmd        = (short)( (packet.payload[ offset + 22 ] << 8) | (packet.payload[ offset + 23 ] & 0xFF) );
        id           = (short)( (packet.payload[ offset + 24 ] << 8) | (packet.payload[ offset + 25 ] & 0xFF) );
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
		copy.z            = z;
		copy.z_filtered   = z_filtered;
		copy.z_estimated  = z_estimated;
		copy.dz_estimated = dz_estimated;
		copy.ddz          = ddz;
		copy.ddz_filtered = ddz_filtered;
		copy.p_term       = p_term;
		copy.i_term       = i_term;
		copy.d_term       = d_term;
		copy.dd_term      = dd_term;
		copy.uz           = uz;
		copy.z_cmd        = z_cmd;
	}

	public short z;
	public short z_filtered;
	public short z_estimated;
	public short dz_estimated;
	public short ddz;
	public short ddz_filtered;
	public short p_term;
	public short i_term;
	public short d_term;
	public short dd_term;
	public short uz;
	public short z_cmd;
	public short id;
}
