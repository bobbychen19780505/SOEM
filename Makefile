
PACKAGE_NAME := "moxa-soem"
VERSION := "1.0.0.5"
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
OUTPUT_PATH ?= "$(ROOT_DIR)/_deb"
TOOLCHAIN_FILE ?= "$(ROOT_DIR)/arm-linux-gnueabihf.cmake"
POSTINST_FILE ?= "$(ROOT_DIR)/scripts/postinst"

PREFIX_x86_64 ?= "$(ROOT_DIR)/_install_x86_64"
OUTPUT_DEB_x86_64 ?= "$(OUTPUT_PATH)/$(PACKAGE_NAME)_$(VERSION)_x86_64.deb"
BUILD_ROOT_x86_64 ?= "$(ROOT_DIR)/build/x86_64"
PREFIX_risc ?= "$(ROOT_DIR)/_install_risc"
OUTPUT_DEB_risc ?= "$(OUTPUT_PATH)/$(PACKAGE_NAME)_$(VERSION)_risc.deb"
BUILD_ROOT_risc ?= "$(ROOT_DIR)/build/risc"


.PHONY: all install clean test distclean

all: 
	@$(ROOT_DIR)/build.sh $(VERSION)

test:

clean:
	@$(ROOT_DIR)/build.sh clean

distclean: clean
	@$(ROOT_DIR)/build.sh distclean
	@rm -rf $(PREFIX_x86_64)
	@rm -rf $(PREFIX_risc)
	@rm -rf $(OUTPUT_PATH)

install_x86_64:
	@mkdir -p $(PREFIX_x86_64)/usr/sbin/soem
	@mkdir -p $(PREFIX_x86_64)/lib/systemd/system
	@cp -f $(BUILD_ROOT_x86_64)/test/linux/red_test/red_test $(PREFIX_x86_64)/usr/sbin/soem
	@cp -f $(BUILD_ROOT_x86_64)/test/linux/simple_test/simple_test $(PREFIX_x86_64)/usr/sbin/soem
	@cp -f $(BUILD_ROOT_x86_64)/test/linux/slaveinfo/slaveinfo $(PREFIX_x86_64)/usr/sbin/soem
	@cp -f $(ROOT_DIR)/scripts/start.sh $(PREFIX_x86_64)/usr/sbin/soem
	@cp -f $(ROOT_DIR)/scripts/moxa-soem.service $(PREFIX_x86_64)/lib/systemd/system/moxa-soem.service

install_risc:
	@mkdir -p $(PREFIX_risc)/usr/sbin/soem
	@mkdir -p $(PREFIX_risc)/lib/systemd/system
	@cp -f $(BUILD_ROOT_risc)/test/linux/red_test/red_test $(PREFIX_risc)/usr/sbin/soem
	@cp -f $(BUILD_ROOT_risc)/test/linux/simple_test/simple_test $(PREFIX_risc)/usr/sbin/soem
	@cp -f $(BUILD_ROOT_risc)/test/linux/slaveinfo/slaveinfo $(PREFIX_risc)/usr/sbin/soem
	@cp -f $(ROOT_DIR)/scripts/start.sh $(PREFIX_risc)/usr/sbin/soem
	@cp -f $(ROOT_DIR)/scripts/moxa-soem.service $(PREFIX_risc)/lib/systemd/system/moxa-soem.service

install: install_x86_64 install_risc

deb_x86_64:
	@echo "[INFO] Create debian package file (control)." ;\
	rm -rf $(PREFIX_x86_64)/DEBIAN ;\
	mkdir -p $(PREFIX_x86_64)/DEBIAN ;\
	if [ -e $(POSTINST_FILE) ]; then cp -f $(POSTINST_FILE) $(PREFIX_x86_64)/DEBIAN; fi ;\
	echo "Package: $(PACKAGE_NAME)" > $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "Version: $(VERSION)" >> $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "Architecture: $(ARCH)" >> $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "Maintainer: $(MAINTAINER)" >> $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "Installed-Size: 0" >> $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "Depends: $(DEPENDS)" >> $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "Section: $(SECTION)" >> $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "Priority: $(PRIORITY)" >> $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "Homepage: $(HOMEPAGE)" >> $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "Description: $(DESCRIPTION)" >> $(PREFIX_x86_64)/DEBIAN/control ;\
	SIZE=`du -sx --exclude DEBIAN "$(PREFIX_x86_64)/DEBIAN/" | awk '{print $$1}'` ;\
	sed -i "s/Installed-Size: 0/Installed-Size: $${SIZE}/g" "$(PREFIX_x86_64)/DEBIAN/control" ;\
	cat $(PREFIX_x86_64)/DEBIAN/control ;\
	echo "[INFO] Build Deb Package." ;\
	if [ ! -d $(OUTPUT_PATH) ]; then mkdir -p $(OUTPUT_PATH); fi ;\
	rm -rf "$(OUTPUT_DEB_x86_64)" ;\
	dpkg -b "$(PREFIX_x86_64)" "$(OUTPUT_DEB_x86_64)" ;\

deb_risc:
	@echo "[INFO] Create debian package file (control)." ;\
	rm -rf $(PREFIX_risc)/DEBIAN ;\
	mkdir -p $(PREFIX_risc)/DEBIAN ;\
	if [ -e $(POSTINST_FILE) ]; then cp -f $(POSTINST_FILE) $(PREFIX_risc)/DEBIAN; fi ;\
	echo "Package: $(PACKAGE_NAME)" > $(PREFIX_risc)/DEBIAN/control ;\
	echo "Version: $(VERSION)" >> $(PREFIX_risc)/DEBIAN/control ;\
	echo "Architecture: $(ARCH)" >> $(PREFIX_risc)/DEBIAN/control ;\
	echo "Maintainer: $(MAINTAINER)" >> $(PREFIX_risc)/DEBIAN/control ;\
	echo "Installed-Size: 0" >> $(PREFIX_risc)/DEBIAN/control ;\
	echo "Depends: $(DEPENDS)" >> $(PREFIX_risc)/DEBIAN/control ;\
	echo "Section: $(SECTION)" >> $(PREFIX_risc)/DEBIAN/control ;\
	echo "Priority: $(PRIORITY)" >> $(PREFIX_risc)/DEBIAN/control ;\
	echo "Homepage: $(HOMEPAGE)" >> $(PREFIX_risc)/DEBIAN/control ;\
	echo "Description: $(DESCRIPTION)" >> $(PREFIX_risc)/DEBIAN/control ;\
	SIZE=`du -sx --exclude DEBIAN "$(PREFIX_risc)/DEBIAN/" | awk '{print $$1}'` ;\
	sed -i "s/Installed-Size: 0/Installed-Size: $${SIZE}/g" "$(PREFIX_risc)/DEBIAN/control" ;\
	cat $(PREFIX_risc)/DEBIAN/control ;\
	echo "[INFO] Build Deb Package." ;\
	if [ ! -d $(OUTPUT_PATH) ]; then mkdir -p $(OUTPUT_PATH); fi ;\
	rm -rf "$(OUTPUT_DEB_risc)" ;\
	dpkg -b "$(PREFIX_risc)" "$(OUTPUT_DEB_risc)" ;\

deb: deb_x86_64 deb_risc
