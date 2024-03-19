#!/usr/bin/env bash

ROOT=$(dirname $(readlink -m ${0}))
BUILD_DIR="${ROOT}/../build"
EXE_DIR="${BUILD_DIR}/test/linux"
EXE_BIN="\
${EXE_DIR}/red_test/red_test \
${EXE_DIR}/simple_test/simple_test \
${EXE_DIR}/slaveinfo/slaveinfo"
SYSD_SCRIPT="${ROOT}/moxa-soem.service"
START_SCRIPT="${ROOT}/start.sh"

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
