#ifndef BCM63XX_DEV_HSSPI_H
#define BCM63XX_DEV_HSSPI_H

#include <linux/types.h>
#include <bcm63xx_io.h>
#include <bcm63xx_regs.h>

int __init bcm63xx_hsspi_register(void);

struct bcm63xx_hsspi_pdata {
	int		bus_num;
	u32		speed_hz;
};

#define bcm_hsspi_readl(o)	bcm_rset_readl(RSET_HSSPI, (o))
#define bcm_hsspi_writel(v, o)	bcm_rset_writel(RSET_HSSPI, (v), (o))

#define HSSPI_PLL_HZ_6328	133333333

#define HSSPI_OP_CODE_SHIFT	13
#define HSSPI_OP_SLEEP		(0 << HSSPI_OP_CODE_SHIFT)
#define HSSPI_OP_READ_WRITE	(1 << HSSPI_OP_CODE_SHIFT)
#define HSSPI_OP_WRITE		(2 << HSSPI_OP_CODE_SHIFT)
#define HSSPI_OP_READ		(3 << HSSPI_OP_CODE_SHIFT)

#define HS_SPI_CLOCK_DEF    40000000
#define HS_SPI_BUFFER_LEN   512

#endif /* BCM63XX_DEV_HSSPI_H */
