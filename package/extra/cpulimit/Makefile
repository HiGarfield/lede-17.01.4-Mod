include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_DATE:=2026-11-01
PKG_SOURCE_VERSION:=b0d8918e529581c2eff532387b142c0f54a484de
PKG_MIRROR_HASH:=0df6d185f55286da76098e9f2f66a6ed0265c61e2e540afb591cf55971ea3e0c
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/cpulimit
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CPU usage limiter
  URL:=https://github.com/HiGarfield/cpulimit
  DEPENDS:=+PACKAGE_librt:librt
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
