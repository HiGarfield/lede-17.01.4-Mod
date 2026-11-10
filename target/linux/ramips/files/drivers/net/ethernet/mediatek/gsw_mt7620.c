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

#include <ralink_regs.h>

#include "mtk_eth_soc.h"
#include "gsw_mt7620.h"

void mtk_switch_w32(struct mt7620_gsw *gsw, u32 val, unsigned reg)
{
	iowrite32(val, gsw->base + reg);
}
EXPORT_SYMBOL_GPL(mtk_switch_w32);

u32 mtk_switch_r32(struct mt7620_gsw *gsw, unsigned reg)
{
	return ioread32(gsw->base + reg);
}
EXPORT_SYMBOL_GPL(mtk_switch_r32);

/* The FE ndo_init()/ndo_uninit() and this platform driver's remove() are
 * two independent paths that both own the same irq action and the
 * soc->swpriv / gsw->priv pair. Without a common lock, both can snapshot a
 * non-NULL fe_priv and call free_irq() twice, or dereference an fe_priv
 * already released by unregister_netdev(). The lock is module-wide, so it
 * stays valid even after the devm gsw object and its back-pointer are gone.
 */
static DEFINE_MUTEX(gsw_teardown_lock);

static irqreturn_t gsw_interrupt_mt7620(int irq, void *_priv)
{
	struct fe_priv *priv = (struct fe_priv *)_priv;
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;
	struct fe_phy *phy;
	unsigned long flags;
	u32 status;
	int i;

	/* swpriv can already be gone while this action is still
	 * registered during teardown; every access below dereferences it
	 */
	if (!gsw)
		return IRQ_HANDLED;

	/* Snapshot the pointer once: fe_mdio_init() can clear priv->phy,
	 * and deciding lock/unlock from two separate reads of the same
	 * mutable pointer can leave the spinlock held forever (or unlock
	 * a different object).
	 */
	phy = priv->phy;

	status = mtk_switch_r32(gsw, GSW_REG_ISR);
	/* Only the port-change sources are unmasked by mtk_gsw_init().
	 * A spurious/shared interrupt has nothing to acknowledge: a 0
	 * write into the W1C register clears no bit, and IRQ_HANDLED would
	 * hide a permanently asserted level line from the generic
	 * spurious-IRQ detection, re-entering this handler forever.
	 */
	if (!(status & PORT_IRQ_ST_CHG))
		return IRQ_NONE;

	/* switch_init() installs and unmasks this handler before
	 * fe_init() reaches fe_mdio_init(), so priv->phy can still be NULL
	 * here. link[] is only ever written under phy->lock (link adjust,
	 * port_init, the MT7621 irq work), and without the lock object
	 * there is no way to serialise an update. Skip the link walk in
	 * that window and just acknowledge the source below: no PHY state
	 * exists yet, and mt7620_handle_carrier() returns immediately
	 * while priv->phy is NULL anyway.
	 */
	if (!phy)
		goto ack_irq;

	/* priv->link[] is shared with the process-context phy
	 * link adjustment, serialise against it
	 */
	spin_lock_irqsave(&phy->lock, flags);
	/* PORT_IRQ_ST_CHG spans all GSW_NUM_PORTS ports and the
	 * ISR is write-one-to-clear. Sampling only the PHY ports
	 * (0..3/4) while acknowledging the whole mask discarded
	 * link-change events for the remaining ports.
	 */
	for (i = 0; i < GSW_NUM_PORTS; i++) {
		u32 status = mtk_switch_r32(gsw, GSW_REG_PORT_STATUS(i));
		int link = status & 0x1;

		if (link != priv->link[i])
			mt7620_print_link_state(priv, i, link,
						(status >> 2) & 3,
						(status >> 1) & 1);

		priv->link[i] = link;
	}
	spin_unlock_irqrestore(&phy->lock, flags);

	mt7620_handle_carrier(priv);
ack_irq:
	/* acknowledge only the bit we actually handled instead of
	 * clearing every status bit, including events that arrived
	 * while this handler was running
	 */
	mtk_switch_w32(gsw, status & PORT_IRQ_ST_CHG, GSW_REG_ISR);

	return IRQ_HANDLED;
}

