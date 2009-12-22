/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   Transceiver.java    Establishes and runs the serial connection between  */
/*                       the JAviator and the control terminal.              */
/*                                                                           */
/*   Copyright (c) 2006-2009  Rainer Trummer, Harald Roeck                   */
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

package javiator.terminal;

import java.awt.Color;

import javiator.util.ReportToGround;
import javiator.util.SensorData;
import javiator.util.MotorSignals;
import javiator.util.CommandData;
import javiator.util.TraceData;
import javiator.util.Packet;
import javiator.util.PacketType;

/*****************************************************************************/
/*                                                                           */
/*   Class Transceiver                                                       */
/*                                                                           */
/*****************************************************************************/

public abstract class Transceiver extends javiator.util.Transceiver
{
    public static final long serialVersionUID = 1;
    public static final int  UDP_CONNECT      = 1;
	public static final int  TCP_CONNECT      = 2;

    public static Transceiver createInstance( ControlTerminal parent, int connectType )
    {
        if( Instance == null )
        {
            if( connectType == UDP_CONNECT )
            {
            	Instance = new UDPTransceiver( parent, parent.relayHost, parent.relayPort );
            }
            else
            if( connectType == TCP_CONNECT )
            {
                Instance = new TCPTransceiver( parent, parent.relayHost, parent.relayPort );
            }
            else
            {
            	System.err.println( "Transceiver.createInstance: undefined connection type");
            }
        }

        return( Instance );
    }

    public void run( )
    {
        receive( );
    }

