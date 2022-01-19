/*
 *  MERCURY MW450R v3 board support
 *
 *  Copyright (C) 2015 Matthias Schiffer <mschiffer@universe-factory.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"


#define MW450R_V3_GPIO_LED_WAN		2
#define MW450R_V3_GPIO_LED_LAN1		7
#define MW450R_V3_GPIO_LED_LAN2		6
#define MW450R_V3_GPIO_LED_LAN3		5
#define MW450R_V3_GPIO_LED_LAN4		4
#define MW450R_V3_GPIO_LED_SYSTEM		18

#define MW450R_V3_GPIO_BTN_RESET		1

#define MW450R_V3_KEYS_POLL_INTERVAL	20
#define MW450R_V3_KEYS_DEBOUNCE_INTERVAL	(3 * MW450R_V3_KEYS_POLL_INTERVAL)


static struct gpio_led mw450r_v3_leds_gpio[] __initdata = {
	{
		.name		= "mw450r:green:wan",
		.gpio		= MW450R_V3_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "mw450r:green:lan1",
		.gpio		= MW450R_V3_GPIO_LED_LAN1,
		.active_low	= 1,
	},
	{
		.name		= "mw450r:green:lan2",
		.gpio		= MW450R_V3_GPIO_LED_LAN2,
		.active_low	= 1,
	},
	{
		.name		= "mw450r:green:lan3",
		.gpio		= MW450R_V3_GPIO_LED_LAN3,
		.active_low	= 1,
	},
	{
		.name		= "mw450r:green:lan4",
		.gpio		= MW450R_V3_GPIO_LED_LAN4,
		.active_low	= 1,
	},
	{
		.name		= "mw450r:green:system",
		.gpio		= MW450R_V3_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_keys_button mw450r_v3_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MW450R_V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MW450R_V3_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};


static const char *mw450r_v3_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data mw450r_v3_flash_data = {
	.part_probes	= mw450r_v3_part_probes,
};


static void __init mw450r_v3_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(&mw450r_v3_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(mw450r_v3_leds_gpio),
				 mw450r_v3_leds_gpio);

	ath79_register_gpio_keys_polled(-1, MW450R_V3_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(mw450r_v3_gpio_keys),
					mw450r_v3_gpio_keys);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	ath79_switch_data.phy4_mii_en = 1;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_wmac(ee, mac);

}

MIPS_MACHINE(ATH79_MACH_MW450R_V3, "MW450R-v3", "MERCURY MW450R v3",
	     mw450r_v3_setup);
