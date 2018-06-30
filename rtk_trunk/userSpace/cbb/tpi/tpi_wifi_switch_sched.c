/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <time.h>

#include "debug.h"
#include "common.h"
#include "wifi.h"
#include "wifi_switch_sched.h"

#include "sys_module.h"


#define WIFI_SWITCH_SCHED_CHECK_SIZE	4*1024
static cyg_handle_t wifi_switch_sched_daemon_handle;
static PI8 wifi_switch_sched_daemon_stack[WIFI_SWITCH_SCHED_CHECK_SIZE];
static cyg_thread wifi_switch_sched_daemon_thread;

static WIFI_SWITCH_SCHED_CHECK_INFO_STRUCT wifi_switch_sched_info;
struct wifi_timer gWifiTimer = {-1, -1};

PI32 gWifiSchedDebug = 0;
PI32 gWifiStatusConfig = 0;

static void tpi_wifi_switch_sched_show_info(P_WIFI_SWITCH_SCHED_CHECK_INFO_STRUCT pcfg);
static RET_INFO tpi_wifi_switch_sched_check_gWifiTimer(void);
static PIU8 tpi_wifi_switch_sched_check_wifishec_pretimer(PI32 tm_wday, PI32 time );
static WL_RADIO_TYPE tpi_wifi_switch_sched_check_time_area(PI32 * sleep_time, struct tm *pt, P_WIFI_SWITCH_SCHED_CHECK_INFO_STRUCT wifi_sched_cfg);
static WL_RADIO_TYPE tpi_wifi_switch_sched_check_time_in_range(PI32 * sleep_time, P_WIFI_SWITCH_SCHED_CHECK_INFO_STRUCT wifi_sched_cfg);
static RET_INFO tpi_wifi_switch_sched_main();
static RET_INFO tpi_wifi_switch_sched_start();
static RET_INFO tpi_wifi_switch_sched_stop();
static RET_INFO tpi_wifi_switch_sched_restart();

