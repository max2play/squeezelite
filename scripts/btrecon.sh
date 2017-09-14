#!/bin/bash
power_state=$1
BLUETOOTHSINK=$( grep -a "BLUETOOTHSINK" /opt/max2play/options.conf | sed -n -e 's/^[A-Z_]*\=//p' | sed 's/bluez_sink.//;s/_/:/g')

if [ "$power_state" = "1" ]
then
# echo $(date) " $power_state Power switch on " >> /opt/squeezelite/log/btrecon.log
bluetoothctl <<EOF
connect $BLUETOOTHSINK
EOF

else
# echo $(date) " $power_state Power switch off " >> /opt/squeezelite/log/btrecon.log
bluetoothctl <<EOF
disconnect $BLUETOOTHSINK
EOF
fi
