#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mips
BOARD:=brcm63xx
BOARDNAME:=Broadcom BCM63xx
SUBTARGETS:=brcm6338 brcm6345 brcm6348 brcm6358 brcm6368
FEATURES:=squashfs usb atm pci pcmcia
LINUX_VERSION:=2.6.32.61

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += kmod-switch kmod-input-gpio-buttons \
	kmod-button-hotplug

define Target/Description
	Build firmware images for Broadcom based xDSL/routers.
	Supports: Broadcom 6338/6345/6348/6358/6368.
endef

$(eval $(call BuildTarget))
