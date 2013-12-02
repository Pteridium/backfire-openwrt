/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 * Copyright (C) 2008 Florian Fainelli <florian@openwrt.org>
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/ssb/ssb.h>
#include <linux/gpio_buttons.h>
#include <linux/input.h>
#include <linux/spi/spi.h>
#include <asm/addrspace.h>
#include <bcm63xx_board.h>
#include <bcm63xx_cpu.h>
#include <bcm63xx_dev_uart.h>
#include <bcm63xx_regs.h>
#include <bcm63xx_io.h>
#include <bcm63xx_dev_pci.h>
#include <bcm63xx_dev_enet.h>
#include <bcm63xx_dev_dsp.h>
#include <bcm63xx_dev_flash.h>
#include <bcm63xx_dev_pcmcia.h>
#include <bcm63xx_dev_usb_ohci.h>
#include <bcm63xx_dev_usb_ehci.h>
#include <bcm63xx_dev_spi.h>
#include <board_bcm963xx.h>
#include <bcm_tag.h>

#define PFX	"board_bcm963xx: "

#define CFE_OFFSET_64K		0x10000
#define CFE_OFFSET_128K		0x20000

static struct bcm963xx_nvram nvram;
static unsigned int mac_addr_used;
static struct board_info board;

/*
 * known 6338 boards
 */
#ifdef CONFIG_BCM63XX_CPU_6338
static struct board_info __initdata board_96338GW = {
	.name				= "96338GW",
	.expected_cpu_id		= 0x6338,

	.has_uart0			= 1,
	.has_enet0			= 1,
	.enet0 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},

	.has_ohci0			= 1,

	.leds = {
		{
			.name		= "96338GW:green:adsl",
			.gpio		= 3,
			.active_low	= 1,
		},
		{
			.name		= "96338GW:green:ses",
			.gpio		= 5,
			.active_low	= 1,
		},
		{
			.name		= "96338GW:green:ppp-fail",
			.gpio		= 4,
			.active_low	= 1,
		},
		{
			.name		= "96338GW:green:power",
			.gpio		= 0,
			.active_low	= 1,
			.default_trigger = "default-on",
		},
		{
			.name		= "96338GW:green:stop",
			.gpio		= 1,
			.active_low	= 1,
		}
	},
};

static struct board_info __initdata board_96338W = {
	.name				= "96338W",
	.expected_cpu_id		= 0x6338,

	.has_uart0			= 1,
	.has_enet0			= 1,
	.enet0 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},

	.leds = {
		{
			.name		= "96338W:green:adsl",
			.gpio		= 3,
			.active_low	= 1,
		},
		{
			.name		= "96338W:green:ses",
			.gpio		= 5,
			.active_low	= 1,
		},
		{
			.name		= "96338W:green:ppp-fail",
			.gpio		= 4,
			.active_low	= 1,
		},
		{
			.name		= "96338W:green:power",
			.gpio		= 0,
			.active_low	= 1,
			.default_trigger = "default-on",
		},
		{
			.name		= "96338W:green:stop",
			.gpio		= 1,
			.active_low	= 1,
		},
	},
};
#endif

/*
 * known 6345 boards
 */
#ifdef CONFIG_BCM63XX_CPU_6345
static struct board_info __initdata board_96345GW2 = {
	.name				= "96345GW2",
	.expected_cpu_id		= 0x6345,

	.has_uart0			= 1,
};
#endif

/*
 * known 6348 boards
 */
#ifdef CONFIG_BCM63XX_CPU_6348
static struct board_info __initdata board_96348GW_11 = {
	.name				= "96348GW-11",
	.expected_cpu_id		= 0x6348,

	.has_uart0			= 1,
	.has_enet0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},

	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},

	.has_ohci0			= 1,
	.has_pccard			= 1,
	.has_ehci0			= 1,

	.leds = {
		{
			.name		= "96348GW-11:green:adsl-fail",
			.gpio		= 2,
			.active_low	= 1,
		},
		{
			.name		= "96348GW-11:green:ppp",
			.gpio		= 3,
			.active_low	= 1,
		},
		{
			.name		= "96348GW-11:green:ppp-fail",
			.gpio		= 4,
			.active_low	= 1,
		},
		{
			.name		= "96348GW-11:green:power",
			.gpio		= 0,
			.active_low	= 1,
			.default_trigger = "default-on",
		},
		{
			.name		= "96348GW-11:green:stop",
			.gpio		= 1,
			.active_low	= 1,
		},
	},

	.buttons = {
		{
			.desc		= "reset",
			.gpio		= 33,
			.active_low	= 1,
			.type		= EV_KEY,
			.code		= KEY_RESTART,
			.threshold	= 3,
		},
	},
};

