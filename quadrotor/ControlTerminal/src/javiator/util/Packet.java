/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */  
/*                                                                           */
/*   Copyright (c) 2006  Harald Roeck                                        */
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
 * Represents a packet in our system that is send and received
 * by a Transceiver.
 * Each packet in our system looks as follows:
 * <pre>
 *          +------+------+------+------+---...---+------+------+ 
 *          | 0xFF | 0xFF | type | size | payload |  checksum   | 
 *          +------+------+------+------+---...---+------+------+ 
 *    byte -2     -1      0      1      2     (size+2)      (size+4) 
 *</pre>
 *
 * The checksum should be calculated explicitly by calling calcChecksum
 * before a packet is send through a Transceiver.
 * 
 * @author hroeck
 *
 */

public class Packet
{
	public static final int MAX_SIZE = 64;
  /** 
   * generates a packet that can hold size bytes.
   * allocates the byte array for payload with new byte[size]
   * 
   * @param size the number of bytes the packet can hold
   */
  public Packet( int size )
  {
    payload = new byte[size];
    this.size = (byte)size;
  }

  /**
   * generates a packet that can hold size bytes
   * and of type type. allocates the byte array for 
   * payload with new byte[size]
   * 
   * @param type the type of the new packet
   * @param size the number of bytes the packet can hold
   */
  public Packet( byte type, int size )
  {   
    this.type = type;
    if( size > 0 )
      payload = new byte[size];
    else
      payload = null;
    this.size = (byte)size;
  }

  /**
   * generates a packet of type type using  
   * payload. the size is set to payload.length
   * 
   * @param type the type of the new packet
   * @param payload the byte array used as payload
   */
  public Packet( byte type, byte[] payload )
  {
    this.type = type;
    this.payload = payload;
    if( payload != null )
      this.size = (byte)(payload.length);
    else
      this.size = 0;
    calcChecksum();
  }
  
  /**
   * calculates, sets and returns the checksum over type, size, and
   * payload
   * @return the checksum of the packet 
   */
  public int calcChecksum()
  {
    checksum = Checksum.calc( type, size, payload, 0 ) ;
    return checksum;
  }

  /**
   * the packet type
   */
  public byte   type;
  
  /**
   * the number of valid bytes in the payload.
   * This may not be bigger than 127 or payload.length
   */
  public byte   size;
  
  /**
   * a byte array that contains the data of this packet.
   * It must be be long enough to contain size bytes.
   */
  public byte[] payload;
  /**
   * 2 byte checksum that is calculated over
   * the two header bytes and the the valid 
   * payload types
   */
  public int    checksum;
}
