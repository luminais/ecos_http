/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_main.c
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.1.27
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.1.27   1.0          new
************************************************************/
#include "tai_main.h"
#include "tai_hash.h"
#include <bcmnvram.h>
#include "flash_cgi.h"
#include "alinkgw_api.h"

#define STACK_SIZE 65535 

int connect_alinkgw_ok = 0 ;

static cyg_handle_t tai_daemon_handle;
static char tai_daemon_stack[STACK_SIZE];
static cyg_thread tai_daemon_thread;
static int tai_running = 0;

extern int gWifiSchedDebug ;
extern int gProbeInfoDebug ;
extern int gTpskDebug ;
extern int gProbed_switch;
extern int gAttack_switch;

struct alinkgw_attr_s{
	char *name;
	ALINKGW_ATTRIBUTE_TYPE_E type;
	ALINKGW_ATTRIBUTE_get_cb get;
	ALINKGW_ATTRIBUTE_set_cb set;	
};

struct alinkgw_subattr_s{
	char *name;
	ALINKGW_ATTRIBUTE_TYPE_E type;
	ALINKGW_ATTRIBUTE_subdevice_get_cb get;
	ALINKGW_ATTRIBUTE_subdevice_set_cb set;	
};

struct alinkgw_serv_s{
	char *name;
	ALINKGW_SERVICE_execute_cb cb;
};

#define ALINKGW_ATTRIBUTE_REGISTER(att, atttype, get, set) \
do{ \
	if(ALINKGW_register_attribute(att,atttype, get, set) != ALINKGW_OK) \
	{ \
		printf("ALINKGW Register attribute %s failed.\n", att); \
	} \
}while(0)
	
#define ALINKGW_SUBATTRIBUTE_REGISTER(att, atttype, get, set) \
do{ \
	if(ALINKGW_register_attribute_subdevice(att,atttype, get, set) != ALINKGW_OK) \
	{ \
		printf("ALINKGW Register attribute %s failed.\n", att); \
	} \
}while(0)
	
#define ALINKGW_SERVICE_REGISTER(service, cb) \
do{ \
	if(ALINKGW_register_service(service, cb) != ALINKGW_OK) \
	{ \
	  printf("ALINKGW Register service:%s failed.\n", service);\
	} \
}while(0)

struct alinkgw_attr_s attr_set[] = {
	{
		ALINKGW_ATTR_WLAN_SWITCH_STATE, ALINKGW_ATTRIBUTE_simple,  
		get_wlan_switch_state, set_wlan_switch_state
	},
	{
		//阿里2.0已取消，此处兼容阿里1.0
		ALINKGW_ATTR_WLAN_SWITCH_SCHEDULER, ALINKGW_ATTRIBUTE_complex,	
		get_wlan_switch_scheduler, set_wlan_switch_scheduler
	},	
	{
		ALINKGW_ATTR_ALI_SECURITY, ALINKGW_ATTRIBUTE_complex, 
		get_ali_sec_info, set_ali_sec_info
	},
	{
		ALINKGW_ATTR_PROBE_NUMBER, ALINKGW_ATTRIBUTE_simple, 
		get_probe_num, set_probe_num
	},
	{
		ALINKGW_ATTR_PROBE_INFO, ALINKGW_ATTRIBUTE_simple,	
		get_probe_info, set_probe_info
	},
	{
		 ALINKGW_ATTR_TPSK, ALINKGW_ATTRIBUTE_simple,
		 get_tpsk, set_tpsk
	 },
	 {
		 ALINKGW_ATTR_TPSK_LIST, ALINKGW_ATTRIBUTE_array, 
		 get_tpsk_list, set_tpsk_list
	 },
	{
		ALINKGW_ATTR_PROBED_SWITCH_STATE, ALINKGW_ATTRIBUTE_simple,
		get_probed_switch_state, set_probed_switch_state		
	},
	{
		ALINKGW_ATTR_ACCESS_ATTACK_SWITCH_STATE, ALINKGW_ATTRIBUTE_simple,
		get_attack_switch_state, set_attack_switch_state
	},
	{
		ALINKGW_SUBDEV_ATTR_WANDLSPEED, ALINKGW_ATTRIBUTE_simple,
		get_wandl_speed, NULL
	},
	{
		ALINKGW_SUBDEV_ATTR_WANULSPEED, ALINKGW_ATTRIBUTE_simple,
		get_wanul_speed, NULL
	},
	{
	 ALINKGW_ATTR_WLAN_SETTING_24, ALINKGW_ATTRIBUTE_complex,
	 get_wlansetting24g, set_wlansetting24g
	},
	{
	 ALINKGW_ATTR_WLAN_SECURITY_24, ALINKGW_ATTRIBUTE_complex,
	 get_wlansecurity24g, set_wlansecurity24g
	},
	{
	 ALINKGW_ATTR_WLAN_CHANNEL_24, ALINKGW_ATTRIBUTE_complex,
	 get_wlan_channel_24g, NULL
	},
	{
	 ALINKGW_ATTR_WLAN_PAMODE, ALINKGW_ATTRIBUTE_simple,
	 get_wlanpamode, set_wlanpamode
	},	 
	{
	 ALINKGW_ATTR_QOS_SETTING, ALINKGW_ATTRIBUTE_complex,
	 get_qos_speedup, set_qos_speedup
	},
	{
		NULL, 0, NULL, NULL
	},
};

