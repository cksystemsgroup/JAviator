/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   DigitalMeter.java    Constructs a digital meter representing altitude   */
/*                        mode, control state, motor signals and offsets.    */
/*                                                                           */
/*   Copyright (c) 2006-2013 Rainer Trummer <rainer.trummer@gmail.com>       */
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
import java.awt.GridLayout;
import java.awt.Color;

import javiator.util.SensorData;
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
        super( );
        initWindow( parent );
    }

    public byte getHeliState( )
    {
        return( javiatorState );
    }

    public byte getHeliMode( )
    {
        return( javiatorMode );
    }

    public MotorSignals getMotorSignals( )
    {
        return( motorSignals );
    }

    public MotorOffsets getMotorOffsets( )
    {
        return( motorOffsets );
    }

    public void setStateAndMode( byte state, byte mode )
    {
        if( javiatorState != state )
        {
            switch( state )
            {
                case ControllerConstants.HELI_STATE_GROUND:
                    heliState.setForeground( getForeground( ) );
                    heliState.setText( "Ground" );
                    break;

                case ControllerConstants.HELI_STATE_FLYING:
                	heliState.setForeground( Color.BLUE );
                    heliState.setText( "Flying" );
                    break;

                case ControllerConstants.HELI_STATE_SHUTDOWN:
                    heliState.setForeground( Color.RED );
                    heliState.setText( "Halt" );
                    break;

                default:
                    heliState.setForeground( getForeground( ) );
                    heliState.setText( ControlTerminal.NIL );
            }

            javiatorState = state;
        }

        if( javiatorMode != mode )
        {
            if( (javiatorMode & ControllerConstants.HELI_MODE_POS_CTRL) !=
                (mode & ControllerConstants.HELI_MODE_POS_CTRL) )
            {
                if( (mode & ControllerConstants.HELI_MODE_POS_CTRL) != 0 )
                {
                    heliMode.setForeground( Color.RED );
                }
                else
                {
                    heliMode.setForeground( Color.LIGHT_GRAY );
                }
            }

            javiatorMode = mode;
        }
    }

    public void setSensorData( SensorData data )
    {
    	/* IMPORTANT: Ubisense location data refer to Cartesian coordinates,
           whereas JAviator location data refer to aircraft coordinates,
           hence x and y must be exchanged when copying the data. */
        positionX.setText( ControlTerminal.NIL + ( (double) data.y / 10 ) );
        positionY.setText( ControlTerminal.NIL + ( (double) data.x / 10 ) );
        mapsLabel.setText( ControlTerminal.NIL + ( (double) data.z / 10 ) );
        tempLabel.setText( ControlTerminal.NIL + ( (double)( data.temp / 10 ) / 10 ) );
        battLabel.setText( ControlTerminal.NIL + ( (double)( data.batt / 100 ) / 10 ) );

        if( battLabel.getForeground( ) == Color.GREEN )
        {
	        if( data.batt < 14000 )
	        {
	        	battLabel.setForeground( Color.ORANGE );
	        }
        }
        else
        if( battLabel.getForeground( ) == Color.ORANGE )
        {
	        if( data.batt < 13500 )
	        {
	        	battLabel.setForeground( Color.RED );
	        }
	        else
	        if( data.batt > 14000 )
	        {
	        	battLabel.setForeground( Color.GREEN );
	        }
        }
        else
        if( battLabel.getForeground( ) == Color.RED )
        {
	        if( data.batt > 13500 )
	        {
	        	battLabel.setForeground( Color.ORANGE );
	        }
        }
        else
        {
	        battLabel.setForeground( Color.RED );
        }
    }

    public void setMotorSignals( MotorSignals signals )
    {
/*
        if( parent.isShow3D( ) )
        {
        	parent.getJaviator3D( ).setRotorSpeed( motorSignals );
        }
*/
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

    public void setMotorOffsets( MotorOffsets offsets )
    {
        if( motorOffsets.roll != offsets.roll )
        {
            offsetRoll.setText( ControlTerminal.NIL + offsets.roll );
            motorOffsets.roll = offsets.roll;
        }

        if( motorOffsets.pitch != offsets.pitch )
        {
            offsetPitch.setText( ControlTerminal.NIL + offsets.pitch );
            motorOffsets.pitch = offsets.pitch;
        }

        if( motorOffsets.yaw != offsets.yaw )
        {
            offsetYaw.setText( ControlTerminal.NIL + offsets.yaw );
            motorOffsets.yaw = offsets.yaw;
        }

        if( motorOffsets.z != offsets.z )
        {
            offsetAlt.setText( ControlTerminal.NIL + offsets.z );
            motorOffsets.z = offsets.z;
        }
    }

    public void resetMotorMeter( )
    {
        javiatorState = 0;
        javiatorMode  = 0;

        motorSignals .reset( );
        motorOffsets .reset( );

        heliState    .setForeground( getForeground( ) );
        heliMode     .setForeground( Color.LIGHT_GRAY );
        battLabel    .setForeground( getForeground( ) );

        heliState    .setText( ControlTerminal.NIL );
        positionX    .setText( ControlTerminal.ZERO );
        positionY    .setText( ControlTerminal.ZERO );
        mapsLabel    .setText( ControlTerminal.ZERO );
        tempLabel    .setText( ControlTerminal.ZERO );
        battLabel    .setText( ControlTerminal.ZERO );

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
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    //private ControlTerminal parent        = null;
    private MotorSignals    motorSignals  = null;
    private MotorOffsets    motorOffsets  = null;
    
    private Label           heliState     = null;
    private Label           heliMode      = null;
    private Label           positionX     = null;
    private Label           positionY     = null;
    private Label           mapsLabel     = null;
    private Label           tempLabel     = null;
    private Label           battLabel     = null;

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

    private byte            javiatorState = 0;
    private byte            javiatorMode  = 0;

    private void initWindow( ControlTerminal parent )
    {
        //this.parent  = parent;
        motorSignals = new MotorSignals( );
        motorOffsets = new MotorOffsets( );

        heliState    = new Label( ControlTerminal.NIL,  Label.CENTER );
        heliMode     = new Label( ControlTerminal.POS,  Label.CENTER );
        positionX    = new Label( ControlTerminal.ZERO, Label.CENTER );
        positionY    = new Label( ControlTerminal.ZERO, Label.CENTER );
        mapsLabel    = new Label( ControlTerminal.ZERO, Label.CENTER );
        tempLabel    = new Label( ControlTerminal.ZERO, Label.CENTER );
        battLabel    = new Label( ControlTerminal.ZERO, Label.CENTER );

        signalFront  = new Label( ControlTerminal.ZERO, Label.CENTER );
        signalRight  = new Label( ControlTerminal.ZERO, Label.CENTER );
        signalRear   = new Label( ControlTerminal.ZERO, Label.CENTER );
        signalLeft   = new Label( ControlTerminal.ZERO, Label.CENTER );

        offsetFront  = new Label( ControlTerminal.ZERO, Label.CENTER );
        offsetRight  = new Label( ControlTerminal.ZERO, Label.CENTER );
        offsetRear   = new Label( ControlTerminal.ZERO, Label.CENTER );
        offsetLeft   = new Label( ControlTerminal.ZERO, Label.CENTER );

        offsetRoll   = new Label( ControlTerminal.ZERO, Label.CENTER );
        offsetPitch  = new Label( ControlTerminal.ZERO, Label.CENTER );
        offsetYaw    = new Label( ControlTerminal.ZERO, Label.CENTER );
        offsetAlt    = new Label( ControlTerminal.ZERO, Label.CENTER );

        heliMode    .setForeground( Color.LIGHT_GRAY );
        offsetRoll  .setForeground( Color.GRAY );
        offsetPitch .setForeground( Color.GRAY );
        offsetYaw   .setForeground( Color.GRAY );
        offsetAlt   .setForeground( Color.GRAY );

        Panel controlDisplay = new Panel( new GridLayout( 6, 3 ) );
        controlDisplay.add( new Label( "State:", Label.LEFT ) );
        controlDisplay.add( heliState );
        controlDisplay.add( heliMode );
        controlDisplay.add( new Label( "Pos. X:", Label.LEFT ) );
        controlDisplay.add( positionX );
        controlDisplay.add( new Label( "cm", Label.CENTER ) );
        controlDisplay.add( new Label( "Pos. Y:", Label.LEFT ) );
        controlDisplay.add( positionY );
        controlDisplay.add( new Label( "cm", Label.CENTER ) );
        controlDisplay.add( new Label( "Pos. Z:", Label.LEFT ) );
        controlDisplay.add( mapsLabel );
        controlDisplay.add( new Label( "cm", Label.CENTER ) );
        controlDisplay.add( new Label( "Temp:", Label.LEFT ) );
        controlDisplay.add( tempLabel );
        controlDisplay.add( new Label( "ºC", Label.CENTER ) );
        controlDisplay.add( new Label( "Power:", Label.LEFT ) );
        controlDisplay.add( battLabel );
        controlDisplay.add( new Label( "V", Label.CENTER ) );

        Panel signalsDisplay = new Panel( new GridLayout( 3, 3 ) );
        signalsDisplay.add( new Label( ) );
        signalsDisplay.add( signalFront );
        signalsDisplay.add( new Label( ) );
        signalsDisplay.add( signalLeft );
        signalsDisplay.add( new Label( "Signals", Label.CENTER ) );
        signalsDisplay.add( signalRight );
        signalsDisplay.add( new Label( ) );
        signalsDisplay.add( signalRear );
        signalsDisplay.add( new Label( ) );

        Panel offsetsDisplay = new Panel( new GridLayout( 3, 3 ) );
        offsetsDisplay.add( offsetYaw );
        offsetsDisplay.add( offsetFront );
        offsetsDisplay.add( offsetRoll );
        offsetsDisplay.add( offsetLeft );
        offsetsDisplay.add( new Label( "Offsets", Label.CENTER ) );
        offsetsDisplay.add( offsetRight );
        offsetsDisplay.add( offsetAlt );
        offsetsDisplay.add( offsetRear );
        offsetsDisplay.add( offsetPitch );

        setLayout( new GridLayout( 3, 1 ) );
        add( signalsDisplay );
        add( controlDisplay );
        add( offsetsDisplay );

        setFocusable( false );
    }
}

/* End of file */