/*
*
*/
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <wlutils.h>
#include <net/if_arp.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <iflib.h>
#include <rc.h>
#include <router_net.h>
#include "wan_mode_check.h"
#include "wan.h"

/*Wan information struct,for whicth contains all the interfaces information.*/
static WAN_INFO_STRUCT wan_info;
static MODULE_WORK_STATUS wan_work_status = MODULE_BEGIN;

// added for traffic control with the fastnat of realtek, by zhuhuan on 2016.03.01
#ifndef CONFIG_RTL_QOS_RATE_LIMIT_CHECK
#define CONFIG_RTL_QOS_RATE_LIMIT_CHECK
extern void rtl_set_wan_if_name(void);
#endif

static PIU8 tpi_wan_link_check();
static void tpi_wan_link_timer_init();
static RET_INFO tpi_wan_stop_handle();
static PIU8 tpi_wan_valid(char *ifname);
static RET_INFO tpi_wan_start_eth_handle(PI8 *wan_ifname);
static RET_INFO tpi_wan_before_start_handle();
static void tpi_wan_set_mtu(PIU32 mtu);
static RET_INFO tpi_wan_static_start();
static RET_INFO tpi_wan_dhcpc_start();
static RET_INFO tpi_wan_pppoe_start();
static RET_INFO tpi_wan_start_handle();

static RET_INFO tpi_wan_get_err_info(P_WAN_ERR_INFO_STRUCT p);
static RET_INFO tpi_wan_get_status(SYS_WORK_MODE wl_mode,WIFISTASTATUS * wifi_status);
static PIU32 tpi_wan_get_err_result(P_WAN_ERR_INFO_STRUCT p);
static PPPOEERRCODE tpi_wan_get_pppoe_err_result();
static DHCPERRCODE tpi_wan_get_dhcp_err_result();
static RET_INFO tpi_wan_get_connected_err_info(P_WAN_ERR_INFO_STRUCT p);
static RET_INFO tpi_wan_get_connecting_err_info(P_WAN_ERR_INFO_STRUCT p,WIFISTASTATUS wifi_status);
#ifdef __CONFIG_GUEST__
extern int gpi_wifi_check_ip_confilct_by_wan(unsigned int wan_ip,unsigned int wan_mask);
#endif
extern void del_all_ns();

