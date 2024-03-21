#!/usr/bin/env bash

USR_LED_UC8200_MIL3="/sys/class/leds/USR:green:programming/trigger"
USR_LED_UC8200_MIL1="/sys/class/leds/UC8200:GREEN:USR4-2/trigger"
USR_LED_UC8100_MIL1="/sys/class/leds/UC8100:GREEN:ZIG/trigger"
USR_LED_UC3100_MIL1="/sys/class/leds/UC3100:GREEN:SYS/trigger"

if [ ${EUID} -ne 0 ]
then
	exit 1 # this is meant to be run as root
fi

if [ -e "${USR_LED_UC8200_MIL3}" ]; then
	echo "heartbeat" > "${USR_LED_UC8200_MIL3}"
elif [ -e "${USR_LED_UC8200_MIL1}" ]; then
	echo "heartbeat" > "${USR_LED_UC8200_MIL1}"
	mx-uart-ctl -p 0 -m 0
elif [ -e "${USR_LED_UC8100_MIL1}" ]; then
	echo "heartbeat" > "${USR_LED_UC8100_MIL1}"
	mx-uart-ctl -p 0 -m 0
elif [ -e "${USR_LED_UC3100_MIL1}" ]; then
	echo "heartbeat" > "${USR_LED_UC3100_MIL1}"
	mx-uart-ctl -p 0 -m 0
fi

stty -F /dev/ttyUSB0 115200
#/usr/sbin/soem/simple_test -q eth0 10000 2&>1 > /dev/ttyUSB0
/usr/sbin/soem/red_test -q eth0 eth1 10000 2&>1 > /dev/ttyUSB0
