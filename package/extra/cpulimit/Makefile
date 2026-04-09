include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_VERSION:=85e8cf183988f4300fc61629ab7d3c18439c70a7
PKG_MIRROR_HASH:=74375fc286a672b6ac46d99b5b71eda50d674caf29fb9f87a7486360cb0108c7
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
