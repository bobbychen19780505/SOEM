#!/usr/bin/env bash

BUILD_ROOT="$PWD/build"
TOOLCHAIN_FILE="arm-linux-gnueabihf.cmake"

build()
{
	local APP_VERSION=${1}
	local BUILD_DATE="$(date '+%y%m%d%H%M')"

	if [ -z "${APP_VERSION}" ]; then
		APP_VERSION="1.0.0.0"
	fi
	mkdir -p ${BUILD_ROOT}
	cd ${BUILD_ROOT}
	cmake .. -DAPP_VERSION="${APP_VERSION}" -DBUILD_DATE="${BUILD_DATE}" -DCMAKE_TOOLCHAIN_FILE="../${TOOLCHAIN_FILE}"
	# cmake .. -DAPP_VERSION="${APP_VERSION}" -DBUILD_DATE="${BUILD_DATE}"
	make clean
	make
}

clean()
{
	if [ -d "${BUILD_ROOT}" ]; then
		cd ${BUILD_ROOT}
		make clean
	fi
}


while true; do
	case "${1}" in
	"clean")
		clean
		exit 0
		;;
	"distclean")
		rm -rf ${BUILD_ROOT}
		exit 0
		;;
	*)
		break
		;;
	esac
	shift
done

build $@

exit 0
