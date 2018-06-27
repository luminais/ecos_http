/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "debug.h"
#include "wan_surf_check.h"

#define WAN_SURFING_CHECK_SIZE	8*1024
static cyg_handle_t wan_surfing_daemon_handle;
static char wan_surfing_daemon_stack[WAN_SURFING_CHECK_SIZE];
static cyg_thread wan_surfing_daemon_thread;

static WAN_SURF_CHECK_INFO_STRUCT wan_surf_check_info;

#ifdef __CONFIG_CE_POWER__
#define POWER_LEVEL_HIGHT 0
#define POWER_LEVEL_NORMAL	3
#define POWER_LEVEL_LOW		6
#define SET_CE_POWER	0
#define SET_NORMAL_POWER	1
static int power_limit_flag = 1;
int ce_power_cli_flag = 0;
typedef enum ce_power_level{
	CE_POWER_LOW = 1,
	CE_POWER_NORMAL,
	CE_POWER_HIGH
}ce_power_level;
int is_ce_power_limit();
static int CE_set_current_power(const char *interface,int cur_level,int flag);
static int CE_get_current_power_config(const char *name);
#endif
/*以下函数用于api调用*/
RET_INFO tpi_wan_surf_check_update_info()
{
	PI8 value[PI_BUFLEN_128] = {0};
	PI8 *token = NULL, *value_p = value;
	PIU8 i = 0,ntp_num = 0;

	wan_surf_check_info.enable = 1;

	strncpy(value, nvram_safe_get(NVRAMNAME_NTP_SERVER), sizeof(value)-1);
	
#if GETHOSTBYNAME_DONAME_EN	
	ntp_num = PING_SERVER_IP_NUM;
#else
	ntp_num = PING_SERVER_IP_NUM - 4;
#endif

	value_p = value;
	for (i = 0; i < ntp_num; i++) 
	{
		token = strsep(&value_p, " ");
		if (NULL == token)
			break;

		strncpy(wan_surf_check_info.ping_server_ip[i],token,PI_IP_STRING_LEN-1);
	}
	
#if GETHOSTBYNAME_DONAME_EN
	value_p = nvram_safe_get(NVRAMNAME_WL_COUNTRYCODE);
	if(strlen(value_p) < 4)
	{
		if(0 == strcasecmp(value_p,"ALL") || 0 == strcasecmp(value_p,"CN"))
		{
			strcpy__(wan_surf_check_info.ping_hostname[0],"www.baidu.com");
			strcpy__(wan_surf_check_info.ping_hostname[1],"www.qq.com");
		}
		else
		{
			strcpy__(wan_surf_check_info.ping_hostname[0],"www.google.com");
			strcpy__(wan_surf_check_info.ping_hostname[1],"www.microsoft.com");				
		}
	}
#else
	/*决策评审加上PING 180.76.76.76 223.5.5.5 119.29.29.29 114.114.114.114*/
	strcpy__(wan_surf_check_info.ping_server_ip[ntp_num],"180.76.76.76");
	strcpy__(wan_surf_check_info.ping_server_ip[ntp_num + 1],"223.5.5.5");
	strcpy__(wan_surf_check_info.ping_server_ip[ntp_num + 2],"119.29.29.29");
	strcpy__(wan_surf_check_info.ping_server_ip[ntp_num + 3],"114.114.114.114");		
#endif

	wan_surf_check_info.init_value = 1;
	return RET_SUC;
}

RET_INFO tpi_wan_surf_check_struct_init()
{
	memset(&wan_surf_check_info,0x0,sizeof(wan_surf_check_info));
	return tpi_wan_surf_check_update_info();
}

RET_INFO tpi_wan_surf_check_first_init()
{
	if(wan_surf_check_info.enable)
		tpi_wan_surf_check_start();
	
	return RET_SUC;
}
/*添加线程退出接口函数lq*/
static int check_wan_surf_exit_flag = 0;
inline void set_wan_surf_exit_flag(int flag)
{
	check_wan_surf_exit_flag = flag;
}

inline int get_wan_surf_exit_flag()
{
	return check_wan_surf_exit_flag;
}

RET_INFO tpi_wan_surf_check_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			ret = tpi_wan_surf_check_start();
			break;
		case OP_STOP:
			ret = tpi_wan_surf_check_stop();
			break;
		case OP_RESTART:
			ret = tpi_wan_surf_check_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}

