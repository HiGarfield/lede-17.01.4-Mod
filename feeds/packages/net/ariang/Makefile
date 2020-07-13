include $(TOPDIR)/rules.mk

PKG_NAME:=ariang
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/mayswind/AriaNg-DailyBuild
PKG_SOURCE_DATE:=2020-05-03
PKG_SOURCE_VERSION:=f60d092f01db9654479d942330fa83dea592ed6a
PKG_MIRROR_HASH:=6c847b7d52c2c309a9a83aec69edcac5d454f832fbe55d3975affe93b753db84

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Ansuel Smith <ansuelsmth@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/ariang/default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Download Manager
  DEPENDS:=+aria2
  TITLE:=AriaNg webui
  URL:=https://github.com/mayswind/AriaNg
  PKGARCH:=all
endef

define Package/ariang/description
	AriaNg is a web frontend making aria2 easier to use. AriaNg is written in pure html & javascript, thus it does not need any compilers or runtime environment.
endef

Package/ariang-nginx/description = $(Package/ariang/description)

define Package/ariang
  $(Package/ariang/default)
  DEPENDS += +uhttpd
  TITLE += for uhttpd webserver
endef

define Package/ariang-nginx
  $(Package/ariang/default)
  DEPENDS += +nginx
  TITLE += for nginx webserver
endef

define Build/Compile
endef

define Package/ariang/install
	$(INSTALL_DIR) \
		$(1)/www/ariang

	$(CP) \
		$(PKG_BUILD_DIR)/{langs,css,js,fonts} \
		$(PKG_BUILD_DIR)/index.html \
		$(PKG_BUILD_DIR)/index.manifest \
		$(PKG_BUILD_DIR)/LICENSE \
		$(PKG_BUILD_DIR)/favicon.ico \
		$(1)/www/ariang
endef

define Package/ariang-nginx/install
	$(Package/ariang/install)
	$(INSTALL_DIR) $(1)/etc/nginx
	$(INSTALL_BIN) ./files/ariang.conf $(1)/etc/nginx/ariang.conf
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/80_ariang-nginx-support $(1)/etc/uci-defaults/80_ariang-nginx-support
endef

$(eval $(call BuildPackage,ariang))
$(eval $(call BuildPackage,ariang-nginx))
