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
#define _GNU_SOURCE 1

#include "LinuxJoystick.h"

jint numDevices=-1;

int fd[MAXJOYSTICKS];
JoystickState info[MAXJOYSTICKS];
JoystickCapabilities capabilities[MAXJOYSTICKS];

void initAll() {
    int i;
    for (i=0; i<MAXJOYSTICKS; i++) {
        fd[i]=-1;
    }
}

int openJoystick(int joystick) {
    char device[100];
    if (joystick<0 || joystick>=MAXJOYSTICKS)
        return -1;
    if (fd[joystick]>=0)
        return fd[joystick];

    sprintf(device, "/dev/js%u", joystick);
    return fd[joystick]=open(device, O_RDONLY);
}

void closeJoystick(int joystick) {
    if (joystick>=0 && joystick<MAXJOYSTICKS) {
        if (fd[joystick]!=-1)
            close(joystick);
        fd[joystick]=-1;
    }
}

void updateInfo(int joystick) {
    if (joystick<0 || joystick>=MAXJOYSTICKS)
        return;
    if (fd[joystick]<0 && openJoystick(joystick)<0)
        return;
    assert(fd[joystick]>=0);

    JoystickEvent ev;
    ev.type=0;

    fd_set rfds;
    int retval;
  
    FD_ZERO(&rfds);
    FD_SET(fd[joystick], &rfds);
    struct timeval interval;

    for (;;) {
        interval.tv_sec  = 0;
        interval.tv_usec = 1;
        retval = select(fd[joystick]+1, &rfds, NULL, NULL, &interval);
        if (retval!=1)
            break;

        TEMP_FAILURE_RETRY(read (fd[joystick], &ev, sizeof(struct js_event)));
    
        // process event
        if ((ev.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON) {
            if (ev.value) {
                info[joystick].buttons |= (1 << ev.number);
            }
            else {
                info[joystick].buttons &= ~(1 << ev.number);
            }
        }    
//    info[joystick].buttons|=8;
        if ((ev.type & ~JS_EVENT_INIT) == JS_EVENT_AXIS) {
            float val=ev.value/32767.0f;
            if (ev.number<MAXAXES)
                info[joystick].axis[ev.number]=val;
        }    
    }
}

void updateCapabilities(int joystick)
{
    if (joystick<0 || MAXJOYSTICKS<=joystick) {
        return;
    }

    capabilities[joystick].povs=0;
    capabilities[joystick].axes=0;
    capabilities[joystick].buttons=0;
    capabilities[joystick].version=0;
    strcpy(capabilities[joystick].name, "Unknown");

    if (fd[joystick]<0 && openJoystick(joystick)<0)
        return;
  
    ioctl(fd[joystick], JSIOCGAXES, &capabilities[joystick].axes);
    ioctl(fd[joystick], JSIOCGBUTTONS, &capabilities[joystick].buttons);
    ioctl(fd[joystick], JSIOCGVERSION, &capabilities[joystick].version);

    int nameLen = ioctl(fd[joystick], JSIOCGNAME(JOYSTICK_NAME_SIZE), capabilities[joystick].name);
    if (nameLen > 0) {
        // NULL terminate just in case.
        capabilities[joystick].name[JOYSTICK_NAME_SIZE - 1] = 0;
    }
    else {
        strcpy(capabilities[joystick].name, "Unknown");
    }
}

int calcNumDevices() {
    if (numDevices>=0)
        return numDevices;
    initAll();
    unsigned i=0;
    for (; i<MAXJOYSTICKS; i++) {
        int fd=openJoystick(i);
        if (fd<0)
            break;
        closeJoystick(i);
    }
    return numDevices=i;
}

