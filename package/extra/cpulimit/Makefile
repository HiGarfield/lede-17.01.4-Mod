include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_VERSION:=c82e286de34550c6d5c4a973177efd29aabfb63e
PKG_MIRROR_HASH:=1c5b8dabb5e8d63eabc7b660e13c55a8682664c59ef5e155501136092efdb758

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
