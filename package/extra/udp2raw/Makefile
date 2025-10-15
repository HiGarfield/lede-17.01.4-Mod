#
# Copyright (c) 2017 Yu Wang <wangyucn@gmail.com>
#
# This is free software, licensed under the MIT.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=udp2raw
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/wangyu-/udp2raw
PKG_SOURCE_VERSION:=4208db6e27c46f3ccec8b98722af7ec23bc62e73
PKG_MIRROR_HASH:=facc09c9e570688bad54b2cc935a255a367b49fa19882edfb337c85af45f5170

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Yu Wang

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/udp2raw
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Tunnel which turns UDP Traffic into Encrypted FakeTCP/UDP/ICMP Traffic
  URL:=https://github.com/wangyu-/udp2raw
  DEPENDS:= +libstdcpp +libpthread +librt
endef

define Package/udp2raw/description
  udp2raw is a tunnel which turns UDP Traffic into Encrypted FakeTCP/UDP/ICMP Traffic by using Raw Socket.
endef

ifeq ($(BOARD),bcm53xx)
  UDP2RAW_TARGET:=arm
else ifeq ($(BOARD),kirkwood)
  UDP2RAW_TARGET:=arm
else ifeq ($(ARCH),x86_64)
  UDP2RAW_TARGET:=x64
else ifeq ($(ARCH),mipsel)
  UDP2RAW_TARGET:=mips
else ifeq ($(ARCH),mips)
  UDP2RAW_TARGET:=mips_be
else ifeq ($(ARCH),i386)
  UDP2RAW_TARGET:=x86
else ifeq ($(ARCH),arm)
  UDP2RAW_TARGET:=arm
else ifeq ($(ARCH),aarch64)
  UDP2RAW_TARGET:=arm64
else
  UDP2RAW_TARGET:=linux
endif

MAKE_VARS += UDP2RAW_GIT_VER=$(PKG_SOURCE_VERSION)
MAKE_FLAGS += $(UDP2RAW_TARGET)

define Package/udp2raw/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/udp2raw $(1)/usr/bin/
endef

$(eval $(call BuildPackage,udp2raw))
