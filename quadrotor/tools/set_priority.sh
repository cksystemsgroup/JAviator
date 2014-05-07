#!/bin/sh

./control -c

# this is busybox style:
#    ps
# this is GNU style:
#    px ax 

PS_CMD="ps"
tmpfile="/tmp/ps_file.txt"

# use the correct PS command
${PS_CMD} > $tmpfile

# find the PIDs in the tmp file
SERIAL_PID=$(grep "IRQ-20" $tmpfile | awk '{print $1}' | head -n 1)
WLAN_PID=$(grep "wlan_main_servi" $tmpfile | awk '{print $1}' | head -n 1)
WLAN_IRQ=$(grep "wlan_reassoc_se" $tmpfile | awk '{print $1}' | head -n 1)
NET_RX=$(grep "softirq-net-rx/" $tmpfile | awk '{print $1}' | head -n 1)
NET_TX=$(grep "softirq-net-tx/" $tmpfile | awk '{print $1}' | head -n 1)

chrt -f -p 95 $SERIAL_PID
chrt -f -p 40 $WLAN_PID
chrt -f -p 40 $WLAN_IRQ
chrt -f -p 90 $NET_RX
chrt -f -p 90 $NET_TX

echo "SERIAL PID:"
chrt -p $SERIAL_PID
echo "WLAN PID:"
chrt -p $WLAN_PID
echo "WLAN IRQ:"
chrt -p $WLAN_IRQ
echo "NET RX:"
chrt -p $NET_RX
echo "NET TX:"
chrt -p $NET_TX

rm $tmpfile

echo ""
echo "now start the controller with a priority > 50 (e.g., chrt -f 60 ./control)"
echo ""

