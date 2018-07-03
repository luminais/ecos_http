#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/hal/bspchip.h>
#include <cyg/infra/diag.h>
#include <cyg/kernel/kapi.h>
#include "gpio_api.h"

#define RTL_R32(addr)       (*(volatile unsigned long *)(addr))
#define RTL_W32(addr, l)    ((*(volatile unsigned long*)(addr)) = (l))

/*****************************************************************************
 函 数 名  : rtl819x_gpio_mux
 功能描述  : 获取 gpio 的复用值
 输入参数  : pin
 输出参数  : value , address
 返 回 值  : make
 
 修改历史      :
  1.日    期   : 2018年6月15日
    作    者   : 陈志
    修改内容   : 新生成函数
    修改人： 陈志 ，走v2.0接口

*****************************************************************************/

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

/*****************************************************************************
 函 数 名  : get_gpio_value
 功能描述  : 得到gpio 引脚的高低电平值
 输入参数  : gpio_id
 输出参数  : 无
 返 回 值  : 0或1
 
 修改历史      :
  1.日    期   : 2018年6月15日
    作    者   : 陈志
    修改内容   : 新生成函数
    修改人： 陈志

*****************************************************************************/
static int get_gpio_value(unsigned int gpio_id)
{
    unsigned int value;

    value = (RTL_R32(BSP_GPIO_DAT_REG(gpio_id)));

    if (value & (1 << BSP_GPIO_BIT(gpio_id))) {
        return 0;
    } else {
        return 1;
    }
}

/*****************************************************************************
 函 数 名  : set_gpio_value
 功能描述  : 设置gpio 引脚的高低电平值
 输入参数  : gpio_id， value:0为低，1为高
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年6月15日
    作    者   : 陈志
    修改内容   : 新生成函数
    修改人： 陈志 
*****************************************************************************/
static void set_gpio_value(unsigned int gpio_id, unsigned int value)
{
    unsigned int reg_val;

    reg_val = REG32(BSP_GPIO_DAT_REG(gpio_id));
    if (value) {
        reg_val &= ~(1 << BSP_GPIO_BIT(gpio_id));
    } else {
        reg_val |= (1 << BSP_GPIO_BIT(gpio_id));
    }
    REG32(BSP_GPIO_DAT_REG(gpio_id)) = reg_val;

    return;
}

/*****************************************************************************
 函 数 名  : set_gpio_mux_mode
 功能描述  : 设置gpio 引脚的复用模式
 输入参数  : gpio_id， mode:0 其他复用模式，1为GPIO模式
 输出参数  : 无
 返 回 值  : 无
 
 修改历史      :
  1.日    期   : 2018年6月15日
    作    者   : 陈志
    修改内容   : 新生成函数
    修改人： 陈志 
*****************************************************************************/
static void set_gpio_mux_mode(unsigned int gpio_id, unsigned int mode)
{
    unsigned int mask, value, address;

    mask = rtl819x_gpio_mux(gpio_id, &value, &address);
    if (mode) {
        RTL_W32(address, (RTL_R32(address) & ~(mask) | (value)));
    } else {
        RTL_W32(address, (RTL_R32(address | mask)));
    }

    return;
}

/*****************************************************************************
 函 数 名  : set_gpio_direction
 功能描述  : 设置gpio 引脚的输入输出方向
 输入参数  : gpio_id， direction:0 输出，1为G
 输出参数  : 无
 返 回 值  : 无
 
 修改历史      :
  1.日    期   : 2018年6月15日
    作    者   : 陈志
    修改内容   : 新生成函数
    修改人： 陈志 
*****************************************************************************/
static void set_gpio_direction(unsigned int gpio_id, unsigned int direction)
{
    unsigned int value;

    set_gpio_mux_mode(gpio_id, 1);

    value = RTL_R32(BSP_GPIO_CNR_REG(gpio_id));
    value &=(~(1 << BSP_GPIO_BIT(gpio_id)));
     
    RTL_W32(BSP_GPIO_CNR_REG(gpio_id), value);

    /* set gpio as input or output */
    value = RTL_R32(BSP_GPIO_DIR_REG(gpio_id));
    if (!direction){
        value |= (1 << BSP_GPIO_BIT(gpio_id));
    } else {
        value &= (~(1 << BSP_GPIO_BIT(gpio_id)));  
    }

    RTL_W32(BSP_GPIO_DIR_REG(gpio_id), value);

    return;
}


