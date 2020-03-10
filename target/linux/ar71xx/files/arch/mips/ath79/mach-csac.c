
/*
 * Qualcomm Atheros CSAC board support
 */

#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"

#define CSAC_GPIO_LED_STATUS		7
#define CSAC_GPIO_LED_WLAN2G		8
#define CSAC_GPIO_LED_WLAN5G		9

#define CSAC_GPIO_BTN_RESET            2
#define CSAC_KEYS_POLL_INTERVAL        20     /* msecs */
#define CSAC_KEYS_DEBOUNCE_INTERVAL    (3 * CSAC_KEYS_POLL_INTERVAL)

#define CSAC_MAC0_OFFSET               0
#define CSAC_WMAC_CALDATA_OFFSET       0x1000


static struct gpio_led csac_leds_gpio[] __initdata =
{
    {
        .name		= "csac:green:wlan2g",
        .gpio		= CSAC_GPIO_LED_WLAN2G,
        .active_low	= 1,
    },
    {
        .name		= "csac:green:wlan5g",
        .gpio		= CSAC_GPIO_LED_WLAN5G,
        .active_low	= 1,
    },
    {
        .name		= "csac:green:system",
        .gpio		= CSAC_GPIO_LED_STATUS,
        .active_low	= 1,
    },
};

static struct gpio_keys_button csac_gpio_keys[] __initdata =
{
    {
        .desc		= "Reset button",
        .type		= EV_KEY,
        .code		= KEY_RESTART,
        .debounce_interval = CSAC_KEYS_DEBOUNCE_INTERVAL,
        .gpio		= CSAC_GPIO_BTN_RESET,
        .active_low	= 1,
    },
};

static struct ar8327_pad_cfg csac_ar8337_pad0_cfg =
{
    .mode = AR8327_PAD_MAC_SGMII,
    .sgmii_delay_en = true,
};

static struct ar8327_platform_data csac_ar8337_data =
{
    .pad0_cfg = &csac_ar8337_pad0_cfg,
    .port0_cfg = {
        .force_link = 1,
        .speed = AR8327_PORT_SPEED_1000,
        .duplex = 1,
        .txpause = 1,
        .rxpause = 1,
    },
};

static struct mdio_board_info csac_mdio0_info[] =
{
    {
        .bus_id = "ag71xx-mdio.0",
        .phy_addr = 0,
        .platform_data = &csac_ar8337_data,
    },
};

static void __init csac_setup(void)
{
    u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

    ath79_register_m25p80(NULL);

    ath79_register_leds_gpio(-1, ARRAY_SIZE(csac_leds_gpio),
                             csac_leds_gpio);

    ath79_register_gpio_keys_polled(-1, CSAC_KEYS_POLL_INTERVAL,
                                    ARRAY_SIZE(csac_gpio_keys),
                                    csac_gpio_keys);

    ath79_register_usb();

    platform_device_register(&ath79_mdio0_device);

    mdiobus_register_board_info(csac_mdio0_info,
                                ARRAY_SIZE(csac_mdio0_info));

    ath79_register_wmac(art + CSAC_WMAC_CALDATA_OFFSET, NULL);
    ath79_register_pci();

    ath79_init_mac(ath79_eth0_data.mac_addr, art + CSAC_MAC0_OFFSET, 0);

    /* GMAC0 is connected to an AR8337 switch */
    ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
    ath79_eth0_data.speed = SPEED_1000;
    ath79_eth0_data.duplex = DUPLEX_FULL;
    ath79_eth0_data.phy_mask = BIT(0);
    ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

    ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_CSAC, "CSAC", "CSAC",
             csac_setup);
