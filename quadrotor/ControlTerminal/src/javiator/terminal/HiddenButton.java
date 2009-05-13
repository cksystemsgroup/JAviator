/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   HiddenButton.java    Constructs a control button appearing as label.    */
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

    public HiddenButton( String label )
    {
        initWindow( label, (byte) 0 );
    }

    public HiddenButton( byte symbol )
    {
        initWindow( "   ", symbol );
    }

    public void paint( Graphics g )
    {
        int w = getWidth() - 1;  // subtract thickness
        int h = getHeight() - 1; // of hairline (1pt)

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
            w = ( w + 1 ) >> 1;
            h = ( h + 1 ) >> 1;

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
        switch( me.getID( ) )
        {
            case MouseEvent.MOUSE_ENTERED:
                color1 = Color.LIGHT_GRAY;
                color2 = Color.LIGHT_GRAY;
                color3 = Color.GRAY;
                color4 = Color.BLACK;
                update( getGraphics( ) );
                break;

            case MouseEvent.MOUSE_PRESSED:
                color1 = Color.GRAY;
                color2 = Color.BLACK;
                color3 = Color.LIGHT_GRAY;
                color4 = Color.LIGHT_GRAY;
                update( getGraphics( ) );
                listeners = getMouseListeners( );
                for( int i = 0; i < listeners.length; ++i )
                {
                    listeners[i].mousePressed( me );
                }
                break;

            case MouseEvent.MOUSE_RELEASED:
                color1 = Color.LIGHT_GRAY;
                color2 = Color.LIGHT_GRAY;
                color3 = Color.GRAY;
                color4 = Color.BLACK;
                update( getGraphics( ) );
                listeners = getMouseListeners( );
                for( int i = 0; i < listeners.length; ++i )
                {
                    listeners[i].mouseReleased( me );
                }
                break;

            case MouseEvent.MOUSE_EXITED:
                color1 = Color.WHITE;
                color2 = Color.WHITE;
                color3 = Color.WHITE;
                color4 = Color.WHITE;
                update( getGraphics( ) );
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

    private void initWindow( String label, byte symbol )
    {
        if( label == null )
        {
            setIgnoreRepaint( true );
            return;
        }

        color1 = Color.WHITE;
        color2 = Color.WHITE;
        color3 = Color.WHITE;
        color4 = Color.WHITE;
        this.symbol = symbol;

        enableEvents( AWTEvent.MOUSE_EVENT_MASK );
        setAlignment( CENTER );
        setFocusable( false );
        setText( label );
    }
}

// End of file.