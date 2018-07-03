/*
*add by lqz
*该模块为定时重启模块
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <netdb.h>

#include "debug.h"
#include "common.h"
#include "reboot_check.h"

#define REBOOT_CHECK_SIZE	4*1024
static cyg_handle_t reboot_check_daemon_handle;
static char reboot_check_daemon_stack[REBOOT_CHECK_SIZE];
static cyg_thread reboot_check_daemon_thread;

static REBOOT_CHECK_INFO_STRUCT reboot_check_info;
static PIU32 reboot_check_first_time = 0;

#ifdef __CONFIG_STREAM_STATISTIC__			//added by yp,for A9, 2016-3-17
extern void get_total_stream_statistic(u_long *tx, u_long *rx);
#endif
static TIME_CHECK_RESULT tpi_reboot_check_time_check(PIU32 *sleep_time);
static RET_INFO tpi_reboot_check_each_day_by_flow(PIU32 *sleep_time);
static RET_INFO tpi_reboot_check_main();
static RET_INFO tpi_reboot_check_start();
static RET_INFO tpi_reboot_check_stop();
static RET_INFO tpi_reboot_check_restart();

/*以下函数用于api调用*/
RET_INFO tpi_reboot_check_update_info()
{
	PI8 reboot_enable[PI_BUFLEN_32] = {0}, reboot_type[PI_BUFLEN_32] = {0}, reboot_max_rx_bytes[PI_BUFLEN_32] = {0}, reboot_max_tx_bytes[PI_BUFLEN_32] = {0};
	PI8	reboot_time[PI_BUFLEN_32] = {0}, start_time[2][PI_BUFLEN_32] = {0}, end_time[2][PI_BUFLEN_32] = {0};

	strcpy__(reboot_enable, nvram_safe_get("reboot_enable"));
	strcpy__(reboot_type, nvram_safe_get("reboot_type"));
	strcpy__(reboot_time, nvram_safe_get("reboot_time"));
	strcpy__(reboot_max_rx_bytes, nvram_safe_get("reboot_max_rx_bytes"));
	strcpy__(reboot_max_tx_bytes, nvram_safe_get("reboot_max_tx_bytes"));

	if (0 == strcmp(reboot_enable, "enable"))
	{
		reboot_check_info.enable = 1;
	}
	else
	{
		reboot_check_info.enable = 0;
	}

	if (0 != strcmp(reboot_type, ""))
	{
		reboot_check_info.type = atoi(reboot_type);
	}
	else
	{
		reboot_check_info.type = REBOOT_CHECK_EACH_DAY_FLOW;
	}
	/*当前直接使用赋值为2点-5点半，先屏蔽下面的代码，nvram使用不了，已发软件的nvram都是*/
	if (nvram_match(SYSCONFIG_WORKMODE, "client+ap") 
		|| nvram_match(SYSCONFIG_WORKMODE, "bridge"))
	{
		reboot_check_info.start_time_sec = 3 * 60 * 60;
	}
	else
	{
		reboot_check_info.start_time_sec = 2 * 60 * 60;
	}
	reboot_check_info.end_time_sec = 5.5 * 60 * 60;
#if 0
	if ( (0 == strcmp(reboot_time, "")) || (sscanf(reboot_time, "%[^:]:%[^-]-%[^:]:%s", start_time[0], start_time[1], end_time[0], end_time[1]) != 4))
	{
		reboot_check_info.start_time_sec = 2 * 60 * 60;
		reboot_check_info.end_time_sec = 5.5 * 60 * 60;
	}
	else
	{
		reboot_check_info.start_time_sec = atoi(start_time[0]) * 60 * 60 + atoi(start_time[1]) * 60;
		reboot_check_info.end_time_sec = atoi(end_time[0]) * 60 * 60 + atoi(end_time[1]) * 60;

		if (reboot_check_info.start_time_sec >= reboot_check_info.end_time_sec)
		{
			PI_ERROR(TPI, "start time is bigger than end time,turn off!\n");
			reboot_check_info.enable = 0;
		}
	}
#endif
	if ( 0 != strcmp(reboot_max_rx_bytes, ""))
	{
		reboot_check_info.max_rx_bytes = atoi(reboot_max_rx_bytes);
	}
	else
	{
		reboot_check_info.max_rx_bytes = REBOOT_CHECK_DEFAULT_RX_SPEED;
	}

	if ( 0 != strcmp(reboot_max_tx_bytes, ""))
	{
		reboot_check_info.max_tx_bytes = atoi(reboot_max_tx_bytes);
	}
	else
	{
		reboot_check_info.max_tx_bytes = REBOOT_CHECK_DEFAULT_RX_SPEED;
	}

	return RET_SUC;
}

