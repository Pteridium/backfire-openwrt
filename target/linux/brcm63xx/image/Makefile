# 
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

LOADADDR = 0x80010000		# RAM start + 16M 
KERNEL_ENTRY = $(LOADADDR)	# Newer kernels add a jmp to the kernel_entry at the start of the binary
RAMSIZE = 0x01000000		# 64MB

LOADER_MAKEOPTS= \
		KDIR=$(KDIR) \
		LOADADDR=$(LOADADDR) \
		KERNEL_ENTRY=$(KERNEL_ENTRY) \
		RAMSIZE=$(RAMSIZE)

define trxalign/jffs2-128k
-a 0x20000
endef
define trxalign/jffs2-64k
-a 0x10000
endef
define trxalign/squashfs
-a 1024
endef

define Image/LimitName16
$(shell expr substr "$(1)" 1 16)
endef

define Image/Build/CFE
	# Generate the tagged image
	$(STAGING_DIR_HOST)/bin/imagetag -i $(KDIR)/vmlinux.lzma.cfe -f $(KDIR)/root.$(1) \
		--output $(BIN_DIR)/openwrt-$(4)-$(1)-cfe.bin \
		--boardid $(2) --chipid $(3) --entry $(KERNEL_ENTRY) \
		--load-addr $(LOADADDR) --rsa-signature "$(5)" \
		--info1 "-$(call Image/LimitName16,$(4))" --info2 $(1) \
		$(6) $(7) $(8) $(9)
endef

define Image/Build/CFEFIXUP
	# Generate the tagged image
	$(STAGING_DIR_HOST)/bin/imagetag -i $(KDIR)/vmlinux.lzma.cfe -f $(KDIR)/root.$(1) \
		--output $(BIN_DIR)/openwrt-$(5)-$(1)-cfe.bin \
		--boardid $(2) --chipid $(4) --entry $(KERNEL_ENTRY) \
		--load-addr $(LOADADDR) --rsa-signature "$(6)" \
		--info1 "+$(call Image/LimitName16,$(3))" --info2 $(1) \
		$(7) $(8) $(9) $(10)
endef

define Image/Build/CFEAGPF
	# Generate the tagged image
	$(STAGING_DIR_HOST)/bin/imagetag -i $(KDIR)/vmlinux.lzma.cfe -f $(KDIR)/root.$(1) \
		--output $(BIN_DIR)/openwrt-$(5)-$(1)-cfe.bin \
		--boardid $(2) --chipid $(3) --entry $(LOADADDR) \
		--load-addr $(LOADADDR) --tag-version 8 \
		--signature2 IMAGE --block-size 0x20000 \
		--image-offset $(4) --info1 "-$(call Image/LimitName16,$(5))" --info2 $(1)
endef

define Image/Build/RG100A
	# Generate the tagged image
	$(STAGING_DIR_HOST)/bin/imagetag -i $(KDIR)/vmlinux.lzma.cfe -f $(KDIR)/root.$(1) \
		--output $(BIN_DIR)/openwrt-$(5)-$(1)-cfe.bin \
		--boardid $(2) --chipid $(3) --entry $(LOADADDR) \
		--load-addr $(LOADADDR) --block-size 0x20000 \
		--image-offset $(4) --info1 "-$(call Image/LimitName16,$(5))" --info2 $(1)
endef

define Image/Build/RedBoot
	cp $(KDIR)/vmlinux.elf $(BIN_DIR)/openwrt-$(1)-vmlinux.elf
	gzip -9 -c $(KDIR)/vmlinux > $(KDIR)/vmlinux.bin.gz
	$(STAGING_DIR_HOST)/bin/lzma e $(KDIR)/vmlinux $(KDIR)/vmlinux.bin.l7
	dd if=$(KDIR)/vmlinux.bin.l7 of=$(BIN_DIR)/openwrt-$(1)-vmlinux.lzma bs=65536 conv=sync
	dd if=$(KDIR)/vmlinux.bin.gz of=$(BIN_DIR)/openwrt-$(1)-vmlinux.gz bs=65536 conv=sync
endef

