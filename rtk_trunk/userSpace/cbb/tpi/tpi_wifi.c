#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <bcmdevs.h>
#include <iflib.h>
#include <rc.h>
#include <bcmnvram.h>
#include <netconf.h>
#include <nvparse.h>
#include <shutils.h>
#include <wlutils.h>
#include <etioctl.h>

#include <cyg/hal/drv_api.h>

//roy add
#include <router_net.h>
#include "sys_module.h"
#include "debug.h"
#include "wifi.h"
#include "sys_timer.h"
#if defined(RTL819X)
#include <wl_utility_rltk.h>
#endif

COUNTRYINFO _COUNTRY_INFO_[] =
{
	{ "US",CHANNELS_1_11,CHANNELS_36_48|CHANNELS_149_165},
	{ "CA",CHANNELS_1_11,CHANNELS_36_48|CHANNELS_149_165},
	{ "MX",CHANNELS_1_13,CHANNELS_36_48|CHANNELS_149_165},
	{ "CN",CHANNELS_1_13,CHANNELS_36_48|CHANNELS_149_165},
	{ "HK",CHANNELS_1_13,CHANNELS_36_48|CHANNELS_149_165},
	{ "TW",CHANNELS_1_11,CHANNELS_36_48|CHANNELS_149_165},
	{ "ID",CHANNELS_1_13,CHANNELS_149_161},
	{ "TH",CHANNELS_1_13,CHANNELS_36_48|CHANNELS_149_165},
	/*以下是欧洲国家，欧洲国家出厂时低功率，按照国家代码来*/
	{ "NO",CHANNELS_1_13,CHANNELS_36_48},
	{ "GB",CHANNELS_1_13,CHANNELS_36_48},
	{ "DE",CHANNELS_1_13,CHANNELS_36_48},
	{ "RO",CHANNELS_1_13,CHANNELS_36_48},
	{ "CZ",CHANNELS_1_13,CHANNELS_36_48},
	{ "PL",CHANNELS_1_13,CHANNELS_36_48},
	{ "HU",CHANNELS_1_13,CHANNELS_36_48},
	{ "FR",CHANNELS_1_13,CHANNELS_36_48},
	{ "ES",CHANNELS_1_13,CHANNELS_36_48},
	{ "IT",CHANNELS_1_13,CHANNELS_36_48},
	{ "PT",CHANNELS_1_13,CHANNELS_36_48},
	{ "TR",CHANNELS_1_13,CHANNELS_36_48},
	{ "GR",CHANNELS_1_13,CHANNELS_36_48},
	{ "AT",CHANNELS_1_13,CHANNELS_36_48},
	{ "RU",CHANNELS_1_13,CHANNELS_36_48|CHANNELS_149_165},
	{ "RS",CHANNELS_1_13,CHANNELS_36_48},
	{ "BA",CHANNELS_1_13,CHANNELS_36_48},
	{ "SI",CHANNELS_1_13,CHANNELS_36_48},
	{ "AU",CHANNELS_1_13,CHANNELS_36_48|CHANNELS_149_165},
	{ "ZA",CHANNELS_1_13,CHANNELS_36_48},
	{ "SA",CHANNELS_1_13,CHANNELS_36_48|CHANNELS_149_161},
	{ "AE",CHANNELS_1_13,CHANNELS_36_48|CHANNELS_149_165},
	{ "BR",CHANNELS_1_13,CHANNELS_36_48|CHANNELS_149_165},
	/*end*/
};

static WIFI_INFO_STRUCT wifi_info; /*2.4G*/
static WIFI_INFO_STRUCT wifi_info_5g; /*5G*/

/*增加互斥锁用于防止获取信息的时候出现死机问题*/
static cyg_drv_mutex_t wifi_get_status_look;

extern PI8 gpi_apclient_dhcpc_addr(PI8 *ip,PI8 *mask);
extern int getWlanLink(char* interface);
extern wlan_mac_state tenda_getWifiStatus();

extern PI32 get_channel_table_size(WLAN_RATE_TYPE wl_rate);
extern CHANNEL_ELEMENT_T *get_channel_table_element(WLAN_RATE_TYPE wl_rate, unsigned char region);
extern PIU16 get_country_regdomain(WLAN_RATE_TYPE wl_rate,char *countrycode);
extern PI32 Is_DFS_support(char *ifname);
extern PI32 Is_DFS_channel(int channel);
static void tpi_wifi_start_handle(char *ifname);
static void tpi_wifi_stop_handle(char *ifname);
static PI8 tpi_wifi_get_cur_channel(WLAN_RATE_TYPE wl_rate);
static PI8 tpi_wifi_get_cur_bandwidth(WLAN_RATE_TYPE wl_rate);
RET_INFO tpi_wifi_update_basic_info(WIFI_SSID_CFG *cfg,PI8 *prefix);
static RET_INFO tpi_wifi_operator(WL_OPERATOR op,char *ifname);
RET_INFO tpi_get_wifi_scan_result(wlan_sscan_list_info_t *scan_list,char *ifname);
#ifdef __CONFIG_GUEST__
static PI32 tpi_wifi_guest_timer();
#endif
extern RET_INFO br_interfaces_action(BR_TYPE type,BR_ACTION action,PI8 *ifname);

