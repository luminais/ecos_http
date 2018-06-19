
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "webs.h"
#include "uemf.h"
#include "route_cfg.h"
#include "flash_cgi.h"
#include "include/router_net.h"

/********************************firewall define********************/
#define FLT_MAC_MAX_NUM_1		10
#define URL_FILTER_COUNT 		10
#define MAC_FILTER_COUNT 		10
/****************************************************************/

extern int webs_Wan_OpenListen(int port,char *HostIP);
extern void webs_Wan_CloseListen();
//roy +++2010/09/19
extern void sys_restart(void);
extern void sys_reboot(void);
//+++
/****************************************************************/

#define VALUEMAXLEN			128
#define NAMEMAXLEN			32

//static char do_cmd_go[128];
unsigned int g_localip,g_localmask,g_remoteIPStart,g_remoteIPEnd;
int g_enable_remote_acl,g_remote_port;

void 		LoadWanListen(int load);
void 		LoadManagelist();
static int 		getfirewall(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspmAclGetIP(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspmAclGetUrl(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspmAclGetMacFilter(int eid, webs_t wp, int argc, char_t **argv);

static void 	fromSafeClientFilter(webs_t wp, char_t *path, char_t *query);
static void 	fromSafeUrlFilter(webs_t wp, char_t *path, char_t *query);
static void 	fromSafeMacFilter(webs_t wp, char_t *path, char_t *query);
static void 	fromSafeWanPing(webs_t wp, char_t *path, char_t *query);
static void	fromSafeWanWebMan(webs_t wp, char_t *path, char_t *query);
static void 	fromSafeNetAttack(webs_t wp, char_t *path, char_t *query);

void firewall_asp_define(void)
{
	websAspDefine(T("getfirewall"), getfirewall);
	websAspDefine(T("mAclGetIP"), aspmAclGetIP);
	websAspDefine(T("mAclGetUrl"), aspmAclGetUrl);
	websAspDefine(T("mAclGetMacFilter"), aspmAclGetMacFilter);
	
	websFormDefine(T("SafeClientFilter"), fromSafeClientFilter);
	websFormDefine(T("SafeUrlFilter"), fromSafeUrlFilter);
	websFormDefine(T("SafeMacFilter"), fromSafeMacFilter);
	websFormDefine(T("SafeWanPing"), fromSafeWanPing);
	websFormDefine(T("SafeWanWebMan"), fromSafeWanWebMan);
	websFormDefine(T("SafeNetAttack"), fromSafeNetAttack);
}

static int getfirewall(int eid, webs_t wp, int argc, char_t **argv)
{
	int retv = 0;
	char *type, *item;
	char_t * v = NULL;
	if (ejArgs(argc, argv, T("%s %s"), &type, &item) < 2) 
	{
		return websWrite(wp, T("Insufficient args\n"));
	}
	if(strcmp(type,"lan") == 0)
	{
		if(strcmp(item,"lanip") == 0)
		{	
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_LAN0_IP, v));
		}
		else if(strcmp(item, "acl_en") == 0)
		{	
			return websWrite(wp, T("%d"),get_filter_mode(_FW_FLT_CLN_EN));
		}
		else if(strcmp(item, "curNum") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_FLT_CLN_CUR_NU, v));
		}
		else if(strcmp(item, "url_en") == 0)
		{
			return websWrite(wp, T("%d"),get_filter_mode(_FW_FLT_URL_EN));
		}
		else if(strcmp(item, "urlNum") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_FLT_URL_CUR_NU, v));
		}
		else if(strcmp(item, "acl_mac") == 0)
		{
			return websWrite(wp, T("%d"),get_filter_mode(_FW_FLT_MAC_EN));
		}
		else if(strcmp(item, "acl_macNum") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_FLT_MAC_CUR_NU, v));
		}
	}
	else if(strcmp(type, "wan") == 0)
	{
		if(strcmp(item, "dos") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_HACKER_ATT, v));
		}
		else if(strcmp(item, "wanweben") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_SYS_RM_EN, v));
		}
		else if(strcmp(item, "webport") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_SYS_RM_PORT, v));
		}
		else if(strcmp(item, "rmanip") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_SYS_RM_IP, v));
		}
		else if(strcmp(item,"ping") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_PING_DIS_WAN, v));
		}
	}

	return retv;
}


static int aspmAclGetIP(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned int item = 0;
	const char_t *v;
	if(ejArgs(argc, argv, T("%d "),&item) < 1)
	{
		return 0;
	}

	item = item - 1;
	if(item>=0 && item <=9){
		_SAFE_GET_VALUE(_FW_FILTER_CLIENT(item),v);
		websWrite(wp,T("%s"), v);
	}
	else 
		websWrite(wp,T("%s"), "");
	return 0;
}

