#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=exfat
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_URL:=https://github.com/namjaejeon/linux-exfat-oot
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=3a977dbbe0fc62137bb037cd82b9a0dd9ea27fd3
PKG_MIRROR_HASH:=5bc8dddd3b8af2c346c29e83aa3b2290bb11d62082a0884dc85cec69fb994025

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-exfat
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Filesystems
  TITLE:=exFAT kernel module
  URL:=https://github.com/namjaejeon/linux-exfat-oot
  FILES:=$(PKG_BUILD_DIR)/exfat.ko
  AUTOLOAD:=$(call AutoProbe,exfat)
  DEPENDS:= +kmod-nls-base
endef

define KernelPackage/exfat/description
  This package provides the kernel module for exfat.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(PKG_EXTRA_KCONFIG) \
	CONFIG_EXFAT_FS=m \
	modules
endef

$(eval $(call KernelPackage,fs-exfat))
