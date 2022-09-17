
/*
 * Qualcomm ACU100 reference board support
 */

#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>


#include <linux/i2c-gpio.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/gpio.h>


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


#define ACU100_GPIO_LED_WLAN		14
//#define ACU100_GPIO_LED_WPS		13
#define ACU100_GPIO_LED_STATUS		13

//#define ACU100_GPIO_LED_WAN		14
/*
#define ACU100_GPIO_LED_LAN1		16
#define ACU100_GPIO_LED_LAN2		15
#define ACU100_GPIO_LED_LAN3		14
#define ACU100_GPIO_LED_LAN4		11
*/

#define ACU100_GPIO_BTN_RESET            4

//#define ACU100_GPIO_BTN_WPS		17

#define ACU100_KEYS_POLL_INTERVAL		20	/* msecs */
#define ACU100_KEYS_DEBOUNCE_INTERVAL	(3 * ACU100_KEYS_POLL_INTERVAL)

#define ACU100_MAC0_OFFSET		0
#define ACU100_MAC1_OFFSET		6
#define ACU100_WMAC_CALDATA_OFFSET	0x1000


/*
static struct gpio_led acu100_leds_gpio[] __initdata = {
	{
		.name		= "acu100:green:status",
		.gpio		= ACU100_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "acu100:green:wlan",
		.gpio		= ACU100_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};


static struct gpio_keys_button acu100_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = ACU100_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ACU100_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};
*/

static struct gpio_keys_button acu100_gpio_keys[] __initdata = {
        {
                .desc           = "Reset button",
                .type           = EV_KEY,
                .code           = KEY_RESTART,
                .debounce_interval = ACU100_KEYS_DEBOUNCE_INTERVAL,
                .gpio           = ACU100_GPIO_BTN_RESET,
                .active_low     = 0,
        },
};

/*
static struct gpio_keys_button acu100_gpio_keys[] __initdata = {
        {
                .desc           = "Reset button",
                .type           = EV_KEY,
                .code           = KEY_RESTART,
                .debounce_interval = ACU100_KEYS_DEBOUNCE_INTERVAL,
                .gpio           = ACU100_GPIO_BTN_RESET,
                .active_low     = 1,
        },
};
*/


static void __init acu100_gpio_led_setup(void)
{

//	ath79_gpio_direction_select(ACU100_GPIO_LED_WLAN, true);
//	ath79_gpio_direction_select(ACU100_GPIO_LED_LAN1, true);
//	ath79_gpio_direction_select(ACU100_GPIO_LED_LAN2, true);
//	ath79_gpio_direction_select(ACU100_GPIO_LED_LAN3, true);
//	ath79_gpio_direction_select(ACU100_GPIO_LED_LAN4, true);

//	ath79_gpio_output_select(ACU100_GPIO_LED_WAN,
//			QCA953X_GPIO_OUT_MUX_LED_LINK5);
//	ath79_gpio_output_select(ACU100_GPIO_LED_LAN1,
//			QCA953X_GPIO_OUT_MUX_LED_LINK1);
//	ath79_gpio_output_select(ACU100_GPIO_LED_LAN2,
//			QCA953X_GPIO_OUT_MUX_LED_LINK2);
//	ath79_gpio_output_select(ACU100_GPIO_LED_LAN3,
//			QCA953X_GPIO_OUT_MUX_LED_LINK3);
//	ath79_gpio_output_select(ACU100_GPIO_LED_LAN4,
//			QCA953X_GPIO_OUT_MUX_LED_LINK4);

//	ath79_register_leds_gpio(-1, ARRAY_SIZE(acu100_leds_gpio),
//			acu100_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ACU100_KEYS_POLL_INTERVAL,
			ARRAY_SIZE(acu100_gpio_keys),
			acu100_gpio_keys);
}


static struct spi_board_info spi_test = {  

    .modalias = "spi_test",  
    //.platform_data = &at25df641_info,  
    .mode = SPI_MODE_0,  
    .irq = 0,  
    .max_speed_hz = 12 * 1000 * 1000,  
    .bus_num = 10,  
    .chip_select = 1, // info: This member is not used for simulating gpio
    .controller_data = (void*)(0),

};


//ldh add tps23861 
#define TPS23861_I2C_SDA_GPIO          3
#define TPS23861_I2C_SCL_GPIO          15
#define TPS23861_I2C_RESET_GPIO        14

#define ARRAY_AND_SIZE(x)	(x), ARRAY_SIZE(x)


// i2c
static struct i2c_gpio_platform_data i2c_bus_data = {
 .sda_pin = TPS23861_I2C_SDA_GPIO,
 .scl_pin = TPS23861_I2C_SCL_GPIO,
 .udelay  = 50,
 //.timeout = 100,

 .sda_is_open_drain = 1, 
 .scl_is_open_drain = 1, 
 .scl_is_output_only= 1,

};


static struct platform_device i2c_bus_device = {
 .name  = "i2c-gpio",
 .id  = 0,// pxa2xx-i2c is bus 0, so start at 1 
 .dev = {
  .platform_data = &i2c_bus_data,
 }
};


static struct i2c_board_info __initdata tps23861_i2c_devices[] = {
 {
  I2C_BOARD_INFO("tps23861", 0x28),
 },

};

static void __init acu100_setup(void)
{

	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *pmac;
	
	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);
	
	spi_register_board_info(&spi_test, 1);

	acu100_gpio_led_setup();
	ath79_register_m25p80(NULL);

	ath79_register_usb();

	//bigred
	//ath79_wmac_set_led_pin(ACU100_GPIO_LED_WLAN);
	ath79_register_wmac(art + ACU100_WMAC_CALDATA_OFFSET, NULL);
  
	ath79_register_mdio(0, 0x0);
	pmac = art + ACU100_WMAC_CALDATA_OFFSET + 2;
	ath79_init_mac(ath79_eth0_data.mac_addr, pmac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, pmac, 2);	

	ath79_setup_ar933x_phy4_switch(true, true);
/*
	ath79_init_mac(ath79_eth0_data.mac_addr, art + ACU100_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + ACU100_MAC1_OFFSET, 0);
*/

#if 1
	/* WAN port */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_register_eth(0);

	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(0);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);
#else
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_init_mac(ath79_eth1_data.mac_addr, art, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, art, 1);
	ath79_register_eth(0);
#endif	

	//ldh add
	//gpio_direction_output(TPS23861_I2C_SDA_GPIO,1);  //sda
	gpio_direction_output(TPS23861_I2C_RESET_GPIO,1);  //reset 
	//gpio_set_value(TPS23861_I2C_SCL_GPIO,0);         //clk 0 auto mode 0k

	platform_device_register(&i2c_bus_device);
	i2c_register_board_info(0, ARRAY_AND_SIZE(tps23861_i2c_devices));

}

//Qualcomm Atheros ACU100 reference board
MIPS_MACHINE(ATH79_MACH_ACU100, "ACU100", "ACU-100",
        acu100_setup);

