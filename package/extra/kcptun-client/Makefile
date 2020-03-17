#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

ifeq ($(ARCH),x86_64)
	PKG_ARCH_KCPTUN:=amd64
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_KCPTUN:=mipsle
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_KCPTUN:=mips
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_KCPTUN:=386
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_KCPTUN:=arm7
endif
ifeq ($(BOARD),bcm53xx)
	PKG_ARCH_KCPTUN:=arm6
endif
ifeq ($(BOARD),kirkwood)
	PKG_ARCH_KCPTUN:=arm5
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_KCPTUN:=arm64
endif

PKG_NAME:=kcptun-client
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=libc
	TITLE:=A Fast & Secure Tunnel Based on KCP with N:M Multiplexing
	URL:=https://github.com/xtaci/kcptun
endef

define Package/$(PKG_NAME)/description
Kcptun is a fast & secure tunnel based on KCP with N:M multiplexing
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/client_linux_$(PKG_ARCH_KCPTUN) $(1)/usr/bin/kcptun-client
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
