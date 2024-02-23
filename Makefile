
PACKAGE_NAME := "moxa-soem"
VERSION := "1.0.0.2"
ARCH := "all"
MAINTAINER := "Bobby Chen \<bobby.chen@moxa.com\>"
DEPENDS := ""
SECTION := "web"
PRIORITY := "optional"
HOMEPAGE := "http://www.moxa.com"
DESCRIPTION := "None"

# CROSS_COMPILE ?= 
# CC := $(CROSS_COMPILE)gcc
# STRIPT := $(CROSS_COMPILE)strip

# CFLAGS += -DDATE="\"$$(date "+%Y%m%d%H%M%S")\"" -DAPP_VERSION="\"$(VERSION)\""
# LDFLAGS += 
# LFLAGS += 

ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

PREFIX ?= "$(ROOT_DIR)/_install"
OUTPUT_PATH ?= "$(ROOT_DIR)/_deb"
DEB_OUT_PATH ?= "$(ROOT_DIR)/../../deb"

BUILD_ROOT ?= "$(ROOT_DIR)/build"
TOOLCHAIN_FILE ?= "$(ROOT_DIR)/arm-linux-gnueabihf.cmake"


.PHONY: all install clean test distclean

all: 
	@./build.sh

test:

clean:
	@./build.sh clean

distclean: clean
	@./build.sh distclean
	@rm -rf $(PREFIX)
	@rm -rf $(OUTPUT_PATH)

install:
	@mkdir -p $(PREFIX)/usr/sbin/soem
	@mkdir -p $(PREFIX)/lib/systemd/system
	@cp -f ./build/test/linux/red_test/red_test $(PREFIX)/usr/sbin/soem
	@cp -f ./scripts/start.sh $(PREFIX)/usr/sbin/soem
	@cp -f ./scripts/moxa-soem.service $(PREFIX)/lib/systemd/system/moxa-soem.service

deb:
	@echo "[INFO] Create debian package file (control)." ;\
	rm -rf $(PREFIX)/DEBIAN ;\
	mkdir -p $(PREFIX)/DEBIAN ;\
	cp -f ./scripts/postinst $(PREFIX)/DEBIAN ;\
	echo "Package: $(PACKAGE_NAME)" > $(PREFIX)/DEBIAN/control ;\
	echo "Version: $(VERSION)" >> $(PREFIX)/DEBIAN/control ;\
	echo "Architecture: $(ARCH)" >> $(PREFIX)/DEBIAN/control ;\
	echo "Maintainer: $(MAINTAINER)" >> $(PREFIX)/DEBIAN/control ;\
	echo "Installed-Size: 0" >> $(PREFIX)/DEBIAN/control ;\
	echo "Depends: $(DEPENDS)" >> $(PREFIX)/DEBIAN/control ;\
	echo "Section: $(SECTION)" >> $(PREFIX)/DEBIAN/control ;\
	echo "Priority: $(PRIORITY)" >> $(PREFIX)/DEBIAN/control ;\
	echo "Homepage: $(HOMEPAGE)" >> $(PREFIX)/DEBIAN/control ;\
	echo "Description: $(DESCRIPTION)" >> $(PREFIX)/DEBIAN/control ;\
	SIZE=`du -sx --exclude DEBIAN "$(PREFIX)/DEBIAN/" | awk '{print $$1}'` ;\
	sed -i "s/Installed-Size: 0/Installed-Size: $${SIZE}/g" "$(PREFIX)/DEBIAN/control" ;\
	cat $(PREFIX)/DEBIAN/control ;\
	echo "[INFO] Build Deb Package." ;\
	if [ ! -d $(OUTPUT_PATH) ]; then mkdir -p $(OUTPUT_PATH); fi ;\
	rm -rf "$(OUTPUT_PATH)/$(PACKAGE_NAME)_$(VERSION).deb" ;\
	dpkg -b "$(PREFIX)" "$(OUTPUT_PATH)/$(PACKAGE_NAME)_$(VERSION).deb" ;\
	if [ -d $(DEB_OUT_PATH) ]; then cp "$(OUTPUT_PATH)/$(PACKAGE_NAME)_$(VERSION).deb" $(DEB_OUT_PATH); fi