static int aspmAclGetUrl(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned int item = 0;
	const char_t *v;
	if(ejArgs(argc, argv, T("%d "),&item) < 1)
	{
		return 0;
	}

	item = item - 1;
	if(item>=0 && item <=9){
		_SAFE_GET_VALUE(_FW_FILTER_URL(item),v);
		websWrite(wp,T("%s"), v);
	}
	else 
		websWrite(wp,T("%s"), "");
	return 0;
}

static int aspmAclGetMacFilter(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned int item = 0;
	const char_t *v;
	if(ejArgs(argc, argv, T("%d "),&item) < 1)
	{
		return 0;
	}

	item = item - 1;
	if(item>=0 && item <=9){
		_SAFE_GET_VALUE(_FW_FILTER_MAC(item),v);
		websWrite(wp,T("%s"), v);
	}
	else 
		websWrite(wp,T("%s"), "");
	return 0;
}

static void fromSafeNetAttack(webs_t wp, char_t *path, char_t *query)
{
	char_t *go,*DoSProtect;
	char_t *oldDos;
	go = websGetVar(wp,T("GO"),T(""));
	DoSProtect = websGetVar(wp,T("DoSProtect"),T("0"));
	_SAFE_GET_VALUE(_FW_HACKER_ATT,oldDos);
	
	if(strncmp(oldDos, DoSProtect,1) != 0)
	{
		_SET_VALUE(_FW_HACKER_ATT,DoSProtect);
		_COMMIT();

		sys_restart();

		websRedirect(wp, T("/firewall_disablewan.asp"));
	}
	else
	{
		websRedirect(wp, T("/firewall_disablewan.asp"));
	}
}

static void fromSafeClientFilter(webs_t wp, char_t *path, char_t *query)
{
	char name[VALUEMAXLEN] ={'\0'};
	int which = 0;
	char_t  *aclen,*stritem,*go,*num;//,*saveTag;
	go = websGetVar(wp, T("GO"), T("")); 
	aclen = websGetVar(wp,T("check"),T("disable"));
	num = websGetVar(wp,T("curNum"),T("1"));
	//saveTag = websGetVar(wp,T("saveTag"),T("1"));
	
	_SET_VALUE(_FW_FLT_CLN_EN,aclen);
	if(strcmp(aclen,"disable") == 0)
		goto ip_done;
	
	_SET_VALUE(_FW_FLT_CLN_CUR_NU,num);
	
	which = atoi(num);
	sprintf(name,"CL%d",which);
	stritem = websGetVar(wp,T(name),T(""));
	which = which -1;
	
	if( which>=0 && which<= 9)
			_SET_VALUE(_FW_FILTER_CLIENT(which), stritem);	
	
ip_done:	
	_COMMIT();

	sys_restart();

	websRedirect(wp, T("/firewall_clientfilter.asp"));
}

//filter_url0=192.168.1.7-192.168.1.8:www.baidu.com,on
static void fromSafeUrlFilter(webs_t wp, char_t *path, char_t *query)
{	
	char name[VALUEMAXLEN] ={'\0'};
	int which = 0;
	char_t  *aclen,*stritem,*go,*num;//,*saveTag;
	go = websGetVar(wp, T("GO"), T("")); 
	aclen = websGetVar(wp,T("check"),T("disable"));
	num = websGetVar(wp,T("curNum"),T("1"));
	//saveTag = websGetVar(wp,T("saveTag"),T("1"));
	
	_SET_VALUE(_FW_FLT_URL_EN,aclen);
	if(strcmp(aclen,"disable") == 0)
		goto url_done;
	
	_SET_VALUE(_FW_FLT_URL_CUR_NU,num);
	
	which = atoi(num);
	sprintf(name,"CL%d",which);
	stritem = websGetVar(wp,T(name),T(""));
	which = which -1;
	
	if( which>=0 && which<= 9)
			_SET_VALUE(_FW_FILTER_URL(which), stritem);	

url_done:	
	_COMMIT();

	sys_restart();

	websRedirect(wp, T("/firewall_urlfilter.asp"));

}

static void fromSafeMacFilter(webs_t wp, char_t *path, char_t *query)
{
	char name[VALUEMAXLEN] ={'\0'};
	int which = 0;
	char_t  *aclen,*stritem,*go,*num;//,*saveTag;

	go = websGetVar(wp, T("GO"), T("")); 
	aclen = websGetVar(wp,T("check"),T("disable"));
	num = websGetVar(wp,T("curNum"),T("1"));
	//saveTag = websGetVar(wp,T("saveTag"),T("1"));
	
	_SET_VALUE(_FW_FLT_MAC_EN,aclen);//禁止deny
	if(strcmp(aclen,"disable") == 0)
		goto mac_done;
	
	_SET_VALUE(_FW_FLT_MAC_CUR_NU,num);
	
	which = atoi(num);
	sprintf(name,"CL%d",which);
	stritem = websGetVar(wp,T(name),T(""));
	which = which -1;
	
	if( which>=0 && which<= 9)
			_SET_VALUE(_FW_FILTER_MAC(which), stritem);	
	
	//00:11:22:33:44:55,0-6星期,0-0时间,on是否开启,1诠释
	
	//00:11:22:33:44:55,1-2,11100-29400,on,1
	
mac_done:	
	_COMMIT();

	sys_restart();

	websRedirect(wp, T("/firewall_mac.asp"));

}

