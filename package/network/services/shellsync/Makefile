include $(TOPDIR)/rules.mk

PKG_NAME:=shellsync
PKG_VERSION:=0.2
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

define Package/shellsync
  CATEGORY:=Utilities
  TITLE:=Sync shell scripts
  DEPENDS:=+libpthread +kmod-macvlan
endef

define Package/shellsync/description
A tool to sync different shell scripts.
Based on syncppp patch by morfast.
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed -flto

define Package/shellsync/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/shellsync $(1)/usr/bin
endef

$(eval $(call BuildPackage,shellsync))
