/*
 * Get filter rules and assign to ipfilter engine.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: filcmd.c,v 1.11 2010-08-19 08:51:39 Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if_var.h>

#include "ip_filfast.h"
#include "macf.h"
#include "urlf.h"
#include "netinet/ip_compat.h"
#include "netinet/ip_fil.h"
#include "netinet/ip_nat.h"
#include "ipf.h"
//#include "ip_defense.h"

#include <iflib.h>
#include <bcmnvram.h>

/* foreach need */
#include <shutils.h>

#include <netconf.h>
#include <nvparse.h>

#include <router_net.h>//roy+++,2010/09/20

#define FLT_MAC_MAX_NUM     50
#define FLT_URL_MAX_NUM     50
#define FLT_CLN_MAX_NUM     50
#define FLT_PKT_MAX_NUM     50
#define FLT_DNS_MAX_NUM     50

//struct rate_limit sync_rate;
/**************为确保编译，临时定义*********************************/

#define MAX_NO_BRIDGE 1

/*******************************************************************/
int opts = 0;

extern int udp_blackhole;
extern int tcp_blackhole;
int drop_portscan;
int auth_port_open;
//roy+++,2010/09/27
unsigned int ip_defense;
//+++
static int firewall_inited;
extern int in_time_area;
extern int wan_primary_ifunit(void);
extern int countbits __P((u_32_t));

//roy+++,2010/09/20

enum
{
    MODE_DISABLED = 0,//禁用
    MODE_DENY,//仅禁止
    MODE_PASS//仅允许
};

enum
{
    STA_UNUSE = 0,
    STA_ADDED,
    STA_DELETED
};
//+++

//add by ll
#define NIS_FASTCHECK_ENABLE 1 

enum {
	RECORD_MODE_DISABLED = 0,//禁用
	RECORD_MODE_ENABLE,//启用
	RECORD_MODE_UNDEFINE
};
//end by ll


static int mac_fil_status[MAX_NVPARSE];
static int client_fil_status[MAX_NVPARSE];
#ifdef __CONFIG_URLFILTER__
static int url_fil_status[MAX_NVPARSE];
#endif

static int client_default_action = 0;
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
extern void parent_control_config_update(void);
int mac_filter_mode = 0;
int url_filter_mode = 0;
#endif

#ifdef __CONFIG_AL_SECURITY__

void al_security_start(int flag)
{
    int fd = -1;
    int mode= -1 ;

    fd = open(IPL_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("open IPL_NAME failed\n");
        return;
    }


    mode = flag;

    printf("#######al_security_start flag:%d\n" , mode);


    int rc = ioctl(fd, SIOCALSECURITY, &mode);
    if(rc)
        printf("Activate al_security failed\n");

    close(fd);

    return;
}


#endif



static int get_filter_mode(char *filter_name)
{
    char value[16] = {0};

    strncpy(value, nvram_safe_get(filter_name), sizeof(value));
    if(strcmp(value,"deny") == 0)
    {
        return MODE_DENY;//仅禁止
    }
    else if(strcmp(value,"pass") == 0)
    {
        return MODE_PASS;//仅允许
    }
    else
    {
        return MODE_DISABLED;//禁用
    }
}
//+++

