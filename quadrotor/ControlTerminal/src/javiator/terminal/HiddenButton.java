/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   HiddenButton.java    Constructs a control button appearing as label.    */
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

import java.awt.Label;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.AWTEvent;

import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;

/*****************************************************************************/
/*                                                                           */
/*   Class HiddenButton                                                      */
/*                                                                           */
/*****************************************************************************/

public class HiddenButton extends Label
{
    public static final long serialVersionUID = 1;
    public static final byte SYMBOL_MINUS     = 1;
    public static final byte SYMBOL_PLUS      = 2;

    public HiddenButton( String text )
    {
    	super( );
        initWindow( text, (byte) 0 );
    }

    public HiddenButton( byte symbol )
    {
    	super( );
        initWindow( "   ", symbol );
    }

    public void paint( Graphics g )
    {
    	if( g != null )
    	{
	        int w = getWidth( ) - 1;  // subtract thickness
	        int h = getHeight( ) - 1; // of hairline (1pt)
	
	        g.setColor( color1 );
	        g.drawLine( 0, 0, 0, h - 1 );
	        g.drawLine( 0, 0, w - 1, 0 );
	        g.setColor( color2 );
	        g.drawLine( 1, 1, 1, h - 2 );
	        g.drawLine( 1, 1, w - 2, 1 );
	        g.setColor( color3 );
	        g.drawLine( w - 1, 1, w - 1, h - 1 );
	        g.drawLine( 1, h - 1, w - 1, h - 1 );
	        g.setColor( color4 );
	        g.drawLine( w, 0, w, h );
	        g.drawLine( 0, h, w, h );
	
	        if( symbol == SYMBOL_MINUS || symbol == SYMBOL_PLUS )
	        {
	            w = (w + 1) / 2;
	            h = (h + 1) / 2;
	
	            g.setColor( getForeground( ) );
	            g.drawLine( w - 4, h, w + 5, h );
	            g.drawLine( w - 4, h + 1, w + 5, h + 1 );
	
	            if( symbol == SYMBOL_PLUS )
	            {
	                g.drawLine( w, h - 4, w, h + 5 );
	                g.drawLine( w + 1, h - 4, w + 1, h + 5 );
	            }
	        }
    	}
    }

    public void update( Graphics g )
    {
        paint( g );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Protected Section                                                   */
    /*                                                                       */
    /*************************************************************************/

    protected void processMouseEvent( MouseEvent me )
    {
        boolean redraw = false;

        switch( me.getID( ) )
        {
            case MouseEvent.MOUSE_ENTERED:
                color1 = UIManagerColor.getButtonHighlight( );
                color2 = UIManagerColor.getButtonLight( );
                color3 = UIManagerColor.getButtonShadow( );
                color4 = UIManagerColor.getButtonDarkShadow( );
                redraw = true;
                break;

            case MouseEvent.MOUSE_PRESSED:
                color1 = UIManagerColor.getButtonDarkShadow( );
                color2 = UIManagerColor.getButtonShadow( );
                color3 = UIManagerColor.getButtonLight( );
                color4 = UIManagerColor.getButtonHighlight( );
                redraw = true;
                listeners = getMouseListeners( );
                for( int i = 0; i < listeners.length; ++i )
                {
                    listeners[i].mousePressed( me );
                }
                break;

            case MouseEvent.MOUSE_RELEASED:
                color1 = UIManagerColor.getButtonHighlight( );
                color2 = UIManagerColor.getButtonLight( );
                color3 = UIManagerColor.getButtonShadow( );
                color4 = UIManagerColor.getButtonDarkShadow( );
                redraw = true;
                listeners = getMouseListeners( );
                for( int i = 0; i < listeners.length; ++i )
                {
                    listeners[i].mouseReleased( me );
                }
                break;

            case MouseEvent.MOUSE_EXITED:
                color1 = UIManagerColor.getButtonBackground( );
                color2 = UIManagerColor.getButtonBackground( );
                color3 = UIManagerColor.getButtonBackground( );
                color4 = UIManagerColor.getButtonBackground( );
                redraw = true;
                break;

            case MouseEvent.MOUSE_CLICKED:
                listeners = getMouseListeners( );
                for( int i = 0; i < listeners.length; ++i )
                {
                    listeners[i].mouseClicked( me );
                }
                break;

            default:
                return;
        }

        if( redraw )
        {
    	    Graphics g = getGraphics( );
    	    
    	    if( g != null )
    	    {
    	    	paint( g );
    	    	g.dispose( );
    	    }
        }
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private MouseListener[] listeners = null;
    private Color           color1    = null;
    private Color           color2    = null;
    private Color           color3    = null;
    private Color           color4    = null;
    private byte            symbol    = 0;

    private void initWindow( String text, byte symbol )
    {
        if( text == null )
        {
            setIgnoreRepaint( true );
            return;
        }

        this.symbol = symbol;

        color1 = UIManagerColor.getButtonBackground( );
        color2 = UIManagerColor.getButtonBackground( );
        color3 = UIManagerColor.getButtonBackground( );
        color4 = UIManagerColor.getButtonBackground( );

        enableEvents( AWTEvent.MOUSE_EVENT_MASK );
        setFocusable( false );
        setAlignment( CENTER );
        setText( text );
    }
}

/* End of file */