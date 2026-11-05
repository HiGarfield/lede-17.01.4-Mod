include $(TOPDIR)/rules.mk

# Only the prebuilt, complete release archive is consumed here. There is no
# npm/node build, no source checkout and no patching of the shipped assets.
#
# Release convention (keep all four lines in sync when bumping PKG_VERSION):
#   tag           : v$(PKG_VERSION)
#   asset         : transmission-web-v$(PKG_VERSION).zip
#   download base : https://github.com/jianxcao/transmission-web/releases/download/v$(PKG_VERSION)
#   sha256        : assets[].digest from releases/tags/v$(PKG_VERSION), or sha256sum of the zip
#
# The zip contains index.html, assets/, sw.js, manifest.webmanifest and icons
# directly at its root (no enclosing directory). UNPACK_CMD puts them into a
# "transmission-web" subdir of $(PKG_BUILD_DIR), and the install rule copies
# that subdir. Re-check "unzip -Z1" on every version bump: if an enclosing
# directory is ever added, drop it with "unzip -j" or adjust the copy path.
PKG_NAME:=transmission-web-jianxcao
PKG_VERSION:=0.1.9
PKG_RELEASE:=1

PKG_SOURCE:=transmission-web-v$(PKG_VERSION).zip
PKG_SOURCE_URL:=https://github.com/jianxcao/transmission-web/releases/download/v$(PKG_VERSION)
PKG_HASH:=d0ed7c9fe2b0dc7c270a935a70629959c950b49d7d7ed336b5e8e72efedc2b4c

# The release zip has its assets at the root with no enclosing directory.
# Unpack into a "transmission-web" subdir of $(PKG_BUILD_DIR) so the install
# rule can copy that subdir only, keeping the package framework's "ipkg-all"
# directory at the top level out of the copy.
UNPACK_CMD=unzip -q -d $(1)/transmission-web $(DL_DIR)/$(PKG_SOURCE)

PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/transmission-web-jianxcao
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=BitTorrent
  DEPENDS:=+transmission-daemon
  CONFLICTS:=transmission-web transmission-web-control
  TITLE:=Transmission Web by jianxcao
  URL:=https://github.com/jianxcao/transmission-web
  PKGARCH:=all
endef

define Package/transmission-web-jianxcao/description
  A prebuilt web interface for Transmission, installed from the complete
  release archive of jianxcao/transmission-web.
endef

define Build/Compile
endef

define Package/transmission-web-jianxcao/install
	$(INSTALL_DIR) $(1)/usr/share/transmission/web
	$(CP) $(PKG_BUILD_DIR)/transmission-web/. $(1)/usr/share/transmission/web
endef

$(eval $(call BuildPackage,transmission-web-jianxcao))
