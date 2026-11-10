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
#include <linux/platform_device.h>
#include <linux/if_vlan.h>
#include <linux/of_net.h>
#include <linux/switch.h>

#include <asm/mach-ralink/ralink_regs.h>

#include <mt7620.h>
#include "mtk_eth_soc.h"
#include "gsw_mt7620.h"
#include "mt7530.h"
#include "mdio.h"

#define MT7620A_CDMA_CSG_CFG	0x400
#define MT7620_DMA_VID		(MT7620A_CDMA_CSG_CFG | 0x30)
#define MT7621_CDMP_IG_CTRL	(MT7620A_CDMA_CSG_CFG + 0x00)
#define MT7621_CDMP_EG_CTRL	(MT7620A_CDMA_CSG_CFG + 0x04)
#define MT7620A_RESET_FE	BIT(21)
#define MT7621_RESET_FE		BIT(6)
#define MT7620A_RESET_ESW	BIT(23)
#define MT7620_L4_VALID		BIT(23)
#define MT7621_L4_VALID		BIT(24)

#define MT7620_TX_DMA_UDF	BIT(15)
#define MT7621_TX_DMA_UDF	BIT(19)
#define TX_DMA_FP_BMAP		((0xff) << 19)

#define CDMA_ICS_EN		BIT(2)
#define CDMA_UCS_EN		BIT(1)
#define CDMA_TCS_EN		BIT(0)

#define GDMA_ICS_EN		BIT(22)
#define GDMA_TCS_EN		BIT(21)
#define GDMA_UCS_EN		BIT(20)

/* frame engine counters */
#define MT7620_REG_MIB_OFFSET	0x1000
#define MT7620_PPE_AC_BCNT0	(MT7620_REG_MIB_OFFSET + 0x00)
#define MT7620_GDM1_TX_GBCNT	(MT7620_REG_MIB_OFFSET + 0x300)
#define MT7620_GDM2_TX_GBCNT	(MT7620_GDM1_TX_GBCNT + 0x40)

#define MT7621_REG_MIB_OFFSET	0x2000
#define MT7621_PPE_AC_BCNT0	(MT7621_REG_MIB_OFFSET + 0x00)
#define MT7621_GDM1_TX_GBCNT	(MT7621_REG_MIB_OFFSET + 0x400)
#define MT7621_GDM2_TX_GBCNT	(MT7621_GDM1_TX_GBCNT + 0x40)

#define GSW_REG_GDMA1_MAC_ADRL	0x508
#define GSW_REG_GDMA1_MAC_ADRH	0x50C

#define MT7621_FE_RST_GL	(FE_FE_OFFSET + 0x04)
#define MT7620_FE_INT_STATUS2	(FE_FE_OFFSET + 0x08)

/* FE_INT_STATUS reg on mt7620 define CNT_GDM1_AF at BIT(29)
 * but after test it should be BIT(13).
 */
#define MT7620_FE_GDM1_AF	BIT(13)
#define MT7621_FE_GDM1_AF	BIT(28)
#define MT7621_FE_GDM2_AF	BIT(29)

static const u16 mt7620_reg_table[FE_REG_COUNT] = {
	[FE_REG_PDMA_GLO_CFG] = RT5350_PDMA_GLO_CFG,
	[FE_REG_PDMA_RST_CFG] = RT5350_PDMA_RST_CFG,
	[FE_REG_DLY_INT_CFG] = RT5350_DLY_INT_CFG,
	[FE_REG_TX_BASE_PTR0] = RT5350_TX_BASE_PTR0,
	[FE_REG_TX_MAX_CNT0] = RT5350_TX_MAX_CNT0,
	[FE_REG_TX_CTX_IDX0] = RT5350_TX_CTX_IDX0,
	[FE_REG_TX_DTX_IDX0] = RT5350_TX_DTX_IDX0,
	[FE_REG_RX_BASE_PTR0] = RT5350_RX_BASE_PTR0,
	[FE_REG_RX_MAX_CNT0] = RT5350_RX_MAX_CNT0,
	[FE_REG_RX_CALC_IDX0] = RT5350_RX_CALC_IDX0,
	[FE_REG_RX_DRX_IDX0] = RT5350_RX_DRX_IDX0,
	[FE_REG_FE_INT_ENABLE] = RT5350_FE_INT_ENABLE,
	[FE_REG_FE_INT_STATUS] = RT5350_FE_INT_STATUS,
	[FE_REG_FE_DMA_VID_BASE] = MT7620_DMA_VID,
	[FE_REG_FE_COUNTER_BASE] = MT7620_GDM1_TX_GBCNT,
	[FE_REG_FE_RST_GL] = MT7621_FE_RST_GL,
	[FE_REG_FE_INT_STATUS2] = MT7620_FE_INT_STATUS2,
};

