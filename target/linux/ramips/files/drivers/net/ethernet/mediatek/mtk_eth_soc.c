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
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/if_vlan.h>
#include <linux/reset.h>
#include <linux/tcp.h>
#include <linux/io.h>
#include <linux/bug.h>
#include <linux/bitmap.h>
#include <linux/switch.h>

#include <asm/mach-ralink/ralink_regs.h>

#include "mtk_eth_soc.h"
#include "mdio.h"
#include "ethtool.h"

#define	MAX_RX_LENGTH		1536
#define FE_RX_ETH_HLEN		(VLAN_ETH_HLEN + VLAN_HLEN + ETH_FCS_LEN)
#define FE_RX_HLEN		(NET_SKB_PAD + FE_RX_ETH_HLEN + NET_IP_ALIGN)
#define DMA_DUMMY_DESC		0xffffffff
#define FE_DEFAULT_MSG_ENABLE \
		(NETIF_MSG_DRV | \
		NETIF_MSG_PROBE | \
		NETIF_MSG_LINK | \
		NETIF_MSG_TIMER | \
		NETIF_MSG_IFDOWN | \
		NETIF_MSG_IFUP | \
		NETIF_MSG_RX_ERR | \
		NETIF_MSG_TX_ERR)

#define TX_DMA_DESP2_DEF	(TX_DMA_LS0 | TX_DMA_DONE)
#define TX_DMA_DESP4_DEF	(TX_DMA_QN(3) | TX_DMA_PN(1))
#define NEXT_TX_DESP_IDX(X)	(((X) + 1) & ((ring)->tx_ring_size - 1))
#define NEXT_RX_DESP_IDX(X)	(((X) + 1) & ((ring)->rx_ring_size - 1))

#define SYSC_REG_RSTCTRL	0x34

static int fe_msg_level = -1;
module_param_named(msg_level, fe_msg_level, int, 0);
MODULE_PARM_DESC(msg_level, "Message level (-1=defaults,0=none,...,16=all)");

static const u16 fe_reg_table_default[FE_REG_COUNT] = {
	[FE_REG_PDMA_GLO_CFG] = FE_PDMA_GLO_CFG,
	[FE_REG_PDMA_RST_CFG] = FE_PDMA_RST_CFG,
	[FE_REG_DLY_INT_CFG] = FE_DLY_INT_CFG,
	[FE_REG_TX_BASE_PTR0] = FE_TX_BASE_PTR0,
	[FE_REG_TX_MAX_CNT0] = FE_TX_MAX_CNT0,
	[FE_REG_TX_CTX_IDX0] = FE_TX_CTX_IDX0,
	[FE_REG_TX_DTX_IDX0] = FE_TX_DTX_IDX0,
	[FE_REG_RX_BASE_PTR0] = FE_RX_BASE_PTR0,
	[FE_REG_RX_MAX_CNT0] = FE_RX_MAX_CNT0,
	[FE_REG_RX_CALC_IDX0] = FE_RX_CALC_IDX0,
	[FE_REG_RX_DRX_IDX0] = FE_RX_DRX_IDX0,
	[FE_REG_FE_INT_ENABLE] = FE_FE_INT_ENABLE,
	[FE_REG_FE_INT_STATUS] = FE_FE_INT_STATUS,
	[FE_REG_FE_DMA_VID_BASE] = FE_DMA_VID0,
	[FE_REG_FE_COUNTER_BASE] = FE_GDMA1_TX_GBCNT,
	[FE_REG_FE_RST_GL] = FE_FE_RST_GL,
};

static const u16 *fe_reg_table = fe_reg_table_default;

struct fe_work_t {
	int bitnr;
	void (*action)(struct fe_priv *);
};

static void __iomem *fe_base;

void fe_w32(u32 val, unsigned reg)
{
	__raw_writel(val, fe_base + reg);
}

u32 fe_r32(unsigned reg)
{
	return __raw_readl(fe_base + reg);
}

void fe_reg_w32(u32 val, enum fe_reg reg)
{
	fe_w32(val, fe_reg_table[reg]);
}

u32 fe_reg_r32(enum fe_reg reg)
{
	return fe_r32(fe_reg_table[reg]);
}

void fe_reset(u32 reset_bits)
{
	u32 t;

	t = rt_sysc_r32(SYSC_REG_RSTCTRL);
	t |= reset_bits;
	rt_sysc_w32(t, SYSC_REG_RSTCTRL);
	usleep_range(10, 20);

	t &= ~reset_bits;
	rt_sysc_w32(t, SYSC_REG_RSTCTRL);
	usleep_range(10, 20);
}
/* esw_rt3050.o and gsw_mt7621.o are separate modules when built as =m */
EXPORT_SYMBOL_GPL(fe_reset);

static inline void fe_int_disable(struct fe_priv *priv, u32 mask)
{
	unsigned long flags;

	/* the read-modify-write must be serialised: the irq handler,
	 * NAPI completion and open/stop paths all race on INT_ENABLE */
	spin_lock_irqsave(&priv->page_lock, flags);
	fe_reg_w32(fe_reg_r32(FE_REG_FE_INT_ENABLE) & ~mask,
		   FE_REG_FE_INT_ENABLE);
	/* flush write */
	fe_reg_r32(FE_REG_FE_INT_ENABLE);
	spin_unlock_irqrestore(&priv->page_lock, flags);
}

static inline void fe_int_enable(struct fe_priv *priv, u32 mask)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->page_lock, flags);
	fe_reg_w32(fe_reg_r32(FE_REG_FE_INT_ENABLE) | mask,
		   FE_REG_FE_INT_ENABLE);
	/* flush write */
	fe_reg_r32(FE_REG_FE_INT_ENABLE);
	spin_unlock_irqrestore(&priv->page_lock, flags);
}

static inline void fe_hw_set_macaddr(struct fe_priv *priv, unsigned char *mac)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->page_lock, flags);
	fe_w32((mac[0] << 8) | mac[1], FE_GDMA1_MAC_ADRH);
	fe_w32((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
	       FE_GDMA1_MAC_ADRL);
	spin_unlock_irqrestore(&priv->page_lock, flags);
}

static int fe_set_mac_address(struct net_device *dev, void *p)
{
	int ret = eth_mac_addr(dev, p);

	if (!ret) {
		struct fe_priv *priv = netdev_priv(dev);

		if (priv->soc->set_mac)
			priv->soc->set_mac(priv, dev->dev_addr);
		else
			fe_hw_set_macaddr(priv, dev->dev_addr);
	}

	return ret;
}

static inline int fe_max_frag_size(int mtu)
{
	/* make sure buf_size will be at least MAX_RX_LENGTH */
	if (mtu + FE_RX_ETH_HLEN < MAX_RX_LENGTH)
		mtu = MAX_RX_LENGTH - FE_RX_ETH_HLEN;

	return SKB_DATA_ALIGN(FE_RX_HLEN + mtu) +
		SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
}

static inline int fe_max_buf_size(int frag_size)
{
	int buf_size = frag_size - NET_SKB_PAD - NET_IP_ALIGN -
		       SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

	/* warn but still return the true capacity: pretending the frag
	 * could hold MAX_RX_LENGTH when it cannot would make the caller
	 * program a larger DMA length than the buffer really has
	 */
	WARN_ON_ONCE(buf_size < MAX_RX_LENGTH);
	return buf_size;
}

static inline void fe_get_rxd(struct fe_rx_dma *rxd, struct fe_rx_dma *dma_rxd)
{
	rxd->rxd1 = dma_rxd->rxd1;
	rxd->rxd2 = dma_rxd->rxd2;
	rxd->rxd3 = dma_rxd->rxd3;
	rxd->rxd4 = dma_rxd->rxd4;
	/* the DONE bit (rxd2) must be ordered before the descriptor
	 * fields it protects: on weakly ordered MIPS the CPU may
	 * otherwise speculate rxd1 (the DMA address) before DONE is
	 * known to be set
	 */
	dma_rmb();
}

static inline void fe_set_txd(struct fe_tx_dma *txd, struct fe_tx_dma *dma_txd)
{
	dma_txd->txd1 = txd->txd1;
	dma_txd->txd3 = txd->txd3;
	dma_txd->txd4 = txd->txd4;
	/* clean dma done flag last */
	dma_txd->txd2 = txd->txd2;
}

static void fe_clean_rx(struct fe_priv *priv)
{
	struct fe_rx_ring *ring = &priv->rx_ring;
	struct page *page;
	int i;

	if (ring->rx_data) {
		for (i = 0; i < ring->rx_ring_size; i++)
			if (ring->rx_data[i]) {
				if (ring->rx_dma && ring->rx_dma[i].rxd1)
					dma_unmap_single(priv->dev,
							 ring->rx_dma[i].rxd1,
							 ring->rx_buf_size,
							 DMA_FROM_DEVICE);
				skb_free_frag(ring->rx_data[i]);
			}

		kfree(ring->rx_data);
		ring->rx_data = NULL;
	}

	if (ring->rx_dma) {
		dma_free_coherent(priv->dev,
				  ring->rx_ring_size * sizeof(*ring->rx_dma),
				  ring->rx_dma,
				  ring->rx_phys);
		ring->rx_dma = NULL;
	}

	if (!ring->frag_cache.va)
	    return;

	page = virt_to_page(ring->frag_cache.va);
	__page_frag_cache_drain(page, ring->frag_cache.pagecnt_bias);
	memset(&ring->frag_cache, 0, sizeof(ring->frag_cache));
}

