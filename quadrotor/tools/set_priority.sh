#!/bin/sh

./control -c

tmpfile="ps_file.txt"

ps > $tmpfile

SERIAL_PID=$(grep "IRQ-20" $tmpfile | awk '{print $1}' | head -n 1)
WLAN_PID=$(grep "wlan_main" $tmpfile | awk '{print $1}' | head -n 1)

chrt -f -p 95 $SERIAL_PID 
chrt -p $SERIAL_PID 
chrt -f -p 50 $WLAN_PID
chrt -p $WLAN_PID

rm $tmpfile

