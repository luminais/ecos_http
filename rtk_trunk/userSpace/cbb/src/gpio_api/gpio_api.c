#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/hal/bspchip.h>
#include <cyg/infra/diag.h>
#include <cyg/kernel/kapi.h>
#include "gpio_api.h"

#define RTL_R32(addr)       (*(volatile unsigned long *)(addr))
#define RTL_W32(addr, l)    ((*(volatile unsigned long*)(addr)) = (l))

#define RTL_GPIO_CNR_GPIOG6 (1 << 22)
#define RTL_GPIO_CNR_GPIOG7 (1 << 23)
#define RTL_GPIO_CNR_GPIOH0 (1 << 24)
#define RTL_GPIO_CNR_GPIOH1 (1 << 25)
#define RTL_GPIO_CNR_GPIOH2 (1 << 26)
#define RTL_GPIO_CNR_GPIOH3 (1 << 27)
#define RTL_GPIO_CNR_GPIOA1 (1<<1)
#define RTL_GPIO_CNR_GPIOA2 (1<<2)
#define RTL_GPIO_CNR_GPIOA3 (1<<3)
#define RTL_GPIO_CNR_GPIOA4 (1<<4)
#define RTL_GPIO_CNR_GPIOA5 (1<<5)
#define RTL_GPIO_CNR_GPIOA6 (1<<6)
#define RTL_GPIO_CNR_GPIOB2 (1<<10)
#define RTL_GPIO_CNR_GPIOB3 (1<<11)
#define RTL_GPIO_CNR_GPIOB4 (1<<12)
#define RTL_GPIO_CNR_GPIOB5 (1<<13)
#define RTL_GPIO_CNR_GPIOB6 (1<<14)
#define RTL_GPIO_CNR_GPIOB7 (1<<15) 
#define RTL_GPIO_CNR_GPIOC0 (1<<16)
#define RTL_GPIO_CNR_GPIOD0 (1<<24)
#define RTL_GPIO_CNR_GPIOE2 (1<<2)
#define RTL_GPIO_CNR_GPIOB2_6 (RTL_GPIO_CNR_GPIOB2|RTL_GPIO_CNR_GPIOB3| \
                                RTL_GPIO_CNR_GPIOB4 |   \
                                RTL_GPIO_CNR_GPIOB5|RTL_GPIO_CNR_GPIOB6)

#define RTL_GPIO_LAN_LED_PIN (RTL_GPIO_CNR_GPIOG6 | RTL_GPIO_CNR_GPIOG7 | RTL_GPIO_CNR_GPIOH0 |RTL_GPIO_CNR_GPIOH1 | RTL_GPIO_CNR_GPIOH2)


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
#define RTL_GPIO_DIR_GPIOD0 (1<<24)
#define RTL_GPIO_DIR_GPIOE2 (1<<2)

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
#define RTL_GPIO_DAT_GPIOD0 (1<<24)     
#define RTL_GPIO_DAT_GPIOE2 (1<<2) 

#define RTL_GPIO_MUX_2_GPIOB6 (3<<12)   //led_port4
#define RTL_GPIO_MUX_2_GPIOB5 (3<<9)    //led_port3
#define RTL_GPIO_MUX_2_GPIOB4 (3<<6)    //led_port2
#define RTL_GPIO_MUX_2_GPIOB3 (3<<3)    //led_port1
#define RTL_GPIO_MUX_2_GPIOB2 (3<<0)    //led_port0
#define RTL_GPIO_MUX_2_GPIOB2_6    (RTL_GPIO_MUX_2_GPIOB2|RTL_GPIO_MUX_2_GPIOB3| \
                                        RTL_GPIO_MUX_2_GPIOB4|RTL_GPIO_MUX_2_GPIOB5| \
                                        RTL_GPIO_MUX_2_GPIOB6)
