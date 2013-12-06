/*
 * Broadcom BCM63XX High Speed SPI Controller driver
 *
 * Copyright 2000-2010 Broadcom Corporation
 * Copyright 2011 Jonas Gorski <jonas.gorski@gmail.com>
 *
 * Licensed under the GNU/GPL. See COPYING for details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>

#include <bcm63xx_regs.h>
#include <bcm63xx_dev_hsspi.h>


#define PFX		KBUILD_MODNAME

struct bcm63xx_hsspi {
	spinlock_t		lock;
	int			irq;
	u8			stopping;

	struct list_head	queue;
	struct workqueue_struct	*workqueue;
	struct work_struct	ws;
	struct completion	done;

	struct spi_transfer	*curr_trans;

	struct platform_device  *pdev;
	void __iomem		*regs;
	struct clk		*clk;

	/* Platform data */
	u32			speed_hz;

	/* data iomem */
	u8 __iomem		*fifo;


};

static void bcm63xx_hsspi_set_clk(struct bcm63xx_hsspi *bs, int hz,
				  int profile)
{
	int clock;

	clock = bs->speed_hz / hz;
	if (bs->speed_hz % HS_SPI_CLOCK_DEF)
		clock++;

	clock = 2048 / clock;
	if (2048 % clock)
		clock++;

	bcm_hsspi_writel(CLK_CTRL_ACCUM_RST_ON_LOOP | clock,
			 HSSPI_PROFILE_CLK_CTRL_REG(profile));
}

static int bcm63xx_hsspi_do_txrx(struct spi_device *spi,
				 struct spi_transfer *t1,
				 struct spi_transfer *t2)
{
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(spi->master);
	u8 chip_select = spi->chip_select;
	u16 opcode = 0;
	int prepend_size = 0;

	init_completion(&bs->done);
	bs->curr_trans = t2 ? t2 : t1;
	bcm63xx_hsspi_set_clk(bs, bs->curr_trans->speed_hz, chip_select);

	BUG_ON(t2 && !t1->tx_buf && t1->rx_buf && t2->tx_buf && !t2->rx_buf);

	if (t2 && !t2->tx_buf)
		prepend_size = t1->len;

	bcm_hsspi_writel(prepend_size<<MODE_CTRL_PREPENDBYTE_CNT_SHIFT |
			 2<<MODE_CTRL_MULTIDATA_WR_STRT_SHIFT |
			 2<<MODE_CTRL_MULTIDATA_RD_STRT_SHIFT | 0xff,
			 HSSPI_PROFILE_MODE_CTRL_REG(chip_select));

	if (t1->rx_buf && t1->tx_buf)
		opcode = HSSPI_OP_READ_WRITE;
	else if (t1->rx_buf || (t2 && t2->rx_buf))
		opcode = HSSPI_OP_READ;
	else if (t1->tx_buf)
		opcode = HSSPI_OP_WRITE;

	BUG_ON(opcode == 0);

	if (opcode == HSSPI_OP_READ && t2)
		opcode |= t2->len;
	else
		opcode |= t1->len;

	if (t1->tx_buf) {
		memcpy_toio(bs->fifo + 2, t1->tx_buf, t1->len);
		if (t2 && t2->tx_buf) {
			memcpy_toio(bs->fifo + 2 + t1->len,
				    t2->tx_buf, t2->len);
			opcode += t2->len;
		}
	}

	memcpy_toio(bs->fifo, &opcode, sizeof(opcode));

	/* enable interrupt */
	bcm_hsspi_writel(HSSPI_PING0_CMD_DONE, HSSPI_INT_MASK_REG);

	/* start the transfer */
	bcm_hsspi_writel(chip_select << PINGPONG_CMD_SS_SHIFT |
			 chip_select << PINGPONG_CMD_PROFILE_SHIFT |
			 PINGPONG_COMMAND_START_NOW,
			 HSSPI_PINGPONG_COMMAND_REG(0));

