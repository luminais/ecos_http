#ifndef __GPIO_API_H__
#define __GPIO_API_H__

#include <bcmnvram.h>

#define BUTTON_PUSH_UP		1
#define BUTTON_PUSH_DOWN	0

#define BSP_PIN_MUX_SEL0            0xB8000800UL
#define BSP_PIN_MUX_SEL1            0xB8000804UL
#define BSP_PIN_MUX_SEL2            0xB8000808UL
#define BSP_PIN_MUX_SEL3            0xB800080CUL
#define BSP_PIN_MUX_SEL4            0xB8000810UL
#define BSP_PIN_MUX_SEL5            0xB8000814UL
#define BSP_PIN_MUX_SEL6            0xB8000818UL
#define BSP_PIN_MUX_SEL7            0xB800081CUL
#define BSP_PIN_MUX_SEL8            0xB8000820UL
#define BSP_PIN_MUX_SEL9            0xB8000824UL
#define BSP_PIN_MUX_SEL10           0xB8000828UL
#define BSP_PIN_MUX_SEL11           0xB800082CUL
#define BSP_PIN_MUX_SEL12           0xB8000830UL
#define BSP_PIN_MUX_SEL13           0xB8000834UL
#define BSP_PIN_MUX_SEL14           0xB8000838UL
#define BSP_PIN_MUX_SEL15           0xB800083CUL
#define BSP_PIN_MUX_SEL16           0xB8000840UL
#define BSP_PIN_MUX_SEL17           0xB8000844UL
#define BSP_PIN_MUX_SEL18           0xB8000848UL

#define BSP_GPIO_BASE               (0xB8003500UL)
#define BSP_PABCD_CNR               (BSP_GPIO_BASE + 0x00)      /* Port ABCD control */
#define BSP_PABCD_PTYPE             (BSP_GPIO_BASE + 0x04)      /* Port ABCD type */
#define BSP_PABCD_DIR               (BSP_GPIO_BASE + 0x08)      /* Port ABCD direction */
#define BSP_PABCD_DAT               (BSP_GPIO_BASE + 0x0C)      /* Port ABCD data */
#define BSP_PABCD_ISR               (BSP_GPIO_BASE + 0x10)      /* Port ABCD interrupt status */
#define BSP_PAB_IMR                 (BSP_GPIO_BASE + 0x14)      /* Port AB interrupt mask */
#define BSP_PCD_IMR                 (BSP_GPIO_BASE + 0x18)      /* Port CD interrupt mask */
#define BSP_PEFGH_CNR               (BSP_GPIO_BASE + 0x1C)      /* Port EFGH control */
#define BSP_PEFGH_PTYPE             (BSP_GPIO_BASE + 0x20)      /* Port EFGH type */
#define BSP_PEFGH_DIR               (BSP_GPIO_BASE + 0x24)      /* Port EFGH direction */
#define BSP_PEFGH_DAT               (BSP_GPIO_BASE + 0x28)      /* Port EFGH data */
#define BSP_PEFGH_ISR               (BSP_GPIO_BASE + 0x2C)      /* Port EFGH interrupt status */
#define BSP_PEF_IMR                 (BSP_GPIO_BASE + 0x30)      /* Port EF interrupt mask */
#define BSP_PGH_IMR                 (BSP_GPIO_BASE + 0x34)      /* Port GH interrupt mask */

/*
 * GPIO PIN
 */
enum BSP_GPIO_PIN
{
    BSP_GPIO_PIN_A0 = 0,
    BSP_GPIO_PIN_A1,
    BSP_GPIO_PIN_A2,
    BSP_GPIO_PIN_A3,
    BSP_GPIO_PIN_A4,
    BSP_GPIO_PIN_A5,
    BSP_GPIO_PIN_A6,
    BSP_GPIO_PIN_A7,

    BSP_GPIO_PIN_B0,
    BSP_GPIO_PIN_B1,
    BSP_GPIO_PIN_B2,
    BSP_GPIO_PIN_B3,
    BSP_GPIO_PIN_B4,
    BSP_GPIO_PIN_B5,
    BSP_GPIO_PIN_B6,
    BSP_GPIO_PIN_B7,
    
    BSP_GPIO_PCD_REG,
    BSP_GPIO_PIN_C0 = BSP_GPIO_PCD_REG,
    BSP_GPIO_PIN_C1,
    BSP_GPIO_PIN_C2,
    BSP_GPIO_PIN_C3,
    BSP_GPIO_PIN_C4,
    BSP_GPIO_PIN_C5,
    BSP_GPIO_PIN_C6,
    BSP_GPIO_PIN_C7,

