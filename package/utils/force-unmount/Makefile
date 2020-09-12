#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=force-unmount
PKG_VERSION:=1
PKG_RELEASE:=2
PKG_FLAGS:=essential

include $(INCLUDE_DIR)/package.mk

define Package/force-unmount
  SECTION:=base
  CATEGORY:=Base system
  MAINTAINER:=HiGarfield <higarfield@126.com>
  TITLE:=force unmount tool
  DEPENDS:=+busybox +@BUSYBOX_CUSTOM +@BUSYBOX_CONFIG_LSOF +@BUSYBOX_CONFIG_PGREP +libubox +ubus
  MENU:=1
endef

define Package/force-unmount/description
Force unmount tool
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/force-unmount/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) ./files/force-unmount $(1)/bin/
endef

$(eval $(call BuildPackage,force-unmount))