/*API中调用*/
RET_INFO tpi_wan_update_info()
{
	/*wan_type wan_old_type*/
	strcpy__(wan_info.wan_proto,nvram_safe_get("wan0_proto"));
	if(0 == strcmp(wan_info.wan_proto,"static"))
		wan_info.wan_type = WAN_STATIC_MODE;
	else if(0 == strcmp(wan_info.wan_proto,"dhcp"))
		wan_info.wan_type = WAN_DHCP_MODE;
	else if(0 == strcmp(wan_info.wan_proto,"pppoe"))
		wan_info.wan_type = WAN_PPPOE_MODE;
	else
		wan_info.wan_type = WAN_NONE_MODE;

	PI8 wan0_old_type[PI_BUFLEN_16] = {0};
	strcpy__(wan0_old_type,nvram_safe_get("wan0_bind_proto"));
	if(0 == strcmp(wan0_old_type,"static"))
		wan_info.wan_old_type = WAN_STATIC_MODE;
	else if(0 == strcmp(wan0_old_type,"dhcp"))
		wan_info.wan_old_type = WAN_DHCP_MODE;
	else if(0 == strcmp(wan0_old_type,"pppoe"))
		wan_info.wan_old_type = WAN_PPPOE_MODE;
	else
		wan_info.wan_old_type = WAN_NONE_MODE;

	/*wan_status*/
	PI8 wan_status[PI_BUFLEN_16] = {0};
	strcpy__(wan_status,nvram_safe_get("wan0_connect"));
	if(0 == strcmp(wan_status,"Connecting"))
	{
		wan_info.wan_status = WAN_CONNECTING;
	}
	else if(0 == strcmp(wan_status,"Connected"))
	{
		wan_info.wan_status = WAN_CONNECTED;
	}
	else
	{
		if(tenda_wan_link_status())
			wan_info.wan_status = WAN_DISCONNECTED;
		else
			wan_info.wan_status = WAN_NO_WIRE;
	}

	/*wan_err_info wan_err_result*/
	tpi_wan_get_err_info(&wan_info.wan_err_info);
	wan_info.wan_err_result = tpi_wan_get_err_result(&wan_info.wan_err_info);

	/*wan_speed*/
	wan_info.wan_speed = atoi(nvram_safe_get("wan_speed"));

	/*wan_hwaddr_info*/
	PI8 wan0_macclone_mode[PI_BUFLEN_16] = {0};
	strcpy__(wan0_macclone_mode,nvram_safe_get("wan0_macclone_mode"));
	if(0 == strcmp(wan0_macclone_mode,"clone"))
	{
		wan_info.wan_hwaddr_info.wan_hwaddr_type = WAN_MAC_PC;
	}
	else if(0 == strcmp(wan0_macclone_mode,"manual"))
	{
		wan_info.wan_hwaddr_info.wan_hwaddr_type = WAN_MAC_HAND;
	}
	else
	{
		wan_info.wan_hwaddr_info.wan_hwaddr_type = WAN_MAC_DEFAULT;
	}

	if(wan_info.wan_err_info.wl_mode == WL_ROUTE_MODE)
		strcpy__(wan_info.wan_hwaddr_info.wan_hwaddr,nvram_safe_get("wan0_hwaddr"));
	else if(wan_info.wan_err_info.wl_mode == WL_WISP_MODE || wan_info.wan_err_info.wl_mode == WL_APCLIENT_MODE)
	{
		if(nvram_match(WLAN24G_WORK_MODE,"sta"))
			strcpy__(wan_info.wan_hwaddr_info.wan_hwaddr,nvram_safe_get(WLAN24G_REPEATER_MAC));
		else
			strcpy__(wan_info.wan_hwaddr_info.wan_hwaddr,nvram_safe_get(WLAN5G_REPEATER_MAC));
	}
	else
		strcpy__(wan_info.wan_hwaddr_info.wan_hwaddr,nvram_safe_get("et0macaddr"));
	strcpy__(wan_info.wan_hwaddr_info.wan_default_hwaddr,nvram_safe_get("et0macaddr"));

	/*wan_ifname wan_old_ifname*/
	if(wan_info.wan_old_type == WAN_PPPOE_MODE)
	{
		strcpy__(wan_info.wan_old_ifname,nvram_safe_get("wan0_pppoe_ifname"));
	}
	else
	{
		strcpy__(wan_info.wan_old_ifname,wan_info.wan_ifname);
	}
	strcpy__(wan_info.wan_ifname,nvram_safe_get("wan0_ifname"));

	/*dhcp info*/
	strcpy__(wan_info.wan_dhcp_info.wan_hostname,nvram_safe_get("wan0_hostname"));
	wan_info.wan_dhcp_info.mtu = atoi(nvram_safe_get("dhcp_wan0_mtu"));

	/*static info*/
	memset(wan_info.wan_static_info.wan_static_dns1,0x0,PI_IP_STRING_LEN);
	memset(wan_info.wan_static_info.wan_static_dns2,0x0,PI_IP_STRING_LEN);

	PI8 wan0_dns1[PI_IP_STRING_LEN] = {0},wan0_dns2[PI_IP_STRING_LEN] = {0};
	wan_info.wan_static_info.mtu = atoi(nvram_safe_get("static_wan0_mtu"));
	strcpy__(wan_info.wan_static_info.wan_static_ip,nvram_safe_get("wan0_ipaddr"));
	strcpy__(wan_info.wan_static_info.wan_static_mask,nvram_safe_get("wan0_netmask"));
	strcpy__(wan_info.wan_static_info.wan_static_gw,nvram_safe_get("wan0_gateway"));
	sscanf(nvram_safe_get("wan0_static_dns"),"%[^ ] %s",wan_info.wan_static_info.wan_static_dns1,wan_info.wan_static_info.wan_static_dns2);

	/*pppoe info*/
	wan_info.wan_pppoe_info.mtu = atoi(nvram_safe_get("pppoe_wan0_mtu"));
	wan_info.wan_pppoe_info.unit = atoi(nvram_safe_get("wan0_unit"));
	strcpy__(wan_info.wan_pppoe_info.wan_pppoe_username,nvram_safe_get("wan0_pppoe_username"));
	strcpy__(wan_info.wan_pppoe_info.wan_pppoe_password,nvram_safe_get("wan0_pppoe_passwd"));

	/*connect time*/
	wan_info.connect_time = ((wan_info.wan_status == WAN_CONNECTED)&&(SYS_wan_conntime))?((cyg_current_time()>SYS_wan_conntime)?(cyg_current_time() - SYS_wan_conntime)/100:0):0;

	/*cur info*/
	memset(wan_info.wan_cur_dns1,0x0,PI_IP_STRING_LEN);
	memset(wan_info.wan_cur_dns2,0x0,PI_IP_STRING_LEN);

	if(wan_info.wan_type == WAN_STATIC_MODE)
	{
		strcpy__(wan_info.wan_cur_ipaddr,wan_info.wan_static_info.wan_static_ip);
		strcpy__(wan_info.wan_cur_mask,wan_info.wan_static_info.wan_static_mask);
		strcpy__(wan_info.wan_cur_gw,wan_info.wan_static_info.wan_static_gw);
		strcpy__(wan_info.wan_cur_dns1,wan_info.wan_static_info.wan_static_dns1);
		strcpy__(wan_info.wan_cur_dns2,wan_info.wan_static_info.wan_static_dns2);
	}
	else
	{
		strcpy__(wan_info.wan_cur_ipaddr,(wan_info.wan_status == WAN_CONNECTED)?NSTR(SYS_wan_ip):"");
		strcpy__(wan_info.wan_cur_mask,(wan_info.wan_status == WAN_CONNECTED)?NSTR(SYS_wan_mask):"");
		strcpy__(wan_info.wan_cur_gw,(wan_info.wan_status == WAN_CONNECTED)?NSTR(SYS_wan_gw):"");
		if(wan_info.wan_type == WAN_DHCP_MODE)
		{
			sscanf(nvram_safe_get("wan0_dns"),"%[^ ] %s",wan_info.wan_cur_dns1,wan_info.wan_cur_dns2);
		}
		else
		{
			sscanf(nvram_safe_get("wan0_autodns"),"%[^ ] %s",wan_info.wan_cur_dns1,wan_info.wan_cur_dns2);
		}
	}

	return RET_SUC;
}

