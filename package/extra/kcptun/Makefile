#
# Copyright (C) 2019 Xingwang Liao
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kcptun
PKG_VERSION:=20230214
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/xtaci/kcptun.git
PKG_SOURCE_VERSION:=432e75c76ad7534a486d30ed6ed35051dba0eb53
PKG_MIRROR_HASH:=97df9670573521c0f0a6b3592440f6dd9a4d82b5bddd2e35cceba4381e40596a
PKG_RELEASE:=1

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.md
PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/xtaci/kcptun

GO_PKG_LDFLAGS:=-s -w -X 'main.VERSION=$(PKG_VERSION)-$(PKG_RELEASE) for OpenWrt'

# Can't use GO_PKG_LDFLAGS_X to define X args with space

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/kcptun/Default
  define Package/kcptun-$(1)
    SECTION:=net
    CATEGORY:=Network
    SUBMENU:=Web Servers/Proxies
    DEPENDS:=$$(GO_ARCH_DEPENDS)
    TITLE:=Simple UDP Tunnel Based On KCP ($1)
    URL:=https://github.com/xtaci/kcptun
  endef

  define Package/kcptun-$(1)/description
  A Stable & Secure Tunnel Based On KCP with N:M Multiplexing.

  This package contains the kcptun $(1).
  endef

  define Build/Compile
	$$(call GoPackage/Build/Compile)
	$$(UPX) $$(GO_PKG_BUILD_BIN_DIR)/$(1)
  endef

  define Package/kcptun-$(1)/install
	$$(call GoPackage/Package/Install/Bin,$$(PKG_INSTALL_DIR))

	$$(INSTALL_DIR) $$(1)/usr/bin
	$$(INSTALL_BIN) $$(PKG_INSTALL_DIR)/usr/bin/$(1) $$(1)/usr/bin/kcptun-$(1)
  endef
endef

KCPTUN_COMPONENTS:=client server

$(foreach component,$(KCPTUN_COMPONENTS), \
  $(eval $(call Package/kcptun/Default,$(component))) \
  $(eval $(call GoBinPackage,kcptun-$(component))) \
  $(eval $(call BuildPackage,kcptun-$(component))) \
)
