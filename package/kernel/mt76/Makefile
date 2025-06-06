include $(TOPDIR)/rules.mk

PKG_NAME:=mt76

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/HiGarfield/mt76
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=3d65a03f8380164644613a58cfc2f38fd7c9cbd3
PKG_MIRROR_HASH:=e40cfe8854ca0247a5da5fe2d06ba44871e6f60b1662203bbfcd3e6c17f0720c

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_BUILD_PARALLEL:=1

PKG_CONFIG_DEPENDS += \
	CONFIG_PACKAGE_kmod-mt76-usb \
	CONFIG_PACKAGE_kmod-mt76x02-common \
	CONFIG_PACKAGE_kmod-mt76x0-common \
	CONFIG_PACKAGE_kmod-mt76x0u \
	CONFIG_PACKAGE_kmod-mt76x2-common \
	CONFIG_PACKAGE_kmod-mt76x2 \
	CONFIG_PACKAGE_kmod-mt76x2u \
	CONFIG_PACKAGE_kmod-mt7603

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt76-default
  SUBMENU:=Wireless Drivers
  DEPENDS:= \
	+kmod-mac80211 \
	+@DRIVER_11AC_SUPPORT +@DRIVER_11N_SUPPORT +@DRIVER_11W_SUPPORT
endef

define KernelPackage/mt76
  SUBMENU:=Wireless Drivers
  TITLE:=MediaTek MT76x2/MT7603 wireless driver (metapackage)
  DEPENDS:= \
	+kmod-mt76-core +kmod-mt76x2 +kmod-mt7603
endef

define KernelPackage/mt76-core
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76xx wireless driver
  HIDDEN:=1
  FILES:=\
	$(PKG_BUILD_DIR)/mt76.ko
endef

define KernelPackage/mt76-usb
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76xx wireless driver USB support
  DEPENDS += +kmod-usb-core +kmod-mt76-core
  HIDDEN:=1
  FILES:=\
	$(PKG_BUILD_DIR)/mt76-usb.ko
endef

define KernelPackage/mt76x02-usb
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76x0/MT76x2 USB wireless driver common code
  DEPENDS+=+kmod-mt76-usb +kmod-mt76x02-common
  HIDDEN:=1
  FILES:=$(PKG_BUILD_DIR)/mt76x02-usb.ko
endef

define KernelPackage/mt76x02-common
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76x0/MT76x2 wireless driver common code
  DEPENDS+=+kmod-mt76-core
  HIDDEN:=1
  FILES:=$(PKG_BUILD_DIR)/mt76x02-lib.ko
endef

define KernelPackage/mt76x0-common
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76x0 wireless driver common code
  DEPENDS+=+kmod-mt76x02-common
  HIDDEN:=1
  FILES:=$(PKG_BUILD_DIR)/mt76x0/mt76x0-common.ko
endef

define KernelPackage/mt76x0e
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76x0E wireless driver
  DEPENDS+=@PCI_SUPPORT +kmod-mt76x0-common
  FILES:=\
	$(PKG_BUILD_DIR)/mt76x0/mt76x0e.ko
  AUTOLOAD:=$(call AutoProbe,mt76x0e)
endef

define KernelPackage/mt76x0u
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76x0U wireless driver
  DEPENDS+=+kmod-mt76x0-common +kmod-mt76x02-usb
  FILES:=\
	$(PKG_BUILD_DIR)/mt76x0/mt76x0u.ko
  AUTOLOAD:=$(call AutoProbe,mt76x0u)
endef

define KernelPackage/mt76x2-common
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76x2 wireless driver common code
  DEPENDS+=+kmod-mt76-core +kmod-mt76x02-common
  HIDDEN:=1
  FILES:=$(PKG_BUILD_DIR)/mt76x2/mt76x2-common.ko
endef

define KernelPackage/mt76x2u
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76x2U wireless driver
  DEPENDS+=+kmod-mt76x2-common +kmod-mt76x02-usb
  FILES:=\
	$(PKG_BUILD_DIR)/mt76x2/mt76x2u.ko
  AUTOLOAD:=$(call AutoProbe,mt76x2u)
