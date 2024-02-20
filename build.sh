#!/usr/bin/env bash

BUILD_ROOT="$PWD/build"
TOOLCHAIN_FILE="arm-linux-gnueabihf.cmake"

build()
{
	mkdir -p ${BUILD_ROOT}
	cd ${BUILD_ROOT}
	cmake .. -DCMAKE_TOOLCHAIN_FILE="../${TOOLCHAIN_FILE}"
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