/*以下用于gpi获取信息函数*/
INTERNET_INFO tpi_wan_surf_check_result()
{
	return wan_surf_check_info.check_result;
}

/*以下为该模块具体执行实现函数*/
PIU8 tpi_wan_surf_check_get_exit_ping()
{	
	return wan_surf_check_info.exit_ping_tag;
}

RET_INFO tpi_wan_surf_check_set_exit_ping(PIU8 enable)
{
	if(1 == enable)
	{
#if GETHOSTBYNAME_DONAME_EN
		set_dns_quit_exit_tag(1);
#endif
		wan_surf_check_info.exit_ping_tag = 1;
	}
	else
	{
#if GETHOSTBYNAME_DONAME_EN
		set_dns_quit_exit_tag(0);
#endif
		wan_surf_check_info.exit_ping_tag = 0;		
	}
	return RET_SUC;
}

RET_INFO tpi_wan_surf_check_wait_exit_ping()
{
	while(1)
	{	
		if(0 == tpi_wan_surf_check_get_exit_ping())
			break;
#if GETHOSTBYNAME_DONAME_EN
		if(0 == get_dns_quit_exit())
			set_dns_quit_exit_tag(1);
#endif
		cyg_thread_delay(RC_MODULE_1S_TIME/10);		
	}
	return RET_SUC;
}
/*在wan_mode_check等其他线程可能会用到*/
RET_INFO tpi_wan_surf_check_set_result(INTERNET_INFO flag)
{
	tpi_wan_surf_check_set_exit_ping(1);

	wan_surf_check_info.check_result = flag;
	
	return RET_SUC;
}

RET_INFO tpi_wan_surf_check_ping_connet_check()
{
	RET_INFO ret = RET_ERR;
	PIU32 dst = 0;
	PIU8 i = 0;
	PI8 *ip_addr = NULL;
	PI8 ping_ip[PI_IP_STRING_LEN] = {0},ip_str[4][4] = {0};
	int cur_power_2g = -1;
	int cur_power_5g = -1;
	
	if (0 == wan_surf_check_info.init_value)
	{
		tpi_wan_surf_check_struct_init();
	}

	for(i = 0;i < (1+PING_SERVER_IP_NUM+PING_GET_HOSTNAME_NUM);i++)
	{
		if(1 == tpi_wan_surf_check_get_exit_ping())
			break;
		#ifdef __CONFIG_CE_POWER__
		if(is_ce_power_limit())
		{
			cur_power_2g = CE_get_current_power_config(WLAN24G_CURRENT_POWER);
			cur_power_5g = CE_get_current_power_config(WLAN5G_CURRENT_POWER);
			//设置为CE功率
			CE_set_current_power(TENDA_WLAN24_AP_IFNAME,cur_power_2g,SET_CE_POWER);
			CE_set_current_power(TENDA_WLAN5_AP_IFNAME,cur_power_5g,SET_CE_POWER);
		}
		#endif
		
		if(0 == i)
		{
			strncpy(ping_ip,wan_surf_check_info.ping_ok_ip,PI_IP_STRING_LEN);
		}
		else if((PING_SERVER_IP_NUM + 1)> i)
		{
			strncpy(ping_ip,wan_surf_check_info.ping_server_ip[i-1],PI_IP_STRING_LEN);	
		}
		else
		{
			strcpy__(ping_ip,"");
#if GETHOSTBYNAME_DONAME_EN	
			RC_MODULE_DEBUG(RC_WAN_SURF_CHECK_MODULE,TPI,"gethostbyname hostname:%s\n",wan_surf_check_info.ping_hostname[i - (PING_SERVER_IP_NUM + 1)])
			if((ip_addr = tpi_wan_surf_check_gethostbyname(wan_surf_check_info.ping_hostname[i - (PING_SERVER_IP_NUM + 1)])) != NULL)
			{
				strncpy(ping_ip,ip_addr,PI_IP_STRING_LEN);	
				
				sscanf(ping_ip,"%[^.].%[^.].%[^.].%s",ip_str[0],ip_str[1],ip_str[2],ip_str[3]);
				
				if(RET_ERR == tpi_wan_surf_check_check_addr(atoi(ip_str[0]),atoi(ip_str[1])))
					strcpy__(ping_ip,"");
			}
#endif	
		}

		if(1 == tpi_wan_surf_check_get_exit_ping())
			break;
		
		RC_MODULE_DEBUG(RC_WAN_SURF_CHECK_MODULE,TPI,"PING ip:%s\n",ping_ip)

		if(strlen(ping_ip) >= 7)
		{
			dst = inet_addr(ping_ip);
			dst = ntohl(dst);
			
			if(0 == icmp_ping(dst, 0, PING_DELAY))
			{
				RC_MODULE_DEBUG(RC_WAN_SURF_CHECK_MODULE,TPI,"PING ok ip:%s\n",ping_ip)
				strncpy(wan_surf_check_info.ping_ok_ip,ping_ip,PI_IP_STRING_LEN);
				ret = RET_SUC;
				break;
			}
		}		
	}
	
	return ret;
}

