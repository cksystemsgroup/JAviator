#!/bin/sh

scp rtrummer@192.168.2.138:workspace/JAviator/quadrotor/robostix/robostix.hex .

uisp --erase --upload if=robostix.hex

