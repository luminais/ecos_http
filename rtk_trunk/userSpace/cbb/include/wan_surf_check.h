#ifndef __WAN_SURF_CHECK_H_
#define __WAN_SURF_CHECK_H_

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

#define ONLINE_CONFIRM_CHECKCOUNT		2
#define NO_ONLINE_CONFIRM_CHECKCOUNT	3
#define PING_DELAY	      				3000

#define NVRAMNAME_NTP_SERVER		"ntp_server"

#define GETHOSTBYNAME_DONAME_EN	0 /*add by lqz for not gethostbyname,only ping ntp services*/

#if GETHOSTBYNAME_DONAME_EN
#define	PING_GET_HOSTNAME_NUM			2
#define	NVRAMNAME_WL_COUNTRYCODE	"wl0_country_code"
#define	PING_SERVER_IP_NUM				4
#else
#define PING_GET_HOSTNAME_NUM			0
#define	PING_SERVER_IP_NUM				8
#endif

typedef struct wan_surf_check_info_struct
{
	PIU8 enable;
	PIU8 init_value;
	INTERNET_INFO check_result;
	PIU8 exit_ping_tag;
	
	PI8 ping_ok_ip[PI_IP_STRING_LEN];
	PI8 ping_server_ip[PING_SERVER_IP_NUM][PI_IP_STRING_LEN];
#if GETHOSTBYNAME_DONAME_EN	
	PI8 ping_hostname[PING_GET_HOSTNAME_NUM][PI_BUFLEN_32];	
#endif
}WAN_SURF_CHECK_INFO_STRUCT,*P_WAN_SURF_CHECK_INFO_STRUCT;

extern RET_INFO tpi_wan_surf_check_update_info();
extern RET_INFO tpi_wan_surf_check_struct_init();
extern RET_INFO tpi_wan_surf_check_first_init();
extern RET_INFO tpi_wan_surf_check_action(RC_MODULES_COMMON_STRUCT *var);

RET_INFO tpi_wan_surf_check_set_result(INTERNET_INFO flag);
extern RET_INFO tpi_wan_surf_check_ping_connet_check();
extern INTERNET_INFO tpi_wan_surf_check_result();
extern PIU8 tpi_wan_surf_check_get_exit_ping();
extern RET_INFO tpi_wan_surf_check_set_exit_ping(PIU8 enable);
extern RET_INFO tpi_wan_surf_check_wait_exit_ping();

#if GETHOSTBYNAME_DONAME_EN
extern int get_dns_quit_exit();
extern void set_dns_quit_exit_tag(int enable);

static RET_INFO tpi_wan_surf_check_check_addr(PIU8 ip0,PIU8 ip1);
static PI8 * tpi_wan_surf_check_gethostbyname(char *str);
#endif

static RET_INFO tpi_wan_surfing_check_main();
static RET_INFO tpi_wan_surf_check_start();
static RET_INFO tpi_wan_surf_check_stop();
static RET_INFO tpi_wan_surf_check_restart();

static RET_INFO api_wan_surf_check_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_wan_surf_check_init();

extern INTERNET_INFO gpi_wan_surf_check_result();
#endif/*__WAN_SURF_CHECK_H_*/
