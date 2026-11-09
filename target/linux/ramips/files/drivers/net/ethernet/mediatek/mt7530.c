/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 * Copyright (C) 2016 Vitaly Chekryzhev <13hakta@gmail.com>
 */

#include <linux/if.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/bitops.h>
#include <linux/bitmap.h>
#include <net/genetlink.h>
#include <linux/switch.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/lockdep.h>
#include <linux/workqueue.h>
#include <linux/of_device.h>

#include "mt7530.h"

#define MT7530_CPU_PORT		6
#define MT7530_NUM_PORTS	8
#ifdef CONFIG_SOC_MT7621
#define MT7530_NUM_VLANS	4095
#else
#define MT7530_NUM_VLANS	16
#endif
#define MT7530_MAX_VID		4095
#define MT7530_MIN_VID		0

#define MT7530_PORT_MIB_TXB_ID	2	/* TxGOC */
#define MT7530_PORT_MIB_RXB_ID	6	/* RxGOC */

#define MT7621_PORT_MIB_TXB_ID	18	/* TxByte */
#define MT7621_PORT_MIB_RXB_ID	37	/* RxByte */

/* registers */
#define REG_ESW_WT_MAC_MFC		0x10

#define REG_ESW_WT_MAC_MFC_MIRROR_ENABLE	BIT(3)
#define REG_ESW_WT_MAC_MFC_MIRROR_DEST_MASK	0x07

#define REG_ESW_VLAN_VTCR		0x90
#define REG_ESW_VLAN_VAWD1		0x94
#define REG_ESW_VLAN_VAWD2		0x98
#define REG_ESW_VLAN_VTIM(x)	(0x100 + 4 * ((x) / 2))

#define REG_ESW_VLAN_VAWD1_IVL_MAC	BIT(30)
#define REG_ESW_VLAN_VAWD1_VTAG_EN	BIT(28)
#define REG_ESW_VLAN_VAWD1_VALID	BIT(0)

/* vlan egress mode */
enum {
	ETAG_CTRL_UNTAG	= 0,
	ETAG_CTRL_TAG	= 2,
	ETAG_CTRL_SWAP	= 1,
	ETAG_CTRL_STACK	= 3,
};

#define REG_ESW_PORT_PCR(x)	(0x2004 | ((x) << 8))
#define REG_ESW_PORT_PVC(x)	(0x2010 | ((x) << 8))
#define REG_ESW_PORT_PPBV1(x)	(0x2014 | ((x) << 8))

#define REG_ESW_PORT_PCR_MIRROR_SRC_RX_BIT	BIT(8)
#define REG_ESW_PORT_PCR_MIRROR_SRC_TX_BIT	BIT(9)
#define REG_ESW_PORT_PCR_MIRROR_SRC_RX_MASK	0x0100
#define REG_ESW_PORT_PCR_MIRROR_SRC_TX_MASK	0x0200

#define REG_HWTRAP		0x7804

#define MIB_DESC(_s , _o, _n)   \
	{                       \
		.size = (_s),   \
		.offset = (_o), \
		.name = (_n),   \
	}

struct mt7xxx_mib_desc {
	unsigned int size;
	unsigned int offset;
	const char *name;
};

static const struct mt7xxx_mib_desc mt7620_mibs[] = {
	MIB_DESC(1, MT7620_MIB_STATS_PPE_AC_BCNT0, "PPE_AC_BCNT0"),
	MIB_DESC(1, MT7620_MIB_STATS_PPE_AC_PCNT0, "PPE_AC_PCNT0"),
	MIB_DESC(1, MT7620_MIB_STATS_PPE_AC_BCNT63, "PPE_AC_BCNT63"),
	MIB_DESC(1, MT7620_MIB_STATS_PPE_AC_PCNT63, "PPE_AC_PCNT63"),
	MIB_DESC(1, MT7620_MIB_STATS_PPE_MTR_CNT0, "PPE_MTR_CNT0"),
	MIB_DESC(1, MT7620_MIB_STATS_PPE_MTR_CNT63, "PPE_MTR_CNT63"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_TX_GBCNT, "GDM1_TX_GBCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_TX_GPCNT, "GDM1_TX_GPCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_TX_SKIPCNT, "GDM1_TX_SKIPCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_TX_COLCNT, "GDM1_TX_COLCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_RX_GBCNT1, "GDM1_RX_GBCNT1"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_RX_GPCNT1, "GDM1_RX_GPCNT1"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_RX_OERCNT, "GDM1_RX_OERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_RX_FERCNT, "GDM1_RX_FERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_RX_SERCNT, "GDM1_RX_SERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_RX_LERCNT, "GDM1_RX_LERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_RX_CERCNT, "GDM1_RX_CERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM1_RX_FCCNT, "GDM1_RX_FCCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_TX_GBCNT, "GDM2_TX_GBCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_TX_GPCNT, "GDM2_TX_GPCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_TX_SKIPCNT, "GDM2_TX_SKIPCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_TX_COLCNT, "GDM2_TX_COLCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_RX_GBCNT, "GDM2_RX_GBCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_RX_GPCNT, "GDM2_RX_GPCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_RX_OERCNT, "GDM2_RX_OERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_RX_FERCNT, "GDM2_RX_FERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_RX_SERCNT, "GDM2_RX_SERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_RX_LERCNT, "GDM2_RX_LERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_RX_CERCNT, "GDM2_RX_CERCNT"),
	MIB_DESC(1, MT7620_MIB_STATS_GDM2_RX_FCCNT, "GDM2_RX_FCCNT")
};

static const struct mt7xxx_mib_desc mt7620_port_mibs[] = {
	MIB_DESC(1, MT7620_MIB_STATS_PORT_TGPCN,  "TxGPC"),
	MIB_DESC(1, MT7620_MIB_STATS_PORT_TBOCN,  "TxBOC"),
	MIB_DESC(1, MT7620_MIB_STATS_PORT_TGOCN,  "TxGOC"),
	MIB_DESC(1, MT7620_MIB_STATS_PORT_TEPCN,  "TxEPC"),
	MIB_DESC(1, MT7620_MIB_STATS_PORT_RGPCN,  "RxGPC"),
	MIB_DESC(1, MT7620_MIB_STATS_PORT_RBOCN,  "RxBOC"),
	MIB_DESC(1, MT7620_MIB_STATS_PORT_RGOCN,  "RxGOC"),
	MIB_DESC(1, MT7620_MIB_STATS_PORT_REPC1N, "RxEPC1"),
	MIB_DESC(1, MT7620_MIB_STATS_PORT_REPC2N, "RxEPC2")
};

