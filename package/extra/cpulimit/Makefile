include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_VERSION:=d56108ca2d3787b03ceb4519bb5867bddd952271
PKG_MIRROR_HASH:=b37d5c0ebc15001229f715ea5829ac6d81b5d5788e03c7c3005adfc00444aef4

include $(INCLUDE_DIR)/package.mk

define Package/cpulimit
  CATEGORY:=Utilities
  TITLE:=cpulimit
  URL:=https://github.com/HiGarfield/cpulimit.git
  DEPENDS:= +librt
endef

MAKE_PATH:=src
MAKE_VARS += UNAME="Linux"
TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -lrt -Wl,--gc-sections,--as-needed -flto

define Package/cpulimit/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(MAKE_PATH)/cpulimit $(1)/usr/bin
endef

$(eval $(call BuildPackage,cpulimit))
