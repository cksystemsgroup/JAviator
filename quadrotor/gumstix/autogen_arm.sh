#!/bin/bash

source cross.inc

./configure --host=i686-pc-linux-gnu --target=${TARGET} CC=$CC CFLAGS="$CFLAGS"