static const struct mt7xxx_mib_desc mt7621_mibs[] = {
	MIB_DESC(1, MT7621_STATS_TDPC, "TxDrop"),
	MIB_DESC(1, MT7621_STATS_TCRC, "TxCRC"),
	MIB_DESC(1, MT7621_STATS_TUPC, "TxUni"),
	MIB_DESC(1, MT7621_STATS_TMPC, "TxMulti"),
	MIB_DESC(1, MT7621_STATS_TBPC, "TxBroad"),
	MIB_DESC(1, MT7621_STATS_TCEC, "TxCollision"),
	MIB_DESC(1, MT7621_STATS_TSCEC, "TxSingleCol"),
	MIB_DESC(1, MT7621_STATS_TMCEC, "TxMultiCol"),
	MIB_DESC(1, MT7621_STATS_TDEC, "TxDefer"),
	MIB_DESC(1, MT7621_STATS_TLCEC, "TxLateCol"),
	MIB_DESC(1, MT7621_STATS_TXCEC, "TxExcCol"),
	MIB_DESC(1, MT7621_STATS_TPPC, "TxPause"),
	MIB_DESC(1, MT7621_STATS_TL64PC, "Tx64Byte"),
	MIB_DESC(1, MT7621_STATS_TL65PC, "Tx65Byte"),
	MIB_DESC(1, MT7621_STATS_TL128PC, "Tx128Byte"),
	MIB_DESC(1, MT7621_STATS_TL256PC, "Tx256Byte"),
	MIB_DESC(1, MT7621_STATS_TL512PC, "Tx512Byte"),
	MIB_DESC(1, MT7621_STATS_TL1024PC, "Tx1024Byte"),
	MIB_DESC(2, MT7621_STATS_TOC, "TxByte"),
	MIB_DESC(1, MT7621_STATS_RDPC, "RxDrop"),
	MIB_DESC(1, MT7621_STATS_RFPC, "RxFiltered"),
	MIB_DESC(1, MT7621_STATS_RUPC, "RxUni"),
	MIB_DESC(1, MT7621_STATS_RMPC, "RxMulti"),
	MIB_DESC(1, MT7621_STATS_RBPC, "RxBroad"),
	MIB_DESC(1, MT7621_STATS_RAEPC, "RxAlignErr"),
	MIB_DESC(1, MT7621_STATS_RCEPC, "RxCRC"),
	MIB_DESC(1, MT7621_STATS_RUSPC, "RxUnderSize"),
	MIB_DESC(1, MT7621_STATS_RFEPC, "RxFragment"),
	MIB_DESC(1, MT7621_STATS_ROSPC, "RxOverSize"),
	MIB_DESC(1, MT7621_STATS_RJEPC, "RxJabber"),
	MIB_DESC(1, MT7621_STATS_RPPC, "RxPause"),
	MIB_DESC(1, MT7621_STATS_RL64PC, "Rx64Byte"),
	MIB_DESC(1, MT7621_STATS_RL65PC, "Rx65Byte"),
	MIB_DESC(1, MT7621_STATS_RL128PC, "Rx128Byte"),
	MIB_DESC(1, MT7621_STATS_RL256PC, "Rx256Byte"),
	MIB_DESC(1, MT7621_STATS_RL512PC, "Rx512Byte"),
	MIB_DESC(1, MT7621_STATS_RL1024PC, "Rx1024Byte"),
	MIB_DESC(2, MT7621_STATS_ROC, "RxByte"),
	MIB_DESC(1, MT7621_STATS_RDPC_CTRL, "RxCtrlDrop"),
	MIB_DESC(1, MT7621_STATS_RDPC_ING, "RxIngDrop"),
	MIB_DESC(1, MT7621_STATS_RDPC_ARL, "RxARLDrop")
};

enum {
	/* Global attributes. */
	MT7530_ATTR_ENABLE_VLAN,
};

struct mt7530_port_entry {
	u16	pvid;
	/* swconfig_set_vlan_ports() stores a VLAN table index here,
	 * while the pvid attribute stores a VID directly. */
	bool	pvid_is_index;
	bool	mirror_rx;
	bool	mirror_tx;
};

struct mt7530_vlan_entry {
	u16	vid;
	u8	member;
	u8	etags;
};

struct mt7530_priv {
	void __iomem		*base;
	struct mii_bus		*bus;
	struct switch_dev	swdev;

	/* protect among processes for registers access */
	struct mutex reg_mutex;

	u8			mirror_dest_port;
	bool			global_vlan_enable;
	struct mt7530_vlan_entry	vlan_entries[MT7530_NUM_VLANS];
	struct mt7530_port_entry	port_entries[MT7530_NUM_PORTS];
	/* On MT7621 a full clear+rewrite of the table is 4095 vtcr
	 * transactions under reg_mutex (seconds to minutes worst case).
	 * Track which entries changed and touch only those.
	 */
	DECLARE_BITMAP(vlan_dirty, MT7530_NUM_VLANS);
	/* output buffer for the MIB string callbacks: a function-local
	 * static buffer is shared across concurrent callers (and across
	 * devices), letting one netlink response be overwritten by
	 * another; keeping it per-device and filling it under reg_mutex
	 * removes that race
	 */
	char			mib_buf[4096];
};

/* Number of VLAN table entries actually described by every
 * mt7530_defaults[] mapping. The arrays themselves are sized
 * MT7530_NUM_VLANS and their tails are zero-filled.
 */
#define MT7530_MAPPING_NUM_VLANS	3

struct mt7530_mapping {
	char	*name;
	u16	pvids[MT7530_NUM_PORTS];
	u8	members[MT7530_NUM_VLANS];
	u8	etags[MT7530_NUM_VLANS];
	u16	vids[MT7530_NUM_VLANS];
} mt7530_defaults[] = {
	{
		.name = "llllw",
		.pvids = { 1, 1, 1, 1, 2, 1, 1 },
		.members = { 0, 0x6f, 0x50 },
		.etags = { 0, 0x40, 0x40 },
		.vids = { 0, 1, 2 },
	}, {
		.name = "wllll",
		.pvids = { 2, 1, 1, 1, 1, 1, 1 },
		.members = { 0, 0x7e, 0x41 },
		.etags = { 0, 0x40, 0x40 },
		.vids = { 0, 1, 2 },
	}, {
		.name = "lwlll",
		.pvids = { 1, 2, 1, 1, 1, 1, 1 },
		.members = { 0, 0x7d, 0x42 },
		.etags = { 0, 0x40, 0x40 },
		.vids = { 0, 1, 2 },
	},
};

struct mt7530_mapping*
mt7530_find_mapping(struct device_node *np)
{
	const char *map;
	int i;

	if (of_property_read_string(np, "mediatek,portmap", &map))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(mt7530_defaults); i++)
		if (!strcmp(map, mt7530_defaults[i].name))
			return &mt7530_defaults[i];

	return NULL;
}

static void
mt7530_apply_mapping(struct mt7530_priv *mt7530, struct mt7530_mapping *map)
{
	int i = 0;

	/* The mapping carries VLAN table indices, not VIDs: port 4 of
	 * "llllw" is 2 for VLAN slot 2. Mark them so apply_config()
	 * resolves them through vlan_entries[] instead of programming
	 * the index verbatim into PPBV1. With the default vids[k] == k
	 * the two are numerically equal, but a mapping with a remapped
	 * VID would otherwise put the wrong VID on the wire.
	 */
	for (i = 0; i < MT7530_NUM_PORTS; i++) {
		mt7530->port_entries[i].pvid = map->pvids[i];
		mt7530->port_entries[i].pvid_is_index = true;
	}

	for (i = 0; i < MT7530_NUM_VLANS; i++) {
		mt7530->vlan_entries[i].member = map->members[i];
		mt7530->vlan_entries[i].etags = map->etags[i];
		/* A mapping only defines the first three VIDs; the rest of
		 * map->vids[] is zero-filled. Copying the whole tail turns
		 * every remaining slot into VID 0 and desynchronises both
		 * get_vid() callbacks from the vid == index invariant that
		 * mt7530_probe() seeds and mt7530_reset_switch() restores.
		 * On MT7621 the VTCR is additionally addressed by VID, so
		 * all those writes would target VID 0. This is not an
		 * MT7621-only problem: keep the seeded VIDs on every SoC.
		 */
		if (i < MT7530_MAPPING_NUM_VLANS)
			mt7530->vlan_entries[i].vid = map->vids[i];
	}
}

