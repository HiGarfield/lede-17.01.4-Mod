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
#include <linux/mutex.h>

#include <ralink_regs.h>

#include "mtk_eth_soc.h"
#include "gsw_mt7620.h"

/* set while one disable_irq_nosync() is outstanding */
#define GSW_IRQ_DISABLED	0

void mtk_switch_w32(struct mt7620_gsw *gsw, u32 val, unsigned reg)
{
	iowrite32(val, gsw->base + reg);
}

u32 mtk_switch_r32(struct mt7620_gsw *gsw, unsigned reg)
{
	return ioread32(gsw->base + reg);
}

/* The MT7530 indirect register window is shared with swconfig and with the
 * deferred link work: both build a page-select plus two data accesses while
 * holding bus->mdio_lock. mt7530_mdio_w32() only serialises the PIAC
 * register itself, so an IMR write done without the bus lock can be
 * interleaved with such a sequence and end up in the wrong switch register.
 */
static int mt7621_gsw_set_imr(struct mt7620_gsw *gsw, struct fe_priv *fe,
			      u32 mask)
{
	struct mii_bus *bus = fe ? fe->mii_bus : NULL;
	int ret;

	if (bus)
		mutex_lock(&bus->mdio_lock);
	ret = mt7530_mdio_w32(gsw, 0x7008, mask);
	if (bus)
		mutex_unlock(&bus->mdio_lock);

	if (ret)
		dev_err(gsw->dev, "failed to %s switch IRQ sources: %d\n",
			mask ? "unmask" : "mask", ret);

	return ret;
}

static void gsw_irq_work_mt7621(struct work_struct *work)
{
	struct mt7620_gsw *gsw = container_of(work, struct mt7620_gsw,
					      irq_work);
	struct fe_priv *priv = gsw->priv;
	struct mii_bus *bus = priv ? priv->mii_bus : NULL;
	/* Snapshot both pointers before the first use: the teardown paths
	 * clear gsw->priv/priv->phy while this work may still be running,
	 * and a lock/unlock pair must not be decided by two separate reads
	 * of a mutable pointer.
	 */
	struct fe_phy *phy = priv ? priv->phy : NULL;
	unsigned long flags;
	u32 reg, i;
	int link[5];
	bool link_valid[5] = { false, };

	/* swconfig reaches the same MT7530 indirect registers through
	 * mt7530.c, which holds bus->mdio_lock across the page-select +
	 * two data accesses. This work builds those three-step transactions
	 * via mt7530_mdio_r32/w32() under gsw->mdio_lock only. Without the
	 * bus lock the two three-step sequences can interleave: one sets
	 * the page, the other overwrites it, and the first then reads or
	 * writes the wrong register. bus is NULL during early hw init and
	 * teardown, when no swconfig user exists yet.
	 */
	if (bus)
		mutex_lock(&bus->mdio_lock);

	/* MDIO transactions involve busy-waiting the PIAC and are far
	 * too slow for hardirq context; they run here in process
	 * context instead.
	 *
	 * The switch interrupt is IRQ_TYPE_LEVEL_HIGH and the MT7530
	 * keeps the line asserted while (ISR & IMR) != 0, so the status
	 * register has to be cleared on *every* exit path. In particular
	 * this must happen before the !priv early return used during
	 * teardown: skipping the write-back leaves the source active and
	 * enable_irq() below re-enters the handler immediately, spinning
	 * the interrupt/work pair forever. Skipping it when the read
	 * failed has the same effect.
	 */
	if (mt7530_mdio_r32(gsw, 0x700c, &reg)) {
		mt7530_mdio_w32(gsw, 0x700c, 0xffffffff);
		goto out_reenable;
	}
	mt7530_mdio_w32(gsw, 0x700c, reg);

	if (!priv)
		goto out_reenable;

	/* Each mt7530_mdio_r32() issues three PIAC transactions and each
	 * transaction can busy-wait up to 2000 iterations. Collect all
	 * port states before taking phy->lock: spin_lock_irqsave() disables
	 * local interrupts, so doing up to five of these MDIO reads inside
	 * the critical section can stall a UP target for tens of ms.
	 */
	for (i = 0; i < 5; i++) {
		unsigned int val;

		if (!(reg & BIT(i)))
			continue;
		if (mt7530_mdio_r32(gsw, 0x3008 + (i * 0x100), &val))
			continue;
		link[i] = val & 0x1;
		link_valid[i] = true;
	}

	/* Keep link[] updates atomic with the PHY-side state machine, like
	 * the MT7620 interrupt path does.
	 */
	if (phy)
		spin_lock_irqsave(&phy->lock, flags);
	for (i = 0; i < 5; i++) {
		if (!link_valid[i] || link[i] == priv->link[i])
			continue;

		priv->link[i] = link[i];
		netdev_info(priv->netdev, "port %d link %s\n", i,
			    link[i] ? "up" : "down");
	}
	if (phy)
		spin_unlock_irqrestore(&phy->lock, flags);

	mt7620_handle_carrier(priv);

out_reenable:
	/* Clear the guard before the line becomes live again: a handler
	 * entering right after enable_irq() must be able to start a new
	 * disable/work cycle. Exactly one enable balances the one
	 * disable_irq_nosync() done by the handler.
	 */
	clear_bit(GSW_IRQ_DISABLED, &gsw->irq_state);
	if (bus)
		mutex_unlock(&bus->mdio_lock);
	enable_irq(gsw->irq);
}