#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
unsigned int g_set_lan_wan_led = 0;
#endif

/*****************************************************************************
 函 数 名  : set_lan_port_as_gpio
 功能描述  : 设置lan_port为GPIO
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年6月15日
    作    者   : cz
    修改内容   : 修改lan口灯GPIOP配置,接口优化，走2.0版本

*****************************************************************************/
void set_lan_port_as_gpio(void)
{
#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
    g_set_lan_wan_led = 1; /*不使用定时器自动控制*/
#else
#if defined(CONFIG_RTL_8367R_SUPPORT)
    /*如果使用一组的led，在此add 控制8367 led 函数*/
#else
    set_gpio_direction(54, 0);
    set_gpio_direction(55, 0);
    set_gpio_direction(56, 0);
    set_gpio_direction(57, 0);
    set_gpio_direction(58, 0);
#endif
#endif
    return;
}

/*****************************************************************************
 函 数 名  : set_lan_port_as_led
 功能描述  : 设置lan_port为led模式，即网口灯模式
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年6月15日
    作    者   : liquan
    修改内容   : 新生成函数
    修改人： 陈志 ，走v2.0接口

*****************************************************************************/
void set_lan_port_as_led(void)
{
#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
        g_set_lan_wan_led = 0; /*使用定时器自动控制恢复正常*/
#else
#if defined(CONFIG_RTL_8367R_SUPPORT)
    /*在此添加8367控制led灯函数相关值*/
#else
    set_gpio_mux_mode(54, 1);
    set_gpio_mux_mode(55, 1);
    set_gpio_mux_mode(56, 1);
    set_gpio_mux_mode(57, 1);
    set_gpio_mux_mode(58, 1);
#endif
#endif
    return;
}

