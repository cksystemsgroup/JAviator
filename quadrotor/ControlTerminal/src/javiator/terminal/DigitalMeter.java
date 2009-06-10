/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   DigitalMeter.java  Constructs a digital meter representing altitude     */
/*                      mode, control state, motor signals and offsets.      */
/*                                                                           */
/*   Copyright (c) 2006-2009  Rainer Trummer                                 */
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

import java.awt.Panel;
import java.awt.Label;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.Color;

import java.awt.event.FocusEvent;

import javiator.util.MotorSignals;
import javiator.util.MotorOffsets;
import javiator.util.ControllerConstants;

/*****************************************************************************/
/*                                                                           */
/*   Class DigitalMeter                                                      */
/*                                                                           */
/*****************************************************************************/

public class DigitalMeter extends Panel
{
    public static final long serialVersionUID = 1;

    public DigitalMeter( ControlTerminal parent )
    {
        initWindow( parent );
    }

    public MotorSignals getMotorSignals( )
    {
        return( motorSignals );
    }

    public MotorOffsets getMotorOffsets( )
    {
        return( motorOffsets );
    }

    public byte getHeliMode( )
    {
        return( heliMode );
    }

    public byte getHeliState( )
    {
        return( heliState );
    }

    public void setMotorSignals( MotorSignals signals )
    {
        if( parent.isShow3D( ) )
        {
        	parent.getJaviator3D( ).setRotorSpeed( motorSignals );
        }

        if( motorSignals.front != signals.front )
        {
            signalFront.setText( ControlTerminal.NIL + signals.front );
            offsetFront.setText( ControlTerminal.NIL + ( signals.front - motorSignals.front ) );
            motorSignals.front = signals.front;
        }

        if( motorSignals.right != signals.right )
        {
            signalRight.setText( ControlTerminal.NIL + signals.right );
            offsetRight.setText( ControlTerminal.NIL + ( signals.right - motorSignals.right ) );
            motorSignals.right = signals.right;
        }

        if( motorSignals.rear != signals.rear )
        {
            signalRear.setText( ControlTerminal.NIL + signals.rear );
            offsetRear.setText( ControlTerminal.NIL + ( signals.rear - motorSignals.rear ) );
            motorSignals.rear = signals.rear;
        }

        if( motorSignals.left != signals.left )
        {
            signalLeft.setText( ControlTerminal.NIL + signals.left );
            offsetLeft.setText( ControlTerminal.NIL + ( signals.left - motorSignals.left ) );
            motorSignals.left = signals.left;
        }
    }

    public void setMotorOffsets( MotorOffsets data )
    {
        if( motorOffsets.roll != data.roll )
        {
            offsetRoll.setText( ControlTerminal.NIL + data.roll );
            motorOffsets.roll = data.roll;
        }

        if( motorOffsets.pitch != data.pitch )
        {
            offsetPitch.setText( ControlTerminal.NIL + data.pitch );
            motorOffsets.pitch = data.pitch;
        }

        if( motorOffsets.yaw != data.yaw )
        {
            offsetYaw.setText( ControlTerminal.NIL + data.yaw );
            motorOffsets.yaw = data.yaw;
        }

        if( motorOffsets.z != data.z )
        {
            offsetAlt.setText( ControlTerminal.NIL + data.z );
            motorOffsets.z = data.z;
        }
    }

