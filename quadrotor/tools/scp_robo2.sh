#!/bin/sh

scp rtrummer@192.168.2.138:workspace/JAviator/quadrotor/robostix/robo2.hex .

uisp --erase --upload if=robo2.hex