/* OS layer implementation */
static char *firewall_wanifname(char *wan_ifname)
{
    int unit = wan_primary_ifunit();
    char tmp[100], prefix[] = "wanXXXXXXXXXX_";

    snprintf(prefix, sizeof(prefix), "wan%d_", unit);
    if (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp") ||
        nvram_match(strcat_r(prefix, "proto", tmp), "static") ||
        nvram_match(strcat_r(prefix, "proto", tmp), "8021x"))
    {
        strcpy(wan_ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
    }
    else
    {
        sprintf(wan_ifname, "ppp%d", unit);
    }
    return wan_ifname;
}

static char *firewall_lanifname(int lanid, char *lan_ifname)
{
    char tmp[100];
    char *value;

    if (lan_ifname == 0)
        return 0;

    /* Get lan interface name */
    if (lanid == 0)
    {
        value = nvram_get("lan_ifname");
    }
    else
    {
        snprintf(tmp, sizeof(tmp), "lan%x_ifname", lanid);
        value = nvram_get(tmp);
    }

    if (value == 0)
        return 0;

    /* Get interface ip and netmask */
    strncpy(lan_ifname, value, IFNAMSIZ);
    lan_ifname[IFNAMSIZ] = 0;
    return lan_ifname;
}


/*
 * Flush out all firewall rule
 */
void firewall_ruleflush(void)
{
    int fl = 0;
    int fd;

    fd = open(IPL_NAME, O_RDWR);
    if (fd >= 0)
    {
        fl |= (FR_OUTQUE|FR_INQUE);
        if (ioctl(fd, SIOCIPFFL, (CYG_ADDRWORD)&fl) != 0)
        {
            perror("ioctl(SIOCIPFFL)");
        }
        close(fd);
    }
}
//roy +++,2010/09/20



void macf_flush(void)
{
    int fd,mac_enable;

    fd = open(IPL_NAME, O_RDWR);
    if (fd < 0)
        return;
    ioctl(fd, SIOCFLMACFR, NULL);
//roy+++,2010/09/20
    mac_enable = MODE_DISABLED;
    ioctl(fd, SIOCSMACFIL, &mac_enable);
//+++
    close(fd);
}

struct rule_str {
	int str_len;
	char *str;
	struct rule_str *next;
};

void clear_rule(struct rule_str *rule_hash[], int rule_hash_len);
extern int init_data_rule(char *data_rule);
extern void print_data_rule(void);
#define DATA_RULE_HASH_LEN (30)
#define DATA_FILE_HASH_LEN (10)
extern struct rule_str *data_rule_hash[DATA_RULE_HASH_LEN];
extern struct rule_str *data_file_hash[DATA_FILE_HASH_LEN];
extern char url_log_server[1024];
extern char post_server_ip[16];

//add by ll
void nis_fastcheck_mode(int enable)
{
	int fd = -1;
	int mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
	{
		printf("open IPL_NAME failed\n");
		return;
	}

	mode = enable;
	int rc = ioctl(fd, SIOCSNISFASTCHECK, &mode);
	if(rc) 
		printf("Activate fast NAT failed\n");
	
	close(fd);
	return;
}

int start_data_rule(char *data_rule)
{
	clear_rule(data_rule_hash, DATA_RULE_HASH_LEN);
	clear_rule(data_file_hash, DATA_FILE_HASH_LEN);
	init_data_rule(data_rule);
	print_data_rule();

	nis_fastcheck_mode(1);
	return 0;
}

/*void url_record_init(void)
{
	int fd,mode;

	mode = RECORD_MODE_UNDEFINE;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;
	
	ioctl(fd, SIOCSURLRECORDINIT, &mode);

	close(fd);
	
	return;
}*/

void url_record_start(void)
{
	int fd,mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	mode = RECORD_MODE_ENABLE;//启用

	ioctl(fd, SIOCSURLRECORDMODE, &mode);

	close(fd);
	
	return;
}

void url_record_stop(void)
{
	int fd,mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	mode = RECORD_MODE_DISABLED;//禁用
	
	ioctl(fd, SIOCSURLRECORDMODE, &mode);

	close(fd);
	
	return;
}

void print_url_log_serv()
{
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	printf("url_log_server = %s\n", url_log_server);
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	return;
}

void url_log_start(char *server_path)
{
	memset(url_log_server, 0x0, sizeof(url_log_server));
	strcpy(url_log_server, server_path);
	memset(post_server_ip, 0x0, sizeof(post_server_ip));
	url_record_start();
	print_url_log_serv();

	return;
}

//end by ll

#if 1
//luminais mark
void js_inject_start(void)
{
	int fd,mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	mode = 1;//启用

	ioctl(fd, SIOCJSINJECTCHECK, &mode);

	close(fd);
	
	return;
}

void js_inject_stop(void)
{
	int fd,mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	mode = 0;//禁用

	ioctl(fd, SIOCJSINJECTCHECK, &mode);

	close(fd);
	
	return;
}
//luminais
#endif


/*****************************************************************************
 函 数 名  : get_macfilter_mode
 功能描述  : 获取MAC过滤模式
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 
 修改历史      :
  1.日    期   : 2016年11月26日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int get_macfilter_mode()
{
	return get_filter_mode("filter_mac_mode");
}

/*****************************************************************************
 函 数 名  : set_macfilter_mode
 功能描述  : 设置MAC过滤模式
 输入参数  : int filter_type
 输出参数  : 无
 返 回 值  : static

 修改历史      :
  1.日    期   : 2016年11月26日
    作    者   : liquan
    修改内容   : 新生成函数

    MODE_DISABLED = 0,//禁用
    MODE_DENY,//仅禁止
    MODE_PASS//仅允许
*****************************************************************************/
static void set_macfilter_mode(int filter_type)
{
    int fd,mac_enable;

    fd = open(IPL_NAME, O_RDWR);
    if (fd < 0)
        return;
    mac_enable = filter_type;
    if(mac_enable == MODE_PASS)
		mac_enable = MODE_DENY;
    ioctl(fd, SIOCSMACFIL, &mac_enable);
    close(fd);
}

/*****************************************************************************
 函 数 名  : add_macfilter_rule
 功能描述  : 添加一条macfilter_rule
 输入参数  : unsigned char *mac  
 输出参数  : 无
 返 回 值  : static
 
 修改历史      :
  1.日    期   : 2016年11月26日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void add_macfilter_rule(unsigned char *mac)
{
    struct macfilter macinfo;
    int fd;

    fd = open(IPL_NAME, O_RDWR);
    if (fd < 0)
        return -1;
    memcpy(macinfo.mac, mac, ETHER_ADDR_LEN);
    macinfo.next = NULL;

    if (ioctl(fd, SIOCADMACFR, &macinfo) == 0)
    {
        printf("blacklist ADD mac filter success. [%02x:%02x:%02x:%02x:%02x:%02x]\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    }
   close(fd);
}
/*****************************************************************************
 函 数 名  : remove_macfilter_rule
 功能描述  : 移除一条macfilte_rule
 输入参数  : unsigned char *mac  
 输出参数  : 无
 返 回 值  : static
 
 修改历史      :
  1.日    期   : 2016年11月26日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void remove_macfilter_rule(unsigned char *mac)
{
    struct macfilter macinfo;
    int fd;

    fd = open(IPL_NAME, O_RDWR);
    if (fd < 0)
        return -1;
    memcpy(macinfo.mac, mac, ETHER_ADDR_LEN);
    macinfo.next = NULL;

    if (ioctl(fd, SIOCRMMACFR, &macinfo) == 0)
    {
        printf("blacklist REMOVE mac filter success. [%02x:%02x:%02x:%02x:%02x:%02x]\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    }
    close(fd);
}

int fastfilter_activate(int activate)
{
    int fd, rc;

    int lan_ip,lan_mask;

    fd = open(IPL_NAME, O_RDWR);
    if (fd < 0)
        return -1;
    rc = ioctl(fd, SIOCSFILFAST, &activate);
    if (rc)
        printf("%s fast filtering failed\n", activate?"Activate":"Deactivate");
//add for quebb
#ifdef __CONFIG_TENDA_MULTI__
    if(nvram_match("nvram_oem_quebb","1"))
    {
        rc =  ioctl(fd, SIOCSFIL_QUEBB, &activate);
        if (rc)
            printf("%s quebb filtering failed\n", activate?"Activate":"Deactivate");
    }
#endif
//add 2011/11/04
    /*block package like this:
      *SRC_MAC:any SRC:any|DST_MAC:wan mac DST:lan side ip
      */
    if(activate)
    {
        lan_ip = SYS_lan_ip;
        lan_mask = SYS_lan_mask;
        rc =  ioctl(fd, SIOCADWAN2LAN_IP, &lan_ip);
        if (rc)
            printf("%s wan2lan set lan ip failed\n", activate?"Activate":"Deactivate");

        rc =  ioctl(fd, SIOCADWAN2LAN_MASK, &lan_mask);
        if (rc)
            printf("%s wan2lan set lan mask failed\n", activate?"Activate":"Deactivate");
    }

    rc =  ioctl(fd, SIOCSWAN2LANFIL, &activate);
    if (rc)
        printf("%s wan2lan filtering failed\n", activate?"Activate":"Deactivate");
//end

    close(fd);
    return rc;
}