/*以下函数用于api调用*/
static RET_INFO wifi_update_info(WLAN_RATE_TYPE wl_rate, WIFI_INFO_STRUCT* wifi_info)
{
	int handle_2g = 0;
	int handle_5g = 0;

	PI8 name[PI_BUFLEN_64] = {0};
	PI8 temp[PI_BUFLEN_128] = {0};

	if(WLAN_RATE_24G == wl_rate)
	{
		handle_2g = 1;
		sprintf(name,"%s_",WL_24G);
	}
	if(WLAN_RATE_5G == wl_rate)
	{
		handle_5g = 1;
		sprintf(name,"%s_",WL_5G);
	}
	
	strcpy__(wifi_info->ifname,nvram_safe_get(strcat_r(name,"ifname",temp)));/*无线接口*/

	wifi_info->work_mode = tpi_wifi_get_wl_work_mode(wl_rate);/*工作模式*/

	strcpy__(wifi_info->coutry_code,nvram_safe_get("country_code")); 	/*国家代码*/

/*网络类型*/
/*将gmode/nmode 修改为nettype(bgn,bg,b,g,n_only,a,an,ac,ac_only)
 * ac:A+N+AC
 * ac_only:AC
 */
	strcpy__(wifi_info->nettype,nvram_safe_get(strcat_r(name,"nettype",temp)));

/*信道*/
	wifi_info->channel = atoi(nvram_safe_get(strcat_r(name,"channel",temp)));	
	/*已经修改为根据国家代码和频宽返回具体的信道列表给页面*/
	//wifi_info->max_channel = tpi_wifi_get_channel_num_default(); 

/*带宽*/
    PI8 bandwidth[PI_BUFLEN_32] = {0};
	strcpy__(wifi_info->bandwidth,nvram_safe_get(strcat_r(name,"bandwidth",temp)));	

/*扩展带宽*/
	//strcpy__(wifi_info->bandside,nvram_safe_get(strcat_r(name,"nctrlsb",temp))); 
	strcpy__(wifi_info->bandside,nvram_safe_get(strcat_r(name,"bandside",temp)));
	
/*功率*/
	strcpy__(wifi_info->power,nvram_safe_get(strcat_r(name,"ctv_power",temp))); 		
	strcpy__(wifi_info->power_percent,nvram_safe_get(strcat_r(name,"country_pwr_power",temp)));

/*桥接接口参数的配置*/
	if(WL_AP_MODE == wifi_info->work_mode)
	{
		if(handle_2g)
			tpi_wifi_update_basic_info(&(wifi_info->ap_ssid_cfg),WL_24G);
		if(handle_5g)
			tpi_wifi_update_basic_info(&(wifi_info->ap_ssid_cfg),WL_5G);
	}
	else if(WL_CLIENT_MODE == wifi_info->work_mode)
	{
		if(handle_2g)
		{
			tpi_wifi_update_basic_info(&(wifi_info->ap_ssid_cfg),WL_24G);
			tpi_wifi_update_basic_info(&(wifi_info->sta_ssid_cfg),WL_24G_REPEATER);
		}
		if(handle_5g)
		{
			tpi_wifi_update_basic_info(&(wifi_info->ap_ssid_cfg),WL_5G);
			tpi_wifi_update_basic_info(&(wifi_info->sta_ssid_cfg),WL_5G_REPEATER);
		}
	}

	/*无线开关*/
	wifi_info->wl_radio = atoi(nvram_safe_get(strcat_r(name,"radio",temp)));

#ifdef __CONFIG_WPS_RTK__
	/*WPS开关*/
	strcpy__(wifi_info->wps_enable,nvram_safe_get(WPS_MODE_ENABLE));
#endif 

	//获取灵敏度开关
#ifdef __CONFIG_DISTURB_EN__
		strcpy__(wifi_info->antijam_en,nvram_safe_get(WLAN_PUBLIC_DIG_SEL));
#endif 
	//wifi beaforming enable
#ifdef __CONFIG_WL_BEAMFORMING_EN__
		wifi_info->wl_beamforming_en = atoi(nvram_safe_get(WLAN_PUBLIC_BEAMFORMING_EN));
#endif

  
	return RET_SUC;
}

RET_INFO tpi_wifi_update_info(WLAN_RATE_TYPE wl_rate)
{
	if(WLAN_RATE_24G == wl_rate || WLAN_RATE_ALL == wl_rate)
	{
		wifi_update_info(WLAN_RATE_24G,&wifi_info);
	}
	if(WLAN_RATE_5G == wl_rate || WLAN_RATE_ALL == wl_rate)
	{
		wifi_update_info(WLAN_RATE_5G,&wifi_info_5g);
	}
	return RET_SUC;
}

RET_INFO tpi_wifi_struct_init()
{
	memset(&wifi_info,0x0,sizeof(wifi_info));
	memset(&wifi_info_5g,0x0,sizeof(wifi_info_5g));
	return tpi_wifi_update_info(WLAN_RATE_ALL);
}

RET_INFO tpi_wifi_first_init()
{
	cyg_mutex_init(&wifi_get_status_look);
#ifdef __CONFIG_GUEST__
	tpi_wifi_guest_timer();
	if(atoi(nvram_safe_get("wl_guest_effective_time")))
	{
		nvram_set(WLAN24G_GUEST_ENABLE,"0");
		nvram_set(WLAN5G_GUEST_ENABLE,"0");	
	}
#endif
	if(wifi_info.wl_radio == 1)
	{
		tpi_wifi_operator(WLAN_START,TENDA_WLAN24_AP_IFNAME);
	}
	
	if(wifi_info_5g.wl_radio == 1)
	{
		tpi_wifi_operator(WLAN_START,TENDA_WLAN5_AP_IFNAME);
	}

	return RET_SUC;
}
#ifdef __CONFIG_GUEST__
static int guest_effective_time = 0;
static int guest_effective_enable = 0;

void guest_effective_tmr_func(void)
{
	if(guest_effective_enable)		
	{
		guest_effective_time--;		
		if(guest_effective_time <= 0)
		{
			nvram_set(WLAN24G_GUEST_ENABLE,"0");
			nvram_set(WLAN5G_GUEST_ENABLE,"0");	
			tpi_wifi_operator(WLAN_STOP,TENDA_WLAN24_GUEST_IFNAME);
			tpi_wifi_operator(WLAN_STOP,TENDA_WLAN5_GUEST_IFNAME);
		}
	}
}

static PI32 tpi_wifi_guest_timer()
{
	DO_TIMER_FUN timer;
	
    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,GUEST_NETWORK_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME;
    timer.fun = guest_effective_tmr_func;
    sys_do_timer_add(&timer);
	return 0;
}
#endif
#ifdef __CONFIG_A9__
static MODULE_WORK_STATUS wifi_work_status = MODULE_BEGIN;
static void tpi_wifi_set_action_complete()
{
    wifi_work_status = MODULE_COMPLETE;
}

MODULE_WORK_STATUS tpi_wifi_get_action_type()
{
    return wifi_work_status;
}

void tpi_wifi_action_reinit()
{
	wifi_work_status = MODULE_BEGIN;
	return ;
}
#endif

RET_INFO tpi_wifi_action(RC_MODULES_COMMON_STRUCT *var)
{	
	PI_PRINTF(TPI,"op=%d,string_info=%s,wlan_ifname=%s\n",var->op,var->string_info,var->wlan_ifname);

#ifdef __CONFIG_A9__
	tpi_wifi_action_reinit();
#endif
	tpi_wifi_update_info(WLAN_RATE_ALL);

	switch(var->op)
	{
		case OP_START:
			tpi_wifi_operator(WLAN_START,var->wlan_ifname);
			break;
		case OP_STOP:
			tpi_wifi_operator(WLAN_STOP,var->wlan_ifname);
			break;
		case OP_RESTART:
			tpi_wifi_operator(WLAN_RESTART,var->wlan_ifname);
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}

	if(0 == strcmp(var->string_info,"button"))
	{
		//当两个AP接口都关闭的情况，才将无线都开启
		if(INTERFACE_DOWN == get_interface_state(TENDA_WLAN24_AP_IFNAME) 
			&& INTERFACE_DOWN == get_interface_state(TENDA_WLAN5_AP_IFNAME))
		{
			nvram_set(WLAN24G_ENABLE,"1");
			nvram_set(WLAN5G_ENABLE,"1");
			#ifdef __CONFIG_WPS_RTK__
			msg_send(MODULE_RC, RC_WPS_MODULE, "op=1");
			#endif
			tpi_wifi_operator(WLAN_START,NULL);
		}
		//其他情况君关闭无线接口1、一开一关2、两开
		else
		{
			nvram_set(WLAN24G_ENABLE,"0");
			nvram_set(WLAN5G_ENABLE,"0");
#ifdef __CONFIG_GUEST__			
			if(atoi(nvram_safe_get("wl_guest_effective_time")))
			{
				nvram_set(WLAN24G_GUEST_ENABLE,"0");
				nvram_set(WLAN5G_GUEST_ENABLE,"0");	
			}
#endif		
			#ifdef __CONFIG_WPS_RTK__
			msg_send(MODULE_RC, RC_WPS_MODULE, "op=2");
			#endif
			tpi_wifi_operator(WLAN_STOP,NULL);
			
		}
		nvram_commit();
	}

#ifdef __CONFIG_A9__
	tpi_wifi_set_action_complete();
#endif

	return RET_SUC;
}