RET_INFO tpi_reboot_check_struct_init()
{
	memset(&reboot_check_info, 0x0, sizeof(reboot_check_info));
	return tpi_reboot_check_update_info();
}

RET_INFO tpi_reboot_check_first_init()
{
	if (reboot_check_info.enable)
	{
		tpi_reboot_check_start();
	}

	return RET_SUC;
}
/*添加线程退出接口函数lq*/
static int check_reboot_main_exit_flag = 0;
inline void set_reboot_main_exit_flag(int flag)
{
	check_reboot_main_exit_flag = flag;
}

inline int get_reboot_main_exit_flag()
{
	return check_reboot_main_exit_flag;
}
/*类似于sntp时区更新需要同步通知相关线程*/
static int check_reboot_main_continue_flag = 0;
inline void set_reboot_main_continue_flag(int flag)
{
	check_reboot_main_continue_flag = flag;
}

inline int get_reboot_main_continue_flag()
{
	return check_reboot_main_continue_flag;
}

RET_INFO tpi_reboot_check_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;

	PI_PRINTF(TPI, "op=%d\n", var->op);

	switch (var->op)
	{
		case OP_START:
			tpi_reboot_check_start();
			break;

		case OP_STOP:
			tpi_reboot_check_stop();
			break;

		case OP_RESTART:
			tpi_reboot_check_restart();
			break;
		case OP_UPDATE:
			set_reboot_main_continue_flag(1);
			break;
		default:
			PI_ERROR(TPI, "op[%d] donnot have handle!\n", var->op);
			break;
	}

	return RET_SUC;
}

/*以下用于gpi获取信息函数*/
PIU8 tpi_reboot_check_enable()
{
	tpi_reboot_check_update_info();
	return reboot_check_info.enable;
}

RET_INFO tpi_reboot_check_time(PIU32 *start_time, PIU32 *end_time)
{
	if (NULL == start_time || NULL == end_time)
	{
		PI_ERROR(TPI, "start_time or end_time is NULL!\n");
		return RET_ERR;
	}

	(*start_time) = reboot_check_info.start_time_sec;
	(*end_time) = reboot_check_info.end_time_sec;

	return RET_SUC;
}

/*以下为该模块具体执行实现函数*/

/*在reboot_check等其他线程可能会用到*/

/*只有本文件里面用*/
static int first_check_stream = 0;
static TIME_CHECK_RESULT tpi_reboot_check_time_check(PIU32 *sleep_time)
{
	TIME_CHECK_RESULT time_status = TIME_OUT_RANGE;
	time_t now;
	struct tm TM;
	PIU32 sleep_time_tmp = 1 * 60;
	PIU32 cur_time_in_second = 0;//以秒为单位
	PIU32 get_dut_up_time = 0; 
	PIU32 offset_time = 0;

	now = time(0);
	gmtime_r(&now, &TM);

	cur_time_in_second = TM.tm_hour * 60 * 60 + TM.tm_min * 60 + TM.tm_sec;

	PI_PRINTF(TPI, "now time:%02d:%02d cur_sec:%d[%d,%d]\n",
	          TM.tm_hour, TM.tm_min, cur_time_in_second, reboot_check_info.start_time_sec, reboot_check_info.end_time_sec);

	if (0 == reboot_check_first_time && 0 == RC_MODULE_DEBUG_ENABLE(RC_REBOOT_CHECK_MODULE))
	{
		reboot_check_first_time = TM.tm_yday;

		sleep_time_tmp = ONE_DAY_IN_SEC - cur_time_in_second;

		
		time_status = TIME_OUT_RANGE;
	}
	else
	{
		if (cur_time_in_second < reboot_check_info.start_time_sec)	//范围前
		{
			time_status = TIME_OUT_RANGE;
			sleep_time_tmp = reboot_check_info.start_time_sec - cur_time_in_second;
		}
		else if (cur_time_in_second > reboot_check_info.end_time_sec)	//范围后
		{
			time_status = TIME_OUT_RANGE;
			sleep_time_tmp = (ONE_DAY_IN_SEC - cur_time_in_second) + reboot_check_info.start_time_sec;
		}
		else if (cur_time_in_second >= reboot_check_info.start_time_sec &&
		         cur_time_in_second <= reboot_check_info.end_time_sec)	//范围内
		{	
			if(first_check_stream == 0)
			{
				if (nvram_match(SYSCONFIG_WORKMODE, "client+ap") 
					|| nvram_match(SYSCONFIG_WORKMODE, "bridge"))
				{
					time_status = TIME_IN_RANGE;
					sleep_time_tmp = 4 * 60;
				}
				else
				{
					/*根据每一台DUT的启动时间计算随机数*/
					get_dut_up_time = cyg_current_time();
					srand(get_dut_up_time);
					first_check_stream = 1;
					time_status = TIME_OUT_RANGE;
					sleep_time_tmp = 0;
					offset_time = (rand()%REBOOT_CHECK_STATR_TIME_INTERVAL_NUM)*5;
					reboot_check_info.start_time_sec += offset_time;
				}
			}
			else
			{
				time_status = TIME_IN_RANGE;
				sleep_time_tmp = 4 * 60;
			}
		}
	}

	(*sleep_time) = (sleep_time_tmp + 10);

	return time_status;
}
#ifdef __CONFIG_SPEEDTEST_IMPROVE__
extern int g_speedtest_reboot_check;
#endif

