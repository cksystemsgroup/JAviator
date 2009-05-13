CROSS_TOOLS=/opt/okl/Linux-i386/arm/gcc-4.2.4-glibc-2.7/arm-unknown-linux-gnueabi/bin/arm-unknown-linux-gnueabi
TARGET=arm-unknown-linux-gnueabi

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
