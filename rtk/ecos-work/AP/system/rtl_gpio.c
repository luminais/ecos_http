/*
 * Copyright 2010 Realtek Semiconductor Corp.
 */

//#define CONFIG_USING_JTAG 1

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/hal/bspchip.h>
#include <cyg/infra/diag.h>
#include <cyg/kernel/kapi.h>
#include "sys_init.h"

#if !defined(CONFIG_CMJ_GPIO)
#undef CONFIG_CUTE_MAHJONG
#undef CONFIG_CUTE_MAHJONG_SELECTABLE
#endif

#define RTL_GPIO_CNR_GPIOA1 (1<<1)
#define RTL_GPIO_CNR_GPIOA2 (1<<2)
#define RTL_GPIO_CNR_GPIOA3 (1<<3)
#define RTL_GPIO_CNR_GPIOA4 (1<<4)
#define RTL_GPIO_CNR_GPIOA5 (1<<5)
#define RTL_GPIO_CNR_GPIOA6 (1<<6)
#define RTL_GPIO_CNR_GPIOB3 (1<<11)
#define RTL_GPIO_CNR_GPIOB4 (1<<12)
#define RTL_GPIO_CNR_GPIOB6 (1<<14)
#define RTL_GPIO_CNR_GPIOB7 (1<<15) 
#define RTL_GPIO_CNR_GPIOC0 (1<<16)
#define RTL_GPIO_CNR_GPIOC1 (1<<17)
#define RTL_GPIO_CNR_GPIOC2 (1<<18)
#define RTL_GPIO_CNR_GPIOC3 (1<<19)
#define RTL_GPIO_CNR_GPIOD0 (1<<24)
#define RTL_GPIO_CNR_GPIOE2 (1<<2)
#define RTL_GPIO_CNR_GPIOG6 (1<<22)
#define RTL_GPIO_CNR_GPIOH0 (1<<24)
#define RTL_GPIO_CNR_GPIOH1 (1<<25)
#define RTL_GPIO_CNR_GPIOH2 (1<<26)

#define RTL_GPIO_DIR_GPIOA1 (1<<1) 
#define RTL_GPIO_DIR_GPIOA2 (1<<2) 
#define RTL_GPIO_DIR_GPIOA3 (1<<3) 
#define RTL_GPIO_DIR_GPIOA4 (1<<4) 
#define RTL_GPIO_DIR_GPIOA5 (1<<5) 
#define RTL_GPIO_DIR_GPIOA6 (1<<6) 
#define RTL_GPIO_DIR_GPIOB3 (1<<11)
#define RTL_GPIO_DIR_GPIOB4 (1<<12)
#define RTL_GPIO_DIR_GPIOB6 (1<<14)
#define RTL_GPIO_DIR_GPIOB7 (1<<15)
#define RTL_GPIO_DIR_GPIOC0 (1<<16)
#define RTL_GPIO_DIR_GPIOC1 (1<<17)
#define RTL_GPIO_DIR_GPIOC2 (1<<18)
#define RTL_GPIO_DIR_GPIOC3 (1<<19)
#define RTL_GPIO_DIR_GPIOD0 (1<<24)
#define RTL_GPIO_DIR_GPIOE2 (1<<2)
#define RTL_GPIO_DIR_GPIOG6 (1<<22)
#define RTL_GPIO_DIR_GPIOH0 (1<<24)
#define RTL_GPIO_DIR_GPIOH1 (1<<25)
#define RTL_GPIO_DIR_GPIOH2 (1<<26)

#define RTL_GPIO_DAT_GPIOA1 (1<<1) 
#define RTL_GPIO_DAT_GPIOA2 (1<<2) 
#define RTL_GPIO_DAT_GPIOA3 (1<<3) 	
#define RTL_GPIO_DAT_GPIOA4 (1<<4) 
#define RTL_GPIO_DAT_GPIOA5 (1<<5) 
#define RTL_GPIO_DAT_GPIOA6 (1<<6) 
#define RTL_GPIO_DAT_GPIOB3 (1<<11)
#define RTL_GPIO_DAT_GPIOB4 (1<<12)
#define RTL_GPIO_DAT_GPIOB6 (1<<14)
#define RTL_GPIO_DAT_GPIOB7 (1<<15) 	
#define RTL_GPIO_DAT_GPIOC0 (1<<16) 
#define RTL_GPIO_DIR_GPIOC1 (1<<17)
#define RTL_GPIO_DIR_GPIOC2 (1<<18)
#define RTL_GPIO_DIR_GPIOC3 (1<<19)
#define RTL_GPIO_DAT_GPIOD0 (1<<24) 	
#define RTL_GPIO_DAT_GPIOE2 (1<<2)
#define RTL_GPIO_DIR_GPIOG6 (1<<22)
#define RTL_GPIO_DIR_GPIOH0 (1<<24)
#define RTL_GPIO_DIR_GPIOH1 (1<<25)
#define RTL_GPIO_DIR_GPIOH2 (1<<26) 

#if defined(CONFIG_RTL_8196C)
	#define RTL_GPIO_MUX_DATA	0x00300000
	#define WPS_BTN_PIN		3	//GPIOA[3]
	#define WPS_LED_PIN		4	//GPIOA[4]
	#define RESET_LED_PIN		6	//GPIOA[6]
	#define RESET_BTN_PIN		5	//GPIOA[5]
#elif defined(CONFIG_RTL_8881A)

  #if defined(CONFIG_CUTE_MAHJONG) || defined(CONFIG_RTL_8881AM)
	
	#define RTL_GPIO_CNR_DATA	(RTL_GPIO_CNR_GPIOA3 | \
					 RTL_GPIO_CNR_GPIOA4 | \
					 RTL_GPIO_CNR_GPIOB4 | \
					 RTL_GPIO_CNR_GPIOB6 | \
					 RTL_GPIO_CNR_GPIOD0)
	#define RTL_GPIO_CNR_DATA_PORT4	(RTL_GPIO_CNR_GPIOA3 | \
					 RTL_GPIO_CNR_GPIOA4 | \
					 RTL_GPIO_CNR_GPIOB4 | \
					 RTL_GPIO_CNR_GPIOB3 | \
					 RTL_GPIO_CNR_GPIOD0)
					 
	#define RTL_GPIO_CNR_DATA2	(RTL_GPIO_CNR_GPIOE2)
	
	#define WPS_BTN_PIN         3	//GPIOA[3]
	#define WPS_LED_PIN         14	//GPIOB[6]
	#define RESET_LED_PIN       14	//GPIOB[6], system LED
	#define RESET_BTN_PIN       4	//GPIOA[4]
	#define SELECT2G5G_DIP_PIN  12	//GPIOB[4]
  #else
  	#define RTL_GPIO_CNR_DATA	(RTL_GPIO_CNR_GPIOA3 | \
					 RTL_GPIO_CNR_GPIOA4 | \
					 RTL_GPIO_CNR_GPIOA5 | \
					 RTL_GPIO_CNR_GPIOD0)
	#define RTL_GPIO_CNR_DATA2	(RTL_GPIO_CNR_GPIOE2)
	
	#define WPS_BTN_PIN		3	//GPIOA[3]
	#define WPS_LED_PIN		2	//GPIOE[2]
	#define RESET_LED_PIN		2	//GPIOE[2], system LED
	#define RESET_BTN_PIN		4	//GPIOA[4]
  #endif /* #if defined(CONFIG_CUTE_MAHJONG) */

