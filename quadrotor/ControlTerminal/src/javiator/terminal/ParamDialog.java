/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   ParamDialog.java	Constructs a non-modal dialog that allows to change  */
/*                      the PIDD controller parameters.                      */
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

import java.awt.Dialog;
import java.awt.Panel;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Color;
import java.awt.Point;
import java.awt.GraphicsEnvironment;

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.KeyListener;
import java.awt.event.KeyEvent;


/*****************************************************************************/
/*                                                                           */
/*   Class ParamDialog                                                       */
/*                                                                           */
/*****************************************************************************/

public class ParamDialog extends Dialog
{
    public static final long serialVersionUID = 1;

    public static ParamDialog createInstance( ControlTerminal parent, String title,
        short[] controlParams, int[] changedParamID )
    {
        if( Instance == null )
        {
            Instance = new ParamDialog( parent, "Params", controlParams, changedParamID );
        }

        return( Instance );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private static final String[] PID_PARAMS     = { "Kp", "Ki", "Kd", "Kdd" };
    private static final String[] REV_PARAMS     = { "Limit", "Ctrl", "Inc", "Dec" };
    private static final int      SCALING_FACTOR = 1000;
    private static final int      MOTION_DELAY   = 50; // ms
    private static final int      PARAM_STEP     = 1;

    private static ParamDialog    Instance       = null;
    private short[]               controlParams  = null;
    private int[]                 changedParamID = null;
    private Label[]               paramLabels    = null;
    private MotionThread          motion         = null;

    private ParamDialog( ControlTerminal parent, String title,
        short[] controlParams, int[] changedParamID )
    {
        super( parent, title, false );

        this.controlParams  = controlParams;
        this.changedParamID = changedParamID;
        paramLabels         = new Label[ controlParams.length ];

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

        Point center = GraphicsEnvironment.getLocalGraphicsEnvironment( ).getCenterPoint( );
        setLocation( center.x - ( getWidth( ) >> 1 ), center.y - ( getHeight( ) >> 1 ) );
        setResizable( false );
        setVisible( true );

        motion = new MotionThread( );
    }
    
    private void closeDialog( )
    {
        motion.terminate( );
        dispose( );
        Instance = null;
    }

    private void makePanel( )
    {
        String[] paramNames  = PID_PARAMS;
        int      sectionRows = 4;
        int      sectionCols = 4;
        Panel    r_p_Section = new Panel( new GridLayout( sectionRows, sectionCols, 20, 0 ) );
        Panel    yaw_Section = new Panel( new GridLayout( sectionRows, sectionCols, 20, 0 ) );
        Panel    alt_Section = new Panel( new GridLayout( sectionRows, sectionCols, 20, 0 ) );
        Panel    x_y_Section = new Panel( new GridLayout( sectionRows, sectionCols, 20, 0 ) );
        Panel    rev_Section = new Panel( new GridLayout( sectionRows, sectionCols, 20, 0 ) );
        Panel    panel;

        for( int i = 0; i < controlParams.length; ++i )
        {
        	paramLabels[i] = new Label( ControlTerminal.NIL
                + (double) controlParams[i] / SCALING_FACTOR, Label.RIGHT );

        	HiddenButton minusButton = new HiddenButton( HiddenButton.SYMBOL_MINUS );
        	minusButton.addMouseListener( new IDMouseAdapter( i )
            {
                public void mousePressed( MouseEvent me )
                {
                	motion.setOffset( getAdapterID( ), -PARAM_STEP );
                }

                public void mouseReleased( MouseEvent me )
                {
                	motion.setOffset( getAdapterID( ), 0 );
                	changedParamID[0] = getAdapterID( );
                }
            } );

            HiddenButton plusButton = new HiddenButton( HiddenButton.SYMBOL_PLUS );
            plusButton.addMouseListener( new IDMouseAdapter( i )
            {
                public void mousePressed( MouseEvent me )
                {
                	motion.setOffset( getAdapterID( ), PARAM_STEP );
                }

                public void mouseReleased( MouseEvent me )
                {
                	motion.setOffset( getAdapterID( ), 0 );
                	changedParamID[0] = getAdapterID( );
                }
            } );
            
            if( i < sectionRows * 1 )
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
            if( i < sectionRows * 5 )
            {
                paramNames = REV_PARAMS;
                panel = rev_Section;
            }
            else
            {
            	System.err.println( "ParamDialog.makePanel: invalid parameter count" );
            	return;
            }

            panel.add( new Label( paramNames[ i % paramNames.length ], Label.LEFT ) );
            panel.add( paramLabels[i] );
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

        Panel rev_Panel = new Panel( new BorderLayout( ) );
        rev_Panel.add( new Label( ), BorderLayout.NORTH );
        rev_Panel.add( new Label( "Revving", Label.CENTER ), BorderLayout.CENTER );
        rev_Panel.add( rev_Section, BorderLayout.SOUTH );

        setLayout( new GridLayout( 5, 1, 0, 0 ) );
        add( r_p_Panel );
        add( yaw_Panel );
        add( alt_Panel );
        add( x_y_Panel );
        add( rev_Panel );
    }

    /*************************************************************************/
    /*                                                                       */
    /* Class IDMouseAdapter                                                  */
    /*                                                                       */
    /*************************************************************************/

    private class IDMouseAdapter extends MouseAdapter
    {
    	IDMouseAdapter( int adapterID )
    	{
    		super( );
    		this.adapterID = adapterID;
    	}
    	
    	public int getAdapterID( )
    	{
    		return( adapterID );
    	}

    	private int adapterID;
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
            start( );
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
	            	paramLabels[ index ].setText( ControlTerminal.NIL
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

// End of file.