/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   ControlTerminal.java	Runs the JAviator Control Terminal application.  */
/*                                                                           */
/*   Copyright (c) 2006-2010  Rainer Trummer                                 */
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

import java.awt.Frame;
import java.awt.Image;
import java.awt.Panel;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.MediaTracker;
import java.awt.AWTEvent;

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowEvent;
import java.awt.event.FocusEvent;
import java.awt.event.KeyEvent;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.IOException;

import javiator.util.CommandData;
import javiator.util.ControllerConstants;
import javiator.util.SensorData;
import javiator.util.MotorSignals;
import javiator.util.MotorOffsets;
import javiator.util.ControlParams;
import javiator.util.TraceData;
import javiator.util.Packet;
import javiator.util.PacketType;

//import javiator.signals.SignalsDialog;

import com.centralnexus.input.Joystick;

/*****************************************************************************/
/*                                                                           */
/* Class ControlTerminal                                                     */
/*                                                                           */
/*****************************************************************************/

public class ControlTerminal extends Frame
{
    public static final long serialVersionUID = 1;

    public static final boolean SMALL_METER   = true;
    public static final boolean SMALL_FONT    = true;
    public static final boolean SHOW_3DWINDOW = false;

    public static final Dimension UBI_RECT    = new Dimension( 7130, 8470 );
    public static final Dimension MAX_RECT    = new Dimension( 3000, 4000 );

    public static final String LOG_FILE_PATH  = "traces/";
    public static final String LOG_FILE_NAME  = LOG_FILE_PATH + "timing_.csv";

    public static final String LOG_TITLE_STR
        = "cmd-roll,cmd-pitch,cmd-yaw,cmd-z," /* 4-DOF (6-DOF) command reference */
        + "ekf-roll,ekf-pitch,ekf-yaw,"       /* EKF-estimated attitude */
        + "droll,dpitch,dyaw,"                /* angular velocity */
        + "ddroll,ddpitch,ddyaw,"             /* angular acceleration */
        + "ekf-x,ekf-y,ekf-z,"                /* EKF-estimated position */
        + "ekf-dx,ekf-dy,ekf-dz,"             /* EKF-estimated velocity */
        + "iir-ddx,iir-ddy,iir-ddz,"          /* IIR-filtered acceleration */
        + "maps,temp,batt,"                   /* air pressure, air temperature, battery level */
        + "front,right,rear,left,"            /* front, right, rear, and left motor signal */
        + "u-roll,u-pitch,u-yaw,u-z,"         /* roll, pitch, yaw, and z control effort */
        + "rot-c-roll,rot-c-pitch,"           /* yaw-rotated y and x control effort */
        + "period,state,"                     /* sampling period and controller state */
        + "start-to-jap,"                     /* start of 'To JAviator Plant' */
        + "end-to-jap,"                       /* end of 'To JAviator Plant' */
        + "end-from-jap,"                     /* end of 'From JAviator Plant' */
        + "end-from-gcs,"                     /* end of 'From Ground Control System' */
        + "end-control,"                      /* end of 'Control Signals Computation' */
        + "end-to-gcs";                       /* end of 'To Ground Control System' */

    public static final String[] PLOT_LIST = {  ""  };

    public ControlTerminal( )
    {
        super( );
    	initWindow( );
    }

    public static void main( String args[] ) throws IOException
    {
        new UIManagerColor( );
        new ControlTerminal( );
    }
/*
    public boolean isShow3D( )
    {
    	return( SHOW_3DWINDOW );
    }

    public JAviator3DControlPanel getJaviator3D( )
    {
        return( visualization );
    }

    public void setJaviator3D( JAviator3DControlPanel visualization )
    {
        this.visualization = visualization;
    }
*/
    public CommandData getCommandData( )
    {
        CommandData data = new CommandData( );

        if( showPosition[0] )
        {
        	/* IMPORTANT: Ubisense location data refer to Cartesian coordinates,
               whereas JAviator location data refer to aircraft coordinates,
               hence x and y must be exchanged when accessing the data. */
        	Point desired = positionDialog.getDesired( );
        	data.roll     = (short) desired.x;
        	data.pitch    = (short) desired.y;
        }
        else
        {
        	data.roll  = (short) meterRoll  .getDesired( );
        	data.pitch = (short) meterPitch .getDesired( );
        }

        data.yaw = (short) meterYaw      .getDesired( );
        data.z   = (short) meterAltitude .getDesired( );

        return( data );
    }

    public boolean isNewIdlingSpeed( )
    {
    	return( newIdlingSpeed );
    }
    
    public void resetNewIdlingSpeed( )
    {
    	newIdlingSpeed = false;
    }

	public boolean isNew_R_P_Params( )
	{
		if( changedParamID[0] >= 0 && changedParamID[0] <= 3 )
		{
            new_R_P_Params = true;
		}

		return( new_R_P_Params );
	}

	public boolean isNew_Yaw_Params( )
	{
		if( changedParamID[0] >= 4 && changedParamID[0] <= 7 )
		{
			new_Yaw_Params = true;
		}

		return( new_Yaw_Params );
	}
	
	public boolean isNew_Alt_Params( )
	{
		if( changedParamID[0] >= 8 && changedParamID[0] <= 11 )
		{
			new_Alt_Params = true;
		}

		return( new_Alt_Params );
	}

	public boolean isNew_X_Y_Params( )
	{
		if( changedParamID[0] >= 12 && changedParamID[0] <= 15 )
		{
            new_X_Y_Params = true;
		}

		return( new_X_Y_Params );
	}

    public ControlParams getNew_R_P_Params( )
    {    	
    	ControlParams ctrlParams = new ControlParams( );

    	ctrlParams.kp  = (short) controlParams[0];
    	ctrlParams.ki  = (short) controlParams[1];
    	ctrlParams.kd  = (short) controlParams[2];
    	ctrlParams.kdd = (short) controlParams[3];
    	new_R_P_Params = false;

    	return( ctrlParams );
    }

    public ControlParams getNew_Yaw_Params( )
    {    	
    	ControlParams ctrlParams = new ControlParams( );

    	ctrlParams.kp  = (short) controlParams[4];
    	ctrlParams.ki  = (short) controlParams[5];
    	ctrlParams.kd  = (short) controlParams[6];
    	ctrlParams.kdd = (short) controlParams[7];
    	new_Yaw_Params = false;

    	return( ctrlParams );
    }