static RET_INFO tpi_reboot_check_each_day_by_flow(PIU32 *sleep_time)
{
	static TIME_UPDATE_RESULT time_update_tag = TIME_UPDATE_FAIL;
	TIME_CHECK_RESULT time_check_in_tag = TIME_OUT_RANGE;

	//检测系统时间是否更新
	if (TIME_UPDATE_FAIL == time_update_tag)
	{
		time_update_tag = gpi_common_time_update_result();

		if (TIME_UPDATE_FAIL == time_update_tag)
		{
			(*sleep_time) = 4 * 60;
			return RET_ERR;
		}
	}

	//检测系统时间是否在重启时间段中
	time_check_in_tag = tpi_reboot_check_time_check(sleep_time);

	if (TIME_OUT_RANGE == time_check_in_tag)
	{
#ifdef __CONFIG_SPEEDTEST_IMPROVE__
		g_speedtest_reboot_check = 0;
#endif

		return RET_ERR;
	}

	//TIME_IN_RANGE
	//检测br0是否有大数据包流量
	PIU8 reboot_tag = 1;
	PIU32 i = 0;
	PIU32 tx_num = 0, before_tx_mun = 0, next_tx_num = 0;
	PIU32 rx_num = 0, before_rx_mun = 0, next_rx_num = 0;

#ifdef __CONFIG_SPEEDTEST_IMPROVE__
	g_speedtest_reboot_check = 1;
#endif

	if (nvram_match(SYSCONFIG_WORKMODE, "client+ap") 
		|| nvram_match(SYSCONFIG_WORKMODE, "bridge"))
	{
		//AP或APCLIENT模式下跳过30分钟的流量检测
		PI_PRINTF(TPI, "device is not router mode skip stream check!\n");
	}
	else
	{
		while (i < REBOOT_CHECK_MAX_NUM)
		{
			//判断线程推出标志是否为请求退出lq
			if(1 == get_reboot_main_exit_flag())
			{	
				reboot_tag = 0;
				break;
			}
			tx_num = 0;
			rx_num = 0;
#ifdef __CONFIG_STREAM_STATISTIC__		//added by yp ,for A9,2016-3-17
			get_total_stream_statistic(&tx_num, &rx_num);
#endif

			if (0 == before_tx_mun && 0 == before_rx_mun)
			{
				before_tx_mun = tx_num;
				before_rx_mun = rx_num;
			}
			else
			{
				next_tx_num = tx_num;
				next_rx_num = rx_num;

				RC_MODULE_DEBUG(RC_REBOOT_CHECK_MODULE, TPI, "before:rx:%lu,tx:%lu now:rx:%lu,tx:%lu!\n",
				                before_rx_mun, before_tx_mun, next_rx_num, next_tx_num);

				if (((next_tx_num - before_tx_mun) > reboot_check_info.max_tx_bytes) || ((next_rx_num - before_rx_mun) > reboot_check_info.max_rx_bytes))
				{
					PI_PRINTF(TPI, "[%d] diff_tx = %ld,diff_rx = %ld \n", i, next_tx_num - before_tx_mun, next_rx_num - before_rx_mun);
					reboot_tag = 0;
					break;
				}
				else
				{
					reboot_tag = 1;
					before_tx_mun = next_tx_num;
					before_rx_mun = next_rx_num;
				}
			}

			i++;
			cyg_thread_delay(REBOOT_TIME_CHECK);
		}
	}


	//如果检测结果为没有大数据包经过br0则重启路由器
	if (1 == reboot_tag && 0 == get_upload_fw_flag())/*升级过程中不能重启系统*/
	{
		PI_PRINTF(TPI, "restart_check_main thread reboot our system!\n");
		sys_reboot();
		return RET_SUC;
	}

	return RET_SUC;
}

