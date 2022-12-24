include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit
PKG_VERSION:=0dcf4d2317dde34c08f910626bc3e27d35695fa0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

UNAME:=Linux

define Package/cpulimit
  CATEGORY:=Utilities
  TITLE:=cpulimit
  URL:=https://github.com/opsengine/cpulimit.git
  DEPENDS:= +librt
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -lrt -Wl,--gc-sections,--as-needed -flto

define Package/cpulimit/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpulimit $(1)/usr/bin
endef

$(eval $(call BuildPackage,cpulimit))