    public void terminate( )
    {
        super.terminate( );
        parent.setConnected( false );
        parent.toggleConnect.setForeground( Color.BLACK );
        parent.toggleConnect.setText( ControlTerminal.CONNECT_TO + ControlTerminal._HELI );
        Instance = null;
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Protected Section                                                   */
    /*                                                                       */
    /*************************************************************************/

    protected ControlTerminal parent       = null;
    protected CommandData     commandData  = null;
    protected SensorData      sensorData   = null;
    protected MotorSignals    motorSignals = null;
    protected CommandData     motorOffsets = null;
    protected TraceData       traceData    = null;

    protected Transceiver( ControlTerminal parent )
    {
        super( parent.getClass( ).getSimpleName( ) );

        this.parent  = parent;
        commandData  = new CommandData( );
        sensorData   = new SensorData( );
        motorSignals = new MotorSignals( );
        motorOffsets = new CommandData( );
        traceData    = new TraceData( );
    }

    protected void setLinked( boolean linked )
    {
        parent.setConnected( linked ); // enable connection-dependent buttons

        try
        {
            if( linked )
            {
                parent.toggleConnect.setForeground( Color.BLUE );
                parent.toggleConnect.setText( "JAviator connected" );
                Thread.sleep( 2000 );
                parent.toggleConnect.setForeground( Color.BLACK );
                parent.toggleConnect.setText( ControlTerminal.DISCONNECT + ControlTerminal._HELI );
            }
            else
            {
                parent.toggleConnect.setText( ControlTerminal.CONNECTION + ControlTerminal._FAILED );

                for( int i = 0; i < 5; ++i )
                {
                    parent.toggleConnect.setForeground( Color.RED );
                    Thread.sleep( 500 );
                    parent.toggleConnect.setForeground( Color.WHITE );
                    Thread.sleep( 250 );
                }

                parent.toggleConnect.setForeground( Color.BLACK );
                parent.toggleConnect.setText( ControlTerminal.CONNECT_TO + ControlTerminal._HELI );
            }
        }
        catch( InterruptedException e )
        {
            System.err.println( "Transceiver.setLinked: " + e.getMessage( ) );
        }
    }

    protected void processPacket( Packet packet )
    {
        switch( packet.type )
        {
       
	        case PacketType.COMM_SENSOR_DATA:
	        	parent.getCommandData( ).copyTo( commandData );
                sendPacket( commandData.toPacket( PacketType.COMM_COMMAND_DATA ) );
                if( parent.isNewIdlingSpeed( ) )
                {
                	byte[] maxIdlingSpeed = { (byte)( parent.idlingSpeed >> 8 ), (byte)( parent.idlingSpeed ) };
                	sendPacket( new Packet( PacketType.COMM_IDLE_LIMIT, maxIdlingSpeed ) );
                	parent.resetNewIdlingSpeed( );
                }
                if( parent.isNew_R_P_Params( ) )
                {
            	    sendPacket( parent.getNew_R_P_Params( ).toPacket( PacketType.COMM_R_P_PARAMS ) );
                }
                if( parent.isNew_Yaw_Params( ) )
                {
            	    sendPacket( parent.getNew_Yaw_Params( ).toPacket( PacketType.COMM_YAW_PARAMS ) );
                }
                if( parent.isNew_Alt_Params( ) )
                {
            	    sendPacket( parent.getNew_Alt_Params( ).toPacket( PacketType.COMM_ALT_PARAMS ) );
                }
                if( parent.isNew_X_Y_Params( ) )
                {
            	    sendPacket( parent.getNew_X_Y_Params( ).toPacket( PacketType.COMM_X_Y_PARAMS ) );
                }
                parent.resetChangedParamID( );
	        	sensorData.fromPacket( packet );
	            parent.setSensorData( sensorData );
            break;

            case PacketType.COMM_MOTOR_SIGNALS:
                motorSignals.fromPacket( packet );
                parent.digitalMeter.setMotorSignals( motorSignals );
                parent.checkJAviatorSettled( motorSignals );
                break;

            case PacketType.COMM_MOTOR_OFFSETS:
                motorOffsets.fromPacket( packet );
                parent.digitalMeter.setMotorOffsets( motorOffsets );
                break;

            case PacketType.COMM_STATE_MODE:
                parent.digitalMeter.setStateAndMode( packet.payload[0], packet.payload[1] );
                break;

            case PacketType.COMM_GROUND_REPORT:
                sendPacket( parent.getCommandData( ).toPacket( PacketType.COMM_COMMAND_DATA ) );
                if( parent.isNewIdlingSpeed( ) )
                {
                	byte[] maxIdlingSpeed = { (byte)( parent.idlingSpeed >> 8 ), (byte)( parent.idlingSpeed ) };
                	sendPacket( new Packet( PacketType.COMM_IDLE_LIMIT, maxIdlingSpeed ) );
                	parent.resetNewIdlingSpeed( );
                }
                if( parent.isNew_R_P_Params( ) )
                {
            	    sendPacket( parent.getNew_R_P_Params( ).toPacket( PacketType.COMM_R_P_PARAMS ) );
                }
                if( parent.isNew_Yaw_Params( ) )
                {
            	    sendPacket( parent.getNew_Yaw_Params( ).toPacket( PacketType.COMM_YAW_PARAMS ) );
                }
                if( parent.isNew_Alt_Params( ) )
                {
            	    sendPacket( parent.getNew_Alt_Params( ).toPacket( PacketType.COMM_ALT_PARAMS ) );
                }
                if( parent.isNew_X_Y_Params( ) )
                {
            	    sendPacket( parent.getNew_X_Y_Params( ).toPacket( PacketType.COMM_X_Y_PARAMS ) );
                }
                parent.resetChangedParamID( );
                ReportToGround report = new ReportToGround( );
                report.fromPacket( packet );
                report.sensorData.copyTo( sensorData );
                parent.setSensorData( sensorData );
                report.motorSignals.copyTo( motorSignals ); 
                parent.digitalMeter.setMotorSignals( motorSignals );
                parent.checkJAviatorSettled( motorSignals );
                report.motorOffsets.copyTo( motorOffsets );
                parent.digitalMeter.setMotorOffsets( motorOffsets );
                parent.digitalMeter.setStateAndMode( report.stateAndMode.state, report.stateAndMode.mode );
                break;

            case PacketType.COMM_TRACE_DATA:
                traceData.fromPacket( packet );
	            parent.writeLogData( commandData, sensorData, motorSignals, motorOffsets, traceData );
                break;

            default:
                return;
        }
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private static Transceiver Instance = null;
}

// End of file.