    public ControlParams getNew_Alt_Params( )
    {    	
    	ControlParams ctrlParams = new ControlParams( );

    	ctrlParams.kp  = (short) controlParams[8];
    	ctrlParams.ki  = (short) controlParams[9];
    	ctrlParams.kd  = (short) controlParams[10];
    	ctrlParams.kdd = (short) controlParams[11];
    	new_Alt_Params = false;

    	return( ctrlParams );
    }

    public ControlParams getNew_X_Y_Params( )
    {    	
    	ControlParams ctrlParams = new ControlParams( );

    	ctrlParams.kp  = (short) controlParams[12];
    	ctrlParams.ki  = (short) controlParams[13];
    	ctrlParams.kd  = (short) controlParams[14];
    	ctrlParams.kdd = (short) controlParams[15];
    	new_X_Y_Params = false;

    	return( ctrlParams );
    }

    public void resetChangedParamID( )
    {
    	changedParamID[0] = -1;
    }

    public int[] getMaximumValues( )
    {
        int[] values = { meterRoll     .getMaximum( ),
                         meterPitch    .getMaximum( ),
                         meterAltitude .getMaximum( ) };

        return( values );
    }

    public void setMaximumValues( int[] values )
    {
        meterRoll     .setMaximum( values[0] );
        meterPitch    .setMaximum( values[1] );
        meterAltitude .setMaximum( values[2] );
    }

	public void setSensorData( SensorData data )
    {
        if( showPosition[0] )
        {
        	/* IMPORTANT: Ubisense location data refer to Cartesian coordinates,
               whereas JAviator location data refer to aircraft coordinates,
               hence x and y must be exchanged when accessing the data. */
        	Point current = new Point( data.y, data.x );
        	positionDialog.setCurrent( current );
        }

        meterRoll     .setCurrent( data.roll );
        meterPitch    .setCurrent( data.pitch );
        meterYaw      .setCurrent( data.yaw );
        meterAltitude .setCurrent( data.z );

        if( meterAltitude.getDesired( ) == 0 && meterYaw.getDesired( ) == 0 )
        {
        	meterYaw.setDesired( data.yaw );
        }
/*
        if( SHOW_3DWINDOW && visualization != null )
        {
            SensorData desiredData = new SensorData( );

            desiredData.roll  = (short) meterRoll     .getDesired( );
            desiredData.pitch = (short) meterPitch    .getDesired( );
            desiredData.yaw   = (short) meterYaw      .getDesired( );
            desiredData.z     = (short) meterAltitude .getDesired( );

            visualization.sendSensorData( data, desiredData );
        }

        if( showDiagrams )
        {
        	if( signalsDialog == null )
        	{ 
        		signalsDialog = new SignalsDialog( );
        	}
        	else
        	if( signalsDialog.isClosed( ) )
        	{
        		showDiagrams = false;
        	}

            MotorSignals motor = digitalMeter.getMotorSignals( );

            signalsDialog.z         .add( data.z );
            signalsDialog.roll      .add( data.roll );
            signalsDialog.pitch     .add( data.pitch );
            signalsDialog.yaw       .add( data.yaw );
            signalsDialog.droll     .add( data.droll );
            signalsDialog.dpitch    .add( data.dpitch );
            signalsDialog.dyaw      .add( data.dyaw );
            signalsDialog.ddroll    .add( data.ddroll );
            signalsDialog.ddpitch   .add( data.ddpitch );
            signalsDialog.ddyaw     .add( data.ddyaw );
            signalsDialog.vx        .add( data.dx );
            signalsDialog.vy        .add( data.dy );
            signalsDialog.vz        .add( data.dz );posOffsetX
            signalsDialog.ddx       .add( data.ddx );
            signalsDialog.ddy       .add( data.ddy );
            signalsDialog.ddz       .add( data.ddz );
            signalsDialog.ref_roll  .add( meterRoll.getDesired( ) );
            signalsDialog.ref_pitch .add( meterPitch.getDesired( ) );
            signalsDialog.ref_yaw   .add( meterYaw.getDesired( ) );
            signalsDialog.ref_z     .add( meterAltitude.getDesired( ) );
            signalsDialog.front     .add( motor.front );
            signalsDialog.right     .add( motor.right );
            signalsDialog.rear      .add( motor.rear );
            signalsDialog.left      .add( motor.left );
        }
*/
    }

    public void writeLogData( CommandData  commandData,
                              SensorData   sensorData,
                              MotorSignals motorSignals,
                              MotorOffsets motorOffsets,
                              TraceData    traceData )
    {
        if( logData && logFile != null )
        {
            String csv
			/* Command Data */
			= NIL + (short) commandData.roll
			+ ',' + (short) commandData.pitch
			+ ',' + (short) commandData.yaw
			+ ',' + (short) commandData.z
			/* Sensor Data */
			+ ',' + (short) sensorData.roll
			+ ',' + (short) sensorData.pitch
			+ ',' + (short) sensorData.yaw
			+ ',' + (short) sensorData.droll
			+ ',' + (short) sensorData.dpitch
			+ ',' + (short) sensorData.dyaw
			+ ',' + (short) sensorData.ddroll
			+ ',' + (short) sensorData.ddpitch
			+ ',' + (short) sensorData.ddyaw
			+ ',' + (short) sensorData.x
			+ ',' + (short) sensorData.y
			+ ',' + (short) sensorData.z
			+ ',' + (short) sensorData.dx
			+ ',' + (short) sensorData.dy
			+ ',' + (short) sensorData.dz
			+ ',' + (short) sensorData.ddx
			+ ',' + (short) sensorData.ddy
			+ ',' + (short) sensorData.ddz
			+ ',' + (short) sensorData.maps
			+ ',' + (short) sensorData.temp
			+ ',' + (short) sensorData.batt
			/* Motor Signals */
			+ ',' + (short) motorSignals.front
			+ ',' + (short) motorSignals.right
			+ ',' + (short) motorSignals.rear
			+ ',' + (short) motorSignals.left
			/* Motor Offsets */
			+ ',' + (short) motorOffsets.roll
			+ ',' + (short) motorOffsets.pitch
			+ ',' + (short) motorOffsets.yaw
			+ ',' + (short) motorOffsets.z
			/* Trace Data */
			+ ',' + (short) traceData.value_1
			+ ',' + (short) traceData.value_2
			+ ',' + (short) traceData.value_3
			+ ',' + (short) traceData.value_4
			+ ',' + (int)( (traceData.value_5  << 16) | (traceData.value_6  & 0xFFFF) )
			+ ',' + (int)( (traceData.value_7  << 16) | (traceData.value_8  & 0xFFFF) )
			+ ',' + (int)( (traceData.value_9  << 16) | (traceData.value_10 & 0xFFFF) )
			+ ',' + (int)( (traceData.value_11 << 16) | (traceData.value_12 & 0xFFFF) )
			+ ',' + (int)( (traceData.value_13 << 16) | (traceData.value_14 & 0xFFFF) )
			+ ',' + (int)( (traceData.value_15 << 16) | (traceData.value_16 & 0xFFFF) )
			+ '\n';

			try
			{
				logFile.write( csv );
			}
			catch( Exception e )
			{
				System.err.println( "ControlTerminal.writeLogData: " + e.getMessage( ) );
			}
        }
    }

