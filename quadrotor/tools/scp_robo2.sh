#!/bin/sh

scp rtrummer@192.168.2.125:workspace/JAviator/quadrotor/robostix/robo2.hex .

uisp --erase --upload if=robo2.hex