static int mt7620_mdio_mode(struct device_node *eth_node)
{
	struct device_node *phy_node, *mdiobus_node;
	u32 reg;
	int ret = 0;

	mdiobus_node = of_get_child_by_name(eth_node, "mdio-bus");

	if (mdiobus_node) {
		if (of_property_read_bool(mdiobus_node, "mediatek,mdio-mode"))
			ret = 1;

		/* of_get_property(..., NULL) gives no length, yet the old
		 * code always dereferenced four bytes; a short "reg"
		 * property was an out-of-bounds DT read.
		 */
		for_each_child_of_node(mdiobus_node, phy_node) {
			if (!of_property_read_u32(phy_node, "reg", &reg) &&
			    reg == 0x1f)
				ret = 1;
		}

		of_node_put(mdiobus_node);
	}

	return ret;
}

static int mt7620_hw_init(struct mt7620_gsw *gsw, int mdio_mode)
{
	u32 i;
	u32 is_BGA = (rt_sysc_r32(0x0c) >> 16) & 1;
	int err;

	/* Every transaction below goes through mt7620_mii_busy_wait(),
	 * which records a timeout in mdio_err while holding mdio_lock.
	 * Clear and sample the field under that same lock so no runtime
	 * MDIO user can race with the init sequence.
	 */
	mutex_lock(&gsw->mdio_lock);
	gsw->mdio_err = 0;
	mutex_unlock(&gsw->mdio_lock);

	/* CFG1 is shared with the other SoC drivers; a read-modify-write
	 * here can drop a concurrent update. Use the atomic masked helper
	 * (soc_mt7620.c already does this for the port mux fields).
	 */
	rt_sysc_m32(0, BIT(8), SYSC_REG_CFG1);
	mtk_switch_w32(gsw, mtk_switch_r32(gsw, GSW_REG_CKGCR) & ~(0x3 << 4), GSW_REG_CKGCR);

	/* Enable MIB stats */
	mtk_switch_w32(gsw, mtk_switch_r32(gsw, GSW_REG_MIB_CNT_EN) | (1 << 1), GSW_REG_MIB_CNT_EN);

	if (mdio_mode) {
		u32 val;

		/* turn off ephy and set phy base addr to 12 */
		mtk_switch_w32(gsw, mtk_switch_r32(gsw, GSW_REG_GPC1) |
			(0x1f << 24) | (0xc << 16),
			GSW_REG_GPC1);

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
	} else {
		/* global page 4 */
		_mt7620_mii_write(gsw, 1, 31, 0x4000);

		_mt7620_mii_write(gsw, 1, 17, 0x7444);
		if (is_BGA)
			_mt7620_mii_write(gsw, 1, 19, 0x0114);
		else
			_mt7620_mii_write(gsw, 1, 19, 0x0117);

		_mt7620_mii_write(gsw, 1, 22, 0x10cf);
		_mt7620_mii_write(gsw, 1, 25, 0x6212);
		_mt7620_mii_write(gsw, 1, 26, 0x0777);
		_mt7620_mii_write(gsw, 1, 29, 0x4000);
		_mt7620_mii_write(gsw, 1, 28, 0xc077);
		_mt7620_mii_write(gsw, 1, 24, 0x0000);

		/* global page 3 */
		_mt7620_mii_write(gsw, 1, 31, 0x3000);
		_mt7620_mii_write(gsw, 1, 17, 0x4838);

		/* global page 2 */
		_mt7620_mii_write(gsw, 1, 31, 0x2000);
		if (is_BGA) {
			_mt7620_mii_write(gsw, 1, 21, 0x0515);
			_mt7620_mii_write(gsw, 1, 22, 0x0053);
			_mt7620_mii_write(gsw, 1, 23, 0x00bf);
			_mt7620_mii_write(gsw, 1, 24, 0x0aaf);
			_mt7620_mii_write(gsw, 1, 25, 0x0fad);
			_mt7620_mii_write(gsw, 1, 26, 0x0fc1);
		} else {
			_mt7620_mii_write(gsw, 1, 21, 0x0517);
			_mt7620_mii_write(gsw, 1, 22, 0x0fd2);
			_mt7620_mii_write(gsw, 1, 23, 0x00bf);
			_mt7620_mii_write(gsw, 1, 24, 0x0aab);
			_mt7620_mii_write(gsw, 1, 25, 0x00ae);
			_mt7620_mii_write(gsw, 1, 26, 0x0fff);
		}
		/* global page 1 */
		_mt7620_mii_write(gsw, 1, 31, 0x1000);
		_mt7620_mii_write(gsw, 1, 17, 0xe7f8);

		/* turn on all PHYs */
		for (i = 0; i <= 4; i++) {
			u16 mii_val;

			/* a timed-out read must not be OR-ed and written
			 * back: it would program 0xffff into the BMCR
			 */
			if (mt7620_mii_read_reg(gsw, i, 0, &mii_val)) {
				dev_warn(gsw->dev,
					 "PHY %d: power-up read failed\n", i);
				continue;
			}
			mii_val &= ~BIT(11);
			_mt7620_mii_write(gsw, i, 0, mii_val);
		}
	}

	/* global page 0 */
	_mt7620_mii_write(gsw, 1, 31, 0x8000);
	_mt7620_mii_write(gsw, 0, 30, 0xa000);
	_mt7620_mii_write(gsw, 1, 30, 0xa000);
	_mt7620_mii_write(gsw, 2, 30, 0xa000);
	_mt7620_mii_write(gsw, 3, 30, 0xa000);

	_mt7620_mii_write(gsw, 0, 4, 0x05e1);
	_mt7620_mii_write(gsw, 1, 4, 0x05e1);
	_mt7620_mii_write(gsw, 2, 4, 0x05e1);
	_mt7620_mii_write(gsw, 3, 4, 0x05e1);

	/* global page 2 */
	_mt7620_mii_write(gsw, 1, 31, 0xa000);
	_mt7620_mii_write(gsw, 0, 16, 0x1111);
	_mt7620_mii_write(gsw, 1, 16, 0x1010);
	_mt7620_mii_write(gsw, 2, 16, 0x1515);
	_mt7620_mii_write(gsw, 3, 16, 0x0f0f);

	/* CPU Port6 Force Link 1G, FC ON */
	mtk_switch_w32(gsw, 0x5e33b, GSW_REG_PORT_PMCR(6));

	/* Set Port 6 as CPU Port */
	mtk_switch_w32(gsw, 0x7f7f7fe0, 0x0010);

	/* setup port 4 */
	if (gsw->port4 == PORT4_EPHY) {
		/* set only the two port4 mux bits: the plain RMW used here
		 * raced other users of the shared CFG1 register
		 */
		rt_sysc_m32(0, 3 << 14, SYSC_REG_CFG1);
		_mt7620_mii_write(gsw, 4, 30, 0xa000);
		_mt7620_mii_write(gsw, 4, 4, 0x05e1);
		_mt7620_mii_write(gsw, 4, 16, 0x1313);
		pr_info("gsw: setting port4 to ephy mode\n");
	} else if (!mdio_mode) {
		rt_sysc_m32(3 << 14, 0, SYSC_REG_CFG1);
		pr_info("gsw: setting port4 to gmac mode\n");
	}

	mutex_lock(&gsw->mdio_lock);
	err = gsw->mdio_err;
	mutex_unlock(&gsw->mdio_lock);

	return err;
}