static int fe_alloc_rx(struct fe_priv *priv)
{
	struct fe_rx_ring *ring = &priv->rx_ring;
	int i, pad;

	ring->rx_data = kcalloc(ring->rx_ring_size, sizeof(*ring->rx_data),
			GFP_KERNEL);
	if (!ring->rx_data)
		goto no_rx_mem;

	for (i = 0; i < ring->rx_ring_size; i++) {
		ring->rx_data[i] = page_frag_alloc(&ring->frag_cache,
						   ring->frag_size,
						   GFP_KERNEL);
		if (!ring->rx_data[i])
			goto no_rx_mem;
	}

	ring->rx_dma = dma_alloc_coherent(priv->dev,
			ring->rx_ring_size * sizeof(*ring->rx_dma),
			&ring->rx_phys,
			GFP_ATOMIC | __GFP_ZERO);
	if (!ring->rx_dma)
		goto no_rx_mem;

	if (priv->flags & FE_FLAG_RX_2B_OFFSET)
		pad = 0;
	else
		pad = NET_IP_ALIGN;
	for (i = 0; i < ring->rx_ring_size; i++) {
		dma_addr_t dma_addr = dma_map_single(priv->dev,
				ring->rx_data[i] + NET_SKB_PAD + pad,
				ring->rx_buf_size,
				DMA_FROM_DEVICE);
		if (unlikely(dma_mapping_error(priv->dev, dma_addr)))
			goto no_rx_mem;
		ring->rx_dma[i].rxd1 = (unsigned int)dma_addr;

		if (priv->flags & FE_FLAG_RX_SG_DMA)
			ring->rx_dma[i].rxd2 = RX_DMA_PLEN0(ring->rx_buf_size);
		else
			ring->rx_dma[i].rxd2 = RX_DMA_LSO;
	}
	ring->rx_calc_idx = ring->rx_ring_size - 1;
	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();

	fe_reg_w32(ring->rx_phys, FE_REG_RX_BASE_PTR0);
	fe_reg_w32(ring->rx_ring_size, FE_REG_RX_MAX_CNT0);
	fe_reg_w32(ring->rx_calc_idx, FE_REG_RX_CALC_IDX0);
	fe_reg_w32(FE_PST_DRX_IDX0, FE_REG_PDMA_RST_CFG);

	return 0;

no_rx_mem:
	return -ENOMEM;
}

static void fe_txd_unmap(struct device *dev, struct fe_tx_buf *tx_buf)
{
	if (dma_unmap_len(tx_buf, dma_len0))
		dma_unmap_page(dev,
			       dma_unmap_addr(tx_buf, dma_addr0),
			       dma_unmap_len(tx_buf, dma_len0),
			       DMA_TO_DEVICE);

	if (dma_unmap_len(tx_buf, dma_len1))
		dma_unmap_page(dev,
			       dma_unmap_addr(tx_buf, dma_addr1),
			       dma_unmap_len(tx_buf, dma_len1),
			       DMA_TO_DEVICE);

	dma_unmap_len_set(tx_buf, dma_len0, 0);
	dma_unmap_len_set(tx_buf, dma_len1, 0);
	if (tx_buf->skb && (tx_buf->skb != (struct sk_buff *)DMA_DUMMY_DESC))
		dev_kfree_skb_any(tx_buf->skb);
	tx_buf->skb = NULL;
}

static void fe_clean_tx(struct fe_priv *priv)
{
	int i;
	struct device *dev = priv->dev;
	struct fe_tx_ring *ring = &priv->tx_ring;

	if (ring->tx_buf) {
		for (i = 0; i < ring->tx_ring_size; i++)
			fe_txd_unmap(dev, &ring->tx_buf[i]);
		kfree(ring->tx_buf);
		ring->tx_buf = NULL;
	}

	if (ring->tx_dma) {
		dma_free_coherent(dev,
				  ring->tx_ring_size * sizeof(*ring->tx_dma),
				  ring->tx_dma,
				  ring->tx_phys);
		ring->tx_dma = NULL;
	}

	netdev_reset_queue(priv->netdev);
}

static int fe_alloc_tx(struct fe_priv *priv)
{
	int i;
	struct fe_tx_ring *ring = &priv->tx_ring;

	ring->tx_free_idx = 0;
	ring->tx_next_idx = 0;
	ring->tx_thresh = max((unsigned long)ring->tx_ring_size >> 2,
			      MAX_SKB_FRAGS);

	ring->tx_buf = kcalloc(ring->tx_ring_size, sizeof(*ring->tx_buf),
			GFP_KERNEL);
	if (!ring->tx_buf)
		goto no_tx_mem;

	ring->tx_dma = dma_alloc_coherent(priv->dev,
			ring->tx_ring_size * sizeof(*ring->tx_dma),
			&ring->tx_phys,
			GFP_ATOMIC | __GFP_ZERO);
	if (!ring->tx_dma)
		goto no_tx_mem;

	for (i = 0; i < ring->tx_ring_size; i++) {
		if (priv->soc->tx_dma)
			priv->soc->tx_dma(&ring->tx_dma[i]);
		ring->tx_dma[i].txd2 = TX_DMA_DESP2_DEF;
	}
	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();

	fe_reg_w32(ring->tx_phys, FE_REG_TX_BASE_PTR0);
	fe_reg_w32(ring->tx_ring_size, FE_REG_TX_MAX_CNT0);
	fe_reg_w32(0, FE_REG_TX_CTX_IDX0);
	fe_reg_w32(FE_PST_DTX_IDX0, FE_REG_PDMA_RST_CFG);

	return 0;

no_tx_mem:
	return -ENOMEM;
}

/* Contract: on failure this function may leave one ring fully or
 * partially allocated; the caller MUST call fe_free_dma(), whose
 * fe_clean_tx()/fe_clean_rx() tolerate (and release) half-initialised
 * rings. fe_alloc_tx()/fe_alloc_rx() deliberately do not roll back
 * their own partial allocations.
 */
static int fe_init_dma(struct fe_priv *priv)
{
	int err;

	/* the descriptor index macros (NEXT_*_DESP_IDX) and the
	 * calc-index initialisation wrap with (size - 1) as a bitmask:
	 * ring sizes must be powers of two
	 */
	if (WARN_ON_ONCE(!is_power_of_2(priv->tx_ring.tx_ring_size) ||
			 !is_power_of_2(priv->rx_ring.rx_ring_size)))
		return -EINVAL;

	/* rxd1/txd1 are 32-bit descriptor fields; the driver silently
	 * truncates dma_addr_t on wider platforms
	 */
	BUILD_BUG_ON(sizeof(dma_addr_t) > sizeof(u32));

	err = fe_alloc_tx(priv);
	if (err)
		return err;

	err = fe_alloc_rx(priv);
	if (err)
		return err;

	return 0;
}

static void fe_free_dma(struct fe_priv *priv)
{
	fe_clean_tx(priv);
	fe_clean_rx(priv);
}

void fe_stats_update(struct fe_priv *priv)
{
	struct fe_hw_stats *hwstats = priv->hw_stats;
	unsigned int base = fe_reg_table[FE_REG_FE_COUNTER_BASE];
	u64 stats;

	u64_stats_update_begin(&hwstats->syncp);

	if (IS_ENABLED(CONFIG_SOC_MT7621)) {
		/* The byte counters are accumulated with += here, which
		 * only makes sense if the hardware clears them on read.
		 * Under that semantic the lo->hi read order below is
		 * already tear-safe: a wrap between the reads folds the
		 * carry into the hi word while the pre-wrap lo word is
		 * still large, so the pair stays consistent. A hi->lo
		 * order (or re-reads) would instead *lose* data because
		 * the second read returns post-clear values. Do not
		 * "fix" the order without changing the accumulation
		 * model.
		 */
		hwstats->rx_bytes			+= fe_r32(base);
		stats					=  fe_r32(base + 0x04);
		if (stats)
			hwstats->rx_bytes		+= (stats << 32);
		hwstats->rx_packets			+= fe_r32(base + 0x08);
		hwstats->rx_overflow			+= fe_r32(base + 0x10);
		hwstats->rx_fcs_errors			+= fe_r32(base + 0x14);
		hwstats->rx_short_errors		+= fe_r32(base + 0x18);
		hwstats->rx_long_errors			+= fe_r32(base + 0x1c);
		hwstats->rx_checksum_errors		+= fe_r32(base + 0x20);
		hwstats->rx_flow_control_packets	+= fe_r32(base + 0x24);
		hwstats->tx_skip			+= fe_r32(base + 0x28);
		hwstats->tx_collisions			+= fe_r32(base + 0x2c);
		hwstats->tx_bytes			+= fe_r32(base + 0x30);
		stats					=  fe_r32(base + 0x34);
		if (stats)
			hwstats->tx_bytes		+= (stats << 32);
		hwstats->tx_packets			+= fe_r32(base + 0x38);
	} else {
		hwstats->tx_bytes			+= fe_r32(base);
		hwstats->tx_packets			+= fe_r32(base + 0x04);
		hwstats->tx_skip			+= fe_r32(base + 0x08);
		hwstats->tx_collisions			+= fe_r32(base + 0x0c);
		hwstats->rx_bytes			+= fe_r32(base + 0x20);
		hwstats->rx_packets			+= fe_r32(base + 0x24);
		hwstats->rx_overflow			+= fe_r32(base + 0x28);
		hwstats->rx_fcs_errors			+= fe_r32(base + 0x2c);
		hwstats->rx_short_errors		+= fe_r32(base + 0x30);
		hwstats->rx_long_errors			+= fe_r32(base + 0x34);
		hwstats->rx_checksum_errors		+= fe_r32(base + 0x38);
		hwstats->rx_flow_control_packets	+= fe_r32(base + 0x3c);
	}

	u64_stats_update_end(&hwstats->syncp);
}

