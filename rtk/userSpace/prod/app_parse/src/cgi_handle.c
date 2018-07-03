#include <stdio.h>
#include <stdlib.h>
#include "cgi_lib.h"
#include "cjson_handle.h"


CGI_OPTION_INFO app_get_module[]=
{
    {"APP_GET_WIFI_TIMER",          		app_get_wifi_timer},
    {"APP_GET_SYS_BASIC_INFO",      		app_get_sys_basic_info},
    {"APP_GET_SYS_ADVANCE_INFO",    		app_get_sys_advance_info},
    {"APP_GET_DECODE_LOGIN_PWD",    		app_get_delogin_pwd},
    {"APP_GET_WIFI_TIMER",          		app_get_wifi_timer},
    {"APP_GET_SAFE_GRADE",          		app_get_safe_grade},
    {"APP_GET_WL_POWER",            		app_get_wl_power},
    {"APP_GET_WL_CHANNEL_GRADE",    		app_get_wl_channel_grade},
    {"APP_GET_WL_PUSH_INFO",        		app_get_wifi_push_info},
#ifdef __CONFIG_GUEST__
    {"APP_GET_WL_GUEST_INFO",       		app_get_wifi_guest_info},
#endif
    {"APP_GET_UCLOUD_BASIC_INFO",   		app_get_ucloud_basic_info},
    {"APP_GET_UCLOUD_INFO_MESSAGE_EN", 		app_get_ucloud_info_manage_en},
    {"APP_GET_UCLOUD_INFO_TRY_CLEAR", 		app_get_ucloud_info_try_clear_acc},
    {"APP_GET_OL_HOST_INFO",        		app_get_ol_host_info},
    {"APP_GET_RUB_NET_BLACK_LIST",			app_get_rub_net_black_list},
    {"APP_GET_STRANGE_INFO",				app_get_strange_host_info},
    {"APP_GET_HAND_QOSINFO",         		app_get_hand_qos_info},
    {"APP_GET_HAND_QOS_MAX_UPLIMIT",  		app_get_hand_qos_max_uplimit},
    {"APP_GET_UPGRADE_IMAGE_PATH",			app_get_upgrade_path},
    {"APP_GET_UPGRADE_MEMORY_STATE",		app_get_upgrade_memory_state},
    {"APP_GET_WIFI_BASIC",          		app_get_wifi_basic_info},
    {"APP_GET_DEV_REMARK",         			app_get_dev_remark},
    {"APP_GET_WAN_BASIC",           		app_get_wan_basic},
    {"APP_GET_WAN_RATE",            		app_get_wan_rate},
    {"APP_GET_WAN_DETECT",          		app_get_wan_detect},
	{"APP_GET_MAC_FILTER_MODE",     		app_get_mac_filter_mode},
#ifdef __CONFIG_LED__
	{"APP_GET_LED_STATUS",					app_get_led_status},
#endif
};