    public void setStateAndMode( byte heliState, byte heliMode )
    {
        if( this.heliMode != heliMode )
        {
            switch( heliMode )
            {
                case ControllerConstants.ALT_MODE_GROUND:
                    altitudeMode.setForeground( colorDefault );
                    altitudeMode.setText( "Ground" );
                    break;

                case ControllerConstants.ALT_MODE_FLYING:
                	altitudeMode.setForeground( colorDefault );
                    altitudeMode.setText( "Flying" );
                    break;

                case ControllerConstants.ALT_MODE_SHUTDOWN:
                    altitudeMode.setForeground( colorAlerting );
                    altitudeMode.setText( "Halt" );
                    break;

                default:
                    altitudeMode.setForeground( colorDefault );
                    altitudeMode.setText( ControlTerminal._MODE + ' ' );
            }

            this.heliMode = heliMode;
        }

        if( this.heliState != heliState )
        {
            if( ( this.heliState & ControllerConstants.ADJUSTING_ROLL ) !=
                ( heliState & ControllerConstants.ADJUSTING_ROLL ) )
            {
                if( ( heliState & ControllerConstants.ADJUSTING_ROLL ) != 0 )
                {
                    controlRight .setForeground( colorAlerting );
                    controlLeft  .setForeground( colorAlerting );
                }
                else if( ( heliState & ControllerConstants.ADJUSTING_YAW ) == 0 )
                {
                    controlRight .setForeground( colorDefault );
                    controlLeft  .setForeground( colorDefault );
                }
            }

            if( ( this.heliState & ControllerConstants.ADJUSTING_PITCH ) !=
                ( heliState & ControllerConstants.ADJUSTING_PITCH ) )
            {
                if( ( heliState & ControllerConstants.ADJUSTING_PITCH ) != 0 )
                {
                    controlFront .setForeground( colorAlerting );
                    controlRear  .setForeground( colorAlerting );
                }
                else if( ( heliState & ControllerConstants.ADJUSTING_YAW ) == 0 )
                {
                    controlFront .setForeground( colorDefault );
                    controlRear  .setForeground( colorDefault );
                }
            }

            if( ( this.heliState & ControllerConstants.ADJUSTING_YAW ) !=
                ( heliState & ControllerConstants.ADJUSTING_YAW ) )
            {
                if( ( heliState & ControllerConstants.ADJUSTING_ROLL ) == 0 )
                {
                    if( ( heliState & ControllerConstants.ADJUSTING_YAW ) != 0 )
                    {
                        controlRight .setForeground( colorWarning );
                        controlLeft  .setForeground( colorWarning );
                    }
                    else
                    {
                        controlRight .setForeground( colorDefault );
                        controlLeft  .setForeground( colorDefault );
                    }
                }

                if( ( heliState & ControllerConstants.ADJUSTING_PITCH ) == 0 )
                {
                    if( ( heliState & ControllerConstants.ADJUSTING_YAW ) != 0 )
                    {
                        controlFront .setForeground( colorWarning );
                        controlRear  .setForeground( colorWarning );
                    }
                    else
                    {
                        controlFront .setForeground( colorDefault );
                        controlRear  .setForeground( colorDefault );
                    }
                }
            }

            if( this.heliMode != ControllerConstants.ALT_MODE_SHUTDOWN &&
                ( this.heliState & ControllerConstants.ADJUSTING_Z ) !=
                ( heliState & ControllerConstants.ADJUSTING_Z ) )
            {
                if( ( heliState & ControllerConstants.ADJUSTING_Z ) != 0 )
                {
                    altitudeMode.setForeground( colorAlerting );
                }
                else
                {
                    altitudeMode.setForeground( colorDefault );
                }
            }

            this.heliState = heliState;
        }
    }

    public void resetMotorMeter( )
    {
        altitudeMode .setForeground( colorDefault );
        controlFront .setForeground( colorDefault );
        controlRight .setForeground( colorDefault );
        controlRear  .setForeground( colorDefault );
        controlLeft  .setForeground( colorDefault );

        signalFront  .setForeground( colorDefault );
        signalRight  .setForeground( colorDefault );
        signalRear   .setForeground( colorDefault );
        signalLeft   .setForeground( colorDefault );

        offsetFront  .setForeground( colorDefault );
        offsetRight  .setForeground( colorDefault );
        offsetRear   .setForeground( colorDefault );
        offsetLeft   .setForeground( colorDefault );

        offsetRoll   .setForeground( colorOffsets );
        offsetPitch  .setForeground( colorOffsets );
        offsetYaw    .setForeground( colorOffsets );
        offsetAlt    .setForeground( colorOffsets );

        altitudeMode .setText( ControlTerminal._MODE + ' ' );
        signalFront  .setText( ControlTerminal.ZERO );
        signalRight  .setText( ControlTerminal.ZERO );
        signalRear   .setText( ControlTerminal.ZERO );
        signalLeft   .setText( ControlTerminal.ZERO );

        offsetFront  .setText( ControlTerminal.ZERO );
        offsetRight  .setText( ControlTerminal.ZERO );
        offsetRear   .setText( ControlTerminal.ZERO );
        offsetLeft   .setText( ControlTerminal.ZERO );

        offsetRoll   .setText( ControlTerminal.ZERO );
        offsetPitch  .setText( ControlTerminal.ZERO );
        offsetYaw    .setText( ControlTerminal.ZERO );
        offsetAlt    .setText( ControlTerminal.ZERO );

        motorSignals .reset( );
        motorOffsets .reset( );

        heliMode  = 0;
        heliState = 0;
    }