static int mt7620_mt7530_probe(struct fe_priv *priv, void __iomem *base,
			       struct mii_bus *bus, int vlan)
{
	/* Capacity check, probe and publication must be atomic with
	 * respect to the teardown paths claiming gsw_dev[].
	 */
	return fe_probe_and_register_switch(priv, base, bus, vlan);
}

static int mt7620_gsw_config_locked(struct fe_priv *priv)
{
	struct mt7620_gsw *gsw = (struct mt7620_gsw *) priv->soc->swpriv;
	int err, start;

	/* swpriv points at devm-owned state that mtk_gsw_cleanup() and
	 * mt7620_gsw_remove() clear while the FE netdev is still alive.
	 */
	if (!gsw)
		return -ENODEV;

	/* is the mt7530 internal or external */
	if (priv->mii_bus && priv->mii_bus->phy_map[0x1f]) {
		/* Snapshot under the same lock the teardown paths use to
		 * rewrite this counter, so the rollback below undoes only
		 * the entries registered by this call.
		 */
		start = fe_switch_start(priv);
		err = mt7620_mt7530_probe(priv, gsw->base, NULL, 0);
		if (err)
			return err;

		err = mt7620_mt7530_probe(priv, NULL, priv->mii_bus, 1);
		if (err) {
			/* Undo exactly the switches this call registered.
			 * gsw_dev_num is a static SoC-global and can already
			 * be non-zero on a re-probe, so the first probe may
			 * have landed in slot 1. Use the same locked
			 * claim-and-clear helper as the teardown paths.
			 */
			fe_unregister_switches_from(priv, start);
			return err;
		}
	} else {
		err = mt7620_mt7530_probe(priv, gsw->base, NULL, 1);
		if (err)
			return err;
	}

	return 0;
}

static int mt7620_gsw_config(struct fe_priv *priv)
{
	int ret;

	/* Wrap the whole config sequence so gsw->base stays valid even if
	 * the GSW platform device is unbound concurrently. The reader
	 * count takes no lock and cannot invert with the teardown paths.
	 */
	if (!fe_gsw_reader_enter())
		return -ENODEV;
	ret = mt7620_gsw_config_locked(priv);
	fe_gsw_reader_exit();

	return ret;
}

static void mt7620_set_mac(struct fe_priv *priv, unsigned char *mac)
{
	struct mt7620_gsw *gsw;
	unsigned long flags;

	/* ndo_set_mac_address can run long after probe; the gsw platform
	 * device may already have been unbound and cleared swpriv. Hold a
	 * reader reference for the two MMIO writes.
	 */
	if (!fe_gsw_reader_enter())
		return;
	gsw = (struct mt7620_gsw *)priv->soc->swpriv;
	if (!gsw)
		goto out_reader;

	spin_lock_irqsave(&priv->page_lock, flags);
	mtk_switch_w32(gsw, (mac[0] << 8) | mac[1], GSW_REG_SMACCR1);
	mtk_switch_w32(gsw, ((u32)mac[2] << 24) | ((u32)mac[3] << 16) |
		((u32)mac[4] << 8) | mac[5], GSW_REG_SMACCR0);
	spin_unlock_irqrestore(&priv->page_lock, flags);
out_reader:
	fe_gsw_reader_exit();
}

static void mt7620_auto_poll(struct mt7620_gsw *gsw)
{
	int phy;
	int lsb = -1, msb = 0;

	for_each_set_bit(phy, &gsw->autopoll, 32) {
		if (lsb < 0)
			lsb = phy;
		msb = phy;
	}

	if (lsb < 0)
		return;

	mtk_switch_w32(gsw, PHY_AN_EN | PHY_PRE_EN | PMY_MDC_CONF(5) |
		(msb << 8) | lsb, ESW_PHY_POLLING);
}

