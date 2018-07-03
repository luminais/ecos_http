#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pkgconf/system.h>
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <bcmnvram.h>
#include <netdb.h>
#include <router_net.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
//#include "../tc/tc.h"
#include "../../../net_drive/ipfilter/sdw_filter.h"
//#include "flash_cgi.h"
#include "lm_login_keep.h"

#include "sys_module.h"
#include "msg.h"
#ifdef REALTEK
#include "trxhdr.h"
#endif

#if 1
extern unsigned long extra_get_stream_statistic(int dir);
#else
extern u_long get_stream_statistic(int dir);
#endif
extern void sys_reboot(void);
extern int net_mbuf_free_mem();
extern int do_upgrade_check(char *stream, int offset_in, int *flash_offset);
extern int tenda_upload_fw(char *stream, int offset_in,int flash_offset);
extern void upgrade_mem_free(char *head);
extern int http_get_file(char *server_host, char *server_path, int port, char *server_ip, char *save_into_router, int *file_len);

extern char *a_query;

int if_device_idle(void)
{
	unsigned long u_kbs0=0, d_kbs0=0;

#if 1
	u_kbs0 = extra_get_stream_statistic(1);
	d_kbs0 = extra_get_stream_statistic(0);
#else
	u_kbs0 = get_stream_statistic(1);
	d_kbs0 = get_stream_statistic(0);
#endif
	if ((u_kbs0) > 1 || (d_kbs0) > 1)
	{
		return -1;
	}

	return 0;
}

int upgrade2(char* wp, char *cgiName, char *query)
{

	int offset,flash_offset;
	int ret;
	int check_ret = 0;
	diag_printf("Start upgrade online...\n");
	flash_offset = 0;
	offset=0;
	check_ret = do_upgrade_check(query,offset,&flash_offset);
	if( -4 == check_ret) {
		ret = NOT_FOR_THIS_PRO;
		goto do_error;
	} else if(-3 == check_ret) {
		ret = CHECK_FILE_ERROR;
		goto do_error;
	}
#ifdef REALTEK
	/*realtek 升级跳过头, llm add*/
	offset += sizeof(struct trx_header);
#endif
	if(tenda_upload_fw(query,offset,flash_offset) < 0){
		ret = WRITE_FILE_FAIL;
		goto do_error;
	}

	upgrade_mem_free(query);
	
	diag_printf("Upgrade online success...\n");

	ret = OK;
	return ret;
	
do_error:
	//upgrade_mem_free(query);
	//sys_reboot();
	return ret;
}

int get_upgrade_file(char *host_url, char *server_path, int port, char *server_ip)
{
	int get_file_times = 0;
	int upgrade_file_len = 0;
	for(get_file_times=0; get_file_times<MAX_TRY_TIMES; get_file_times++)
	{
		if(OK != http_get_file(host_url, server_path, port, server_ip, NULL, &upgrade_file_len))
		{
			cyg_thread_delay(120*100);
		}
		else
			break;
	}

	if(get_file_times == MAX_TRY_TIMES)
	{
		diag_printf("[%s]get file failed, reboot\n", __FUNCTION__);
		return -1;
	}
	else
	{
		if(0 == upgrade_file_len)
		{
			diag_printf("[%s]upgrade_file_len = 0\n", __FUNCTION__);
			return -1;
		}
		else
		{
			diag_printf("[%s]get file success\n", __FUNCTION__);
			return 0;
		}
	}
}

int get_upgrade_serv_ip(char *host_url, char *ip)
{
	int get_ip_times = 0;
	for(get_ip_times=0; get_ip_times<MAX_TRY_TIMES; get_ip_times++)
	{			
		if(0 != get_ip(host_url, ip) )
		{
			memset(ip , 0 , sizeof(ip));
			cyg_thread_delay(120*100);
		}
		else
			break;
	}

	if(get_ip_times == MAX_TRY_TIMES)
	{
		diag_printf("[%s]get upgrade serv ip failed, reboot\n", __FUNCTION__);
		return -1;
	}
	else
	{
		ip[IP_LEN_16-1] = '\0';
		diag_printf("[%s]get upgrade serv ip success\n", __FUNCTION__);
		return 0;
	}
}

int wait_until_idle(void)
{
	int i = 0;

	while(i<60)
	{
		if(0 == if_device_idle())
		{
			i++;
		}
		else
		{
			i = 0;
		}
		cyg_thread_delay(100*60);
	}

	return 0;
}

