#ifndef BOARD_BCM963XX_H_
#define BOARD_BCM963XX_H_

#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/gpio_buttons.h>
#include <linux/leds.h>
#include <bcm63xx_dev_enet.h>
#include <bcm63xx_dev_dsp.h>

/*
 * flash mapping
 */
#define BCM963XX_CFE_VERSION_OFFSET	0x570
#define BCM963XX_NVRAM_OFFSET		0x580

struct bcm63xx_caldata {
	unsigned int	vendor;
	unsigned int	slot;
	u32		caldata_offset;
	/* Atheros */
	unsigned int	endian_check:1;
	int		led_pin;
	/* Ralink */
	char*		eeprom;
};

/*
 * board definition
 */
struct board_info {
	u8		name[16];
	unsigned int	expected_cpu_id;

	/* enabled feature/device */
	unsigned int	has_enet0:1;
	unsigned int	has_enet1:1;
	unsigned int	has_enetsw:1;
	unsigned int	has_pci:1;
	unsigned int	has_pccard:1;
	unsigned int	has_ohci0:1;
	unsigned int	has_ehci0:1;
	unsigned int	has_dsp:1;
	unsigned int	has_uart0:1;
	unsigned int	has_uart1:1;
	unsigned int	has_caldata:2;

	/* wifi calibration data config */
	struct bcm63xx_caldata caldata[2];

	/* ethernet config */
	struct bcm63xx_enet_platform_data enet0;
	struct bcm63xx_enet_platform_data enet1;
	struct bcm63xx_enetsw_platform_data enetsw;

	/* DSP config */
	struct bcm63xx_dsp_platform_data dsp;

	/* GPIO LEDs */
	struct gpio_led leds[14];

	/* Reset button */
	struct gpio_button buttons[4];

	/* Additional platform devices */
	struct platform_device **devs;
	unsigned int	num_devs;

	/* Additional platform devices */
	struct spi_board_info *spis;
	unsigned int	num_spis;
};

#endif /* ! BOARD_BCM963XX_H_ */