endef

define KernelPackage/mt76x2
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76x2 wireless driver
  DEPENDS+=@PCI_SUPPORT +kmod-mt76x2-common
  FILES:=\
	$(PKG_BUILD_DIR)/mt76x2/mt76x2e.ko
  AUTOLOAD:=$(call AutoProbe,mt76x2e)
endef

define KernelPackage/mt7603
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT7603 wireless driver
  DEPENDS+=@PCI_SUPPORT +kmod-mt76-core
  FILES:=\
	$(PKG_BUILD_DIR)/mt7603/mt7603e.ko
  AUTOLOAD:=$(call AutoProbe,mt7603e)
endef

define KernelPackage/mt7615-common
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT7615 wireless driver common code
  HIDDEN:=1
  DEPENDS+=@PCI_SUPPORT +kmod-mt76-core
  FILES:= $(PKG_BUILD_DIR)/mt7615/mt7615-common.ko
endef

define KernelPackage/mt7615e
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT7615e wireless driver
  DEPENDS+=@PCI_SUPPORT +kmod-mt7615-common
  FILES:=\
	$(PKG_BUILD_DIR)/mt7615/mt7615e.ko
  AUTOLOAD:=$(call AutoProbe,mt7615e)
endef

define KernelPackage/mt7663-firmware-ap
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT7663e firmware (optimized for AP)
  DEPENDS+=+kmod-mt7615e
  FILES:=\
	$(PKG_BUILD_DIR)/mt7615/mt7615e.ko
endef

define KernelPackage/mt7663-firmware-sta
 $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT7663e firmware (client mode offload)
  DEPENDS+=+kmod-mt7615e
  FILES:=\
	$(PKG_BUILD_DIR)/mt7615/mt7615e.ko
endef

NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR) \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-include backport/autoconf.h \
	-include backport/backport.h

ifdef CONFIG_PACKAGE_MAC80211_MESH
  NOSTDINC_FLAGS += -DCONFIG_MAC80211_MESH
endif

ifdef CONFIG_PACKAGE_kmod-mt76-usb
  PKG_MAKE_FLAGS += CONFIG_MT76_USB=m
endif
ifdef CONFIG_PACKAGE_kmod-mt76x02-common
  PKG_MAKE_FLAGS += CONFIG_MT76x02_LIB=m
endif
ifdef CONFIG_PACKAGE_kmod-mt76x02-usb
  PKG_MAKE_FLAGS += CONFIG_MT76x02_USB=m
endif
ifdef CONFIG_PACKAGE_kmod-mt76x0-common
  PKG_MAKE_FLAGS += CONFIG_MT76x0_COMMON=m
endif
ifdef CONFIG_PACKAGE_kmod-mt76x0e
  PKG_MAKE_FLAGS += CONFIG_MT76x0E=m
endif
ifdef CONFIG_PACKAGE_kmod-mt76x0u
  PKG_MAKE_FLAGS += CONFIG_MT76x0U=m
endif
ifdef CONFIG_PACKAGE_kmod-mt76x2-common
  PKG_MAKE_FLAGS += CONFIG_MT76x2_COMMON=m
endif
ifdef CONFIG_PACKAGE_kmod-mt76x2
  PKG_MAKE_FLAGS += CONFIG_MT76x2E=m
endif
ifdef CONFIG_PACKAGE_kmod-mt76x2u
  PKG_MAKE_FLAGS += CONFIG_MT76x2U=m
endif
ifdef CONFIG_PACKAGE_kmod-mt7603
  PKG_MAKE_FLAGS += CONFIG_MT7603E=m
endif
ifdef CONFIG_PACKAGE_kmod-mt7615-common
  PKG_MAKE_FLAGS += CONFIG_MT7615_COMMON=m
endif
ifdef CONFIG_PACKAGE_kmod-mt7615e
  PKG_MAKE_FLAGS += CONFIG_MT7615E=m
  ifdef CONFIG_TARGET_mediatek_mt7622
    PKG_MAKE_FLAGS += CONFIG_MT7622_WMAC=y
    NOSTDINC_FLAGS += -DCONFIG_MT7622_WMAC
  endif