/*****************************************************************************
 函 数 名  : lan_wan_port_led_on_off
 功能描述  : 设置lan_wan 同时常亮或同时常灭
 输入参数  : val: 0 灭， 1亮
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月7日
    作    者   : cz
    修改内容   : 修改lan_wan口灯GPIOP配置
    修改日期：2018/6/15
    修改人： 陈志 ，走v2.0接口

*****************************************************************************/
void lan_wan_port_led_on_off(int val)
{
    set_lan_port_as_gpio();

#if defined(__CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__)
    set_lan_led_on_off(val);
#if defined(__CONFIG_WAN_LED_GPIO_NUM__)
    cyg_thread_delay(2);
    set_wan_led_on_off(val);
#endif

#else
#if defined(CONFIG_RTL_8367R_SUPPORT)
    /*如果使用8367接多个外置网口灯，此处添加控制函数*/
#else
    set_gpio_value(54, val);
    set_gpio_value(55, val);
    set_gpio_value(56, val);
    set_gpio_value(57, val);
    set_gpio_value(58, val);
#endif
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
    修改日期：2018/6/15
    修改人： 陈志 ，走v2.0接口

*****************************************************************************/
void lan_port_led_on_off(int val)
{
    lan_wan_port_led_on_off(val);

    return;
}

/*****************************************************************************
 函 数 名  : tenda_button_status
 功能描述  : 获取按键值
 输入参数  : int gpio  
 输出参数  : 无
 返 回 值  : 0 或 1 
 
 修改历史      :
  1.日    期   : 2017年11月7日
    作    者   : cz
    修改内容   : 使用统一获取gpio值函数
    修改日期：2018/6/15
    修改人： 陈志 ，走v2.0接口

*****************************************************************************/
int tenda_button_status(unsigned int gpio)
{
    int status; 

    status = get_gpio_value(gpio);
    if (status == 1) {
        return BUTTON_PUSH_UP;
    } else {
        return BUTTON_PUSH_DOWN;
    }
}

/*****************************************************************************
 函 数 名  : set_lan_led_on_off
 功能描述  : 普通 gpio 作为 lan 口灯使用时
 输入参数  : int value  
 输出参数  : 无
 返 回 值  : 无
 
 修改历史      :
    作    者   : cz
    新增函数   : 设置lan口灯的状态值
    修改日期：2018/6/15

*****************************************************************************/
#if defined(__CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__)
void set_lan_led_on_off(unsigned int value)
{
#if defined(__CONFIG_SW_LED_LOW_ACTIVE__)
    set_gpio_value(__CONFIG_LAN_LED_GPIO_NUM__, ((value == LED_ON) ? LED_OFF : LED_ON));
#else
    set_gpio_value(__CONFIG_LAN_LED_GPIO_NUM__, value);
#endif
}

/*****************************************************************************
 函 数 名  : set_wan_led_on_off
 功能描述  : 普通 gpio 作为 wan口灯使用时
 输入参数  : int value  
 输出参数  : 无
 返 回 值  : 无
 
 修改历史      :
    作    者   : cz
    新增函数   : 设置wan口灯的状态值
    修改日期：2018/6/15

*****************************************************************************/
#if defined(__CONFIG_WAN_LED_GPIO_NUM__)
void set_wan_led_on_off(unsigned int value)
{
#if defined(__CONFIG_SW_LED_LOW_ACTIVE__)
    set_gpio_value(__CONFIG_WAN_LED_GPIO_NUM__, ((value == LED_ON) ? LED_OFF : LED_ON));
#else
    set_gpio_value(__CONFIG_WAN_LED_GPIO_NUM__, value);
#endif  
    return;
}
#endif
#endif /*__CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__ end */

/*****************************************************************************
 函 数 名  : sys_led_turn_on_off
 功能描述  : 使用统一设置gpio值函数设置系统灯状态
 输入参数  : int val  
 输出参数  : 无
 返 回 值  : 无
 
 修改历史      :
    修改日期：2018/6/15
    修改人：cz

*****************************************************************************/
void sys_led_turn_on_off(int val)
{
    if(val) {
        set_gpio_value(SYSTEM_LED_PIN,LED_ON);
    } else {
        set_gpio_value(SYSTEM_LED_PIN,LED_OFF);
    }

    return;
}

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
    /* 解决当WPS灯闪烁的时候系统灯不正常 */
    if(wps_led_blink_flag) {
        if (val == LED_OFF) {
             set_gpio_value(SYSTEM_LED_PIN, LED_OFF);
        } else {
             set_gpio_value(SYSTEM_LED_PIN, LED_ON);
        }
    }
    cyg_thread_delay(2);
    set_gpio_value(WPS_LED_PIN, val);
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


#ifdef __CONFIG_LED__
extern int set_sys_led_turn(int turn_flag);
int tenda_set_all_led_off(void)
{
    lan_wan_port_led_on_off(0);
#ifdef __CONFIG_WPS_RTK__
    wps_led_control(0); 
#endif
    set_sys_led_turn(0);
    sys_led_turn_on_off(0); 
}
/*****************************************************************************
 函 数 名  : tenda_set_all_led_on
 功能描述  : 设置设置所有的LED开启, 恢复正常模式
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
    sys_led_turn_on_off(1);
    return 0;
}
#endif

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
    lan_wan_port_led_on_off(val);
#ifdef __CONFIG_WPS_RTK__
    wps_led_control(val); 
#endif
    sys_led_turn_on_off(val); 
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