static int mt7530_apply_config(struct switch_dev *dev);

static int
mt7530_reset_switch(struct switch_dev *dev)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	int i;

	/* Clearing the VLAN table while VLAN mode stays enabled makes the
	 * next apply_config() program every port in security mode with no
	 * VLAN membership, isolating the whole switch. The mirror target
	 * would also keep being reported through swconfig. Reset both
	 * globals with the arrays, like esw_reset_switch() does.
	 */
	/* apply_config() reads all of this under reg_mutex */
	mutex_lock(&priv->reg_mutex);
	priv->global_vlan_enable = false;
	priv->mirror_dest_port = 0;

	memset(priv->port_entries, 0, sizeof(priv->port_entries));
	memset(priv->vlan_entries, 0, sizeof(priv->vlan_entries));

	/* set default vid of each vlan to the same number of vlan, so the vid
	 * won't need be set explicitly.
	 */
	for (i = 0; i < MT7530_NUM_VLANS; i++) {
		priv->vlan_entries[i].vid = i;
	}

	/*
	 * The software state is empty again while the hardware still
	 * holds the old table.  Mark every slot dirty so the next
	 * apply_config() rewrites them: with dirty tracking, an empty
	 * bitmap would otherwise skip the whole table and leave
	 * hardware and software permanently diverged.
	 */
	bitmap_fill(priv->vlan_dirty, MT7530_NUM_VLANS);
	mutex_unlock(&priv->reg_mutex);

	/* swconfig's reset command only calls reset_switch(), never
	 * apply_config(). Clearing the software state without pushing it
	 * to the hardware leaves the switch still running the previous
	 * VLAN/mirror configuration while the API reports the defaults,
	 * i.e. software and hardware stay diverged until the user happens
	 * to issue an apply. esw_reset_switch() already reapplies.
	 */
	return mt7530_apply_config(dev);
}

static int
mt7530_get_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	mutex_lock(&priv->reg_mutex);
	val->value.i = priv->global_vlan_enable;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_set_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	/* swconfig does not enforce attr->max. Without this check any
	 * non-zero value is silently normalised to 1, so "set 2" reports
	 * success while "get" returns a different value. Reject it like
	 * every other setter in this driver (and esw_rt3050.c) does.
	 */
	if (val->value.i > (u32)attr->max)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	priv->global_vlan_enable = val->value.i != 0;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_read_reg(struct mt7530_priv *priv, u32 reg, u32 *val)
{
	if (priv->bus) {
		int high = 0, low = 0, ret;

		/* The page select and both data reads are one indirect
		 * access. A failed transfer used to be truncated into a
		 * u16 and concatenated into a plausible looking register
		 * value, which every read-modify-write caller then wrote
		 * back into the switch. Stop at the first error, like
		 * mt7530_w32() does: otherwise the high-word read is
		 * issued against a bus/page that already failed.
		 */
		mutex_lock(&priv->bus->mdio_lock);
		ret = __mdiobus_write(priv->bus, 0x1f, 0x1f,
				      (reg >> 6) & 0x3ff);
		if (!ret) {
			low = __mdiobus_read(priv->bus, 0x1f,
					     (reg >> 2) & 0xf);
			if (low >= 0)
				high = __mdiobus_read(priv->bus, 0x1f, 0x10);
		}
		mutex_unlock(&priv->bus->mdio_lock);

		if (ret)
			return ret;
		if (low < 0)
			return low;
		if (high < 0)
			return high;

		*val = ((u32)high << 16) | (low & 0xffff);
		return 0;
	}

	*val = ioread32(priv->base + reg);
	pr_debug("MT7530 MDIO Read [%04x]=%08x\n", reg, *val);

	return 0;
}

/* The MIB getters only report numbers: show zero instead of a bogus
 * counter when the bus read failed.
 */
static u32
mt7530_r32(struct mt7530_priv *priv, u32 reg)
{
	u32 val;

	if (mt7530_read_reg(priv, reg, &val))
		return 0;

	return val;
}

static int
mt7530_w32(struct mt7530_priv *priv, u32 reg, u32 val)
{
	if (priv->bus) {
		int ret;

		/* The page select and the two data writes are one indirect
		 * access. A failed page select leaves the old page active,
		 * so continuing would write the data into the wrong
		 * register. Stop at the first error and let callers that
		 * care (mt7530_vtcr) propagate it.
		 */
		mutex_lock(&priv->bus->mdio_lock);
		ret = __mdiobus_write(priv->bus, 0x1f, 0x1f,
				      (reg >> 6) & 0x3ff);
		if (!ret)
			ret = __mdiobus_write(priv->bus, 0x1f,
					      (reg >> 2) & 0xf,
					      val & 0xffff);
		if (!ret)
			ret = __mdiobus_write(priv->bus, 0x1f, 0x10,
					      val >> 16);
		mutex_unlock(&priv->bus->mdio_lock);

		return ret;
	}

	pr_debug("MT7530 MDIO Write[%04x]=%08x\n", reg, val);
	iowrite32(val, priv->base + reg);

	return 0;
}

static int
mt7530_vtcr(struct mt7530_priv *priv, u32 cmd, u32 val)
{
	int i, ret;

	ret = mt7530_w32(priv, REG_ESW_VLAN_VTCR,
			 BIT(31) | (cmd << 12) | val);
	if (ret)
		return ret;

	for (i = 0; i < 20; i++) {
		u32 vtcr;

		ret = mt7530_read_reg(priv, REG_ESW_VLAN_VTCR, &vtcr);
		if (ret)
			return ret;
		if ((vtcr & BIT(31)) == 0)
			break;

		udelay(1000);
	}
	if (i == 20) {
		printk("mt7530: vtcr timeout\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int
mt7530_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	u16 pvid;

	if (port < 0 || port >= MT7530_NUM_PORTS)
		return -EINVAL;

	/* Pair with mt7530_set_port_pvid() and mt7530_set_vlan_ports():
	 * both only update the software state, PPBV1 is programmed later
	 * by mt7530_apply_config(). Reading the hardware reported the old
	 * configuration for a pending change and stale data after a
	 * failed apply or a reset_switch(), just like the vid and VLAN
	 * member getters that were already converted to software state.
	 * Resolve a VLAN table index to that entry's VID so the result
	 * matches the VID written to PPBV1 by apply_config().
	 */
	mutex_lock(&priv->reg_mutex);
	pvid = priv->port_entries[port].pvid;
	if (priv->port_entries[port].pvid_is_index &&
	    pvid < MT7530_NUM_VLANS)
		pvid = priv->vlan_entries[pvid].vid;
	mutex_unlock(&priv->reg_mutex);

	*val = pvid;

	return 0;
}

static int
mt7530_set_port_pvid(struct switch_dev *dev, int port, int pvid)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	if (port < 0 || port >= MT7530_NUM_PORTS)
		return -EINVAL;

	/* 4095 (MT7530_MAX_VID) is the reserved 802.1Q VID and has no
	 * slot in the MT7621 VLAN table; the attribute advertises 0..4094
	 */
	if (pvid < MT7530_MIN_VID || pvid >= MT7530_MAX_VID)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	priv->port_entries[port].pvid = pvid;
	priv->port_entries[port].pvid_is_index = false;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	u8 member, etags;
	int i;

	val->len = 0;

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_VLANS)
		return -EINVAL;

	/* Pair with mt7530_set_vlan_ports(), which only updates the
	 * software table: the VAWD1/VAWD2 registers are programmed later
	 * by apply_config(). Reading the hardware reported the previous
	 * configuration for a pending change and stale data after a
	 * failed apply or a reset_switch().
	 *
	 * Software etags is a one-bit-per-port bitmap, while hardware
	 * VAWD2 uses a two-bit field per port; mt7530_write_vlan_entry()
	 * performs that expansion. Assign p->flags instead of OR-ing
	 * into memory swconfig did not initialise.
	 */
	mutex_lock(&priv->reg_mutex);
	member = priv->vlan_entries[val->port_vlan].member;
	etags = priv->vlan_entries[val->port_vlan].etags;
	mutex_unlock(&priv->reg_mutex);

	for (i = 0; i < MT7530_NUM_PORTS; i++) {
		struct switch_port *p;

		if (!(member & BIT(i)))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		p->flags = (etags & BIT(i)) ?
			   BIT(SWITCH_PORT_FLAG_TAGGED) : 0;
	}

	return 0;
}