/* Flush all firewall entries */
void firewall_flush(void)
{
    firewall_inited = 0;

    macf_flush();
#ifdef __CONFIG_URLFILTER__
    firewall_urlf_flush();
#endif /* __CONFIG_URLFILTER__ */
    /* Flush all ipfilter firewall rules */
    firewall_ruleflush();

    /* Disable and unhook fast filter */
    fastfilter_activate(0);

}
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__



/*
    return 0 :  当前时间不在日程范围内
    return 1 :  当前时间在日程范围内，应添加到过滤
*/
int is_time_in_area(struct tm *pt, int stime, int etime, unsigned char *wday)
{
    int yesterday;

    if(pt->tm_wday == 0)
        yesterday = 6;
    else
        yesterday = pt->tm_wday - 1;

    int now_time = pt->tm_hour * 3600 + pt->tm_min * 60 + pt->tm_sec;

    if( stime < etime )//没有跨天
    {
        if(wday[pt->tm_wday] == 1)
        {
            if((now_time >= stime) && (now_time <= etime))
                return 1;
            else
                return 0;
        }
        else
            return 0;
    }
    else if( stime >= etime )//跨天
    {
        if((now_time >= stime)&&(wday[pt->tm_wday] == 1))
            return 1;
        else if((now_time <= etime)&&(wday[yesterday] == 1))
        {
            return 1;
        }
        else
            return 0;
    }
    return 0;
}

