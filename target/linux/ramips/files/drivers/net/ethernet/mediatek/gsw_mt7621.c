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
#include <linux/delay.h>

#include <ralink_regs.h>

#include "mtk_eth_soc.h"
#include "gsw_mt7620.h"

/* set while one disable_irq_nosync() is outstanding */
#define GSW_IRQ_DISABLED	0
/* set once teardown has taken the irq action down */
#define GSW_IRQ_TORNDOWN	1

/* Serialise the teardown decision in gsw_irq_reenable() against
 * mt7621_gsw_stop_irq()'s set_bit(GSW_IRQ_TORNDOWN) + free_irq().
 * A bare test_bit()/enable_irq() pair has a check-use race: on 4.4
 * free_irq() only calls synchronize_irq(), which waits for in-flight
 * hardirq handlers and not for irq_work. A work item that already
 * sampled GSW_IRQ_TORNDOWN as clear can therefore reach enable_irq()
 * after free_irq() returned, while the teardown sits in flush_work()
 * waiting for that very work item. Both sides hold this lock across
 * the decision and the action, so enable_irq() can never land after
 * free_irq(). Teardown drops the lock before flush_work(), and the
 * hardirq handler takes no lock, so the wait-for graph stays acyclic.
 * Module-wide, like gsw_teardown_lock: a queued work item can still
 * reach this lock after the devm gsw object has been torn down.
 */
static DEFINE_MUTEX(gsw_irq_reenable_lock);

/* The handler's disable_irq_nosync() needs this matching enable_irq()
 * during normal operation. During teardown free_irq() has already
 * called irq_shutdown(), which sets desc->depth = 1 and masks the
 * line; the next request_irq() calls irq_startup(), which resets the
 * depth to 0 and enables it. A work item flushed after free_irq()
 * must not undo that shutdown by enabling a line with no action.
 */
static void gsw_irq_reenable(struct mt7620_gsw *gsw)
{
	mutex_lock(&gsw_irq_reenable_lock);
	if (!test_bit(GSW_IRQ_TORNDOWN, &gsw->irq_state))
		enable_irq(gsw->irq);
	mutex_unlock(&gsw_irq_reenable_lock);
}

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
 * two independent paths that both own the same irq, irq_work and the
 * soc->swpriv / gsw->priv pair. Without a common lock both can snapshot a
 * non-NULL fe_priv, call free_irq() twice or dereference an fe_priv already
 * released by unregister_netdev(). The lock is module-wide, so it stays
 * valid after the devm gsw object is gone. Define it before mtk_gsw_init()
 * so the init path can take it too.
 */
static DEFINE_MUTEX(gsw_teardown_lock);

/* also used by the mt7621_gsw_set_imr() failure rollback in mtk_gsw_init() */
static void mt7621_gsw_stop_irq(struct mt7620_gsw *gsw,
				struct fe_priv *owner);

/* The MT7530 indirect register window is shared with swconfig and with the
 * deferred link work: both build a page-select plus two data accesses while
 * holding bus->mdio_lock. mt7530_mdio_w32() only serialises the PIAC
 * register itself, so an IMR write done without the bus lock can be
 * interleaved with such a sequence and end up in the wrong switch register.
 */