#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	#define RTL_GPIO_MUX_GPIOA2_6 (6<<0)
	#define RTL_GPIO_MUX_GPIOA0_1 (3<<12)
	#define RTL_GPIO_MUX_2_GPIOB7 (4<<15)	
	#define RTL_GPIO_MUX_2_GPIOC0 (4<<18)
	
	/*Special Defination For 96ES_INIC*/
	#define RTL_96ES_WPS_LED_PIN         (4)
	#define RTL_96ES_RESET_PIN  	      (0)
	#define RTL_96ES_PUSH_BUTTON_PIN (7)
	
#if defined(CONFIG_RTL_8197D)
	#define RTL_GPIO_MUX_DATA	(RTL_GPIO_MUX_GPIOA0_1 | RTL_GPIO_MUX_GPIOA2_6)
	#define RTL_GPIO_MUX_2_DATA	(RTL_GPIO_MUX_2_GPIOB7 | RTL_GPIO_MUX_2_GPIOC0)
	#define RTL_GPIO_CNR_DATA	(RTL_GPIO_CNR_GPIOA1 | \
					 RTL_GPIO_CNR_GPIOA2 | \
					 RTL_GPIO_CNR_GPIOA3 | \
					 RTL_GPIO_CNR_GPIOA4 | \
					 RTL_GPIO_CNR_GPIOA5 | \
					 RTL_GPIO_CNR_GPIOA6 | \
					 RTL_GPIO_CNR_GPIOB7 | \
					 RTL_GPIO_CNR_GPIOC0)
	#define WPS_LED_PIN 		6
	#define WPS_BTN_PIN         	3
	#define RESET_LED_PIN          	6//reset led will be turn off by timer function
	#define RESET_BTN_PIN          	5
#elif defined(CONFIG_RTL_8197DL)
	#define RTL_GPIO_MUX_DATA	(RTL_GPIO_MUX_GPIOA2_6)
	#define RTL_GPIO_CNR_DATA	(RTL_GPIO_CNR_GPIOA2 | \
					 RTL_GPIO_CNR_GPIOA3 | \
					 RTL_GPIO_CNR_GPIOA4 | \
					 RTL_GPIO_CNR_GPIOA5 | \
					 RTL_GPIO_CNR_GPIOA6)
	#define WPS_LED_PIN 		6
	#define WPS_BTN_PIN         	3
	#define RESET_LED_PIN          	6//reset led will be turn off by timer function
	#define RESET_BTN_PIN          	5
#elif defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E)
	#define RTL_GPIO_MUX_DATA	(RTL_GPIO_MUX_GPIOA2_6)
	#define RTL_GPIO_CNR_DATA	(RTL_GPIO_CNR_GPIOA2 | \
					 RTL_GPIO_CNR_GPIOA4 | \
					 RTL_GPIO_CNR_GPIOA5 | \
					 RTL_GPIO_CNR_GPIOA6)
	#define WPS_BTN_PIN		2	//GPIOA[2]
	#define WPS_LED_PIN		6	//GPIOA[6]
	#define RESET_LED_PIN		6	//GPIOA[6], system LED
	#define RESET_BTN_PIN		5	//GPIOA[5]
#endif
#elif defined(CONFIG_RTL_8197F)
  #if defined(CONFIG_RTL_8367R_SUPPORT)
  #define RTL_GPIO_CNR				BSP_PEFGH_CNR
  #define RTL_GPIO_CNR_DATA			(RTL_GPIO_CNR_GPIOG6 | RTL_GPIO_CNR_GPIOH0 | RTL_GPIO_CNR_GPIOH1)
  #define RTL_GPIO_DIR				BSP_PEFGH_DIR
  #define RTL_GPIO_DIR_IN_DATA		(RTL_GPIO_DIR_GPIOG6 | RTL_GPIO_DIR_GPIOH0)
  #define RTL_GPIO_DIR_OUT_DATA		RTL_GPIO_DIR_GPIOH1
  #define WPS_LED_DAT				BSP_PEFGH_DAT
  #define WPS_LED_PIN				25				
  #define WPS_LED_MUX_SEL			BSP_PIN_MUX_SEL13
  #define WPS_LED_MUX				(0x2 << 16)
  #define WPS_LED_MASK				(0xf << 16)
  #define WPS_BTN_DAT				BSP_PEFGH_DAT
  #define WPS_BTN_PIN				24
  #define WPS_BTN_MUX_SEL			BSP_PIN_MUX_SEL13
  #define WPS_BTN_MUX				(0x3 << 20)
  #define WPS_BTN_MASK				(0xf << 20)
  #define RESET_LED_DAT 			BSP_PEFGH_DAT
  #define RESET_LED_PIN				25
  #define RESET_LED_MUX_SEL			BSP_PIN_MUX_SEL13
  #define RESET_LED_MUX				(0x2 << 16)
  #define RESET_LED_MASK			(0xf << 16)
  #define RESET_BTN_DAT				BSP_PEFGH_DAT
  #define RESET_BTN_PIN				22
  #define RESET_BTN_MUX_SEL			BSP_PIN_MUX_SEL13
  #define RESET_BTN_MUX				(0x3 << 28)
  #define RESET_BTN_MASK			(0xf << 28)
  #else /* CONFIG_RTL_8367R_SUPPORT */
  #define RTL_GPIO_CNR				BSP_PABCD_CNR
  #define RTL_GPIO_CNR_DATA			(RTL_GPIO_CNR_GPIOB7 | RTL_GPIO_CNR_GPIOC1 | RTL_GPIO_CNR_GPIOC3)
  #define RTL_GPIO_DIR				BSP_PABCD_DIR
  #define RTL_GPIO_DIR_IN_DATA		(RTL_GPIO_DIR_GPIOC1 | RTL_GPIO_DIR_GPIOC3)
  #define RTL_GPIO_DIR_OUT_DATA		RTL_GPIO_DIR_GPIOB7
  #define WPS_LED_DAT				BSP_PABCD_DAT
  #define WPS_LED_PIN				15
  #define WPS_LED_MUX_SEL			BSP_PIN_MUX_SEL2
  #define WPS_LED_MUX				(0x8 << 24)
  #define WPS_LED_MASK				(0xf << 24)
  #define WPS_BTN_DAT				BSP_PABCD_DAT
  #define WPS_BTN_PIN				17
  #define WPS_BTN_MUX_SEL			BSP_PIN_MUX_SEL2
  #define WPS_BTN_MUX				(0x7 << 12)
  #define WPS_BTN_MASK				(0xf << 12)
  #define RESET_LED_DAT				BSP_PABCD_DAT
  #define RESET_LED_PIN				15
  #define RESET_LED_MUX_SEL			BSP_PIN_MUX_SEL2
  #define RESET_LED_MUX				(0x8 << 24)
  #define RESET_LED_MASK			(0xf << 24)
  #define RESET_BTN_DAT				BSP_PABCD_DAT
  #define RESET_BTN_PIN				19
  #define RESET_BTN_MUX_SEL			BSP_PIN_MUX_SEL2
  #define RESET_BTN_MUX				(0x6 << 4)
  #define RESET_BTN_MASK			(0xf << 4)
  #endif /* CONFIG_RTL_8367R_SUPPORT */
