#ifndef __WAN_H__
#define __WAN_H__

#ifndef  __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef  __RC_MODULE_H__
#include "rc_module.h"
#endif

#ifndef __SYS_TIMER_H__
#include "sys_timer.h"
#endif




typedef enum
{
    WAN_MAC_DEFAULT = 0,
    WAN_MAC_PC,
    WAN_MAC_HAND,
} WAN_HWADDR_TYPE;

typedef enum
{
    WAN_SPEED_AUTO = 0,
    WAN_SPEED_10_HALF,
    WAN_SPEED_10_FULL,
    WAN_SPEED_100_HALF,
    WAN_SPEED_100_FULL,
} WAN_SPEED_TYPE;

typedef struct wan_static_info_struct
{
    PIU32 mtu;
    PI8 wan_static_ip[PI_IP_STRING_LEN];
    PI8 wan_static_mask[PI_IP_STRING_LEN];
    PI8 wan_static_gw[PI_IP_STRING_LEN];
    PI8 wan_static_dns1[PI_IP_STRING_LEN];
    PI8 wan_static_dns2[PI_IP_STRING_LEN];
} WAN_STATIC_INFO_STRUCT,*P_WAN_STATIC_INFO_STRUCT;

typedef struct wan_dhcp_info_struct
{
    PIU32 mtu;
    PI8 wan_hostname[PI_BUFLEN_32];
} WAN_DHCP_INFO_STRUCT,*P_WAN_DHCP_INFO_STRUCT;

typedef struct wan_pppoe_info_struct
{
    PIU32 mtu;
    PIU8 unit;
    PI8 wan_pppoe_username[PI_BUFLEN_128+1];
    PI8 wan_pppoe_password[PI_BUFLEN_128+1];
} WAN_PPPOE_INFO_STRUCT,*P_WAN_PPPOE_INFO_STRUCT;

typedef struct wan_err_info_struct
{
    PIU8 network_check;/*0-3表示WAN口环境检测结果,0无结果或者不检测,1STATIC,2DHCP,3PPPOE,4检测中*/
    PIU8 code;/*0-99表示错误代码*/
    PIU8 wan_mode;/*1-3表示WAN口类型,1STATIC,2DHCP,3PPPOE*/
    PIU8 wl_mode;/*0-2表示工作模式,0AP,1WISP,2APCLIENT*/
    PIU8 time_action;/*0-1表示显示IP信息,0不显示,1显示*/
    PIU8 color_action;/*1-3表示页面颜色,1错误,2尝试,3成功*/
    PIU8 button_action;/*1-2表示按钮类型,1断开,2连接*/
} WAN_ERR_INFO_STRUCT,*P_WAN_ERR_INFO_STRUCT;

typedef struct wan_hwaddr_info_struct
{
    PI8 wan_hwaddr_type;
    PI8 wan_hwaddr[PI_MAC_STRING_LEN];
    PI8 wan_default_hwaddr[PI_MAC_STRING_LEN];
} WAN_HWADDR_INFO_STRUCT,*P_WAN_HWADDR_INFO_STRUCT;

typedef struct wan_info_struct
{
    PI8 wan_proto[PI_BUFLEN_16];
    WANMODE wan_type;
    WANMODE wan_old_type;

    WANSTATUS wan_status;

    WAN_ERR_INFO_STRUCT wan_err_info;
    PIU32 wan_err_result;

    PIU8 wan_speed;

    WAN_HWADDR_INFO_STRUCT wan_hwaddr_info;

    PI8 wan_ifname[PI_BUFLEN_16];
    PI8 wan_old_ifname[PI_BUFLEN_16];

    cyg_tick_count_t connect_time;

    PI8 wan_cur_ipaddr[PI_IP_STRING_LEN];
    PI8 wan_cur_mask[PI_IP_STRING_LEN];
    PI8 wan_cur_gw[PI_IP_STRING_LEN];
    PI8 wan_cur_dns1[PI_IP_STRING_LEN];
    PI8 wan_cur_dns2[PI_IP_STRING_LEN];

    WAN_STATIC_INFO_STRUCT wan_static_info;

    WAN_DHCP_INFO_STRUCT wan_dhcp_info;

    WAN_PPPOE_INFO_STRUCT wan_pppoe_info;
} WAN_INFO_STRUCT,*P_WAN_INFO_STRUCT;

typedef struct wan_currcet_info_struct
{
    PIU32 mtu;
    PIU8 wan_speed;
} WAN_CURRCET_INFO_STRUCT,*P_WAN_CURRCET_INFO_STRUCT;

typedef enum
{
    COMMON_NONE                     = 0,
    COMMON_NO_WIRE                  = 1,
    COMMON_NOT_CONNECT              = 2,
    COMMON_CONNECTING               = 3,
    COMMON_CONNECTED_ONLINEING      = 4,
    COMMON_NOT_ONLINE               = 5,
    COMMON_ONLINEED                 = 6,
} COMMONERRCODE;

typedef enum
{
    STATIC_WL_CHECKED_PASSWORD_FAIL = (COMMON_ONLINEED + 1),
} STATICERRCODE;

typedef enum
{
    DHCP_IP_CONFLLICT               = (COMMON_ONLINEED + 1),
    DHCP_NO_RESPOND                 = (COMMON_ONLINEED + 2),
    DHCP_WL_CHECKED_PASSWORD_FAIL   = (COMMON_ONLINEED + 3),
} DHCPERRCODE;

typedef enum
{
    PPPOE_CHECKED_PASSWORD_FAIL     = (COMMON_ONLINEED + 1),
    PPPOE_NO_RESPOND                = (COMMON_ONLINEED + 2),
    PPPOE_WL_CHECKED_PASSWORD_FAIL  = (COMMON_ONLINEED + 3),
} PPPOEERRCODE;

/*API*/
RET_INFO api_wan_systools_handle(RC_MODULES_COMMON_STRUCT *var);

/*GPI*/
PIU32 gpi_wan_get_err_result_info();
P_WAN_ERR_INFO_STRUCT gpi_wan_get_err_info_other();
P_WAN_INFO_STRUCT gpi_wan_get_info();
P_WAN_HWADDR_INFO_STRUCT gpi_wan_get_hwaddr_info();
P_WAN_CURRCET_INFO_STRUCT gpi_wan_get_currcet_info(P_WAN_CURRCET_INFO_STRUCT p);

/*TPI*/
RET_INFO tpi_wan_update_info();
RET_INFO tpi_wan_struct_init();
RET_INFO tpi_wan_first_init();
RET_INFO tpi_wan_systools_action(RC_MODULES_COMMON_STRUCT *var);
MODULE_WORK_STATUS tpi_wan_get_action_type();
void tpi_wan_action_reinit();
PIU32 tpi_wan_get_err_result_info();
P_WAN_ERR_INFO_STRUCT tpi_wan_get_err_info_other();
P_WAN_INFO_STRUCT tpi_wan_get_info();
P_WAN_HWADDR_INFO_STRUCT tpi_wan_get_hwaddr_info();
P_WAN_CURRCET_INFO_STRUCT tpi_wan_get_currcet_info(P_WAN_CURRCET_INFO_STRUCT p);
RET_INFO tpi_wan_get_connect();
#endif
