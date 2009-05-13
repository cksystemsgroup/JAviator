package scene;

import javax.media.j3d.Appearance;
import javax.media.j3d.Material;
import javax.media.j3d.PolygonAttributes;
import javax.media.j3d.Shape3D;
import javax.media.j3d.TransformGroup;
import javax.media.j3d.TransparencyAttributes;
import javax.vecmath.Color3f;
import javax.vecmath.Point3f;

import com.sun.j3d.utils.geometry.GeometryInfo;
import com.sun.j3d.utils.geometry.NormalGenerator;

public class Plane extends TransformGroup
{
    private static Point3f A = new Point3f( -1.0f, 0.0f, -1.0f );
    private static Point3f B = new Point3f( -1.0f, 0.0f, 1.0f );
    private static Point3f C = new Point3f( 1.0f, 0.0f, 1.0f );
    private static Point3f D = new Point3f( 1.0f, 0.0f, -1.0f );
    public Plane(float r, float g, float b){
        
        this.setCapability( TransformGroup.ALLOW_TRANSFORM_READ );
        this.setCapability( TransformGroup.ALLOW_TRANSFORM_WRITE );
        
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
        floor.setAppearance( createPlaneApp( r, g, b ) );
        this.addChild( floor );
    }
    
    private Appearance createPlaneApp( float r, float g, float b )
    {
        Appearance app = new Appearance( );
        Material mat = new Material( );
        mat.setDiffuseColor( new Color3f( r, g, b ) );
        mat.setSpecularColor( new Color3f( 0.0f, 0.0f, 0.0f ) ); // ¹õ
        app.setMaterial( mat );
        TransparencyAttributes tattr = new TransparencyAttributes( TransparencyAttributes.NICEST, 0.8f );
        app.setTransparencyAttributes( tattr );
        PolygonAttributes pattr = new PolygonAttributes( PolygonAttributes.POLYGON_FILL, PolygonAttributes.CULL_NONE, 0.0f, true );
        app.setPolygonAttributes( pattr );
        return app;
    }
}