endif

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

define Package/kmod-mt76/install
	true
endef

define KernelPackage/mt76x0-common/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
	cp \
		$(PKG_BUILD_DIR)/firmware/mt7610e.bin \
		$(1)/lib/firmware/mediatek
endef

define KernelPackage/mt76x2-common/install
	$(INSTALL_DIR) $(1)/lib/firmware
	cp \
		$(PKG_BUILD_DIR)/firmware/mt7662_rom_patch.bin \
		$(PKG_BUILD_DIR)/firmware/mt7662.bin \
		$(1)/lib/firmware
endef

define KernelPackage/mt76x0u/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
	ln -sf mt7610e.bin $(1)/lib/firmware/mediatek/mt7610u.bin
endef

define KernelPackage/mt76x2u/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
	ln -sf ../mt7662.bin $(1)/lib/firmware/mediatek/mt7662u.bin
	ln -sf ../mt7662_rom_patch.bin $(1)/lib/firmware/mediatek/mt7662u_rom_patch.bin
endef

define KernelPackage/mt7603/install
	$(INSTALL_DIR) $(1)/lib/firmware
	cp $(if $(CONFIG_TARGET_ramips_mt7628)$(CONFIG_TARGET_ramips_mt7688), \
		$(PKG_BUILD_DIR)/firmware/mt7628_e1.bin \
		$(PKG_BUILD_DIR)/firmware/mt7628_e2.bin \
		,\
		$(PKG_BUILD_DIR)/firmware/mt7603_e1.bin \
		$(PKG_BUILD_DIR)/firmware/mt7603_e2.bin \
		) \
		$(1)/lib/firmware
endef

define KernelPackage/mt7615e/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
	cp \
		$(PKG_BUILD_DIR)/firmware/mt7615_cr4.bin \
		$(PKG_BUILD_DIR)/firmware/mt7615_n9.bin \
		$(PKG_BUILD_DIR)/firmware/mt7615_rom_patch.bin \
		$(if $(CONFIG_TARGET_mediatek_mt7622), \
			$(PKG_BUILD_DIR)/firmware/mt7622_n9.bin \
			$(PKG_BUILD_DIR)/firmware/mt7622_rom_patch.bin) \
		$(1)/lib/firmware/mediatek
endef

define KernelPackage/mt7663-firmware-ap/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
	cp \
		$(PKG_BUILD_DIR)/firmware/mt7663_n9_rebb.bin \
		$(PKG_BUILD_DIR)/firmware/mt7663pr2h_rebb.bin \
		$(1)/lib/firmware/mediatek
endef

define KernelPackage/mt7663-firmware-sta/install
	$(INSTALL_DIR) $(1)/lib/firmware/mediatek
	cp \
		$(PKG_BUILD_DIR)/firmware/mt7663_n9_v3.bin \
		$(PKG_BUILD_DIR)/firmware/mt7663pr2h.bin \
		$(1)/lib/firmware/mediatek
endef

$(eval $(call KernelPackage,mt76-core))
$(eval $(call KernelPackage,mt76-usb))
$(eval $(call KernelPackage,mt76x02-usb))
$(eval $(call KernelPackage,mt76x02-common))
$(eval $(call KernelPackage,mt76x0-common))
$(eval $(call KernelPackage,mt76x0e))
$(eval $(call KernelPackage,mt76x0u))
$(eval $(call KernelPackage,mt76x2-common))
$(eval $(call KernelPackage,mt76x2u))
$(eval $(call KernelPackage,mt76x2))
$(eval $(call KernelPackage,mt7603))
$(eval $(call KernelPackage,mt7615-common))
$(eval $(call KernelPackage,mt7615e))
$(eval $(call KernelPackage,mt7663-firmware-ap))
$(eval $(call KernelPackage,mt7663-firmware-sta))
$(eval $(call KernelPackage,mt76))
