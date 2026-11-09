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
#include <linux/mutex.h>
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
	 *
	 * mdiobus_register() invokes bus->reset without holding
	 * bus->mdio_lock, but every other user of the MT7530 indirect
	 * window holds that lock across its page-select plus two data
	 * transfers, and only takes gsw->mdio_lock per single PIAC
	 * transaction. mt7620_mdio_reset() touches the shared PIAC
	 * START/BUSY bit while holding only gsw->mdio_lock, so without
	 * the outer lock it can slip between another user's page select
	 * and data write and make that write hit the wrong register.
	 * Take the same outer lock (bus->mdio_lock -> gsw->mdio_lock)
	 * as gsw_irq_work_mt7621(), mt7621_gsw_set_imr() and
	 * mt7621_reset_ports(). mdiobus_register() initialises
	 * mdio_lock before calling reset, so there is no self-deadlock.
	 */
	mutex_lock(&bus->mdio_lock);
	if (priv->soc->mdio_reset)
		priv->soc->mdio_reset(priv);
	mutex_unlock(&bus->mdio_lock);

	return 0;
}

static void fe_phy_link_adjust(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned long flags;
	int i;

	spin_lock_irqsave(&priv->phy->lock, flags);
	for (i = 0; i < ARRAY_SIZE(priv->phy->phy); i++) {
		struct phy_device *phydev = priv->phy->phy[i];
		int status_change = 0;

		/* PHYs found by the bus scan are attached straight from
		 * phy_map[] and never stored in phy[]/phy_node[]. Their
		 * state machine calls this callback too, so include those
		 * slots in the walk instead of gating on phy_node[].
		 */
		if (!phydev && priv->mii_bus &&
		    test_bit(i, &priv->phy->phy_attached))
			phydev = priv->mii_bus->phy_map[i];

		/* phy_node[] is set before the connect fills phy[];
		 * a state machine running in that window (or after a
		 * failed connect) must not dereference a NULL phydev.
		 */
		if (!phydev)
			continue;

		if (phydev->link)
			if (priv->phy->duplex[i] != phydev->duplex ||
			    priv->phy->speed[i] != phydev->speed)
				status_change = 1;

		if (phydev->link != priv->link[i])
			status_change = 1;

		/* A PHY can report an unknown speed (phy_init() explicitly
		 * starts with speed 0, and a link-down PHY may do the same).
		 * The switch only caches the negotiated values: keeping the
		 * link state and the adjust_link notification inside it made
		 * those events disappear, leaving priv->link[] and the
		 * carrier at their old value after a link loss.
		 */
		switch (phydev->speed) {
		case SPEED_1000:
		case SPEED_100:
		case SPEED_10:
			priv->phy->duplex[i] = phydev->duplex;
			priv->phy->speed[i] = phydev->speed;
			break;
		}

		priv->link[i] = phydev->link;
		if (status_change && priv->soc->mdio_adjust_link)
			priv->soc->mdio_adjust_link(priv, i);
	}
	spin_unlock_irqrestore(&priv->phy->lock, flags);
}

/*
 * Drop the phy_node[] slot used for this connect, and release it.
 *
 * Callers use that array as the "this port has a PHY" flag, so a
 * non-NULL phy_node[i] with a NULL phy[i] is later skipped by
 * fe_phy_link_adjust() and handed to phy_ethtool_sset(). Every filled
 * slot owns its own of_parse_phandle() reference, so clearing "the
 * first slot whose pointer matches" is wrong when two ports reference
 * the same PHY node: the failing connect belongs to `port`, not to the
 * first match, and the wrong slot would be left behind (or have its
 * reference dropped while still in use).
 */
static void fe_phy_forget_node(struct fe_priv *priv,
			       struct device_node *phy_node, int port)
{
	int valid_port = port >= 0 &&
			 port < ARRAY_SIZE(priv->phy->phy_node);
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->phy->phy_node); i++) {
		/* A valid port identifies exactly the slot being connected.
		 * Only when the port is out of range is there no slot to
		 * identify, so release every matching slot.
		 */
		if (valid_port && i != port)
			continue;
		if (priv->phy->phy_node[i] != phy_node)
			continue;

		priv->phy->phy_node[i] = NULL;
		of_node_put(phy_node);
	}
}