define Image/Build/CFEOLD
	$(TOPDIR)/scripts/brcmImage.pl -t -p	\
		-b $(2) -c $(3)			\
		-k $(KDIR)/vmlinux.lzma.cfe	\
		-r $(KDIR)/root.$(1)		\
		-o $(BIN_DIR)/openwrt-$(2)-$(1)-cfe.bin
endef

define Build/Clean
	$(MAKE) -C lzma-loader clean
endef

define Image/Prepare
	# Standard LZMA kernel
	cat $(KDIR)/vmlinux | $(STAGING_DIR_HOST)/bin/lzma e -si -so -eos -lc1 -lp2 -pb2 > $(KDIR)/vmlinux.lzma

	# CFE is a LZMA nazi! It took me hours to find out the parameters!
	# Also I think lzma has a bug cause it generates different output depending on
	# if you use stdin / stdout or not. Use files instead of stdio here, cause
	# otherwise CFE will complain and not boot the image.
	$(STAGING_DIR_HOST)/bin/lzma e -d22 -fb64 -a1 $(KDIR)/vmlinux $(KDIR)/vmlinux.lzma.tmp

	# Strip out the length, CFE doesn't like this
	dd if=$(KDIR)/vmlinux.lzma.tmp of=$(KDIR)/vmlinux.lzma.cfe bs=5 count=1
	dd if=$(KDIR)/vmlinux.lzma.tmp of=$(KDIR)/vmlinux.lzma.cfe ibs=13 obs=5 skip=1 seek=1 conv=notrunc
	rm -f $(KDIR)/vmlinux.lzma.tmp

	# Build the LZMA loader
	rm -f $(KDIR)/loader.gz
	$(MAKE) -C lzma-loader \
		BUILD_DIR="$(KDIR)" \
		TARGET="$(KDIR)" \
		clean install

	echo -ne "\\x00" >> $(KDIR)/loader.gz
	rm -f $(KDIR)/fs_mark
	touch $(KDIR)/fs_mark
	$(call prepare_generic_squashfs,$(KDIR)/fs_mark)
endef

ifeq ($(CONFIG_TARGET_brcm63xx_brcm6338),y)

# Generic 6338GW
define Image/Build/Profile/6338GW
	$(call Image/Build/CFE,$(1),6338GW,6338,6338GW)
endef
# Generic 6338W
define Image/Build/Profile/6338W
	$(call Image/Build/CFE,$(1),6338W,6338,6338W)
endef

endif

ifeq ($(CONFIG_TARGET_brcm63xx_brcm6345),y)

# Generic 96345GW2
define Image/Build/Profile/96345GW2
	$(call Image/Build/CFE,$(1),96345GW2,6345,96345GW2)
endef

endif

ifeq ($(CONFIG_TARGET_brcm63xx_brcm6348),y)

# Comtrend CT-5621
define Image/Build/Profile/CT5621
	$(call Image/Build/CFE,$(1),96348GW-11,6348,CT-5621)
endef
# Comtrend CT-536+
define Image/Build/Profile/CT536PLUS
	$(call Image/Build/CFE,$(1),96348GW-11,6348,CT-536+)
endef
# Comtrend CT-5361
define Image/Build/Profile/CT5361
	$(call Image/Build/CFE,$(1),96348GW-11,6348,CT-5361)
endef

endif

ifeq ($(CONFIG_TARGET_brcm63xx_brcm6358),y)

# D-Link DSL-2650U
define Image/Build/Profile/DSL2650U
	$(call Image/Build/CFE,$(1),96358VW2,6358,DSL-2650U)
endef

# Huawei HG520v
define Image/Build/Profile/HG520V
	$(call Image/Build/CFE,$(1),HW6358GW_B,6358,HG520v)
endef

endif

ifeq ($(CONFIG_TARGET_brcm63xx_brcm6368),y)

# Comtrend VR-3025u
define Image/Build/Profile/VR3025U
        $(call Image/Build/CFE,$(1),96368M-1541N,6368,VR-3025u,,--pad 16)
endef

endif

define Image/Build
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync

	$(call Image/Build/Profile/$(PROFILE),$(1))
endef

$(eval $(call BuildImage))
