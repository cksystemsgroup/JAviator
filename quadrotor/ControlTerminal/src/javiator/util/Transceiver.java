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

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;

/**
 * Abstract class that represents a bidirectional port. A thread is used to poll
 * the input source for new data.
 * @author hroeck
 */
public abstract class Transceiver implements Runnable, Port
{
    public static final long serialVersionUID = 1;

    /**
     * calls connect and starts the receive function
     */
    public void run( )
    {
        connect( );
        receive( );
        System.out.println( "Receive loop stopped - thread terminates... " );
    }

    /**
     * @return lastPacket
     */
    public Packet getPacket( )
    {
        return( lastPacket );
    }

    /**
     * @return Returns the connected.
     */
    public synchronized boolean isConnected( )
    {
        return( connected );
    }

    /**
     * @param connected
     *            The connected to set. wakes threads that wait for the
     *            connection
     */
    protected synchronized void setConnected( boolean connected )
    {
        this.connected = connected;
        this.notifyAll( );
    }

    /**
     * @return Returns the halt.
     */
    public synchronized boolean isHalt( )
    {
        return( halt );
    }

    /**
     * @param halt
     *            The halt to set. Wakes threads that wait for the connection
     */
    protected synchronized void setHalt( boolean halt )
    {
        this.halt = halt;
        this.notifyAll( );
    }

    protected synchronized void flush( ) throws java.io.IOException
    {
    	if( output != null )
    	{
    		output.flush( );
    	}
    }
    
    /**
     * sends a packet using transmitByte
     * @param packet
     *            The packet to send to the output
     */
    public synchronized void sendPacket( Packet packet )
    {    	
        try
        {
            transmitByte( new byte[] {
                PacketType.COMM_PACKET_MARK,
            	PacketType.COMM_PACKET_MARK,
                packet.type,
                packet.size
            } );

            if( packet.payload != null && packet.payload.length > 0 )
            {
                transmitByte( packet.payload );
            }

            transmitByte( (byte)( packet.checksum >> 8 ) );
            transmitByte( (byte)( packet.checksum ) );
            flush( );
        }
        catch( java.io.IOException e )
        {
            System.err.println( name + " sendPacket ERROR: " + e.getMessage( ) );
            e.printStackTrace( );
            setHalt( true );
            disconnect( );
        }
    }

    /**
     * instructs the receiver thread to terminate
     */
    public void terminate( )
    {
        setHalt( true );
        disconnect( );
    }

    protected Transceiver( String name )
    {
        this.name = name;
    }

    /**
     * to be implemented by a subclass. The connect method shall open the
     * connection to remote system and sets Input and Output to valid input and
     * output streams respectivelly.
     * @return succesfully connected or not
     */
    public abstract void connect( );

    /**
     * not used yet
     */
    public abstract void disconnect( );

    protected byte readByte( ) throws EndOfStreamException, IOException
    {
        int t;

        if( input == null )
        {
            throw new IOException( "no input source" );
        }

        t = input.read( );
      
        if( t == -1 )
        {
            throw new EndOfStreamException( );
        }

        return( (byte) t );
    }

    /**
     * Polls the input stream Input for new data. The run method calls this
     * function when the connection was successfully established. Polls the
     * input until terminate is called and generates packets from the input data
     * and hands them over to processPacket
     */
    protected void receive( )
    {
        byte data, type, size;
        int  checksum;

        while( isConnected( ) && !isHalt( ) )
        {        	
            try
            {            	
                do
                {
                    data = readByte( );
                }
                while( data != PacketType.COMM_PACKET_MARK );

                arrived( );
                data = readByte( );

                if( data != PacketType.COMM_PACKET_MARK )
                {
                    continue;
                }

                type = readByte( );

                /* check for valid type */
                if( type < 0x01 || type > PacketType.COMM_PACKET_LIMIT )
                {
                    continue;
                }

                size = readByte( );

                /* check for allowed size */
                if( size < 0 )
                {
                    continue;
                }

                Packet packet = new Packet( (byte) type, size );

                for( int i = 0; i < size; ++i )
                {
                    packet.payload[i] = readByte( );
                }

                checksum = readByte( ) & 0xFF;
                checksum = (checksum << 8) | (readByte( ) & 0xFF);

                if( checksum == packet.calcChecksum( ) )
                {
                    processPacket( packet );
                    lastPacket = packet;
                }
                else
                {
                    System.err.println( name + " receive Checksum ERROR: "
                        + checksum + " != " + packet.calcChecksum( ) );

                    System.err.print( " " + packet.type + " " + packet.size + " " );

                    for( int i = 0; i < packet.size; ++i )
                    {
                        System.err.print( " " + ( (int) packet.payload[i] & 0xFF ) );
                    }

                    System.out.println( " " );
                }
            }
            catch( EndOfStreamException e )
            {
                System.err.println( name + " reached end of input " + e.getMessage( ) );
                e.printStackTrace( );
                disconnect( );

                if( !isHalt( ) )
                {
                    System.err.println( "trying to reconnect..." );
                    connect( );
                }
            }
            catch( IOException e )
            {
                System.err.println( name + " receive ERROR: " + e.getMessage( ) );
                e.printStackTrace( );
                disconnect( );

                if( !isHalt( ) )
                {
                    System.err.println( "trying to reconnect..." );
                    connect( );
                }
            }
            try
            {
            	flush( );
            }
            catch( IOException e )
            {
            	System.err.println( name + " flush ");
            	e.printStackTrace( );
            	disconnect( );
            }
        }

        setHalt( true );
    }