static int
mt7530_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	u8 member = 0;
	u8 etags = 0;
	int i;

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_VLANS ||
			val->len > MT7530_NUM_PORTS)
		return -EINVAL;

	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];

		if (p->id < 0 || p->id >= MT7530_NUM_PORTS)
			return -EINVAL;

		member |= BIT(p->id);

		if (p->flags & BIT(SWITCH_PORT_FLAG_TAGGED))
			etags |= BIT(p->id);
	}
	mutex_lock(&priv->reg_mutex);

	/* Freeze every index reference to this VLAN before replacing the
	 * member list. An untagged member stores the table index in pvid;
	 * if it is now tagged or has left the VLAN, that reference must not
	 * survive, otherwise a later set_vid() on this VLAN silently
	 * changes an unrelated port's PVID when apply_config() resolves
	 * the stale index. Ports that stay untagged are re-marked below.
	 */
	for (i = 0; i < MT7530_NUM_PORTS; i++) {
		struct mt7530_port_entry *entry = &priv->port_entries[i];

		if (!entry->pvid_is_index ||
		    entry->pvid != val->port_vlan)
			continue;

		entry->pvid = priv->vlan_entries[val->port_vlan].vid;
		entry->pvid_is_index = false;
	}

	priv->vlan_entries[val->port_vlan].member = member;
	priv->vlan_entries[val->port_vlan].etags = etags;
	set_bit(val->port_vlan, priv->vlan_dirty);

	/* swconfig_set_vlan_ports() calls set_port_pvid() with the VLAN
	 * table index for every untagged member before reaching this
	 * callback. Mark those entries as indices so apply_config() can
	 * resolve them to the entry's VID; an explicit "set pvid" writes
	 * a real VID and clears the flag.
	 */
	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];

		if (p->flags & BIT(SWITCH_PORT_FLAG_TAGGED))
			continue;

		priv->port_entries[p->id].pvid = val->port_vlan;
		priv->port_entries[p->id].pvid_is_index = true;
	}
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_set_vid(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	int vlan;
	u16 vid;

	vlan = val->port_vlan;

	if (vlan < 0 || vlan >= MT7530_NUM_VLANS)
		return -EINVAL;

	/* val->value.i is u32 and swconfig does not enforce attr->max.
	 * Narrowing it to u16 before the range check let values such as
	 * 0x10001 pass as VID 1. Validate the original value first; VID
	 * 4095 is reserved by 802.1Q and has no MT7621 table slot.
	 */
	if (val->value.i >= (u32)MT7530_MAX_VID)
		return -EINVAL;
	vid = val->value.i;

#ifdef CONFIG_SOC_MT7621
	/* On MT7621 the VLAN table is indexed by VID directly:
	 * get_vlan_ports() reads entries back by VID, so remapping a
	 * slot to a different VID would desynchronise get and set.
	 * Keep index == VID.
	 */
	if (vid != vlan)
		return -EINVAL;
#endif

	mutex_lock(&priv->reg_mutex);
	priv->vlan_entries[vlan].vid = vid;
	set_bit(vlan, priv->vlan_dirty);
	mutex_unlock(&priv->reg_mutex);
	return 0;
}

static int
mt7621_get_vid(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_VLANS)
		return -EINVAL;

	/* pair with mt7530_set_vid(): report the configured VID, not
	 * the array index
	 */
	mutex_lock(&priv->reg_mutex);
	val->value.i = priv->vlan_entries[val->port_vlan].vid;
	mutex_unlock(&priv->reg_mutex);
	return 0;
}

