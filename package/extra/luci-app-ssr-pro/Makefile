# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ssr-pro

define Package/luci-app-ssr-pro/conffiles
/etc/config/ssrpro
/etc/shadowsocksr/addinip.txt
/etc/shadowsocksr/base-gfwlist.txt
endef

LUCI_TITLE:=LuCI support for SSR Pro
LUCI_DEPENDS:= \
	+iptables-mod-tproxy +kmod-ipt-tproxy +ip +ipset-lists \
	+shadowsocksr-libev-ssr-redir \
	+dnsforwarder +coreutils \
	+coreutils-base64 +coreutils-nohup +dnsmasq-full
LUCI_PKGARCH:=all
PKG_VERSION:=2
PKG_RELEASE:=26

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
