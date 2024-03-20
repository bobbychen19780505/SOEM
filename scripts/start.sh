#!/usr/bin/env bash

if [ ${EUID} -ne 0 ]
then
	exit 1 # this is meant to be run as root
fi

if [ -e "/sys/class/leds/USR:green:programming/trigger" ]; then
	echo "heartbeat" > /sys/class/leds/USR:green:programming/trigger
elif [ -e "/sys/class/leds/UC8200:GREEN:USR/trigger" ]; then
	echo "heartbeat" > /sys/class/leds/UC8200:GREEN:USR/trigger
fi

stty -F /dev/ttyUSB0 115200
#/usr/sbin/soem/simple_test -q eth1 10000 > /dev/ttyUSB0 2&>1
/usr/sbin/soem/red_test -q eth0 eth1 10000 > /dev/ttyUSB0 2&>1