#if defined(__CONFIG_LAN1_PORT__) && defined(__CONFIG_LAN2_PORT__) && defined(__CONFIG_LAN3_PORT__)
#define RTL_GPIO_MUX_2_GPIOB_LAN    ((3 << (__CONFIG_LAN1_PORT__ * 3))| \
                                        (3 << (__CONFIG_LAN2_PORT__ * 3))|(3 << (__CONFIG_LAN3_PORT__ * 3)))
 #else
 #define RTL_GPIO_MUX_2_GPIOB_LAN    (RTL_GPIO_MUX_2_GPIOB3| \
                                        RTL_GPIO_MUX_2_GPIOB4|RTL_GPIO_MUX_2_GPIOB5| \
                                        RTL_GPIO_MUX_2_GPIOB6)
 #endif



#define TENDA_GPIO_CRN_DATA (1<<WPS_BTN_PIN)    \
                                |(1<<RESET_BTN_PIN) \
                                |(1<SYSTEM_LED_PIN)

static unsigned int rtl819x_gpio_mux(unsigned int pin, 
    unsigned int *value, unsigned int *address )
{
	unsigned int mask = 0;

	switch(pin) {
	case BSP_GPIO_PIN_A0:
		mask = 0xf<<28;
		*value = 0x8<<28;
		*address = BSP_PIN_MUX_SEL7;
		break;
	case BSP_GPIO_PIN_A1:
		mask = 0xf<<28;
		*value = 0x8<<28;
		*address = BSP_PIN_MUX_SEL6;
		break;
	case BSP_GPIO_PIN_A2:
		mask = 0xf<<24;
		*value = 0xb<<24;
		*address = BSP_PIN_MUX_SEL6;
		break;
	case BSP_GPIO_PIN_A3:
		mask = 0xf<<16;
		*value = 0x8<<16;
		*address = BSP_PIN_MUX_SEL7;
		break;
	case BSP_GPIO_PIN_A4:
		mask = 0xf<<24;
		*value = 0x7<<24;
		*address = BSP_PIN_MUX_SEL7;
		break;
	case BSP_GPIO_PIN_A5:
		mask = 0xf<<20;
		*value = 0x6<<20;
		*address = BSP_PIN_MUX_SEL7;
		break;
	case BSP_GPIO_PIN_A6:
		mask = 0xf<<16;
		*value = 0x5<<16;
		*address = BSP_PIN_MUX_SEL0;
		break;
	case BSP_GPIO_PIN_A7:
		mask = 0xf<<20;
		*value = 0x6<<20;
		*address = BSP_PIN_MUX_SEL0;
		break;

	case BSP_GPIO_PIN_B0:
		mask = 0xf<<24;
		*value = 0x8<<24;
		*address = BSP_PIN_MUX_SEL0;
		break;
	case BSP_GPIO_PIN_B1:
		mask = 0xf<<16;
		*value = 0xa<<16;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_B2:
		mask = 0xf<<16;
		*value = 0x8<<16;
		*address = BSP_PIN_MUX_SEL1;
		break;
	case BSP_GPIO_PIN_B3:
		mask = 0xf<<20;
		*value = 0x8<<20;
		*address = BSP_PIN_MUX_SEL1;
		break;
	case BSP_GPIO_PIN_B4:
		mask = 0xf<<24;
		*value = 0x8<<24;
		*address = BSP_PIN_MUX_SEL1;
		break;
	case BSP_GPIO_PIN_B5:
		mask = 0xf<<28;
		*value = 0x7<<28;
		*address = BSP_PIN_MUX_SEL1;
		break;
	case BSP_GPIO_PIN_B6:
		mask = 0xf<<28;
		*value = 0x8<<28;
		*address = BSP_PIN_MUX_SEL0;
		break;
	case BSP_GPIO_PIN_B7:
		mask = 0xf<<24;
		*value = 0x8<<24;
		*address = BSP_PIN_MUX_SEL2;
		break;

	case BSP_GPIO_PIN_C0:
		mask = 0xf<<20;
		*value = 0x6<<20;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_C1:
		mask = 0xf<<12;
		*value = 0x7<<12;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_C2:
		mask = 0xf<<8;
		*value = 0x6<<8;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_C3:
		mask = 0xf<<4;
		*value = 0x6<<4;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_C4:
		mask = 0xf<<16;
		*value = 0x2<<16;
		*address = BSP_PIN_MUX_SEL16;
		break;
	case BSP_GPIO_PIN_C5:
		mask = 0xf<<12;
		*value = 0x6<<12;
		*address = BSP_PIN_MUX_SEL16;
		break;
	case BSP_GPIO_PIN_C6:
		mask = 0xf<<8;
		*value = 0x8<<8;
		*address = BSP_PIN_MUX_SEL16;
		break;
	case BSP_GPIO_PIN_C7:
		mask = 0xf<<4;
		*value = 0x5<<4;
		*address = BSP_PIN_MUX_SEL16;
		break;

	case BSP_GPIO_PIN_D0:
		mask = 0xf<<0;
		*value = 0x5<<0;
		*address = BSP_PIN_MUX_SEL16;
		break;
	case BSP_GPIO_PIN_D1:
		mask = 0xf<<28;
		*value = 0x5<<28;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D2:
		mask = 0xf<<24;
		*value = 0x5<<24;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D3:
		mask = 0xf<<20;
		*value = 0x5<<20;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D4:
		mask = 0xf<<16;
		*value = 0x5<<16;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D5:
		mask = 0xf<<12;
		*value = 0x7<<12;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D6:
		mask = 0xf<<8;
		*value = 0x6<<8;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D7:
		mask = 0xf<<4;
		*value = 0x7<<4;
		*address = BSP_PIN_MUX_SEL15;
		break;
	
	case BSP_GPIO_PIN_E0:
		mask = 0xf<<0;
		*value = 0x7<<0;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_E1:
		mask = 0xf<<28;
		*value = 0x1<<28;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E2:
		mask = 0xf<<24;
		*value = 0x1<<24;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E3:
		mask = 0xf<<20;
		*value = 0x1<<20;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E4:
		mask = 0xf<<16;
		*value = 0x1<<16;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E5:
		mask = 0xf<<12;
		*value = 0x1<<12;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E6:
		mask = 0xf<<8;
		*value = 0x1<<8;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E7:
		mask = 0xf<<4;
		*value = 0x1<<4;
		*address = BSP_PIN_MUX_SEL17;
		break;

	case BSP_GPIO_PIN_F0:
		mask = 0xf<<0;
		*value = 0x1<<0;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_F1:
		mask = 0xf<<28;
		*value = 0x2<<28;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F2:
		mask = 0xf<<24;
		*value = 0x4<<24;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F3:
		mask = 0xf<<20;
		*value = 0x4<<20;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F4:
		mask = 0xf<<16;
		*value = 0x6<<16;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F5:
		mask = 0xf<<12;
		*value = 0x6<<12;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F6:
		mask = 0xf<<24;
		*value = 0x6<<24;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_F7:
		mask = 0xf<<28;
		*value = 0x6<<28;
		*address = BSP_PIN_MUX_SEL8;
		break;

	case BSP_GPIO_PIN_G0:
		mask = 0xf<<20;
		*value = 0x6<<20;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_G1:
		mask = 0xf<<16;
		*value = 0x7<<16;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_G2:
		mask = 0xf<<12;
		*value = 0x7<<12;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_G3:
		mask = 0xf<<28;
		*value = 0x2<<28;
		*address = BSP_PIN_MUX_SEL9;
		break;
	case BSP_GPIO_PIN_G4:
		mask = 0xf<<24;
		*value = 0x1<<24;
		*address = BSP_PIN_MUX_SEL9;
		break;
	case BSP_GPIO_PIN_G5:
		mask = 0xf<<20;
		*value = 0x0<<20;
		*address = BSP_PIN_MUX_SEL9;
		break;
	case BSP_GPIO_PIN_G6:
		mask = 0xf<<28;
		*value = 0x3<<28;
		*address = BSP_PIN_MUX_SEL13;
		break;
	case BSP_GPIO_PIN_G7:
		mask = 0xf<<24;
		*value = 0x3<<24;
		*address = BSP_PIN_MUX_SEL13;
		break;

	case BSP_GPIO_PIN_H0:
		mask = 0xf<<20;
		*value = 0x3<<20;
		*address = BSP_PIN_MUX_SEL13;
		break;
	case BSP_GPIO_PIN_H1:
		mask = 0xf<<16;
		*value = 0x2<<16;
		*address = BSP_PIN_MUX_SEL13;
		break;
	case BSP_GPIO_PIN_H2:
		mask = 0xf<<28;
		*value = 0x2<<28;
		*address = BSP_PIN_MUX_SEL14;
		break;
	case BSP_GPIO_PIN_H3:
		mask = 0xf<<24;
		*value = 0x1<<24;
		*address = BSP_PIN_MUX_SEL12;
		break;
	case BSP_GPIO_PIN_H4:
		mask = 0xf<<28;
		*value = 0x1<<28;
		*address = BSP_PIN_MUX_SEL12;
		break;
	case BSP_GPIO_PIN_H5:
		mask = 0xf<<0;
		*value = 0x1<<0;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_H6:
		break;

	case BSP_GPIO_PIN_H7:
		break;

	case BSP_UART1_PIN:
		break;
	case BSP_UART2_PIN:
		break;
	default:
		break;
	}

	return mask;
}

