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

import java.awt.Color;
import java.io.FileNotFoundException;
import java.util.Hashtable;

import javax.media.j3d.Appearance;
import javax.media.j3d.BoundingSphere;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.DirectionalLight;
import javax.media.j3d.Material;
import javax.media.j3d.Shape3D;
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
public class JAviatorModel extends BranchGroup implements Model
{

    private Point3f              location;

    private float                roll;

    private float                pitch;

    private float                yaw;

    private TransformGroup       objTrans;

    private TransformGroup       propTrans1;

    private TransformGroup       propTrans2;

    private TransformGroup       propTrans3;

    private TransformGroup       propTrans4;

    private Transform3D          trans         = new Transform3D( );

    private Transform3D          trans1        = new Transform3D( );

    private Transform3D          rotX          = new Transform3D( );

    private Transform3D          rotY          = new Transform3D( );

    private Transform3D          rotZ          = new Transform3D( );

    private TransformGroup       prop1         = new TransformGroup( );

    private TransformGroup       prop2         = new TransformGroup( );

    private TransformGroup       prop3         = new TransformGroup( );

    private TransformGroup       prop4         = new TransformGroup( );

    private TransformGroup       body          = new TransformGroup( );

    private TransformGroup       frame1        = new TransformGroup( );

    private TransformGroup       frame2        = new TransformGroup( );

    private String               filename      = "models/javiator5.3ds";

    private String               prop_filename = "models/prop.3ds";

    private String               name;

    private static final Color3f SPECULAR      = new Color3f( Color.WHITE );

    private static final Color3f AMBIENT       = new Color3f( Color.LIGHT_GRAY );

    private static final Color3f EMISSIVE      = new Color3f( Color.BLACK );
    