#endif /* CONFIG_RTL_8197F */

#ifdef CONFIG_RTL_8367R_SUPPORT
#define BSP_GPIO_BASE	(0xB8003500)
#define BSP_PABCD_DIR	(0x008 + BSP_GPIO_BASE) /* Port ABCD direction */
#define BSP_PABCD_DAT	(0x00C + BSP_GPIO_BASE) /* Port ABCD data */
#endif

#define PROBE_TIME	5

#define PROBE_NULL		0
#define PROBE_ACTIVE		1
#define PROBE_RESET		2
#define PROBE_RELOAD		3
#define RTL_R32(addr)		(*(volatile unsigned long *)(addr))
#define RTL_W32(addr, l)	((*(volatile unsigned long*)(addr)) = (l))
#define RTL_R8(addr)		(*(volatile unsigned char*)(addr))
#define RTL_W8(addr, l)		((*(volatile unsigned char*)(addr)) = (l))

//#define  GPIO_DEBUG
#ifdef GPIO_DEBUG
#define DPRINTK(fmt, args...) diag_printf("%s: " fmt, __FUNCTION__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif

static unsigned int probe_counter;
static unsigned int probe_state;

#if defined(CONFIG_RTL_8881A) 
#if defined(CONFIG_RTL_8881AM) || defined(CONFIG_CUTE_MAHJONG)
static int cmj_board_use_port4 = 0;
#endif
#endif

#ifndef HAVE_NOETH
extern int bonding_type;
#define 	BOND_8196ES	(0xD)
#endif

#ifdef CONFIG_RTL_8196E
int RTLWIFINIC_GPIO_read(unsigned int gpio_num);
void RTLWIFINIC_GPIO_write(unsigned int gpio_num, unsigned int value);

int is_96ES_INIC(void)
{
	#ifndef HAVE_NOETH
	return (bonding_type == BOND_8196ES);
	#else
	return 0;
	#endif
}
#endif


#ifdef HAVE_WPS
static unsigned int wps_led_blink_flag = 0;
static unsigned int wps_led_toggle_flag = 0;

void wps_led_off(void)
{
#if defined(CONFIG_RTL_8881A) && defined(CONFIG_CUTE_MAHJONG)

	if (cmj_board_use_port4==1) 
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) | RTL_GPIO_DAT_GPIOB3));
	else
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) | (1 << WPS_LED_PIN)));
	
#elif defined(CONFIG_RTL_8881A)
	RTL_W32(BSP_PEFGH_DAT, (RTL_R32(BSP_PEFGH_DAT) | (1 << WPS_LED_PIN)));
#elif defined(CONFIG_RTL_8197F)
	RTL_W32(WPS_LED_DAT, (RTL_R32(WPS_LED_DAT) | (1 << WPS_LED_PIN)));
#else
#ifdef CONFIG_RTL_8196E
#ifndef HAVE_NOWIFI
	if(is_96ES_INIC())
		RTLWIFINIC_GPIO_write(RTL_96ES_WPS_LED_PIN,0);
	else
#endif
#endif		
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) | (1 << WPS_LED_PIN)));
#endif
	wps_led_blink_flag = 0;
}

void wps_led_on(void)
{
#if defined(CONFIG_RTL_8881A) && defined(CONFIG_CUTE_MAHJONG)

	if (cmj_board_use_port4==1) 
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~RTL_GPIO_DAT_GPIOB3)));
	else
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~(1 << WPS_LED_PIN))));
	
#elif defined(CONFIG_RTL_8881A)
	RTL_W32(BSP_PEFGH_DAT, (RTL_R32(BSP_PEFGH_DAT) & (~(1 << WPS_LED_PIN))));
#elif defined(CONFIG_RTL_8197F)
	RTL_W32(WPS_LED_DAT, (RTL_R32(WPS_LED_DAT) & (~(1 << WPS_LED_PIN))));
#else
#ifdef CONFIG_RTL_8196E
#ifndef HAVE_NOWIFI
	if(is_96ES_INIC())
		RTLWIFINIC_GPIO_write(RTL_96ES_WPS_LED_PIN,1);
	else
#endif
#endif		
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~(1 << WPS_LED_PIN))));
#endif
	wps_led_blink_flag = 0;
}

void wps_led_blink(void)
{
#if defined(CONFIG_RTL_8881A) && defined(CONFIG_CUTE_MAHJONG)

	if (cmj_board_use_port4==1) 
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~RTL_GPIO_DAT_GPIOB3)));
	else
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~(1 << WPS_LED_PIN))));

#elif defined(CONFIG_RTL_8881A)
	RTL_W32(BSP_PEFGH_DAT, (RTL_R32(BSP_PEFGH_DAT) & (~(1 << WPS_LED_PIN))));
