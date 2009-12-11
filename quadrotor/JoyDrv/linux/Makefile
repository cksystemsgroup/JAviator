SHELL = /bin/sh

DEPOPTS= -M
DEPLINE= 1> Makedepend.linux

# C++ source files
SWSRCS= com_centralnexus_input_Joystick.cpp \
	com_centralnexus_input_ExtendedJoystick.cpp \
	LinuxJoystick.cpp

SRCS=$(SWSRCS) $(ARCHSRCS)

# C source
SWCSRCS=

CSRCS=$(SWCSRCS) $(ARCHCSRCS)

# source files with no associated .o 
SRCSNOO=

# source files with no associated .h 
SRCSNOH=

# header files with no associated .cpp
HDRSNOS=

# sources that are #include'd, and have no associated .o
TABLESRCS=

ALLSRCS=$(SRCSNOH) $(CSRCS) $(SRCS) $(SRCSNOO) $(TABLESRCS) 

# All the local .h files (these are not automatically depended)
HDRS=$(HDRSNOS) ${SRCS:.cpp=.h} ${CSRCS:.c=.h} ${SRCSNOO:.cpp=.h}

# All .o files generated
COBJS=${CSRCS:.c=.o}
OBJS= $(COBJS) ${SRCS:.cpp=.o}
ALLOBJS=$(OBJS) ${SRCSNOH:.cpp=.o}
PROTOOBJS = ${PROTOS:.cpp=.o}

# Man pages, etc. (leave blank if none)
DOCS=

# Application name
TARGET=../lib/libjjstick.so

##############################################################################
# Application-specific definitions  -- EDIT THESE
##############################################################################

INCS =	-I. -I/usr/lib/jvm/java-6-sun/include -I/usr/lib/jvm/java-6-sun/include/linux

# other C and C++ common options

# CC options

#INOPTS=-DNDEBUG -O9 -fstrength-reduce \
#	 -march=i686 -mno-ieee-fp \
#	 -funroll-loops -fomit-frame-pointer  -ffast-math \
#	-Wdisabled-optimization -fno-exceptions
# -mmmx -msse -mfpmath=sse -mthreads
#DEBUGOPTS= -ggdb -g3
INOPTS=-O -fomit-frame-pointer

OPTS=  $(INOPTS) $(DEBUGOPTS)

# C specific options
COPTS=

# C++ specific options
CCOPTS=-fno-rtti

# linker options
MLIBS=-L/opt/IBMJava2-13/bin/java/include

# ----------------------------------------------------------------------------
# -- You shouldn't need to edit anything below here, unless you have more
# -- than one target or are building a library, or you're doing something
# -- nonstandard.
# ----------------------------------------------------------------------------

##############################################################################
# Language definitions
##############################################################################

CC=g++
cc=gcc

CFLAGS= $(INCS) $(DEFS) $(DEBUG) $(OPTS) $(COPTS) 
CCFLAGS=$(INCS) $(DEFS) $(DEBUG) $(OPTS) $(CCOPTS)

TARGET_COMPILER_FLAGS=$(CCFLAGS) $(LIBOPTS) 
TARGET_COMPILER=$(CC)

##############################################################################
# extra default rules
##############################################################################
.SUFFIXES:
.SUFFIXES: .C .cpp .c .sl .slo .C.o .cpp.o .c.o .sl.o .slo.o .o

.cpp:
	$(CC)  $(CCFLAGS) -o $@ $<

.C:
	$(CC)  $(CCFLAGS) -o $@ $<

.c:
	$(cc) $(CFLAGS) -o $@ $<

.C.o:
	$(CC)  $(CCFLAGS) -c $<

.cpp.o:
	$(CC)  $(CCFLAGS) -c $<

.c.o:
	$(cc) $(CFLAGS) -c $<

.sl:
	$(SHADECOM) $@ $<

.sl.slo:
	$(SHADECOM) $<


# This rule must be first.  It insures that the desired target is the default.


all: $(TARGET)


##############################################################################
# Application-specific rules and dependencies - may need editing
##############################################################################

VERSION_MAJOR = 0
VERSION_MINOR = 6

../lib/libjjstick.a: $(ALLOBJS)
	rm -f ../lib/libjjstick.a
	ar csq ../lib/libjjstick.a $(ALLOBJS)

../lib/libjjstick.so: $(ALLOBJS)
	javah -jni -classpath ..:../Joystick.jar com.centralnexus.input.Joystick
	javah -jni -classpath ..:../Joystick.jar com.centralnexus.input.ExtendedJoystick
	rm -f ../lib/libjjstick.so.$(VERSION_MAJOR).$(VERSION_MINOR)
	$(TARGET_COMPILER) -shared -Wl,-soname,libjjstick.so.$(VERSION_MAJOR).$(VERSION_MINOR) -o ../lib/libjjstick.so.$(VERSION_MAJOR).$(VERSION_MINOR) $(ALLOBJS)
	ln -f -s libjjstick.so.$(VERSION_MAJOR).$(VERSION_MINOR) ../lib/libjjstick.so.$(VERSION_MAJOR)
	ln -f -s libjjstick.so.$(VERSION_MAJOR).$(VERSION_MINOR) ../lib/libjjstick.so
	$(_NULLCMD_)


depend: 
	$(TARGET_COMPILER) $(ALLSRCS) $(TARGET_COMPILER_FLAGS) $(DEPOPTS) $(DEPLINE)

##############################################################################
# General-purpose targets - do not edit, in general:
##############################################################################

clean:	
	rm -f $(TARGET) $(OBJS) *.slo *.o *~ *.s \
		*.a *..c ptrepository/* TAGS \
		Makedepend.linux \
		core a.out \#* *.bak *.BAK *.CKP \
		*.l *.Addrs *.Counts *.pixie .\#*; 
.IGNORE:    

# DO NOT DELETE THIS LINE -- make depend depends on it.

sinclude Makedepend.linux
