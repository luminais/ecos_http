#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include "sys_timer.h"
#include "debug.h"
#include "firewall.h"

static FIREWALL_INFO_STRUCT firewall_info;

static RET_INFO tpi_firewall_start();
static RET_INFO tpi_firewall_stop();
static RET_INFO tpi_firewall_restart();

#ifndef __CONFIG_A9__
extern void firewall_parent_control_update(void);
#endif
#define _FW_FILTER_CLIENT(i)        racat("filter_client",i) 
#define _FW_FILTER_URL(i)           racat("filter_url",i)


/*获取各个子模块的信息start*/
static RET_INFO tpi_dmz_update_info()
{
    firewall_info.dmz_info.dmz_ipaddr_en = nvram_safe_get("dmz_ipaddr_en");
    firewall_info.dmz_info.dmz_ipaddr = nvram_safe_get("dmz_ipaddr");

    return RET_SUC;
}

static RET_INFO tpi_hostfilter_update_info()
{
    PIU32 i = 0;
	
    firewall_info.hostf_info.filter_client_mode   = nvram_safe_get("filter_client_mode");
    firewall_info.hostf_info.filter_client_cur_nu = nvram_safe_get("filter_client_cur_nu");
	
    for(i = 0;i < FW_HOST_FILTER_MAX_NUM;i++)
    {
        firewall_info.hostf_info.filter_client[i] = nvram_safe_get(_FW_FILTER_CLIENT(i));
    }
	
    return RET_SUC;
}

static RET_INFO tpi_macfilter_update_info()
{
    PIU32 i = 0;
	
    firewall_info.macf_info.filter_mac_mode   = nvram_safe_get("filter_mac_mode");
    firewall_info.macf_info.filter_mac_cur_nu = nvram_safe_get("filter_mac_cur_nu");
	
    for(i = 0;i < FW_MAC_FILTER_MAX_NUM;i++)
    {
        firewall_info.macf_info.filter_mac[i] = nvram_safe_get(ADVANCE_MACFILTER_DENY_RULE(i));
    }
	
    return RET_SUC;
}

static RET_INFO tpi_urlfilter_update_info()
{
    PIU32 i = 0;
	
    firewall_info.urlf_info.filter_url_mode   = nvram_safe_get("filter_url_mode");
    firewall_info.urlf_info.filter_url_cur_nu = nvram_safe_get("filter_url_cur_nu");
	
	for(i = 0;i < FW_URL_FILTER_MAX_NUM;i++)
    {
        firewall_info.urlf_info.filter_url[i] = nvram_safe_get(_FW_FILTER_URL(i));
    }

    return RET_SUC;
}

static RET_INFO tpi_portfoward_update_info()
{
    PIU32 i = 0;
	
    firewall_info.portfd_info.forward_port_list = nvram_safe_get("forward_port_list");

	for(i = 0;i < FW_PORTFORWAD_MAX_NUM;i++)
    {
        firewall_info.portfd_info.forward_port[i] = nvram_safe_get(ADVANCE_PORT_FORWART_RULE(i));
    }
	
    return RET_SUC;
}
/*获取各个子模块的信息end*/

/*以下函数用于api调用*/
RET_INFO tpi_firewall_update_info()
{
    if(0 == strcmp("0",nvram_safe_get("fw_disable")))
        firewall_info.enable = 1;
    else
        firewall_info.enable = 0;

    tpi_dmz_update_info();
    tpi_hostfilter_update_info();
    tpi_macfilter_update_info();
    tpi_urlfilter_update_info();
    tpi_portfoward_update_info();

    return RET_SUC;
}

static void tpi_parent_control_timer_init()
{
	#ifndef __CONFIG_A9__
    DO_TIMER_FUN timer;
	
    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,PARENT_CTL_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME;
    timer.fun = firewall_parent_control_update;
    sys_do_timer_add(&timer);
	#endif
	return;
}

