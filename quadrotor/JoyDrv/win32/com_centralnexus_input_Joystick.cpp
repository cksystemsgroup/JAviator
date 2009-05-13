/*
THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. NEITHER RECIPIENT NOR
ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING WITHOUT
LIMITATION LOST PROFITS), HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM
OR THE EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.

The name of the Copyright Holder may not be used to endorse or promote
products derived from this software without specific prior written permission.

Copyright 2000-2002 George Rhoten and others.

*/

// Joystick.cpp : Defines the entry point for the DLL application.
//
// original author: George Rhoten
// original date: 5/26/2000

#include "stdafx.h"
#include "com_centralnexus_input_Joystick.h"

/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_getNumDevices
  (JNIEnv *, jclass)
{
    return numDevices;
}

/*
 */
JNIEXPORT jboolean JNICALL Java_com_centralnexus_input_Joystick_isPluggedIn
  (JNIEnv *, jclass, jint id)
{
    JOYINFO info;
    if (id < 0 || numDevices <= id) {
        return false;
    }
    return joyGetPos(id + JOYSTICKID1, &info) == JOYERR_NOERROR;
}

/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_poll
  (JNIEnv *env, jclass, jint id, jfloatArray axisArray)
{
    JOYINFO info;
    jfloat *axisValues = env->GetFloatArrayElements(axisArray, 0);

    joyGetPos(id + JOYSTICKID1, &info);
    axisValues[com_centralnexus_input_Joystick_AXIS_X]
        = (info.wXpos - HALF_AXIS) / HALF_AXIS; 
    axisValues[com_centralnexus_input_Joystick_AXIS_Y]
        = (info.wYpos - HALF_AXIS) / HALF_AXIS; 
    axisValues[com_centralnexus_input_Joystick_AXIS_Z]
        = (info.wZpos - HALF_AXIS) / HALF_AXIS;
    env->ReleaseFloatArrayElements(axisArray, axisValues, 0);
    return info.wButtons;
}

/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_getCapabilities
  (JNIEnv *, jclass, jint id)
{
    JOYCAPS caps;
    if (joyGetDevCaps(id + JOYSTICKID1, &caps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
        return 0;

    return caps.wCaps;
}

/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_getNumButtons
  (JNIEnv *, jclass, jint id)
{
    JOYCAPS caps;
    if (joyGetDevCaps(id + JOYSTICKID1, &caps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
        return 0;

    return caps.wNumButtons;
}

/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_getNumAxes
  (JNIEnv *, jclass, jint id)
{
    JOYCAPS caps;
    if (joyGetDevCaps(id + JOYSTICKID1, &caps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
        return 0;

    return caps.wNumAxes;
}

/*
 */
JNIEXPORT jstring JNICALL Java_com_centralnexus_input_Joystick_toString
  (JNIEnv *env, jclass, jint id)
{
    JOYCAPS caps;
    if (joyGetDevCaps(id + JOYSTICKID1, &caps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
        return 0;

    return env->NewStringUTF(caps.szPname);
}

