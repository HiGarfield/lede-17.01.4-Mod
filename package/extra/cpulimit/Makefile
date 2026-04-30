include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_VERSION:=9d2a3cb9fd8ac7755840ae15561fcd0dc3c25767
PKG_MIRROR_HASH:=f257d492c9fec967238aa36eba44df919e58e9350058b2392d7f4a83e4c91ee7
PKG_VERSION:=$(PKG_SOURCE_VERSION)
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/cpulimit
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CPU usage limiter
  URL:=https://github.com/HiGarfield/cpulimit.git
  DEPENDS:=+librt
endef

CMAKE_OPTIONS += \
	-DCPULIMIT_BUILD_TESTS=OFF

define Package/cpulimit/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/cpulimit \
		$(1)/usr/bin/
endef

$(eval $(call BuildPackage,cpulimit))
