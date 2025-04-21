#
# Copyright (C) 2006-2025 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=eject-disk
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/eject-disk
  SECTION:=base
  CATEGORY:=Base system
  MAINTAINER:=HiGarfield <higarfield@126.com>
  TITLE:=eject disk tool
  DEPENDS:=+force-unmount +hd-idle
  MENU:=1
endef

define Package/eject-disk/description
  Eject disk tool
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/eject-disk/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) ./files/eject-disk $(1)/bin/
endef

$(eval $(call BuildPackage,eject-disk))