/*以下函数用于api调用*/
RET_INFO tpi_wifi_switch_sched_update_info()
{	
	PI8 mib_name[64];
	PI8 *mib_val;
	PI32 num;
	PI32 i,w;
	PI32 zone_hh,zone_mm;
	PI32 sec,min,hour;
	PI32 s_week,e_week;
	PI8 week[16]={0};
	PI32 esat_time_zone_flag = 1;
	PI8 *ptr;

	gWifiTimer.wday = -1;
	gWifiTimer.time = -1;

	memset(&wifi_switch_sched_info,0x0,sizeof(wifi_switch_sched_info));
	
	mib_val = nvram_safe_get(WLAN_PUBLIC_SCHEDULE_ENABLE);
	wifi_switch_sched_info.enable = atoi(mib_val);

	mib_val = nvram_safe_get(WLAN_PUBLIC_SCHEDULE_LIST_NUM);
	wifi_switch_sched_info.sche_count = atoi(mib_val);

	if( atoi(mib_val) > WLAN_SCHEDULE_LIST_MAX || atoi(mib_val) == 0)
	{
		PI_ERROR(TPI,"schedule count[ %d ] wrong\n",atoi(mib_val));
		return RET_ERR;	
	}
	num = atoi(mib_val);
	for(i = 1; i <= num; i++)
	{
		sprintf(mib_name,WLAN_PUBLIC_SCHEDULE_OFFTIME_LIST,i);
	
		mib_val = nvram_safe_get(mib_name);
		if (0 == strcmp(mib_val,""))
			continue;
		if(strchr(mib_val,'+'))
			sscanf(mib_val,"UTC+%d:%d %d %d %d ? * %s",&zone_hh,&zone_mm,&sec,&min,&hour,week);
		else if(strchr(mib_val,'-'))
		{
			esat_time_zone_flag = 0;
			sscanf(mib_val,"UTC-%d:%d %d %d %d ? * %s",&zone_hh,&zone_mm,&sec,&min,&hour,week);
		}
		else
			continue;
		
		wifi_switch_sched_info.timer_period_offtime[i-1].is_time_zone_east = esat_time_zone_flag;
		wifi_switch_sched_info.timer_period_offtime[i-1].time_zone = zone_hh;
		wifi_switch_sched_info.timer_period_offtime[i-1].timer = hour*60*60 + min*60 + sec;
		
		if(strchr(week,'-')){
			sscanf(week,"%d-%d",&s_week,&e_week);
			for(w=s_week;w<=e_week;w++)
			{
				if( w>=1 && w<=7)
					wifi_switch_sched_info.timer_period_offtime[i-1].select_week[w-1] = 1;
			}
		}
		else
		{
			ptr = week; 
			while( *ptr!= '\0')
			{
				switch(*ptr)
				{	
					case ',':
						break;
					case '-':
						break;
					case '8':
					case '9':
					case '0':
						PI_PRINTF(TPI,"illegal PI8acter %c\n",*ptr);
						break;
					default:
						if(*ptr >= '1' && *ptr <= '7')//星期天到星期六
						{
							wifi_switch_sched_info.timer_period_offtime[i-1].select_week[atoi(ptr) - 1] = 1;
						}
						break;
				}
				ptr++;
			}
		}
		sprintf(mib_name,WLAN_PUBLIC_SCHEDULE_ONTIME_LIST,i);
		mib_val = nvram_safe_get(mib_name);
		if (0 == strcmp(mib_val,""))
			continue;
		if(strchr(mib_val,'+'))
			sscanf(mib_val,"UTC+%d:%d %d %d %d ? * %s",&zone_hh,&zone_mm,&sec,&min,&hour,week);
		else  if(strchr(mib_val,'-'))
		{
			esat_time_zone_flag = 0;
			sscanf(mib_val,"UTC-%d:%d %d %d %d ? * %s",&zone_hh,&zone_mm,&sec,&min,&hour,week);
		}
		else
			continue;
		
		wifi_switch_sched_info.timer_period_ontime[i-1].is_time_zone_east = esat_time_zone_flag;
		wifi_switch_sched_info.timer_period_ontime[i-1].time_zone = zone_hh;
		wifi_switch_sched_info.timer_period_ontime[i-1].timer = hour*60*60 + min*60 + sec;
		if(strchr(week,'-')){
			sscanf(week,"%d-%d",&s_week,&e_week);
			for(w=s_week;w<=e_week;w++)
			{
				if( w>=1 && w<=7)
					wifi_switch_sched_info.timer_period_ontime[i-1].select_week[w-1] = 1;
			}
		}
		else
		{
			ptr = week; 
			while( *ptr!= '\0')
			{
				switch(*ptr)
				{	
					case ',':
						break;
					case '-':
						break;
					case '8':
					case '9':
					case '0':
						PI_PRINTF(TPI,"illegal PI8acter %c\n",*ptr);
						break;
					default:
						if(*ptr >= '1' && *ptr <= '7')//星期天到星期六
						{
							wifi_switch_sched_info.timer_period_ontime[i-1].select_week[atoi(ptr) - 1] = 1;
						}
						break;
				}
				ptr++;
			}
		}	
	}
	return RET_SUC;
}

RET_INFO tpi_wifi_switch_sched_struct_init()
{
	return tpi_wifi_switch_sched_update_info();
}

RET_INFO tpi_wifi_switch_sched_first_init()
{
	if(wifi_switch_sched_info.enable)
		tpi_wifi_switch_sched_start();
	
	return RET_SUC;
}
/*添加线程退出接口函数lq*/
static int check_wifi_switch_sched_exit_flag = 0;
inline void set_wifi_switch_sched_exit_flag(int flag)
{
	check_wifi_switch_sched_exit_flag = flag;
}

inline int get_wifi_switch_sched_exit_flag()
{
	return check_wifi_switch_sched_exit_flag;
}

/*类似于sntp时区更新需要同步通知相关线程*/
static int check_wifi_sched_main_continue_flag = 0;
inline void set_wifi_sched_main_continue_flag(int flag)
{
	check_wifi_sched_main_continue_flag = flag;
}

inline int get_wifi_sched_main_continue_flag()
{
	return check_wifi_sched_main_continue_flag;
}

RET_INFO tpi_wifi_switch_sched_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_wifi_switch_sched_start();
			break;
		case OP_STOP:
			tpi_wifi_switch_sched_stop();
			break;
		case OP_RESTART:
			tpi_wifi_switch_sched_restart();
			break;
		case OP_UPDATE:
			set_wifi_sched_main_continue_flag(1);
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}