void firewall_parent_control_update(void)
{

    int fd;
    int mode;
    int mode_default = MODE_DISABLED;
    int ret = 0;
    time_t now = 0;
    struct tm *time_local = NULL;
    struct macfilter macinfo;
    struct parentCtl_devices_list  *devlist;
    struct backlist_device_list *backlist;
    static int within_time = 0;
    static int without_time = 0;
    fd = open(IPL_NAME, O_RDWR);
    if (fd < 0)
        return;

    /*家长控制处理*/
    if(gParentCtlConfig.devlist == NULL )
        goto EXIT;

    now = time(0);
    time_local = localtime(&now);
    if(time_local == NULL)
        goto EXIT;
    if(time_local->tm_year == 70)
    {
        goto EXIT;
    }

    ret = is_time_in_area(time_local, gParentCtlConfig.stime, gParentCtlConfig.etime, gParentCtlConfig.wday);
    /*0 不在时间段内，执行MAC地址过滤
       1 若在时间段内，执行URL过滤
    */
    if(0 == ret && (firewall_inited == 0 || without_time == 1))
    {

	firewall_inited = 1;
	within_time = 1;
	without_time = 0;
	in_time_area = 0;
        for(devlist = gParentCtlConfig.devlist; devlist != NULL; devlist = devlist->next)
        {
		add_macfilter_rule(devlist->mac);
        }
    }
    else if(1 == ret && (firewall_inited == 0 ||within_time == 1))
    {
    	firewall_inited = 1;
	within_time = 0;
	without_time = 1;
	in_time_area = 1;
      	for(devlist = gParentCtlConfig.devlist; devlist != NULL; devlist = devlist->next)
       {
		remove_macfilter_rule(devlist->mac);
       }

        mode = gParentCtlConfig.mode;
        if(url_filter_mode != mode)
        {
            printf("time is in area. set url filter.mode=%d\n",mode);
            ioctl(fd, SIOCSURLFIL, &mode);
        }
        if(gParentCtlConfig.url_filter_status == STA_ADDED)
            goto EXIT;
        if (ioctl(fd, SIOCADURLFR, NULL) == 0)
            gParentCtlConfig.url_filter_status = STA_ADDED;
    }

EXIT:
    close(fd);
}

void update_parent_ctl_ip(in_addr_t cur_ip, char* cur_mac)
{
    struct parentCtl_devices_list  *ptr;
    for(ptr = gParentCtlConfig.devlist; ptr != NULL; ptr = ptr->next)
    {
        if(strncmp(ptr->mac,cur_mac,6) == 0)
        {
            ptr->ip=cur_ip;
            break;
        }
    }
}

