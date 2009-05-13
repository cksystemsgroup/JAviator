
package model;

import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

public class RigidBody
{
    private double   mass;
    private Vector3f v;
    private Vector3f a;

    private Vector3f force;
    private float    U;

    private Vector3f momentum;

    /*
     * 
     */

    public RigidBody( float U, float mass )
    {

        this.U = U;
        this.mass = mass;
        momentum = new Vector3f( );
        v = new Vector3f( );
        a = new Vector3f( );
        force = new Vector3f( );

    }

    public Point3f calculatePosition( Point3f oldPosition, float roll, float pitch, float yaw, float timeStep )
    {

        Point3f newPosition = new Point3f( );
        calculateAcceleration( roll, pitch, yaw, timeStep );
        v.setX( a.getX( ) * timeStep );
        v.setY( a.getZ( ) * timeStep );
        newPosition.setX( (float) ( oldPosition.getX( ) - v.getX( ) * timeStep + 0.5 * a.getX( ) * timeStep * timeStep ) );
        newPosition.setZ( (float) ( oldPosition.getZ( ) + v.getZ( ) * timeStep + 0.5 * a.getZ( ) * timeStep * timeStep ) );
        System.out.println("Position X : " + newPosition.getX( ) + " m");
        System.out.println("Position Z : " + newPosition.getZ( ) + " m");

        return newPosition;
    }

    private void calculateAcceleration( float roll, float pitch, float yaw, float timeStep )
    {
        calculateForce( roll, pitch, yaw, timeStep );
        a.setX( (float) ( force.getX( ) / mass ) );
        a.setZ( (float) ( force.getZ( ) / mass ) );
        System.out.println("Acceleration X : " + force.getX( ) + " m/s2");
        System.out.println("Acceleration Z : " + force.getZ( ) + " m/s2");
    }

    private void calculateForce( float roll, float pitch, float yaw, float timeStep )
    {

        force.setX( (float) ( U * ( Math.sin( roll ) * Math.cos( yaw ) - Math.sin( pitch ) * Math.sin( yaw ) ) ) + momentum.getX( ) );
        force.setZ( (float) ( U * ( Math.sin( pitch ) * Math.cos( yaw ) - Math.sin( roll ) * Math.sin( yaw ) ) ) + momentum.getZ( ) );
        momentum.setX( force.getX( ) * timeStep );
        momentum.setZ( force.getZ( ) * timeStep );
        System.out.println("Force X : " + force.getX( ) + " N");
        System.out.println("Force Z : " + force.getZ( ) + " N");
        System.out.println("Momentum X : " + momentum.getX( ) + " kg*m/s");
        System.out.println("Momentum Z : " + momentum.getZ( ) + " kg*m/s");
    }

}