static struct rtnl_link_stats64 *fe_get_stats64(struct net_device *dev,
				struct rtnl_link_stats64 *storage)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_hw_stats *hwstats = priv->hw_stats;
	unsigned int base = fe_reg_table[FE_REG_FE_COUNTER_BASE];
	unsigned int start;

	if (!base) {
		netdev_stats_to_stats64(storage, &dev->stats);
		return storage;
	}

	if (netif_running(dev) && netif_device_present(dev)) {
		/* take the lock unconditionally: a failed trylock silently
		 * skipped the update, and increments sitting in the 32-bit
		 * hardware counters were lost once they wrapped
		 */
		spin_lock_bh(&hwstats->stats_lock);
		fe_stats_update(priv);
		spin_unlock_bh(&hwstats->stats_lock);
	}

	do {
		start = u64_stats_fetch_begin_irq(&hwstats->syncp);
		storage->rx_packets = hwstats->rx_packets;
		storage->tx_packets = hwstats->tx_packets;
		storage->rx_bytes = hwstats->rx_bytes;
		storage->tx_bytes = hwstats->tx_bytes;
		storage->collisions = hwstats->tx_collisions;
		storage->rx_length_errors = hwstats->rx_short_errors +
			hwstats->rx_long_errors;
		storage->rx_over_errors = hwstats->rx_overflow;
		storage->rx_crc_errors = hwstats->rx_fcs_errors;
		storage->rx_errors = hwstats->rx_checksum_errors;
		storage->tx_aborted_errors = hwstats->tx_skip;
	} while (u64_stats_fetch_retry_irq(&hwstats->syncp, start));

	storage->tx_errors = priv->netdev->stats.tx_errors;
	storage->rx_dropped = priv->netdev->stats.rx_dropped;
	storage->tx_dropped = priv->netdev->stats.tx_dropped;

	return storage;
}

static int fe_vlan_rx_add_vid(struct net_device *dev,
			      __be16 proto, u16 vid)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 idx = (vid & 0xf);
	u32 vlan_cfg;

	if (!((fe_reg_table[FE_REG_FE_DMA_VID_BASE]) &&
	      (dev->features & NETIF_F_HW_VLAN_CTAG_TX)))
		return 0;

	if (test_bit(idx, &priv->vlan_map) && priv->vlan_vid[idx] == vid)
		return 0;

	/* The slot is either free or occupied by a colliding VID. The
	 * old code disabled TX VLAN offload for the whole device on a
	 * collision and never recovered nor updated the register, which
	 * both broke offload permanently and left a stale VID behind.
	 * Replace the slot contents instead.
	 */
	vlan_cfg = fe_r32(fe_reg_table[FE_REG_FE_DMA_VID_BASE] +
			((idx >> 1) << 2));
	if (idx & 0x1) {
		vlan_cfg &= 0xffff;
		vlan_cfg |= (vid << 16);
	} else {
		vlan_cfg &= 0xffff0000;
		vlan_cfg |= vid;
	}
	fe_w32(vlan_cfg, fe_reg_table[FE_REG_FE_DMA_VID_BASE] +
			((idx >> 1) << 2));
	priv->vlan_vid[idx] = vid;
	set_bit(idx, &priv->vlan_map);

	return 0;
}

static int fe_vlan_rx_kill_vid(struct net_device *dev,
			       __be16 proto, u16 vid)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 idx = (vid & 0xf);

	if (!((fe_reg_table[FE_REG_FE_DMA_VID_BASE]) &&
	      (dev->features & NETIF_F_HW_VLAN_CTAG_TX)))
		return 0;

	/* clear the register slot as well, not just the bookkeeping
	 * bit, so a removed VID does not keep matching in hardware
	 */
	if (test_bit(idx, &priv->vlan_map) && priv->vlan_vid[idx] == vid) {
		u32 vlan_cfg;

		vlan_cfg = fe_r32(fe_reg_table[FE_REG_FE_DMA_VID_BASE] +
				((idx >> 1) << 2));
		if (idx & 0x1)
			vlan_cfg &= 0xffff;
		else
			vlan_cfg &= 0xffff0000;
		fe_w32(vlan_cfg, fe_reg_table[FE_REG_FE_DMA_VID_BASE] +
				((idx >> 1) << 2));
		clear_bit(idx, &priv->vlan_map);
	}

	return 0;
}

static inline u32 fe_empty_txd(struct fe_tx_ring *ring)
{
	barrier();
	return (u32)(ring->tx_ring_size -
			((ring->tx_next_idx - ring->tx_free_idx) &
			 (ring->tx_ring_size - 1)) - 1);
}

struct fe_map_state {
	struct device *dev;
	struct fe_tx_dma txd;
	u32 def_txd4;
	int ring_idx;
	int i;
};

static void fe_tx_dma_write_desc(struct fe_tx_ring *ring, struct fe_map_state *st)
{
	fe_set_txd(&st->txd, &ring->tx_dma[st->ring_idx]);
	memset(&st->txd, 0, sizeof(st->txd));
	st->txd.txd4 = st->def_txd4;
	st->ring_idx = NEXT_TX_DESP_IDX(st->ring_idx);
}

static int __fe_tx_dma_map_page(struct fe_tx_ring *ring, struct fe_map_state *st,
				struct page *page, size_t offset, size_t size)
{
	struct device *dev = st->dev;
	struct fe_tx_buf *tx_buf;
	dma_addr_t mapped_addr;

	mapped_addr = dma_map_page(dev, page, offset, size, DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(dev, mapped_addr)))
		return -EIO;

	if (st->i && !(st->i & 1))
	    fe_tx_dma_write_desc(ring, st);

	tx_buf = &ring->tx_buf[st->ring_idx];
	if (st->i & 1) {
		st->txd.txd3 = mapped_addr;
		st->txd.txd2 |= TX_DMA_PLEN1(size);
		dma_unmap_addr_set(tx_buf, dma_addr1, mapped_addr);
		dma_unmap_len_set(tx_buf, dma_len1, size);
	} else {
		tx_buf->skb = (struct sk_buff *)DMA_DUMMY_DESC;
		st->txd.txd1 = mapped_addr;
		st->txd.txd2 = TX_DMA_PLEN0(size);
		dma_unmap_addr_set(tx_buf, dma_addr0, mapped_addr);
		dma_unmap_len_set(tx_buf, dma_len0, size);
	}
	st->i++;

	return 0;
}

static int fe_tx_dma_map_page(struct fe_tx_ring *ring, struct fe_map_state *st,
			      struct page *page, size_t offset, size_t size)
{
	int cur_size;
	int ret;

	while (size > 0) {
		cur_size = min_t(size_t, size, TX_DMA_BUF_LEN);

		ret = __fe_tx_dma_map_page(ring, st, page, offset, cur_size);
		if (ret)
			return ret;

		size -= cur_size;
		offset += cur_size;
	}

	return 0;
}

static int fe_tx_dma_map_skb(struct fe_tx_ring *ring, struct fe_map_state *st,
			     struct sk_buff *skb)
{
	struct page *page = virt_to_page(skb->data);
	size_t offset = offset_in_page(skb->data);
	size_t size = skb_headlen(skb);

	return fe_tx_dma_map_page(ring, st, page, offset, size);
}

static inline struct sk_buff *
fe_next_frag(struct sk_buff *head, struct sk_buff *skb)
{
	if (skb != head)
		return skb->next;

	if (skb_has_frag_list(skb))
		return skb_shinfo(skb)->frag_list;

	return NULL;
}

static int fe_tx_map_dma(struct sk_buff *skb, struct net_device *dev,
			 int tx_num, struct fe_tx_ring *ring)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_map_state st = {
		.dev = priv->dev,
		.ring_idx = ring->tx_next_idx,
	};
	struct sk_buff *head = skb;
	struct fe_tx_buf *tx_buf;
	unsigned int nr_frags;
	int i, j;

	/* init tx descriptor */
	if (priv->soc->tx_dma)
		priv->soc->tx_dma(&st.txd);
	else
		st.txd.txd4 = TX_DMA_DESP4_DEF;
	st.def_txd4 = st.txd.txd4;

	/* TX Checksum offload */
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		st.txd.txd4 |= TX_DMA_CHKSUM;

	/* VLAN header offload */
	if (skb_vlan_tag_present(skb)) {
		u16 tag = skb_vlan_tag_get(skb);

		if (IS_ENABLED(CONFIG_SOC_MT7621))
			st.txd.txd4 |= TX_DMA_INS_VLAN_MT7621 | tag;
		else
			st.txd.txd4 |= TX_DMA_INS_VLAN |
				((tag >> VLAN_PRIO_SHIFT) << 4) |
				(tag & 0xF);
	}

	/* TSO: fill MSS info in tcp checksum field */
	if (skb_is_gso(skb)) {
		if (skb_cow_head(skb, 0)) {
			netif_warn(priv, tx_err, dev,
				   "GSO expand head fail.\n");
			goto err_out;
		}
		if (skb_shinfo(skb)->gso_type &
				(SKB_GSO_TCPV4 | SKB_GSO_TCPV6)) {
			st.txd.txd4 |= TX_DMA_TSO;
			tcp_hdr(skb)->check = htons(skb_shinfo(skb)->gso_size);
		}
	}