static void gpio_set_direction(unsigned int pin, unsigned int direction)
{
    unsigned int mask, value, address;

    mask = rtl819x_gpio_mux(pin, &value, &address);

    /* set pin mux as gpio */
    RTL_W32(address, (RTL_R32(address) & ~(mask) | (value)));

    /* turn on gpio pin */
    RTL_W32(BSP_GPIO_CNR_REG(pin), 
           (RTL_R32(BSP_GPIO_CNR_REG(pin)) 
           & (~(1 << BSP_GPIO_BIT(pin)))));

    /* set gpio as input or output */
    if (!direction){
        RTL_W32(BSP_GPIO_DIR_REG(pin), 
            (RTL_R32(BSP_GPIO_DIR_REG(pin)) 
            | ((1 << BSP_GPIO_BIT(pin)))));
	} else {
        RTL_W32(BSP_GPIO_DIR_REG(pin), 
            (RTL_R32(BSP_GPIO_DIR_REG(pin)) 
            & (~(1 << BSP_GPIO_BIT(pin)))));
    }
}

static void set_gpio_value(unsigned int gpio_nr, unsigned int active)
{
    unsigned int reg_val;

    gpio_set_direction(gpio_nr, 0);//输出
    reg_val = REG32(BSP_GPIO_DAT_REG(gpio_nr));
    if (active) {
        reg_val &= ~(1 << BSP_GPIO_BIT(gpio_nr));
    } else {
        reg_val |= (1 << BSP_GPIO_BIT(gpio_nr));
    }
    
    REG32(BSP_GPIO_DAT_REG(gpio_nr)) = reg_val;
}

