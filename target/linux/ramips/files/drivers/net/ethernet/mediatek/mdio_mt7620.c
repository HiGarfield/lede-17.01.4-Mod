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

#include "mtk_eth_soc.h"
#include "gsw_mt7620.h"
#include "mdio.h"

void mt7620_mdio_reset(struct fe_priv *priv)
{
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;

	if (!gsw)
		return;

	/* clear the busy/start bit so a wedged engine accepts new
	 * commands again
	 */
	mtk_switch_w32(gsw,
		       mtk_switch_r32(gsw, MT7620A_GSW_REG_PIAC) &
		       ~GSW_MDIO_ACCESS,
		       MT7620A_GSW_REG_PIAC);
	udelay(10);
}

/* The PIAC busy-wait runs inside spin_lock_irqsave(&gsw->mdio_lock),
 * i.e. with local interrupts disabled. jiffies only advances from the
 * timer interrupt, so on the UP targets (every ramips subtarget except
 * mt7621) a jiffies based timeout can never expire: a wedged engine
 * would spin forever with interrupts off. Use a bounded retry count
 * that does not depend on the tick instead.
 */
#define GSW_MDIO_BUSY_RETRIES	2000

static int mt7620_mii_busy_wait(struct mt7620_gsw *gsw)
{
	unsigned int i;

	for (i = 0; i < GSW_MDIO_BUSY_RETRIES; i++) {
		if (!(mtk_switch_r32(gsw, MT7620A_GSW_REG_PIAC) & GSW_MDIO_ACCESS))
			return 0;
		udelay(1);
	}

	dev_err(gsw->dev, "mdio: MDIO timeout\n");
	/* without this recovery the busy bit would stay set and every
	 * later transfer would time out as well
	 */
	mtk_switch_w32(gsw,
		       mtk_switch_r32(gsw, MT7620A_GSW_REG_PIAC) &
		       ~GSW_MDIO_ACCESS,
		       MT7620A_GSW_REG_PIAC);
	udelay(10);
	return -ETIMEDOUT;
}

/* unlocked variants: the PIAC is a single command/status register,
 * so a complete transaction (wait - issue - wait - collect) must be
 * atomic wrt other users. Direct callers (swconfig, the MT7621 irq
 * handler) bypass the mdiobus mutex, hence this dedicated lock.
 */
static int __mt7620_mii_write(struct mt7620_gsw *gsw, u32 phy_addr,
			      u32 phy_register, u32 write_data)
{
	if (mt7620_mii_busy_wait(gsw))
		return -ETIMEDOUT;

	write_data &= 0xffff;

	mtk_switch_w32(gsw, GSW_MDIO_ACCESS | GSW_MDIO_START | GSW_MDIO_WRITE |
		(phy_register << GSW_MDIO_REG_SHIFT) |
		(phy_addr << GSW_MDIO_ADDR_SHIFT) | write_data,
		MT7620A_GSW_REG_PIAC);

	if (mt7620_mii_busy_wait(gsw))
		return -ETIMEDOUT;

	return 0;
}

int _mt7620_mii_write(struct mt7620_gsw *gsw, u32 phy_addr,
		      u32 phy_register, u32 write_data)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&gsw->mdio_lock, flags);
	ret = __mt7620_mii_write(gsw, phy_addr, phy_register, write_data);
	spin_unlock_irqrestore(&gsw->mdio_lock, flags);

	return ret;
}

static int __mt7620_mii_read(struct mt7620_gsw *gsw, int phy_addr,
			     int phy_reg, u16 *val)
{
	if (mt7620_mii_busy_wait(gsw))
		return -ETIMEDOUT;

	mtk_switch_w32(gsw, GSW_MDIO_ACCESS | GSW_MDIO_START | GSW_MDIO_READ |
		(phy_reg << GSW_MDIO_REG_SHIFT) |
		(phy_addr << GSW_MDIO_ADDR_SHIFT),
		MT7620A_GSW_REG_PIAC);

	if (mt7620_mii_busy_wait(gsw))
		return -ETIMEDOUT;

	*val = mtk_switch_r32(gsw, MT7620A_GSW_REG_PIAC) & 0xffff;

	return 0;
}

