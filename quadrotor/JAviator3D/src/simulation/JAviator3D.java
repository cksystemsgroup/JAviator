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

package simulation;

import java.applet.Applet;
import java.awt.BorderLayout;

import javax.media.j3d.Transform3D;

import javax.vecmath.Matrix3d;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

import com.sun.j3d.utils.applet.MainFrame;

import scene.Scene;
import model.JAviatorModel;

/**
 * @author scraciunas
 *
 */
public class JAviator3D
{

    private static Scene  scene;

    private JAviatorModel javiatorModel;

    public JAviator3D( )
    {
    }

    public void resetModel( )
    {
        javiatorModel.moveModel( new Point3f( ) );
        javiatorModel.rotateModel( 0.0f, 0.0f, 0.0f );
    }

    public void sendSensorData( float roll, float pitch, float yaw, float x, float y, float z, float desiredRoll, float desiredPitch, float desiredYaw, float desiredAltitude )
    {
     
        float helicopterX, helicopterY, helicopterZ, helicopterRoll, helicopterPitch, helicopterYaw;
        
        helicopterX = - y / 1000f;
        helicopterY = z / 1000f;
        helicopterZ = x /1000f;
        helicopterRoll = - roll / 1000f;
        helicopterPitch = pitch / 1000f;
        helicopterYaw = yaw / 1000f;
        
        if( z > 0 )
        {
            javiatorModel.rotateModel( helicopterRoll, helicopterPitch, helicopterYaw );
            javiatorModel.moveModel( new Point3f( helicopterX, helicopterY, helicopterZ ) );
            scene.setDesiredPosition( helicopterX, helicopterY + 0.1f, helicopterZ, desiredRoll/ 1000f, desiredPitch/ 1000f, desiredYaw/ 1000f );
            scene.setDesiredAltitude( helicopterX, helicopterY + 0.1f, helicopterZ );
            Transform3D posView = new Transform3D( );
            posView.getRotationScale( new Matrix3d() );
            if( scene.isFollow( ) )
            {
                posView.setTranslation( new Vector3f( helicopterX, Constants.CAMERA_VIEWING_HEIGHT, helicopterZ + Constants.CAMERA_VIEWING_DISTANCE_Y ) );
                scene.getViewTrans( ).setTransform( posView );
            }
        }
    }

    public void setRotorSpeed( short front, short rear, short right, short left )
    {
        javiatorModel.setRotorSpeed( front, rear, right, left, Constants.CONTROLLER_PERIOD );
    }

    /**
     * 
     */
    public void createModel( )
    {
        // create the JAviator
        javiatorModel = new JAviatorModel( "JAviatorTest", new Point3f( 0.0f, 0.0f, 0.0f ), "models/javiator5.3ds" );
        //javiatorModel.setPickable(true);
        scene = new Scene( );
        scene.addKeyListener( scene );
        //add it to the scene
        scene.addGroupToScene( javiatorModel );
     
        Applet a = new Applet();
        a.setLayout( new BorderLayout() );
        a.add("Center",scene); 
        MainFrame main = new MainFrame(a,500,500);
        
        main.setTitle( "JAviator3D" );
        main.setName("Javiator3D - University of Salzburg");
        
    }

}