static void rtk_gpio_write(unsigned int gpio,int val)
{
#if defined(CONFIG_RTL_8367R_SUPPORT)
	gpio -= 32;
	unsigned int setting = RTL_R32(BSP_PEFGH_DAT);
#else
    unsigned int setting = RTL_R32(BSP_PABCD_DAT);
#endif
    if (val) {
        setting |= (1 << gpio);
    } else {
        setting &= (~(1 << gpio));
    }
#if defined(CONFIG_RTL_8367R_SUPPORT)
	RTL_W32(BSP_PEFGH_DAT, setting);
#else
    RTL_W32(BSP_PABCD_DAT, setting);
#endif
}

static int rtk_gpio_read(unsigned int gpio)
{
#if defined(CONFIG_RTL_8367R_SUPPORT)
	if (RTL_R32(BSP_PEFGH_DAT) & (1 << (gpio - 32))) 
#else
    if (RTL_R32(BSP_PABCD_DAT) & (1 << gpio))
#endif
        return 0;
    else
        return 1;
}

static void rtk_gpio_out(unsigned int gpio)
{
#if defined(CONFIG_RTL_8367R_SUPPORT)
	RTL_W32(BSP_PEFGH_DIR, (RTL_R32(BSP_PEFGH_DIR) | (1 << (gpio - 32))));
#else
    RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | (1 << gpio)));
