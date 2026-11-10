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
#include <linux/atomic.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/switch.h>

#include <asm/mach-ralink/ralink_regs.h>

#include "mtk_eth_soc.h"
#include "mdio.h"
#include "ethtool.h"
#include "mt7530.h"

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

/* RSTCTRL is a shared system-controller register: fe_init() resets the
 * frame engine while the ESW/GSW drivers reset the switch or EPHY with
 * the same helper. Serialise the whole assert/deassert sequence, and
 * re-read the register before deasserting so changes made by another
 * driver during the delay are not written back from a stale snapshot.
 * All callers run in process context, so a mutex is safe across the
 * sleeps.
 */
static DEFINE_MUTEX(fe_reset_lock);

void fe_reset(u32 reset_bits)
{
	u32 t;

	mutex_lock(&fe_reset_lock);
	t = rt_sysc_r32(SYSC_REG_RSTCTRL);
	t |= reset_bits;
	rt_sysc_w32(t, SYSC_REG_RSTCTRL);
	usleep_range(10, 20);

	t = rt_sysc_r32(SYSC_REG_RSTCTRL);
	t &= ~reset_bits;
	rt_sysc_w32(t, SYSC_REG_RSTCTRL);
	usleep_range(10, 20);
	mutex_unlock(&fe_reset_lock);
}
/* esw_rt3050.o and gsw_mt7621.o are separate objects from
 * mtk-eth-soc.o, so the shared RSTCTRL helper must be exported even
 * though the subsystem is now built-in only.
 */
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

/* fe_stop() sets __FE_DOWN and then masks the rx/tx sources under
 * page_lock. A NAPI poll that already passed an unlocked state check
 * could otherwise re-enable those sources after that mask, leaving an
 * administratively down device with live interrupts and a disabled
 * NAPI. Re-check the state inside the same critical section that
 * serialises the register write: either this enable comes first and
 * fe_stop()'s disable is last, or the bit is already set and the
 * enable is skipped.
 */
static inline void fe_int_enable_if_running(struct fe_priv *priv, u32 mask)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->page_lock, flags);
	if (!test_bit(__FE_DOWN, &priv->state))
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
	/* mac[2] promotes to a signed int; shifting it by 24 is UB for
	 * values >= 0x80. Widen first so the whole expression stays u32.
	 */
	fe_w32(((u32)mac[2] << 24) | ((u32)mac[3] << 16) |
	       ((u32)mac[4] << 8) | mac[5], FE_GDMA1_MAC_ADRL);
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
	unsigned int len;

	if (mtu < 0)
		return -EINVAL;

	len = mtu;
	/* make sure buf_size will be at least MAX_RX_LENGTH */
	if (len < MAX_RX_LENGTH - FE_RX_ETH_HLEN)
		len = MAX_RX_LENGTH - FE_RX_ETH_HLEN;

	/* Keep the arithmetic unsigned: mtu + FE_RX_ETH_HLEN used to be a
	 * signed addition that overflowed for large MTUs, wrapped to a
	 * negative value and was then clamped back to the default 1500
	 * byte size. Callers bound the result by PAGE_SIZE, so the sum
	 * always fits the signed return type here.
	 */
	return (int)(SKB_DATA_ALIGN(FE_RX_HLEN + len) +
		     SKB_DATA_ALIGN(sizeof(struct skb_shared_info)));
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
	/* DMA publishes the payload words before setting RX_DMA_DONE.
	 * Sample the ownership word first and keep the remaining reads
	 * behind a barrier: a dma_rmb() placed after the whole copy
	 * cannot order reads that were already issued, so on a weakly
	 * ordered MIPS CPU rxd1/rxd3/rxd4 could still be stale.
	 */
	rxd->rxd2 = dma_rxd->rxd2;
	dma_rmb();
	rxd->rxd1 = dma_rxd->rxd1;
	rxd->rxd3 = dma_rxd->rxd3;
	rxd->rxd4 = dma_rxd->rxd4;
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
				/* Only dma_mapping_error() can tell whether a
				 * mapping failed: dma address 0 is legal and
				 * rxd1 == 0 is not a valid "unmapped" flag.
				 * Use the recorded mapping count instead so
				 * partial allocations are unwound exactly.
				 */
				if (ring->rx_dma && i < ring->rx_mapped)
					dma_unmap_single(priv->dev,
							 ring->rx_dma[i].rxd1,
							 ring->rx_buf_size,
							 DMA_FROM_DEVICE);
				skb_free_frag(ring->rx_data[i]);
			}

		kfree(ring->rx_data);
		ring->rx_data = NULL;
	}
	ring->rx_mapped = 0;

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

	/* RX_DMA_PLEN0 masks the buffer length down to 14 bits. A larger
	 * value would be silently truncated in every descriptor while
	 * dma_map_single() still maps the full rx_buf_size, so refuse the
	 * geometry instead of letting software and hardware disagree.
	 */
	if (WARN_ON_ONCE(ring->rx_buf_size > RX_DMA_PLEN0_LEN_MASK))
		return -EINVAL;

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
	ring->rx_mapped = 0;
	for (i = 0; i < ring->rx_ring_size; i++) {
		dma_addr_t dma_addr = dma_map_single(priv->dev,
				ring->rx_data[i] + NET_SKB_PAD + pad,
				ring->rx_buf_size,
				DMA_FROM_DEVICE);
		if (unlikely(dma_mapping_error(priv->dev, dma_addr)))
			goto no_rx_mem;
		ring->rx_dma[i].rxd1 = (unsigned int)dma_addr;
		ring->rx_mapped++;

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
		/* These counters are clear-on-read, which is why the
		 * values are accumulated with += instead of being read
		 * as an absolute total. A low-word wrap between the lo
		 * and hi reads still folds a carry into the hi word while
		 * lo holds the pre-wrap value, so this pair is not tear
		 * safe and can add a spurious 2^32 to the accumulator.
		 * Reversing the order or re-reading (hi/lo/hi) does not
		 * help: the second read returns an already cleared value
		 * and loses the whole delta. There is no software-visible
		 * snapshot latch here, so do not add a re-read loop
		 * without one.
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
	unsigned long flags;

	if (!base) {
		netdev_stats_to_stats64(storage, &dev->stats);
		return storage;
	}

	if (netif_running(dev) && netif_device_present(dev)) {
		/* Take the lock unconditionally: a failed trylock silently
		 * skipped the update, and increments sitting in the 32-bit
		 * hardware counters were lost once they wrapped. Use the
		 * IRQ-safe form like every other writer: fe_poll() can run
		 * from netpoll with hard IRQs disabled, and mixing _bh with
		 * plain spin_lock() on the same lock is a lockdep error and
		 * a hardirq/process self-deadlock on a UP target.
		 */
		spin_lock_irqsave(&hwstats->stats_lock, flags);
		fe_stats_update(priv);
		spin_unlock_irqrestore(&hwstats->stats_lock, flags);
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
		/* rx_errors is the total receive error count, not just one
		 * of its components. Reporting only the checksum failures
		 * made /proc/net/dev and 'ip -s' show zero errors on links
		 * with FCS, overflow or length errors.
		 */
		storage->rx_errors = hwstats->rx_checksum_errors +
			hwstats->rx_overflow + hwstats->rx_fcs_errors +
			hwstats->rx_short_errors + hwstats->rx_long_errors;
		storage->tx_aborted_errors = hwstats->tx_skip;
	} while (u64_stats_fetch_retry_irq(&hwstats->syncp, start));

	storage->tx_errors = priv->netdev->stats.tx_errors;
	storage->rx_dropped = priv->netdev->stats.rx_dropped;
	storage->tx_dropped = priv->netdev->stats.tx_dropped;

	return storage;
}

