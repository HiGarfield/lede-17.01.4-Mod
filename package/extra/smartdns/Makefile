#
# Copyright (c) 2018-2020 Nick Peng (pymumu@gmail.com)
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=smartdns

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://www.github.com/pymumu/smartdns.git
PKG_SOURCE_VERSION:=b525170bfd627607ee5ac81f97ae0f1f4f087d6b
PKG_MIRROR_HASH:=2926c58ce687a7fd715646a120414d21031ea66b17a2ddda5d32fefc0a9a3483
PKG_RELEASE:=1

PKG_MAINTAINER:=Nick Peng <pymumu@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

MAKE_PATH:=src

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed -flto

define Package/smartdns
  SECTION:=net
  CATEGORY:=Network
  TITLE:=smartdns server
  DEPENDS:=+libpthread +libopenssl
  URL:=https://www.github.com/pymumu/smartdns/
endef

define Package/smartdns/description
SmartDNS is a local DNS server which accepts DNS query requests from local network clients,
gets DNS query results from multiple upstream DNS servers concurrently, and returns the fastest IP to clients.
Unlike dnsmasq's all-servers, smartdns returns the fastest IP. 
endef

define Package/smartdns/conffiles
/etc/config/smartdns
/etc/smartdns/address.conf
/etc/smartdns/blacklist-ip.conf
/etc/smartdns/custom.conf
endef

define Package/smartdns/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/config $(1)/etc/init.d $(1)/etc/smartdns
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/smartdns $(1)/usr/sbin/smartdns
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/package/openwrt/files/etc/init.d/smartdns $(1)/etc/init.d/smartdns
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/package/openwrt/address.conf $(1)/etc/smartdns/address.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/package/openwrt/blacklist-ip.conf $(1)/etc/smartdns/blacklist-ip.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/package/openwrt/custom.conf $(1)/etc/smartdns/custom.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/package/openwrt/domain-block.list $(1)/etc/smartdns/domain-block.list
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/package/openwrt/domain-forwarding.list $(1)/etc/smartdns/domain-forwarding.list
	$(INSTALL_CONF) files/smartdns.conf $(1)/etc/config/smartdns
endef

$(eval $(call BuildPackage,smartdns))
