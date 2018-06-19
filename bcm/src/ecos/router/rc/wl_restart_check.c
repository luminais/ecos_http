
#include <sys/param.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <rc.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <time.h>
#include <route_cfg.h>
#include <string.h>
#include <stdlib.h>
#include <rc.h>
#include <ecos_oslib.h>
#include <bcmnvram.h>
#include "flash_cgi.h"

#define WL_RESTART_THREAD_SIZE	4*1024

#define TOTAL_TIME_IN_SEC		86400	//24*60*60

#define TIME_UPDATE_SUCC    1
#define TIME_UPDATE_FAIL  	0

#define TIME_IN_RANGE		1
#define TIME_OUT_RANGE		0
#define TIME_UNDEFINE		-1

static cyg_handle_t wl_restart_daemon_handle = 0;
static char wl_restart_daemon_stack[WL_RESTART_THREAD_SIZE];
static cyg_thread wl_restart_daemon_thread;

struct timer_period
{
	int  is_time_zone_east;
	int time_zone;
	unsigned int timer;     //起始时间（从一天的0时计算，单位为s）  
	char select_week[7]; 
};

#define ALILINK_WLAN_SCHE_MAX	10
/* wifi time switch */
typedef struct wifi_switch_sched_cfg{
	int enable;                       //是否启用
	int sel_week[7];                  //选择的星期，since sunday ,if workday selected, set {0,1,1,1,1,1,0}
	int sche_count;
	struct timer_period timer_period_offtime[ALILINK_WLAN_SCHE_MAX];           
	struct timer_period timer_period_ontime[ALILINK_WLAN_SCHE_MAX]; 
}TPI_WIFI_SWITCH_SCHED_CFG, *PTPI_WIFI_SWITCH_SCHED_CFG;

PTPI_WIFI_SWITCH_SCHED_CFG g_wifi_sched_cfg = NULL;	

/* 记录wifi定时当前时间的上一个时间*/
struct wifi_timer{
	int wday;
	int time;
};
struct wifi_timer gWifiTimer = {-1, -1};

int gWifiSchedDebug = 0;
int gWifiStatusConfig = 0;
extern int g_cur_wl_radio_status;

int check_time_update(void);
void wl_restart_check_main(void);

void wl_restart_check_debug_level(int level)
{
    if (level > 0)
        gWifiSchedDebug = 1;
    else
        gWifiSchedDebug = 0;

    printf("wl_restart_check debug level = %d\n", gWifiSchedDebug);
}

int check_time_update(void)
{
	time_t now;
	struct tm TM;
	now = time(0);
	gmtime_r(&now,&TM);

	if(TM.tm_year != 70)
	{
		printf("time update sucess!\n");
		return TIME_UPDATE_SUCC;
	}
	
	//printf("time update fail!\n");
	return TIME_UPDATE_FAIL;
}

void check_gWifiTimer(void)
{
	time_t now;
	struct tm TM;
/*
	if( NULL == g_wifi_sched_cfg)
		return;
	
	if (0 == g_wifi_sched_cfg->enable || 0 == g_wifi_sched_cfg->sche_count)
		return;
*/
	if(gWifiTimer.wday == -1 || gWifiTimer.time == -1)
	{
		now = time(0);
		gmtime_r(&now,&TM);

		gWifiTimer.wday = TM.tm_wday;
		gWifiTimer.time = TM.tm_hour*60*60 + TM.tm_min*60 + TM.tm_sec;
	}
	
	return;
}

