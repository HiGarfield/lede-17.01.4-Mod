#
# Copyright (c) 2017 Yu Wang <wangyucn@gmail.com>
#
# This is free software, licensed under the MIT.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=udp2raw-tunnel
PKG_RELEASE:=1

PKG_SOURCE_DATE:=2022-02-11
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/udp2raw.git
PKG_SOURCE_VERSION:=a06c85ce4db4219b243b42488e6e4ef713e0c4df
PKG_MIRROR_HASH:=skip

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Yu Wang

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

define Package/udp2raw-tunnel/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/udp2raw $(1)/usr/bin/
endef

$(eval $(call BuildPackage,udp2raw-tunnel))
