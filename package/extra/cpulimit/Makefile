include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_VERSION:=c7cb43ef80d2a222002a2e6aec0e5eabb6712938
PKG_MIRROR_HASH:=924fdc1751eccb0176f621a2e45cd74e6262e62d8204bff8a6d50fe1ad1e0353

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