static struct board_info __initdata board_CT5621 = {
	.name				= "CT-5621",
	.expected_cpu_id		= 0x6348,

	.has_uart0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},

	.has_ohci0			= 1,
	.has_pccard			= 1,
	.has_ehci0			= 1,

	.leds = {
		{
			.name		= "CT-5621:green:adsl-fail",
			.gpio		= 2,
			.active_low	= 1,
		},
		{
			.name		= "CT-5621:green:power",
			.gpio		= 0,
			.active_low	= 1,
			.default_trigger = "default-on",
		},
	},

	.buttons = {
		{
			.desc		= "reset",
			.gpio		= 33,
			.active_low	= 1,
			.type		= EV_KEY,
			.code		= KEY_RESTART,
			.threshold	= 3,
		},
	},
};

static struct board_info __initdata board_CT536PLUS = {
	.name				= "CT-536+",
	.expected_cpu_id		= 0x6348,

	.has_uart0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},

	.has_ohci0			= 1,
	.has_pccard			= 1,
	.has_ehci0			= 1,

	.leds = {
		{
			.name		= "CT-536+:green:adsl-fail",
			.gpio		= 2,
			.active_low	= 1,
		},
		{
			.name		= "CT-536+:green:power",
			.gpio		= 0,
			.active_low	= 1,
			.default_trigger = "default-on",
		},
	},

	.buttons = {
		{
			.desc		= "reset",
			.gpio		= 33,
			.active_low	= 1,
			.type		= EV_KEY,
			.code		= KEY_RESTART,
			.threshold	= 3,
		},
	},
};

static struct board_info __initdata board_CT5361 = {
	.name				= "CT-5361",
	.expected_cpu_id		= 0x6348,

	.has_uart0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},

	.has_ohci0			= 1,
	.has_pccard			= 1,
	.has_ehci0			= 1,

	.leds = {
		{
			.name		= "CT-5361:green:adsl-fail",
			.gpio		= 2,
			.active_low	= 1,
		},
		{
			.name		= "CT-5361:green:power",
			.gpio		= 0,
			.active_low	= 1,
			.default_trigger = "default-on",
		},
	},

	.buttons = {
		{
			.desc		= "reset",
			.gpio		= 33,
			.active_low	= 1,
			.type		= EV_KEY,
			.code		= KEY_RESTART,
			.threshold	= 3,
		},
	},
};
#endif

/*
 * known 6358 boards
 */
#ifdef CONFIG_BCM63XX_CPU_6358
static struct board_info __initdata board_DSL2650U = {
	.name				= "96358VW2",
	.expected_cpu_id		= 0x6358,

	.has_uart0			= 1,
	.has_enet0			= 1,
	.has_enet1			= 1,
	.has_pci			= 1,

	.enet0 = {
		.has_phy		= 1,
		.use_internal_phy	= 1,
	},

	.enet1 = {
		.force_speed_100	= 1,
		.force_duplex_full	= 1,
	},

	.has_ohci0			= 1,
	.has_pccard			= 1,
	.has_ehci0			= 1,

	.leds = {
		{
			.name		= "DSL-2650U:green:adsl",
			.gpio		= 22,
			.active_low	= 1,
		},
		{
			.name		= "DSL-2650U:green:ppp-fail",
			.gpio		= 23,
		},
		{
			.name		= "DSL-2650U:green:power",
			.gpio		= 5,
			.active_low	= 1,
			.default_trigger = "default-on",
		},
		{
			.name		= "DSL-2650U:green:stop",
			.gpio		= 4,
			.active_low	= 1,
		},
	},
};

static struct board_info __initdata board_HG520v = {
	.name                           = "HW6358GW_B",
	.expected_cpu_id                = 0x6358,

	.has_uart0			= 1,
	.has_enet1			= 1,
	.has_pci                        = 1,

	.enet1 = {
		.force_speed_100        = 1,
		.force_duplex_full      = 1,
	},

	.has_ohci0			= 1,
	.has_ehci0			= 1,

