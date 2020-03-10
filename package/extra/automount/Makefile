#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=automount
PKG_VERSION:=1
PKG_RELEASE:=14
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/automount
  TITLE:=Mount autoconfig hotplug script.
  MAINTAINER:=Lean
  DEPENDS:=+block-mount +kmod-usb-storage +kmod-usb-storage-extras +kmod-fs-ext4 +e2fsprogs +kmod-fs-vfat +dosfstools +kmod-fs-exfat +ntfs-3g +ntfs-3g-utils +kmod-nls-utf8 +kmod-nls-cp437 +kmod-nls-cp936 +kmod-nls-iso8859-1 +fdisk +hd-idle 

endef

define Package/automount/description
A usb autoconfig hotplug script.
endef

define Build/Compile
endef

define Package/automount/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/block
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/09-automount $(1)/etc/hotplug.d/block/09-automount
	$(INSTALL_BIN) ./files/99_automount-settings $(1)/etc/uci-defaults/99_automount-settings
endef

$(eval $(call BuildPackage,automount))