static const struct of_device_id mediatek_gsw_match[] = {
	{ .compatible = "mediatek,mt7620-gsw" },
	{},
};
MODULE_DEVICE_TABLE(of, mediatek_gsw_match);

int mtk_gsw_init(struct fe_priv *priv)
{
	struct device_node *np = priv->switch_np;
	struct platform_device *pdev = of_find_device_by_node(np);
	struct mt7620_gsw *gsw;
	int err = 0;
	int mdio_mode;

	if (!pdev)
		return -ENODEV;

	if (!of_device_is_compatible(np, mediatek_gsw_match->compatible)) {
		put_device(&pdev->dev);
		return -EINVAL;
	}

	/* Both teardown paths (mtk_gsw_cleanup() and mt7620_gsw_remove())
	 * read gsw->priv and release the irq action under gsw_teardown_lock.
	 * Publishing swpriv/gsw->priv and installing the action must use
	 * the very same lock: otherwise remove() can free_irq() with a
	 * dev_id that init has not registered yet, or init can install the
	 * action after remove() has already cleared both pointers, leaving
	 * a live handler on a devm object (and later on a freed fe_priv).
	 */
	mutex_lock(&gsw_teardown_lock);

	gsw = platform_get_drvdata(pdev);
	put_device(&pdev->dev);
	/* the platform device exists but has no drvdata yet: the gsw
	 * driver has not finished probing. -ENODEV would make fe_init()
	 * fail permanently and take the whole ethernet down with no
	 * retry, so ask for a deferred probe instead.
	 */
	if (!gsw) {
		err = -EPROBE_DEFER;
		goto out_unlock;
	}

	priv->soc->swpriv = gsw;
	/* Keep the back-pointer even when no interrupt is configured: the
	 * remove path needs it to make the FE side forget this devm object.
	 */
	gsw->priv = priv;

	/* mt7620_hw_init() powers the PHYs down/up and forces the port
	 * link state, which raises the level-triggered port-change status.
	 * The handler is only registered below, and the chip's power-on
	 * IMR is not guaranteed to mask every source, so an asserted line
	 * with no action installed is reported as a spurious interrupt and
	 * genirq may disable the IRQ permanently. Mask all sources first;
	 * the port-change bits are unmasked again after request_irq().
	 */
	mtk_switch_w32(gsw, ~0, GSW_REG_IMR);

	mdio_mode = mt7620_mdio_mode(priv->dev->of_node);
	err = mt7620_hw_init(gsw, mdio_mode);
	if (err) {
		/* A single PIAC busy-wait timeout is often transient; the
		 * register sequence is idempotent, so try it once more
		 * before declaring the switch unusable.
		 */
		dev_err(gsw->dev, "switch hardware init failed: %d, retrying\n",
			err);
		err = mt7620_hw_init(gsw, mdio_mode);
	}
	if (err) {
		/* A timed-out page select or calibration write leaves the
		 * switch half configured. Do not publish it: fe_init()
		 * treats a switch_init() failure as terminal, no irq
		 * action is registered yet, and the IMR is still fully
		 * masked.
		 */
		dev_err(gsw->dev, "switch hardware init failed: %d\n", err);
		priv->soc->swpriv = NULL;
		gsw->priv = NULL;
		goto out_unlock;
	}

	if (gsw->irq) {
		err = request_irq(gsw->irq, gsw_interrupt_mt7620, 0,
				  "gsw", priv);

		if (err) {
			dev_err(gsw->dev, "request_irq failed: %d\n", err);
			/* do not leave swpriv pointing at a switch the
			 * caller now considers uninitialised. Roll back
			 * under the lock so no teardown path can observe
			 * a half-published pair of back-pointers.
			 */
			priv->soc->swpriv = NULL;
			gsw->priv = NULL;
			goto out_unlock;
		}
		mtk_switch_w32(gsw, ~PORT_IRQ_ST_CHG, GSW_REG_IMR);
	}

out_unlock:
	/* Only a successful init may re-allow switch registration. Do it
	 * while still holding gsw_teardown_lock: this makes the flag
	 * clear atomic with the swpriv publication above, so a GSW
	 * unbind cannot publish switch_dying in between and then have
	 * that claim erased by fe_init().
	 */
	if (!err)
		fe_switch_mark_alive(priv);
	mutex_unlock(&gsw_teardown_lock);
	return err;
}
EXPORT_SYMBOL_GPL(mtk_gsw_init);

