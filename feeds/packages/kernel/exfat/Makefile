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
PKG_SOURCE_VERSION:=7f592ba5718e3e757cd0edd2d934fe2fe87f7d61
PKG_MIRROR_HASH:=df6c69f90db577dd23245dd4dcf05f5aa29c81348dda4aa5ebec578d563049ea

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
