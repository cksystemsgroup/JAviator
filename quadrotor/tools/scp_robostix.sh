#!/bin/sh

scp rtrummer@192.168.2.138:workspace/quadrotor/robostix/robostix.hex .

rs/restore_ssp

uisp --erase --upload if=robostix.hex
