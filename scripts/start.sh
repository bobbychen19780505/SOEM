#!/usr/bin/env bash

USR_LED_UC8200_MIL3="/sys/class/leds/USR:green:programming/trigger"
USR_LED_UC8200_MIL1="/sys/class/leds/UC8200:GREEN:USR4-2/trigger"
USR_LED_UC8100_MIL1="/sys/class/leds/UC8100:GREEN:ZIG/trigger"
USR_LED_UC3100_MIL1="/sys/class/leds/UC3100:GREEN:SYS/trigger"

if [ ${EUID} -ne 0 ]
then
	exit 1 # this is meant to be run as root
fi

run_risc()
{
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
}

run_x86()
{
	if [ -e "/sys/class/net/eno11" ] || [ -e "/sys/class/net/eno12" ]; then
		ip link set eno11 up
		ip link set eno12 up
		sleep 1
		/usr/sbin/soem/red_test -q eno11 eno12 10000 &
	fi
	if [ -e "/sys/class/net/eno9" ] || [ -e "/sys/class/net/eno10" ]; then
		ip link set eno9 up
		ip link set eno10 up
		sleep 1
		/usr/sbin/soem/red_test -q eno9 eno10 10000 &
	fi
	if [ -e "/sys/class/net/eno7" ] || [ -e "/sys/class/net/eno8" ]; then
		ip link set eno7 up
		ip link set eno8 up
		sleep 1
		/usr/sbin/soem/red_test -q eno7 eno8 10000 &
	fi
	if [ -e "/sys/class/net/eno5" ] || [ -e "/sys/class/net/eno6" ]; then
		ip link set eno5 up
		ip link set eno6 up
		sleep 1
		/usr/sbin/soem/red_test -q eno5 eno6 10000 &
	fi
	if [ -e "/sys/class/net/eno3" ] || [ -e "/sys/class/net/eno4" ]; then
		ip link set eno3 up
		ip link set eno4 up
		sleep 1
		/usr/sbin/soem/red_test -q eno3 eno4 10000 &
	fi
	if [ -e "/sys/class/net/eno1" ] || [ -e "/sys/class/net/eno2" ]; then
		ip link set eno1 up
		ip link set eno2 up
		sleep 1
		/usr/sbin/soem/red_test -q eno1 eno2 10000 
	fi
}

if [ "$(uname -m)" == "x86_64" ]; then
	run_x86
else
	run_risc
fi