/*以下用于gpi获取信息函数*/
RET_INFO tpi_wifi_switch_sched_web_info(PI8 *enable,PI8 *times,PI8 *weeks)
{
	if(NULL == enable || NULL == times || NULL == weeks)
		return RET_ERR;

	strcpy__(enable,nvram_safe_get(WLAN_PUBLIC_SCHEDULE_ENABLE));
	strcpy__(times,nvram_safe_get(WLAN_PUBLIC_SCHEDULE_TIME));
	strcpy__(weeks,nvram_safe_get(WLAN_PUBLIC_SCHEDULE_WEEK));

	return RET_SUC;	
}

/*以下为该模块具体执行实现函数*/

/*在wifi_switch_sched等其他线程可能会用到*/
void tpi_wifi_switch_sched_debug_level(PI32 level)
{
    if (level > 0)
        gWifiSchedDebug = 1;
    else
        gWifiSchedDebug = 0;

    PI_PRINTF(TPI,"wl_restart_check debug level = %d\n", gWifiSchedDebug);
}

/*只有本文件里面用*/
void tpi_wifi_switch_sched_show_info(P_WIFI_SWITCH_SCHED_CHECK_INFO_STRUCT pcfg)
{
	PI32 i;
	printf("###############tpi_wifi_switch_sched_show_info##################\n");
	printf("enable:			%d\n",pcfg->enable);
	printf("sche_count:		%d\n",pcfg->sche_count);
	if(pcfg->sche_count > 0)
		printf("++++++offtime++++++\n");
	for(i = 0; i < pcfg->sche_count; i++)
	{
		printf("offtime num %d\n",i+1);
		printf("timer: 	%d\n",pcfg->timer_period_offtime[i].timer);
		
		printf("select_week[sunday    ]: %d\n",pcfg->timer_period_offtime[i].select_week[0]);
		printf("select_week[Monday    ]: %d\n",pcfg->timer_period_offtime[i].select_week[1]);
		printf("select_week[Tuesday   ]: %d\n",pcfg->timer_period_offtime[i].select_week[2]);
		printf("select_week[Wednesday ]: %d\n",pcfg->timer_period_offtime[i].select_week[3]);
		printf("select_week[Thursday  ]: %d\n",pcfg->timer_period_offtime[i].select_week[4]);
		printf("select_week[Friday    ]: %d\n",pcfg->timer_period_offtime[i].select_week[5]);
		printf("select_week[Saturday  ]: %d\n",pcfg->timer_period_offtime[i].select_week[6]);
	}
	if(pcfg->sche_count > 0)
		printf("++++++ontime++++++\n");
	for(i = 0; i < pcfg->sche_count; i++)
	{
		printf("ontime num %d\n",i+1);
		printf("timer: 	%d\n",pcfg->timer_period_ontime[i].timer);
		printf("select_week[sunday    ]: %d\n",pcfg->timer_period_ontime[i].select_week[0]);
		printf("select_week[Monday    ]: %d\n",pcfg->timer_period_ontime[i].select_week[1]);
		printf("select_week[Tuesday   ]: %d\n",pcfg->timer_period_ontime[i].select_week[2]);
		printf("select_week[Wednesday ]: %d\n",pcfg->timer_period_ontime[i].select_week[3]);
		printf("select_week[Thursday  ]: %d\n",pcfg->timer_period_ontime[i].select_week[4]);
		printf("select_week[Friday    ]: %d\n",pcfg->timer_period_ontime[i].select_week[5]);
		printf("select_week[Saturday  ]: %d\n",pcfg->timer_period_ontime[i].select_week[6]);
	}
	
	printf("###############tpi_wifi_switch_sched_show_info end##################\n");
}

static RET_INFO tpi_wifi_switch_sched_check_gWifiTimer(void)
{
	time_t now;
	struct tm TM;

	if(gWifiTimer.wday == -1 || gWifiTimer.time == -1)
	{
		now = time(0);
		gmtime_r(&now,&TM);

		gWifiTimer.wday = TM.tm_wday;
		gWifiTimer.time = TM.tm_hour*60*60 + TM.tm_min*60 + TM.tm_sec;
	}
	
	return RET_SUC;
}

