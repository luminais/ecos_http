#ifndef SYS_INIT_H
#define SYS_INIT_H

#include <time.h>
#include <cyg/kernel/kapi.h>

#define NULL_FILE 0
#define NULL_STR ""

/*for SYS INIT*/
#define BIT(x)  (1UL<<(x))
#define RESET_EVENT BIT(0)
#define RELOAD_DEFAULT_EVENT BIT(1)
#define WAN_EVENT    BIT(2)
#define LAN_EVENT     BIT(3)
#define REINIT_EVENT  BIT(4)
#define FIREWARE_EVENT BIT(5)
#define QOS_EVENT           BIT(6)
#define DHCP_EVENT         BIT(7)
#define DNS_EVENT           BIT(8)
#define SNTP_EVENT         BIT(9)
#define DDNS_EVENT         BIT(10)
#define WLAN_APP_EVENT BIT(11)
#define ROUTE_EVENT        BIT(12)
#define WLAN_BRIDGE_EVENT         BIT(13)
#define WLAN_REINIT_EVENT	BIT(14)
#define WLAN_IAPP_EVENT	BIT(15)
#define WAN_PPP_DISCONNECT_EVENT	BIT(16)
#define WAN_PPP_DIAL_EVENT	BIT(17)
#define WAN_PPP_DEMAND_DIAL_EVENT	BIT(17)
#define WAN_PPP_LCP_TERMINATE_EVENT BIT(18)
#define INDICATE_WISP_STATUS_EVENT BIT(19)
#define SYS_LOG_EVENT BIT(20)
#define WAN_LINK_UP_EVENT BIT(21)
#define WAN_LINK_DOWN_EVENT BIT(22)
#define AUTODHCP_EVENT BIT(23)
#define START_DHCP_PPP_EVENT BIT(24)
#define REINIT_SYSTEM_EVENT  BIT(25)
#define BRIDGE_WLAN_CONNECT  BIT(26)


/*eth(6) wlan(2*6)*/
#define MAX_BRIDGE_INTF_COUNT 20


/*sys Reinit*/
#define SYS_WAN_M BIT(0)
#define SYS_LAN_M  BIT(1)
#define SYS_WIFI_M BIT(2)
#define SYS_BRIDGE_M BIT(3)
#define SYS_NAPT_M BIT(4)
#define SYS_MISC_M BIT(5)
#define SYS_RIP_M BIT(6)
#define SYS_WIFI_APP_M BIT(7)
#define SYS_TR069_M BIT(8)
#define SYS_DOS_M BIT(9)
#ifdef CONFIG_RTL_DHCP_PPPOE
#define SYS_WAN_PPPOE_M BIT(10)
#endif
#ifdef CONFIG_ECOS_AP_SUPPORT
#define SYS_AP_LAN_M BIT(11) //when AP lan connect should run ntp
#endif
#define SYS_REINIT_ALL BIT(31)


#define SYS_ALL_M    (SYS_WAN_M | SYS_LAN_M | SYS_WIFI_M | SYS_BRIDGE_M \
	| SYS_NAPT_M | SYS_MISC_M | SYS_RIP_M | SYS_WIFI_APP_M | SYS_TR069_M)

unsigned long get_reinit_flag();
unsigned long clear_reinit_flag(unsigned long value);
unsigned long set_reinit_flag(unsigned long value);
void  sys_reinit_main(unsigned int flag);
unsigned long get_skip_ppp_disconnect_flag(void);
unsigned long set_skip_ppp_disconnect_flag(unsigned long value);
unsigned long clear_skip_ppp_disconnect_flag(void);
void kick_event(unsigned int event);
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_GPIO_APIV2)
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
#endif
#endif /* SYS_INIT_H */


