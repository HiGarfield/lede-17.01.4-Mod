#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autosamba
PKG_VERSION:=1
PKG_RELEASE:=8
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/autosamba
  TITLE:=Samba autoconfig hotplug script.
  DEPENDS:=+luci-app-samba
endef

define Package/autosamba/description
A hotplug script to config Samba share automatically.
endef

define Build/Compile
endef

define Package/autosamba/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/block
	$(INSTALL_BIN) ./files/samba.hotplug $(1)/etc/hotplug.d/block/60-samba
endef

$(eval $(call BuildPackage,autosamba))