static void mt7620_port_init_locked(struct fe_priv *priv,
				    struct device_node *np)
{
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;
	struct device_node *phy_np;
	u32 port_reg = 0;
	int phy_mode, size, id;
	int shift = 12;
	u32 mask = 0;
	int min;

	/* swpriv is cleared on gsw unbind/cleanup; do not dereference it
	 * in the declaration block below.
	 */
	if (!gsw) {
		dev_err(priv->dev, "switch device is gone, skipping port init\n");
		return;
	}
	min = (gsw->port4 == PORT4_EPHY) ? (5) : (4);

	/* of_get_property(..., NULL) returns no length, but the old code
	 * unconditionally dereferenced four bytes of the "reg" property.
	 * A short/empty property was an out-of-bounds DT read that could
	 * yield a random port id.
	 */
	if (of_property_read_u32(np, "reg", &port_reg) ||
	    port_reg < (u32)min || port_reg > 5) {
		pr_err("%s: invalid port id %u\n", np->name, port_reg);
		return;
	}

	id = (int)port_reg;

	if (id == 4)
		shift = 14;

	priv->phy->phy_fixed[id] = of_get_property(np, "mediatek,fixed-link",
						   &size);
	if (priv->phy->phy_fixed[id] &&
	    (size != (4 * sizeof(*priv->phy->phy_fixed[id])))) {
		pr_err("%s: invalid fixed link property\n", np->name);
		priv->phy->phy_fixed[id] = NULL;
		return;
	}

	phy_mode = of_get_phy_mode(np);
	switch (phy_mode) {
	case PHY_INTERFACE_MODE_RGMII:
		mask = 0;
		break;
	case PHY_INTERFACE_MODE_MII:
		mask = 1;
		break;
	case PHY_INTERFACE_MODE_RMII:
		mask = 2;
		break;
	default:
		if (!priv->phy->phy_fixed[id]) {
			dev_err(priv->dev, "port %d - invalid phy mode\n", id);
			return;
		}
		/* a fixed link does not need a phy-mode to talk to its
		 * peer; warn and fall back to RGMII, mirroring
		 * rt2880_port_init()
		 */
		dev_warn(priv->dev,
			 "port %d - invalid phy mode, using rgmii for fixed link\n",
			 id);
		mask = 0;
		break;
	}

	/* phy_ralink is a module-wide singleton and ndo_init can run more
	 * than once for a netdev. Drop a reference held by a previous
	 * parse before overwriting the slot, otherwise it leaks (and the
	 * slot keeps pointing at the old node while phy[] describes a
	 * different PHY).
	 */
	phy_np = of_parse_phandle(np, "phy-handle", 0);
	of_node_put(priv->phy->phy_node[id]);
	priv->phy->phy_node[id] = phy_np;
	if (!priv->phy->phy_node[id] && !priv->phy->phy_fixed[id])
		return;

	/* atomic rmw: a manual read-modify-write of CFG1 races the other
	 * drivers touching this shared register
	 */
	rt_sysc_m32(3 << shift, mask << shift, SYSC_REG_CFG1);

