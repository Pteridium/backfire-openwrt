#ifndef BCM63XX_DEV_SPI_H
#define BCM63XX_DEV_SPI_H

#include <linux/types.h>
#include <bcm63xx_io.h>
#include <bcm63xx_regs.h>

int __init bcm63xx_spi_register(void);

struct bcm63xx_spi_pdata {
	unsigned int	fifo_size;
	int		bus_num;
	int		num_chipselect;
	u32		speed_hz;
};

enum bcm63xx_regs_spi {
        SPI_CMD,
        SPI_INT_STATUS,
        SPI_INT_MASK_ST,
        SPI_INT_MASK,
        SPI_ST,
        SPI_CLK_CFG,
        SPI_FILL_BYTE,
        SPI_MSG_TAIL,
        SPI_RX_TAIL,
        SPI_MSG_CTL,
        SPI_MSG_DATA,
        SPI_RX_DATA,
};

#define __GEN_SPI_RSET_BASE(__cpu, __rset)				\
	case SPI_## __rset:						\
		return SPI_BCM_## __cpu ##_SPI_## __rset;

#define __GEN_SPI_RSET(__cpu)						\
	switch (reg) {							\
	__GEN_SPI_RSET_BASE(__cpu, CMD)					\
	__GEN_SPI_RSET_BASE(__cpu, INT_STATUS)				\
	__GEN_SPI_RSET_BASE(__cpu, INT_MASK_ST)				\
	__GEN_SPI_RSET_BASE(__cpu, INT_MASK)				\
	__GEN_SPI_RSET_BASE(__cpu, ST)					\
	__GEN_SPI_RSET_BASE(__cpu, CLK_CFG)				\
	__GEN_SPI_RSET_BASE(__cpu, FILL_BYTE)				\
	__GEN_SPI_RSET_BASE(__cpu, MSG_TAIL)				\
	__GEN_SPI_RSET_BASE(__cpu, RX_TAIL)				\
	__GEN_SPI_RSET_BASE(__cpu, MSG_CTL)				\
	__GEN_SPI_RSET_BASE(__cpu, MSG_DATA)				\
	__GEN_SPI_RSET_BASE(__cpu, RX_DATA)				\
	}

#define __GEN_SPI_REGS_TABLE(__cpu)					\
	[SPI_CMD]		= SPI_BCM_## __cpu ##_SPI_CMD,		\
	[SPI_INT_STATUS]	= SPI_BCM_## __cpu ##_SPI_INT_STATUS,	\
	[SPI_INT_MASK_ST]	= SPI_BCM_## __cpu ##_SPI_INT_MASK_ST,	\
	[SPI_INT_MASK]		= SPI_BCM_## __cpu ##_SPI_INT_MASK,	\
	[SPI_ST]		= SPI_BCM_## __cpu ##_SPI_ST,		\
	[SPI_CLK_CFG]		= SPI_BCM_## __cpu ##_SPI_CLK_CFG,	\
	[SPI_FILL_BYTE]		= SPI_BCM_## __cpu ##_SPI_FILL_BYTE,	\
	[SPI_MSG_TAIL]		= SPI_BCM_## __cpu ##_SPI_MSG_TAIL,	\
	[SPI_RX_TAIL]		= SPI_BCM_## __cpu ##_SPI_RX_TAIL,	\
	[SPI_MSG_CTL]		= SPI_BCM_## __cpu ##_SPI_MSG_CTL,	\
	[SPI_MSG_DATA]		= SPI_BCM_## __cpu ##_SPI_MSG_DATA,	\
	[SPI_RX_DATA]		= SPI_BCM_## __cpu ##_SPI_RX_DATA,

static inline unsigned long bcm63xx_spireg(enum bcm63xx_regs_spi reg)
{
#ifdef BCMCPU_RUNTIME_DETECT
	extern const unsigned long *bcm63xx_regs_spi;
        return bcm63xx_regs_spi[reg];
#else
#ifdef CONFIG_BCM63XX_CPU_6338
	__GEN_SPI_RSET(6338)
#endif
#ifdef CONFIG_BCM63XX_CPU_6348
	__GEN_SPI_RSET(6348)
#endif
#ifdef CONFIG_BCM63XX_CPU_6358
	__GEN_SPI_RSET(6358)
#endif
#endif
	return 0;
}

#endif /* BCM63XX_DEV_SPI_H */
