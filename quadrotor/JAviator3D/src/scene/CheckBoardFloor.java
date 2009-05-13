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

import javax.media.j3d.BranchGroup;
import javax.media.j3d.GeometryArray;
import javax.media.j3d.IndexedQuadArray;
import javax.media.j3d.Shape3D;
import javax.vecmath.Color3f;
import javax.vecmath.Point3f;

/**
 * The ChessBoard Floor 
 * 
 * @author scraciunas
 *
 */
public class CheckBoardFloor extends BranchGroup
{

    /**
     * 
     */
    public CheckBoardFloor( )
    {
        this.setCapability( BranchGroup.ALLOW_DETACH );
        generateFloor( );
        this.setName( "Floor" );

    }

    /**
     * 
     */
    private void generateFloor( )
    {

        int roop = 80;

        Point3f[] vertices = new Point3f[roop * roop];

        float start = -40.0f;

        float x = start;
        float z = start;

        float step = 10.0f;

        int[] indices = new int[( roop - 1 ) * ( roop - 1 ) * 4];
        int n = 0;

        Color3f[] colors = { Colors.white, Colors.black };

        int[] colorindices = new int[indices.length];

        for( int i = 0; i < roop; i++ )
        {
            for( int j = 0; j < roop; j++ )
            {
                vertices[i * roop + j] = new Point3f( x, -0.01f, z );

                z += step;
                if( i < ( roop - 1 ) && j < ( roop - 1 ) )
                {
                    int cindex = ( i % 2 + j ) % 2;
                    colorindices[n] = cindex;
                    indices[n++] = i * roop + j;
                    colorindices[n] = cindex;
                    indices[n++] = i * roop + ( j + 1 );
                    colorindices[n] = cindex;
                    indices[n++] = ( i + 1 ) * roop + ( j + 1 );
                    colorindices[n] = cindex;
                    indices[n++] = ( i + 1 ) * roop + j;
                }
            }
            z = start;
            x += step;
        }

        IndexedQuadArray geom = new IndexedQuadArray( vertices.length, GeometryArray.COORDINATES | GeometryArray.COLOR_3, indices.length );
        geom.setCoordinates( 0, vertices );
        geom.setCoordinateIndices( 0, indices );
        geom.setColors( 0, colors );

        geom.setColorIndices( 0, colorindices );

        Shape3D floor = new Shape3D( geom );
        floor.setCollidable( true );

        this.addChild( floor );

        this.compile( );

    }
}
