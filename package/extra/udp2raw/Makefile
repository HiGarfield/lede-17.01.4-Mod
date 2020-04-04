#
# Copyright (c) 2017 Yu Wang <wangyucn@gmail.com>
#
# This is free software, licensed under the MIT.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=udp2raw-tunnel
PKG_VERSION:=20190407.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/wangyu-/udp2raw-tunnel.git
PKG_SOURCE_VERSION:=7d481d26b98917868d8a77ca4427eea225c56e26
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.xz

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Yu Wang

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/udp2raw-tunnel
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Tunnel which turns UDP Traffic into Encrypted FakeTCP/UDP/ICMP Traffic
  URL:=https://github.com/wangyu-/udp2raw-tunnel
  DEPENDS:= +libstdcpp +libpthread +librt
endef

define Package/udp2raw-tunnel/description
  udp2raw-tunnel is a tunnel which turns UDP Traffic into Encrypted FakeTCP/UDP/ICMP Traffic by using Raw Socket.
endef

ifeq ($(ARCH),x86_64)
  PKG_ARCH_UDP2RAW:=x64
endif
ifeq ($(ARCH),mipsel)
  PKG_ARCH_UDP2RAW:=mips
endif
ifeq ($(ARCH),mips)
  PKG_ARCH_UDP2RAW:=mips_be
endif
ifeq ($(ARCH),i386)
  PKG_ARCH_UDP2RAW:=x86
endif
ifeq ($(ARCH),arm)
  PKG_ARCH_UDP2RAW:=arm
endif
ifeq ($(BOARD),bcm53xx)
  PKG_ARCH_UDP2RAW:=arm
endif
ifeq ($(BOARD),kirkwood)
  PKG_ARCH_UDP2RAW:=arm
endif
ifeq ($(ARCH),aarch64)
  PKG_ARCH_UDP2RAW:=arm64
endif

ifndef PKG_ARCH_UDP2RAW
  PKG_ARCH_UDP2RAW:=cross
endif

MAKE_FLAGS += $(PKG_ARCH_UDP2RAW)

define Package/udp2raw-tunnel/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/udp2raw $(1)/usr/bin/udp2raw
endef

$(eval $(call BuildPackage,udp2raw-tunnel))
