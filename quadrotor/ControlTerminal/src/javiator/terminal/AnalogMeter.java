/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   AnalogMeter.java   Constructs a panel meter representing either roll,   */
/*                      pitch, yaw, or altitude user and heli data.          */
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
import java.awt.Image;
import java.awt.Color;
import java.awt.Polygon;
import java.awt.MediaTracker;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.RenderingHints;
import java.awt.Toolkit;

import java.awt.image.BufferedImage;

import java.awt.geom.AffineTransform;

import java.awt.event.FocusEvent;

/*****************************************************************************/
/*                                                                           */
/*   Class AnalogMeter                                                       */
/*                                                                           */
/*****************************************************************************/

public class AnalogMeter extends Canvas
{
    public static final long   serialVersionUID = 1;
    public static final double MRAD_TO_MM       = 0.628;
    public static final double MRAD_TO_PER      = 3.142;

    public static final int    DEG_45           = 785;  /* mrad */
    public static final int    DEG_90           = 1571; /* mrad */
    public static final int    DEG_135          = 2356; /* mrad */
    public static final int    DEG_180          = 3142; /* mrad */
    public static final int    DEG_360          = 6284; /* mrad */

    public static final byte   TYPE_ROLL        = 1;
    public static final byte   TYPE_PITCH       = 2;
    public static final byte   TYPE_YAW         = 3;
    public static final byte   TYPE_ALTITUDE    = 4;
    public static final byte   TYPE_THRUST      = 5;
    
    public AnalogMeter( boolean smallMeter, boolean smallFont, byte meterType )
    {
        super( );
        initWindow( smallMeter, smallFont, meterType );
    }

    public static Image getImage( String filename )
    {
        return Toolkit.getDefaultToolkit( ).createImage(
            AnalogMeter.class.getResource( "/img/" + filename ) );
    }

    public String getMeterName( )
    {
        return( meterName );
    }

    public byte getMeterType( )
    {
        return( meterType );
    }

    public int getMaximum( )
    {
        return( maximum );
    }

    public int getDesired( )
    {
        return( desired );
    }

    public int getCurrent( )
    {
        return( current );
    }

    public boolean isValidMaximum( int angle )
    {
        if( meterType == TYPE_ALTITUDE )
        {
            angle = (int)( (double) angle * MRAD_TO_MM );
        }
        else
        if( meterType == TYPE_THRUST )
        {
            angle = (int)( (double) angle * MRAD_TO_PER );
        }

        switch( meterType )
        {
            case TYPE_ROLL:
            case TYPE_ALTITUDE:
            case TYPE_THRUST:
                return( angle >= 0 && angle <= DEG_180 );
            case TYPE_PITCH:
                return( angle >= 0 && angle <= DEG_90 );
            case TYPE_YAW:
                return( true ); /* every angle is valid */
            default:
                return( false );
        }
    }

    public boolean isValidDesired( int angle )
    {
        switch( meterType )
        {
            case TYPE_ROLL:
            case TYPE_PITCH:
                return( angle >= -maximum && angle <= maximum );
            case TYPE_YAW:
            	return( angle >= -DEG_180 && angle <= DEG_180 );
            case TYPE_ALTITUDE:
            case TYPE_THRUST:
                return( angle >= 0 && angle <= maximum );
            default:
                return( false );
        }
    }

    public boolean isValidCurrent( int angle )
    {
        if( meterType == TYPE_ALTITUDE )
        {
            angle = (int)( (double) angle * MRAD_TO_MM );
        }
        else
        if( meterType == TYPE_THRUST )
        {
            angle = (int)( (double) angle * MRAD_TO_PER );
        }

        switch( meterType )
        {
            case TYPE_ROLL:
            case TYPE_YAW:
                return( angle >= -DEG_180 && angle <= DEG_180 );
            case TYPE_PITCH:
                return( angle >= -DEG_90 && angle <= DEG_90 );
            case TYPE_ALTITUDE:
            case TYPE_THRUST:
                return( angle >= 0 && angle <= DEG_180 );
            default:
                return( false );
        }
    }