RET_INFO tpi_firewall_struct_init()
{
    memset(&firewall_info,0x0,sizeof(firewall_info));
    return tpi_firewall_update_info();
}

RET_INFO tpi_firewall_first_init()
{
    tpi_parent_control_timer_init();
    return tpi_firewall_start();
}

RET_INFO tpi_firewall_action(RC_MODULES_COMMON_STRUCT *var)
{
    PI_PRINTF(TPI,"op=%d\n",var->op);
    switch(var->op)
    {
        case OP_START:
            tpi_firewall_start();
            break;
        case OP_STOP:
            tpi_firewall_stop();
            break;
        case OP_RESTART:
            tpi_firewall_restart();
            break;
        default:
            PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
            break;
    }

    return RET_SUC;
}

#ifdef __CONFIG_TC__
RET_INFO tpi_tc_firewall_action(RC_MODULES_COMMON_STRUCT *var)
{
    PI_PRINTF(TPI,"op=%d\n",var->op);
    switch(var->op)
    {
        case OP_START:
        case OP_STOP:
        case OP_RESTART:
            tpi_filter_restart();
            break;
        default:
            PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
            break;
    }

    return RET_SUC;
}
#endif

/*以下用于gpi获取信息函数*/
P_FIREWALL_INFO_STRUCT tpi_firewall_get_info()
{
	tpi_firewall_update_info();
    return &firewall_info;
}

/*其他线程调用*/
#if defined(__CONFIG_PPTP__) || defined(__CONFIG_L2TP__) || defined(__CONFIG_PPPOE2__)
void tpi_firewall_update_tunnel(PI8 *pppname)
{
	PIU8 unit;
	PI8 tmp[100], prefix[] = "wanXXXXXXXXXX_";
	PI8 *tunnel_ifname;

	unit = ppp_ifunit(pppname);
	if (unit < 0)
		return;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	/* Get bound ifname */
	tunnel_ifname = nvram_get(strcat_r(prefix, "bind_ifname", tmp));
	if (tunnel_ifname) {
	 	/* Set tunnel firewall and nat rules */
		firewall_basic_rule(tunnel_ifname);
		ipnat_napt_init(tunnel_ifname);
	}
}
#endif

#if defined(CONFIG_RTL_HARDWARE_NAT)
extern int flush_hw_table_flag;
#endif
/*由于当前的TC是放在协议栈之前做的，所以不需要考虑
走加速的问题，不进行重启nat，只重启filter*/
RET_INFO tpi_filter_restart()
{
	firewall_flush();
	tpi_firewall_update_info();
	if(1 == firewall_info.enable)
	{
		firewall_init();
		return RET_SUC;
	}
	else
	{
	       PI_ERROR(TPI,"the mib is off, connot start!\n");
		return RET_ERR;
	}
	
}
/*本文件调用*/
static RET_INFO tpi_firewall_start()
{
    tpi_firewall_update_info();
    if(1 == firewall_info.enable)
    {
#ifndef __CONFIG_A9__
    	firewall_init();
#endif
        ipnat_init();
        PI_PRINTF(TPI,"start success!\n");
    }
    else
    {
        PI_ERROR(TPI,"the mib is off, connot start!\n");
    }
	

#if defined(CONFIG_RTL_HARDWARE_NAT)
	flush_hw_table_flag = 1;
	rtl_hwNatOnOffByApp();
#endif
	
    return RET_SUC;
}

static RET_INFO tpi_firewall_stop()
{
    tpi_firewall_update_info();

    ipnat_deinit();
#ifndef __CONFIG_A9__
	firewall_flush();
#endif
    PI_PRINTF(TPI,"stop success!\n");
    return RET_SUC;
}

static RET_INFO tpi_firewall_restart()
{
    RET_INFO ret = RET_SUC;

    if(RET_ERR == tpi_firewall_stop() || RET_ERR == tpi_firewall_start())
    {
        PI_ERROR(TPI,"restart error!\n");
    }

    return ret;
}