	.leds = {
		{
			.name		= "HG520v:green:net",
			.gpio		= 32,
			.active_low	= 1,
		},
	},

	.buttons = {
		{
			.desc		= "reset",
			.gpio		= 37,
			.active_low	= 1,
			.type		= EV_KEY,
			.code		= KEY_RESTART,
			.threshold	= 3,
		},
	},
};

static struct board_info __initdata board_HG553 = {
	.name                           = "HW553",
	.expected_cpu_id                = 0x6358,

	.has_uart0			= 1,
	.has_enet1                      = 1,
	.has_pci                        = 1,

	.enet1 = {
		.has_phy		= 1,
		.phy_id			= 0,
		.force_speed_100        = 1,
		.force_duplex_full      = 1,
	},

	.has_ohci0 = 1,
	.has_ehci0 = 1,

	.leds = {
		{
			.name		= "HG553:blue:power",
			.gpio		= 4,
			.active_low	= 1,
			.default_trigger = "default-on",
		},
		{
			.name		= "HG553:red:power",
			.gpio		= 5,
			.active_low	= 1,
		},
		{
			.name		= "HG553:red:internetkey",
			.gpio		= 12,
			.active_low	= 1,
		},
		{
			.name		= "HG553:blue:internetkey",
			.gpio		= 13,
			.active_low	= 1,
		},
		{
			.name		= "HG553:red:adsl",
			.gpio		= 22,
			.active_low	= 1,
		},
		{
			.name		= "HG553:blue:adsl",
			.gpio		= 23,
			.active_low	= 1,
		},
		{
			.name		= "HG553:red:wifi",
			.gpio		= 25,
			.active_low	= 1,
		},
		{
			.name		= "HG553:red:lan",
			.gpio		= 34,
			.active_low	= 1,
		},
		{
			.name		= "HG553:blue:lan",
			.gpio		= 35,
			.active_low	= 1,
		},
	},

	.buttons = {
		{
			.desc		= "wps",
			.gpio		= 9,
			.active_low	= 1,
			.type		= EV_KEY,
			.code		= KEY_WPS_BUTTON,
			.threshold	= 3,
		},
		{
			.desc		= "reset",
			.gpio		= 37,
			.active_low	= 1,
			.type		= EV_KEY,
			.code		= KEY_RESTART,
			.threshold	= 3,
		},
	},
};
#endif

/*
 * known 6368 boards
 */
#ifdef CONFIG_BCM63XX_CPU_6368
static struct board_info __initdata board_VR3025u = {
	.name				= "96368M-1541N",
	.expected_cpu_id		= 0x6368,

	.has_uart0			= 1,
	.has_pci			= 1,
	.has_enetsw			= 1,

	.enetsw = {
		.used_ports = {
			[0] = {
				.used	= 1,
				.phy_id	= 1,
				.name	= "port1",
			},
			[1] = {
				.used	= 1,
				.phy_id	= 2,
				.name	= "port2",
			},
			[2] = {
				.used	= 1,
				.phy_id	= 3,
				.name	= "port3",
			},
			[3] = {
				.used	= 1,
				.phy_id	= 4,
				.name	= "port4",
			},
		},
	},

	.leds = {
		{
			.name		= "VR-3025u:green:dsl",
			.gpio		= 2,
			.active_low	= 1,
		},
		{
			.name		= "VR-3025u:green:inet",
			.gpio		= 5,
		},
		{
			.name		= "VR-3025u:green:power",
			.gpio		= 22,
			.default_trigger = "default-on",
		},
		{
			.name		= "VR-3025u:red:power",
			.gpio		= 24,
		},
		{
			.name		= "VR-3025u:red:inet",
			.gpio		= 31,
		},
	},

	.buttons = {
		{
			.desc		= "reset",
			.gpio		= 34,
			.active_low	= 1,
			.type		= EV_KEY,
			.code		= KEY_RESTART,
			.threshold	= 3,
		},
	},
};

static struct board_info __initdata board_VR3025un = {
	.name				= "96368M-1341N",
	.expected_cpu_id		= 0x6368,

	.has_uart0			= 1,
	.has_pci			= 1,
	.has_enetsw			= 1,

	.enetsw = {
		.used_ports = {
			[0] = {
				.used	= 1,
				.phy_id	= 1,
				.name	= "port1",
			},
			[1] = {
				.used	= 1,
				.phy_id	= 2,
				.name	= "port2",
			},
			[2] = {
				.used	= 1,
				.phy_id	= 3,
				.name	= "port3",
			},
			[3] = {
				.used	= 1,
				.phy_id	= 4,
				.name	= "port4",
			},
		},
	},

