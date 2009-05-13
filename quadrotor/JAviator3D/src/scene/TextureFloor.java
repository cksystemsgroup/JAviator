/*--------------------------------------------------------------------------
 *
 * The code is part of JAviator project (http://javiator.cs.uni-salzburg.at)
 *
 *--------------------------------------------------------------------------
 * Date: 11-Apr-2007
 *--------------------------------------------------------------------------
 *
 * Copyright (c) 2006 The University of Salzburg.
 * All rights reserved. Permission is hereby granted, without written 
 * agreement and without license or royalty fees, to use, copy, modify, and 
 * distribute this software and its documentation for any purpose, provided 
 * that the above copyright notice and the following two paragraphs appear 
 * in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF SALZBURG BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * THE UNIVERSITY OF SALZBURG HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.

 * THE UNIVERSITY OF SALZBURG SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * SALZBURG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 *
 *--------------------------------------------------------------------------
 *Created by Craciunas Silviu (silviu.craciunas@cs.uni-salzburg.at)
 *--------------------------------------------------------------------------
 */
package scene;

import java.awt.Component;

import javax.media.j3d.Appearance;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.ImageComponent2D;
import javax.media.j3d.Material;
import javax.media.j3d.Shape3D;
import javax.media.j3d.Texture2D;
import javax.media.j3d.TextureAttributes;
import javax.media.j3d.Transform3D;
import javax.vecmath.Color4f;
import javax.vecmath.Point3f;

import com.sun.j3d.utils.geometry.GeometryInfo;
import com.sun.j3d.utils.geometry.NormalGenerator;
import com.sun.j3d.utils.image.TextureLoader;

public class TextureFloor extends BranchGroup
{

    private static Point3f A = new Point3f( -10.0f, 0.0f, -10.0f );
    private static Point3f B = new Point3f( -10.0f, 0.0f, 10.0f );
    private static Point3f C = new Point3f( 10.0f, 0.0f, 10.0f );
    private static Point3f D = new Point3f( 10.0f, 0.0f, -10.0f );

    private String         filename;

    public TextureFloor( String filename )
    {
        
        Shape3D floor = new Shape3D( );
        Point3f[] pts = new Point3f[8];
        //      front
        pts[0] = C;
        pts[1] = D;
        pts[2] = A;
        pts[3] = B;
        //     back
        pts[4] = C;
        pts[5] = B;
        pts[6] = A;
        pts[7] = D;
        int[] stripCounts = new int[2];
        stripCounts[0] = 4;
        stripCounts[1] = 4;

        int[] contourCount = new int[2];
        contourCount[0] = 1;
        contourCount[1] = 1;

        GeometryInfo gInf = new GeometryInfo( GeometryInfo.POLYGON_ARRAY );

        gInf.setCoordinates( pts );
        gInf.setStripCounts( stripCounts );
        gInf.setContourCounts( contourCount );

        NormalGenerator ng = new NormalGenerator( );
        ng.setCreaseAngle( (float) Math.toRadians( 30 ) );
        ng.generateNormals( gInf );
        floor.setGeometry( gInf.getGeometryArray( ) );
        
        this.filename = filename;

        //floor.setCapability( Shape3D.ALLOW_APPEARANCE_READ );
        //floor.setCapability( Shape3D.ALLOW_APPEARANCE_WRITE );
        
        floor.setAppearance( texture( filename ) );

        floor.setCollidable( true );
        this.addChild( floor );
    }

    public Appearance texture( String s )
    {
        Component obs = new Component( )
        {

            /**
             * 
             */
            private static final long serialVersionUID = 1L;

        };
        TextureLoader loader = new TextureLoader( s, obs );
        ImageComponent2D image = loader.getImage( );
        Texture2D texture = new Texture2D( );
        int form = texture.getFormat( );
        int base = texture.getMipMapMode( );
        texture = new Texture2D( base, form, 1024, 1024 );
        texture.setImage( 0, image );
        Appearance appear = new Appearance( );
        appear.setTexture( texture );
        TextureAttributes texat = new TextureAttributes( TextureAttributes.REPLACE , new Transform3D( ), new Color4f( ), TextureAttributes.NICEST );
        appear.setTextureAttributes( texat );
        appear.setMaterial( new Material( ) );
        return ( appear );
    }

    public String getFilename( )
    {
        return filename;
    }

    public void setFilename( String filename )
    {
        this.filename = filename;
    }

}
