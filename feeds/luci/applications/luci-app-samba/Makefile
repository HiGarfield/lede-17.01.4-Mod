#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Network Shares - Samba SMB/CIFS module
LUCI_DEPENDS:=+samba36-server \
				+@BUSYBOX_CONFIG_ADDUSER \
				+@BUSYBOX_CONFIG_FEATURE_CHECK_NAMES

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
