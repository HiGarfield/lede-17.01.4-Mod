/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   Copyright (C) 2009-2015 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2015 Felix Fietkau <nbd@nbd.name>
 *   Copyright (C) 2013-2015 Michael Lee <igvtee@gmail.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/phy.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>

#include "mtk_eth_soc.h"
#include "mdio.h"

static int fe_mdio_reset(struct mii_bus *bus)
{
	struct fe_priv *priv = bus->priv;

	/* A timed-out transaction leaves the controller's busy bit set
	 * and every later transfer times out as well. Let the SoC glue
	 * clear that bit so the engine accepts commands again.
	 */
	if (priv->soc->mdio_reset)
		priv->soc->mdio_reset(priv);

	return 0;
}

static void fe_phy_link_adjust(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned long flags;
	int i;

	spin_lock_irqsave(&priv->phy->lock, flags);
	for (i = 0; i < 8; i++) {
		if (priv->phy->phy_node[i]) {
			struct phy_device *phydev = priv->phy->phy[i];
			int status_change = 0;

			/* phy_node[] is set before the connect fills phy[];
			 * a state machine running in that window (or after a
			 * failed connect) must not be dereferenced
			 */
			if (!phydev)
				continue;

			if (phydev->link)
				if (priv->phy->duplex[i] != phydev->duplex ||
				    priv->phy->speed[i] != phydev->speed)
					status_change = 1;

			if (phydev->link != priv->link[i])
				status_change = 1;

			switch (phydev->speed) {
			case SPEED_1000:
			case SPEED_100:
			case SPEED_10:
				priv->link[i] = phydev->link;
				priv->phy->duplex[i] = phydev->duplex;
				priv->phy->speed[i] = phydev->speed;

				if (status_change &&
				    priv->soc->mdio_adjust_link)
					priv->soc->mdio_adjust_link(priv, i);
				break;
			}
		}
	}
	spin_unlock_irqrestore(&priv->phy->lock, flags);
}

/*
 * Drop the phy_node[] slot that refers to phy_node, and release it.
 *
 * The port number is not known on every failure path of
 * fe_connect_phy_node(), so look the node up by identity instead of
 * by index.  Leaving a stale phy_node[] entry behind is not harmless:
 * callers use that array as the "this port has a PHY" flag, so a
 * non-NULL phy_node[i] with a NULL phy[i] later gets dereferenced by
 * fe_phy_link_adjust() and handed to phy_ethtool_sset() in
 * fe_set_settings().
 */
static void fe_phy_forget_node(struct fe_priv *priv,
			       struct device_node *phy_node)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->phy->phy_node); i++) {
		if (priv->phy->phy_node[i] != phy_node)
			continue;

		priv->phy->phy_node[i] = NULL;
		of_node_put(phy_node);
		break;
	}
}

int fe_connect_phy_node(struct fe_priv *priv, struct device_node *phy_node)
{
	const __be32 *_port = NULL;
	struct phy_device *phydev;
	int phy_mode, port;

	_port = of_get_property(phy_node, "reg", NULL);

	/*
	 * "reg" is the MDIO address, so 0..0x1f is what the bus can
	 * address.  struct fe_phy only has 8 entries per port though,
	 * and every index below (phy[], link[]) is used to subscript
	 * those arrays, so the real limit is the array size.  Bound
	 * against the array instead of the bus width, otherwise a
	 * device tree with reg = 8..31 writes past the end of
	 * priv->phy->phy[] / priv->link[].
	 */
	if (!_port ||
	    (be32_to_cpu(*_port) >= ARRAY_SIZE(priv->phy->phy))) {
		pr_err("%s: invalid port id\n", phy_node->name);
		fe_phy_forget_node(priv, phy_node);
		return -EINVAL;
	}
	port = be32_to_cpu(*_port);
	phy_mode = of_get_phy_mode(phy_node);
	if (phy_mode < 0) {
		dev_err(priv->dev, "incorrect phy-mode %d\n", phy_mode);
		fe_phy_forget_node(priv, phy_node);
		return -EINVAL;
	}

	phydev = of_phy_connect(priv->netdev, phy_node, fe_phy_link_adjust,
				0, phy_mode);
	if (!phydev) {
		dev_err(priv->dev, "could not connect to PHY\n");
		fe_phy_forget_node(priv, phy_node);
		return -ENODEV;
	}

	phydev->supported &= PHY_GBIT_FEATURES;
	phydev->advertising = phydev->supported;
	phydev->no_auto_carrier_off = 1;

	dev_info(priv->dev,
		 "connected port %d to PHY at %s [uid=%08x, driver=%s]\n",
		 port, dev_name(&phydev->dev), phydev->phy_id,
		 phydev->drv->name);

	priv->phy->phy[port] = phydev;
	priv->link[port] = 0;

	return 0;
}