static void fromSafeWanPing(webs_t wp, char_t *path, char_t *query)
{
	char_t  *pingwan;//, *go;
	char_t *oldPing;
	pingwan = websGetVar(wp, T("PingWan"), T("0")); 

	_SAFE_GET_VALUE(_FW_PING_DIS_WAN, oldPing);
		
	if(strcmp(oldPing, pingwan) != 0)
	{		
		_SET_VALUE(_FW_PING_DIS_WAN, pingwan);
		_COMMIT();

		sys_restart();
		
		websRedirect(wp, T("/firewall_wanping.asp"));
	}
	else
	{
		websRedirect(wp, T("/firewall_wanping.asp"));
	}
}


///******************************************************************************
//*/远端WEB管理

static void	fromSafeWanWebMan(webs_t wp, char_t *path, char_t *query)
{
	char_t	*wanwebip, *webport, *en, *go;
	//int oldEn;

	wanwebip = websGetVar(wp, T("IP"), T("")); 
	go = websGetVar(wp, T("GO"), T("")); 
	en = websGetVar(wp, T("RMEN"), T("0")); 
	webport = websGetVar(wp, T("port"), T("")); 

	if(atoi(en) == 1)
	{
		_SET_VALUE(_SYS_RM_EN, en);
		_SET_VALUE(_SYS_RM_PORT,webport);
		_SET_VALUE(_SYS_RM_IP,wanwebip);
	}else{
		_SET_VALUE(_SYS_RM_EN, en);
	}

	_COMMIT();
	
	LoadWanListen(atoi(en));

	websRedirect(wp, T("/system_remote.asp"));

}

void LoadWanListen(int load)
{
	char WanIP[24] = {0};
	int port;
	char_t *WanPort,*pHost;

	LoadManagelist();

	if(load){

		webs_Wan_CloseListen();

		_SAFE_GET_VALUE(_SYS_RM_PORT,WanPort);
		port = atoi(WanPort);
		if(SYS_wan_up != 1/*CONNECTED*/)
			return ;

		strcpy(WanIP,NSTR(SYS_wan_ip));
		pHost = WanIP;
		
		webs_Wan_OpenListen(port,pHost);
	}
	else{
		webs_Wan_CloseListen();
	}

}

void LoadManagelist()
{
	char bufTmp[128]; //enable[4];
	//char remote_port[32];
	char  *enable, *lan_ip, *lan_msk, *rm_ip;
	char *remote_port;
//	char *ip1,*ip2;
	g_enable_remote_acl = 0;
	memset(bufTmp,0,sizeof(bufTmp));
	
	_SAFE_GET_VALUE(_SYS_RM_EN, enable);
	_SAFE_GET_VALUE(_SYS_RM_PORT, remote_port);
	_SAFE_GET_VALUE(_LAN0_IP, lan_ip);
	_SAFE_GET_VALUE(_LAN0_NETMASK, lan_msk);
	
	strcpy(bufTmp,lan_ip);
	inet_aton(bufTmp,&g_localip);

	strcpy(bufTmp,lan_msk);
	inet_aton(bufTmp,&g_localmask);
	g_remote_port = atoi(remote_port);
	
	if(atoi(enable) == 1)
	{//remote
		g_enable_remote_acl = 1;
		
		_SAFE_GET_VALUE(_SYS_RM_IP,rm_ip);
		strcpy(bufTmp,rm_ip);
		if(strlen(bufTmp) >=3/*0-0*/){
#if 0
			ip1 = strchr(bufTmp,';');
			if(ip1){
				*ip1 = '\0';
				ip2 = ip1+1;
				ip1 = bufTmp;
				inet_aton(ip1,&g_remoteIPStart);
				inet_aton(ip2,&g_remoteIPEnd);
			}else{
				g_enable_remote_acl = 0;/*wrong format*/	
			}
#else
			inet_aton(bufTmp,&g_remoteIPStart);
#endif
		}else{
			g_enable_remote_acl = 0;
		}
	}	
}

int checkAcl(unsigned int ip,int port)
{
	if((ip & g_localmask) == (g_localip & g_localmask))
	{
		return 1;
	} 
	else//wan ip
	{
		if(g_enable_remote_acl != 1)
		{
			return 0;
		}

		if((g_remoteIPStart == 0) /*&& (g_remoteIPEnd == 0)*/ && port == g_remote_port)
		{
			return 1;
		}
		//if((ip >= g_remoteIPStart) && (ip <= g_remoteIPEnd) && port == g_remote_port) 
		if(ip == g_remoteIPStart && port == g_remote_port) 
		{
			return 1;
		} else
		{
			return 0;
		}
	}
}

