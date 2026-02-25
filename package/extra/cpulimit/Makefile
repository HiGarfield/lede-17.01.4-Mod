include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/HiGarfield/cpulimit.git
PKG_SOURCE_VERSION:=e50ff99f08a42460caa9d3f1a2e9cfdfae3242c7
PKG_MIRROR_HASH:=9ff83d064f30c5ace0a15ed994c0ed7e6c6179074192fcc73b4017ff45d02aef
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
