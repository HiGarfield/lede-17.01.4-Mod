
include $(TOPDIR)/rules.mk

PKG_SOURCE_URL:=https://github.com/meyerd/n2n
PKG_SOURCE_VERSION:=33fa8874a89cb01dc7d52f8060b90c0cebf627bb

PKG_NAME:=n2n_v2
PKG_VERSION:=2.1-$(PKG_SOURCE_VERSION)
PKG_RELEASE:=1
PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_SOURCE_PROTO:=git

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/n2n_v2
  SECTION:=net
  CATEGORY:=Network
  TITLE:=N2N VPN tunneling daemon(V2)
  URL:=http://www.ntop.org/n2n/
  SUBMENU:=VPN
  DEPENDS:=+kmod-tun +resolveip +libopenssl
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed,--no-undefined,--no-allow-shlib-undefined

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) $(PKG_BUILD_DIR)/n2n_v2/* $(PKG_BUILD_DIR)
endef

define Package/n2n_v2/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/edge $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/supernode $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,n2n_v2))