/*只有本文件里面用*/
#if GETHOSTBYNAME_DONAME_EN
static RET_INFO tpi_wan_surf_check_check_addr(PIU8 ip0,PIU8 ip1)
{
	if(ip0 != 10 && ip0 != 172 && ip0 != 192)
	{
		return RET_SUC;
	}
	else
	{
		if((ip0 == 172) && ((ip1 < 16) || (ip1>31)))
		{
			return RET_SUC;
		}
		if((ip0 == 192) && (ip1 != 168))
		{
			return RET_SUC;
		}
		return RET_ERR;
	}
}

static PI8 * tpi_wan_surf_check_gethostbyname(char *str)
{
	struct hostent *hptr;
	static PI8 dst[32] = {0};
	if((hptr = gethostbyname(str)) == NULL)
	{
		return NULL;
	}
	else
	{
		inet_ntop(hptr->h_addrtype,hptr->h_addr,dst,sizeof(dst));
		return dst;
	}
}
#endif
/*添加线程等待的接口函数lq*/
static int wan_surfing_delay_time(int times)
{
	int i = 0;
	for(i = 0; i< times;i++)
	{
		if(1 == get_wan_surf_exit_flag())
			return 1;
		cyg_thread_delay(RC_MODULE_1S_TIME);
	}
	return 0;
}

#ifdef __CONFIG_CE_POWER__
int get_ce_power_cli_flag()
{
	return ce_power_cli_flag;
}
void set_ce_power_cli_flag(int value)
{
	ce_power_cli_flag = value;
	return ;
}
int is_ce_power_limit()
{
	return power_limit_flag;
}
static int ce_power_limit_set(int value)
{
	power_limit_flag = value;
}