	if (priv->phy->phy_fixed[id]) {
		const __be32 *link = priv->phy->phy_fixed[id];
		unsigned long flags;
		u32 sp, dup, tfc, rfc;
		u32 val = 0;

		if (priv->phy->phy_node[id]) {
			of_node_put(priv->phy->phy_node[id]);
			priv->phy->phy_node[id] = NULL;
		}

		/*
		 * Word order of "mediatek,fixed-link" is
		 * <speed duplex tx_fc rx_fc>.  This must stay in sync with
		 * rt2880_port_init() in mdio_rt2880.c, which parses the very
		 * same DT property.  Keep tx_fc/rx_fc in priv->phy as well so
		 * the port state is complete regardless of the SoC.
		 *
		 * Validate the whole tuple before publishing it. Only speed
		 * was checked: duplex=2 was then used as a boolean here but
		 * compared against DUPLEX_FULL in mdio_rt2880.c, so the same
		 * DT value produced opposite duplex settings on the two
		 * paths. Flow-control cells must be boolean too.
		 */
		sp  = be32_to_cpup(link++);
		dup = be32_to_cpup(link++);
		tfc = be32_to_cpup(link++);
		rfc = be32_to_cpup(link++);

		if (sp != SPEED_10 && sp != SPEED_100 && sp != SPEED_1000) {
			dev_err(priv->dev, "invalid link speed: %u\n", sp);
			goto invalid_fixed_link;
		}
		if (dup > 1) {
			dev_err(priv->dev, "invalid fixed-link duplex: %u\n",
				dup);
			goto invalid_fixed_link;
		}
		if (tfc > 1 || rfc > 1) {
			dev_err(priv->dev,
				"invalid fixed-link flow control: %u/%u\n",
				tfc, rfc);
			goto invalid_fixed_link;
		}

		/* The GSW irq handler/irq_work and fe_phy_link_adjust() all
		 * read these fields and link[] under phy->lock (the switch
		 * interrupt is already registered by the time fe_init() runs
		 * port_init). Publish the validated tuple under that lock.
		 */
		spin_lock_irqsave(&priv->phy->lock, flags);
		priv->phy->speed[id] = sp;
		priv->phy->duplex[id] = dup;
		priv->phy->tx_fc[id] = tfc;
		priv->phy->rx_fc[id] = rfc;
		priv->link[id] = 1;
		spin_unlock_irqrestore(&priv->phy->lock, flags);

		switch (sp) {
		case SPEED_10:
			val = 0;
			break;
		case SPEED_100:
			val = 1;
			break;
		case SPEED_1000:
			val = 2;
			break;
		}
		val = PMCR_SPEED(val);
		val |= PMCR_LINK | PMCR_BACKPRES | PMCR_BACKOFF | PMCR_RX_EN |
			PMCR_TX_EN | PMCR_FORCE | PMCR_MAC_MODE | PMCR_IPG;
		if (tfc)
			val |= PMCR_TX_FC;
		if (rfc)
			val |= PMCR_RX_FC;
		if (dup)
			val |= PMCR_DUPLEX;
		mtk_switch_w32(gsw, val, GSW_REG_PORT_PMCR(id));
		dev_info(priv->dev, "using fixed link parameters\n");
		return;

invalid_fixed_link:
		/* Drop the bad description and make sure a previous
		 * ndo_init's link state cannot stay up for a rejected port.
		 */
		priv->phy->phy_fixed[id] = 0;
		spin_lock_irqsave(&priv->phy->lock, flags);
		priv->link[id] = 0;
		spin_unlock_irqrestore(&priv->phy->lock, flags);
		return;
	}

	if (priv->phy->phy_node[id]) {
		u32 val = PMCR_BACKPRES | PMCR_BACKOFF | PMCR_RX_EN |
			PMCR_TX_EN |  PMCR_MAC_MODE | PMCR_IPG;

		/* The old check keyed on phy_map[id], requiring the MDIO
		 * address to equal the port id; a PHY described by a node
		 * connects by node regardless of its bus address. Pass the
		 * eth-port id explicitly so phy[] and phy_node[] use the
		 * same index. Only program PMCR/autopoll once the connect
		 * succeeded -- fe_connect_phy_node() already dropped the
		 * node on failure.
		 */
		if (fe_connect_phy_node(priv, priv->phy->phy_node[id], id))
			return;

		mtk_switch_w32(gsw, val, GSW_REG_PORT_PMCR(id));
		gsw->autopoll |= BIT(id);
		mt7620_auto_poll(gsw);
	}
}

static void mt7620_port_init(struct fe_priv *priv, struct device_node *np)
{
	/* This function has many early returns. Wrapping it once keeps
	 * the reader reference paired on every one of them without
	 * touching the body.
	 */
	if (!fe_gsw_reader_enter())
		return;
	mt7620_port_init_locked(priv, np);
	fe_gsw_reader_exit();
}

static void mt7620_fe_reset(void)
{
	fe_reset(MT7620A_RESET_FE | MT7620A_RESET_ESW);
}

