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

import javax.media.j3d.Appearance;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.Material;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.media.j3d.TransparencyAttributes;
import javax.vecmath.Color3f;
import javax.vecmath.Vector3d;

import com.sun.j3d.utils.geometry.Box;
import com.sun.j3d.utils.geometry.Cone;
import com.sun.j3d.utils.geometry.Sphere;

/**
 * some test scene
 * @author scraciunas
 *
 */
public class TestScene extends BranchGroup
{

    TransparencyAttributes tattr            = new TransparencyAttributes( );

    float                  transparecyValue = 0.1f;

    private Appearance createAppearance( Color3f color )
    {
        Appearance ap = new Appearance( );
        ap.setCapability( Appearance.ALLOW_TRANSPARENCY_ATTRIBUTES_WRITE );

        Material mat = new Material( );
        mat.setDiffuseColor( color );
        mat.setSpecularColor( new Color3f( 1.0f, 1.0f, 1.0f ) );
        mat.setShininess( 45.3f );
        ap.setMaterial( mat );

        tattr.setCapability( TransparencyAttributes.ALLOW_MODE_WRITE );
        tattr.setCapability( TransparencyAttributes.ALLOW_VALUE_WRITE );

        ap.setTransparencyAttributes( tattr );

        return ap;
    }

    public TestScene( )
    {
        Transform3D t3d = new Transform3D( );
        t3d.rotX( Math.PI / 8.0 );
        Transform3D roty = new Transform3D( );
        roty.rotY( Math.PI / 4.0 );
        t3d.mul( roty );

        TransformGroup trans = new TransformGroup( t3d );
        this.addChild( trans );

        Appearance yapp = createAppearance( new Color3f( 1.0f, 1.0f, 0.0f ) );
        Appearance rapp = createAppearance( new Color3f( 1.0f, 0.0f, 0.0f ) );
        Appearance bapp = createAppearance( new Color3f( 0.0f, 0.0f, 1.0f ) );

        Transform3D bt3d = new Transform3D( );
        bt3d.set( new Vector3d( -0.22, -0.02, 0.22 ) );
        TransformGroup btrans = new TransformGroup( bt3d );
        btrans.addChild( new Box( 0.16f, 0.16f, 0.16f, yapp ) );
        trans.addChild( btrans );

        Transform3D ct3d = new Transform3D( );
        ct3d.set( new Vector3d( -0.2, 0.0, -0.2 ) );
        TransformGroup ctrans = new TransformGroup( ct3d );
        ctrans.addChild( new Cone( 0.2f, 0.4f, Cone.BODY | Cone.CAP, rapp ) );
        trans.addChild( ctrans );

        Transform3D st3d = new Transform3D( );
        st3d.set( new Vector3d( 0.2, 0.0, 0.2 ) );
        TransformGroup strans = new TransformGroup( st3d );
        strans.addChild( new Sphere( 0.2f, bapp ) );
        trans.addChild( strans );

    }
}