#elif defined(CONFIG_RTL_8197F)
	RTL_W32(WPS_LED_DAT, (RTL_R32(WPS_LED_DAT) & (~(1 << WPS_LED_PIN))));
#else
#ifdef CONFIG_RTL_8196E
	if(is_96ES_INIC())
		RTLWIFINIC_GPIO_write(RTL_96ES_WPS_LED_PIN,1);
	else
#endif		
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~(1 << WPS_LED_PIN))));
#endif
	wps_led_blink_flag = 1;
	wps_led_toggle_flag = 1;
}

int wps_button_pressed(void)
{
#ifdef CONFIG_RTL_8196E
	if(is_96ES_INIC()) {
#ifndef HAVE_NOWIFI
		return RTLWIFINIC_GPIO_read(RTL_96ES_PUSH_BUTTON_PIN);
#endif
	}	
	else	
#endif		
	{
#ifdef CONFIG_RTL_8197F
		if (RTL_R32(WPS_BTN_DAT) & (1 << WPS_BTN_PIN))
#else
		if (RTL_R32(BSP_PABCD_DAT) & (1 << WPS_BTN_PIN))
#endif
			return 0;
		else
			return 1;
	}
}
#endif

void system_led_on(void)
{
#if defined(CONFIG_RTL_8881A) && (defined(CONFIG_RTL_8881AM) || defined(CONFIG_CUTE_MAHJONG))
	if (cmj_board_use_port4==1) 
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~RTL_GPIO_DAT_GPIOB3)));
	else
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~(1 << RESET_LED_PIN))));
#elif defined(CONFIG_RTL_8881A)
	RTL_W32(BSP_PEFGH_DAT, (RTL_R32(BSP_PEFGH_DAT) & (~(1 << RESET_LED_PIN))));
#elif defined(CONFIG_RTL_8197F)
	RTL_W32(RESET_LED_DAT, (RTL_R32(RESET_LED_DAT) & (~(1 << RESET_LED_PIN))));
#else
#if defined(CONFIG_RTL_ULINKER)
#ifndef HAVE_NOWIFI
		RTLWIFINIC_GPIO_write(4, 1);
#endif
#else
#ifdef CONFIG_RTL_8196E
#ifndef HAVE_NOWIFI
	if(is_96ES_INIC()) {
		RTLWIFINIC_GPIO_write(RTL_96ES_WPS_LED_PIN,1);
	} else
#endif
#endif	
	{
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~(1 << RESET_LED_PIN))));
	}	
#endif
#endif
}

void system_led_off(void)
{
#if defined(CONFIG_RTL_8881A) && (defined(CONFIG_RTL_8881AM) || defined(CONFIG_CUTE_MAHJONG))
	if (cmj_board_use_port4==1) 
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) | RTL_GPIO_DAT_GPIOB3));
	else
		RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) | (1 << RESET_LED_PIN)));
#elif defined(CONFIG_RTL_8881A)
	RTL_W32(BSP_PEFGH_DAT, (RTL_R32(BSP_PEFGH_DAT) | (1 << RESET_LED_PIN)));
#elif defined(CONFIG_RTL_8197F)
	RTL_W32(RESET_LED_DAT, (RTL_R32(RESET_LED_DAT) | (1 << RESET_LED_PIN)));
#else
#if defined(CONFIG_RTL_ULINKER)
#ifndef HAVE_NOWIFI
		RTLWIFINIC_GPIO_write(4, 0);
#endif
#else
#ifdef CONFIG_RTL_8196E
#ifndef HAVE_NOWIFI
	if(is_96ES_INIC()) {
		RTLWIFINIC_GPIO_write(RTL_96ES_WPS_LED_PIN,0);
	} else
#endif
#endif	
	{
        /* 板子加电完成自检后，系统灯需要点亮，这个点灯动作在交换芯片初始化完成，
         * 所以这里不关闭，added by zhuhuan on 2016.07.20 */
		// RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) | (1 << RESET_LED_PIN)));
	}
#endif
#endif
}

int reset_button_pressed(void)
{
#ifdef CONFIG_RTL_8196E
	if(is_96ES_INIC()) {
#ifndef HAVE_NOWIFI
		return RTLWIFINIC_GPIO_read(RTL_96ES_RESET_PIN);
#endif
	} else
#endif	
	{	
#ifdef CONFIG_RTL_8197F
		if (RTL_R32(RESET_BTN_DAT) & (1 << RESET_BTN_PIN))
#else
		if (RTL_R32(BSP_PABCD_DAT) & (1 << RESET_BTN_PIN))
#endif
			return 0;
		else
			return 1;
	}
}

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
void band_selectable_dip_switch(void)
{
	static int dip_priv = -1, dip = -1;
	dip = RTL_R32(BSP_PABCD_DAT) & (1 << SELECT2G5G_DIP_PIN);

	if (dip_priv == -1)
		dip_priv = dip;

	if (dip_priv != -1 && dip_priv !=dip) {
		dip_priv = dip;
		kick_event(RESET_EVENT);
	}
}
#endif

void rtl_gpio_timer(void)
{
	unsigned int pressed=1;

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
	band_selectable_dip_switch();
#endif

	if (reset_button_pressed()) {
		DPRINTK("Key pressed %d!\n", probe_counter+1);
	}
	else {
		pressed = 0;
#ifdef CONFIG_RTL_8196E		
		if(is_96ES_INIC())
		{
			/*WPS SYS LED share*/
#ifdef HAVE_WPS
			if(wps_led_blink_flag==0)
#endif
				system_led_off();			
		}
		else
#endif			
		{
		#if defined(CONFIG_CMJ_INDICATE_WISP_STATUS)
		#ifdef HAVE_WPS
			if (wps_led_blink_flag == 0)
		#endif
             	kick_event(INDICATE_WISP_STATUS_EVENT); //INDICATE_WISP_STATUS_EVENT
		#else
			system_led_off();
		#endif
		}
	}

	if (probe_state == PROBE_NULL) {
		if (pressed) {
		#if defined(CONFIG_CMJ_INDICATE_WISP_STATUS)
			system_led_off();
		#endif
			probe_state = PROBE_ACTIVE;
			probe_counter++;
		}
		else {
			probe_counter = 0;
		}
	}
	else if (probe_state == PROBE_ACTIVE) {
		if (pressed) {
			probe_counter++;
			if ((probe_counter >=2 ) && (probe_counter<=PROBE_TIME)) {
				DPRINTK("2-5 turn on led\n");
				system_led_on();
			}
			else if (probe_counter >= PROBE_TIME) {
				DPRINTK(">5 \n");
			#if (defined(CONFIG_RTL_8881A) && defined(CONFIG_CUTE_MAHJONG)) ||defined(CONFIG_RTL_ULINKER) || defined(CONFIG_RTL_8197F)
				if (probe_counter%2==0)
					system_led_on();
				else
					system_led_off();
			#endif
			}
		}
		else {
			if (probe_counter < 2) {
				probe_state = PROBE_NULL;
				probe_counter = 0;
				DPRINTK("<2 \n");
			}
			else if (probe_counter >= PROBE_TIME) {
				//reload default
				probe_counter = 0;
				kick_event(RELOAD_DEFAULT_EVENT);
				return;
			}
			else {
				DPRINTK("2-5 reset\n");
				kick_event(RESET_EVENT);
				return;
			}
		}
	}

#ifdef HAVE_WPS
	if (wps_led_blink_flag==1)
	{
		if (wps_led_toggle_flag) {
#if defined(CONFIG_RTL_8881A) && defined(CONFIG_CUTE_MAHJONG)

			if (cmj_board_use_port4==1) 
				RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) | RTL_GPIO_DAT_GPIOB3));
			else
				RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) | (1 << WPS_LED_PIN)));