#endif
}

static void rtk_gpio_in(unsigned int gpio)
{
    RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR)&(~(1 << gpio))));
}

int reset_pin_status(void)
{
    rtk_gpio_in(RESET_BTN_PIN);
    return rtk_gpio_read(RESET_BTN_PIN);
}

int wps_pin_status(void)
{
    rtk_gpio_in(WPS_BTN_PIN);
    return rtk_gpio_read(WPS_BTN_PIN);
}

#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
unsigned g_set_lan_wan_led = 0;
#endif

#ifdef __CONFIG_WPS_RTK__

static unsigned int wps_led_blink_flag = 0;
static unsigned int wps_led_toggle_flag = 0;

/*****************************************************************************
 函 数 名  : wps_led_on_off
 功能描述  : 控制wps指示灯开、关
 输入参数  : int val  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年10月9日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
void wps_led_on_off(int val)
{
 
    if(nvram_match("BOARD_NAME","F3_V2.0"))
    {   
        //F3海外版使用了射频芯片的GPIO0作为wps指示灯
        RTLWIFINIC_GPIO_write_proc(NULL, 0, val);
    }
    else
    {
        /* 解决当WPS灯闪烁的时候系统灯不正常 */
        if(wps_led_blink_flag)
        {
            if (val == LED_OFF)
            {
                 set_gpio_value(SYSTEM_LED_PIN, LED_OFF);
            } 
            else
            {
                 set_gpio_value(SYSTEM_LED_PIN, LED_ON);
            }
        }
       set_gpio_value(WPS_LED_PIN, val);
    }
}

void wps_led_off(void)
{
    wps_led_on_off(LED_OFF);
    wps_led_blink_flag = 0;
    printf("%s,wps_led_blink_flag= %u\n",__func__,wps_led_blink_flag);
}

void wps_led_on(void)
{
    wps_led_on_off(LED_ON);
    wps_led_blink_flag = 0;
    printf("%s,wps_led_blink_flag= %u\n",__func__,wps_led_blink_flag);
}

void wps_led_blink(void)
{
    wps_led_on_off(LED_OFF);
    wps_led_blink_flag = 1;
    wps_led_toggle_flag = 1;
    printf("%s,wps_led_blink_flag= %u\n",__func__,wps_led_blink_flag);
}

/*****************************************************************************
 函 数 名  : wps_led_timer
 功能描述  : wps指示灯定时器接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年10月9日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
void wps_led_timer()
{
    if (wps_led_blink_flag==1)
    {
        if (wps_led_toggle_flag) 
        {
            wps_led_on_off(LED_OFF);
        }
        else 
        {
            wps_led_on_off(LED_ON);
        }
        wps_led_toggle_flag = wps_led_toggle_flag ? 0 : 1;
    }
}


/*****************************************************************************
 函 数 名  : wps_led_control
 功能描述  : 提供给wps模块的接口，用于控制wps指示灯
 输入参数  : int value  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年10月9日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
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
}

#endif

int wifi_swtich_pin_status(void)
{
    rtk_gpio_in(WIFI_SWITCH_BTN_PIN);
    return rtk_gpio_read(WIFI_SWITCH_BTN_PIN);
}

void sys_led_turn_on_off(int val)
{
    
    rtk_gpio_out(SYSTEM_LED_PIN);
    if(val) {
        rtk_gpio_write(SYSTEM_LED_PIN,LED_ON);
    } else {
        rtk_gpio_write(SYSTEM_LED_PIN,LED_OFF);
    }
}

#ifdef __CONFIG_EXTEND_LED__
void extend_led_on_off(int val)
{
    unsigned int setting;

    /* SET GPIOB2~GPIOB6 as GPIO PIN*/
    RTL_W32(BSP_PIN_MUX_SEL_2, (RTL_R32(BSP_PIN_MUX_SEL_2) | RTL_GPIO_MUX_2_GPIOB5));
    RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~(RTL_GPIO_CNR_GPIOB5)));
    RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~(RTL_GPIO_CNR_GPIOB5)));
    /* SET GPIOB2~GPIOB6 as OUTPUT */
    RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | RTL_GPIO_CNR_GPIOB5));

    /* SET value */
    setting = RTL_R32(BSP_PABCD_DAT);
    if (val == LED_OFF) {
        setting |= RTL_GPIO_CNR_GPIOB5;
    } else {
        setting &= (~RTL_GPIO_CNR_GPIOB5);
    }
    RTL_W32(BSP_PABCD_DAT, setting);
}


