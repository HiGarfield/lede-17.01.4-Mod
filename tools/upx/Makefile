#
# Copyright (C) 2011-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=upx
PKG_SOURCE_PROTO:=git

PKG_SOURCE_URL:=https://github.com/upx/upx
PKG_SOURCE_VERSION:=f1f95dbcaea1959967ef2864c88b29cc09140a3e
PKG_MIRROR_HASH:=skip

HOST_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/host-build.mk

define Host/Prepare
	$(Host/Prepare/Default)
	cd $(HOST_BUILD_DIR) &&	git submodule update --init
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/build/release/upx $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/upx
endef

$(eval $(call HostBuild))