static int mt7621_gsw_set_imr(struct mt7620_gsw *gsw, struct fe_priv *fe,
			      u32 mask)
{
	struct mii_bus *bus;
	int ret;

	if (!fe) {
		/* No FE instance to resolve the bus from: the other
		 * teardown path has already masked the sources and there
		 * is no swconfig user left. Do not issue an indirect
		 * write that cannot take bus->mdio_lock.
		 */
		return 0;
	}

	bus = fe->mii_bus;
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
	int ack_err;
	int retry;
	bool status_read_ok;

	/* swconfig reaches the same MT7530 indirect registers through
	 * mt7530.c, which holds bus->mdio_lock across the page-select +
	 * two data accesses. This work builds those three-step transactions
	 * via mt7530_mdio_r32/w32() under gsw->mdio_lock only. Without the
	 * bus lock the two three-step sequences can interleave: one sets
	 * the page, the other overwrites it, and the first then reads or
	 * writes the wrong register. bus is NULL only during early hw init,
	 * before fe_mdio_init() creates it and before switch_dev is
	 * registered, when no swconfig user exists yet. Teardown keeps
	 * gsw->priv alive until this work has drained (see
	 * mt7621_gsw_stop_irq()), so the bus lock is taken there too.
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
	status_read_ok = !mt7530_mdio_r32(gsw, 0x700c, &reg);
	if (!status_read_ok)
		/* The read failed: clear every source so none stays
		 * asserted. */
		reg = 0xffffffff;
	ack_err = mt7530_mdio_w32(gsw, 0x700c, reg);
	if (ack_err)
		goto out_masked;

	if (!status_read_ok || !priv)
		goto out_reenable;

	/* switch_init() installs the handler and unmasks the sources
	 * before fe_init() reaches fe_mdio_init(), so priv->phy can still
	 * be NULL on an early work item. link[] is only ever written under
	 * phy->lock (link adjust, port_init, the MT7620 interrupt path);
	 * with no lock object there is no serialised way to update it.
	 * Skip the link walk in that window but still refresh the carrier
	 * and acknowledge/re-enable the line below.
	 */
	if (phy) {
		/* Each mt7530_mdio_r32() issues three PIAC transactions and
		 * each transaction can busy-wait up to 2000 iterations.
		 * Collect all port states before taking phy->lock:
		 * spin_lock_irqsave() disables local interrupts, so doing
		 * up to five of these MDIO reads inside the critical
		 * section can stall a UP target for tens of ms.
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

		/* Keep link[] updates atomic with the PHY-side state
		 * machine, like the MT7620 interrupt path does.
		 */
		spin_lock_irqsave(&phy->lock, flags);
		for (i = 0; i < 5; i++) {
			if (!link_valid[i] || link[i] == priv->link[i])
				continue;

			priv->link[i] = link[i];
			netdev_info(priv->netdev, "port %d link %s\n", i,
				    link[i] ? "up" : "down");
		}
		spin_unlock_irqrestore(&phy->lock, flags);
	}

	mt7620_handle_carrier(priv);
	goto out_reenable;

out_masked:
	/* The W1C write failed, so the level-high source is still
	 * asserted. Retry the acknowledge once (still under
	 * bus->mdio_lock) and clear every source so no event that arrived
	 * in the meantime can keep the line asserted; on success follow
	 * the normal re-enable path. If that also fails, the backoff loop
	 * below gives the PIAC more time instead of leaving the line
	 * disabled until teardown compensates the depth.
	 */
	if (!mt7530_mdio_w32(gsw, 0x700c, 0xffffffff))
		goto out_reenable;

	/* Both acknowledges failed. The handler's disable_irq_nosync()
	 * still holds the line disabled, so no new handler can run while
	 * we back off: retrying here cannot spin the interrupt/work pair.
	 * Release the bus lock across each sleep so a transient PIAC
	 * stall can clear and other MDIO users are not blocked; the
	 * teardown path drains this work before freeing the bus.
	 */
	for (retry = 5; retry > 0; retry--) {
		if (bus)
			mutex_unlock(&bus->mdio_lock);
		msleep(20);
		if (bus)
			mutex_lock(&bus->mdio_lock);
		if (!mt7530_mdio_w32(gsw, 0x700c, 0xffffffff))
			goto out_reenable;
	}

	/* Every acknowledge failed. Mask every source so that
	 * (ISR & IMR) == 0 and the level-high line deasserts; only then
	 * is enable_irq() safe. Drop the guard too, so
	 * mt7621_gsw_stop_irq() does not enable a second time. A later
	 * mtk_gsw_init() re-unmasks the intended sources: link reporting
	 * stays degraded until then, but the disable depth and guard no
	 * longer leak for the whole lifetime of this instance.
	 */
	/* 0x7008 here is the MT7530 switch's own interrupt register,
	 * reached through the indirect MDIO window. Do not spell it as
	 * GSW_REG_IMR: that symbol names the MT7620 SoC's memory-mapped
	 * GSW register, whose bit polarity is the opposite of this one.
	 * Keep the numeric form used by every other MT7530 access in
	 * this file and by mt7621_gsw_set_imr().
	 */
	if (!mt7530_mdio_w32(gsw, 0x7008, 0)) {
		clear_bit(GSW_IRQ_DISABLED, &gsw->irq_state);
		if (bus)
			mutex_unlock(&bus->mdio_lock);
		gsw_irq_reenable(gsw);
		return;
	}

	/* Even the mask write failed: keep the guard set and leave the
	 * line disabled. Teardown releases the action and the kernel's
	 * irq_shutdown() owns the disable depth from here on.
	 */
	dev_err(gsw->dev, "failed to clear switch IRQ status: %d\n", ack_err);
	if (bus)
		mutex_unlock(&bus->mdio_lock);
	return;

out_reenable:
	/* Clear the guard before the line becomes live again: a handler
	 * entering right after enable_irq() must be able to start a new
	 * disable/work cycle. Exactly one enable balances the one
	 * disable_irq_nosync() done by the handler -- unless teardown has
	 * already freed the action, in which case gsw_irq_reenable()
	 * leaves the kernel's shutdown state alone.
	 */
	clear_bit(GSW_IRQ_DISABLED, &gsw->irq_state);
	if (bus)
		mutex_unlock(&bus->mdio_lock);
	gsw_irq_reenable(gsw);
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

static int mt7621_hw_init(struct mt7620_gsw *gsw, struct device_node *np)
{
	u32 i;
	u32 val;
	int err;

	/* Every transaction below goes through mt7620_mii_busy_wait(),
	 * which records a timeout in mdio_err while holding mdio_lock.
	 * Clear and sample the field under that same lock so no runtime
	 * MDIO user can race with the init sequence.
	 */
	mutex_lock(&gsw->mdio_lock);
	gsw->mdio_err = 0;
	mutex_unlock(&gsw->mdio_lock);

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

	mutex_lock(&gsw->mdio_lock);
	err = gsw->mdio_err;
	mutex_unlock(&gsw->mdio_lock);

	return err;
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
	int err = 0;

	if (!pdev)
		return -ENODEV;

	if (!of_device_is_compatible(np, mediatek_gsw_match->compatible)) {
		put_device(&pdev->dev);
		return -EINVAL;
	}

	/* Both teardown paths (mtk_gsw_cleanup() and mt7621_gsw_remove())
	 * read gsw->priv, flush irq_work and free_irq() while holding
	 * gsw_teardown_lock. Publishing swpriv/gsw->priv, installing the
	 * action and resetting irq_state must use the same lock: an
	 * unbind racing with ndo_init() could otherwise free an action
	 * that was never registered, or let init install a handler after
	 * remove() dropped both back-pointers, leaving the work item and
	 * the hardirq handler to dereference a freed fe_priv.
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

	if (gsw->irq) {
		/* The chip's power-on IMR is not guaranteed to mask every
		 * source. An IRQ arriving between request_irq() and the
		 * disable_irq() below makes the handler take one
		 * disable_irq_nosync() and queue irq_work. If that work
		 * ends in out_masked it deliberately leaves the guard set
		 * and cannot re-enable the line, and mt7621_gsw_stop_irq()
		 * only compensates at teardown. The successful init path
		 * would then keep a net disable depth of one for the whole
		 * lifetime and lose every link change. Mask all sources
		 * before the action becomes live, as the MT7620 init path
		 * already does with GSW_REG_IMR. Retry a PIAC timeout.
		 */
		if (mt7621_gsw_set_imr(gsw, priv, 0))
			mt7621_gsw_set_imr(gsw, priv, 0);

		/* a previous instance has been drained by
		 * mt7621_gsw_stop_irq(); start with a clean guard and let
		 * irq_work enable the line again. request_irq() calls
		 * irq_startup(), which resets the irq_desc disable depth
		 * that the previous teardown left behind. The work item
		 * itself is initialised once by mt7621_gsw_probe():
		 * ndo_init() can run again after an unregister/re-register
		 * while a work item is still queued or running, and
		 * INIT_WORK() on a live work_struct corrupts its list node.
		 */
		clear_bit(GSW_IRQ_DISABLED, &gsw->irq_state);
		/* Clear the teardown flag under the same lock that
		 * gsw_irq_reenable() reads it with, so every write to
		 * GSW_IRQ_TORNDOWN is ordered against every decision
		 * to enable the line.
		 */
		mutex_lock(&gsw_irq_reenable_lock);
		clear_bit(GSW_IRQ_TORNDOWN, &gsw->irq_state);
		mutex_unlock(&gsw_irq_reenable_lock);
		err = request_irq(gsw->irq, gsw_interrupt_mt7621, 0,
				  "gsw", priv);

		if (err) {
			dev_err(gsw->dev, "request_irq failed: %d\n", err);
			/* do not leave swpriv pointing at a switch the
			 * caller now considers uninitialised
			 */
			priv->soc->swpriv = NULL;
			gsw->priv = NULL;
			goto out_unlock;
		}
		disable_irq(gsw->irq);

		/* If both mask writes above timed out, an already
		 * asserted level line can enter the handler between
		 * request_irq() and disable_irq(). That handler takes
		 * one disable_irq_nosync() and queues irq_work. Drain
		 * it before hw_init: otherwise a PIAC timeout in the
		 * work pollutes gsw->mdio_err and makes a successful
		 * hardware init look like a failure. Also balance the
		 * extra disable depth: a work ending in out_masked
		 * keeps GSW_IRQ_DISABLED set and never re-enables the
		 * line. The action is live here, so an enable_irq()
		 * below is the correct balance; teardown must not add
		 * one after free_irq(). The work takes neither
		 * gsw_teardown_lock nor any lock held here, so
		 * flushing cannot deadlock.
		 */
		flush_work(&gsw->irq_work);
		if (test_and_clear_bit(GSW_IRQ_DISABLED,
				       &gsw->irq_state))
			enable_irq(gsw->irq);
	}

	err = mt7621_hw_init(gsw, np);
	if (err) {
		/* A single PIAC busy-wait timeout is often transient; the
		 * reset and register sequence is idempotent, so try it
		 * once more before declaring the switch unusable.
		 */
		dev_err(gsw->dev, "switch hardware init failed: %d, retrying\n",
			err);
		err = mt7621_hw_init(gsw, np);
	}
	if (err) {
		dev_err(gsw->dev, "switch hardware init failed: %d\n", err);
		goto err_late;
	}

	if (gsw->irq) {
		int irqerr;

		/* only now unmask the switch interrupt sources, when the
		 * handler is actually registered
		 */
		irqerr = mt7621_gsw_set_imr(gsw, priv, 0x1f);
		if (irqerr) {
			/* The three-step MDIO write failed, so no
			 * link-change event would ever reach the handler.
			 */
			err = irqerr;
			goto err_late;
		}
		enable_irq(gsw->irq);
	}
	goto out_unlock;

err_late:
	/* Both failures happen after request_irq()/disable_irq() and
	 * after fe_init() has no rollback of its own. Mask the sources,
	 * then enable_irq() once to balance the disable_irq() done by
	 * this init attempt while the action is still live. Afterwards
	 * reuse the helper shared by the teardown paths: it frees the
	 * action, drains an irq_work that may have been queued between
	 * request_irq() and disable_irq(), and clears gsw->priv. A plain
	 * free_irq() would leave that work running against a NULL
	 * back-pointer, and the work could enable a released action if
	 * it were not suppressed by GSW_IRQ_TORNDOWN.
	 */
	if (gsw->irq) {
		if (mt7621_gsw_set_imr(gsw, priv, 0))
			mt7621_gsw_set_imr(gsw, priv, 0);
		enable_irq(gsw->irq);
		mt7621_gsw_stop_irq(gsw, priv);
	} else {
		gsw->priv = NULL;
	}
	priv->soc->swpriv = NULL;

out_unlock:
	/* Only a successful init may re-allow switch registration. Do it
	 * while still holding gsw_teardown_lock: this makes the flag
	 * clear atomic with the swpriv publication above, so a GSW
	 * unbind cannot publish switch_dying in between and then have
	 * that claim erased by fe_init().
	 */
	if (!err) {
		/* A fresh swpriv was published above. Let the MDIO and
		 * carrier readers in again after a previous unbind raised
		 * the gate. Both actions are under gsw_teardown_lock, so
		 * they are atomic with respect to remove().
		 */
		fe_gsw_readers_open();
		fe_switch_mark_alive(priv);
	}
	mutex_unlock(&gsw_teardown_lock);
	return err;
}
EXPORT_SYMBOL_GPL(mtk_gsw_init);

