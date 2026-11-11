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

#include "mtk_eth_soc.h"

static const char fe_gdma_str[][ETH_GSTRING_LEN] = {
#define _FE(x...)	# x,
FE_STAT_REG_DECLARE
#undef _FE
};

static int fe_get_settings(struct net_device *dev,
			   struct ethtool_cmd *cmd)
{
	struct fe_priv *priv = netdev_priv(dev);
	int err;

	if (!priv->phy_dev)
		goto out_gset;

	/* refresh the state for both attachment styles; the PORT branch
	 * used to report stale cached values
	 */
	err = phy_read_status(priv->phy_dev);
	/* an MDIO failure is not a missing PHY: report the real error
	 * instead of collapsing it into -ENODEV
	 */
	if (err)
		return err;

	return phy_ethtool_gset(priv->phy_dev, cmd);

out_gset:
	return -ENODEV;
}

static int fe_set_settings(struct net_device *dev,
			   struct ethtool_cmd *cmd)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct phy_device *new_phy = NULL;
	unsigned long flags;
	int i;

	/* This driver only ever manages MDIO addresses 0..7:
	 * fe_phy_connect(), fe_phy_start/stop/disconnect() and
	 * fe_phy_link_adjust() all walk the eight port-sized arrays and
	 * phy_attached only has bits 0..7 set. Bounding by PHY_MAX_ADDR
	 * promised addresses that can never be attached, and the old
	 * comment claiming phy_map[]/phy_attached cover the whole bus
	 * was wrong: widening those loops to PHY_MAX_ADDR would write
	 * past link[8], phy[8] and speed[8]. Reject them up front.
	 * ARRAY_SIZE() is a compile-time constant, so this is safe even
	 * before the priv->phy NULL check below.
	 */
	if (cmd->phy_address >= ARRAY_SIZE(priv->phy->phy))
		return -EINVAL;

	if (!priv->phy_dev || !priv->phy)
		goto out_sset;

	if (cmd->phy_address != priv->phy_dev->addr) {
		/* Serialise the switch against other set_settings() calls
		 * and the MDIO paths that hold phy->lock. The other ethtool
		 * readers are lockless, so the sset call below must use the
		 * local snapshot instead of re-reading priv->phy_dev after
		 * the lock has been dropped.
		 */
		spin_lock_irqsave(&priv->phy->lock, flags);

		/* phy[] and phy_node[] are indexed by eth-port id, while
		 * cmd->phy_address is an MDIO address. Since phy[] was
		 * changed to be port-indexed the two are no longer
		 * interchangeable: locate the described PHY by its real
		 * bus address instead of subscripting those arrays.
		 */
		for (i = 0; i < ARRAY_SIZE(priv->phy->phy); i++) {
			if (priv->phy->phy[i] &&
			    priv->phy->phy[i]->addr == cmd->phy_address) {
				new_phy = priv->phy->phy[i];
				priv->phy_flags = FE_PHY_FLAG_PORT;
				break;
			}
		}

		/* Fall back to a bus-scanned PHY only when this driver
		 * actually attached it: phy_ethtool_sset() ends up in
		 * phydev->drv and must not run on an unattached phydev.
		 */
		/* cmd->phy_address is now bounded by the same 8 slots that
		 * fe_phy_connect() walks, so phy_map[] indexing is safe and
		 * phy_attached can only be set for an address this driver
		 * actually attached in phy_init().
		 */
		if (!new_phy &&
		    priv->mii_bus &&
		    priv->mii_bus->phy_map[cmd->phy_address] &&
		    test_bit(cmd->phy_address, &priv->phy->phy_attached)) {
			new_phy = priv->mii_bus->phy_map[cmd->phy_address];
			priv->phy_flags = FE_PHY_FLAG_ATTACH;
		}

		if (!new_phy) {
			spin_unlock_irqrestore(&priv->phy->lock, flags);
			goto out_sset;
		}
		priv->phy_dev = new_phy;
		spin_unlock_irqrestore(&priv->phy->lock, flags);

		/* A concurrent switch can replace priv->phy_dev as soon as
		 * the lock is released; apply the settings to the PHY that
		 * was just selected, not to whatever is published now.
		 */
		return phy_ethtool_sset(new_phy, cmd);
	}

	return phy_ethtool_sset(priv->phy_dev, cmd);