struct alinkgw_subattr_s subdev_attr_set[] = 
{
	{
		ALINKGW_ATTR_SUBDEVICE_ULSPEED	, 	ALINKGW_ATTRIBUTE_simple,
	 	subdevice_get_upspeed_value		, 	NULL
	},

	{
		ALINKGW_ATTR_SUBDEVICE_DLSPEED	, 	ALINKGW_ATTRIBUTE_simple,
		subdevice_get_downspeed_value	,	NULL
	},
	
	{NULL , 0 , NULL , NULL }
} ;



struct alinkgw_serv_s serv_set[] = {
	{ALINKGW_SERVICE_AUTHDEVICE		, 	authDevice					},
	{ALINKGW_SERVICE_CHANGEPASSWORD	, 	ChangePassword				},
	{ALINKGW_SERVICE_BWCHECK		, 	set_wan_speed_detect		}, 
	{ALINKGW_SERVICE_WL_CHANNEL		, 	rt_srv_refineWlanChannel	},
	{NULL, NULL},
};

static int tai_register_alinkgw(void)
{
	
	const struct alinkgw_attr_s *atts;
	const struct alinkgw_subattr_s *sub_attrs;
	const struct alinkgw_serv_s *servs;
	
#ifndef NKGW_RELEASE_VERSION
	int ret = ALINKGW_ERR;
	ret = ALINKGW_set_sandbox_mode();
	if(ret != ALINKGW_OK)
	{
	    SHOWDEBUG("set_sandbox_mode failed\n");       
	}
#endif

	//register attribute
	SHOWDEBUG("Register attribute\n");
	atts = attr_set;
	while(atts->name != NULL)
	{
		ALINKGW_ATTRIBUTE_REGISTER(atts->name, atts->type, atts->get ,atts->set);
		atts++;
	}	

	//register service
	SHOWDEBUG("Register service\n");
	servs = serv_set;
	while(servs->name != NULL)
	{
		ALINKGW_SERVICE_REGISTER(servs->name, servs->cb);
		servs++;
	}

	//register subdevice
	sub_attrs = subdev_attr_set;
	while(sub_attrs->name)
	{
		ALINKGW_SUBATTRIBUTE_REGISTER(sub_attrs->name , sub_attrs->type, sub_attrs->get , sub_attrs->set) ;
		sub_attrs++ ;
	}
		
	//register mib operatie cb func
	SHOWDEBUG("Register Save&Get handler\n");
	ALINKGW_set_kvp_cb(save_kvp_value , get_kvp_value);
	
	SHOWDEBUG("Register connection update handler\n");
	ALINKGW_set_conn_status_cb(connection_status_update);

	return ALINKGW_OK;
	
}

