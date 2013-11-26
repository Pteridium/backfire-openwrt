/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2011 Florian Fainelli <florian@openwrt.org>
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <bcm63xx_cpu.h>

static struct resource trng_resources[] = {
	{
		.start		= -1, /* filled at runtime */
		.end		= -1, /* filled at runtime */
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device bcm63xx_trng_device = {
	.name		= "bcm63xx-trng",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(trng_resources),
	.resource	= trng_resources,
};

int __init bcm63xx_trng_register(void)
{
	if (!BCMCPU_IS_6368())
		return -ENODEV;

	trng_resources[0].start = bcm63xx_regset_address(RSET_TRNG);
	trng_resources[0].end = trng_resources[0].start;
	trng_resources[0].end += RSET_TRNG_SIZE - 1;

	return platform_device_register(&bcm63xx_trng_device);
}
arch_initcall(bcm63xx_trng_register);