static int phy_init(struct fe_priv *priv, struct phy_device *phy)
{
	struct phy_device *p;
	int phy_mode;

	/* honour the phy-mode from the PHY's own DT node instead of
	 * hardcoding MII: RGMII/RMII boards would otherwise attach
	 * with the wrong interface mode
	 */
	/* This is the fallback path for PHYs that the bus scan found and
	 * that therefore have no DT node: their dev.of_node is NULL and
	 * handing NULL to the OF accessors dereferences it.
	 */
	phy_mode = phy->dev.of_node ? of_get_phy_mode(phy->dev.of_node) : -ENODEV;
	if (phy_mode < 0)
		phy_mode = PHY_INTERFACE_MODE_MII;

	p = phy_attach(priv->netdev, dev_name(&phy->dev), phy_mode);
	/* phy_attach() returns an ERR_PTR() on failure, not NULL */
	if (IS_ERR(p))
		return PTR_ERR(p);

	phy->autoneg = AUTONEG_ENABLE;
	phy->speed = 0;
	phy->duplex = 0;
	phy->supported &= IS_ENABLED(CONFIG_NET_MEDIATEK_MDIO_MT7620) ?
			PHY_GBIT_FEATURES : PHY_BASIC_FEATURES;
	phy->advertising = phy->supported | ADVERTISED_Autoneg;

	phy_start_aneg(phy);

	return 0;
}

static int fe_phy_connect(struct fe_priv *priv)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (priv->phy->phy_node[i]) {
			if (!priv->phy_dev) {
				priv->phy_dev = priv->phy->phy[i];
				priv->phy_flags = FE_PHY_FLAG_PORT;
			}
		} else if (priv->mii_bus && priv->mii_bus->phy_map[i]) {
			if (phy_init(priv, priv->mii_bus->phy_map[i]))
				continue;
			set_bit(i, &priv->phy->phy_attached);
			if (!priv->phy_dev) {
				priv->phy_dev = priv->mii_bus->phy_map[i];
				priv->phy_flags = FE_PHY_FLAG_ATTACH;
			}
		}
	}

	return 0;
}

static void fe_phy_disconnect(struct fe_priv *priv)
{
	unsigned long flags;
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->phy->phy); i++) {
		if (priv->phy->phy_fixed[i]) {
			spin_lock_irqsave(&priv->phy->lock, flags);
			priv->link[i] = 0;
			if (priv->soc->mdio_adjust_link)
				priv->soc->mdio_adjust_link(priv, i);
			spin_unlock_irqrestore(&priv->phy->lock, flags);
		} else if (priv->phy->phy[i]) {
			phy_disconnect(priv->phy->phy[i]);
			priv->phy->phy[i] = NULL;
		} else if (priv->mii_bus && priv->mii_bus->phy_map[i] &&
			   test_bit(i, &priv->phy->phy_attached)) {
			/* only detach PHYs that were actually attached by
			 * fe_phy_connect(); detaching a never-attached PHY
			 * unbalances the phy core's references
			 */
			phy_detach(priv->mii_bus->phy_map[i]);
			clear_bit(i, &priv->phy->phy_attached);
		}

		if (priv->phy->phy_node[i]) {
			of_node_put(priv->phy->phy_node[i]);
			priv->phy->phy_node[i] = NULL;
		}
	}

	/*
	 * priv->phy_dev aliases one of the objects released just above and
	 * priv->phy_flags says how it was obtained.  The ethtool paths use
	 * both without re-deriving them from phy[], so a stale value here is
	 * a dangling phy_device pointer.  Both callers of this function
	 * (fe_init's error path and fe_uninit) are terminal, so dropping the
	 * alias unconditionally is correct.
	 */
	priv->phy_dev = NULL;
	priv->phy_flags = 0;

	/* drop the fixed-link pointers as well: they alias DT property
	 * storage, and leaving them behind lets a re-probe of the global
	 * phy_ralink singleton act on stale values
	 */
	for (i = 0; i < ARRAY_SIZE(priv->phy->phy_fixed); i++)
		priv->phy->phy_fixed[i] = NULL;
}

