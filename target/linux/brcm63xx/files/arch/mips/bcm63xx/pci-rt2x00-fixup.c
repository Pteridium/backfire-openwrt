/*
 *  Broadcom BCM63XX RT2x00 EEPROM fixup helper.
 *
 *  Copyright (C) 2012 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 *  Based on
 *
 *  Broadcom BCM63XX Ath9k EEPROM fixup helper.
 *
 *  Copyright (C) 2012 Jonas Gorski <jonas.gorski@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/if_ether.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/rt2x00_platform.h>

#include <bcm63xx_nvram.h>
#include <pci_rt2x00_fixup.h>

struct rt2x00_fixup {
	unsigned slot;
	u8 mac[ETH_ALEN];
	struct rt2x00_platform_data pdata;
};

static int rt2x00_num_fixups;
static struct rt2x00_fixup rt2x00_fixups[2] = {
	{
		.slot = 255,
	},
	{
		.slot = 255,
	},
};

static void rt2x00_pci_fixup(struct pci_dev *dev)
{
	unsigned i;
	struct rt2x00_platform_data *pdata = NULL;

	for (i = 0; i < rt2x00_num_fixups; i++) {
		if (rt2x00_fixups[i].slot != PCI_SLOT(dev->devfn))
			continue;

		pdata = &rt2x00_fixups[i].pdata;
		break;
	}

	dev->dev.platform_data = pdata;
}
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_RALINK, PCI_ANY_ID, rt2x00_pci_fixup);

void __init pci_enable_rt2x00_fixup(unsigned slot, char* eeprom)
{
	if (rt2x00_num_fixups >= ARRAY_SIZE(rt2x00_fixups))
		return;

	rt2x00_fixups[rt2x00_num_fixups].slot = slot;
	rt2x00_fixups[rt2x00_num_fixups].pdata.eeprom_file_name = kstrdup(eeprom, GFP_KERNEL);

	if (bcm63xx_nvram_get_mac_address(rt2x00_fixups[rt2x00_num_fixups].mac))
		return;

	rt2x00_fixups[rt2x00_num_fixups].pdata.mac_address = rt2x00_fixups[rt2x00_num_fixups].mac;
	rt2x00_num_fixups++;
}

