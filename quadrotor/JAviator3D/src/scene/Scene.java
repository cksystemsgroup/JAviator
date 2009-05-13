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

import java.awt.BorderLayout;
import java.awt.GraphicsConfiguration;
import java.awt.Panel;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;

import javax.media.j3d.Background;
import javax.media.j3d.BoundingBox;
import javax.media.j3d.BoundingSphere;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.Canvas3D;
import javax.media.j3d.DirectionalLight;
import javax.media.j3d.ImageComponent2D;
import javax.media.j3d.Light;
import javax.media.j3d.Node;
import javax.media.j3d.PointLight;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.media.j3d.TransparencyAttributes;
import javax.media.j3d.View;
import javax.vecmath.AxisAngle4f;
import javax.vecmath.Color3f;
import javax.vecmath.Point3d;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

import simulation.Constants;
import util.Configuration;

import com.sun.j3d.utils.behaviors.mouse.MouseRotate;
import com.sun.j3d.utils.behaviors.mouse.MouseTranslate;
import com.sun.j3d.utils.behaviors.mouse.MouseZoom;
import com.sun.j3d.utils.behaviors.vp.OrbitBehavior;
import com.sun.j3d.utils.image.TextureLoader;
import com.sun.j3d.utils.picking.PickCanvas;
import com.sun.j3d.utils.universe.SimpleUniverse;

/**
 * @author scraciunas
 *
 */
public class Scene extends Panel implements KeyListener
{

    private static final long      serialVersionUID   = 1L;

    private BoundingBox            bounds             = new BoundingBox( new Point3d( -1000.0, -1000.0, -1000.0 ), new Point3d( 1000.0, 1000.0, 1000.0 ) );

    private TransparencyAttributes tattr              = new TransparencyAttributes( );

    private BranchGroup            group;

    private Configuration          configuration      = new Configuration( );

    PickCanvas                     pickCanvas;

    boolean                        running            = false;

    boolean                        done               = true;

    private Canvas3D               canvas;
    
    private GraphicsConfiguration  config;
    
    private SimpleUniverse         universe;

    private TransformGroup         viewTrans;

    private boolean                follow             = false;

    private boolean                floor              = true;

    private Transform3D            transform3D        = new Transform3D( );

    private Transform3D            rotationTransformX = new Transform3D( );

    private Transform3D            rotationTransformY = new Transform3D( );

    private Transform3D            rotationTransformZ = new Transform3D( );
    
    //--------------- Scene objects ---------------------------//

    private Plane                  desiredAltitudePlane;

    private Plane                  desiredPositionPlane;

    private CheckBoardFloor        floorPlane;

    private Background             background;

    private SceneFog               sceneFog;


    /**
     * 
     */
    public Scene( )
    {
        // do the initial config stuff
        config = SimpleUniverse.getPreferredConfiguration( );
        config.getBounds( ).setBounds( -10000, -10000, 20000, 20000 );
        // our canvas
        canvas = new Canvas3D( config );
        canvas.setBounds( -1000, -10000, 20000, 20000 );
        // create the universe
        universe = new SimpleUniverse( canvas );
        View view = universe.getViewer( ).getView( );
        view.setProjectionPolicy( View.PERSPECTIVE_PROJECTION );
        
        //set view platform and camera position
        universe.getViewingPlatform( ).setNominalViewingTransform( );
        viewTrans = universe.getViewingPlatform( ).getViewPlatformTransform( );

        // do we have transparency?
        tattr.setTransparencyMode( TransparencyAttributes.NICEST );
        tattr.setTransparency( configuration.getTransparecyValue( ) );



        // add the mouse behavior
        if( configuration.getMouseBehavior( ) == Constants.MOUSE_BEHAVIOR_ORBITAL )
        {
            OrbitBehavior orbit = new OrbitBehavior( canvas, OrbitBehavior.REVERSE_ALL );
            orbit.setSchedulingBounds( bounds );
            universe.getViewingPlatform( ).setViewPlatformBehavior( orbit );
        }
        else
        {
            addMouseBehavior( viewTrans, group );
        }

        this.setLayout( new BorderLayout( ) );
        this.add( "Center", canvas );

        //view.addCanvas3D( canvas );

        view.getCanvas3D( 0 ).addKeyListener( this );
        universe.getViewingPlatform( ).setNominalViewingTransform( );

        Transform3D posView = new Transform3D( );
        posView.setTranslation( new Vector3f( 0.0f, Constants.CAMERA_VIEWING_DISTANCE_Z, Constants.CAMERA_VIEWING_DISTANCE_Y ) );
        viewTrans.setTransform( posView );        
        //create the scene
        group = createScene( );

        universe.addBranchGraph( group );

        this.addKeyListener( this );

    }

    /**
     * @return
     */
    private Background createBackground( )
    {

        Background back = new Background( );
        back.setName( Constants.BACKGROUND_NAME );
        back.setApplicationBounds( bounds );
        TextureLoader myLoader = new TextureLoader( configuration.getBackgroundImage( ), this );
        ImageComponent2D myImage = myLoader.getImage( );
        back.setImage( myImage );
        return back;
    }

    /**
     * @param color
     * @param vec
     * @return
     */
    public Light createLight( Color3f color, Vector3f vec )
    {

        DirectionalLight light = new DirectionalLight( color, vec );
        light.setInfluencingBounds( new BoundingSphere( new Point3d( ), 100.0 ) );

        return light;
    }