    public void resetMeterNeedles( )
    {
        if( connected )
        {
            if( showPosition[0] )
            {
            	positionDialog.setDesired( positionDialog.getCurrent( ) );
                positionDialog.clearWindow( true, true, true );
            }
            else
            {
            	meterRoll  .setDesired( 0 );
            	meterPitch .setDesired( 0 );
            }

            meterYaw.setDesired( meterYaw.getCurrent( ) );
        }
        else
        {
            if( showPosition[0] )
            {
            	Point zero = new Point( 0, 0 );
            	positionDialog.setDesired( zero );
            	positionDialog.setCurrent( zero );
                positionDialog.clearWindow( true, true, true );
            }

            meterRoll     .setDesired( 0 );
            meterPitch    .setDesired( 0 );
            meterYaw      .setDesired( 0 );
            meterAltitude .setDesired( 0 );

            meterRoll     .setCurrent( 0 );
            meterPitch    .setCurrent( 0 );
            meterYaw      .setCurrent( 0 );
            meterAltitude .setCurrent( 0 );

            digitalMeter  .resetMotorMeter( );
/*
            if( SHOW_3DWINDOW && visualization != null )
            {
                visualization.resetModel( );
            }
*/
        }
    }

    public void setDefaultLimits( )
    {
        meterRoll     .setMaximum( AnalogMeter.DEG_45 );
        meterPitch    .setMaximum( AnalogMeter.DEG_45 );
        meterAltitude .setMaximum( AnalogMeter.DEG_135 );
    }

    public boolean loadUserLimits( )
    {
        int[] values = null;
        int   offset = 3, i;

        try
        {
            ObjectInputStream stream = new ObjectInputStream(
                new FileInputStream( getFileName( EXT_CFG ) ) );
            values = (int[]) stream.readObject( );
            stream.close( );
        }
        catch( Exception e )
        {
            return( false );
        }

        if( values.length < offset + controlParams.length )
        {
            meterRoll     .setMaximum( 0 );
            meterPitch    .setMaximum( 0 );
            meterAltitude .setMaximum( 0 );

            for( i = 0; i < controlParams.length; ++i )
            {
                controlParams[i] = 0;
            }
        }
        else
        {
        	meterRoll     .setMaximum( values[0] );
            meterPitch    .setMaximum( values[1] );
            meterAltitude .setMaximum( values[2] );

            for( i = 0; i < controlParams.length; ++i )
            {
                controlParams[i] = (short) values[ offset + i ];
            }
        }

        return( true );
    }

    public boolean saveUserLimits( )
    {
        int[] values = {
            meterRoll     .getMaximum( ),
		    meterPitch    .getMaximum( ),
		    meterAltitude .getMaximum( ),
		    controlParams[0],  controlParams[1],  controlParams[2],  controlParams[3],
		    controlParams[4],  controlParams[5],  controlParams[6],  controlParams[7],
		    controlParams[8],  controlParams[9],  controlParams[10], controlParams[11],
		    controlParams[12], controlParams[13], controlParams[14], controlParams[15]
		};

        try
        {
            ObjectOutputStream stream = new ObjectOutputStream(
                new FileOutputStream( getFileName( EXT_CFG ) ) );
            stream.writeObject( values );
            stream.close( );
        }
        catch( Exception e )
        {
            return( false );
        }

        return( true );
    }

    public synchronized void checkJAviatorSettled( MotorSignals motors )
    {
    	if( motors.front == 0 &&
    		motors.right == 0 &&
    		motors.rear  == 0 &&
    		motors.left  == 0 )
        {
            notifyAll( );
        }
    }

    /*************************************************************************/
    /*                                                                       */
    /* Protected Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    protected static final String NIL              = "";
    protected static final String ZERO             = "0";
    protected static final String EXT_CFG          = "cfg";
    protected static final String ROLL             = "Roll";
    protected static final String PITCH            = "Pitch";
    protected static final String YAW              = "Yaw";
    protected static final String ALTITUDE         = "Altitude";
    protected static final String THRUST           = "Thrust";
    protected static final String JOYSTICK         = "Joystick";
    protected static final String KEYBOARD         = "Keyboard";
    protected static final String _MODE            = " Mode";
    protected static final String SWITCH           = "Switch";
    protected static final String _HELI            = " Heli";
    protected static final String _STATE           = " State";
    protected static final String TCP_             = "TCP ";
    protected static final String UDP_             = "UDP ";
    protected static final String CONNECTION       = "Connection";
    protected static final String CONNECT_TO       = "Connect to";
    protected static final String DISCONNECT       = "Disconnect";
    protected static final String SHUT_DOWN        = "Shut Down";
    protected static final String LOGGING          = "Logging";
    protected static final String TOGGLE_          = "Toggle ";
    protected static final String _FAILED          = " failed";
    protected static final String _DATA            = " Data";
    protected static final String PORT_SETTINGS    = "Port Settings";
    protected static final String SET_             = "Set ";
    protected static final String PARAMETERS       = "Parameters";
    protected static final String RESET_NEEDLES    = "Reset Needles";
    protected static final String KEY_ASSISTANCE   = "Key Assistance";
    protected static final String ABOUT_TERMINAL   = "About Terminal";
    protected static final String DIAGRAMS         = "Diagrams";
    protected static final String POS              = "POS";

    protected AnalogMeter         meterRoll        = null;
    protected AnalogMeter         meterPitch       = null;
    protected AnalogMeter         meterYaw         = null;
    protected AnalogMeter         meterAltitude    = null;
    protected DigitalMeter        digitalMeter     = null;
    protected HiddenButton        toggleControl    = null;
    protected HiddenButton        toggleConnMode   = null;
    protected HiddenButton        toggleConnect    = null;
    protected HiddenButton        switchHeliState  = null;
    protected HiddenButton        shutDownHeli     = null;
    protected HiddenButton        setPortAndHost   = null;
    protected HiddenButton        setParameters    = null;
    protected HiddenButton        resetNeedles     = null;
    protected HiddenButton        showKeyAssist    = null;
    protected HiddenButton        showAboutInfo    = null;
    protected FileWriter          logFile          = null;
    protected String              relayHost        = "192.168.2.3";
    protected int                 relayPort        = 7000;
    protected int                 motionDelay      = 10;
    protected boolean             stickControl     = false;
    protected boolean             udpConnect       = true;
    protected boolean             connected        = false;

    protected void finalize( )
    {
        motionThread.terminate( );
    }

    protected void processWindowEvent( WindowEvent we )
    {
        if( we.getID( ) == WindowEvent.WINDOW_CLOSING )
        {
            System.out.print( "Saving user limits ... " );

            if( !saveUserLimits( ) )
            {
                System.out.println( "failed" );
            }
            else
            {
                System.out.println( "ok" );
            }

            dispose( );
            System.exit( 0 );
        }
    }

    protected void processFocusEvent( FocusEvent fe )
    {
        motionThread.setOffsetMaximumRollPitch( 0 );
        motionThread.setOffsetMaximumAltitude ( 0 );
        motionThread.setOffsetDesiredRoll     ( 0 );
        motionThread.setOffsetDesiredPitch    ( 0 );
        motionThread.setOffsetDesiredYaw      ( 0 );
        motionThread.setOffsetDesiredAltitude ( 0 );

        meterRoll     .processFocusEvent( fe );
        meterPitch    .processFocusEvent( fe );
        meterYaw      .processFocusEvent( fe );
        meterAltitude .processFocusEvent( fe );
    }

    protected void processKeyEvent( KeyEvent ke )
    {
        switch( ke.getID( ) )
        {
            case KeyEvent.KEY_PRESSED:
                processKeyPressed( ke.getKeyCode( ) );
                break;

            case KeyEvent.KEY_RELEASED:
                processKeyReleased( ke.getKeyCode( ) );
                break;

            default:
                return;
        }
    }

    protected void processKeyPressed( int keyCode )
    {
        switch( keyCode )
        {
            /* command keys assigned to the right hand */
            case KeyEvent.VK_LEFT: /* roll left */
                motionThread.setOffsetDesiredRoll( -MOTION_STEP );
                break;