static irqreturn_t gsw_interrupt_mt7621(int irq, void *_priv)
{
	struct fe_priv *priv = (struct fe_priv *)_priv;
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;

	/* swpriv is the only way this handler reaches the work item; it
	 * can be cleared while the action is still live during teardown
	 */
	if (!gsw)
		return IRQ_HANDLED;

	/* The source is level-high: on SMP a second handler can enter on
	 * another CPU before the line is fully masked. schedule_work()
	 * coalesces an already queued work item, so a second
	 * disable_irq_nosync() would never be balanced by a second
	 * enable_irq() and the interrupt would stay disabled forever.
	 * Only the first entry owns the disable/work cycle.
	 */
	if (test_and_set_bit(GSW_IRQ_DISABLED, &gsw->irq_state))
		return IRQ_HANDLED;

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
	/* Keep the back-pointer even when no interrupt is configured: the
	 * remove path needs it to make the FE side forget this devm object.
	 */
	gsw->priv = priv;

	if (gsw->irq) {
		int irqerr;

		/* a previous instance has been drained by
		 * mt7621_gsw_stop_irq(); start with a clean guard
		 */
		clear_bit(GSW_IRQ_DISABLED, &gsw->irq_state);
		INIT_WORK(&gsw->irq_work, gsw_irq_work_mt7621);
		irqerr = request_irq(gsw->irq, gsw_interrupt_mt7621, 0,
				     "gsw", priv);

		if (irqerr) {
			dev_err(gsw->dev, "request_irq failed: %d\n", irqerr);
			/* do not leave swpriv pointing at a switch the
			 * caller now considers uninitialised
			 */
			priv->soc->swpriv = NULL;
			gsw->priv = NULL;
			return irqerr;
		}
		disable_irq(gsw->irq);
	}

	mt7621_hw_init(gsw, np);

	if (gsw->irq) {
		/* only now unmask the switch interrupt sources, when the
		 * handler is actually registered
		 */
		mt7621_gsw_set_imr(gsw, priv, 0x1f);
		enable_irq(gsw->irq);
	}

	return 0;
}

/* Both the FE teardown and this driver's remove() may be the first to
 * release the interrupt, so make this helper idempotent. The hardirq
 * handler calls disable_irq_nosync() and queues irq_work, whose last
 * step performs the matching enable_irq(). Teardown must not add an
 * extra disable_irq() of its own: that call can never be balanced when
 * cancel_work_sync() drops a still pending work, which leaves the IRQ
 * permanently disabled (and makes a later re-probe unable to receive
 * link changes).
 */
static void mt7621_gsw_stop_irq(struct mt7620_gsw *gsw)
{
	struct fe_priv *fe = gsw->priv;

	if (!gsw->irq || !fe)
		return;

	/* With the action gone no new work can be queued. Clearing the
	 * back-pointer makes a queued work take its no-op path, which
	 * still re-enables the line; flush_work() (unlike
	 * cancel_work_sync()) waits for that to happen and guarantees
	 * nothing references this devm object after remove() returns.
	 */
	free_irq(gsw->irq, fe);
	gsw->priv = NULL;
	flush_work(&gsw->irq_work);
}

void mtk_gsw_cleanup(struct fe_priv *priv)
{
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;

	if (!gsw)
		return;

	/* mask the port-change source before the handler goes away: the
	 * IMR was opened in mtk_gsw_init(), and the MT7530 keeps a level
	 * line asserted while (ISR & IMR) != 0
	 */
	if (gsw->irq)
		mt7621_gsw_set_imr(gsw, gsw->priv, 0);

	/* the irq was requested with the fe_priv as dev_id; release it
	 * on the fe side too, otherwise the handler may run after the
	 * netdev/fe_priv is gone (UAF), and a retried fe_init would
	 * request the same irq twice
	 */
	mt7621_gsw_stop_irq(gsw);

	/* The FE netdev and its fe_priv are freed shortly after
	 * ndo_uninit(). Clear the back-pointer unconditionally, also when
	 * mt7621_gsw_stop_irq() returned early because there is no irq,
	 * or a later gsw remove() would dereference freed fe_priv.
	 */
	gsw->priv = NULL;
	priv->soc->swpriv = NULL;
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
		struct fe_priv *fe = gsw->priv;

		/* The FE driver caches this devm object in soc->swpriv, and
		 * gsw_interrupt_mt7621() resolves it through that pointer
		 * before queueing &gsw->irq_work. Mask the source and drain
		 * both the action and the work BEFORE dropping swpriv,
		 * otherwise an interrupt in that window queues a work item
		 * from a NULL gsw.
		 */
		if (gsw->irq)
			mt7621_gsw_set_imr(gsw, fe, 0);
		/* gsw is devm allocated: drain the deferred link work
		 * before returning, and keep the irq disable depth
		 * balanced (see mt7621_gsw_stop_irq()).
		 */
		mt7621_gsw_stop_irq(gsw);

		if (fe)
			fe->soc->swpriv = NULL;
		gsw->priv = NULL;
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