//参数ext绿色，sys红色
void extend_and_sys_led_on_off(int ext,int sys)
{
    unsigned int setting;

    /* SET GPIOB2~GPIOB6 as GPIO PIN*/
    RTL_W32(BSP_PIN_MUX_SEL_2, (RTL_R32(BSP_PIN_MUX_SEL_2) | RTL_GPIO_MUX_2_GPIOB5));
    RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~(RTL_GPIO_CNR_GPIOB5)));
    RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~(RTL_GPIO_CNR_GPIOB5)));
    /* SET GPIOB2~GPIOB6 as OUTPUT */
    RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | RTL_GPIO_CNR_GPIOB5));

    /* SET value */
    setting = RTL_R32(BSP_PABCD_DAT);
    if (ext == LED_OFF) {//绿色灯
        setting |= RTL_GPIO_CNR_GPIOB5;
    } else {
        setting &= (~RTL_GPIO_CNR_GPIOB5);
    }

    if (sys == LED_OFF) {//红色灯
        setting |= (1 << SYSTEM_LED_PIN);
    } else {
        setting &= (~(1 << SYSTEM_LED_PIN));
    }
    RTL_W32(BSP_PABCD_DAT, setting);
}
#endif

#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
void set_lan_led_on_off(unsigned int val)
{
    unsigned int reg_data;
    unsigned int setting;
      
    reg_data = RTL_R32(BSP_PIN_MUX_SEL13);
    reg_data = (reg_data & ~0xf0000000 | 0x30000000);//暂时先写死，后续添加复用表
    RTL_W32(BSP_PIN_MUX_SEL13,reg_data);
    RTL_W32(BSP_PEFGH_CNR, (RTL_R32(BSP_PEFGH_DIR) | RTL_GPIO_CNR_GPIOG6));
    RTL_W32(BSP_PEFGH_DIR, (RTL_R32(BSP_PEFGH_DIR) | RTL_GPIO_CNR_GPIOG6));

    /* SET value */
    setting = RTL_R32(BSP_PEFGH_DAT);
    if (val == LED_OFF) {
        setting |= RTL_GPIO_CNR_GPIOG6;
    } else {
        setting &= (~RTL_GPIO_CNR_GPIOG6);
    }
    RTL_W32(BSP_PEFGH_DAT, setting);
}
#endif

