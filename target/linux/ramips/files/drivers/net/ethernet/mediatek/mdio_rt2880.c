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
#include <linux/errno.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>

#include "mtk_eth_soc.h"
#include "mdio_rt2880.h"
#include "mdio.h"

#define FE_MDIO_RETRY	1000

static const char *rt2880_speed_str(struct fe_priv *priv)
{
	switch (priv->phy->speed[0]) {
	case SPEED_1000:
		return "1000";
	case SPEED_100:
		return "100";
	case SPEED_10:
		return "10";
	}

	return "?";
}

void rt2880_mdio_link_adjust(struct fe_priv *priv, int port)
{
	u32 mdio_cfg;

	if (!priv->link[0]) {
		netif_carrier_off(priv->netdev);
		netdev_info(priv->netdev, "link down\n");
		return;
	}

	mdio_cfg = FE_MDIO_CFG_TX_CLK_SKEW_200 |
		   FE_MDIO_CFG_RX_CLK_SKEW_200 |
		   FE_MDIO_CFG_GP1_FRC_EN;

	if (priv->phy->duplex[0] == DUPLEX_FULL)
		mdio_cfg |= FE_MDIO_CFG_GP1_DUPLEX;

	if (priv->phy->tx_fc[0])
		mdio_cfg |= FE_MDIO_CFG_GP1_FC_TX;

	if (priv->phy->rx_fc[0])
		mdio_cfg |= FE_MDIO_CFG_GP1_FC_RX;

	switch (priv->phy->speed[0]) {
	case SPEED_10:
		mdio_cfg |= FE_MDIO_CFG_GP1_SPEED_10;
		break;
	case SPEED_100:
		mdio_cfg |= FE_MDIO_CFG_GP1_SPEED_100;
		break;
	case SPEED_1000:
		mdio_cfg |= FE_MDIO_CFG_GP1_SPEED_1000;
		break;
	default:
		/* link[0] can already be 1 here (fixed link or a PHY
		 * reporting an unknown speed). Returning without touching
		 * the carrier leaves the interface up with the old MAC
		 * speed configuration.
		 */
		WARN_ON(1);
		netif_carrier_off(priv->netdev);
		return;
	}

	fe_w32(mdio_cfg, FE_MDIO_CFG);

	netif_carrier_on(priv->netdev);
	netdev_info(priv->netdev, "link up (%sMbps/%s duplex)\n",
		    rt2880_speed_str(priv),
		    (priv->phy->duplex[0] == DUPLEX_FULL) ? "Full" : "Half");
}

void rt2880_mdio_reset(struct fe_priv *priv)
{
	/* clear the busy/start bit so a wedged controller accepts new
	 * commands again
	 */
	fe_w32(fe_r32(FE_MDIO_ACCESS) & ~BIT(31), FE_MDIO_ACCESS);
	udelay(10);
}

static int rt2880_mdio_wait_ready(struct fe_priv *priv)
{
	int retries;

	retries = FE_MDIO_RETRY;
	while (1) {
		u32 t;

		t = fe_r32(FE_MDIO_ACCESS);
		if ((t & BIT(31)) == 0)
			return 0;

		if (retries-- == 0)
			break;

		udelay(1);
	}

	dev_err(priv->dev, "MDIO operation timed out\n");
	/* without this recovery the busy bit would stay set and every
	 * later transfer would time out as well
	 */
	rt2880_mdio_reset(priv);
	return -ETIMEDOUT;
}

/* FE_MDIO_ACCESS carries 5-bit PHY address (bits 24..28) and register
 * (bits 16..20) fields. Bit 30 selects a write and bit 31 starts the
 * transaction, so an out-of-range operand shifts into those control bits
 * and issues a different command; shifting a large signed int is also
 * undefined behaviour. mdiobus read/write pass their operands straight
 * through, so enforce the hardware field width here.
 */
static inline int rt2880_mdio_check_addr(int phy_addr, int phy_reg)
{
	if ((phy_addr & ~0x1f) || (phy_reg & ~0x1f))
		return -EINVAL;

	return 0;
}

int rt2880_mdio_read(struct mii_bus *bus, int phy_addr, int phy_reg)
{
	struct fe_priv *priv = bus->priv;
	int err;
	u32 t;

	err = rt2880_mdio_check_addr(phy_addr, phy_reg);
	if (err)
		return err;

	err = rt2880_mdio_wait_ready(priv);
	if (err)
		return err;

	t = ((u32)phy_addr << 24) | ((u32)phy_reg << 16);
	fe_w32(t, FE_MDIO_ACCESS);
	t |= BIT(31);
	fe_w32(t, FE_MDIO_ACCESS);

	err = rt2880_mdio_wait_ready(priv);
	if (err)
		return err;

	pr_debug("%s: addr=%04x, reg=%04x, value=%04x\n", __func__,
		 phy_addr, phy_reg, fe_r32(FE_MDIO_ACCESS) & 0xffff);

	return fe_r32(FE_MDIO_ACCESS) & 0xffff;
}