    public void setGrayed( boolean grayed )
    {
        if( grayed )
        {
            colorDefault  = Color.DARK_GRAY;
            colorOffsets  = Color.GRAY;
            colorWarning  = Color.LIGHT_GRAY;
            colorAlerting = Color.GRAY;
        }
        else
        {
            colorDefault  = Color.BLACK;
            colorOffsets  = Color.BLUE;
            colorWarning  = Color.ORANGE;
            colorAlerting = Color.RED;
        }

        update( );
    }

    public void update( )
    {
        byte state = heliState;
        byte mode  = heliMode;

        setStateAndMode( (byte) 0, (byte) 0 );
        setStateAndMode( state, mode );

        offsetRoll  .setForeground( colorOffsets );
        offsetPitch .setForeground( colorOffsets );
        offsetYaw   .setForeground( colorOffsets );
        offsetAlt   .setForeground( colorOffsets );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Protected Section                                                   */
    /*                                                                       */
    /*************************************************************************/

    protected void processFocusEvent( FocusEvent fe )
    {
        switch( fe.getID( ) )
        {
            case FocusEvent.FOCUS_GAINED:
                setGrayed( false );
                break;

            case FocusEvent.FOCUS_LOST:
                setGrayed( true );
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

    private ControlTerminal parent        = null;
    private Label           altitudeMode  = null;
    private Label           controlFront  = null;
    private Label           controlRight  = null;
    private Label           controlRear   = null;
    private Label           controlLeft   = null;

    private Label           signalFront   = null;
    private Label           signalRight   = null;
    private Label           signalRear    = null;
    private Label           signalLeft    = null;

    private Label           offsetFront   = null;
    private Label           offsetRight   = null;
    private Label           offsetRear    = null;
    private Label           offsetLeft    = null;

    private Label           offsetRoll    = null;
    private Label           offsetPitch   = null;
    private Label           offsetYaw     = null;
    private Label           offsetAlt     = null;

    private Color           colorDefault  = null;
    private Color           colorOffsets  = null;
    private Color           colorWarning  = null;
    private Color           colorAlerting = null;

    private MotorSignals    motorSignals  = null;
    private MotorOffsets    motorOffsets  = null;
    private byte            heliState     = 0;
    private byte            heliMode      = 0;

    private void initWindow( ControlTerminal parent )
    {
        this.parent   = parent;

        altitudeMode  = new Label( ControlTerminal._MODE + " ", Label.CENTER );
        controlFront  = new Label( "Front", Label.CENTER );
        controlRight  = new Label( "Right", Label.CENTER );
        controlRear   = new Label( "Rear",  Label.CENTER );
        controlLeft   = new Label( "Left",  Label.CENTER );

        signalFront   = new Label( ControlTerminal.ZERO, Label.CENTER );
        signalRight   = new Label( ControlTerminal.ZERO, Label.CENTER );
        signalRear    = new Label( ControlTerminal.ZERO, Label.CENTER );
        signalLeft    = new Label( ControlTerminal.ZERO, Label.CENTER );

        offsetFront   = new Label( ControlTerminal.ZERO, Label.CENTER );
        offsetRight   = new Label( ControlTerminal.ZERO, Label.CENTER );
        offsetRear    = new Label( ControlTerminal.ZERO, Label.CENTER );
        offsetLeft    = new Label( ControlTerminal.ZERO, Label.CENTER );

        offsetRoll    = new Label( ControlTerminal.ZERO, Label.RIGHT );
        offsetPitch   = new Label( ControlTerminal.ZERO, Label.RIGHT );
        offsetYaw     = new Label( ControlTerminal.ZERO, Label.LEFT );
        offsetAlt     = new Label( ControlTerminal.ZERO, Label.LEFT );

        colorDefault  = Color.BLACK;
        colorOffsets  = Color.BLUE;
        colorWarning  = Color.ORANGE;
        colorAlerting = Color.RED;

        motorSignals  = new MotorSignals( );
        motorOffsets  = new MotorOffsets( );

        offsetRoll  .setForeground( colorOffsets );
        offsetPitch .setForeground( colorOffsets );
        offsetYaw   .setForeground( colorOffsets );
        offsetAlt   .setForeground( colorOffsets );

        Panel controlLayout = new Panel( new GridLayout( 3, 3 ) );
        controlLayout.add( new Label( ) );
        controlLayout.add( controlFront );
        controlLayout.add( new Label( ) );
        controlLayout.add( controlLeft );
        controlLayout.add( altitudeMode );
        controlLayout.add( controlRight );
        controlLayout.add( new Label( ) );
        controlLayout.add( controlRear );
        controlLayout.add( new Label( ) );

        Panel controlDisplay = new Panel( new BorderLayout( ) );
        controlDisplay.add( makeSeparator( ), BorderLayout.NORTH );
        controlDisplay.add( controlLayout, BorderLayout.CENTER );

        Panel offsetsLayout = new Panel( new GridLayout( 3, 3 ) );
        offsetsLayout.add( offsetYaw );
        offsetsLayout.add( offsetFront );
        offsetsLayout.add( offsetRoll );
        offsetsLayout.add( offsetLeft );
        offsetsLayout.add( new Label( "Offsets", Label.CENTER ) );
        offsetsLayout.add( offsetRight );
        offsetsLayout.add( offsetAlt );
        offsetsLayout.add( offsetRear );
        offsetsLayout.add( offsetPitch );

        Panel offsetsDisplay = new Panel( new BorderLayout( ) );
        offsetsDisplay.add( makeSeparator( ), BorderLayout.NORTH );
        offsetsDisplay.add( offsetsLayout, BorderLayout.CENTER );

        Panel signalsLayout = new Panel( new GridLayout( 3, 3 ) );
        signalsLayout.add( new Label( ) );
        signalsLayout.add( signalFront );
        signalsLayout.add( new Label( ) );
        signalsLayout.add( signalLeft );
        signalsLayout.add( new Label( "Signals", Label.CENTER ) );
        signalsLayout.add( signalRight );
        signalsLayout.add( new Label( ) );
        signalsLayout.add( signalRear );
        signalsLayout.add( new Label( ) );

        Panel signalsDisplay = new Panel( new BorderLayout( ) );
        signalsDisplay.add( makeSeparator( ), BorderLayout.NORTH );
        signalsDisplay.add( signalsLayout, BorderLayout.CENTER );
        signalsDisplay.add( makeSeparator( ), BorderLayout.SOUTH );

        setLayout( new GridLayout( 3, 3 ) );
        add( controlDisplay );
        add( offsetsDisplay );
        add( signalsDisplay );
/*
        setLayout( new GridLayout( SENSOR_DATA_LIST.length >> 1, 7 ) );

	    for( int i = 0; i < SENSOR_DATA_LIST.length; i += 2 )
	    {
	        add( new Label( ) );
	        add( new Label( SENSOR_DATA_LIST[i], Label.LEFT ) );
	        add( new Label( "" + (521 * i), Label.RIGHT ) );
	        //add( new Label( ) );
	        add( new Label( ) );
	        add( new Label( SENSOR_DATA_LIST[i+1], Label.LEFT ) );
	        add( new Label( "" + (327 * i), Label.RIGHT ) );
	        add( new Label( ) );
	    }
*/
    }

    private Label makeSeparator( )
    {
        Label blankLabel = new Label( "@@@@@@@@@@@@@@", Label.CENTER );
        blankLabel.setForeground( Color.WHITE );

        return( blankLabel );
    }
}

// End of file.