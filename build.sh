#!/usr/bin/env bash

BUILD_ROOT="$PWD/build"
BUILD_ROOT_x86_64="${BUILD_ROOT}/x86_64"
BUILD_ROOT_risc="${BUILD_ROOT}/risc"
TOOLCHAIN_FILE="arm-linux-gnueabihf.cmake"

build_x86_64()
{
	local APP_VERSION=${1}
	local BUILD_DATE=${2}

	mkdir -p "${BUILD_ROOT_x86_64}"
	cd "${BUILD_ROOT_x86_64}"
	cmake ../.. -DAPP_VERSION="${APP_VERSION}" -DBUILD_DATE="${BUILD_DATE}"
	make clean
	make
}

build_risc()
{
	local APP_VERSION=${1}
	local BUILD_DATE=${2}

	mkdir -p "${BUILD_ROOT_risc}"
	cd "${BUILD_ROOT_risc}"
	cmake ../.. -DAPP_VERSION="${APP_VERSION}" -DBUILD_DATE="${BUILD_DATE}" -DCMAKE_TOOLCHAIN_FILE="../../${TOOLCHAIN_FILE}"
	make clean
	make
}

build()
{
	local APP_VERSION=${1}
	local BUILD_DATE="$(date '+%y%m%d%H%M')"

	if [ -z "${APP_VERSION}" ]; then
		APP_VERSION="1.0.0.0"
	fi

	build_x86_64 "${APP_VERSION}" "${BUILD_DATE}"
	build_risc "${APP_VERSION}" "${BUILD_DATE}"
}

clean()
{
	if [ -d "${BUILD_ROOT_x86_64}" ]; then
		cd ${BUILD_ROOT_x86_64}
		make clean
	fi
	if [ -d "${BUILD_ROOT_risc}" ]; then
		cd ${BUILD_ROOT_risc}
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
