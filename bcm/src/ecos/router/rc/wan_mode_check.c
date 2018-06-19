/***********************************************************
Copyright (C), 2013, Tenda Tech. Co., Ltd.
FileName: wan_surfing_check.c
Description: 开机检测网络环境线程 
Author: ly;
Version : 1.0
Date: 2013-11-05
Function List:   
1. int wan_mode_check()
2. int wan_mode_check_main_loop()
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



extern int pppoe_pado_tag;
extern int pppoe_send_padi_num;
extern int pppoe0_send_padi_num;
extern int dhcp_offer_tag ;
extern int dhcp_send_discover_num ;
extern int change_tag;

int check_tag = 0;
int network_tpye = 3;
int dhcp_mode_check_tag = 0;

#define ING 				2
#define ON 					1
#define OFF 				0
#define PPPOE_CHECK_SIZE	12*1024
#define PPPOE_CHECK_MAX_NUM 3
#define DHCP_CHECK_MAX_NUM  3
#define _NONE_ 				0
#define _STATIC_ 			1
#define _DHCP_ 				2
#define _PPPOE_				3

static cyg_handle_t pppoe_daemon_handle;
static char pppoe_daemon_stack[PPPOE_CHECK_SIZE];
static cyg_thread pppoe_daemon_thread;

extern int ping_connet_check();
int wan_mode_check();
int static_first_check();
int dhcp_first_check();
int pppoe_first_check();


/************************************************************
Function:	 wan_mode_check               
Description:  检测网络环境的主函数

Input:                                          

Output: 	    

Return:      将检测结果放回给前台显示给用户(结果存在全局变量中)  

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/
int wan_mode_check()
{
	char *wan_mode = NULL;
	int wan_mode_num = _DHCP_;
	wan_mode = nvram_safe_get("wan0_proto");

	//开始判断是否执行联网检测
	if(strcmp(nvram_safe_get("mode_need_switch"),"yes") != 0 
			||strcmp(nvram_safe_get("wl0_mode"),"ap") != 0 
				|| strcmp(nvram_safe_get("wl0_wds"),"") != 0)
	{
		check_tag = 1;
		return 0;
	}
	//读取默认的接入方式
	if(strcmp(wan_mode,"pppoe") == 0)
	{
		wan_mode_num = _PPPOE_;
	}
	else if(strcmp(wan_mode,"dhcp") == 0)
	{
		wan_mode_num = _DHCP_;
	}
	else if(strcmp(wan_mode,"static") == 0)
	{
		wan_mode_num = _STATIC_;
	}
	else
	{
		wan_mode_num = _NONE_;
	}

	//根据默认的接入方式选择相应的分支
	switch(wan_mode_num)
	{
		case _PPPOE_:
			pppoe_first_check();
			break;
		case _DHCP_:
			dhcp_first_check();
			break;
		case _STATIC_:
			static_first_check();
			break;
		default:
			//如果wan口接入方式为L2TP PPTP等，则默认使用静态检测(如果需要更改，在这里更改)
			static_first_check();
			break;
	}
	return 0;
}

/************************************************************
Function:	 dhcp_first_check               
Description:  如果默认接入方式为dhcp，则调用该函数进行检测

Input:                                          

Output: 	    

Return:      将检测结果放回给前台显示给用户(结果存在全局变量中)  

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

int dhcp_first_check()
{
	char wan_connstatus[16]={0}, wan_ifname[16]={0}, wan_ipaddr[16]={0}, wan_mask[16]={0};
	static int wan_mode_check_tag = 0,ppp2_tag = ON;
	int dhcp_tag = 1;
	pppoe_pado_tag = 0;
	pppoe_send_padi_num = 0;
	dhcp_offer_tag = 0;
	dhcp_send_discover_num = 0;
	network_tpye = 3;
	int ping_tag = 1;
	char wan_mode_check_verdict[10] = "dhcp";

	while(1)
	{
		cyg_thread_delay(100);
		//如果默认的DHCP连接方式在发出3个发现报文都没有收到回应包的时候
		//则认为环境不是DHCP方式直接开启PPP2线程，检查是否是PPPOE环境
		if(dhcp_offer_tag != 1 && dhcp_send_discover_num > DHCP_CHECK_MAX_NUM)
		{
			dhcp_tag = 0;
			goto next;
		}
		//检测wan口是否获取到IP，如果没有获取到IP则继续循环检测，直到获取到IP之后检测DHCP是否能上外网
		strcpy(wan_connstatus, nvram_safe_get(_WAN0_CONNECT));
		if(strcmp(wan_connstatus,"Connected") != 0)
		{			
			continue;
		}
		if(change_tag == 1)
		{
			goto finish;
		}
		//ping 百度、谷歌
		if(ping_tag == 1)
		{
			if(1 == ping_connet_check())
			{
				goto down;
			}
			if(change_tag == 1)
			{
				goto finish;
			}
			
			ping_tag = 0;
		}
next:	
		if(change_tag == 1)
		{
			goto finish;
		}
		//开启ppp2线程	
		if(ppp2_tag == ON)
		{
			wan_pppoe_start2();
			ppp2_tag = ING;
		}
		//如果接受到回应包则切换到PPPOE
		if(pppoe_pado_tag == 1)
		{
			//当前上网方式为pppoe
			diag_printf("\t %d wanmode is pppoe!\n",__LINE__);			
			network_tpye = 2;
			wan_mode_check_tag = 1;
			snprintf(wan_mode_check_verdict , sizeof(wan_mode_check_verdict) , "pppoe" );
		}
		//如果发PADI包超过5个并且还没有收到回应报文则是DHCP连接方式
		else if(pppoe_send_padi_num >= PPPOE_CHECK_MAX_NUM)
		{
			if(dhcp_tag == 1)
			{
				diag_printf("\t %d wanmode is dhcp!\n",__LINE__);
				network_tpye = 1;
			snprintf(wan_mode_check_verdict , sizeof(wan_mode_check_verdict) , "dhcp" );
			}
			else
			{
				diag_printf("\t %d wanmode is static ip!\n",__LINE__);
				network_tpye = 0;
			snprintf(wan_mode_check_verdict , sizeof(wan_mode_check_verdict) , "static" );
			}
			wan_mode_check_tag = 1;
		}
		//关闭ppp2线程
		if(wan_mode_check_tag == 1)
		{
			diag_printf("dowm ppp2!\n");
			wan_pppoe_down2("ppp2");
			ppp2_tag = OFF;
				
			sprintf(wan_ifname, "%s", nvram_safe_get("wan0_ifname"));
			sprintf(wan_ipaddr, "%s", nvram_safe_get("wan0_ipaddr"));
			sprintf(wan_mask, "%s", nvram_safe_get("wan0_netmask"));
			if (strcmp(nvram_safe_get("wan0_ifname"), "vlan2") == 0){
				ifconfig(wan_ifname, IFF_UP,wan_ipaddr, wan_mask);
				 route_add(wan_ifname, 0, "0.0.0.0", nvram_safe_get("wan0_gateway"), "0.0.0.0");
			}
			nvram_set("wan0_isonln","no");
			goto down;
		}
	}
down:	
	nvram_set("mode_need_switch","no");
finish:	
	if(ppp2_tag == ING)
	{
		diag_printf("dowm ppp2!\n");
		wan_pppoe_down2("ppp2");
	}
	ppp2_tag = OFF;
	check_tag = 1;
	nvram_set("wan_mode_check_verdict",wan_mode_check_verdict);
	nvram_commit();
	return 0;
}
/************************************************************
Function:	 pppoe_first_check               
Description:  如果默认接入方式为pppoe，则调用该函数进行检测

Input:                                          

Output: 	    

Return:      将检测结果放回给前台显示给用户(结果存在全局变量中)  

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

int pppoe_first_check()
{
	pppoe_pado_tag = 0;
	pppoe_send_padi_num = 0;
	dhcp_offer_tag = 0;
	dhcp_send_discover_num = 0;
	network_tpye = 3;
	int dhcp_check_tag = ON;
	char wan_mode_check_verdict[10] = "pppoe";
	
	while(1)
	{
		cyg_thread_delay(100);
		if(pppoe_pado_tag == 1 && pppoe0_send_padi_num <= PPPOE_CHECK_MAX_NUM)
		{
			//当前上网方式为pppoe
			diag_printf("\t %d wanmode is pppoe!\n",__LINE__);			
			network_tpye = 2;
			snprintf(wan_mode_check_verdict , sizeof(wan_mode_check_verdict) , "pppoe" );
			goto down;
		}
		if(change_tag == 1)
		{
			goto finish;
		}
		if(pppoe_pado_tag != 1 && pppoe0_send_padi_num <= PPPOE_CHECK_MAX_NUM)
		{
			continue;
		}
		//开启dhcp服务器发报文
		if(dhcp_check_tag == ON)
		{
			dhcp_mode_check_tag = 1;
			dhcpc_start("vlan2", "wandhcpc", "Tenda");
			dhcp_check_tag = ING;
		}
		//如果接收到offer报文并且发包个数少于3个，则是dhcp接入
		if(dhcp_offer_tag == 1 && dhcp_send_discover_num <= DHCP_CHECK_MAX_NUM)
		{
			diag_printf("\t %d wanmode is dhcp!\n",__LINE__);
			network_tpye = 1;
			dhcpc_stop("vlan2");
			dhcp_check_tag = OFF;
			snprintf(wan_mode_check_verdict , sizeof(wan_mode_check_verdict) , "dhcp" );
			goto down; 
		}
		//
		if(change_tag == 1)
		{
			goto finish;
		}
		if(dhcp_offer_tag != 1 && dhcp_send_discover_num <= DHCP_CHECK_MAX_NUM)
		{
			continue;
		}
		//其他为静态IP
		{
			diag_printf("\t %d wanmode is static ip!\n",__LINE__);
			network_tpye = 0;
			snprintf(wan_mode_check_verdict , sizeof(wan_mode_check_verdict) , "static" );
			goto down;
		}
	}
down:	
	nvram_set("mode_need_switch","no");
finish:	
	if(dhcp_check_tag == ING && strcmp(nvram_safe_get("wan0_proto"),"dhcp") != 0)
	{
		dhcpc_stop("vlan2");
	}
	dhcp_check_tag = OFF;
	check_tag = 1;
	nvram_set("wan_mode_check_verdict",wan_mode_check_verdict);
	nvram_commit();
	return 0;
}
/************************************************************
Function:	 static_first_check               
Description:  如果默认接入方式为static，则调用该函数进行检测

Input:                                          

Output: 	    

Return:      将检测结果放回给前台显示给用户(结果存在全局变量中)  

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

int static_first_check()
{
	static int ppp2_tag = 1;
	pppoe_pado_tag = 0;
	pppoe_send_padi_num = 0;
	dhcp_offer_tag = 0;
	dhcp_send_discover_num = 0;
	network_tpye = 3;
	char wan_mode_check_verdict[10] = "static";
	int dhcp_check_tag = ON;

	while(1)
	{
		cyg_thread_delay(100);
		if(change_tag == 1)
		{
			goto finish;
		}
		if(ppp2_tag == 1)
		{
			wan_pppoe_start2();
			ppp2_tag = 0;
		}
		if(pppoe_pado_tag == 1 && pppoe_send_padi_num <= PPPOE_CHECK_MAX_NUM)
		{
			//当前上网方式为pppoe
			diag_printf("\t %d wanmode is pppoe!\n",__LINE__);			
			network_tpye = 2; 
			snprintf(wan_mode_check_verdict , sizeof(wan_mode_check_verdict) , "pppoe" );
			goto down;
		}
		if(change_tag == 1)
		{
			goto finish;
		}
		if(pppoe_pado_tag != 1 && pppoe_send_padi_num <= PPPOE_CHECK_MAX_NUM)
		{
			continue;
		}
		//开启dhcp服务器发报文
		if(dhcp_check_tag == ON)
		{
			dhcp_mode_check_tag = 1;
			dhcpc_start("vlan2", "wandhcpc", "check");
			dhcp_check_tag = ING;
		}
		//如果接收到offer报文并且发包个数少于3个，则是dhcp接入
		if(dhcp_offer_tag == 1 && dhcp_send_discover_num <= DHCP_CHECK_MAX_NUM)
		{
			diag_printf("\t %d wanmode is dhcp!\n",__LINE__);
			network_tpye = 1;
			dhcpc_stop("vlan2");
			dhcp_check_tag = OFF;
			snprintf(wan_mode_check_verdict , sizeof(wan_mode_check_verdict) , "dhcp" );
			goto down; 
		}
		if(change_tag == 1)
		{
			goto finish;
		}
		if(dhcp_offer_tag != 1 && dhcp_send_discover_num <= DHCP_CHECK_MAX_NUM)
		{
			continue;
		}
		//其他为静态IP
		{
			diag_printf("\t %d wanmode is static ip!\n",__LINE__);
			network_tpye = 0;
			snprintf(wan_mode_check_verdict , sizeof(wan_mode_check_verdict) , "static" );
			goto down;
		}
	}
down:	
	nvram_set("mode_need_switch","no");
finish:	
	if(dhcp_check_tag == ING && strcmp(nvram_safe_get("wan0_proto"),"dhcp") != 0)
	{
		dhcpc_stop("vlan2");
	}
	dhcp_check_tag = OFF;
	check_tag = 1;
	nvram_set("wan_mode_check_verdict",wan_mode_check_verdict);
	nvram_commit();
	return 0;
}
/************************************************************
Function:	 wan_mode_check_main_loop               
Description:  检测网络环境的创建线程函数，在RC.C
			中调用该函数启动该线程

Input:                                          

Output: 	    

Return:     

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

int wan_mode_check_main_loop()
{	
	cyg_thread_create(
		8, 
		(cyg_thread_entry_t *)wan_mode_check,
		0, 
		"wan_mode_check",
		pppoe_daemon_stack, 
		sizeof(pppoe_daemon_stack), 
		&pppoe_daemon_handle, 
		&pppoe_daemon_thread);
	cyg_thread_resume(pppoe_daemon_handle);

	cyg_thread_delay(1);
printf("function[%s] , line[%d] , start end \n" , __FUNCTION__ , __LINE__);
	return 0;
}