void print_keep_version(struct lm_keep_version *keep_version)
{
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
//	printf("type : %d\n", keep_version->type);
	printf("action : %d\n", keep_version->action);
	printf("verlen : %d\n", keep_version->verlen);
	printf("version : %s\n", keep_version->version);
	printf("urllen : %d\n", keep_version->urllen);
	printf("url : %s\n", keep_version->url);
	printf("md5len : %d\n", keep_version->md5len);
#if 0
	int i;
	for(i=0; i<keep_version->md5len; i++)
	{
		keep_version->md5[i] = toupper(keep_version->md5[i]);
	}
#endif
	printf("md5 : %s\n", keep_version->md5);
	return;
}

void upgrade_online_main(cyg_addrword_t keep_version_mem)
{
	char sdw_upgrade_serv_ip[IP_LEN_16] = {0};
	char url_host[64] = {0}, url_path[64] = {0};
	int port = 80;
	struct lm_keep_version *keep_version;
	
	keep_version = (struct lm_keep_version *)keep_version_mem;

	if(NULL == keep_version)
	{
		diag_printf("[%s][%d] invalid para\n", __FUNCTION__, __LINE__);
		return;
	}
#if 1
	print_keep_version(keep_version);
	//return;
#endif
	memset(url_host , 0 , sizeof(url_host));
	memset(url_path , 0 , sizeof(url_path));
	if(0 != get_url_path(keep_version->url, url_host, url_path, &port))
	{
		diag_printf("[%s]url_host = %s, url_path = %s\n", __FUNCTION__, url_host, url_path);
		return;
	}
	diag_printf("[%s][%d] <%d> host : %s, server_path : %s, port : %d\n", __FUNCTION__, __LINE__, keep_version->action, url_host, url_path, port);

	switch(keep_version->action)
	{
		case 1://立即升级
			diag_printf("[%s][%d] upgrade immediately. \n", __FUNCTION__, __LINE__);
			break;
		case 2://空闲升级
			diag_printf("[%s][%d] upgrade when idle. \n", __FUNCTION__, __LINE__);
			wait_until_idle();
			break;
		default:
			diag_printf("[%s][%d] unknow action type. \n", __FUNCTION__, __LINE__);
			break;
	}
	
	memset(sdw_upgrade_serv_ip , 0 , sizeof(sdw_upgrade_serv_ip));
#if 0
	strcpy(sdw_upgrade_serv_ip, "192.168.0.100");
#else
	if(0 != get_upgrade_serv_ip(url_host, sdw_upgrade_serv_ip))
	{
		diag_printf("get_upgrade_serv_ip failed\n");
		return;
	}
#endif
	tapf_watchdog_disable();
	diag_printf("net_mbuf_free_mem %d\n", net_mbuf_free_mem());
	if( 0 != get_upgrade_file(url_host, url_path, port, sdw_upgrade_serv_ip))//luminais
	{
		goto now_reboot;
	}
	diag_printf(".net_mbuf_free_mem %d\n", net_mbuf_free_mem());
	if(OK ==  upgrade2(NULL, NULL, a_query))
	{
		diag_printf("[%s]upgrade online success\n", __FUNCTION__);
	}
now_reboot:
	//sys_reboot();
	msg_send(MODULE_RC,RC_SYSTOOLS_MODULE,"string_info=reboot");

	diag_printf("[%s][%d] exit!!!\n", __FUNCTION__, __LINE__);
	return;
}

static cyg_handle_t upgrade_online_daemon_handle;
static char upgrade_online_daemon_stack[8*1024];
static cyg_thread upgrade_online_daemon_thread;

void upgrade_online_start(struct lm_keep_version *keep_version)
{	
	int pid;
	
	pid = oslib_getpidbyname("upgrade_online");
	if (pid != 0)//线程已存在，直接返回
		return;
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	cyg_thread_create(
		8, 
		(cyg_thread_entry_t *)upgrade_online_main,
		(cyg_addrword_t)keep_version, 
		"upgrade_online",
		upgrade_online_daemon_stack, 
		sizeof(upgrade_online_daemon_stack), 
		&upgrade_online_daemon_handle, 
		&upgrade_online_daemon_thread);
	cyg_thread_resume(upgrade_online_daemon_handle);
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	return;
}