int fe_connect_phy_node(struct fe_priv *priv, struct device_node *phy_node,
			int port)
{
	struct phy_device *phydev;
	unsigned long flags;
	int phy_mode;

	/* phy_node[] is indexed by the eth-port id, not by the MDIO
	 * address held in the PHY node's "reg" property. Using that
	 * address to index phy[] made the two arrays disagree whenever
	 * a PHY sat at an address other than its port id: link adjust,
	 * connect and the ethtool paths all walk them with one index.
	 * of_phy_connect() resolves the address from the node itself.
	 */
	if (port < 0 || port >= ARRAY_SIZE(priv->phy->phy)) {
		dev_err(priv->dev, "invalid port id %d\n", port);
		fe_phy_forget_node(priv, phy_node, port);
		return -EINVAL;
	}

	phy_mode = of_get_phy_mode(phy_node);
	if (phy_mode < 0) {
		dev_err(priv->dev, "incorrect phy-mode %d\n", phy_mode);
		fe_phy_forget_node(priv, phy_node, port);
		return -EINVAL;
	}

	phydev = of_phy_connect(priv->netdev, phy_node, fe_phy_link_adjust,
				0, phy_mode);
	if (!phydev) {
		dev_err(priv->dev, "could not connect to PHY\n");
		fe_phy_forget_node(priv, phy_node, port);
		return -ENODEV;
	}

	phydev->supported &= PHY_GBIT_FEATURES;
	phydev->advertising = phydev->supported;
	phydev->no_auto_carrier_off = 1;

	dev_info(priv->dev,
		 "connected port %d to PHY at %s [uid=%08x, driver=%s]\n",
		 port, dev_name(&phydev->dev), phydev->phy_id,
		 phydev->drv->name);

	/* of_phy_connect() already started the PHY state machine, and
	 * fe_phy_link_adjust() walks phy[] and link[] while holding
	 * phy->lock. Publish both fields under the same lock instead of
	 * handing the state machine a half-updated slot.
	 */
	spin_lock_irqsave(&priv->phy->lock, flags);
	priv->phy->phy[port] = phydev;
	priv->link[port] = 0;
	spin_unlock_irqrestore(&priv->phy->lock, flags);

	return 0;
}

static int phy_init(struct fe_priv *priv, struct phy_device *phy)
{
	struct phy_device *p;
	int phy_mode, err;

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

	/* phy_attach() neither installs phydev->adjust_link nor starts the
	 * state machine, while phy_state_machine() calls that callback
	 * unconditionally and phy_start() only changes the state. A
	 * bus-scanned PHY attached this way would either oops on a NULL
	 * function pointer or never report link changes. phy_connect()
	 * does both and matches of_phy_connect() used for DT PHYs.
	 */
	p = phy_connect(priv->netdev, dev_name(&phy->dev),
			fe_phy_link_adjust, phy_mode);
	/* phy_connect() returns an ERR_PTR() on failure, not NULL */
	if (IS_ERR(p))
		return PTR_ERR(p);

	phy->autoneg = AUTONEG_ENABLE;
	phy->speed = 0;
	phy->duplex = 0;
	/* mdio.o is only built for RT2880/RT3883/MT7620/MT7621, all of
	 * which are gigabit SoCs: rt2880_mdio_link_adjust() programs
	 * GP1_SPEED_1000 and rt2880_port_init() accepts RGMII. Keying the
	 * mask off the MDIO implementation's config symbol forced those
	 * boards down to 10/100 for bus-scanned PHYs, while the very same
	 * PHY connected through a DT handle got PHY_GBIT_FEATURES in
	 * fe_connect_phy_node(). Keep both attachment paths consistent;
	 * the PHY's own supported mask still limits what is advertised.
	 * The 10/100-only RT3050/RT5350 have no mdio ops, so phy_init()
	 * is never reached there.
	 */
	phy->supported &= PHY_GBIT_FEATURES;
	phy->advertising = phy->supported | ADVERTISED_Autoneg;

	/* Ignoring this result reported a successfully attached PHY whose
	 * negotiation was never started (e.g. after a failed MDIO write to
	 * BMCR), leaving the port permanently down with no error to the
	 * caller. fe_phy_connect() only records phy_attached on success,
	 * so roll the attach back here before returning the error or the
	 * PHY core reference would be leaked.
	 */
	err = phy_start_aneg(phy);
	if (err) {
		/* phy_connect() started the state machine: stop it as
		 * well as dropping the attach reference.
		 */
		phy_disconnect(p);
	}

	return err;
}

