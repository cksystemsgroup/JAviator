#define JNIEXPORT
#define JNIIMPORT
#define JNICALL

#include <jni.h>
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

typedef struct js_event JoystickEvent;
#define MAXJOYSTICKS 20
#define MAXAXES 33
typedef struct {
  unsigned int buttons;
  float axis[MAXAXES];
} JoystickState;

#define JOYSTICK_NAME_SIZE 256
typedef struct {
  int version; // of driver
  char axes;
  char povs;
  char buttons;
  char name[JOYSTICK_NAME_SIZE]; // of joystick
} JoystickCapabilities;

extern jint numDevices;
extern int fd[MAXJOYSTICKS];
extern JoystickState info[MAXJOYSTICKS];
extern JoystickCapabilities capabilities[MAXJOYSTICKS];

void initAll();
int openJoystick(int joystick);
void closeJoystick(int joystick);
void updateInfo(int joystick);
void updateCapabilities(int joystick);
int calcNumDevices();