            case KeyEvent.VK_RIGHT: /* roll right */
                motionThread.setOffsetDesiredRoll( MOTION_STEP );
                break;

            case KeyEvent.VK_UP: /* pitch forward */
                motionThread.setOffsetDesiredPitch( -MOTION_STEP );
                break;

            case KeyEvent.VK_DOWN: /* pitch backward */
                motionThread.setOffsetDesiredPitch( MOTION_STEP );
                break;

            /* command keys assigned to the left hand */
            case KeyEvent.VK_A: /* yaw left */
                motionThread.setOffsetDesiredYaw( -MOTION_STEP );
                break;

            case KeyEvent.VK_D: /* yaw right */
                motionThread.setOffsetDesiredYaw( MOTION_STEP );
                break;

            case KeyEvent.VK_S: /* descend */
                motionThread.setOffsetDesiredAltitude( -MOTION_STEP );
                break;

            case KeyEvent.VK_W: /* ascend */
                motionThread.setOffsetDesiredAltitude( MOTION_STEP );
                break;

            /* command keys for different purposes */
            case KeyEvent.VK_M:
            	doSwitchHeliMode( );
                break;

            case KeyEvent.VK_P:
                doPlotPositionData( );
                break;

            case KeyEvent.VK_F1:
                if( showKeyAssist.isEnabled( ) )
                {
                    doShowKeyAssist( );
                }
                break;

            case KeyEvent.VK_F2:
                if( showAboutInfo.isEnabled( ) )
                {
                    doShowAboutInfo( );
                }
                break;
            case KeyEvent.VK_F3:
                //doToggleDiagram( );
            	doDisplayPosition( );
                break;

            case KeyEvent.VK_F4:
                doToggleLogData( );
                break;

            case KeyEvent.VK_F5:
                if( toggleControl.isEnabled( ) )
                {
                    doToggleControl( );
                }
                break;

            case KeyEvent.VK_F6:
                if( toggleConnMode.isEnabled( ) )
                {
                    doToggleConnMode( );
                }
                break;

            case KeyEvent.VK_F7:
                if( toggleConnect.isEnabled( ) )
                {
                    doToggleConnect( );
                }
                break;

            case KeyEvent.VK_F8:
                if( switchHeliState.isEnabled( ) )
                {
                    doSwitchHeliState( );
                }
                break;

            case KeyEvent.VK_F9:
                if( shutDownHeli.isEnabled( ) )
                {
                    doShutDownHeli( );
                }
                break;

            case KeyEvent.VK_F10:
                if( setPortAndHost.isEnabled( ) )
                {
                    doSetPortAndHost( );
                }
                break;

            case KeyEvent.VK_F11:
                if( setParameters.isEnabled( ) )
                {
                    doSetParameters( );
                }
                break;

            case KeyEvent.VK_F12:
                if( resetNeedles.isEnabled( ) )
                {
                    doResetNeedles( );
                }
                break;