out_sset:
	return -ENODEV;
}

static void fe_get_drvinfo(struct net_device *dev,
			   struct ethtool_drvinfo *info)
{
	struct fe_priv *priv = netdev_priv(dev);

	strlcpy(info->driver, priv->dev->driver->name, sizeof(info->driver));
	strlcpy(info->version, MTK_FE_DRV_VERSION, sizeof(info->version));
	strlcpy(info->bus_info, dev_name(priv->dev), sizeof(info->bus_info));

	/* Advertise exactly the counters this device can actually report.
	 * The counter object is per-netdev; deriving its existence from a
	 * module-wide register table allowed n_stats to disagree with the
	 * installed ethtool ops when that table was replaced.
	 */
	if (priv->hw_stats)
		info->n_stats = ARRAY_SIZE(fe_gdma_str);
}

static u32 fe_get_msglevel(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);

	return priv->msg_enable;
}

static void fe_set_msglevel(struct net_device *dev, u32 value)
{
	struct fe_priv *priv = netdev_priv(dev);

	priv->msg_enable = value;
}

static int fe_nway_reset(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);

	if (!priv->phy_dev)
		goto out_nway_reset;

	return genphy_restart_aneg(priv->phy_dev);

out_nway_reset:
	return -EOPNOTSUPP;
}

static u32 fe_get_link(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	int err;

	if (!priv->phy_dev)
		return ethtool_op_get_link(dev);

	/* Refresh for both attachment types. Only the bus-scanned
	 * (FE_PHY_FLAG_ATTACH) path called genphy_update_link(), so a
	 * DT phy-handle PHY returned a cached link value until its state
	 * machine next polled. fe_get_settings() already calls
	 * phy_read_status(), which made one ethtool report show a stale
	 * link alongside freshly read speed and duplex.
	 */
	err = phy_read_status(priv->phy_dev);
	if (err)
		return ethtool_op_get_link(dev);

	return priv->phy_dev->link;
}

static int fe_set_ringparam(struct net_device *dev,
			    struct ethtool_ringparam *ring)
{
	/* The DMA descriptor rings are allocated once at probe time.
	 * Rebuilding them while the device is up would race with NAPI,
	 * interrupts and skb lifetimes, so this is not supported.
	 */
	return -EOPNOTSUPP;
}

static void fe_get_ringparam(struct net_device *dev,
			     struct ethtool_ringparam *ring)
{
	struct fe_priv *priv = netdev_priv(dev);

	/* set_ringparam is not supported, so there is no meaningful
	 * 'max' beyond the current fixed sizes; reporting MAX_DMA_DESC
	 * here while the MT7621 ring was larger contradicted itself
	 */
	ring->rx_max_pending = priv->rx_ring.rx_ring_size;
	ring->tx_max_pending = priv->tx_ring.tx_ring_size;
	ring->rx_pending = priv->rx_ring.rx_ring_size;
	ring->tx_pending = priv->tx_ring.tx_ring_size;
}

static void fe_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	switch (stringset) {
	case ETH_SS_STATS:
		memcpy(data, *fe_gdma_str, sizeof(fe_gdma_str));
		break;
	}
}

static int fe_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(fe_gdma_str);
	default:
		return -EOPNOTSUPP;
	}
}