static PIU8 tpi_wifi_switch_sched_check_wifishec_pretimer(PI32 tm_wday, PI32 time )
{
	PIU8 find = 0;
	if(gWifiTimer.wday == -1 || gWifiTimer.time == -1)
	{
		gWifiTimer.wday = tm_wday;
		gWifiTimer.time = time;
		if(gWifiSchedDebug)
			PI_PRINTF(TPI,"update gWifiTimer\n");
	}
	
	if(gWifiTimer.wday != tm_wday || gWifiTimer.time != time)
	{
		gWifiTimer.wday = tm_wday;
		gWifiTimer.time = time;
		gWifiStatusConfig = 0;
		if(gWifiSchedDebug)
			PI_PRINTF(TPI,"got a wifi shec time %d\n",time);
	}
	else
	{
		if(gWifiStatusConfig)
		{
			find = 1;
		}
	}
	
	return find;
	
}

static WL_RADIO_TYPE tpi_wifi_switch_sched_check_time_area(PI32 * sleep_time, struct tm *pt, P_WIFI_SWITCH_SCHED_CHECK_INFO_STRUCT wifi_sched_cfg)
{
	WL_RADIO_TYPE wl_status = WL_WATING;
	TIME_UPDATE_RESULT wl_cur_status = WL_WATING;
	PI32 offtimer = 0,ontimer = 0;
	PI32 i = 0;
	PI32 curtime = pt->tm_hour*60*60 + pt->tm_min*60 + pt->tm_sec;
	PI32 on_time_pre = -1 , off_time_pre = -1;
	PI32 on_time_next = -1 , off_time_next = -1;
	PI32 yesterday;

	if (curtime < 0 || curtime > ONE_DAY_IN_SEC)
	{
		PI_PRINTF(TPI,"curtime[%d] is error .\n",curtime);
		(*sleep_time) = 5*60;
		return WL_WATING;
	}

	if(gWifiSchedDebug)
	{
		PI_PRINTF(TPI,"+++%s curtime:%d pt->tm_wday:%d+++\n",__func__,curtime,pt->tm_wday);
		PI_PRINTF(TPI,"curtime %d-%d-%d %d:%d:%d week:%d\n",pt->tm_year+1900,pt->tm_mon+1,pt->tm_mday,
			pt->tm_hour,pt->tm_min,pt->tm_sec,pt->tm_wday);
		PI_PRINTF(TPI,"+++%s sche_count:%d gWifiStatusConfig:%d+++\n",__func__,wifi_sched_cfg->sche_count,gWifiStatusConfig);
	}
	
	for(i=0; i<wifi_sched_cfg->sche_count; i++)
	{
		RC_MODULE_DEBUG(RC_WIFI_SWITCH_SCHED_MODULE,TPI,"[list%d]week[%d]:%d,curtime:%02d:%02d:%02d[%02d:%02d:%02d,%02d:%02d:%02d]\n",\
			i,pt->tm_wday,wifi_sched_cfg->timer_period_ontime[i].select_week[pt->tm_wday],curtime/3600,curtime%3600/60,curtime%3600%60,\
			(wifi_sched_cfg->timer_period_offtime[i].timer)/3600,(wifi_sched_cfg->timer_period_offtime[i].timer)%3600/60,\
			(wifi_sched_cfg->timer_period_offtime[i].timer)%3600%60,(wifi_sched_cfg->timer_period_ontime[i].timer)/3600,\
			(wifi_sched_cfg->timer_period_ontime[i].timer)%3600/60,(wifi_sched_cfg->timer_period_ontime[i].timer)%3600%60);

		if(wifi_sched_cfg->timer_period_ontime[i].select_week[pt->tm_wday] == 1)
		{
			ontimer = wifi_sched_cfg->timer_period_ontime[i].timer;
			if(curtime >= ontimer)
			{
				if(on_time_pre == -1)
					on_time_pre = ontimer;
				else if(ontimer > on_time_pre)
					on_time_pre = ontimer;		
			}
			else
			{
				if(on_time_next == -1)
					on_time_next = ontimer;
				else if(ontimer < on_time_next)
					on_time_next = ontimer;
			}

		}
		
		if(wifi_sched_cfg->timer_period_offtime[i].select_week[pt->tm_wday] == 1)
		{
			offtimer = wifi_sched_cfg->timer_period_offtime[i].timer;
			
			if(curtime >= offtimer)
			{
				if(off_time_pre == -1)
					off_time_pre = offtimer;
				else if(offtimer > off_time_pre)
					off_time_pre = offtimer;		
			}
			else
			{
				if(off_time_next == -1)
					off_time_next = offtimer;
				else if(offtimer < off_time_next)
					off_time_next = offtimer;
			}
		}
	}
	if(gWifiSchedDebug){
		PI_PRINTF(TPI,"%s %d on_time_pre:%d off_time_pre:%d\n",__func__,__LINE__, on_time_pre, off_time_pre);
		PI_PRINTF(TPI,"%s %d curtime:%d  on_time_next:%d off_time_next:%d\n",__func__,__LINE__,curtime, on_time_next, off_time_next);
	}
	if(on_time_next != -1 &&  off_time_next != -1)
	{
		if(on_time_next >= off_time_next)
			(*sleep_time)  = off_time_next - curtime + 5;
		else
			(*sleep_time)  = on_time_next - curtime + 5;
			
	}
	else if(on_time_next != -1 ||  off_time_next != -1)
	{
		if(on_time_next > off_time_next)
			(*sleep_time)  = on_time_next - curtime + 5;
		else
			(*sleep_time)  = off_time_next - curtime + 5;
	}
	else
		(*sleep_time)  = ONE_DAY_IN_SEC - curtime + 5;
	
	if(gWifiSchedDebug)
		PI_PRINTF(TPI,"sleep_time:%d \n",*sleep_time);
	if(on_time_pre != -1 || off_time_pre != -1)
	{
		if(on_time_pre >= off_time_pre)
		{
			wl_status = WL_ON;
			if(tpi_wifi_switch_sched_check_wifishec_pretimer(pt->tm_wday, on_time_pre))
			{
				if(gWifiSchedDebug)
					PI_PRINTF(TPI,"%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
				wl_status = wl_cur_status;
			}
		}
		else
		{
			wl_status = WL_OFF;
			if(tpi_wifi_switch_sched_check_wifishec_pretimer(pt->tm_wday, off_time_pre))
			{
				if(gWifiSchedDebug)
					PI_PRINTF(TPI,"%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
				wl_status = wl_cur_status;
			}
		}
		
	}
	else
	{
		if(pt->tm_wday == 0)
			yesterday = 6;
		else
			yesterday = pt->tm_wday - 1;
		
		for(i=0; i<wifi_sched_cfg->sche_count; i++)
		{
			if(wifi_sched_cfg->timer_period_ontime[i].select_week[yesterday] == 1)
			{
				ontimer = wifi_sched_cfg->timer_period_ontime[i].timer;
				
				if(on_time_pre == -1)
					on_time_pre = ontimer;
				else if(ontimer > on_time_pre)
					on_time_pre = ontimer;		
			
			}
			
			if(wifi_sched_cfg->timer_period_offtime[i].select_week[yesterday] == 1)
			{
				offtimer = wifi_sched_cfg->timer_period_offtime[i].timer;
				
				if(off_time_pre == -1)
					off_time_pre = offtimer;
				else if(offtimer > off_time_pre)
					off_time_pre = offtimer;		
				
			}
		}
		if(gWifiSchedDebug)
			PI_PRINTF(TPI,"%s %d on_time_pre:%d off_time_pre:%d\n",__func__,__LINE__, on_time_pre, off_time_pre);

		if(on_time_pre != -1 || off_time_pre != -1)
		{
			if(on_time_pre >= off_time_pre)
			{
				wl_status = WL_ON;
				if(tpi_wifi_switch_sched_check_wifishec_pretimer(yesterday, on_time_pre))
				{
					if(gWifiSchedDebug)
						PI_PRINTF(TPI,"%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
					wl_status = wl_cur_status;
				}
			}
			else
			{
				wl_status = WL_OFF;
				if(tpi_wifi_switch_sched_check_wifishec_pretimer(yesterday, off_time_pre))
				{
					if(gWifiSchedDebug)
						PI_PRINTF(TPI,"%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
					wl_status = wl_cur_status;
				}
			}
		}
		else
		{
			wl_status = WL_ON;
			if(gWifiStatusConfig)
			{
				if(gWifiSchedDebug)
					PI_PRINTF(TPI,"%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
				wl_status = wl_cur_status;
			}
			tpi_wifi_switch_sched_check_gWifiTimer();
		}
	}
	if(gWifiSchedDebug)
		PI_PRINTF(TPI,"+++%s wl_status:%d gWifiStatusConfig:%d+++\n",__func__,wl_status,gWifiStatusConfig);
	return wl_status;
}

static WL_RADIO_TYPE tpi_wifi_switch_sched_check_time_in_range(PI32 * sleep_time, P_WIFI_SWITCH_SCHED_CHECK_INFO_STRUCT wifi_sched_cfg)
{
	WL_RADIO_TYPE wifi_status = WL_ON;	
	time_t now;
	struct tm TM;
	
	now = time(0);
	gmtime_r(&now,&TM);

	wifi_status = tpi_wifi_switch_sched_check_time_area(sleep_time, &TM,wifi_sched_cfg);	
	
	return wifi_status;
}

static RET_INFO tpi_wifi_switch_sched_main()
{		
	TIME_UPDATE_RESULT time_update_status = TIME_UPDATE_FAIL;	
	PIU32 sleep_time = 45;
	PIU32 times = 0;
	time_t sys_pre_time = 0;
	PI8 msg_tmp[PI_BUFLEN_32] = {0};
	WL_RADIO_TYPE radio_status_to_change = WL_ON;
	WL_RADIO_TYPE wlan24g_now_status = WL_ON;
	WL_RADIO_TYPE wlan5g_now_status = WL_ON;

	//如果当前无线都是关闭的,则不开启WiFi定时
	if(WL_OFF == tpi_get_wlan24g_enable() && WL_OFF == tpi_get_wlan5g_enable())
	{
		PI_PRINTF(TPI,"Currently wireless(2.4g&5g) is off,close wifi_switch_sched\n");
		goto finish;
	}

	//关闭无线定时  注意: 联调时需要考虑app下发的开关
	if (0 == wifi_switch_sched_info.enable || 0 == wifi_switch_sched_info.sche_count)
	{
		wlan24g_now_status = get_interface_state(TENDA_WLAN24_AP_IFNAME);
		wlan5g_now_status = get_interface_state(TENDA_WLAN5_AP_IFNAME);
		
		if(INTERFACE_UP != wlan24g_now_status && WL_ON == tpi_get_wlan24g_enable())
		{
			/*起2.4G 无线接口*/
			snprintf(msg_tmp,sizeof(msg_tmp),"op=%d,wlan_ifname=%s",OP_START,TENDA_WLAN24_AP_IFNAME);
			msg_send(MODULE_RC,RC_WIFI_MODULE,msg_tmp);
		}
		if(INTERFACE_UP != wlan5g_now_status && WL_ON == tpi_get_wlan5g_enable())
		{
			/*起5G 无线接口*/
			snprintf(msg_tmp,sizeof(msg_tmp),"op=%d,wlan_ifname=%s",OP_START,TENDA_WLAN5_AP_IFNAME);
			msg_send(MODULE_RC,RC_WIFI_MODULE,msg_tmp);
		}
		PI_PRINTF(TPI,"wl wifictl disable,return!!!\n");
		
		goto finish;
	}
	
	while(1)
	{
		//判断线程推出标志是否为请求退出lq
		if(1 == get_wifi_switch_sched_exit_flag())
			break;
		//检测系统时间是否更新
		if (TIME_UPDATE_FAIL == time_update_status)
		{
			time_update_status = gpi_common_time_update_result();
			if(TIME_UPDATE_FAIL == time_update_status)	
			{
				sleep_time = 45;
				goto next;
			}
		}
		//检测系统时间是否在重启时间段中
		radio_status_to_change = tpi_wifi_switch_sched_check_time_in_range(&sleep_time, &wifi_switch_sched_info);
		wlan24g_now_status = get_interface_state(TENDA_WLAN24_AP_IFNAME) ? WL_OFF : WL_ON;
		wlan5g_now_status = get_interface_state(TENDA_WLAN5_AP_IFNAME) ? WL_OFF : WL_ON;
		
		PI_PRINTF(TPI,"wifi want to turn %s,now wifi_2.4g status is %s,now wifi_5g status is %s\n",
			radio_status_to_change == WL_ON?"ON":"OFF",wlan24g_now_status == WL_ON?"ON":"OFF",
			wlan5g_now_status == WL_ON?"ON":"OFF");
		/*2.4G*/	
		if(WL_ON == tpi_get_wlan24g_enable() 
			&& wlan24g_now_status != radio_status_to_change 
			&& WL_WATING != radio_status_to_change)
		{
			memset(msg_tmp,0x0,sizeof(msg_tmp));
			snprintf(msg_tmp,sizeof(msg_tmp),"op=%d,wlan_ifname=%s",
				radio_status_to_change == WL_ON ? OP_START : OP_STOP,TENDA_WLAN24_AP_IFNAME);
			msg_send(MODULE_RC,RC_WIFI_MODULE,msg_tmp);
#ifdef __CONFIG_GUEST__
			if(radio_status_to_change == WL_OFF)
			{
				if(atoi(nvram_safe_get("wl_guest_effective_time")))
				{
					nvram_set(WLAN24G_GUEST_ENABLE,"0");
				}	
			}
#endif 			
		}
		/*5G*/
		if(WL_ON == tpi_get_wlan5g_enable() 
			&& wlan5g_now_status != radio_status_to_change
			&& WL_WATING != radio_status_to_change)
		{
			memset(msg_tmp,0x0,sizeof(msg_tmp));
			snprintf(msg_tmp,sizeof(msg_tmp),"op=%d,wlan_ifname=%s",
				radio_status_to_change == WL_ON ? OP_START : OP_STOP,TENDA_WLAN5_AP_IFNAME);
			msg_send(MODULE_RC,RC_WIFI_MODULE,msg_tmp);
			
#ifdef __CONFIG_GUEST__
			if(radio_status_to_change == WL_OFF)
			{
				if(atoi(nvram_safe_get("wl_guest_effective_time")))
				{
					nvram_set(WLAN5G_GUEST_ENABLE,"0");
				}	
			}
#endif
		}					
next:	
		/*开启debug时,跳过sheep时间,以便快速验证*/
		if(RC_MODULE_DEBUG_ENABLE(RC_WIFI_SWITCH_SCHED_MODULE))
		{
			sleep_time = 5;
		}
		
		RC_MODULE_DEBUG(RC_WIFI_SWITCH_SCHED_MODULE,TPI," sleep_time[%d]s\n",sleep_time);	
		//将原有的休眠修改为1s为单位的循环，以保证线程退出最多等待1s,lq
		for(times = 0;times < sleep_time;times++)
		{
			if(1 == get_wifi_switch_sched_exit_flag())
				break;

			//sntp update time
			if(1 == get_wifi_sched_main_continue_flag())
			{
				set_wifi_sched_main_continue_flag(0);
				break;
			}
			cyg_thread_delay(RC_MODULE_1S_TIME);
		}
	}

finish:
	//删除此处，并警示不要再线程内部将自己县城句柄设置为0，这样在退出时删除该线程会出现死机
	//wifi_switch_sched_daemon_handle = 0;

	return RET_SUC;
}

static RET_INFO tpi_wifi_switch_sched_start()
{
	RET_INFO ret = RET_SUC;

	if(wifi_switch_sched_daemon_handle == 0)
	{
		tpi_wifi_switch_sched_update_info();
		
		if(gWifiSchedDebug)
			tpi_wifi_switch_sched_show_info(&wifi_switch_sched_info);

#if 0
		if(wifi_switch_sched_info.enable)
#else
		/*由于需要在关闭WIFI定时开关的时候打开WIFI，故这里需要不管开关都需要进去*/
		if(1)
#endif
		{
			cyg_thread_create(
				8, 
				(cyg_thread_entry_t *)tpi_wifi_switch_sched_main,
				0, 
				"wifi_switch_sched",
				wifi_switch_sched_daemon_stack, 
				sizeof(wifi_switch_sched_daemon_stack), 
				&wifi_switch_sched_daemon_handle, 
				&wifi_switch_sched_daemon_thread);
			
			cyg_thread_resume(wifi_switch_sched_daemon_handle);     
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

static RET_INFO tpi_wifi_switch_sched_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;

	if(wifi_switch_sched_daemon_handle != 0)
	{
		//设置退出标志为退出lq
		set_wifi_switch_sched_exit_flag(1);

		/* Wait until thread exit */
		pid = oslib_getpidbyname("wifi_switch_sched");
		if (pid)
		{
			while(oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}
		
		cyg_thread_delete(wifi_switch_sched_daemon_handle);
		
		PI_PRINTF(TPI,"stop success!\n");
		set_wifi_switch_sched_exit_flag(0);
		wifi_switch_sched_daemon_handle = 0;
	}
	else
	{
		PI_PRINTF(TPI,"is already stop!\n");		
	}
	
	return ret;
}

static RET_INFO tpi_wifi_switch_sched_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_wifi_switch_sched_stop() || RET_ERR == tpi_wifi_switch_sched_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;
}

