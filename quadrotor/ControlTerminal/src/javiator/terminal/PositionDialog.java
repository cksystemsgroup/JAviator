/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   PositionDialog.java    Constructs a non-modal dialog for displaying the */
/*                          helicopter's desired and current position.       */
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
import java.awt.GraphicsEnvironment;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Rectangle;
import java.awt.Point;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.KeyListener;
import java.awt.event.KeyEvent;

/*****************************************************************************/
/*                                                                           */
/*   Class PositionDialog                                                    */
/*                                                                           */
/*****************************************************************************/

public class PositionDialog extends Dialog
{
    public static final long serialVersionUID = 1;

    public static PositionDialog createInstance( ControlTerminal parent, String title,
    		Rectangle ubiRect, Rectangle maxRect, boolean[] showPosition )
    {
        if( Instance == null )
        {
            Instance = new PositionDialog( parent, title,
                ubiRect, maxRect, showPosition );
        }

        return( Instance );
    }

    public Rectangle getUbiRect( )
    {
        return positionMeter.getUbiRect( );
    }

    public Rectangle getMaxRect( )
    {
        return positionMeter.getMaxRect( );
    }

    public Point getDesired( )
    {
        return positionMeter.getDesired( );
    }

    public Point getCurrent( )
    {
        return positionMeter.getCurrent( );
    }

    public boolean isValidUbiRect( Rectangle rect )
    {
        return positionMeter.isValidUbiRect( rect );
    }

    public boolean isValidMaxRect( Rectangle rect )
    {
        return positionMeter.isValidMaxRect( rect );
    }

    public boolean isValidDesired( Point pnt )
    {
        return positionMeter.isValidDesired( pnt );
    }

    public boolean isValidCurrent( Point pnt )
    {
        return positionMeter.isValidCurrent( pnt );
    }

    public void setUbiRect( Rectangle ubiRect )
    {
    	positionMeter.setUbiRect( ubiRect );
    }

    public void setMaxRect( Rectangle maxRect )
    {
    	positionMeter.setMaxRect( maxRect );
    }

    public void setDesired( Point desired )
    {
    	positionMeter.setDesired( desired );
    }

    public void setCurrent( Point current )
    {
    	positionMeter.setCurrent( current );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private static PositionDialog Instance      = null;
    private PositionMeter         positionMeter = null;
    private boolean[]             showPosition  = null;

    private PositionDialog( ControlTerminal parent, String title,
        Rectangle ubiRect, Rectangle maxRect, boolean[] showPosition )
    {
        super( parent, title, false );

        this.showPosition = showPosition;
        positionMeter     = new PositionMeter( );
        positionMeter.setUbiRect( ubiRect );
        positionMeter.setMaxRect( maxRect );

        setBackground( Color.WHITE );
        setLayout( new BorderLayout( ) );
        add( positionMeter, BorderLayout.CENTER );
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

        Rectangle bounds = GraphicsEnvironment.getLocalGraphicsEnvironment( ).getMaximumWindowBounds( );
        int spaceLeft    = parent.getX( );
        int spaceRight   = bounds.width - parent.getX( ) - parent.getWidth( );

        if( spaceRight > spaceLeft )
        {
            setLocation( parent.getX( ) + parent.getWidth( ), parent.getY( ) );
            setSize( spaceRight, parent.getHeight( ) );
        }
        else
        {
            setLocation( 0, parent.getY( ) );
            setSize( spaceLeft, parent.getHeight( ) );
        }

        this.showPosition[0] = true;
        setVisible( true );
    }
    
    private void closeDialog( )
    {
        showPosition[0] = false;
        dispose( );
        Instance = null;
    }
}

/* End of file */