#elif defined(CONFIG_RTL_8881A)
			RTL_W32(BSP_PEFGH_DAT, (RTL_R32(BSP_PEFGH_DAT) | (1 << WPS_LED_PIN)));
#elif defined(CONFIG_RTL_8197F)
			RTL_W32(WPS_LED_DAT, (RTL_R32(WPS_LED_DAT) | (1 << WPS_LED_PIN)));
#else
#ifdef CONFIG_RTL_8196E			
#ifndef HAVE_NOWIFI
			if(is_96ES_INIC())
				RTLWIFINIC_GPIO_write(RTL_96ES_WPS_LED_PIN,0);
			else
#endif
#endif				
				RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) | (1 << WPS_LED_PIN)));
#endif
		}
		else {
#if defined(CONFIG_RTL_8881A) && defined(CONFIG_CUTE_MAHJONG)

			if (cmj_board_use_port4==1) 
				RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~RTL_GPIO_DAT_GPIOB3)));
			else
				RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~(1 << WPS_LED_PIN))));

#elif defined(CONFIG_RTL_8881A)
			RTL_W32(BSP_PEFGH_DAT, (RTL_R32(BSP_PEFGH_DAT) & (~(1 << WPS_LED_PIN))));
#elif defined(CONFIG_RTL_8197F)
			RTL_W32(WPS_LED_DAT, (RTL_R32(WPS_LED_DAT) & (~(1 << WPS_LED_PIN))));
#else
#ifdef CONFIG_RTL_8196E			
#ifndef HAVE_NOWIFI
			if(is_96ES_INIC())
				RTLWIFINIC_GPIO_write(RTL_96ES_WPS_LED_PIN,1);
			else
#endif
#endif				
				RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~(1 << WPS_LED_PIN))));
#endif
		}
		wps_led_toggle_flag = wps_led_toggle_flag ? 0 : 1;
	}
#endif
}

#ifdef HAVE_WPS
void wps_led_control(int value)
{
	if (value == 0)
	{
		wps_led_off();
	}	
	else if (value == 1)
	{		
		wps_led_on();
	}	
	else if (value == 2)
	{		
		wps_led_blink();
	}	
	/*
	else if (value == 4){
		start_count_time= 1;
		sscanf(buffer, "%s %s", start_count, wait);
		Reboot_Wait = (simple_strtol(wait,NULL,0))*100;
	}
	*/
}
#endif

#if defined(CONFIG_RTL_ULINKER)
#include <sys/param.h>

#ifndef BIT
#define BIT(x)	(1 << (x))
#endif

//extern int PCIE_reset_procedure(int portnum, int Use_External_PCIE_CLK, int mdio_reset);
extern int PCIE_reset_procedure(int PCIE_Port0and1_8196B_208pin, int Use_External_PCIE_CLK, int mdio_reset,unsigned long conf_addr);

unsigned long rtl_simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;

	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if ((*cp == 'x') && isxdigit(cp[1])) {
				cp++;
				base = 16;
			}
		}
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}


