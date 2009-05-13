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

// ExtendedJoystick.cpp : Defines the entry point for the DLL application.
//
// original author: George Rhoten
// original date: 5/26/2000

#include "stdafx.h"
#include "com_centralnexus_input_ExtendedJoystick.h"

/*
 */
JNIEXPORT jboolean JNICALL Java_com_centralnexus_input_ExtendedJoystick_isPluggedIn
  (JNIEnv *, jclass, jint id)
{
    JOYINFOEX info;
    if (id < 0 || numDevices <= id) {
        return false;
    }

	info.dwSize = sizeof(JOYINFOEX);
    info.dwFlags = JOY_RETURNALL;
    return joyGetPosEx(id + JOYSTICKID1, &info) == JOYERR_NOERROR;
}
//#include <stdlib.h>
/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_ExtendedJoystick_poll
  (JNIEnv *env, jclass, jint id, jfloatArray axisArray)
{
    JOYINFOEX info;
    jfloat *axisValues = env->GetFloatArrayElements(axisArray, 0);

	info.dwSize = sizeof(JOYINFOEX);
    info.dwFlags = JOY_RETURNALL;
    joyGetPosEx(id + JOYSTICKID1, &info);

    axisValues[com_centralnexus_input_ExtendedJoystick_AXIS_X]
        = (info.dwXpos - HALF_AXIS) / HALF_AXIS; 
    axisValues[com_centralnexus_input_ExtendedJoystick_AXIS_Y]
        = (info.dwYpos - HALF_AXIS) / HALF_AXIS; 
    axisValues[com_centralnexus_input_ExtendedJoystick_AXIS_Z]
        = (info.dwRpos - HALF_AXIS) / HALF_AXIS;
    axisValues[com_centralnexus_input_ExtendedJoystick_AXIS_R]
        = (info.dwZpos - HALF_AXIS) / HALF_AXIS;
    axisValues[com_centralnexus_input_ExtendedJoystick_AXIS_U]
        = (info.dwUpos - HALF_AXIS) / HALF_AXIS;
    axisValues[com_centralnexus_input_ExtendedJoystick_AXIS_V]
        = (info.dwVpos - HALF_AXIS) / HALF_AXIS;
    axisValues[com_centralnexus_input_ExtendedJoystick_AXIS_POV]
        = (jfloat)((jshort)info.dwPOV) / 100.0f;
    env->ReleaseFloatArrayElements(axisArray, axisValues, 0);
//    printf("%X %X %X %X %X %X %X %d\n", info.dwFlags, info.dwXpos, info.dwYpos, info.dwZpos,
//        info.dwRpos, info.dwUpos, info.dwVpos, info.dwPOV);
    return info.dwButtons;

}