static void fe_get_ethtool_stats(struct net_device *dev,
				 struct ethtool_stats *stats, u64 *data)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_hw_stats *hwstats = priv->hw_stats;
	u64 *data_src, *data_dst;
	unsigned int start;
	unsigned long flags;
	int i;

	/* the copy below relies on the FE_STAT_REG_DECLARE expansion
	 * filling the tail of struct fe_hw_stats in the same order the
	 * string table lists them, starting at tx_bytes
	 */
	BUILD_BUG_ON(offsetof(struct fe_hw_stats, tx_bytes) +
		     ARRAY_SIZE(fe_gdma_str) * sizeof(u64) >
		     sizeof(struct fe_hw_stats));

	/* fe_get_stats64() and fe_poll() both guard this pointer, and the
	 * stats ops table is only installed when hw_stats was allocated.
	 * Keep the same guard here: the register table used to decide
	 * that installation is module-wide state, not this device's.
	 * ethtool still copies n_stats u64 values back to userspace, so
	 * do not leave the caller's buffer uninitialised on that path.
	 */
	if (!hwstats) {
		memset(data, 0, ARRAY_SIZE(fe_gdma_str) * sizeof(u64));
		return;
	}

	if (netif_running(dev) && netif_device_present(dev)) {
		/* Take the lock unconditionally, like fe_get_stats64():
		 * a failed trylock skips the clear-on-read hardware
		 * counters, and 32-bit increments can wrap away before
		 * the next successful update. Use the IRQ-safe form used
		 * by fe_poll() as well: netpoll can run that poll with
		 * hard IRQs disabled, so a _bh/plain spin_lock() mix on
		 * this lock deadlocks the same CPU.
		 */
		spin_lock_irqsave(&hwstats->stats_lock, flags);
		fe_stats_update(priv);
		spin_unlock_irqrestore(&hwstats->stats_lock, flags);
	}

	do {
		data_src = &hwstats->tx_bytes;
		data_dst = data;
		start = u64_stats_fetch_begin_irq(&hwstats->syncp);

		for (i = 0; i < ARRAY_SIZE(fe_gdma_str); i++)
			*data_dst++ = *data_src++;

	} while (u64_stats_fetch_retry_irq(&hwstats->syncp, start));
}

static const struct ethtool_ops fe_ethtool_ops = {
	.get_settings		= fe_get_settings,
	.set_settings		= fe_set_settings,
	.get_drvinfo		= fe_get_drvinfo,
	.get_msglevel		= fe_get_msglevel,
	.set_msglevel		= fe_set_msglevel,
	.nway_reset		= fe_nway_reset,
	.get_link		= fe_get_link,
	.set_ringparam		= fe_set_ringparam,
	.get_ringparam		= fe_get_ringparam,
};

static const struct ethtool_ops fe_ethtool_ops_stats = {
	.get_settings		= fe_get_settings,
	.set_settings		= fe_set_settings,
	.get_drvinfo		= fe_get_drvinfo,
	.get_msglevel		= fe_get_msglevel,
	.set_msglevel		= fe_set_msglevel,
	.nway_reset		= fe_nway_reset,
	.get_link		= fe_get_link,
	.set_ringparam		= fe_set_ringparam,
	.get_ringparam		= fe_get_ringparam,
	.get_strings		= fe_get_strings,
	.get_sset_count		= fe_get_sset_count,
	.get_ethtool_stats	= fe_get_ethtool_stats,
};

void fe_set_ethtool_ops(struct net_device *netdev)
{
	struct fe_priv *priv = netdev_priv(netdev);

	/* Install the stats-capable table only when this device owns the
	 * counter object those callbacks dereference. fe_probe() allocates
	 * hw_stats from the same per-device decision, so ops, n_stats,
	 * fe_get_stats64() and fe_get_ethtool_stats() can no longer
	 * disagree after the module-wide register table is replaced.
	 */
	if (priv->hw_stats)
		netdev->ethtool_ops = &fe_ethtool_ops_stats;
	else
		netdev->ethtool_ops = &fe_ethtool_ops;
}
