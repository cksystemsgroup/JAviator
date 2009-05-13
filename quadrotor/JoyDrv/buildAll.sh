#!/bin/sh

pushd com/centralnexus/input/
javac *.java
popd

./makeJar.sh

pushd linux/
make
popd

