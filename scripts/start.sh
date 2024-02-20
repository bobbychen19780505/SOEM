#!/usr/bin/env bash

if [ ${EUID} -ne 0 ]
then
	exit 1 # this is meant to be run as root
fi

echo "heartbeat" > /sys/class/leds/USR:green:programming/trigger
/usr/sbin/soem/red_test eth0 eth1 100000

