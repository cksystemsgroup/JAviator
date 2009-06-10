package javiator.util;

/**
 *  Represents the actuator data
 * @author hroeck
 */
public class MotorSignals extends NumeratedSendable
{
	public static final int PACKET_SIZE = 10;
		
    public MotorSignals( )
    {
        reset( );
    }

    public void reset( )
    {
        front = 0;
        right = 0;
        rear  = 0;
        left  = 0;
        id    = 0;
    }

    public String toString( )
    {
        String result;
        result  = " front: " + front;
        result += " right: " + right;
        result += " rear: "  + rear;
        result += " left: "  + left;
        result += " ID: "    + id;

        return( result );
    }

    public void copySignals( MotorSignals data )
    {
        front = data.front;
        right = data.right;
        rear  = data.rear;
        left  = data.left;
        id    = data.id;
    }

    public synchronized Packet toPacket( )
    {
        Packet packet = new Packet( PACKET_SIZE );
        encode( packet, 0 );

        return( packet );
    }

	public void encode( Packet packet, int offset )
	{
		packet.payload[ offset + 0 ] = (byte)( front >> 8 );
		packet.payload[ offset + 1 ] = (byte)( front );
		packet.payload[ offset + 2 ] = (byte)( right >> 8 );
		packet.payload[ offset + 3 ] = (byte)( right );
		packet.payload[ offset + 4 ] = (byte)( rear >> 8 );
		packet.payload[ offset + 5 ] = (byte)( rear );
		packet.payload[ offset + 6 ] = (byte)( left >> 8 );
		packet.payload[ offset + 7 ] = (byte)( left );
		packet.payload[ offset + 8 ] = (byte)( id >> 8 );
		packet.payload[ offset + 9 ] = (byte)( id );
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
        front = (short)( (packet.payload[ offset + 0 ] << 8) | (packet.payload[ offset + 1 ] & 0xFF) );
        right = (short)( (packet.payload[ offset + 2 ] << 8) | (packet.payload[ offset + 3 ] & 0xFF) );
        rear  = (short)( (packet.payload[ offset + 4 ] << 8) | (packet.payload[ offset + 5 ] & 0xFF) );
        left  = (short)( (packet.payload[ offset + 6 ] << 8) | (packet.payload[ offset + 7 ] & 0xFF) );
        id    = (char) ( (packet.payload[ offset + 8 ] << 8) | (packet.payload[ offset + 9 ] & 0xFF) );
    }

    public synchronized Object clone( )
    {
        MotorSignals copy = new MotorSignals( );
        copyTo( copy );

        return( copy );
    }
    
    public MotorSignals deepClone( )
    {
        return (MotorSignals) clone( );
    }

    public void copyTo( Copyable to )
	{
    	super.copyTo( to );
    	MotorSignals copy = (MotorSignals) to;
        copy.front        = front;
        copy.right        = right;
        copy.rear         = rear;
        copy.left         = left;
        copy.id           = id;
	}

    public short front;
    public short right;
    public short rear;
    public short left;
    public char  id;
}