static int CE_get_current_power_config(const char *name)
{
	int power_level = -1;
	
	char wlan_power[16] = {0};
	char country_power[16] = {0};	//页面功率等级: 100:低  92,100:中	78,92,100:高
	if(NULL == name)
	{
		return -1;
	}
	
	strcpy(wlan_power,nvram_safe_get(name));
	if(0 == strcmp("high",wlan_power))
	{
		power_level = POWER_LEVEL_HIGHT;
	}
	else if(0 == strcmp("normal",wlan_power))
	{
		power_level = POWER_LEVEL_NORMAL;
	}
	else if(0 == strcmp("low",wlan_power))
	{
		power_level = POWER_LEVEL_LOW;
	}
	else
	{
		return -1;
	}
	return power_level;
}
static int CE_set_current_power(const char *interface,int cur_level,int flag)
{
	char buf[64];
    char *nvramval=NULL;
	if(!interface)
	{
		return -1;
	}
	//flag为0表示要降低到CE功率
	if(0 == flag)
	{
		sprintf((char *)buf, "-0%c%d", ',',cur_level);
		RunSystemCmd(0, "iwpriv", interface,"rf_pwr",buf,"");
	}
	else if(1 == flag)
	{
		sprintf((char *)buf, "+0%c%d", ',',cur_level);
		RunSystemCmd(0, "iwpriv", interface,"rf_pwr",buf, "");
	}
	else
	{
		printf("func:%s line:%d cmd argument is error\n",__func__,__LINE__);
		return -1;
	}
	return 0;
}
#endif
static RET_INFO tpi_wan_surfing_check_main()
{  
	PIU8 i = 0,not_online_count = 0,online_count = 0,connect_change_tag = 1,ping_result = 0;
#ifdef __CONFIG_CE_POWER__
	int cur_power_2g = -1;
	int cur_power_5g = -1;
	int ce_power_tag = 1;
	PI8 wl_mode[16] = {0};

	char FinishTestFlag[64] = {0};

	strcpy(FinishTestFlag,nvram_safe_get("FinishTestFlag"));
#endif

	while(1)
	{
		//判断线程推出标志是否为请求退出lq
		
		if(1 == tpi_wan_surf_check_get_exit_ping())
		{
			tpi_wan_surf_check_set_exit_ping(0);
		}
#ifdef __CONFIG_CE_POWER__
		//产测未完成之前不做降功率操作
		if(0 == strcmp("0",FinishTestFlag))
		{
			ce_power_limit_set(0);
		}
		
		strcpy(wl_mode,nvram_safe_get(SYSCONFIG_WORKMODE));
		if((strcmp(wl_mode,"wisp") == 0) || (strcmp(wl_mode,"client+ap") == 0))
		{
			ce_power_limit_set(0);		
		}
	
		if(is_ce_power_limit())
		{
			cur_power_2g = CE_get_current_power_config(WLAN24G_CURRENT_POWER);
			cur_power_5g = CE_get_current_power_config(WLAN5G_CURRENT_POWER);
			//设置为CE功率
			CE_set_current_power(TENDA_WLAN24_AP_IFNAME,cur_power_2g,SET_CE_POWER);
			CE_set_current_power(TENDA_WLAN5_AP_IFNAME,cur_power_5g,SET_CE_POWER);
		}
		if(1 == get_ce_power_cli_flag())
		{
			if(is_ce_power_limit())
			{
				cur_power_2g = CE_get_current_power_config(WLAN24G_CURRENT_POWER);
				cur_power_5g = CE_get_current_power_config(WLAN5G_CURRENT_POWER);
				//恢复正常配置的功率
				CE_set_current_power(TENDA_WLAN24_AP_IFNAME,cur_power_2g,SET_NORMAL_POWER);
				CE_set_current_power(TENDA_WLAN5_AP_IFNAME,cur_power_5g,SET_NORMAL_POWER);
				ce_power_limit_set(0);		//解除功率限制
			}
		}
#endif
		
		/*1.如果是未连接，则将联网状态置换成未联网
		* 2.如果是连接中，则将联网状态置换成联网中	
		* 3.如果是已连接且是状态切换的首次，则立马将联网状态置成已联网，
	    *然后再进行PING或者域名解析操作再最终确定是否能够联网
	    *之后检测间隔时间为15*RC_MODULE_1S_TIME
		*/
		if(strcmp(nvram_safe_get("wan0_connect"),"Disconnected") == 0
			|| strcmp(nvram_safe_get("wan0_connect"),"") == 0)
		{
			wan_surf_check_info.check_result = INTERNET_NO;
			connect_change_tag = 1;
			cyg_thread_delay(RC_MODULE_1S_TIME);
			continue;
		}
		else if(strcmp(nvram_safe_get("wan0_connect"),"Connecting") == 0)
		{
			wan_surf_check_info.check_result = INTERNET_TRY;
			connect_change_tag = 1;
			cyg_thread_delay(RC_MODULE_1S_TIME);
			continue;
		}

		if(1 == connect_change_tag)
		{
			if(nvram_match(WAN0_PROTO,"static"))
			{
				wan_surf_check_info.check_result = INTERNET_TRY;
			}
			else
			{
				wan_surf_check_info.check_result = INTERNET_YES;
			}
			connect_change_tag = 0;
		}
		
		/*检测能够联网次数为ONLINE_CONFIRM_CHECKCOUNT，检测不能联网次数为NO_ONLINE_CONFIRM_CHECKCOUNT*/
		for(not_online_count = 0,online_count = 0;
			not_online_count <  NO_ONLINE_CONFIRM_CHECKCOUNT && online_count < ONLINE_CONFIRM_CHECKCOUNT;)
		{
			ping_result = tpi_wan_surf_check_ping_connet_check();
			
			if(1 == tpi_wan_surf_check_get_exit_ping())
				break;
			
			if(RET_SUC == ping_result)
			{
				online_count++;
				not_online_count = 0;
				if(INTERNET_YES == wan_surf_check_info.check_result)
				{
					break;
				}
				cyg_thread_delay(RC_MODULE_1S_TIME);
			}
			else
			{
				online_count = 0;		
				not_online_count++;	
				#ifdef __CONFIG_CE_POWER__
				if(is_ce_power_limit())
				{
					cur_power_2g = CE_get_current_power_config(WLAN24G_CURRENT_POWER);
					cur_power_5g = CE_get_current_power_config(WLAN5G_CURRENT_POWER);
					//设置为CE功率
					CE_set_current_power(TENDA_WLAN24_AP_IFNAME,cur_power_2g,SET_CE_POWER);
					CE_set_current_power(TENDA_WLAN5_AP_IFNAME,cur_power_5g,SET_CE_POWER);
					
				}
				#endif
				//判断线程推出标志是否为请求退出lq
				if(wan_surfing_delay_time(5))
				{
					break;
				}
			}
		}
			
		if(1 == tpi_wan_surf_check_get_exit_ping())
		{
			tpi_wan_surf_check_set_exit_ping(0);
			//判断线程推出标志是否为请求退出lq
			if(wan_surfing_delay_time(5))
			{
				break;
			}			
		}
		else
		{
			if(online_count)
			{
				wan_surf_check_info.check_result = INTERNET_YES;
#ifdef __CONFIG_CE_POWER__
				if(is_ce_power_limit())
				{
					cur_power_2g = CE_get_current_power_config(WLAN24G_CURRENT_POWER);
					cur_power_5g = CE_get_current_power_config(WLAN5G_CURRENT_POWER);
					//恢复正常配置的功率
					CE_set_current_power(TENDA_WLAN24_AP_IFNAME,cur_power_2g,SET_NORMAL_POWER);
					CE_set_current_power(TENDA_WLAN5_AP_IFNAME,cur_power_5g,SET_NORMAL_POWER);
					ce_power_limit_set(0);		//联网成功之后解除功率限制
				}
#endif
			}
			else
			{
				wan_surf_check_info.check_result = INTERNET_NO;
			}
			
			RC_MODULE_DEBUG(RC_WAN_SURF_CHECK_MODULE,TPI,"check result:%d!\n",wan_surf_check_info.check_result);
			//判断线程推出标志是否为请求退出lq
			if(wan_surfing_delay_time(15))
			{
				break;
			}
		}	
	}
	return 0;
}

