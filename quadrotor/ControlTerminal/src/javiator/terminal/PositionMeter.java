/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   PositionMeter.java     Constructs a panel meter displaying the desired  */
/*                          and current position of the helicopter.          */
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

import java.awt.Canvas;
import java.awt.Color;
import java.awt.Rectangle;
import java.awt.Point;
import java.awt.Graphics;

import java.util.Vector;
import java.util.Enumeration;

/*****************************************************************************/
/*                                                                           */
/*   Class PositionMeter                                                     */
/*                                                                           */
/*****************************************************************************/

public class PositionMeter extends Canvas
{
    public static final long serialVersionUID = 1;
    
    public PositionMeter( )
    {
        initWindow( );
    }

    public Rectangle getUbiRect( )
    {
        return( ubiRect );
    }

    public Rectangle getMaxRect( )
    {
        return( maxRect );
    }

    public Point getDesired( )
    {
        return( desired );
    }

    public Point getCurrent( )
    {
        return( current );
    }

    public boolean isValidUbiRect( Rectangle rect )
    {
    	if( rect.x < 0 || rect.width  < GRID_SPACE ||
            rect.y < 0 || rect.height < GRID_SPACE )
    	{
            return( false );
    	}

        return( true );
    }

    public boolean isValidMaxRect( Rectangle rect )
    {
    	if( rect.x < ubiRect.x || rect.x + rect.width  > ubiRect.x + ubiRect.width ||
            rect.y < ubiRect.y || rect.y + rect.height > ubiRect.y + ubiRect.height )
        {
    		return( false );
    	}

        return( true );
    }

    public boolean isValidDesired( Point pnt )
    {
    	if( pnt.x < maxRect.x || pnt.x > maxRect.x + maxRect.width ||
            pnt.y < maxRect.y || pnt.y > maxRect.y + maxRect.height )
        {
            return( false );
        }

        return( true );
    }

    public boolean isValidCurrent( Point pnt )
    {
        return( true ); /* every location is valid */
    }

    public void setUbiRect( Rectangle ubiRect )
    {
        if( (this.ubiRect.x != ubiRect.x || this.ubiRect.width  != ubiRect.width   ||
             this.ubiRect.y != ubiRect.y || this.ubiRect.height != ubiRect.height) &&
            isValidUbiRect( ubiRect ) )
        {
        	this.ubiRect = ubiRect;
        	computeScaledBackground( );
        	drawBackgnd  = true;
        	redraw( );
        }
    }

    public void setMaxRect( Rectangle maxRect )
    {
        if( (this.maxRect.x != maxRect.x || this.maxRect.width  != maxRect.width   ||
             this.maxRect.y != maxRect.y || this.maxRect.height != maxRect.height) &&
            isValidMaxRect( maxRect ) )
        {
        	this.maxRect = maxRect;
        	drawBackgnd  = true;
        	redraw( );
        }
    }

    public void setDesired( Point desired )
    {
        if( (this.desired.x != desired.x || this.desired.y != desired.y) &&
            isValidDesired( desired ) )
        {
        	this.desired = desired;        
            drawDesired  = true;
            redraw( );
        }
    }

    public void setCurrent( Point current )
    {
        if( this.current.x != current.x || this.current.y != current.y )
        {                           /* Do not check for validity, since this point */
        	this.current = current; /* represents heli data, and thus, must always */
            drawCurrent  = true;    /* be accepted as it is, even if out of range. */
            redraw( );
        }
    }