int parent_mac_check(const char * mac)
{
	int flag = 0;
	struct parentCtl_devices_list  *devlist = NULL;
	for(devlist = gParentCtlConfig.devlist; devlist != NULL; devlist = devlist->next)
    {
        	if (!memcmp(mac, devlist->mac, ETHER_ADDR_LEN))
		{

			return 1;
		}
    }
	
#ifdef  __CONFIG_GUEST__
	flag = tenda_arp_is_wireless_client(mac);
	if(0 != flag)
		return 1;
#endif
	return 0;	
}
int firewall_cmd(int argc, char* argv[])
{
    struct parentCtl_devices_list  *ptr;
    struct parentCtl_url_list *entry;
    struct in_addr temp;
    entry = gParentCtlConfig.urllist;
    if(argc == 3)
    {
        if(strcmp(argv[1],"show") == 0)
        {
            printf("SHOW ");
            if(strcmp(argv[2],"dev") == 0)
            {
                printf("DEV LIST:\n");
                for(ptr = gParentCtlConfig.devlist; ptr != NULL; ptr = ptr->next)
                {
                    printf("MAC:%02X:%02X:%02X:%02X:%02X:%02X      ",ptr->mac[0],ptr->mac[1],ptr->mac[2],ptr->mac[3],ptr->mac[4],ptr->mac[5]);
                    temp.s_addr = ptr->ip;
                    printf("IP: %s\n",inet_ntoa(temp));
                }
            }
            else if(strcmp(argv[2],"url") == 0)
            {
                for(entry = gParentCtlConfig.urllist; entry != NULL; entry = entry->next)
                {
                    printf("URL:%s\n",entry->url);
                }
            }
            else if(strcmp(argv[2],"config") == 0)
            {
                printf("START TIME:%d END TIME:%d\n",gParentCtlConfig.stime, gParentCtlConfig.etime);

                printf("sunday select=%d\n",gParentCtlConfig.wday[0]);
                printf("Monday select=%d\n",gParentCtlConfig.wday[1]);
                printf("Tuesday select=%d\n",gParentCtlConfig.wday[2]);
                printf("Wednesday select=%d\n",gParentCtlConfig.wday[3]);
                printf("Thursday select=%d\n",gParentCtlConfig.wday[4]);
                printf("Friday select=%d\n",gParentCtlConfig.wday[5]);
                printf("Saturday select=%d\n",gParentCtlConfig.wday[6]);

                printf("MDOE:%d\n",gParentCtlConfig.mode);
            }
            else
            {
                printf("fw [show\\set\\delete][dev\\url\\config][]\n");
            }
        }

    }
    else
    {
        printf("fw [show\\set\\delete][dev\\url\\config][]\n");
    }

	return 0;	//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改	
}


#endif
#ifdef __CONFIG_WL_MAC_FILTER__
extern void refresh_acl_table();
#endif
/*****************************************************************************
 函 数 名  : init_macfilter
 功能描述  : 初始化mac过滤
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月30日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int init_macfilter(void)
{
	
	char name[] = "filter_macXXXXXXXXXX", value[1000];
	char *filter_mac = NULL, *desc = NULL;
	int macfilter_mode = 0;
	int which = 0;
	int fd;
	int mode = MODE_DENY;
	struct ether_addr *hw_addr = NULL;
	struct macfilter macinfo;

	fd = open(IPL_NAME, O_RDWR);
    	if (fd < 0)
        	return FALSE;		//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改
	ioctl(fd, SIOCSMACFIL, &mode);

	macfilter_mode = get_macfilter_mode();
	if(MODE_PASS == macfilter_mode)
	{
		close(fd);
		return FALSE;	//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改
	}   
	for (which = 0; which < 20; which++) {
		
		snprintf(name, sizeof(name), "filter_mac%d", which);
		strncpy(value, nvram_safe_get(name), sizeof(value));
		
		desc = value;
		filter_mac = strsep(&desc, ",");
		if (!filter_mac){
			continue;
		}
		
		hw_addr = ether_aton(filter_mac);
		if(!hw_addr){
			continue;
		}

		memcpy(macinfo.mac, hw_addr->octet, ETHER_ADDR_LEN);
        	macinfo.next = NULL;

        	if (ioctl(fd, SIOCADMACFR, &macinfo) == 0)
        	{
            		printf("blacklist ADD mac filter success. [%02x:%02x:%02x:%02x:%02x:%02x]\n",macinfo.mac[0],macinfo.mac[1],macinfo.mac[2],
						macinfo.mac[3],macinfo.mac[4],macinfo.mac[5]); 
        	}
		
    }
	close(fd);
	return TRUE;
	
}
/* Initialize firewall */

void firewall_init(void)
{
    	char wan_ifname[32];
#ifdef __CONFIG_WL_MAC_FILTER__
    	refresh_acl_table();
#endif

    firewall_wanifname(wan_ifname);
    if (iflib_getifaddr(wan_ifname, 0, 0) != 0)
        return;

	if(nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		||nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
	{
		//桥模式下不需要开启fast filter等功能
		printf("%s dut is not router mode\n",__func__);
		return;
	}
    /* Enable fast filter */
    fastfilter_activate(1);

    init_macfilter();
    parent_control_config_update();
    firewall_parent_control_update();

    /* Create restriction filters among bridges */
}

/* Print frentry list */
static void
printlist(frentry_t *fp)
{
    for (; fp; fp = fp->fr_next)
    {
        printf("[%u hits] ", (uint)fp->fr_hits);
        printfr(fp);
        if (fp->fr_grp)
            printlist(fp->fr_grp);
    }
}
