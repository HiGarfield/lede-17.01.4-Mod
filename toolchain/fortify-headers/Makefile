#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=fortify-headers
PKG_VERSION:=1.1
PKG_RELEASE=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/jvoisin/$(PKG_NAME).git
PKG_SOURCE_DATE:=2019-04-14
PKG_SOURCE_VERSION:=e3fee64643279c144efd3d6856ed4e818c0d5ca2
PKG_MIRROR_HASH:=ceab7f3bc13170445f79bb68caced109ca2f93378156fa580d4a4e4204e6cd3c

include $(INCLUDE_DIR)/toolchain-build.mk

define Host/Compile
	true
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) PREFIX="" DESTDIR="$(TOOLCHAIN_DIR)" install
endef

$(eval $(call HostBuild))
