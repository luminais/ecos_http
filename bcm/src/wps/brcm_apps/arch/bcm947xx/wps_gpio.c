/*
 * WPS GPIO functions
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wps_gpio.c 241182 2011-02-17 21:50:03Z gmo $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <assert.h>
#include <bcmgpio.h>
#include <bcmnvram.h>

#include <wps.h>
#include <wps_gpio.h>
#include <wps_wl.h>



#ifdef __ECOS
#include <sys/select.h>
#include <sys/time.h>
#define CLOCK_REALTIME	0
#endif

extern int clock_gettime();

#define WPS_GPIO_BUTTON_VALUE	"wps_button"
#define WPS_GPIO_LED_VALUE	"wps_led"
#define WPS_GPIO_LED_Y_VALUE	"wps_led_y"
#define WPS_GPIO_LED_R_VALUE	"wps_led_r"
#define WPS_GPIO_LED_G_VALUE	"wps_led_g"

#define WPS_NV_BTN_ASSERTLVL	"wps_button_assertlvl"
#define WPS_NV_LED_ASSERTLVL	"wps_led_assertlvl"
#define WPS_NV_LED_Y_ASSERTLVL	"wps_led_y_assertlvl"
#define WPS_NV_LED_R_ASSERTLVL	"wps_led_r_assertlvl"
#define WPS_NV_LED_G_ASSERTLVL	"wps_led_g_assertlvl"

#define WPS_GPIO_LED_DEF_NAME	"wps_led_def"


#ifdef _TUDEBUGTRACE
#define WPS_GPIO(fmt, arg...)	printf(fmt, ##arg)
#define WPS_ERROR(fmt, arg...) printf(fmt, ##arg)
#else
#define WPS_GPIO(fmt, arg...)
#define WPS_ERROR(fmt, arg...)
#endif

typedef struct wps_led_s {
	int gpio;
	int y_gpio;
	int r_gpio;
	int g_gpio;
	int assertlvl;
	int y_assertlvl;
	int r_assertlvl;
	int g_assertlvl;
	int count;
	int gpio_active;
	int assertlvl_active;

	uint blinkticks1; /* total number of on/off ticks */
	uint blinkloops1; /* total number of on/off loop */
	uint16 blinkon1; /* # ticks on */
	uint16 blinkoff1; /* # ticks off */
	uint16 looptimes1; /* loop of stage1 */
	uint blinkticks2; /* total number of on/off ticks */
	uint blinkloops2; /* total number of on/off loop */
	uint16 blinkon2; /* # ticks on */
	uint16 blinkoff2; /* # ticks off */
	uint16 looptimes2; /* loop of stage2 */
	int stopstage; /* recycle flag */
} wps_led_t;
wps_led_t wps_led;


typedef struct wps_btn_s {
	int gpio;
	int assertlvl;

	wps_btnpress_t status;
	bool first_time;
	struct timespec start_ts;
	unsigned long prev_assertval;
} wps_btn_t;
wps_btn_t wps_btn;


static void wps_gpio_led_off(int gpio_active, int assertlvl_active);

int
wps_gpio_btn_init()
{
	char *value;

	/* Reset wps_btn structure */
	memset(&wps_btn, 0, sizeof(wps_btn));
	wps_btn.gpio = BCMGPIO_UNDEFINED;
	wps_btn.assertlvl = WPS_BTN_ASSERTLVL;

	wps_btn.status = WPS_NO_BTNPRESS;
	wps_btn.first_time = TRUE;
	wps_btn.prev_assertval = wps_btn.assertlvl ? 0 : 1;

	/* Determine the GPIO pins for the WPS Button */
	wps_btn.gpio = bcmgpio_getpin(WPS_GPIO_BUTTON_VALUE, BCMGPIO_UNDEFINED);

	if (wps_btn.gpio != BCMGPIO_UNDEFINED) {
		WPS_GPIO("Using pin %d for wps button input\n", wps_btn.gpio);

		bcmgpio_disconnect(wps_btn.gpio);	//解决WPS功能不可用，一直显示“Init”的bug，liuke add 2011/11/24
		if (bcmgpio_connect(wps_btn.gpio, BCMGPIO_DIRN_IN) != 0) {
			WPS_ERROR("Error connecting GPIO %d to wps button\n", wps_btn.gpio);
			return -1;
		}

		value = nvram_get(WPS_NV_BTN_ASSERTLVL);
		if (value) {
			wps_btn.assertlvl = atoi(value) ? 1 : 0;
			wps_btn.prev_assertval = wps_btn.assertlvl ? 0 : 1;
			WPS_GPIO("Using assertlvl %d for wps button\n", wps_btn.assertlvl);
		}
	}

	return 0;
}