void mtk_gsw_cleanup(struct fe_priv *priv)
{
	struct mt7620_gsw *gsw;
	struct fe_priv *owner;

	mutex_lock(&gsw_teardown_lock);
	gsw = (struct mt7620_gsw *)priv->soc->swpriv;
	if (!gsw) {
		mutex_unlock(&gsw_teardown_lock);
		return;
	}

	/* mask the port-change source before the handler goes away: the
	 * IMR was opened in mtk_gsw_init(), and an asserted level line
	 * with no action registered fires spurious interrupts
	 */
	if (gsw->irq)
		mtk_switch_w32(gsw, ~0, GSW_REG_IMR);

	/* Re-read the back-pointer under the lock: only the path that
	 * still owns it may release the irq action.
	 */
	owner = gsw->priv;
	if (gsw->irq && owner)
		free_irq(gsw->irq, owner);

	/* The FE netdev and its fe_priv are freed shortly after
	 * ndo_uninit(). Clear the back-pointer unconditionally, also on
	 * boards without a switch irq, or a later gsw remove() would
	 * dereference freed fe_priv to clear soc->swpriv.
	 */
	gsw->priv = NULL;
	priv->soc->swpriv = NULL;
	mutex_unlock(&gsw_teardown_lock);
}
EXPORT_SYMBOL_GPL(mtk_gsw_cleanup);