int PCIE_Host_Init(int argc, char* argv[])
{
	int portnum=0;
	if(argc >= 1) 
	{	portnum= rtl_simple_strtoul((const char*)(argv[0]), (char **)NULL, 16);	
	}

#define PCIE0_RC_CFG_BASE (0xb8b00000)
#define PCIE0_RC_EXT_BASE (PCIE0_RC_CFG_BASE + 0x1000)
#define PCIE0_EP_CFG_BASE (0xb8b10000)

#define PCIE1_RC_CFG_BASE (0xb8b20000)
#define PCIE1_RC_EXT_BASE (PCIE1_RC_CFG_BASE + 0x1000)
#define PCIE1_EP_CFG_BASE (0xb8b30000)

#define PCIE0_MAP_IO_BASE  (0xb8c00000)
#define PCIE0_MAP_MEM_BASE (0xb9000000)

#define PCIE1_MAP_IO_BASE  (0xb8e00000)
#define PCIE1_MAP_MEM_BASE (0xba000000)

#define MAX_READ_REQSIZE_128B    0x00
#define MAX_READ_REQSIZE_256B    0x10
#define MAX_READ_REQSIZE_512B    0x20
#define MAX_READ_REQSIZE_1KB     0x30
#define MAX_READ_REQSIZE_2KB     0x40
#define MAX_READ_REQSIZE_4KB     0x50

#define MAX_PAYLOAD_SIZE_128B    0x00
#define MAX_PAYLOAD_SIZE_256B    0x20
#define MAX_PAYLOAD_SIZE_512B    0x40
#define MAX_PAYLOAD_SIZE_1KB     0x60
#define MAX_PAYLOAD_SIZE_2KB     0x80
#define MAX_PAYLOAD_SIZE_4KB     0xA0

	int rc_cfg, cfgaddr;
	int iomapaddr;
	int memmapaddr;

	if(portnum==0)
	{	rc_cfg=PCIE0_RC_CFG_BASE;
		cfgaddr=PCIE0_EP_CFG_BASE;
		iomapaddr=PCIE0_MAP_IO_BASE;
		memmapaddr=PCIE0_MAP_MEM_BASE;
	}
	else if(portnum==1)
	{	rc_cfg=PCIE1_RC_CFG_BASE;
		cfgaddr=PCIE1_EP_CFG_BASE;
		iomapaddr=PCIE1_MAP_IO_BASE;
		memmapaddr=PCIE1_MAP_MEM_BASE;	
	}
	else 
	{	return 0;
	}
	
	int t=REG32(rc_cfg);
	unsigned int vid=t&0x0000ffff;   //0x819810EC
	unsigned int pid=(t&0xffff0000) >>16;
	
	if( (vid!= 0x10ec) || (pid!=0x8196))
	{	//DBG_PRINT("VID=%x, PID=%x \n", vid, pid );
		//DBG_PRINT(" !!!  Read VID PID fail !!! \n");
		//at_errcnt++;
		return;
	}

	//STATUS
	//bit 4: capabilties List

	//CMD
	//bit 2: Enable Bys master, 
	//bit 1: enable memmap, 
	//bit 0: enable iomap

	REG32(rc_cfg + 0x04)= 0x00100007;   

	//Device Control Register 
	//bit [7-5]  payload size
	REG32(rc_cfg + 0x78)= (REG32(rc_cfg + 0x78 ) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B;  // Set MAX_PAYLOAD_SIZE to 128B,default
	  
      REG32(cfgaddr + 0x04)= 0x00100007;    //0x00180007

	//bit 0: 0:memory, 1 io indicate
      REG32(cfgaddr + 0x10)= (iomapaddr | 0x00000001) & 0x1FFFFFFF;  // Set BAR0

	//bit 3: prefetch
	//bit [2:1] 00:32bit, 01:reserved, 10:64bit 11:reserved
      REG32(cfgaddr + 0x18)= (memmapaddr | 0x00000004) & 0x1FFFFFFF;  // Set BAR1  


	//offset 0x78 [7:5]
      REG32(cfgaddr + 0x78) = (REG32(cfgaddr + 0x78) & (~0xE0)) | (MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B

	//offset 0x79: [6:4] 
      REG32(cfgaddr + 0x78) = (REG32(cfgaddr + 0x78) & (~0x7000)) | (MAX_READ_REQSIZE_256B<<8);  // Set MAX_REQ_SIZE to 256B,default

	  
	//check
//      if(REG32(cfgaddr + 0x10) != ((iomapaddr | 0x00000001) & 0x1FFFFFFF))
      {	//at_errcnt++;
      		//DBG_PRINT("Read Bar0=%x \n", REG32(cfgaddr + 0x10)); //for test
      	}
	  

//	if(REG32(cfgaddr + 0x18)!=((memmapaddr| 0x00000004) & 0x1FFFFFFF))
	{	//at_errcnt++;
      		//DBG_PRINT("Read Bar1=%x \n", REG32(cfgaddr + 0x18));      //for test
	}
	//DBG_PRINT("Set BAR finish \n");


	//io and mem limit, setting to no litmit
	REG32(rc_cfg+ 0x1c) = (2<<4) | (0<<12);   //  [7:4]=base  [15:12]=limit
	REG32(rc_cfg+ 0x20) = (2<<4) | (0<<20);   //  [15:4]=base  [31:20]=limit	
	REG32(rc_cfg+ 0x24) = (2<<4) | (0<<20);   //  [15:4]=base  [31:20]=limit		


#undef PCIE0_RC_EXT_BASE
#undef PCIE0_EP_CFG_BASE

#undef PCIE1_RC_CFG_BASE
#undef PCIE1_RC_EXT_BASE
#undef PCIE1_EP_CFG_BASE

#undef PCIE0_MAP_IO_BASE
#undef PCIE0_MAP_MEM_BASE

#undef PCIE1_MAP_IO_BASE
#undef PCIE1_MAP_MEM_BASE

#undef MAX_READ_REQSIZE_128B
#undef MAX_READ_REQSIZE_256B
#undef MAX_READ_REQSIZE_512B
#undef MAX_READ_REQSIZE_1KB
#undef MAX_READ_REQSIZE_2KB
#undef MAX_READ_REQSIZE_4KB

#undef MAX_PAYLOAD_SIZE_128B
#undef MAX_PAYLOAD_SIZE_256B
#undef MAX_PAYLOAD_SIZE_512B
#undef MAX_PAYLOAD_SIZE_1KB
#undef MAX_PAYLOAD_SIZE_2KB
#undef MAX_PAYLOAD_SIZE_4KB
}

//spinlock_t sysled_lock = SPIN_LOCK_UNLOCKED;
static int sysled_toggle = 1;
static struct callout sysled_timer;
externC int cyg_hz;
#define SYSLED_TIMER_CHECK	cyg_hz/2
extern void renable_sw_LED(void);

void system_led_control(unsigned long data)
{
#define PCIE0_EP_CFG_BASE (0xb8b10000)
#define GPIO_PIN_CTRL     0x044

	static int on = 0;
	unsigned int reg;

	if (sysled_toggle == 1) {
		reg = ((REG32(PCIE0_EP_CFG_BASE + 0x18) & 0xffff0000) | 0xb0000000) + GPIO_PIN_CTRL;
		if (on == 1) {
			RTL_W32(reg, RTL_R32(reg) & ~BIT(12));
			on = 0;
		}
		else {
			RTL_W32(reg, RTL_R32(reg) | BIT(12));
			on = 1;
		}

		cyg_callout_reset(&sysled_timer, SYSLED_TIMER_CHECK , system_led_control, 0);
	}

#undef PCIE0_EP_CFG_BASE
#undef GPIO_PIN_CTRL
}

void system_led_init()
{
#define PCIE0_EP_CFG_BASE (0xb8b10000)

	char *arg_v[] = {"hinit", "0"};
	unsigned int reg;
	static int init = 0;

	if (init == 0) {
		//spin_lock_init(&sysled_lock);

		PCIE_reset_procedure(0, 0, 1,PCIE0_EP_CFG_BASE);
		PCIE_Host_Init(2, arg_v);
		reg = (REG32(PCIE0_EP_CFG_BASE + 0x18) & 0xffff0000) | 0xb0000000;	
		(*(volatile unsigned int *)(reg + 0x44)) = 0x30300000;

		cyg_callout_init(&sysled_timer);
		cyg_callout_reset(&sysled_timer, SYSLED_TIMER_CHECK , system_led_control, 0);
	}
#undef PCIE0_EP_CFG_BASE
}

int set_ulinker_led(int action)
{

	if (action==0) {
		sysled_toggle = 0;
#ifndef HAVE_NOWIFI
		RTLWIFINIC_GPIO_write(4, 0);
#endif
		renable_sw_LED();
	}
	else if (action==1) {
		sysled_toggle = 1;
		cyg_callout_reset(&sysled_timer, SYSLED_TIMER_CHECK , system_led_control, 0);
	}

	return 0;
}

#endif

void rtl_gpio_init(void)
{
    unsigned int reg_data;

	diag_printf("Realtek GPIO init\n");

#if defined(CONFIG_RTL_ULINKER)
	system_led_init();
#endif

#if defined(CONFIG_RTL_8881A) && (defined(CONFIG_RTL_8881AM) || defined(CONFIG_CUTE_MAHJONG))
	if ((REG32(0xB800000C) & 0xF)==0xA) { 		// Check is RTL8881AM 
		REG32(0xB8000040) = (REG32(0xB8000040)&~(0x7<<7))|(0x3<<7); //Set MUX to GPIO
		REG32(BSP_PEFGH_CNR) = REG32(BSP_PEFGH_CNR) & ~(0x80); //Set E7 for  Gpio
		REG32(BSP_PEFGH_DIR) = REG32(BSP_PEFGH_DIR) & ~(0x80); //Set E7 for Input Mode
		if((REG32(BSP_PEFGH_DAT)&0x80) == 0x80) { //Pull high
			cmj_board_use_port4 = 1;
		}
		else {
			cmj_board_use_port4 = 0;
		}		
    }		
#endif

#if defined(CONFIG_RTL_8881A) && (defined(CONFIG_RTL_8881AM) || defined(CONFIG_CUTE_MAHJONG))
	//reg_iocfg_led_s2, reg_iocfg_led_p0, reg_iocfg_wbb_0
	//config as gpio mode, gpioB[3,4]/gpioD[0]
	REG32(BSP_PIN_MUX_SEL_2) = (REG32(BSP_PIN_MUX_SEL_2) & ~(7<<6))  | ((3<<6));
#ifndef CONFIG_USING_JTAG
	//reg_iocfg_jtag_trstn, reg_iocfg_jtag_tms, reg_iocfg_jtag_tdi
	//config as gpio mode,gpioA[3,4]
	REG32(BSP_PIN_MUX_SEL_3) |= ((6<<4)|(6<<8));
#endif

	if (cmj_board_use_port4==1) {
		// BSP_PIN_MUX_SEL_2: reg_iocfg_led_p0 (n13:12) = LED-SW (2'b00)
		// BSP_PIN_MUX_SEL_2: reg_iocfg_led_s1 (n5:3) = GPIO (3'b011)
		REG32(BSP_PIN_MUX_SEL_2) = (REG32(BSP_PIN_MUX_SEL_2) & ~((3<<12)|(7<<3)))  | ((3<<3));
	  	//set gpioA[3,4]/gpioB[4,6]/gpioD[0]as GPIO PIN
		REG32(BSP_PABCD_CNR) &= ~(RTL_GPIO_CNR_DATA_PORT4);
		REG32(BSP_PABCD_DIR) |= RTL_GPIO_DIR_GPIOB3;
	}
	else { // cmj use port 1
		// BSP_PIN_MUX_SEL_2: reg_iocfg_led_p0 (n13:12) = GPIO (2'b11)
		// BSP_PIN_MUX_SEL_2: reg_iocfg_led_s1 (n5:3) = LED-SW (3'b000)
		REG32(BSP_PIN_MUX_SEL_2) = (REG32(BSP_PIN_MUX_SEL_2) & ~(7<<3))  | ((3<<12));
	  	//set gpioA[3,4]/gpioB[4,6]/gpioD[0]as GPIO PIN
		REG32(BSP_PABCD_CNR) &= ~(RTL_GPIO_CNR_DATA);
		REG32(BSP_PABCD_DIR) |= RTL_GPIO_DIR_GPIOB6;
	}
	//set direction, gpioA[3,4]/gpioB[4] INPUT, gpioD[0]/gpioE[2] OUTPUT
	REG32(BSP_PABCD_DIR) &= ~(RTL_GPIO_DIR_GPIOA3);
	REG32(BSP_PABCD_DIR) &= ~(RTL_GPIO_DIR_GPIOA4);
	REG32(BSP_PABCD_DIR) &= ~(RTL_GPIO_DIR_GPIOB4);
	REG32(BSP_PABCD_DIR) |= RTL_GPIO_DIR_GPIOD0;
#elif defined(CONFIG_RTL_8881A)
	//reg_iocfg_p0mii config as gpio mode, gpioE[2]
	REG32(BSP_PIN_MUX_SEL) |= (3<<7);
	//reg_iocfg_wbb_0 config as gpio mode, gpioD[0]
	REG32(BSP_PIN_MUX_SEL_2) |= (3<<15);
#ifndef CONFIG_USING_JTAG
	//reg_iocfg_jtag_trstn, reg_iocfg_jtag_tms, reg_iocfg_jtag_tdi
	//config as gpio mode,gpioA[3,4,5]
	REG32(BSP_PIN_MUX_SEL_3) |= ((6<<4)|(6<<8)|(6<<12));
#endif
	//set gpioA[3,4,5]/gpioD[0]/gpioE[2]as GPIO PIN
	REG32(BSP_PABCD_CNR) &= ~(RTL_GPIO_CNR_DATA);
	REG32(BSP_PEFGH_CNR) &= ~(RTL_GPIO_CNR_DATA2);
	//set direction, gpioA[3,4,5] INPUT, gpioD[0]/gpioE[2] OUTPUT
	REG32(BSP_PABCD_DIR) &= ~(RTL_GPIO_DIR_GPIOA3);
	REG32(BSP_PABCD_DIR) &= ~(RTL_GPIO_DIR_GPIOA4);
	REG32(BSP_PABCD_DIR) &= ~(RTL_GPIO_DIR_GPIOA5);
	REG32(BSP_PABCD_DIR) |= RTL_GPIO_DIR_GPIOD0;
	REG32(BSP_PEFGH_DIR) |= RTL_GPIO_DIR_GPIOE2;
#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#if defined(CONFIG_RTL_8197D)
	//reg_iocfg_jtag config as gpio mode,gpioA[0~1], gpioA[2~6]
	#ifndef CONFIG_USING_JTAG
	RTL_W32(BSP_PIN_MUX_SEL, (RTL_R32(BSP_PIN_MUX_SEL) | RTL_GPIO_MUX_DATA));
	#endif
	//reg_iocfg_led_p1, reg_iocfg_led_p2, config as gpio mode,GPIOB[7], GPIOC[0]
	RTL_W32(BSP_PIN_MUX_SEL_2, (RTL_R32(BSP_PIN_MUX_SEL_2) | RTL_GPIO_MUX_2_DATA));  	
	//set gpioA[1~6],gpioB[7],gpioC[0] as GPIO PIN
	RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~(RTL_GPIO_CNR_DATA)));
	//set direction, GPIOA[1,3,4,5] INPUT, GPIOA[2,6] OUTPUT
	#ifdef CONFIG_RTL_8367R_SUPPORT
	// for 8367r h/w reset pin
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | ((RTL_GPIO_DIR_GPIOA1))));
	RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) & (~(RTL_GPIO_DAT_GPIOA1))));
	#else
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA1))));
	#endif
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA3))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA4))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA5))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | ((RTL_GPIO_DIR_GPIOA2))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | ((RTL_GPIO_DIR_GPIOA6))));
	//set direction, GPIOB[7] INPUT, GPIOC[0] OUTPUT
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOB7))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | ((RTL_GPIO_DIR_GPIOC0))));
#elif defined(CONFIG_RTL_8197DL)
	//reg_iocfg_jtag config as gpio mode,gpioA[0~1], gpioA[2~6]
	#ifndef CONFIG_USING_JTAG
	RTL_W32(BSP_PIN_MUX_SEL, (RTL_R32(BSP_PIN_MUX_SEL) | RTL_GPIO_MUX_DATA));
	#endif
	//set gpioA[2~6] as GPIO PIN
	RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~(RTL_GPIO_CNR_DATA)));
	//set direction, GPIOA[1,3,4,5] INPUT, GPIOA[2,6] OUTPUT
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA3))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA4))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA5))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | ((RTL_GPIO_DIR_GPIOA2))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | ((RTL_GPIO_DIR_GPIOA6))));
#elif defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E)
#if defined(CONFIG_RTL_8196E)
	//reg_iocfg_jtag config as gpio mode,gpioA[2~6]
	if(is_96ES_INIC()) {
		/*GPIO 0(I) 4(O) 5(O) 7(I) on 88e. Init in rtl_8196es_gpio_init*/
	}
	else 