    public void setMaximum( int maximum )
    {
        if( meterType != TYPE_YAW && /* no maximum needle in yaw meter */
            this.maximum != maximum && isValidMaximum( maximum ) )
        {
        	this.maximum = maximum;
        	redrawMeter  = true;

            if( desired < 0 && desired < -maximum )
            {
                setDesired( -maximum );
            }
            else
            if( desired > 0 && desired > maximum )
            {
                setDesired( maximum );
            }
        }
    }

    public void setDesired( int desired )
    {
        if( this.desired != desired && isValidDesired( desired ) )
        {
            this.desired = desired;        
            redrawMeter  = true; 
        }
    }

    public void setCurrent( int current )
    {
        if( this.current != current )
        {                           /* Do not check for validity, since this point */
        	this.current = current; /* represents heli data, and thus, must always */
        	redrawMeter  = true;    /* be accepted as it is, even if out of range. */
        }
    }

    public void update( Graphics g )
    {
        if( redrawMeter && g != null )
        {
            Graphics2D g2 = (Graphics2D) paintImage.getGraphics( );
            
            if( g2 != null )
            {
	            g2.setRenderingHint( RenderingHints.KEY_ANTIALIASING,
	                                 RenderingHints.VALUE_ANTIALIAS_ON );
	            g2.drawImage( meterImage, 0, 0, this );
				g2.setColor( colorDesired );
	
				if( meterType != TYPE_YAW )
				{
					rotateNeedle( needleMaximum, maximum );
					g2.drawLine( needleMaximum.xpoints[0],
	                             needleMaximum.ypoints[0],
	                             needleMaximum.xpoints[3],
	                             needleMaximum.ypoints[3] );
	
					if( meterType == TYPE_ROLL || meterType == TYPE_PITCH )
					{
					    g2.drawLine( needleOffsetW - needleMaximum.xpoints[0],
	                                 needleMaximum.ypoints[0],
	                                 needleOffsetW - needleMaximum.xpoints[3],
	                                 needleMaximum.ypoints[3] );
					}
				}
	
	            rotateNeedle( needleDesired, desired );
	            g2.setColor( colorDesired );
	            g2.fillPolygon( needleDesired );
	            g2.setColor( colorOutline );
	            g2.drawPolygon( needleDesired );
	
	            rotateNeedle( needleCurrent, current );
	            g2.setColor( colorCurrent );
	            g2.fillPolygon( needleCurrent );
	            g2.setColor( colorOutline );
	            g2.drawPolygon( needleCurrent );
	
	            g.drawImage( paintImage, 0, 0, this );
	            redrawMeter = false;
	            g2.dispose( );
            }
        }
    }

    public void setGrayed( boolean grayed )
    {
        Graphics g = getGraphics( );

        if( g != null )
        {
	        if( grayed )
		    {
		        colorDesired = Color.LIGHT_GRAY;
		        colorCurrent = Color.GRAY;
		        colorOutline = Color.DARK_GRAY;
		    }
		    else
		    {
		        colorDesired = Color.GREEN;
		        colorCurrent = Color.RED;
		        colorOutline = Color.BLACK;
		    }
		
		    redrawMeter = true;
		    update( g );
	        g.dispose( );
        }
    }