    BSP_GPIO_PIN_D0,
    BSP_GPIO_PIN_D1,
    BSP_GPIO_PIN_D2,
    BSP_GPIO_PIN_D3,
    BSP_GPIO_PIN_D4,
    BSP_GPIO_PIN_D5,
    BSP_GPIO_PIN_D6,
    BSP_GPIO_PIN_D7,

    BSP_GPIO_2ND_REG, 
    BSP_GPIO_PIN_E0 = BSP_GPIO_2ND_REG,
    BSP_GPIO_PIN_E1,
    BSP_GPIO_PIN_E2,
    BSP_GPIO_PIN_E3,
    BSP_GPIO_PIN_E4,
    BSP_GPIO_PIN_E5,
    BSP_GPIO_PIN_E6,
    BSP_GPIO_PIN_E7,

    BSP_GPIO_PIN_F0,
    BSP_GPIO_PIN_F1,
    BSP_GPIO_PIN_F2,
    BSP_GPIO_PIN_F3,
    BSP_GPIO_PIN_F4,
    BSP_GPIO_PIN_F5,
    BSP_GPIO_PIN_F6,
    BSP_GPIO_PIN_F7,

    BSP_GPIO_PGH_REG,
    BSP_GPIO_PIN_G0 = BSP_GPIO_PGH_REG,
    BSP_GPIO_PIN_G1,
    BSP_GPIO_PIN_G2,
    BSP_GPIO_PIN_G3,
    BSP_GPIO_PIN_G4,
    BSP_GPIO_PIN_G5,
    BSP_GPIO_PIN_G6,
    BSP_GPIO_PIN_G7,

    BSP_GPIO_PIN_H0,
    BSP_GPIO_PIN_H1,
    BSP_GPIO_PIN_H2,
    BSP_GPIO_PIN_H3,
    BSP_GPIO_PIN_H4,
    BSP_GPIO_PIN_H5,
    BSP_GPIO_PIN_H6,
    BSP_GPIO_PIN_H7,
    BSP_GPIO_PIN_MAX,
    BSP_UART1_PIN,
    BSP_UART2_PIN,
    TOTAL_PIN_MAX
};

#define BSP_GPIO_BIT(pin)       (pin & ~(BSP_GPIO_2ND_REG))
#define BSP_GPIO_2BITS(pin)     ((pin & ~(BSP_GPIO_PCD_REG | BSP_GPIO_2ND_REG)) * 2)
#define BSP_GPIO_CNR_REG(pin)   ((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_CNR : BSP_PABCD_CNR)
#define BSP_GPIO_DIR_REG(pin)   ((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_DIR : BSP_PABCD_DIR)
#define BSP_GPIO_DAT_REG(pin)   ((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_DAT : BSP_PABCD_DAT)
#define BSP_GPIO_ISR_REG(pin)   ((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_ISR : BSP_PABCD_ISR)
#define BSP_GPIO_IMR_REG(pin)   ((pin & BSP_GPIO_2ND_REG) ? \
                                        (((pin & BSP_GPIO_PGH_REG) == BSP_GPIO_PGH_REG) ? BSP_PGH_IMR : BSP_PEF_IMR) : \
                                        ((pin & BSP_GPIO_PCD_REG) ? BSP_PCD_IMR : BSP_PAB_IMR))

#define LED_ON  1
#define LED_OFF 0
#define LED_BLINK 2

#define WIFI_SWITCH_BTN_PIN     __CONFIG_WIFI_BTN_PIN__ 
#define WPS_BTN_PIN     __CONFIG_WPS_BTN_PIN__  
#define RESET_BTN_PIN   __CONFIG_RESET_BTN_PIN__ 
#define SYSTEM_LED_PIN  __CONFIG_SYS_LED_PIN__
#define WPS_LED_PIN     __CONFIG_WPS_LED_PIN__  

void lan_wan_port_led_on_off(int val);
void lan_port_led_on_off(int val);
void set_lan_port_as_gpio();
void set_lan_port_as_led();
int tenda_button_status(unsigned int gpio);
int reset_pin_status(void);
int wps_pin_status(void);
int wifi_swtich_pin_status(void);
void sys_led_turn_on_off(int val);
void tenda_gpio_init(void);
void wps_led_control(int value);
#ifdef __CONFIG_EXTEND_LED__
void extend_led_on_off(int val);
#endif
//¡Ÿ ±πÿ±’
//#ifdef __CONFIG_WPS_RTK__
extern void wps_led_timer();
//#endif

#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
void set_lan_led_on_off(unsigned int val);
#if defined(__CONFIG_WAN_PORT_MASk__)
void set_wan_led_on_off(unsigned int val);
#endif
#endif
#endif
