#ifndef __DHCP_SERVER_H__
#define __DHCP_SERVER_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

#define DHCPD_STATIC_LEASE_NU 19
#define PI_REMARK_STRING_LEN (64 + 1)			/* 最大主机备注长度 */

/* 静态IP配置参数 */
typedef struct static_ip_list
{
    PI8 ip_addr[PI_IP_STRING_LEN];
    PI8 mac_addr[PI_MAC_STRING_LEN];
	PI8 remark[PI_REMARK_STRING_LEN];
    struct static_ip_list *next;
}STATIC_IP_LIST, *P_STATIC_IP_LIST;

/* DHCP服务器配置参数 */
 typedef struct dhcp_server_info_struct{
	PIU8 enable;                      	 			/*开关*/
	PI8 ifname[PI_BUFLEN_16];          			/*绑定接口,例如br0,br1*/
	PIU32 lease_time;							 	/*租约*/

	SYS_WORK_MODE wl_mode;								/*无线工作模式*/

	PI8 lan_ip[PI_IP_STRING_LEN];					/*lan口IP*/
	PI8 lan_mask[PI_IP_STRING_LEN];				/*lan 口子网掩码*/
	PI8 start_ip[PI_IP_STRING_LEN];			 	/*开始IP*/
	PI8 end_ip[PI_IP_STRING_LEN];					/*结束IP*/
	PI8 gateway[PI_IP_STRING_LEN];					/*默认网关*/
	PI8 dns[3*PI_IP_STRING_LEN];						/*DNS*/

	/*静态IP绑定*/
	P_STATIC_IP_LIST static_ip_bind_info;
}DHCP_SERVER_INFO_STRUCT,*P_DHCP_SERVER_INFO_STRUCT; 

/*API*/

/*GPI*/

/*TPI*/
extern RET_INFO tpi_dhcp_server_update_info();
extern RET_INFO tpi_dhcp_server_struct_init();
extern RET_INFO tpi_dhcp_server_first_init();
extern RET_INFO tpi_dhcp_server_action(RC_MODULES_COMMON_STRUCT *var);
#ifdef __CONFIG_TC__
extern RET_INFO tpi_tc_dhcp_server_action(RC_MODULES_COMMON_STRUCT *var);
#endif

extern RET_INFO tpi_static_ip_update_info();
extern P_STATIC_IP_LIST tpi_static_ip_get_info();

P_DHCP_SERVER_INFO_STRUCT tpi_dhcp_server_get_info();

#endif/*__DHCP_SERVER_H__*/