next_frag:
	if (skb_headlen(skb) && fe_tx_dma_map_skb(ring, &st, skb))
		goto err_dma;

	/* TX SG offload */
	nr_frags = skb_shinfo(skb)->nr_frags;
	for (i = 0; i < nr_frags; i++) {
		struct skb_frag_struct *frag;

		frag = &skb_shinfo(skb)->frags[i];
		if (fe_tx_dma_map_page(ring, &st, skb_frag_page(frag),
				       frag->page_offset, skb_frag_size(frag)))
			goto err_dma;
	}

	skb = fe_next_frag(head, skb);
	if (skb)
		goto next_frag;

	/* set last segment */
	if (st.i & 0x1)
		st.txd.txd2 |= TX_DMA_LS0;
	else
		st.txd.txd2 |= TX_DMA_LS1;

	/* store skb to cleanup */
	tx_buf = &ring->tx_buf[st.ring_idx];
	tx_buf->skb = head;

	netdev_sent_queue(dev, head->len);
	skb_tx_timestamp(head);

	fe_tx_dma_write_desc(ring, &st);
	ring->tx_next_idx = st.ring_idx;

	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();
	if (unlikely(fe_empty_txd(ring) <= ring->tx_thresh)) {
		netif_stop_queue(dev);
		smp_mb();
		if (unlikely(fe_empty_txd(ring) > ring->tx_thresh))
			netif_wake_queue(dev);
	}

	if (netif_xmit_stopped(netdev_get_tx_queue(dev, 0)) || !head->xmit_more)
		fe_reg_w32(ring->tx_next_idx, FE_REG_TX_CTX_IDX0);

	return 0;

err_dma:
	j = ring->tx_next_idx;
	for (i = 0; i < tx_num; i++) {
		/* unmap dma */
		fe_txd_unmap(priv->dev, &ring->tx_buf[j]);
		ring->tx_dma[j].txd2 = TX_DMA_DESP2_DEF;

		j = NEXT_TX_DESP_IDX(j);
	}
	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();

err_out:
	return -1;
}

static inline int fe_skb_padto(struct sk_buff *skb, struct fe_priv *priv)
{
	unsigned int len;
	int ret;

	ret = 0;
	if (unlikely(skb->len < VLAN_ETH_ZLEN)) {
		/* FE_FLAG_PADDING_64B: the hardware pads short frames to
		 * 64 bytes itself, so software only has to act when
		 * FE_FLAG_PADDING_BUG says that mechanism is broken.
		 *
		 * Tag handling: a present (hwaccel) tag is not part of
		 * the wire length the MAC sees, so pad to ETH_ZLEN; an
		 * inlined 8021Q header makes the wire frame VLAN_ETH_ZLEN.
		 */
		if ((priv->flags & FE_FLAG_PADDING_64B) &&
		    !(priv->flags & FE_FLAG_PADDING_BUG))
			return ret;

		if (skb_vlan_tag_present(skb))
			len = ETH_ZLEN;
		else if (skb->protocol == cpu_to_be16(ETH_P_8021Q))
			len = VLAN_ETH_ZLEN;
		else if (!(priv->flags & FE_FLAG_PADDING_64B))
			len = ETH_ZLEN;
		else
			return ret;

		if (skb->len < len) {
			ret = skb_pad(skb, len - skb->len);
			if (ret < 0)
				return ret;
			skb->len = len;
			skb_set_tail_pointer(skb, len);
		}
	}

	return ret;
}

static inline int fe_cal_txd_req(struct sk_buff *skb)
{
	struct sk_buff *head = skb;
	struct skb_frag_struct *frag;
	unsigned int i, len;
	int nfrags = 0;

next_frag:
	/*
	 * fe_tx_dma_map_page() splits anything longer than TX_DMA_BUF_LEN
	 * over several descriptors, so every length has to be rounded the
	 * same way here.  Counting nr_frags as-is, and only rounding up
	 * when skb_is_gso(), let a plain non-GSO skb whose fragment or
	 * linear part exceeds TX_DMA_BUF_LEN come out too small.
	 *
	 * tx_num is used twice: as the ring-space reservation in
	 * fe_start_xmit(), and as the iteration count of the rollback
	 * loop in fe_tx_map_dma().  Under-counting both overran the
	 * reservation and left already-dma_map_page()'d buffers without
	 * a matching dma_unmap_page() on the error path.
	 */
	if ((len = skb_headlen(skb)) != 0)
		nfrags += DIV_ROUND_UP(len, TX_DMA_BUF_LEN);
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		frag = &skb_shinfo(skb)->frags[i];
		nfrags += DIV_ROUND_UP(skb_frag_size(frag), TX_DMA_BUF_LEN);
	}

	skb = fe_next_frag(head, skb);
	if (skb)
		goto next_frag;

	/* fe_tx_map_dma() always emits a final descriptor, even if empty */
	len = DIV_ROUND_UP(nfrags, 2);
	return len > 1 ? len : 1;
}

static int fe_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_tx_ring *ring = &priv->tx_ring;
	struct net_device_stats *stats = &dev->stats;
	int tx_num;
	int len;

	if (unlikely(test_bit(__FE_DOWN, &priv->state))) {
		stats->tx_dropped++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	if (fe_skb_padto(skb, priv)) {
		netif_warn(priv, tx_err, dev, "tx padding failed!\n");
		stats->tx_dropped++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/* sample the length after padding: fe_skb_padto() may have
	 * grown the skb
	 */
	len = skb->len;

	tx_num = fe_cal_txd_req(skb);
	if (unlikely(fe_empty_txd(ring) <= tx_num)) {
		netif_stop_queue(dev);
		netif_err(priv, tx_queued, dev,
			  "Tx Ring full when queue awake!\n");
		return NETDEV_TX_BUSY;
	}

	if (fe_tx_map_dma(skb, dev, tx_num, ring) < 0) {
		stats->tx_dropped++;
		dev_kfree_skb_any(skb);
	} else {
		stats->tx_packets++;
		stats->tx_bytes += len;
	}

	return NETDEV_TX_OK;
}

static int fe_poll_rx(struct napi_struct *napi, int budget,
		      struct fe_priv *priv, u32 rx_intr)
{
	struct net_device *netdev = priv->netdev;
	struct net_device_stats *stats = &netdev->stats;
	struct fe_soc_data *soc = priv->soc;
	struct fe_rx_ring *ring = &priv->rx_ring;
	int idx = ring->rx_calc_idx;
	u32 checksum_bit;
	struct sk_buff *skb;
	u8 *data, *new_data;
	struct fe_rx_dma *rxd, trxd;
	int done = 0, pad;

	if (netdev->features & NETIF_F_RXCSUM)
		checksum_bit = soc->checksum_bit;
	else
		checksum_bit = 0;

	if (priv->flags & FE_FLAG_RX_2B_OFFSET)
		pad = 0;
	else
		pad = NET_IP_ALIGN;

	while (done < budget) {
		unsigned int pktlen;
		dma_addr_t dma_addr;

		idx = NEXT_RX_DESP_IDX(idx);
		rxd = &ring->rx_dma[idx];
		data = ring->rx_data[idx];

		fe_get_rxd(&trxd, rxd);
		if (!(trxd.rxd2 & RX_DMA_DONE))
			break;

		/* Never trust the hardware-reported length blindly: a
		 * corrupted descriptor could make skb_put() run past the
		 * end of the frag. Drop the packet and recycle the old,
		 * still-mapped buffer instead.
		 */
		pktlen = RX_DMA_GET_PLEN0(trxd.rxd2);
		if (unlikely(pktlen > ring->rx_buf_size)) {
			stats->rx_dropped++;
			goto release_desc;
		}

		/* alloc new buffer */
		new_data = page_frag_alloc(&ring->frag_cache, ring->frag_size,
					   GFP_ATOMIC);
		if (unlikely(!new_data)) {
			stats->rx_dropped++;
			goto release_desc;
		}
		dma_addr = dma_map_single(priv->dev,
					  new_data + NET_SKB_PAD + pad,
					  ring->rx_buf_size,
					  DMA_FROM_DEVICE);
		if (unlikely(dma_mapping_error(priv->dev, dma_addr))) {
			skb_free_frag(new_data);
			/* count the drop like the other two allocation
			 * failure paths do
			 */
			stats->rx_dropped++;
			goto release_desc;
		}

		/* receive data */
		skb = build_skb(data, ring->frag_size);
		if (unlikely(!skb)) {
			dma_unmap_single(priv->dev, dma_addr, ring->rx_buf_size,
					 DMA_FROM_DEVICE);
			skb_free_frag(new_data);
			stats->rx_dropped++;
			goto release_desc;
		}
		skb_reserve(skb, NET_SKB_PAD + NET_IP_ALIGN);

		dma_unmap_single(priv->dev, trxd.rxd1,
				 ring->rx_buf_size, DMA_FROM_DEVICE);
		skb->dev = netdev;
		skb_put(skb, pktlen);
		if (trxd.rxd4 & checksum_bit)
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb_checksum_none_assert(skb);
		skb->protocol = eth_type_trans(skb, netdev);

		if (netdev->features & NETIF_F_HW_VLAN_CTAG_RX &&
		    RX_DMA_TAG & trxd.rxd2 && RX_DMA_VID(trxd.rxd3))
			__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q),
					       RX_DMA_VID(trxd.rxd3));

		stats->rx_packets++;
		stats->rx_bytes += pktlen;

		napi_gro_receive(napi, skb);

		ring->rx_data[idx] = new_data;
		rxd->rxd1 = (unsigned int)dma_addr;

release_desc:
		if (priv->flags & FE_FLAG_RX_SG_DMA)
			rxd->rxd2 = RX_DMA_PLEN0(ring->rx_buf_size);
		else
			rxd->rxd2 = RX_DMA_LSO;

		ring->rx_calc_idx = idx;
		done++;
	}

	if (done) {
		/* make sure that all changes to the dma ring are flushed before
		 * we continue
		 */
		wmb();
		fe_reg_w32(ring->rx_calc_idx, FE_REG_RX_CALC_IDX0);
	}

	return done;
}

