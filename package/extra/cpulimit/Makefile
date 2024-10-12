include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_VERSION:=c7ce7ea6a5f6aeafb37a0eb6e132603fccb02956
PKG_MIRROR_HASH:=ddadb5770d2e43549969e53235c74b55fea435d9aa22950893b3c3dcc4e995d9

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