	wait_for_completion(&bs->done);
	return t1->len + (t2 ? t2->len : 0);
}
static int bcm63xx_hsspi_setup(struct spi_device *spi)
{
	struct bcm63xx_hsspi *bs;
	u32 reg;
	bs = spi_master_get_devdata(spi->master);

	if (bs->stopping)
		return -ESHUTDOWN;

	if (!spi->bits_per_word)
		spi->bits_per_word = 8;

	if (spi->bits_per_word != 8)
		return -EINVAL;

	if (spi->max_speed_hz == 0)
		return -EINVAL;

	reg = bcm_hsspi_readl(HSSPI_PROFILE_SIGNAL_CTRL_REG(spi->chip_select));
	reg &= ~(SIGNAL_CTRL_LAUNCH_RISING | SIGNAL_CTRL_LATCH_RISING);

	if (spi->mode & SPI_CPHA)
		reg |= SIGNAL_CTRL_LAUNCH_RISING;
	else
		reg |= SIGNAL_CTRL_LATCH_RISING;

	bcm_hsspi_writel(reg, HSSPI_PROFILE_SIGNAL_CTRL_REG(spi->chip_select));

	return 0;
}


static int bcm63xx_hsspi_transfer(struct spi_device *spi,
				  struct spi_message *msg)
{
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(spi->master);
	struct spi_transfer *t, *prev = NULL;

	if (unlikely(list_empty(&msg->transfers)))
		return -EINVAL;

	if (bs->stopping)
		return -ESHUTDOWN;

	list_for_each_entry(t, &msg->transfers, transfer_list)	{
		/* check transfer parameters */
		if (!t->tx_buf && !t->rx_buf)
			return -EINVAL;

		if (t->speed_hz == 0)
			t->speed_hz = spi->max_speed_hz;

		if (t->speed_hz > spi->max_speed_hz)
			return -EINVAL;

		if (t->len > HS_SPI_BUFFER_LEN)
			return -EINVAL;

		/* reject if we have to combine two tx transfers and their
		 * combined length is bigger than the buffer
		 */
		if (prev && !prev->cs_change && !t->cs_change && prev->tx_buf &&
		    t->tx_buf && (prev->len + t->len) > HS_SPI_BUFFER_LEN)
			return -EINVAL;

		prev = t;
	}


	msg->actual_length = 0;

#if 0
	/* disable interrupts for the SPI controller
	   using spin_lock_irqsave would disable all interrupts */
	bcm_hsspi_writel(0, HSSPI_INT_MASK_REG);
#endif
	spin_lock(&bs->lock);
	list_add_tail(&msg->queue, &bs->queue);
	queue_work(bs->workqueue, &bs->ws);
	spin_unlock(&bs->lock);

#if 0
	bcm_hsspi_writel(HSSPI_PING0_CMD_DONE, HSSPI_INT_MASK_REG);
#endif
	return 0;
}

static void bcm63xx_hsspi_do_work(struct work_struct *work)
{
	struct bcm63xx_hsspi *bs = container_of(work, struct bcm63xx_hsspi,
						ws);
	struct spi_message *msg;
	struct spi_transfer *prev = NULL;
	struct spi_transfer *t;
	u32 reg;

	int len = 0;

	spin_lock(&bs->lock);
	msg = list_entry(bs->queue.next, struct spi_message, queue);
	list_del(&msg->queue);
	spin_unlock(&bs->lock);

	if (bs->stopping) {
		msg->status = -ESHUTDOWN;
		goto out;
	}

	/* setup clock polarity */
	reg = bcm_hsspi_readl(HSSPI_GLOBAL_CTRL_REG);
	reg &= ~GLOBAL_CTRL_CLK_POLARITY;

	if (msg->spi->mode & SPI_CPOL)
		reg |= GLOBAL_CTRL_CLK_POLARITY;

	bcm_hsspi_writel(reg, HSSPI_GLOBAL_CTRL_REG);

	list_for_each_entry(t, &msg->transfers, transfer_list) {
		/*
		 * This controller does not support keeping the chip select
		 * active between transfers.
		 * This logic currently supports combining:
		 *  write then read with no cs_change (e.g. m25p80 RDSR)
		 *  write then write with no cs_change (e.g. m25p80 PP)
		 */
		if (prev && prev->tx_buf && !prev->cs_change && !t->cs_change) {
			/* combine write with following transfer */
			len += bcm63xx_hsspi_do_txrx(msg->spi, prev, t);
			prev = NULL;
			continue;
		}

		/* write the previous pending transfer */
		if (prev != NULL)
			len += bcm63xx_hsspi_do_txrx(msg->spi, prev, NULL);

		prev = t;
	}

	/* do last pending transfer */
	if (prev != NULL)
		len += bcm63xx_hsspi_do_txrx(msg->spi, prev, NULL);

	msg->status = 0;
	msg->actual_length = len;
out:
	msg->complete(msg->context);
}

