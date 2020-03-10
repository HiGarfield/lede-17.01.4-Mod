#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lede-info
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_FLAGS:=essential

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=LEDE machine informatiom utility
	DEPENDS:=+busybox
endef

define Package/$(PKG_NAME)/description
LEDE machine informatiom utility
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/lede-info $(1)/etc/init.d
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
