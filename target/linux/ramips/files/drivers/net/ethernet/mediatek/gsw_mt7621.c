/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2009-2015 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2015 Felix Fietkau <nbd@nbd.name>
 *   Copyright (C) 2013-2015 Michael Lee <igvtee@gmail.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/workqueue.h>

#include <ralink_regs.h>

#include "mtk_eth_soc.h"
#include "gsw_mt7620.h"

void mtk_switch_w32(struct mt7620_gsw *gsw, u32 val, unsigned reg)
{
	iowrite32(val, gsw->base + reg);
}

u32 mtk_switch_r32(struct mt7620_gsw *gsw, unsigned reg)
{
	return ioread32(gsw->base + reg);
}

static void gsw_irq_work_mt7621(struct work_struct *work)
{
	struct mt7620_gsw *gsw = container_of(work, struct mt7620_gsw,
					      irq_work);
	struct fe_priv *priv = gsw->priv;
	u32 reg, i;

	/* MDIO transactions involve busy-waiting the PIAC and are far
	 * too slow for hardirq context; they run here in process
	 * context instead
	 */
	if (!priv)
		goto out_reenable;

	/* The switch interrupt is IRQ_TYPE_LEVEL_HIGH and the MT7530
	 * keeps the line asserted while (ISR & IMR) != 0, so the status
	 * register has to be cleared on *every* exit path. Skipping the
	 * write-back when the read failed leaves the source active and
	 * enable_irq() below re-enters the handler immediately, which
	 * spins the interrupt/work pair forever.
	 */
	if (mt7530_mdio_r32(gsw, 0x700c, &reg)) {
		mt7530_mdio_w32(gsw, 0x700c, 0xffffffff);
		goto out_reenable;
	}
	mt7530_mdio_w32(gsw, 0x700c, reg);

	for (i = 0; i < 5; i++)
		if (reg & BIT(i)) {
			unsigned int link;

			if (mt7530_mdio_r32(gsw, 0x3008 + (i * 0x100),
					    &link))
				continue;
			link &= 0x1;

			if (link != priv->link[i]) {
				priv->link[i] = link;
				if (link)
					netdev_info(priv->netdev,
						    "port %d link up\n", i);
				else
					netdev_info(priv->netdev,
						    "port %d link down\n", i);
			}
		}

	mt7620_handle_carrier(priv);

out_reenable:
	enable_irq(gsw->irq);
}

static irqreturn_t gsw_interrupt_mt7621(int irq, void *_priv)
{
	struct fe_priv *priv = (struct fe_priv *)_priv;
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;

	disable_irq_nosync(irq);
	schedule_work(&gsw->irq_work);

	return IRQ_HANDLED;
}

