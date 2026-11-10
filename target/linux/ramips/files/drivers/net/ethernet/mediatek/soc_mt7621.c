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
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/switch.h>

#include <asm/mach-ralink/ralink_regs.h>

#include "mtk_eth_soc.h"
#include "gsw_mt7620.h"
#include "mt7530.h"
#include "mdio.h"

#define MT7620A_CDMA_CSG_CFG	0x400
#define MT7621_CDMP_IG_CTRL	(MT7620A_CDMA_CSG_CFG + 0x00)
#define MT7621_CDMP_EG_CTRL	(MT7620A_CDMA_CSG_CFG + 0x04)
#define MT7621_CDMP_EG_RXVLAN	BIT(0)
#define MT7621_RESET_FE		BIT(6)
#define MT7621_L4_VALID		BIT(24)

#define MT7621_TX_DMA_UDF	BIT(19)
#define MT7621_TX_DMA_FPORT	BIT(25)

#define CDMA_ICS_EN		BIT(2)
#define CDMA_UCS_EN		BIT(1)
#define CDMA_TCS_EN		BIT(0)

#define GDMA_ICS_EN		BIT(22)
#define GDMA_TCS_EN		BIT(21)
#define GDMA_UCS_EN		BIT(20)

/* frame engine counters */
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

static const u16 mt7621_reg_table[FE_REG_COUNT] = {
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
	[FE_REG_FE_DMA_VID_BASE] = 0,
	[FE_REG_FE_COUNTER_BASE] = MT7621_GDM1_TX_GBCNT,
	[FE_REG_FE_RST_GL] = MT7621_FE_RST_GL,
	[FE_REG_FE_INT_STATUS2] = MT7620_FE_INT_STATUS2,
};

static int mt7621_gsw_config(struct fe_priv *priv)
{
	/* A GSW platform-device unbind can clear swpriv while the FE
	 * netdev stays alive. Refuse to publish a switch_dev in that
	 * state, exactly as mt7620_gsw_config() already does: the MDIO
	 * bus alone is not proof that the GSW backing this switch is
	 * still present.
	 */
	if (!priv->soc->swpriv) {
		dev_err(priv->dev, "GSW is not available\n");
		return -ENODEV;
	}

	/* the MT7530 must answer at MDIO address 0x1f; succeeding
	 * silently without it would leave FE_FLAG_HAS_SWITCH set with
	 * no switch behind it
	 */
	if (!priv->mii_bus || !priv->mii_bus->phy_map[0x1f]) {
		dev_err(priv->dev, "MT7530 not found on the MDIO bus\n");
		return -ENODEV;
	}

	/* The capacity check, mt7530_probe() and the publication into
	 * gsw_dev[] must be one locked operation: mt7530_probe()
	 * allocates state, registers a switch_dev and rewrites the
	 * hardware VLAN table, none of which an overflow path could
	 * fully undo, and a racing GSW unbind must not leave a newly
	 * registered switch behind after releasing its devm mapping.
	 */
	return fe_probe_and_register_switch(priv, NULL, priv->mii_bus, 1);
}

static void mt7621_fe_reset(void)
{
	fe_reset(MT7621_RESET_FE);
}

static void mt7621_rxcsum_config(bool enable)
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

static void mt7621_rxvlan_config(bool enable)
{
	/* This is a shared CDMA egress control register, not a single-bit
	 * latch: writing a bare 1 or 0 also clears every other field that
	 * the bootloader/hardware programmed into it. mt7621_fwd_config()
	 * is called on every frame-engine init, so toggle only the RX VLAN
	 * bit and preserve the rest, like mt7621_rxcsum_config() does.
	 */
	u32 val = fe_r32(MT7621_CDMP_EG_CTRL) & ~MT7621_CDMP_EG_RXVLAN;

	if (enable)
		val |= MT7621_CDMP_EG_RXVLAN;

	fe_w32(val, MT7621_CDMP_EG_CTRL);
}

static int mt7621_fwd_config(struct fe_priv *priv)
{
	fe_w32(fe_r32(MT7620A_GDMA1_FWD_CFG) & ~0xffff,
	       MT7620A_GDMA1_FWD_CFG);

	/* mt7621 doesn't have txcsum config */
	mt7621_rxcsum_config((priv->fwd_features & NETIF_F_RXCSUM));
	mt7621_rxvlan_config(priv->fwd_features & NETIF_F_HW_VLAN_CTAG_RX);

	return 0;
}

static void mt7621_tx_dma(struct fe_tx_dma *txd)
{
	/* fe_tx_map_dma() places VLAN, checksum and TSO information in
	 * the user-defined field of txd4. MT7621 needs its UDF enable bit
	 * in the SoC default value for the hardware to interpret them.
	 */
	txd->txd4 = MT7621_TX_DMA_FPORT | MT7621_TX_DMA_UDF;
}

static void mt7621_init_data(struct fe_soc_data *data,
			     struct net_device *netdev)
{
	struct fe_priv *priv = netdev_priv(netdev);

