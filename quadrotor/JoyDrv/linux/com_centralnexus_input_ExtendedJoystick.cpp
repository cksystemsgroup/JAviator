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

#include "com_centralnexus_input_ExtendedJoystick.h"
#include "LinuxJoystick.h"

/*
 */
JNIEXPORT jboolean JNICALL Java_com_centralnexus_input_ExtendedJoystick_isPluggedIn
  (JNIEnv *env, jclass, jint id) {

  return (0<=id && id<calcNumDevices());
}

static int min(int a, int b) { return a<b?a:b; }
//#include <stdlib.h>
/*
 */
JNIEXPORT jint JNICALL Java_com_centralnexus_input_ExtendedJoystick_poll
(JNIEnv *env, jclass, jint id, jfloatArray axisArray) {
    int len;
    int i;
    jfloat *axisValues = env->GetFloatArrayElements(axisArray, 0);

    updateInfo(id);
    len = min(env->GetArrayLength(axisArray), MAXAXES);
    for (i=0; i<len; i++) {
        axisValues[i]=info[id].axis[i];
    }

    env->ReleaseFloatArrayElements(axisArray, axisValues, 0);
    return info[id].buttons;
}
