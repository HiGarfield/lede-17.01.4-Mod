# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=irqbalance
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/Irqbalance/irqbalance.git
PKG_SOURCE_VERSION:=9322ac1353d39839a3b76ddfaeb6392cb259ff0f
PKG_MIRROR_HASH:=a9f41bcc037663d3aab386c694b578e7d104afe3a1e2e787501a051611d7235c

PKG_MAINTAINER:=Hannu Nyman <hannu.nyman@iki.fi>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_REMOVE_FILES:=autogen.sh

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DEPENDS += glib2

define Package/irqbalance
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=$(ICONV_DEPENDS) $(INTL_DEPENDS)
  TITLE:=IRQ usage balancing for multi-core systems
  URL:=https://github.com/Irqbalance/irqbalance
endef

define Package/irqbalance/description
 The purpose of irqbalance is to distribute hardware interrupts across
 processors/cores on a multiprocessor/multicore system in order to
 increase performance.
endef

CONFIGURE_ARGS+= \
	--disable-numa \
	--with-libcap_ng=no \
	--with-systemd=no \
	--without-irqbalance-ui \
	--enable-static=glib2

GLIB2_LIBS="$(STAGING_DIR)/usr/lib/libglib-2.0.a"

ifeq ($(CONFIG_BUILD_NLS),y)
	GLIB2_LIBS += $(INTL_LDFLAGS) -lintl
endif

ifeq ($(CONFIG_USE_GLIBC),y)
	GLIB2_LIBS += -lpthread
endif

CONFIGURE_VARS += \
	GLIB2_LIBS="$(GLIB2_LIBS) \
		$(ICONV_LDFLAGS) -liconv"

define Package/irqbalance/conffiles
/etc/config/irqbalance
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed -flto

define Package/irqbalance/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/irqbalance $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/irqbalance.init $(1)/etc/init.d/irqbalance
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/irqbalance.config $(1)/etc/config/irqbalance
endef

$(eval $(call BuildPackage,irqbalance))