    /**
     * @param group
     */
    private void addLights( BranchGroup group )
    {

        Light light1 = createLight( new Color3f( 1f, 1f, 1f ), new Vector3f( 0.5f, -0.5f, -0.7f ) );

        group.addChild( light1 );

        Light light2 = createLight( new Color3f( 1f, 1f, 1f ), new Vector3f( -0.5f, -0.5f, -0.7f ) );
        group.addChild( light2 );

        PointLight sunLight = new PointLight( new Color3f( 0.9f, 0.9f, 0.9f ), new Point3f( 0.0f, 1.0f, 1.0f ), new Point3f( 1.5f, 0.0f, 0.0f ) );
        BoundingSphere inflZone = new BoundingSphere( new Point3d( 0.0f, 0.0f, 0.0f ), 100.0f );
        sunLight.setInfluencingBounds( inflZone );
        group.addChild( sunLight );

    }

    public void setDesiredAltitude( float x, float y, float z )
    {
        Transform3D desiredAltitudeTransform = new Transform3D( );
        desiredAltitudeTransform.setTranslation( new Vector3f( x, y, z ) );
        desiredAltitudePlane.setTransform( desiredAltitudeTransform );
    }

    public void setDesiredPosition( float x, float y, float z, float roll, float pitch, float yaw )
    {

        rotationTransformX.rotX( pitch );
        rotationTransformY.rotY( yaw );
        rotationTransformZ.rotZ( -roll );

        transform3D.setRotation( new AxisAngle4f( 1.0f, 1.0f, 1.0f, 0.0f ) );
        transform3D.setTranslation( new Vector3f( x, y, z ) );

        transform3D.mul( rotationTransformX );
        transform3D.mul( rotationTransformY );
        transform3D.mul( rotationTransformZ );

        desiredPositionPlane.setTransform( transform3D );

    }

    /**
     * @return
     */
    private BranchGroup createScene( )
    {
        BranchGroup group = new BranchGroup( );
        group.setName( Constants.SCENE_NAME );

        addLights( group );

        // do we want fog?
        if( configuration.getFog( ) == 1 )
        {
            sceneFog = new SceneFog( Colors.white, 0.05f, bounds );
            group.addChild( sceneFog );
        }

        // create some background
        background = createBackground( );

        desiredAltitudePlane = new Plane( 1f, 0f, 0f );
        desiredPositionPlane = new Plane( 0f, 1f, 0f );
        floorPlane = new CheckBoardFloor( );
        //set capabilities to extend scene
        group.setCapability( BranchGroup.ALLOW_CHILDREN_EXTEND );
        group.setCapability( BranchGroup.ALLOW_CHILDREN_READ );
        group.setCapability( BranchGroup.ALLOW_CHILDREN_WRITE );
        group.setCapability( BranchGroup.ENABLE_PICK_REPORTING );

        group.addChild( desiredPositionPlane );
        group.addChild( desiredAltitudePlane );
        group.addChild( background );
        group.addChild( floorPlane );

        // add the axis
        if( configuration.getAxis( ) == 1 )
        {
            group.addChild( new Axis( ) );
        }

        group.compile( );

        return group;
    }

    /**
     * @param node
     */
    public void addGroupToScene( Node node )
    {
        this.group.addChild( node );
    }

    /**
     * @param name
     * @return
     */
    public Node getSceneNode( String name )
    {
        for( int groupIndex = 0; groupIndex < group.numChildren( ); groupIndex++ )
        {
            Node groupNode = group.getChild( groupIndex );
            if( groupNode.getName( ) != null && groupNode.getName( ).equals( name ) )
                return groupNode;
        }
        return null;
    }

    /**
     * @param viewTrans
     * @param group
     */
    private void addMouseBehavior( TransformGroup viewTrans, BranchGroup group )
    {

        MouseRotate behavior1 = new MouseRotate( viewTrans );
        group.addChild( behavior1 );
        behavior1.setSchedulingBounds( bounds );

        MouseZoom behavior2 = new MouseZoom( viewTrans );
        group.addChild( behavior2 );
        behavior2.setSchedulingBounds( bounds );

        MouseTranslate behavior3 = new MouseTranslate( viewTrans );
        group.addChild( behavior3 );
        behavior3.setSchedulingBounds( bounds );

    }

    public TransformGroup getViewTrans( )
    {
        return viewTrans;
    }

    public void setViewTrans( TransformGroup viewTrans )
    {
        this.viewTrans = viewTrans;
    }

    public void keyPressed( KeyEvent arg0 )
    {
        if( arg0.getKeyChar( ) == 'c' )
        {
            follow = !follow;
        }
        if( arg0.getKeyChar( ) == 'a' )
        {

            if( configuration.getAxis( ) == 1 )
            {
                configuration.setAxis( 0 );
                group.removeChild( getSceneNode( "Axes" ) );
            }
            else
            {
                configuration.setAxis( 1 );
                group.addChild( new Axis( ) );
            }

        }

        if( arg0.getKeyChar( ) == 'f' )
        {
            floor = !floor;
            if( floor )
            {
                group.addChild( new CheckBoardFloor( ) );
            }
            else
            {
                group.removeChild( getSceneNode( "Floor" ) );
            }
        }

    }

    public void keyReleased( KeyEvent arg0 )
    {
        // TODO Auto-generated method stub

    }

    public void keyTyped( KeyEvent arg0 )
    {
        // TODO Auto-generated method stub

    }

    public boolean isFollow( )
    {
        return follow;
    }

    public void setFollow( boolean follow )
    {
        this.follow = follow;
    }

}
