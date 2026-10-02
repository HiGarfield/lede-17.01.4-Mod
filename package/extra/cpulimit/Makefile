include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_DATE:=2026-10-03
PKG_SOURCE_VERSION:=99d937043bf947e53b4da764ffcbc1c2dcbfa2f7
PKG_MIRROR_HASH:=755ce7c6e1f33910bd1534145b48542b2ab088ba9c431098f04d72c099123457
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