/* Both the FE teardown and this driver's remove() may be the first to
 * release the interrupt, so make this helper idempotent. The hardirq
 * handler calls disable_irq_nosync() and queues irq_work, whose last
 * step performs the matching enable_irq() while the action is live.
 * Teardown must not add an extra disable_irq() of its own, and must
 * not try to "balance" that depth after free_irq(): on 4.4
 * __free_irq() calls irq_shutdown(), which sets desc->depth = 1 and
 * masks the line, and the next request_irq() calls irq_startup(),
 * which resets depth to 0 and enables it. Calling enable_irq() after
 * free_irq() would instead undo that shutdown on a line with no
 * action (see gsw_irq_reenable()).
 */
static void mt7621_gsw_stop_irq(struct mt7620_gsw *gsw,
				struct fe_priv *owner)
{
	if (!gsw->irq)
		return;

	/* Only the first teardown path owns the irq action. The action was
	 * requested with the fe_priv as dev_id, so free_irq() must receive
	 * that exact pointer. gsw->priv is not a reliable source here: the
	 * GSW remove() path clears the back-pointer before the FE cleanup
	 * runs. Reading it back would skip free_irq() entirely, leaking the
	 * action with a handler still holding the old fe_priv; a later
	 * fe_init() would then fail to request the same irq and the switch
	 * would never come back. Callers pass the owner they are tearing
	 * down, or NULL when the other teardown path already released it.
	 */
	/* Publish the teardown state and release the action under
	 * gsw_irq_reenable_lock. Holding the lock across free_irq() is
	 * what closes the race: a work item can only run its
	 * test_bit()/enable_irq() pair while it owns the same lock, so
	 * either it finishes first (action still live, the enable is a
	 * valid balance) or it sees GSW_IRQ_TORNDOWN afterwards and
	 * skips the enable. free_irq()'s synchronize_irq() waits only
	 * for hardirq handlers, which take no lock, so this cannot
	 * deadlock. Release the lock before flush_work(): a work item
	 * waiting on this lock while teardown waits on that work would
	 * otherwise be a cycle.
	 */
	mutex_lock(&gsw_irq_reenable_lock);
	set_bit(GSW_IRQ_TORNDOWN, &gsw->irq_state);
	if (owner)
		free_irq(gsw->irq, owner);
	mutex_unlock(&gsw_irq_reenable_lock);