    private RigidBody javiatorPhysicalModel = new RigidBody(Constants.JAVIATOR_THRUST,Constants.JAVIATOR_MASS);

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
    public JAviatorModel( String name, Point3f location, String filename )
    {
        this.location = location;
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

        propTrans1 = new TransformGroup( );
        propTrans2 = new TransformGroup( );
        propTrans3 = new TransformGroup( );
        propTrans4 = new TransformGroup( );

        propTrans1.setCapability( TransformGroup.ALLOW_TRANSFORM_WRITE );
        propTrans1.setCapability( TransformGroup.ALLOW_TRANSFORM_READ );
        propTrans2.setCapability( TransformGroup.ALLOW_TRANSFORM_WRITE );
        propTrans2.setCapability( TransformGroup.ALLOW_TRANSFORM_READ );
        propTrans3.setCapability( TransformGroup.ALLOW_TRANSFORM_WRITE );
        propTrans3.setCapability( TransformGroup.ALLOW_TRANSFORM_READ );
        propTrans4.setCapability( TransformGroup.ALLOW_TRANSFORM_WRITE );
        propTrans4.setCapability( TransformGroup.ALLOW_TRANSFORM_READ );

        try
        {
            Loader3DS loader = new Loader3DS( );
            // optional options to be used
            // loader.setLogging(true); // turns on writing a log file
            // loader.setDetail(7); // sets level of detail of report log
            loader.setTextureLightingOn( ); // turns on texture modulate
            // mode
            // loader.setTexturePath(""); // optional alternate
            // path to find texture files
            loader.noTextures( ); // if you do not want to load textures

            Scene theScene = loader.load( filename );
            Scene theScene2 = loader.load( prop_filename );

            Hashtable<String,TransformGroup> nameTable = theScene.getNamedObjects( );
            BranchGroup root = theScene.getSceneGroup( );
            Hashtable<String,TransformGroup> nameTable2 = theScene2.getNamedObjects( );
            BranchGroup root2 = theScene2.getSceneGroup( );

            root.removeAllChildren( );
            root2.removeAllChildren( );

            prop1 = nameTable2.get( "prop1" );
            Shape3D prop1Shape = (Shape3D) prop1.getChild( 0 );
            prop1Shape.setAppearance( getAppearance( Colors.darkGrey ) );
            ( (BranchGroup) prop1.getParent( ) ).removeAllChildren( );

            prop1.setCapability( TransformGroup.ALLOW_TRANSFORM_WRITE );
            prop1.setCapability( TransformGroup.ALLOW_TRANSFORM_READ );

            prop2 = (TransformGroup) prop1.cloneTree( );
            prop3 = (TransformGroup) prop1.cloneTree( );
            prop4 = (TransformGroup) prop1.cloneTree( );

            body = nameTable.get( "body" );
            ( (BranchGroup) body.getParent( ) ).removeAllChildren( );
            Shape3D bodyShape = (Shape3D) body.getChild( 0 );
            bodyShape.setAppearance( getAppearance( Colors.darkGrey ) );
         
            
            frame1 = nameTable.get( "frame1" );
            ( (BranchGroup) frame1.getParent( ) ).removeAllChildren( );
            Shape3D frame1Shape = (Shape3D) frame1.getChild( 0 );
            frame1Shape.setAppearance( getAppearance( Colors.grey ) );

            frame2 = nameTable.get( "frame2" );
            ( (BranchGroup) frame2.getParent( ) ).removeAllChildren( );
            Shape3D frame2Shape = (Shape3D) frame2.getChild( 0 );
            frame2Shape.setAppearance( getAppearance( Colors.grey ) );

            propTrans1.addChild( prop1 );
            propTrans2.addChild( prop2 );
            propTrans3.addChild( prop3 );
            propTrans4.addChild( prop4 );
            objTrans.addChild( body );
            objTrans.addChild( frame2 );
            objTrans.addChild( frame1 );

        }
        catch( FileNotFoundException fnf )
        {
            // Couldn't find the file you requested - deal with it!
            // yes I am lazy
        }

        Transform3D pos1 = new Transform3D( );
        pos1.setTranslation( new Vector3f( 0.0f, 0.0f, 0.0f ) );

        objTrans.setTransform( pos1 );
        // left propeller
        propTrans1.setTransform( pos1 );
        // right propeller
        propTrans2.setTransform( pos1 );
        // front propeller
        propTrans3.setTransform( pos1 );
        // rear propeller
        propTrans4.setTransform( pos1 );

        trans.setTranslation( new Vector3f( location.getX( ), location.getY( ), location.getZ( ) ) );

        // if you want to scale it down
        trans.setScale( Constants.SCENE_SCALE );

        objTrans.setTransform( trans );
        trans.setTranslation( new Vector3f( location.getX( ) + Constants.JAVIATOR_DISPLACEMENT1, location.getY( ) + Constants.JAVIATOR_DISPLACEMENT2, location.getZ( ) ) );
        propTrans1.setTransform( trans );

        trans.setTranslation( new Vector3f( location.getX( ) - Constants.JAVIATOR_DISPLACEMENT1, location.getY( ) + Constants.JAVIATOR_DISPLACEMENT2, location.getZ( ) ) );
        propTrans2.setTransform( trans );

        trans.setTranslation( new Vector3f( location.getX( ), location.getY( ) + Constants.JAVIATOR_DISPLACEMENT2, location.getZ( ) + Constants.JAVIATOR_DISPLACEMENT1 ) );
        propTrans3.setTransform( trans );

        trans.setTranslation( new Vector3f( location.getX( ), location.getY( ) + Constants.JAVIATOR_DISPLACEMENT2, location.getZ( ) - Constants.JAVIATOR_DISPLACEMENT1 ) );
        propTrans4.setTransform( trans );

        // Create a white light that shines for 10m from the origin
        Color3f light1Color = Colors.white;
        BoundingSphere bounds = new BoundingSphere( new Point3d( 0.0, 0.0, 0.0 ), 10.0 );
        Vector3f light1Direction = new Vector3f( 4.0f, -7.0f, -12.0f );
        DirectionalLight light1 = new DirectionalLight( light1Color, light1Direction );
        light1.setInfluencingBounds( bounds );

        // add it all to the scene
        this.addChild( light1 );
        this.addChild( objTrans );
        this.addChild( propTrans1 );
        this.addChild( propTrans2 );
        this.addChild( propTrans3 );
        this.addChild( propTrans4 );

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

        prop1.getTransform( trans1 );
        rotY.rotY( right * Constants.JAVIATOR_ROTOR_MAX_SPIN / 500 * Constants.RPM_TO_RAD * period );
        trans1.mul( rotY );
        prop1.setTransform( trans1 );

        prop2.getTransform( trans );
        rotY.rotY( left * Constants.JAVIATOR_ROTOR_MAX_SPIN / 500 * Constants.RPM_TO_RAD * period );
        trans.mul( rotY );
        prop2.setTransform( trans );

        prop3.getTransform( trans );
        rotY.rotY( rear * Constants.JAVIATOR_ROTOR_MAX_SPIN / 500 * Constants.RPM_TO_RAD * period );
        trans.mul( rotY );
        prop3.setTransform( trans );

        prop4.getTransform( trans );
        rotY.rotY( front * Constants.JAVIATOR_ROTOR_MAX_SPIN / 500 * Constants.RPM_TO_RAD * period );
        trans.mul( rotY );
        prop4.setTransform( trans );
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

        rotX.rotX( this.pitch );
        rotY.rotY( this.yaw );
        rotZ.rotZ( this.roll );

        trans.setRotation( new AxisAngle4f( 1.0f, 1.0f, 1.0f, 0.0f ) );

        trans.mul( rotX );
        trans.mul( rotY );
        trans.mul( rotZ );

        trans.setTranslation( new Vector3f( location.getX( ), location.getY( ), location.getZ( ) ) );
        
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

        /*
         * x(t) position
         * P(t) linear momentum
         * v(t) velocity
         * 
         */
        
        //float F = Constants.JAVIATOR_THRUST;
        this.roll = roll;
        this.pitch = pitch;
        this.yaw = yaw;
        
        Point3f position = javiatorPhysicalModel.calculatePosition( oldPosition, roll, -pitch, -yaw, period );
        //position.setX( oldPosition.getX( ) - period* ( F * ( (float) Math.sin( roll ) * (float) Math.cos( yaw ) - (float) Math.sin( pitch ) * (float) Math.sin( yaw ) ) ) );
        //position.setZ( oldPosition.getZ( ) - period * ( F * ( (float) Math.sin( pitch ) * (float) Math.cos( yaw ) - (float) Math.sin( roll ) * (float) Math.sin( yaw ) ) ) );
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

        trans.setTranslation( new Vector3f( location.getX( ) - Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.cos( yaw ), 
            location.getY( ) + Constants.JAVIATOR_DISPLACEMENT2 - Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.sin( roll ), 
            location.getZ( ) + Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.sin( yaw ) ) );
        propTrans1.setTransform( trans );

        trans.setTranslation( new Vector3f( location.getX( ) + Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.cos( yaw ), 
            location.getY( ) + Constants.JAVIATOR_DISPLACEMENT2 + Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.sin( roll ), 
            location.getZ( ) - Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.sin( yaw ) ) );
        propTrans2.setTransform( trans );

        trans.setTranslation( new Vector3f( location.getX( ) + Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.sin( yaw ), 
            location.getY( ) + Constants.JAVIATOR_DISPLACEMENT2 - Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.sin( pitch ), 
            location.getZ( ) + Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.cos( yaw ) ) );
        propTrans3.setTransform( trans );

        trans.setTranslation( new Vector3f( location.getX( ) - Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.sin( yaw ), 
            location.getY( ) + Constants.JAVIATOR_DISPLACEMENT2 + Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.sin( pitch ), 
            location.getZ( ) - Constants.JAVIATOR_DISPLACEMENT1 * (float) Math.cos( yaw ) ) );
        propTrans4.setTransform( trans );
        
        

    }

