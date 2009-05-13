
package util;

import java.applet.*;
import java.awt.*;
import javax.media.j3d.*;
import javax.vecmath.*;
import com.sun.j3d.utils.universe.SimpleUniverse;
import com.sun.j3d.utils.universe.PlatformGeometry;
import com.sun.j3d.utils.behaviors.keyboard.*;

public class KeyNavigatorTest extends Applet
{
    /**
     * 
     */
    private static final long serialVersionUID = 1L;

    private SimpleUniverse    universe         = null;

    /**
     * 
     */
    public KeyNavigatorTest( )
    {
        GraphicsConfiguration config = SimpleUniverse.getPreferredConfiguration( );
        Canvas3D canvas = new Canvas3D( config );
        this.setLayout( new BorderLayout( ) );
        this.add( canvas, BorderLayout.CENTER );

        universe = new SimpleUniverse( canvas );
        universe.getViewingPlatform( ).setNominalViewingTransform( );
        universe.getViewer( ).getView( ).setBackClipDistance( 100.0 );
        BranchGroup scene = createSceneGraph( );

        universe.addBranchGraph( scene );
    }

    /**
     * @return
     */
    private BranchGroup createSceneGraph( )
    {
        BranchGroup root = new BranchGroup( );

        BoundingSphere bounds = new BoundingSphere( new Point3d( ), 100.0 );

        TransformGroup viewtrans = universe.getViewingPlatform( ).getViewPlatformTransform( );
        KeyNavigatorBehavior keybehavior = new KeyNavigatorBehavior( viewtrans );
        keybehavior.setSchedulingBounds( bounds );
        PlatformGeometry vp = new PlatformGeometry( );
        vp.addChild( keybehavior );
        universe.getViewingPlatform( ).setPlatformGeometry( vp );

        root.addChild( createFloor( ) );

        return root;
    }

    /**
     * @return
     */
    private BranchGroup createFloor( )
    {
        BranchGroup bg = new BranchGroup( );

        int roop = 100;

        Point3f[] vertices = new Point3f[roop * roop];

        float start = -100.0f;

        float x = start;
        float z = start;

        float step = 2.0f;

        int[] indices = new int[( roop - 1 ) * ( roop - 1 ) * 4];
        int n = 0;

        Color3f white = new Color3f( 1.0f, 1.0f, 1.0f );
        Color3f black = new Color3f( 0.0f, 0.0f, 0.0f );
        Color3f[] colors = { white, black };

        int[] colorindices = new int[indices.length];

        for( int i = 0; i < roop; i++ )
        {
            for( int j = 0; j < roop; j++ )
            {
                vertices[i * roop + j] = new Point3f( x, -2.0f, z );
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
        bg.addChild( floor );

        bg.compile( );

        return bg;
    }

}