    public void update( Graphics g )
    {
        if( drawBackgnd )
        {
        	Enumeration en;
        	int linePos;

    		g.setColor( colorGrid );

    		for( en = gridLinesX.elements( ); en.hasMoreElements( ); )
    		{
    			linePos = ( (Integer) en.nextElement( ) ).intValue( );
    			g.drawLine( linePos, 0, linePos, getHeight( ) );
    		}

    		for( en = gridLinesY.elements( ); en.hasMoreElements( ); )
    		{
    			linePos = ( (Integer) en.nextElement( ) ).intValue( );
    			g.drawLine( 0, linePos, getWidth( ), linePos );
    		}

			g.setColor( colorUbiRect );

			g.drawRect( ubiOrigin.x,
                        ubiOrigin.y,
                        (int)( gridFactor * ubiRect.width ) - 1,
                        (int)( gridFactor * ubiRect.height ) - 1 );

			g.setColor( colorMaxRect );

			g.drawRect( ubiOrigin.x + (int)( gridFactor * maxRect.x ),
        		        ubiOrigin.y + (int)( gridFactor * maxRect.y ),
                        (int)( gridFactor * maxRect.width ),
                        (int)( gridFactor * maxRect.height ) );

            drawBackgnd = false;
        }

	    if( drawDesired )
	    {
	    	g.setColor( colorDesired );

            g.fillOval( ubiOrigin.x + (int)( gridFactor * desired.x ) - pointSize / 2,
            		    ubiOrigin.y + (int)( gridFactor * (ubiRect.height - desired.y) ) - pointSize / 2,
                        pointSize,
                        pointSize );

            drawDesired = false;
	    }
	        
	    if( drawCurrent )
	    {
			g.setColor( colorCurrent );

            g.fillOval( ubiOrigin.x + (int)( gridFactor * current.x ) - pointSize / 2,
            		    ubiOrigin.y + (int)( gridFactor * (ubiRect.height - current.y) ) - pointSize / 2,
                        pointSize,
                        pointSize );

            drawCurrent = false;
        }
    }
    
    public void paint( Graphics g )
    {
    	computeScaledBackground( );
    	drawBackgnd = true;
        update( g );
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private static final int GRID_SPACE   = 1000; /* mm */
    private static final int POINT_SIZE   = 100;

    private Rectangle        ubiRect      = null;
    private Rectangle        maxRect      = null;
    private Point            desired      = null;
    private Point            current      = null;
    private Point            ubiOrigin    = null;
    private Vector<Integer>  gridLinesX   = null;
    private Vector<Integer>  gridLinesY   = null;
    private Color            colorGrid    = null;
    private Color            colorUbiRect = null;
    private Color            colorMaxRect = null;
    private Color            colorDesired = null;
    private Color            colorCurrent = null;
    private double           gridFactor   = 0.0;
    private int              pointSize    = 0;
    private boolean          drawBackgnd  = false;
    private boolean          drawDesired  = false;
    private boolean          drawCurrent  = false;

    private void initWindow( )
    {
        setFocusable( false );

        ubiRect      = new Rectangle( );
        maxRect      = new Rectangle( );
        desired      = new Point( 2000, 2000 );
        current      = new Point( );
        ubiOrigin    = new Point( );
        gridLinesX   = new Vector<Integer>( );
        gridLinesY   = new Vector<Integer>( );
        colorGrid    = Color.LIGHT_GRAY;
        colorUbiRect = Color.ORANGE;
        colorMaxRect = Color.GREEN;
        colorDesired = Color.GREEN;
        colorCurrent = Color.RED;
    }

    private void computeScaledBackground( )
    {
    	double factorWidth  = (double) getWidth( ) / ubiRect.width;
    	double factorHeight = (double) getHeight( ) / ubiRect.height;
    	
    	if( factorWidth < factorHeight )
    	{
    		gridFactor = factorWidth;
    	}
    	else
    	{
    		gridFactor = factorHeight;
    	}

		ubiOrigin.x = (getWidth( ) - (int)( gridFactor * ubiRect.width )) / 2;
		ubiOrigin.y = (getHeight( ) - (int)( gridFactor * ubiRect.height )) / 2;
    	pointSize   = (int)( gridFactor * POINT_SIZE );

    	int linePos, space = (int)( gridFactor * GRID_SPACE );

    	gridLinesX.clear( );

    	for( linePos = getWidth( ) / 2; linePos > 0; linePos -= space )
    	{
    		gridLinesX.add( (Integer) linePos );
    	}

    	for( linePos = getWidth( ) / 2; linePos < getWidth( ); linePos += space )
    	{
    		gridLinesX.add( (Integer) linePos );
    	}

    	gridLinesY.clear( );

    	for( linePos = getHeight( ) / 2; linePos > 0; linePos -= space )
    	{
    		gridLinesY.add( (Integer) linePos );
    	}

    	for( linePos = getHeight( ) / 2; linePos < getHeight( ); linePos += space )
    	{
    		gridLinesY.add( (Integer) linePos );
    	}
    }
    
    private void redraw( )
    {
        Graphics graphics = getGraphics( );

        if( graphics != null )
        {
            update( graphics );
        }
    }
}

/* End of file */