    private static final Appearance getAppearance( Color3f color )
    {
        Appearance app = new Appearance( );
        app.setMaterial( new Material( AMBIENT, EMISSIVE, color, SPECULAR, 100F ) );
        return app;
    }

    public Point3f getLocation( )
    {
        return location;
    }

    public float getPitch( )
    {
        return pitch;
    }

    public float getRoll( )
    {
        return roll;
    }

    public float getYaw( )
    {
        return yaw;
    }

    public TransformGroup getBody( )
    {
        return body;
    }

    public String getFilename( )
    {
        return filename;
    }

    public TransformGroup getFrame1( )
    {
        return frame1;
    }

    public TransformGroup getFrame2( )
    {
        return frame2;
    }

    public String getName( )
    {
        return name;
    }

    public TransformGroup getObjTrans( )
    {
        return objTrans;
    }

    public TransformGroup getProp1( )
    {
        return prop1;
    }

    public TransformGroup getProp2( )
    {
        return prop2;
    }

    public TransformGroup getProp3( )
    {
        return prop3;
    }

    public TransformGroup getProp4( )
    {
        return prop4;
    }

    public Transform3D getRotX( )
    {
        return rotX;
    }

    public Transform3D getRotY( )
    {
        return rotY;
    }

    public Transform3D getRotZ( )
    {
        return rotZ;
    }

    public Transform3D getTrans( )
    {
        return trans;
    }

}