	/* Keep gsw->priv alive until the work has drained. The work
	 * resolves fe->mii_bus through that pointer in order to take
	 * bus->mdio_lock around its three-step MDIO transactions.
	 * Clearing it first made a queued work skip the bus lock while
	 * the switch_dev was still registered and swconfig could
	 * interleave its own page select and data accesses.
	 *
	 * Every caller must flush, including the second teardown path
	 * where fe is already NULL: returning early there would leave
	 * an in-flight work touching this devm object after remove().
	 * The work may still issue its W1C status write, which must
	 * complete before the PIAC window goes away; GSW_IRQ_TORNDOWN
	 * makes it skip the enable_irq() that would otherwise undo the
	 * shutdown just performed by free_irq().
	 */
	flush_work(&gsw->irq_work);

	/* gsw_irq_work_mt7621() normally clears GSW_IRQ_DISABLED and
	 * enables the line before returning; gsw_irq_reenable() now
	 * suppresses that enable once GSW_IRQ_TORNDOWN is set, so a work
	 * item drained by the flush above cannot unmask a line whose
	 * action is already gone. Its out_masked path deliberately keeps
	 * the guard set when re-enabling a still asserted level-high
	 * source would spin the irq/work pair.
	 *
	 * No depth compensation belongs here. irq_shutdown() already
	 * discarded the handler's outstanding disable_irq_nosync() by
	 * setting desc->depth = 1, and the next request_irq() calls
	 * irq_startup() to reset it to 0. An enable_irq() at this point
	 * would take __enable_irq()'s "case 1" path: irq_enable() clears
	 * IRQD_IRQ_DISABLED and unmasks the line while desc->action is
	 * NULL, so a still asserted level-high source re-enters the
	 * handler with no action and genirq marks it spurious.
	 * free_irq() has synchronised all handlers and flush_work() has
	 * drained the work, so the guard is stable; just clear it.
	 */
	clear_bit(GSW_IRQ_DISABLED, &gsw->irq_state);

