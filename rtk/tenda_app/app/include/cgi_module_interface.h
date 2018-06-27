#ifndef __CGI_MODULE_INTERFACE__
#define __CGI_MODULE_INTERFACE__
#include <stdlib.h>
#include "cJSON.h"
/***********************interface function**************************/
extern int app_get_module_param(cJSON *cj_query,cJSON *cj_get);
extern int app_set_module_param(cJSON *cj_query,cJSON *cj_set);
/***************************************************************/

/***************************************************************/
extern int cjson_get_number(cJSON *root, char *name, int defaultGetValue);
extern char *cjson_get_value(cJSON *root, char *name, char *defaultGetValue);

/***************************************************************/


/**************************module name***************************/
#define GET_WIFI_TIMER 					"APP_GET_WIFI_TIMER"
#define GET_SYS_BASIC_INFO				"APP_GET_SYS_BASIC_INFO"
#define GET_SYS_ADVANCE_INFO			"APP_GET_SYS_ADVANCE_INFO"
#define GET_DECODE_LOGIN_PWD			"APP_GET_DECODE_LOGIN_PWD"
#define GET_SAFE_GRADE					"APP_GET_SAFE_GRADE"
#define GET_WL_POWER					"APP_GET_WL_POWER"
#define GET_WL_CHANNEL_GRADE			"APP_GET_WL_CHANNEL_GRADE"
#define GET_WL_PUSH_INFO        		"APP_GET_WL_PUSH_INFO"
#define GET_WL_GUEST_INFO       		"APP_GET_WL_GUEST_INFO"
#define GET_OL_HOST_INFO           	 	"APP_GET_OL_HOST_INFO"
#define GET_UCLOUD_BASIC_INFO        	"APP_GET_UCLOUD_BASIC_INFO"
#define GET_UCLOUD_INFO_MESSAGE_EN   	"APP_GET_UCLOUD_INFO_MESSAGE_EN"
#define GET_UCLOUD_INFO_TRY_CLEAR    	"APP_GET_UCLOUD_INFO_TRY_CLEAR"
#define GET_OL_HOST_INFO             	"APP_GET_OL_HOST_INFO"
#define GET_RUB_NET_BLACK_LIST       	"APP_GET_RUB_NET_BLACK_LIST"
#define GET_STRANGE_INFO             	"APP_GET_STRANGE_INFO"
#define GET_HAND_QOSINFO             	"APP_GET_HAND_QOSINFO"
#define GET_HAND_QOS_MAX_UPLIMIT     	"APP_GET_HAND_QOS_MAX_UPLIMIT"
#define GET_UPGRADE_IMAGE_PATH       	"APP_GET_UPGRADE_IMAGE_PATH"
#define GET_UPGRADE_MEMORY_STATE    	"APP_GET_UPGRADE_MEMORY_STATE"
#define GET_WIFI_BASIC          		"APP_GET_WIFI_BASIC"
#define GET_DEV_REMARK          		"APP_GET_DEV_REMARK"
#define GET_WAN_BASIC           		"APP_GET_WAN_BASIC"
#define GET_WAN_RATE            		"APP_GET_WAN_RATE"
#define GET_WAN_DETECT          		"APP_GET_WAN_DETECT"
#define GET_MAC_FILTER_MODE				"APP_GET_MAC_FILTER_MODE"
#define GET_LED							"APP_GET_LED_STATUS"

#define SET_WIFI_TIMER					"APP_SET_WIFI_TIMER"
#define SET_RESET						"APP_SET_RESET"
#define SET_REBOOT						"APP_SET_REBOOT"
#define SET_LOGIN_PWD					"APP_SET_LOGIN_PWD"
#define SET_WIZARD_SUCCEED 				"APP_SET_WIZARD_SUCCEED"
#define SET_WL_POWER_PROCESS			"APP_SET_WL_POWER_PROCESS"
#define SET_WL_POWER					"APP_SET_WL_POWER"
#define SET_WL_CHANNEL_GRADE			"APP_SET_WL_CHANNEL_GRADE"
#define SET_UCLOUD_INFO_SN              "APP_SET_UCLOUD_INFO_SN"
#define SET_UCLOUD_INFO_MESSAGE_EN      "APP_SET_UCLOUD_INFO_MESSAGE_EN"
#define SET_UCLOUD_INFO_CLEAR_ACCOUNT   "APP_SET_UCLOUD_INFO_CLEAR_ACCOUNT"
#define SET_RUB_NET_ADD_BLACKLIST       "APP_SET_RUB_NET_ADD_BLACKLIST"
#define SET_RUB_NET_DEL_BLACKLIST       "APP_SET_RUB_NET_DEL_BLACKLIST"
#define SET_RUB_NET_ADD_TO_TRUSTLIST    "APP_SET_RUB_NET_ADD_TO_TRUSTLIST"
#define SET_RUB_NET_DEL_TO_TRUSTLIST    "APP_SET_RUB_NET_DEL_TO_TRUSTLIST"
#define SET_HAND_QOSINFO                "APP_SET_HAND_QOSINFO"
#define SET_WIFI_BASIC          		"APP_SET_WIFI_BASIC"
#define SET_WIFI_BASIC_PROCESS   		"APP_SET_WIFI_BASIC_PROCESS"
#define SET_WL_GUEST_INFO      			"APP_SET_WL_GUEST_INFO"
#define SER_WL_GUEST_INFO_PROCESS 		"APP_SET_WL_GUEST_INFO_PROCESS"
#define SET_DEV_REMARK          		"APP_SET_DEV_REMARK"
#define SET_WAN_BASIC           		"APP_SET_WAN_BASIC"
#define SET_WAN_BASIC_PROCESS    		"APP_SET_WAN_BASIC_PROCESS"
#define SET_MAC_FILTER_MODE				"APP_SET_MAC_FILTER_MODE"
#define SET_RUB_NET_FLUSH_BLACKLIST 	"APP_SET_RUB_NET_FLUSH_BLACKLIST"
#define SET_LED							"APP_SET_LED_STATUS"
/***************************************************************/

/**************************module param**************************/
		
/***************************************************************/

#endif
