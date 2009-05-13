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
 * Created by Craciunas Silviu (silviu.craciunas@cs.uni-salzburg.at)
 *--------------------------------------------------------------------------
 */

package model;

import java.io.FileNotFoundException;

import javax.media.j3d.BoundingSphere;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.DirectionalLight;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.vecmath.AxisAngle4f;
import javax.vecmath.Color3f;
import javax.vecmath.Point3d;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

import scene.Colors;
import simulation.Constants;

import com.mnstarfire.loaders3d.Loader3DS;
import com.sun.j3d.loaders.Scene;

/**
 * The JAviator model for a 3D simulation
 * 
 * @author scraciunas
 * 
 */
public class HelicopterModel extends BranchGroup implements Model
{

    private Point3f        location;

    private float          roll;

    private float          pitch;

    private float          yaw;

    private TransformGroup objTrans;

    private Transform3D    trans    = new Transform3D( );

    private Transform3D    rotX     = new Transform3D( );

    private Transform3D    rotY     = new Transform3D( );

    private Transform3D    rotZ     = new Transform3D( );

    private String         filename = "models/javiator.3ds";

    private String         name;

    /**
     * Constructor
     * 
     * @param name
     *            the name of this model instance
     * @param location
     *            the initial location
     * @param filename
     *            the filename of the 3ds model to render the JAviator
     * @param isDefault
     *            is the default model being used?
     */
    public HelicopterModel( String name, Point3f location, String filename )
    {
        this.location = location;
        if( filename != null )
            this.filename = filename;
        this.name = name;
        this.setCapability( TransformGroup.ENABLE_PICK_REPORTING );
        createModel( );
    }

    /**
     * create the graphical appearance of the model
     */
    @SuppressWarnings( "unchecked" ) private void createModel( )
    {
        objTrans = new TransformGroup( );
        objTrans.setName( this.name );
        objTrans.setCapability( TransformGroup.ALLOW_TRANSFORM_WRITE );
        objTrans.setCapability( TransformGroup.ALLOW_TRANSFORM_READ );

        try
        {
            Loader3DS loader = new Loader3DS( );
            loader.setTextureLightingOn( );
            loader.noTextures( );

            Scene theScene = loader.load( filename );
            objTrans.addChild( theScene.getSceneGroup( ) );

        }
        catch( FileNotFoundException fnf )
        {
            // Couldn't find the file you requested - deal with it!
            // yes I am lazy
        }

        Transform3D pos1 = new Transform3D( );
        pos1.setTranslation( new Vector3f( 0.0f, 0.0f, 0.0f ) );

        objTrans.setTransform( pos1 );

        trans.setTranslation( new Vector3f( location.getX( ), location.getY( ), location.getZ( ) ) );

        // if you want to scale it down
        trans.setScale( Constants.SCENE_SCALE );

        objTrans.setTransform( trans );
        Color3f light1Color = Colors.white;
        BoundingSphere bounds = new BoundingSphere( new Point3d( 0.0, 0.0, 0.0 ), 10.0 );
        Vector3f light1Direction = new Vector3f( 4.0f, -7.0f, -12.0f );
        DirectionalLight light1 = new DirectionalLight( light1Color, light1Direction );
        light1.setInfluencingBounds( bounds );

        // add it all to the scene
        this.addChild( light1 );
        this.addChild( objTrans );

        // let java3D do the optimizations
        this.compile( );
    }

    /**
     * @param front
     * @param rear
     * @param right
     * @param left
     */
    public void setRotorSpeed( short front, short rear, short right, short left, float period )
    {
    }

    /**
     * rotate the model according to the 3 angles in radians
     * 
     * @param roll
     * @param pitch
     * @param yaw
     */
    public void rotateModel( float roll, float pitch, float yaw )
    {
        this.roll = roll;
        this.pitch = pitch;
        this.yaw = yaw;

        rotX.rotX( -pitch );
        rotY.rotY( -yaw );
        rotZ.rotZ( roll );

        trans.setRotation( new AxisAngle4f( 1.0f, 1.0f, 1.0f, 0.0f ) );

        trans.mul( rotX );
        trans.mul( rotY );
        trans.mul( rotZ );

        objTrans.setTransform( trans );
    }

    /**
     * Calculate new XZ position according to previous location and the 3 angles
     * 
     * Equation of motion for a quadrator is : we only need XZ (in our world the
     * vertical axis is Y)
     * 
     * newXPosition = oldXPosition + timeFrame * Thrust * (sin(roll) * cos(yaw) -
     * sin(pitch) * sin(yaw)) newZPosition = oldZPosition + timeFrame * Thrust *
     * (sin(pitch) * cos(yaw) - sin(roll) * sin(yaw))
     * 
     * we assume there is no drag or air friction and the thrust is constant
     * 
     * @param oldPosition
     * @param pitch
     * @param roll
     * @param yaw
     * @return
     */
    public Point3f calculateLocation( Point3f oldPosition, float pitch, float roll, float yaw, float period )
    {

        float F = Constants.JAVIATOR_THRUST;
        this.roll = roll;
        this.pitch = pitch;
        this.yaw = yaw;
        Point3f position = new Point3f( );
        position.setX( oldPosition.getX( ) - period * ( F * ( 
                        (float) Math.sin( roll ) * (float) Math.cos( yaw ) - 
                        (float) Math.sin( pitch ) * (float) Math.sin( yaw ) ) ) );
        position.setZ( oldPosition.getZ( ) - period * ( F * ( 
                        (float) Math.sin( pitch ) * (float) Math.cos( yaw ) - 
                        (float) Math.sin( roll ) * (float) Math.sin( yaw ) ) ) );
        return position;

    }

    /**
     * move the model to specified point
     * 
     * @param locationd
     */
    public void moveModel( Point3f locationd )
    {
        this.location.setX( locationd.getX( ) );
        this.location.setY( locationd.getY( ) );
        this.location.setZ( locationd.getZ( ) );
        trans.setTranslation( new Vector3f( location.getX( ), location.getY( ), location.getZ( ) ) );
        objTrans.setTransform( trans );

    }

    public float getPitch( )
    {
        return pitch;
    }

    public void setPitch( float pitch )
    {
        this.pitch = pitch;
    }

    public float getRoll( )
    {
        return roll;
    }

    public void setRoll( float roll )
    {
        this.roll = roll;
    }

    public float getYaw( )
    {
        return yaw;
    }

    public void setYaw( float yaw )
    {
        this.yaw = yaw;
    }

    public Point3f getLocation( )
    {

        return location;
    }

}
