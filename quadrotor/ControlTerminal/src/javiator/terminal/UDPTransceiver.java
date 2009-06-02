package javiator.terminal;

import java.io.IOException;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketException;

import javiator.util.LeanPacket;
import javiator.util.Packet;
import javiator.util.PacketType;

/**
 * An alternate JControlTransceiver that uses UDP instead of TCP
 */
public class UDPTransceiver extends Transceiver
{
	private static final boolean DEBUG = false;
	private DatagramSocket sendSocket, recvSocket;
	private int listenPort;
	private InetSocketAddress socketAddress;
	private boolean haveTraffic;
	
	/**
	 * Create a new UDP Transceiver for talking to the JControl
	 * @param parent the ControlTerminal object needed by superclass
	 * @param listenPort the port to listen on for datagrams
	 * @param sendHost the host to which to send datagrams
	 * @param sendPort the port to which to send datagrams
	 */
	public UDPTransceiver(ControlTerminal parent, String sendHost, int sendPort)
	{
		super(parent);
		this.listenPort = sendPort;
		socketAddress = new InetSocketAddress(sendHost, sendPort);
	}

	/* (non-Javadoc)
	 * @see javiator.util.Transceiver#connect()
	 */
	public void connect()
	{
		try {
		//	recvSocket = new DatagramSocket(listenPort);
			sendSocket = new DatagramSocket(listenPort);
			haveTraffic = false;
			setConnected(true);
	    setLinked(true);
	   
	    
	    	Packet packet = new Packet(PacketType.COMM_SWITCH_MODE, 0);
	    	sendPacket(packet);   	
	    	
	    	
		} catch (SocketException e) {
			e.printStackTrace();
	    setConnected(false);
	    setLinked(false);
		}
	}

	/* (non-Javadoc)
	 * @see javiator.util.Transceiver#disconnect()
	 */
	public void disconnect()
	{
		
		if (recvSocket != null) {
			recvSocket.close();
			recvSocket = null;
		}
		
		if (sendSocket != null) {
			sendSocket.close();
			sendSocket = null;
		}
		setConnected(false);
		haveTraffic = false;
	}

	/* (non-Javadoc)
	 * @see javiator.util.Transceiver#receive()
	 */
	public void receive()
	{
		
		while (isConnected() && !isHalt()) {  	
			byte[] buffer = new byte[Packet.MAX_SIZE];
			DatagramPacket recvPacket = new DatagramPacket(buffer, 0, buffer.length);
			try {
				if (!haveTraffic) {
					System.err.println("Waiting for messages from control application");
				}
				if (DEBUG) {
					System.err.println("Trying to receive");
				}
				sendSocket.receive(recvPacket);
				if (!haveTraffic) {
					System.err.println("First message received from control application");
					haveTraffic = true;
				}
				arrived();
				
				if (!LeanPacket.checksOut(buffer)) {
					throw new IOException("Corrupt packet received");
				}
				
				if (DEBUG) {
					System.err.println("Processing packet: " + LeanPacket.dumpPacket(buffer, 0, buffer.length));
				}
				Packet thePacket = new Packet(buffer[2], buffer[3]);
				System.arraycopy(buffer,LeanPacket.PAYLOAD_OFFSET, thePacket.payload, 0, buffer[3]);
				processPacket(thePacket);
				if (DEBUG) {
					System.err.println("Processing complete");
				}
				lastPacket = thePacket;
			} catch (IOException e) {
				e.printStackTrace();
				System.err.println(LeanPacket.dumpPacket(buffer, 0, buffer.length));
				disconnect();
				if (!isHalt()) {
					connect();
				}
			}
		}
		setHalt(true);
	}

	/* (non-Javadoc)
	 * @see javiator.util.Transceiver#sendPacket(javiator.util.Packet)
	 */
	public synchronized void sendPacket(Packet packet)
	{
		byte[] buffer = new byte[packet.size + LeanPacket.OVERHEAD];
		LeanPacket.fillHeader(buffer, packet.type, packet.size);
		if (packet.payload != null) {
			System.arraycopy(packet.payload,0, buffer,LeanPacket.PAYLOAD_OFFSET, packet.size);
		}
		LeanPacket.addChecksum(buffer);
		if (DEBUG) {
			System.err.println("Sending packet: " + LeanPacket.dumpPacket(buffer, 0, buffer.length));
		}
		try {
			DatagramPacket sendPacket = new DatagramPacket(buffer, 0, buffer.length, socketAddress);
			sendSocket.send(sendPacket);
		} catch (IOException e) {
			e.printStackTrace();
			disconnect();
			if (!isHalt()) {
				connect();
			}
		}
	}
}
