/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   Copyright (c) 2007  IBM Corporation.  All Rights Reserved.              */
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
 * Static methods for low-level packet building
 */
public class LeanPacket
{
	private LeanPacket() {} /* not instantiable */
	
	/** Total bytes of overhead */
	public static final int OVERHEAD = 6;
	
	/** Offset of payload within packet */
	public static final int PAYLOAD_OFFSET = 4;
	
	/** Offset of type within packet */
	public static final int TYPE_OFFSET = 2;
	
	/** Offset of length within packet */
	public static final int LENGTH_OFFSET = 3;
	
	/**
	 * Dump the packet for diagnostic purposes (only where allocation is legal)
	 * @param packet the packet to dump
	 * @param offset the offset at which to start dumping
	 * @param length the number of bytes to dump
	 * @return the result as a String
	 */
	public static String dumpPacket(byte[] packet, int offset, int length)
	{
		StringBuffer toDump = new StringBuffer();
		int sequence = 0;
		for (int i = offset; i < offset + length; i++) {
			int item = packet[i] & 0xff;
			String hex = Integer.toHexString(item);
			if (hex.length() < 2) {
				toDump.append("0");
			}
			toDump.append(hex);
			sequence++;
			if (sequence == 4 && i < offset + length - 1) {
				toDump.append(" ");
				sequence = 0;
			}
		}
		return toDump.toString();
	}

	/** Fill in header */
	public static void fillHeader(byte[] buffer, int type, int length)
	{
		buffer[0] = (byte) 0xff;
		buffer[1] = (byte) 0xff;
		buffer[TYPE_OFFSET] = (byte) type;
		buffer[LENGTH_OFFSET] = (byte) length;
	}
	
	/** Add checksum to packet */
	public static void addChecksum(byte[] buffer)
	{
		int at = PAYLOAD_OFFSET + buffer[LENGTH_OFFSET];
		int checksum = Checksum.calc(buffer[TYPE_OFFSET], buffer[LENGTH_OFFSET], buffer, PAYLOAD_OFFSET);
		buffer[at] = (byte) (checksum >> 8);
		buffer[at + 1] = (byte) checksum;
	}
	
	/** Sanity-check a packet (has correct header and checksum) */
	public static boolean checksOut(byte[] buffer)
	{
		if (buffer[0] != (byte) 0xff || buffer[1] != (byte) 0xff) {
			return false;
		}
		int at = PAYLOAD_OFFSET + buffer[LENGTH_OFFSET];
		if (at < 0 || at > buffer.length - 2) {
			return false;
		}
		int checksum = buffer[at] & 0xFF;
		checksum = (checksum << 8) | buffer[at + 1] & 0xFF;
		int compare = Checksum.calc( buffer[TYPE_OFFSET], buffer[LENGTH_OFFSET], buffer, PAYLOAD_OFFSET);
		return checksum == compare;
	}
}