int check_wifishec_pretimer(int tm_wday, int time )
{
	int find = 0;
	if(gWifiTimer.wday == -1 || gWifiTimer.time == -1)
	{
		gWifiTimer.wday = tm_wday;
		gWifiTimer.time = time;
		if(gWifiSchedDebug)
			printf("update gWifiTimer\n");
	}
	
	if(gWifiTimer.wday != tm_wday || gWifiTimer.time != time)
	{
		gWifiTimer.wday = tm_wday;
		gWifiTimer.time = time;
		gWifiStatusConfig = 0;
		if(gWifiSchedDebug)
			printf("got a wifi shec time %d\n",time);
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

#define	TIME_ONE_DAY_SEC	86400
int check_time_area(int * sleep_time, struct tm *pt, PTPI_WIFI_SWITCH_SCHED_CFG wifi_sched_cfg)
{
	int wl_status ;
	int wl_cur_status = g_cur_wl_radio_status;
	int offtimer = 0,ontimer = 0;
	int i = 0;
	int curtime = pt->tm_hour*60*60 + pt->tm_min*60 + pt->tm_sec;
	int on_time_pre = -1 , off_time_pre = -1;
	int on_time_next = -1 , off_time_next = -1;
	int yesterday;

	if (curtime < 0 || curtime > TIME_ONE_DAY_SEC)
	{
		printf("curtime[%d] is error .\n",curtime);
		(*sleep_time) = 5*60;
		return wl_cur_status;
	}
	
	if(gWifiSchedDebug)
	{
		printf("+++%s curtime:%d pt->tm_wday:%d+++\n",__func__,curtime,pt->tm_wday);
		printf("cur time %d-%d-%d %d:%d:%d week:%d",pt->tm_year+1900,pt->tm_mon+1,pt->tm_mday,
			pt->tm_hour,pt->tm_min,pt->tm_sec,pt->tm_wday);
		printf("+++%s sche_count:%d gWifiStatusConfig:%d+++\n",__func__,wifi_sched_cfg->sche_count,gWifiStatusConfig);
	}
	for(i=0; i<wifi_sched_cfg->sche_count; i++)
	{
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
		printf("%s %d on_time_pre:%d off_time_pre:%d\n",__func__,__LINE__, on_time_pre, off_time_pre);
		printf("%s %d curtime:%d  on_time_next:%d off_time_next:%d\n",__func__,__LINE__,curtime, on_time_next, off_time_next);
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
		(*sleep_time)  = TIME_ONE_DAY_SEC - curtime + 5;
	
	if(gWifiSchedDebug)
		printf("sleep_time:%d \n",*sleep_time);
	if(on_time_pre != -1 || off_time_pre != -1)
	{
		if(on_time_pre >= off_time_pre)
		{
			wl_status = WL_RADIO_ON;
			if(check_wifishec_pretimer(pt->tm_wday, on_time_pre))
			{
				if(gWifiSchedDebug)
					printf("%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
				wl_status = wl_cur_status;
			}
		}
		else
		{
			wl_status = WL_RADIO_OFF;
			if(check_wifishec_pretimer(pt->tm_wday, off_time_pre))
			{
				if(gWifiSchedDebug)
					printf("%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
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
			printf("%s %d on_time_pre:%d off_time_pre:%d\n",__func__,__LINE__, on_time_pre, off_time_pre);

		if(on_time_pre != -1 || off_time_pre != -1)
		{
			if(on_time_pre >= off_time_pre)
			{
				wl_status = WL_RADIO_ON;
				if(check_wifishec_pretimer(yesterday, on_time_pre))
				{
					if(gWifiSchedDebug)
						printf("%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
					wl_status = wl_cur_status;
				}
			}
			else
			{
				wl_status = WL_RADIO_OFF;
				if(check_wifishec_pretimer(yesterday, off_time_pre))
				{
					if(gWifiSchedDebug)
						printf("%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
					wl_status = wl_cur_status;
				}
			}
		}
		else
		{
			wl_status = WL_RADIO_ON;
			if(gWifiStatusConfig)
			{
				if(gWifiSchedDebug)
					printf("%s %d wl_cur_status:%d shec_status:%d \n",__func__,__LINE__,wl_cur_status,wl_status);
				wl_status = wl_cur_status;
			}
			check_gWifiTimer();
		}
	}
	if(gWifiSchedDebug)
		printf("+++%s wl_status:%d gWifiStatusConfig:%d+++\n",__func__,wl_status,gWifiStatusConfig);
	return wl_status;
}

int check_time_in_range(int * sleep_time, PTPI_WIFI_SWITCH_SCHED_CFG wifi_sched_cfg)
{
	int wifi_status = WL_RADIO_ON;	
	time_t now;
	struct tm TM;
	
	now = time(0);
	gmtime_r(&now,&TM);

	wifi_status = check_time_area(sleep_time, &TM,wifi_sched_cfg);
	if(gWifiSchedDebug)
		printf("#####check wifi_status:%d#####\n",wifi_status);	
	return wifi_status;
}

void show_wifi_switch_sched_cfg(PTPI_WIFI_SWITCH_SCHED_CFG pcfg)
{
	int i;
	printf("###############show_wifi_switch_sched_cfg##################\n");
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
	
	printf("###############show_wifi_switch_sched_cfg end##################\n");
}

int update_wifi_sched_config(PTPI_WIFI_SWITCH_SCHED_CFG pcfg)
{
	char mib_name[64];
	char *mib_val;
	int num;
	int i,w;
	int zone_hh,zone_mm;
	int sec,min,hour;
	int s_week,e_week;
	char week[16]={0};
	int esat_time_zone_flag = 1;
	char *ptr;

	if(pcfg == NULL)
		return -1;	
	
	_GET_VALUE(WLN0_CTL_ENABLE, mib_val);
	pcfg->enable = atoi(mib_val);

	_GET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, mib_val);	
	pcfg->sche_count = atoi(mib_val);

	if( atoi(mib_val) > ALILINK_WLAN_SCHE_MAX|| atoi(mib_val) == 0)
	{
		printf("schedule count[ %d ] wrong\n",atoi(mib_val));
		return 0;	
	}
	num = atoi(mib_val);
	for(i = 1; i <= num; i++)
	{
		sprintf(mib_name,ALILINK_WLAN_SCHE_OFF_LIST,i);
	
		_GET_VALUE(mib_name, mib_val);
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
		
		pcfg->timer_period_offtime[i-1].is_time_zone_east = esat_time_zone_flag;
		pcfg->timer_period_offtime[i-1].time_zone = zone_hh;
		pcfg->timer_period_offtime[i-1].timer = hour*60*60 + min*60 + sec;
		
		if(strchr(week,'-')){
			sscanf(week,"%d-%d",&s_week,&e_week);
			for(w=s_week;w<=e_week;w++)
			{
				if( w>=1 && w<=7)
					pcfg->timer_period_offtime[i-1].select_week[w-1] = 1;
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
						printf("illegal character %c\n",*ptr);
						break;
					default:
						if(*ptr >= '1' && *ptr <= '7')//星期天到星期六
						{
							pcfg->timer_period_offtime[i-1].select_week[atoi(ptr) - 1] = 1;
						}
						break;
				}
				ptr++;
			}
		}
		sprintf(mib_name,ALILINK_WLAN_SCHE_ON_LIST,i);
		_GET_VALUE(mib_name, mib_val);
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
		
		pcfg->timer_period_ontime[i-1].is_time_zone_east = esat_time_zone_flag;
		pcfg->timer_period_ontime[i-1].time_zone = zone_hh;
		pcfg->timer_period_ontime[i-1].timer = hour*60*60 + min*60 + sec;
		if(strchr(week,'-')){
			sscanf(week,"%d-%d",&s_week,&e_week);
			for(w=s_week;w<=e_week;w++)
			{
				if( w>=1 && w<=7)
					pcfg->timer_period_ontime[i-1].select_week[w-1] = 1;
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
						printf("illegal character %c\n",*ptr);
						break;
					default:
						if(*ptr >= '1' && *ptr <= '7')//星期天到星期六
						{
							pcfg->timer_period_ontime[i-1].select_week[atoi(ptr) - 1] = 1;
						}
						break;
				}
				ptr++;
			}
		}
		
	}	
	return 0;
}

void wl_restart_check_main(void)
{
	int time_update_status = TIME_UPDATE_FAIL;	
	int sleep_time = 45;

	int radio_status_to_change = WL_RADIO_ON;
	gWifiTimer.wday = -1;
	gWifiTimer.time = -1;
	
	if (NULL == g_wifi_sched_cfg)
	{
		g_wifi_sched_cfg = (PTPI_WIFI_SWITCH_SCHED_CFG)malloc(sizeof(TPI_WIFI_SWITCH_SCHED_CFG));
		if (NULL == g_wifi_sched_cfg)
		{
			printf("malloc failed!\n");
			return ;
		}
	}
	
	memset(g_wifi_sched_cfg, 0, sizeof(TPI_WIFI_SWITCH_SCHED_CFG));
	update_wifi_sched_config(g_wifi_sched_cfg);
	
	if(gWifiSchedDebug)
		show_wifi_switch_sched_cfg(g_wifi_sched_cfg);

	//注意: 联调时需要考虑app下发的开关
	if (0 == g_wifi_sched_cfg->enable || 0 == g_wifi_sched_cfg->sche_count)
	{
		if (g_cur_wl_radio_status != WL_RADIO_ON)
		{
			g_cur_wl_radio_status = WL_RADIO_ON;
			sys_restart();
		}
		printf("wl wifictl disable,return!!!\n");	
		return;
	}
	
	while(1)
	{
		//检测系统时间是否更新
		if (TIME_UPDATE_FAIL == time_update_status)
		{
			time_update_status = check_time_update();
			if(TIME_UPDATE_FAIL == time_update_status)	
			{
				sleep_time = 45;
				goto next;
			}
		}
		
		//检测系统时间是否在重启时间段中
		radio_status_to_change = check_time_in_range(&sleep_time, g_wifi_sched_cfg);
			
		printf("==>radio_status_to_change[%d]\n",radio_status_to_change);
		if (g_cur_wl_radio_status != radio_status_to_change)
		{
			printf("restart_check thread restart wl... ...!\n");
			
			g_cur_wl_radio_status = radio_status_to_change;
		
			sys_restart();
		}
next:
		
		printf("==>sleep_time[%d]\n",sleep_time);
		
		cyg_thread_delay(sleep_time*100);
	}
	
	return;
}

void wl_restart_thread_exit(void)
{
	int pid = 0;

	if (wl_restart_daemon_handle != 0)
	{
		cyg_thread_kill(wl_restart_daemon_handle);

		/* Wait until thread exit */
		pid = oslib_getpidbyname("wl_restart_check");
		if (pid) {
			while (oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}

		wl_restart_daemon_handle = 0;
	}
	return;	
}

extern char *get_wl0_mode(char *value);

int wl_restart_check_main_loop(void)
{
	char wl_mode[32] = {0};

    get_wl0_mode(wl_mode);

	if(0 != strcmp(wl_mode,"ap"))
		return -1;
	
	wl_restart_thread_exit();
	
	cyg_thread_create(
		12, 
		(cyg_thread_entry_t *)wl_restart_check_main,
		0, 
		"wl_restart_check",
		wl_restart_daemon_stack, 
		sizeof(wl_restart_daemon_stack), 
		&wl_restart_daemon_handle, 
		&wl_restart_daemon_thread);
	cyg_thread_resume(wl_restart_daemon_handle);

	cyg_thread_delay(1);
	return 0;
}
