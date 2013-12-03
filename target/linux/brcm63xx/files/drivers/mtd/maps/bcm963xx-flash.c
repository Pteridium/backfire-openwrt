/*
 * Copyright (C) 2006-2008  Florian Fainelli <florian@openwrt.org>
 * 			    Mike Albon <malbon@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <linux/crc32.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <linux/mtd/map.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/magic.h>
#include <linux/jffs2.h>

#include <asm/io.h>

#include <asm/mach-bcm63xx/bcm63xx_nvram.h>
#include <asm/mach-bcm63xx/bcm_tag.h>
#include <asm/mach-bcm63xx/bcm63xx_board.h>

#define BCM63XX_CFE_BLOCK_SIZE	SZ_64K		/* always at least 64KiB */

#define BCM63XX_EXTENDED_SIZE	0xBFC00000	/* Extended flash address */

#define BUSWIDTH 2                     /* Buswidth */

#define PFX KBUILD_MODNAME ": "

struct squashfs_super_block {
	__le32 s_magic;
	__le32 pad0[9];
	__le64 bytes_used;
};

extern int parse_redboot_partitions(struct mtd_info *master, struct mtd_partition **pparts, unsigned long fis_origin);
static struct mtd_partition *parsed_parts;

static struct mtd_info *bcm963xx_mtd_info;

static struct map_info bcm963xx_map = {
       .name		= "bcm963xx",
       .bankwidth	= BUSWIDTH,
};

static int parse_cfe_partitions( struct mtd_info *master, struct mtd_partition **pparts)
{
	/* CFE, NVRAM and global Linux are always present */
	int nrparts = 3, curpart = 0;
	struct bcm_tag *buf;
	struct mtd_partition *parts;
	int ret;
	size_t retlen;
	unsigned int rootfsaddr, kerneladdr, spareaddr, nvramaddr;
	unsigned int rootfslen, kernellen, sparelen, totallen;
	unsigned int cfelen, nvramlen;
	unsigned int cfe_erasesize;
	int i;
	u32 computed_crc;
	bool rootfs_first = false;

	if (!bcm63xx_is_cfe_present())
		return -EINVAL;

	cfe_erasesize = max_t(uint32_t, master->erasesize,
			      BCM63XX_CFE_BLOCK_SIZE);

	cfelen = cfe_erasesize;

	nvramlen = bcm63xx_nvram_get_psi_size() * SZ_1K;
	nvramlen = roundup(nvramlen, cfe_erasesize); 
	nvramaddr = master->size - nvramlen;

	/* Allocate memory for buffer */
	buf = vmalloc(sizeof(struct bcm_tag));
	if (!buf)
		return -ENOMEM;

	/* Get the tag */
	ret = master->read(master, master->erasesize, sizeof(struct bcm_tag),
			&retlen, (void *)buf);

	if (retlen != sizeof(struct bcm_tag)) {
		vfree(buf);
		return -EIO;
	}

	computed_crc = crc32_le(IMAGETAG_CRC_START, (u8 *)buf,
				offsetof(struct bcm_tag, header_crc));
	if (computed_crc == buf->header_crc) {
		char *boardid = &(buf->board_id[0]);
		char *tagversion = &(buf->tag_version[0]);

		sscanf(buf->flash_image_start, "%u", &rootfsaddr);
		sscanf(buf->kernel_address, "%u", &kerneladdr);
		sscanf(buf->kernel_length, "%u", &kernellen);
		sscanf(buf->total_length, "%u", &totallen);

		printk(KERN_INFO "CFE boot tag found with version %s and board type %s\n",
			tagversion, boardid);

		kerneladdr = kerneladdr - BCM63XX_EXTENDED_SIZE;
		rootfsaddr = rootfsaddr - BCM63XX_EXTENDED_SIZE;
		spareaddr = roundup(totallen, master->erasesize) + cfelen;

		if (rootfsaddr < kerneladdr) {
			/* default Broadcom layout */
			rootfslen = kerneladdr - rootfsaddr;
			rootfs_first = true;
		} else {
			/* OpenWrt layout */
			rootfsaddr = kerneladdr + kernellen;
			rootfslen = buf->real_rootfs_length;
			spareaddr = rootfsaddr + rootfslen;
		}
	} else {
		printk(KERN_WARNING "CFE boot tag CRC invalid (expected %08x, actual %08x)\n",
			buf->header_crc, computed_crc);
		kernellen = 0;
		rootfslen = 0;
		rootfsaddr = 0;
		spareaddr = cfelen;
	}
	sparelen = nvramaddr - spareaddr;

	/* Determine number of partitions */
	if (rootfslen > 0)
		nrparts++;

	if (kernellen > 0)
		nrparts++;

	/* Ask kernel for more memory */
	parts = kzalloc(sizeof(*parts) * nrparts + 10 * nrparts, GFP_KERNEL);
	if (!parts) {
		vfree(buf);
		return -ENOMEM;
	}

	/* Start building partition list */
	parts[curpart].name = "CFE";
	parts[curpart].offset = 0;
	parts[curpart].size = cfelen;
	curpart++;

	if (kernellen > 0) {
		int kernelpart = curpart;

		if (rootfslen > 0 && rootfs_first)
			kernelpart++;
		parts[kernelpart].name = "kernel";
		parts[kernelpart].offset = kerneladdr;
		parts[kernelpart].size = kernellen;
		curpart++;
	}

	if (rootfslen > 0) {
		int rootfspart = curpart;

		if (kernellen > 0 && rootfs_first)
			rootfspart--;
		parts[rootfspart].name = "rootfs";
		parts[rootfspart].offset = rootfsaddr;
		parts[rootfspart].size = rootfslen;
		if (sparelen > 0  && !rootfs_first)
			parts[rootfspart].size += sparelen;
		curpart++;
	}

	parts[curpart].name = "nvram";
	parts[curpart].offset = nvramaddr;
	parts[curpart].size = nvramlen;
	curpart++;

	/* Global partition "linux" to make easy firmware upgrade */
	parts[curpart].name = "linux";
	parts[curpart].offset = cfelen;
	parts[curpart].size = nvramaddr - cfelen;

	for (i = 0; i < nrparts; i++)
		printk(KERN_INFO "Partition %d is %s offset %llx and length %llx\n", i,
			parts[i].name, parts[i].offset,	parts[i].size);

	printk(KERN_INFO "Spare partition is offset %x and length %x\n",	spareaddr,
		sparelen);

	*pparts = parts;
	vfree(buf);

	return nrparts;
};

