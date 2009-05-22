#!/bin/sh

# this is busybox style:
#    ps
# this is GNU style:
#    px ax 

PS_CMD="ps ax"
WLAN_PRIO=40

./control -c

tmpfile="ps_file.txt"

# use the correct ps command:

# this is gnu style
${PS_CMD} > $tmpfile

# find the pids in the ps file
#SERIAL_PID=$(grep "IRQ-20" $tmpfile | awk '{print $1}' | head -n 1)

SPI_IRQ=$(grep "\[IRQ-16\]" $tmpfile | awk '{print $1}' | head -n 1)
PXA_PID=$(grep "pxa2xx-spi" $tmpfile | awk '{print $1}' | head -n 1)
WLAN_PID=$(grep "wlan_main" $tmpfile | awk '{print $1}' | head -n 1)
WLAN_IRQ=$(grep "IRQ-160" $tmpfile | awk '{print $1}' | head -n 1)
NET_RX=$(grep "net-rx" $tmpfile | awk '{print $1}' | head -n 1)
NET_TX=$(grep "net-tx" $tmpfile | awk '{print $1}' | head -n 1)

#chrt -f -p 95 $SERIAL_PID 
#chrt -p $SERIAL_PID 
chrt -f -p 95 $SPI_IRQ
chrt -f -p 90 $PXA_PID
chrt -f -p ${WLAN_PRIO} $WLAN_PID
chrt -f -p ${WLAN_PRIO} $WLAN_IRQ
chrt -f -p ${WLAN_PRIO} $NET_RX
chrt -f -p ${WLAN_PRIO} $NET_TX

chrt -p $SPI_IRQ
chrt -p $PXA_PID
chrt -p $WLAN_PID
chrt -p $WLAN_IRQ
chrt -p $NET_RX
chrt -p $NET_TX

rm $tmpfile

echo "now start the controller with a priority > 50 (e.g. 60)"