            default:
                return;
        }
    }

    protected void processKeyReleased( int keyCode )
    {
        switch( keyCode )
        {
            case KeyEvent.VK_LEFT:
            case KeyEvent.VK_RIGHT:
                motionThread.setOffsetDesiredRoll( 0 );
                break;

            case KeyEvent.VK_UP:
            case KeyEvent.VK_DOWN:
                motionThread.setOffsetDesiredPitch( 0 );
                break;

            case KeyEvent.VK_A:
            case KeyEvent.VK_D:
                motionThread.setOffsetDesiredYaw( 0 );
                break;

            case KeyEvent.VK_S:
            case KeyEvent.VK_W:
                motionThread.setOffsetDesiredAltitude( 0 );
                break;

            default:
                return;
        }
    }

    protected String getFileName( String extension )
    {
        String className = getClass( ).getName( );
        return( className.substring( className.lastIndexOf( '.' ) + 1 ) + '.' + extension );
    }

    protected void setConnected( boolean connected )
    {
        if( this.connected = connected )
        {
        	newIdlingSpeed = true;
            new_R_P_Params = true;
            new_Yaw_Params = true;
            new_Alt_Params = true;
            new_X_Y_Params = true;
        }

        toggleConnMode  .setEnabled( !connected );
        switchHeliState .setEnabled(  connected );
        shutDownHeli    .setEnabled(  connected );
        setPortAndHost  .setEnabled( !connected );
    }

    /*************************************************************************/
    /*                                                                       */
    /* Private Section                                                       */
    /*                                                                       */
    /*************************************************************************/

    private static final int       MOTION_STEP    = 5; /* mrad/mm */
    private static final int       CTRL_PARAMS    = 16;

    private Image                  iconImage      = null;
    //private JAviator3DControlPanel visualization  = null;
    //private SignalsDialog          signalsDialog  = null;
    private PositionDialog         positionDialog = null;
    private MotionThread           motionThread   = null;
    private Thread                 remoteThread   = null;
    private Transceiver            remote         = null;
    private Joystick               stick          = null;
    private Label                  logDataLabel   = null;
    private short[]                controlParams  = null;
    private int[]                  changedParamID = null;
    private int                    plotIndex      = 0;
    private boolean[]              showPosition   = null;
    private boolean                newIdlingSpeed = false;
    private boolean                new_R_P_Params = false;
    private boolean                new_Yaw_Params = false;
    private boolean                new_Alt_Params = false;
    private boolean                new_X_Y_Params = false;
    private boolean                logData        = false;
    //private boolean                showDiagrams   = false;

    private void initWindow( )
    {
        motionThread   = new MotionThread( );
        controlParams  = new short[ CTRL_PARAMS ];
        changedParamID = new int[1];
        showPosition   = new boolean[1];

        for( int i = 0; i < controlParams.length; ++i )
        {
            controlParams[i] = 0;
        }

        changedParamID[0] = -1;
        showPosition[0]   = false;

        setTitle( "JAviator Control Terminal" );
        setBackground( UIManagerColor.getButtonBackground( ) );
        setForeground( UIManagerColor.getButtonForeground( ) );
        setLayout( new BorderLayout( ) );
        add( makeNorthPanel( ), BorderLayout.NORTH );
        add( makeCenterPanel( ), BorderLayout.CENTER );
        add( makeSouthPanel( ), BorderLayout.SOUTH );
        setConnected( false );
        pack( );

        System.out.print( "Loading user limits ... " );

        if( !loadUserLimits( ) )
        {
            System.out.println( "failed" );
            setDefaultLimits( );
            System.out.println( "Default limits have been set" );
        }
        else
        {
            System.out.println( "ok" );
        }

        enableEvents( AWTEvent.WINDOW_EVENT_MASK |
        		      AWTEvent.FOCUS_EVENT_MASK |
        		      AWTEvent.KEY_EVENT_MASK );
        setLocation( 0, 0 );
        setVisible( true );
/*
        if( SHOW_3DWINDOW )
        {
            visualization = new JAviator3DControlPanel( );
            visualization.createModel( );
        }        
*/
        try
        {
            iconImage = AnalogMeter.getImage( "pilot_icon.png" );
        }
        catch( Exception e )
        {
            System.err.println( "ControlTerminal.initWindow: " + e.getMessage( ) );
            return;
        }

        MediaTracker tracker = new MediaTracker( this );
        tracker.addImage( iconImage, 0 );

        try
        {
            tracker.waitForID( 0 );

            if( tracker.isErrorID( 0 ) )
            {
                System.err.println( "ControlTerminal.initWindow: tracker.isErrorID" );
                return;
            }
        }
        catch( Exception e )
        {
            System.err.println( "ControlTerminal.initWindow: " + e.getMessage( ) );
            return;
        }

        setIconImage( iconImage );
        motionThread.start( );
    }

    private Panel makeCenterPanel( )
    {
        meterRoll     = new AnalogMeter( SMALL_METER, SMALL_FONT, AnalogMeter.TYPE_ROLL );
        meterPitch    = new AnalogMeter( SMALL_METER, SMALL_FONT, AnalogMeter.TYPE_PITCH );
        meterYaw      = new AnalogMeter( SMALL_METER, SMALL_FONT, AnalogMeter.TYPE_YAW );
        meterAltitude = new AnalogMeter( SMALL_METER, SMALL_FONT, AnalogMeter.TYPE_ALTITUDE );
        digitalMeter  = new DigitalMeter( this );
        logDataLabel  = new Label( LOGGING + _DATA, Label.CENTER );
        logDataLabel.setForeground( Color.LIGHT_GRAY );

        HiddenButton incAltitudeLimit = new HiddenButton( HiddenButton.SYMBOL_PLUS );
        incAltitudeLimit.setForeground( Color.GRAY );
        incAltitudeLimit.addMouseListener( new MouseAdapter( )
        {
            public void mousePressed( MouseEvent me )
            {
                motionThread.setOffsetMaximumAltitude( MOTION_STEP );
            }

            public void mouseReleased( MouseEvent me )
            {
                motionThread.setOffsetMaximumAltitude( 0 );
            }
        } );

        HiddenButton decAltitudeLimit = new HiddenButton( HiddenButton.SYMBOL_MINUS );
        decAltitudeLimit.setForeground( Color.GRAY );
        decAltitudeLimit.addMouseListener( new MouseAdapter( )
        {
            public void mousePressed( MouseEvent me )
            {
                motionThread.setOffsetMaximumAltitude( -MOTION_STEP );
            }

            public void mouseReleased( MouseEvent me )
            {
                motionThread.setOffsetMaximumAltitude( 0 );
            }
        } );

        HiddenButton incRollPitchLimit = new HiddenButton( HiddenButton.SYMBOL_PLUS );
        incRollPitchLimit.setForeground( Color.GRAY );
        incRollPitchLimit.addMouseListener( new MouseAdapter( )
        {
            public void mousePressed( MouseEvent me )
            {
                motionThread.setOffsetMaximumRollPitch( MOTION_STEP );
            }

            public void mouseReleased( MouseEvent me )
            {
                motionThread.setOffsetMaximumRollPitch( 0 );
            }
        } );

        HiddenButton decRollPitchLimit = new HiddenButton( HiddenButton.SYMBOL_MINUS );
        decRollPitchLimit.setForeground( Color.GRAY );
        decRollPitchLimit.addMouseListener( new MouseAdapter( )
        {
            public void mousePressed( MouseEvent me )
            {
                motionThread.setOffsetMaximumRollPitch( -MOTION_STEP );
            }

            public void mouseReleased( MouseEvent me )
            {
                motionThread.setOffsetMaximumRollPitch( 0 );
            }
        } );

        Panel altitudeLimitButtons = new Panel( new GridLayout( 1, 6 ) );
        altitudeLimitButtons.add( new Label( ) );
        altitudeLimitButtons.add( new Label( ) );
        altitudeLimitButtons.add( decAltitudeLimit );
        altitudeLimitButtons.add( incAltitudeLimit );
        altitudeLimitButtons.add( new Label( ) );
        altitudeLimitButtons.add( new Label( ) );

        Panel rollPitchLimitButtons = new Panel( new GridLayout( 1, 6 ) );
        rollPitchLimitButtons.add( new Label( ) );
        rollPitchLimitButtons.add( new Label( ) );
        rollPitchLimitButtons.add( decRollPitchLimit );
        rollPitchLimitButtons.add( incRollPitchLimit );
        rollPitchLimitButtons.add( new Label( ) );
        rollPitchLimitButtons.add( new Label( ) );

        Panel yawAltitudePanel = new Panel( new BorderLayout( ) );
        yawAltitudePanel.add( meterYaw, BorderLayout.NORTH );
        yawAltitudePanel.add( meterAltitude, BorderLayout.CENTER );
        yawAltitudePanel.add( altitudeLimitButtons, BorderLayout.SOUTH );

        Panel rollPitchPanel = new Panel( new BorderLayout( ) );
        rollPitchPanel.add( meterRoll, BorderLayout.NORTH );
        rollPitchPanel.add( meterPitch, BorderLayout.CENTER );
        rollPitchPanel.add( rollPitchLimitButtons, BorderLayout.SOUTH );

        Panel motorMeterPanel = new Panel( new BorderLayout( ) );
        motorMeterPanel.add( digitalMeter, BorderLayout.CENTER );
        motorMeterPanel.add( logDataLabel, BorderLayout.SOUTH );

        Panel centerPanel = new Panel( new BorderLayout( ) );
        centerPanel.add( yawAltitudePanel, BorderLayout.WEST );
        centerPanel.add( motorMeterPanel, BorderLayout.CENTER );
        centerPanel.add( rollPitchPanel, BorderLayout.EAST );

        return( centerPanel );
    }

    private Panel makeNorthPanel( )
    {
        toggleControl = new HiddenButton( JOYSTICK + _MODE );
        toggleControl.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doToggleControl( );
            }
        } );

        toggleConnMode = new HiddenButton( TCP_ + CONNECTION );
        toggleConnMode.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doToggleConnMode( );
            }
        } );

        toggleConnect = new HiddenButton( CONNECT_TO + _HELI );
        toggleConnect.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doToggleConnect( );
            }
        } );

        switchHeliState = new HiddenButton( SWITCH + _HELI + _STATE );
        switchHeliState.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doSwitchHeliState( );
            }
        } );

        shutDownHeli = new HiddenButton( SHUT_DOWN + _HELI );
        shutDownHeli.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doShutDownHeli( );
            }
        } );

        Panel buttonPanel = new Panel( new GridLayout( 1, 5 ) );
        buttonPanel.add( toggleControl );
        buttonPanel.add( toggleConnMode );
        buttonPanel.add( toggleConnect );
        buttonPanel.add( switchHeliState );
        buttonPanel.add( shutDownHeli );

        Panel northPanel = new Panel( new BorderLayout( ) );
        northPanel.add( buttonPanel, BorderLayout.NORTH );
        northPanel.add( new Label( ), BorderLayout.SOUTH );

        return( northPanel );
    }

    private Panel makeSouthPanel( )
    {
        setPortAndHost = new HiddenButton( PORT_SETTINGS );
        setPortAndHost.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doSetPortAndHost( );
            }
        } );

        setParameters = new HiddenButton( SET_ + PARAMETERS );
        setParameters.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doSetParameters( );
            }
        } );

        resetNeedles = new HiddenButton( RESET_NEEDLES );
        resetNeedles.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doResetNeedles( );
            }
        } );

        showKeyAssist = new HiddenButton( KEY_ASSISTANCE );
        showKeyAssist.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doShowKeyAssist( );
            }
        } );

        showAboutInfo = new HiddenButton( ABOUT_TERMINAL );
        showAboutInfo.addMouseListener( new MouseAdapter( )
        {
            public void mouseClicked( MouseEvent me )
            {
                doShowAboutInfo( );
            }
        } );

        Panel buttonPanel = new Panel( new GridLayout( 1, 5 ) );
        buttonPanel.add( setPortAndHost );
        buttonPanel.add( setParameters );
        buttonPanel.add( resetNeedles );
        buttonPanel.add( showKeyAssist );
        buttonPanel.add( showAboutInfo );

        Panel southPanel = new Panel( new BorderLayout( ) );
        southPanel.add( new Label( ), BorderLayout.NORTH );
        southPanel.add( buttonPanel, BorderLayout.SOUTH );

        return( southPanel );
    }

    private void doToggleControl( )
    {
        if( !stickControl )
        {
            try
            {
                stick = Joystick.createInstance( );
                stick.setPollInterval( motionDelay );
                stick.setDeadZone( 0.0f );
            }
            catch( Exception e )
            {
                toggleControl.setForeground( Color.RED );
                toggleControl.setText( JOYSTICK + _FAILED );
                System.err.println( "ControlTerminal.doToggleControl: " + e.getMessage( ) );
                return;
            }

            stickControl = true;
            toggleControl.setForeground( UIManagerColor.getButtonForeground( ) );
            toggleControl.setText( KEYBOARD + _MODE );
        }
        else
        {
            stickControl = false;
            toggleControl.setForeground( UIManagerColor.getButtonForeground( ) );
            toggleControl.setText( JOYSTICK + _MODE );
        }
    }

    private void doToggleConnMode( )
    {
        if( ( udpConnect = !udpConnect ) )
        {
            toggleConnMode.setText( TCP_ + CONNECTION );
        }
        else
        {
            toggleConnMode.setText( UDP_ + CONNECTION );
        }
    }

    private void doToggleConnect( )
    {
        if( !connected )
        {
            resetMeterNeedles( );
            remote = Transceiver.createInstance( this,
                udpConnect ? Transceiver.UDP_CONNECT : Transceiver.TCP_CONNECT );
            remote.connect( );

            if( remote.isConnected( ) )
            {
                remoteThread = new Thread( remote );
                remoteThread.start( );
            }
        }
        else
        {
            remote.terminate( );

            try
            {
                remoteThread.join( );
            }
            catch( Exception e )
            {
                System.err.println( "ControlTerminal.doToggleConnect: " + e.getMessage( ) );
            }

            remote       = null;
            remoteThread = null;
        }
    }

    private Packet switchStatePacket = new Packet( PacketType.COMM_SWITCH_STATE, null );
    private void doSwitchHeliState( )
    {
    	if( remote != null )
    	{
            if( digitalMeter.getHeliState( ) == ControllerConstants.HELI_STATE_GROUND )
            {
            	resetMeterNeedles( );
            }

            remote.sendPacket( switchStatePacket );
    	}
    }

    private Packet shutDownPacket = new Packet( PacketType.COMM_SHUT_DOWN, null );
    private void doShutDownHeli( )
    {
    	if( remote != null )
    	{
    		remote.sendPacket( shutDownPacket );
    	}
    }

	private void doSetPortAndHost( )
    {
        PortDialog.createInstance( this, PORT_SETTINGS );
    }

    private void doSetParameters( )
    {
        ParamDialog.createInstance( this, PARAMETERS, controlParams, changedParamID );
    }

    private void doResetNeedles( )
    {
        resetMeterNeedles( );
    }

    private void doShowKeyAssist( )
    {
        InfoDialog.createInstance( this, KEY_ASSISTANCE, InfoDialog.TYPE_KEY_ASSISTANCE );
    }

    private void doShowAboutInfo( )
    {
        InfoDialog.createInstance( this, ABOUT_TERMINAL, InfoDialog.TYPE_ABOUT_TERMINAL );
    }