	.leds = {
		{
			.name		= "VR-3025un:green:dsl",
			.gpio		= 2,
			.active_low	= 1,
		},
		{
			.name		= "VR-3025un:green:inet",
			.gpio		= 5,
		},
		{
			.name		= "VR-3025un:green:power",
			.gpio		= 22,
			.default_trigger = "default-on",
		},
		{
			.name		= "VR-3025un:red:power",
			.gpio		= 24,
		},
		{
			.name		= "VR-3025un:red:inet",
			.gpio		= 31,
		},
	},

	.buttons = {
		{
			.desc		= "reset",
			.gpio		= 34,
			.active_low	= 1,
			.type		= EV_KEY,
			.code		= KEY_RESTART,
			.threshold	= 3,
		},
	},
};
#endif

/*
 * all boards
 */
static const struct board_info __initdata *bcm963xx_boards[] = {
#ifdef CONFIG_BCM63XX_CPU_6338
	&board_96338GW,
	&board_96338W,
#endif
#ifdef CONFIG_BCM63XX_CPU_6345
	&board_96345GW2,
#endif
#ifdef CONFIG_BCM63XX_CPU_6348
	&board_96348GW_11,
	&board_CT5621,
	&board_CT536PLUS,
	&board_CT5361,
#endif
#ifdef CONFIG_BCM63XX_CPU_6358
	&board_DSL2650U,
	&board_HG520v,
	&board_HG553,
#endif
#ifdef CONFIG_BCM63XX_CPU_6368
	&board_VR3025u,
	&board_VR3025un,
#endif
};

/*
 * Register a sane SPROMv2 to make the on-board
 * bcm4318 WLAN work
 */
#ifdef CONFIG_SSB_PCIHOST
static struct ssb_sprom bcm63xx_sprom = {
	.revision		= 0x02,
	.board_rev		= 0x17,
	.country_code		= 0x0,
	.ant_available_bg 	= 0x3,
	.pa0b0			= 0x15ae,
	.pa0b1			= 0xfa85,
	.pa0b2			= 0xfe8d,
	.pa1b0			= 0xffff,
	.pa1b1			= 0xffff,
	.pa1b2			= 0xffff,
	.gpio0			= 0xff,
	.gpio1			= 0xff,
	.gpio2			= 0xff,
	.gpio3			= 0xff,
	.maxpwr_bg		= 0x004c,
	.itssi_bg		= 0x00,
	.boardflags_lo		= 0x2848,
	.boardflags_hi		= 0x0000,
};
#endif

/*
 * return board name for /proc/cpuinfo
 */
const char *board_get_name(void)
{
	return board.name;
}

static void __init boardid_fixup(u8 *boot_addr)
{
	struct bcm_tag *tag = (struct bcm_tag *)(boot_addr + CFE_OFFSET_64K);

	/* check if bcm_tag is at 64k offset */
	if (strncmp(nvram.name, tag->board_id, BOARDID_LEN) != 0) {
		/* else try 128k */
		tag = (struct bcm_tag *)(boot_addr + CFE_OFFSET_128K);
		if (strncmp(nvram.name, tag->board_id, BOARDID_LEN) != 0) {
			/* No tag found */
			printk(KERN_DEBUG "No bcm_tag found!\n");
			return;
		}
	}
	/* check if we should override the boardid */
	if (tag->information1[0] != '+')
		return;

	strncpy(nvram.name, &tag->information1[1], BOARDID_LEN);

	printk(KERN_INFO "Overriding boardid with '%s'\n", nvram.name);
}

/*
 * early init callback, read nvram data from flash and checksum it
 */