static int
mt7530_get_vid(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_VLANS)
		return -EINVAL;

	/* mt7530_set_vid() only updates vlan_entries[]; the VTIM register
	 * is written later by apply_config(). Report the configured VID,
	 * as mt7621_get_vid() already does, so a pending or failed apply
	 * cannot make "get vid" disagree with "set vid".
	 */
	mutex_lock(&priv->reg_mutex);
	val->value.i = priv->vlan_entries[val->port_vlan].vid;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_get_mirror_monitor_port(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	mutex_lock(&priv->reg_mutex);
	val->value.i = priv->mirror_dest_port;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_set_mirror_monitor_port(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	/* swconfig does not enforce attr->max. A value outside the port
	 * range used to be truncated into the u8 field and then written
	 * unmasked into WT_MAC_MFC, clobbering bits outside the 3-bit
	 * mirror destination field.
	 */
	if (val->value.i >= MT7530_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	priv->mirror_dest_port = val->value.i;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_get_port_mirror_rx(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	val->value.i =  priv->port_entries[val->port_vlan].mirror_rx;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_set_port_mirror_rx(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_PORTS)
		return -EINVAL;

	/* swconfig does not enforce attr->max. Assigning an arbitrary u32
	 * to the bool field truncates it: 2 enables mirroring while 256
	 * silently disables it. Reject anything outside the advertised
	 * 0..1 range, like the other validated setters in this driver.
	 */
	if (val->value.i > 1)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	priv->port_entries[val->port_vlan].mirror_rx = val->value.i;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_get_port_mirror_tx(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	val->value.i =  priv->port_entries[val->port_vlan].mirror_tx;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_set_port_mirror_tx(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_PORTS)
		return -EINVAL;

	/* swconfig does not enforce attr->max; see the mirror_rx setter. */
	if (val->value.i > 1)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	priv->port_entries[val->port_vlan].mirror_tx = val->value.i;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
mt7530_write_vlan_entry(struct mt7530_priv *priv, int vlan, u16 vid,
	                    u8 ports, u8 etags)
{
	int port, ret;
	u32 val;

#ifndef CONFIG_SOC_MT7621
	/* vid of vlan */
	ret = mt7530_read_reg(priv, REG_ESW_VLAN_VTIM(vlan), &val);
	if (ret)
		return ret;
	if (vlan % 2 == 0) {
		val &= 0xfff000;
		val |= vid;
	} else {
		val &= 0xfff;
		val |= (vid << 12);
	}
	ret = mt7530_w32(priv, REG_ESW_VLAN_VTIM(vlan), val);
	if (ret)
		return ret;
#endif

	/* vlan port membership */
	if (ports)
		ret = mt7530_w32(priv, REG_ESW_VLAN_VAWD1,
			REG_ESW_VLAN_VAWD1_IVL_MAC |
			REG_ESW_VLAN_VAWD1_VTAG_EN | (ports << 16) |
			REG_ESW_VLAN_VAWD1_VALID);
	else
		ret = mt7530_w32(priv, REG_ESW_VLAN_VAWD1, 0);
	if (ret)
		return ret;

	/* egress mode */
	val = 0;
	for (port = 0; port < MT7530_NUM_PORTS; port++) {
		if (etags & BIT(port))
			val |= ETAG_CTRL_TAG << (port * 2);
		else
			val |= ETAG_CTRL_UNTAG << (port * 2);
	}
	ret = mt7530_w32(priv, REG_ESW_VLAN_VAWD2, val);
	if (ret)
		return ret;

	/* write to vlan table */
#ifdef CONFIG_SOC_MT7621
	return mt7530_vtcr(priv, 1, vid);
#else
	return mt7530_vtcr(priv, 1, vlan);
#endif
}

static int
mt7530_apply_config(struct switch_dev *dev)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	int i, j, err = 0;
	u8 tag_ports;
	u8 untag_ports;
	bool is_mirror = false;

	mutex_lock(&priv->reg_mutex);

	if (!priv->global_vlan_enable) {
		u32 mfc;

		for (i = 0; i < MT7530_NUM_PORTS; i++) {
			err = mt7530_w32(priv, REG_ESW_PORT_PCR(i),
					 0x00400000);
			if (err)
				goto out_unlock;
		}

		err = mt7530_w32(priv, REG_ESW_PORT_PCR(MT7530_CPU_PORT),
				 0x00ff0000);
		if (err)
			goto out_unlock;

		for (i = 0; i < MT7530_NUM_PORTS; i++) {
			err = mt7530_w32(priv, REG_ESW_PORT_PVC(i),
					 0x810000c0);
			if (err)
				goto out_unlock;
		}

		/* The mirror setup at the end of this function is skipped
		 * in VLAN-disabled mode, but MIRROR_ENABLE is a global bit.
		 * Leaving it set after reset_switch() or "enable_vlan 0"
		 * keeps mirroring active in hardware while the software
		 * state reports no monitor and no source ports.
		 */
		err = mt7530_read_reg(priv, REG_ESW_WT_MAC_MFC, &mfc);
		if (err)
			goto out_unlock;
		mfc &= ~REG_ESW_WT_MAC_MFC_MIRROR_ENABLE;
		err = mt7530_w32(priv, REG_ESW_WT_MAC_MFC, mfc);
		if (err)
			goto out_unlock;

		/* The mirror source bits in every PCR were overwritten with
		 * 0x00400000 above. Clear the software state in the same
		 * critical section so the getters do not keep reporting a
		 * monitor port and source ports that the hardware no longer
		 * has programmed.
		 */
		priv->mirror_dest_port = 0;
		for (i = 0; i < MT7530_NUM_PORTS; i++) {
			priv->port_entries[i].mirror_rx = false;
			priv->port_entries[i].mirror_tx = false;
		}

		mutex_unlock(&priv->reg_mutex);

		return 0;
	}

	/* set all ports as security mode */
	for (i = 0; i < MT7530_NUM_PORTS; i++) {
		err = mt7530_w32(priv, REG_ESW_PORT_PCR(i), 0x00ff0003);
		if (err)
			goto out_unlock;
	}

	/* check if a port is used in tag/untag vlan egress mode */
	tag_ports = 0;
	untag_ports = 0;

	for (i = 0; i < MT7530_NUM_VLANS; i++) {
		u8 member = priv->vlan_entries[i].member;
		u8 etags = priv->vlan_entries[i].etags;

		if (!member)
			continue;

		for (j = 0; j < MT7530_NUM_PORTS; j++) {
			if (!(member & BIT(j)))
				continue;

			if (etags & BIT(j))
				tag_ports |= 1u << j;
			else
				untag_ports |= 1u << j;
		}
	}

	/* set all untag-only ports as transparent and the rest as user port */
	for (i = 0; i < MT7530_NUM_PORTS; i++) {
		u32 pvc_mode = 0x81000000;

		if (untag_ports & BIT(i) && !(tag_ports & BIT(i)))
			pvc_mode = 0x810000c0;

		err = mt7530_w32(priv, REG_ESW_PORT_PVC(i), pvc_mode);
		if (err)
			goto out_unlock;
	}

	/* Rewrite only the entries that changed since the last apply:
	 * on MT7621 a full-table clear+rewrite is 4095 vtcr transactions
	 * while holding reg_mutex (seconds to minutes worst case),
	 * blocking every other swconfig and link operation. Clean
	 * entries are known to match the hardware already; a dirty
	 * entry with no members is invalidated by the write itself.
	 */
	for_each_set_bit(i, priv->vlan_dirty, MT7530_NUM_VLANS) {
		u16 vid = priv->vlan_entries[i].vid;
		u8 member = priv->vlan_entries[i].member;
		u8 etags = priv->vlan_entries[i].etags;

		err = mt7530_write_vlan_entry(priv, i, vid, member, etags);
		if (err)
			goto out_unlock;
		/* Only forget an entry once it is known to match the
		 * hardware. Clearing the whole bitmap unconditionally made
		 * a failed MDIO write permanent: apply_config() reported
		 * the error once and the next apply skipped the entry,
		 * leaving the software and hardware VLAN tables diverged.
		 */
		clear_bit(i, priv->vlan_dirty);
	}

	/* Port Default PVID */
	for (i = 0; i < MT7530_NUM_PORTS; i++) {
		unsigned int idx = priv->port_entries[i].pvid;
		/* Both writers are bounded: mt7530_set_port_pvid() checks
		 * against MT7530_MAX_VID and swconfig bounds the VLAN index
		 * by swdev->vlans, so the stored value always fits a VID.
		 */
		u16 pvid = (u16)idx;
		u32 val;

		/* PPBV1 holds a VID. Only a value written by
		 * swconfig_set_vlan_ports() is a VLAN table index and has
		 * to be resolved through vlan_entries[]; a value from the
		 * pvid attribute is already a VID and must be used as is,
		 * even when it happens to name a populated table slot.
		 * Guessing from the value alone made "set pvid 2" program
		 * the VID of VLAN entry 2 instead of VID 2.
		 */
		if (priv->port_entries[i].pvid_is_index &&
		    idx < MT7530_NUM_VLANS)
			pvid = priv->vlan_entries[idx].vid;

		err = mt7530_read_reg(priv, REG_ESW_PORT_PPBV1(i), &val);
		if (err)
			goto out_unlock;
		val &= ~0xfff;
		val |= pvid;
		err = mt7530_w32(priv, REG_ESW_PORT_PPBV1(i), val);
		if (err)
			goto out_unlock;
	}

	/* set mirroring source port */
	for (i = 0; i < MT7530_NUM_PORTS; i++)	{
		u32 val;

		err = mt7530_read_reg(priv, REG_ESW_PORT_PCR(i), &val);
		if (err)
			goto out_unlock;
		/* The source bits are sticky in hardware. Clear them before
		 * OR-ing the requested state, otherwise disabling a mirror
		 * (or moving it to another port) leaves the old port still
		 * mirroring; it becomes active again as soon as any other
		 * port re-enables the global MIRROR_ENABLE bit.
		 */
		val &= ~(REG_ESW_PORT_PCR_MIRROR_SRC_RX_BIT |
			 REG_ESW_PORT_PCR_MIRROR_SRC_TX_BIT);
		if (priv->port_entries[i].mirror_rx) {
			val |= REG_ESW_PORT_PCR_MIRROR_SRC_RX_BIT;
			is_mirror = true;
		}

		if (priv->port_entries[i].mirror_tx) {
			val |= REG_ESW_PORT_PCR_MIRROR_SRC_TX_BIT;
			is_mirror = true;
		}

		err = mt7530_w32(priv, REG_ESW_PORT_PCR(i), val);
		if (err)
			goto out_unlock;
	}

	/* set mirroring monitor port */
	{
		u32 val;

		err = mt7530_read_reg(priv, REG_ESW_WT_MAC_MFC, &val);
		if (err)
			goto out_unlock;
		if (is_mirror) {
			val |= REG_ESW_WT_MAC_MFC_MIRROR_ENABLE;
			val &= ~REG_ESW_WT_MAC_MFC_MIRROR_DEST_MASK;
			val |= priv->mirror_dest_port &
			       REG_ESW_WT_MAC_MFC_MIRROR_DEST_MASK;
		} else {
			/* mirroring must also be turned off again: the
			 * enable bit used to stay set forever once any
			 * mirror had been applied
			 */
			val &= ~REG_ESW_WT_MAC_MFC_MIRROR_ENABLE;
		}
		err = mt7530_w32(priv, REG_ESW_WT_MAC_MFC, val);
		if (err)
			goto out_unlock;
	}

	mutex_unlock(&priv->reg_mutex);

	return 0;

out_unlock:
	/* Propagate MDIO and vtcr failures: pretending the apply
	 * succeeded would leave the software and hardware switch state
	 * diverged with no way for the caller to retry.
	 */
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int
mt7530_get_port_link(struct switch_dev *dev,  int port,
			struct switch_port_link *link)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	u32 speed, pmsr;
	int ret;

	if (port < 0 || port >= MT7530_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	ret = mt7530_read_reg(priv, 0x3008 + (0x100 * port), &pmsr);
	mutex_unlock(&priv->reg_mutex);
	if (ret)
		return ret;

	link->link = pmsr & 1;
	link->duplex = (pmsr >> 1) & 1;
	speed = (pmsr >> 2) & 3;

	switch (speed) {
	case 0:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case 1:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case 2:
	case 3: /* forced gige speed can be 2 or 3 */
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	default:
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
		break;
	}

	return 0;
}

static int mt7530_set_port_link(struct switch_dev *sw_dev, int port,
			     struct switch_port_link *link)
{
	struct mt7530_priv *priv = container_of(sw_dev, struct mt7530_priv, swdev);
	int ret;

	if (port < 0 || port >= MT7530_NUM_PORTS)
		return -EINVAL;

	if (!priv->bus)
		return -EOPNOTSUPP;

	mutex_lock(&priv->reg_mutex);

	/* Setup autoneg advertise here */
	if (link->aneg) {
		u16 bmsr, adv, gctrl;
		bool ercap;

		/* a failed read would leave bmsr holding garbage (or a
		 * truncated negative errno) and ercap would steer the
		 * 1000M configuration down the wrong branch
		 */
		if (sw_dev->ops->phy_read16(sw_dev, port, MII_BMSR, &bmsr)) {
			mutex_unlock(&priv->reg_mutex);
			return -EIO;
		}
		/* ERCAP means we have MII_CTRL1000 register */
		ercap = !!(bmsr & BMSR_ERCAP);

		adv = ADVERTISE_CSMA | ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM;
		gctrl = CTL1000_ENABLE_MASTER;

		switch (link->speed) {
		case SWITCH_PORT_SPEED_10:
			if (link->duplex)
				adv |= ADVERTISE_10FULL;
			else
				adv |= ADVERTISE_10HALF;
			break;
		case SWITCH_PORT_SPEED_100:
			if (link->duplex)
				adv |= ADVERTISE_100FULL;
			else
				adv |= ADVERTISE_100HALF;
			break;
		case SWITCH_PORT_SPEED_1000:
			if (!ercap || !link->duplex) {
				mutex_unlock(&priv->reg_mutex);
				return -ENOTSUPP;
			}
			/* PHY only supports 1000FULL */
			gctrl |= ADVERTISE_1000FULL;
			break;
		default:
			/* For unknown input speed just enable all speed grades */
			if (link->duplex) {
				adv |= ADVERTISE_FULL;
				gctrl |= ADVERTISE_1000FULL;
			} else {
				adv |= ADVERTISE_10HALF | ADVERTISE_100HALF;
				gctrl = 0x0;
			}
			break;
		}

		/* A failed MDIO write leaves the old advertisement in the
		 * PHY; restarting autoneg with it would silently negotiate
		 * the wrong speed or duplex.
		 */
		if (sw_dev->ops->phy_write16(sw_dev, port, MII_ADVERTISE, adv)) {
			mutex_unlock(&priv->reg_mutex);
			return -EIO;
		}
		if (ercap &&
		    sw_dev->ops->phy_write16(sw_dev, port, MII_CTRL1000, gctrl)) {
			mutex_unlock(&priv->reg_mutex);
			return -EIO;
		}
		/* Autoneg restart will be triggered in switch_generic_set_link */
	}
	/* Let switch_generic_set_link handle not autoneg case */
	ret = switch_generic_set_link(sw_dev, port, link);

	mutex_unlock(&priv->reg_mutex);

	return ret;
}

static u64 get_mib_counter(struct mt7530_priv *priv, int i, int port)
{
	unsigned int port_base;
	u64 lo;

	port_base = MT7621_MIB_COUNTER_BASE +
		    MT7621_MIB_COUNTER_PORT_OFFSET * port;

	lo = mt7530_r32(priv, port_base + mt7621_mibs[i].offset);
	if (mt7621_mibs[i].size == 2) {
		u64 hi;

		hi = mt7530_r32(priv, port_base + mt7621_mibs[i].offset + 4);
		lo |= hi << 32;
	}

	return lo;
}

/* snprintf() returns the length it would have written, not the length
 * actually written; accumulating that into len let it grow past the
 * buffer size and made "sizeof(buf) - len" wrap to a huge size_t.
 * 1dae27ef already replaced snprintf() with scnprintf(), which does
 * return the bytes really written. This helper uses the equivalent
 * vscnprintf() and an explicit size check so len stays strictly below
 * size and buf stays NUL-terminated, including on the second append
 * of an iteration (the guard d8fdcc46 added to just one loop).
 */
static void mt7530_mib_append(char *buf, size_t size, size_t *len,
			      const char *fmt, ...)
{
	va_list args;

	if (*len >= size)
		return;

	va_start(args, fmt);
	*len += vscnprintf(buf + *len, size - *len, fmt, args);
	va_end(args);
}

static int mt7621_sw_get_port_mib(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	char *buf = priv->mib_buf;
	size_t len = 0;
	int i;

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);

	mt7530_mib_append(buf, sizeof(priv->mib_buf), &len,
			  "Port %d MIB counters\n", val->port_vlan);

	for (i = 0; i < ARRAY_SIZE(mt7621_mibs); ++i) {
		u64 counter;

		mt7530_mib_append(buf, sizeof(priv->mib_buf), &len,
				  "%-11s: ", mt7621_mibs[i].name);
		counter = get_mib_counter(priv, i, val->port_vlan);
		mt7530_mib_append(buf, sizeof(priv->mib_buf), &len,
				  "%llu\n", counter);
	}

	val->value.s = buf;
	val->len = len;

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static u64 get_mib_counter_7620(struct mt7530_priv *priv, int i)
{
	return mt7530_r32(priv, MT7620_MIB_COUNTER_BASE + mt7620_mibs[i].offset);
}

static u64 get_mib_counter_port_7620(struct mt7530_priv *priv, int i, int port)
{
	return mt7530_r32(priv,
			MT7620_MIB_COUNTER_BASE_PORT +
			(MT7620_MIB_COUNTER_PORT_OFFSET * port) +
			mt7620_port_mibs[i].offset);
}

static int mt7530_sw_get_mib(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	char *buf = priv->mib_buf;
	size_t len = 0;
	int i;

	mutex_lock(&priv->reg_mutex);

	mt7530_mib_append(buf, sizeof(priv->mib_buf), &len,
			  "Switch MIB counters\n");

	for (i = 0; i < ARRAY_SIZE(mt7620_mibs); ++i) {
		u64 counter;

		mt7530_mib_append(buf, sizeof(priv->mib_buf), &len,
				  "%-11s: ", mt7620_mibs[i].name);
		counter = get_mib_counter_7620(priv, i);
		mt7530_mib_append(buf, sizeof(priv->mib_buf), &len,
				  "%llu\n", counter);
	}

	val->value.s = buf;
	val->len = len;

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int mt7530_sw_get_port_mib(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	char *buf = priv->mib_buf;
	size_t len = 0;
	int i;

	if (val->port_vlan < 0 || val->port_vlan >= MT7530_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);

	mt7530_mib_append(buf, sizeof(priv->mib_buf), &len,
			  "Port %d MIB counters\n", val->port_vlan);

	for (i = 0; i < ARRAY_SIZE(mt7620_port_mibs); ++i) {
		u64 counter;

		mt7530_mib_append(buf, sizeof(priv->mib_buf), &len,
				  "%-11s: ", mt7620_port_mibs[i].name);
		counter = get_mib_counter_port_7620(priv, i, val->port_vlan);
		mt7530_mib_append(buf, sizeof(priv->mib_buf), &len,
				  "%llu\n", counter);
	}

	val->value.s = buf;
	val->len = len;

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int mt7530_get_port_stats(struct switch_dev *dev, int port,
					struct switch_port_stats *stats)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	if (port < 0 || port >= MT7530_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);

	stats->tx_bytes = get_mib_counter_port_7620(priv, MT7530_PORT_MIB_TXB_ID, port);
	stats->rx_bytes = get_mib_counter_port_7620(priv, MT7530_PORT_MIB_RXB_ID, port);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int mt7621_get_port_stats(struct switch_dev *dev, int port,
					struct switch_port_stats *stats)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	u64 tx, rx;

	if (port < 0 || port >= MT7530_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);

	tx = get_mib_counter(priv, MT7621_PORT_MIB_TXB_ID, port);
	rx = get_mib_counter(priv, MT7621_PORT_MIB_RXB_ID, port);

	/* The MT7621 byte counters are 64 bit, while swconfig's
	 * switch_port_stats uses unsigned long (32 bit on ramips). An
	 * implicit truncation silently dropped the high word and made the
	 * reported counters wrap; saturate at the representable maximum.
	 */
	stats->tx_bytes = (unsigned long)min_t(u64, tx, ~0UL);
	stats->rx_bytes = (unsigned long)min_t(u64, rx, ~0UL);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int mt7530_phy_read16(struct switch_dev *dev, int addr,
					u8 reg, u16 *value)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);
	int ret;

	if (!priv->bus)
		return -EOPNOTSUPP;

	ret = mdiobus_read(priv->bus, addr, reg);
	/* mdiobus_read() returns a negative errno on failure. Storing
	 * that directly into *value used to hand callers a truncated
	 * 0xffxx "register value" while reporting success, which also
	 * made the BMSR check in mt7530_set_port_link() ineffective.
	 */
	if (ret < 0)
		return ret;

	*value = ret;

	return 0;
}

static int mt7530_phy_write16(struct switch_dev *dev, int addr,
					u8 reg, u16 value)
{
	struct mt7530_priv *priv = container_of(dev, struct mt7530_priv, swdev);

	if (!priv->bus)
		return -EOPNOTSUPP;

	return mdiobus_write(priv->bus, addr, reg, value);
}

static const struct switch_attr mt7530_global[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "VLAN mode (1:enabled)",
		.max = 1,
		.id = MT7530_ATTR_ENABLE_VLAN,
		.get = mt7530_get_vlan_enable,
		.set = mt7530_set_vlan_enable,
	}, {
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for switch",
		.get = mt7530_sw_get_mib,
		.set = NULL,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "mirror_monitor_port",
		.description = "Mirror monitor port",
		.set = mt7530_set_mirror_monitor_port,
		.get = mt7530_get_mirror_monitor_port,
		.max = MT7530_NUM_PORTS - 1
	},
};

/* MT7621 has no global GDM/PPE MIB block: mt7530_sw_get_mib() reads the
 * MT7620 counter base (0x1010) with the MT7620 descriptor table, while
 * the MT7621 counters start at 0x4000 and are laid out per port. Expose
 * only the attributes that are valid on MT7621; the per-port "mib"
 * attribute already uses mt7621_mibs and the correct base.
 */
static const struct switch_attr mt7621_global[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "VLAN mode (1:enabled)",
		.max = 1,
		.id = MT7530_ATTR_ENABLE_VLAN,
		.get = mt7530_get_vlan_enable,
		.set = mt7530_set_vlan_enable,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "mirror_monitor_port",
		.description = "Mirror monitor port",
		.set = mt7530_set_mirror_monitor_port,
		.get = mt7530_get_mirror_monitor_port,
		.max = MT7530_NUM_PORTS - 1
	},
};

static const struct switch_attr mt7621_port[] = {
	{
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.get = mt7621_sw_get_port_mib,
		.set = NULL,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_mirror_rx",
		.description = "Enable mirroring of RX packets",
		.set = mt7530_set_port_mirror_rx,
		.get = mt7530_get_port_mirror_rx,
		.max = 1,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_mirror_tx",
		.description = "Enable mirroring of TX packets",
		.set = mt7530_set_port_mirror_tx,
		.get = mt7530_get_port_mirror_tx,
		.max = 1,
	},
};

static const struct switch_attr mt7621_vlan[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "vid",
		.description = "VLAN ID (0-4094)",
		.set = mt7530_set_vid,
		.get = mt7621_get_vid,
		.max = 4094,
	},
};

static const struct switch_attr mt7530_port[] = {
	{
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.get = mt7530_sw_get_port_mib,
		.set = NULL,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_mirror_rx",
		.description = "Enable mirroring of RX packets",
		.set = mt7530_set_port_mirror_rx,
		.get = mt7530_get_port_mirror_rx,
		.max = 1,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_mirror_tx",
		.description = "Enable mirroring of TX packets",
		.set = mt7530_set_port_mirror_tx,
		.get = mt7530_get_port_mirror_tx,
		.max = 1,
	},
};

static const struct switch_attr mt7530_vlan[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "vid",
		.description = "VLAN ID (0-4094)",
		.set = mt7530_set_vid,
		.get = mt7530_get_vid,
		.max = 4094,
	},
};

static const struct switch_dev_ops mt7621_ops = {
	.attr_global = {
		.attr = mt7621_global,
		.n_attr = ARRAY_SIZE(mt7621_global),
	},
	.attr_port = {
		.attr = mt7621_port,
		.n_attr = ARRAY_SIZE(mt7621_port),
	},
	.attr_vlan = {
		.attr = mt7621_vlan,
		.n_attr = ARRAY_SIZE(mt7621_vlan),
	},
	.get_vlan_ports = mt7530_get_vlan_ports,
	.set_vlan_ports = mt7530_set_vlan_ports,
	.get_port_pvid = mt7530_get_port_pvid,
	.set_port_pvid = mt7530_set_port_pvid,
	.get_port_link = mt7530_get_port_link,
	.set_port_link = mt7530_set_port_link,
	.get_port_stats = mt7621_get_port_stats,
	.apply_config = mt7530_apply_config,
	.reset_switch = mt7530_reset_switch,
	.phy_read16 = mt7530_phy_read16,
	.phy_write16 = mt7530_phy_write16,
};

static const struct switch_dev_ops mt7530_ops = {
	.attr_global = {
		.attr = mt7530_global,
		.n_attr = ARRAY_SIZE(mt7530_global),
	},
	.attr_port = {
		.attr = mt7530_port,
		.n_attr = ARRAY_SIZE(mt7530_port),
	},
	.attr_vlan = {
		.attr = mt7530_vlan,
		.n_attr = ARRAY_SIZE(mt7530_vlan),
	},
	.get_vlan_ports = mt7530_get_vlan_ports,
	.set_vlan_ports = mt7530_set_vlan_ports,
	.get_port_pvid = mt7530_get_port_pvid,
	.set_port_pvid = mt7530_set_port_pvid,
	.get_port_link = mt7530_get_port_link,
	.set_port_link = mt7530_set_port_link,
	.get_port_stats = mt7530_get_port_stats,
	.apply_config = mt7530_apply_config,
	.reset_switch = mt7530_reset_switch,
	.phy_read16 = mt7530_phy_read16,
	.phy_write16 = mt7530_phy_write16,
};

struct switch_dev *
mt7530_probe(struct device *dev, void __iomem *base, struct mii_bus *bus, int vlan)
{
	struct switch_dev *swdev;
	struct mt7530_priv *mt7530;
	struct mt7530_mapping *map;
	int i, ret;

	/* This state belongs to the switch_dev, not to the frame-engine
	 * platform device (which stays alive across ndo_init() retries).
	 * devres would only release it when that device is removed, so a
	 * failed probe -- or an unregister/re-probe cycle -- would leak the
	 * whole private state including the 4095-entry VLAN table. Allocate
	 * explicitly and pair it with mt7530_remove().
	 */
	mt7530 = kzalloc(sizeof(struct mt7530_priv), GFP_KERNEL);
	if (!mt7530)
		return ERR_PTR(-ENOMEM);

	mt7530->base = base;
	mt7530->bus = bus;
	mt7530->global_vlan_enable = vlan;

	mutex_init(&mt7530->reg_mutex);

	swdev = &mt7530->swdev;
	/* register_switch() calls of_switch_load_portmap(), which walks the
	 * swconfig,segment / swconfig,portmap children of swdev->of_node.
	 * Leaving it NULL silently ignores that generic DT port map. Use the
	 * same node mt7530_find_mapping() reads its mediatek,portmap from.
	 */
	swdev->of_node = dev->of_node;
	/* MT7621's built-in switch is probed with bus != NULL (soc_mt7621.c
	 * passes priv->mii_bus), so testing bus first named it "mt7530"
	 * while swdev->ops, the VLAN table size and the VTCR addressing are
	 * selected by CONFIG_SOC_MT7621; it also made the "mt7621" branch
	 * unreachable. Use the same compile-time criterion for the identity
	 * as for the ops, and fall back to the external/built-in MT7530
	 * naming on the other SoCs.
	 */
	if (IS_ENABLED(CONFIG_SOC_MT7621)) {
		swdev->alias = "mt7621";
		swdev->name = "mt7621";
	} else if (bus) {
		swdev->alias = "mt7530";
		swdev->name = "mt7530";
	} else {
		swdev->alias = "mt7620";
		swdev->name = "mt7620";
	}
	swdev->cpu_port = MT7530_CPU_PORT;
	swdev->ports = MT7530_NUM_PORTS;
	swdev->vlans = MT7530_NUM_VLANS;
	if (IS_ENABLED(CONFIG_SOC_MT7621))
		swdev->ops = &mt7621_ops;
	else
		swdev->ops = &mt7530_ops;

	/*
	 * On MT7621 the VLAN table is addressed by VID
	 * (mt7530_write_vlan_entry() -> mt7530_vtcr(priv, 1, vid)), so a
	 * slot whose vid is still 0 makes the first apply write VID 0
	 * MT7530_NUM_VLANS times and never invalidate VID 1..4094.  Seed
	 * vid == index first: that is the invariant mt7530_set_vid()
	 * enforces on MT7621, what mt7621_get_vid() reports, what
	 * mt7530_reset_switch() already restores, and what the removed
	 * full-table clear loop ((priv, i, i, 0, 0)) relied on.  A DT
	 * mapping applied below still overrides it.
	 */
	for (i = 0; i < MT7530_NUM_VLANS; i++)
		mt7530->vlan_entries[i].vid = i;

	map = mt7530_find_mapping(dev->of_node);
	if (map)
		mt7530_apply_mapping(mt7530, map);
	/* the very first apply must program the whole table */
	bitmap_fill(mt7530->vlan_dirty, MT7530_NUM_VLANS);
	ret = mt7530_apply_config(swdev);
	if (ret) {
		/* a VTCR timeout leaves hardware and software VLAN
		 * tables diverged; do not publish such a switch
		 */
		dev_err(dev, "failed to apply initial switch config: %d\n",
			ret);
		mutex_destroy(&mt7530->reg_mutex);
		kfree(mt7530);
		return ERR_PTR(ret);
	}

	/* magic vodoo */
	if (!IS_ENABLED(CONFIG_SOC_MT7621) && bus) {
		u32 hwtrap;

		mutex_lock(&mt7530->reg_mutex);
		ret = mt7530_read_reg(mt7530, REG_HWTRAP, &hwtrap);
		if (!ret && hwtrap != 0x1117edf) {
			dev_info(dev,
				 "fixing up MHWTRAP register - bootloader probably played with it\n");
			ret = mt7530_w32(mt7530, REG_HWTRAP, 0x1117edf);
		}
		mutex_unlock(&mt7530->reg_mutex);
		if (ret) {
			mutex_destroy(&mt7530->reg_mutex);
			kfree(mt7530);
			return ERR_PTR(ret);
		}
	}

	/*
	 * Program the switch before publishing it.  register_switch()
	 * makes the device reachable through the switch API, and
	 * mt7530_apply_config() rewrites every port PCR/PVC, the VLAN
	 * group table, the PVID registers and the mirror setup from the
	 * zeroed priv->vlan_entries[] / priv->ports[].  A configuration
	 * arriving through the API in that window was therefore reverted
	 * a moment later.
	 *
	 * Neither mt7530_apply_mapping() nor mt7530_apply_config()
	 * requires the device to be registered; both reach the private
	 * state through container_of() on the embedded swdev.
	 */
	ret = register_switch(swdev, NULL);
	if (ret) {
		dev_err(dev, "failed to register mt7530\n");
		mutex_destroy(&mt7530->reg_mutex);
		kfree(mt7530);
		return ERR_PTR(ret);
	}

	dev_info(dev, "loaded %s driver\n", swdev->name);

	return swdev;
}
EXPORT_SYMBOL_GPL(mt7530_probe);

void mt7530_remove(struct switch_dev *swdev)
{
	struct mt7530_priv *mt7530 = container_of(swdev,
						  struct mt7530_priv, swdev);

	/* unregister_switch() drains in-flight swconfig calls, so nothing
	 * can still dereference the private state once it returns.
	 */
	unregister_switch(swdev);
	mutex_destroy(&mt7530->reg_mutex);
	kfree(mt7530);
}
EXPORT_SYMBOL_GPL(mt7530_remove);