#endif
	{
#ifndef CONFIG_USING_JTAG
	RTL_W32(BSP_PIN_MUX_SEL, (RTL_R32(BSP_PIN_MUX_SEL) | RTL_GPIO_MUX_DATA));
#endif	
	//set gpioA[2,4,5,6]as GPIO PIN
	RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~(RTL_GPIO_CNR_DATA)));
	//set direction, GPIOA[2,4,5] INPUT, GPIOA[6] OUTPUT
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA2))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA4))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA5))));
#if defined(CONFIG_RTL_ULINKER)
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & (~(RTL_GPIO_DIR_GPIOA6))));
#else
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | ((RTL_GPIO_DIR_GPIOA6))));
#endif
	}

#endif
#elif defined(CONFIG_RTL_8197F)
   /*****配置B4,B7,C1,C3,为GPIO模式/wps_led、sys_led,reset/wps_btn,wifi_btn****/
	RTL_W32(BSP_PIN_MUX_SEL1, ((RTL_R32(BSP_PIN_MUX_SEL1) & ~((0xf << 24))) | (0x8 << 24)));    
    RTL_W32(BSP_PIN_MUX_SEL2, ((RTL_R32(BSP_PIN_MUX_SEL2) & ~((0xf << 4))) | (0x6 << 4)));  
    RTL_W32(BSP_PIN_MUX_SEL2, ((RTL_R32(BSP_PIN_MUX_SEL2) & ~((0xf << 24))) | (0x8 << 24)));
    RTL_W32(BSP_PIN_MUX_SEL2, ((RTL_R32(BSP_PIN_MUX_SEL2) & ~((0xf << 4))) | (0x6 << 4)));
    RTL_W32(BSP_PIN_MUX_SEL2, ((RTL_R32(BSP_PIN_MUX_SEL2) & ~((0xf << 12))) | (0x7 << 12)));
    RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~((1<<15) | (1<<12) |(1<<19) |(1<<17))));  
    RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & ~((1<<17) | (1<<19))));    
    RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | ((1<< 15)|(1<<12))));
    RTL_W32(BSP_PABCD_DAT, (RTL_R32(BSP_PABCD_DAT) &  (~(1<< 15))|(~(1<<12))));

    /******初始化时网口灯配置为复用led/phy模式******/
    reg_data = RTL_R32(BSP_PIN_MUX_SEL13); //G6-H1
    reg_data = (reg_data & ~0xffff0000);
    RTL_W32(BSP_PIN_MUX_SEL13,reg_data);
    reg_data = RTL_R32(BSP_PIN_MUX_SEL14);
    reg_data = (reg_data & ~(0xf << 28)); //H2
    RTL_W32(BSP_PIN_MUX_SEL14,reg_data);
 
