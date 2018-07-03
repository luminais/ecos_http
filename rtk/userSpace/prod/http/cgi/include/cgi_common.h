#ifndef __CGI_COMMON_H__
#define __CGI_COMMON_H__

#include "cJSON.h"

#include "pi_common.h"

#include "sys_module.h"

#include "rc_module.h"

#include "sys_option.h"

#include "msg.h"

#define MAX_MODULE_NUM 16
#define MAX_MSG_NUM MAX_MODULE_NUM
#define MAX_MODULE_MSG_MAX_LEN MAX_MBOX_MSG_LEN

typedef enum
{
	CGI_NONE = 0,
	CGI_GET = 1,
	CGI_SET = 2,
}CGI_TYPE;

typedef struct cgi_msg_module{
	PIU8 id;
	PI8 msg[MAX_MODULE_MSG_MAX_LEN];
}CGI_MSG_MODULE;

typedef struct cgi_module_info{
	PI8 *name;
	CGI_TYPE type;	
	union
	{
		RET_INFO (*get_fun)(webs_t wp, cJSON *root, void *info);
		RET_INFO (*set_fun)(webs_t wp, CGI_MSG_MODULE *msg, PI8 *err_code,void *info);
	} action;
	
}CGI_MODULE_INFO;

#define CGI_GET_FUN(node) (((node)->action).get_fun)
#define CGI_SET_FUN(node) (((node)->action).set_fun)

#define FREE_P(p) if(*p) \
    { \
        free(*p); \
        *p=NULL; \
    }


/* function declaration */
extern void add_msg_to_list(CGI_MSG_MODULE *msg_list, CGI_MSG_MODULE *msg);
extern void remove_msg_to_list(CGI_MSG_MODULE *msg_list, CGI_MSG_MODULE msg);
//extern void cgi_get_dispatch(webs_t wp, CGI_MODULE_INFO *modules, int modules_len, void *params);
//extern void cgi_set_dispatch(webs_t wp, CGI_MODULE_INFO *modules, int modules_len, void *params);

void formGet(webs_t wp, char_t *path, char_t *query);
void formSet(webs_t wp, char_t *path, char_t *query);

#ifdef __CONFIG_WEB_TO_APP__
extern void form_appGet(webs_t wp, char_t *path, char_t *query);
extern void form_appSet(webs_t wp, char_t *path, char_t *query);
#endif

/**************************************CGI SET PRIVATE MODULS***************************************/
RET_INFO cgi_set_wan(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
RET_INFO cgi_wizard_set_succeed(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
/**************************************************************************************************/

/**************************************CGI SET PUBLIC MODULS***************************************/
extern RET_INFO cgi_accessParam_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info);
extern RET_INFO cgi_syamanage_lan_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info);
extern RET_INFO cgi_wifiBasic_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info);
extern RET_INFO cgi_wifiPower_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code,void *info);
extern RET_INFO cgi_wifiSched_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info);
#ifdef __CONFIG_WPS_RTK__
extern RET_INFO cgi_wifiWps_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code,void *info);
#endif
extern RET_INFO cgi_wifiParam_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info);
extern RET_INFO cgi_wifiRelay_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info);
extern RET_INFO cgi_wizard_systime_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info);
extern RET_INFO cgi_sysmanage_loginpwd_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info);
extern RET_INFO cgi_sysmanage_wan_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
extern RET_INFO cgi_sysmanage_operate_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void * info);
extern RET_INFO cgi_sysmanage_fireware_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void * info);
extern RET_INFO cgi_sysmanage_systime_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info);
extern RET_INFO cgi_sysmanage_remoteWeb_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info);
extern RET_INFO cgi_nat_staticip_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
extern RET_INFO cgi_nat_portForward_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
extern RET_INFO cgi_nat_ddns_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
extern RET_INFO cgi_nat_dmz_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
extern RET_INFO cgi_nat_upnp_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
extern RET_INFO  cgi_macfilter_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info);
extern RET_INFO  cgi_tc_set_qoslist(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info);
extern RET_INFO cgi_parent_set_onlineList(webs_t wp, CGI_MSG_MODULE *msg, char *err_code,void *info);
extern RET_INFO cgi_parent_set_parentAccessCtrl(webs_t wp, CGI_MSG_MODULE *msg, char *err_code,void *info);
extern RET_INFO cgi_nat_ping_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
#ifdef __CONFIG_IPTV__
extern RET_INFO cgi_nat_iptv_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void * info);
#endif
#ifdef __CONFIG_LED__
extern RET_INFO cgi_power_save_led_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void * info);
#endif

/*******************************************************************************************/

/**************************************CGI GET PUBLIC MODULS***************************************/
extern RET_INFO cgi_accessParam_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_system_get_internet_status(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_sysmanage_lan_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_sysmanage_wan_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_wifiRelay_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_wifiEn_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_wifiBasic_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_wifiPower_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_wifiSched_get(webs_t wp, cJSON *root, void *info);

#ifdef __CONFIG_WPS_RTK__
extern RET_INFO cgi_wifiWps_get(webs_t wp, cJSON *root, void *info);
#endif

extern RET_INFO cgi_wps_hasmodule(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_wifiParam_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_wifiScanresault(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_wanDetection_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_isWifiClient_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_sysmanage_loginpwd_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_sysmanage_fireware_get(webs_t wp, cJSON *root, void * info);
extern RET_INFO cgi_sysmanage_systime_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_sysmanage_remoteWeb_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_system_get_system_info(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_nat_staticip_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_nat_portForward_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_nat_ddns_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_nat_dmz_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_nat_upnp_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_get_macfilter_list(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_tc_get_online_list(webs_t wp, cJSON *root, void *info);
#ifdef __CONFIG_GUEST__
extern RET_INFO cgi_guest_get_online_list(webs_t wp, cJSON *root, void *info);
#endif
extern RET_INFO cgi_system_get_device_statistics(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_get_localhost(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_parent_get_online_list(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_parent_get_parentAccessCtrl(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_nat_ping_get(webs_t wp, cJSON *root, void *info);
#ifdef __CONFIG_IPTV__
extern RET_INFO cgi_nat_iptv_get(webs_t wp, cJSON *root, void * info);
#endif
#ifdef __CONFIG_LED__
extern RET_INFO cgi_power_save_led_get(webs_t wp, cJSON *root, void * info);
#endif

/*******************************************************************************************/

#ifdef __CONFIG_TENDA_APP__
/*在线升级相关*/
extern RET_INFO cgi_notNow_upgrade_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info);/*暂不升级*/
extern RET_INFO cgi_ucloud_version_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_ucloud_upgrade_get(webs_t wp, cJSON *root, void *info);
#endif

#ifdef __CONFIG_AUTO_CONN_CLIENT__
extern RET_INFO cgi_auto_sync_info_get(webs_t wp, cJSON *root, void *info);
#endif

#ifdef __CONFIG_GUEST__
extern RET_INFO cgi_wifi_guest_info_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_wifi_guest_info_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info);
#endif

#ifdef __CONFIG_WL_BEAMFORMING_EN__
extern RET_INFO cgi_beaforming_enable_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_beaforming_enable_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info);
#endif
extern RET_INFO cgi_nat_elink_get(webs_t wp, cJSON *root, void *info);
extern RET_INFO cgi_nat_elink_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info);
#endif/*__CGI_COMMON_H__*/