RET_INFO tpi_wan_struct_init()
{
	memset(&wan_info,0x0,sizeof(wan_info));

	nvram_set("wan0_connect","Disconnected");
	nvram_set("err_check","0");
	nvram_unset("resolv_conf");


	if (!strcmp(nvram_safe_get("wan0_hwaddr"), ""))
	{
		nvram_set("wan0_hwaddr", nvram_safe_get("et0macaddr"));
	}

	return tpi_wan_update_info();
}

RET_INFO tpi_wan_first_init()
{
	/*WAN模块比较特殊，由定时器去拉起WAN口,这里不起WAN口*/
	tpi_wan_link_timer_init();
	/*配置双工模式*/
	rtl_set_wan_if_name();
	ifr_set_link_speed2(wan_info.wan_speed);

	del_all_ns();

	return RET_SUC;
}

static void tpi_wan_set_action_complete()
{
	wan_work_status = MODULE_COMPLETE;
}

MODULE_WORK_STATUS tpi_wan_get_action_type()
{
	return wan_work_status;
}

void tpi_wan_action_reinit()
{
	wan_work_status = MODULE_BEGIN;
	return ;
}
RET_INFO tpi_wan_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO    ret = RET_SUC;

	if(RET_SUC != tpi_wan_update_info())
	{
		PI_PRINTF(TPI,"update error!\n");
		return RET_ERR;
	}
	//根据coverity分析结果修改，原来为无效的判断:if(var->string_info)  2017/1/11 F9项目修改
	if(var != NULL)
	{
		if(0 == strcmp(var->string_info,"WanSpeedChange"))
		{

			if(RET_SUC != tpi_wan_stop_handle())
			{
				PI_ERROR(TPI,"WanSpeedChange fail!\n");
				ret = RET_ERR;
			}
			ifr_set_link_speed2(wan_info.wan_speed);
			if(RET_SUC != tpi_wan_start_handle())
			{
				PI_ERROR(TPI,"WanSpeedChange fail!\n");
				ret = RET_ERR;
			}
			return ret;
		}
	}

	tpi_wan_action_reinit();

	switch(var->op)
	{
		case OP_START:
			ret = tpi_wan_start_handle();
			break;
		case OP_STOP:
			ret = tpi_wan_stop_handle();
			break;
		case OP_RESTART:
			if(RET_SUC != tpi_wan_stop_handle() || RET_SUC != tpi_wan_start_handle())
			{
				ret = RET_ERR;
			}
			break;
		default:
			PI_ERROR(TPI,"OP[%d] don not have handle!\n",var->op);
			break;
	}

	tpi_wan_set_action_complete();

	return ret;
}

RET_INFO tpi_wan_systools_action(RC_MODULES_COMMON_STRUCT *var)
{
	//根据coverity分析结果修改，原来为无效的判断:NULL == var->string_info  2017/1/11 F9项目修改
	if(NULL == var || 0 == strcmp("",var->string_info))
	{
		return RET_ERR;
	}

	if(0 == memcmp("reboot",var->string_info,strlen("reboot")))
	{
		tpi_wan_stop_handle();
	}

	return RET_SUC;
}

/*GPI中调用*/
PIU32 tpi_wan_get_err_result_info()
{
	tpi_wan_get_err_info(&wan_info.wan_err_info);
	return tpi_wan_get_err_result(&wan_info.wan_err_info);
}

P_WAN_ERR_INFO_STRUCT tpi_wan_get_err_info_other()
{
	tpi_wan_get_err_info(&wan_info.wan_err_info);
	return (&wan_info.wan_err_info);
}

P_WAN_INFO_STRUCT tpi_wan_get_info()
{
	tpi_wan_update_info();
	return &wan_info;
}

P_WAN_HWADDR_INFO_STRUCT tpi_wan_get_hwaddr_info()
{
	tpi_wan_update_info();
	return &wan_info.wan_hwaddr_info;
}