#ifdef __CONFIG_AUTO_CONN_CLIENT__
/*关联在自动桥接模块下的消息处理函数*/
extern void tpi_auto_conn_client_set_restarting_tag(PIU8 tag);
RET_INFO tpi_auto_connn_wifi_action(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		return RET_ERR;
	}

	PI_PRINTF(TPI, "op=%d\n", var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_wifi_operator(WLAN_RESTART,NULL);//是否需要添加5G？？？？
			break;
		case OP_STOP:
		case OP_RESTART:
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}

	/*无线重启完成，将标志位置位置0*/
	tpi_auto_conn_client_set_restarting_tag(0);

	return RET_SUC;
}
#endif

/*以下用于gpi获取信息函数*/
P_WIFI_INFO_STRUCT tpi_wifi_get_info(WLAN_RATE_TYPE wl_rate)
{
	tpi_wifi_update_info(wl_rate);

	if(WLAN_RATE_24G == wl_rate)
	{
		return &wifi_info;
	}
	if(WLAN_RATE_5G == wl_rate)
	{
		return &wifi_info_5g;
	}
}

P_WIFI_CURRCET_INFO_STRUCT tpi_wifi_get_curret_info(WLAN_RATE_TYPE wl_rate,P_WIFI_CURRCET_INFO_STRUCT cfg)
{
	if(NULL != cfg && (WLAN_RATE_24G == wl_rate || WLAN_RATE_5G == wl_rate))
	{
		cfg->channel = tpi_wifi_get_cur_channel(wl_rate);
		cfg->bandwidth = tpi_wifi_get_cur_bandwidth(wl_rate);
	}
	return cfg;
}

PI8 tpi_wifi_wl_vif_enabled(const PI8 *name)
{
	int ret = 1;
	int ret_priv = 1;	

	if(strstr(name,TENDA_WLAN24_AP_IFNAME))
	{
		ret_priv = atoi(nvram_safe_get(WLAN24G_ENABLE));
	}
	else if(strstr(name,TENDA_WLAN5_AP_IFNAME) )
	{
		ret_priv = atoi(nvram_safe_get(WLAN5G_ENABLE));
	}

	if( !strcmp(name,TENDA_WLAN24_REPEATER_IFNAME) )
	{
		ret = atoi(nvram_safe_get(WLAN24G_REPEATER_ENABLE));
	}
#ifdef __CONFIG_GUEST__
	else if( !strcmp(name,TENDA_WLAN24_GUEST_IFNAME) )
	{
		ret = atoi(nvram_safe_get(WLAN24G_GUEST_ENABLE));
	}
#endif
	else if( !strcmp(name,TENDA_WLAN5_REPEATER_IFNAME) )
	{
		ret = atoi(nvram_safe_get(WLAN5G_REPEATER_ENABLE));
	}
#ifdef __CONFIG_GUEST__
	else if( !strcmp(name,TENDA_WLAN5_GUEST_IFNAME) )
	{
		ret = atoi(nvram_safe_get(WLAN5G_GUEST_ENABLE));
	}
#endif
	else if( !strcmp(name,TENDA_WLAN24_VIRTUAL_IFNAME) )
	{
		ret = atoi(nvram_safe_get(WLAN24G_VIRTUAL_ENABLE));
	}
	else if( !strcmp(name,TENDA_WLAN5_VIRTUAL_IFNAME) )
	{
		ret = atoi(nvram_safe_get(WLAN5G_VIRTUAL_ENABLE));
	}
	return ret_priv && ret;
}

RET_INFO tpi_wifi_get_bridge_rssi(WLAN_RATE_TYPE wl_rate,P_WIFI_BRIDGE_INFO_STRUCT bridge_info)
{
	RET_INFO ret = RET_SUC;
	WLAN_STA_INFO_T bridge_info_buf;

	if(WLAN_RATE_24G == wl_rate)
	{
		ret = getBridgeInfo(TENDA_WLAN24_REPEATER_IFNAME,&bridge_info_buf);
	}
	else if(WLAN_RATE_5G == wl_rate)
	{
		ret = getBridgeInfo(TENDA_WLAN5_REPEATER_IFNAME,&bridge_info_buf);
	}
	
	if(ret <= 0)
		ret = RET_ERR;

	bridge_info->link_time = bridge_info_buf.link_time;
	bridge_info->rssi = bridge_info_buf.rssi;

	return ret;
}

void tpi_wifi_up_interface(char* ifname)
{
	if(ifname == NULL)
		return ;

	if(tpi_wifi_wl_vif_enabled(ifname))
	{
		wlconf(ifname);
		/*起服务*/
		wlconf_start(ifname); 
        //同步AC6V2 svn7122，系统上电时出现发射功率异常，导致RX丢包的问题
        if (0 == strcmp(ifname, "wlan1"))
        {
            wlconf_down(ifname);
        
            wlconf(ifname);

            wlconf_start(ifname);
        }
		/*加入桥*/
		if(RET_ERR == br_interfaces_action(BR_WIRLESS,BR_UP,ifname))
		{
			printf("==add bridge error:%s==%s [%d]\n",ifname, __FUNCTION__, __LINE__);
		}
	}
}

