#!/bin/sh

scp rtrummer@192.168.2.138:quadrotor/JAviator/workspace/robostix/robostix.hex .

rs/restore_ssp

uisp --erase --upload if=robostix.hex

