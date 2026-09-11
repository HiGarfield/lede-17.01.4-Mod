#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=automake
PKG_VERSION:=1.15

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/automake
PKG_HASH:=9908c75aabd49d13661d6dcb1bc382252d22cc77bf733a2d55e87f2aa2db8636

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--datarootdir=$(STAGING_DIR_HOST)/share \
	--disable-silent-rules

HOST_CONFIGURE_VARS += \
	PERL="/usr/bin/env perl" \
	am_cv_prog_PERL_ithreads=no

define Host/Configure
	(cd $(HOST_BUILD_DIR); $(AM_TOOL_PATHS) STAGING_DIR="" ./bootstrap.sh)
	$(call Host/Configure/Default)
endef

define Host/Install
	# remove old automake resources to avoid version conflicts
	rm -rf $(STAGING_DIR_HOST)/share/aclocal-[0-9]*
	rm -rf $(STAGING_DIR_HOST)/share/automake-[0-9]*
	$(MAKE) -C $(HOST_BUILD_DIR) install
	mv $(STAGING_DIR_HOST)/bin/aclocal $(STAGING_DIR_HOST)/bin/aclocal.real
	$(INSTALL_BIN) ./files/aclocal $(STAGING_DIR_HOST)/bin
	# automake 1.15's py-compile imports the removed 'imp' module and fails
	# under Python 3.12+ while byte-compiling installed .py files (e.g. glib's
	# gdbus-codegen).  py_compile computes the cache path itself, so drop the
	# imp dependency from the installed script.
	python3 $(TOPDIR)/tools/automake/files/fixup-py-compile.py $(STAGING_DIR_HOST)/share/automake-1.15/py-compile
	# A tarball's Makefile.in invokes aclocal/automake by the version it was
	# cut with - aria2 1.37.0 wants 1.16, flex 2.6.1 wants 1.15 - and the
	# rebuild rules abort with "command not found" when that name is missing,
	# even when nothing really needs regenerating.  The installed automake is
	# 1.15, so every versioned name simply points at the real 1.15 tools; the
	# aclocal.m4 written and the automake that reads it are then the same 1.15
	# release, so there is no version mismatch to trip.
	( \
		api=18; \
		while [ "$$$$api" -ge 9 ]; do \
			ln -sf aclocal "$(STAGING_DIR_HOST)/bin/aclocal-1.$$$$api"; \
			ln -sf automake "$(STAGING_DIR_HOST)/bin/automake-1.$$$$api"; \
			api=$$$$(($$$$api - 1)); \
		done; \
		ln -sf aclocal "$(STAGING_DIR_HOST)/bin/aclocal-1.11.6"; \
		ln -sf automake "$(STAGING_DIR_HOST)/bin/automake-1.11.6"; \
	)
endef

define Host/Clean
	-$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
endef

$(eval $(call HostBuild))
