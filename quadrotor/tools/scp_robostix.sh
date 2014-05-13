#!/bin/sh

scp rtrummer@192.168.2.125:workspace/GitHub/projects/JAviator/quadrotor/robostix/robostix.hex .

uisp --erase --upload if=robostix.hex