int _mt7620_mii_read(struct mt7620_gsw *gsw, int phy_addr, int phy_reg)
{
	unsigned long flags;
	u16 d;

	spin_lock_irqsave(&gsw->mdio_lock, flags);
	if (__mt7620_mii_read(gsw, phy_addr, phy_reg, &d))
		d = 0xffff;
	spin_unlock_irqrestore(&gsw->mdio_lock, flags);

	return d;
}

int mt7620_mii_read_reg(struct mt7620_gsw *gsw, int phy_addr, int phy_reg,
			u16 *val)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&gsw->mdio_lock, flags);
	ret = __mt7620_mii_read(gsw, phy_addr, phy_reg, val);
	spin_unlock_irqrestore(&gsw->mdio_lock, flags);

	return ret;
}

int mt7620_mdio_write(struct mii_bus *bus, int phy_addr, int phy_reg, u16 val)
{
	struct fe_priv *priv = bus->priv;
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;

	return _mt7620_mii_write(gsw, phy_addr, phy_reg, val);
}

int mt7620_mdio_read(struct mii_bus *bus, int phy_addr, int phy_reg)
{
	struct fe_priv *priv = bus->priv;
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;

	return _mt7620_mii_read(gsw, phy_addr, phy_reg);
}

int mt7530_mdio_w32(struct mt7620_gsw *gsw, u32 reg, u32 val)
{
	unsigned long flags;
	int ret;

	/* the indirect-address write and the two data writes are one
	 * logical access: hold the lock across the whole sequence so no
	 * other transaction can clobber the address register in
	 * between. Abort on the first failed transaction.
	 */
	spin_lock_irqsave(&gsw->mdio_lock, flags);
	ret = __mt7620_mii_write(gsw, 0x1f, 0x1f, (reg >> 6) & 0x3ff);
	if (!ret)
		ret = __mt7620_mii_write(gsw, 0x1f, (reg >> 2) & 0xf,
					 val & 0xffff);
	if (!ret)
		ret = __mt7620_mii_write(gsw, 0x1f, 0x10, val >> 16);
	spin_unlock_irqrestore(&gsw->mdio_lock, flags);

	return ret;
}

int mt7530_mdio_r32(struct mt7620_gsw *gsw, u32 reg, u32 *val)
{
	unsigned long flags;
	u16 high, low;
	int ret;

	spin_lock_irqsave(&gsw->mdio_lock, flags);
	ret = __mt7620_mii_write(gsw, 0x1f, 0x1f, (reg >> 6) & 0x3ff);
	if (!ret)
		ret = __mt7620_mii_read(gsw, 0x1f, (reg >> 2) & 0xf, &low);
	if (!ret)
		ret = __mt7620_mii_read(gsw, 0x1f, 0x10, &high);
	spin_unlock_irqrestore(&gsw->mdio_lock, flags);

	if (ret)
		return ret;

	*val = (high << 16) | low;

	return 0;
}

static unsigned char *fe_speed_str(int speed)
{
	switch (speed) {
	case 2:
	case SPEED_1000:
		return "1000";
	case 1:
	case SPEED_100:
		return "100";
	case 0:
	case SPEED_10:
		return "10";
	}

	return "? ";
}

int mt7620_has_carrier(struct fe_priv *priv)
{
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;
	int i;

	/* a DT without a gsw node leaves swpriv NULL; report carrier
	 * rather than dereferencing it
	 */
	if (!gsw)
		return 1;

	for (i = 0; i < GSW_PORT6; i++)
		if (mtk_switch_r32(gsw, GSW_REG_PORT_STATUS(i)) & 0x1)
			return 1;
	return 0;
}


void mt7620_handle_carrier(struct fe_priv *priv)
{
	if (!priv->phy)
		return;

	if (mt7620_has_carrier(priv))
		netif_carrier_on(priv->netdev);
	else
		netif_carrier_off(priv->netdev);
}

void mt7620_print_link_state(struct fe_priv *priv, int port, int link,
			     int speed, int duplex)
{
	if (link)
		netdev_info(priv->netdev, "port %d link up (%sMbps/%s duplex)\n",
			    port, fe_speed_str(speed),
			    (duplex) ? "Full" : "Half");
	else
		netdev_info(priv->netdev, "port %d link down\n", port);
}

void mt7620_mdio_link_adjust(struct fe_priv *priv, int port)
{
	mt7620_print_link_state(priv, port, priv->link[port],
				priv->phy->speed[port],
				(priv->phy->duplex[port] == DUPLEX_FULL));
	mt7620_handle_carrier(priv);
}
