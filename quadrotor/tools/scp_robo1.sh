#!/bin/sh

scp rtrummer@192.168.2.125:workspace/GitHub/projects/JAviator/quadrotor/robostix/robo1.hex .

uisp --erase --upload if=robo1.hex