	priv->flags = FE_FLAG_PADDING_64B | FE_FLAG_RX_2B_OFFSET |
		FE_FLAG_RX_SG_DMA | FE_FLAG_NAPI_WEIGHT |
		FE_FLAG_HAS_SWITCH | FE_FLAG_JUMBO_FRAME;

	netdev->hw_features = NETIF_F_IP_CSUM | NETIF_F_RXCSUM |
		NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_CTAG_RX |
		NETIF_F_SG | NETIF_F_IPV6_CSUM;
}

static void mt7621_set_mac(struct fe_priv *priv, unsigned char *mac)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->page_lock, flags);
	fe_w32((mac[0] << 8) | mac[1], GSW_REG_GDMA1_MAC_ADRH);
	fe_w32(((u32)mac[2] << 24) | ((u32)mac[3] << 16) |
	       ((u32)mac[4] << 8) | mac[5], GSW_REG_GDMA1_MAC_ADRL);
	spin_unlock_irqrestore(&priv->page_lock, flags);
}

static int mt7621_reset_ports_locked(struct fe_priv *priv)
{
	struct mt7620_gsw *gsw = priv->soc->swpriv;
	struct mii_bus *bus;
	int ret = 0;
	u8 i;

	if (!gsw)
		return 0;

	/* swconfig (mt7530.c) performs its indirect register accesses as a
	 * page-select plus two data transfers under bus->mdio_lock, while
	 * gsw_irq_work_mt7621() takes the same lock before touching PIAC.
	 * This reset runs from the TX-timeout work, i.e. with the mii_bus
	 * alive and swconfig callable, but only held gsw->mdio_lock for
	 * each single transfer. Without the bus lock a PHY access here can
	 * land between another user's page select and its data transfer and
	 * make that transfer hit the wrong register. Hold the same mutex
	 * for the whole port power-cycle.
	 */
	bus = priv->mii_bus;
	if (bus)
		mutex_lock(&bus->mdio_lock);

	/* Disable all ports */
	for (i = 0; i <= 4; i++) {
		u16 mii_val;

		/* a timed-out read returns garbage that must not be
		 * OR-ed and written back into the BMCR
		 */
		if (mt7620_mii_read_reg(gsw, i, 0x0, &mii_val)) {
			dev_warn(priv->dev, "PHY %d: power-down read failed\n",
				 i);
			ret = -EIO;
			continue;
		}
		mii_val |= BIT(11);
		if (_mt7620_mii_write(gsw, i, 0x0, mii_val)) {
			dev_warn(priv->dev, "PHY %d: power-down write failed\n",
				 i);
			ret = -EIO;
		}
	}

	/* Allow ports a (short) time to settle */
	udelay(1000);

	/* Enable ports */
	for (i = 0; i <= 4; i++) {
		u16 mii_val;

		if (mt7620_mii_read_reg(gsw, i, 0, &mii_val)) {
			dev_warn(priv->dev, "PHY %d: power-up read failed\n",
				 i);
			ret = -EIO;
			continue;
		}
		mii_val &= ~BIT(11);
		if (_mt7620_mii_write(gsw, i, 0, mii_val)) {
			dev_warn(priv->dev, "PHY %d: power-up write failed\n",
				 i);
			ret = -EIO;
		}
	}

	if (bus)
		mutex_unlock(&bus->mdio_lock);

	return ret;
}

static int mt7621_reset_ports(struct fe_priv *priv)
{
	int ret;

	/* This runs from the TX-timeout work, which does not hold
	 * gsw_teardown_lock. Keep the devm GSW alive for the whole
	 * power-cycle even if the platform device is unbound meanwhile.
	 */
	fe_gsw_reader_enter();
	ret = mt7621_reset_ports_locked(priv);
	fe_gsw_reader_exit();

	return ret;
}

static struct fe_soc_data mt7621_data = {
	.init_data = mt7621_init_data,
	.reset_fe = mt7621_fe_reset,
	.set_mac = mt7621_set_mac,
	.fwd_config = mt7621_fwd_config,
	.tx_dma = mt7621_tx_dma,
	.switch_init = mtk_gsw_init,
	.switch_cleanup = mtk_gsw_cleanup,
	.switch_config = mt7621_gsw_config,
	.reg_table = mt7621_reg_table,
	.pdma_glo_cfg = FE_PDMA_SIZE_16DWORDS,
	.rx_int = RT5350_RX_DONE_INT,
	.tx_int = RT5350_TX_DONE_INT,
	.status_int = (MT7621_FE_GDM1_AF | MT7621_FE_GDM2_AF),
	.checksum_bit = MT7621_L4_VALID,
	.has_carrier = mt7620_has_carrier,
	.mdio_read = mt7620_mdio_read,
	.mdio_write = mt7620_mdio_write,
	.mdio_reset = mt7620_mdio_reset,
	.mdio_adjust_link = mt7620_mdio_link_adjust,
	.reset_ports = mt7621_reset_ports,
};

const struct of_device_id of_fe_match[] = {
	{ .compatible = "mediatek,mt7621-eth", .data = &mt7621_data },
	{},
};

MODULE_DEVICE_TABLE(of, of_fe_match);