static int
wps_gpio_led_multi_color_init(char *nv_wps_led, int *gpio_pin,
	char *nv_led_assertlvl, int *led_assertlvl)
{
	char *value;

	*gpio_pin = bcmgpio_getpin(nv_wps_led, BCMGPIO_UNDEFINED);
	if (*gpio_pin != BCMGPIO_UNDEFINED) {
		WPS_GPIO("Using pin %d for %s output\n", *gpio_pin, nv_wps_led);

		if (bcmgpio_connect(*gpio_pin, BCMGPIO_DIRN_OUT) != 0) {
			WPS_GPIO("Error connecting GPIO %d to %s\n",
				*gpio_pin, nv_wps_led);
			*gpio_pin = BCMGPIO_UNDEFINED;
			return -1;
		}

		value = nvram_get(nv_led_assertlvl);
		if (value) {
			*led_assertlvl = atoi(value) ? 1 : 0;
			WPS_GPIO("Using assertlvl %d for %s\n",
				*led_assertlvl, nv_wps_led);
		}
	}

	return 0;
}

static void
wps_gpio_led_multi_color_cleanup(int *gpio_pin)
{
	if (*gpio_pin != BCMGPIO_UNDEFINED) {
		bcmgpio_disconnect(*gpio_pin);
		*gpio_pin = BCMGPIO_UNDEFINED;
	}
}

static void
wps_gpio_led_multi_color_set_active(wps_blinktype_t blinktype)
{
	unsigned long led_mask;
	unsigned long value;
	int new_gpio_active, new_assertlvl_active;
	int old_gpio_active = BCMGPIO_UNDEFINED;
	int old_assertlvl_active = WPS_LED_ASSERTLVL;

	new_gpio_active = wps_led.gpio;
	new_assertlvl_active = wps_led.assertlvl;

	switch ((int)blinktype) {
	case WPS_BLINKTYPE_INPROGRESS: /* Multi-color yellow */
		if (wps_led.y_gpio != BCMGPIO_UNDEFINED) {
			new_gpio_active = wps_led.y_gpio;
			new_assertlvl_active = wps_led.y_assertlvl;
		}
		break;
	case WPS_BLINKTYPE_ERROR: /* Multi-color red */
	case WPS_BLINKTYPE_OVERLAP: /* Multi-color red */
		if (wps_led.r_gpio != BCMGPIO_UNDEFINED) {
			new_gpio_active = wps_led.r_gpio;
			new_assertlvl_active = wps_led.r_assertlvl;
		}
		break;
	case WPS_BLINKTYPE_SUCCESS: /* Multi-color green */
		if (wps_led.g_gpio != BCMGPIO_UNDEFINED) {
			new_gpio_active = wps_led.g_gpio;
			new_assertlvl_active = wps_led.g_assertlvl;
		}
		break;
	default:
		new_gpio_active = BCMGPIO_UNDEFINED;
		new_assertlvl_active = WPS_LED_ASSERTLVL;
		break;
	}

	if (wps_led.gpio_active != new_gpio_active) {
		old_gpio_active = wps_led.gpio_active;
		old_assertlvl_active = wps_led.assertlvl_active;
	}

	/* change active led first */
	wps_led.gpio_active = new_gpio_active;
	wps_led.assertlvl_active = new_assertlvl_active;

	/* then off old active led */
	if (old_gpio_active == BCMGPIO_UNDEFINED)
		return;

	led_mask = ((unsigned long)1 << old_gpio_active);
	value = ((unsigned long) ~old_assertlvl_active << old_gpio_active);
	value &= led_mask;
	bcmgpio_out(led_mask, value);

	return;
}

