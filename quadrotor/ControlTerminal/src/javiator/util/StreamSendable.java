/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   Copyright (c) 2006-2010  Harald Roeck                                   */
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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * A Sendable that encapsulates the contents of a stream that is open to a
 *   a local resource such as a file or byte array.  The length of the stream
 *   must be computable (i.e. it won't work with socket streams).
 */
public class StreamSendable implements Sendable
{
	/** The contents of the sendable */
	private byte[] contents;
	
	/**
	 * Create a new StreamSendable with no contents
	 */
	public StreamSendable()
	{
	}
	
	/**
	 * Create a new StreamSendable from an InputStream
	 * @param stream the InputStream containing the contents
	 */
	public StreamSendable(InputStream stream)
	{
		try {
			contents = new byte[stream.available()];
			stream.read(contents);
		} catch (IOException e) {
			throw new IllegalArgumentException(e);
		}
	}
	
	public InputStream getStream()
	{
		if (contents == null) {
			return null;
		}
		return new ByteArrayInputStream(contents);
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#clone()
	 */
	public Object clone()
	{
		StreamSendable ans = new StreamSendable();
		ans.contents = (byte[]) contents.clone();
		return ans;
	}
	
	/* (non-Javadoc)
	 * @see javiator.util.Sendable#fromPacket(javiator.util.Packet)
	 */
	public void fromPacket(Packet packet)
	{
		contents = (byte[]) packet.payload.clone();
	}

	/* (non-Javadoc)
	 * @see javiator.util.Sendable#getSequence()
	 */
	public long getSequence()
	{
		return 0;
	}

	/* (non-Javadoc)
	 * @see javiator.util.Sendable#reset()
	 */
	public void reset()
	{
		contents = null;
	}

	/* (non-Javadoc)
	 * @see javiator.util.Sendable#setSequence(long)
	 */
	public void setSequence(long seq)
	{
	}

	/* (non-Javadoc)
	 * @see javiator.util.Sendable#toPacket()
	 */
	public Packet toPacket()
	{
		return toPacket(PacketType.COMM_STREAM);
	}

	/* (non-Javadoc)
	 * @see javiator.util.Sendable#toPacket(byte)
	 */
	public Packet toPacket(byte type)
	{
		return new Packet(type, contents);
	}
}
