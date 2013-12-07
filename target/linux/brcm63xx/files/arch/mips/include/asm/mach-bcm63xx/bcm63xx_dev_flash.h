#ifndef __BCM63XX_FLASH_H
#define __BCM63XX_FLASH_H

#include <board_bcm963xx.h>

enum {
	BCM63XX_FLASH_TYPE_PARALLEL,
	BCM63XX_FLASH_TYPE_SERIAL,
	BCM63XX_FLASH_TYPE_NAND,
};

int __init bcm63xx_flash_register(int num_caldata, struct bcm63xx_caldata *caldata);

#endif /* __BCM63XX_FLASH_H */
