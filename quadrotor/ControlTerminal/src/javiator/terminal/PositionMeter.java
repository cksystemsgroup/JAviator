/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   PositionMeter.java    Constructs a panel meter displaying the desired   */
/*                         and current position of the helicopter.           */
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

import java.awt.Canvas;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Graphics;
import java.awt.Font;

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
        super( );
        initWindow( );
    }

    public Dimension getUbiRect( )
    {
        return( ubiRect );
    }

    public Dimension getMaxRect( )
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

    public boolean isValidUbiRect( Dimension dim )
    {
    	if( dim.width < GRID_SPACE || dim.height < GRID_SPACE )
    	{
            return( false );
    	}

        return( true );
    }

    public boolean isValidMaxRect( Dimension dim )
    {
    	if( dim.width > ubiRect.width || dim.height > ubiRect.height )
        {
    		return( false );
    	}

        return( true );
    }

    public boolean isValidDesired( Point pnt )
    {
    	if( pnt.x < -maxRect.width / 2  || pnt.x > maxRect.width / 2 ||
            pnt.y < -maxRect.height / 2 || pnt.y > maxRect.height / 2 )
        {
            return( false );
        }

        return( true );
    }

    public boolean isValidCurrent( Point pnt )
    {
        return( true ); /* every location is valid */
    }

    public void setUbiRect( Dimension ubiRect )
    {
        if( (this.ubiRect.width != ubiRect.width || this.ubiRect.height != ubiRect.height) &&
            isValidUbiRect( ubiRect ) )
        {
        	this.ubiRect.width  = ubiRect.width;
        	this.ubiRect.height = ubiRect.height;
        	drawBackgnd         = true;

        	computeScaledBackground( );
        	redraw( );
        }
    }

    public void setMaxRect( Dimension maxRect )
    {
        if( (this.maxRect.width != maxRect.width || this.maxRect.height != maxRect.height) &&
            isValidMaxRect( maxRect ) )
        {
        	this.maxRect.width  = maxRect.width;
        	this.maxRect.height = maxRect.height;
        	drawBackgnd         = true;

        	redraw( );
        }
    }

    public void setDesired( Point desired )
    {
        if( (this.desired.x != desired.x || this.desired.y != desired.y) &&
            isValidDesired( desired ) )
        {
        	this.desired.x = desired.x;
        	this.desired.y = desired.y;
            drawDesired    = true;

            redraw( );
            cmdPoints.add( desired );
        }
    }

    public void setCurrent( Point current )
    {
        if( this.current.x != current.x || this.current.y != current.y )
        {
        	this.current.x = current.x; /* Do not check for validity, since this point */
        	this.current.y = current.y; /* represents heli data, and thus, must always */
            drawCurrent    = true;      /* be accepted as it is, even if out of range. */

            redraw( );
            posPoints.add( current );
        }
    }

    public void update( Graphics g )
    {
        if( g != null )
        {
	        if( drawBackgnd )
	        {
	        	Enumeration en;
	        	String unitStr;
	        	double unitNum;
	        	int linePos, halfFontHeight = (g.getFontMetrics( g.getFont( ) ).getHeight( ) - 5) / 2;

	    		g.setColor( colorMeter );
				g.fillRect( getPaintX( ),
						    getPaintY( ),
						    getPaintW( ) + 1,
						    getPaintH( ) + 1 );
	    		g.setColor( colorGrid );

	    		for( en = gridLinesX.elements( ); en.hasMoreElements( ); )
	    		{
	    			linePos = ( (Integer) en.nextElement( ) ).intValue( );
	    			g.drawLine( linePos, getPaintY( ), linePos, getPaintY( ) + getPaintH( ) );
	    		}

	    		for( en = gridLinesY.elements( ); en.hasMoreElements( ); )
	    		{
	    			linePos = ( (Integer) en.nextElement( ) ).intValue( );
	    			g.drawLine( getPaintX( ), linePos, getPaintX( ) + getPaintW( ), linePos );
	    		}

	    		g.setColor( colorUnits );
	    		g.setFont( new Font( Font.DIALOG, Font.PLAIN, 10 ) );

	    		for( en = gridLinesX.elements( ), unitNum = -gridLinesX.size( ) / 2 * GRID_SPACE;
	    		    en.hasMoreElements( ); unitNum += GRID_SPACE )
	    		{
	                linePos = ( (Integer) en.nextElement( ) ).intValue( );
	                unitStr = "" + ( (double)( unitNum / 100 ) / 10 );
	    			g.drawString( unitStr, linePos -
	                    g.getFontMetrics( g.getFont( ) ).stringWidth( unitStr ) / 2,
	                    getPaintY( ) + getPaintH( ) + 3 * halfFontHeight + 2 );
	    		}

	    		for( en = gridLinesY.elements( ), unitNum = -gridLinesY.size( ) / 2 * GRID_SPACE;
	                en.hasMoreElements( ); unitNum += GRID_SPACE )
				{
		            linePos = ( (Integer) en.nextElement( ) ).intValue( );
		            unitStr = "" + ( (double)( unitNum / 100 ) / 10 );
					g.drawString( unitStr, getPaintX( ) - halfFontHeight -
	                    g.getFontMetrics( g.getFont( ) ).stringWidth( unitStr ) - 2,
	                    linePos + halfFontHeight );
				}

	    		g.setFont( new Font( Font.DIALOG, Font.PLAIN, 12 ) );

				linePos = (Integer) gridLinesX.elementAt( gridLinesX.size( ) / 2 );
	            unitStr = "X (m)";
				g.drawString( unitStr, linePos -
	                g.getFontMetrics( g.getFont( ) ).stringWidth( unitStr ) / 2,
	                getPaintY( ) - halfFontHeight - 3 );

	            linePos = (Integer) gridLinesY.elementAt( gridLinesY.size( ) / 2 );
	            unitStr = "Y (m)";
				g.drawString( unitStr, getPaintX( ) + getPaintW( ) + halfFontHeight + 4,
	                linePos + halfFontHeight );

				g.setColor( colorUbiRect );
				g.drawRect( plotOrigin.x - (int)( gridFactor * ubiRect.width ) / 2,
						    plotOrigin.y - (int)( gridFactor * ubiRect.height ) / 2,
	                        (int)( gridFactor * ubiRect.width ),
	                        (int)( gridFactor * ubiRect.height ) );

				g.setColor( colorMaxRect );
				g.drawRect( plotOrigin.x - (int)( gridFactor * maxRect.width ) / 2,
					        plotOrigin.y - (int)( gridFactor * maxRect.height ) / 2,
	                        (int)( gridFactor * maxRect.width ),
	                        (int)( gridFactor * maxRect.height ) );

		        g.setColor( UIManagerColor.getButtonDarkShadow( ) );
		        g.drawLine( getPaintX( ) - 2,
		        		    getPaintY( ) - 2,
		        		    getPaintX( ) - 2,
		        		    getPaintY( ) + getPaintH( ) + 1 );
		        g.drawLine( getPaintX( ) - 2,
		        		    getPaintY( ) - 2,
		        		    getPaintX( ) + getPaintW( ) + 1,
		        		    getPaintY( ) - 2 );

		        g.setColor( UIManagerColor.getButtonShadow( ) );
		        g.drawLine( getPaintX( ) - 1,
		        		    getPaintY( ) - 1,
		        		    getPaintX( ) - 1,
		        		    getPaintY( ) + getPaintH( ) );
		        g.drawLine( getPaintX( ) - 1,
		        		    getPaintY( ) - 1,
		        		    getPaintX( ) + getPaintW( ),
		        		    getPaintY( ) - 1 );

		        g.setColor( UIManagerColor.getButtonLight( ) );
		        g.drawLine( getPaintX( ) + getPaintW( ) + 1,
		        		    getPaintY( ) - 1,
		        		    getPaintX( ) + getPaintW( ) + 1,
		        		    getPaintY( ) + getPaintH( ) + 1);
		        g.drawLine( getPaintX( ) - 1,
		        		    getPaintY( ) + getPaintH( ) + 1,
		        		    getPaintX( ) + getPaintW( ) + 1,
		        		    getPaintY( ) + getPaintH( ) + 1);

		        g.setColor( UIManagerColor.getButtonHighlight( ) );
		        g.drawLine( getPaintX( ) + getPaintW( ) + 2,
		        		    getPaintY( ) - 2,
		        		    getPaintX( ) + getPaintW( ) + 2,
		        		    getPaintY( ) + getPaintH( ) + 2 );
		        g.drawLine( getPaintX( ) - 2,
		        		    getPaintY( ) + getPaintH( ) + 2,
		        		    getPaintX( ) + getPaintW( ) + 2,
		        		    getPaintY( ) + getPaintH( ) + 2 );

	            drawBackgnd = false;
	        }

		    if( drawDesired )
		    {
		    	g.setColor( colorDesired );
	            g.fillOval( plotOrigin.x + (int)( gridFactor * desired.x ) - pointSize / 2,
	                        plotOrigin.y - (int)( gridFactor * desired.y ) - pointSize / 2,
	                        pointSize,
	                        pointSize );

	            drawDesired = false;
		    }

		    if( drawCurrent )
		    {
				g.setColor( colorCurrent );
	            g.fillOval( plotOrigin.x + (int)( gridFactor * current.x ) - pointSize / 2,
	                        plotOrigin.y - (int)( gridFactor * current.y ) - pointSize / 2,
	                        pointSize,
	                        pointSize );

	            drawCurrent = false;
	        }
        }
    }

    public void clearWindow( boolean drawBackgnd,
        boolean drawDesired, boolean drawCurrent )
    {
        Graphics g = getGraphics( );

        if( g != null )
        {
	        this.drawBackgnd = drawBackgnd;
	        this.drawDesired = drawDesired;
	        this.drawCurrent = drawCurrent;
	
	        g.clearRect( 0, 0, getWidth( ), getHeight( ) );
	        update( g );
	        g.dispose();
	
	        cmdPoints.clear( );
	        posPoints.clear( );
        }
    }

    public void paint( Graphics g )
    {
    	Enumeration en;

    	drawBackgnd = true;
    	computeScaledBackground( );
        update( g );

		for( en = cmdPoints.elements( ); en.hasMoreElements( ); )
		{
            Point point = (Point) en.nextElement( );
            desired.x   = point.x;
            desired.y   = point.y;
        	drawDesired = true;
            update( g );
		}

		for( en = posPoints.elements( ); en.hasMoreElements( ); )
		{
            Point point = (Point) en.nextElement( );
            current.x   = point.x;
            current.y   = point.y;
        	drawCurrent = true;
            update( g );
		}
    }

    /*************************************************************************/
    /*                                                                       */
    /*   Private Section                                                     */
    /*                                                                       */
    /*************************************************************************/

    private static final int GRID_SPACE   = 500; /* mm */
    private static final int UNIT_SPACE   = 40;  /* pt */
    private static final int POINT_SIZE   = 16;  /* pt */

    private Dimension        ubiRect      = null;
    private Dimension        maxRect      = null;
    private Point            desired      = null;
    private Point            current      = null;
    private Point            plotOrigin   = null;
    private Vector<Integer>  gridLinesX   = null;
    private Vector<Integer>  gridLinesY   = null;
    private Vector<Point>    cmdPoints    = null;
    private Vector<Point>    posPoints    = null;
    private Color            colorMeter   = null;
    private Color            colorGrid    = null;
    private Color            colorUnits   = null;
    private Color            colorUbiRect = null;
    private Color            colorMaxRect = null;
    private Color            colorDesired = null;
    private Color            colorCurrent = null;
    private double           gridFactor   = 0;
    private int              pointSize    = 0;
    private boolean          drawBackgnd  = false;
    private boolean          drawDesired  = false;
    private boolean          drawCurrent  = false;

    private void initWindow( )
    {
        ubiRect      = new Dimension( );
        maxRect      = new Dimension( );
        desired      = new Point( );
        current      = new Point( );
        plotOrigin   = new Point( );
        gridLinesX   = new Vector<Integer>( );
        gridLinesY   = new Vector<Integer>( );
        cmdPoints    = new Vector<Point>( );
        posPoints    = new Vector<Point>( );
        colorMeter   = Color.WHITE;
        colorGrid    = Color.LIGHT_GRAY;
        colorUnits   = UIManagerColor.getButtonForeground( );
        colorUbiRect = Color.RED;
        colorMaxRect = Color.GREEN;
        colorDesired = Color.GREEN;
        colorCurrent = Color.RED;

        setFocusable( false );
    }

    private int getPaintX( )
    {
    	return( UNIT_SPACE );
    }

    private int getPaintY( )
    {
    	return( UNIT_SPACE );
    }

    private int getPaintW( )
    {
    	return( getWidth( ) - 2 * UNIT_SPACE );
    }

    private int getPaintH( )
    {
    	return( getHeight( ) - 2 * UNIT_SPACE );
    }

    private void computeScaledBackground( )
    {
    	if( getPaintW( ) == 0 || getPaintH( ) == 0 )
    	{
    		return;
    	}

    	double factorWidth  = (double) getPaintW( ) / ubiRect.width;
    	double factorHeight = (double) getPaintH( ) / ubiRect.height;

    	if( factorWidth < factorHeight )
    	{
    		gridFactor = factorWidth;
    	}
    	else
    	{
    		gridFactor = factorHeight;
    	}

		plotOrigin.x = getWidth( ) / 2;
		plotOrigin.y = getHeight( ) / 2;
    	pointSize    = (int)( gridFactor * POINT_SIZE ) + 1;

    	int linePos, space = (int)( gridFactor * GRID_SPACE );

    	gridLinesX.clear( );

    	for( linePos = (getPaintW( ) / 2) % space; linePos < getPaintW( ); linePos += space )
    	{
    		gridLinesX.add( (Integer)( getPaintX( ) + linePos ) );
    	}

    	gridLinesY.clear( );

    	for( linePos = getPaintH( ) - (getPaintH( ) / 2) % space; linePos >= 0; linePos -= space )
    	{
    		gridLinesY.add( (Integer)( getPaintY( ) + linePos ) );
    	}
    }
    
    private void redraw( )
    {
        Graphics g = getGraphics( );

        if( g != null )
        {
            update( g );
            g.dispose( );
        }
    }
}

/* End of file */