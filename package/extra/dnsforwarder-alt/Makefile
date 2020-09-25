include $(TOPDIR)/rules.mk

PKG_NAME:=dnsforwarder-alt
PKG_VERSION:=6.1.15
PKG_RELEASE:=10

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/holmium/dnsforwarder.git
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=587e61ae4d75dc976f538088b715a3c8ee26c144
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_MAINTAINER:=Dennis <openwrt@tossp.com>
PKG_LICENSE:=GPL-3.0

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A simple DNS forwarder
  URL:=https://github.com/holmium/dnsforwarder
  DEPENDS:=+libpthread +coreutils-base64
endef

define Package/$(PKG_NAME)/description
Forwarding queries to customized domains (and their subdomains) to specified servers over a specified protocol (UDP or TCP). non-standard ports are supported.
endef

CONFIGURE_ARGS += --enable-downloader=no

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS +=  -Wl,--gc-sections,--as-needed,--no-undefined,--no-allow-shlib-undefined -flto

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dnsforwarder $(1)/usr/sbin/dnsforwarder
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