    /**
     * Override in subclasses to track packet arrivals
     */
    protected void arrived( )
	{
	}

	/**
     * writes one byte to Output
     * @param b the byte to write to the output stream
     */
    protected void transmitByte( final int b ) throws IOException
    {        
        if( output == null )
        {
            throw new IOException( "output is null" );
        }

        output.write( b & 0xFF );
    }

    protected void transmitByte( final byte[] b ) throws IOException
    {
        for( int i = 0; i < b.length; ++i )
        {
            transmitByte( b[i] );
        }
    }

    /**
     * dummy function that forwards all known packets to the relay Transceiver.
     * This method should be overwritten by subclasses to intercept packets that
     * are interesting for the respective implementation. A subclass should call
     * <code>super.processPacket( packet )</code> for packets it is not
     * interested in.
     */
    protected void processPacket( Packet packet )
    {
        switch( packet.type )
        {
            case PacketType.COMM_CTRL_PERIOD:
            case PacketType.COMM_JAVIATOR_DATA:
            case PacketType.COMM_INERTIAL_DATA:
            case PacketType.COMM_SENSOR_DATA:
            case PacketType.COMM_MOTOR_SIGNALS:
            case PacketType.COMM_MOTOR_OFFSETS:
            case PacketType.COMM_STATE_MODE:
            case PacketType.COMM_GROUND_REPORT:
            case PacketType.COMM_TRACE_DATA:
            case PacketType.COMM_COMMAND_DATA:
            case PacketType.COMM_R_P_PARAMS:
            case PacketType.COMM_YAW_PARAMS:
            case PacketType.COMM_ALT_PARAMS:
            case PacketType.COMM_X_Y_PARAMS:
            case PacketType.COMM_IDLE_LIMIT:
            case PacketType.COMM_STREAM:
            case PacketType.COMM_SWITCH_MODE:
            case PacketType.COMM_TEST_MODE:
            case PacketType.COMM_SHUT_DOWN:
            case PacketType.COMM_EN_SENSORS:

                if( relay != null && relay.isConnected( ) )
                {
                    relay.sendPacket( packet );
                }
                break;

            default:
                System.err.println( "processPacket WARNING: unkown packet type "
                                    + packet.type );
                break;
        }
    }

    /** input from where we pull the data */
    protected InputStream       input       = null;

    /** output to where we send the data */
    protected OutputStream      output      = null;

    /** last raw packet processed */
    protected Packet            lastPacket  = null;

    private boolean             connected   = false;
    
    private Thread              thread      = null;
    /**
     * flag to indicate that the receiver thread should halt
     */
    protected boolean           halt        = false;

    /**
     * the identification string send when a connection is established
     */
    protected String            name;

    /**
     * the transceiver to where packets should be forwarded if we cannot handle
     * them.
     */
    protected Transceiver       relay       = null;

    /**
     * sets the relay transceiver where packets are forwarded
     * @param relay
     *            The relay transceiver to set.
     */
    public synchronized void setRelay( Transceiver relay )
    {
        this.relay = relay;
    }

	public Thread getThread( )
	{
		return( thread );
	}

	public void setThread( Thread thread )
	{
		this.thread = thread;
	}
}

// End of file.