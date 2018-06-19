/***********************************************************
Copyright (C), 2013, Tenda Tech. Co., Ltd.
FileName: restart_check.c
Description: 检测联网状态线程 
Author: ly;
Version : 1.0
Date: 2013-11-05
Function List:   
1. 
2. 
3. 

History:
<author>   <time>   <version >   <desc>
hqw        2013-11-05   1.0        新建线程文件
************************************************************/

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
#include <ecos_oslib.h>

#define SURFING_CHECK_SIZE	8*1024

static cyg_handle_t restart_daemon_handle;
static char restart_daemon_stack[SURFING_CHECK_SIZE];
static cyg_thread restart_daemon_thread;

#define TIME_UPDATE_SUCC    1
#define TIME_UPDATE_FAIL  	0

#define TIME_IN_RANGE		1
#define TIME_OUT_RANGE		0
#define TIME_UNDEFINE		-1

#define CHECK_INTERVAL_SEC	1	

#define CHECK_TOTAL_SEC 		60*30		/*total 5 min*/
#define CHECK_MIN_RX_NUM	(3000 * CHECK_INTERVAL_SEC)	
#define CHECK_MIN_TX_NUM	(3000 * CHECK_INTERVAL_SEC)	


int first_time_check = -1;

int restart_check_interval_sec = CHECK_INTERVAL_SEC;
int restart_min_rx_bytes = CHECK_MIN_RX_NUM;
int restart_min_tx_bytes = CHECK_MIN_TX_NUM;

//用户设置的时间段，如果想要增加分钟，在这里增加
struct restart_time_span
{
	unsigned int begin_sec;
	unsigned int end_sec;
};
	

extern void get_total_stream_statistic(u_long * tx,u_long * rx);
extern int get_upload_fw_flag();

//extern int if_TX_RX_status(char * ifname,u_long * tx,u_long * rx);
int update_check(void);
int time_check(int * sleep_time,struct restart_time_span restart_time);
//int pack_RX_TX_check(char * ifname,u_long * tx,u_long * rx);
void restart_check_main(void);

void init_restart_min_txrx_byte(void)
{
	char  check_interval_sec[8]={0},min_rx_bytes[8] = {0}, min_tx_bytes[8] = {0};

	strcpy(check_interval_sec,nvram_safe_get("restart_check_interval_sec"));
	if (strlen(check_interval_sec) > 0)
	{
		restart_check_interval_sec = atoi(check_interval_sec);
	}

	strcpy(min_rx_bytes,nvram_safe_get("restart_min_rx_bytes"));
	if (strlen(min_rx_bytes) > 0)
	{
		restart_min_rx_bytes = atoi(min_rx_bytes)*restart_check_interval_sec;
	}
	
	strcpy(min_tx_bytes,nvram_safe_get("restart_min_tx_bytes"));
	if (strlen(min_rx_bytes) > 0)
	{	
		restart_min_tx_bytes = atoi(min_tx_bytes)*restart_check_interval_sec;
	}

	printf("restart_check_interval_sec:%d  restart_min_rx_byte:%d  restart_min_tx_byte:%d\n",restart_check_interval_sec,restart_min_rx_bytes,restart_min_tx_bytes);

	return;
}