static void mt7620_rxcsum_config(bool enable)
{
	if (enable)
		fe_w32(fe_r32(MT7620A_GDMA1_FWD_CFG) | (GDMA_ICS_EN |
					GDMA_TCS_EN | GDMA_UCS_EN),
				MT7620A_GDMA1_FWD_CFG);
	else
		fe_w32(fe_r32(MT7620A_GDMA1_FWD_CFG) & ~(GDMA_ICS_EN |
					GDMA_TCS_EN | GDMA_UCS_EN),
				MT7620A_GDMA1_FWD_CFG);
}

static void mt7620_txcsum_config(bool enable)
{
	if (enable)
		fe_w32(fe_r32(MT7620A_CDMA_CSG_CFG) | (CDMA_ICS_EN |
					CDMA_UCS_EN | CDMA_TCS_EN),
				MT7620A_CDMA_CSG_CFG);
	else
		fe_w32(fe_r32(MT7620A_CDMA_CSG_CFG) & ~(CDMA_ICS_EN |
					CDMA_UCS_EN | CDMA_TCS_EN),
				MT7620A_CDMA_CSG_CFG);
}

static int mt7620_fwd_config(struct fe_priv *priv)
{
	fe_w32(fe_r32(MT7620A_GDMA1_FWD_CFG) & ~7, MT7620A_GDMA1_FWD_CFG);

	/* MT7620 advertises NETIF_F_IPV6_CSUM as well on newer ECO
	 * revisions. Honour both TX offload flags, like the generic
	 * fe_csum_config() does, or enabling only IPv6 checksum offload
	 * leaves the hardware CDMA checksum generator disabled.
	 */
	mt7620_txcsum_config((priv->fwd_features &
			      (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM)) != 0);
	mt7620_rxcsum_config((priv->fwd_features & NETIF_F_RXCSUM));

	return 0;
}

static void mt7620_tx_dma(struct fe_tx_dma *txd)
{
	/* The core only fills TX_DMA_DESP4_DEF itself when the SoC does
	 * not provide a tx_dma callback. MT7620 uses a different txd4
	 * layout: the forwarding-port bitmap and the UDF bit must be
	 * programmed here. An empty callback left txd4 at zero on every
	 * transmit, dropping queue/port selection and offload flags.
	 */
	txd->txd4 = TX_DMA_FP_BMAP | MT7620_TX_DMA_UDF;
}

static void mt7620_init_data(struct fe_soc_data *data,
			     struct net_device *netdev)
{
	struct fe_priv *priv = netdev_priv(netdev);

	priv->flags = FE_FLAG_PADDING_64B | FE_FLAG_RX_2B_OFFSET |
		FE_FLAG_RX_SG_DMA | FE_FLAG_HAS_SWITCH;

	netdev->hw_features = NETIF_F_IP_CSUM | NETIF_F_RXCSUM |
		NETIF_F_HW_VLAN_CTAG_TX;
	if (mt7620_get_eco() >= 5)
		netdev->hw_features |= NETIF_F_SG | NETIF_F_TSO | NETIF_F_TSO6 |
			NETIF_F_IPV6_CSUM;
}

static struct fe_soc_data mt7620_data = {
	.init_data = mt7620_init_data,
	.reset_fe = mt7620_fe_reset,
	.set_mac = mt7620_set_mac,
	.fwd_config = mt7620_fwd_config,
	.tx_dma = mt7620_tx_dma,
	.switch_init = mtk_gsw_init,
	.switch_cleanup = mtk_gsw_cleanup,
	.switch_config = mt7620_gsw_config,
	.port_init = mt7620_port_init,
	.reg_table = mt7620_reg_table,
	.pdma_glo_cfg = FE_PDMA_SIZE_16DWORDS,
	.rx_int = RT5350_RX_DONE_INT,
	.tx_int = RT5350_TX_DONE_INT,
	.status_int = MT7620_FE_GDM1_AF,
	.checksum_bit = MT7620_L4_VALID,
	.has_carrier = mt7620_has_carrier,
	.mdio_read = mt7620_mdio_read,
	.mdio_write = mt7620_mdio_write,
	.mdio_reset = mt7620_mdio_reset,
	.mdio_adjust_link = mt7620_mdio_link_adjust,
};

const struct of_device_id of_fe_match[] = {
	{ .compatible = "mediatek,mt7620-eth", .data = &mt7620_data },
	{},
};

MODULE_DEVICE_TABLE(of, of_fe_match);
