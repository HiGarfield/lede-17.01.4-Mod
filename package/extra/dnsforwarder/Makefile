include $(TOPDIR)/rules.mk

PKG_NAME:=dnsforwarder
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/lifenjoiner/dnsforwarder
PKG_SOURCE_VERSION:=2a46406fe7c7d9baceea1e6ba2653d29ea9c465f
PKG_MIRROR_HASH:=349974bbe40524b06a9efe4007f0c50adb5d2c712680e4b31dd77ad2baec59c5

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/dnsforwarder/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A simple DNS forwarder
  URL:=https://github.com/lifenjoiner/dnsforwarder
  DEPENDS:=+libpthread +coreutils-base64
  PROVIDES:=dnsforwarder
endef

define Package/dnsforwarder-nodl
  $(call Package/dnsforwarder/Default)
  TITLE+= (without downloader)
  VARIANT:=no
  CONFLICTS:=
endef

define Package/dnsforwarder-wget
  $(call Package/dnsforwarder/Default)
  TITLE+= (with downloader based on wget)
  DEPENDS+=+wget-ssl
  VARIANT:=wget
  CONFLICTS:=dnsforwarder-nodl
endef

define Package/dnsforwarder-libcurl
  $(call Package/dnsforwarder/Default)
  TITLE+= (with downloader based on libcurl)
  DEPENDS+=+libcurl
  VARIANT:=libcurl
  DEFAULT_VARIANT:=1
  CONFLICTS:=dnsforwarder-nodl dnsforwarder-wget
endef

define Package/dnsforwarder/description/Default
Forwarding queries to customized domains (and their subdomains) to specified servers over a specified protocol (UDP or TCP). non-standard ports are supported.
endef

Package/dnsforwarder-nodl/description = $(Package/dnsforwarder/description/Default)
Package/dnsforwarder-wget/description = $(Package/dnsforwarder/description/Default)
Package/dnsforwarder-libcurl/description = $(Package/dnsforwarder/description/Default)

CONFIGURE_ARGS += --enable-downloader=$(BUILD_VARIANT)

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed -flto

define Package/dnsforwarder/install/Default
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dnsforwarder $(1)/usr/sbin/
endef

Package/dnsforwarder-nodl/install = $(Package/dnsforwarder/install/Default)
Package/dnsforwarder-wget/install = $(Package/dnsforwarder/install/Default)
Package/dnsforwarder-libcurl/install = $(Package/dnsforwarder/install/Default)

$(eval $(call BuildPackage,dnsforwarder-nodl))
$(eval $(call BuildPackage,dnsforwarder-wget))
$(eval $(call BuildPackage,dnsforwarder-libcurl))