    public void paint( Graphics g )
    {
        redrawMeter = true;
        update( g );
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

    private static final int   PAINT_DELAY    = 50; /* ms */
    private static final int   N_POINTS       = 6;

    private static PaintThread paintThread    = null;
    private AnalogMeter        nextMeter      = null;
    private String             meterName      = null;
    private Image              paintImage     = null;
    private Image              meterImage     = null;
    private AffineTransform    transformation = null;
    private Polygon            needleMaximum  = null;
    private Polygon            needleDesired  = null;
    private Polygon            needleCurrent  = null;
    private double[]           srcPoints      = null;
    private double[]           dstPoints      = null;
    private Color              colorDesired   = null;
    private Color              colorCurrent   = null;
    private Color              colorOutline   = null;
    private int                needleOriginX  = 0;
    private int                needleOriginY  = 0;
    private int                needleOffsetW  = 0;
    private int                maximum        = 0;
    private int                desired        = 0;
    private int                current        = 0;
    private byte               meterType      = 0;
    private boolean            redrawMeter    = true;

    private void initWindow( boolean smallMeter, boolean smallFont, byte meterType )
    {
        switch( meterType )
        {
            case TYPE_ROLL:
            	meterName = "roll";
                break;
            case TYPE_PITCH:
            	meterName = "pitch";
                break;
            case TYPE_YAW:
            	meterName = "yaw";
                break;
            case TYPE_ALTITUDE:
            	meterName = "altitude";
                break;
            case TYPE_THRUST:
            	meterName = "thrust";
                break;
            default:
                setIgnoreRepaint( true );
                return;
        }

        this.meterType = meterType;

        Image tmpImage;

        try
        {
        	tmpImage = getImage( (smallFont ? "small_" : "large_") + meterName + ".png" );
        }
        catch( Exception e )
        {
            setIgnoreRepaint( true );
            System.err.println( "AnalogMeter.initWindow: " + e.getMessage( ) );
            return;
        }

        MediaTracker tracker = new MediaTracker( this );
        tracker.addImage( tmpImage, 0 );

        try
        {
            tracker.waitForID( 0 );

            if( tracker.isErrorID( 0 ) )
            {
                setIgnoreRepaint( true );
                System.err.println( "AnalogMeter.initWindow: tracker.isErrorID" );
                return;
            }
        }
        catch( Exception e )
        {
            setIgnoreRepaint( true );
            System.err.println( "AnalogMeter.initWindow: " + e.getMessage( ) );
            return;
        }

        GraphicsConfiguration gc = GraphicsEnvironment.getLocalGraphicsEnvironment( ).
            getDefaultScreenDevice( ).getDefaultConfiguration( );
        BufferedImage bufImage = gc.createCompatibleImage( tmpImage.getWidth( this ),
            tmpImage.getHeight( this ) );
        Graphics2D g2 = bufImage.createGraphics( );

    	g2.setXORMode( Color.BLACK );
		g2.drawImage( tmpImage, 0, 0, this );
    	g2.setXORMode( Color.WHITE );
        g2.setColor( UIManagerColor.getButtonBackground( ) );
        g2.fillRect( 0, 0, bufImage.getWidth( ), bufImage.getHeight( ) );
		g2.dispose( );

        double factor = smallMeter ? 0.375 : 0.5;
        int w = (int)( factor * bufImage.getWidth( ) );
        int h = (int)( factor * bufImage.getHeight( ) );
        paintImage = gc.createCompatibleImage( w, h );
        meterImage = gc.createCompatibleImage( w, h );
        meterImage.getGraphics( ).drawImage(
            bufImage.getScaledInstance( w, h, Image.SCALE_SMOOTH ), 0, 0, this );

        int[] xpoints = new int[ N_POINTS ];
        int[] ypoints = new int[ N_POINTS ];
		needleOriginX = w / 2;
		needleOffsetW = w - 1;

        if( smallMeter )
        {
    		needleOriginY = needleOriginX - 8;

            if( meterType == TYPE_ALTITUDE || meterType == TYPE_THRUST )
            {
                xpoints[0] = needleOriginX + 4;
                ypoints[0] = needleOriginY;
                xpoints[1] = needleOriginX + 2;
                ypoints[1] = needleOriginY - 2;
                xpoints[2] = needleOriginX - 92;
                ypoints[2] = needleOriginY - 2;
                xpoints[3] = needleOriginX - 101;
                ypoints[3] = needleOriginY;
                xpoints[4] = needleOriginX - 92;
                ypoints[4] = needleOriginY + 2;
                xpoints[5] = needleOriginX + 2;
                ypoints[5] = needleOriginY + 2;
            }
            else
            {
                xpoints[0] = needleOriginX;
                ypoints[0] = needleOriginY + 4;
                xpoints[1] = needleOriginX + 2;
                ypoints[1] = needleOriginY + 2;
                xpoints[2] = needleOriginX + 2;
                ypoints[2] = needleOriginY - 92;
                xpoints[3] = needleOriginX;
                ypoints[3] = needleOriginY - 101;
                xpoints[4] = needleOriginX - 2;
                ypoints[4] = needleOriginY - 92;
                xpoints[5] = needleOriginX - 2;
                ypoints[5] = needleOriginY + 2;
            }
        }
        else
        {
    		needleOriginY = needleOriginX - 10;

            if( meterType == TYPE_ALTITUDE || meterType == TYPE_THRUST )
            {
                xpoints[0] = needleOriginX + 4;
                ypoints[0] = needleOriginY;
                xpoints[1] = needleOriginX + 3;
                ypoints[1] = needleOriginY - 3;
                xpoints[2] = needleOriginX - 126;
                ypoints[2] = needleOriginY - 3;
                xpoints[3] = needleOriginX - 135;
                ypoints[3] = needleOriginY;
                xpoints[4] = needleOriginX - 126;
                ypoints[4] = needleOriginY + 3;
                xpoints[5] = needleOriginX + 3;
                ypoints[5] = needleOriginY + 3;
            }
            else
            {
                xpoints[0] = needleOriginX;
                ypoints[0] = needleOriginY + 4;
                xpoints[1] = needleOriginX + 3;
                ypoints[1] = needleOriginY + 3;
                xpoints[2] = needleOriginX + 3;
                ypoints[2] = needleOriginY - 126;
                xpoints[3] = needleOriginX;
                ypoints[3] = needleOriginY - 135;
                xpoints[4] = needleOriginX - 3;
                ypoints[4] = needleOriginY - 126;
                xpoints[5] = needleOriginX - 3;
                ypoints[5] = needleOriginY + 3;
            }
        }

    	paintThread    = new PaintThread( );
        transformation = new AffineTransform( );
        needleMaximum  = new Polygon( xpoints, ypoints, N_POINTS );
        needleDesired  = new Polygon( xpoints, ypoints, N_POINTS );
        needleCurrent  = new Polygon( xpoints, ypoints, N_POINTS );
        srcPoints      = new double[ 2 * N_POINTS ];
        dstPoints      = new double[ 2 * N_POINTS ];

        for( int i = 0, j = 0; j < N_POINTS; i += 2, ++j )
        {
            srcPoints[i]   = xpoints[j];
            srcPoints[i+1] = ypoints[j];
        }

        colorDesired = Color.GREEN;
        colorCurrent = Color.RED;
        colorOutline = Color.BLACK;

        setSize( meterImage.getWidth( this ), meterImage.getHeight( this ) );
        //setBackground( UIManagerColor.getButtonBackground( ) );
        //setForeground( UIManagerColor.getButtonForeground( ) );
        setFocusable( false );

        paintThread.addMeter( this );
        paintThread.start( );
    }

    private void rotateNeedle( Polygon needle, double angle )
    {
        if( needle != null && transformation != null )
        {
	        if( meterType == TYPE_ALTITUDE )
	        {
	            angle *= MRAD_TO_MM;
	        }
	        else
            if( meterType == TYPE_THRUST )
	        {
	            angle *= MRAD_TO_PER;
	        }
	
	        transformation.setToRotation( angle / 1000, needleOriginX, needleOriginY );
	        transformation.transform( srcPoints, 0, dstPoints, 0, N_POINTS );
	
	        for( int i = 0, j = 0; i < N_POINTS; ++i, j += 2 )
	        {
	            needle.xpoints[i] = (int) dstPoints[j];
	            needle.ypoints[i] = (int) dstPoints[j+1];
	        }
        }
    }

    private AnalogMeter getNextMeter( )
    {    	
    	return( nextMeter );
    }
    
    private void setNextMeter( AnalogMeter meter )
    {
    	nextMeter = meter;
    }
    
    /*************************************************************************/
    /*                                                                       */
    /* Class PaintThread                                                     */
    /*                                                                       */
    /*************************************************************************/

    private class PaintThread extends Thread
    {
        public PaintThread( )
        {
        }

    	public void run( )
    	{
    		AnalogMeter meter;
    		Graphics    g;

    		while( true )
    		{
				meter = list;

				while( meter != null )
				{
                    g = meter.getGraphics( );

                    if( g != null )
                    {
                    	meter.update( g );
                    	g.dispose( );
                    }

                    meter = meter.getNextMeter( );
				}

				try
				{
					Thread.sleep( PAINT_DELAY );
				}
				catch( InterruptedException e )
				{
                    System.err.println( "Painter.run: " + e.getMessage( ) );
                    break;
				}
    		}
    	}

    	public void addMeter( AnalogMeter meter )
    	{
    		meter.setNextMeter( list );
    		list = meter;
    	}

    	private AnalogMeter list = null;
    }
}

/* End of file */