static irqreturn_t bcm63xx_hsspi_interrupt(int irq, void *dev_id)
{
	struct spi_master *master = (struct spi_master *)dev_id;
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(master);

	if (bcm_hsspi_readl(HSSPI_INT_STATUS_MASKED_REG) == 0)
		return IRQ_NONE;

	bcm_hsspi_writel(HSSPI_INT_CLEAR_ALL, HSSPI_INT_STATUS_REG);
	bcm_hsspi_writel(0, HSSPI_INT_MASK_REG);

	spin_lock(&bs->lock);

	if (bs->curr_trans && bs->curr_trans->rx_buf)
		memcpy_fromio(bs->curr_trans->rx_buf,  bs->fifo,
			      bs->curr_trans->len);

	complete(&bs->done);
	spin_unlock(&bs->lock);

	return IRQ_HANDLED;
}


static void bcm63xx_hsspi_cleanup(struct spi_device *spi)
{
	/* would free spi_controller memory here if any was allocated */
}

static int __devinit bcm63xx_hsspi_probe(struct platform_device *pdev)
{

	struct spi_master *master;
	struct bcm63xx_hsspi *bs;
	struct resource *res_mem;
	struct device *dev = &pdev->dev;
	struct bcm63xx_hsspi_pdata *pdata = pdev->dev.platform_data;
	struct clk *clk;
	int irq;
	int ret;

	res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res_mem) {
		dev_err(dev, "no iomem\n");
		return -ENXIO;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "no irq\n");
		return -ENXIO;
	}

	clk = clk_get(dev, "hsspi");

	if (IS_ERR(clk)) {
		ret = PTR_ERR(clk);
		goto out_release;
	}
	clk_enable(clk);

	master = spi_alloc_master(&pdev->dev, sizeof(*bs));
	if (!master) {
		ret = -ENOMEM;
		goto out_disable_clk;
	}

	bs = spi_master_get_devdata(master);
	init_completion(&bs->done);
	bs->pdev = pdev;
	bs->clk = clk;

	if (!devm_request_mem_region(&pdev->dev, res_mem->start,
					resource_size(res_mem), PFX)) {
		dev_err(dev, "iomem request failed\n");
		ret = -ENXIO;
		goto out_put_master;
	}

	bs->regs = devm_ioremap_nocache(&pdev->dev, res_mem->start,
							resource_size(res_mem));
	if (!bs->regs) {
		dev_err(dev, "unable to ioremap regs\n");
		ret = -ENOMEM;
		goto out_put_master;
	}

	master->bus_num        = pdata->bus_num;
	master->num_chipselect = 8;
	master->setup          = bcm63xx_hsspi_setup;
	master->transfer       = bcm63xx_hsspi_transfer;
	master->cleanup        = bcm63xx_hsspi_cleanup;
	master->mode_bits      = SPI_CPOL | SPI_CPHA;

	bs->speed_hz = pdata->speed_hz;
	bs->fifo = (u8 *)(bs->regs + HSSPI_FIFO_REG(0));

	platform_set_drvdata(pdev, master);

	spin_lock_init(&bs->lock);
	INIT_LIST_HEAD(&bs->queue);
	INIT_WORK(&bs->ws, bcm63xx_hsspi_do_work);
	bs->workqueue = create_singlethread_workqueue(pdev->name);
	bs->curr_trans = NULL;

	/* Initialize the hardware */
	bcm_hsspi_writel(0, HSSPI_INT_MASK_REG);

	bcm_hsspi_writel(bcm_hsspi_readl(HSSPI_GLOBAL_CTRL_REG) |
			 GLOBAL_CTRL_CLK_GATE_SSOFF,
			 HSSPI_GLOBAL_CTRL_REG);

	ret = request_irq(irq, bcm63xx_hsspi_interrupt, IRQF_SHARED, pdev->name,
			  master);

	if (ret)
		goto out_destroy_workqueue;

	spin_lock(&bs->lock);
	bs->irq = irq;
	spin_unlock(&bs->lock);

	/* register and we are done */
	ret = spi_register_master(master);
	if (ret)
		goto out_free_irq;

	return 0;