/* phy[] is indexed by the eth-port id, while mii_bus->phy_map[] is
 * indexed by the MDIO address. A PHY described by a "phy-handle" has
 * already been attached by fe_connect_phy_node(), but it is still
 * present in phy_map[] at its bus address. When the port id and the
 * MDIO address differ, the same phy_device would otherwise be seen a
 * second time by the bus-scan branch of fe_phy_connect() and attached
 * again -- and phy_attached would then make fe_phy_start(),
 * fe_phy_stop() and fe_phy_disconnect() act on it twice as well.
 * Match by object identity instead of by array index.
 */
static bool fe_phy_is_port_phy(struct fe_priv *priv,
			       struct phy_device *phydev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->phy->phy); i++)
		if (priv->phy->phy[i] == phydev)
			return true;

	return false;
}

static int fe_phy_connect(struct fe_priv *priv)
{
	int i, err, ret = 0;

	for (i = 0; i < 8; i++) {
		if (priv->phy->phy_node[i]) {
			if (!priv->phy_dev) {
				priv->phy_dev = priv->phy->phy[i];
				priv->phy_flags = FE_PHY_FLAG_PORT;
			}
		}

		/* phy_node[]/phy[] are keyed by eth-port id, phy_map[] by
		 * MDIO address. A port may reference a PHY at a different
		 * address, so the presence of phy_node[i] says nothing about
		 * the PHY sitting at address i: an else-if here skipped that
		 * bus PHY entirely. fe_phy_is_port_phy() already filters out
		 * the case where both slots hold the same object.
		 */
		if (priv->mii_bus && priv->mii_bus->phy_map[i] &&
		    !fe_phy_is_port_phy(priv,
					priv->mii_bus->phy_map[i])) {
			err = phy_init(priv, priv->mii_bus->phy_map[i]);
			if (err) {
				dev_err(priv->dev,
					"failed to attach PHY at MDIO address %d: %d\n",
					i, err);
				if (!ret)
					ret = err;
				continue;
			}
			set_bit(i, &priv->phy->phy_attached);
			if (!priv->phy_dev) {
				priv->phy_dev = priv->mii_bus->phy_map[i];
				priv->phy_flags = FE_PHY_FLAG_ATTACH;
			}
		}
	}

	/* fe_init() has a rollback path for a failed connect. A usable PHY
	 * or a fixed link is enough to keep going; switch-only SoCs get
	 * their carrier from soc->has_carrier() and legitimately have
	 * neither. Everything else has no link source at all: registering
	 * the netdev would leave it permanently carrier-off with ethtool
	 * and MII ioctls always failing with -ENODEV. In particular, a DT
	 * whose mdio-bus has no enabled PHY produced no phy_init() error,
	 * so ret was still 0 here.
	 */
	if (priv->phy_dev)
		return 0;
	for (i = 0; i < ARRAY_SIZE(priv->phy->phy_fixed); i++)
		if (priv->phy->phy_fixed[i])
			return 0;
	if (priv->soc->has_carrier)
		return 0;

	return ret ? ret : -ENODEV;
}

