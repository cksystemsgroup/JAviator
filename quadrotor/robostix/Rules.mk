###########################################################################
#
# Rules.mk - Contains common makefile support
#
###########################################################################

ifeq ($(AVR_MCU),)
AVR_MCU = atmega128
endif
all: $(TARGET1) $(TARGET2)

$(TARGET1) : % : %.hex
$(TARGET2) : % : %.hex
 
.PHONY: all $(TARGET1)
.PHONY: all $(TARGET2)

CROSS_COMPILE = avr-

CC      = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

AVR_MCU_FLAGS = -mmcu=$(AVR_MCU)

vpath %.c ./src

CFLAGS   += -O3 -Wall -I../
CPPFLAGS += $(AVR_MCU_FLAGS) -I . -I ./include -I ./src
LDFLAGS  += $(AVR_MCU_FLAGS) -Wl,-Map,$(basename $@).map

DEP_OUTPUT_OPTION = -MMD -MF $(@:.o=.d)

ECHO = @echo
RM   = rm

COMMON_DEPS1 = $(strip $(COMMON_OBJS1:.o=.d))
COMMON_DEPS2 = $(strip $(COMMON_OBJS2:.o=.d))

DEP_FILES1 = $(TARGET1).d $(COMMON_DEPS1)
DEP_FILES2 = $(TARGET2).d $(COMMON_DEPS2)

#--------------------------------------------------------------------------
#
# 	Run make with v=1 or verbose=1 to get verbose output
#

ifeq ($(v),)
export verbose = 0
else
export verbose = 1
endif

ifeq ($(verbose),)
export verbose = 0
endif

ifeq ($(verbose),0)
	Q = @
	MAKEFLAGS += -s
else
	Q =
endif
export Q

#--------------------------------------------------------------------------
#
#	CFLAGS 		= flags used for C compiler
#	CXXFLAGS	= flags for the C++ compiler
#	CPPFLAGS	= flags for the C preprocessor (-D, -I)
#

PREPROCESS.c = $(CC) $(CPPFLAGS) $(TARGET1_ARCH) -E -Wp,-C,-dD,-dI
PREPROCESS.c = $(CC) $(CPPFLAGS) $(TARGET2_ARCH) -E -Wp,-C,-dD,-dI

#--------------------------------------------------------------------------
#
#	The FORCE target can be used for things that you want to have rebuild
# 	every single time (like *.pp files)
#

FORCE:

.PHONY: FORCE

#--------------------------------------------------------------------------
#
# Rule to update svn-version.h file. If a C file includes this file then
# the dependcy generator will cause this file to be generated.
#
# To prevent unnecessary updates, a temporary file is created and compared
# to svn-version.h, so that svn-version.h is only touched if a change
# is detected.
#

svn-version.h: FORCE
	echo "/*" > svn-version.tmp
	echo " * Generated file - Do NOT edit" >> svn-version.tmp
	echo " */" >> svn-version.tmp
	echo >> svn-version.tmp
	echo "#define	SVN_REVISION $(subst Revision:,,$(shell svn info | grep Revision:))" >> svn-version.tmp
	cmp --quiet svn-version.tmp svn-version.h || cp svn-version.tmp svn-version.h
	rm svn-version.tmp

#--------------------------------------------------------------------------
#
# Compile C source files
#
#   COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET1_ARCH) -c
#   COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET2_ARCH) -c
#
# Create a rule for when the dependency exists, and one for when it doesn't
#

.PRECIOUS: %.o

%.o : %.c %.d
	$(ECHO) "Compiling $< ..."
	$(Q)$(COMPILE.c) $(DEP_OUTPUT_OPTION) $(OUTPUT_OPTION) $<

%.o : %.c
	$(ECHO) "Compiling $< ..."
	$(Q)$(COMPILE.c) $(DEP_OUTPUT_OPTION) $(OUTPUT_OPTION) $<

#--------------------------------------------------------------------------
#
# Assemble gas assembly files.
#
#   COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET1_ARCH) -c
#   COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET2_ARCH) -c
#

%.o : %.s
	$(ECHO) "Assembling $< ..."
	$(Q)$(COMPILE.S) $(OUTPUT_OPTION) $<


# Making the .d's be phony and providing a dummy rule to make them, combined
# with the %.o : %.c %.d causes make to work properly when the .d file is
# removed, but the .o file isn't. make will now regenerate the .d by 
# recompiling the .o
	
.PHONY: %.d
%.d :
	# Empty rule - dependencies are created by the compiler


#--------------------------------------------------------------------------
#
# Generate Preprocessed files from C source (useful for debugging)
#

%.pp : %.c FORCE
	$(ECHO) "Preprocessing $< ..."
	$(Q)$(PREPROCESS.c) $< > $@

#--------------------------------------------------------------------------
#
# Generate C/asm listing
#

%.cod : %.c FORCE
	$(ECHO) "Listing $< ..."
	$(Q)$(COMPILE.c) -gstabs -Wa,-ahdlms=$@ $<

#--------------------------------------------------------------------------
#
# Create a hex file from an elf file
#

%.hex : %.elf
	$(ECHO) "Creating $@ ..."
	$(Q)$(OBJCOPY) -j .text -j .data -O ihex $< $@
	$(ECHO)

#--------------------------------------------------------------------------
#
# Create simple executables
#

.PRECIOUS: %.elf

ifeq ($(MAIN_OBJS1),)
MAIN_OBJS1 = $(TARGET1).o
endif

$(TARGET1).elf : $(MAIN_OBJS1) $(COMMON_OBJS1)
	$(ECHO) "Linking $@ ..."
	$(Q)$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
	$(ECHO)
	./avr-mem.sh $@ $(AVR_MCU)
	$(ECHO)

ifeq ($(MAIN_OBJS2),)
MAIN_OBJS2 = $(TARGET2).o
endif

$(TARGET2).elf : $(MAIN_OBJS2) $(COMMON_OBJS2)
	$(ECHO) "Linking $@ ..."
	$(Q)$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
	$(ECHO)
	./avr-mem.sh $@ $(AVR_MCU)
	$(ECHO)

clean: clean-other clean-hex

clean-other:
	$(ECHO) "Removing generated files ..."
	$(Q)$(RM) -f *.d *.o *.elf *.map

clean-hex:
	$(ECHO) "Removing hex files ..."
	$(Q)$(RM) -f *.hex

#--------------------------------------------------------------------------
#
# Include dependencies.
#
# The '-' at the beginning tells make not to complain about missing
# dependency files.
#

ifneq ($(DEP_FILES1),)
ifeq ($(strip $(filter clean% exec print-%, $(MAKECMDGOALS))),)
-include $(DEP_FILES1)
endif
endif

ifneq ($(DEP_FILES2),)
ifeq ($(strip $(filter clean% exec print-%, $(MAKECMDGOALS))),)
-include $(DEP_FILES2)
endif
endif

