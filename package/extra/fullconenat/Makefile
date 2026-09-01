#
# Copyright (C) 2022 Chion Tang <tech@chionlab.moe>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fullconenat
PKG_RELEASE:=1

PKG_SOURCE_DATE:=2026-09-01
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/netfilter-full-cone-nat.git
PKG_SOURCE_VERSION:=a144c739936eeeba8ba6f4505c9be4f05731c6aa
PKG_MIRROR_HASH:=81dfbdddb49dcd83ce67be0c8a57be93f89fca70a0a71ee85e834d396bad8486

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Chion Tang <tech@chionlab.moe>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/iptables-mod-fullconenat
  SUBMENU:=Firewall
  SECTION:=net
  CATEGORY:=Network
  TITLE:=FULLCONENAT iptables extension
  DEPENDS:=+iptables +kmod-ipt-fullconenat
endef

define Package/iptables-mod-fullconenat/install
	$(INSTALL_DIR) $(1)/usr/lib/iptables
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libipt_FULLCONENAT.so $(1)/usr/lib/iptables
endef

define KernelPackage/ipt-fullconenat
  SUBMENU:=Netfilter Extensions
  TITLE:=FULLCONENAT netfilter module
  DEPENDS:=+kmod-nf-ipt +kmod-nf-nat +kmod-ipt-nat6
  KCONFIG:= \
    CONFIG_NF_CONNTRACK_EVENTS=y \
    CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y \
    CONFIG_NF_NAT_MASQUERADE_IPV6=m
  FILES:=$(PKG_BUILD_DIR)/xt_FULLCONENAT.ko
  AUTOLOAD:=$(call AutoLoad,09,xt_FULLCONENAT)
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
		modules
	$(call Build/Compile/Default)
endef

$(eval $(call KernelPackage,ipt-fullconenat))
$(eval $(call BuildPackage,iptables-mod-fullconenat))