static RET_INFO tpi_wan_surf_check_start()
{
	RET_INFO ret = RET_SUC;

	if(wan_surfing_daemon_handle == 0)
	{
		tpi_wan_surf_check_update_info();
		if(wan_surf_check_info.enable)
		{
			cyg_thread_create(
				8, 
				(cyg_thread_entry_t *)tpi_wan_surfing_check_main,
				0, 
				"wan_surfing_check",
				wan_surfing_daemon_stack, 
				sizeof(wan_surfing_daemon_stack), 
				&wan_surfing_daemon_handle, 
				&wan_surfing_daemon_thread);
			
			cyg_thread_resume(wan_surfing_daemon_handle);     
			cyg_thread_delay(1);
			PI_PRINTF(TPI,"start success!\n");
		}
		else
		{
			PI_ERROR(TPI,"the mib is off, connot start!\n");
		}
	}
	else
	{
		PI_PRINTF(TPI,"is already start!\n");			
	}
	
	return ret;
}

static RET_INFO tpi_wan_surf_check_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;

	if(wan_surfing_daemon_handle != 0)
	{
		tpi_wan_surf_check_set_exit_ping(1);		
		tpi_wan_surf_check_wait_exit_ping();
		//设置退出标志为退出lq
		set_wan_surf_exit_flag(1);
		/* Wait until thread exit */
		pid = oslib_getpidbyname("wan_surfing_check");
		if (pid)
		{
			while(oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}
		
		cyg_thread_delete(wan_surfing_daemon_handle);
		
		PI_PRINTF(TPI,"stop success!\n");
		set_wan_surf_exit_flag(0);
		wan_surfing_daemon_handle = 0;
	}
	else
	{
		PI_PRINTF(TPI,"is already stop!\n");		
	}
	
	return ret;
}

static RET_INFO tpi_wan_surf_check_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_wan_surf_check_stop() || RET_ERR == tpi_wan_surf_check_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;
}
