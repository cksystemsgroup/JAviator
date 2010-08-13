/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   ParamDialog.java	Constructs a non-modal dialog that allows to change  */
/*                      the PID/PIDD controller parameters.                  */
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

import java.awt.Dialog;
import java.awt.Panel;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Color;

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.KeyListener;
import java.awt.event.KeyEvent;

import javiator.util.ControllerConstants;


/*****************************************************************************/
/*                                                                           */
/*   Class ParamDialog                                                       */
/*                                                                           */
/*****************************************************************************/

public class ParamDialog extends Dialog
{
    public static final long serialVersionUID = 1;

    public static ParamDialog createInstance( ControlTerminal parent,
    	String title, short[] controlParams, int[] changedParamID )
    {
        if( Instance == null )
        {
            Instance = new ParamDialog( parent,
            	title, controlParams, changedParamID );
        }

        return( Instance );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private static final String[] PARAMETERS     = { "Kp", "Ki", "Kd", "Kdd" };
    private static final int      SCALING_FACTOR = 1000;
    private static final int      MOTION_DELAY   = 50; /* ms */
    private static final int      PARAM_STEP     = 1;

    private static ParamDialog    Instance       = null;
    private short[]               controlParams  = null;
    private int[]                 changedParamID = null;
    private EditField[]           paramFields    = null;
    private MotionThread          motionThread   = null;

    private ParamDialog( ControlTerminal parent, String title,
        short[] controlParams, int[] changedParamID )
    {
        super( parent, title, false );

        this.controlParams  = controlParams;
        this.changedParamID = changedParamID;
        paramFields         = new EditField[ controlParams.length ];
        motionThread        = new MotionThread( );

        setBackground( Color.WHITE );
        makePanel( );
        pack( );

        addWindowListener( new WindowAdapter( )
        {
            public void windowClosing( WindowEvent we )
            {
                closeDialog( );
            }
        } );
        
        addKeyListener( new KeyListener( )
        {
            public void keyPressed( KeyEvent ke )
            {
            	int keyCode = ke.getKeyCode( );

            	if( keyCode == KeyEvent.VK_ESCAPE || keyCode == KeyEvent.VK_ENTER )
            	{
            		closeDialog( );
            	}
            }

            public void keyReleased( KeyEvent ke )
            {
            }

            public void keyTyped( KeyEvent ke )
            {
            }
        } );

        if( parent.getX( ) < getWidth( ) )
        {
            setLocation( parent.getX( ) + parent.getWidth( ), parent.getY( ) );
        }
        else
        {
            setLocation( parent.getX( ) - getWidth( ), parent.getY( ) );
        }

        setSize( getWidth( ), parent.getHeight( ) );
        setVisible( true );
        motionThread.start( );
    }
    
    private void closeDialog( )
    {
        motionThread.terminate( );
        dispose( );
        Instance = null;
    }

    private void makePanel( )
    {
        int   sectionRows = 4;
        int   sectionCols = 4;
        Panel r_p_Section = new Panel( new GridLayout( sectionRows, sectionCols, 20, 0 ) );
        Panel yaw_Section = new Panel( new GridLayout( sectionRows, sectionCols, 20, 0 ) );
        Panel alt_Section = new Panel( new GridLayout( sectionRows, sectionCols, 20, 0 ) );
        Panel x_y_Section = new Panel( new GridLayout( sectionRows, sectionCols, 20, 0 ) );
        Panel panel;

        for( int i = 0; i < controlParams.length; ++i )
        {
        	paramFields[i] = new EditField( ControlTerminal.NIL
                + (double) controlParams[i] / SCALING_FACTOR, 60, 10 );
        	paramFields[i].addKeyListener( new IDKeyListener( i ) );

        	HiddenButton minusButton = new HiddenButton( HiddenButton.SYMBOL_MINUS );
        	minusButton.addMouseListener( new IDMouseAdapter( i, -PARAM_STEP ) );

            HiddenButton plusButton = new HiddenButton( HiddenButton.SYMBOL_PLUS );
            plusButton.addMouseListener( new IDMouseAdapter( i, PARAM_STEP ) );

            if( i < sectionRows )
            {
                panel = r_p_Section;
            }
            else
            if( i < sectionRows * 2 )
            {
                panel = yaw_Section;
            }
            else
            if( i < sectionRows * 3 )
            {
                panel = alt_Section;
            }
            else
            if( i < sectionRows * 4 )
            {
                panel = x_y_Section;
            }
            else
            {
            	System.err.println( "ParamDialog.makePanel: invalid parameter count" );
            	return;
            }

            panel.add( new Label( PARAMETERS[ i % PARAMETERS.length ], Label.LEFT ) );
            panel.add( paramFields[i] );
            Panel buttonPanel = new Panel( new BorderLayout( ) );
            buttonPanel.add( minusButton, BorderLayout.WEST );
            buttonPanel.add( plusButton, BorderLayout.EAST );
            Panel rightJustifiedButtonPanel = new Panel( new BorderLayout( ) );
            rightJustifiedButtonPanel.add( buttonPanel, BorderLayout.EAST );
            panel.add( rightJustifiedButtonPanel );
        }

        Panel r_p_Panel = new Panel( new BorderLayout( ) );
        r_p_Panel.add( new Label( ), BorderLayout.NORTH );
        r_p_Panel.add( new Label( "   " + ControlTerminal.ROLL + " / "
            + ControlTerminal.PITCH + "   ", Label.CENTER ), BorderLayout.CENTER );
        r_p_Panel.add( r_p_Section, BorderLayout.SOUTH );

        Panel yaw_Panel = new Panel( new BorderLayout( ) );
        yaw_Panel.add( new Label( ), BorderLayout.NORTH );
        yaw_Panel.add( new Label( ControlTerminal.YAW, Label.CENTER ), BorderLayout.CENTER );
        yaw_Panel.add( yaw_Section, BorderLayout.SOUTH );

        Panel alt_Panel = new Panel( new BorderLayout( ) );
        alt_Panel.add( new Label( ), BorderLayout.NORTH );
        alt_Panel.add( new Label( ControlTerminal.ALTITUDE, Label.CENTER ), BorderLayout.CENTER );
        alt_Panel.add( alt_Section, BorderLayout.SOUTH );

        Panel x_y_Panel = new Panel( new BorderLayout( ) );
        x_y_Panel.add( new Label( ), BorderLayout.NORTH );
        x_y_Panel.add( new Label( "X / Y", Label.CENTER ), BorderLayout.CENTER );
        x_y_Panel.add( x_y_Section, BorderLayout.SOUTH );

        Panel gridPanel = new Panel( new GridLayout( 4, 1 ) );
        gridPanel.add( r_p_Panel );
        gridPanel.add( yaw_Panel );
        gridPanel.add( alt_Panel );
        gridPanel.add( x_y_Panel );

        setLayout( new BorderLayout( ) );
        add( gridPanel, BorderLayout.CENTER );
        add( new Label( ), BorderLayout.SOUTH );
    }

    /*************************************************************************/
    /*                                                                       */
    /* Class IDKeyListener                                                   */
    /*                                                                       */
    /*************************************************************************/

    private class IDKeyListener implements KeyListener
    {
    	public IDKeyListener( int listenerID )
    	{
    		super( );
    		this.listenerID = listenerID;
    	}

        public void keyPressed( KeyEvent ke )
        {
        	if( ke.getKeyCode( ) == KeyEvent.VK_ENTER )
        	{
        		try
        		{
                    int scaledParam = (int)( Double.parseDouble(
                        paramFields[ listenerID ].getText( ) ) * SCALING_FACTOR );

                    if( scaledParam < -ControllerConstants.MOTOR_MAX ||
                        scaledParam >  ControllerConstants.MOTOR_MAX )
                    {
                    	paramFields[ listenerID ].setForeground( Color.RED );
                    }
                    else
                    {
                    	paramFields[ listenerID ].setForeground( Color.BLUE );
    	            	paramFields[ listenerID ].setText( ControlTerminal.NIL
                            + (double) scaledParam / SCALING_FACTOR );
                    	controlParams[ listenerID ] = (short) scaledParam;
                        changedParamID[0] = listenerID;
            	    }
        		}
        		catch( NumberFormatException nfe )
        		{
        			paramFields[ listenerID ].setForeground( Color.RED );
        			paramFields[ listenerID ].setText( "NaN" );
        		}
        	}
        }

        public void keyReleased( KeyEvent ke )
        {
        }

        public void keyTyped( KeyEvent ke )
        {
        }

    	private int listenerID;
    }

    /*************************************************************************/
    /*                                                                       */
    /* Class IDMouseAdapter                                                  */
    /*                                                                       */
    /*************************************************************************/

    private class IDMouseAdapter extends MouseAdapter
    {
    	public IDMouseAdapter( int adapterID, int paramStep )
    	{
    		super( );
    		this.adapterID = adapterID;
            this.paramStep = paramStep;
    	}

        public void mousePressed( MouseEvent me )
        {
        	motionThread.setOffset( adapterID, paramStep );
        }

        public void mouseReleased( MouseEvent me )
        {
        	motionThread.setOffset( adapterID, 0 );
        	changedParamID[0] = adapterID;
        }

    	private int adapterID;
    	private int paramStep;
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

	            if( offset != 0 )
	            {
	            	controlParams[ index ] += (short) offset;
	            	paramFields[ index ].setText( ControlTerminal.NIL
                        + (double) controlParams[ index ] / SCALING_FACTOR );
	            }

                try
                {
                    sleep( MOTION_DELAY );
                }
                catch( InterruptedException e )
                {
                    System.err.println( "MotionThread.run: " + e.getMessage( ) );
                    break;
                }
            }
        }

        public void setOffset( int index, int offset )
        {
            this.index  = index;
            this.offset = offset;
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

        private int     index  = 0;
        private int     offset = 0;
        private boolean halt   = false;
    }
}

/* End of file */