static int fe_poll_tx(struct fe_priv *priv, int budget, u32 tx_intr,
		      int *tx_again)
{
	struct net_device *netdev = priv->netdev;
	unsigned int bytes_compl = 0;
	struct sk_buff *skb;
	struct fe_tx_buf *tx_buf;
	int done = 0;
	u32 idx, hwidx;
	struct fe_tx_ring *ring = &priv->tx_ring;

	idx = ring->tx_free_idx;
	hwidx = fe_reg_r32(FE_REG_TX_DTX_IDX0);

	while ((idx != hwidx) && budget) {
		tx_buf = &ring->tx_buf[idx];
		skb = tx_buf->skb;

		if (!skb)
			break;

		if (skb != (struct sk_buff *)DMA_DUMMY_DESC) {
			bytes_compl += skb->len;
			done++;
			budget--;
		}
		fe_txd_unmap(priv->dev, tx_buf);
		idx = NEXT_TX_DESP_IDX(idx);
	}
	ring->tx_free_idx = idx;

	/* done only counts skb-carrying descriptors: with done == 0 and
	 * idx != hwidx there can still be unreclaimed (dummy)
	 * descriptors left, which would otherwise wait for the next TX
	 * interrupt instead of another poll
	 */
	/* Ask for another poll only while the blocking descriptor still
	 * carries an skb. idx != hwidx with skb == NULL means the ring
	 * state is inconsistent (nothing can ever be reclaimed from that
	 * slot); requesting another poll there would make fe_poll()
	 * return the full budget forever, so napi_complete_done() and
	 * fe_int_enable() are never reached and the interface dies with
	 * ksoftirqd spinning.
	 */
	if (idx != hwidx && ring->tx_buf[idx].skb)
		*tx_again = 1;

	if (done) {
		netdev_completed_queue(netdev, done, bytes_compl);
		smp_mb();
		if (unlikely(netif_queue_stopped(netdev) &&
			     (fe_empty_txd(ring) > ring->tx_thresh)))
			netif_wake_queue(netdev);
	}

	return done;
}

static int fe_poll(struct napi_struct *napi, int budget)
{
	struct fe_priv *priv = container_of(napi, struct fe_priv, rx_napi);
	struct fe_hw_stats *hwstat = priv->hw_stats;
	int tx_done, rx_done, tx_again;
	u32 status, fe_status, status_reg, mask;
	u32 tx_intr, rx_intr, status_intr;

	if (unlikely(test_bit(__FE_DOWN, &priv->state))) {
		napi_complete(napi);
		return 0;
	}

	status = fe_reg_r32(FE_REG_FE_INT_STATUS);
	fe_status = status;
	tx_intr = priv->soc->tx_int;
	rx_intr = priv->soc->rx_int;
	status_intr = priv->soc->status_int;
	tx_done = 0;
	rx_done = 0;
	tx_again = 0;

	if ((status & (tx_intr | rx_intr))) {
		fe_reg_w32((status & (tx_intr | rx_intr)), FE_REG_FE_INT_STATUS);
	}

	if (fe_reg_table[FE_REG_FE_INT_STATUS2]) {
		fe_status = fe_reg_r32(FE_REG_FE_INT_STATUS2);
		status_reg = FE_REG_FE_INT_STATUS2;
	} else {
		status_reg = FE_REG_FE_INT_STATUS;
	}

	tx_done = fe_poll_tx(priv, budget, tx_intr, &tx_again);
	rx_done = fe_poll_rx(napi, budget, priv, rx_intr);
	/* the rx/tx status bits were already cleared on entry above;
	 * writing them again here based on the stale entry snapshot
	 * would also clear completion events that arrived *during* this
	 * poll and whose descriptors have not been processed yet
	 */

	if (unlikely(fe_status & status_intr)) {
		if (hwstat && spin_trylock(&hwstat->stats_lock)) {
			fe_stats_update(priv);
			spin_unlock(&hwstat->stats_lock);
			/* only acknowledge the counter-full event once the
			 * counters have actually been folded in; otherwise
			 * this increment is lost forever
			 */
			fe_reg_w32(status_intr, status_reg);
		}
	}

	if (unlikely(netif_msg_intr(priv))) {
		mask = fe_reg_r32(FE_REG_FE_INT_ENABLE);
		netdev_info(priv->netdev,
			    "done tx %d, rx %d, intr 0x%08x/0x%x\n",
			    tx_done, rx_done, status, mask);
	}

	if (!tx_again && (rx_done < budget)) {
		if (likely(napi_complete_done(napi, rx_done) && !test_bit(__FE_DOWN, &priv->state))) {
			fe_int_enable(priv, tx_intr | rx_intr);
		}
	} else if (tx_again) {
		/* report the full budget only to keep NAPI polling for
		 * the pending TX reclaim; the real rx_done is what it is
		 */
		return budget;
	}

	return rx_done;
}

static void fe_tx_timeout(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_tx_ring *ring = &priv->tx_ring;

	priv->netdev->stats.tx_errors++;
	netif_err(priv, tx_err, dev,
		  "transmit timed out\n");
	netif_info(priv, drv, dev, "dma_cfg:%08x\n",
		   fe_reg_r32(FE_REG_PDMA_GLO_CFG));
	netif_info(priv, drv, dev, "tx_ring=%d, "
		   "base=%08x, max=%u, ctx=%u, dtx=%u, fdx=%hu, next=%hu\n",
		   0, fe_reg_r32(FE_REG_TX_BASE_PTR0),
		   fe_reg_r32(FE_REG_TX_MAX_CNT0),
		   fe_reg_r32(FE_REG_TX_CTX_IDX0),
		   fe_reg_r32(FE_REG_TX_DTX_IDX0),
		   ring->tx_free_idx,
		   ring->tx_next_idx);
	netif_info(priv, drv, dev,
		   "rx_ring=%d, base=%08x, max=%u, calc=%u, drx=%u\n",
		   0, fe_reg_r32(FE_REG_RX_BASE_PTR0),
		   fe_reg_r32(FE_REG_RX_MAX_CNT0),
		   fe_reg_r32(FE_REG_RX_CALC_IDX0),
		   fe_reg_r32(FE_REG_RX_DRX_IDX0));

	if (!test_and_set_bit(FE_FLAG_RESET_PENDING, priv->pending_flags))
		schedule_work(&priv->pending_work);
}