/*
    private void doToggleDiagram( )
    {
    	if( (showDiagrams = !showDiagrams) )
    	{
    		if( signalsDialog == null )
    		{
    			signalsDialog = new SignalsDialog( );
    		}
    		
    		signalsDialog.open( );
    	}
    }
*/
    private void doDisplayPosition( )
    {
        positionDialog = PositionDialog.createInstance( this, "Helicopter Position",
            UBI_RECT, MAX_RECT, showPosition );
    }

    private void doToggleLogData( )
    {
        if( (logData = !logData) )
        {
	        try
	        {
	            logFile = new FileWriter( LOG_FILE_NAME, true );
	            logFile.write( LOG_TITLE_STR + "\n" );
	            logDataLabel.setForeground( Color.RED );
	        }
	        catch( Exception e )
	        {
	            System.err.println( "ControlTerminal.doToggleLogData: " + e.getMessage( ) );
	        }
        }
        else
        {
	        if( logFile != null )
	        {
		        try
		        {
		        	logFile.close( );
		        	logDataLabel.setForeground( Color.LIGHT_GRAY );
		        }
		        catch( Exception e )
		        {
		            System.err.println( "ControlTerminal.doToggleLogData: " + e.getMessage( ) );
		        }
		        
		        logFile = null;
	        }
        }
    }

    private Packet switchModePacket = new Packet( PacketType.COMM_SWITCH_MODE, null );
    private void doSwitchHeliMode( )
    {
    	if( remote != null )
    	{
            remote.sendPacket( switchModePacket );
    	}
    }

    private Packet storeTrimPacket = new Packet( PacketType.COMM_STORE_TRIM, null );
    private void doStoreTrimValues( )
    {
    	if( remote != null )
    	{
            remote.sendPacket( storeTrimPacket );
    	}
    }

    private Packet clearTrimPacket = new Packet( PacketType.COMM_CLEAR_TRIM, null );
    private void doClearTrimValues( )
    {
    	if( remote != null )
    	{
            remote.sendPacket( clearTrimPacket );
    	}
    }

    private void doPlotPositionData( )
    {
    	if( positionDialog == null )
    	{
    		return;
    	}

        String str = new String( );

        try
        {
            BufferedReader buf = new BufferedReader( new FileReader(
                LOG_FILE_PATH + PLOT_LIST[ plotIndex ] ) );
            int ch;

            while( (ch = buf.read( ) ) != -1 )
            {
            	str += (char) ch;
            }
        }
        catch( IOException e )
        {
        	System.err.println( "ControlTerminal.doPlotPositionData: " + e.getMessage( ) );
            return;
        }

        if( ++plotIndex >= PLOT_LIST.length )
        {
        	plotIndex = 0;
        }

        int p = 0;
        int c = 0;
        int q = str.indexOf( '\n' );
        int eof = str.lastIndexOf( '\n' );

        positionDialog.clearWindow( true, false, false );

        while( q < eof )
        {
	        String substr = str.substring( p, q + 1 );
	        c = substr.indexOf( ',' );
	        positionDialog.setCurrent( new Point( Integer.parseInt( substr.substring( 0, c ) ),
                Integer.parseInt( substr.substring( c + 1, substr.indexOf( '\n' ) ) ) ) );
	        p = q + 1;
	        q = str.indexOf( '\n', p );
        }
    }

    /*************************************************************************/
    /*                                                                       */
    /* Class MotionThread                                                    */
    /*                                                                       */
    /*************************************************************************/

    private class MotionThread extends Thread
    {
        public MotionThread( )
        {
        }

        public void run( )
        {
            while( true )
            {
                synchronized( this )
                {
                    if( halt )
                    {
                        break;
                    }
                }

                if( stickControl )
                {
                    processJoystick( );
                }
                else
                {
                    processKeyboard( );
                }

                /* animate the three maximum-value needles */
                if( offsetMaximumRollPitch != 0 )
                {
                    meterPitch .setMaximum( meterPitch .getMaximum( ) + offsetMaximumRollPitch );
                	meterRoll  .setMaximum( meterRoll  .getMaximum( ) + offsetMaximumRollPitch );
                }

                if( offsetMaximumAltitude != 0 )
                {
                    meterAltitude.setMaximum( meterAltitude.getMaximum( ) + offsetMaximumAltitude );
                }

                try
                {
                    sleep( motionDelay );
                }
                catch( InterruptedException e )
                {
                    System.err.println( "MotionThread.run: " + e.getMessage( ) );
                    break;
                }
            }
        }

        public void setOffsetMaximumRollPitch( int offsetMaximumRollPitch )
        {
            this.offsetMaximumRollPitch = offsetMaximumRollPitch;
        }

        public void setOffsetMaximumAltitude( int offsetMaximumAltitude )
        {
            this.offsetMaximumAltitude = offsetMaximumAltitude;
        }

        public void setOffsetDesiredRoll( int offsetDesiredRoll )
        {
            this.offsetDesiredRoll = offsetDesiredRoll;
        }

        public void setOffsetDesiredPitch( int offsetDesiredPitch )
        {
            this.offsetDesiredPitch = offsetDesiredPitch;
        }

        public void setOffsetDesiredYaw( int offsetDesiredYaw )
        {
            this.offsetDesiredYaw = offsetDesiredYaw;
        }

        public void setOffsetDesiredAltitude( int offsetDesiredAltitude )
        {
            this.offsetDesiredAltitude = offsetDesiredAltitude;
        }

        public void terminate( )
        {
            synchronized( this )
            {
                halt = true;
            }

            try
            {
                join( );
            }
            catch( Exception e )
            {
                System.err.println( "MotionThread.terminate: " + e.getMessage( ) );
            }
        }

        /*********************************************************************/
        /*                                                                   */
        /* Private Section                                                   */
        /*                                                                   */
        /*********************************************************************/

        private int     offsetMaximumRollPitch = 0;
        private int     offsetMaximumAltitude  = 0;
        private int     offsetDesiredRoll      = 0;
        private int     offsetDesiredPitch     = 0;
        private int     offsetDesiredYaw       = 0;
        private int     offsetDesiredAltitude  = 0;
        private int     buttonsPressed         = 0;
        private int     buttonNotPressed       = 0;
        private boolean alreadyToggledButton3  = false;
        private boolean alreadyToggledButton4  = false;
        private boolean alreadyToggledButton5  = false;
        private boolean alreadyToggledButton6  = false;
        private boolean alreadyToggledButton9  = false;
        private boolean alreadyToggledButton10 = false;
        private boolean halt                   = false;

        private int correct_yaw( int yaw )
        {
            if( yaw < -AnalogMeter.DEG_180 )
	        {
                yaw += AnalogMeter.DEG_360;
            }
            else
            if( yaw > AnalogMeter.DEG_180 )
	        {
                yaw -= AnalogMeter.DEG_360;
            }

            return( yaw );
        }

        private void processJoystick( )
        {
            stick.poll( );
            buttonsPressed = stick.getButtons( );

			if( !stick.isButtonDown( Joystick.BUTTON1 ) ) 
            {
				if( ++buttonNotPressed > 5 && (digitalMeter.getHeliState( ) &
                    ControllerConstants.HELI_STATE_SHUTDOWN) == 0 )
				{
					doShutDownHeli( );
				}
			}
            else
            if( (buttonsPressed & Joystick.BUTTON10) != 0 )
            {
				/* toggle the heli-state button */
				if( !alreadyToggledButton10 )
                {
                    alreadyToggledButton10 = true;

					if( switchHeliState.isEnabled( ) )
                    {
						doSwitchHeliState( );
					}
				}

				buttonNotPressed = 0;
			}
            else
            {
            	buttonNotPressed = 0;
                alreadyToggledButton10 = false;
            }

            /* toggle the heli-mode button */
            if( (buttonsPressed & Joystick.BUTTON3) != 0 )
            {
				if( !alreadyToggledButton3 )
                {
                    alreadyToggledButton3 = true;
                    doSwitchHeliMode( );
                }
            }
            else
            {
                alreadyToggledButton3 = false;
            }

            /* toggle the heli-mode button */
            if( (buttonsPressed & Joystick.BUTTON4) != 0 )
            {
				if( !alreadyToggledButton4 )
                {
                    alreadyToggledButton4 = true;
                    doSwitchHeliMode( );
                }
            }
            else
            {
                alreadyToggledButton4 = false;
            }

            /* send a store-trim-values command */
            if( (buttonsPressed & Joystick.BUTTON5) != 0 )
            {
				if( !alreadyToggledButton5 )
                {
                    alreadyToggledButton5 = true;
            	    doStoreTrimValues( );
                }
            }
            else
            {
                alreadyToggledButton5 = false;
            }

            /* send a clear-trim-values command */
            if( (buttonsPressed & Joystick.BUTTON6) != 0 )
            {
				if( !alreadyToggledButton6 )
                {
                    alreadyToggledButton6 = true;
            	    doClearTrimValues( );
                }
            }
            else
            {
                alreadyToggledButton6 = false;
            }

            /* issue a fast-yaw-left command */
            if( (buttonsPressed & Joystick.BUTTON7) != 0 )
            {
				meterYaw.setDesired( correct_yaw( meterYaw.getDesired( ) - (MOTION_STEP << 1) ) );
            }

            /* issue a fast-yaw-right command */
            if( (buttonsPressed & Joystick.BUTTON8) != 0 )
            {
                meterYaw.setDesired( correct_yaw( meterYaw.getDesired( ) + (MOTION_STEP << 1) ) );
            }

            /* reset the green meter needles */
            if( (buttonsPressed & Joystick.BUTTON9) != 0 )
            {
				if( !alreadyToggledButton9 )
                {
                    alreadyToggledButton9 = true;
                    resetMeterNeedles( );
                }
            }
            else
            {
                alreadyToggledButton9 = false;
            }

            /* update the four desired-value needles */
            if( showPosition[0] )
            {
            	Point desired = positionDialog.getDesired( );
            	positionDialog.setDesired( new Point(
                    desired.x + (int)( stick.getX( ) * 10.0f ),
                    desired.y - (int)( stick.getY( ) * 10.0f ) ) );
            }
            else
            {
            	meterRoll  .setDesired( (int)( meterRoll  .getMaximum( ) * stick.getX( ) ) );
            	meterPitch .setDesired( (int)( meterPitch .getMaximum( ) * stick.getY( ) ) );
            }

            if( (buttonsPressed & Joystick.BUTTON2) != 0 )
            {
	            meterYaw.setDesired( correct_yaw( meterYaw.getDesired( ) +
                    (int)( stick.getZ( ) * 10.0f ) ) );
            }

            meterAltitude.setDesired( (int)( meterAltitude.getMaximum( ) *
                ( stick.getR( ) - 1.0f ) * -0.5f ) );
        }

        private void processKeyboard( )
        {
            /* animate the four desired-value needles */
            if( showPosition[0] )
            {
	            if( offsetDesiredRoll != 0 || offsetDesiredPitch != 0 )
	            {
	            	Point desired = positionDialog.getDesired( );
            	    positionDialog.setDesired( new Point(
                        desired.x + offsetDesiredRoll,
                        desired.y - offsetDesiredPitch ) );
	            }
            }
            else
            {
	            if( offsetDesiredRoll != 0 )
		        {
		            meterRoll.setDesired( meterRoll.getDesired( ) + offsetDesiredRoll );
		        }
	
		        if( offsetDesiredPitch != 0 )
		        {
		            meterPitch.setDesired( meterPitch.getDesired( ) + offsetDesiredPitch );
	            }
            }

	        if( offsetDesiredYaw != 0 )
	        {
                meterYaw.setDesired( correct_yaw( meterYaw.getDesired( ) + offsetDesiredYaw ) );
	        }

	        if( offsetDesiredAltitude != 0 )
	        {
	            meterAltitude.setDesired( meterAltitude.getDesired( ) + offsetDesiredAltitude );
            }
        }
    }
}

/* End of file */