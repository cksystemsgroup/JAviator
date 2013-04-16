# JAviator

The JAviator (Java Aviator) is a high-precision, custom-built, electric quadrotor helicopter that serves as an aerial software testbed for conducting real-time, real-world experiments. It was created originally in the scope of a collaborative research project with IBM Research and at that time programmed exclusively in Java. Besides acting as live demonstrator, the JAviator is exploited primarily for verifying time-portable software implementations, which can be simulated only to some extent and therefore need to be tested under real-time and real-world conditions.

Homepage: http://javiator.cs.uni-salzburg.at


## Software System

The JAviator's software runs on the [Robostix expansion board](https://www.gumstix.com/store/index.php?cPath=31), the [Gumstix Verdex Pro XL6P](https://www.gumstix.com/store/index.php?cPath=27), and ground system hardware. The software is divided into three layers called JAviator Plant (JAP), Flight Control System (FCS), and Ground Control System (GCS). The JAP is written in C, runs on the Robostix, and implements device drivers for sensors and actuators. The FCS, in the present version, is written in C, runs on the Gumstix, and implements the flight control algorithms. The GCS is written in Java, runs on any computer providing a JVM (Java Virtual Machine), and implements the Control Terminal application for piloting the JAviator and displaying its status.


## Dependencies

### JAviator Plant

* [AVR Toolchain](http://docwiki.gumstix.org/index.php?title=Robostix_avr_gcc)

Note that the following steps refer to Linux-based systems. Users of Windows and other systems are referred to the corresponding guidelines provided by the tool suppliers.

Install the AVR compiler:

    sudo apt-get install gcc-avr avr-libc gawk

Install the AVR runtime library:

    wget http://mirror.csclub.uwaterloo.ca/nongnu/avr-libc/avr-libc-1.6.7.tar.bz2
    tar xjf avr-libc-1.6.7.tar.bz2
    cd avr-libc-1.6.7
    ./configure --build=`./config.guess` --host=avr --prefix=/usr/lib
    make
    sudo make install

### Flight Controller

* [OpenEmbedded Build System](http://openembedded.org/wiki/Main_Page)
* [Openmoko Toolchain](http://wiki.openmoko.org/wiki/Main_Page)

Install the OpenEmbedded build system for the Ångström distribution by following the "Building Ångström" instructions given [here](http://www.angstrom-distribution.org/building-angstrom).

Install the Openmoko toolchain by following the "Downloading and Installing" instructions given [here](http://wiki.openmoko.org/wiki/Toolchain).

### Control Terminal

* [Java Runtime Environment](http://openjdk.java.net/)

Install the Java runtime environment by following the "OpenJDK > Installation of Java Runtime Environment" instructions given [here](https://help.ubuntu.com/community/Java).


## Building

### JAviator Plant

Build the Robostix programs:

    cd JAviator/quadrotor/robostix
    make

You should now have the files `robo1.hex` and `robo2.hex`. Copy these binaries to the Gumstix and upload them to the Robostix flashes by following the ISP (In-System Programming) instructions given [here](http://docwiki.gumstix.org/index.php?title=Robostix_gumstix_ISP).

### Flight Controller

Build the Gumstix program:

    cd JAviator/quadrotor/gumstix
    ./autogen_arm.sh
    make

You should now have the file `control`. Copy this binary to the Gumstix. If you have installed a kernel version with real-time extensions and support of high-resolution timers, then also copy the file `JAviator/quadrotor/tools/set_priority.sh` to the Gumstix. This script can be used for improving the controller's I/O handling behavior by adapting the relevant process priorities.

### Control Terminal

Note that the following steps refer to the Eclipse (&ge; 3.2) Java development tools, but may be similar to some extent for other Java development environments. Use "Import > Existing Projects into Workspace" to create the three projects `ControlTerminal`, `JAviator3D`, and `JoyDrv` from the existing file systems `JAviator/quadrotor/ControlTerminal`, `JAviator/quadrotor/JAviator3D`, and `JAviator/quadrotor/JoyDrv`, respectively. Then add the projects `JAviator3D` and `JoyDrv` under "ControlTerminal > Properties > Java Build Path > Projects" to the build path of the project `ControlTerminal`. Finally, refer to `JAviator/quadrotor/JoyDrv/docs/html/index.html` for installing the system-specific joystick driver.


## Operation

### JAviator Plant

The two Robostix programs `robo1.hex` and `robo2.hex` start immediately upon power is supplied to the boards. After a few milliseconds, as soon as the initialization process has completed, the yellow LED of Robostix 1 should go on, which indicates that the program is waiting for connectivity initiated by the control program running on the Gumstix.

### Flight Controller

Without real-time extensions and support of high-resolution timers, start the control program using the following command:

    ./control -t 15

If your Linux system on the Gumstix has been modified for real-time support, then you may want to optimize the controller-specific I/O process priorities using the following command:

    ./set_priority.sh

Now start the control program with an arbitrary process priority between 50 and 90 using the "change real-time" command:

    chrt -f 60 ./control -t 15

After successfully establishing a connection to the Robostix program, which will be displayed in the console window, the yellow LED on Robostix 1 should be blinking.

### Control Terminal

Start the Control Terminal application by selecting "ControlTerminal > Run As > Java Application". For a brief introduction to the basic functionality of the Control Terminal, please refer to [3. Starting the Control Terminal](http://javiator.cs.uni-salzburg.at/releases/control_system_demo_software/how_to.html). Note that displaying the "JAviator 3D" window by passing a command line argument has been disabled (commented out) in this stripped version. Furthermore, activating the "Signals Dialog" by pressing the F3 key has been disabled temporarily in favor of the "Helicopter Position" window, which is now assigned to the F3 key.


## License

Copyright (c) 2006-2013, the JAviator Project Authors.
All rights reserved. Please see the AUTHORS file for details.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the JAviator Project.
