package javiator.util;

public class StateAndMode extends NumeratedSendable
{
	public static final int PACKET_SIZE = 2;

    public StateAndMode( byte state, byte mode )
    {
        this.state = state;
        this.mode  = mode;
    }

    public void reset( )
    {
        state = 0;
        mode  = 0;
    }

    public synchronized Packet toPacket( )
    {
        Packet packet = new Packet( PACKET_SIZE );
        encode( packet, 0 );

        return( packet );
    }

	public void encode( Packet packet, int offset )
	{
		packet.payload[ offset + 0 ] = state;
		packet.payload[ offset + 1 ] = mode;
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

	public void decode( Packet packet, int offset )
	{
		state = (byte)( packet.payload[ offset + 0 ] & 0xFF );
		mode  = (byte)( packet.payload[ offset + 1 ] & 0xFF );
	}

    public synchronized Object clone( )
    {
        return new StateAndMode( state, mode );
    }

    public void copyTo( Copyable to )
	{
    	super.copyTo( to );
    	StateAndMode copy = (StateAndMode) to;
    	copy.state           = state;
    	copy.mode            = mode;
	}

    public byte state;
    public byte mode;
}
