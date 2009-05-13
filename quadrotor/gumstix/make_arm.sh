#!/bin/bash

source cross.inc

make

$STRIP ./src/control