/************************************************************
Function:	 update_check               
Description:  检测系统时间是否更新

Input:                                          

Output: 	    

Return:      TURE 成功 FAILE 失败

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

int update_check(void)
{
	time_t now;
	struct tm TM;
	now = time(0);
	gmtime_r(&now,&TM);
	printf("%04d-%02d-%02d,the %d days of the weak,%02d:%02d:%02d\n",
		TM.tm_year,TM.tm_mon,TM.tm_mday,TM.tm_wday+1,
			TM.tm_hour,TM.tm_min,TM.tm_sec);
	
	if(TM.tm_year != 70)
	{
		printf("time has update!\n");
		return TIME_UPDATE_SUCC;
	}
	
	printf("time has not update!\n");
	return TIME_UPDATE_FAIL;
}


#define	ONE_DAY_IN_SEC	86400	//24*60*60

int time_check(int * sleep_time,struct restart_time_span restart_time)
{
	int time_status = TIME_UNDEFINE;
	
	time_t now;
	struct tm TM;
	int sleep_time_tmp = 1*60;

	int cur_time_in_second = 0;//以秒为单位
	
	now = time(0);
	gmtime_r(&now,&TM);

	cur_time_in_second = TM.tm_hour*60*60 + TM.tm_min*60 + TM.tm_sec;

	if (cur_time_in_second <= 0)
	{
		(*sleep_time) = 2*60;

		return TIME_UNDEFINE;
	}

	printf("now time:%02d:%02d cur_time_in_second:%d\n",TM.tm_hour,TM.tm_min,cur_time_in_second);
	
	if(-1 == first_time_check)
	{
		first_time_check = TM.tm_yday;

		sleep_time_tmp = ONE_DAY_IN_SEC - cur_time_in_second;
		
		time_status = TIME_OUT_RANGE;
	}
	else 
	{
		if (cur_time_in_second < restart_time.begin_sec)	//范围前
		{
			time_status = TIME_OUT_RANGE;
			sleep_time_tmp = restart_time.begin_sec - cur_time_in_second;
		}
		else if (cur_time_in_second > restart_time.end_sec)	//范围后
		{
			time_status = TIME_OUT_RANGE;
			sleep_time_tmp = (ONE_DAY_IN_SEC - cur_time_in_second) + restart_time.begin_sec;
		}
		else if (cur_time_in_second >= restart_time.begin_sec && 
			cur_time_in_second <= restart_time.end_sec)	//范围内
		{
			time_status = TIME_IN_RANGE;
			
			sleep_time_tmp = 4 * 60;
		}
	}
	
	(*sleep_time) = sleep_time_tmp + 10;

	return time_status;
}

#if 0
/************************************************************
Function:	 pack_RX_TX_check               
Description:  从ifname接口中读取上传以及下载的数据包字节大小

Input:         ifname  接口         tx     上传数据包大小          rx   下载数据包大小                  

Output: 	    

Return:     -1 失败  0 成功

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

//统计接收上传数据包
int pack_RX_TX_check(char * ifname,u_long * tx,u_long * rx)
{
	return if_TX_RX_status(ifname,tx,rx);
}
#endif

/************************************************************
Function:	 restart_check_main               
Description:  定时重启主函数

Input:         

Output: 	    

Return:     

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

//统计接收上传数据包

void restart_check_main(void)
{
	int update_check_tag = TIME_UPDATE_FAIL;
	int time_check_tag = TIME_UNDEFINE;
	int restart_tag = 0;
	u_long tx_num = 0;
	u_long rx_num =0;
	int sleep_time = 1*60;

	char wifi_down_time[50] = {0};
	char hour_down[3] = {0};
	char min_down[3] = {0};
	char hour_up[3] = {0};
	char min_up[3] = {0};
	
	struct restart_time_span restart_time;
	if(nvram_match("restart_enable","enable") != 1)
	{
		printf("restart_enable disable,return!!!\n");
		return;
	}
		
	int tag = 0;
	char  *debug_restart_check = NULL;
	debug_restart_check = nvram_safe_get("debug_restart_check");
	if (strlen(debug_restart_check) > 0)
	{
		tag = atoi(debug_restart_check);
	}
	
	
	strcpy(wifi_down_time,nvram_safe_get("restart_time"));
	
	if ((0==strcmp(wifi_down_time,"")) || (sscanf(wifi_down_time,"%[^:]:%[^-]-%[^:]:%s",hour_down,min_down,hour_up,min_up)!=4))
	{
		printf("get date wrong,return!!!\n");
		
		return;
	}

	restart_time.begin_sec =atoi(hour_down)*60*60 + atoi(min_down)*60 + 1;
	restart_time.end_sec =atoi(hour_up)*60*60 + atoi(min_up)*60 -1;
	
	printf("restart time:%d-%d\n",restart_time.begin_sec,restart_time.end_sec);
	
	if(restart_time.end_sec < restart_time.begin_sec)
	{
		printf("==>time range span day,exit!!!\n");
		
		return;
	}

	init_restart_min_txrx_byte();

	while(1)
	{
		restart_tag = 0;

		//检测系统时间是否更新
		if (TIME_UPDATE_FAIL == update_check_tag)
		{
			update_check_tag = update_check();
			if(TIME_UPDATE_FAIL == update_check_tag)	
			{
				sleep_time = 4*60;
				goto next;
			}
		}

		//检测系统时间是否在重启时间段中
		time_check_tag = time_check(&sleep_time,restart_time);
		
		if((TIME_OUT_RANGE == time_check_tag) || (TIME_UNDEFINE == time_check_tag))
		{
			goto next;
		}

		//TIME_IN_RANGE
		//检测br0是否有大数据包流量
		int i = 0;
		u_long before_tx_mun = 0;
		u_long next_tx_num = 0;
		u_long before_rx_mun = 0;
		u_long next_rx_num = 0;

		int check_num = CHECK_TOTAL_SEC/restart_check_interval_sec;
		printf("check_num = %d,in sec = %d\n",check_num,CHECK_TOTAL_SEC);
		while(i < check_num)
		{
			tx_num = 0;
			rx_num = 0;
#if 0
			if(pack_RX_TX_check("br0",&tx_num,&rx_num) == -1)
			{
				printf("get TX RX error!\n");
				cyg_thread_delay(1*60*100);
				continue;
			}
#else
			get_total_stream_statistic(&tx_num,&rx_num);
#endif			
			if(0 == before_tx_mun && 0 == before_rx_mun)
			{
				before_tx_mun = tx_num;
				before_rx_mun = rx_num;
			}
			else
			{
				next_tx_num = tx_num;
				next_rx_num = rx_num;
				
				if(tag)
				    printf("[%d] diff_tx = %ld,diff_rx = %ld \n",i, next_tx_num - before_tx_mun,next_rx_num - before_rx_mun);	
				
				if(((next_tx_num - before_tx_mun) > restart_min_tx_bytes) || ((next_rx_num - before_rx_mun) > restart_min_rx_bytes))
				{
					restart_tag = 0;
					break;
				}
				else
				{
					restart_tag = 1;
					before_tx_mun = next_tx_num;
					before_rx_mun = next_rx_num;
				}
			}
			i++;
				
			cyg_thread_delay(restart_check_interval_sec*100);
		}

		
		//如果检测结果为没有大数据包经过br0则重启路由器
		printf("restart_tag = %d\n",restart_tag);
		if (1 == restart_tag && 0 == get_upload_fw_flag())/*升级过程中不能重启系统*/
		{
			printf("restart_check_main thread reboot our system!\n");
			sys_reboot();
			return;
		}