void tpi_wifi_down_interface(char* ifname)
{
	if(RET_ERR == br_interfaces_action(BR_WIRLESS,BR_DOWN,ifname))
	{
		printf("==remove bridge error:%s==%s [%d]\n",ifname, __FUNCTION__, __LINE__);
	}

#ifdef __CONFIG_WIFI_DOUBLEBAND_UNITY__
	char buf[64] = {0};
	//关闭双频优选
	if(tpi_wifi_check_is_priv_interface(ifname))
	{
		memset(buf, 0x0, sizeof(buf));
		sprintf((char *)buf,"wlan0 steer write %d",0);
		run_clicmd(buf);
	}
#endif

	wlconf_down(ifname);
}
#ifdef __CONFIG_GUEST__
extern int delete_all_online_client(char *ifname);
void tpi_wifi_config_guest()
{
	int effective_time = 0;

	guest_effective_enable = 0;
	if(nvram_match(SYSCONFIG_WORKMODE,"route")
		&&(nvram_match(WLAN24G_GUEST_ENABLE,"1")
		||nvram_match(WLAN5G_GUEST_ENABLE,"1")))
	{
		if(INTERFACE_UP == get_interface_state(TENDA_WLAN24_GUEST_IFNAME))
		{
			
			delete_all_online_client(TENDA_WLAN24_GUEST_IFNAME);
			tenda_ifconfig(TENDA_WLAN24_GUEST_IFNAME, 0,NULL,NULL);
			tenda_ifconfig(TENDA_WLAN24_GUEST_IFNAME, IFUP,NULL,NULL);
		}
		if(INTERFACE_UP == get_interface_state(TENDA_WLAN5_GUEST_IFNAME))
		{
			delete_all_online_client(TENDA_WLAN5_GUEST_IFNAME);
			tenda_ifconfig(TENDA_WLAN5_GUEST_IFNAME, 0,NULL,NULL);
			tenda_ifconfig(TENDA_WLAN5_GUEST_IFNAME, IFUP,NULL,NULL);
		}
		
		if(INTERFACE_UP == get_interface_state(TENDA_WLAN24_GUEST_IFNAME))
		{
			tenda_ifconfig(TENDA_WLAN24_GUEST_IFNAME, IFUP
			,nvram_safe_get("lan1_ipaddr"), nvram_safe_get("lan1_netmask"));
		}
		else if(INTERFACE_UP == get_interface_state(TENDA_WLAN5_GUEST_IFNAME))
		{
			tenda_ifconfig(TENDA_WLAN5_GUEST_IFNAME, IFUP
			,nvram_safe_get("lan1_ipaddr"), nvram_safe_get("lan1_netmask"));
		}

		effective_time = atoi(nvram_safe_get("wl_guest_effective_time"));
		if(effective_time)
		{
			guest_effective_enable = 1;
			guest_effective_time = effective_time;
		}
		
		msg_send(MODULE_RC, RC_DHCP_SERVER_MODULE, "op=3");
	}
}
#endif
/* Bring up wireless interfaces */
void tpi_wifi_start_wl(char *ifname)
{
	if(tpi_wifi_check_is_priv_interface(ifname))
	{
		if(!strcmp(ifname,TENDA_WLAN24_AP_IFNAME))
		{	
			/*配置无线参数*/	
			tpi_wifi_up_interface(TENDA_WLAN24_AP_IFNAME);
#ifdef __CONFIG_GUEST__
			if(nvram_match(SYSCONFIG_WORKMODE,"route") 
				&&nvram_match(WLAN24G_GUEST_ENABLE,"1"))
			{
				tpi_wifi_up_interface(TENDA_WLAN24_GUEST_IFNAME);
			}
#endif			
			if(wifi_info.work_mode == WL_CLIENT_MODE)
			{
				tpi_wifi_up_interface(TENDA_WLAN24_REPEATER_IFNAME);
			}
		}
		else if(!strcmp(ifname,TENDA_WLAN5_AP_IFNAME))
		{
			/*配置无线参数*/	
			tpi_wifi_up_interface(TENDA_WLAN5_AP_IFNAME);
#ifdef __CONFIG_GUEST__
			if(nvram_match(SYSCONFIG_WORKMODE,"route") 
				&&nvram_match(WLAN5G_GUEST_ENABLE,"1"))
			{
				tpi_wifi_up_interface(TENDA_WLAN5_GUEST_IFNAME);
			}
#endif
			if(wifi_info_5g.work_mode == WL_CLIENT_MODE)
			{
				tpi_wifi_up_interface(TENDA_WLAN5_REPEATER_IFNAME);
			}
		}

	}
	else
	{
		tpi_wifi_up_interface(ifname);
	}
	return;
}


static void tpi_wifi_stop_wl(char *ifname)
{	

	if(tpi_wifi_check_is_priv_interface(ifname))
	{	
		if(!strcmp(ifname,TENDA_WLAN24_AP_IFNAME))
		{
#ifdef __CONFIG_GUEST__
			if(INTERFACE_UP == get_interface_state(TENDA_WLAN24_GUEST_IFNAME))
			{
				tpi_wifi_down_interface(TENDA_WLAN24_GUEST_IFNAME);
			}
#endif
			if(INTERFACE_UP == get_interface_state(TENDA_WLAN24_REPEATER_IFNAME))
			{
				tpi_wifi_down_interface(TENDA_WLAN24_REPEATER_IFNAME);
			}

			tpi_wifi_down_interface(TENDA_WLAN24_AP_IFNAME);
		}else if(!strcmp(ifname,TENDA_WLAN5_AP_IFNAME))
		{
#ifdef __CONFIG_GUEST__
			if(INTERFACE_UP == get_interface_state(TENDA_WLAN5_GUEST_IFNAME))
			{
				tpi_wifi_down_interface(TENDA_WLAN5_GUEST_IFNAME);
			}
#endif
			if(INTERFACE_UP == get_interface_state(TENDA_WLAN5_REPEATER_IFNAME))
			{
				tpi_wifi_down_interface(TENDA_WLAN5_REPEATER_IFNAME);
			}

			tpi_wifi_down_interface(TENDA_WLAN5_AP_IFNAME);
		}
	}
	else
	{
		tpi_wifi_down_interface(ifname);
	}
	
	return;
}


static void set_wl_pwr_percent(char *ifname)
{	
    if(!strcmp(ifname,TENDA_WLAN24_AP_IFNAME))
    {
        tpi_wifi_set_offset_power(WLAN_RATE_24G);
    }else if(!strcmp(ifname,TENDA_WLAN5_AP_IFNAME))
    {
        tpi_wifi_set_offset_power(WLAN_RATE_5G);
    }  
    
	return;
}