static irqreturn_t fe_handle_irq(int irq, void *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 status, int_mask;

	status = fe_reg_r32(FE_REG_FE_INT_STATUS);

	if (unlikely(!status))
		return IRQ_NONE;

	int_mask = (priv->soc->rx_int | priv->soc->tx_int);
	if (likely(status & int_mask)) {
		if (likely(napi_schedule_prep(&priv->rx_napi))) {
			fe_int_disable(priv, int_mask);
			__napi_schedule(&priv->rx_napi);
		}
	} else {
		fe_reg_w32(status, FE_REG_FE_INT_STATUS);
	}

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void fe_poll_controller(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 int_mask = priv->soc->tx_int | priv->soc->rx_int;

	fe_int_disable(priv, int_mask);
	fe_handle_irq(dev->irq, dev);
	fe_int_enable(priv, int_mask);
}
#endif

int fe_set_clock_cycle(struct fe_priv *priv)
{
	unsigned long sysclk = priv->sysclk;

	sysclk /= FE_US_CYC_CNT_DIVISOR;
	sysclk <<= FE_US_CYC_CNT_SHIFT;

	fe_w32((fe_r32(FE_FE_GLO_CFG) &
			~(FE_US_CYC_CNT_MASK << FE_US_CYC_CNT_SHIFT)) |
			sysclk,
			FE_FE_GLO_CFG);
	return 0;
}

void fe_fwd_config(struct fe_priv *priv)
{
	u32 fwd_cfg;

	fwd_cfg = fe_r32(FE_GDMA1_FWD_CFG);

	/* disable jumbo frame */
	if (priv->flags & FE_FLAG_JUMBO_FRAME)
		fwd_cfg &= ~FE_GDM1_JMB_EN;

	/* set unicast/multicast/broadcast frame to cpu */
	fwd_cfg &= ~0xffff;

	fe_w32(fwd_cfg, FE_GDMA1_FWD_CFG);
}

static void fe_rxcsum_config(bool enable)
{
	if (enable)
		fe_w32(fe_r32(FE_GDMA1_FWD_CFG) | (FE_GDM1_ICS_EN |
					FE_GDM1_TCS_EN | FE_GDM1_UCS_EN),
				FE_GDMA1_FWD_CFG);
	else
		fe_w32(fe_r32(FE_GDMA1_FWD_CFG) & ~(FE_GDM1_ICS_EN |
					FE_GDM1_TCS_EN | FE_GDM1_UCS_EN),
				FE_GDMA1_FWD_CFG);
}

static void fe_txcsum_config(bool enable)
{
	if (enable)
		fe_w32(fe_r32(FE_CDMA_CSG_CFG) | (FE_ICS_GEN_EN |
					FE_TCS_GEN_EN | FE_UCS_GEN_EN),
				FE_CDMA_CSG_CFG);
	else
		fe_w32(fe_r32(FE_CDMA_CSG_CFG) & ~(FE_ICS_GEN_EN |
					FE_TCS_GEN_EN | FE_UCS_GEN_EN),
				FE_CDMA_CSG_CFG);
}

void fe_csum_config(struct fe_priv *priv)
{
	struct net_device *dev = priv_netdev(priv);

	fe_txcsum_config((dev->features &
			  (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM)) != 0);
	fe_rxcsum_config((dev->features & NETIF_F_RXCSUM));
}

static int fe_hw_init(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	int i, err;

	err = request_irq(dev->irq, fe_handle_irq, 0, dev_name(priv->dev), dev);
	if (err)
		return err;

	if (priv->soc->set_mac)
		priv->soc->set_mac(priv, dev->dev_addr);
	else
		fe_hw_set_macaddr(priv, dev->dev_addr);

	/* disable delay interrupt */
	fe_reg_w32(0, FE_REG_DLY_INT_CFG);

	/* disable all interrupts during hw init */
	fe_int_disable(priv, ~0);

	/* frame engine will push VLAN tag regarding to VIDX feild in Tx desc */
	if (fe_reg_table[FE_REG_FE_DMA_VID_BASE])
		for (i = 0; i < 16; i += 2)
			fe_w32(((i + 1) << 16) + i,
			       fe_reg_table[FE_REG_FE_DMA_VID_BASE] +
			       (i * 2));

	err = priv->soc->fwd_config(priv);
	if (err) {
		netdev_err(dev, "unable to configure frame engine\n");
		/* the irq above is ours to release: fe_init() cannot tell
		 * whether request_irq() or a later step failed
		 */
		free_irq(dev->irq, dev);
		return err;
	}

	if (fe_reg_table[FE_REG_FE_RST_GL]) {
		fe_reg_w32(1, FE_REG_FE_RST_GL);
		fe_reg_w32(0, FE_REG_FE_RST_GL);
	}

	return 0;
}

static int fe_open(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned long flags;
	u32 val;

	/* __FE_DOWN is only a private flag and fe_open() clears it, so
	 * it cannot by itself keep a ring-less device down: a failed
	 * fe_realloc_dma() rollback leaves tx_dma/rx_dma NULL and the
	 * next open (user, or the fe_tx_timeout() reset) would enable
	 * DMA on freed descriptor memory.
	 */
	if (!priv->tx_ring.tx_dma || !priv->rx_ring.rx_dma) {
		set_bit(__FE_DOWN, &priv->state);
		return -ENOMEM;
	}

	if (priv->phy)
		priv->phy->start(priv);

	/* fe_stop() quiesced the DMA engine; start it again */
	spin_lock_irqsave(&priv->page_lock, flags);
	val = FE_TX_WB_DDONE | FE_RX_DMA_EN | FE_TX_DMA_EN;
	if (priv->flags & FE_FLAG_RX_2B_OFFSET)
		val |= FE_RX_2B_OFFSET;
	val |= priv->soc->pdma_glo_cfg;
	fe_reg_w32(val, FE_REG_PDMA_GLO_CFG);
	spin_unlock_irqrestore(&priv->page_lock, flags);

	if (priv->soc->has_carrier && priv->soc->has_carrier(priv))
		netif_carrier_on(dev);

	clear_bit(__FE_DOWN, &priv->state);

	napi_enable(&priv->rx_napi);
	fe_int_enable(priv, priv->soc->tx_int | priv->soc->rx_int);
	netif_start_queue(dev);

	return 0;
}

static int fe_stop(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned long flags;
	int i;

	set_bit(__FE_DOWN, &priv->state);

	netif_tx_disable(dev);
	fe_int_disable(priv, priv->soc->tx_int | priv->soc->rx_int);
	/*
	 * Must be the synchronising variant: napi_disable_sync() does not
	 * exist on kernel 4.4, but 4.4's napi_disable() already sleeps
	 * until a poll running on another CPU has finished (it only
	 * became a separate _sync variant in much later kernels).
	 * Without the wait, fe_poll() could keep walking
	 * tx_dma/tx_buf/rx_dma/rx_data while unregister_netdev() calls
	 * fe_stop() and then ndo_uninit(), which frees exactly those
	 * rings.
	 */
	napi_disable(&priv->rx_napi);

	if (priv->phy)
		priv->phy->stop(priv);

	/* Quiesce the DMA engine before waiting for it: the busy bits
	 * never clear while the engine stays enabled, so polling them
	 * without clearing FE_TX_DMA_EN/FE_RX_DMA_EN first was a fake
	 * wait that always timed out after 200ms while the hardware kept
	 * writing to the rings the callers are about to free.
	 */
	spin_lock_irqsave(&priv->page_lock, flags);
	fe_reg_w32(fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
		     ~(FE_TX_WB_DDONE | FE_RX_DMA_EN | FE_TX_DMA_EN),
		     FE_REG_PDMA_GLO_CFG);
	spin_unlock_irqrestore(&priv->page_lock, flags);

	/* wait dma stop */
	for (i = 0; i < 10; i++) {
		if (fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
				(FE_TX_DMA_BUSY | FE_RX_DMA_BUSY)) {
			msleep(20);
			continue;
		}
		break;
	}

	return 0;
}

static void fe_unregister_switches(struct fe_priv *priv)
{
	int i;

	for (i = 0; i < priv->soc->gsw_dev_num; i++) {
		unregister_switch(priv->soc->gsw_dev[i]);
		priv->soc->gsw_dev[i] = NULL;
	}
	priv->soc->gsw_dev_num = 0;
}

static int __init fe_init(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct device_node *port;
	const char *mac_addr;
	unsigned long flags;
	u32 val;
	int err;

	priv->soc->reset_fe();

	if (priv->soc->switch_init) {
		int swerr = priv->soc->switch_init(priv);

		if (swerr) {
			netdev_err(dev, "failed to initialize switch core: %d\n",
				   swerr);
			/* propagate -EPROBE_DEFER instead of collapsing it
			 * into -ENODEV: a switch that has not probed yet
			 * must not permanently kill this device
			 */
			return swerr;
		}
	}

	mac_addr = of_get_mac_address(priv->dev->of_node);
	if (mac_addr)
		ether_addr_copy(dev->dev_addr, mac_addr);

	/* If the mac address is invalid, use random mac address  */
	if (!is_valid_ether_addr(dev->dev_addr)) {
		eth_hw_addr_random(dev);
		dev_err(priv->dev, "generated random MAC address %pM\n",
			dev->dev_addr);
	}

	err = fe_mdio_init(priv);
	if (err)
		goto err_switch_cleanup;

	if (priv->soc->port_init)
		for_each_child_of_node(priv->dev->of_node, port)
			if (of_device_is_compatible(port, "mediatek,eth-port") &&
			    of_device_is_available(port))
				priv->soc->port_init(priv, port);

	if (priv->phy) {
		err = priv->phy->connect(priv);
		if (err)
			goto err_phy_disconnect;
	}

	err = fe_hw_init(dev);
	if (err)
		goto err_phy_disconnect;

	if ((priv->flags & FE_FLAG_HAS_SWITCH) && priv->soc->switch_config)
		priv->soc->switch_config(priv);

	set_bit(__FE_DOWN, &priv->state);
	err = fe_init_dma(priv);
	if (err) {
		fe_free_dma(priv);
		/* fe_hw_init() requested the irq; ndo_uninit is not
		 * called when ndo_init fails, so release it here.
		 */
		free_irq(dev->irq, dev);
		goto err_phy_disconnect;
	}

	spin_lock_irqsave(&priv->page_lock, flags);
	val = FE_TX_WB_DDONE | FE_RX_DMA_EN | FE_TX_DMA_EN;
	if (priv->flags & FE_FLAG_RX_2B_OFFSET)
		val |= FE_RX_2B_OFFSET;
	val |= priv->soc->pdma_glo_cfg;
	fe_reg_w32(val, FE_REG_PDMA_GLO_CFG);
	spin_unlock_irqrestore(&priv->page_lock, flags);

	return 0;

err_phy_disconnect:
	if (priv->phy)
		priv->phy->disconnect(priv);
	fe_mdio_cleanup(priv);
err_switch_cleanup:
	/* switch_init() may already have registered switch devices
	 * (gsw_dev[]): leaving them registered while this probe fails
	 * would let swconfig operations run on devm-allocated private
	 * data that is released when the probe unwinds (UAF).
	 */
	if (priv->soc->switch_cleanup)
		priv->soc->switch_cleanup(priv);
	fe_unregister_switches(priv);

	return err;
}

static void fe_uninit(struct net_device *dev)
{
	unsigned long flags;
	struct fe_priv *priv = netdev_priv(dev);
	int i;

	spin_lock_irqsave(&priv->page_lock, flags);
	fe_reg_w32(fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
		     ~(FE_TX_WB_DDONE | FE_RX_DMA_EN | FE_TX_DMA_EN),
		     FE_REG_PDMA_GLO_CFG);
	spin_unlock_irqrestore(&priv->page_lock, flags);

	/* the engine does not stop the instant the enable bits clear:
	 * wait for it to go quiet before releasing the rings it may
	 * still be writing to
	 */
	for (i = 0; i < 10; i++) {
		if (!(fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
		      (FE_TX_DMA_BUSY | FE_RX_DMA_BUSY)))
			break;
		msleep(20);
	}

	/* cut the interrupt sources before the lengthy cleanup below:
	 * phy disconnect and MDIO teardown can take long enough for the
	 * irq handler to run against half-torn-down state
	 */
	fe_reg_w32(0, FE_REG_FE_INT_ENABLE);
	free_irq(dev->irq, dev);

	/* release the switch irq requested with this fe_priv as dev_id
	 * before this instance goes away
	 */
	if (priv->soc->switch_cleanup)
		priv->soc->switch_cleanup(priv);

	fe_free_dma(priv);

	if (priv->phy)
		priv->phy->disconnect(priv);
	fe_mdio_cleanup(priv);
}

static int fe_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct fe_priv *priv = netdev_priv(dev);

	if (!priv->phy_dev)
		return -ENODEV;


	return phy_mii_ioctl(priv->phy_dev, ifr, cmd);
}