static void
wps_gpio_led_multi_color_stop()
{
	switch (wps_led.count) {
	case 0:
		if (wps_led.gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_off(wps_led.gpio, wps_led.assertlvl);
		break;

	case 2:
	case 3:
		/* do multi-color cleanup */
		if (wps_led.y_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_off(wps_led.y_gpio, wps_led.y_assertlvl);
		if (wps_led.r_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_off(wps_led.r_gpio, wps_led.r_assertlvl);
		if (wps_led.g_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_off(wps_led.g_gpio, wps_led.g_assertlvl);
		break;

	default:
		break;
	}

	return;
}

/* Reset blink related variables */
static void
wps_gpio_led_blink_reset()
{
	wps_led.blinkticks1 = 0;
	wps_led.blinkloops1 = 0;
	wps_led.blinkon1 = 0;
	wps_led.blinkoff1 = 0;
	wps_led.looptimes1 = 0;
	wps_led.blinkticks2 = 0;
	wps_led.blinkloops2 = 0;
	wps_led.blinkon2 = 0;
	wps_led.blinkoff2 = 0;
	wps_led.looptimes2 = 0;
	wps_led.stopstage = 0;
}

int
wps_gpio_led_init()
{
	char *wps_led_def;

	/* Reset wps_led structure */
	memset(&wps_led, 0, sizeof(wps_led));
	wps_led.gpio = BCMGPIO_UNDEFINED;
	wps_led.y_gpio = BCMGPIO_UNDEFINED;
	wps_led.r_gpio = BCMGPIO_UNDEFINED;
	wps_led.g_gpio = BCMGPIO_UNDEFINED;
	wps_led.assertlvl = WPS_LED_ASSERTLVL;
	wps_led.y_assertlvl = WPS_LED_ASSERTLVL;
	wps_led.r_assertlvl = WPS_LED_ASSERTLVL;
	wps_led.g_assertlvl = WPS_LED_ASSERTLVL;
	wps_led.count = 0;
	wps_led.gpio_active = BCMGPIO_UNDEFINED;
	wps_led.assertlvl_active = WPS_LED_ASSERTLVL;

	/* Determine the GPIO pins for the WPS led */
	/*
	 * WPS led configuration rules:
	 * 1. if "wps_led_y" && "wps_led_r" && "wps_led_g" are valid then "wps_led" is unused.
	 * 2. if two of ("wps_led_y", "wps_led_r", "wps_led_g") are valid then wps_led_def is used
	 *    to indicate the default wps led.
	 * 3. if none of ("wps_led_y", "wps_led_r", "wps_led_g") are valid then "wps_led" is used.
	 * 4. if one of the initializations failed, return -1.
	 */
	if ((wps_led.y_gpio = bcmgpio_getpin(WPS_GPIO_LED_Y_VALUE, BCMGPIO_UNDEFINED))
		!= BCMGPIO_UNDEFINED)
		wps_led.count++;
	if ((wps_led.r_gpio = bcmgpio_getpin(WPS_GPIO_LED_R_VALUE, BCMGPIO_UNDEFINED))
		!= BCMGPIO_UNDEFINED)
		wps_led.count++;
	if ((wps_led.g_gpio = bcmgpio_getpin(WPS_GPIO_LED_G_VALUE, BCMGPIO_UNDEFINED))
		!= BCMGPIO_UNDEFINED)
		wps_led.count++;

	switch (wps_led.count) {
	case 0:
		wps_led.gpio = bcmgpio_getpin(WPS_GPIO_LED_VALUE, BCMGPIO_UNDEFINED);
		if (wps_led.gpio == BCMGPIO_UNDEFINED) {
			WPS_GPIO("No default wps led configuration\n");
			return -1;
		} else {
			return wps_gpio_led_multi_color_init(WPS_GPIO_LED_VALUE, &wps_led.gpio,
				WPS_NV_LED_ASSERTLVL, &wps_led.assertlvl);
		}
	case 2:
		/* does default wps led match one of two */
		wps_led_def = nvram_get(WPS_GPIO_LED_DEF_NAME);
		if (wps_led_def == NULL) {
			WPS_GPIO("No default wps led configuration\n");
			return -1;
		} else {
			wps_led.gpio = atoi(wps_led_def);
			if (wps_led.y_gpio != wps_led.gpio &&
			    wps_led.r_gpio != wps_led.gpio &&
			    wps_led.g_gpio != wps_led.gpio) {
				WPS_GPIO("Default wps led configuration does not match"
					"any multi-color led\n");
				return -1;
			}
			WPS_GPIO("Using pin %d for wps led default\n", wps_led.gpio);
		}
		/* do multi-color init */
		if (wps_led.y_gpio != BCMGPIO_UNDEFINED &&
			wps_gpio_led_multi_color_init(WPS_GPIO_LED_Y_VALUE, &wps_led.y_gpio,
			WPS_NV_LED_Y_ASSERTLVL, &wps_led.y_assertlvl) != 0) {
			return -1;
		} else if (wps_led.r_gpio != BCMGPIO_UNDEFINED &&
			wps_gpio_led_multi_color_init(WPS_GPIO_LED_R_VALUE, &wps_led.r_gpio,
			WPS_NV_LED_R_ASSERTLVL, &wps_led.r_assertlvl) != 0) {
			if (wps_led.y_gpio != BCMGPIO_UNDEFINED)
				wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
			return -1;
		} else if (wps_led.g_gpio != BCMGPIO_UNDEFINED &&
			wps_gpio_led_multi_color_init(WPS_GPIO_LED_G_VALUE, &wps_led.g_gpio,
			WPS_NV_LED_G_ASSERTLVL, &wps_led.g_assertlvl) != 0) {
			if (wps_led.y_gpio != BCMGPIO_UNDEFINED)
				wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
			if (wps_led.r_gpio != BCMGPIO_UNDEFINED)
				wps_gpio_led_multi_color_cleanup(&wps_led.r_gpio);
			return -1;
		}
		break;
	case 3:
		if (wps_gpio_led_multi_color_init(WPS_GPIO_LED_Y_VALUE, &wps_led.y_gpio,
			WPS_NV_LED_Y_ASSERTLVL, &wps_led.y_assertlvl) != 0) {
			return -1;
		} else if (wps_gpio_led_multi_color_init(WPS_GPIO_LED_R_VALUE, &wps_led.r_gpio,
			WPS_NV_LED_R_ASSERTLVL, &wps_led.r_assertlvl) != 0) {
			wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
			return -1;
		} else if (wps_gpio_led_multi_color_init(WPS_GPIO_LED_G_VALUE, &wps_led.g_gpio,
			WPS_NV_LED_G_ASSERTLVL, &wps_led.g_assertlvl) != 0) {
			wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
			wps_gpio_led_multi_color_cleanup(&wps_led.r_gpio);
			return -1;
		}
		/*
		 * set default wps led to wps_led.y_gpio, we use wps_led.gpio
		 * to indicate wps gpio led init successed.
		 */
		wps_led.gpio = wps_led.y_gpio;
		break;

	default:
		WPS_GPIO("Wrong wps led NV configuration, count %d\n", wps_led.count);
			return -1;
	}

	return 0;
}

void
wps_gpio_btn_cleanup()
{
	if (wps_btn.gpio != BCMGPIO_UNDEFINED) {
		bcmgpio_disconnect(wps_btn.gpio);
		wps_btn.gpio = BCMGPIO_UNDEFINED;
	}
}

void
wps_gpio_led_cleanup()
{
	switch (wps_led.count) {
	case 0:
		if (wps_led.gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_multi_color_cleanup(&wps_led.gpio);
		break;

	case 2:
	case 3:
		/* do multi-color cleanup */
		if (wps_led.y_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
		if (wps_led.r_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_multi_color_cleanup(&wps_led.r_gpio);
		if (wps_led.g_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_multi_color_cleanup(&wps_led.g_gpio);
		break;

	default:
		break;
	}
}

#define WPS_RESET_BUTTON_GPIO 20

#ifdef __CONFIG_WPS_LED__
extern void wps_button(void);
extern void start_wps_negotiate();
#endif
extern void reset_button(void);

#ifdef __CONFIG_TENDA_HTTPD_V3__
extern int ate_is_doing;
#endif

wps_btnpress_t wps_gpio_btn_check()
{
	unsigned long btn_mask;
	unsigned long value;
	static cyg_tick_count_t last_tick;
	cyg_tick_count_t cur_tick;
	static bool first_time = TRUE;
	char *security_mode;
	char *encry_type;
	
	btn_mask = ((unsigned long)1 << WPS_RESET_BUTTON_GPIO);

	bcmgpio_in_other(btn_mask, &value);
	value >>= WPS_RESET_BUTTON_GPIO;

	cur_tick=cyg_current_time();
	security_mode = nvram_safe_get("wl_akm");
	encry_type = nvram_safe_get("wl_crypto");

	if (value == 0)
	{
		if(first_time==TRUE)
		{
			last_tick=cyg_current_time();
			first_time=FALSE;
		}
		else
		{

			if ((cur_tick - last_tick)/100 >= WPS_LONG_PRESSTIME && nvram_match("restore_defaults","0"))
			{
				reset_button();	
				
				first_time=TRUE;
				return WPS_NO_BTNPRESS;
			}
		}
		return WPS_NO_BTNPRESS;
	}
	else
	{
		if(first_time==FALSE)
		{
			if (((cur_tick - last_tick)>15)&& (cur_tick - last_tick)/100 < WPS_LONG_PRESSTIME)
			{
				char *wireless_enable = nvram_safe_get("wl_radio");
				
				if (!strcmp(wireless_enable, "1")){
			#ifdef __CONFIG_WPS_LED__
				/*	pxy revise, 2013.12.06
				 * 	no matter wps mode is enabled or disabled, start it.
				 */
				 if(0 == strcmp(security_mode, "") 
				 	|| 0 == strcmp(security_mode, "psk2") && 0 != strcmp(encry_type, "tkip")
				 	|| 0 == strcmp(security_mode, "psk psk2") && 0 != strcmp(encry_type, "tkip"))
				 {
				 	#ifdef __CONFIG_TENDA_HTTPD_V3__
					if(ate_is_doing == 0)
					#endif
					{
						first_time=TRUE;
						start_wps_negotiate();
						return WPS_NO_BTNPRESS;
					}
				 }
				#endif
					//first_time=TRUE;
					//return WPS_NO_BTNPRESS;
				}
			}
			first_time=TRUE;
			return WPS_NO_BTNPRESS;
		}
		first_time=TRUE;
		return WPS_NO_BTNPRESS;
	}
}

#if 0
wps_btnpress_t wps_gpio_btn_check()
{
	unsigned long btn_mask;
	unsigned long value;
	static cyg_tick_count_t last_tick;
	cyg_tick_count_t cur_tick;
	static bool first_time = TRUE;

	
	if (wps_btn.gpio == BCMGPIO_UNDEFINED)
		return WPS_NO_BTNPRESS;

	btn_mask = ((unsigned long)1 << wps_btn.gpio);

	bcmgpio_in(btn_mask, &value);
	value >>= wps_btn.gpio;

	cur_tick=cyg_current_time();
	

	if (value == wps_btn.assertlvl)
	{

		if(first_time==TRUE)
		{
			last_tick=cyg_current_time();
			first_time=FALSE;

		}
		else
		{

			if ((cur_tick - last_tick)/100 >= WPS_LONG_PRESSTIME && nvram_match("restore_defaults","0"))
			{
				reset_button();	//Notice system reboot, system restart.
				
				first_time=TRUE;
				return WPS_NO_BTNPRESS;//WPS_LONG_BTNPRESS;
			}
		}
		return WPS_NO_BTNPRESS;//WPS_SHORT_BTNPRESS;

	}
	else
	{
		if(first_time==FALSE)
		{
			if (((cur_tick - last_tick)>15)&& (cur_tick - last_tick)/100 < WPS_LONG_PRESSTIME)
			{
				diag_printf("wps start \n");

				char *wps_enable=NULL;
				char *wps_vif_enable=NULL;
				char *wireless_enable=NULL;
				wps_enable=nvram_safe_get("wl0_wps_mode");
				wps_vif_enable=nvram_safe_get("wl0.1_wps_mode");
				wireless_enable = nvram_safe_get("wl_radio");
				
			//	if (strcmp(wireless_enable, "1") == 0 &&
			//		(strcmp(wps_enable, "enabled") == 0 || strcmp(wps_vif_enable, "enabled") == 0)){
#ifdef __CONFIG_WPS_LED__
				//	wps_button();
				/*	pxy revise, 2013.12.06
				 * 	no matter wps mode is enabled or disabled, start it.
				 */
					start_wps_negotiate();
#endif
					first_time=TRUE;
					return WPS_NO_BTNPRESS;
			//	}
			}
			first_time=TRUE;
			return WPS_NO_BTNPRESS;//WPS_SHORT_BTNPRESS;
		}
		first_time=TRUE;
		return WPS_NO_BTNPRESS;
	}
}


wps_btnpress_t wps_gpio_btn_check_other()
{
	unsigned long btn_mask;
	unsigned long value;
	static cyg_tick_count_t last_tick;
	cyg_tick_count_t cur_tick;
	static bool first_time = TRUE;


	btn_mask = ((unsigned long)1 << 20);


	bcmgpio_in_other(btn_mask, &value);
	
	value >>= 20;

	cur_tick=cyg_current_time();

	if (value == 0)
	{

		if(first_time==TRUE)
		{
			last_tick=cyg_current_time();
			first_time=FALSE;
		}
		else
		{	
			if ((cur_tick - last_tick)/100 >= WPS_LONG_PRESSTIME && nvram_match("restore_defaults","0"))
			{
				reset_button();	//Notice system reboot, system restart.
				
				first_time=TRUE;
				return WPS_NO_BTNPRESS;//WPS_LONG_BTNPRESS;
			}
		}	
	}
	else if (value == 1)
	{
		first_time=TRUE;
		return WPS_NO_BTNPRESS;//WPS_SHORT_BTNPRESS;
	}
}
#endif

wps_btnpress_t
wps_gpio_btn_pressed()
{
	wps_btn.status = wps_gpio_btn_check();
	return wps_btn.status;
}

static void
wps_gpio_led_on()
{
	unsigned long led_mask;
	unsigned long value;

	if (wps_led.gpio_active == BCMGPIO_UNDEFINED)
		return;

	led_mask = ((unsigned long)1 << wps_led.gpio_active);
	value = ((unsigned long) wps_led.assertlvl_active << wps_led.gpio_active);
	bcmgpio_out(led_mask, value);
}

static void
wps_gpio_led_off(int gpio_active, int assertlvl_active)
{
	unsigned long led_mask;
	unsigned long value;

	if (gpio_active == BCMGPIO_UNDEFINED)
		return;

	led_mask = ((unsigned long)1 << gpio_active);
	value = ((unsigned long) ~assertlvl_active << gpio_active);
	value &= led_mask;

	bcmgpio_out(led_mask, value);
}

/* call wps_gpio_led_blink_timer in each WPS_LED_BLINK_TIME_UNIT */
void
wps_gpio_led_blink_timer()
{
	if (wps_led.gpio == BCMGPIO_UNDEFINED ||
	    wps_led.gpio_active == BCMGPIO_UNDEFINED)
		return;

	/* first stage check */
	if (wps_led.blinkticks1) {
		if (wps_led.blinkticks1 > wps_led.blinkoff1) {
			wps_gpio_led_on();
		} else {
			wps_gpio_led_off(wps_led.gpio_active, wps_led.assertlvl_active);
		}

		/* first stage finished, check second stage */
		if (--wps_led.blinkticks1 == 0) {
			/* check loop */
			if (wps_led.blinkloops1 && --wps_led.blinkloops1) {
				/* need to loop again */
				wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
				return;
			}

			if (wps_led.stopstage == 1) {
				/* clear all blinktick number to stop blink */
				wps_led.blinkticks1 = 0;
				wps_led.blinkticks2 = 0;
				return; /* break after first stage */
			}
		}
	}

	if (wps_led.blinkticks1 == 0 && wps_led.blinkticks2) {
		if (wps_led.blinkticks2 > wps_led.blinkoff2) {
			wps_gpio_led_on();
		} else {
			wps_gpio_led_off(wps_led.gpio_active, wps_led.assertlvl_active);
		}

		/* second stage finished */
		if (--wps_led.blinkticks2 == 0) {
			/* check loop */
			if (wps_led.blinkloops2 && --wps_led.blinkloops2) {
				/* need to loop again */
				wps_led.blinkticks2 = wps_led.blinkon2 + wps_led.blinkoff2;
				return;
			}

			if (wps_led.stopstage == 2) {
				/* clear all blinktick number to stop blink */
				wps_led.blinkticks1 = 0;
				wps_led.blinkticks2 = 0;
				return; /* break after second satge */
			}
		}
	}

	if (wps_led.blinkticks1 == 0 &&
		wps_led.blinkticks2 == 0 &&
		wps_led.stopstage == 0) {
		/* recycle blinkticks */
		wps_led.blinkticks2 = wps_led.blinkon2 + wps_led.blinkoff2;
		wps_led.blinkloops2 = wps_led.looptimes2;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		wps_led.blinkloops1 = wps_led.looptimes1;
	}

	return;
}

void
wps_gpio_led_blink(wps_blinktype_t blinktype)
{
	if (wps_led.gpio == BCMGPIO_UNDEFINED)
		return;

	switch ((int)blinktype) {
	case WPS_BLINKTYPE_INPROGRESS: /* Multi-color yellow */
		wps_gpio_led_blink_reset();
		wps_led.blinkon1 = 200 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkoff1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		break;

	case WPS_BLINKTYPE_ERROR: /* Multi-color red */
		wps_gpio_led_blink_reset();
		wps_led.blinkon1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkoff1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		break;

	case WPS_BLINKTYPE_OVERLAP: /* Multi-color red */
		wps_gpio_led_blink_reset();
		wps_led.blinkon1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkoff1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		wps_led.looptimes1 = 5;
		wps_led.blinkloops1 = wps_led.looptimes1;
		wps_led.blinkon2 = 0;
		wps_led.blinkoff2 = 500 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkticks2 = wps_led.blinkon2 + wps_led.blinkoff2;
		break;

	case WPS_BLINKTYPE_SUCCESS: /* Multi-color green */
		wps_gpio_led_blink_reset();
		wps_led.blinkon1 = (1000 * 300)  / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkoff1 = 1;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		wps_led.stopstage = 1;
		break;

	case WPS_BLINKTYPE_STOP_MULTI: /* clear all multi-color leds */
		wps_gpio_led_blink_reset();
		wps_gpio_led_multi_color_stop();
		return; /* return now */

	default:
		wps_gpio_led_blink_reset();
		wps_gpio_led_off(wps_led.gpio_active, wps_led.assertlvl_active);
		break;
	}

	wps_gpio_led_multi_color_set_active(blinktype);

	return;
}

//tenda add
#ifdef __CONFIG_SYS_LED__
void sys_led_test_init()
{
       int sys_led_test_gpio = -1;
       /* Initial */
       sys_led_test_gpio = bcmgpio_getpin("sys_led", -1);
	diag_printf("sys_led_test_gpio=%d\n", sys_led_test_gpio);
       if (sys_led_test_gpio == -1) {
              printf("No default sys led configuration\n");
              return;
       } else {
              printf("Using pin %d for sys_led output\n", sys_led_test_gpio);
              if (bcmgpio_connect(sys_led_test_gpio, 1 /* BCMGPIO_DIRN_OUT */) != 0) {
                     printf("Error connecting GPIO %d to sys_led\n", sys_led_test_gpio);
                     return;
              }
       }
}
void sys_led_test_proc(int on)
{
	char *value;
	int sys_led_test_gpio,sys_led_test_assertlvl;
	unsigned long led_mask,led_value;

	value = nvram_safe_get("sys_led_assertlvl");
	sys_led_test_assertlvl = atoi(value) ? 1 : 0;

	sys_led_test_gpio = bcmgpio_getpin("sys_led", -1);
	if(sys_led_test_gpio == -1){
		diag_printf("sys_led_test_gpio error!\n");
		return;
	}
	
	if(on){
              led_mask = ((unsigned long)1 << sys_led_test_gpio);
              led_value = ((unsigned long)sys_led_test_assertlvl << sys_led_test_gpio);
              bcmgpio_out(led_mask, led_value);
	}else{
              led_mask = ((unsigned long)1 << sys_led_test_gpio);
              led_value = ((unsigned long) ~sys_led_test_assertlvl << sys_led_test_gpio);
              led_value &= led_mask;
              bcmgpio_out(led_mask, led_value);
	}
}
#endif/* __CONFIG_SYS_LED__*/

#ifdef __CONFIG_WPS_LED__
static int wps_led_test_gpio = -1;
static int wps_led_test_assertlvl = 1;
 int wps_led_on = 0;
extern int wps_led_init_g;

void wps_led_test_init(void){
	char *value;	   
       wps_led_test_gpio = bcmgpio_getpin("wps_led", -1);
	diag_printf("wps_led_test_gpio=%d\n", wps_led_test_gpio);
       if (wps_led_test_gpio == -1) {
              printf("No default wps led configuration\n");
              return;
       } else {
              if (bcmgpio_connect(wps_led_test_gpio, 1 /* BCMGPIO_DIRN_OUT */) != 0) {
                     printf("Error connecting GPIO %d to wps_led\n", wps_led_test_gpio);
                     return;
              }
              value = nvram_get("wps_led_assertlvl");
              if (value) {
			diag_printf("wps_led_assertlvl=%s\n", value);
                     wps_led_test_assertlvl = atoi(value) ? 1 : 0;
                     printf("Using assertlvl %d for sys_led\n", wps_led_test_assertlvl);
              }
       }
	return;
}

void wps_led_test_on(void){
       unsigned long led_mask;
       unsigned long led_value;	   
        /* off */
      if(wps_led_on == 1 || wps_led_init_g == 1){
      		led_mask = ((unsigned long)1 << wps_led_test_gpio);
      		led_value = ((unsigned long)wps_led_test_assertlvl << wps_led_test_gpio);
	      // diag_printf("wps led off\n");
	       bcmgpio_out(led_mask, led_value);
		wps_led_on = 0;
      	}
	return;
}

extern void wps_led_test_off(void);
void wps_led_test_off(void){
       unsigned long led_mask;
       unsigned long led_value;

          /* on*/
	if(wps_led_on == 0 || wps_led_init_g == 1){
      		led_mask = ((unsigned long)1 << wps_led_test_gpio);
		led_value = ((unsigned long) ~wps_led_test_assertlvl << wps_led_test_gpio);
	  	led_value &= led_mask;
		//diag_printf("wps led on\n");
		bcmgpio_out(led_mask, led_value);
		wps_led_on = 1;
	}
      return;
}
#endif /*__CONFIG_WPS_LED__*/

//end