void __init board_prom_init(void)
{
	unsigned int check_len, i;
	u8 *boot_addr, *cfe, *p;
	char cfe_version[32];
	u32 val;

	/* read base address of boot chip select (0)
	 * 6328 does not have MPI but boots from a fixed address */
	if (BCMCPU_IS_6328())
		val = 0x18000000;
	else {
		val = bcm_mpi_readl(MPI_CSBASE_REG(0));
		val &= MPI_CSBASE_BASE_MASK;
	}
	boot_addr = (u8 *)KSEG1ADDR(val);

	/* dump cfe version */
	cfe = boot_addr + BCM963XX_CFE_VERSION_OFFSET;
	if (strstarts(cfe, "cfe-")) {
		if(cfe[4] == 'v') {
			if(cfe[5] == 'd')
				snprintf(cfe_version, 11, "%s", (char *) &cfe[5]);
			else if (cfe[10] > 0)
				snprintf(cfe_version, sizeof(cfe_version), "%u.%u.%u-%u.%u-%u",
					 cfe[5], cfe[6], cfe[7], cfe[8], cfe[9], cfe[10]);
			else
				snprintf(cfe_version, sizeof(cfe_version), "%u.%u.%u-%u.%u",
					 cfe[5], cfe[6], cfe[7], cfe[8], cfe[9]);
		} else {
			snprintf(cfe_version, 12, "%s", (char *) &cfe[4]);
		}
	} else
		strcpy(cfe_version, "unknown");
	printk(KERN_INFO PFX "CFE version: %s\n", cfe_version);

	/* extract nvram data */
	memcpy(&nvram, boot_addr + BCM963XX_NVRAM_OFFSET, sizeof(nvram));

	/* check checksum before using data */
	if (nvram.version <= 4)
		check_len = offsetof(struct bcm963xx_nvram, checksum_old);
	else
		check_len = sizeof(nvram);
	val = 0;
	p = (u8 *)&nvram;
	while (check_len--)
		val += *p;
	if (val) {
		printk(KERN_ERR PFX "invalid nvram checksum\n");
		return;
	}

	if (strcmp(cfe_version, "unknown") != 0) {
		/* cfe present */
		boardid_fixup(boot_addr);
	}

	/* find board by name */
	for (i = 0; i < ARRAY_SIZE(bcm963xx_boards); i++) {
		if (strncmp(nvram.name, bcm963xx_boards[i]->name,
			    sizeof(nvram.name)))
			continue;
		/* copy, board desc array is marked initdata */
		memcpy(&board, bcm963xx_boards[i], sizeof(board));
		break;
	}

	/* bail out if board is not found, will complain later */
	if (!board.name[0]) {
		char name[17];
		memcpy(name, nvram.name, 16);
		name[16] = 0;
		printk(KERN_ERR PFX "unknown bcm963xx board: %s\n",
		       name);
		return;
	}

	/* setup pin multiplexing depending on board enabled device,
	 * this has to be done this early since PCI init is done
	 * inside arch_initcall */
	val = 0;

#ifdef CONFIG_PCI
	if (board.has_pci) {
		if (BCMCPU_IS_6348())
			val |= GPIO_MODE_6348_G2_PCI;

		if (BCMCPU_IS_6368())
			val |= GPIO_MODE_6368_PCI_REQ1 |
				GPIO_MODE_6368_PCI_GNT1 |
				GPIO_MODE_6368_PCI_INTB |
				GPIO_MODE_6368_PCI_REQ0 |
				GPIO_MODE_6368_PCI_GNT0;
	}
#endif

	if (board.has_pccard) {
		if (BCMCPU_IS_6348())
			val |= GPIO_MODE_6348_G1_MII_PCCARD;

		if (BCMCPU_IS_6368())
			val |= GPIO_MODE_6368_PCMCIA_CD1 |
				GPIO_MODE_6368_PCMCIA_CD2 |
				GPIO_MODE_6368_PCMCIA_VS1 |
				GPIO_MODE_6368_PCMCIA_VS2;
	}

	if (board.has_enet0 && !board.enet0.use_internal_phy) {
		if (BCMCPU_IS_6348())
			val |= GPIO_MODE_6348_G3_EXT_MII |
				GPIO_MODE_6348_G0_EXT_MII;
	}

	if (board.has_enet1 && !board.enet1.use_internal_phy) {
		if (BCMCPU_IS_6348())
			val |= GPIO_MODE_6348_G3_EXT_MII |
				GPIO_MODE_6348_G0_EXT_MII;
		else if (BCMCPU_IS_6358())
			val |= GPIO_MODE_6358_ENET1_MII_CLK_INV;
	}

	bcm_gpio_writel(val, GPIO_MODE_REG);
}

/*
 * second stage init callback, good time to panic if we couldn't
 * identify on which board we're running since early printk is working
 */