	gsw->priv = NULL;
}

void mtk_gsw_cleanup(struct fe_priv *priv)
{
	struct mt7620_gsw *gsw;

	mutex_lock(&gsw_teardown_lock);
	gsw = (struct mt7620_gsw *)priv->soc->swpriv;
	if (!gsw) {
		mutex_unlock(&gsw_teardown_lock);
		return;
	}

	/* mask the port-change source before the handler goes away: the
	 * IMR was opened in mtk_gsw_init(), and the MT7530 keeps a level
	 * line asserted while (ISR & IMR) != 0. Pass the priv being torn
	 * down, not gsw->priv: the gsw remove() path may have cleared the
	 * back-pointer already, and fe_mdio_cleanup() has not run yet on
	 * this path, so priv->mii_bus is still valid for the bus lock.
	 */
	/* A PIAC timeout is transient; retry once so a still-asserted
	 * level-high source is not left enabled after free_irq() below.
	 */
	if (gsw->irq && mt7621_gsw_set_imr(gsw, priv, 0))
		mt7621_gsw_set_imr(gsw, priv, 0);

	/* the irq was requested with the fe_priv as dev_id; release it
	 * on the fe side too, otherwise the handler may run after the
	 * netdev/fe_priv is gone (UAF), and a retried fe_init would
	 * request the same irq twice. gsw->priv may already have been
	 * cleared by the remove() path, so pass priv explicitly.
	 */
	mt7621_gsw_stop_irq(gsw, priv);

	/* The FE netdev and its fe_priv are freed shortly after
	 * ndo_uninit(). Clear the back-pointer unconditionally, also when
	 * mt7621_gsw_stop_irq() returned early because there is no irq,
	 * or a later gsw remove() would dereference freed fe_priv.
	 */
	gsw->priv = NULL;
	priv->soc->swpriv = NULL;
	mutex_unlock(&gsw_teardown_lock);
}
EXPORT_SYMBOL_GPL(mtk_gsw_cleanup);

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
	mutex_init(&gsw->mdio_lock);
	/* Initialise the work item once, while the devm object is fresh.
	 * Teardown must be able to flush_work() even when FE ndo_init()
	 * never got as far as request_irq().
	 */
	INIT_WORK(&gsw->irq_work, gsw_irq_work_mt7621);
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
	struct mt7620_gsw *gsw;
	struct fe_priv *fe;

	mutex_lock(&gsw_teardown_lock);
	gsw = platform_get_drvdata(pdev);

	if (gsw) {
		fe = gsw->priv;

		/* Unregister the switch_devs before touching the irq or
		 * any devm state. fe_unregister_switches() raises
		 * switch_dying in the same critical section that claims
		 * gsw_dev[], so a concurrent fe_init()/switch_config()
		 * (GSW unbind does not hold RTNL) is rejected with
		 * -ENODEV instead of publishing a switch_dev that this
		 * remove would immediately claim and destroy. Doing this
		 * after stop_irq() left a window where swpriv, mii_bus
		 * and phy_map[0x1f] were all still valid but switch_dying
		 * was still false, so fe_init() could succeed and then
		 * silently lose every switch operation. The bus and the
		 * devm mapping are both alive here, which is exactly what
		 * unregister_switch() needs to drain in-flight swconfig
		 * calls safely.
		 */
		if (fe)
			fe_unregister_switches(fe);

		/* The FE driver caches this devm object in soc->swpriv, and
		 * gsw_interrupt_mt7621() resolves it through that pointer
		 * before queueing &gsw->irq_work. Mask the source and drain
		 * both the action and the work BEFORE dropping swpriv,
		 * otherwise an interrupt in that window queues a work item
		 * from a NULL gsw.
		 */
		if (gsw->irq && mt7621_gsw_set_imr(gsw, fe, 0))
			mt7621_gsw_set_imr(gsw, fe, 0);
		/* gsw is devm allocated: drain the deferred link work
		 * before returning, and keep the irq disable depth
		 * balanced (see mt7621_gsw_stop_irq()). fe is the same
		 * pointer the irq action was requested with, and may be
		 * NULL when the FE cleanup path already released it; in
		 * that case only the work drain / disable-depth balance
		 * is needed here.
		 */
		mt7621_gsw_stop_irq(gsw, fe);

		/* Reject new readers before dropping swpriv. Drain below
		 * only waits for readers counted so far; without this gate
		 * a reader starting after the wait returns would depend
		 * solely on seeing the cleared pointer.
		 */
		fe_gsw_readers_close();

		if (fe)
			fe->soc->swpriv = NULL;
		gsw->priv = NULL;
		platform_set_drvdata(pdev, NULL);
	}
	/* swpriv was cleared and the closing gate raised under the
	 * teardown lock. Drain the readers that already snapshotted the
	 * old pointer BEFORE releasing that lock. No open() may slip
	 * between close() and this wait: it would reopen the gate for
	 * readers the drain does not cover, breaking the "count reached
	 * zero means no reader can still enter" rule. devres frees gsw
	 * and unmaps gsw->base as soon as remove() returns. No reader
	 * ever takes gsw_teardown_lock, so waiting while holding it
	 * cannot form a cycle.
	 *
	 * Only the branch that actually raised the gate has readers to
	 * drain. fe_gsw_readers_close() is inside if (gsw); when drvdata
	 * was NULL (probe never published it, or a second remove) the
	 * gate was never raised and new readers can keep arriving. An
	 * unconditional wait_event() in that case blocks every
	 * mtk_gsw_init(), mtk_gsw_cleanup() and remove() on
	 * gsw_teardown_lock with no bound and nothing to make the reader
	 * count converge. Match the condition of the close() call.
	 *
	 * The gate is only a drain barrier, not a permanent "GSW absent"
	 * flag. close() exists solely in remove(), while open() only
	 * existed at the end of a successful mtk_gsw_init(). A GSW unbind
	 * leaves the FE netdev alive, and a later rebind runs probe()
	 * only, never ndo_init(), so mtk_gsw_init() is not reached again
	 * and fe_gsw_closing stayed set forever. Every reader then failed
	 * at fe_gsw_reader_enter(): MDIO transfers returned -ENODEV, the
	 * carrier helper always claimed link up, mdio_reset did nothing
	 * and switch config/port reset bailed out. mtk_gsw_cleanup() also
	 * never opens the gate. Reopen it here, under the same lock, once
	 * the drain proves no reader holds the old object. swpriv and
	 * drvdata are already NULL and every reader checks swpriv for
	 * NULL, so a reader admitted afterwards returns safely instead of
	 * dereferencing the devm object devres is about to free. The
	 * mtk_gsw_init() open() remains as an idempotent confirmation.
	 */
	if (gsw)
		fe_gsw_drain_readers();
	fe_gsw_readers_open();
	mutex_unlock(&gsw_teardown_lock);

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