#else //CONFIG_RTL_819XD || CONFIG_RTL_8196E
	//switch shared pins to GPIOA[6:2]
#ifndef CONFIG_USING_JTAG
    	RTL_W32(BSP_PIN_MUX_SEL, (RTL_R32(BSP_PIN_MUX_SEL) | (RTL_GPIO_MUX_DATA)));
#endif
    	
	//reset button, input pin
	RTL_W32(BSP_PABCD_CNR,(RTL_R32(BSP_PABCD_CNR)&(~(1 << RESET_BTN_PIN))));
	RTL_W32(BSP_PABCD_DIR,(RTL_R32(BSP_PABCD_DIR)&(~(1 << RESET_BTN_PIN))));

	//reset/system led, output pin
	RTL_W32(BSP_PABCD_CNR,(RTL_R32(BSP_PABCD_CNR)&(~(1 << RESET_LED_PIN))));
	RTL_W32(BSP_PABCD_DIR,(RTL_R32(BSP_PABCD_DIR)|((1 << RESET_LED_PIN))));

#ifdef HAVE_WPS
	//WPS button, input pin
	RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR)&(~(1 << WPS_BTN_PIN))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR)&(~(1 << WPS_BTN_PIN))));

	//WPS led, output pin
	RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR)&(~(1 << WPS_LED_PIN))));
	RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | (1 << WPS_LED_PIN)));
#endif
#endif

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
	set_gpio_2g5g();
#endif

	// turn off system/WPS led in the beginning
	system_led_off();
#ifdef HAVE_WPS
	wps_led_off();
#endif

	probe_counter = 0;
	probe_state = PROBE_NULL;
}