static int tai_ALINKGW_start()
{
	int ret = ALINKGW_ERR ;
		
	struct DEV_ATTRIBUTE router_attribute ;

	get_router_common_attribute(&router_attribute) ;

	ret = ALINKGW_start(router_attribute.sn ,
					router_attribute.name,
					router_attribute.brand,
					router_attribute.type,
					router_attribute.category,					
					router_attribute.manufacturer,
					router_attribute.version,
					router_attribute.mac,
					router_attribute.model,
					router_attribute.cid,
					router_attribute.key,
					router_attribute.secret );			
	printf("%s key:[%s] secret:[%s] model:[%s] name:[%s]\n",__func__,router_attribute.key,router_attribute.secret, router_attribute.model,router_attribute.name);

	if(ret != ALINKGW_OK)
	{
		SHOWDEBUG("ALINKGW Service start Failed\n");
		
		return ret ;
	}

	return ALINKGW_OK;
}
#define ALILINK_PROBED_SWITCH			"alilink_probed_switch"
#define ALILINK_ATTACK_SWITCH			"alilink_attack_switch"
void tai_init_global_value()
{
	char *val;
	_GET_VALUE(ALILINK_PROBED_SWITCH, val);
	gProbed_switch = atoi(val);

	_GET_VALUE(ALILINK_ATTACK_SWITCH, val);
	gAttack_switch = atoi(val);
	
}
static int tai_handle_ALINKGW()
{
	int ret = ALINKGW_ERR ;
       
    //增加nvram 控制阿里打印信息
    if(!strcmp ("1",  nvram_safe_get("debug_ali"))  )
    {
        ALINKGW_set_loglevel(4);
     	gWifiSchedDebug = 1;
     	gProbeInfoDebug = 1;
     	gTpskDebug = 1;
    }
    else 
    {
        ALINKGW_set_loglevel(0);
    }
	
   	tai_init_global_value(); //初始化全局变量

	init_ip_list();
	   
	tai_register_alinkgw() ;

	tai_ALINKGW_start();
	
	ret =  ALINKGW_wait_connect(-1);
	
	
	if(ret != ALINKGW_OK)
	{
		SHOWDEBUG("Connect to AliNKGW Failed\n");	

		connect_alinkgw_ok = 0 ;
		
		return ret ;
	}
	
	SHOWDEBUG("Connect to AliNKGW Cloud OK\n");

	connect_alinkgw_ok =1 ;
	
	return ALINKGW_OK ;
}

extern int al_qos_priority  ;
extern int start_qos_flag  ;
extern int handle_al_qos_priority(char * mac_str) ;
extern char qos_sign_mac[ARRAY_LEN_20] ;

static int tai_mainloop(void)
{

	while(1)
	{
		if(0 == ali_get_connection_status())
			goto loop;
#if 0		
		if(check_wlanswitch_state())
		{
			report_wlanswitch_state();
		}
#endif
		check_wlanswitch_state() ;

		if(check_wlanswitchscheduler_state())
		{
			report_wlanswitchscheduler_state();
		}

		if(gProbed_switch)
			check_alilink_probe_info();
		
		check_and_report_clientlist_state();
		
		wan_speed_report();
		//休眠30秒
#if 1
AL_2_DEBUG("[al_qos_priority:%d]\n" , al_qos_priority);
		if(al_qos_priority)
		{
			printf("%%%%%%%%%%%%%%%%%%%% handle_al_qos_priority \n");
			handle_al_qos_priority(qos_sign_mac);
		}
#endif

loop:
		cyg_thread_delay(30*100);

	}
	return 0 ;
}


static void tai_active_submit_main(void)
{
	
    load_tpsk_config();  //转载tpsk相关配置
	init_manu_table() ;
	tai_handle_ALINKGW();
	tai_running = 1 ;

	tai_mainloop() ;

	tai_running = 0 ;
	    	
	return ;
}

int tai_active_submit_start(void)
{
	if (tai_running == 0) 
	{
		cyg_thread_create(
			8, 
			(cyg_thread_entry_t *)tai_active_submit_main,
			0, 
			"tai_active_submit",
			tai_daemon_stack, 
			sizeof(tai_daemon_stack), 
			&tai_daemon_handle, 
			&tai_daemon_thread);
		cyg_thread_resume(tai_daemon_handle);
	}
	return 0;
}