int fe_realloc_dma(struct fe_priv *priv, u16 new_tx_size, u16 new_rx_size,
		   u16 new_frag, u16 new_buf)
{
	bool was_running = netif_running(priv->netdev);
	u16 old_tx = priv->tx_ring.tx_ring_size;
	u16 old_rx = priv->rx_ring.rx_ring_size;
	u16 old_frag = priv->rx_ring.frag_size;
	u16 old_buf = priv->rx_ring.rx_buf_size;
	int err;

	if (was_running) {
		fe_stop(priv->netdev);
	} else {
		/* The DMA engine may still be running here: it is enabled
		 * in fe_init() (ndo_init) and only stopped again by
		 * fe_stop()/fe_uninit(). Quiesce it before freeing the
		 * rings the hardware may still be writing to.
		 */
		unsigned long flags;
		int i;

		spin_lock_irqsave(&priv->page_lock, flags);
		fe_reg_w32(fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
			   ~(FE_TX_WB_DDONE | FE_RX_DMA_EN | FE_TX_DMA_EN),
			   FE_REG_PDMA_GLO_CFG);
		spin_unlock_irqrestore(&priv->page_lock, flags);
		for (i = 0; i < 10; i++) {
			if (!(fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
			      (FE_TX_DMA_BUSY | FE_RX_DMA_BUSY)))
				break;
			msleep(20);
		}
	}

	/* free the old rings while rx_buf_size still describes their
	 * mapping; only then switch to the new buffer geometry
	 */
	fe_free_dma(priv);

	priv->tx_ring.tx_ring_size = new_tx_size;
	priv->rx_ring.rx_ring_size = new_rx_size;
	priv->rx_ring.frag_size = new_frag;
	priv->rx_ring.rx_buf_size = new_buf;

	err = fe_init_dma(priv);
	if (err) {
		/*
		 * The failed attempt may have allocated one of the two
		 * rings before failing on the other. Release it before
		 * reinitialising, otherwise kcalloc()/dma_alloc_coherent()
		 * run again on top of the still-live pointers and the
		 * previous tx_buf/tx_dma/rx_data/rx_dma are lost.
		 */
		fe_free_dma(priv);

		priv->tx_ring.tx_ring_size = old_tx;
		priv->rx_ring.rx_ring_size = old_rx;
		priv->rx_ring.frag_size = old_frag;
		priv->rx_ring.rx_buf_size = old_buf;
		if (fe_init_dma(priv)) {
			netif_err(priv, tx_err, priv->netdev,
				  "DMA realloc rollback failed\n");
			/* the rollback itself may die halfway: release
			 * whatever it allocated instead of leaking it,
			 * and keep the device down -- there are no rings
			 * to receive into or transmit from anymore
			 */
			fe_free_dma(priv);
			/* Keep the original geometry. A zero ring size is
			 * not a power of two, so a later fe_init_dma()
			 * would trip its WARN_ON_ONCE() and fail, and
			 * NEXT_*_DESP_IDX() would mask with (0 - 1) and
			 * walk off the NULL ring. fe_open() refuses to
			 * start DMA without rings, so the interface stays
			 * down instead of oopsing.
			 */
			priv->tx_ring.tx_ring_size = old_tx;
			priv->rx_ring.rx_ring_size = old_rx;
			priv->rx_ring.frag_size = old_frag;
			priv->rx_ring.rx_buf_size = old_buf;
			set_bit(__FE_DOWN, &priv->state);
			return err;
		}
	}

	if (was_running) {
		int oerr = fe_open(priv->netdev);

		if (oerr && !err)
			err = oerr;
	}

	return err;
}

static void fe_jumbo_config(struct net_device *dev, int frag_size)
{
	u32 fwd_cfg;

	if (IS_ENABLED(CONFIG_SOC_MT7621))
		return;

	fwd_cfg = fe_r32(FE_GDMA1_FWD_CFG);
	if (dev->mtu <= ETH_DATA_LEN) {
		fwd_cfg &= ~FE_GDM1_JMB_EN;
	} else {
		fwd_cfg &= ~(FE_GDM1_JMB_LEN_MASK << FE_GDM1_JMB_LEN_SHIFT);
		fwd_cfg |= (DIV_ROUND_UP(frag_size, 1024) <<
				FE_GDM1_JMB_LEN_SHIFT) | FE_GDM1_JMB_EN;
	}
	fe_w32(fwd_cfg, FE_GDMA1_FWD_CFG);
}

static int fe_change_mtu(struct net_device *dev, int new_mtu)
{
	struct fe_priv *priv = netdev_priv(dev);
	int frag_size, old_mtu, err;
	u16 new_frag, new_buf;

	if (!(priv->flags & FE_FLAG_JUMBO_FRAME))
		return eth_change_mtu(dev, new_mtu);

	if (IS_ENABLED(CONFIG_SOC_MT7621))
		if (new_mtu > 2048)
			return -EINVAL;

	frag_size = fe_max_frag_size(new_mtu);
	/* 68 is the minimum payload (ETH_MIN_MTU does not exist in the
	 * 4.4 headers); it already accounts for the minimum
	 * 802.1q-tagged frame the MAC must transport.
	 * The upper bound is implicit: frag_size is capped by PAGE_SIZE
	 * (RX buffers come from page frags), so MTUs beyond roughly
	 * PAGE_SIZE - overhead are rejected here rather than by an
	 * explicit limit; MT7621 adds its own stricter cap above.
	 */
	if (new_mtu < 68 || frag_size > PAGE_SIZE)
		return -EINVAL;

	old_mtu = dev->mtu;
	dev->mtu = new_mtu;

	/* return early if the buffer sizes will not change */
	if (old_mtu <= ETH_DATA_LEN && new_mtu <= ETH_DATA_LEN)
		return 0;
	if (old_mtu > ETH_DATA_LEN && new_mtu > ETH_DATA_LEN) {
		/* the rings stay as they are, but the hardware jumbo
		 * frame length must still track the new MTU
		 * (e.g. 9000 -> 2000)
		 */
		fe_jumbo_config(dev, frag_size);
		return 0;
	}

	if (new_mtu <= ETH_DATA_LEN)
		new_frag = fe_max_frag_size(ETH_DATA_LEN);
	else
		new_frag = PAGE_SIZE;
	new_buf = fe_max_buf_size(new_frag);

	/* Rebuild the rings in the !running case too: they were
	 * allocated by fe_init() with the old buffer geometry, and
	 * merely updating frag_size/rx_buf_size would leave every
	 * later unmap/build_skb disagreeing with what is actually
	 * mapped in the ring.
	 */
	err = fe_realloc_dma(priv, priv->tx_ring.tx_ring_size,
			     priv->rx_ring.rx_ring_size, new_frag, new_buf);
	if (err) {
		dev->mtu = old_mtu;
		return err;
	}

	/*
	 * Program the jumbo frame bits only once the reallocation has
	 * succeeded. They used to be written before it, and the error
	 * path rolled back dev->mtu and the ring sizes but left the
	 * hardware describing a frame size the driver no longer believes
	 * in.
	 */
	fe_jumbo_config(dev, frag_size);

	return 0;
}