out_free_irq:
	free_irq(bs->irq, master);
out_destroy_workqueue:
	flush_workqueue(bs->workqueue);
	destroy_workqueue(bs->workqueue);
	iounmap(bs->regs);
out_put_master:
	spi_master_put(master);
out_disable_clk:
	clk_disable(clk);
	clk_put(clk);
out_release:
	release_mem_region(res_mem->start, resource_size(res_mem));

	return ret;
}


static int __exit bcm63xx_hsspi_remove(struct platform_device *pdev)
{
	struct spi_master *master = platform_get_drvdata(pdev);
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(master);
	struct spi_message *msg;

	cancel_work_sync(&bs->ws);

	/* reset the hardware and block queue progress */
	bcm_hsspi_writel(0, HSSPI_INT_MASK_REG);

	spin_lock(&bs->lock);
	/* HW shutdown */
	bs->stopping = 1;
	spin_unlock(&bs->lock);


	/* Terminate remaining queued transfers */
	list_for_each_entry(msg, &bs->queue, queue) {
		msg->status = -ESHUTDOWN;
		msg->complete(msg->context);
	}


	free_irq(bs->irq, master);
	flush_workqueue(bs->workqueue);
	destroy_workqueue(bs->workqueue);

	clk_disable(bs->clk);
	clk_put(bs->clk);

	spi_unregister_master(master);
	return 0;
}

#ifdef CONFIG_PM
static int bcm63xx_hsspi_suspend(struct platform_device *pdev,
				 pm_message_t mesg)
{
	struct spi_master	*master = platform_get_drvdata(pdev);
	struct bcm63xx_hsspi	*bs = spi_master_get_devdata(master);

	clk_disable(bs->clk);

	return 0;
}

static int bcm63xx_hsspi_resume(struct platform_device *pdev)
{
	struct spi_master	*master = platform_get_drvdata(pdev);
	struct bcm63xx_hsspi	*bs = spi_master_get_devdata(master);

	clk_enable(bs->clk);

	return 0;
}

static const struct dev_pm_ops bcm63xx_hsspi_pm_ops = {
	.suspend	= bcm63xx_hsspi_suspend,
	.resume		= bcm63xx_hsspi_resume,
};
#else
#define bcm63xx_spi_suspend	NULL
#define bcm63xx_spi_resume	NULL
#endif

static struct platform_driver bcm63xx_hsspi_driver = {
	.driver = {
		.name	= "bcm63xx-hsspi",
		.owner	= THIS_MODULE,
	},
	.probe		= bcm63xx_hsspi_probe,
	.remove		= __exit_p(bcm63xx_hsspi_remove),
	.suspend	= bcm63xx_spi_suspend,
	.resume		= bcm63xx_spi_resume,
};

static int __init bcm63xx_hsspi_init(void)
{
	return platform_driver_register(&bcm63xx_hsspi_driver);
}

static void __exit bcm63xx_hsspi_exit(void)
{
	platform_driver_unregister(&bcm63xx_hsspi_driver);
}

module_init(bcm63xx_hsspi_init);
module_exit(bcm63xx_hsspi_exit);

MODULE_ALIAS("platform:bcm63xx_hsspi");
MODULE_DESCRIPTION("Broadcom BCM63xx HS SPI Controller driver");
MODULE_AUTHOR("Jonas Gorski <jonas.gorski@gmail.com>");
MODULE_LICENSE("GPL");