/*****************************************************************************
 函 数 名  : set_lan_port_as_gpio
 功能描述  : 设置lan_port为GPIO
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月7日
    作    者   : cz
    修改内容   : 修改lan口灯GPIOP配置

*****************************************************************************/
void set_lan_port_as_gpio()
{
    /* SET GPIO6~GPIOH2 as GPIO PIN*/
    unsigned int reg_data;
    
#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
    g_set_lan_wan_led = 1;
#endif
#if !defined(CONFIG_RTL_8367R_SUPPORT)
    reg_data = RTL_R32(BSP_PIN_MUX_SEL13);
    reg_data = ((reg_data & ~0xffff0000) | 0x33320000); //G6-H1
    RTL_W32(BSP_PIN_MUX_SEL13,reg_data);
    reg_data = RTL_R32(BSP_PIN_MUX_SEL14);
    reg_data = ((reg_data & ~(0xf << 28)) | (0x2 << 28)); //H2
    RTL_W32(BSP_PIN_MUX_SEL14,reg_data);
    RTL_W32(BSP_PEFGH_CNR, (RTL_R32(BSP_PEFGH_DIR) | RTL_GPIO_LAN_LED_PIN));
    RTL_W32(BSP_PEFGH_DIR, (RTL_R32(BSP_PEFGH_DIR) | RTL_GPIO_LAN_LED_PIN));
#endif
}
/*****************************************************************************
 函 数 名  : set_lan_port_as_led
 功能描述  : 设置lan_port为led
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月16日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void set_lan_port_as_led()
{
    unsigned int reg_data;

#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
        g_set_lan_wan_led = 0;
#endif
#if !defined(CONFIG_RTL_8367R_SUPPORT)
    reg_data = RTL_R32(BSP_PIN_MUX_SEL13); //G6-H1
    reg_data = (reg_data & ~0xffff0000);
    RTL_W32(BSP_PIN_MUX_SEL13,reg_data);
    reg_data = RTL_R32(BSP_PIN_MUX_SEL14);
    reg_data = (reg_data & ~(0xf << 28)); //H2
    RTL_W32(BSP_PIN_MUX_SEL14,reg_data);
#endif
}
/*****************************************************************************/

void lan_wan_port_led_on_off(int val)
{
    unsigned int setting;

    set_lan_port_as_gpio();
#if !defined(CONFIG_RTL_8367R_SUPPORT)	
    cyg_thread_delay(1);

    /* SET value */
    setting = RTL_R32(BSP_PEFGH_DAT);
    if (val == LED_OFF) {
        setting |= RTL_GPIO_LAN_LED_PIN;
    } else {
        setting &= (~RTL_GPIO_LAN_LED_PIN);
    }
    RTL_W32(BSP_PEFGH_DAT, setting);
    cyg_thread_delay(1);
#endif
}
/*****************************************************************************
 函 数 名  : lan_port_led_on_off
 功能描述  : 开关所有lan口的灯
 输入参数  : int val  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月7日
    作    者   : cz
    修改内容   : 修改lan口灯GPIOP配置

*****************************************************************************/
void lan_port_led_on_off(int val)
{
    unsigned int setting;

    set_lan_port_as_gpio();
    /* SET value */
    setting = RTL_R32(BSP_PEFGH_DAT);
    if (val == LED_OFF) {
        setting |= RTL_GPIO_LAN_LED_PIN;
    } else {
        setting &= (~RTL_GPIO_LAN_LED_PIN);
    }
    RTL_W32(BSP_PEFGH_DAT, setting);
}

/*****************************************************************************
 函 数 名  : tenda_all_led_on_off
 功能描述  : 开关所有LED灯
 输入参数  : int val  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月7日
    作    者   : cz
    修改内容   : led全开全关接口设置
*****************************************************************************/
void tenda_all_led_on_off(int val)
{
    char buf[32];

    /***set lan_wan_led***/
    lan_wan_port_led_on_off((val));
#ifdef __CONFIG_WPS_RTK__
    wps_led_control(val); 
#endif
    sys_led_turn_on_off(!val); 
   /*********2.4_5g  led**********/
    if (val == 0) {
        sprintf(buf,"wlan0 led 0");
        run_clicmd(buf);
        memset(buf,0x0,sizeof(buf));
	    sprintf(buf,"wlan1 led 0");
        run_clicmd(buf);
    } else {
        sprintf(buf,"wlan0 led 1");
        run_clicmd(buf);
        memset(buf,0x0,sizeof(buf));
	    sprintf(buf,"wlan1 led 1");
        run_clicmd(buf);
    }

}

#ifdef __CONFIG_LED__
extern int set_sys_led_turn(int turn_flag);
int tenda_set_all_led_off(void)
{
	lan_wan_port_led_on_off(0);
#ifdef __CONFIG_WPS_RTK__
	wps_led_control(0); 
#endif
	set_sys_led_turn(0);
	sys_led_turn_on_off(1); 
}
/*****************************************************************************
 函 数 名  : tenda_set_all_led_on
 功能描述  : 设置设置所有的LED开启
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : 
    修改内容   : 新生成函数

*****************************************************************************/
int tenda_set_all_led_on(void)
{
	set_lan_port_as_led();

	set_sys_led_turn(1);
	sys_led_turn_on_off(0);
	return 0;
}
#endif
int tenda_button_status(unsigned int gpio)
{
    int status; 
    status = rtk_gpio_read(gpio);
    if (status == 1) {
        return BUTTON_PUSH_UP;
    } else {
        return BUTTON_PUSH_DOWN;
    }
}

