#!/bin/sh

WLAN_PRIO=40


./control -c

tmpfile="ps_file.txt"

ps ax > $tmpfile

#SERIAL_PID=$(grep "IRQ-20" $tmpfile | awk '{print $1}' | head -n 1)
SPI_PID=$(grep "\[IRQ-16\]" $tmpfile | awk '{print $1}' | head -n 1)
PXA_PID=$(grep "pxa2xx-spi" $tmpfile | awk '{print $1}' | head -n 1)
WLAN_PID=$(grep "wlan_main" $tmpfile | awk '{print $1}' | head -n 1)
WLAN_IRQ=$(grep "IRQ-160" $tmpfile | awk '{print $1}' | head -n 1)
NET_RX=$(grep "net-rx" $tmpfile | awk '{print $1}' | head -n 1)
NET_TX=$(grep "net-tx" $tmpfile | awk '{print $1}' | head -n 1)

#chrt -f -p 95 $SERIAL_PID 
#chrt -p $SERIAL_PID 
chrt -f -p 95 $SPI_PID 
chrt -p $SPI_PID 
chrt -f -p 52 $PXA_PID
chrt -p $PXA_PID
chrt -f -p ${WLAN_PRIO} $WLAN_PID
chrt -p $WLAN_PID
chrt -f -p ${WLAN_PRIO} $WLAN_IRQ
chrt -p $WLAN_IRQ
chrt -f -p ${WLAN_PRIO} $NET_RX
chrt -f -p ${WLAN_PRIO} $NET_TX
chrt -p $NET_RX
chrt -p $NET_TX

rm $tmpfile

echo "now start the controller with a priority > 50 (e.g. 60)"

