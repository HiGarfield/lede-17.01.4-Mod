#
#-- Copyright (C) 2021 dz <dingzhong110@gmail.com>
#

include $(TOPDIR)/rules.mk

LUCI_TITLE := LuCI Support for easymesh
LUCI_DEPENDS := +kmod-cfg80211 +batctl-default +kmod-batman-adv +wpad-mesh-openssl +dawn +luci-proto-batman-adv
PKG_VERSION := 2.0.24123002

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