static RET_INFO tpi_reboot_check_main()
{
	PIU32 sleep_time = 1 * 60;
	PIU32 times = 0;
	while (1)
	{
		//判断线程推出标志是否为请求退出lq
		if(1 == get_reboot_main_exit_flag())
		{
			break;
		}

		switch (reboot_check_info.type)
		{
			case REBOOT_CHECK_EACH_DAY_FLOW:
				tpi_reboot_check_each_day_by_flow(&sleep_time);
				break;

			case REBOOT_CHECK_EACH_DAY:
			case REBOOT_CHECK_ONE_DAY:
			case REBOOT_CHECK_ONE_DAY_FLOW:
			default:
				PI_ERROR(TPI, "TYPE[%d] donnot have handle!\n", reboot_check_info.type);
				return RET_SUC;
				break;
		}

		RC_MODULE_DEBUG(RC_REBOOT_CHECK_MODULE, TPI, "reboot_check sleep(%d)s!\n", sleep_time);
		//判断线程推出标志是否为请求退出lq
		for(times = 0; times < sleep_time; times++)
		{
			if(1 == get_reboot_main_exit_flag())
			{
				break;
			}
			
			if(1 == get_reboot_main_continue_flag())
			{
				set_reboot_main_continue_flag(0);
				break;
			}
			cyg_thread_delay(RC_MODULE_1S_TIME);
		}
	}


	return RET_SUC;
}

static RET_INFO tpi_reboot_check_start()
{
	RET_INFO ret = RET_SUC;

	if (reboot_check_daemon_handle == 0)
	{
		tpi_reboot_check_update_info();
		first_check_stream = 0;
		if (reboot_check_info.enable)
		{
			cyg_thread_create(
			    8,
			    (cyg_thread_entry_t *)tpi_reboot_check_main,
			    0,
			    "reboot_check",
			    reboot_check_daemon_stack,
			    sizeof(reboot_check_daemon_stack),
			    &reboot_check_daemon_handle,
			    &reboot_check_daemon_thread);

			cyg_thread_resume(reboot_check_daemon_handle);
			cyg_thread_delay(1);
			PI_PRINTF(TPI, "start success!\n");
		}
		else
		{
			PI_ERROR(TPI, "the mib is off, connot start!\n");
		}
	}
	else
	{
		PI_PRINTF(TPI, "is already start!\n");
	}

	return ret;
}

static RET_INFO tpi_reboot_check_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;

	if (reboot_check_daemon_handle != 0)
	{
		//设置退出标志为退出lq
		 set_reboot_main_exit_flag(1);
		/* Wait until thread exit */
		pid = oslib_getpidbyname("reboot_check");

		if (pid)
		{
			while (oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}

		cyg_thread_delete(reboot_check_daemon_handle);

		PI_PRINTF(TPI, "stop success!\n");
		set_reboot_main_exit_flag(0);
		reboot_check_daemon_handle = 0;
	}
	else
	{
		PI_PRINTF(TPI, "is already stop!\n");
	}

	return ret;
}

static RET_INFO tpi_reboot_check_restart()
{
	RET_INFO ret = RET_SUC;

	if (RET_ERR == tpi_reboot_check_stop() || RET_ERR == tpi_reboot_check_start())
	{
		PI_ERROR(TPI, "restart error!\n");
	}

	return ret;
}