static void fe_phy_disconnect(struct fe_priv *priv)
{
	unsigned long flags;
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->phy->phy); i++) {
		/* A fixed-link entry and a bus-scanned PHY at the same index
		 * are not mutually exclusive: fe_phy_connect() attaches the
		 * latter whenever phy_node[] is empty, even if phy_fixed[]
		 * is set. Handle the two states independently so the PHY is
		 * always detached again; an else-if chain leaked its phy
		 * core reference and left phy_attached stale for a re-probe.
		 */
		if (priv->phy->phy_fixed[i]) {
			spin_lock_irqsave(&priv->phy->lock, flags);
			priv->link[i] = 0;
			if (priv->soc->mdio_adjust_link)
				priv->soc->mdio_adjust_link(priv, i);
			spin_unlock_irqrestore(&priv->phy->lock, flags);
		}

		if (priv->phy->phy[i]) {
			phy_disconnect(priv->phy->phy[i]);
			priv->phy->phy[i] = NULL;
		}
		if (priv->mii_bus && priv->mii_bus->phy_map[i] &&
		    test_bit(i, &priv->phy->phy_attached)) {
			/* only release PHYs that were actually connected by
			 * fe_phy_connect(); phy_disconnect() also cancels
			 * the state machine started by phy_connect(), which
			 * would otherwise run against a detached phy_device.
			 * A port PHY and a bus PHY at the same index are two
			 * different objects here: phy_attached is only set
			 * when fe_phy_is_port_phy() reported no match, so
			 * this cannot disconnect the same phydev twice.
			 */
			phy_disconnect(priv->mii_bus->phy_map[i]);
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

	/* The loop above only clears an attached bit when the mii_bus and
	 * its phy_map[] entry are still present. A bus that was already
	 * torn down leaves the bit set in the module-wide singleton; the
	 * next probe would then detach a PHY it never attached.
	 */
	priv->phy->phy_attached = 0;
}

static void fe_phy_start(struct fe_priv *priv)
{
	unsigned long flags;
	int i;

	for (i = 0; i < 8; i++) {
		/* see fe_phy_disconnect(): a fixed link does not preclude a
		 * bus-scanned PHY at this index, both states must be handled
		 */
		if (priv->phy->phy_fixed[i]) {
			spin_lock_irqsave(&priv->phy->lock, flags);
			priv->link[i] = 1;
			if (priv->soc->mdio_adjust_link)
				priv->soc->mdio_adjust_link(priv, i);
			spin_unlock_irqrestore(&priv->phy->lock, flags);
		}

		if (priv->phy->phy[i])
			phy_start(priv->phy->phy[i]);
		if (priv->mii_bus && priv->mii_bus->phy_map[i] &&
		    test_bit(i, &priv->phy->phy_attached)) {
			/* PHYs found by the bus scan are attached by
			 * fe_phy_connect() but never stored in phy[].
			 * phy_start_aneg() only kicks off one negotiation;
			 * without phy_start() the state machine never runs
			 * and link up/down changes are never reported.
			 */
			phy_start(priv->mii_bus->phy_map[i]);
		}
	}
}

static void fe_phy_stop(struct fe_priv *priv)
{
	unsigned long flags;
	int i;

	for (i = 0; i < 8; i++) {
		/* see fe_phy_disconnect(): keep fixed-link and PHY handling
		 * independent so start/stop stay balanced on every port
		 */
		if (priv->phy->phy_fixed[i]) {
			spin_lock_irqsave(&priv->phy->lock, flags);
			priv->link[i] = 0;
			if (priv->soc->mdio_adjust_link)
				priv->soc->mdio_adjust_link(priv, i);
			spin_unlock_irqrestore(&priv->phy->lock, flags);
		}

		if (priv->phy->phy[i])
			phy_stop(priv->phy->phy[i]);
		if (priv->mii_bus && priv->mii_bus->phy_map[i] &&
		    test_bit(i, &priv->phy->phy_attached)) {
			/* keep the start/stop calls balanced for bus-scanned
			 * PHYs as well
			 */
			phy_stop(priv->mii_bus->phy_map[i]);
		}
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