static void fe_vlan_write_slot(struct fe_priv *priv, u32 idx, u16 vid)
{
	u32 reg = fe_reg_table[FE_REG_FE_DMA_VID_BASE] + ((idx >> 1) << 2);
	u32 vlan_cfg = fe_r32(reg);

	if (idx & 0x1) {
		vlan_cfg &= 0xffff;
		vlan_cfg |= (u32)vid << 16;
	} else {
		vlan_cfg &= 0xffff0000;
		vlan_cfg |= vid;
	}
	fe_w32(vlan_cfg, reg);
}

static int fe_vlan_rx_add_vid(struct net_device *dev,
			      __be16 proto, u16 vid)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 idx = vid & 0xf;

	if (!fe_reg_table[FE_REG_FE_DMA_VID_BASE])
		return 0;

	/* active_vids is a VLAN_N_VID bitmap; vid is a u16 and is not
	 * guaranteed to be bounded by every ndo_vlan_rx_add_vid() caller.
	 * Reject it before set_bit() writes past the bitmap.
	 */
	if (vid >= VLAN_N_VID)
		return -EINVAL;

	set_bit(vid, priv->active_vids);

	/* Keep the hardware VID table in sync with active_vids even while
	 * TX VLAN offload is switched off. ndo_set_features() only
	 * reprograms the checksum and RX-VLAN bits, never these slots, so
	 * VIDs registered while the feature was off used to stay
	 * unprogrammed forever; when ethtool re-enabled the offload,
	 * fe_tx_map_dma() resolved VIDX through a stale table and frames
	 * got the wrong VLAN tag. A hash collision is the only case where
	 * a VID must stay without a slot; fe_fix_features() blocks the
	 * offload until it is gone.
	 */
	if (priv->vlan_offload_conflict)
		return 0;

	if (test_bit(idx, &priv->vlan_map)) {
		if (priv->vlan_vid[idx] == vid)
			return 0;

		/* The hardware resolves the VIDX field of txd4 through
		 * this slot, so two VIDs with the same hash cannot both
		 * be offloaded. Overwriting the slot would tag frames
		 * of one VLAN with the other VLAN's VID. Let the stack
		 * insert the tags in software until the collision is
		 * gone instead.
		 */
		netdev_warn(dev,
			    "TX VLAN offload disabled: VID %u collides with VID %u in slot %u\n",
			    vid, priv->vlan_vid[idx], idx);
		priv->vlan_offload_conflict = true;
		/* Never touch dev->wanted_features: it records the user's
		 * ethtool -K choice. fe_fix_features() strips the feature
		 * from dev->features while the conflict lasts, and clearing
		 * the flag plus netdev_update_features() on recovery lets
		 * that choice take effect again. Sampling the old value and
		 * OR-ing it back on recovery silently overrode an explicit
		 * "tx-vlan-offload off" issued during the conflict.
		 */
		netdev_update_features(dev);
		return 0;
	}

	fe_vlan_write_slot(priv, idx, vid);
	priv->vlan_vid[idx] = vid;
	set_bit(idx, &priv->vlan_map);

	return 0;
}

static int fe_vlan_rx_kill_vid(struct net_device *dev,
			       __be16 proto, u16 vid)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 idx = vid & 0xf;

	if (!fe_reg_table[FE_REG_FE_DMA_VID_BASE])
		return 0;

	if (vid >= VLAN_N_VID)
		return -EINVAL;

	clear_bit(vid, priv->active_vids);

	/* Clear the register slot as well as the bookkeeping bit, so a
	 * removed VID does not keep matching in hardware.
	 */
	if (test_bit(idx, &priv->vlan_map) && priv->vlan_vid[idx] == vid) {
		fe_vlan_write_slot(priv, idx, 0);
		clear_bit(idx, &priv->vlan_map);
	}

	/* VIDs registered while the feature was off were never
	 * programmed into a slot, so offload may only come back once
	 * every registered VID is gone; otherwise those frames would
	 * be mistagged by the stale table.
	 */
	if (priv->vlan_offload_conflict &&
	    bitmap_empty(priv->active_vids, VLAN_N_VID)) {
		/* Drop the block and let the core recompute dev->features
		 * from dev->wanted_features, which still reflects the
		 * user's most recent ethtool -K command.
		 */
		priv->vlan_offload_conflict = false;
		netdev_update_features(dev);
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
	int i, j, err = -EIO;

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
			err = -ENOMEM;
			goto err_out;
		}
		if (skb_shinfo(skb)->gso_type &
				(SKB_GSO_TCPV4 | SKB_GSO_TCPV6)) {
			st.txd.txd4 |= TX_DMA_TSO;
			tcp_hdr(skb)->check = htons(skb_shinfo(skb)->gso_size);
		}
	}

next_frag:
	if (skb_headlen(skb)) {
		err = fe_tx_dma_map_skb(ring, &st, skb);
		if (err)
			goto err_dma;
	}

	/* TX SG offload */
	nr_frags = skb_shinfo(skb)->nr_frags;
	for (i = 0; i < nr_frags; i++) {
		struct skb_frag_struct *frag;

		frag = &skb_shinfo(skb)->frags[i];
		err = fe_tx_dma_map_page(ring, &st, skb_frag_page(frag),
					 frag->page_offset,
					 skb_frag_size(frag));
		if (err)
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
	return err;
}