void tenda_gpio_init(void)
{
    /*****配置B4,B7,C1,C3,为GPIO模式/wps_led、sys_led,reset/wps_btn,wifi_btn****/
    RTL_W32(BSP_PIN_MUX_SEL1, ((RTL_R32(BSP_PIN_MUX_SEL1) & ~((0xf << 24))) | (0x8 << 24)));    
    RTL_W32(BSP_PIN_MUX_SEL2, ((RTL_R32(BSP_PIN_MUX_SEL2) & ~((0xf << 4))) | (0x6 << 4)));  
    RTL_W32(BSP_PIN_MUX_SEL2, ((RTL_R32(BSP_PIN_MUX_SEL2) & ~((0xf << 24))) | (0x8 << 24)));
    RTL_W32(BSP_PIN_MUX_SEL2, ((RTL_R32(BSP_PIN_MUX_SEL2) & ~((0xf << 4))) | (0x6 << 4)));
    RTL_W32(BSP_PIN_MUX_SEL2, ((RTL_R32(BSP_PIN_MUX_SEL2) & ~((0xf << 12))) | (0x7 << 12)));
    RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~((1<<15) | (1<<12) |(1<<19) |(1<<17))));  
    RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) & ~((1<<17) | (1<<19))));    
    RTL_W32(BSP_PABCD_DIR, (RTL_R32(BSP_PABCD_DIR) | ((1<< 15)|(1<<12))));

    /******初始化时网口灯配置为复用led/phy模式******/
    set_lan_port_as_led();
}

#if 0
int gpio_cmd_main(int argc,char **argv)
{
    unsigned int gpio;
    int val = 0;

    if (argc == 1) {
        gpio = strtoul(argv[0],NULL,0);
        printf("Read  gpio:0x%x val:0x%x\n",gpio,rtk_gpio_read(gpio));
    } else if (argc == 2) {
        gpio = strtoul(argv[0],NULL,0);
        val = strtoul(argv[1],NULL,0);
        rtk_gpio_out(gpio);
        rtk_gpio_write(gpio,val);
        
        printf("Write gpio1:0x%x val:0x%x\n",gpio,val);
    } else if(argc == 3) {
        tenda_gpio_init();
        //printf("--enable gpio b6\n");
        //RTL_W32(BSP_PIN_MUX_SEL_2, (RTL_R32(BSP_PIN_MUX_SEL_2) | RTL_GPIO_MUX_2_GPIOB6));
        //RTL_W32(BSP_PABCD_CNR, (RTL_R32(BSP_PABCD_CNR) & ~(RTL_GPIO_CNR_GPIOB6)));
    } else {
        //printf("unkown cmds!!! \n");
        printf("BSP_PIN_MUX_SEL  (0x%x) = 0x%x\n",BSP_PIN_MUX_SEL,RTL_R32(BSP_PIN_MUX_SEL));
        printf("BSP_PIN_MUX_SEL_2(0x%x) = 0x%x\n",BSP_PIN_MUX_SEL_2,RTL_R32(BSP_PIN_MUX_SEL_2));
        printf("BSP_PABCD_CNR    (0x%x) = 0x%x\n",BSP_PABCD_CNR,RTL_R32(BSP_PABCD_CNR));
        printf("BSP_PABCD_DIR    (0x%x) = 0x%x\n",BSP_PABCD_DIR,RTL_R32(BSP_PABCD_DIR));
        printf("BSP_PABCD_DAT    (0x%x) = 0x%x\n",BSP_PABCD_DAT,RTL_R32(BSP_PABCD_DAT));
    }
    return 0;
}
#endif
