#!/bin/sh
autostart_bluetooth=$(cat /opt/max2play/autostart.conf | grep bluetooth=1 | wc -l)
if [ "0" -lt "$autostart_bluetooth" ]; then     
        BLUETOOTHSINK=$(grep -a "BLUETOOTHSINK" /opt/max2play/options.conf | sed -n -e 's/^[A-Z_]*\=//p')
        playing_index=$(pacmd list-sinks | grep $BLUETOOTHSINK | wc -l)
        if [ "$playing_index" -lt "1" ]; then
                exit 0;
        else
                exit 1;
        fi      
else
  exit 1;
fi
