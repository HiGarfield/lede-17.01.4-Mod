#
# Copyright (C) 2007-2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Support for batman-adv
LUCI_DEPENDS:=+batctl +kmod-batman-adv
LUCI_PKGARCH:=all
PKG_VERSION:=v23.1.2
PKG_RELEASE:=1714

PKG_MAINTAINER:=wulishui <wulishuia@msn.com>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
