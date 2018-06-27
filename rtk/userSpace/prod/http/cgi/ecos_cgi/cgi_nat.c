#include "cgi_lib.h"

RET_INFO cgi_nat_staticip_get(webs_t wp, cJSON *root, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO get_info;
    PIU8 modules[] =
    {
        MODULE_GET_NAT_STATICIP_LIST,
    };
    cJSON_AddItemToObject(root, "staticIPList", obj = cJSON_CreateArray());
    get_info.wp = wp;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info,NULL);
    return RET_SUC;
}


RET_INFO cgi_nat_staticip_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
    {
        MODULE_SET_NAT_STATICIP,
    };

    set_info.wp = wp;
    set_info.root = NULL;
    set_info.modules = modules;
    set_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_set(set_info,msg,err_code,NULL);
    return(RET_SUC);
}


RET_INFO cgi_nat_portForward_get(webs_t wp, cJSON *root, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO get_info;
    PIU8 modules[] =
    {
        MODULE_GET_NAT_PORT_FORWARD,
    };
    cJSON_AddItemToObject(root, "portList", obj = cJSON_CreateArray());

    get_info.wp = wp;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info,NULL);

    return RET_SUC;
}

RET_INFO cgi_nat_portForward_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
    {
        MODULE_SET_NAT_PORT_FORWARD,
    };

    set_info.wp = wp;
    set_info.root = NULL;
    set_info.modules = modules;
    set_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_set(set_info,msg,err_code,NULL);
    return(RET_SUC);
}

RET_INFO cgi_nat_ddns_get(webs_t wp, cJSON *root, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO get_info;
    PIU8 modules[] =
    {
        MODULE_GET_NAT_DDNS,
    };
    cJSON_AddItemToObject(root, "ddns", obj = cJSON_CreateObject());

    get_info.wp = wp;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info,NULL);

    return RET_SUC;
}

RET_INFO cgi_nat_ddns_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{

    cJSON *obj = NULL;
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
    {
        MODULE_SET_NAT_DDNS,
    };

    set_info.wp = wp;
    set_info.root = NULL;
    set_info.modules = modules;
    set_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_set(set_info,msg,err_code,NULL);
    return(RET_SUC);
}

RET_INFO cgi_nat_dmz_get(webs_t wp, cJSON *root, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO get_info;
    PIU8 modules[] =
    {
        MODULE_GET_NAT_DMZ,
    };
    cJSON_AddItemToObject(root, "dmz", obj = cJSON_CreateObject());

    get_info.wp = wp;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info,NULL);

    return RET_SUC;
}

RET_INFO cgi_nat_dmz_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
    {
        MODULE_SET_NAT_DMZ,
    };

    set_info.wp = wp;
    set_info.root = NULL;
    set_info.modules = modules;
    set_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_set(set_info,msg,err_code,NULL);
    return(RET_SUC);
}


RET_INFO cgi_nat_upnp_get(webs_t wp, cJSON *root, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO get_info;
    PIU8 modules[] =
    {
        MODULE_GET_NAT_UPNP,
    };
    cJSON_AddItemToObject(root, "upnp", obj = cJSON_CreateObject());

    get_info.wp = wp;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info,NULL);

    return RET_SUC;
}
/*******************WAN ¿Ú·Àping¹¦ÄÜ********************/
RET_INFO cgi_nat_ping_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{
	char_t *pingEn;
	int firewall_restart = 0;
	char_t old_ping_enable[32] = {0};

	strcpy(old_ping_enable ,nvram_safe_get(_PING_WAN_EN));
	
	pingEn = websGetVar(wp, "pingEn", "true");

	if (0 == strcmp(pingEn, "true"))
	{
		pingEn = "0";
	}	
	else
	{
		pingEn = "1";
	}

	if(0 != strcmp(pingEn, old_ping_enable))
	{

		firewall_restart = 1;
		_SET_VALUE(_PING_WAN_EN, pingEn);
	}


	if(firewall_restart)
	{	
		printf("firewall restart.\n");
		int i;
		for(i = 0; i < MAX_MSG_NUM; i++)
		{
			if(msg[i].id == 0)
			{
				msg[i].id = RC_FIREWALL_MODULE;
				sprintf(msg[i].msg, "op=%d", OP_RESTART);
				break;
			}
			else if(msg[i].id == RC_FIREWALL_MODULE)
			{
				break;
			}
		}
	}
	
	if(!err_code[0])
	{
		strcpy(err_code, "0");
	}

	return RET_SUC;
}

RET_INFO cgi_nat_ping_get(webs_t wp, cJSON *root, void *info)
{
	char *value = NULL;
	cJSON *obj = NULL;

	cJSON_AddItemToObject(root, "ping", obj = cJSON_CreateObject());
	value =  nvram_safe_get(_PING_WAN_EN);
	if (0 == strcmp(value, "0"))
		cJSON_AddStringToObject(obj, "pingEn", "true");
	else
		cJSON_AddStringToObject(obj, "pingEn", "false");

	return RET_SUC;
}




RET_INFO cgi_nat_upnp_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
    {
        MODULE_SET_NAT_UPNP,
    };

    set_info.wp = wp;
    set_info.root = NULL;
    set_info.modules = modules;
    set_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_set(set_info,msg,err_code,NULL);
    return(RET_SUC);
}


#ifdef __CONFIG_IPTV__
RET_INFO cgi_nat_iptv_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void * info)
{
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
    {
        MODULE_SET_NAT_IPTV,
    };

    set_info.wp = wp;
    set_info.root = NULL;
    set_info.modules = modules;
    set_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_set(set_info,msg,err_code,NULL);

	return(RET_SUC);
}


RET_INFO cgi_nat_iptv_get(webs_t wp, cJSON *root, void * info)
{

	cJSON *obj = NULL;
    CGI_LIB_INFO get_info;
    PIU8 modules[] =
    {
        MODULE_GET_NAT_IPTV,
    };
    cJSON_AddItemToObject(root, "IPTV", obj = cJSON_CreateObject());

    get_info.wp = wp;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info,NULL);

	return(RET_SUC);
}


#endif
RET_INFO cgi_nat_elink_get(webs_t wp, cJSON *root, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO get_info;
    PIU8 modules[] =
    {
        MODULE_GET_NAT_ELINK,
    };
    cJSON_AddItemToObject(root, "elink", obj = cJSON_CreateObject());

    get_info.wp = wp;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info,NULL);

    return RET_SUC;
}

RET_INFO cgi_nat_elink_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
    {
        MODULE_SET_NAT_ELINK,
    };

    printf("%s [%d]\n", __FUNCTION__, __LINE__);

    set_info.wp = wp;
    set_info.root = NULL;
    set_info.modules = modules;
    set_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_set(set_info,msg,err_code,NULL);
    return(RET_SUC);
}