static inline int fe_skb_padto(struct sk_buff *skb, struct fe_priv *priv)
{
	unsigned int len;
	int ret;

	ret = 0;
	if (unlikely(skb->len < VLAN_ETH_ZLEN)) {
		/* FE_FLAG_PADDING_64B means the hardware pads short frames
		 * to 64 bytes by itself. With a working implementation
		 * (no FE_FLAG_PADDING_BUG) there is nothing to do here.
		 *
		 * The SoCs that set FE_FLAG_PADDING_BUG also set
		 * PADDING_64B. For them this function only corrects VLAN
		 * tagged frames; the final else deliberately leaves plain
		 * untagged short frames to the hardware. Software padding
		 * of those frames is NOT performed, so do not assume the
		 * PADDING_BUG flag means every short frame is grown here.
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
			/* skb_pad() already calls __skb_put(): it grows
			 * skb->len and advances skb->tail by the pad length
			 * while leaving data_len alone. Overwriting both
			 * fields with the total frame length makes
			 * skb_headlen() larger than the linear area that was
			 * actually padded whenever the skb has frags, so
			 * fe_tx_dma_map_skb() then DMA-maps past the end of
			 * the linear buffer and skb->len no longer equals
			 * headlen + data_len.
			 */
			ret = skb_pad(skb, len - skb->len);
			if (ret < 0) {
				/* skb_pad() frees the skb itself when
				 * pskb_expand_head() fails. Ownership has
				 * already been consumed: the caller must
				 * report the drop without freeing it.
				 */
				return ret;
			}
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
		/* The only non-zero return comes from skb_pad(), which
		 * already called kfree_skb() on this skb. Freeing it a
		 * second time corrupts the skbuff cache.
		 */
		return NETDEV_TX_OK;
	}

	/* sample the length after padding: fe_skb_padto() may have
	 * grown the skb
	 */
	len = skb->len;

	tx_num = fe_cal_txd_req(skb);
	if (unlikely(fe_empty_txd(ring) <= tx_num)) {
		netif_stop_queue(dev);
		/* fe_poll_tx() only wakes a queue that is already stopped.
		 * It can finish reclaiming the missing descriptors (and
		 * therefore skip the wake) between the check above and this
		 * stop. Pair the stop with the same barrier and re-check the
		 * successful mapping path uses, or the queue stays XOFF with
		 * no in-flight descriptors left to raise another TX interrupt.
		 */
		smp_mb();
		if (unlikely(fe_empty_txd(ring) > tx_num))
			netif_wake_queue(dev);
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
	netdev_features_t features = netdev->features;
	int idx = ring->rx_calc_idx;
	u32 checksum_bit;
	struct sk_buff *skb;
	u8 *data, *new_data;
	struct fe_rx_dma *rxd, trxd;
	int done = 0, pad;

	/* Sample the feature set once per poll batch. The core can commit
	 * a new dev->features while NAPI runs, and reading it separately
	 * for the checksum and RX-VLAN decisions could apply two different
	 * offload sets to the same skb.
	 */
	if (features & NETIF_F_RXCSUM)
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

		/* Never trust the hardware-reported length blindly: an
		 * oversized length could make skb_put() run past the end
		 * of the frag, and a length below ETH_HLEN would hand
		 * eth_type_trans()/napi_gro_receive() an skb too short to
		 * even hold an Ethernet header. Drop either case and
		 * recycle the old, still-mapped buffer instead.
		 */
		pktlen = RX_DMA_GET_PLEN0(trxd.rxd2);
		if (unlikely(pktlen < ETH_HLEN ||
			     pktlen > ring->rx_buf_size)) {
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

		if (features & NETIF_F_HW_VLAN_CTAG_RX &&
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
	/* Software indices are always kept modulo tx_ring_size by
	 * NEXT_TX_DESP_IDX(), but the raw hardware index is not bounded
	 * that way (e.g. while the engine is reset or reconfigured).
	 * Comparing an unmasked value against idx can never become false,
	 * so reclaim would walk on until it hits an empty slot, advance
	 * tx_free_idx to a bogus position and wedge fe_empty_txd()'s
	 * free-space accounting (and therefore the transmit queue).
	 */
	hwidx = fe_reg_r32(FE_REG_TX_DTX_IDX0) & (ring->tx_ring_size - 1);

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

	if (done)
		netdev_completed_queue(netdev, done, bytes_compl);

	/* A poll can also reclaim nothing but DUMMY descriptors: that still
	 * advances tx_free_idx and frees ring space. Gating the wake on
	 * done > 0 can leave a stopped queue with no completions left to
	 * wake it. Order the index store before the queue-state read.
	 */
	smp_mb();
	if (unlikely(netif_queue_stopped(netdev) &&
		     (fe_empty_txd(ring) > ring->tx_thresh)))
		netif_wake_queue(netdev);

	return done;
}

static int fe_poll(struct napi_struct *napi, int budget)
{
	struct fe_priv *priv = container_of(napi, struct fe_priv, rx_napi);
	struct fe_hw_stats *hwstat = priv->hw_stats;
	int tx_done, rx_done, tx_again;
	u32 status, fe_status, status_reg, mask;
	u32 tx_intr, rx_intr, status_intr;
	unsigned long flags;

	if (unlikely(test_bit(__FE_DOWN, &priv->state))) {
		napi_complete(napi);
		return 0;
	}

	/* status_int is never OR-ed into FE_REG_FE_INT_ENABLE, so the
	 * counter-full source cannot raise an IRQ on its own. The MIB
	 * counters are clear-on-read 32-bit values: if they are only
	 * folded when a user reads /proc/net/dev or ethtool -S, they wrap
	 * and the increment is lost. Fold them from NAPI at most once a
	 * second instead of enabling that interrupt source, which would
	 * need extra acknowledge paths in fe_handle_irq(), fe_stop() and
	 * the __FE_DOWN early return and could spin the IRQ line.
	 * Match the existing fold below: hwstat may be NULL and the
	 * IRQ-safe lock form is required for the netpoll context.
	 */
	if (hwstat && time_after(jiffies, priv->stats_jiffies)) {
		spin_lock_irqsave(&hwstat->stats_lock, flags);
		fe_stats_update(priv);
		spin_unlock_irqrestore(&hwstat->stats_lock, flags);
		priv->stats_jiffies = jiffies + HZ;
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

	/* fe_handle_irq() leaves counter-full bits pending in the primary
	 * FE_REG_FE_INT_STATUS snapshot, while on MT7620/MT7621 fe_status
	 * is re-read from FE_REG_FE_INT_STATUS2. Gate the fold/ack on the
	 * union of both snapshots: testing only fe_status made the primary
	 * register ack below unreachable in exactly the case it was added
	 * for, leaving that W1C source asserted forever.
	 */
	if (unlikely((fe_status | status) & status_intr)) {
		if (hwstat) {
			/* Take the lock unconditionally, like the two process
			 * context readers: fe_stats_update() folds
			 * clear-on-read 32-bit counters, so a failed trylock
			 * loses their increments once they wrap.
			 *
			 * netpoll calls this poll with hard IRQs disabled,
			 * so spin_lock() here could spin on a lock held by a
			 * preempted process-context writer on the same CPU.
			 * Use the same IRQ-safe form on all three writers.
			 */
			spin_lock_irqsave(&hwstat->stats_lock, flags);
			fe_stats_update(priv);
			spin_unlock_irqrestore(&hwstat->stats_lock, flags);
		}
		/* fe_handle_irq() deliberately leaves the counter-full bits
		 * pending for this poll. Acknowledge them even when there is
		 * no hw_stats object to fold (counter base == 0): otherwise
		 * the W1C source is never cleared and the level/shared line
		 * re-enters the handler forever. Acknowledge only the bits
		 * present in this snapshot, and only after the counters have
		 * been folded in, so increments arriving meanwhile are not
		 * discarded.
		 */
		fe_reg_w32(fe_status & status_intr, status_reg);

		/* On MT7620/MT7621 status_reg is FE_REG_FE_INT_STATUS2, but
		 * fe_handle_irq() reads and leaves the counter-full bits
		 * pending in the primary FE_REG_FE_INT_STATUS. Acknowledge
		 * the primary snapshot bits too, otherwise that W1C source
		 * has no clear path at all and a level/shared interrupt line
		 * keeps re-entering the handler. Only the bits from the entry
		 * snapshot are written, so events arriving during this poll
		 * are not discarded.
		 */
		if (status_reg != FE_REG_FE_INT_STATUS &&
		    (status & status_intr))
			fe_reg_w32(status & status_intr,
				   FE_REG_FE_INT_STATUS);
	}

	if (unlikely(netif_msg_intr(priv))) {
		mask = fe_reg_r32(FE_REG_FE_INT_ENABLE);
		netdev_info(priv->netdev,
			    "done tx %d, rx %d, intr 0x%08x/0x%x\n",
			    tx_done, rx_done, status, mask);
	}

	if (!tx_again && (rx_done < budget)) {
		if (likely(napi_complete_done(napi, rx_done)))
			fe_int_enable_if_running(priv, tx_intr | rx_intr);
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
		} else {
			/* NAPI is already scheduled, so this interrupt will
			 * not start a new poll that acknowledges the status
			 * bits on entry. Leaving a level-triggered source
			 * asserted re-enters the handler immediately -- in
			 * particular fe_poll_controller() re-enables the
			 * line unconditionally after calling us.
			 */
			fe_reg_w32(status & int_mask, FE_REG_FE_INT_STATUS);
		}
	} else {
		/* The counter-full sources are clear-on-write, but their
		 * 32-bit hardware counters are only folded into hw_stats by
		 * fe_poll(), which acknowledges those bits *after* the
		 * update. ACK-ing the whole snapshot here would throw the
		 * increments away (e.g. on a shared line or when netpoll
		 * calls this handler directly). Leave them pending for the
		 * next NAPI poll.
		 */
		fe_reg_w32(status & ~priv->soc->status_int,
			   FE_REG_FE_INT_STATUS);

		/* With no rx/tx source this interrupt has not started a
		 * poll, so a plain "leave them pending" has no next poll:
		 * when status consists only of counter-full bits the write
		 * above is zero, nothing is acknowledged, and a level or
		 * shared line re-enters the handler forever while the
		 * clear-on-read counters are never folded into hw_stats.
		 * Start the poll that fe_poll() uses to update and ACK
		 * them. If NAPI is already scheduled that poll will see
		 * the still-pending bits, so no second schedule is needed.
		 */
		if ((status & priv->soc->status_int) &&
		    likely(napi_schedule_prep(&priv->rx_napi))) {
			fe_int_disable(priv, int_mask);
			__napi_schedule(&priv->rx_napi);
		}
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
	/* Use the same __FE_DOWN check as fe_poll(): fe_stop() sets the bit
	 * and then masks these sources under page_lock. A plain enable here
	 * would undo that mask for an administratively down device, leaving
	 * live interrupts with NAPI disabled.
	 */
	fe_int_enable_if_running(priv, int_mask);
}
#endif

int fe_set_clock_cycle(struct fe_priv *priv)
{
	unsigned long sysclk = priv->sysclk;

	sysclk /= FE_US_CYC_CNT_DIVISOR;
	/* FE_US_CYC_CNT is an 8-bit field at FE_US_CYC_CNT_SHIFT. Only the
	 * masked low byte is cleared below, so a clock above 255 MHz (the
	 * usual case on RT305x/RT3883) would OR bits 16 and above into
	 * unrelated FE_FE_GLO_CFG configuration bits. Masking alone is not
	 * enough: wrapping the value silently (480 MHz becomes 224) makes
	 * the microsecond cycle count and delay interrupt run at a wrong
	 * rate with no diagnostic. Clamp to the largest representable
	 * value instead of truncating.
	 */
	if (sysclk > FE_US_CYC_CNT_MASK) {
		dev_warn(priv->dev,
			 "sysclk %lu MHz exceeds the 8-bit US_CYC_CNT field, clamping to %u\n",
			 sysclk, (unsigned)FE_US_CYC_CNT_MASK);
		sysclk = FE_US_CYC_CNT_MASK;
	}
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
	fe_txcsum_config((priv->fwd_features &
			  (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM)) != 0);
	fe_rxcsum_config((priv->fwd_features & NETIF_F_RXCSUM));
}

static int fe_hw_init(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	int i, err;

	/* Mask every source before the handler becomes live. ndo_init can
	 * run again on the same netdev, and INT_ENABLE may still carry the
	 * rx/tx bits programmed by the previous instance's fe_open(). With
	 * NAPI still disabled, napi_schedule_prep() cannot succeed, so an
	 * asserted level line (in particular a pending counter-full bit)
	 * would re-enter fe_handle_irq() with no poll able to acknowledge
	 * it until the mask below is reached.
	 */
	fe_int_disable(priv, ~0);

	err = request_irq(dev->irq, fe_handle_irq, 0, dev_name(priv->dev), dev);
	if (err)
		return err;

	if (priv->soc->set_mac)
		priv->soc->set_mac(priv, dev->dev_addr);
	else
		fe_hw_set_macaddr(priv, dev->dev_addr);

	/* disable delay interrupt */
	fe_reg_w32(0, FE_REG_DLY_INT_CFG);

	/* frame engine will push VLAN tag regarding to VIDX feild in Tx desc */
	if (fe_reg_table[FE_REG_FE_DMA_VID_BASE]) {
		for (i = 0; i < 16; i += 2)
			fe_w32(((i + 1) << 16) + i,
			       fe_reg_table[FE_REG_FE_DMA_VID_BASE] +
			       (i * 2));

		/* This table is rewritten from scratch on every ndo_init,
		 * but ndo_init can run again on the same netdev after an
		 * unregister/re-register cycle. The software mirror is only
		 * zeroed by alloc_etherdev(), so stale slot bits would make
		 * fe_vlan_rx_add_vid() decide a VID is already programmed
		 * and skip fe_vlan_write_slot(). For any VID whose value
		 * differs from its (vid & 0xf) slot the hardware then holds
		 * the reset value and frames are tagged with the wrong VID.
		 */
		priv->vlan_map = 0;
		memset(priv->vlan_vid, 0, sizeof(priv->vlan_vid));

		/* active_vids and vlan_offload_conflict are private to
		 * this driver: the VLAN core only re-issues add_vid, it
		 * never clears them. Leaving a stale conflict behind makes
		 * every add_vid return without programming a slot, makes
		 * fe_fix_features strip NETIF_F_HW_VLAN_CTAG_TX forever,
		 * and blocks the recovery condition in
		 * fe_vlan_rx_kill_vid() because active_vids is never empty.
		 * Restore the fresh state. Do not compensate
		 * dev->wanted_features here: an unconditional OR would
		 * re-enable offload against an explicit ethtool -K off
		 * and, without netdev_update_features(), would leave
		 * wanted_features and features diverged.
		 */
		bitmap_clear(priv->active_vids, 0, VLAN_N_VID);
		priv->vlan_offload_conflict = false;
	}

	/* Seed the hardware feature snapshot before the first
	 * fwd_config(): the helpers no longer read dev->features, which
	 * the core may still change after ndo_set_features() returns.
	 */
	priv->fwd_features = dev->features;
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

	/* A netdev starts with the carrier up. Only calling
	 * netif_carrier_on() leaves LOWER_UP asserted when every switch
	 * port is down, and mt7620_handle_carrier() returns immediately
	 * on switch-only boards with no priv->phy to correct it later.
	 * Keep the SoC-specific carrier source authoritative in both
	 * directions; SoCs without has_carrier keep their PHY-driven
	 * (or default) state untouched.
	 */
	if (priv->soc->has_carrier) {
		if (priv->soc->has_carrier(priv))
			netif_carrier_on(dev);
		else
			netif_carrier_off(dev);
	}

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

	/* ndo_stop can be entered twice for one shutdown: both
	 * fe_reset_pending() and the catastrophic fe_realloc_dma() rollback
	 * call fe_stop() themselves and then call dev_close(), which sees
	 * IFF_UP still set (fe_stop() never clears it) and invokes ndo_stop
	 * again. Running napi_disable()/netif_tx_disable()/DMA quiescing a
	 * second time without a matching fe_open() is not balanced. Make
	 * the teardown one-shot using the same __FE_DOWN bit fe_open()
	 * clears on a successful start.
	 */
	if (test_and_set_bit(__FE_DOWN, &priv->state))
		return 0;

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
		if (!(fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
				(FE_TX_DMA_BUSY | FE_RX_DMA_BUSY)))
			break;
		msleep(20);
	}
	/* The loop used to fall through after ten iterations without
	 * checking the result. Callers free the descriptor rings right
	 * after fe_stop(), so a wedged engine would keep writing into
	 * memory that has already been unmapped or reallocated.
	 */
	if (i == 10 &&
	    (fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
			(FE_TX_DMA_BUSY | FE_RX_DMA_BUSY))) {
		netdev_err(dev, "DMA engine did not quiesce, rings kept\n");
		return -EBUSY;
	}

	return 0;
}

/* soc->swpriv points at a devm-owned GSW object that is freed as soon
 * as the GSW platform driver's remove() returns. Its runtime readers
 * (MDIO callbacks, has_carrier, set_mac, port_init, reset_ports) cannot
 * take gsw_teardown_lock: mdiobus_read() already holds bus->mdio_lock
 * before calling bus->read, while mt7621_gsw_set_imr() takes
 * gsw_teardown_lock -> bus->mdio_lock. Adding the teardown lock to
 * those readers would create an ABBA.
 *
 * Use a lock-free reader count plus a closing gate. remove() raises
 * the gate before clearing swpriv, releases the teardown lock, then
 * waits for the count to drain before returning to devres. Raising
 * the gate is essential: without it a reader that starts only after
 * the wait has returned is not counted at all, and its safety would
 * depend solely on observing the cleared pointer rather than on the
 * drain itself. Readers only do atomic operations, never take a mutex.
 */
static atomic_t fe_gsw_readers = ATOMIC_INIT(0);
static atomic_t fe_gsw_closing = ATOMIC_INIT(0);
static DECLARE_WAIT_QUEUE_HEAD(fe_gsw_drain_wq);

void fe_gsw_reader_exit(void)
{
	if (atomic_dec_and_test(&fe_gsw_readers))
		wake_up(&fe_gsw_drain_wq);
}
EXPORT_SYMBOL_GPL(fe_gsw_reader_exit);

bool fe_gsw_reader_enter(void)
{
	if (atomic_read(&fe_gsw_closing))
		return false;

	atomic_inc(&fe_gsw_readers);
	/* This is a Dekker/store-buffering pattern with
	 * fe_gsw_readers_close(): each side needs a full barrier between
	 * its own store and its read of the other side. close() has an
	 * smp_mb() after its atomic_set; without the matching barrier here
	 * a weakly ordered CPU may let the second read of closing complete
	 * before the reader count is globally visible. remove() would then
	 * see count == 0, return to devres, and this reader could still
	 * report success and touch the released GSW.
	 */
	smp_mb();
	/* close() may have run between the first check and the
	 * increment. A reader counted after the drain started is not
	 * waited for, so undo the count and refuse to touch the GSW.
	 * The double check is what makes "count reached zero" imply
	 * "no reader can still enter".
	 */
	if (atomic_read(&fe_gsw_closing)) {
		fe_gsw_reader_exit();
		return false;
	}
	return true;
}
EXPORT_SYMBOL_GPL(fe_gsw_reader_enter);

void fe_gsw_readers_close(void)
{
	atomic_set(&fe_gsw_closing, 1);
	smp_mb();
}
EXPORT_SYMBOL_GPL(fe_gsw_readers_close);

void fe_gsw_readers_open(void)
{
	atomic_set(&fe_gsw_closing, 0);
	smp_mb();
}
EXPORT_SYMBOL_GPL(fe_gsw_readers_open);

void fe_gsw_drain_readers(void)
{
	wait_event(fe_gsw_drain_wq, atomic_read(&fe_gsw_readers) == 0);
}
EXPORT_SYMBOL_GPL(fe_gsw_drain_readers);

/* gsw_dev[] and gsw_dev_num live in the module-wide fe_soc_data. They
 * are touched by three independent teardown paths (fe_remove, fe_uninit
 * and fe_init's rollback, plus the two GSW platform drivers' remove())
 * and by the switch_config probe path. Those platform devices have
 * separate device locks, so two of these paths can run at the same time.
 */
static DEFINE_MUTEX(fe_switch_lock);

/* Called by mtk_gsw_init() from inside gsw_teardown_lock on its
 * successful exit, before switch_config() may publish any switch_dev.
 * A full teardown sets switch_dying in the same critical section that
 * claims gsw_dev[], so a new registration round has to clear it.
 * Keeping this call under gsw_teardown_lock is essential: an
 * independent GSW unbind takes that lock to publish switch_dying and
 * drop swpriv. If fe_init() cleared the flag after switch_init()
 * released the lock, a concurrent unbind's claim would be erased and
 * a switch_dev could be published against a detached GSW.
 */
void fe_switch_mark_alive(struct fe_priv *priv)
{
	mutex_lock(&fe_switch_lock);
	priv->soc->switch_dying = false;
	mutex_unlock(&fe_switch_lock);
}
EXPORT_SYMBOL_GPL(fe_switch_mark_alive);

int fe_switch_start(struct fe_priv *priv)
{
	int start;

	/* Every writer of gsw_dev_num holds fe_switch_lock. The MT7620
	 * config path needs a consistent snapshot to know which entries
	 * belong to its own two probes, so it must read under that lock
	 * too instead of racing a concurrent teardown.
	 */
	mutex_lock(&fe_switch_lock);
	start = priv->soc->gsw_dev_num;
	mutex_unlock(&fe_switch_lock);

	return start;
}
EXPORT_SYMBOL_GPL(fe_switch_start);

int fe_probe_and_register_switch(struct fe_priv *priv, void __iomem *base,
				 struct mii_bus *bus, int vlan)
{
	struct switch_dev *swdev;
	int err = -ENOSPC;

	/* Keep the whole probe and its publication inside one critical
	 * section. If the GSW device is unbound while mt7530_probe() is
	 * still calling register_switch(), the remove path must be able
	 * to claim the new entry as soon as it appears; otherwise a
	 * switch_dev pointing into the released devm mapping survives.
	 */
	mutex_lock(&fe_switch_lock);
	/* A GSW unbind may have won this lock while the probe was waiting.
	 * Its claim-and-clear found nothing to remove, it then dropped
	 * swpriv and returned, and devres released gsw/gsw->base. The
	 * base/bus pointers this probe carries are already stale, and no
	 * later teardown path is guaranteed to run for a surviving FE.
	 * switch_dying is raised in that same claim critical section, so
	 * refuse to publish a switch_dev that would stay on the global
	 * swconfig list forever. mt7620_gsw_config()/mt7621_gsw_config()
	 * propagate this error out of fe_init().
	 */
	if (priv->soc->switch_dying) {
		err = -ENODEV;
		goto out_unlock_probe;
	}
	if (priv->soc->gsw_dev_num < ARRAY_SIZE(priv->soc->gsw_dev)) {
		swdev = mt7530_probe(priv->dev, base, bus, vlan);
		if (IS_ERR(swdev))
			err = PTR_ERR(swdev);
		else {
			priv->soc->gsw_dev[priv->soc->gsw_dev_num++] = swdev;
			err = 0;
		}
	}
out_unlock_probe:
	mutex_unlock(&fe_switch_lock);

	return err;
}
EXPORT_SYMBOL_GPL(fe_probe_and_register_switch);

static void fe_unregister_switches_range(struct fe_priv *priv, int start,
					 bool full_teardown)
{
	struct switch_dev *claimed[ARRAY_SIZE(priv->soc->gsw_dev)];
	int i;

	/* Claim-and-clear under the lock so a concurrent teardown sees
	 * NULL instead of a pointer that is about to be freed. The actual
	 * removal runs outside the lock: unregister_switch() drains
	 * sleeping swconfig callbacks and must not hold fe_switch_lock
	 * while a probe is waiting for it.
	 */
	mutex_lock(&fe_switch_lock);
	/* Only a real teardown declares the switch gone for good. This
	 * used to be inferred from start == 0, but the first probe of a
	 * fresh MT7620 also starts at 0: a failed second probe then ran
	 * a partial rollback that set switch_dying and permanently
	 * blocked any switch_config retry. Keep the two meanings
	 * separate and publish the flag in the same critical section as
	 * the claim, so a probe already waiting on fe_switch_lock cannot
	 * repopulate gsw_dev[] after we release it.
	 */
	if (full_teardown)
		priv->soc->switch_dying = true;
	for (i = 0; i < ARRAY_SIZE(priv->soc->gsw_dev); i++) {
		claimed[i] = (i >= start) ? priv->soc->gsw_dev[i] : NULL;
		if (i >= start)
			priv->soc->gsw_dev[i] = NULL;
	}
	if (start == 0)
		priv->soc->gsw_dev_num = 0;
	else if (priv->soc->gsw_dev_num > start)
		priv->soc->gsw_dev_num = start;
	mutex_unlock(&fe_switch_lock);

	for (i = 0; i < ARRAY_SIZE(priv->soc->gsw_dev); i++)
		if (claimed[i])
			mt7530_remove(claimed[i]);
}

void fe_unregister_switches_from(struct fe_priv *priv, int start)
{
	/* Undo one failed config attempt; a retry is still allowed. */
	fe_unregister_switches_range(priv, start, false);
}
EXPORT_SYMBOL_GPL(fe_unregister_switches_from);

void fe_unregister_switches(struct fe_priv *priv)
{
	/* Walk the whole array, not just gsw_dev_num: a failed probe can
	 * leave a registered entry behind a stale counter.
	 */
	fe_unregister_switches_range(priv, 0, true);
}
EXPORT_SYMBOL_GPL(fe_unregister_switches);

/* Defined below fe_init(); needed to replay the programmed jumbo state
 * after fe_hw_init() has cleared FE_GDM1_JMB_EN.
 */
static void fe_jumbo_config(struct net_device *dev, int frag_size);

/* ndo_init is reached through the persistent fe_netdev_ops table, so it
 * can run long after module init (deferred probe, unbind/rebind). It must
 * not be placed in .init.text, which is freed after boot.
 */
static int fe_init(struct net_device *dev)
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

		/* mtk_gsw_init() clears switch_dying on its successful
		 * exit while still holding gsw_teardown_lock. Do not do it
		 * here: that lock has been released, so a concurrent GSW
		 * unbind could publish switch_dying in between and have
		 * its claim silently erased.
		 */
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

	if ((priv->flags & FE_FLAG_HAS_SWITCH) && priv->soc->switch_config) {
		err = priv->soc->switch_config(priv);
		if (err) {
			netdev_err(dev, "failed to configure switch core: %d\n",
				   err);
			/* ndo_uninit() is not called when ndo_init() fails,
			 * so release the irq installed by fe_hw_init()
			 * before the common error path unwinds.
			 */
			free_irq(dev->irq, dev);
			goto err_phy_disconnect;
		}
	}

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

	/* fe_hw_init() -> fwd_config() clears FE_GDM1_JMB_EN on every
	 * ndo_init. An unregister/re-register cycle (for example a netns
	 * move) keeps dev->mtu but loses that hardware bit, so a jumbo
	 * MTU would silently drop every large frame until the user
	 * changed the MTU again. Replay the programmed state from the
	 * current MTU and the ring geometry fe_init_dma() just used,
	 * with the same length source as fe_change_mtu().
	 */
	if (priv->flags & FE_FLAG_JUMBO_FRAME)
		fe_jumbo_config(dev, fe_max_frag_size(dev->mtu));

	return 0;

err_phy_disconnect:
	if (priv->phy)
		priv->phy->disconnect(priv);
err_switch_cleanup:
	/* switch_init() may already have registered switch devices
	 * (gsw_dev[]): leaving them registered while this probe fails
	 * would let swconfig operations run on devm-allocated private
	 * data that is released when the probe unwinds (UAF).
	 */
	if (priv->soc->switch_cleanup)
		priv->soc->switch_cleanup(priv);
	/* unregister_switch() waits for in-flight swconfig calls. The
	 * MDIO-attached switch still references priv->mii_bus, so it has
	 * to be removed before fe_mdio_cleanup() frees that bus.
	 */
	fe_unregister_switches(priv);
	fe_mdio_cleanup(priv);

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
	/* unregister_netdev() cannot be aborted at this point, so the
	 * rings must still be released; make the otherwise silent wedged
	 * engine visible instead of letting it look like a random oops.
	 */
	if (i == 10 &&
	    (fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
		    (FE_TX_DMA_BUSY | FE_RX_DMA_BUSY)))
		netdev_err(dev, "DMA engine did not quiesce during uninit\n");

	/* cut the interrupt sources before the lengthy cleanup below:
	 * phy disconnect and MDIO teardown can take long enough for the
	 * irq handler to run against half-torn-down state
	 */
	/* Use the locked helper: fe_poll()/fe_open() can still re-enable
	 * rx/tx sources through fe_int_enable() while this teardown runs,
	 * and a plain unlocked write of 0 is a read-modify-write race that
	 * would leave the interrupt firing against the rings freed below.
	 */
	fe_int_disable(priv, ~0);
	free_irq(dev->irq, dev);

	/* release the switch irq requested with this fe_priv as dev_id
	 * before this instance goes away
	 */
	if (priv->soc->switch_cleanup)
		priv->soc->switch_cleanup(priv);

	/* ndo_uninit() is not only reached from fe_remove(), which already
	 * unregisters the switches. register_netdevice() also calls it on
	 * any failure that happens after ndo_init() succeeded, and a
	 * namespace move can unregister the device as well. In those
	 * paths the switch_dev objects were still in the global swconfig
	 * list while fe_mdio_cleanup() freed the mii_bus they reference,
	 * and the probe unwind released their devm state (UAF). The
	 * helper is idempotent, so calling it again here is harmless.
	 */
	fe_unregister_switches(priv);

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
		/* Do not free the rings while the engine is still writing
		 * to them. fe_change_mtu() already rolls dev->mtu back on
		 * any error from this function.
		 */
		err = fe_stop(priv->netdev);
		if (err) {
			/* fe_stop() already stopped the queue, disabled
			 * NAPI and interrupts, stopped the PHYs and cleared
			 * the DMA enable bits before reporting the timeout.
			 * Leaving IFF_UP set would report the interface as
			 * running while it can neither transmit nor receive,
			 * and no in-flight descriptor can raise a TX timeout
			 * to recover. Close it so an explicit up can call
			 * fe_open() on the still-intact rings.
			 * ndo_change_mtu callers hold RTNL, which is what
			 * dev_close() requires; the rollback path below
			 * already uses the same call.
			 */
			dev_close(priv->netdev);
			return err;
		}
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
		if (i == 10 &&
		    (fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
			    (FE_TX_DMA_BUSY | FE_RX_DMA_BUSY))) {
			netdev_err(priv->netdev,
				   "DMA engine did not quiesce, rings kept\n");
			return -EBUSY;
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
			/* fe_stop() at the top of this function already
			 * stopped the queue, disabled NAPI/IRQ and the DMA
			 * engine. __FE_DOWN is only a private flag: without
			 * closing the device, netif_running() stays true and
			 * the interface is reported up while permanently
			 * dead (no rings, no way to trigger fe_tx_timeout's
			 * recovery). ndo_change_mtu callers hold RTNL, which
			 * is what dev_close() requires.
			 */
			if (was_running)
				dev_close(priv->netdev);
			return err;
		}
		/* The old geometry has been reallocated successfully and,
		 * when the device was up, fe_open() below brings it back.
		 * Clear the error from the first attempt: returning it
		 * makes fe_change_mtu() roll the MTU back and report a
		 * failure even though the rings are live again. It also
		 * prevents the "oerr && !err" check below from recording
		 * a genuine fe_open() failure.
		 */
		err = 0;
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
	struct fe_priv *priv = netdev_priv(dev);
	u32 fwd_cfg;
	int units, max_units;

	if (IS_ENABLED(CONFIG_SOC_MT7621))
		return;

	fwd_cfg = fe_r32(FE_GDMA1_FWD_CFG);
	if (dev->mtu <= ETH_DATA_LEN) {
		fwd_cfg &= ~FE_GDM1_JMB_EN;
	} else {
		/* FE_GDM1_JMB_LEN_MASK is 0xf at bit 28. Shifting that
		 * int constant by 28 overflows signed int (UB), and the
		 * field is only 4 bits wide: a frag size above 15 KiB
		 * used to be truncated into a bogus length. Widen the
		 * mask to u32 and clamp the number of 1 KiB units.
		 */
		units = DIV_ROUND_UP(frag_size, 1024);
		if (units < 1)
			units = 1;
		if (units > FE_GDM1_JMB_LEN_MASK)
			units = FE_GDM1_JMB_LEN_MASK;

		/* On SoCs without FE_FLAG_RX_SG_DMA the RX descriptor
		 * carries no buffer length (rxd2 is RX_DMA_LSO), so
		 * FE_GDM1_JMB_LEN is the only ceiling on the hardware DMA
		 * write. fe_alloc_rx() maps just rx_buf_size bytes starting
		 * at NET_SKB_PAD + NET_IP_ALIGN inside a page_frag of
		 * frag_size bytes. Rounding frag_size up to whole KiB units
		 * can exceed both the mapping and the object: with a 4 KiB
		 * page and MTU 3000, frag_size ~3360 requests a 4 KiB
		 * ceiling while only ~3774 bytes are mapped from offset 66
		 * of a 4096-byte frag. Clamp to whole KiB units that fit
		 * the mapping, rounding DOWN so the hardware can never
		 * write past it. fe_poll_rx() checks the length only after
		 * the DMA has already happened and cannot prevent it.
		 */
		if (priv->rx_ring.rx_buf_size) {
			max_units = priv->rx_ring.rx_buf_size / 1024;
			if (max_units < 1)
				max_units = 1;
			if (units > max_units)
				units = max_units;
		}

		fwd_cfg &= ~((u32)FE_GDM1_JMB_LEN_MASK <<
			     FE_GDM1_JMB_LEN_SHIFT);
		fwd_cfg |= ((u32)units << FE_GDM1_JMB_LEN_SHIFT) |
			   FE_GDM1_JMB_EN;
	}
	fe_w32(fwd_cfg, FE_GDMA1_FWD_CFG);
}

static int fe_set_features(struct net_device *dev,
			   netdev_features_t features)
{
	struct fe_priv *priv = netdev_priv(dev);
	netdev_features_t saved = priv->fwd_features;
	int ret;

	/* netdev_update_features() only calls this hook when the set
	 * actually changes, so always reprogram. The SoC fwd_config()
	 * helpers read priv->fwd_features, not dev->features: the core
	 * commits dev->features only after this hook returns, and NAPI
	 * reads that field locklessly. Publishing a temporary value (and
	 * rolling it back on failure) into the netdev field used to expose
	 * an intermediate offload set to the receive path while the
	 * hardware was still being reprogrammed. A driver-private
	 * snapshot is only ever read under RTNL here and in fe_hw_init(),
	 * so no concurrent reader sees the transient state.
	 */
	priv->fwd_features = features;
	ret = priv->soc->fwd_config(priv);
	if (ret) {
		priv->fwd_features = saved;
		return ret;
	}

	/* fwd_config() rewrites GDMA1_FWD_CFG and, on jumbo-capable SoCs,
	 * clears FE_GDM1_JMB_EN. Restore the programmed jumbo state for
	 * the current MTU so an ethtool -K does not silently disable it.
	 * fe_change_mtu() derives the length from fe_max_frag_size(), not
	 * from rx_ring.frag_size: for a jumbo MTU the ring fragment is a
	 * whole PAGE_SIZE while the jumbo field must cover the frame plus
	 * its headroom. Using the ring value here used to shrink the
	 * hardware limit after any ethtool -K and silently drop frames
	 * that fit the configured MTU.
	 */
	fe_jumbo_config(dev, fe_max_frag_size(dev->mtu));

	return 0;
}

static int fe_change_mtu(struct net_device *dev, int new_mtu)
{
	struct fe_priv *priv = netdev_priv(dev);
	int frag_size, old_mtu, err;
	int new_frag, new_buf;

	if (!(priv->flags & FE_FLAG_JUMBO_FRAME))
		return eth_change_mtu(dev, new_mtu);

	if (IS_ENABLED(CONFIG_SOC_MT7621))
		if (new_mtu > 2048)
			return -EINVAL;

	/* 68 is the minimum payload (ETH_MIN_MTU does not exist in the
	 * 4.4 headers); it already accounts for the minimum
	 * 802.1q-tagged frame the MAC must transport.
	 *
	 * The upper bound has to be explicit. RX buffers come from page
	 * frags, so no MTU at or above PAGE_SIZE can ever be served by the
	 * rings. Relying only on "frag_size > PAGE_SIZE" was not enough:
	 * fe_max_frag_size() used to overflow its signed addition for an
	 * MTU near INT_MAX and return the small default size, which passed
	 * that check and let dev->mtu be set to a value no buffer can hold.
	 * MT7621 adds its own stricter cap above.
	 */
	if (new_mtu < 68 || new_mtu > PAGE_SIZE)
		return -EINVAL;

	frag_size = fe_max_frag_size(new_mtu);
	if (frag_size < 0 || frag_size > PAGE_SIZE)
		return -EINVAL;

	/* fe_realloc_dma() and the ring geometry fields are u16, while
	 * PAGE_SIZE may be 64 KiB on MIPS. Computing these values into
	 * u16 locals truncated new_frag to 0 and turned a negative
	 * fe_max_buf_size() into a huge u16. Validate the full-width
	 * results before narrowing, and do it before dev->mtu is changed
	 * so no rollback is needed.
	 *
	 * The RX descriptor length is only 14 bits wide. Using a whole
	 * page as the frag is fine on 4K/8K/16K pages, but on 32K/64K
	 * pages it produces an rx_buf_size that RX_DMA_PLEN0 truncates
	 * (software maps the full size, hardware sees the low 14 bits),
	 * and on 64K pages it also exceeds the u16 field so every jumbo
	 * MTU was rejected outright. Size the frag from the MTU instead
	 * whenever a whole page cannot fit the descriptor field.
	 */
	if (new_mtu <= ETH_DATA_LEN)
		new_frag = fe_max_frag_size(ETH_DATA_LEN);
	else if (PAGE_SIZE > RX_DMA_PLEN0_LEN_MASK)
		new_frag = frag_size;
	else
		new_frag = PAGE_SIZE;
	if (new_frag <= 0 || new_frag > 0xffff)
		return -EINVAL;
	new_buf = fe_max_buf_size(new_frag);
	if (new_buf <= 0 || new_buf > RX_DMA_PLEN0_LEN_MASK)
		return -EINVAL;

	old_mtu = dev->mtu;
	dev->mtu = new_mtu;

	/* return early if the buffer sizes will not change */
	if (old_mtu <= ETH_DATA_LEN && new_mtu <= ETH_DATA_LEN)
		return 0;
	if (old_mtu > ETH_DATA_LEN && new_mtu > ETH_DATA_LEN &&
	    new_frag == priv->rx_ring.frag_size &&
	    new_buf == priv->rx_ring.rx_buf_size) {
		/* The rings already have the right geometry, so only the
		 * hardware jumbo frame length has to track the new MTU
		 * (e.g. 9000 -> 2000).
		 */
		fe_jumbo_config(dev, frag_size);
		return 0;
	}

	/* On large pages new_frag is derived from the MTU instead of a
	 * whole PAGE_SIZE, so a jumbo-to-jumbo change can require a
	 * different ring geometry. Falling through to fe_realloc_dma()
	 * rebuilds it before fe_jumbo_config() programs the hardware.
	 * Taking the fast path unconditionally used to keep a small
	 * old ring while accepting a much larger dev->mtu: the DMA
	 * mapping stayed short, every large frame was dropped, and
	 * ndo_change_mtu still reported success.
	 */

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

static netdev_features_t fe_fix_features(struct net_device *dev,
					 netdev_features_t features)
{
	struct fe_priv *priv = netdev_priv(dev);

	/* The hardware resolves the VIDX field of txd4 through its 16 VID
	 * slots. When two active VIDs collide on one slot, the colliding
	 * VID has no slot of its own and offloading it would tag those
	 * frames with the other VLAN's VID. Refuse to turn the feature on
	 * until fe_vlan_rx_kill_vid() has removed every active VID.
	 */
	if (priv->vlan_offload_conflict)
		features &= ~NETIF_F_HW_VLAN_CTAG_TX;

	return features;
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
	.ndo_fix_features	= fe_fix_features,
	.ndo_set_features	= fe_set_features,
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
	/* A -EBUSY here means the engine never went quiet. The rings are
	 * still allocated and this is a recovery path, so deliberately
	 * proceed with the up/down cycle, but do not hide the condition.
	 */
	if (fe_stop(dev))
		netif_alert(priv, ifup, dev,
			    "DMA engine wedged during up/down cycle.\n");

	err = fe_open(dev);
	if (err) {
		netif_alert(priv, ifup, dev,
			    "Driver up/down cycle failed, closing device.\n");
		dev_close(dev);
	} else if (priv->soc->reset_ports) {
		/* reset_ports() dereferences priv->soc->swpriv and
		 * priv->mii_bus (MT7621 powers the switch PHYs down/up
		 * through the MDIO bus). Both are released by
		 * fe_uninit()/fe_mdio_cleanup() while unregister_netdev()
		 * runs from fe_remove(), and that path needs RTNL too.
		 * Keep the port reset under the same RTNL section so
		 * teardown cannot free those objects underneath the
		 * pending work. Only reset after a successful open, and
		 * do not report a recovered interface when a PHY was
		 * left powered down.
		 */
		err = priv->soc->reset_ports(priv);
		if (err) {
			netif_alert(priv, ifup, dev,
				    "Switch port reset failed, closing device.\n");
			dev_close(dev);
		}
	}
	rtnl_unlock();
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

	err = device_reset(&pdev->dev);
	/* -EPROBE_DEFER means the reset controller has not probed yet and
	 * must reach the driver core. -ENOENT is the normal result when the
	 * ethernet node has no "resets" property; the -ENOTSUPP variants
	 * are returned when reset controller support is not built in. Those
	 * are the only outcomes in which no reset was requested at all.
	 * Every other error means the requested reset did not happen: the
	 * frame engine registers would then be programmed from an unknown
	 * state, so do not silently continue probing.
	 */
	if (err == -EPROBE_DEFER)
		return err;
	if (err && err != -ENOENT && err != -ENOTSUPP && err != -EOPNOTSUPP) {
		dev_err(&pdev->dev, "failed to reset device: %d\n", err);
		return err;
	}

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
		/* -EPROBE_DEFER must reach the driver core: collapsing it
		 * into -EADDRNOTAVAIL turns a temporary deferral into a
		 * permanent probe failure and hides the real cause
		 */
		err = PTR_ERR(fe_base);
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

	/* net_device::irq is an unsigned int on this kernel: storing a
	 * negative platform_get_irq() error in it turns -EPROBE_DEFER and
	 * -ENXIO into large positive values, so a subsequent "< 0" test
	 * can never fire. Keep the signed result in err until it is known
	 * to be a valid IRQ number.
	 */
	err = platform_get_irq(pdev, 0);
	if (err < 0) {
		dev_err(&pdev->dev, "no IRQ resource found: %d\n", err);
		goto err_free_dev;
	}
	netdev->irq = err;

	if (soc->init_data)
		soc->init_data(soc, netdev);
	netdev->vlan_features = netdev->hw_features &
				~(NETIF_F_HW_VLAN_CTAG_TX |
				  NETIF_F_HW_VLAN_CTAG_RX);
	netdev->features |= netdev->hw_features;

	/* fake rx vlan filter func. to support tx vlan offload func */
	if (fe_reg_table[FE_REG_FE_DMA_VID_BASE])
		netdev->features |= NETIF_F_HW_VLAN_CTAG_FILTER;

	/* ndo_set_features() may run before fe_hw_init() seeds this from
	 * dev->features; give it the same initial set in that case.
	 */
	priv->fwd_features = netdev->features;

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
	} else if (PTR_ERR(sysclk) == -EPROBE_DEFER) {
		/* The clock provider has not probed yet. Collapsing the
		 * deferral into -ENXIO (or ignoring it on SoCs without
		 * FE_FLAG_CALIBRATE_CLK) would make the driver core give
		 * up permanently, exactly the failure already handled
		 * for device_reset(), ioremap and platform_get_irq().
		 */
		err = -EPROBE_DEFER;
		goto err_free_dev;
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
	/* register_netdev() already ran ndo_init(), which installed the
	 * irq and enabled DMA. fe_tx_timeout() may have queued
	 * pending_work, and fe_pending_work() re-arms itself while
	 * pending_flags is non-empty. Drain it before free_netdev(),
	 * just as fe_remove() does, otherwise the work runs against a
	 * freed fe_priv.
	 */
	cancel_work_sync(&priv->pending_work);
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
err_out:
	/* restore the module-wide register table on every failure path,
	 * including the devm_ioremap_resource() error (notably
	 * -EPROBE_DEFER) which jumps straight here without an iounmap.
	 * Leaving the SoC table installed would make a later probe retry
	 * use the wrong register map.
	 */
	fe_reg_table = fe_reg_table_default;
	return err;
}

static int fe_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct fe_priv *priv = netdev_priv(dev);

	fe_unregister_switches(priv);

	/* unregister first: while the device is still registered and up,
	 * a TX timeout may re-arm pending_work; cancelling the work
	 * before unregister_netdev() would let that work run after
	 * free_netdev() on an already freed net_device.
	 */
	unregister_netdev(dev);
	cancel_work_sync(&priv->pending_work);

	/* switch_np is an of_parse_phandle() reference used by
	 * mtk_gsw_init(), which is reached again from ndo_init on an
	 * unregister/re-register cycle. Dropping it before
	 * unregister_netdev() publishes a node whose refcount is already
	 * gone through that path, and leaving the non-NULL pointer behind
	 * allows a second put. Release it only after all device teardown
	 * callbacks have finished and clear the pointer.
	 */
	of_node_put(priv->switch_np);
	priv->switch_np = NULL;

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
