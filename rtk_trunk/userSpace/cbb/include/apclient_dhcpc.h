#ifndef __APCLIENT_DHCPC_H__
#define __APCLIENT_DHCPC_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

#ifdef __CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__
/*CONNECTED_COUNT_MAX更改为1由王工决策，认为链路连接上了就可以开启DHCP服务器获取IP，没必要检测那么多次数；
  DISCONNECTED_COUNT_MAX更改为3是由于如果上级是ECOS系统的话，上级更改LAN口IP，时间大概在10秒钟，那么就可能会导致一直不会重连*/
#define CONNECTED_COUNT_MAX			1
#define CONNECTED_COUNT_MIN			1
#define DISCONNECTED_COUNT_MAX 		3
#define APCLINET_DHCPC_SLEEP		(2 * RC_MODULE_1S_TIME)
#else
#define CONNECTED_COUNT_MAX			1
#define CONNECTED_COUNT_MIN			1
#define DISCONNECTED_COUNT_MAX 		3
#define APCLINET_DHCPC_SLEEP		(2 * RC_MODULE_1S_TIME)
#endif

#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
#define MAX_STA_NUM						64	
#define ETHER_ADDR_LEN         			6

struct apclient_client_info
{
	char dev_name[255];
	struct in_addr client_ip;
	unsigned char mac[ETHER_ADDR_LEN];
	struct apclient_client_info* next;
};

enum client_action
{
	ADD_CLIENT = 1, 
	REMOVE_CLIENT,
	UPDATE_CLIENT,
	FLUSH_CLIENT,
};
#endif
typedef struct apclient_dhcpc_info_struct
{
	PIU8 enable;
	PI8 lan_ifnames[PI_IP_STRING_LEN];
	PI8 ipaddr[PI_IP_STRING_LEN];
	PI8 mask[PI_IP_STRING_LEN];
	PI8 gateway[PI_IP_STRING_LEN];
	WANSTATUS now_status;
	WANSTATUS before_status;
	PIU32 connected_count;
	PIU32 disconnected_count;
#ifdef __CONFIG_A9__
	PIU8 ping_enable;
#endif
} APCLIENT_DHCPC_INFO_STRUCT,*P_APCLIENT_DHCPC_INFO_STRUCT;

#ifdef __CONFIG_A9__
#define APCLIENT_DHCPC_PING_TIME	100 /*0.1s*/
#define APCLIENT_DHCPC_PING_NO_CLINET_NUM	15
#define APCLIENT_DHCPC_PING_HAS_CLINET_NUM	30
#define APCLIENT_DHCPC_WEB_WAIT_WRITE_IP_TIME (15 * RC_MODULE_1S_TIME)/*表示DHCPC_ETH0获取到上级给的IP之后配置到ETH0接口上最大等待页面返回时间*/
typedef struct apclient_dhcpc_web_wait_struct
{
	PIU8 apclient_dhcpc_web_set_wait;/*页面重新扩展的时候，等待页面获取扩展成功，1表示等待，0表示不等待*/
	PIU8 apclient_dhcpc_web_get_return;/*页面重新扩展的时候，页面是否获取扩展成功标志，1表示成功，0表示不成功*/
} APCLIENT_DHCPC_WEB_WAIT_STRUCT,*P_APCLIENT_DHCPC_WEB_WAIT_STRUCT;
#endif
/*API*/

/*GPI*/
extern PI8 gpi_apclient_dhcpc_addr(PI8 *ip,PI8 *mask);
extern PI8 gpi_apclient_dhcpc_ping_gateway();
extern PI8 gpi_apclient_dhcpc_enable();
extern PI8 gpi_apclient_dhcpc_enable_by_mib();
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
RET_INFO gpi_apclient_dhcpc_client_action(void* data,int action);
struct apclient_client_info* gpi_apclient_dhcpc_get_client_info(unsigned char* mac);
#endif
/*TPI*/
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
RET_INFO tpi_apclient_dhcpc_remove_client(unsigned char* mac);
RET_INFO tpi_apclient_dhcpc_add_client(struct apclient_client_info client);
RET_INFO tpi_apclient_dhcpc_update_client_info();
RET_INFO tpi_apclient_dhcpc_flush_client_info();
struct apclient_client_info* tpi_apclient_dhcpc_get_client_info(unsigned char* mac);
#endif
extern RET_INFO tpi_apclient_dhcpc_update_info();
extern RET_INFO tpi_apclient_dhcpc_struct_init();
extern RET_INFO tpi_apclient_dhcpc_first_init();
extern RET_INFO tpi_apclient_dhcpc_action(RC_MODULES_COMMON_STRUCT *var);

extern PI8 tpi_apclient_dhcpc_get_enable();
extern RET_INFO tpi_apclient_dhcpc_get_ip(PI8 *ip,PI8 *mask);

extern RET_INFO tpi_apclient_dhcpc_set_ip(PI8 *ip,PI8 *mask,PI8 *gateway);
extern RET_INFO tpi_apclient_dhcpc_lan_dhcp_action_handle(MODULE_COMMON_OP_TYPE action);
#endif/*__APCLIENT_DHCPC_H__*/
