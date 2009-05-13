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

/**
 * @author scraciunas
 *
 */
public final class Constants
{

    public static float  SCENE_SCALE                      = 1f;
    public static int    MOUSE_BEHAVIOR_ORBITAL           = 0;
    public static int    MOUSE_BEHAVIOR_CAMERA            = 1;
    public static float  CONTROLLER_PERIOD                = 0.02f;
    public static String BACKGROUND_NAME                  = "Background";
    public static String SCENE_NAME                       = "JAviator3D Scene";
    public static float  JAVIATOR_MASS                    = 1.7f;                                 //Masse [kg]
    public static float  JAVIATOR_LENGTH                  = 1.1f;                                 // Lange [m]
    public static float  JAVIATOR_ROTOR_LENGTH            = 0.2f;                                 //Länge eines Hauptrotorblattes [m]
    public static float  JAVIATOR_ROTOR_RADIUS            = 0.424f;                               // Radius des Rotors [m]
    public static float  JAVIATOR_MAX_ROTOR_Z             = 0.055f;                               // Maximale abweichung des rotos auf Z-axis [m]
    public static float  JAVIATOR_CONNECTION_PIPES_LENGTH = 0.232f;                               // Lange der pipes [m]
    public static float  JAVIATOR_BODY_PIPES_LENGTH       = 0.115f;
    public static float  JAVIATOR_ROTOR_MAX_SPIN          = 1800f;                                //Umdrehungen pro Minute des Hauptrotors (upmRot) [1/min]
    public static float  JAVIATOR_DISPLACEMENT1           = 0.59f;                                // LAnge von der mitte bis zum rotor[m]
    public static float  JAVIATOR_DISPLACEMENT2           = 0.1f;                                 // Hohe des rotors zum grund[m]
    public static float  JAVIATOR_THRUST                  = 400.0f;
    public static float  JAVIATOR_DRAG                    = 0.0f;

    public static float  AIR_RESISTANCE                   = 0.0f;

    public static float  RPM_TO_RAD                       = 2 * (float) Math.PI / 60;

    public static String copyText                         = "JAviator3D - University of Salzburg";

    public static float  CAMERA_VIEWING_DISTANCE_Z        = 1.0f;
    public static float  CAMERA_VIEWING_DISTANCE_Y        = 4.5f;
    
    public static float  CAMERA_VIEWING_HEIGHT            = 0.8f;

}