void __init board_setup(void)
{
	if (!board.name[0])
		panic("unable to detect bcm963xx board");
	printk(KERN_INFO PFX "board name: %s\n", board.name);

	/* make sure we're running on expected cpu */
	if (bcm63xx_get_cpu_id() != board.expected_cpu_id)
		panic("unexpected CPU for bcm963xx board");
}

/*
 * register & return a new board mac address
 */
static int board_get_mac_address(u8 *mac)
{
	u8 *p;
	int count;

	if (mac_addr_used >= nvram.mac_addr_count) {
		printk(KERN_ERR PFX "not enough mac address\n");
		return -ENODEV;
	}

	memcpy(mac, nvram.mac_addr_base, ETH_ALEN);
	p = mac + ETH_ALEN - 1;
	count = mac_addr_used;

	while (count--) {
		do {
			(*p)++;
			if (*p != 0)
				break;
			p--;
		} while (p != mac);
	}

	if (p == mac) {
		printk(KERN_ERR PFX "unable to fetch mac address\n");
		return -ENODEV;
	}

	mac_addr_used++;
	return 0;
}

static struct gpio_led_platform_data bcm63xx_led_data;

static struct platform_device bcm63xx_gpio_leds = {
	.name			= "leds-gpio",
	.id			= 0,
	.dev.platform_data	= &bcm63xx_led_data,
};

static struct gpio_buttons_platform_data bcm63xx_gpio_buttons_data = {
	.poll_interval  = 20,
};

static struct platform_device bcm63xx_gpio_buttons_device = {
	.name		= "gpio-buttons",
	.id		= 0,
	.dev.platform_data = &bcm63xx_gpio_buttons_data,
};

/*
 * third stage init callback, register all board devices.
 */
int __init board_register_devices(void)
{
	int led_count = 0;
	int button_count = 0;

	if (board.has_uart0)
		bcm63xx_uart_register(0);

	if (board.has_uart1)
		bcm63xx_uart_register(1);

	if (board.has_pccard)
		bcm63xx_pcmcia_register();

	if (board.has_enet0 &&
	    !board_get_mac_address(board.enet0.mac_addr))
		bcm63xx_enet_register(0, &board.enet0);

	if (board.has_enet1 &&
	    !board_get_mac_address(board.enet1.mac_addr))
		bcm63xx_enet_register(1, &board.enet1);

	if (board.has_enetsw &&
	    !board_get_mac_address(board.enetsw.mac_addr))
		bcm63xx_enetsw_register(&board.enetsw);

	if (board.has_ehci0)
		bcm63xx_ehci_register();

	if (board.has_ohci0)
		bcm63xx_ohci_register();

	if (board.has_dsp)
		bcm63xx_dsp_register(&board.dsp);

	/* Generate MAC address for WLAN and
	 * register our SPROM */
#ifdef CONFIG_SSB_PCIHOST
	if (!board_get_mac_address(bcm63xx_sprom.il0mac)) {
		memcpy(bcm63xx_sprom.et0mac, bcm63xx_sprom.il0mac, ETH_ALEN);
		memcpy(bcm63xx_sprom.et1mac, bcm63xx_sprom.il0mac, ETH_ALEN);
		if (ssb_arch_set_fallback_sprom(&bcm63xx_sprom) < 0)
			printk(KERN_ERR "failed to register fallback SPROM\n");
	}
#endif

	bcm63xx_spi_register();

	if (board.num_devs)
		platform_add_devices(board.devs, board.num_devs);

	if (board.num_spis)
		spi_register_board_info(board.spis, board.num_spis);

	bcm63xx_flash_register();

	/* count number of LEDs defined by this device */
	while (led_count < ARRAY_SIZE(board.leds) && board.leds[led_count].name)
		led_count++;

	if (led_count) {
		bcm63xx_led_data.num_leds = led_count;
		bcm63xx_led_data.leds = board.leds;

		platform_device_register(&bcm63xx_gpio_leds);
	}

	/* count number of BUTTONs defined by this device */
	while (button_count < ARRAY_SIZE(board.buttons) && board.buttons[button_count].desc)
		button_count++;

	if (button_count) {
		bcm63xx_gpio_buttons_data.nbuttons = button_count;
		bcm63xx_gpio_buttons_data.buttons = board.buttons;

		platform_device_register(&bcm63xx_gpio_buttons_device);
	}

#ifdef CONFIG_PCI
	if (board.has_pci)
		bcm63xx_pci_register();
#endif

	return 0;
}