P_WAN_CURRCET_INFO_STRUCT tpi_wan_get_currcet_info(P_WAN_CURRCET_INFO_STRUCT ptr)
{
	PIU8 wan_speed = WAN_SPEED_AUTO;
	if(ptr)
	{
		if(WAN_STATIC_MODE == wan_info.wan_type)
			ptr->mtu = wan_info.wan_static_info.mtu;
		else if(WAN_DHCP_MODE == wan_info.wan_type)
			ptr->mtu = wan_info.wan_dhcp_info.mtu;
		else if(WAN_PPPOE_MODE == wan_info.wan_type)
			ptr->mtu = wan_info.wan_pppoe_info.mtu;
		else
			ptr->mtu = 1500;

		ptr->wan_speed = WAN_SPEED_AUTO;

		if(wan_info.wan_err_info.wl_mode == WL_ROUTE_MODE)
		{
			wan_speed = ifr_link_speed("eth1");
			if(0 == wan_speed)
			{
				ptr->wan_speed = WAN_SPEED_10_HALF;
			}
			else if(1 == wan_speed)
			{
				ptr->wan_speed = WAN_SPEED_10_FULL;
			}
			else if(2 == wan_speed)
			{
				ptr->wan_speed = WAN_SPEED_100_HALF;
			}
			else if(3 == wan_speed)
			{
				ptr->wan_speed = WAN_SPEED_100_FULL;
			}
		}
	}
	return ptr;
}

/*具体实现函数*/

/*其他线程中可能使用*/

/*本文件中使用*/
static PIU8 tpi_wan_link_check()
{
	return wan_link_check();
}

static void tpi_wan_link_timer_init()
{
	DO_TIMER_FUN timer;
	memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,WAN_LINK_TIMER);
	timer.enable = DO_TIMER_ON;
	timer.sleep_time = DO_TIMER_MIN_TIME;
	timer.fun = tpi_wan_link_check;
	sys_do_timer_add(&timer);

	return;
}

static RET_INFO tpi_wan_stop_handle()
{
	RET_INFO ret = RET_SUC;

	PI_PRINTF(TPI,"----[%s]wan down!---\n",(wan_info.wan_old_type == WAN_STATIC_MODE)?"static":((wan_info.wan_type == WAN_DHCP_MODE)?"DHCP":"PPP"));

	nvram_set("err_check","0");
	nvram_set("wan0_connect","Disconnected");
#ifdef __CONFIG_WAN_SURF_CHECK__
	tpi_wan_surf_check_set_result(INTERNET_NO);
#endif

	switch(wan_info.wan_old_type)
	{
		case WAN_STATIC_MODE:
			del_all_ns();
			wan_down(wan_info.wan_old_ifname);
			break;
		case WAN_DHCP_MODE:
			del_all_ns();
			dhcpc_stop(wan_info.wan_old_ifname);
			break;
#ifdef __CONFIG_PPPOE__				//added by yp,2016-3-17
		case WAN_PPPOE_MODE:
			del_all_ns();
			wan_pppoe_down(wan_info.wan_old_ifname);
			break;
#endif
		default:
			ret = RET_ERR;
			PI_ERROR(TPI,"wan_type[%d] error!\n",wan_info.wan_old_type);
			break;
	}

	return RET_SUC;
}

static PIU8 tpi_wan_valid(char *ifname)
{
	char name[80] = {0};
	char *next = NULL;

	if(NULL == ifname)
		PI_ERROR(TPI,"wan0_ifname fail!\n");

	foreach(name, nvram_safe_get("wan_ifnames"), next)
	{
		if (ifname && !strcmp(ifname, name))
		{
			return 1;
		}
	}
	return 0;
}

