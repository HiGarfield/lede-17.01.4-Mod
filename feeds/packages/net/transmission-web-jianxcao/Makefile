include $(TOPDIR)/rules.mk

PKG_NAME:=transmission-web-jianxcao
PKG_VERSION:=0.2.2
PKG_RELEASE:=1

PKG_SOURCE:=transmission-web-v$(PKG_VERSION).zip
PKG_SOURCE_URL:=https://github.com/jianxcao/transmission-web/releases/download/v$(PKG_VERSION)
PKG_HASH:=f6f24f85f5009c7158b59d3f7ca8aeffab12fe80c1f1c2c51b5c5be376f49e8e

UNPACK_CMD=unzip -q -d $(1)/transmission-web $(DL_DIR)/$(PKG_SOURCE)

PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/transmission-web-jianxcao
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=BitTorrent
  DEPENDS:=+transmission-daemon
  CONFLICTS:=transmission-web transmission-web-control
  TITLE:=Transmission Web by jianxcao
  URL:=https://github.com/jianxcao/transmission-web
  PKGARCH:=all
endef

define Package/transmission-web-jianxcao/description
  A prebuilt web interface for Transmission, installed from the complete
  release archive of jianxcao/transmission-web.
endef

define Build/Compile
endef

define Package/transmission-web-jianxcao/install
	$(INSTALL_DIR) $(1)/usr/share/transmission/web
	$(CP) $(PKG_BUILD_DIR)/transmission-web/. $(1)/usr/share/transmission/web
endef

$(eval $(call BuildPackage,transmission-web-jianxcao))
