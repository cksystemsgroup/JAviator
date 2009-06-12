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
		filtered_z   = 0;
		estimated_z  = 0;
		estimated_dz = 0;
		ddz          = 0;
		filtered_ddz = 0;
        p_term       = 0;
        i_term       = 0;
        d_term       = 0;
        dd_term      = 0;
		uz           = 0;
		cmd_z        = 0;
		id           = 0;
	}

	public String toString( )
	{
		String result;
		result  = " z: "            + z;
		result += " filtered z: "   + filtered_z;
		result += " estimated z: "  + estimated_z;
		result += " estimated dz: " + estimated_dz;
		result += " ddz: "          + ddz;
		result += " filtered ddz: " + filtered_ddz;
		result += " p-term: "       + p_term;
		result += " i-term: "       + i_term;
		result += " d-term: "       + d_term;
		result += " dd-term: "      + dd_term;
		result += " uz: "           + uz;
		result += " z-command: "    + cmd_z;
		result += " ID: "           + id;

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
		packet.payload[ offset + 2 ]  = (byte)( filtered_z >> 8 );
		packet.payload[ offset + 3 ]  = (byte)( filtered_z );
		packet.payload[ offset + 4 ]  = (byte)( estimated_z >> 8 );
		packet.payload[ offset + 5 ]  = (byte)( estimated_z );
		packet.payload[ offset + 6 ]  = (byte)( estimated_dz >> 8 );
		packet.payload[ offset + 7 ]  = (byte)( estimated_dz );
		packet.payload[ offset + 8 ]  = (byte)( ddz >> 8 );
		packet.payload[ offset + 9 ]  = (byte)( ddz );
		packet.payload[ offset + 10 ] = (byte)( filtered_ddz >> 8 );
		packet.payload[ offset + 11 ] = (byte)( filtered_ddz );
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
		packet.payload[ offset + 22 ] = (byte)( cmd_z >> 8 );
		packet.payload[ offset + 23 ] = (byte)( cmd_z );
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
        filtered_z   = (short)( (packet.payload[ offset + 2 ]  << 8) | (packet.payload[ offset + 3 ]  & 0xFF) );
        estimated_z  = (short)( (packet.payload[ offset + 4 ]  << 8) | (packet.payload[ offset + 5 ]  & 0xFF) );
        estimated_dz = (short)( (packet.payload[ offset + 6 ]  << 8) | (packet.payload[ offset + 7 ]  & 0xFF) );
        ddz          = (short)( (packet.payload[ offset + 8 ]  << 8) | (packet.payload[ offset + 9 ]  & 0xFF) );
        filtered_ddz = (short)( (packet.payload[ offset + 10 ] << 8) | (packet.payload[ offset + 11 ] & 0xFF) );
        p_term       = (short)( (packet.payload[ offset + 12 ] << 8) | (packet.payload[ offset + 13 ] & 0xFF) );
        i_term       = (short)( (packet.payload[ offset + 14 ] << 8) | (packet.payload[ offset + 15 ] & 0xFF) );
        d_term       = (short)( (packet.payload[ offset + 16 ] << 8) | (packet.payload[ offset + 17 ] & 0xFF) );
        dd_term      = (short)( (packet.payload[ offset + 18 ] << 8) | (packet.payload[ offset + 19 ] & 0xFF) );
        uz           = (short)( (packet.payload[ offset + 20 ] << 8) | (packet.payload[ offset + 21 ] & 0xFF) );
        cmd_z        = (short)( (packet.payload[ offset + 22 ] << 8) | (packet.payload[ offset + 23 ] & 0xFF) );
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
		copy.filtered_z   = filtered_z;
		copy.estimated_z  = estimated_z;
		copy.estimated_dz = estimated_dz;
		copy.ddz          = ddz;
		copy.filtered_ddz = filtered_ddz;
		copy.p_term       = p_term;
		copy.i_term       = i_term;
		copy.d_term       = d_term;
		copy.dd_term      = dd_term;
		copy.uz           = uz;
		copy.cmd_z        = cmd_z;
	}

	public short z;
	public short filtered_z;
	public short estimated_z;
	public short estimated_dz;
	public short ddz;
	public short filtered_ddz;
	public short p_term;
	public short i_term;
	public short d_term;
	public short dd_term;
	public short uz;
	public short cmd_z;
	public short id;
}