static void fe_phy_start(struct fe_priv *priv)
{
	unsigned long flags;
	int i;

	for (i = 0; i < 8; i++) {
		if (priv->phy->phy_fixed[i]) {
			spin_lock_irqsave(&priv->phy->lock, flags);
			priv->link[i] = 1;
			if (priv->soc->mdio_adjust_link)
				priv->soc->mdio_adjust_link(priv, i);
			spin_unlock_irqrestore(&priv->phy->lock, flags);
		} else if (priv->phy->phy[i]) {
			phy_start(priv->phy->phy[i]);
		}
	}
}

static void fe_phy_stop(struct fe_priv *priv)
{
	unsigned long flags;
	int i;

	for (i = 0; i < 8; i++)
		if (priv->phy->phy_fixed[i]) {
			spin_lock_irqsave(&priv->phy->lock, flags);
			priv->link[i] = 0;
			if (priv->soc->mdio_adjust_link)
				priv->soc->mdio_adjust_link(priv, i);
			spin_unlock_irqrestore(&priv->phy->lock, flags);
		} else if (priv->phy->phy[i]) {
			phy_stop(priv->phy->phy[i]);
		}
}

static struct fe_phy phy_ralink = {
	/* statically initialised: re-running spin_lock_init() on every
	 * fe_mdio_init() would reset the lock while a previous
	 * instance's PHY state machine could still hold it
	 */
	.lock = __SPIN_LOCK_UNLOCKED(phy_ralink.lock),
	.connect = fe_phy_connect,
	.disconnect = fe_phy_disconnect,
	.start = fe_phy_start,
	.stop = fe_phy_stop,
};

int fe_mdio_init(struct fe_priv *priv)
{
	struct device_node *mii_np;
	int err;

	if (!priv->soc->mdio_read || !priv->soc->mdio_write)
		return 0;

	priv->phy = &phy_ralink;

	mii_np = of_get_child_by_name(priv->dev->of_node, "mdio-bus");
	if (!mii_np) {
		dev_err(priv->dev, "no %s child node found", "mdio-bus");
		priv->phy = NULL;
		return -ENODEV;
	}

	if (!of_device_is_available(mii_np)) {
		/*
		 * A disabled mdio-bus is not fatal: ports described with
		 * "mediatek,fixed-link" never touch the MDIO bus, and every
		 * mii_bus dereference is guarded
		 * (rt2880_port_init(), mt7620_port_init(), fe_phy_connect()).
		 * Succeed with priv->phy set and priv->mii_bus left NULL
		 * instead of aborting the whole interface.
		 */
		dev_info(priv->dev, "mdio-bus is disabled, fixed-link only\n");
		of_node_put(mii_np);
		return 0;
	}

	priv->mii_bus = mdiobus_alloc();
	if (!priv->mii_bus) {
		err = -ENOMEM;
		goto err_put_node;
	}

	priv->mii_bus->name = "mdio";
	priv->mii_bus->read = priv->soc->mdio_read;
	priv->mii_bus->write = priv->soc->mdio_write;
	priv->mii_bus->reset = fe_mdio_reset;
	priv->mii_bus->priv = priv;
	priv->mii_bus->parent = priv->dev;

	snprintf(priv->mii_bus->id, MII_BUS_ID_SIZE, "%s", mii_np->name);
	err = of_mdiobus_register(priv->mii_bus, mii_np);
	if (err)
		goto err_free_bus;

	return 0;

err_free_bus:
	/* mdiobus_alloc() pairs with mdiobus_free(), not kfree() */
	mdiobus_free(priv->mii_bus);
err_put_node:
	of_node_put(mii_np);
	priv->mii_bus = NULL;
	/* undo the early priv->phy assignment above: on failure the
	 * caller must not see a phy provider with no mii_bus behind it
	 */
	priv->phy = NULL;
	return err;
}

void fe_mdio_cleanup(struct fe_priv *priv)
{
	if (!priv->mii_bus)
		return;

	mdiobus_unregister(priv->mii_bus);
	of_node_put(priv->mii_bus->dev.of_node);
	mdiobus_free(priv->mii_bus);
	/*
	 * Drop our own reference too.  The "if (!priv->mii_bus) return"
	 * guard above only makes this function idempotent while this
	 * assignment exists; without it a second call would pass the
	 * freed pointer to mdiobus_unregister() again.
	 */
	priv->mii_bus = NULL;
}
