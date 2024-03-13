#!/usr/bin/env bash

EXE_DIR="../build/test/linux"
EXE_BIN="\
${EXE_DIR}/red_test/red_test \
${EXE_DIR}/simple_test/simple_test \
${EXE_DIR}/slaveinfo/slaveinfo"
SYSD_SCRIPT="moxa-soem.service"
START_SCRIPT="start.sh"

if [ ${EUID} -ne 0 ]
then
	exit 1 # this is meant to be run as root
fi

if [ -d "${EXE_DIR}" ] && [ -e "${START_SCRIPT}" ]
then
	mkdir -p /usr/sbin/soem
	cp -f ${EXE_BIN} /usr/sbin/soem
	cp -f ${START_SCRIPT} /usr/sbin/soem
fi

if [ -e "${SYSD_SCRIPT}" ]
then
	cp -f ${SYSD_SCRIPT} /lib/systemd/system/${SYSD_SCRIPT}
	systemctl daemon-reload
	systemctl enable ${SYSD_SCRIPT}
fi