static int bcm963xx_probe(struct platform_device *pdev)
{
	int err = 0;
	int parsed_nr_parts = 0;
	char *part_type;
	struct resource *r;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	bcm963xx_map.phys = r->start;
	bcm963xx_map.size = (r->end - r->start) + 1;
	bcm963xx_map.virt = ioremap(r->start, r->end - r->start + 1);

	if (!bcm963xx_map.virt) {
		printk(KERN_ERR PFX "Failed to ioremap\n");
		return -EIO;
	}
	printk(KERN_INFO PFX "0x%08lx at 0x%08x\n", bcm963xx_map.size, bcm963xx_map.phys);

	simple_map_init(&bcm963xx_map);

	bcm963xx_mtd_info = do_map_probe("cfi_probe", &bcm963xx_map);
	if (!bcm963xx_mtd_info) {
		printk(KERN_ERR PFX "Failed to probe using CFI\n");
		err = -EIO;
		goto err_probe;
	}

	bcm963xx_mtd_info->owner = THIS_MODULE;

	/* This is mutually exclusive */
	if (bcm63xx_is_cfe_present()) {
		printk(KERN_INFO PFX "CFE bootloader detected\n");
		if (parsed_nr_parts == 0) {
			int ret = parse_cfe_partitions(bcm963xx_mtd_info, &parsed_parts);
			if (ret > 0) {
				part_type = "CFE";
				parsed_nr_parts = ret;
			}
		}
	} else {
		printk(KERN_INFO PFX "assuming RedBoot bootloader\n");
		if (bcm963xx_mtd_info->size > 0x00400000) {
			printk(KERN_INFO PFX "Support for extended flash memory size : 0x%llx ; ONLY 64MBIT SUPPORT\n", bcm963xx_mtd_info->size);
			bcm963xx_map.virt = (u32)(BCM63XX_EXTENDED_SIZE);
		}

#ifdef CONFIG_MTD_REDBOOT_PARTS
		if (parsed_nr_parts == 0) {
			int ret = parse_redboot_partitions(bcm963xx_mtd_info, &parsed_parts, 0);
			if (ret > 0) {
				part_type = "RedBoot";
				parsed_nr_parts = ret;
			}
		}
#endif
	}

	return add_mtd_partitions(bcm963xx_mtd_info, parsed_parts, parsed_nr_parts);

err_probe:
	iounmap(bcm963xx_map.virt);
	return err;
}

static int bcm963xx_remove(struct platform_device *pdev)
{
	if (bcm963xx_mtd_info) {
		del_mtd_partitions(bcm963xx_mtd_info);
		map_destroy(bcm963xx_mtd_info);
	}

	if (bcm963xx_map.virt) {
		iounmap(bcm963xx_map.virt);
		bcm963xx_map.virt = 0;
	}

	return 0;
}

static struct platform_driver bcm63xx_mtd_dev = {
	.probe	= bcm963xx_probe,
	.remove = bcm963xx_remove,
	.driver = {
		.name	= "bcm963xx-flash",
		.owner	= THIS_MODULE,
	},
};

static int __init bcm963xx_mtd_init(void)
{
	return platform_driver_register(&bcm63xx_mtd_dev);
}

static void __exit bcm963xx_mtd_exit(void)
{
	platform_driver_unregister(&bcm63xx_mtd_dev);
}

module_init(bcm963xx_mtd_init);
module_exit(bcm963xx_mtd_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Broadcom BCM63xx MTD partition parser/mapping for CFE and RedBoot");
MODULE_AUTHOR("Florian Fainelli <florian@openwrt.org>");
MODULE_AUTHOR("Mike Albon <malbon@openwrt.org>");
