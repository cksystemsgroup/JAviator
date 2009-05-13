docs\license.txt     - The license agreement that this product is released under
docs\html\index.html - API documentation
com\centralnexus\input - The Java source code
lib\                 - *.DLL and *.LIB files that are needed by the API
win32\               - Windows implementation of the API
Joystick.jar         - Java Joystick Driver interface JAR file
runTest.bat          - A demo program for the Java Joystick Driver

A special thanks goes out to Ed Burns for giving me useful feedback.

The files that you need for a Windows binary distribution is lib\jjstick.dll
and Joystick.jar.  You must of course agree to the license included in this
distribution, which includes giving credit to the authors for the creation
of this library.

Thank you for you interest in the library.
George Rhoten (main author)

------------------------------------------

lib\                 - *.so files that are needed by the API
linux\               - Linux 2.4 implementation of the API
runTest		     - shell script to test joystick driver

Here is some information on getting your Linux Joystick working
in Java.

This doesn't work with the Linux 2.2 kernel, but doesn't really
take advantage of update only events. The code that was written
for Linux requires joydev.o. It should be at
"/lib/modules/2.4.18/kernel/drivers/input/".

This also requires the driver for your particular joystick. For an HID
joystick you just need to enable USB and HID support under USB support
and Joystick support under Input Core support in the kernel
configuration. For something like the Logitech WingMan you need to
enable Serial port device support and input line discipline under
Character Devices->Joysticks, then you turn on that particular Joystick
module under the Joystick heading. Turning on traditional joysticks is
similar, you just add Game Port support and then add the module for your
particular joystick.

ls -l /dev/js?    gives me
lrwxrwxrwx    1 root     root            9 Jun  8  2001 /dev/js0 -> input/js0
lrwxrwxrwx    1 root     root            9 Jun  8  2001 /dev/js1 -> input/js1
lrwxrwxrwx    1 root     root            9 Jun  8  2001 /dev/js2 -> input/js2
lrwxrwxrwx    1 root     root            9 Jun  8  2001 /dev/js3 -> input/js3

ls -l dev/input/js?  gives me
crw-------    1 danielk  root      13,   0 Mar 23  2001 /dev/input/js0
crw-------    1 danielk  root      13,   1 Mar 23  2001 /dev/input/js1
crw-------    1 danielk  root      13,   2 Mar 23  2001 /dev/input/js2
crw-------    1 danielk  root      13,   3 Mar 23  2001 /dev/input/js3

The joydev module must be loaded for this API to find any joysticks.
Depending on a variety of factors this may or may not be autoloaded
when you plug in a joystick, if it isn't you must do a "modprobe joydev"
as root to load it. If it doesn't load, you probably don't have the
joystick driver loaded (this is often the case when you compile your
own kernel). You can use "modprobe -l | grep joystick" to see what
drivers are available. Simple joysticks that use the joystick port
can use the ns558 driver. I don't think anyone using a commercial
distibution out of the box would have the this problem, but
programmers are likely to.

If you wanted everyone to be able to use the joystick the permissions
should be crw-rw-rw, this may be the problem you're having since a lot
of distributions have the joystick available to root only.

lsmod   gives me
Module                  Size  Used by
warrior                 1552   0  (unused)
serio                   1024   0  [warrior]
joydev                  5792   0  (unused)
input                   3328   0  [warrior joydev]
... other stuff ...

This is for the WingMan USB gamepad (with 7 axes).

When compiling on Linux you may need to specify where your
Java C++ headers are located. You can specify the headers when
using "make" by doing the following in the linux directory:

make INCS="-I/usr/java/jdk1.3.1_01/include/ -I/usr/java/jdk1.3.1_01/include/linux/"

Daniel Kristjansson (linux port)
George Rhoten (main author)
