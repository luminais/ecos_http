#ifndef __FIREWALL_H__
#define __FIREWALL_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

/*子模块配置参数*/
#define FW_HOST_FILTER_MAX_NUM 10
struct hostfilter_info_struct
{
    PI8_P       filter_client_mode;
    PI8_P       filter_client_cur_nu;
    PI8_P       filter_client[FW_HOST_FILTER_MAX_NUM];
};

#define FW_MAC_FILTER_MAX_NUM 20
struct macfilter_info_struct
{
    PI8_P       filter_mac_mode;
    PI8_P       filter_mac_cur_nu;
    PI8_P       filter_mac[FW_MAC_FILTER_MAX_NUM];

};

#define FW_URL_FILTER_MAX_NUM 10
struct urlfilter_info_struct
{
    PI8_P       filter_url_mode;
    PI8_P       filter_url_cur_nu;
    PI8_P       filter_url[FW_URL_FILTER_MAX_NUM];

};

#define FW_PORTFORWAD_MAX_NUM 16
struct portforwad_info_struct
{
    PI8_P       forward_port_list;
    PI8_P       forward_port[FW_PORTFORWAD_MAX_NUM];
};

struct dmz_info_struct
{
    PI8_P       dmz_ipaddr_en;
    PI8_P       dmz_ipaddr;
};

/* FIREWALL 配置参数 */
typedef struct firewall_info_struct
{
    PIU8                    				enable;     /*开关*/
    struct hostfilter_info_struct           hostf_info;
    struct macfilter_info_struct            macf_info;
    struct urlfilter_info_struct            urlf_info;
    struct portforwad_info_struct           portfd_info;
    struct dmz_info_struct                  dmz_info;

} FIREWALL_INFO_STRUCT,*P_FIREWALL_INFO_STRUCT;

/*API*/
extern RET_INFO api_lan_firewall_handle(RC_MODULES_COMMON_STRUCT *var);

/*GPI*/

/*TPI*/
extern RET_INFO tpi_firewall_update_info();
extern RET_INFO tpi_firewall_struct_init();
extern RET_INFO tpi_firewall_first_init();
extern RET_INFO tpi_firewall_action(RC_MODULES_COMMON_STRUCT *var);
#ifdef __CONFIG_TC__
extern RET_INFO tpi_tc_firewall_action(RC_MODULES_COMMON_STRUCT *var);
#endif
extern RET_INFO tpi_filter_restart();
extern P_FIREWALL_INFO_STRUCT tpi_firewall_get_info();

#if defined(__CONFIG_PPTP__) || defined(__CONFIG_L2TP__) || defined(__CONFIG_PPPOE2__)
extern void tpi_firewall_update_tunnel(PI8 *pppname);
#endif
#endif/*__FIREWALL_H__*/