static RET_INFO tpi_wan_start_eth_handle(PI8 *wan_ifname)
{
	int s;
	char eabuf[PI_BUFLEN_32] = {0},tmp[PI_BUFLEN_32] = {0};
	struct ifreq ifr;

	//tenda add

	if(WL_ROUTE_MODE != gpi_wifi_get_mode())
		tenda_ifconfig(wan_ifname, IFF_UP, "0.0.0.0", NULL);
	else
		tenda_ifconfig(wan_ifname, 0, NULL, NULL);


	/* Set i/f hardware address before bringing it up */
	if((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
	{
		PI_PRINTF(TPI,"\n");
		return RET_ERR;
	}
	strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
	ifr.ifr_hwaddr.sa_family = AF_LINK;

	/* Configure i/f only once, specially for wl i/f shared by multiple connections */
	if(ioctl(s, SIOCGIFFLAGS, &ifr))
	{
		close(s);
		PI_PRINTF(TPI,"\n");
		return RET_ERR;
	}

	if(!(ifr.ifr_flags & IFF_UP))
	{
		/* Sync connection nvram address and i/f hardware address */
		memset(ifr.ifr_hwaddr.sa_data, 0, ETHER_ADDR_LEN);
		if(!strcmp(wan_info.wan_hwaddr_info.wan_hwaddr, "") ||
		   !ether_atoe(wan_info.wan_hwaddr_info.wan_hwaddr,
		               (unsigned char *)ifr.ifr_hwaddr.sa_data) ||
		   !memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN))
		{
			if(ioctl(s, SIOCGIFHWADDR, &ifr))
			{
				close(s);
				PI_PRINTF(TPI,"\n");
				return RET_ERR;
			}
			nvram_set(("wan0_hwaddr"),ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		}
		else
		{
			ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
			ioctl(s, SIOCSIFHWADDR, &ifr);
		}
		/* Bring up i/f */
		tenda_ifconfig(wan_ifname, IFUP, NULL, NULL);

	}
	close(s);

	return RET_SUC;
}

static RET_INFO tpi_wan_before_start_handle()
{
	RET_INFO ret = RET_SUC;

	/* Don't start WAN when in bridge mode */
	if(nvram_match(SYSCONFIG_WORKMODE, "client+ap")
	   || nvram_match(SYSCONFIG_WORKMODE, "bridge"))
		return RET_ERR;

	/* Clear bound_ifname */
	nvram_unset("wan0_bind_proto");

	/* disable the connection if the i/f is not in wan_ifnames */
	if(!tpi_wan_valid(wan_info.wan_ifname))
	{
		nvram_set("wan0_proto", "disabled");
	}
	ret = tpi_wan_start_eth_handle(wan_info.wan_ifname);

	nvram_set("wan0_bind_proto",wan_info.wan_proto);

	return ret;
}

static void tpi_wan_set_mtu(PIU32 mtu)
{
	PI8 wan0_mtu[PI_BUFLEN_16] = {0};

	sprintf(wan0_mtu,"%d",mtu);
	nvram_set("wan0_mtu",wan0_mtu);

	return;
}

static RET_INFO tpi_wan_static_start()
{
	int mtu;
	/*add by gong*/
	struct in_addr g_in_ipaddr = {0};
	struct in_addr g_in_netmask = {0};
	struct in_addr g_in_gateway = {0};
	PI8 dns[2*PI_IP_STRING_LEN]= {0};

	tpi_wan_set_mtu(wan_info.wan_static_info.mtu);

	/*For resolving a request by clientele,which is about the alterable mask*/
	inet_aton(wan_info.wan_static_info.wan_static_ip, &g_in_ipaddr);
	inet_aton(wan_info.wan_static_info.wan_static_mask, &g_in_netmask);
	inet_aton(wan_info.wan_static_info.wan_static_gw, &g_in_gateway);
#ifdef __CONFIG_GUEST__
	gpi_wifi_check_ip_confilct_by_wan(g_in_ipaddr.s_addr, g_in_netmask.s_addr);
#endif	
	if((g_in_ipaddr.s_addr & g_in_netmask.s_addr) == (g_in_gateway.s_addr & g_in_netmask.s_addr))
	{
		tenda_ifconfig(wan_info.wan_ifname, IFUP,wan_info.wan_static_info.wan_static_ip,wan_info.wan_static_info.wan_static_mask);
	}
	else
	{
		tenda_ifconfig(wan_info.wan_ifname, IFUP, wan_info.wan_static_info.wan_static_ip, "255.0.0.0");
	}

	/*Get the mtu*/
	mtu = wan_info.wan_static_info.mtu;
	if(mtu > 0)
		ifconfig_mtu(wan_info.wan_ifname,mtu);

#ifdef CONFIG_RTL_HARDWARE_NAT
#ifndef HAVE_NOETH
	extern int rtl_setWanNetifMtu(int mtu);
	rtl_setWanNetifMtu(mtu);
#endif
#endif

	sprintf(dns,"%s %s",wan_info.wan_static_info.wan_static_dns1,wan_info.wan_static_info.wan_static_dns2);

	add_ns(dns, NULL, ADD_NS_PUSH);
	/* We are done configuration */
	wan_up(wan_info.wan_ifname);

	return RET_SUC;
}

static RET_INFO tpi_wan_dhcpc_start()
{
	tpi_wan_set_mtu(wan_info.wan_dhcp_info.mtu);
	dhcpc_start(wan_info.wan_ifname, "wandhcpc", wan_info.wan_dhcp_info.wan_hostname);

	return RET_SUC;
}

static RET_INFO tpi_wan_pppoe_start()
{
	tpi_wan_set_mtu(wan_info.wan_pppoe_info.mtu);
	wan_pppoe_start(wan_info.wan_pppoe_info.unit);

	return RET_SUC;
}

extern int gHwNatEnabled;
extern void rtl865x_arp_init(void);
static RET_INFO tpi_wan_start_handle()
{
	RET_INFO ret = RET_SUC;

	if(0 == tenda_wan_link_status())
	{
		PI_PRINTF(TPI,"wan_link is not ok.connot start wan!\n");
		return RET_ERR;
	}
	if(wan_info.wan_err_info.wl_mode == WL_APCLIENT_MODE || RET_ERR == tpi_wan_before_start_handle())
	{
		PI_PRINTF(TPI,"wl_mode=%d,tpi_wan_before_start_handle error!\n",wan_info.wan_err_info.wl_mode);
		return RET_ERR;
	}

#if defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
	rtl_set_wan_if_name();
#endif

	PI_PRINTF(TPI,"----[%s]wan up!---\n",(wan_info.wan_type == WAN_STATIC_MODE)?"static":((wan_info.wan_type == WAN_DHCP_MODE)?"DHCP":"PPP"));

	nvram_set("err_check","0");
	nvram_set("wan0_connect","Connecting");

#ifdef __CONFIG_WAN_SURF_CHECK__
	tpi_wan_surf_check_set_result(INTERNET_TRY);
#endif
	/*当wan口启动的时候关闭硬件加速，清楚eth arp*/
	if(gHwNatEnabled)
	{
	//	rtl_hwNatOnOff(0);
	//	rtl865x_arp_init();
	}
	
	switch(wan_info.wan_type)
	{
		case WAN_STATIC_MODE:
			tpi_wan_static_start();
			break;
		case WAN_DHCP_MODE:
			tpi_wan_dhcpc_start();
			break;
#ifdef __CONFIG_PPPOE__         //added by yp, 2016-3-17
		case WAN_PPPOE_MODE:
			tpi_wan_pppoe_start();	//这里进行起PPPOE服务器进行拨号
			break;
#endif
		default:
			ret = RET_ERR;

			nvram_set("wan0_connect","Disconnected");
#ifdef __CONFIG_WAN_SURF_CHECK__
			tpi_wan_surf_check_set_result(INTERNET_NO);
#endif
			PI_ERROR(TPI,"wan_type[%d] error!\n",wan_info.wan_type);
			break;
	}
	return RET_SUC;
}

static void tpi_wan_get_route_status(WIFISTASTATUS * wifi_status)
{
	PI8 wan_status[PI_BUFLEN_16] = {0};
	strcpy__(wan_status,nvram_safe_get("wan0_connect"));
	if(0 == strcmp(wan_status,"Connecting"))
	{
		wan_info.wan_status = WAN_CONNECTING;
	}
	else if(0 == strcmp(wan_status,"Connected"))
	{
		wan_info.wan_status = WAN_CONNECTED;
	}
	else
	{
		if(tenda_wan_link_status())
			wan_info.wan_status = WAN_DISCONNECTED;
		else
			wan_info.wan_status = WAN_NO_WIRE;
	}
}

static void tpi_wan_get_bridgeap_status()
{
	if(tenda_wan_link_status())
	{
		if(gpi_apclient_dhcpc_ping_gateway())
		{
			wan_info.wan_status = WAN_CONNECTED;
		}
		else
		{
			wan_info.wan_status = WAN_DISCONNECTED;
		}
	}
	else
	{
		//本来此处可以判断为WAN_NO_WIRE(未插网线)，但规格需求不需要该状态，故为WAN_DISCONNECTED
		wan_info.wan_status = WAN_DISCONNECTED;
	}
}

static void tpi_wan_get_wlmode_status(SYS_WORK_MODE wl_mode)
{
	if(wl_mode == WL_APCLIENT_MODE)
	{
		char ip[17] = {0},mask[17] = {0};
		tpi_apclient_dhcpc_get_ip(ip,mask);
		if(0 == strcmp(ip,"") || 0 == strcmp(mask,""))
		{
			wan_info.wan_status = WAN_CONNECTING;
		}
		else

		{
			wan_info.wan_status = WAN_CONNECTED;
		}
	}
	else
	{
		if(wl_mode == WL_WISP_MODE)
		{
			if(0 == strcmp(nvram_safe_get("wan0_connect"),"Connected"))
			{
				wan_info.wan_status = WAN_CONNECTED;
			}
			else
			{
				wan_info.wan_status = WAN_CONNECTING;
			}
		}
		else
		{
			wan_info.wan_status = WAN_CONNECTED;
		}
	}
}

static void tpi_wan_get_wisp_or_apclient_status(SYS_WORK_MODE wl_mode,WIFISTASTATUS * wifi_status)
{
	if(NULL == wifi_status)
	{
		return RET_ERR;
	}
	if(nvram_match(WLAN24G_WORK_MODE,"sta"))
		(*wifi_status) = tpi_get_sta_info(TENDA_WLAN24_REPEATER_IFNAME);
	else
		(*wifi_status) = tpi_get_sta_info(TENDA_WLAN5_REPEATER_IFNAME);

	if(WIFI_INIT_FAIL == *wifi_status)
	{
		wan_info.wan_status = WAN_DISCONNECTED;
	}
	else if(WIFI_OK == *wifi_status)
	{
		tpi_wan_get_wlmode_status(wl_mode);
	}
	else
	{
		wan_info.wan_status = WAN_CONNECTING;
	}
}


/*下面是WAN口错误诊断*/
static RET_INFO tpi_wan_get_status(SYS_WORK_MODE wl_mode,WIFISTASTATUS * wifi_status)
{
	RET_INFO ret = RET_SUC;	
	switch(wl_mode)
	{
		case WL_ROUTE_MODE:
			tpi_wan_get_route_status(wifi_status);
			break;
		case WL_BRIDGEAP_MODE:
			tpi_wan_get_bridgeap_status();
			break;
		case WL_WISP_MODE:
		case WL_APCLIENT_MODE:
			tpi_wan_get_wisp_or_apclient_status(wl_mode,wifi_status);
			break;
		case WL_WDS_MODE:
		default:
			ret = RET_ERR;
			break;
	}
	return ret;
}

RET_INFO tpi_wan_get_connect()
{
	SYS_WORK_MODE mode = WL_ROUTE_MODE;
	WIFISTASTATUS wifi_status = WIFI_INIT_FAIL;
	mode = gpi_wifi_get_mode();

	tpi_wan_get_status(mode,&wifi_status);

	if(wan_info.wan_status == WAN_CONNECTED)
	{
		return 1;
	}
	return 0;
}

static PIU32 tpi_wan_get_err_result(P_WAN_ERR_INFO_STRUCT p)
{
	PIU32 ret = 0;

	ret = (p->network_check) + 10 * ((p->code)%100) + 1000 * ((p->wan_mode)%10)
	      + 10000 * ((p->wl_mode)%10) + 100000 * ((p->time_action)%10)
	      + 1000000 * ((p->color_action)%10) + 10000000 * ((p->button_action)%10);

	return ret;
}

static PPPOEERRCODE tpi_wan_get_pppoe_err_result()
{
	PPPOEERRCODE ret = COMMON_NONE;
	char pppoe_check[5]= {0};

	sprintf(pppoe_check,"%s",nvram_safe_get("err_check"));

	if(!strcmp(pppoe_check,"5"))//无响应
	{
		ret = PPPOE_NO_RESPOND;
	}
	else if(!strcmp(pppoe_check,"7"))//正在验证
	{
		ret = COMMON_CONNECTING;
	}
	else if(!strcmp(pppoe_check,"2"))//用户名，密码错误
	{
		ret = PPPOE_CHECKED_PASSWORD_FAIL;
	}
	else if(!strcmp(pppoe_check,"3"))//成功
	{
		ret = COMMON_CONNECTED_ONLINEING;
	}

	return ret;
}

static DHCPERRCODE tpi_wan_get_dhcp_err_result()
{
	DHCPERRCODE ret = COMMON_NONE;
	char dhcp_check[5] = {0};

	sprintf(dhcp_check,"%s",nvram_safe_get("err_check"));

	if(0 == strcmp(dhcp_check,"11"))
	{
		ret = DHCP_IP_CONFLLICT;
	}
	else if(0 == strcmp(dhcp_check,"5"))
	{
		ret = DHCP_NO_RESPOND;
	}

	return ret;
}

static RET_INFO tpi_wan_get_connected_err_info(P_WAN_ERR_INFO_STRUCT p)
{
	RET_INFO ret = RET_SUC;
	int internetStat = 0;

	if(NULL == p)
	{
		return RET_ERR;
	}

	p->button_action = BUTTON_DOWN;
	p->color_action = COLOR_TRY;
	p->time_action = TIME_SHOW;
	p->code = COMMON_CONNECTED_ONLINEING;

	switch(p->wl_mode)
	{
		case WL_ROUTE_MODE:
		case WL_WISP_MODE:
			internetStat = get_wan_onln_connstatus();
			if(0 == internetStat)
			{
				p->color_action = COLOR_ERR;
				p->code = COMMON_NOT_ONLINE;
			}
			else
			{
				if((1 == internetStat) && (p->wan_mode == WAN_STATIC_MODE))
				{
					p->color_action = COLOR_TRY;
					p->code = COMMON_CONNECTED_ONLINEING;															
				}
				else
				{
					p->color_action = COLOR_SUC;
					p->code = COMMON_ONLINEED;
				}
			}
			break;
		case WL_APCLIENT_MODE:
			p->time_action = TIME_SHOW;
			p->color_action = COLOR_SUC;
#ifdef __CONFIG_APCLIENT_DHCPC__
			char ip[17] = {0},mask[17] = {0};
			tpi_apclient_dhcpc_get_ip(ip,mask);
			if(0 == strcmp(ip,"") || 0 == strcmp(ip,""))
			{
				p->color_action = COLOR_TRY;
				p->code = COMMON_CONNECTING;
				p->time_action = TIME_NONE;
			}
#endif
			break;
#ifdef __CONFIG_BRIDGE_AP__
		case WL_BRIDGEAP_MODE:
			p->color_action = COLOR_SUC;
			p->code = COMMON_CONNECTED_ONLINEING;
			break;
#endif
		default:
			ret = RET_ERR;
			break;
	}

	return ret;
}

static RET_INFO tpi_wan_get_connecting_err_info(P_WAN_ERR_INFO_STRUCT p,WIFISTASTATUS wifi_status)
{
	RET_INFO ret = RET_SUC;

	if(NULL == p)
	{
		return RET_ERR;
	}

	p->button_action = BUTTON_DOWN;
	p->color_action = COLOR_TRY;
	p->time_action = TIME_NONE;
	p->code = COMMON_CONNECTING;

	if(WL_ROUTE_MODE != p->wl_mode && (WIFI_AUTHENTICATED_FAIL == wifi_status || WIFI_AUTH_FAIL == wifi_status))
	{
		p->color_action = COLOR_ERR;
		switch(p->wan_mode)
		{
			case WAN_STATIC_MODE:
				p->code = STATIC_WL_CHECKED_PASSWORD_FAIL;
				break;
			case WAN_DHCP_MODE:
				p->code = DHCP_WL_CHECKED_PASSWORD_FAIL;
				break;
			case WAN_PPPOE_MODE:
				p->code = PPPOE_WL_CHECKED_PASSWORD_FAIL;
				break;
			default:
				ret = RET_ERR;
				break;
		}
		return ret;
	}

	switch(p->wl_mode)
	{
		case WL_ROUTE_MODE:
		case WL_WISP_MODE:
			if(WAN_DHCP_MODE == p->wan_mode)
			{
				DHCPERRCODE dhcp_err = tpi_wan_get_dhcp_err_result();
				if(dhcp_err == DHCP_IP_CONFLLICT)
				{
					p->color_action = COLOR_ERR;
					p->code = DHCP_IP_CONFLLICT;
				}
				else if(dhcp_err == DHCP_NO_RESPOND)
				{
					p->color_action = COLOR_ERR;
					p->code = DHCP_NO_RESPOND;
				}
			}
			else if(WAN_PPPOE_MODE == p->wan_mode)
			{
				PPPOEERRCODE pppoe_err = tpi_wan_get_pppoe_err_result();

				if(pppoe_err == COMMON_CONNECTING)
				{
					p->color_action = COLOR_TRY;
					p->code = COMMON_CONNECTING;
				}
#if 0
				//此时虽然拨号成功，但是还没有获取到IP
				else if(pppoe_err == COMMON_CONNECTED_ONLINEING)
				{
					p->color_action = COLOR_TRY;
					p->code = COMMON_CONNECTED_ONLINEING;
				}
#endif
				else if(pppoe_err == PPPOE_CHECKED_PASSWORD_FAIL)
				{
					p->color_action = COLOR_ERR;
					p->code = PPPOE_CHECKED_PASSWORD_FAIL;
				}
				else if(pppoe_err == PPPOE_NO_RESPOND)
				{
					p->color_action = COLOR_ERR;
					p->code = PPPOE_NO_RESPOND;
				}
			}
			break;
		case WL_APCLIENT_MODE:
			p->time_action = TIME_NONE;
			break;
		default:
			break;
	}
	return ret;
}

static RET_INFO tpi_wan_get_err_info(P_WAN_ERR_INFO_STRUCT p)
{
	RET_INFO ret = RET_SUC;
	WIFISTASTATUS wifi_status = WIFI_INIT_FAIL;

	memset(p,0x0,sizeof(WAN_ERR_INFO_STRUCT));

	p->wl_mode = gpi_wifi_get_mode();

	if(p->wl_mode == WL_APCLIENT_MODE)
		wan_info.wan_type = WAN_DHCP_MODE;
	else if(p->wl_mode == WL_WDS_MODE)
		wan_info.wan_type = WAN_NONE_MODE;
	else if(p->wl_mode == WL_BRIDGEAP_MODE)
		wan_info.wan_type = WAN_DHCP_MODE;

	p->wan_mode = wan_info.wan_type;

	if(WAN_MAX_MODE == p->wan_mode || WAN_NONE_MODE == p->wan_mode || RET_ERR == tpi_wan_get_status(p->wl_mode,&wifi_status))
	{
		return RET_ERR;
	}

	if(WL_ROUTE_MODE == p->wl_mode && WAN_NO_WIRE != wan_info.wan_status && WAN_DISCONNECTED != wan_info.wan_status)
	{
		p->network_check = gpi_wan_mode_check_get_check_result();
	}

	switch(wan_info.wan_status)
	{
		case WAN_NO_WIRE:
			if(WL_ROUTE_MODE != p->wl_mode)
			{
				return RET_ERR;
			}
			p->button_action = BUTTON_CONNECTED;
			p->color_action = COLOR_ERR;
			p->time_action = TIME_NONE;
			p->code = COMMON_NO_WIRE;
			break;
		case WAN_DISCONNECTED:
			p->button_action = BUTTON_CONNECTED;
			p->color_action = COLOR_ERR;
			p->time_action = TIME_NONE;
			p->code = COMMON_NOT_CONNECT;
			break;
		case WAN_CONNECTING:
			if(RET_ERR == tpi_wan_get_connecting_err_info(p,wifi_status))
			{
				return RET_ERR;
			}
			break;
		case WAN_CONNECTED:
			if(RET_ERR == tpi_wan_get_connected_err_info(p))
			{
				return RET_ERR;
			}
			break;
		default:
			return RET_ERR;
	}

	return ret;
}