next:

		if(time_check_tag != TIME_IN_RANGE)
		{
			printf("restart_check_main sleep_time sec[%d]... ...\n",sleep_time);
			
			cyg_thread_delay(sleep_time*100);
		}
	}
	
	return;
}

void restart_check_thread_exit(void)
{
	int pid = 0;

	if (restart_daemon_handle != 0)
	{
		cyg_thread_kill(restart_daemon_handle);

		/* Wait until thread exit */
		pid = oslib_getpidbyname("restart_check");
		if (pid) {
			while (oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}
		
		cyg_thread_delete(restart_daemon_handle);
		
		restart_daemon_handle = 0;
	}
	return;	
}


/************************************************************
Function:	 restart_check_main_loop               
Description:  	  定时重启创建线程函数入口，在主线程
			  RC.C中调用
Input:                                          

Output: 

Return:         

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

int restart_check_main_loop()
{	
	restart_check_thread_exit();
	
	if(nvram_match("restart_enable","enable") != 1)
	{
		printf("restart_enable disable,return!!!\n");
		return 0;
	}
	
	cyg_thread_create(
		10, 
		(cyg_thread_entry_t *)restart_check_main,
		0, 
		"restart_check",
		restart_daemon_stack, 
		sizeof(restart_daemon_stack), 
		&restart_daemon_handle, 
		&restart_daemon_thread);
	cyg_thread_resume(restart_daemon_handle);

	cyg_thread_delay(1);
printf("function[%s] , line[%d] , start end \n" , __FUNCTION__ , __LINE__);
	return 0;
}