CGI_OPTION_INFO app_set_module[]=
{
    {"APP_SET_REBOOT",              		app_set_reboot},
    {"APP_SET_RESET",              		 	app_set_reset},
    {"APP_SET_LOGIN_PWD",           		app_set_login_pwd},
    {"APP_SET_WIFI_TIMER",          		app_set_wifi_timer},
    {"APP_SET_WL_POWER",            		app_set_wl_power},
    {"APP_SET_WL_POWER_PROCESS",    		app_set_wl_power_process},
    {"APP_SET_WL_CHANNEL_GRADE",    		app_set_wl_channel_grade},
    {"APP_SET_UCLOUD_INFO_SN",      		app_set_ucloud_info_sn},
    {"APP_SET_UCLOUD_INFO_MESSAGE_EN", 		app_set_ucloud_info_manage_en},
    {"APP_SET_UCLOUD_INFO_CLEAR_ACCOUNT", 	app_set_ucloud_info_clear_account},
    {"APP_SET_RUB_NET_ADD_BLACKLIST",		app_set_rub_net_add_blacklist},
    {"APP_SET_RUB_NET_DEL_BLACKLIST",		app_set_rub_net_delete_blacklist},
    {"APP_SET_RUB_NET_ADD_TO_TRUSTLIST",	app_set_rub_net_add_to_trustlist},
    {"APP_SET_RUB_NET_DEL_TO_TRUSTLIST",	app_set_rub_net_delete_from_trustlist},
    {"APP_SET_HAND_QOSINFO",				app_set_hand_qos_info},
    {"APP_SET_WIFI_BASIC",          		app_set_wifi_basic_info},
    {"APP_SET_WIFI_BASIC_PROCESS",  		app_set_wifi_process},
    {"APP_SET_DEV_REMARK",          		app_set_dev_remark},
    {"APP_SET_WAN_BASIC",           		app_set_wan_basic},
    {"APP_SET_WAN_BASIC_PROCESS",   		app_set_wan_basic_process},
#ifdef __CONFIG_GUEST__
    {"APP_SET_WL_GUEST_INFO",       		app_set_wifi_guest_info},
    {"APP_SET_WL_GUEST_INFO_PROCESS",       app_set_wifi_guest_info_process},
#endif
    {"APP_SET_WIZARD_SUCCEED",   			app_set_wizard_succeed},
    {"APP_SET_MAC_FILTER_MODE",   			app_set_macfilter_mode},
    {"APP_SET_RUB_NET_FLUSH_BLACKLIST",   	app_set_rub_net_flush_blacklist},
#ifdef __CONFIG_LED__
	{"APP_SET_LED_STATUS",					app_set_led_status},
#endif
};
extern int APP_DEBUG;
int app_set_module_param(cJSON *cj_query,cJSON *cj_set)
{
    PIU32 result_code = 0;
    PIU32 index = 0;
    cJSON * pItem = NULL;
    cJSON *pArray = NULL;
    int nCount = 0;
    CGI_MSG_MODULE msg_list[MAX_MSG_NUM];
    PI8 private_event[PI_BUFLEN_32]  = {0};
    char* cj_debug_string = NULL;
    int i;

    memset((char *)&msg_list, 0x0, MAX_MSG_NUM * sizeof(CGI_MSG_MODULE));
    if(NULL == cj_query || NULL == cj_set)
    {
        printf("cj_query or cj_set is NULL!!!!\n");
        return result_code;
    }
    pArray = cJSON_GetObjectItem(cj_query,"module");
    if(APP_DEBUG == 1)
    {
        printf("-------------------start--------------------------\n");
        cj_debug_string = cJSON_Print(cj_query);
    	 printf("cj_query:\n %s\n\n",cj_debug_string);
        free(cj_debug_string);
	 cj_debug_string = NULL;
	 cj_debug_string = cJSON_Print(cj_set);
    	 printf("cj_set:\n %s\n",cj_debug_string);
        free(cj_debug_string);
	 cj_debug_string = NULL;
	 printf("-------------------end--------------------------\n\n");
    }

    if(NULL == pArray)
    {
        printf("module is NULL!!!!\n");
        return result_code;
    }
    nCount = cJSON_GetArraySize (pArray);

    for(i = 0; i < nCount; i++)
    {
        pItem = cJSON_GetArrayItem(pArray,i);
         if(pItem != NULL)
        {
            for(i = 0; i<ARRAY_SIZE(app_set_module); i++)
            {
                if((pItem->valuestring != NULL) &&
                   (strcmp(pItem->valuestring,app_set_module[i].name) == 0))
                {
                      APP_SET_FUN(&app_set_module[i])(cj_set,msg_list,&result_code,NULL);
			break;
                }
               
            }
        }
    }
	/*修改commit为发送消息，保证及时返回页面*/
    msg_send(MODULE_RC,RC_SYSTOOLS_MODULE,"string_info=commit");

    for(index = 0; index < MAX_MSG_NUM; ++index)
    {
        if(msg_list[index].id != 0)
        {
            msg_send(MODULE_RC, msg_list[index].id, msg_list[index].msg);
        }
    }
    return result_code;
}

