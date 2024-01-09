#
# Copyright (C) 2011-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=upx
PKG_VERSION:=4.2.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-src.tar.xz
PKG_SOURCE_URL:=https://github.com/upx/upx/releases/download/v$(PKG_VERSION)
PKG_HASH:=42ee0455eea610ef7ee732aa1f657b34a351ebcfa64a24c1e2a7aaec74c1e038

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)-src
HOST_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/build/release/upx $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/upx
endef

$(eval $(call HostBuild))
