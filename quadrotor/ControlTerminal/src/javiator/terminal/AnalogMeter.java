/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   AnalogMeter.java   Constructs a panel meter representing either roll,   */
/*                      pitch, yaw, or altitude user and heli data.          */
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

import java.awt.Canvas;
import java.awt.Image;
import java.awt.Color;
import java.awt.Polygon;
import java.awt.MediaTracker;
import java.awt.Graphics;
import java.awt.Toolkit;

import java.awt.event.FocusEvent;

import java.awt.geom.AffineTransform;

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

    public static final int    DEG_45           = 785;  // mrad
    public static final int    DEG_90           = 1571; // mrad
    public static final int    DEG_135          = 2356; // mrad
    public static final int    DEG_180          = 3142; // mrad
    public static final int    DEG_360          = 6284; // mrad

    public static final byte   TYPE_ROLL        = 1;
    public static final byte   TYPE_PITCH       = 2;
    public static final byte   TYPE_YAW         = 3;
    public static final byte   TYPE_ALTITUDE    = 4;
    public static final byte   TYPE_THRUST      = 5;
    
    public AnalogMeter( boolean smallDisplay, byte meterType )
    {
        initWindow( smallDisplay, meterType );
    }

    public static Image getImage( String filename )
    {
        return Toolkit.getDefaultToolkit( ).createImage(
            AnalogMeter.class.getResource( "/img/" + filename ) );
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
        else if( meterType == TYPE_THRUST )
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
                return( true ); // every angle is valid
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
            angle = (int)( (double)angle * MRAD_TO_MM );
        }
        else if( meterType == TYPE_THRUST )
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
        if( meterType != TYPE_YAW && // no maximum needle in yaw meter
            this.maximum != maximum && isValidMaximum( maximum ) )
        {
        	this.maximum = maximum;
        	updateMaximum = true;

            if( desired < 0 && desired < -maximum )
            {
                setDesired( -maximum );
            }
            else if( desired > 0 && desired > maximum )
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
            updateDesired = true; 
        }        
    }

    public void setCurrent( int current )
    {
        if( this.current != current ) // Do not check for validity, since
        {                             // this value represents heli data,
        	this.current = current;   // and thus, must always be accepted
            updateCurrent = true;     // as it is, even if out of range.   
        }
    }

    public void update( Graphics g )
    {    	
        if( updateMaximum || updateDesired || updateCurrent )
        {
			g.drawImage( meterImage, 0, 0, this );
			g.setColor( colorDesired );

			if( meterType != TYPE_YAW )
			{
				rotateNeedle( needleMaximum, maximum );
				g.drawLine( needleMaximum.xpoints[0],
                            needleMaximum.ypoints[0],
                            needleMaximum.xpoints[3],
                            needleMaximum.ypoints[3] );

				if( meterType == TYPE_PITCH || meterType == TYPE_ROLL )
				{
				    g.drawLine( offsetW - needleMaximum.xpoints[0],
                                needleMaximum.ypoints[0],
                                offsetW - needleMaximum.xpoints[3],
                                needleMaximum.ypoints[3] );
				}
			}

            rotateNeedle( needleDesired, desired );
            g.setColor( colorDesired );
            g.fillPolygon( needleDesired );
            g.setColor( colorOutline );
            g.drawPolygon( needleDesired );
    
            rotateNeedle( needleCurrent, current );
            g.setColor( colorCurrent );
            g.fillPolygon( needleCurrent );
            g.setColor( colorOutline );
            g.drawPolygon( needleCurrent );
    
            updateMaximum = false;
            updateDesired = false;
            updateCurrent = false;
        }     
    }

    public void setGrayed( boolean grayed )
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

        paint( getGraphics( ) );
    }
    
    public void paint( Graphics g )
    {
        updateMaximum = true;
        updateDesired = true;
        updateCurrent = true;
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

    private static final int PAINT_DELAY    = 50;
    private static final int N_POINTS       = 6;

    private static Painter   painter        = null;

    private AnalogMeter       nextMeter      = null;
    private Image            meterImage     = null;
    private Color            colorDesired   = null;
    private Color            colorCurrent   = null;
    private Color            colorOutline   = null;
    private AffineTransform  transformation = null;
    private Polygon          needleMaximum  = null;
    private Polygon          needleDesired  = null;
    private Polygon          needleCurrent  = null;
    private double[]         srcPoints      = null;
    private double[]         dstPoints      = null;
    private int              originX        = 0;
    private int              originY        = 0;
    private int              offsetW        = 0;
    private int              maximum        = 0;
    private int              desired        = 0;
    private int              current        = 0;
    private byte             meterType      = 0;
    private boolean          updateMaximum  = true;
    private boolean          updateDesired  = true;
    private boolean          updateCurrent  = true;

    private void initWindow( boolean smallDisplay, byte meterType )
    {
        String name;

        switch( meterType )
        {
            case TYPE_ROLL:
                name = "roll";
                break;
            case TYPE_PITCH:
                name = "pitch";
                break;
            case TYPE_YAW:
                name = "yaw";
                break;
            case TYPE_ALTITUDE:
                name = "altitude";
                break;
            case TYPE_THRUST:
                name = "thrust";
                break;
            default:
                setIgnoreRepaint( true );
                return;
        }

        this.meterType = meterType;

        try
        {
            meterImage = getImage( (smallDisplay ? "small_" : "large_") + name + ".jpg" );
        }
        catch( Exception e )
        {
            setIgnoreRepaint( true );
            System.err.println( "AnalogMeter.initWindow: " + e.getMessage( ) );
            return;
        }

        MediaTracker tracker = new MediaTracker( this );
        tracker.addImage( meterImage, 0 );

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

        colorDesired   = Color.GREEN;
        colorCurrent   = Color.RED;
        colorOutline   = Color.BLACK;
        transformation = new AffineTransform( );
        int[] xpoints  = new int[N_POINTS];
        int[] ypoints  = new int[N_POINTS];

        if( smallDisplay )
        {
            originX = 150;
            originY = 143;
            offsetW = 299;

            if( meterType == TYPE_ALTITUDE || meterType == TYPE_THRUST )
            {
                xpoints[0] = originX + 4;
                ypoints[0] = originY;
                xpoints[1] = originX + 2;
                ypoints[1] = originY - 2;
                xpoints[2] = originX - 92;
                ypoints[2] = originY - 2;
                xpoints[3] = originX - 101;
                ypoints[3] = originY;
                xpoints[4] = originX - 92;
                ypoints[4] = originY + 2;
                xpoints[5] = originX + 2;
                ypoints[5] = originY + 2;
            }
            else
            {
                xpoints[0] = originX;
                ypoints[0] = originY + 4;
                xpoints[1] = originX + 2;
                ypoints[1] = originY + 2;
                xpoints[2] = originX + 2;
                ypoints[2] = originY - 92;
                xpoints[3] = originX;
                ypoints[3] = originY - 101;
                xpoints[4] = originX - 2;
                ypoints[4] = originY - 92;
                xpoints[5] = originX - 2;
                ypoints[5] = originY + 2;
            }
        }
        else
        {
            originX = 200;
            originY = 190;
            offsetW = 399;

            if( meterType == TYPE_ALTITUDE || meterType == TYPE_THRUST )
            {
                xpoints[0] = originX + 4;
                ypoints[0] = originY;
                xpoints[1] = originX + 3;
                ypoints[1] = originY - 3;
                xpoints[2] = originX - 126;
                ypoints[2] = originY - 3;
                xpoints[3] = originX - 135;
                ypoints[3] = originY;
                xpoints[4] = originX - 126;
                ypoints[4] = originY + 3;
                xpoints[5] = originX + 3;
                ypoints[5] = originY + 3;
            }
            else
            {
                xpoints[0] = originX;
                ypoints[0] = originY + 4;
                xpoints[1] = originX + 3;
                ypoints[1] = originY + 3;
                xpoints[2] = originX + 3;
                ypoints[2] = originY - 126;
                xpoints[3] = originX;
                ypoints[3] = originY - 135;
                xpoints[4] = originX - 3;
                ypoints[4] = originY - 126;
                xpoints[5] = originX - 3;
                ypoints[5] = originY + 3;
            }
        }

        needleMaximum = new Polygon( xpoints, ypoints, N_POINTS );
        needleDesired = new Polygon( xpoints, ypoints, N_POINTS );
        needleCurrent = new Polygon( xpoints, ypoints, N_POINTS );
        srcPoints     = new double[N_POINTS<<1];
        dstPoints     = new double[N_POINTS<<1];

        for( int i = 0, j = 0; j < N_POINTS; i += 2, ++j )
        {
            srcPoints[i]   = xpoints[j];
            srcPoints[i+1] = ypoints[j];
        }

        setSize( meterImage.getWidth( this ), meterImage.getHeight( this ) );
        setFocusable( false );

    	painter = new Painter( );
        painter.addMeter( this );
    }

    private void rotateNeedle( Polygon needle, double angle )
    {
        if( needle == null || transformation == null )
        {
            return;
        }

        if( meterType == TYPE_ALTITUDE )
        {
            angle *= MRAD_TO_MM;
        }
        else if( meterType == TYPE_THRUST )
        {
            angle *= MRAD_TO_PER;
        }

        transformation.setToRotation( angle / 1000, originX, originY );
        transformation.transform( srcPoints, 0, dstPoints, 0, N_POINTS );

        for( int i = 0, j = 0; i < N_POINTS; ++i, j += 2 )
        {
            needle.xpoints[i] = (int) dstPoints[j];
            needle.ypoints[i] = (int) dstPoints[j+1];
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
    /* Class Painter                                                         */
    /*                                                                       */
    /*************************************************************************/

    private class Painter extends Thread
    {
        public Painter( )
        {
            start( );
        }

    	public void run( )
    	{
    		AnalogMeter meter;
    		Graphics   graphics;

    		while( true )
    		{
				meter = list;

				while( meter != null )
				{
					graphics = meter.getGraphics( );

					if( graphics != null )
					{
						meter.update( graphics );
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

// End of file.