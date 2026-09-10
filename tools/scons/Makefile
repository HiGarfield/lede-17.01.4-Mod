#
# Copyright (C) 2011-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=scons
PKG_VERSION:=2.5.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/scons \
		http://fossies.org/linux/misc/
PKG_HASH:=0b25218ae7b46a967db42f2a53721645b3d42874a65f9552ad16ce26d30f51f2

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
endef

define Host/Compile
endef

# scons 2.5.x only runs on Python 2.  On hosts that no longer ship a Python 2
# interpreter there is nothing to install, so skip it with a warning instead
# of failing the whole build - only gpsd, iotivity and smartsnmpd use it.
#
# Keep this free of unbalanced parentheses: make closes $(shell ...) at the
# first ")" without a matching "(", so the obvious
#   case "$($p -V 2>&1)" in "Python 2"*) ...
# gets truncated and handed to the shell half finished.
PYTHON2:=$(firstword $(foreach p,python python2 python2.7 python2.6 python2.5 python2.4,$(shell $p -V 2>&1 | grep -q "^Python 2" && echo $p)))

ifeq ($(PYTHON2),)
define Host/Install
	echo "WARNING: no Python 2.x interpreter found, skipping scons (only needed by gpsd, iotivity and smartsnmpd)" >&2
endef
else
define Host/Install
	./files/pywrap.sh $(HOST_BUILD_DIR)/setup.py install --prefix=$(STAGING_DIR_HOST)
	rm -f $(STAGING_DIR_HOST)/bin/scons*.py
	for bin in $(STAGING_DIR_HOST)/bin/scons*; do \
		mv "$$$$bin" "$$$$bin.py";                \
		cp ./files/pywrap.sh "$$$$bin";           \
	done
endef
endif

$(eval $(call HostBuild))
