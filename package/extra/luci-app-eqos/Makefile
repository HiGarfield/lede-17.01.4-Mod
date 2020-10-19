#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-eqos
LUCI_TITLE:=Easy QoS(Support speed limit based on IP address)
LUCI_DEPENDS:=+luci-base +tc +ip-full +kmod-sched-core +kmod-ifb
LUCI_PKGARCH:=all

define Package/luci-app-eqos/conffiles
/etc/config/eqos
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