int app_get_module_param(cJSON *cj_query,cJSON *cj_get)
{
    int i,j,ret = 0;
    cJSON  *pItem = NULL;
    cJSON  *pArray = NULL;
    char* cj_debug_string = NULL;
    int nCount = 0;

    if(NULL == cj_query)
    {
        printf("cj_query is NULL!!!!\n");
        return ret;
    }
    pArray = cJSON_GetObjectItem(cj_query,"module");
    if(APP_DEBUG == 1)
    {
        printf("-------------------start--------------------------\n");
        cj_debug_string = cJSON_Print(cj_query);
    	 printf("cj_query:\n %s\n\n",cj_debug_string);
        free(cj_debug_string);
	 cj_debug_string = NULL;
    }

    if(NULL == pArray)
    {
        printf("module is NULL!!!!\n");
        return ret;
    }
    nCount = cJSON_GetArraySize (pArray);

    for(j = 0; j < nCount; j++)
    {
        pItem = cJSON_GetArrayItem(pArray,j);
        if(pItem != NULL)
        {
            for(i = 0; i<ARRAY_SIZE(app_get_module); i++)
            {
                if((pItem->valuestring != NULL) &&
                   (strcmp(pItem->valuestring,app_get_module[i].name) == 0))
                {
                    APP_GET_FUN(&app_get_module[i])(cj_query,cj_get,NULL);
                    break;
                }

            }

        }
    }
    if(APP_DEBUG && cj_get)
    {
        cj_debug_string = cJSON_Print(cj_get);
    	 printf("cj_get:\n %s\n",cj_debug_string);
        free(cj_debug_string);
		cj_debug_string = NULL;
		printf("-------------------end--------------------------\n\n");
    }
    return ret;
}



#ifdef __CONFIG_WEB_TO_APP__
void form_appGet(webs_t wp, char_t *path, char_t *query) 
{ 
	cJSON *cj_query = NULL; 
	cJSON *cj_get = NULL; 
	char *send_buf = NULL; 
	char *query_buf = NULL; 
	cJSON *data = NULL; 

	if(APP_DEBUG == 0)
	{
		return ;
	}

	cj_get = cJSON_CreateObject(); 
	if(NULL == cj_get) 
	{ 
		return ; 
	}   

	query_buf = malloc(strlen(wp->query) * sizeof(char) + 1); 
	if(NULL == query_buf) 
	{ 
		cJSON_Delete(cj_get); 
		return; 
	} 
	memset(query_buf,0x0,strlen(query_buf)+1); 
	strcpy(query_buf,wp->query); 
	data = cJSON_Parse(query_buf);

	cj_query = cJSON_GetObjectItem(data,"cj_query"); 

	if(NULL == cj_query) 
	{ 
		printf("cj_query is NULL!!\n"); 
		free(query_buf);
		cJSON_Delete(data);
		cJSON_Delete(cj_get); 
		return; 
	} 

	
	app_get_module_param(cj_query,cj_get); //调用app get接口 

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n")); 
	send_buf = cJSON_Print(cj_get); 
	if(NULL != send_buf) 
	{ 
		websWriteLongString(wp, send_buf); 
	} 

	websDone(wp, 200); 

	free(query_buf); 
	free(send_buf);
	cJSON_Delete(data);
	cJSON_Delete(cj_get); 

	return; 
}



void form_appSet(webs_t wp, char_t *path, char_t *query) 
{  
	cJSON *cj_query = NULL; 
	cJSON *data = NULL; 
	cJSON *cj_set = NULL; 
	int len = 0;
	char *buf = NULL;

	if(APP_DEBUG == 0)
	{
		return ;
	}

	len = strlen(wp->query) + 1;
	buf = malloc(len);
	if(NULL == buf)
	{
		printf("malloc buf fail\n");
		return ;
	}
	memset(buf,0x0,len);
	strncpy(buf,wp->query,len - 1);
	  
	data = cJSON_Parse(buf);
	if(NULL == data)
	{
		printf("%d line data is null\n",__LINE__);
		free(buf);
		return ;
	}

	cj_query = cJSON_GetObjectItem(data,"cj_query");
	cj_set = cJSON_GetObjectItem(data,"cj_set");

	if((NULL == cj_query) || (NULL == cj_set))
	{
		printf("%d line return\n",__LINE__);
		free(buf);
		return ;
	}

	app_set_module_param(cj_query,cj_set);
	
	free(buf);
	cJSON_Delete(data);
	
	return; 

}

#endif