static int mt7620_gsw_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	const char *port4 = NULL;
	struct mt7620_gsw *gsw;
	struct device_node *np = pdev->dev.of_node;

	gsw = devm_kzalloc(&pdev->dev, sizeof(struct mt7620_gsw), GFP_KERNEL);
	if (!gsw)
		return -ENOMEM;

	gsw->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(gsw->base))
		return PTR_ERR(gsw->base);

	gsw->dev = &pdev->dev;
	mutex_init(&gsw->mdio_lock);

	of_property_read_string(np, "mediatek,port4", &port4);
	if (port4 && !strcmp(port4, "ephy"))
		gsw->port4 = PORT4_EPHY;
	else if (port4 && !strcmp(port4, "gmac"))
		gsw->port4 = PORT4_EXT;
	else
		gsw->port4 = PORT4_EPHY;

	gsw->irq = platform_get_irq(pdev, 0);
	/* do not swallow -EPROBE_DEFER: mapping it to 0 made the probe
	 * succeed permanently without an interrupt
	 */
	if (gsw->irq == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (gsw->irq < 0)
		gsw->irq = 0;

	platform_set_drvdata(pdev, gsw);

	return 0;
}

static int mt7620_gsw_remove(struct platform_device *pdev)
{
	struct mt7620_gsw *gsw;
	struct fe_priv *fe;

	mutex_lock(&gsw_teardown_lock);
	gsw = platform_get_drvdata(pdev);

	if (gsw) {
		fe = gsw->priv;

		/* The FE driver caches this devm object in soc->swpriv, and
		 * gsw_interrupt_mt7620() resolves it through that pointer.
		 * Mask the source and unregister the action BEFORE dropping
		 * swpriv, otherwise an interrupt arriving in that window
		 * dereferences NULL. This mirrors mtk_gsw_cleanup().
		 */
		if (gsw->irq)
			mtk_switch_w32(gsw, ~0, GSW_REG_IMR);
		if (gsw->irq && fe)
			free_irq(gsw->irq, fe);

		/* mt7620_gsw_config() handed gsw->base to mt7530_probe(),
		 * and that switch_dev is still on the swconfig global list.
		 * It is devm-owned by this platform device: as soon as
		 * remove() returns the mapping is released while swconfig
		 * can still call mt7530_read_reg() on it. Unregister (and
		 * thereby drain in-flight swconfig calls) while the
		 * mapping is still valid. fe is NULL when the FE side has
		 * already torn everything down.
		 */
		if (fe)
			fe_unregister_switches(fe);

		if (fe)
			fe->soc->swpriv = NULL;
		/* Clear the back-pointer unconditionally: when there is no
		 * irq, mtk_gsw_cleanup() early-returns on the now-NULL
		 * swpriv and cannot do it for us.
		 */
		gsw->priv = NULL;
		platform_set_drvdata(pdev, NULL);
	}
	mutex_unlock(&gsw_teardown_lock);

	return 0;
}

static struct platform_driver gsw_driver = {
	.probe = mt7620_gsw_probe,
	.remove = mt7620_gsw_remove,
	.driver = {
		.name = "mt7620-gsw",
		.owner = THIS_MODULE,
		.of_match_table = mediatek_gsw_match,
	},
};

module_platform_driver(gsw_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("GBit switch driver for Mediatek MT7620 SoC");
MODULE_VERSION(MTK_FE_DRV_VERSION);
