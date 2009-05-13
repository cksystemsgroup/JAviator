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

Copyright 2002 George Rhoten, Daniel Kristjansson and others.

*/

#define JNIEXPORT
#define JNIIMPORT
#define JNICALL

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#include "com_centralnexus_input_Joystick.h"
#include "LinuxJoystick.h"

/*typedef unsigned int __u32;
typedef signed short __s16;
typedef unsigned char __u8;*/

/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_getNumDevices(JNIEnv *x, jclass y) {
  return calcNumDevices();
}

/*
 */
JNIEXPORT jboolean JNICALL Java_com_centralnexus_input_Joystick_isPluggedIn(JNIEnv *x, jclass y, jint id) {
  
  return (0<=id && id < calcNumDevices());
}


/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_poll(
    JNIEnv *env, jclass x, jint id, jfloatArray axisArray) {
  jfloat *axisValues = env->GetFloatArrayElements(axisArray, 0);

  updateInfo(id);
  axisValues[com_centralnexus_input_Joystick_AXIS_X] = info[id].axis[0];
  axisValues[com_centralnexus_input_Joystick_AXIS_Y] = info[id].axis[1];
  axisValues[com_centralnexus_input_Joystick_AXIS_Z] = info[id].axis[2];

  env->ReleaseFloatArrayElements(axisArray, axisValues, 0);
  return info[id].buttons;
}

static unsigned min(unsigned a, unsigned b) { return a<b?a:b; }
/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_getCapabilities
  (JNIEnv *x, jclass y, jint id)
{
    unsigned int axes=0;
    unsigned int maxaxes;
    int i;

    updateCapabilities(id);

    // calc return field
    maxaxes = min(capabilities[id].axes, 6)-2; //6 for V,  -2 for x&y assumed
    for (i=0; i<maxaxes; i++) {
        axes|=1<<i;
    }

    return axes;
}

/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_getNumButtons
  (JNIEnv *x, jclass y, jint id)
{
    updateCapabilities(id);

    return capabilities[id].buttons;
}

/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_getNumAxes
  (JNIEnv *x, jclass y, jint id)
{
    updateCapabilities(id);
  
    return capabilities[id].axes;
}

JNIEXPORT jint JNICALL Java_com_centralnexus_input_Joystick_getNumPOVs
(JNIEnv *, jclass, jint id)
{
    updateCapabilities(id);

    return capabilities[id].povs;
}

/*
 */
JNIEXPORT jstring JNICALL Java_com_centralnexus_input_Joystick_toString
  (JNIEnv *env, jclass y, jint id)
{
    updateCapabilities(id);

    return env->NewStringUTF(capabilities[id].name);
}
