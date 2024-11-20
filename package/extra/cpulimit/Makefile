include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_VERSION:=d6a03fcf0a043f95813fdfb43ee4a0e15aa17b15
PKG_MIRROR_HASH:=358e81a99b73fb01e14fd84987eeb1492dc370d4632a1b4d4acc72939f2f0568

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
