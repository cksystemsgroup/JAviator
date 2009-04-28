#!/bin/bash

CROSS_TOOLS=/usr/local/openmoko/arm/bin/arm-angstrom-linux-gnueabi
TARGET=arm-angstrom-linux-gnueabi

CFLAGS="-Wall -O2"
PATH=$CROSS_TOOLS:$PATH

AS=${CROSS_TOOLS}-as
CC=${CROSS_TOOLS}-gcc
CXX=${CROSS_TOOLS}-g++
LD=${CROSS_TOOLS}-ld
NM=${CROSS_TOOLS}-nm
OBJDUMP=${CROSS_TOOLS}-objdump
AR=${CROSS_TOOLS}-ar
RANLIB=${CROSS_TOOLS}-ranlib
STRIP=${CROSS_TOOLS}-strip

./configure --host=i686-pc-linux-gnu --target=arm-angstrom-linux-gnueabi CC=$CC CFLAGS="$CFLAGS"

