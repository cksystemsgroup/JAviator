package javiator.terminal;

import java.io.IOException;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketException;

import javiator.util.LeanPacket;
import javiator.util.Packet;
import javiator.util.PacketType;
import javiator.util.ReportToGround;

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
	public UDPTransceiver(ControlTerminal parent, String sendHost, int sendPort, int listenPort)
	{
		super(parent);
		this.listenPort = listenPort;
		socketAddress = new InetSocketAddress(sendHost, sendPort);
	}

	/* (non-Javadoc)
	 * @see javiator.util.Transceiver#connect()
	 */
	public void connect()
	{
		try {
			recvSocket = new DatagramSocket(listenPort);
			sendSocket = new DatagramSocket();
			haveTraffic = false;
			setConnected(true);
	    setLinked(true);
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
		if (recvSocket != null) {
			sendSocket.close();
			sendSocket = null;
		}
		setConnected(false);
		haveTraffic = false;
	}

	/* (non-Javadoc)
	 * @see javiator.util.Transceiver#receive()
	 */
	protected void receive()
	{
		while (isConnected() && !isHalt()) {        	
			byte[] buffer = new byte[ReportToGround.PACKET_SIZE + LeanPacket.OVERHEAD];
			DatagramPacket recvPacket = new DatagramPacket(buffer, 0, buffer.length);
			try {
				if (!haveTraffic) {
					System.err.println("Waiting for messages from JControl");
				}
				if (DEBUG) {
					System.err.println("Trying to receive");
				}
				recvSocket.receive(recvPacket);
				if (!haveTraffic) {
					System.err.println("First message received from JControl");
					haveTraffic = true;
				}
				arrived();
				if (recvPacket.getLength() != buffer.length) {
					throw new IOException("Wrong length packet received: " + recvPacket.getLength());
				}
				if (!LeanPacket.checksOut(buffer)) {
					throw new IOException("Corrupt packet received");
				}
				if (buffer[LeanPacket.TYPE_OFFSET] != PacketType.COMM_GROUND_REPORT) {
					throw new IOException("Bad packet type " + buffer[LeanPacket.TYPE_OFFSET] + " received");
				}
				if (DEBUG) {
					System.err.println("Processing packet: " + LeanPacket.dumpPacket(buffer, 0, buffer.length));
				}
				Packet thePacket = new Packet(PacketType.COMM_GROUND_REPORT, ReportToGround.PACKET_SIZE);
				System.arraycopy(buffer,LeanPacket.PAYLOAD_OFFSET, thePacket.payload,0, ReportToGround.PACKET_SIZE);
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