static const struct net_device_ops fe_netdev_ops = {
	.ndo_init		= fe_init,
	.ndo_uninit		= fe_uninit,
	.ndo_open		= fe_open,
	.ndo_stop		= fe_stop,
	.ndo_start_xmit		= fe_start_xmit,
	.ndo_set_mac_address	= fe_set_mac_address,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= fe_do_ioctl,
	.ndo_change_mtu		= fe_change_mtu,
	.ndo_tx_timeout		= fe_tx_timeout,
	.ndo_get_stats64        = fe_get_stats64,
	.ndo_vlan_rx_add_vid	= fe_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= fe_vlan_rx_kill_vid,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= fe_poll_controller,
#endif
};

static void fe_reset_pending(struct fe_priv *priv)
{
	struct net_device *dev = priv->netdev;
	int err;

	/*
	 * The reset is queued by fe_tx_timeout(), which only fires while
	 * the interface is up, but this runs later from the workqueue.
	 * The interface may have been closed in between, and the fe_open()
	 * below would then clear __FE_DOWN, re-enable NAPI and restart the
	 * transmit queue for a device that is administratively down.
	 */
	/* the netif_running() check must happen under rtnl: tested
	 * locklessly, dev_close() could complete between the check and
	 * the reset, and the fe_open() below would then bring an
	 * administratively down device back up
	 */
	rtnl_lock();
	if (!netif_running(dev)) {
		rtnl_unlock();
		return;
	}
	fe_stop(dev);

	err = fe_open(dev);
	if (err) {
		netif_alert(priv, ifup, dev,
			    "Driver up/down cycle failed, closing device.\n");
		dev_close(dev);
	}
	rtnl_unlock();

	if (priv->soc->reset_ports)
		priv->soc->reset_ports(priv);
}

static const struct fe_work_t fe_work[] = {
	{FE_FLAG_RESET_PENDING, fe_reset_pending},
};

static void fe_pending_work(struct work_struct *work)
{
	struct fe_priv *priv = container_of(work, struct fe_priv, pending_work);
	int i;
	bool pending;

	for (i = 0; i < ARRAY_SIZE(fe_work); i++) {
		pending = test_and_clear_bit(fe_work[i].bitnr,
					     priv->pending_flags);
		if (pending)
			fe_work[i].action(priv);
	}

	/* a timeout arriving while an action ran re-set its bit after
	 * we cleared it; run again instead of dropping the request
	 */
	if (!bitmap_empty(priv->pending_flags, FE_FLAG_MAX))
		schedule_work(&priv->pending_work);
}

static int fe_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	const struct of_device_id *match;
	struct fe_soc_data *soc;
	struct net_device *netdev;
	struct fe_priv *priv;
	struct clk *sysclk;
	int err, napi_weight;

	device_reset(&pdev->dev);

	match = of_match_device(of_fe_match, &pdev->dev);
	if (!match)
		return -ENODEV;

	soc = (struct fe_soc_data *)match->data;

	/* validate the callbacks used unconditionally later on:
	 * reset_fe() and fwd_config() are called without a NULL check
	 * in fe_init()/fe_hw_init()
	 */
	if (!soc->reset_fe || !soc->fwd_config) {
		dev_err(&pdev->dev, "soc data misses required callbacks\n");
		return -ENODEV;
	}

	if (fe_base) {
		dev_err(&pdev->dev, "driver supports a single instance only\n");
		return -EBUSY;
	}

	if (soc->reg_table)
		fe_reg_table = soc->reg_table;
	else
		soc->reg_table = fe_reg_table;

	fe_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(fe_base)) {
		err = -EADDRNOTAVAIL;
		fe_base = NULL;
		goto err_out;
	}

	netdev = alloc_etherdev(sizeof(*priv));
	if (!netdev) {
		dev_err(&pdev->dev, "alloc_etherdev failed\n");
		err = -ENOMEM;
		goto err_iounmap;
	}

	/*
	 * Set priv up before any path that can reach err_free_dev:
	 * that label frees priv->hw_stats and puts priv->switch_np.
	 */
	priv = netdev_priv(netdev);
	spin_lock_init(&priv->page_lock);

	SET_NETDEV_DEV(netdev, &pdev->dev);
	netdev->netdev_ops = &fe_netdev_ops;
	netdev->base_addr = (unsigned long)fe_base;

	netdev->irq = platform_get_irq(pdev, 0);
	if (netdev->irq < 0) {
		dev_err(&pdev->dev, "no IRQ resource found\n");
		err = -ENXIO;
		goto err_free_dev;
	}

	if (soc->init_data)
		soc->init_data(soc, netdev);
	netdev->vlan_features = netdev->hw_features &
				~(NETIF_F_HW_VLAN_CTAG_TX |
				  NETIF_F_HW_VLAN_CTAG_RX);
	netdev->features |= netdev->hw_features;

	/* fake rx vlan filter func. to support tx vlan offload func */
	if (fe_reg_table[FE_REG_FE_DMA_VID_BASE])
		netdev->features |= NETIF_F_HW_VLAN_CTAG_FILTER;

	if (fe_reg_table[FE_REG_FE_COUNTER_BASE]) {
		priv->hw_stats = kzalloc(sizeof(*priv->hw_stats), GFP_KERNEL);
		if (!priv->hw_stats) {
			err = -ENOMEM;
			goto err_free_dev;
		}
		spin_lock_init(&priv->hw_stats->stats_lock);
		u64_stats_init(&priv->hw_stats->syncp);
	}

	sysclk = devm_clk_get(&pdev->dev, NULL);
	if (!IS_ERR(sysclk)) {
		priv->sysclk = clk_get_rate(sysclk);
	} else if ((priv->flags & FE_FLAG_CALIBRATE_CLK)) {
		dev_err(&pdev->dev, "this soc needs a clk for calibration\n");
		err = -ENXIO;
		goto err_free_dev;
	}

	priv->switch_np = of_parse_phandle(pdev->dev.of_node, "mediatek,switch", 0);
	if ((priv->flags & FE_FLAG_HAS_SWITCH) && !priv->switch_np) {
		dev_err(&pdev->dev, "failed to read switch phandle\n");
		err = -ENODEV;
		goto err_free_dev;
	}

	priv->netdev = netdev;
	priv->dev = &pdev->dev;
	priv->soc = soc;
	priv->msg_enable = netif_msg_init(fe_msg_level, FE_DEFAULT_MSG_ENABLE);
	priv->rx_ring.frag_size = fe_max_frag_size(ETH_DATA_LEN);
	priv->rx_ring.rx_buf_size = fe_max_buf_size(priv->rx_ring.frag_size);
	priv->tx_ring.tx_ring_size = NUM_DMA_DESC;
	priv->rx_ring.rx_ring_size = NUM_DMA_DESC;
	INIT_WORK(&priv->pending_work, fe_pending_work);

	napi_weight = 16;
	if (priv->flags & FE_FLAG_NAPI_WEIGHT) {
		napi_weight *= 4;
		priv->tx_ring.tx_ring_size *= 4;
		priv->rx_ring.rx_ring_size *= 4;
	}
	netif_napi_add(netdev, &priv->rx_napi, fe_poll, napi_weight);
	fe_set_ethtool_ops(netdev);

	err = register_netdev(netdev);
	if (err) {
		dev_err(&pdev->dev, "error bringing up device\n");
		goto err_napi_del;
	}

	platform_set_drvdata(pdev, netdev);

	netif_info(priv, probe, netdev, "mediatek frame engine at 0x%08lx, irq %d\n",
		   netdev->base_addr, netdev->irq);

	return 0;

err_napi_del:
	/* pairs with netif_napi_add(): free_netdev() does not remove
	 * the napi_struct from the poll list on this kernel
	 */
	netif_napi_del(&priv->rx_napi);
err_free_dev:
	kfree(priv->hw_stats);
	of_node_put(priv->switch_np);
	free_netdev(netdev);
err_iounmap:
	devm_iounmap(&pdev->dev, fe_base);
	/* leave no dangling state behind: a stale fe_base makes the next
	 * probe bail out with -EBUSY and points fe_r32/fe_w32 at an
	 * unmapped address
	 */
	fe_base = NULL;
	fe_reg_table = fe_reg_table_default;
err_out:
	return err;
}

static int fe_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct fe_priv *priv = netdev_priv(dev);

	fe_unregister_switches(priv);

	of_node_put(priv->switch_np);

	/* unregister first: while the device is still registered and up,
	 * a TX timeout may re-arm pending_work; cancelling the work
	 * before unregister_netdev() would let that work run after
	 * free_netdev() on an already freed net_device.
	 */
	unregister_netdev(dev);
	cancel_work_sync(&priv->pending_work);

	netif_napi_del(&priv->rx_napi);
	kfree(priv->hw_stats);
	free_netdev(dev);
	platform_set_drvdata(pdev, NULL);
	fe_base = NULL;
	fe_reg_table = fe_reg_table_default;

	return 0;
}

static struct platform_driver fe_driver = {
	.probe = fe_probe,
	.remove = fe_remove,
	.driver = {
		.name = "mtk_soc_eth",
		.owner = THIS_MODULE,
		.of_match_table = of_fe_match,
	},
};

module_platform_driver(fe_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("Ethernet driver for Ralink SoC");
MODULE_VERSION(MTK_FE_DRV_VERSION);