static void mt7621_hw_init(struct mt7620_gsw *gsw, struct device_node *np)
{
	u32 i;
	u32 val;

	/* wardware reset the switch */
	fe_reset(RST_CTRL_MCM);
	mdelay(10);

	/* reduce RGMII2 PAD driving strength */
	rt_sysc_m32(3 << 4, 0, SYSC_PAD_RGMII2_MDIO);

	/* gpio mux - RGMII1=Normal mode */
	rt_sysc_m32(BIT(14), 0, SYSC_GPIO_MODE);

	/* set GMAC1 RGMII mode */
	rt_sysc_m32(3 << 12, 0, SYSC_REG_CFG1);

	/* enable MDIO to control MT7530 */
	rt_sysc_m32(3 << 12, 0, SYSC_GPIO_MODE);

	/* turn off all PHYs */
	for (i = 0; i <= 4; i++) {
		u16 mii_val;

		/* a timed-out read returns garbage that must not be
		 * OR-ed and written back into the BMCR
		 */
		if (mt7620_mii_read_reg(gsw, i, 0x0, &mii_val)) {
			dev_warn(gsw->dev, "PHY %d: power-down read failed\n",
				 i);
			continue;
		}
		mii_val |= BIT(11);
		_mt7620_mii_write(gsw, i, 0x0, mii_val);
	}

	/* reset the switch */
	mt7530_mdio_w32(gsw, 0x7000, 0x3);
	usleep_range(10, 20);

	/* Disable Flow Control Globally - MT7621 bug */
	usleep_range(10, 20);
	if (!mt7530_mdio_r32(gsw, 0x1fe0, &val)) {
		val &= ~BIT(31);
		mt7530_mdio_w32(gsw, 0x1fe0, val);
	}

	/* Disable flow control on Port 5 (GMAC) and Port 6 (CPU) */
	/* (GE1, Force 1000M/FD, FC OFF, MAX_RX_LENGTH 1536) */
	mtk_switch_w32(gsw, 0x2305e30b, GSW_REG_MAC_P0_MCR);
	for (i = 5; i <= 6; i++) {
		mt7530_mdio_w32(gsw, 0x3000 + (i * 0x100), 0x5e30b);
		usleep_range(10, 20);
	}

	/* turn off pause advertisement on all PHYs */
	for (i = 0; i <= 4; i++) {
		u16 mii_val;

		if (mt7620_mii_read_reg(gsw, i, 0x04, &mii_val)) {
			dev_warn(gsw->dev, "PHY %d: pause read failed\n", i);
			continue;
		}
		mii_val &= ~BIT(10);
		_mt7620_mii_write(gsw, i, 0x04, mii_val);
	}

	/* (GE2, Link down) */
	mtk_switch_w32(gsw, 0x8000, GSW_REG_MAC_P1_MCR);

	/* Set switch max RX frame length to 2k */
	mt7530_mdio_w32(gsw, GSW_REG_GMACCR, 0x3F0B);

	/* Enable Port 6, P5 as GMAC5, P5 disable */
	if (!mt7530_mdio_r32(gsw, 0x7804, &val)) {
		val &= ~BIT(8);
		val |= BIT(6) | BIT(13) | BIT(16);
		mt7530_mdio_w32(gsw, 0x7804, val);
	}

	val = rt_sysc_r32(0x10);
	val = (val >> 6) & 0x7;
	if (val >= 6) {
		/* 25Mhz Xtal - do nothing */
	} else if (val >= 3) {
		/* 40Mhz */

		/* disable MT7530 core clock */
		_mt7620_mii_write(gsw, 0, 13, 0x1f);
		_mt7620_mii_write(gsw, 0, 14, 0x410);
		_mt7620_mii_write(gsw, 0, 13, 0x401f);
		_mt7620_mii_write(gsw, 0, 14, 0x0);

		/* disable MT7530 PLL */
		_mt7620_mii_write(gsw, 0, 13, 0x1f);
		_mt7620_mii_write(gsw, 0, 14, 0x40d);
		_mt7620_mii_write(gsw, 0, 13, 0x401f);
		_mt7620_mii_write(gsw, 0, 14, 0x2020);

		/* for MT7530 core clock = 500Mhz */
		_mt7620_mii_write(gsw, 0, 13, 0x1f);
		_mt7620_mii_write(gsw, 0, 14, 0x40e);
		_mt7620_mii_write(gsw, 0, 13, 0x401f);
		_mt7620_mii_write(gsw, 0, 14, 0x119);

		/* enable MT7530 PLL */
		_mt7620_mii_write(gsw, 0, 13, 0x1f);
		_mt7620_mii_write(gsw, 0, 14, 0x40d);
		_mt7620_mii_write(gsw, 0, 13, 0x401f);
		_mt7620_mii_write(gsw, 0, 14, 0x2820);

		usleep_range(20, 40);

		/* enable MT7530 core clock */
		_mt7620_mii_write(gsw, 0, 13, 0x1f);
		_mt7620_mii_write(gsw, 0, 14, 0x410);
		_mt7620_mii_write(gsw, 0, 13, 0x401f);
	} else {
		/* 20Mhz Xtal - TODO */
	}

	/* RGMII */
	_mt7620_mii_write(gsw, 0, 14, 0x1);

	/* set MT7530 central align */
	if (!mt7530_mdio_r32(gsw, 0x7830, &val)) {
		val &= ~BIT(0);
		val |= BIT(1);
		mt7530_mdio_w32(gsw, 0x7830, val);
	}
	if (!mt7530_mdio_r32(gsw, 0x7a40, &val)) {
		val &= ~BIT(30);
		mt7530_mdio_w32(gsw, 0x7a40, val);
	}
	mt7530_mdio_w32(gsw, 0x7a78, 0x855);

	/* delay setting for 10/1000M */
	mt7530_mdio_w32(gsw, 0x7b00, 0x102);
	mt7530_mdio_w32(gsw, 0x7b04, 0x14);

	/* lower Tx Driving*/
	mt7530_mdio_w32(gsw, 0x7a54, 0x44);
	mt7530_mdio_w32(gsw, 0x7a5c, 0x44);
	mt7530_mdio_w32(gsw, 0x7a64, 0x44);
	mt7530_mdio_w32(gsw, 0x7a6c, 0x44);
	mt7530_mdio_w32(gsw, 0x7a74, 0x44);
	mt7530_mdio_w32(gsw, 0x7a7c, 0x44);

	/* Disable EEE */
	for (i = 0; i <= 4; i++) {
		_mt7620_mii_write(gsw, i, 13, 0x7);
		_mt7620_mii_write(gsw, i, 14, 0x3C);
		_mt7620_mii_write(gsw, i, 13, 0x4007);
		_mt7620_mii_write(gsw, i, 14, 0x0);
	}

	/* Disable EEE 10Base-Te */
	for (i = 0; i <= 4; i++) {
		_mt7620_mii_write(gsw, i, 13, 0x1f);
		_mt7620_mii_write(gsw, i, 14, 0x027b);
		_mt7620_mii_write(gsw, i, 13, 0x401f);
		_mt7620_mii_write(gsw, i, 14, 0x1177);
	}

	/* turn on all PHYs */
	for (i = 0; i <= 4; i++) {
		u16 mii_val;

		if (mt7620_mii_read_reg(gsw, i, 0, &mii_val)) {
			dev_warn(gsw->dev, "PHY %d: power-up read failed\n", i);
			continue;
		}
		mii_val &= ~BIT(11);
		_mt7620_mii_write(gsw, i, 0, mii_val);
	}

	if (!mt7530_mdio_r32(gsw, 0x7808, &val)) {
		val |= 3 << 16;
		mt7530_mdio_w32(gsw, 0x7808, val);
	}
	/* the IMR write moved to mtk_gsw_init(): enabling the switch
	 * interrupt mask without a registered handler would leave a
	 * spurious interrupt source hanging
	 */
}