int rt2880_mdio_write(struct mii_bus *bus, int phy_addr, int phy_reg, u16 val)
{
	struct fe_priv *priv = bus->priv;
	int err;
	u32 t;

	err = rt2880_mdio_check_addr(phy_addr, phy_reg);
	if (err)
		return err;

	pr_debug("%s: addr=%04x, reg=%04x, value=%04x\n", __func__,
		 phy_addr, phy_reg, val);

	err = rt2880_mdio_wait_ready(priv);
	if (err)
		return err;

	t = (1u << 30) | ((u32)phy_addr << 24) | ((u32)phy_reg << 16) | val;
	fe_w32(t, FE_MDIO_ACCESS);
	t |= BIT(31);
	fe_w32(t, FE_MDIO_ACCESS);

	return rt2880_mdio_wait_ready(priv);
}

void rt2880_port_init(struct fe_priv *priv, struct device_node *np)
{
	struct device_node *phy_np;
	u32 port_reg = 0;
	const __be32 *link;
	int size;
	int phy_mode;

	/* A short "reg" property must not be dereferenced as a full
	 * __be32; of_property_read_u32() validates the length.
	 */
	if (of_property_read_u32(np, "reg", &port_reg) || port_reg != 0) {
		/* only port 0 is wired up on these SoCs; a DT describing
		 * additional ports (e.g. rt3883's second GMAC) used to be
		 * dropped silently
		 */
		dev_warn(priv->dev,
			 "%s: port id %u not supported by this driver\n",
			 np->name, port_reg);
		return;
	}

	priv->phy->phy_fixed[0] = of_get_property(np,
						  "mediatek,fixed-link", &size);
	if (priv->phy->phy_fixed[0] &&
	    (size != (4 * sizeof(*priv->phy->phy_fixed[0])))) {
		pr_err("%s: invalid fixed link property\n", np->name);
		priv->phy->phy_fixed[0] = NULL;
		return;
	}

	phy_mode = of_get_phy_mode(np);
	switch (phy_mode) {
	case PHY_INTERFACE_MODE_RGMII:
		break;
	case PHY_INTERFACE_MODE_MII:
		break;
	case PHY_INTERFACE_MODE_RMII:
		break;
	default:
		/* this was printing priv->phy->speed[0] (uninitialised or
		 * garbage here) as the port number
		 */
		if (!priv->phy->phy_fixed[0]) {
			dev_err(priv->dev, "port 0 - invalid phy mode\n");
			return;
		}
		/* fixed-link does not use this setting for PHY attach */
		dev_warn(priv->dev,
			 "port 0 - invalid phy mode, using rgmii for fixed link\n");
		break;
	}

	/* phy_ralink is a module-wide singleton: a repeated ndo_init must
	 * not overwrite a slot that still owns an of_parse_phandle()
	 * reference.
	 */
	phy_np = of_parse_phandle(np, "phy-handle", 0);
	of_node_put(priv->phy->phy_node[0]);
	priv->phy->phy_node[0] = phy_np;
	if (!priv->phy->phy_node[0] && !priv->phy->phy_fixed[0])
		return;

	if (priv->phy->phy_fixed[0]) {
		if (priv->phy->phy_node[0]) {
			of_node_put(priv->phy->phy_node[0]);
			priv->phy->phy_node[0] = NULL;
		}
		link = priv->phy->phy_fixed[0];
		priv->phy->speed[0] = be32_to_cpup(link++);
		priv->phy->duplex[0] = be32_to_cpup(link++);
		priv->phy->tx_fc[0] = be32_to_cpup(link++);
		priv->phy->rx_fc[0] = be32_to_cpup(link++);

		priv->link[0] = 1;
		switch (priv->phy->speed[0]) {
		case SPEED_10:
			break;
		case SPEED_100:
			break;
		case SPEED_1000:
			break;
		default:
			dev_err(priv->dev, "invalid link speed: %d\n",
				priv->phy->speed[0]);
			priv->phy->phy_fixed[0] = 0;
			/* undo the early link-up above: the carrier must
			 * not stay up for a port we just rejected
			 */
			priv->link[0] = 0;
			return;
		}
		dev_info(priv->dev, "using fixed link parameters\n");
		rt2880_mdio_link_adjust(priv, 0);
		return;
	}

	/* connect by node: of_phy_connect() finds the PHY on the bus
	 * regardless of its MDIO address, so requiring phy_map[0] here
	 * kept PHYs at non-zero addresses from ever connecting.
	 * fe_connect_phy_node() drops the node itself on failure.
	 */
	if (priv->phy->phy_node[0]) {
		/* fe_connect_phy_node() already drops the DT node on
		 * failure, so ignoring its error leaves the interface
		 * running with no PHY and no diagnostic -- the same
		 * problem 7bada2a1 fixed for mt7620_port_init().
		 */
		int err = fe_connect_phy_node(priv,
					      priv->phy->phy_node[0], 0);

		if (err)
			dev_err(priv->dev, "failed to connect PHY: %d\n", err);
	}
}
