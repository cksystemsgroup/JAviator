#!/bin/csh
#setenv LD_LIBRARY_PATH `pwd`/lib:$LD_LIBRARY_PATH
setenv LD_LIBRARY_PATH `pwd`/lib
java -classpath .:Joystick.jar com.centralnexus.test.WindowTest -d:0.25 -d2:0.1
#gij -Dawt.toolkit=gnu.awt.gtk.GtkToolkit com.centralnexus.test.WindowTest -d:0.25 -d2:0.1