int tpi_wifi_check_is_priv_interface(char *ifname)
{
	if(!strcmp(ifname,TENDA_WLAN24_AP_IFNAME) 
		|| !strcmp(ifname,TENDA_WLAN5_AP_IFNAME))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void tpi_wifi_start_handle(char *ifname)
{
	if(tpi_wifi_check_is_priv_interface(ifname))
		set_wl_pwr_percent(ifname);//设置Realtek功率偏移 
	tpi_wifi_start_wl(ifname); //配置开启无线
	
	return;
}

static void tpi_wifi_stop_handle(char *ifname)
{
	tpi_wifi_stop_wl(ifname);

	return;
}

/*在其他线程可能会用到(wifi定时用到)*/
WL_RADIO_TYPE tpi_get_wlan24g_enable(void)
{
	PI8 ret = -1;
	ret = nvram_match(WLAN24G_ENABLE,"1") ? WL_ON : WL_OFF;
	return ret;
}

WL_RADIO_TYPE tpi_get_wlan5g_enable(void)
{
	PI8 ret = -1;
	ret = nvram_match(WLAN5G_ENABLE,"1") ? WL_ON : WL_OFF;
	return ret;
}

WIFISTASTATUS tpi_get_sta_info(PI8* ifname)
{
    PI32 ret;
    PI8 buf[1526] = {0};
    sta_info_t *sta;
    struct ether_addr ea;
    PI32 off;
    PI32 sta_flag;
    PI32 *wpa;

	if(NULL == ifname)
	{
		return WIFI_INIT_FAIL;
	}

	cyg_drv_mutex_lock(&wifi_get_status_look);
	/*
	STATE_DISABLED=0, STATE_IDLE, STATE_SCANNING, STATE_STARTED, STATE_CONNECTED, STATE_WAITFORKEY
	*/
	/*add by fh,when the mode is apclient, the wan_timer is not to call update wifi_status,so in here we must call it when is apclient mode*/
	if(tpi_wifi_get_sys_work_mode() == WL_APCLIENT_MODE)	
	{
		getWlanLink(ifname);
	}
    
	wlan_mac_state wifi_status = tenda_getWifiStatus();
	WIFISTASTATUS status = WIFI_INIT_FAIL;
	static WIFISTASTATUS old_wifi_status = WIFI_SCANNING;//旧无线状态
	static PIU8 fail_count = 0;

	if(wifi_status != STATE_WAITFORKEY)
		fail_count = 0;
	static PIU32 count = STATUS_CHANGE_COUNT;

	switch(wifi_status)
	{
		case STATE_SCANNING:/*扫描中，尝试连接*/
			/*密码错误的情况下会跳到扫描状态*/
			if(count < STATUS_CHANGE_COUNT)
			{
				count++;
			}
			else if(count == STATUS_CHANGE_COUNT)
			{
				status = WIFI_SCANNING;
				count++;
			}
			old_wifi_status = WIFI_SCANNING;
			break;
		case STATE_CONNECTED:/*无线链路已经链接上*/
			count= STATUS_CHANGE_COUNT;
			status = WIFI_OK;
			break;
		case STATE_WAITFORKEY:/*等待密码，即密码错误*/
			count= 0;
			if(fail_count == 3)
			{
				status = WIFI_AUTH_FAIL;
				old_wifi_status = WIFI_AUTH_FAIL;
			}
			else if(fail_count < 3)
			{
				fail_count++;
			}
			break;
		case STATE_STARTED:/*我的理解是无线初始状态*/
			status = WIFI_INIT_FAIL;
			count= STATUS_CHANGE_COUNT;
			break;

		case STATE_DISABLED:/*我的理解是无线不可用*/
			if(WIFI_AUTH_FAIL == old_wifi_status && nvram_match("wl0.1_radio","1"))
			{
				/*无线开启的时候，状态从密码错误变为不可用，仍然为密码错误，密码错误后较长时间处于DISABLE状态*/
				status = WIFI_AUTH_FAIL;
			}
			else
			{
				status = WIFI_INIT_FAIL;
			}
			count= STATUS_CHANGE_COUNT;
			break;

		case STATE_IDLE:/*我的理解是无线闲置，没有去连接*/
		default:
			count= STATUS_CHANGE_COUNT;
			status = WIFI_INIT_FAIL;
			break;
	}
	cyg_drv_mutex_unlock(&wifi_get_status_look);
	return status;

}

WL_WORK_MODE tpi_wifi_get_wl_work_mode(WLAN_RATE_TYPE rete_type)
{
    WL_WORK_MODE ret = WL_AP_MODE;

    if(WLAN_RATE_24G == rete_type)
    {
    	if(nvram_match(WLAN24G_WORK_MODE,"ap"))
	    {
	        ret = WL_AP_MODE;
	    }
	    else 
	    {
	        ret = WL_CLIENT_MODE;
	    }
    }
    else
    {
    	if(nvram_match(WLAN5G_WORK_MODE,"ap"))
	    {
	        ret = WL_AP_MODE;
	    }
	    else
	    {
	        ret = WL_CLIENT_MODE;
	    }    	
    }
    return ret;
}

SYS_WORK_MODE tpi_wifi_get_sys_work_mode()
{
    SYS_WORK_MODE ret = WL_ROUTE_MODE;
	
    if(nvram_match(SYSCONFIG_WORKMODE,"wisp"))
    {
        ret = WL_WISP_MODE;
    }
    else if(nvram_match(SYSCONFIG_WORKMODE,"client+ap"))
    {
        ret = WL_APCLIENT_MODE;
    }
    else if(nvram_match(SYSCONFIG_WORKMODE,"wds"))
    {
        ret = WL_WDS_MODE;
    }
    else if(nvram_match(SYSCONFIG_WORKMODE,"bridge"))
    {
        ret = WL_BRIDGEAP_MODE;
    }
    else
    {
        ret = WL_ROUTE_MODE;
    }

    return ret;
}

RET_INFO tpi_wifi_get_wifi_status(WANSTATUS *wan_status,WIFISTASTATUS *wifi_status)
{
	RET_INFO ret = RET_SUC;
	SYS_WORK_MODE mode = WL_ROUTE_MODE;
	PI32 conStat = 0;
	struct ether_addr bssid = {{0}};

	if(NULL == wan_status || NULL == wifi_status)
	{
		return RET_ERR;
	}

	mode = tpi_wifi_get_sys_work_mode();

	switch(mode)
	{
		case WL_WISP_MODE:
			conStat = get_wan_connstatus();
			if(0 == conStat)
	        {
	            (*wan_status) = WAN_DISCONNECTED;
	        }
	        else if(1 == conStat)
	        {
	            (*wan_status) = WAN_CONNECTING;
	        }
	        else if(2 == conStat)
	        {
	            (*wan_status) = WAN_CONNECTED;
	        }
			if(wifi_info.work_mode == WL_CLIENT_MODE)
			(*wifi_status) = tpi_get_sta_info(TENDA_WLAN24_REPEATER_IFNAME);
			else
				(*wifi_status) = tpi_get_sta_info(TENDA_WLAN5_REPEATER_IFNAME);
			break;
		case WL_APCLIENT_MODE:
			if(wifi_info.work_mode == WL_CLIENT_MODE)
			(*wifi_status) = tpi_get_sta_info(TENDA_WLAN24_REPEATER_IFNAME);
			else
				(*wifi_status) = tpi_get_sta_info(TENDA_WLAN5_REPEATER_IFNAME);
	        if(WIFI_INIT_FAIL == *wifi_status)
	        {
	            (*wan_status) = WAN_DISCONNECTED;
	        }
	        else if(WIFI_OK == *wifi_status)
	        {
				PI8 ip[17] = {0},mask[17] = {0};
				gpi_apclient_dhcpc_addr(ip,mask);
				if(0 == strcmp(ip,"") || 0 == strcmp(mask,""))
				{
					(*wan_status) = WAN_CONNECTING;
				}
				else
				{
	            	(*wan_status) = WAN_CONNECTED;
				}
	        }
	        else
	        {
	            (*wan_status) = WAN_CONNECTING;
	        }
			//PI_PRINTF(TPI,"APCLIENT:%d,%d\n",(*wan_status),(*wifi_status));
			break;
		case WL_BRIDGEAP_MODE:
			/* 桥AP模式下使用有线链路状态，替换无线关联状态 */
			if(nvram_match(SYSCONFIG_WORKMODE, "client+ap")
			   || nvram_match(SYSCONFIG_WORKMODE, "bridge"))
			{
				if(tenda_wan_link_status())
				{
					(*wifi_status) = WIFI_OK;
				}
				else
				{
					(*wifi_status) = WIFI_INIT_FAIL;
				}
				
		        if(WIFI_INIT_FAIL == *wifi_status)
		        {
		            (*wan_status) = WAN_DISCONNECTED;
		        }
		        else if(WIFI_OK == *wifi_status)
		        {
#ifdef __CONFIG_APCLIENT_DHCPC__
					PI8 ip[17] = {0},mask[17] = {0};
					gpi_apclient_dhcpc_addr(ip,mask);
					if(0 == strcmp(ip,"") || 0 == strcmp(mask,""))
					{
						(*wan_status) = WAN_CONNECTING;
					}
					else if(gpi_apclient_dhcpc_ping_gateway())
					{
						(*wan_status) = WAN_CONNECTED;
					}
					else
#endif	        	
					{
		            	(*wan_status) = WAN_DISCONNECTED;
					}
		        }
				ret = RET_SUC;
			}
			break;
		case WL_ROUTE_MODE:
		default:
			ret = RET_ERR;
			break;
	}
	
    return ret;
}

//返回在或者不在当前DUT配置的国家所支持的信道中
RET_INFO tpi_wifi_channles_in_country(WLAN_RATE_TYPE wl_rate,int channels)
{
    PI8 *country = NULL;
    RET_INFO ret = RET_ERR;
    int i = 0;
	
 	country = nvram_safe_get(WLAN_PUBLIC_COUNTRY_CODE);

	for(i = 0;i < sizeof(_COUNTRY_INFO_)/sizeof(COUNTRYINFO);i++)
	{
		if(0 == strcmp(_COUNTRY_INFO_[i].Country,country))
		{
			switch(wl_rate)
			{
				case WLAN_RATE_24G:
					if(CHANNELS_1_11 == _COUNTRY_INFO_[i].channels_24)
					{
						if(0 <= channels && channels <= 11)
							ret = RET_SUC;
					}
					else if(CHANNELS_1_13 == _COUNTRY_INFO_[i].channels_24)
					{
						if(0 <= channels && channels <= 13)
							ret = RET_SUC;						
					}
					break;
				case WLAN_RATE_5G:
					if(_COUNTRY_INFO_[i].channels_5 & CHANNELS_36_48)
					{
						if(36 <= channels && channels <= 48)
						{
							ret = RET_SUC;
							break;
						}
					}
					
					if(_COUNTRY_INFO_[i].channels_5 & CHANNELS_149_161)
					{
						if(149 <= channels && channels <= 161)
						{
							ret = RET_SUC;
							break;
						}
					}
					
					if(_COUNTRY_INFO_[i].channels_5 & CHANNELS_149_165)
					{
						if(149 <= channels && channels <= 165)
						{
							ret = RET_SUC;
							break;
						}
					}
					break;
				default:
					break;
			}
			break;
		}
	}	
	return ret;

}
//根据国家代码 返回当前国家代码所支持的所有信道
PI32 tpi_wifi_get_channels(WLAN_RATE_TYPE wl_rate, PI32 bandwidth, PI8 *country, PIU16 *list, PI32 len)
{
	CHANNEL_ELEMENT_T *ch = NULL;
	unsigned char region;
	char temp[PI_BUFLEN_256] = {0};
	int size;
	int i;
	int start, end;
	int ch_nums = 0;
	int dfs_support = 0;

	if(country == NULL || list == NULL)
	{
		PI_ERROR(TPI,"Invalid paramaters!\n");
		return 0;
	}

	//PI_DEBUG(TPI,"rate:%d, bandwidth: %d, country: %s\n", wl_rate,bandwidth, country);
	
	region = get_country_regdomain(wl_rate, country);
	size = get_channel_table_size(wl_rate);
	if(region < 1 || region > size)
	{
		PI_ERROR(TPI,"Invalid country %s\n", country);
		return 0;
	}

	ch = get_channel_table_element(wl_rate, region);
	if(len < ch->channel_num)
	{
		PI_ERROR(TPI,"List buffer too short!\n");
		return 0;
	}
	
	if(wl_rate == WLAN_RATE_24G)
	{		
		if(bandwidth == 20)
		{
			start = 0;
			end = ch->channel_num - 1;
		}
		else if(bandwidth == 40)
		{
			/* 40MHz we use upper channels default */
			if(ch->channel_num > 4)
			{
				start = 4;
				end = ch->channel_num - 1;
			}
			else if(ch->channel_num == 4)
			{
				start = 3;
				end = 3;
			}
			else
			{
				PI_ERROR(TPI,"Invalid country %s\n", country);
				return 0;
			}
		}
		else
		{
			PI_ERROR(TPI,"Invalid bandwidth %d\n", bandwidth);
			return 0;
		}
			
	}
	else if(wl_rate == WLAN_RATE_5G)
	{
		char ifname[PI_BUFLEN_16] = {0};
		dfs_support = 0;//Is_DFS_support(TENDA_WLAN5_AP_IFNAME);
		
		if(bandwidth == 20)
		{
			start = 0;
			end = ch->channel_num - 1;
		}
		else if(bandwidth == 40 || bandwidth == 80)
		{
			start = 0;
			end = ch->channel_num - 1;
			/* 40MHz or 80MHz no 165 channel */
			if(ch->channels[end] == 165)
				end--;
		}
		else
		{
			PI_ERROR(TPI,"Invalid bandwidth %d\n", bandwidth);
			return 0;
		}

	}
	else
	{
	
		PI_ERROR(TPI,"Invalid band %d\n", wl_rate);
		return 0;
	}

	for(i = start; i <= end; i++)
	{
		/* If driver not support DFS or disabled, ignore DFS channels */
		
		if(wl_rate == WLAN_RATE_5G)
		{
			if(Is_DFS_channel(ch->channels[i]) && !dfs_support)
			{
				//PI_DEBUG(TPI,"ignore channel %d\n", ch->channels[i]);
				continue;
			}
		}
		/* Indonesia has no 40MHz and 80MHz bandwidth in 5G band */
		if(!strcmp(country, "ID") && (bandwidth == 40|| bandwidth == 80))
		{
			continue;
		}

		/* Russia has no 80MHz bandwidth in 5G band */
		if(!strcmp(country, "RU") && bandwidth == 80)
		{
			continue;
		}

		list[ch_nums++] = ch->channels[i];
		
	}

	//PI_DEBUG(TPI,"ch_nums:%d channels: %s\n",ch_nums,list);

	return ch_nums;
}
void tpi_wifi_get_power_default(WLAN_RATE_TYPE wl_rate,PI8 *power)
{
    PI8 *power_default = NULL;
	int power_num = 0;
	PI8 power_index[3][8] = {"100","100","100"};
	PI8 power_percent[PI_BUFLEN_16];
	PI8 name[PI_BUFLEN_128] = {0};
	PI8 temp[PI_BUFLEN_128] = {0};
	
    if(NULL != power)
    {
		if(WLAN_RATE_24G == wl_rate)
		{
			snprintf(name,sizeof(name),"%s_",WL_24G);
		}
		else if(WLAN_RATE_5G == wl_rate)
		{
			snprintf(name,sizeof(name),"%s_",WL_5G);
		}
		strcpy__(power_percent,nvram_safe_get(strcat_r(name,"country_pwr_power",temp)));
		power_num = sscanf(power_percent, "%[^,],%[^,],%s", power_index[0], power_index[1], power_index[2]);
		
		if (power_num == 1)
		{
			strcpy(power,"low");
		}
		else if (power_num == 2)
		{
			strcpy(power,"normal");
		}
		else
		{
			strcpy(power,"high");
		}
    }

    return;
}

void tpi_wifi_get_current_power(WLAN_RATE_TYPE wl_rate,PI8 *power)
{
	PI8 name[PI_BUFLEN_128] = {0};
	PI8 temp[PI_BUFLEN_128] = {0};
	
	if(WLAN_RATE_24G == wl_rate)
	{
		snprintf(name,sizeof(name),"%s_",WL_24G);
	}
	else if(WLAN_RATE_5G == wl_rate)
	{
		snprintf(name,sizeof(name),"%s_",WL_5G);	
	}
	strcpy__(power,nvram_safe_get(strcat_r(name,"ctv_power",temp)));
	return;
}

//根据传入的接口名设置功率偏移
void tpi_wifi_set_offset_power(WLAN_RATE_TYPE wl_rate)
{
    PI8 current_power[PI_BUFLEN_32] = {0};
    PI8 support_power_level[PI_BUFLEN_32] = {0};
    PI8 *pwr_offset_percent = NULL;
    PI8 pwr_percent[3][8] = {"0","0","0"};
    PI32 pwrlevel = 0;
    PI8 pwrlevel_str[PI_BUFLEN_8] = {0};
	PI8 name[PI_BUFLEN_32] = {0};
	PI8 temp[PI_BUFLEN_128] = {0};

    tpi_wifi_get_power_default(wl_rate,support_power_level); //DUT 支持的功率等级	 
	tpi_wifi_get_current_power(wl_rate,current_power);//当前设置的功率:low、normal、high
	
    if( 0 == strcmp(support_power_level,"") ||0 == strcmp(current_power,""))
    {
        PI_ERROR(TPI,"support_power_level or current_power is NULL\n");
        return;
    }
    
	pwr_offset_percent = nvram_safe_get(WLAN_PUBLIC_PWR_OFFSET_PERCENT);//功率偏移量12、6、0 对应6dbm、3dbm、0dbm

    if(NULL == pwr_offset_percent || 0 == strcmp(pwr_offset_percent,""))
    {
        PI_ERROR(TPI,"pwr_offset_percent(12,6,0) is NULL\n");
        return;
    }

	sscanf(pwr_offset_percent,"%[^,],%[^,],%s",pwr_percent[0],pwr_percent[1],pwr_percent[2]);
	
	if(0 == strcmp(support_power_level,"low"))//低功率产品，功率不进行调节
	{
		pwrlevel = 0;
	}
	else if(0 == strcmp(support_power_level,"normal"))//中功率产品，支持2个等级的功率调节
	{
		if(0 == strcmp(current_power,"high"))
		{
			pwrlevel = atoi(pwr_percent[2]);
		}
		else
		{
			//2.4G中功率产品与5G中功率产品的偏移量不一样
			if(WLAN_RATE_5G == wl_rate)
			{
				pwrlevel = atoi(pwr_percent[0]);
			}
			else
			{
				pwrlevel = atoi(pwr_percent[1]);
			}
		}
	}
	else//高功率产品，支持3个等级的功率调节
	{
		if(0 == strcmp(current_power,"low"))
		{
			pwrlevel = atoi(pwr_percent[0]);
		}
		else if(0 == strcmp(current_power,"normal"))
		{
			pwrlevel = atoi(pwr_percent[1]);				
		}
		else if(0 == strcmp(current_power,"high"))
		{
			pwrlevel = atoi(pwr_percent[2]);				
		}
	}
    
	snprintf(pwrlevel_str,sizeof(pwrlevel_str),"%d",pwrlevel);
	 
	if(WLAN_RATE_24G == wl_rate)
	{
		snprintf(name,sizeof(name),"%s_",WL_24G);
	}
	else if(WLAN_RATE_5G == wl_rate)
	{
		snprintf(name,sizeof(name),"%s_",WL_5G);
	}
	
	nvram_set(strcat_r(name,"offset_power",temp),pwrlevel_str);
	
	return;
}

void tpi_wifi_get_country_default(PI8 *country)
{
    PI8 *country_default = NULL;
	
    if(NULL != country)
    {
        country_default = nvram_get(WLAN_PUBLIC_COUNTRY_CODE);
        if(NULL != country_default)
        {
            strcpy(country,country_default);
        }
    }

    return;
}

//获取中继扫描结果列表
RET_INFO tpi_get_wifi_scan_result(wlan_sscan_list_info_t *scan_list,char* ifname)
{
	
	int scan_status = 0,retry=0;
	recreate:
		//扫描请求
		switch(getWlSiteSurveyRequest(ifname, &scan_status)) 
		{ 
			case -2: 
				printf("-2:Auto scan running!!please wait...\n"); 
				break; 
			case -1: 
				scan_status = -1;
				printf("-1:Site-survey request failed and restart %s!\n",ifname);
				/*lrl 2018/4/8 概率出现无线扫描不到任何信号，初步分析为接口状态有问题
				  还没找到根因，暂时这样规避*/
				RunSystemCmd(0, "ifconfig", ifname, "down", "");
				RunSystemCmd(0, "ifconfig", ifname, "up", "");
				break; 
			default: 
				break; 
		} 
		if( 0 != scan_status  && retry++ < WIFI_SCAN_RETRY_TIMES)
		{
			diag_printf("scan retry=%d\n",retry);
			goto recreate;
		}
		retry = 0;
		//生成页面所需数据
		if(0 == scan_status)
		{
			while(255 == createWlSiteSurveyList(scan_list,ifname))
			{
				if(WIFI_SCAN_RETRY_TIMES * 2 == retry++ )//扫描重试为3次，获取结果重试次数为6
				{
					return RET_ERR;
				}
				sleep(1);
			}
			return RET_SUC;
		}
		else
		{
			return RET_ERR;
		}
}


/*只有本文件里面用*/
static PI8 tpi_wifi_get_cur_channel(WLAN_RATE_TYPE wl_rate)
{
	PI8 ifname[PI_BUFLEN_32] = {0};
	PIU8 channel = 0;
	bss_info bss;

	if(WLAN_RATE_24G == wl_rate)
	{
		strncpy(ifname,TENDA_WLAN24_AP_IFNAME,sizeof(ifname)-1);
	}
	else if(WLAN_RATE_5G == wl_rate)
	{
		strncpy(ifname,TENDA_WLAN5_AP_IFNAME,sizeof(ifname)-1);
	}
	
	if ( getWlBssInfo(ifname, &bss) < 0) 
	{
		printf("Get bssinfo failed!");
	}
	else
	{
		//printf("bss.channel:%d",bss.channel);
		//sprintf(buf,"%d",bss.channel);
		channel = bss.channel;
	}

	return channel;
}


static PI8 tpi_wifi_get_cur_bandwidth(WLAN_RATE_TYPE wl_rate)
{
	PI8 ifname[PI_BUFLEN_32] = {0};
	PIU8 bandwidth = 20;
	bss_info bss;

	if(WLAN_RATE_24G == wl_rate)
	{
		strncpy(ifname,TENDA_WLAN24_AP_IFNAME,sizeof(ifname)-1);
	}
	else if(WLAN_RATE_5G == wl_rate)
	{
		strncpy(ifname,TENDA_WLAN5_AP_IFNAME,sizeof(ifname)-1);
	}
	
	if ( getWlBssInfo(ifname, &bss) < 0) 
	{
		printf("Get bssinfo failed!");
	}
	else
	{
		if(2 == bss.wide)
		{
			bandwidth = 80;
		}
		else if(1 == bss.wide)
		{
			bandwidth = 40;
		}
		else
		{
			bandwidth = 20;
		}
	}
	return bandwidth;
}


RET_INFO tpi_wifi_update_basic_info(WIFI_SSID_CFG *cfg,PI8 *prefix)
{
	PI8 name[PI_BUFLEN_32] = {0},tmp[PI_BUFLEN_64] = {0};

	if(NULL == cfg || NULL == prefix)
	{
		return RET_ERR;
	}
	
	sprintf(name,"%s_",prefix);

/*SSID开关*/
	//cfg->bss_enable = atoi(nvram_safe_get(strcat_r(prefix,"bss_enabled",tmp)));
	cfg->bss_enable = atoi(nvram_safe_get(strcat_r(name,"radio",tmp)));//使用radio参数控制页面的显示
	
/*SSID隐藏开关*/
	cfg->ssid_hide = atoi(nvram_safe_get(strcat_r(name,"closed",tmp)));		
	
/*SSID*/
	strcpy__(cfg->ssid,nvram_safe_get(strcat_r(name,"ssid",tmp)));

/*加密*/
	PI8 akm[PI_BUFLEN_16] = {0};
	strcpy__(akm,nvram_safe_get(strcat_r(name,"akm",tmp)));
	if(0 == strcmp(akm,""))
	{
		strcpy__(cfg->security,"none");
	}
	else if(0 == strcmp(akm,"0"))
	{
		strcpy__(cfg->security,"wep");

		PI8 auth[PI_BUFLEN_4] = {0};
		strcpy__(auth,nvram_safe_get(strcat_r(name,"auth",tmp)));

		if(0 == strcmp(auth,"1"))
		{
			strcpy__(cfg->wep_type,"shared");
		}
		else
		{
			strcpy__(cfg->wep_type,"open");			
		}

		cfg->wep_key = atoi(nvram_safe_get(strcat_r(name,"key",tmp)));		
		
		strcpy__(cfg->wep_key_arry[0],nvram_safe_get(strcat_r(name,"key1",tmp)));
		strcpy__(cfg->wep_key_arry[1],nvram_safe_get(strcat_r(name,"key2",tmp)));
		strcpy__(cfg->wep_key_arry[2],nvram_safe_get(strcat_r(name,"key3",tmp)));
		strcpy__(cfg->wep_key_arry[3],nvram_safe_get(strcat_r(name,"key4",tmp)));		
	}
	else
	{
		strcpy__(cfg->security,"wpapsk");
		
		strcpy__(cfg->wpa_psk_type,akm);	
		strcpy__(cfg->wap_psk_crypto,nvram_safe_get(strcat_r(name,"crypto",tmp)));
		strcpy__(cfg->wpa_psk_key,nvram_safe_get(strcat_r(name,"wpa_psk",tmp)));
	}

	return RET_SUC;
}

static RET_INFO tpi_wifi_operator(WL_OPERATOR op,char *ifname)
{
	int opera_all_interface = 0;
	
	if(NULL == ifname || 0 == strcmp(ifname,""))
	{
		opera_all_interface = 1;
	}
	
	if(WLAN_STOP == op || WLAN_RESTART == op)
	{
		if(opera_all_interface)
		{
			tpi_wifi_stop_handle(TENDA_WLAN24_AP_IFNAME);
			tpi_wifi_stop_handle(TENDA_WLAN5_AP_IFNAME);
		}else
		{
			tpi_wifi_stop_handle(ifname);
		}
	}

	if(WLAN_START== op || WLAN_RESTART == op)
	{
		if(opera_all_interface)
		{
			tpi_wifi_start_handle(TENDA_WLAN24_AP_IFNAME);
			tpi_wifi_start_handle(TENDA_WLAN5_AP_IFNAME);
		}else
		{
			tpi_wifi_start_handle(ifname);
		}
	}
#ifdef __CONFIG_GUEST__
	tpi_wifi_config_guest();
#endif
	PI_PRINTF(TPI,"stop success!\n");	
	return RET_SUC;
}

RET_INFO tpi_wifi_check_ip_confilct_by_wan(unsigned int wan_ip,unsigned int wan_mask)
{
	if(check_guest_net_with_other(wan_ip, wan_mask))
	{
		if(INTERFACE_UP == get_interface_state(TENDA_WLAN24_GUEST_IFNAME))
		{
			tenda_ifconfig(TENDA_WLAN24_GUEST_IFNAME, 0,NULL,NULL);
			tenda_ifconfig(TENDA_WLAN24_GUEST_IFNAME, IFUP,NULL,NULL);
		}
		if(INTERFACE_UP == get_interface_state(TENDA_WLAN5_GUEST_IFNAME))
		{
			tenda_ifconfig(TENDA_WLAN5_GUEST_IFNAME, 0,NULL,NULL);
			tenda_ifconfig(TENDA_WLAN5_GUEST_IFNAME, IFUP,NULL,NULL);
		}
		tpi_wifi_operator(WLAN_RESTART,TENDA_WLAN24_GUEST_IFNAME);
		tpi_wifi_operator(WLAN_RESTART,TENDA_WLAN5_GUEST_IFNAME);
		return 1;
	}
	return 0;
}