static const struct of_device_id mediatek_gsw_match[] = {
	{ .compatible = "mediatek,mt7621-gsw" },
	{},
};
MODULE_DEVICE_TABLE(of, mediatek_gsw_match);

int mtk_gsw_init(struct fe_priv *priv)
{
	struct device_node *np = priv->switch_np;
	struct platform_device *pdev = of_find_device_by_node(np);
	struct mt7620_gsw *gsw;

	if (!pdev)
		return -ENODEV;

	if (!of_device_is_compatible(np, mediatek_gsw_match->compatible)) {
		put_device(&pdev->dev);
		return -EINVAL;
	}

	gsw = platform_get_drvdata(pdev);
	put_device(&pdev->dev);
	/* the platform device exists but has no drvdata yet: the gsw
	 * driver has not finished probing. -ENODEV would make fe_init()
	 * fail permanently and take the whole ethernet down with no
	 * retry, so ask for a deferred probe instead.
	 */
	if (!gsw)
		return -EPROBE_DEFER;

	priv->soc->swpriv = gsw;

	if (gsw->irq) {
		int irqerr;

		INIT_WORK(&gsw->irq_work, gsw_irq_work_mt7621);
		irqerr = request_irq(gsw->irq, gsw_interrupt_mt7621, 0,
				     "gsw", priv);

		if (irqerr) {
			dev_err(gsw->dev, "request_irq failed: %d\n", irqerr);
			return irqerr;
		}
		gsw->priv = priv;
		disable_irq(gsw->irq);
	}

	mt7621_hw_init(gsw, np);

	if (gsw->irq) {
		/* only now unmask the switch interrupt sources, when the
		 * handler is actually registered
		 */
		mt7530_mdio_w32(gsw, 0x7008, 0x1f);
		enable_irq(gsw->irq);
	}

	return 0;
}

void mtk_gsw_cleanup(struct fe_priv *priv)
{
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;

	if (!gsw)
		return;

	/* the irq was requested with the fe_priv as dev_id; release it
	 * on the fe side too, otherwise the handler may run after the
	 * netdev/fe_priv is gone (UAF), and a retried fe_init would
	 * request the same irq twice
	 */
	if (gsw->irq && gsw->priv) {
		/* Stop the source first and drain the deferred link work
		 * *before* dropping the handler: the work re-enables the
		 * irq on exit, which would resurrect an interrupt that
		 * has already been freed ("Unbalanced enable for IRQ").
		 */
		disable_irq(gsw->irq);
		cancel_work_sync(&gsw->irq_work);
		free_irq(gsw->irq, gsw->priv);
		gsw->priv = NULL;
	}
}

static int mt7621_gsw_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct mt7620_gsw *gsw;

	gsw = devm_kzalloc(&pdev->dev, sizeof(struct mt7620_gsw), GFP_KERNEL);
	if (!gsw)
		return -ENOMEM;

	gsw->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(gsw->base))
		return PTR_ERR(gsw->base);

	gsw->dev = &pdev->dev;
	spin_lock_init(&gsw->mdio_lock);
	gsw->irq = platform_get_irq(pdev, 0);
	/* do not swallow -EPROBE_DEFER: mapping it to 0 made the probe
	 * succeed permanently without an interrupt (same fix as
	 * mt7620_gsw_probe)
	 */
	if (gsw->irq == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (gsw->irq < 0)
		gsw->irq = 0;

	platform_set_drvdata(pdev, gsw);

	return 0;
}

static int mt7621_gsw_remove(struct platform_device *pdev)
{
	struct mt7620_gsw *gsw = platform_get_drvdata(pdev);

	if (gsw) {
		if (gsw->irq && gsw->priv) {
			/* gsw is devm allocated: returning from remove
			 * frees it, so the queued link work must be gone
			 * first or it dereferences freed memory.
			 */
			disable_irq(gsw->irq);
			cancel_work_sync(&gsw->irq_work);
			free_irq(gsw->irq, gsw->priv);
			/* both this remove and mtk_gsw_cleanup() release the
			 * same irq; drop the dev_id pointer so whoever runs
			 * second sees nothing left to free
			 */
			gsw->priv = NULL;
		}
		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}

static struct platform_driver gsw_driver = {
	.probe = mt7621_gsw_probe,
	.remove = mt7621_gsw_remove,
	.driver = {
		.name = "mt7621-gsw",
		.owner = THIS_MODULE,
		.of_match_table = mediatek_gsw_match,
	},
};

module_platform_driver(gsw_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("GBit switch driver for Mediatek MT7621 SoC");
MODULE_VERSION(MTK_FE_DRV_VERSION);
