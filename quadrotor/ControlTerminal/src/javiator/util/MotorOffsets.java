package javiator.util;

/**
 *  Represents the actuator data
 * @author hroeck
 */
public class MotorOffsets extends NumeratedSendable
{
	public static final int PACKET_SIZE = 8;
		
    public MotorOffsets( )
    {
        reset( );
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

    public void copySignals( MotorOffsets data )
    {
        roll  = data.roll;
        pitch = data.pitch;
        yaw   = data.yaw;
        z     = data.z;
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
    
    public synchronized void decode( Packet packet, int offset )
    {
        roll  = (short)( (packet.payload[ offset + 0 ] << 8) | (packet.payload[ offset + 1 ] & 0xFF) );
        pitch = (short)( (packet.payload[ offset + 2 ] << 8) | (packet.payload[ offset + 3 ] & 0xFF) );
        yaw   = (short)( (packet.payload[ offset + 4 ] << 8) | (packet.payload[ offset + 5 ] & 0xFF) );
        z     = (short)( (packet.payload[ offset + 6 ] << 8) | (packet.payload[ offset + 7 ] & 0xFF) );
    }

    public synchronized Object clone( )
    {
        MotorOffsets copy = new MotorOffsets( );
        copyTo( copy );

        return( copy );
    }
    
    public MotorOffsets deepClone( )
    {
        return (MotorOffsets) clone( );
    }

    public void copyTo( Copyable to )
	{
    	super.copyTo( to );
    	MotorOffsets copy = (MotorOffsets) to;
        copy.roll         = roll;
        copy.pitch        = pitch;
        copy.yaw          = yaw;
        copy.z            = z;
	}

    public short roll;
    public short pitch;
    public short yaw;
    public short z;
}
