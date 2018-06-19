/*
* mtenda.c for bcm ecos, 2010/08/25 roy
*/

/********************************* Includes ***********************************/

#include <stdio.h>
#include <bcmnvram.h>
#include <iflib.h>
#include <ecos_oslib.h>
#include <net/if_var.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/param.h>
#include <net/if_dl.h>
#include <net/radix.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sys/sockio.h>
#include <autoconf.h>

#include  "uemf.h"
#include  "wsIntrn.h"
#include  "route_cfg.h"
#include  "flash_cgi.h"
#include  "../ddns/include/ddns.h"
#include  "chlan.h"
#include  "../rc/rc.h"

#ifdef __CONFIG_APCLIENT_DHCPC__
#include "pi_common.h"
extern int gpi_get_apclient_dhcpc_enable();
extern int gpi_get_apclient_dhcpc_addr(char *ip,char *mask);
#endif
/******************************** extern Description *********************************/

/******************************** local Description *********************************/

/*
 * The number of bytes in an ethernet (MAC) address.
 */
#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif

#ifndef DHCPD_STATIC_LEASE_NU
#define DHCPD_STATIC_LEASE_NU 19
#endif 

#ifndef VTS_MAX_NUM_1
#define VTS_MAX_NUM_1 10 //VTS_MAX_NUM in natcmd.c
#endif

#ifdef __CONFIG_MEMORY_SIZE__
	#if (8 == __CONFIG_MEMORY_SIZE__)
		#ifdef __CONFIG_TENDA_WLJB__
		#define W311R_ECOS_SV "V11.11.01.11_T"
		#else	
		#define W311R_ECOS_SV "V11.11.01.11"
		#endif
	#elif (16 == __CONFIG_MEMORY_SIZE__)
		#ifdef __CONFIG_TENDA_WLJB__
		#define W311R_ECOS_SV "V11.13.01.16_T"
		#else
		#define W311R_ECOS_SV "V11.13.01.16"
		#endif
	#else
		#ifdef __CONFIG_TENDA_WLJB__
		#define W311R_ECOS_SV "V11.13.01.07_T"
		#else
		#define W311R_ECOS_SV "V11.13.01.07"
		#endif	
	#endif
#else
	#if 0
		#ifdef __CONFIG_TENDA_WLJB__
		#define W311R_ECOS_SV "V11.11.01.09_T"
		#else	
		#define W311R_ECOS_SV "V11.11.01.09"
		#endif
	#else
		#ifdef __CONFIG_TENDA_WLJB__
		#define W311R_ECOS_SV "V11.13.01.09_T"
		#else
		#define W311R_ECOS_SV "V11.13.01.09"
		#endif
	#endif
#endif

#define W311R_ECOS_HV	""

#define W311R_ECOS_BV	"V5.100.27.21" //sdk version

#define TWL300N_FIRMWARE_BUILD_DATE		__DATE__

#if 1	//translate by web page
#define LAN_WAN_IP_ERROR 				"ERROR: WAN NET is same as LAN"
#define WAN_IP_ERROR 					"ERROR: Wrong WAN IP Settings"
#define LOGIN_USERS_FULL 				"ERROR: login forbi"
#define PASSWORD_ERROR					"Password error"
#define CONFIG_SUCCESS					"Successfully Configure"
#ifdef __CONFIG_QUICK_SET__	
#define CONFIG_INTERNETOK				"internetok"
#endif
#endif

static char gWLErrorMsg[64] = {0};
static char gWLErrorRtnUrl[64] = {0};

static int g_user_pass_Flag = 0;
static char bForceLogout = 1;
static int g_login_where = 0;
//syslog
static int logNum;
static int gLog_cur_page = 1;

int change_tag = 0;
extern int check_tag;

char g_User[64] = {0},g_Pass[64] = {0};

int dns_redirect_dag = 0;//add for redirect to advance.asp by ldm 20121127

extern login_ip_time loginUserInfo[MAX_USER_NUM];

#define IFVALUE(value)	 if(value==NULL)	\
							value=""

//roy +++2010/09/07
extern void sys_restart(void);
extern void sys_reboot(void);
extern int    arpioctl(int req, caddr_t data, struct proc *p);
extern void show_network_tables2(int (*mtenda_routing_entry)(struct radix_node *, void *),void *pr);
extern int	    	syslog_clear();
extern int 	syslog_open();
extern int 	syslog_close();
extern int 	syslog_get2(char *log_time,char *log_type,char *log_data);
extern int get_argv(char *string, char *argv[]);


extern int network_tpye;

//start: add by z10312  
extern void set_all_led_on( void );
extern void set_all_led_off( void );
//end: add by z10312  



//extern uint32 wps_gen_pin(char *devPwd, int devPwd_len);
/* ARP ioctl structure */
struct arp_arpentry {
	struct in_addr addr;
	char enaddr[ETHER_ADDR_LEN];
};

//roy+++
#ifdef __CONFIG_QUICK_SET__
struct tz_info
{
	int index;
	char *time_zone;
	char *tz_str;
	int tz_offset;
};

static struct tz_info time_zones[] =
{
	{0,  		"GMT-12:00",	"WPT+12:00" ,-12*3600},        
	{1,  		"GMT-11:00",	"WPT+11:00" ,-11*3600},      
	{2,  		"GMT-10:00",	"WPT+10:00" ,-10*3600},      
	{3,  		"GMT-09:00",	"WPT+09:00" ,-9*3600},       
	{4,  		"GMT-08:00",	"WPT+08:00" ,-8*3600},       
	{5,  		"GMT-07:00",	"WPT+07:00" ,-7*3600},       
	{6,  		"GMT-07:00",	"WPT+07:00" ,-7*3600},       
	{7,  		"GMT-07:00",	"WPT+07:00" ,-7*3600},       
	{8,  		"GMT-06:00",	"WPT+06:00" ,-6*3600},
	{9,  		"GMT-06:00",	"WPT+06:00" ,-6*3600},
	{10,   "GMT-05:00" , "WPT+05:00"  ,-5*3600},
	{11,   "GMT-05:00" , "WPT+05:00"  ,-5*3600},
	{12,   "GMT-05:00" , "WPT+05:00"  ,-5*3600},
	{13,   "GMT-04:30" , "WPT+04:30"  ,-4*3600-30*60},    
	{14,   "GMT-04:00" , "WPT+04:00"  ,-4*3600},       
	{15,   "GMT-04:00" , "WPT+04:00"  ,-4*3600},       
	{16,   "GMT-03:30" , "WPT+03:30"  ,-3*3600-30*60},    
	{17,   "GMT-03:00" , "WPT+03:00"  ,-3*3600},       
	{18,   "GMT-03:00" , "WPT+03:00"  ,-3*3600},       
	{19,   "GMT-03:00" , "WPT+03:00"  ,-3*3600},       
	{20,   "GMT-02:00" , "WPT+02:00"  ,-2*3600},       
	{21,   "GMT-01:00" , "WPT+01:00"  ,-1*3600},       
	{22,   "GMT-01:00" , "WPT+01:00"  ,-1*3600},       
	{23,   "GMT+00:00" , "WPT-00:00"  ,+0*3600},       
	{24,   "GMT+00:00" , "WPT-00:00"  ,+0*3600},       
	{25,   "GMT+01:00" , "WPT-01:00"	,+1*3600},       
	{26,   "GMT+01:00" , "WPT-01:00"  ,+1*3600},       
	{27,   "GMT+01:00" , "WPT-01:00"  ,+1*3600},       
	{28,   "GMT+01:00" , "WPT-01:00"  ,+1*3600},       
	{29,   "GMT+01:00" , "WPT-01:00"	,+1*3600},       
	{30,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},       
	{31,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},       
	{32,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},       
	{33,   "GMT+02:00" , "WPT-02:00"	,+2*3600},       
	{34,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},       
	{35,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},       
	{36,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},       
	{37,   "GMT+03:00" , "WPT-03:00"  ,+3*3600},       
	{38,   "GMT+03:00" , "WPT-03:00"  ,+3*3600},       
	{39,   "GMT+03:00" , "WPT-03:00"  ,+3*3600},       
	{40,   "GMT+03:00" , "WPT-03:00"  ,+3*3600},       
	{41,   "GMT+03:30" , "WPT-03:30"  ,+3*3600+30*60},    
	{42,   "GMT+04:00" , "WPT-04:00"  ,+4*3600},       
	{43,   "GMT+04:00" , "WPT-04:00"  ,+4*3600},       
	{44,   "GMT+04:00" , "WPT-04:00"  ,+4*3600},       
	{45,   "GMT+04:00" , "WPT-04:00"  ,+4*3600},       
	{46,   "GMT+04:30" , "WPT-04:30"  ,+4*3600+30*60},    
	{47,   "GMT+05:00" , "WPT-05:00"  ,+5*3600},       
	{48,   "GMT+05:00" , "WPT-05:00"  ,+5*3600},       
	{49,   "GMT+05:30" , "WPT-05:30"  ,+5*3600+30*60},    
	{50,   "GMT+05:45" , "WPT-05:45"  ,+5*3600+45*60},    
	{51,   "GMT+06:00" , "WPT-06:00"  ,+6*3600},       
	{52,   "GMT+06:00" , "WPT-06:00"  ,+6*3600},       
	{53,   "GMT+06:00" , "WPT-06:00"  ,+6*3600},       
	{54,   "GMT+06:30" , "WPT-06:30"  ,+6*3600+30*60},    
	{55,   "GMT+07:00" , "WPT-07:00"  ,+7*3600},       
	{56,   "GMT+07:00" , "WPT-07:00"  ,+7*3600},       
	{57,   "GMT+08:00" , "WPT-08:00"  ,+8*3600},       
	{58,   "GMT+08:00" , "WPT-08:00"  ,+8*3600},       
	{59,   "GMT+08:00" , "WPT-08:00"  ,+8*3600},       
	{60,   "GMT+08:00" , "WPT-08:00"  ,+8*3600},       
	{61,   "GMT+09:00" , "WPT-09:00"  ,+9*3600},       
	{62,   "GMT+09:00" , "WPT-09:00"  ,+9*3600},       
	{63,   "GMT+09:00" , "WPT-09:00"  ,+9*3600},       
	{64,   "GMT+09:30" , "WPT-09:30"  ,+9*3600+30*60},    
	{65,   "GMT+09:30" , "WPT-09:30"  ,+9*3600+30*60},    
	{66,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},      
	{67,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},      
	{68,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},      
	{69,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},      
	{70,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},      
	{71,   "GMT+11:00" , "WPT-11:00"  ,+11*3600},      
	{72,   "GMT+12:00" , "WPT-12:00"  ,+12*3600},      
	{73,   "GMT+12:00" , "WPT-12:00"  ,+12*3600},      
	{74,   "GMT+13:00" , "WPT-13:00"  ,+13*3600}       
};

#define TIME_ZONES_NUMBER 75
#endif


/*
 *asp define here
 */
static int  	aspTendaGetStatus(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspmGetRouteTable(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspmNatPortSegment(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspTendaGetDhcpClients(int eid, webs_t wp, int argc, char_t **argv);
static int  	sysTendaGetStatus(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspErrorMsg(int eid,webs_t wp,int argc,char_t **argv);
static int 		aspErrorRedirect(int eid,webs_t wp,int argc,char_t **argv);
static int 		aspSysLogGet(int eid, webs_t wp, int argc, char_t **argv);

/*
 *form define here
 */
static void	fromAdvSetLanip(webs_t wp, char_t *path, char_t *query);
static void	fromWizardHandle(webs_t wp, char_t *path, char_t *query);
static void 	fromAdvSetMacClone(webs_t wp, char_t *path, char_t *query);
static void 	fromAdvSetWan(webs_t wp, char_t *path, char_t *query);
static void 	fromAdvSetDns(webs_t wp, char_t *path, char_t *query);
static void	fromVirSerDMZ(webs_t wp, char_t *path, char_t *query);
static void 	fromVirSerUpnp(webs_t wp, char_t *path, char_t *query);
static void	fromVirSerSeg(webs_t wp, char_t *path, char_t *query);
static void	fromSysToolTime(webs_t wp, char_t *path, char_t *query);
static void 	fromalgform(webs_t wp, char_t *path, char_t *query);
static void	fromsetWanSpeed(webs_t wp, char_t *path, char_t *query);//huangxiaoli modify
//roy+++
static void 	fromDhcpSetSer(webs_t wp, char_t *path, char_t *query);
static void 	fromDhcpListClient(webs_t wp, char_t *path, char_t *query);
//beili+++
static void 	fromSysToolChangePwd(webs_t wp, char_t *path, char_t *query);
static void 	fromSysToolReboot(webs_t wp, char_t *path, char_t *query);
static void 	fromSysToolRestoreSet(webs_t wp, char_t *path, char_t *query);
static void 	fromSysToolSysLog(webs_t wp, char_t *path, char_t *query);
static void 	fromSysToolDDNS(webs_t wp, char_t *path, char_t *query);
static void 	fromRouteStatic(webs_t wp, char_t *path, char_t *query);
static void 	fromSysAted(webs_t wp, char_t *path, char_t *query);
static void 	fromSysStatusHandle(webs_t wp, char_t *path, char_t *query);
static void 	fromDDNSinit(webs_t wp, char_t *path, char_t *query);
static void 	fromSysStatusInit(webs_t wp, char_t *path, char_t *query);
static void	fromSetWizard(webs_t wp, char_t *path, char_t *query);
static void	fromGetWizard(webs_t wp, char_t *path, char_t *query);
/****************************hqw add for F307*********************************************/
static void 	fromNetWorkSetupInit(webs_t wp, char_t *path, char_t *query);
static void 	fromNetWorkSetupSave(webs_t wp, char_t *path, char_t *query);
static void 	fromSystemManageInit(webs_t wp, char_t *path, char_t *query);
static void 	fromSystemManageSave(webs_t wp, char_t *path, char_t *query);
#ifdef __CONFIG_QUICK_SET__
/***********************************add liuchengchi 2014-11-11****************************************************/
static void 	fromgetWanConnectStatus(webs_t wp, char_t *path, char_t *query);
static void 	fromsaveQuickSetData(webs_t wp, char_t *path, char_t *query);
/***************************************************************************************/
#endif

//#define __CONFIG_AL_SECURITY__
#ifdef __CONFIG_AL_SECURITY__
static void formalInsertWhite(webs_t wp, char_t *path, char_t *query) ;
#endif
#ifdef __CONFIG_ALINKGW__
static void fromAlinkgwEnable(webs_t wp, char_t *path, char_t *query);
static void fromAlinkgwInit(webs_t wp, char_t *path, char_t *query);
#endif

extern char* get_product_pwr_info();


void asp_define()
{ 
    	websAspDefine(T("aspTendaGetStatus"), aspTendaGetStatus);
	websAspDefine(T("mGetRouteTable"), aspmGetRouteTable);	
	websAspDefine(T("mNatPortSegment"), aspmNatPortSegment);
	websAspDefine(T("TendaGetDhcpClients"), aspTendaGetDhcpClients);
	websAspDefine(T("sysTendaGetStatus"), sysTendaGetStatus);
	websAspDefine(T("asp_error_message"),aspErrorMsg);
	websAspDefine(T("asp_error_redirect_url"),aspErrorRedirect);
	websAspDefine(T("aspSysLogGet"),aspSysLogGet);
}

void goform_define()
{
	websFormDefine(T("setWizard"), fromSetWizard);
	websFormDefine(T("getWizard"), fromGetWizard);
	websFormDefine(T("AdvSetLanip"), fromAdvSetLanip);
	websFormDefine(T("WizardHandle"), fromWizardHandle);
	websFormDefine(T("AdvSetMacClone"), fromAdvSetMacClone);
	websFormDefine(T("AdvSetWan"), fromAdvSetWan);
	websFormDefine(T("AdvSetDns"), fromAdvSetDns);
	websFormDefine(T("VirSerDMZ"), fromVirSerDMZ);
	websFormDefine(T("VirSerUpnp"), fromVirSerUpnp);
	websFormDefine(T("VirSerSeg"), fromVirSerSeg);
	websFormDefine(T("SysToolTime"), fromSysToolTime);
	websFormDefine(T("algform"), fromalgform);
	websFormDefine(T("setWanSpeed"),fromsetWanSpeed);
//roy+++
	websFormDefine(T("DhcpSetSer"), fromDhcpSetSer);
	websFormDefine(T("DhcpListClient"), fromDhcpListClient);
//beili+++
	websFormDefine(T("SysToolChangePwd"), fromSysToolChangePwd);
	websFormDefine(T("SysToolReboot"), fromSysToolReboot);
	websFormDefine(T("SysToolRestoreSet"), fromSysToolRestoreSet);
	websFormDefine(T("SysToolSysLog"), fromSysToolSysLog);
	websFormDefine(T("SysToolDDNS"), fromSysToolDDNS);
	websFormDefine(T("RouteStatic"), fromRouteStatic);
	websFormDefine(T("ate"), fromSysAted);
	websFormDefine(T("SysStatusHandle"), fromSysStatusHandle);
	websFormDefine(T("DDNSinit"), fromDDNSinit);	
	websFormDefine(T("SysStatusInit"), fromSysStatusInit);
    /**************************hqw add for F307**********************************/
	websFormDefine(T("NetWorkSetupInit"), fromNetWorkSetupInit);
	websFormDefine(T("NetWorkSetupSave"), fromNetWorkSetupSave);
	websFormDefine(T("SystemManageInit"), fromSystemManageInit);
	websFormDefine(T("SystemManageSave"), fromSystemManageSave);
#ifdef __CONFIG_QUICK_SET__	
    /*************************add liuchengchi 20114-11-08***********************************/
	websFormDefine(T("getWanConnectStatus"),fromgetWanConnectStatus);
	websFormDefine(T("saveQuickSetData"),fromsaveQuickSetData);		
    /*************************************************************************/
#endif	

#ifdef __CONFIG_AL_SECURITY__
	websFormDefine(T("InsertWhite"), formalInsertWhite);
#endif

#ifdef __CONFIG_ALINKGW__
	websFormDefine(T("SetCloud"), fromAlinkgwEnable);
	websFormDefine(T("GetCloud"), fromAlinkgwInit);
#endif

}

#ifdef __CONFIG_ALINKGW__

void fromAlinkgwInit(webs_t wp, char_t *path, char_t *query)
{
	char result[64] = {0};
	
	strncat(result, "{", 1);	
	
	string_cat(result,"enable",nvram_safe_get(ALILINKGW_ENABLE));
	strncat(result, ",", 1);
	
	string_cat(result,"wifi-power", get_product_pwr_info());
	
	strncat(result, "}", 1);	
	strncat(result, "\n", 1);
	
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);	

}

void fromAlinkgwEnable(webs_t wp, char_t *path, char_t *query)
{
	int reboot_flag = 0, alisec_restart = 0;
	char_t *ali_enable = NULL/*, *alisec_enable = NULL*/;
	
	ali_enable = websGetVar(wp, T("enable"), T("1"));
//	alisec_enable = websGetVar(wp, T("alisec_enable"), T("1")); 
	if(strcmp(ali_enable, nvram_safe_get(ALILINKGW_ENABLE)) != 0)
	{
		reboot_flag = 1;
		_SET_VALUE(ALILINKGW_ENABLE, ali_enable);
		
	}
	
#if 0
	if(strcmp(alisec_enable, nvram_safe_get(ALILINKGW_SEC_ENABLE)) != 0)
	{
		reboot_flag = 1;
		_SET_VALUE(ALILINKGW_SEC_ENABLE, ali_enable);
	}
#endif

	if( reboot_flag )
	{
		_COMMIT();
		sys_reboot();
	}
	else
	{
		websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
		websWrite(wp, "1");
		websDone(wp, 200);
	}
	
}
#endif

#ifdef __CONFIG_AL_SECURITY__
	extern int insert_into_white_list(char* mac , char* domain) ;	
	
	void formalInsertWhite(webs_t wp, char_t *path, char_t *query)
	{
		char_t *flag , *mac , *url ;
		flag = websGetVar(wp, T("flag"), T("0")); 
		mac = websGetVar(wp, T("mac"), T("0")); 
		url = websGetVar(wp, T("url"), T("0")); 	
			
		websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	
		if(atoi(flag) == 1)
		{
			insert_into_white_list(mac , url ) ;
		}
		websWrite(wp, "1");
		websDone(wp, 200);
		return;
	}
#endif




void SetErrorArg(char *msg,char *url)
{
	memset(gWLErrorMsg,0,sizeof(gWLErrorMsg));
	memset(gWLErrorRtnUrl,0,sizeof(gWLErrorRtnUrl));
	strcpy(gWLErrorMsg,msg);
	strcpy(gWLErrorRtnUrl,url);
}

static int aspErrorMsg(int eid,webs_t wp,int argc,char_t **argv)
{

	if(gWLErrorMsg[0] == 0)
	{
		strcpy(gWLErrorMsg,"system error!");
		strcpy(gWLErrorRtnUrl,"system_status.asp");
	}
	websWrite(wp,T(gWLErrorMsg));
	memset(gWLErrorMsg,0,sizeof(gWLErrorMsg));

	return 0;
}

static int aspErrorRedirect(int eid,webs_t wp,int argc,char_t **argv)
{
	if(gWLErrorRtnUrl[0] == 0)
	{
		strcpy(gWLErrorRtnUrl,"system_status.asp");
	}
	return websWrite(wp,T(gWLErrorRtnUrl));
}

/***********************************************************************/


void _mask(struct sockaddr *sa, char *buf, int _len)//roy modify
{
    unsigned char *cp = ((char *)sa) + 4;
    int len = sa->sa_len - 4;
    int tot = 0;

    while (len-- > 0) {
        if (tot) *buf++ = '.';
        buf += diag_sprintf(buf, "%d", *cp++);
        tot++;
    }

    while (tot < 4) {
        if (tot) *buf++ = '.';
        buf += diag_sprintf(buf, "%d", 0);
        tot++;
    }
}

/***********************************************************************/


static int  	sysTendaGetStatus(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*item=NULL, *type=NULL;
	int retv = 0,wan_status,wan_type,wan_onln_status;
	unsigned int time_diff;
	char val2[64];
	char *v=NULL;
	
	if (ejArgs(argc, argv, T("%s %s"), &type, &item) < 2) {
	}

	//diag_printf("==%s=type=[%s]===item=[%s]\n",__FUNCTION__,type,item);

	wan_status = get_wan_connstatus();
	wan_onln_status = get_wan_onln_connstatus();//gong add  
	wan_type = get_wan_type();

	if (strcmp(type,"wan") == 0){
		if(strcmp(item,"onln_status") == 0){
			
			retv = websWrite(wp,T("%d"),wan_onln_status);
		}
		else if(strcmp(item,"contstatus") == 0){
			/*0:dic,1:connecting,2:connected*/
			retv = websWrite(wp,T("%d"),wan_status);
		}
		else if(strcmp(item,"linkstatus") == 0){
			/*0:dic,1:connecting,2:connected*/
			retv = websWrite(wp,T("%d"),get_wan_linkstatus());
		}
		else if(strcmp(item,"wanip") == 0){
			if(wan_status == 2)
				retv = websWrite(wp,T("%s"),NSTR(SYS_wan_ip));
			else
				retv = websWrite(wp,T(""));
		}
		else if(strcmp(item,"wanmask") == 0){
			if(wan_status == 2)
				retv = websWrite(wp,T("%s"),NSTR(SYS_wan_mask));
			else
				retv = websWrite(wp,T(""));
		}
		else if(strcmp(item,"gateway") == 0){
			if(wan_status == 2)
				retv = websWrite(wp,T("%s"),NSTR(SYS_wan_gw));
			else
				retv = websWrite(wp,T(""));
		}
		else if(strcmp(item,"dns1") == 0){
			if(wan_status == 2)
				if(wan_type == PPPOEMODE ||wan_type == PPTPMODE ||
					wan_type == L2TPMODE||wan_type==PPPOEMODE2	){
					retv = websWrite(wp,T("%s"),SYS_dns_1);
				}else{
					memset(val2,0,sizeof(val2));
					get_wan0_dns(1,val2);
					retv = websWrite(wp,T("%s"),val2);
				}
			else
				retv = websWrite(wp,T(""));
		}
		else if(strcmp(item,"dns2") == 0){
			if(wan_status == 2)
				if(wan_type == PPPOEMODE ||wan_type == PPTPMODE ||
					wan_type == L2TPMODE||wan_type==PPPOEMODE2	){
					retv = websWrite(wp,T("%s"),SYS_dns_2);
				}else{
					memset(val2,0,sizeof(val2));
					get_wan0_dns(2,val2);
					retv = websWrite(wp,T("%s"),val2);
				}
			else
				retv = websWrite(wp,T(""));
		}
		else if(strcmp(item,"connetctime") == 0){
			if(wan_status == 2){
				if (SYS_wan_conntime)
					time_diff = time(0) - SYS_wan_conntime;
				else
					time_diff = 0;
			}else
				time_diff = 0;
			
			retv = websWrite(wp,T("%u"),time_diff);
		}
		else if(strcmp(item,"login_where") == 0){
			
			retv = websWrite(wp,T("%d"),g_login_where);
		}
		else if(strcmp(item,"auto_chmode_dis") == 0){
			if(!strcmp(nvram_safe_get(_WAN0_CHECK),"1"))
				retv = websWrite(wp,T("%s"),"0");
			else
				retv = websWrite(wp,T("%s"),"1");
		}
	}
	else if(strcmp(type,"lan") == 0){
		if(strcmp(item,"lanip") == 0){
			retv = websWrite(wp,T("%s"),NSTR(SYS_lan_ip));
		}
		else if(strcmp(item,"lanmask") == 0){
			retv = websWrite(wp,T("%s"),NSTR(SYS_lan_mask));
		}
	}
	else if(strcmp(type,"pppoe")==0)
	{
		if(strcmp(item,"err_check")==0)
			{
			retv=websWrite(wp,T("%s"),_SAFE_GET_VALUE("err_check",v));
			}
		else if(strcmp(item,"index")==0)
			{
			retv=websWrite(wp,T("%s"),_SAFE_GET_VALUE("pppoe_index",v));
			_SET_VALUE("pppoe_index","0");
			}
	}

	return retv;
}

static int  aspTendaGetStatus(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*item, *type;
	int retv=0;
	char *v = NULL;
	char *u = NULL;
	
 	if (ejArgs(argc, argv, T("%s %s"), &type, &item) < 2) 
	{
	}

	//diag_printf("==%s=type=[%s]===item=[%s]\n",__FUNCTION__,type,item);

	if(strcmp(type,"lan") == 0)
	{
		if(strcmp(item,"lanip") == 0)
		{
			return websWrite(wp,T("%s"),_SAFE_GET_VALUE(_LAN0_IP,v));
		}
		else if(strcmp(item,"lanmask") == 0)
		{
			return websWrite(wp,T("%s"),_SAFE_GET_VALUE(_LAN0_NETMASK,v));
		}
		else if(strcmp(item,"dhcps") == 0)
		{
			return websWrite(wp,T("%d"),get_dhcpd_en());
		}
		else if(strcmp(item,"dhcps_start") == 0)
		{
			 _SAFE_GET_VALUE(_LAN0_DHCPD_START,v);
		}
		else if(strcmp(item,"dhcps_end") == 0)
		{
			_SAFE_GET_VALUE(_LAN0_DHCPD_END,v);
		}
		else if(strcmp(item,"lease_time") == 0)
		{
			_SAFE_GET_VALUE(_LAN0_DHCPD_LEASET,v);
		}
		else if(strcmp(item,"dmzen") == 0)
		{
			_SAFE_GET_VALUE(_FW_DMZ_IPADDR_EN,v);
		}
		else if(strcmp(item,"dmzip") == 0)
		{
			_SAFE_GET_VALUE(_FW_DMZ_IPADDR,v);
		}
		
		return websWrite(wp, T("%s"),v);
	}
	else if(strcmp(type,"sys") == 0)
	{
		if(strcmp(item,"lanmac") == 0)
		{
			_SAFE_GET_VALUE(_LAN0_HWADDR, v);
		}
		//add by ldm
		else if(strcmp(item,"clnway") == 0)
		{
			_SAFE_GET_VALUE("clnway", v);
		}
		else if(strcmp(item,"config_num") == 0)
		{
			_SAFE_GET_VALUE("config_index", v);
		}
		//end
		else if(strcmp(item,"wanmac") == 0)
		{
			
			_SAFE_GET_VALUE("wl_mode", u);
			if(strcmp(u,"sta")==0)
				_SAFE_GET_VALUE("wl0.1_hwaddr", v);
			else
				_SAFE_GET_VALUE(_WAN0_HWADDR, v);
			//printf("+00 u =%s, v =%s\n",u,v);
#if 0			
			char wan_mac[6]={0,0,0,0,0,0};
			_SAFE_GET_VALUE(_WAN0_IFNAME, v);
			
			iflib_getifhwaddr(v,wan_mac);

			retv = websWrite(wp,T("%02X:%02X:%02X:%02X:%02X:%02X"),
				wan_mac[0]&0XFF,
				wan_mac[1]&0XFF,
				wan_mac[2]&0XFF,
				wan_mac[3]&0XFF,
				wan_mac[4]&0XFF,
				wan_mac[5]&0XFF);
				return retv;
#endif
		}
		else if(strcmp(item,"manmac") == 0)
		{
			char pcMac[20];
			char uu[20]={0};
			((struct in_addr*) pcMac)->s_addr=inet_addr(wp->ipaddr);
			//printf("%s %d pcMac:%s\n",__func__,__LINE__,pcMac);	
			if(arpioctl(SIOCGARPRT, pcMac, NULL) == 0)
			{	
				sprintf(uu,"%02X:%02X:%02X:%02X:%02X:%02X",
							pcMac[4]&0XFF,
							pcMac[5]&0XFF,
							pcMac[6]&0XFF,
							pcMac[7]&0XFF,
							pcMac[8]&0XFF,
							pcMac[9]&0XFF);
				retv = websWrite(wp,T("%02X:%02X:%02X:%02X:%02X:%02X"),
							pcMac[4]&0XFF,
							pcMac[5]&0XFF,
							pcMac[6]&0XFF,
							pcMac[7]&0XFF,
							pcMac[8]&0XFF,
							pcMac[9]&0XFF);
			}
			else
			{
				sprintf(uu,"%02X:%02X:%02X:%02X:%02X:%02X",
							0,
							0,
							0,
							0,
							0,
							0);
				retv = websWrite(wp,T("%02X:%02X:%02X:%02X:%02X:%02X"),
							0,
							0,
							0,
							0,
							0,
							0);//wrong value
			}
			_SET_VALUE(_LAN0_HWADDR1,uu);
			strcpy(uu,wp->ipaddr);
			_SET_VALUE(_LAN0_IP1,uu);
			return retv;
		}
		else if(strcmp(item,"fmac") == 0)
		{
			char fmmac[6]={0,0,0,0,0,0};
			
			iflib_getifhwaddr("br0",fmmac);

			retv = websWrite(wp,T("%02X:%02X:%02X:%02X:%02X:%02X"),
				fmmac[0]&0XFF,
				fmmac[1]&0XFF,
				fmmac[2]&0XFF,
				fmmac[3]&0XFF,
				fmmac[4]&0XFF,
				fmmac[5]&0XFF);
			return retv;
		}
		else if(strcmp(item,"upnpen") == 0)
		{
			_SAFE_GET_VALUE(_FW_UPNP_EN, v);
		}
		else if(strcmp(item,"timezone") == 0){
			_SAFE_GET_VALUE(_SYS_TZONE,v);
		}
		else if(strcmp(item,"timeMode") == 0){
			_SAFE_GET_VALUE(_SYS_NTPTYPE,v);
		}
		else if(strcmp(item,"sysver") == 0){
#if defined(__CONFIG_WEB_VERSION__)
			return websWrite(wp,T("%s_%s"),W311R_ECOS_SV,__CONFIG_WEB_VERSION__);
#else
			return websWrite(wp,T("%s"),W311R_ECOS_SV);
#endif
		}
		else if(strcmp(item,"hardwarever") == 0){
			return websWrite(wp,T("%s"),W311R_ECOS_HV);
		}
		else if(strcmp(item,"compimetime") == 0){
			return websWrite(wp,T("%s"),TWL300N_FIRMWARE_BUILD_DATE);
		}
		else if(strcmp(item,"runtime") == 0){
			return websWrite(wp, T("%u"),(unsigned int)(cyg_current_time()/100));
		}
		else if(strcmp(item,"username") == 0){
			_SAFE_GET_VALUE(HTTP_USERNAME, v);
		}
		/*add by ldm for system_password*/
		else if(strcmp(item,"password") == 0){
			_SAFE_GET_VALUE(HTTP_PASSWD, u);
			if(strcmp(u,"")==0)
				return websWrite(wp, T("%s"),"0");
			else
				return websWrite(wp, T("%s"),"1");
		}
		/*add end*/
		else if(strcmp(item,"passwordtip") == 0){
			_SAFE_GET_VALUE(HTTP_PASSWD_TIP, v);
		}
		else if(strcmp(item,"systime") == 0){
			char time_info[40];
			time_t now;
			struct tm TM;
			now = time(0);
			gmtime_r(&now,&TM);
			//diag_printf("%04d-%02d-%02d \n", TM.tm_year,TM.tm_mon, TM.tm_mday);
			if(TM.tm_year == 70)
			{
				sprintf(time_info,"%04d-%02d-%02d %02d:%02d:%02d",
				TM.tm_year + 1900+41,TM.tm_mon + 1+3,TM.tm_mday,
					TM.tm_hour,TM.tm_min,TM.tm_sec);
			}
			else
			{
				sprintf(time_info,"%04d-%02d-%02d %02d:%02d:%02d",
				TM.tm_year + 1900,TM.tm_mon + 1,TM.tm_mday,
					TM.tm_hour,TM.tm_min,TM.tm_sec);
			}
			
			return websWrite(wp, T("%s"), time_info);
		}
		else if(strcmp(item,"conclient") == 0){
			char conclient[20];
			struct arp_arpentry *arp_ent;
			memset(conclient,0,sizeof(conclient));
				
			if(arpioctl(SIOCGARPNU, conclient, NULL) == 0)
			{
				arp_ent = (struct arp_arpentry *)conclient;
				return websWrite(wp, T("%d"),arp_ent->addr.s_addr);
			}else{
				return websWrite(wp, T("%d"),1);
			}
		}
		else if(strcmp(item,"RedirectPPPoEwebDis") == 0){
			_SAFE_GET_VALUE("never_prompt_pppoe",v);
		}
		else if(strcmp(item,"RedirectWLwebDis") == 0){
			_SAFE_GET_VALUE("never_prompt_wlpwd",v);
		}
		else if(strcmp(item,"NvramChanged") == 0){
			_SAFE_GET_VALUE("nvram_changed",v);
		}
		return websWrite(wp, T("%s"),v);
	}
	else if (strcmp(type,"wan") == 0)
	{
		if(strcmp(item,"connecttype") == 0){
			return websWrite(wp, T("%d"), get_wan_type());
		}
		else if(strcmp(item,"connectsave") == 0){
			return websWrite(wp, T("%d"), get_wan_type_index());
		}
		else if(strcmp(item,"dns1") == 0){
			//wan_dns.asp
			char dns1[20]={0};
			get_wan0_dns(1,dns1);
			v = dns1;
		}
		else if(strcmp(item,"dns2") == 0){
			//wan_dns.asp
			char dns2[20]={0};
			get_wan0_dns(2,dns2);
			v = dns2;
		}
		else if(strcmp(item,"dnsen") == 0){
			_SAFE_GET_VALUE(_WAN0_DNS_FIX, v);	
		}
		else if(strcmp(item,"wanip")==0)
		{
			 _SAFE_GET_VALUE(_WAN0_IPADDR,v);
		}
		else if(strcmp(item,"wanmask")==0)
		{
			_SAFE_GET_VALUE(_WAN0_NETMASK,v);
		}
		else if(strcmp(item,"staticgateway")==0)
		{ 
			 _SAFE_GET_VALUE(_WAN0_GATEWAY,v);
		}
		else if(strcmp(item,"staticMTU")==0)
		{ 
			 _SAFE_GET_VALUE(_WAN0_MTU,v);
		}
		else if(strcmp(item,"dynamicMTU")==0)
		{ 
			_SAFE_GET_VALUE(_WAN0_MTU,v);
		}
		else if(strcmp(item,"l2tpIP")==0)
		{
			_SAFE_GET_VALUE(_WAN0_l2TP_SERVER_NAM,v);
		}
		else if(strcmp(item,"l2tpPUN")==0)
		{
			_SAFE_GET_VALUE(_WAN0_l2TP_USERNAME,v);
		}
		else if(strcmp(item,"l2tpPPW")==0)
		{
			_SAFE_GET_VALUE(_WAN0_l2TP_PASSWD,v);
		}
		else if(strcmp(item,"l2tpAdrMode")==0)
		{
			_SAFE_GET_VALUE(_WAN0_l2TP_STATIC,v);
		}
		else if(strcmp(item,"l2tpMTU")==0)
		{
			_SAFE_GET_VALUE(_WAN0_l2TP_MTU,v);
		}
		else if(strcmp(item,"l2tpWANIP")==0)
		{
			_SAFE_GET_VALUE(_WAN0_l2TP_IPADDR,v);
		}
		else if(strcmp(item,"l2tpWANMSK")==0)
		{
			_SAFE_GET_VALUE(_WAN0_l2TP_NETMASK,v);
		}
		else if(strcmp(item,"l2tpWANGW")==0)
		{
			_SAFE_GET_VALUE(_WAN0_l2TP_GATEWAY,v);
		}
		else if(strcmp(item,"l2tpDNS1")==0)
		{
			char l2tpdns1[20]={0};
			
			get_l2tp_dns(1,l2tpdns1);
			v=l2tpdns1;
		}
		else if(strcmp(item,"l2tpDNS2")==0)
		{
			char l2tpdns2[20]={0};
			
			get_l2tp_dns(2,l2tpdns2);
			v=l2tpdns2;
		}
		else if(strcmp(item,"pptpIP")==0){
			_SAFE_GET_VALUE(_WAN0_PPTP_SERVER_NAME,v);
		}
		else if(strcmp(item,"pptpPUN")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPTP_USERNAME,v);
		}
		else if(strcmp(item,"pptpPPW")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPTP_PASSWD,v);
		}
		else if(strcmp(item,"pptpAdrMode")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPTP_STATIC,v);
		}
		else if(strcmp(item,"pptpMTU")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPTP_MTU,v);
		}
		else if(strcmp(item,"pptpWANIP")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPTP_IPADDR,v);
		}
		else if(strcmp(item,"pptpWANMSK")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPTP_NETMASK,v);
		}
		else if(strcmp(item,"pptpWANGW")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPTP_GATEWAY,v);
		}
		else if(strcmp(item,"pptpMPPE")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPTP_MPPE_EN,v);
		}
		else if(strcmp(item,"pptpDNS1")==0)
		{
			char pptpdns1[20]={0};
			
			get_pptp_dns(1,pptpdns1);
			v=pptpdns1;
		}
		else if(strcmp(item,"pptpDNS2")==0)
		{
			char pptpdns2[20]={0};
			
			get_pptp_dns(2,pptpdns2);
			v=pptpdns2;
		}
		else if(strcmp(item,"wan0_route") == 0){
			_SAFE_GET_VALUE(_WAN0_ROUTE,v);
		}
		/*huangxiaoli modify*/
		else if(strcmp(item, "wanspeed") == 0)
		{
			_SAFE_GET_VALUE("wan_speed",v);
		}
		/*end modify*/
		/* huangxiaoli add for 8021x */
		else if(strcmp(item, "x1name") == 0)
		{
			_SAFE_GET_VALUE("wan0_1x_username",v);
		}
		else if(strcmp(item, "x1pwd") == 0)
		{
			_SAFE_GET_VALUE("wan0_1x_password",v);
		}
		else if(strcmp(item, "x1mode") == 0)
		{
			_SAFE_GET_VALUE("wan0_1x_ardmode",v);
		}
		else if(strcmp(item, "x1mtu") == 0)
		{
			_SAFE_GET_VALUE("wan0_1x_mtu",v);
		}
		/* end add */
		return websWrite(wp, T("%s"),v);
	}
	else if(strcmp(type,"ppoe") == 0){
		int h_s,m_s,h_e,m_e;
		if(strcmp(item,"userid")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_USERNAME, v);
			char pppoe_username[64];
			memset(pppoe_username,0,64);
			slprintf(pppoe_username, sizeof(pppoe_username), "%.*v", strlen(v),v);
			retv = websWrite(wp, T("%s"),pppoe_username);
		}
		else if(strcmp(item,"pwd")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_PASSWD, v);
			char pppoe_passwd[64];
			memset(pppoe_passwd,0,64);
			slprintf(pppoe_passwd, sizeof(pppoe_passwd), "%.*v", strlen(v), v);
			retv = websWrite(wp, T("%s"),pppoe_passwd);
		}
		else if(strcmp(item,"idletime")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_IDLETIME, v);
			retv = websWrite(wp, T("%s"),v);
		}
		else if(strcmp(item,"conmode")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_DEMAND, v);
			retv = websWrite(wp, T("%s"),v);
		}
		else if(strcmp(item,"mtu")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_MTU, v);
			retv = websWrite(wp, T("%s"),v);
		}
		else if(strcmp(item,"sev")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_SERVICE, v);
			retv = websWrite(wp, T("%s"),v);
		}
		else if(strcmp(item,"ac")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_AC, v);
			retv = websWrite(wp, T("%s"),v);
		}
		else if(strcmp(item,"pppoeAdrMode")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE2_STATIC,v);
			retv = websWrite(wp, T("%s"),v);
		}
		else if(strcmp(item,"pppoeWANIP")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE2_IPADDR,v);
			retv = websWrite(wp, T("%s"),v);
		}
		else if(strcmp(item,"pppoeWANMSK")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE2_NETMASK,v);
			retv = websWrite(wp, T("%s"),v);
		}
		else if(strcmp(item,"pppoeWANGW")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE2_GATEWAY,v);
			retv = websWrite(wp, T("%s"),v);
		}
		else if(strcmp(item,"h_s")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_ST, v);
			h_s = atoi(v)/3600;
			retv = websWrite(wp,T("%d"),h_s);
		}
		else if(strcmp(item,"m_s")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_ST, v);
			h_s = atoi(v)%3600;
			m_s = h_s/60;
			retv = websWrite(wp,T("%d"),m_s);
		}
		else if(strcmp(item,"h_e")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_ET, v);
			h_e = atoi(v)/3600;
			retv = websWrite(wp,T("%d"),h_e);
		}
		else if(strcmp(item,"m_e")==0)
		{
			_SAFE_GET_VALUE(_WAN0_PPPOE_ET, v);
			h_e = atoi(v)%3600;
			m_e = h_e/60;
			retv = websWrite(wp,T("%d"),m_e);
		}

		return retv;

	}
	else if(strcmp(type,"ddns") == 0){

		char *arglists[9];
		char ddns_value[128];
		memset(ddns_value,0,sizeof(ddns_value));
		if(strcmp(item,"en") == 0){
			_SAFE_GET_VALUE(_DDNS_ENABLE,v);
			
			if(atoi(v) >= 1){
				retv = websWrite(wp,T("%d"),1);
			}else
				retv = websWrite(wp,T("%d"),0);
		}else if(strcmp(item,"provide")==0){
			_SAFE_GET_VALUE(_DDNS_ENABLE,v);
			if(atoi(v) >= 1){
				retv = websWrite(wp,T("%d"),atoi(v) -1);
			}else
				retv = websWrite(wp,T("%d"),0);
		}else if(strcmp(item,"url") == 0){
			_SAFE_GET_VALUE(_DDNS_SET1,v);
			strncpy(ddns_value,v,sizeof(ddns_value));
			
			if(str2arglist(ddns_value, arglists, ';', 9) == 9){
				retv = websWrite(wp,T("%s"),arglists[2]);
			}else
				retv = websWrite(wp,T("%s"),"");
			
		}else if(strcmp(item,"user_name") == 0){
			_SAFE_GET_VALUE(_DDNS_SET1,v);
			strncpy(ddns_value,v,sizeof(ddns_value));
			
			if(str2arglist(ddns_value, arglists, ';', 9) == 9){
				retv = websWrite(wp,T("%s"),arglists[3]);
			}else
				retv = websWrite(wp,T("%s"),"");
		}else if(strcmp(item,"pwd") == 0){
			_SAFE_GET_VALUE(_DDNS_SET1,v);
			strncpy(ddns_value,v,sizeof(ddns_value));
			
			if(str2arglist(ddns_value, arglists, ';', 9) == 9){
				retv = websWrite(wp,T("%s"),arglists[4]);
			}else
				retv = websWrite(wp,T("%s"),"");
		}
		return retv;

	}
	
	return websWrite(wp,T(""));
}

static int aspmNatPortSegment(int eid, webs_t wp, int argc, char_t **argv)
{
	int which;
	char value[128]={'\0'},newv[128]={'\0'};
	int retv = 0;
	char *v;
	
	if(ejArgs(argc, argv, T("%d"),&which) < 1)
	{
	}
	if(which >=1 && which <=10){
		which -= 1;
		_SAFE_GET_VALUE(_FW_FORWARD_PORT(which),v);
	}
	else{
		v=NULL;
	}
	//forward_port0=50-60>192.168.1.23:50-60,tcp,on
	if(v){
		strcpy(value,v);
		if(parse_portforward2webstr(value,newv))
			retv = websWrite(wp,T("%s"),newv);
	}
	
	return retv;
}

//0x27-->'
static void dhcpd_entry2(char *buf, int id, void *arg1)
{
	webs_t req = (webs_t )arg1;
	char *p = buf;
	
	if (id>1)
		websWrite(req, T("%s"),",");

	websWrite(req, T("%c"),0x27);
	
	for(;*p != '\0';p++){				
		if(*p != 0x27) websWrite(req, T("%c"), *p);
	}
	websWrite(req, T("%c"),0x27);
}

extern int show_lan_leases(void *wp, void (*dhcpd_entry2)(char *buf, int id, void *arg1));



static int 		aspTendaGetDhcpClients(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *type=NULL;
	int i;
	char *value = NULL;
	//char name[32];
	ejArgs(argc, argv, T("%s"), &type);

	if(strcmp(type,"staticlist")==0)
	{
		for(i = 1;i<=DHCPD_STATIC_LEASE_NU;i++){
			//sprintf(name,"dhcp_static_lease%d",i);
			//_SAFE_GET_VALUE(name,value);
			_SAFE_GET_VALUE(LAN0_DHCP_SATIC(i),value);
			
			if(value && strlen(value) >4/*simple check*/ && i > 1)
				websWrite(wp, T("%s"),",");
			
			if(strlen(value) >4/*simple check*/)websWrite(wp, T("'%s'"),value);
		}
	}
	else if(strcmp(type,"list")==0)
	{	
		show_lan_leases(wp, dhcpd_entry2);
	}
	else if(strcmp(type,"dhcpipmacbind")==0)
	{
		websWrite(wp,"");
	}

	return 0;
}

void set_gLog_cur_page(int cur)
{
//call from syslog.c
	gLog_cur_page = cur;
}

static int aspSysLogGet(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*item, *type;
	int retv = 0;
	int index = 0;
	char log_time[32],log_type[32],log_data[__CONFIG_SYSLOG_LOG_SIZE__];
	char *p;
	
	if(ejArgs(argc, argv, T("%s %s"), &type, &item) < 2) {
	}

	if(strcmp(type,"system")==0)//return log context
	{
		logNum = 0;
		syslog_open();
		memset(log_time,0,sizeof(log_time));
		memset(log_type,0,sizeof(log_type));
		memset(log_data,0,sizeof(log_data));
		while(syslog_get2(log_time,log_type,log_data))
		{	
			index++;
			logNum++;
			
			if(index >= (10 * (gLog_cur_page - 1) + 1) && index < (10 * gLog_cur_page + 1))
			{
				websWrite(wp,T("<tr><td width=30>&nbsp;%d</td>"),index);
				websWrite(wp,T("<td width=120>&nbsp;%s</td>"),log_time);
				websWrite(wp,T("<td width=50>&nbsp;%s</td>"),log_type);
				p = strchr(log_data,'\n');
				if(p) *p = '\0';//remove '\n'
				websWrite(wp,T("<td>&nbsp;%s</td ></tr>"),log_data);	
			}
			memset(log_time,0,sizeof(log_time));
			memset(log_type,0,sizeof(log_type));
			memset(log_data,0,sizeof(log_data));		
		}

		syslog_close();
	}
	else if(strcmp(type,"count")==0)//retrun log number
	{
		retv = websWrite(wp,T("%d"),logNum);
	}
	else if(strcmp(type,"curcount")==0)//return cur page number
	{
		retv = websWrite(wp,T("%d"),gLog_cur_page);
	}
	return retv;
}


static void	fromAdvSetLanip(webs_t wp, char_t *path, char_t *query)
{
	char_t  *lan_ip, *go,*lan_mask,*value;
	char_t dhcp_ip_start[20],dhcp_ip_end[20];
	unsigned int dhcp_ip[4],lan_ip2[4];

	char_t old_lan_ip[20],old_lanmask[20],wan_ip[20],wan_mask[20];

	//unsigned int ul_dhcp_start_ip,ul_dhcp_end_ip,ul_lanip,ul_lanmask;
	//int which;

	struct in_addr;
	 //struct inip;

	lan_ip = websGetVar(wp, T("LANIP"), T("192.168.0.1")); 
	lan_mask = websGetVar(wp,T("LANMASK"),T("255.255.255.0"));
	go = websGetVar(wp, T("GO"), T("")); 

	strcpy(old_lan_ip,_GET_VALUE(_LAN0_IP,value));
	strcpy(old_lanmask,_GET_VALUE(_LAN0_NETMASK,value));

	_SET_VALUE(_LAN0_IP,lan_ip);
	_SET_VALUE(_LAN0_NETMASK,lan_mask);

	if(get_wan_type() == STATICMODE){
		strcpy(wan_ip,_GET_VALUE(_WAN0_IPADDR,value));
		strcpy(wan_mask,_GET_VALUE(_WAN0_NETMASK,value));
		if (CGI_same_net_with_lan(inet_addr(wan_ip),inet_addr(wan_mask)))
		{
			_SET_VALUE(_LAN0_IP,old_lan_ip);
			_SET_VALUE(_LAN0_NETMASK,old_lanmask);
			
			SetErrorArg(LAN_WAN_IP_ERROR,"lan.asp");
			websRedirect(wp, T("error.asp"));
			return;
		}
	}

	
	if(CGI_same_net_with_lan(SYS_lan_ip,SYS_lan_mask) == 0){
		//change dhcp settings
		strcpy(dhcp_ip_start,_GET_VALUE(_LAN0_DHCPD_START,value));
		strcpy(dhcp_ip_end,_GET_VALUE(_LAN0_DHCPD_END,value));

		sscanf(lan_ip, "%d.%d.%d.%d", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);
		
		sscanf(dhcp_ip_start, "%d.%d.%d.%d", &dhcp_ip[0], &dhcp_ip[1], &dhcp_ip[2], &dhcp_ip[3]);
		sprintf(dhcp_ip_start,"%d.%d.%d.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],dhcp_ip[3]);

		sscanf(dhcp_ip_end, "%d.%d.%d.%d", &dhcp_ip[0], &dhcp_ip[1], &dhcp_ip[2], &dhcp_ip[3]);
		sprintf(dhcp_ip_end,"%d.%d.%d.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],dhcp_ip[3]);
		
#if 0		
		ul_dhcp_start_ip = inet_addr(dhcp_ip_start);
		ul_dhcp_end_ip = inet_addr(dhcp_ip_end);
		ul_lanip = inet_addr(lan_ip);

		ul_dhcp_start_ip = ((ul_dhcp_start_ip & 0xff000000) | (ul_lanip & 0x00ffffff));
		ul_dhcp_end_ip = ((ul_dhcp_end_ip & 0xff000000) | (ul_lanip & 0x00ffffff));

		memset(dhcp_ip_start,0,sizeof(dhcp_ip_start));
		memset(dhcp_ip_end,0,sizeof(dhcp_ip_end));
		inip.s_addr = ul_dhcp_start_ip;
		strcpy(dhcp_ip_start,inet_ntoa(inip));
		inip.s_addr = ul_dhcp_end_ip;
		strcpy(dhcp_ip_end,inet_ntoa(inip));
#endif
		_SET_VALUE(_LAN0_DHCPD_START,dhcp_ip_start);
		_SET_VALUE(_LAN0_DHCPD_END,dhcp_ip_end);
		
		//set filter
		modify_filter_virtual_server(lan_ip);
#if 0		
		//clean static dhcp lease
		for(which = 0; which<= DHCPD_STATIC_LEASE_NU; which++){
			_SET_VALUE(LAN0_DHCP_SATIC(which),"");
		}

		//clean url filter
		_SET_VALUE(_FW_FLT_URL_EN,"disable");
		_SET_VALUE(_FW_FLT_URL_CUR_NU,"1");
		for(which=0;which<= 9;which++){
			_SET_VALUE(_FW_FILTER_URL(which), "");	
		}

		//clean ip filter
		_SET_VALUE(_FW_FLT_CLN_EN,"disable");
		_SET_VALUE(_FW_FLT_CLN_CUR_NU,"1");
		for(which=0;which<= 9;which++){
			_SET_VALUE(_FW_FILTER_CLIENT(which), "");	
		}
		//clean mac filter
		_SET_VALUE(_FW_FLT_MAC_CUR_NU,"1");
		for(which=0;which<= 9;which++){
			_SET_VALUE(_FW_FILTER_MAC(which), "");	
		}
#endif
		//clean dmz
		//_SET_VALUE(_FW_DMZ_IPADDR_EN,"0");
		//_SET_VALUE(_FW_DMZ_IPADDR,"");

		//clean static route
		//_SET_VALUE(_WAN0_ROUTE,"");
	}
	
	_SET_VALUE("err_check","0");
	_COMMIT();
	websRedirect(wp, T("/direct_reboot.asp"));	
	
	cyg_thread_delay(200);
		
	_REBOOT();
}
/*huangxiaoli modify*/
static void	fromsetWanSpeed(webs_t wp, char_t *path, char_t *query)
{
	char_t *wanspeed;
	int speed;

	wanspeed =websGetVar(wp, T("ws"), T("0"));
	speed = atoi(wanspeed);
	_SET_VALUE("wan_speed", wanspeed);
	_COMMIT();
	ifr_set_link_speed2(speed);
	websRedirect(wp, T("/wan_speed.asp"));
}
/*end modify*/
extern void set_wireless_secur_from_indexasp(char *pass_phrase_str);

static void fromWizardHandle(webs_t wp, char_t *path, char_t *query)
{
	int want;
	char_t *rebootFlag, *indexssid,*mac;
	char_t *wantstr, *go, *ssid1, *wlpassword;
	
	mac = websGetVar(wp, T("MACC"), T("")); 
	rebootFlag = websGetVar(wp, T("rebootTag"), T("1"));
	wantstr = websGetVar(wp, T("WANT1"), T("127"));
	go = websGetVar(wp, T("GO"), T(""));
	ssid1 =  websGetVar(wp, T("wirelessusername"), T(""));
	wlpassword =  websGetVar(wp, T("wirelesspassword"), T(""));

	/*wantstr = 1 static ip, =2 DHCP, =3 PPPOE, =4 PPTP, =5 L2TP, =6 1x, =7 DHCP+ */
	want = atoi(wantstr);

	if(DHCPMODE == want){
		//dhcp
		_SET_VALUE(_WAN0_MTU,"1500");
	}
	else if(want == PPPOEMODE){
		//pppoe
		char_t *pppoeuser, *pppoepwd, *conmode;
		char *xkjs_user_id,*xkjs_pwd;
		char tmp_xkjs_user_id[64];
		char tmp_xkjs_pwd[64];

		_SET_VALUE("plugplay_flag","n");//gong add
		pppoeuser = websGetVar(wp, T("PUN"), T("")); 
		pppoepwd = websGetVar(wp, T("PPW"), T("")); 
		conmode = websGetVar(wp, T("PCM"), T("0"));
		char_t *v12_time = websGetVar(wp,T("v12_time"),T("0"));

		
		_SAFE_GET_VALUE(_WAN0_PPPOE_USERNAME, xkjs_user_id);
		slprintf(tmp_xkjs_user_id, sizeof(tmp_xkjs_user_id), "%.*v", strlen(xkjs_user_id),xkjs_user_id);
		_SAFE_GET_VALUE(_WAN0_PPPOE_PASSWD, xkjs_pwd);
		slprintf(tmp_xkjs_pwd, sizeof(tmp_xkjs_pwd), "%.*v", strlen(xkjs_pwd),xkjs_pwd);
		if(strcmp(pppoeuser,tmp_xkjs_user_id) == 0)
			;
		else{
			_SET_VALUE(_WAN0_PPPOE_USERNAME,pppoeuser);
		}
		if(strcmp(pppoepwd,tmp_xkjs_pwd) == 0)
			;
		else{
			_SET_VALUE(_WAN0_PPPOE_PASSWD,pppoepwd);
		}
		
		//conmode:0,auto;1,traffic;2,hand;3,time
		_SET_VALUE(_WAN0_PPPOE_DEMAND,"0");//auto,default value
#if defined(CONFIG_CHINA_NET_CLIENT)
		_SET_VALUE(_WAN0_PPPOE_XKJX_TIME,v12_time);
#endif
	}
	
	/*mac clone*/
	if(strcmp(mac,"") != 0){
		_SET_VALUE(_WAN0_HWADDR,mac);
	}
	set_wan_str(want);
	set_wan_str_index(want);
	set_wireless_secur_from_indexasp(wlpassword);
#ifdef __CONFIG_WPS__			
	stop_wireless_wps();
#endif
#ifdef __CONFIG_WPS_LED__
	wps_led_test_off();		
#endif
	_SET_VALUE("wl0_ssid",ssid1);
	_SET_VALUE("err_check","0");
	_SET_VALUE("config_index","1");
	_SET_VALUE("wan0_check","0");
	_SET_VALUE("mode_need_switch","no");
	
	_COMMIT();
	_RESTART_ALL();

	SetErrorArg(CONFIG_SUCCESS,"index.asp");
	websRedirect(wp, T("index.asp"));	
}

static int rte_num1;

static int mtenda_routing_entry(struct radix_node *rn, void *vw)
{
   struct rtentry *rt = (struct rtentry *)rn;   
    
    struct sockaddr *dst, *gate, *netmask, *genmask;
    char addr[32]={0}, addr1[32]={0}, addr2[32]={0},ifname[32]={0};
    webs_t req = (webs_t )vw;

    dst = rt_key(rt);
    gate = rt->rt_gateway;
    netmask = rt_mask(rt);
    genmask = rt->rt_genmask;
    if ((rt->rt_flags & (RTF_UP | RTF_LLINFO)) == RTF_UP) 
	{
	        _inet_ntop(dst, addr, sizeof(addr));

	        if (netmask != NULL) {
	            _mask(netmask, addr2, sizeof(addr2));
	        } else {
	        	if(rt->rt_flags & RTF_HOST) 
	        		sprintf(addr2, "255.255.255.255");
	        	else
	            	sprintf(addr2, " ");
	        }
	       
	        if (gate != NULL && gate->sa_family == AF_INET) {
	            _inet_ntop(gate, addr1, sizeof(addr1));
	        } else if(gate != NULL && gate->sa_family == AF_LINK) {
	        	_inet_ntop((struct sockaddr *)rt_key(rt), addr1, sizeof(addr1));
	        } else {
	            sprintf(addr1,"unknown");
	        }

		 if_indextoname(rt->rt_ifp->if_index, ifname);
		 if(strcmp(ifname,"lo0") != 0){
		 	//don't show lo0 interface
			if (rte_num1++!=0)
				websWrite(req,",");
				
		        websWrite(req, T("'%s,%s,%s,%d,"), addr,addr2,addr1,rt->rt_rmx.rmx_hopcount);
			
			 websWrite(req, T("%s'"), ifname);
		 }
    }
    return 0;
}


static int aspmGetRouteTable(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t  *type, *item;
	
	if(ejArgs(argc, argv, T("%s %s"), &type, &item) < 2)
	{
	}

	if(strcmp(type,"sys") == 0)
	{
		websWrite(wp,"");//write a default value first
		rte_num1 = 0;
		show_network_tables2(mtenda_routing_entry,wp);
	}

	return 0;
}


static void fromSysStatusHandle(webs_t wp, char_t *path, char_t *query)
{
	char_t  *ac, *go;
	int iac = 0;
	

	ac = websGetVar(wp, T("action"), T(""));
	go = websGetVar(wp, T("GO"), T(""));
	iac = atoi(ac);
	_SET_VALUE("pppoe_index",ac);	

	
	CGI_do_wan_connect_tenda(iac);

	cyg_thread_delay(200);

	websRedirect(wp, T(go));
}

static void fromDDNSinit(webs_t wp, char_t *path, char_t *query)
{
	char result[2048] = {0};
	char *arglists[9]= {NULL};
	char *v = NULL;
	
	char ddns_value[128] = {0};
	char en[64] = {0}, status[64] = {0};
	char isp[64] = {0};
	char hostinfo[128] = {0};
	char username[128] = {0};
	char password[128] = {0};
	
	/*ddns enable*/
	_SAFE_GET_VALUE(_DDNS_ENABLE,v);
	sprintf(en,"%s",v);
	strncat(result, en, 2048);
	strncat(result, "\r", 2048);
	
	/*ddns isp*/
	_SAFE_GET_VALUE(_DDNS_ISP,v);
	sprintf(isp,"%s",v);
	strncat(result, isp, 2048);
	strncat(result, "\r", 2048);
	
	_SAFE_GET_VALUE(_DDNS_SET1,v);
	strncpy(ddns_value,v,sizeof(ddns_value));
	if(str2arglist(ddns_value, arglists, ';', 9) == 9){
		sprintf(username,"%s",arglists[3]);
		sprintf(password,"%s",arglists[4]);
		sprintf(hostinfo,"%s",arglists[2]);
	}else{
		sprintf(username,"%s","");
		sprintf(password,"%s","");
		sprintf(hostinfo,"%s","");
	}

	/*ddns username*/
	strncat(result, username, 2048);
	strncat(result, "\r", 2048);

	/*ddns password*/
	strncat(result, password, 2048);
	strncat(result, "\r", 2048);
	
	/*ddns hostinfo*/
	strncat(result, hostinfo, 2048);
	strncat(result, "\r", 2048);

	/*ddns status*/
	_SAFE_GET_VALUE(_DDNS_STATUS,v);
	sprintf(status,"%s",v);
	strncat(result, status, 2048);
	strncat(result, "\r", 2048);
	
	strncat(result, "\n", 2048);

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);	
	
	return ;
}



static void fromSysStatusInit(webs_t wp, char_t *path, char_t *query)
{
	char result[2048] = {0};
	
	char wan_connsta[8] = {0}, wan_conntyp[8] = {0};
	char wan_ip[32] = {0}, wan_mask[32] = {0}, wan_gw[32] = {0};
	char wan_dns_1[32] = {0}, wan_dns_2[32] = {0};
	char wan_contime[64] = {0};
	char wan_ppp_chk[8] = {0}, network_sta[8] = {0}, wanlink_sta[16] = {0};

	char lan_mac[32] = {0}, wan_mac[32] = {0};
	char sys_time[64] = {0}, run_time[64] = {0};
	char cli_num[8] = {0};
	char sw_vers[64] = {0}, hw_vers[64] = {0};

	char lan_ip[32] = {0}, lan_mask[32] = {0};

	char SSID1_mode[16] = {0};	
	char SSID1_name[128] = {0}, SSID1_passwd[128] = {0};

	wan_link_check();
	int wanConnSta = get_wan_connstatus();
	int wanConnTyp = get_wan_type();
	int wanLinkSta = get_wan_linkstatus();
	int	netWorkSta = get_wan_onln_connstatus();
	
//WAN	
	/*wan conStatus*/
	sprintf(wan_connsta,"%d",wanConnSta);
	//printf("---------%d\n",wanConnSta);
	strncat(result, wan_connsta, 2048);
	strncat(result, "\r", 2048);
	
	/*wan conType*/
	sprintf(wan_conntyp,"%d",wanConnTyp);
	strncat(result, wan_conntyp, 2048);
	strncat(result, "\r", 2048);

	/*wan ip*/
	sprintf(wan_ip,"%s", wanConnSta==2?NSTR(SYS_wan_ip):"");
	strncat(result, wan_ip, 2048);
	strncat(result, "\r", 2048);

	/*wan mask*/
	sprintf(wan_mask,"%s",wanConnSta==2?NSTR(SYS_wan_mask):"");
	strncat(result, wan_mask, 2048);
	strncat(result, "\r", 2048);
	
	/*wan gateway*/
	sprintf(wan_gw,"%s",wanConnSta==2?NSTR(SYS_wan_gw):"");
	strncat(result, wan_gw, 2048);
	strncat(result, "\r", 2048);

	/*wan dns1*/
	if(wanConnSta == 2)
		if(wanConnTyp == PPPOEMODE ||wanConnTyp == PPTPMODE ||
			wanConnTyp == L2TPMODE||wanConnTyp==PPPOEMODE2	){
			sprintf(wan_dns_1,T("%s"),SYS_dns_1);
		}else{
			get_wan0_dns(1,wan_dns_1);
		}
	else
		sprintf(wan_dns_1,"%s","");

	strncat(result, wan_dns_1, 2048);
	strncat(result, "\r", 2048);
	
	/*wan dns2*/
	if(wanConnSta == 2)
		if(wanConnTyp == PPPOEMODE ||wanConnTyp == PPTPMODE ||
			wanConnTyp == L2TPMODE||wanConnTyp==PPPOEMODE2	){
			sprintf(wan_dns_2,T("%s"),SYS_dns_2);
		}else{
			get_wan0_dns(2,wan_dns_2);
		}
	else
		sprintf(wan_dns_2,"%s","");

	strncat(result, wan_dns_2, 2048);
	strncat(result, "\r", 2048);

	/*wan connect time*/
	sprintf(wan_contime,"%u",(wanConnSta==2&&SYS_wan_conntime)?(time(0)-SYS_wan_conntime):0);
	strncat(result, wan_contime, 2048);
	strncat(result, "\r", 2048);	

	/*wan pppoe check*/
	sprintf(wan_ppp_chk,"%s",nvram_safe_get("err_check"));
	strncat(result, wan_ppp_chk, 2048);
	strncat(result, "\r", 2048);

	/*wan link status*/
	sprintf(wanlink_sta,"%d",wanLinkSta);
	strncat(result, wanlink_sta, 2048);
	strncat(result, "\r", 2048);

	/*network status*/
	sprintf(network_sta,"%d",netWorkSta);
	strncat(result, network_sta, 2048);
	strncat(result, "\r", 2048);
		
//SYS	
	/*lan mac*/
	sprintf(lan_mac,"%s",nvram_safe_get(_LAN0_HWADDR));
	strncat(result, lan_mac, 2048);
	strncat(result, "\r", 2048);	

	/*wan mac*/	
	char *u, *v;
	
	_SAFE_GET_VALUE("wl0_mode", u);
	if(strcmp(u,"sta")==0)
		_SAFE_GET_VALUE("wl0.1_hwaddr", v);
	else
		_SAFE_GET_VALUE(_WAN0_HWADDR, v);
	
	sprintf(wan_mac,"%s",v);
	strncat(result, wan_mac, 2048);
	strncat(result, "\r", 2048);

	/*sys time*/
	time_t now;
	struct tm TM;
	
	now = time(0);
	gmtime_r(&now,&TM);
	
	if(TM.tm_year == 70){
		sprintf(sys_time,"%04d-%02d-%02d %02d:%02d:%02d",
		TM.tm_year + 1900+41,TM.tm_mon + 1+3,TM.tm_mday,
			TM.tm_hour,TM.tm_min,TM.tm_sec);
	}
	else{
		sprintf(sys_time,"%04d-%02d-%02d %02d:%02d:%02d",
		TM.tm_year + 1900,TM.tm_mon + 1,TM.tm_mday,
			TM.tm_hour,TM.tm_min,TM.tm_sec);
	}
	
	strncat(result, sys_time, 2048);
	strncat(result, "\r", 2048);
	
	/*run time*/
	sprintf(run_time,"%u",(unsigned int)(cyg_current_time()/100));
	strncat(result, run_time, 2048);
	strncat(result, "\r", 2048);
	
	/*client num*/
	char conclient[20] = {0};
	struct arp_arpentry *arp_ent;
		
	if(arpioctl(SIOCGARPNU, conclient, NULL) == 0){
		arp_ent = (struct arp_arpentry *)conclient;
		sprintf(cli_num,"%d",arp_ent->addr.s_addr);
	}else{
		sprintf(cli_num,"%d",1);
	}
	
	strncat(result, cli_num, 2048);
	strncat(result, "\r", 2048);
	
	/*software version*/
#if defined(__CONFIG_WEB_VERSION__)
	sprintf(sw_vers,T("%s_%s"),W311R_ECOS_SV,__CONFIG_WEB_VERSION__);
#else
	sprintf(sw_vers,T("%s"),W311R_ECOS_SV);
#endif
	strncat(result, sw_vers, 2048);
	strncat(result, "\r", 2048);	

	/*hardware version*/
	sprintf(hw_vers,"%s",W311R_ECOS_HV);
	strncat(result, hw_vers, 2048);
	strncat(result, "\r", 2048);
//LAN
	/*lan ip*/
	sprintf(lan_ip,"%s",nvram_safe_get(_LAN0_IP));
	strncat(result, lan_ip, 2048);
	strncat(result, "\r", 2048);
	
	/*lan mask*/
	sprintf(lan_mask,"%s",nvram_safe_get(_LAN0_NETMASK));
	strncat(result, lan_mask, 2048);
	strncat(result, "\r", 2048);

//WL
	/*wireless mode*/
	get_wl0_mode(SSID1_mode);
	strncat(result, SSID1_mode, 2048);
	strncat(result, "\r", 2048);

	/*ssid1 name*/
	sprintf(SSID1_name,"%s",nvram_safe_get("wl0_ssid"));
	strncat(result, SSID1_name, 2048);
	strncat(result, "\r", 2048);

	/*ssid1 password*/
	get_wl0_passwd(SSID1_passwd);
	strncat(result, SSID1_passwd, 2048);
	strncat(result, "\r", 2048);	
	
	strncat(result, "\n", 2048);

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);	
	
	return ;
}


static void fromAdvSetWan(webs_t wp, char_t *path, char_t *query)

{
	int t,wanmod;
	//int rc;
	char val[128];
	char dns3[128];
	char_t *mac;

	char *want = websGetVar(wp, T("WANT2"), T("127")); 

	mac = websGetVar(wp, T("MACC"), T("NULL")); 
	
	wanmod = atoi(want);//get_wan_type();

	if(C8021XMODE == wanmod)
	{
		char_t  *x1name,*x1pwd,*x1ardmode,*wanip, *wanmsk, *gw,*dns1, *dns2,*c8021xMtu;
		x1name = websGetVar(wp, T("x1_name"), T("")); 
		x1pwd = websGetVar(wp, T("x1_pwd"), T("")); 
		x1ardmode = websGetVar(wp, T("x1AdrMode"), T("")); 
		wanip = websGetVar(wp, T("WANIP"), T("0.0.0.0")); 
		wanmsk = websGetVar(wp, T("WANMSK"), T("0.0.0.0")); 
		gw = websGetVar(wp, T("WANGW"), T("0.0.0.0")); 
		dns1 = websGetVar(wp, T("DS1"), T(""));
		dns2 = websGetVar(wp, T("DS2"), T(""));
		c8021xMtu = websGetVar(wp, T("x1_MTU"), T("1500"));
		if (CGI_same_net_with_lan(inet_addr(wanip),inet_addr(wanmsk)))
		{
			SetErrorArg(LAN_WAN_IP_ERROR,"wan_connected.asp");
			websRedirect(wp, T("error.asp"));
			return;
		}
		_SET_VALUE("wan0_1x_username",x1name);
		_SET_VALUE("wan0_1x_password",x1pwd);
		_SET_VALUE("wan0_1x_ardmode",x1ardmode);
		_SET_VALUE(_WAN0_IPADDR,wanip);
		_SET_VALUE(_WAN0_NETMASK,wanmsk);
		_SET_VALUE(_WAN0_GATEWAY,gw);

		sprintf(dns3,"%s %s",dns1,dns2);
		_SET_VALUE(_WAN0_DNS,dns3);
		_SET_VALUE("wan0_1x_mtu",c8021xMtu);
		_SET_VALUE(_WAN0_MTU,c8021xMtu);
	}
	else if( STATICMODE ==wanmod)
	{
		char_t  *wanip, *wanmsk, *gw,*dns1, *dns2,*staticMtu;	
		wanip = websGetVar(wp, T("WANIP"), T("0.0.0.0")); 
		wanmsk = websGetVar(wp, T("WANMSK"), T("0.0.0.0")); 
		gw = websGetVar(wp, T("WANGW"), T("0.0.0.0")); 
		dns1 = websGetVar(wp, T("DS1"), T(""));
		dns2 = websGetVar(wp, T("DS2"), T(""));
		staticMtu = websGetVar(wp, T("staticMTU"), T("1500"));
		if (CGI_same_net_with_lan(inet_addr(wanip),inet_addr(wanmsk)))
		{
			SetErrorArg(LAN_WAN_IP_ERROR,"wan_connected.asp");
			websRedirect(wp, T("error.asp"));
			return;
		}

		if (strcmp(wanip,gw)==0||strcmp(wanip,dns1)==0||strcmp(wanip,dns2)==0)
		{
			SetErrorArg(WAN_IP_ERROR,"wan_connected.asp");
			websRedirect(wp, T("error.asp"));
			return;
		}
		_SET_VALUE(_WAN0_IPADDR,wanip);
		_SET_VALUE(_WAN0_NETMASK,wanmsk);
		_SET_VALUE(_WAN0_GATEWAY,gw);

		sprintf(dns3,"%s %s",dns1,dns2);
		_SET_VALUE(_WAN0_DNS,dns3);
		_SET_VALUE(_WAN0_MTU,staticMtu);
	}
	else if(DHCPMODE == wanmod)
	{
		char_t  *DynStaticMtu;
		
		DynStaticMtu = websGetVar(wp, T("dynamicMTU"), T("1500"));

		 _SET_VALUE(_WAN0_MTU,DynStaticMtu);
	}
	else if(PPPOEMODE == wanmod)
	{
		char_t *user_id, *pwd, *mtu, *ac, *sev, *conmode;
		char_t *idle_time, *hour_s, *min_s, *hour_e, *min_e;
		char_t *v12_time;

		char *xkjs_user_id,*xkjs_pwd;
		char tmp_xkjs_user_id[64];
		char tmp_xkjs_pwd[64];

		diag_printf("============advset=====\n");
		_SET_VALUE("plugplay_flag","n");//gong add
		user_id = websGetVar(wp, T("PUN"), T("")); 
		pwd = websGetVar(wp, T("PPW"), T("")); 
		mtu = websGetVar(wp, T("MTU"), T("1492"));
		ac = websGetVar(wp, T("AC"), T(""));
		sev = websGetVar(wp, T("SVC"), T(""));
		conmode = websGetVar(wp, T("PCM"), T("0"));
		idle_time = websGetVar(wp, T("PIDL"), T("60"));
		
		hour_s = websGetVar(wp, T("hour1"), T("0"));
		min_s = websGetVar(wp, T("minute1"), T("0"));
		hour_e = websGetVar(wp, T("hour2"), T("0"));
		min_e = websGetVar(wp, T("minute2"), T("0"));
		v12_time = websGetVar(wp,T("v12_time"),T("0"));

		///*
		//gong modify start 
		_SAFE_GET_VALUE(_WAN0_PPPOE_USERNAME, xkjs_user_id);
		slprintf(tmp_xkjs_user_id, sizeof(tmp_xkjs_user_id), "%.*v", strlen(xkjs_user_id),xkjs_user_id);
		_SAFE_GET_VALUE(_WAN0_PPPOE_PASSWD, xkjs_pwd);
		slprintf(tmp_xkjs_pwd, sizeof(tmp_xkjs_pwd), "%.*v", strlen(xkjs_pwd),xkjs_pwd);
		//diag_printf("============tmp_xkjs_user_id=====%s\n",tmp_xkjs_user_id);
		//diag_printf("============user_id=====%s\n",user_id);
		if(strcmp(user_id,tmp_xkjs_user_id) == 0)
			;
			else
			{_SET_VALUE(_WAN0_PPPOE_USERNAME,user_id);}
		if(strcmp(pwd,tmp_xkjs_pwd) == 0)
			;
			else
			{_SET_VALUE(_WAN0_PPPOE_PASSWD,pwd);}
		//gong modify finish
		//*/
		
	//	_SET_VALUE(_WAN0_PPPOE_USERNAME,user_id);
	//	_SET_VALUE(_WAN0_PPPOE_PASSWD,pwd);
		_SET_VALUE(_WAN0_PPPOE_MTU,mtu);
		_SET_VALUE(_WAN0_PPPOE_MRU,mtu);//important for mtu
		_SET_VALUE(_WAN0_PPPOE_AC,ac);
		_SET_VALUE(_WAN0_PPPOE_SERVICE,sev);
		//conmode:0,auto;1,traffic;2,hand;3,time
		_SET_VALUE(_WAN0_PPPOE_DEMAND,conmode);
		//to be added idle_time
		if(atoi(conmode) == PPPOE_TRAFFIC)
			_SET_VALUE(_WAN0_PPPOE_IDLETIME,idle_time);

		if(atoi(conmode) == PPPOE_BY_TIME){
			t = atoi(hour_s)*3600 + atoi(min_s)*60;
			sprintf(val,"%d",t);
			
			_SET_VALUE(_WAN0_PPPOE_ST,val);

			t = atoi(hour_e)*3600 + atoi(min_e)*60;
			sprintf(val,"%d",t);

			_SET_VALUE(_WAN0_PPPOE_ET,val);
		}
#if defined(CONFIG_CHINA_NET_CLIENT)
//wan0_pppoe_xkjx_time
		_SET_VALUE(_WAN0_PPPOE_XKJX_TIME,v12_time);
#endif
	}
	else if(PPPOEMODE2 == wanmod)
	{//pppoe
		char_t *user_id, *pwd, *p2MTU,*pppoeip,*pppoemode, *pppoemask, *pppoegw;
		//char_t pppoedns[40],*pppoedns1,*pppoedns2;
		char_t *pppoe2ac,*pppoe2ser,*pppoe2hostname,*pppoe2mtu2;

		user_id = websGetVar(wp, T("PUN"), T("")); 
		pwd = websGetVar(wp, T("PPW"), T("")); 
		p2MTU = websGetVar(wp, T("MTU"), T("1492"));

		
		pppoemode = websGetVar(wp, T("pppoeAdrMode"), T("1"));
		pppoeip = websGetVar(wp, T("pppoeWANIP"), T(""));
		pppoemask = websGetVar(wp, T("pppoeWANMSK"), T(""));
		pppoegw = websGetVar(wp, T("pppoeWANGW"), T(""));

		pppoe2ac = websGetVar(wp, T("AC"), T("1"));
		pppoe2ser = websGetVar(wp, T("SVC"), T(""));
		pppoe2hostname = websGetVar(wp, T("pppoehostname"), T(""));
		pppoe2mtu2 = websGetVar(wp, T("dynamicMTU"), T("1460"));
	
		
		if(strcmp(pppoemode,"1") == 0){
			if (CGI_same_net_with_lan(inet_addr(pppoeip),inet_addr(pppoemask)))
			{
				SetErrorArg(LAN_WAN_IP_ERROR,"wan_connected.asp");
				websRedirect(wp, T("error.asp"));
				return;
			}
		}
				
		_SET_VALUE(_WAN0_PPPOE2_USERNAME,user_id);
		_SET_VALUE(_WAN0_PPPOE2_PASSWD,pwd);
		_SET_VALUE(_WAN0_PPPOE2_MTU,p2MTU);
		_SET_VALUE(_WAN0_PPPOE2_MRU,p2MTU);
		_SET_VALUE(_WAN0_PPPOE2_AC,pppoe2ac);
		_SET_VALUE(_WAN0_PPPOE2_SERVICE,pppoe2ser);

		
		_SET_VALUE(_WAN0_PPPOE2_STATIC,pppoemode);

		if(strcmp(pppoemode,"1") == 0){
			_SET_VALUE(_WAN0_PPPOE2_IPADDR,pppoeip);
			_SET_VALUE(_WAN0_PPPOE2_NETMASK,pppoemask);
			_SET_VALUE(_WAN0_PPPOE2_GATEWAY,pppoegw);
		}else{
			_SET_VALUE(_WAN0_PPPOE2_HAOTNAME,pppoe2hostname);
		}
		_SET_VALUE(_WAN0_PPPOE2_MTU2,pppoe2mtu2);
		
	}
	else if(wanmod == PPTPMODE || wanmod == PPTPMODE2)
	{//pptp
		char_t  *pptpip, *pptpuser, *pptppwd, *pptpmtu, *pptpsrv, *pptpmode, *pptpmask, *pptpgw, *pptpMppe;
		char_t pptpdns[40],*pptpdns1,*pptpdns2;
		
		pptpsrv = websGetVar(wp, T("pptpIP"), T("0.0.0.0"));
		pptpuser = websGetVar(wp, T("pptpPUN"), T(""));
		pptppwd = websGetVar(wp, T("pptpPPW"), T(""));
		pptpmtu = websGetVar(wp, T("pptpMTU"), T("1452"));
		
		pptpmode = websGetVar(wp, T("pptpAdrMode"), T("1"));

		pptpip = websGetVar(wp, T("pptpWANIP"), T(""));
		pptpmask = websGetVar(wp, T("pptpWANMSK"), T(""));
		pptpgw = websGetVar(wp, T("pptpWANGW"), T(""));

		pptpdns1 = websGetVar(wp, T("pptpDNS1"), T(""));
		pptpdns2 = websGetVar(wp, T("pptpDNS2"), T(""));

		pptpMppe = websGetVar(wp, T("mppeEn"), T("0"));

		if(strcmp(pptpmode,"1") == 0){
			if (CGI_same_net_with_lan(inet_addr(pptpip),inet_addr(pptpmask)))
			{
				SetErrorArg(LAN_WAN_IP_ERROR,"wan_connected.asp");
				websRedirect(wp, T("error.asp"));
				return;
			}
			if (strcmp(pptpip,pptpgw)==0||strcmp(pptpip,pptpdns1)==0||strcmp(pptpip,pptpdns2)==0)
			{
				SetErrorArg(WAN_IP_ERROR,"wan_connected.asp");
				websRedirect(wp, T("error.asp"));
				return;
			}
		}
	

		_SET_VALUE(_WAN0_PPTP_SERVER_NAME,pptpsrv);
		_SET_VALUE(_WAN0_PPTP_USERNAME,pptpuser);
		_SET_VALUE(_WAN0_PPTP_PASSWD,pptppwd);

		_SET_VALUE(_WAN0_PPTP_MTU,pptpmtu);
		_SET_VALUE(_WAN0_PPTP_MRU,pptpmtu);//important for mtu

		_SET_VALUE(_WAN0_PPTP_STATIC,pptpmode);

		if(strcmp(pptpmode,"1") == 0){
			_SET_VALUE(_WAN0_PPTP_IPADDR,pptpip);
			_SET_VALUE(_WAN0_PPTP_NETMASK,pptpmask);
			_SET_VALUE(_WAN0_PPTP_GATEWAY,pptpgw);
			sprintf(pptpdns,"%s %s",pptpdns1,pptpdns2);
			 _SET_VALUE(_WAN0_PPTP_DNS,pptpdns);
		}
		
		 _SET_VALUE(_WAN0_PPTP_MPPE_EN,pptpMppe);
				
	}
	else if(wanmod == L2TPMODE)
	{//l2tp
		char_t  *l2tpip, *l2tpuser, *l2tppwd, *l2tpmode, *l2tpmtu, *l2tpwip, *l2tpwmask, *l2tpwgw;
		char_t l2tpdns[40],*l2tpdns1,*l2tpdns2;
		
		l2tpip = websGetVar(wp, T("l2tpIP"), T(""));

		l2tpuser = websGetVar(wp, T("l2tpPUN"), T(""));
		l2tppwd = websGetVar(wp, T("l2tpPPW"), T(""));
		l2tpmtu = websGetVar(wp, T("l2tpMTU"), T("1400"));

		
		l2tpmode = websGetVar(wp, T("l2tpAdrMode"), T("1"));

		l2tpwip = websGetVar(wp, T("l2tpWANIP"), T(""));
		l2tpwmask = websGetVar(wp, T("l2tpWANMSK"), T(""));
		l2tpwgw = websGetVar(wp, T("l2tpWANGW"), T(""));

		l2tpdns1 = websGetVar(wp, T("pptpDNS1"), T(""));
		l2tpdns2 = websGetVar(wp, T("pptpDNS2"), T(""));


		if(strcmp(l2tpmode,"1") == 0){
			if (CGI_same_net_with_lan(inet_addr(l2tpwip),inet_addr(l2tpwmask)))
			{
				SetErrorArg(LAN_WAN_IP_ERROR,"wan_connected.asp");
				websRedirect(wp, T("error.asp"));
				return;
			}
			if (strcmp(l2tpwip,l2tpwgw)==0||strcmp(l2tpwip,l2tpdns1)==0||strcmp(l2tpwip,l2tpdns2)==0)
			{
				SetErrorArg(WAN_IP_ERROR,"wan_connected.asp");
				websRedirect(wp, T("error.asp"));
				return;
			}
		}

		_SET_VALUE(_WAN0_l2TP_SERVER_NAM,l2tpip);
		_SET_VALUE(_WAN0_l2TP_USERNAME,l2tpuser);
		_SET_VALUE(_WAN0_l2TP_PASSWD,l2tppwd);

		_SET_VALUE(_WAN0_l2TP_MTU,l2tpmtu);
		_SET_VALUE(_WAN0_l2TP_MRU,l2tpmtu);//important for mtu

		_SET_VALUE(_WAN0_l2TP_STATIC,l2tpmode);
		
		if(strcmp(l2tpmode,"1") == 0){
			_SET_VALUE(_WAN0_l2TP_IPADDR,l2tpwip);
			_SET_VALUE(_WAN0_l2TP_NETMASK,l2tpwmask);
			_SET_VALUE(_WAN0_l2TP_GATEWAY,l2tpwgw);
			sprintf(l2tpdns,"%s %s",l2tpdns1,l2tpdns2);
			 _SET_VALUE(_WAN0_PPTP_DNS,l2tpdns);
		}

	}
	// 1x remain added	
	_SET_VALUE("err_check","0");
	set_wan_str(wanmod);
	if(strcmp(mac,"NULL") != 0)
		_SET_VALUE(_WAN0_HWADDR,mac);

#if 0//huangxiaoli modify	
	set_wan_mac(wp->ipaddr);
#endif
	_SET_VALUE("config_index","1");

	_SET_VALUE("wan0_check","0");
	//_SET_VALUE("onlncheck","no");
	_SET_VALUE("mode_need_switch","no");
	_COMMIT();
	_RESTART_ALL();
	
	//cyg_thread_delay(100);
//#ifdef __CONFIG_TENDA_MULTI__
	websRedirect(wp, T("/system_status.asp"));
/*#else
	websRedirect(wp, T("/wan_connected.asp"));
#endif*/
}

static void 	fromAdvSetMacClone(webs_t wp, char_t *path, char_t *query)
{
	char_t  *mac, *go,*way;

	mac = websGetVar(wp, T("WMAC"), T("")); 
	go = websGetVar(wp, T("GO"), T(""));
	way= websGetVar(wp, T("autoenable"), T("0"));
	_SET_VALUE(_WAN0_HWADDR,mac);
	_SET_VALUE("clnway",way);
	_SET_VALUE("auto_clone_flag","no");
	_COMMIT();
	
	_RESTART_ALL();

	websRedirect(wp, go);
}


static void 	fromAdvSetDns(webs_t wp, char_t *path, char_t *query)
{
	char_t  *dns1, *dns2, *enable, *go; 
	char_t *rebootFlag;
	char_t *wan_type;
	char_t dns3[40];
	
	rebootFlag = websGetVar(wp, T("rebootTag"), T("1"));

	dns1 = websGetVar(wp, T("DS1"), T("")); 
	dns2 = websGetVar(wp, T("DS2"), T("")); 

	enable = websGetVar(wp, T("DSEN"), T("0")); 

	go = websGetVar(wp, T("GO"), T("")); 

	_GET_VALUE(_WAN0_PROTO,wan_type);
	_SET_VALUE(_WAN0_DNS_FIX,enable);
	
	if(strcmp(enable, "1") == 0)
	{
		sprintf(dns3,"%s %s",dns1,dns2);
		_SET_VALUE(_WAN0_DNS,dns3);
	}
	_COMMIT();
	
	_RESTART();

	websRedirect(wp, T("/wan_dns.asp"));	
}

static void fromVirSerDMZ(webs_t wp, char_t *path, char_t *query)
{
	char_t  *dmzip, *go, *en;
	go = websGetVar(wp, T("GO"), T("")); 
	dmzip = websGetVar(wp, T("dmzip"), T("")); 
	en = websGetVar(wp, T("en"), T("0")); 
		
	_SET_VALUE(_FW_DMZ_IPADDR_EN,en);
	_SET_VALUE(_FW_DMZ_IPADDR,dmzip);

	_COMMIT();

	_RESTART();

	websRedirect(wp, T("/nat_dmz.asp"));	
}

static void fromVirSerUpnp(webs_t wp, char_t *path, char_t *query)
{
	char_t  *go, *upnpstatu;
	char_t  *val;

	go = websGetVar(wp, T("GO"), T("0")); 
 
	upnpstatu = websGetVar(wp, T("UpnpStatus"), T("0"));  

	_SAFE_GET_VALUE(_FW_UPNP_EN,val);

	if(strncmp(val,upnpstatu,1) != 0){
		_SET_VALUE(_FW_UPNP_EN,upnpstatu);
		_COMMIT();

		_RESTART();
		
		websRedirect(wp, T("/upnp_config.asp"));	
	}else{
		websRedirect(wp, T("/upnp_config.asp"));
	}	
}

static void fromVirSerSeg(webs_t wp, char_t *path, char_t *query)
{
	char name[32] ={'\0'};
	int i = 0;
	char_t  *stritem;
	char oldvalue[64],newvalue[64];

	for(i = 0;i < VTS_MAX_NUM_1;i ++)
	{
		sprintf(name,"PL%d",i + 1);
		stritem = websGetVar(wp,T(name),T(""));
		if(strlen(stritem) <5/*simple check*/){
			
			_SET_VALUE(_FW_FORWARD_PORT(i),"");	
			continue;
		}
		strcpy(oldvalue,stritem);

		if(!parse_webstr2portforward(oldvalue, newvalue))
			continue;

		//diag_printf("=dfdf=%s==%s==[%s]\n",__FUNCTION__,newvalue,_FW_FORWARD_PORT(i));

		_SET_VALUE(_FW_FORWARD_PORT(i),newvalue);			
	}

	_COMMIT();

	_RESTART();

	websRedirect(wp, T("/nat_virtualportseg.asp"));
}

extern void update_application_time(void);

static void fromSysToolTime(webs_t wp, char_t *path, char_t *query)
{
	char_t  *timezone,*timezonesel,*go,*mode,*manual;
	int manual_time;

	go = websGetVar(wp, T("GO"), T("")); 
	timezone = websGetVar(wp, T("TZ"), T("57"));//57:beijing 
	timezonesel = websGetVar(wp, T("TZSel"), T("")); 
	mode = websGetVar(wp,T("manualEN"),T("0"));//0:sntp mode
	manual = websGetVar(wp,T("time"),T("1275384461"));
	
	_SET_VALUE(_SYS_NTPTYPE,mode);

	if(atoi(mode)){
		//by hand
		_COMMIT();

		manual_time = strtol(manual, NULL, 10);
			
		cyg_libc_time_settime(manual_time);

		update_application_time();
		
		websRedirect(wp, T("/system_hostname.asp"));

	}else{
		_SET_VALUE(_SYS_TZONE,timezone);

		_COMMIT();

		_RESTART();

		websRedirect(wp, T("/system_hostname.asp"));	
	}
}

static void fromalgform(webs_t wp, char_t *path, char_t *query)
{

}

static void 	fromDhcpSetSer(webs_t wp, char_t *path, char_t *query)
{
	char_t *dhen=NULL,*dips=NULL,*dipe=NULL,*dhlt=NULL,*go=NULL;
	go = websGetVar(wp, T("GO"), T("")); 

	dhen = websGetVar(wp,T("dhcpEn"),T("0"));
	dips = websGetVar(wp,T("dips"),T(""));
	dipe = websGetVar(wp,T("dipe"),T(""));
	dhlt = websGetVar(wp,T("DHLT"),T("3600"));

	set_dhcpd_en(atoi(dhen));

	_SET_VALUE(_LAN0_DHCPD_START,dips);
	_SET_VALUE(_LAN0_DHCPD_END,dipe);
	_SET_VALUE(_LAN0_DHCPD_LEASET,dhlt);

	_COMMIT();
	
	_RESTART();
	
	
	websRedirect(wp, T("/lan_dhcps.asp"));
}

//lan_dhcp_clients.asp&list1='192.168.0.200;22:33:44:55:66:77'&list2='192.168.0.150;24:33:44:55:66:77'&IpMacEN=11&LISTLEN=2
static void fromDhcpListClient(webs_t wp, char_t *path, char_t *query)
{
	char_t	*lease_str=NULL, *listcnt=NULL;
	int i = 0, cnt;
	char strlist[32];	
	
	listcnt = websGetVar(wp, T("LISTLEN"), T("0")); 
	cnt = atoi(listcnt);

	for(i=1; i<=cnt; i++)
	{	
		memset(strlist, 0, sizeof(strlist));
		sprintf(strlist,"%s%d","list",i);
		lease_str = websGetVar(wp, T(strlist), T(""));
		if(lease_str == NULL || lease_str[0] == '\0' || strlen(lease_str) <5)
			break;
		printf("%s:%s\n",LAN0_DHCP_SATIC(i),lease_str);
		_SET_VALUE(LAN0_DHCP_SATIC(i),lease_str);
	}

	for(i = cnt+1; i<= DHCPD_STATIC_LEASE_NU; i++){
		_SET_VALUE(LAN0_DHCP_SATIC(i),"");
	}
	
	_COMMIT();
	
	_RESTART();

	websRedirect(wp, T("/lan_dhcp_clients.asp"));
}

static void fromSysToolChangePwd(webs_t wp, char_t *path, char_t *query)
{
	char_t  *old_pwd=NULL, *new_pwd1=NULL,
		*new_pwd2=NULL , *pwd_tip=NULL, *go=NULL;

	go = websGetVar(wp, T("GO"), T("")); 
//	user_id = websGetVar(wp, T("SYSUN"), T("")); 
	old_pwd = websGetVar(wp, T("SYSOPS"), T("")); 
	new_pwd1 = websGetVar(wp, T("SYSPS"), T("")); 
	new_pwd2 = websGetVar(wp, T("SYSPS2"), T("")); 
 	pwd_tip = websGetVar(wp, T("SYSPSTIP"), T("")); 
	
	if(strcmp(g_Pass,old_pwd)!=0) 
	{
		SetErrorArg(PASSWORD_ERROR,"system_password.asp");
		websRedirect(wp, T("error.asp"));
	} else
	{
	//	_SET_VALUE(HTTP_USERNAME, user_id);
		_SET_VALUE(HTTP_PASSWD, new_pwd1);
		_SET_VALUE(HTTP_PASSWD_TIP, pwd_tip);
		_COMMIT();
	//	strncpy(g_User,user_id,sizeof(g_User));
		strncpy(g_Pass,new_pwd1,sizeof(g_Pass));

		websRedirect(wp,T("system_password.asp"));
	}
}


static void fromSysToolReboot(webs_t wp, char_t *path, char_t *query)
{
	sys_reboot();
}


void _envram_set_value(char *name, char *value)
{
	char *argv[3];
	char cmdbuf[256];
	
	memset(argv,0,sizeof(argv));
	memset(cmdbuf,0,sizeof(cmdbuf));

	if (strlen(name)>0 && strlen(value)>0)
	{
		sprintf(cmdbuf, "%s=%s",name, value);

		argv[2] = cmdbuf;

		envram_set(3, argv);
	}
}

void pre_deal_when_restore(void)
{
	//add wl default interference value,2014.10.27
	_SET_VALUE("sb/1/interference","2");
	_SET_VALUE("sgi_tx","1");
	//end add

#ifdef A5
	_SET_VALUE("vlan1ports", "1 2 3 4 5*");
	_SET_VALUE("vlan2ports", "0 5");
#endif 

#ifdef NEDD_PRE_DEAL_WL_PA		
	/*wifi PA  for NH325 */
	if (!nvram_match("wlpa_had_reset","1"))
	{
		printf("Reset wl pa before restore!!\n");
		//for envram
		_envram_set_value("sb/1/boardflags", "0x710");
		_envram_set_value("sb/1/pdetrange2g", "0x3");
		_envram_set_value("sb/1/extpagain2g", "0x3");
		_envram_set_value("sb/1/maxp2ga0"	, "0x5e");
		_envram_set_value("sb/1/maxp2ga1"	, "0x5e");
		_envram_set_value("sb/1/mcs2gpo0", "0x6543");
		_envram_set_value("sb/1/mcs2gpo1", "0xa987");
		_envram_set_value("sb/1/mcs2gpo2", "0xa987");
		_envram_set_value("sb/1/mcs2gpo3", "0xedcb");
		_envram_set_value("sb/1/mcs2gpo4", "0x6543");
		_envram_set_value("sb/1/mcs2gpo5", "0xa987");
		_envram_set_value("sb/1/mcs2gpo6", "0xa987");
		_envram_set_value("sb/1/mcs2gpo7", "0xedcb");
		_envram_set_value("sb/1/ofdm2gpo", "0xa9876543");
		_envram_set_value("sb/1/sb/1/cck2gpo", "0x1100");
		
		_envram_set_value("sb/1/pa2gw2a1", "0xf965");
		_envram_set_value("sb/1/pa2gw1a1", "0x1b4a");
		_envram_set_value("sb/1/pa2gw0a1", "0xfe67");
		
		_envram_set_value("sb/1/pa2gw0a0", "0xfe67");
		_envram_set_value("sb/1/pa2gw1a0", "0x1b19");
		_envram_set_value("sb/1/pa2gw2a0", "0xf979");	

		_envram_set_value("wlpa_had_reset","1");
		
		envram_commit(0, NULL);

		cyg_thread_delay(10);//sleep 100ms
		
		//for nvram
		_SET_VALUE("sb/1/boardflags", "0x710");
		_SET_VALUE("sb/1/pdetrange2g", "0x3");
		_SET_VALUE("sb/1/extpagain2g", "0x3");
		_SET_VALUE("sb/1/maxp2ga0"	, "0x5e");
		_SET_VALUE("sb/1/maxp2ga1"	, "0x5e");
		_SET_VALUE("sb/1/mcs2gpo0", "0x6543");
		_SET_VALUE("sb/1/mcs2gpo1", "0xa987");
		_SET_VALUE("sb/1/mcs2gpo2", "0xa987");
		_SET_VALUE("sb/1/mcs2gpo3", "0xedcb");
		_SET_VALUE("sb/1/mcs2gpo4", "0x6543");
		_SET_VALUE("sb/1/mcs2gpo5", "0xa987");
		_SET_VALUE("sb/1/mcs2gpo6", "0xa987");
		_SET_VALUE("sb/1/mcs2gpo7", "0xedcb");
		_SET_VALUE("sb/1/ofdm2gpo", "0xa9876543");
		_SET_VALUE("sb/1/sb/1/cck2gpo", "0x1100");
		
		_SET_VALUE("sb/1/pa2gw2a1", "0xf965");
		_SET_VALUE("sb/1/pa2gw1a1", "0x1b4a");
		_SET_VALUE("sb/1/pa2gw0a1", "0xfe67");
		
		_SET_VALUE("sb/1/pa2gw0a0", "0xfe67");
		_SET_VALUE("sb/1/pa2gw1a0", "0x1b19");
		_SET_VALUE("sb/1/pa2gw2a0", "0xf979");

		_SET_VALUE("wlpa_had_reset","1");
		/* End*/
	}
	else
	{
		printf("==>Not need reset wl pa again!!\n");
	}
#endif

	return;
}


/*check wpspin is reasonable*/

static void fromSysToolRestoreSet(webs_t wp, char_t *path, char_t *query)
{
#if 0
	char *v = NULL;
	_SET_VALUE(_RESTORE_DEFAULTS,"1");
	_SET_VALUE(_LAN0_IP,_GET_DEFAULT(_LAN0_IP,v));

	pre_deal_when_restore();

	_COMMIT();
#else
	nvram_erase();
#endif
	
	websRedirect(wp, T("/direct_reboot.asp"));

	sys_reboot();
	
	
}

void  reset_button(void){
#if 0
	//char *v = NULL;
	_SET_VALUE(_RESTORE_DEFAULTS,"1");

	pre_deal_when_restore();
	
	_COMMIT();
#else
	nvram_erase();
#endif
	
	diag_printf("%s\n", __FUNCTION__);
	sys_reboot();
	return;
}

static void fromSysToolSysLog(webs_t wp, char_t *path, char_t *query)
{
	char_t *go=NULL,*optType=NULL,*curpage=NULL;
	go = websGetVar(wp, T("GO"), T("")); 
	optType = websGetVar(wp,T("TYPE"),T(""));
	curpage = websGetVar(wp,T("curPage"),T(""));


	if(strcmp(optType,"0") == 0)
	{
		syslog_clear();
		logNum=0;
		gLog_cur_page = 1;
	}
	else if(strcmp(optType,"1") == 0)
	{
		int cur = atoi(curpage);
		if(0 == cur)
			cur = 1;
		
		gLog_cur_page = cur;
	}

	websRedirect(wp, T(go));
}

static void fromSysToolDDNS(webs_t wp, char_t *path, char_t *query)
{
	char_t  *go=NULL, *ddnsen=NULL, *userName=NULL, *password=NULL,
		*hostName=NULL, *serverName=NULL;
	char_t val[128];
	char_t *serName = NULL;
	
	go = websGetVar(wp, T("GO"), T("")); 

	ddnsen = websGetVar(wp, T("ddnsEnable"), T("0")); 
	userName = websGetVar(wp, T("userName"), T("")); 
	password = websGetVar(wp, T("password"), T("")); 
	hostName = websGetVar(wp, T("hostName"), T("")); 
	serverName = websGetVar(wp,T("serverName"),T("0"));

	memset(val,0,sizeof(val));
	if (atoi(ddnsen)) {
		if (atoi(serverName) == 0) {
			//oray.net
			_SET_VALUE(_DDNS_ENABLE,"1");
			serName = ORAY_NAME;
		} else if (atoi(serverName) == 1) {
			//dyndns.org
			_SET_VALUE(_DDNS_ENABLE,"2");
			serName = DYNDNS_NAME;
		} else if (atoi(serverName) == 2) {
			//88ip.com
			_SET_VALUE(_DDNS_ENABLE,"3");
			serName = M88IP_NAME;
		} else if (atoi(serverName) == 3) {
			//freedns.afraid.org
			_SET_VALUE(_DDNS_ENABLE,"4");
			serName = "afraid";
		} else if (atoi(serverName) == 4) {
			//zoneedit.com 
			_SET_VALUE(_DDNS_ENABLE,"5");
			serName = ZOE_NAME;
		} else if (atoi(serverName) == 5) {
			//no-ip.com
			_SET_VALUE(_DDNS_ENABLE,"6");
			serName = NOIP_NAME;
		} else if (atoi(serverName) == 6) {
			//3322.org
			_SET_VALUE(_DDNS_ENABLE,"7");
			serName = QDNS_NAME;
		} else if (atoi(serverName) == 7) {
			//gnhostlinux
			_SET_VALUE(_DDNS_ENABLE,"8");
			serName = "gnhostlinux";
		} 

		snprintf(val,sizeof(val),"%s;%s;%s;%s;%s;%s;%s;%s;%s",
			"1",serName,hostName,userName,password,"","","","60"
		);
		_SET_VALUE(_DDNS_SET1,val);
		_SET_VALUE(_DDNS_HOST_NAME,hostName);		
		_SET_VALUE(_DDNS_ISP,serverName);
		_SET_VALUE(_DDNS_ENABLE,"1");
	} else {
		_SET_VALUE(_DDNS_ENABLE,"0");
	}

	_COMMIT();

	_RESTART();
	
	websRedirect(wp, T("/ddns_config.asp"));	
}

//192.168.5.0:255.255.255.0:192.168.5.1:1 192.168.6.0:255.255.255.0:192.168.6.1:1
static void fromRouteStatic(webs_t wp, char_t *path, char_t *query)
{
	char_t  *go, *str, *cmd;
	char *oldvalue;

	go = websGetVar(wp, T("GO"), T("")); 
	cmd = websGetVar(wp, T("cmd"), T(""));

	str= websGetVar(wp, T("wan0_route"), T("")); 

	_SAFE_GET_VALUE(_WAN0_ROUTE,oldvalue);

	_SET_VALUE(_LAN0_ROUTE,oldvalue);

	_SET_VALUE(_WAN0_ROUTE,str);

	_COMMIT();

	_RESTART();

	websRedirect(wp, T("/routing_static.asp"));	
}

int quit_redirect_web(webs_t wp,char *url)
{
	if(NULL == url)
	{
		return -1;
	}
		
	if(1 == nvram_match(_RESTORE_QUICK_SET,"1"))
	{
		if(0 == strncmp(url, "/userProtocal.html", strlen("/userProtocal.html")))
			return 1;
		if(0 != strcmp(url,"/quickset.html") && 0 != strcmp(url,"/goform/getWizard") && 0 != strcmp(url,"/goform/setWizard"))
		{
			websRedirect(wp, T("/quickset.html")); 
			return 1;
		}
	}
	else
	{
		if(0 == strcmp(url,"/quickset.html"))
		{
			websRedirect(wp, T("/"));
			return 1;
		}
	}	
	
	return 0;
}


#if 0
int TWL300NWebsSecurityHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
						char_t *url, char_t *path, char_t *query)
{
	char_t			*userid=NULL, *password=NULL;
	//char_t			*wwsuserid=NULL, *wwspassword=NULL;
	char_t 			*value;
	
	char bReLogin;

	static clock_t save_security_time = 0;
	clock_t security_time_val;

	a_assert(websValid(wp));
	a_assert(url && *url);
	a_assert(path && *path);

	if(g_user_pass_Flag == 0)
	{
		memset(g_User,0,sizeof(g_User));
		memset(g_Pass,0,sizeof(g_Pass));

		strncpy( g_User,_SAFE_GET_VALUE(HTTP_USERNAME,value),sizeof(g_User));
		strncpy( g_Pass,_SAFE_GET_VALUE(HTTP_PASSWD,value),sizeof(g_Pass));

		g_user_pass_Flag = 1;
	}

	password = websGetRequestPassword(wp);
	userid = websGetRequestUserName(wp);

	security_time_val = time(0) - save_security_time;
	if (security_time_val > 2 * 60 * 100 )
		bReLogin = 1;
	else
		bReLogin = 0;

	if (userid == NULL)
	{
			goto ERROREXIT;
	} else
	{
		if ((gstrcmp(userid,g_User) != 0) || (bReLogin)  || (bForceLogout))
		{
			goto ERROREXIT;
		}
	}

	if (password == NULL)
	{
		goto ERROREXIT;
	} else
	{
		if ((gstrcmp(password,g_Pass) != 0) || (bReLogin) || (bForceLogout))
		{
			goto ERROREXIT;
		}
	}

	save_security_time = time(0);
	bForceLogout = 0;
	return 0;

ERROREXIT:
	save_security_time = time(0);
	bForceLogout = 0;
 

	websResponse(wp, 401, NULL, NULL);
	return 0;
}
#else
extern int dns_redirect_disable;
extern int dns_redirect_web;

#define PUBLIC_FOLDER "/public"

//hqw add
const char cookie_suffix[20][4] = {"1qw","ded","ert","fcv","vgf","nrg","xcb","jku","cvd","wdv","2dw","njk","ftb","efv","azx","tuo","cvb","bcx","eee","mfw"};
//end

int TWL300NWebsSecurityByCookieHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
						char_t *url, char_t *path, char_t *query)
{
	char *userid = NULL, *password = NULL;
	char *pfiletype = NULL;
	char *purlbuf = NULL;
	char *pcookie = NULL;
	int i = 0, k = 0;
	
	char *p_user_cookie,*p_cookie_tmp, *value;;
	//char user_cookie[512]={0};
	char urlbuf[256] = {0};
	char lan_ip[20]={0};
	char wan_ip[20]={0};
	char default_Pwd[64];

//	char lan_pppoe[64]={0}, wan_pppoe[64]={0};
	char lan_wlpwd[64]={0}, wan_wlpwd[64]={0};	
	char lan_index[64]={0}, wan_index[64]={0};
//#ifdef __CONFIG_QUICK_SET__
	char lan_quickset[64]={0};//add liuchengchi
//#endif	
	char lan_login[64]={0};
	char wan_login[64]={0}; 
	char lan_login_error[64]={0};
	char wan_login_error[64]={0};

	char http_true_passwd[256] = {0};

	a_assert(websValid(wp));
	a_assert(url && *url);
	a_assert(path && *path);

#ifdef __CONFIG_ALINKGW__
	if(strstr(url , "warning") || strstr(url , "InsertWhite"))
	{
		return 0 ;
	}

#endif

	/*(add by chenwb,2012-1-11)  */
	if(strncmp(url,PUBLIC_FOLDER,strlen(PUBLIC_FOLDER))==0
		|| strcmp(url,"/favicon.ico")==0 || wp->host_str == NULL) 
		return 0;

	if(g_user_pass_Flag == 0)
	{
		memset(g_User,0,sizeof(g_User));
		memset(g_Pass,0,sizeof(g_Pass));

		strncpy(g_User,_SAFE_GET_VALUE(HTTP_USERNAME,value),sizeof(g_User));
		strncpy(g_Pass,_SAFE_GET_VALUE(HTTP_PASSWD,value),sizeof(g_Pass));

		g_user_pass_Flag = 1;
	}
	strncpy(default_Pwd,_SAFE_GET_VALUE(HTTP_DEFAULTPWD,value),sizeof(default_Pwd));
	
	memcpy(urlbuf, url, 255);	

	if ((purlbuf = strchr(urlbuf, '?')) != NULL)
		*purlbuf = '\0';
#if 0
	if(wp->cookie)
	{
		k = 0;
		p_cookie_tmp = wp->cookie;
		while( k<32 && *p_cookie_tmp && *p_cookie_tmp!=':' )
		{	 
			user_cookie[k] = *p_cookie_tmp;
			if(user_cookie[k] == 0x20)
				k = 0;
			else
				k++;
			p_cookie_tmp++;
		}
		user_cookie[k] = '\0' ;
		pcookie = user_cookie;
	}
#else
	if(wp->cookie)
	{
		p_user_cookie = wp->cookie;
		while ((p_cookie_tmp = strstr(p_user_cookie,"ecos_pw="))!=NULL)
		{
			pcookie = p_cookie_tmp + strlen("ecos_pw=");
			p_user_cookie = pcookie;
		}
	}
#endif

	if (strlen(urlbuf)>3)
	{
		pfiletype = strrchr(urlbuf, '.');
		if (pfiletype)
		{
			pfiletype++;
			if (!memcmp(pfiletype, "gif", 3)||!memcmp(pfiletype, "js", 2)||!memcmp(pfiletype, "css", 3)||!memcmp(pfiletype, "png", 3) ||!memcmp(pfiletype, "jpg", 3)||!memcmp(pfiletype, "xml", 3)||!memcmp(pfiletype, "ico", 3)||!memcmp(pfiletype, "ttf", 3)||!memcmp(pfiletype, "woff", 4)||!memcmp(pfiletype, "eot", 3))
				
				return 0;
		}
	}


    if ( g_Pass!=NULL && (strcmp(g_Pass,default_Pwd) == 0) && wp->url!=NULL && (strstr(wp->url,"/goform/ate")!=NULL) )
    	 return 0;

#ifdef __CONFIG_APCLIENT_DHCPC__
	char apclient_lan_ip[20] = {0},apclient_lan_mask[20] = {0};
	if(0 == strcmp(wp->host_str,"www.tendawifi.com") || 0 == strcmp(wp->host_str,"tendawifi.com"))
	{
		strncpy(lan_ip,wp->host_str,sizeof(lan_ip));
		strncpy(wan_ip,wp->host_str,sizeof(wan_ip));
	}
	else if(1 == gpi_get_apclient_dhcpc_enable())
	{
		/*当APCLIENT模式下应为要从上级获取IP，故使用的是br0上面的IP而不是MIB值里面的LAN口IP值*/
		strncpy(lan_ip,NSTR(SYS_lan_ip),sizeof(lan_ip));
		strncpy(wan_ip,NSTR(SYS_wan_ip),sizeof(wan_ip));
		if(PI_SUC == gpi_get_apclient_dhcpc_addr(apclient_lan_ip,apclient_lan_mask))
		{
			if(0 == strcmp(apclient_lan_ip,"") || 0 == strcmp(apclient_lan_ip,"0.0.0.0")
				|| 0 == strcmp(apclient_lan_mask,"") || 0 == strcmp(apclient_lan_mask,"0.0.0.0"))
			{
				;
			}
			else
			{
				strncpy(lan_ip,apclient_lan_ip,sizeof(lan_ip));				
			}
		}
	}
	else
#endif
	{	
		strncpy(lan_ip,NSTR(SYS_lan_ip),sizeof(lan_ip));
		strncpy(wan_ip,NSTR(SYS_wan_ip),sizeof(wan_ip));
	}

	snprintf(lan_login,sizeof(lan_login),"http://%s/login.asp",lan_ip); 
	snprintf(wan_login,sizeof(wan_login),"http://%s/login.asp",wp->host_str);
	
	snprintf(lan_login_error,sizeof(lan_login_error),"http://%s/login.asp?0",lan_ip);
	snprintf(wan_login_error,sizeof(wan_login_error),"http://%s/login.asp?0",wp->host_str);
	
//	snprintf(lan_pppoe,sizeof(lan_pppoe),"http://%s/redirect_set_pppoe.asp",lan_ip);
	snprintf(lan_wlpwd,sizeof(lan_wlpwd),"http://%s/redirect_set_wlsec.asp",lan_ip);
	snprintf(lan_index,sizeof(lan_index),"http://%s/index.htm",lan_ip);

//#ifdef __CONFIG_QUICK_SET__	
	snprintf(lan_quickset,sizeof(lan_quickset),"http://%s/quickset.html",lan_ip);//add liuchengchi
//#endif
//	snprintf(wan_pppoe,sizeof(wan_pppoe),"http://%s/redirect_set_pppoe.asp",wp->host_str);
	snprintf(wan_wlpwd,sizeof(wan_wlpwd),"http://%s/redirect_set_wlsec.asp",wp->host_str);
	snprintf(wan_index,sizeof(wan_index),"http://%s/index.htm",wp->host_str);

	if(1 == quit_redirect_web(wp,urlbuf))
	{
		return 0;
	}
	
	for (i=0; i<MAX_USER_NUM; i++)
	{
		if(!strcmp(loginUserInfo[i].ip, wp->ipaddr)) 
		{
			break;
		}
	}
	
	if(i<MAX_USER_NUM)	
	{
	//	printf("Old User:%s, wp->host_str=%s,url=%s \n",wp->ipaddr, wp->host_str,url);
		
		/*neither lan nor wan*/
		if (strncmp(wp->host_str,lan_ip,strlen(lan_ip))!=0 && strncmp(wp->host_str,wan_ip,strlen(wan_ip))!=0)
		{
			if(dns_redirect_web == 1){
				websResponse(wp,302,NULL,T(lan_wlpwd));
				dns_redirect_web = -1;
			}else{
				websResponse(wp,302,NULL,T(lan_login));
			}
			return 0;
		}
		
		if(strcmp(g_Pass, "") == 0){
			if((!memcmp(urlbuf, "/login.asp", 10) || (strlen(urlbuf)==1&&urlbuf[0]=='/')) || (!memcmp(urlbuf, "/LoginCheck", 11)))
			{
				goto LOGINOK;
			} 
			else
			{
				//printf("1\n");
				loginUserInfo[i].time = (unsigned int)cyg_current_time();
				return 0;
			}
		}

		if ( wp->ipaddr!=NULL)
		{
			sprintf(http_true_passwd,"%s%s",g_Pass,cookie_suffix[((unsigned int)inet_addr(wp->ipaddr))%20]);
		}
		else
		{
			sprintf(http_true_passwd,"%s",g_Pass);
		}

		if(pcookie && !strncmp(pcookie, http_true_passwd,strlen(http_true_passwd))){
			if(!memcmp(urlbuf, "/login.asp", 10) || (strlen(urlbuf)==1 && urlbuf[0]=='/') || !memcmp(urlbuf, "/LoginCheck", 11))
			{
				goto LOGINOK;
			}

			else if((!memcmp(urlbuf, "/goform/NetWorkSetupInit", strlen("/goform/NetWorkSetupInit")))
						||(!memcmp(urlbuf, "/goform/SpeedControlInit", strlen("/goform/SpeedControlInit")))
							||(!memcmp(urlbuf, "/goform/WirelessRepeatInit", strlen("/goform/WirelessRepeatInit")))
								||(!memcmp(urlbuf, "/goform/WirelessRepeatApInit", strlen("/goform/WirelessRepeatApInit")))
									||(!memcmp(urlbuf, "/goform/SystemManageInit", strlen("/goform/SystemManageInit"))))
			{
				return 0;
			}			
			else
			{
				loginUserInfo[i].time =(unsigned int)cyg_current_time();
				return 0;
			}	
		}
		else{
			if(strstr(wp->url,"login")!=NULL || !memcmp(urlbuf, "/LoginCheck", 11) 
				|| !memcmp(urlbuf, "/redirect_set_wlsec", 18)|| !memcmp(urlbuf, "/goform/redirectSetwlSecurity", 29))
			{
				goto RELOGIN;
			}
			else{
				websResponse(wp,302,NULL,T(lan_login));
				//printf("3\n");				
				loginUserInfo[i].time = (unsigned int)cyg_current_time();
				return 0;
			}
		}
	} 
	else	
	{
	//	printf("New User:%s, wp->host_str=%s,url=%s \n",wp->ipaddr, wp->host_str,url);
		
		/*neither lan nor wan*/
		if ( strncmp(wp->host_str,lan_ip,strlen(lan_ip))!=0 && strncmp(wp->host_str,wan_ip,strlen(wan_ip))!=0 )
		{
			if(dns_redirect_web == 1){
				websResponse(wp,302,NULL,T(lan_wlpwd));
				dns_redirect_web = -1;
			}else{
				websResponse(wp,302,NULL,T(lan_login));
			}
			return 0;
		}
		
		if(strcmp(g_Pass, "") == 0){
			if ((!memcmp(urlbuf, "/error.asp", 10)))
				return 0;
			
			goto NOPASSWORD;
		}		

RELOGIN:
		if (strlen(urlbuf)==1&&urlbuf[0]=='/')
			goto ERROREXIT;		
		else if (!memcmp(urlbuf, "/login.asp", 10))
			return 0;
		else if (!memcmp(urlbuf, "/error.asp", 10))
			return 0;
	/*	else if (!memcmp(urlbuf, "/redirect_set_wlsec.asp", 23))
			return 0;
		else if (!memcmp(urlbuf, "/goform/redirectSetwlSecurity", 29))
			return 0;	*/
		else if (!memcmp(urlbuf, "/LoginCheck", 11))
		{
			password = websGetVar(wp, T("Password"), T(""));
			userid = websGetVar(wp, T("Username"), T(""));
			
			//printf("userid=%s, g_User=%s, password=%s, g_Pass=%s\n",userid,g_User,password,g_Pass);

			if (!strcmp(userid, g_User) && !strcmp(password, g_Pass))
			{
				for(i=0; i<MAX_USER_NUM; i++)
				{
				//	printf("loginUserInfo[%d].ip == %s\n",i,loginUserInfo[i].ip);
					if (strlen(loginUserInfo[i].ip) == 0 || !strcmp(loginUserInfo[i].ip, wp->ipaddr))
					{
						memcpy(loginUserInfo[i].ip , wp->ipaddr, IP_SIZE);			
						//printf("4\n");
						
						loginUserInfo[i].time = (unsigned int)cyg_current_time();
						goto LOGINOK;
						break;
						
					}
				}
				if(i == MAX_USER_NUM) 
				{
					SetErrorArg(LOGIN_USERS_FULL,"login.asp");
					websRedirect(wp, T("error.asp"));
				//	printf("up to 4 people!\n");
					return 0;
				}
			}
			else
			{
				//printf("go LOGINERR \n");
				goto LOGINERR;
			}		
		}
		else{
			goto ERROREXIT;
		}
	}
	return 0;
	
ERROREXIT:
	
	if(strncmp(wp->host_str,wan_ip,strlen(wan_ip))==0){
		/*remote access router*/
		websRedirect(wp, T(wan_login));
	}
	else{
		/*local access router*/
		websRedirect(wp, T(lan_login));
	}
	return 0;

LOGINOK:
	//当第一次联网的时候，重定向到无线设置页面在这里，但是由于输入192.168.0.1该标志也置为1，导致也进入设置无线密码的地方
	if (dns_redirect_web == 1 && 0 != strcmp(wp->host_str,NSTR(SYS_lan_ip))){
		websResponseWithCookie(wp,302,NULL,T(lan_wlpwd), 1);
		dns_redirect_web = -1;
	}
	else{
		if(strncmp(wp->host_str,wan_ip,strlen(wan_ip))==0){
			/*remote access router*/
			websResponseWithCookie(wp,302,NULL,T(wan_index), 1);
		}
		else{
			/*local access router*/
#if 1//def __CONFIG_QUICK_SET__			
			//add liuchengchi 2014-11-11
			char *restore_quick_set;
			_GET_VALUE(_RESTORE_QUICK_SET,restore_quick_set);
			
			if(atoi(restore_quick_set) == 1){
				/*if((strcmp(nvram_safe_get("mode_need_switch"),"yes") != 0 
			||strcmp(nvram_safe_get("wl0_mode"),"ap") != 0 
				|| strcmp(nvram_safe_get("wl0_wds"),"") != 0) && network_tpye >= 3 ){
				
					websResponseWithCookie(wp,302,NULL,T(lan_index), 1);
				}else*/{
					websResponseWithCookie(wp,302,NULL,T(lan_quickset), 1);
				}

			}else{			
					websResponseWithCookie(wp,302,NULL,T(lan_index), 1);
			}
#else
			websResponseWithCookie(wp,302,NULL,T(lan_index), 1);
#endif
			
		}
	}
	//printf("Login OK !!!\n");
	return 0;
	
LOGINERR:
	if(strncmp(wp->host_str,wan_ip,strlen(wan_ip))==0)
		websRedirect(wp, T(wan_login_error));
	else
		websRedirect(wp, T(lan_login_error));
	//printf("Login ERROR !!!\n");
	return 0;
	
NOPASSWORD:
	if(strcmp(g_Pass, "") == 0)
	{
		for(i=0; i<MAX_USER_NUM; i++)
		{
			if (strlen(loginUserInfo[i].ip) == 0)
			{
				memcpy(loginUserInfo[i].ip , wp->ipaddr, IP_SIZE);				
				//printf("5\n");
				loginUserInfo[i].time = (unsigned int)cyg_current_time();
				//printf("NOPASSWORD: urlbuf=%s\n",urlbuf);

				if((!memcmp(urlbuf, "/login.asp", 10) || (strlen(urlbuf)==1&&urlbuf[0]=='/')) || (!memcmp(urlbuf, "/LoginCheck", 11)))
				{
					//printf("NOPASSWORD: goto LOGINOK\n");
					goto LOGINOK;
				} 
				else
				{
					//printf("NOPASSWORD: return\n");
					//printf("6\n");
					loginUserInfo[i].time = (unsigned int)cyg_current_time();
					return 0;
				}	

				break;
				
			}	
		}

		if(i == MAX_USER_NUM) 
		{
			if(memcmp(urlbuf, "/prompt.asp", 10)!=0)
			{
				SetErrorArg(LOGIN_USERS_FULL,"prompt.asp");
				websRedirect(wp, T("error.asp"));
			}
		}
	}
	return 0;

}
#endif


typedef uint16 chanspec_t;
#define WLC_UP					2
#define WLC_DOWN				3
#define BCME_OK				0	/* Success */
#define WLC_GET_UP				162

extern void mfg_start(void);
static void 	fromSysAted(webs_t wp, char_t *path, char_t *query)
{
	mfg_start();
	websWrite(wp,T("load mfg success."));
	websDone(wp,200);
}

typedef struct
{
	uint8 ant_config[4];	/* antenna configuration */
	uint8 num_antcfg;	/* number of available antenna configurations */
} wlc_antselcfg_t;


#define SUCCESS  0
#define ERROR    -1
#define INFO     1
extern int envram_get(int argc, char* argv[]);
extern int envram_set(int argc, char* argv[]);
extern int envram_commit(int argc, char* argv[]);

static int mfg_set_value(const char *name, const char *value) 		
{	
    if( nvram_set(name, value) == 0 )
	{
		return SUCCESS;
	}
	return ERROR;
}
static char *mfg_get_value(char *name) 	        
{
    return nvram_safe_get(name);
}
static int mfg_commit() 		
{
    if( nvram_commit()==0 )//_COMMIT();
	{   
	    return SUCCESS;
	}
    return ERROR;
}
static void mfg_html_info(char *buf) 	
{
    char *lanmac,*wanmac,*ssid,*pin,*county_code,*maxpower,*sn;
	char cmdbuf[256],*argv[3];
	int power=0; 
    lanmac = mfg_get_value("et0macaddr");
	wanmac = mfg_get_value("wan0_hwaddr");
	ssid = mfg_get_value("wl_ssid");
	pin = mfg_get_value("wps_device_pin");
	sn =  mfg_get_value("device_sn");
	sprintf(cmdbuf, "%s", "country_code");
    argv[2] = cmdbuf;
    envram_get(3, argv);
	county_code=argv[2];

	sprintf(cmdbuf, "%s", "country_power");
    argv[2] = cmdbuf;
    envram_get(3, argv);
	maxpower=argv[2];

	if(0 == strcmp(maxpower,"low"))
	{           	
		power=0;
	}
	else if(0 == strcmp(maxpower,"normal"))
	{				
		power=1;
	}
	else if(0 == strcmp(maxpower,"high"))
	{
		power=2;						
	}
	  
    sprintf(buf,"SV=%s_%s;LANMAC=%s;WAN1MAC=%s;WAN2MAC="";SSID=%s;PIN=%s;WPA_PW="";COUNTRY=%s;MAXPOWER=%d;SN=%s;DATA=""",
	W311R_ECOS_SV,__CONFIG_WEB_VERSION__,lanmac,wanmac,ssid,pin,county_code,power,sn);
	return;
}
static void mfg_reboot()
{     
   sys_reboot();	
}

static int  wl_wps_pin_check(char *pin_string)
{
    unsigned long pin = strtoul(pin_string, NULL, 10);
    unsigned long int accum = 0;
    unsigned int len = strlen(pin_string);
    if (len != 4 && len != 8)
        return 	ERROR;
    if (len == 8)
    {
        accum += 3 * ((pin / 10000000) % 10);
        accum += 1 * ((pin / 1000000) % 10);
        accum += 3 * ((pin / 100000) % 10);
        accum += 1 * ((pin / 10000) % 10);
        accum += 3 * ((pin / 1000) % 10);
        accum += 1 * ((pin / 100) % 10);
        accum += 3 * ((pin / 10) % 10);
        accum += 1 * ((pin / 1) % 10);
        if (0 == (accum % 10))
            return SUCCESS;
    }
    else if (len == 4)
        return SUCCESS;
    return ERROR;
}//end PIN check


#define EFFECT_LEVEL 0 
#define WPS_RESET_BUTTON_GPIO 20
#define BUTTON2_GPIO 21

static int ate_button_poll()
{
	unsigned long btn_mask;	
	unsigned long value;
	unsigned int utimes = 0;
	int flag = 0;
	int gpio = 0;	

	for(utimes = 0; utimes < 2; utimes++)
	{	
		gpio = WPS_RESET_BUTTON_GPIO;
		bcmgpio_connect(gpio, 0);
		btn_mask = ((unsigned long)1 << gpio);		
		bcmgpio_in(btn_mask, &value);		
		value >>= gpio;			

		/* Other gpio Here. */

		if(value == EFFECT_LEVEL)
		{
			flag = 1;
			printf("\n---Button pressed!!!\n\n");	
			printf("ledtest  off \n");
            set_all_led_off();
			cyg_thread_delay(50); // 0.5 s
			printf("ledtest  on \n");
			set_all_led_on();	
			break;
		}
		
		if (nvram_match("btn2_status","WIFI_SWITCH")){	
			gpio = BUTTON2_GPIO;
			bcmgpio_connect(gpio, 0);
			btn_mask = ((unsigned long)1 << gpio);		
			bcmgpio_in(btn_mask, &value);		
			value >>= gpio;
			if(value == EFFECT_LEVEL)
			{
				flag = 1;
				printf("\n---Button pressed!!!\n\n");	
				printf("ledtest  off \n");
	            set_all_led_off();
				cyg_thread_delay(50); // 0.5 s
				printf("ledtest  on \n");
				set_all_led_on();	
				break;
			}
		}
		cyg_thread_delay(10); // 0.1 s
	}	

	return 0;
}


extern int mfg_reset_button_check_tag;
extern int mfg_wifi_button_check_tag;
extern int mfg_button_check_tag;
extern int mfg_mode;
extern mfg_button_hold_status s_mfg_button_hold_status;
void mfg_mainloop(void)
{
    int mfgfd;
    int ilen;
	struct sockaddr_in local;
	struct sockaddr remote;
    char recvbuf[1024], sendbuf[2048];
	int			argc;
	char *argv[32];
	//char *macaddr1=NULL;
	char *bootwait=NULL;
	char *waittime=NULL;
    char cmdbuf[256];
	
    struct ether_addr *hwaddr;
    unsigned char macaddr[32];
	fd_set rfds;
	struct timeval tv={2,0};
    int retval;
    int mfgflag;
	int	argc_wl;
	char *argv_wl[32];
	char get_buf[2048]={0};
	char wl_error[256]={0};

	mfg_reset_button_check_tag = 0;

	mfg_wifi_button_check_tag = 0;
	
	mfg_button_check_tag = 0;
    memset( &local, 0, sizeof(local) );         
    local.sin_family = AF_INET; 
	local.sin_port = htons(7329);               
    local.sin_addr.s_addr = SYS_lan_ip;         

    mfgfd = socket( AF_INET, SOCK_DGRAM, 0 );	
    if ( mfgfd < 0 )                           
    {
        diag_printf("MfgThread socket error.\n");
        return ;
    }
    if (bind(mfgfd, (struct sockaddr *)&local, sizeof(local)) < 0)  	 
    {
        diag_printf("MfgThread bind error.\n");
		close(mfgfd);//hqw add for tcp 2014.01.24
        return;
    }
    printf("MfgThread start loop.\n");
	mfg_mode = 1;
	s_mfg_button_hold_status.reset_button_hold = TRUE;
	s_mfg_button_hold_status.wifi_button_hold = TRUE;
    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(mfgfd, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 200 *1000; //0.2 second
        ate_button_poll();
		
        retval = select(mfgfd + 1, &rfds, NULL, NULL, &tv);

        if (retval <= 0 || !FD_ISSET(mfgfd, &rfds))
            continue;

        memset(recvbuf,0,sizeof(recvbuf));   
        ilen = sizeof(remote);
        ilen = recvfrom(mfgfd, recvbuf, sizeof(recvbuf), 0, &remote, &ilen); 

        if (ilen < 10)			    
            continue;
        if (ilen == 1024 )
            recvbuf[1024] = 0;

        printf("MfgThread recv %d[%s]\n",ilen,recvbuf);
		 
        memset(argv, 0, sizeof(argv));
        argc = get_argv(recvbuf, argv);

        mfgflag = ERROR;
	
        switch (argc)
        {
        case 2://Tenda_mfg
			if ( strcmp(argv[0], "Tenda_mfg")==0 )
			{
				if( strcmp(argv[1], "reboot")==0 )
	            {
	                 strcpy(sendbuf, "success");
	                 ilen= strlen(sendbuf);
	                 sendto(mfgfd, (char *)sendbuf, ilen, 0, (struct sockaddr *)&remote, sizeof(remote));
                     mfg_reboot();
	            }
	            else if( strcmp(argv[1], "default")==0 )
	            {			
					mfg_set_value(_RESTORE_DEFAULTS, "1");
					mfg_set_value(_LAN0_IP,mfg_get_value(_LAN0_IP));	

					pre_deal_when_restore();

					mfgflag = mfg_commit(); 
                    printf("default = %s ,mfgflag = %d\n",mfg_get_value(_RESTORE_DEFAULTS),mfgflag);
					sys_reboot();

	            }
	            else if ( strcmp(argv[1], "htmlVersionInfo")==0 )
	            {
                     mfg_html_info(cmdbuf);
                     mfgflag = INFO;
	            }
				
			}
                //start: add by z10312  
                else if(strcmp(argv[0], "ledtest")==0)
                {
                    if(strcmp(argv[1], "on")==0)
                    {
                        set_all_led_on();
                        mfgflag = SUCCESS;
                    }else if(strcmp(argv[1], "off")==0)
                    {
                        set_all_led_off();
                        mfgflag = SUCCESS;
                    }
                }
                //end: add by z10312  
            break;
        case 3://wlctrl
        	//Tenda_mfg check ResetButton
        	if (0 == strcmp(argv[0], "Tenda_mfg"))
        	{
        		if (0 == strcmp(argv[1], "Check")) 
        		{
					if(0 == strcmp(argv[2], "ResetButton"))
	        		{
	        			mfg_wifi_button_check_tag = 0;
						mfg_button_check_tag = 0;
						s_mfg_button_hold_status.reset_button_hold = TRUE;
	        			int i = 0;
						if (!nvram_match("btn2_status", "WIFI_SWITCH")){
		                    for(i = 0; i < 15 ; i++)
							{
								if (1 == mfg_button_check_tag)
								{
									break;
								}
								cyg_thread_delay(1*100);
							}

							if (1 == mfg_button_check_tag)
							{
								printf("ResetButton test OK\n");
								mfgflag = SUCCESS;
								mfg_button_check_tag = 0;
							}
							else
							{
								printf("ResetButton test ERROR\n");
								mfgflag = ERROR;
							}
						}else{
		                    for(i = 0; i < 15 ; i++)
							{
								if (1 == mfg_button_check_tag)
								{
									break;
								}
								cyg_thread_delay(1*100);
							}

							if (1 == mfg_button_check_tag)
							{
								printf("ResetButton test OK\n");
								mfgflag = SUCCESS;
								mfg_button_check_tag = 0;
							}
							else
							{
								printf("ResetButton test ERROR\n");
								mfgflag = ERROR;
							}

						}
						s_mfg_button_hold_status.reset_button_hold = FALSE;
	        		}
					else if(0 == strcmp(argv[2], "WiFiButton"))
	        		{
	        			mfg_wifi_button_check_tag = 0;
						s_mfg_button_hold_status.wifi_button_hold = TRUE;
	        			int j = 0;
	                    for(j = 0; j < 15 ; j++)
						{
							if (1 == mfg_wifi_button_check_tag)
							{
								break;
							}
							cyg_thread_delay(1*100);
						}

						if (1 == mfg_wifi_button_check_tag)
						{
							printf("WiFiButton test OK\n");
							mfgflag = SUCCESS;
							mfg_wifi_button_check_tag = 0;
						}
						else
						{
							printf("WiFiButton test ERROR\n");
							mfgflag = ERROR;
						}
						s_mfg_button_hold_status.wifi_button_hold = FALSE;
	        		}
        		}
        	}
            else if (strcmp(argv[0], "wlctrl") == 0)
            {
                if (strcmp(argv[1], "set_ant") == 0)
                {
                    int nget1, nget2;
                    int nset;
                    int nreturn;

                    wl_set("eth1", WLC_DOWN, &nset, sizeof(nset));
                    wl_get("eth1", WLC_GET_UP, &nget1, sizeof(nget1));
                    if (nget1 != 0)
                    {
                        printf("wl status is not [down] status,return false!!!\n");
                        mfgflag = ERROR;
                        break;
                    }
					
                    if (strcmp(argv[2], "0") == 0 || strcmp(argv[2], "1") == 0)
                    {
                        nset = atoi(argv[2]) + 1;

                        wl_iovar_setint("eth1", "txchain", nset);
                        wl_iovar_setint("eth1", "rxchain", nset);

                        wl_iovar_getint("eth1", "rxchain", &nget1);
                        wl_iovar_getint("eth1", "txchain", &nget2);
                        if ( nget1 == nset && nget2 == nset )
                            mfgflag = SUCCESS;
                        else
                            mfgflag = ERROR;

                        printf("wl txchain get:%d  rxchain get:%d,mfgflag value:%d\n", nget1, nget2, mfgflag);
                    }
#ifdef __CONFIG_UTILS__
                    else if (strcmp(argv[2], "2") == 0)
                    {
                        wlc_antselcfg_t val = {{0}, 0};
                        val.ant_config[0] = (uint8)strtol("0x12", NULL, 0);
                        val.ant_config[1] = (uint8)strtol("0x12", NULL, 0);
                        val.ant_config[2] = (uint8)strtol("0x12", NULL, 0);
                        val.ant_config[3] = (uint8)strtol("0x12", NULL, 0);
                        nreturn= wlu_iovar_set("eth1", "phy_antsel", &val, sizeof(wlc_antselcfg_t));
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        nreturn = wl_iovar_setint("eth1", "txchain", 1);
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        nreturn = wl_iovar_setint("eth1", "rxchain", 1);
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        else
                            mfgflag = SUCCESS;
                        printf("nreturn = %d, mfgflag = %d\n", nreturn, mfgflag );
                    }
                    else if (strcmp(argv[2], "3") == 0)
                    {
                        nreturn = wl_iovar_setint("eth1", "txchain", 2);
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        nreturn = wl_iovar_setint("eth1", "rxchain", 2);
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        wlc_antselcfg_t val = {{0}, 0};
                        val.ant_config[0] = (uint8)strtol("0x20", NULL, 0);
                        val.ant_config[1] = (uint8)strtol("0x20", NULL, 0);
                        val.ant_config[2] = (uint8)strtol("0x20", NULL, 0);
                        val.ant_config[3] = (uint8)strtol("0x20", NULL, 0);
                        nreturn = wlu_iovar_set("eth1", "phy_antsel", &val, sizeof(wlc_antselcfg_t));
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        nreturn = wl_iovar_setint("eth1", "txchain", 2);
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        nreturn = wl_iovar_setint("eth1", "rxchain", 2);
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        else
                            mfgflag = SUCCESS;
                        printf("nreturn = %d, mfgflag = %d\n", nreturn, mfgflag );
                    }
#endif
                    else if (strcmp(argv[2], "4") == 0)
                    {
                        nreturn = wl_iovar_setint("eth1", "txchain", 3);
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        nreturn = wl_iovar_setint("eth1", "rxchain", 3);
                        if (nreturn != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        if (nreturn == 0)
                        {
                            mfgflag = SUCCESS;
                        }
                        else
                        {
                            mfgflag = ERROR;
                            break;
                        }
                        printf("wl txchain get:3 rxchain get:3,mfgflag value:%d\n", mfgflag );
                    }
					
			wl_set("eth1", WLC_UP, &nset, sizeof(nset));
	                wl_get("eth1", WLC_GET_UP, &nget1, sizeof(nget1));
	                if ( nget1 != 1)
	                {
	                    printf("wl status is not [up] status,return false!!!\n");
	                    mfgflag = ERROR;
	                    break;
	                }
				} //end set_ant
				else if (strcmp(argv[1], "set_channel") == 0)
                {    
                     mfg_set_value("wl0_channel", argv[2]);   				 
					 mfg_set_value("wl_channel", argv[2]);    
					 if( atoi(argv[2]) < 5 )                  
                     {
				          mfg_set_value("wl_nctrlsb", "lower");
						  mfg_set_value("wl0_nctrlsb", "lower");
                     }
			         else 
					 {
				          mfg_set_value("wl_nctrlsb", "upper");
						  mfg_set_value("wl0_nctrlsb", "upper");
			         }
			  
					 mfg_commit();
					 cyg_thread_delay(100);               
                     strcpy(argv[1],mfg_get_value("wl0_channel"));	
                     if (strcmp(argv[1],argv[2]) == 0)
                     {                           
                         mfgflag = SUCCESS;
						 printf("argv1channel = %s,mfgflag = %d\n",argv[1],mfgflag);
						 _RESTART();						
                     }
                     else
                     {                       
                        mfgflag = ERROR;
						break;
                     }  
					 
                }//end set_channel             
            }//end wlctrl
            break;

        case 4://nvram set && Tenda_mfg check USB

            memset(cmdbuf,0,sizeof(cmdbuf));

            if (strcmp(argv[0], "nvram") == 0 && strcmp(argv[1], "set") == 0)
            {
                if (strcmp(argv[2], "PIN") == 0)
                {
                    char wps_pin[16] = {0};
                    if (wl_wps_pin_check(argv[3]) == 0)
                    {
                        sprintf(cmdbuf, "wps_device_pin=%s", argv[3]);
                        argv[2] = cmdbuf;     
                        mfgflag = envram_set(3, argv);                  
                        mfgflag = envram_commit(0, NULL);
						if(mfgflag != SUCCESS)
						{
							break;
						}
						
                        cyg_thread_delay(200);               
					
                        mfg_set_value("wps_device_pin",argv[3]);					
                        mfg_commit();
		
                        strcpy(wps_pin, mfg_get_value("wps_device_pin"));
                        if (strcmp(wps_pin, argv[3]) == 0)
                        {                          
                            mfgflag = SUCCESS;
							printf("mfg_get : wps_pin = %s,mfgflag = %d\n",wps_pin,mfgflag);
                        }
                        else
                        {   
                            mfgflag = ERROR;
                        }
			
                    }//end PINcheck
                }//end PIN
                else if (strcmp(argv[2], "MAC") == 0)
                { 
                    hwaddr = ether_aton(argv[3]);
					#ifdef __CONFIG_AUTO_CONN__
					unsigned int mac_wl = 0, mac_wan = 0;
					unsigned char macaddr_wl[32], macaddr_wan[32];
                    if (hwaddr)
                    {
                        
                     
                        memset(macaddr, 0, sizeof(macaddr));
                        snprintf(macaddr, sizeof(macaddr), "%02X:%02X:%02X:%02X:%02X:%02X",
                                 hwaddr->octet[0] & 0XFF,
                                 hwaddr->octet[1] & 0XFF,
                                 hwaddr->octet[2] & 0XFF,
                                 hwaddr->octet[3] & 0XFF,
                                 hwaddr->octet[4] & 0XFF,
                                 hwaddr->octet[5] & 0XFF);
						
						mac_wl = (hwaddr->octet[3]&0XFF);
						mac_wl = (mac_wl<<8) + (hwaddr->octet[4]&0XFF);
						mac_wl = (mac_wl<<8) + (hwaddr->octet[5]&0XFF);
						mac_wl += 1;
						memset(macaddr_wl, 0, sizeof(macaddr_wl));
                        snprintf(macaddr_wl, sizeof(macaddr_wl), "%02X:%02X:%02X:%02X:%02X:%02X",
                                 hwaddr->octet[0] & 0XFF,
                                 hwaddr->octet[1] & 0XFF,
                                 hwaddr->octet[2] & 0XFF,
                                 (mac_wl>>16) & 0XFF,
                                 (mac_wl>>8) & 0XFF,
                                 (mac_wl) & 0XFF);

						mac_wan = mac_wl + 3;
						memset(macaddr_wan, 0, sizeof(macaddr_wan));
                        snprintf(macaddr_wan, sizeof(macaddr_wan), "%02X:%02X:%02X:%02X:%02X:%02X",
                                 hwaddr->octet[0] & 0XFF,
                                 hwaddr->octet[1] & 0XFF,
                                 hwaddr->octet[2] & 0XFF,
                                 (mac_wan >> 16) & 0XFF,
                                 (mac_wan>>8) & 0XFF,
                                 (mac_wan) & 0XFF);
						
                        sprintf(cmdbuf, "et0macaddr=%s", macaddr);
                        argv[2] = cmdbuf;
                        mfgflag = envram_set(3, argv);
						if(mfgflag != SUCCESS) 
						{
                           printf("set et0macaddr error\n");
						   break;
						}

                        sprintf(cmdbuf, "sb/1/macaddr=%s", macaddr_wl);
                        argv[2] = cmdbuf;
                        mfgflag = envram_set(3, argv);
						if(mfgflag != SUCCESS) 
						{
                           printf("set sb/1/macaddr error\n");
						   break;
						}
                        mfgflag = envram_commit(0, NULL);
						if(mfgflag != SUCCESS) 
						{
                           printf("commit error\n");
						   break;
						}
						
                        cyg_thread_delay(50);
						
                        macaddr[17] = '\0';
                        	
                        sprintf(cmdbuf, "%s", "sb/1/macaddr");
                        argv[2] = cmdbuf;
                        envram_get(3, argv);
                        if (strcmp(argv[2], macaddr_wl) != 0)
                        {   
                            mfgflag = ERROR;
                            break;
                        }
                        sprintf(cmdbuf, "%s", "et0macaddr");
                        argv[2] = cmdbuf;
                        envram_get(3, argv);
                        if (strcmp(argv[2], macaddr) != 0)
                        {   
                            mfgflag = ERROR;
                            break;
                        }
						
						mfgflag = SUCCESS;

                    }//end hwaddr
                    if (mfgflag == SUCCESS)  	
                    {
                        mfg_set_value(_RESTORE_DEFAULTS, "1");

						pre_deal_when_restore();

                        mfg_set_value("sb/1/macaddr", macaddr_wl);
                        mfg_set_value("et0macaddr", macaddr); 
						mfg_set_value("wan0_hwaddr", macaddr_wan);
					    mfg_commit();
                    }
					#else
					if (hwaddr)
                    {

                        memset(macaddr, 0, sizeof(macaddr));
                        snprintf(macaddr, sizeof(macaddr), "%02X:%02X:%02X:%02X:%02X:%02X",
                                 hwaddr->octet[0] & 0XFF,
                                 hwaddr->octet[1] & 0XFF,
                                 hwaddr->octet[2] & 0XFF,
                                 hwaddr->octet[3] & 0XFF,
                                 hwaddr->octet[4] & 0XFF,
                                 hwaddr->octet[5] & 0XFF);

                        sprintf(cmdbuf, "et0macaddr=%s", macaddr);
                        argv[2] = cmdbuf;
                        mfgflag = envram_set(3, argv);
						if(mfgflag != SUCCESS) 
						{
                           printf("set et0macaddr error\n");
						   break;
						}

                        sprintf(cmdbuf, "sb/1/macaddr=%s", macaddr);
                        argv[2] = cmdbuf;
                        mfgflag = envram_set(3, argv);
						if(mfgflag != SUCCESS) 
						{
                           printf("set sb/1/macaddr error\n");
						   break;
						}
                        mfgflag = envram_commit(0, NULL);
						if(mfgflag != SUCCESS) 
						{
                           printf("commit error\n");
						   break;
						}
						
                        cyg_thread_delay(50);
						
                        macaddr[17] = '\0';
                        		
                        sprintf(cmdbuf, "%s", "sb/1/macaddr");
                        argv[2] = cmdbuf;
                        envram_get(3, argv);
                        if (strcmp(argv[2], macaddr) != 0)
                        {   
                            mfgflag = ERROR;
                            break;
                        }
                        sprintf(cmdbuf, "%s", "et0macaddr");
                        argv[2] = cmdbuf;
                        envram_get(3, argv);
                        if (strcmp(argv[2], macaddr) != 0)
                        {   
                            mfgflag = ERROR;
                            break;
                        }
						
						mfgflag = SUCCESS;

                    }//end hwaddr
                    if (mfgflag == SUCCESS)  	
                    {
                        mfg_set_value(_RESTORE_DEFAULTS, "1");

						pre_deal_when_restore();

                        mfg_set_value("sb/1/macaddr", macaddr);
                        mfg_set_value("et0macaddr", macaddr);    
					    mfg_commit();
                    }
					#endif
                }//end MAC
                else if (strcmp(argv[2], "PLC_PW") == 0)
                {
                   
                }//end PLC_PW
                else if (strcmp(argv[2], "WAN_MODE") == 0)
                {
                   
                }//end WAN_MODE
                else if (strcmp(argv[2], "SSID") == 0)
                {
                    char ssid[64]={0};							
                    mfg_set_value("wl0_ssid",argv[3]);
					mfg_set_value("wl_ssid",argv[3]);
                    mfg_commit();
					
                    strcpy(ssid, mfg_get_value("wl0_ssid"));
					strcpy(ssid, mfg_get_value("wl_ssid"));
                    if (strcmp(ssid,argv[3]) == 0)
                    {                           
                        mfgflag = SUCCESS;
                    }
                    else
                    {                       
                        mfgflag = ERROR;
                    }                 
                }//end SSID
                else if (strcmp(argv[2], "boot_wait") == 0)
                {
                    if (strcmp(argv[3], "on") == 0 || strcmp(argv[3], "off") == 0)
                    {
                        sprintf(cmdbuf, "boot_wait=%s", argv[3]);
                        argv[2] = cmdbuf;
                        envram_set(3, argv);
                        envram_commit(0, NULL);

                        
						sprintf(cmdbuf,"%s","boot_wait");
						argv[2] = cmdbuf;                        			
                        envram_get(3, argv);
                        if (strcmp(argv[2], argv[3]) != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
						
                        mfg_set_value("boot_wait", argv[3]);
                        mfg_commit();			
						
                        
                        bootwait = mfg_get_value("boot_wait");
                        if (strcmp(bootwait, argv[3]) == 0)
                        {   
                            mfgflag = SUCCESS;
							printf("bootwait = %s,mfgflag = %d\n",bootwait,mfgflag);
                        }
                        else
                        {
                            mfgflag = ERROR;
                        }                      
                    }//end on/off
                }//en boot_wait
                else if(strcmp(argv[2], "wait_time") == 0)
				{
					if(argv[3]!=NULL)
					{
						sprintf(cmdbuf, "wait_time=%s", argv[3]);
                        argv[2] = cmdbuf;
                        envram_set(3, argv);
                        envram_commit(0, NULL);

						sprintf(cmdbuf,"%s","wait_time");
						argv[2] = cmdbuf;                        			
                        envram_get(3, argv);
                        if (strcmp(argv[2], argv[3]) != 0)
                        {
                            mfgflag = ERROR;
                            break;
                        }
						
                        mfg_set_value("wait_time", argv[3]);
                        mfg_commit();			
						
                        waittime = mfg_get_value("wait_time");
                        if (strcmp(waittime, argv[3]) == 0)
                        {   
                            mfgflag = SUCCESS;
							printf("wait_time = %s,mfgflag = %d\n",waittime,mfgflag);
                        }
                        else
                        {
                            mfgflag = ERROR;
                        }
					}
				}
                else if (strcmp(argv[2], "lan") == 0)
                {
                        // add for A5 V1
                     mfg_set_value("vlan1ports", "0 1 2 3 4 5*");
                     mfg_set_value("vlan2ports", "5");
                     mfg_commit();
                     mfgflag = SUCCESS;
                }//end lan
                else if (strcmp(argv[2], "wan") == 0)
                {
                     mfg_set_value("vlan1ports", "1 2 3 4 5*");
                     mfg_set_value("vlan2ports", "0 5");
                     mfg_commit();
                     mfgflag = SUCCESS;                      
                } //end wan
                else if (strcmp(argv[2], "WPS") == 0)
                {
                    if (strcmp(argv[3],"off") == 0)
                    {    
                         char wps[16] = {0};
                         mfg_set_value("lan_wps_oob", "enabled");
						 mfg_set_value("wl_wps_mode", "disabled");
                         mfg_set_value("wl0_wps_mode", "disabled");
                         mfg_set_value("wl_wps_method", "");
                         mfg_commit();
						 strcpy(wps,mfg_get_value("wl0_wps_mode"));
                         if (strcmp(wps,"disabled") == 0)
                         {
                             mfgflag = SUCCESS;
							 printf("wps = %s,mfgflag=%d\n",wps,mfgflag);
						 }
						 else
                         {
                            mfgflag = ERROR;
                         }                    
                    }
                }//end WPS
                else if (strcasecmp(argv[2], "SN") == 0)
                {
			   char sn[128]={0};
			   memset(cmdbuf, 0x0, sizeof(cmdbuf));
			   sprintf(cmdbuf, "device_sn=%s", argv[3]);//用argv[2]代替cmdbuf
			   argv[2] = cmdbuf;
			   mfgflag = envram_set(3, argv);
			   mfgflag = envram_commit(0, NULL);
			   if(mfgflag != SUCCESS)
			   {
				   break;
			   }
			   
			   cyg_thread_delay(200);				
			   
			   mfg_set_value("device_sn",argv[3]);
			   mfg_commit();
			   
			   strcpy(sn, mfg_get_value("device_sn"));
			   if (strcmp(sn,argv[3]) == 0)
			   {						   
				   mfgflag = SUCCESS;
			   }
			   else
			   {					   
				   mfgflag = ERROR;
			   }			 
		   }
            }//end nvram set    
            else if (strcmp(argv[0], "Tenda_mfg") == 0 && strcmp(argv[1], "check" )== 0&& strcmp(argv[2], "USB") == 0)
            {
                    
            }
			break;
		case 6: //wlctrl set wl -i eth1 -- interactive,mpc,ap,ALL
			if(strcmp(argv[0],"wlctrl")==0&&strcmp(argv[1],"set")==0&&strcmp(argv[2],"wl")==0)
			{
				char *wl_cmd=NULL,*tmp=NULL;
				char wl_buf[256]={0},wl_buf2[256]={0};
				int wl_err=-1;
				int nsetl ,ngetl1;
				tmp=argv[5];

				wl_set(argv[4], WLC_DOWN, &nsetl, sizeof(nsetl));
                wl_get(argv[4], WLC_GET_UP, &ngetl1, sizeof(ngetl1));
                if (ngetl1 != 0)
                {
                    printf("wl status is not [down] status,return false!!!\n");
                    mfgflag = ERROR;
                    break;
                }

				while(tmp)
				{
					wl_cmd=strchr(tmp,',');
					if(wl_cmd!=NULL)
					{
						*wl_cmd='\0';
					}
					strcpy(wl_buf,tmp);
					sprintf(wl_buf2,"wl -i %s %s",argv[4],wl_buf);
					memset(argv_wl, 0, sizeof(argv_wl));
        			argc_wl = get_argv(wl_buf2, argv_wl);
					wl_err=wl(argc_wl,argv_wl);
					if(wl_err!=0) 
					{
						sprintf(wl_error,"%s ,excute error!\n",wl_buf);
						break;
					}
					if(!strcmp(wl_buf,"counters"))
					{
						wl_get_buf(get_buf);
						printf("%s\n",get_buf);
					}
					if(wl_cmd!=NULL)
					{
						tmp=wl_cmd+1;
					}else{
						tmp=NULL;
					}
				}

				if(wl_err==0){
					mfgflag = SUCCESS;
				}else{
					mfgflag = ERROR;
				}

				wl_set(argv[4], WLC_UP, &nsetl, sizeof(nsetl));
	            wl_get(argv[4], WLC_GET_UP, &ngetl1, sizeof(ngetl1));
	            if ( ngetl1 != 1)
	            {
	                printf("wl status is not [up] status,return false!!!\n");
	                mfgflag = ERROR;
	            }
			}
			break;
	
    default:	
            mfgflag = ERROR;

        }//end switch
    if (mfgflag == SUCCESS)
    {
        //sucess
        strcpy(sendbuf, "success");
		if(strlen(get_buf))
		{
			strcat(sendbuf,":");
			strcat(sendbuf,get_buf);
		}
		get_buf[0]='\0';
    }
    else if (mfgflag == INFO)
    {    
         //htmlVersionInfo
         strcpy(sendbuf, cmdbuf);
    }
	else
    {
         //error
          strcpy(sendbuf, "error");
		 if(strlen(wl_error))
		 {
		 	strcat(sendbuf,":");
			strcat(sendbuf,wl_error);
		 }
		 wl_error[0]='\0';
		  printf("aaaaaaa\n");
    }
    ilen = strlen(sendbuf);
    sendto(mfgfd, (char *)sendbuf, ilen, 0, (struct sockaddr *)&remote, sizeof(remote));

    continue;

    }//end while
	s_mfg_button_hold_status.reset_button_hold = FALSE;
	s_mfg_button_hold_status.wifi_button_hold = FALSE;

    return;

}//end main
static void fromSetWizard(webs_t wp, char_t *path, char_t *query)
{
	char_t *wantstr;
	char *wlunit,*ssid,*pass_phrase_str;
	char_t	*DynStatic_mtu;
	char_t *wanip,*wanmsk, *gw, *dns1, *dns2,dns[40];
	char_t *pppoeuser, *pppoepwd;
	char *xkjs_user_id,*xkjs_pwd;
	char tmp_xkjs_user_id[64];
	char tmp_xkjs_pwd[64];
	int i = 0 ;
	int	i_timezone;
	float ftime_zone;
	char value_timezone[15] = {0};

	wantstr = websGetVar(wp, T("wanType"), T("dhcp"));
	pppoeuser = websGetVar(wp, T("wizardPPPoEUser"), T("")); 
	pppoepwd = websGetVar(wp, T("wizardPPPoEPwd"), T("")); 
	char_t *v12_time = websGetVar(wp,T("v12_time"),T("0"));
	wanip = websGetVar(wp, T("wizardWanIP"), T("0.0.0.0"));
	wanmsk = websGetVar(wp, T("wizardWanMask"), T("0.0.0.0"));
	gw = websGetVar(wp, T("wizardWanGateway"), T("0.0.0.0"));
	dns1 = websGetVar(wp, T("wizardWanDns1"), T(""));
	dns2 = websGetVar(wp, T("wizardWanDns2"), T(""));
	ssid = websGetVar(wp, T("wizardSSID"), T(""));
	pass_phrase_str = websGetVar(wp, T("wizardSSIDPwd"), T(""));
	char* time_zone = websGetVar(wp, T("timeZone"), T(""));
	_SET_VALUE(_RESTORE_QUICK_SET,"0");//标志着快速设置结束，由于页面分2次传送数据过来，故这里放在最后一步修改无线里面
#if 0
	if(time_zone[0] != 0)
	{
		float ftime_zone;
		ftime_zone = atof(time_zone);
		i_timezone = ftime_zone * 3600;
		for(i = 0 ; i < TIME_ZONES_NUMBER ; i++ )
		{
			if(time_zones[i].tz_offset == i_timezone )
			{
				sprintf(value_timezone , "%d" , time_zones[i].index );
				_SET_VALUE(_SYS_TZONE,value_timezone);
				break;
			}
		}
	}
#endif
	if(strcmp(wantstr, "dhcp") == 0){
		//dhcp
	}
	else if(strcmp(wantstr, "pppoe") == 0){  
		//pppoe
		//_SET_VALUE("plugplay_flag","n");//gong add
		_SET_VALUE(_WAN0_PPPOE_USERNAME,pppoeuser);
		_SET_VALUE(_WAN0_PPPOE_PASSWD,pppoepwd);
		//conmode:0,auto;1,traffic;2,hand;3,time
		_SET_VALUE(_WAN0_PPPOE_DEMAND,"0");//auto,default value
#if defined(CONFIG_CHINA_NET_CLIENT)
		_SET_VALUE(_WAN0_PPPOE_XKJX_TIME,v12_time);
#endif
	}
	else if(strcmp(wantstr, "static") == 0){
		//static ip
		if(CGI_same_net_with_lan(inet_addr(wanip),inet_addr(wanmsk)))
		{
			websRedirect(wp, T("error.asp"));
			return;
		}
		if (strcmp(wanip,gw)==0||strcmp(wanip,dns1)==0||strcmp(wanip,dns2)==0)
		{
			websRedirect(wp, T("error.asp"));
			return;
		}
		_SET_VALUE(_WAN0_IPADDR, wanip);
		_SET_VALUE(_WAN0_NETMASK, wanmsk);
		_SET_VALUE(_WAN0_GATEWAY, gw);
		sprintf(dns,"%s %s",dns1,dns2);
		 _SET_VALUE(_WAN0_DNS, dns);			
	}
	_SET_VALUE(_WAN0_PROTO, wantstr);
	_SET_VALUE(_WAN0_PROTO_INDEX, wantstr);
	_SET_VALUE("err_check","0");
	_SET_VALUE("config_index","1");
	_SET_VALUE("wan0_check","0");
	_SET_VALUE("mode_need_switch","no");
	if(strlen(ssid) > 0) 
		_SET_VALUE(WLN0_SSID0, ssid);
	if(pass_phrase_str != NULL && pass_phrase_str[0] != '\0'  ){
		_SET_VALUE(WLN0_WEP, DISABLE);
		_SET_VALUE(WLN0_AUTH_MODE, OPEN);
		_SET_VALUE(WLN0_SECURITY_TYPE, "psk2");
		_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
		confWPAGeneral( wp);
	}else {
		_SET_VALUE(WLN0_AUTH_MODE, OPEN);
		_SET_VALUE(WLN0_WEP, DISABLE);
		_SET_VALUE(WLN0_SECURITY_TYPE,"");
		_SET_VALUE(WLN0_ENCRYP_TYPE, "");
	}	
	wl_unit("0",1);
#ifdef __CONFIG_WPS__			
	stop_wireless_wps();
#endif
#ifdef __CONFIG_WPS_LED__
	wps_led_test_off();		
#endif
	change_tag = 1;
	_COMMIT();
	sys_restart2();
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n")); 
	websWrite(wp, T("%s"), "{\"errCode\":\"0\"}"); 
	websDone(wp, 200);
	return;
}

static void fromGetWizard(webs_t wp, char_t *path, char_t *query)
{
	char result[2048] = {0};
	char wan_detection[32] = {0};     //detecting|disabled|pppoe|dhcp|static
	char value[128] = {0};

	strncat(result, "{", 1);	
	wan_link_check();
	int wanConnSta = get_wan_connstatus();
	int wanConnTyp = get_wan_type();
	int wanLinkSta = get_wan_linkstatus();
	int	netWorkSta = get_wan_onln_connstatus();
	if(wanLinkSta == 0){
		sprintf(wan_detection , "disabled" );
	}else{
		if(network_tpye == 0)
			sprintf(wan_detection , "static");
		else if(network_tpye == 1)
			sprintf(wan_detection , "dhcp");
		else if(network_tpye == 2)
			sprintf(wan_detection , "pppoe");
		else if(network_tpye == 3)
			sprintf(wan_detection , "detecting");
		else if(network_tpye == 4)
			sprintf(wan_detection , "connected");
	}
	string_cat(result,"wizardWANDetection",wan_detection);
	strncat(result, ",", 1);
	memset(value , 0 , sizeof(value));
	snprintf(value , sizeof(value) ,  "%s" ,nvram_safe_get(_WAN0_PROTO) );
	string_cat(result,"wanType", value);
	strncat(result, ",", 1);
	sprintf(value,"%s",nvram_safe_get("wl0_ssid"));
	string_cat(result,"wizardSSID",value);
	strncat(result, ",", 1);
	string_cat(result,"lanIP", nvram_safe_get(_LAN0_IP));
	strncat(result, ",", 1);
	string_cat(result,"lanMask", nvram_safe_get(_LAN0_NETMASK));
	int	internetStat = get_wan_onln_connstatus();		
	memset(value , 0 ,sizeof(value));
	strncat(result, ",", 1);
	if(2 == internetStat)
		sprintf(value,"%s","true");
	else
		sprintf(value,"%s","false");
	string_cat(result,"connectInternet",value);
	strncat(result, ",", 1);
	string_cat(result,"wifiMobileDevice",iplookflag(inet_addr(wp->ipaddr))?"true":"false"); 
	
	strncat(result, "}", 1);	
	strncat(result, "\n", 1);
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);
}

static void fromNetWorkSetupInit(webs_t wp, char_t *path, char_t *query)
{
	char result[2048] = {0};
	
	char wan_connsta[8] = {0}, wan_conntyp[8] = {0};
	char wan_ip[32] = {0}, wan_mask[32] = {0}, wan_gw[32] = {0};
	char wan_dns_1[32] = {0}, wan_dns_2[32] = {0};
	char wan_contime[64] = {0};
	char wan_ppp_chk[8] = {0}, network_sta[8] = {0}, wanlink_sta[16] = {0}, network_sta_check[8] = {0};

	char lan_mac[32] = {0}, wan_mac[32] = {0};
	char sys_time[64] = {0}, run_time[64] = {0};
	char cli_num[8] = {0};
	char sw_vers[64] = {0}, hw_vers[64] = {0};

	char lan_ip[32] = {0}, lan_mask[32] = {0};

	char SSID1_mode[16] = {0};	
	char SSID1_name[128] = {0}, SSID1_passwd[128] = {0};
	char vlaue[20]={0};
	char c[128]={0};
	int i=0;
	char ip_part[4][4]={0};

	wan_link_check();
	int wanConnSta = get_wan_connstatus();
	int wanConnTyp = get_wan_type();
	int wanLinkSta = get_wan_linkstatus();
	int	netWorkSta = get_wan_onln_connstatus();

	strncat(result, "{", 1);	

	sprintf(c,"%d",iplookflag(inet_addr(wp->ipaddr)));
	string_cat(result,"transmitter",c);
	strncat(result, ",", 1);

	char temp_user[256]={0};
	string_cat(result,"adsl-user",encodeSSID(nvram_safe_get(_WAN0_PPPOE_USERNAME),temp_user));
	strncat(result, ",", 1);

	char temp_adsl_pwd[256]={0};	
	string_cat(result,"adsl-pwd",encodeSSID(nvram_safe_get(_WAN0_PPPOE_PASSWD),temp_adsl_pwd));
	strncat(result, ",", 1);

	/*wan conType*/	
	sprintf(wan_conntyp,"%d",wanConnTyp-1);
	string_cat(result,"con-type",wan_conntyp);
	strncat(result, ",", 1);

	/*network status*/		
	sprintf(network_sta,"%d",netWorkSta);
	string_cat(result,"internet-state",network_sta);
	strncat(result, ",", 1);

	/*network tpye*/	
	sprintf(network_sta_check,"%d",network_tpye);
	string_cat(result,"internet-check-type",network_sta_check);
	strncat(result, ",", 1);

	char pcMac[20];
	char mm[20]={0};
	((struct in_addr*) pcMac)->s_addr=inet_addr(wp->ipaddr);
	if(arpioctl(SIOCGARPRT, pcMac, NULL) == 0)
	{	
		sprintf(mm,"%02X:%02X:%02X:%02X:%02X:%02X",
					pcMac[4]&0XFF,
					pcMac[5]&0XFF,
					pcMac[6]&0XFF,
					pcMac[7]&0XFF,
					pcMac[8]&0XFF,
					pcMac[9]&0XFF);
	}
	else
	{
		sprintf(mm,"%02X:%02X:%02X:%02X:%02X:%02X",
					0,
					0,
					0,
					0,
					0,
					0);
	}
	_SET_VALUE(_LAN0_HWADDR1,mm);
	strcpy(mm,wp->ipaddr);
	_SET_VALUE(_LAN0_IP1,mm);	
	string_cat(result,"mac-pc",nvram_safe_get(_LAN0_HWADDR1));
	strncat(result, ",", 1);
	
	string_cat(result,"lan-ip",nvram_safe_get(_LAN0_IP));
	strncat(result, ",", 1);
	
	if(wanConnTyp == 1)
	{
	/*wan ip*/
		memset(vlaue , 0 , sizeof(vlaue));
		snprintf(vlaue , sizeof(vlaue) ,  "%s" , nvram_safe_get(_WAN0_IPADDR) );
		if(strncmp(vlaue , "0.0.0.0" , strlen("0.0.0.0")) == 0 ){
			string_cat(result,"ipval", "");
		}else{
			string_cat(result,"ipval", vlaue);
		}
		strncat(result, ",", 1);

	/*wan mask*/
	
		memset(vlaue , 0 , sizeof(vlaue));
		snprintf(vlaue , sizeof(vlaue) ,  "%s" ,nvram_safe_get(_WAN0_NETMASK) );
		if(strncmp(vlaue , "0.0.0.0" , strlen("0.0.0.0")) == 0 ){
			string_cat(result,"submask", "");
		}else{
			string_cat(result,"submask",vlaue);
		}
		
		strncat(result, ",", 1);

	/*wan gateway*/
		memset(vlaue , 0 , sizeof(vlaue));
		snprintf(vlaue , sizeof(vlaue) ,  "%s" ,nvram_safe_get(_WAN0_GATEWAY) );
		if(strncmp(vlaue , "0.0.0.0" , strlen("0.0.0.0")) == 0 ){
			string_cat(result,"gateway", "");
		}else{
			string_cat(result,"gateway",vlaue);
		}
		strncat(result, ",", 1);

	/*wan dns1*/
		char *dns,dns1[30]={0},dns2[30]={0};
		_GET_VALUE(_WAN0_DNS,dns);
		sscanf(dns,"%s %s",dns1,dns2);
		string_cat(result,"dns1",dns1);
		strncat(result, ",", 1);
		
	/*wan dns2*/	
		string_cat(result,"dns2",dns2);
		strncat(result, ",", 1);
		
	}
	else
	{
	/*wan ip*/			
		string_cat(result,"ipval","");
		strncat(result, ",", 1);

	/*wan mask*/	
		string_cat(result,"submask","");
		strncat(result, ",", 1);

	/*wan gateway*/
		string_cat(result,"gateway","");
		strncat(result, ",", 1);

	/*wan dns1*/	
		string_cat(result,"dns1","");
		strncat(result, ",", 1);
		
	/*wan dns2*/
		string_cat(result,"dns2","");
		strncat(result, ",", 1);
	}
	#if 0
	else
	{
		/*wan ip*/			
		string_cat(result,"ipval",wanConnSta==2?NSTR(SYS_wan_ip):"");
		strncat(result, ",", 1);

	/*wan mask*/	
		string_cat(result,"submask",wanConnSta==2?NSTR(SYS_wan_mask):"");
		strncat(result, ",", 1);

	/*wan gateway*/
		string_cat(result,"gateway",wanConnSta==2?NSTR(SYS_wan_gw):"");
		strncat(result, ",", 1);

	/*wan dns1*/
		if(wanConnSta == 2)
			if(wanConnTyp == PPPOEMODE ||wanConnTyp == PPTPMODE ||
				wanConnTyp == L2TPMODE||wanConnTyp==PPPOEMODE2	){
				sprintf(wan_dns_1,T("%s"),SYS_dns_1);
			}else{
				get_wan0_dns(1,wan_dns_1);
			}
		else
			sprintf(wan_dns_1,"%s","");
		
		string_cat(result,"dns1",wan_dns_1);
		strncat(result, ",", 1);
		
	/*wan dns2*/
		if(wanConnSta == 2)
			if(wanConnTyp == PPPOEMODE ||wanConnTyp == PPTPMODE ||
				wanConnTyp == L2TPMODE||wanConnTyp==PPPOEMODE2	){
				sprintf(wan_dns_2,T("%s"),SYS_dns_2);
			}else{
				get_wan0_dns(2,wan_dns_2);
			}
		else
			sprintf(wan_dns_2,"%s","");

		string_cat(result,"dns2",wan_dns_2);
		strncat(result, ",", 1);
	}
	#endif
		
/*wan conStatus*/
	sprintf(wan_connsta,"%d",wanConnSta);
	string_cat(result,"con-stat",wan_connsta);
	strncat(result, ",", 1);


/*ssid name*/
	char *value;
	char temSSID[256]={0};
	if((value = get_vif_ssid()) != NULL){
		memset(temSSID,0,sizeof(temSSID));
		value = encodeSSID(value,temSSID);
	}
	else
	{
		value = get_wl0_ssid();
		IFVALUE(value);
		value = encodeSSID(value,temSSID);
	}
	
	string_cat(result,"ssid",value);
	strncat(result, ",", 1);

/*ssid password*/
	char *unit="0",*v_unit="0.1";
	get_wl0_mode(SSID1_mode);
	if(strcmp(SSID1_mode,"ap") != 0)
	{
		
		copy_wl_index_to_unindex(v_unit);
	}else{
		
		copy_wl_index_to_unindex(unit);
	}
	char tempwd[256]={0};	
	string_cat(result,"ssid-pwd",encodeSSID(nvram_safe_get("wl_wpa_psk"),tempwd));
	strncat(result, ",", 1);


	char SSID_mode[16] = {0};	
	get_wl0_mode(SSID_mode);
	char *uu;
	char *mid_value[25] = {0};
	if(strcmp(SSID_mode,"ap") == 0)
		_SAFE_GET_VALUE("wl_wps_mode", uu);
	else
		_SAFE_GET_VALUE("wl0.1_wps_mode", uu);
	if(!strcmp(uu,"enabled"))
		strcpy(mid_value,"1");
	else		
		strcpy(mid_value,"0");
	
	string_cat(result,"enwps",mid_value);
	strncat(result, ",", 1);
	
	get_wl0_mode(SSID1_mode);
	if(strcmp(SSID1_mode,"ap") == 0)
		strcpy(mid_value,"0");
	else if(strcmp(SSID1_mode,"sta") == 0)		
		strcpy(mid_value,"1");
	else		
		strcpy(mid_value,"2");
	
	string_cat(result,"wl-mod",mid_value);
	strncat(result, ",", 1);
	
	/*wan pppoe check*/
	char check[5]={0};
	sprintf(wan_ppp_chk,"%s",nvram_safe_get("err_check"));
	if(strcmp(wan_ppp_chk,"11") == 0)
		sprintf(check,"%d",11);
	else
		sprintf(check,"%d",4);
	get_wl0_mode(SSID1_mode);
	if(wanLinkSta == 0 && !strcmp(SSID1_mode,"ap"))
	{
		sprintf(check, "%d", 0);
	}
	else if(wanConnTyp == 3)
	{
		if(!strcmp(wan_ppp_chk,"5"))
			sprintf(check, "%d", 3);
		else if(!strcmp(wan_ppp_chk,"7"))
			sprintf(check, "%d", 1);
		else if(!strcmp(wan_ppp_chk,"2"))
			sprintf(check, "%d", 2);
		else if(!strcmp(wan_ppp_chk,"3"))
			sprintf(check, "%d", 4);
	}	
	string_cat(result,"error-info",check);
	strncat(result, ",", 1);
	
	if(wanLinkSta == 0 && !strcmp(SSID1_mode,"ap"))		
		strcpy(mid_value,"0");
	else		
		strcpy(mid_value,"1");
	
	string_cat(result,"wan-link",mid_value);

	//F3 and F450 power ,add by ll 
	strncat(result, ",", 1);
	string_cat(result,"wifi-power", get_product_pwr_info());
	//end by ll
	
	strncat(result, "}", 1);	
	
	strncat(result, "\n", 1);
	
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);	
	
	return ;
}

static void set_oob_status(char *ssid, char *pwd)
{
	char *value = nvram_get("default_ssid");	
	int ischange = 0;
	
	if(ssid == NULL || pwd == NULL)
		return;

	if(value && strcmp(value, ssid) != 0)
		ischange = 1;
	
	if(strcmp(pwd, "") != 0)
		ischange = 1;

	/*
	 * if ssid has been changed or wlpwd has been set
	 */
	if(ischange){
		nvram_set(WLN0_WPS_OOB, "disabled");
	}

	return;
}

#ifdef __CONFIG_ALINKGW__
extern int report_wifiset_state(void) ;
extern int report_wifisecurity_state(void) ;
extern int g_alinkgw_enable; 
#endif

static void fromNetWorkSetupSave(webs_t wp, char_t *path, char_t *query)

{

	char_t *go, *con_type;
	con_type = websGetVar(wp, T("con-type"), T(""));
	go = websGetVar(wp, T("GO"), T("")); 
	int sys_reboot = 0,wl_reboot = 0,wan_clone = 0;

	
	if(iplookflag(inet_addr(wp->ipaddr)) == 0)
	{
		char pc_mac[20];
		int netWorkSta = get_wan_onln_connstatus();
		((struct in_addr*) pc_mac)->s_addr=inet_addr(wp->ipaddr);
		if(arpioctl(SIOCGARPRT, pc_mac, NULL) == 0)
		{	
			char save_pc_mac[20]={0};
			sprintf(save_pc_mac,"%02X:%02X:%02X:%02X:%02X:%02X",
						pc_mac[4]&0XFF,
						pc_mac[5]&0XFF,
						pc_mac[6]&0XFF,
						pc_mac[7]&0XFF,
						pc_mac[8]&0XFF,
						pc_mac[9]&0XFF);
			
			if((atoi(con_type) != (get_wan_type() -1) || 2 != netWorkSta) && 
				strcmp(nvram_safe_get(_WAN0_HWADDR),save_pc_mac) != 0)
			{
				_SET_VALUE(_WAN0_HWADDR,save_pc_mac);
				wan_clone = 1;
			}
		}
	}

	
	int tm,wan_mod,old_wan_mode;
	char val[128];
	char dns3[128];
	old_wan_mode = get_wan_type();
	if(strcmp(con_type,"") != 0)
	{	
		wan_mod = atoi(con_type);
		if( 0 ==wan_mod)
		{
			char_t  *wan_ip, *wan_msk, *gw,*dns1, *dns2,*static_mtu;	
			wan_ip = websGetVar(wp, T("ipval"), T("0.0.0.0")); 
			wan_msk = websGetVar(wp, T("submask"), T("0.0.0.0")); 
			gw = websGetVar(wp, T("gateway"), T("0.0.0.0")); 
			dns1 = websGetVar(wp, T("dns1"), T(""));
			dns2 = websGetVar(wp, T("dns2"), T(""));

			sprintf(dns3,"%s %s",dns1,dns2);
			if(strcmp(nvram_safe_get(_WAN0_IPADDR),wan_ip) != 0 
						||strcmp(nvram_safe_get(_WAN0_NETMASK),wan_msk) != 0 
								|| strcmp(nvram_safe_get(_WAN0_GATEWAY),gw) != 0
									||strcmp(nvram_safe_get(_WAN0_DNS),dns3) != 0)
			{
				sys_reboot = 1;
				_SET_VALUE(_WAN0_IPADDR,wan_ip);
				_SET_VALUE(_WAN0_NETMASK,wan_msk);
				_SET_VALUE(_WAN0_GATEWAY,gw);

				_SET_VALUE(_WAN0_DNS,dns3);
				_GET_VALUE(_STATIC_WAN0_MTU, static_mtu);
				_SET_VALUE(_WAN0_MTU,static_mtu);
			}
		}
		else if(1 == wan_mod)
		{
			char_t  *DynStatic_mtu;
			if((old_wan_mode - 1) != wan_mod)
			{
				sys_reboot = 1;
				_GET_VALUE(_DHCP_WAN0_MTU, DynStatic_mtu);
				_SET_VALUE(_WAN0_MTU,DynStatic_mtu);
			}
		}
		else if(2 == wan_mod)
		{
			char_t *user_id, *pwd, *mtu, *ac, *sev, *conmode;
			char_t *idle_time, *hour_s, *min_s, *hour_e, *min_e;
			char_t *v12_time;

			char *xkjs_user_id,*xkjs_pwd;
			char tmp_xkjs_user_id[64];
			char tmp_xkjs_pwd[64];
			
			_SET_VALUE("plugplay_flag","n");//gong add
			user_id = websGetVar(wp, T("adsl-user"), T("")); 
			pwd = websGetVar(wp, T("adsl-pwd"), T("")); 
			ac = websGetVar(wp, T("AC"), T(""));
			sev = websGetVar(wp, T("SVC"), T(""));
			conmode = websGetVar(wp, T("PCM"), T("0"));
			idle_time = websGetVar(wp, T("PIDL"), T("60"));
			
			hour_s = websGetVar(wp, T("hour1"), T("0"));
			min_s = websGetVar(wp, T("minute1"), T("0"));
			hour_e = websGetVar(wp, T("hour2"), T("0"));
			min_e = websGetVar(wp, T("minute2"), T("0"));
			v12_time = websGetVar(wp,T("v12_time"),T("0"));

			if(strcmp(nvram_safe_get(_WAN0_PPPOE_USERNAME),user_id) != 0 
						|| strcmp(nvram_safe_get(_WAN0_PPPOE_PASSWD),pwd) != 0)
			{
				sys_reboot = 1;
				
				_SAFE_GET_VALUE(_WAN0_PPPOE_USERNAME, xkjs_user_id);
				slprintf(tmp_xkjs_user_id, sizeof(tmp_xkjs_user_id), "%.*v", strlen(xkjs_user_id),xkjs_user_id);
				_SAFE_GET_VALUE(_WAN0_PPPOE_PASSWD, xkjs_pwd);
				slprintf(tmp_xkjs_pwd, sizeof(tmp_xkjs_pwd), "%.*v", strlen(xkjs_pwd),xkjs_pwd);
				if(strcmp(user_id,tmp_xkjs_user_id) == 0)
					;
				else
					_SET_VALUE(_WAN0_PPPOE_USERNAME,user_id);
				if(strcmp(pwd,tmp_xkjs_pwd) == 0)
					;
				else
					_SET_VALUE(_WAN0_PPPOE_PASSWD,pwd);
				_GET_VALUE(_PPPOE_WAN0_MTU,mtu);
				_SET_VALUE(_WAN0_PPPOE_MTU,mtu);
				_SET_VALUE(_WAN0_PPPOE_MRU,mtu);
				_SET_VALUE(_WAN0_PPPOE_AC,ac);
				_SET_VALUE(_WAN0_PPPOE_SERVICE,sev);
				_SET_VALUE(_WAN0_PPPOE_DEMAND,conmode);
				if(atoi(conmode) == PPPOE_TRAFFIC)
					_SET_VALUE(_WAN0_PPPOE_IDLETIME,idle_time);

				if(atoi(conmode) == PPPOE_BY_TIME){
					tm = atoi(hour_s)*3600 + atoi(min_s)*60;
					sprintf(val,"%d",tm);
					
					_SET_VALUE(_WAN0_PPPOE_ST,val);

					tm = atoi(hour_e)*3600 + atoi(min_e)*60;
					sprintf(val,"%d",tm);

					_SET_VALUE(_WAN0_PPPOE_ET,val);
				}
#if defined(CONFIG_CHINA_NET_CLIENT)
				_SET_VALUE(_WAN0_PPPOE_XKJX_TIME,v12_time);
#endif
			}
		}
		if((old_wan_mode - 1) != wan_mod)
		{
			sys_reboot = 1;
		}
		if(sys_reboot == 1)
		{
			_SET_VALUE("err_check","0");
			set_wan_str(wan_mod+1);
			_SET_VALUE("config_index","1");

			_SET_VALUE("wan0_check","0");
			_SET_VALUE("mode_need_switch","no");
			_SET_VALUE("wan0_connect","Connecting");
		}
		network_tpye = 4;
	}

	
	char *ssid,*pwd;
	char old_ssid[64] = {0}, old_pwd[128] = {0};
	ssid = websGetVar(wp, T("ssid"), T(""));
	pwd = websGetVar(wp, T("ssid-pwd"), T(""));	

	char SSID_mode[16] = {0};	
	get_wl0_mode(SSID_mode);

	if(strcmp(SSID_mode,"ap") == 0)
	{
		
		//copy_wl_index_to_unindex("0");
		strcpy(old_ssid, nvram_safe_get("wl0_ssid"));
		strcpy(old_pwd, nvram_safe_get("wl0_wpa_psk"));
	}else{
		
		//copy_wl_index_to_unindex("0.1"); 
		strcpy(old_ssid, nvram_safe_get("wl0.1_ssid"));
		strcpy(old_pwd, nvram_safe_get("wl0.1_wpa_psk"));
	}
	
	if(strcmp(ssid,old_ssid) != 0 || strcmp(pwd,old_pwd) != 0)
	{
		char *wlunit, *wps_enable;
		
		//char SSID_mode[16] = {0};	
		//get_wl0_mode(SSID_mode);
		if(strcmp(SSID_mode,"ap") == 0)
		{
			
			copy_wl_index_to_unindex("0");
		}else{
			
			copy_wl_index_to_unindex("0.1"); 
		}
		

		_GET_VALUE(WLN0_WPS_ENABLE,wps_enable);
		if(strcmp(wps_enable,"enabled") != 0)
		{
			if(strcmp(pwd,"") != 0)
			{
				_SET_VALUE(WLN0_SSID0, ssid);
				_SET_VALUE(WLN0_WEP, DISABLE);
				_SET_VALUE(WLN0_AUTH_MODE, OPEN);
				_SET_VALUE(WLN0_SECURITY_TYPE, "psk psk2");	
				_SET_VALUE(WLN0_WPA_PSK1, pwd);
				_SET_VALUE(WLN0_ENCRYP_TYPE,"aes");
				_SET_VALUE(WLN0_REKEY_INTERVAL, "3600");
			}
			else
			{
				_SET_VALUE(WLN0_SSID0, ssid);
				_SET_VALUE(WLN0_AUTH_MODE, OPEN);
				_SET_VALUE(WLN0_WEP, DISABLE);
				_SET_VALUE(WLN0_SECURITY_TYPE,"");
				_SET_VALUE(WLN0_ENCRYP_TYPE,"");
				_SET_VALUE(WLN0_WPA_PSK1,"");
				_SET_VALUE(WLN0_ENCRYP_TYPE, "");
			}
			
			_GET_VALUE(WLN0_UNIT,wlunit);
			wl_unit(wlunit,1);	
			
			_SET_VALUE("err_check","0");
			_SET_VALUE("config_index","1");
			_SET_VALUE("wan0_check","0");
		}
		wl_reboot = 1;
	}

	set_oob_status(ssid, pwd);
	change_tag = 1;
	//hqw add 
	int i = 0;
	char SSID1_mode[16] = {0};	
	int wanLinkSta = get_wan_linkstatus();
	get_wl0_mode(SSID1_mode);
	if(wanLinkSta != 0 && !strcmp(SSID1_mode,"ap"))
	{
		while(check_tag != 1 && i < 10)
		{
			i++;
			cyg_thread_delay(100);
		}
	}
	//end
	change_tag = 0;

	
	_COMMIT();

#ifdef __CONFIG_ALINKGW__
	if( g_alinkgw_enable )
	{
		if(strcmp(ssid,old_ssid) != 0)
		{
			 report_wifiset_state()	;		
		}
	
		if( strcmp(pwd,old_pwd) != 0)
		{
			report_wifisecurity_state();
		}
	}
	
#endif

	if(sys_reboot == 1 || wan_clone == 1)
	{
		stop_wan();
		if (wan_link_status() || wan_clone == 1)
		{
			start_wan();
		}
	}
	if(wl_reboot == 1)
	{
		_RESTART();
	}
	
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, CONFIG_SUCCESS);
	websDone(wp, 200);	
}


static void fromSystemManageInit(webs_t wp, char_t *path, char_t *query)
{
	char result[2048] = {0};
	
	char wan_connsta[8] = {0}, wan_conntyp[8] = {0};
	char wan_ip[32] = {0} , wan_mask[32] = {0}, wan_gw[32] = {0};
	char wan_dns_1[32] = {0}, wan_dns_2[32] = {0};
	char wan_contime[64] = {0};
	char wan_ppp_chk[8] = {0}, network_sta[8] = {0}, wanlink_sta[16] = {0};

	char lan_mac[32] = {0}, wan_mac[32] = {0};
	char sys_time[64] = {0}, run_time[64] = {0};
	char cli_num[8] = {0};
	char sw_vers[64] = {0}, hw_vers[64] = {0};

	char lan_ip[32] = {0}, lan_mask[32] = {0};

	char SSID1_mode[16] = {0};	
	char SSID1_name[128] = {0}, SSID1_passwd[128] = {0};
	char vlaue[20]={0};
	char *value;
	char * temp_value  = NULL ;
	char time_str[30] = {0};
	
	wan_link_check();
	int wanConnSta = get_wan_connstatus();
	int wanConnTyp = get_wan_type();
	int wanLinkSta = get_wan_linkstatus();
	int	netWorkSta = get_wan_onln_connstatus();



	strncat(result, "{", 1);	

	//add by ll
	string_cat(result,"wifi-power", get_product_pwr_info());
	strncat(result, ",", 1);
	//end by ll

	sprintf(vlaue,"%d",iplookflag(inet_addr(wp->ipaddr)));
	string_cat(result,"transmitter",vlaue);
	strncat(result, ",", 1);

	string_cat(result,"hasoldpwd",nvram_safe_get("http_defaultpwd1"));	
	strncat(result, ",", 1);	

	string_cat(result,"resultpwd",nvram_safe_get("http_passwd_tip1"));	
	strncat(result, ",", 1);
	
	sprintf(sw_vers,T("%s_%s"),W311R_ECOS_SV,__CONFIG_WEB_VERSION__);
	string_cat(result,"softVer",sw_vers);	
	strncat(result, ",", 1);
	
	string_cat(result,"error-info",nvram_safe_get("err_check"));	
	strncat(result, ",", 1);	

	/*network status*/
	sprintf(network_sta,"%d",netWorkSta);	
	string_cat(result,"network-type",network_sta);	
	strncat(result, ",", 1);
	

	string_cat(result,"mtu-dhcp",nvram_safe_get("dhcp_wan0_mtu"));	
	strncat(result, ",", 1); 

	string_cat(result,"mtu-static",nvram_safe_get("static_wan0_mtu"));	
	strncat(result, ",", 1);
	
	string_cat(result,"mtu-adsl",nvram_safe_get("pppoe_wan0_mtu"));	
	strncat(result, ",", 1); 


	char fmmac[6]={0,0,0,0,0,0};
	iflib_getifhwaddr("br0",fmmac);

	#ifdef __CONFIG_AUTO_CONN__
	unsigned int mac_def = 0;
	mac_def = (fmmac[3]&0XFF);
	mac_def = (mac_def<<8) + (fmmac[4]&0XFF);
	mac_def = (mac_def<<8) + (fmmac[5]&0XFF);
	mac_def += 4;

	sprintf(vlaue,"%02X:%02X:%02X:%02X:%02X:%02X",
				fmmac[0]&0XFF,
				fmmac[1]&0XFF,
				fmmac[2]&0XFF,
				(mac_def>>16) & 0XFF,
				(mac_def>>8) & 0XFF,
				(mac_def) & 0XFF);	
	#else
	sprintf(vlaue,"%02X:%02X:%02X:%02X:%02X:%02X",
				fmmac[0]&0XFF,
				fmmac[1]&0XFF,
				fmmac[2]&0XFF,
				fmmac[3]&0XFF,
				fmmac[4]&0XFF,
				fmmac[5]&0XFF);	
	#endif
	string_cat(result,"mac-def",vlaue);	
	strncat(result, ",", 1);

/*lan mac*/
	char pcMac[20];
	char mm[20]={0};
	((struct in_addr*) pcMac)->s_addr=inet_addr(wp->ipaddr);
	if(arpioctl(SIOCGARPRT, pcMac, NULL) == 0)
	{	
		sprintf(mm,"%02X:%02X:%02X:%02X:%02X:%02X",
					pcMac[4]&0XFF,
					pcMac[5]&0XFF,
					pcMac[6]&0XFF,
					pcMac[7]&0XFF,
					pcMac[8]&0XFF,
					pcMac[9]&0XFF);
	}
	else
	{
		sprintf(mm,"%02X:%02X:%02X:%02X:%02X:%02X",
					0,
					0,
					0,
					0,
					0,
					0);
	}
	_SET_VALUE(_LAN0_HWADDR1,mm);
	strcpy(mm,wp->ipaddr);
	_SET_VALUE(_LAN0_IP1,mm);	
	string_cat(result,"mac-pc",nvram_safe_get(_LAN0_HWADDR1));	
	strncat(result, ",", 1);

/*wan mac*/ 
	char *u, *v;
	_SAFE_GET_VALUE("wl0_mode", u);
	if(strcmp(u,"sta")==0)
		_SAFE_GET_VALUE("wl0.1_hwaddr", v);
	else
		_SAFE_GET_VALUE(_WAN0_HWADDR, v);	
	string_cat(result,"mac-wan",v);	
	strncat(result, ",", 1);

/*lan ip*/
	string_cat(result,"lanip",nvram_safe_get(_LAN0_IP));	
	strncat(result, ",", 1);
	
/*lan mask*/
	string_cat(result,"submask",wanConnSta==2?NSTR(SYS_wan_mask):"");	
	strncat(result, ",", 1);

/*wan ip*/
	string_cat(result,"wanip",wanConnSta==2?NSTR(SYS_wan_ip):"");	
	strncat(result, ",", 1);

/*wan gateway*/
	string_cat(result,"gateway",wanConnSta==2?NSTR(SYS_wan_gw):"");	
	strncat(result, ",", 1);

/*wan dns1*/
	if(wanConnSta == 2)
		if(wanConnTyp == PPPOEMODE ||wanConnTyp == PPTPMODE ||
			wanConnTyp == L2TPMODE||wanConnTyp==PPPOEMODE2	){
			sprintf(wan_dns_1,T("%s"),SYS_dns_1);
		}else{
			get_wan0_dns(1,wan_dns_1);
		}
	else
		sprintf(wan_dns_1,"%s","");	
	string_cat(result,"dns1",wan_dns_1);	
	strncat(result, ",", 1);
	
/*wan dns2*/
	if(wanConnSta == 2)
		if(wanConnTyp == PPPOEMODE ||wanConnTyp == PPTPMODE ||
			wanConnTyp == L2TPMODE||wanConnTyp==PPPOEMODE2	){
			sprintf(wan_dns_2,T("%s"),SYS_dns_2);
		}else{
			get_wan0_dns(2,wan_dns_2);
		}
	else
		sprintf(wan_dns_2,"%s","");
	string_cat(result,"dns2",wan_dns_2);	
	strncat(result, ",", 1);

/*wan conStatus*/
	sprintf(wan_connsta,"%d",wanConnSta);
	string_cat(result,"con-state",wan_connsta);	
	strncat(result, ",", 1);
	
/*wan conType*/
	sprintf(wan_conntyp,"%d",wanConnTyp-1);
	string_cat(result,"con-type",wan_conntyp);	
	strncat(result, ",", 1);
	
	unsigned int  run_time0,run_time1,run_time2,run_time3;
	
	//add by z10312
	int  curtime_temp = time(0);
	if((curtime_temp - SYS_wan_conntime) < 0 )
	{
		printf(" %s %d  timerror conntime_temp=%d= SYS_wan_conntime=%d= \n", __FUNCTION__, __LINE__, 
			curtime_temp, SYS_wan_conntime);
		SYS_wan_conntime = curtime_temp;
	}
	
	run_time0=(wanConnSta==2&&SYS_wan_conntime)?(curtime_temp - SYS_wan_conntime):0;
	run_time1 = run_time0/3600;
	run_time2 = (run_time0%3600)/60;
	run_time3 = run_time0%60;
	
	
	sprintf(run_time,"%02u:%02u:%02u:%02u", run_time1/24, run_time1%24, run_time2,run_time3);
	string_cat(result,"con-time",run_time);
	
	strncat(result, ",", 1);
	
	char SSID_mode[16] = {0};	
	get_wl0_mode(SSID_mode);
	char *uu;
	char mid_value[25] = {0};
	if(strcmp(SSID_mode,"ap") == 0)
		_SAFE_GET_VALUE("wl_wps_mode", uu);
	else
		_SAFE_GET_VALUE("wl0.1_wps_mode", uu);
	if(!strcmp(uu,"enabled"))
		sprintf(mid_value,"%d",1);
	else	
		sprintf(mid_value,"%d",0);
	string_cat(result,"enwps",mid_value); 
	strncat(result, ",", 1);

	if(strcmp(SSID_mode,"ap") == 0)
		_GET_VALUE(WLN0_SECURITY_TYPE, value);
	else
		_GET_VALUE("wl0.1_akm", value);

	if(!strcmp(value,"0"))
		strcpy(mid_value,"OPEN/");
	else if(!strcmp(value,"1"))		
		strcpy(mid_value,"SHARED/");		
	else if(!strcmp(value,"psk"))		
	    strcpy(mid_value,"WPA/");
	else if(!strcmp(value,"psk2"))		
		strcpy(mid_value,"WPA2/");
	else if(!strcmp(value,"psk psk2"))		
		strcpy(mid_value,"WPAWPA2/");
	else		
		strcpy(mid_value,"NONE");

	if(strcmp(SSID_mode,"ap") == 0)
		_GET_VALUE(WLN0_ENCRYP_TYPE, value);
	else
		_GET_VALUE("wl0.1_crypto", value);
	if(!strcmp(value,"wep"))
		strncat(mid_value, "WEP", strlen("WEP"));
	else if(!strcmp(value,"aes"))
		strncat(mid_value, "AES", strlen("AES"));
	else if(!strcmp(value,"tkip"))
		strncat(mid_value, "TKIP", strlen("AES"));
	else if(!strcmp(value,"tkip+aes"))
		strncat(mid_value, "AESTKIP", strlen("AESTKIP"));
	
	string_cat(result,"super-encrypt",mid_value);
	strncat(result, ",", 1);

	_GET_VALUE("wan_speed", value);
	char wanspeed[10] = {0};
	if(!strcmp(value,"0"))
		strcpy(mid_value,"auto");
	else if(!strcmp(value,"1"))
		strcpy(mid_value,"-10");
	else if(!strcmp(value,"2"))
		strcpy(mid_value,"10");
	else if(!strcmp(value,"3"))
		strcpy(mid_value,"-100");
	else if(!strcmp(value,"4"))
		strcpy(mid_value,"100");
	
	string_cat(result,"wanspeed",mid_value);
#ifdef __CONFIG_PPPOE_AUTO_MTU__
	strncat(result, ",", 1);

	_GET_VALUE("pppoe_auto", value);
	if(!strcmp(value,"enable"))
		strcpy(mid_value,"1");
	else
		strcpy(mid_value,"0");
	
	string_cat(result,"autoSetMtu",mid_value);
#endif
	strncat(result, ",", 1);

	
	_GET_VALUE(_LAN0_DHCPD_EN, value);
	if(!strcmp(value,"static"))
		strcpy(mid_value,"0");
	else
		strcpy(mid_value,"1");
	
	string_cat(result,"dhcpEn",mid_value);

	strncat(result, ",", 1);

	
	string_cat(result,"broadcastssid",nvram_safe_get(WLN0_HIDE_SSID));

	strncat(result, ",", 1);
	
	string_cat(result,"channel",nvram_safe_get(WLN0_CHANNEL1));

	char wlBandwidth[10] = {0};
	_GET_VALUE(WLN0_HT_BW1, value);
	_GET_VALUE(WLN0_HT_BW_FAKE_AUTO, temp_value);

#if 0
	if(!strcmp(value,"0"))
	{
		strcpy(mid_value,"0");//20
	}
	else if(!strcmp(value,"1"))
	{
		_GET_VALUE(WLN0_OBSS1, value);
		if(!strcmp(value,"1"))
		{
			strcpy(mid_value,"auto");//20/40
		}
		else
		{
			strcpy(mid_value,"1");//40
		}
	}
#endif
	if(!strcmp(value,"1"))
	{
		if(!strcmp("1" ,  temp_value ))
		{
			strcpy(mid_value,"auto");//auto
		}
		else
		{
			strcpy(mid_value,"1");//40
		}		
	}
	else
	{
		strcpy(mid_value,"0");//20
	}

	strncat(result, ",", 1);
	
	string_cat(result,"wlBandwidth",mid_value);

	//add liuchengchi 2014-11-12
	strncat(result, ",", 1);
	
	
	
	_GET_VALUE("wl0_mode", value);
	string_cat(result,"wanMode",value);
	//endadd

#ifdef __CONFIG_QUICK_SET__
//add liuchengchi 2014-11-18
	strncat(result, ",", 1);
	_GET_VALUE("time_zone", value);
	time_t   cur_time_t = time(0);
	sprintf(vlaue , "%d" , cur_time_t - time_zones[atoi(value)].tz_offset);
	string_cat(result,"timeZone",value);
	//printf("[LCC] %s %d : timeZone is %s \n" , __FUNCTION__ , __LINE__ ,value );

	strncat(result, ",", 1);
	if(get_sntp_update_time_success() != 1){	
		string_cat(result,"internet-state","0");
	}else{
		string_cat(result,"internet-state","1");
	}
	strncat(result, ",", 1);
	string_cat(result,"curTime",vlaue);


	struct tm *cur_time_tm;
	cur_time_tm = localtime(&cur_time_t);
	snprintf(time_str,sizeof(time_str),"%d-%02d-%02d %02d:%02d:%02d" , 1900+cur_time_tm->tm_year ,cur_time_tm->tm_mon+1 , 
		cur_time_tm->tm_mday, cur_time_tm->tm_hour , cur_time_tm->tm_min , cur_time_tm->tm_sec);

	strncat(result, ",", 1);
	string_cat(result,"time_str",time_str);	
#endif

	strncat(result, ",", 1);
	if(nvram_match("restart_enable","enable"))
		string_cat(result,"automaticEn","1");
	else
		string_cat(result,"automaticEn","0");

	strncat(result, "}", 1);	
	
	strncat(result, "\n", 1);
	
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);	
	
	return ;
}


extern void stop_wps_when_failed(void);
extern void set_wireless_wps(webs_t wp, char_t *path, char_t *query);
#ifdef __CONFIG_WPS_LED__
extern void wps_led_test_on(void);
#endif
extern void wl_unit(char *unit,int from_secu);
	
#ifdef __CONFIG_ALINKGW__
extern int report_wifichannel_state(void) ;
extern int report_wifiset_state(void) ;
#endif
static void fromSystemManageSave(webs_t wp, char_t *path, char_t *query)
{	
	char_t  *old_pwd=NULL,*new_pwd1=NULL,*pwd_tip=NULL, *new_mtu , *wan_mode;
	char_t result[128] = {0};

	old_pwd = websGetVar(wp, T("oldpwd"), T("")); 
	new_pwd1 = websGetVar(wp, T("Enewpwd"), T("")); 
	new_mtu = websGetVar(wp, T("mtuval"), T(""));

#ifdef __CONFIG_ALINKGW__
	int broadcast_flag = 0;
	int channel_band_flag = 0 ;
#endif


	int pwd_temp = -1;
	int reboot_temp = 0;
	int restart_temp = 0;
	int wl_temp = 0;
	int wanConnTyp = get_wan_type();
	int i=0;
	
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
							
#ifdef __CONFIG_QUICK_SET__		
	//add liuchengchi 2014-11-18
	char_t  *time_zone;
	char_t  *curr_time;
	time_t	new_time;
	int     old_time_zone;
															
	old_time_zone = atoi(nvram_safe_get("time_zone"));		
													
	time_zone  = websGetVar(wp, T("timeZone"), T("57"));
	_SET_VALUE(_SYS_TZONE, time_zone);
	
	if(get_sntp_update_time_success() != 1 ){	
		curr_time = websGetVar(wp, T("curTime"), T("123456"));
		new_time = (time_t) atoi(curr_time);
		new_time += time_zones[atoi(time_zone)].tz_offset;
	}else{
		new_time = time(0) - time_zones[old_time_zone].tz_offset + time_zones[atoi(time_zone)].tz_offset;
	}
	cyg_libc_time_settime(new_time);
	update_application_time();
		
#endif

	
	_GET_VALUE(_WAN0_PROTO, wan_mode);
	char SSID1_mode[16] = {0};
	get_wl0_mode(SSID1_mode);
	if(strcmp(new_mtu,"")!=0)
	{

		if(!strcmp(wan_mode , "static") && strcmp(new_mtu ,nvram_safe_get(_STATIC_WAN0_MTU)) != 0)
		{
			_SET_VALUE(_STATIC_WAN0_MTU, new_mtu);
			restart_temp = 1;
		}
		else if(!strcmp(wan_mode , "dhcp") && strcmp(new_mtu ,nvram_safe_get(_DHCP_WAN0_MTU)) != 0)
		{
			_SET_VALUE(_DHCP_WAN0_MTU, new_mtu);
			restart_temp = 1;
		}
		else if(!strcmp(wan_mode , "pppoe") && strcmp(new_mtu ,nvram_safe_get(_PPPOE_WAN0_MTU))!= 0)
		{
			restart_temp = 1;
			_SET_VALUE(_PPPOE_WAN0_MTU, new_mtu);
			_SET_VALUE(_WAN0_PPPOE_MTU,	new_mtu);
			_SET_VALUE(_WAN0_PPPOE_MRU,	new_mtu);
#ifdef __CONFIG_PPPOE_AUTO_MTU__
			char *autoSetMtu = NULL;
			autoSetMtu = websGetVar(wp, T("autoSetMtu"), T("0"));
			if(strcmp(autoSetMtu,"1") == 0)
			{
				_SET_VALUE("pppoe_auto","enable");
			}
			else if(strcmp(autoSetMtu,"0") == 0)
			{
				_SET_VALUE("pppoe_auto","disable");
			}
#endif
		}
		_SET_VALUE(_WAN0_MTU,new_mtu);
		strncat(result,"mtu ",strlen("mtu "));
	}

	char_t  *mac;

	mac = websGetVar(wp, T("mac-wan"), T("")); 
	if(strcmp(mac,"") != 0 && strcmp(mac,nvram_safe_get(_WAN0_HWADDR)) != 0)
	{
		_SET_VALUE(_WAN0_HWADDR,mac);
		strncat(result,"mac ",strlen("mac "));
		restart_temp = 1;
	}

	
	char_t *wanspeed;
	char save_wanspeed[5] = {0};
	int speed = 0;

	wanspeed =websGetVar(wp, T("wanspeed"), T(""));
	if(strcmp(wanspeed,"") != 0)
	{
		if(strcmp(wanspeed,"auto") == 0)
		{
			speed = 0;
		}
		else if(strcmp(wanspeed,"-10") == 0)
		{
			speed = 1;
		}
		else if(strcmp(wanspeed,"10") == 0)
		{
			speed = 2;
		}
		else if(strcmp(wanspeed,"-100") == 0)
		{
			speed = 3;
		}
		else if(strcmp(wanspeed,"100") == 0)
		{
			speed = 4;
		}
		sprintf(save_wanspeed,"%d",speed);
		if(strcmp(save_wanspeed,nvram_safe_get("wan_speed")) != 0)
		{
			_SET_VALUE("wan_speed", save_wanspeed);
			ifr_set_link_speed2(speed);
			strncat(result,"wanspeed ",strlen("wanspeed "));
		}
		
	}

#ifdef __CONFIG_WPS__
	
	char_t *security_mode,  *wpsenable,*old_wps,old_wps_str[16];
	char_t *wlunit,*old_ssid,wps_mode[10]={0};
	char old_ssid_value[128] = {0};
	wpsenable = websGetVar(wp, T("enwps"), T(""));

	_GET_VALUE(WLN0_WPS_ENABLE,old_wps);
	//must do this
	strcpy(old_wps_str,old_wps);
	if(strcmp(wpsenable,"1") == 0)
		strcpy(wps_mode,"enabled");
	else if(strcmp(wpsenable,"0") == 0)
		strcpy(wps_mode,"disabled");
	if(strcmp(wpsenable,"") !=0 && strcmp(old_wps_str,wps_mode) != 0)
	{
		char *unit="0",*v_unit="0.1";
		get_wl0_mode(SSID1_mode);
		if(strcmp(SSID1_mode,"ap") != 0)
		{
			
			copy_wl_index_to_unindex(v_unit);
		}else{
			
			copy_wl_index_to_unindex(unit);
		}
		if(!strcmp(wpsenable,"1"))
		{	
#ifdef __CONFIG_WPS__
			//first,let's stop wps enabled former
			stop_wps_when_failed();
#endif	
			if(strcmp(old_wps_str,"enabled") == 0){
				/* it has no effect, rm it */
			//	set_wireless_wps( wp,  path,  query);
			}else{
				nvram_set(WLN0_WPS_ENABLE, "enabled");
				nvram_set(WLN0_WPS_METHOD, "pbc");
#ifdef __CONFIG_WPS_LED__
				wps_led_test_on();	
#endif
			}
			_GET_VALUE(WLN0_UNIT,wlunit);
			wl_unit(wlunit,1);
#ifdef __CONFIG_WPS__
			tenda_stop_wps_timer();	
#endif
		}
		else
		{
			if(strcmp(old_wps_str,"enabled") == 0){
#ifdef __CONFIG_WPS__
			set_wireless_wps( wp,  path,  query);
#endif
			//_SET_VALUE(WLN0_WPS_OOB, "enabled");
			_SET_VALUE(WLN0_WPS_ENABLE, "disabled");	
			_SET_VALUE(WLN0_WPS_METHOD, "");
#ifdef __CONFIG_WPS__			
			stop_wireless_wps();
#endif
#ifdef __CONFIG_WPS_LED__
			wps_led_test_off();	
#endif
	 		}	
		}
		_SET_VALUE(WLN0_WPS_METHOD, "pbc");	
		strncat(result,"wps ",strlen("wps "));
		_GET_VALUE(WLN0_UNIT,wlunit);		
		wl_unit(wlunit,1);
		wl_temp = 1;
	}
#endif	
	
	char_t *dhen = NULL;

	dhen = websGetVar(wp,T("dhcpEn"),T("1"));
	if((strcmp(nvram_safe_get(_LAN0_DHCPD_EN),"static") == 0 && strcmp(dhen,"1")==0)
			||(strcmp(nvram_safe_get(_LAN0_DHCPD_EN),"dhcp") == 0 && strcmp(dhen,"0")==0))
	{
		set_dhcpd_en(atoi(dhen));
		wl_temp = 1;
	}

	
	char_t *sz11bChannel = NULL, *hidessid = NULL,*wlBandwidth = NULL;
	sz11bChannel = websGetVar(wp, T("channel"), T("0"));
	hidessid =  websGetVar(wp, T("broadcastssid"), T("1"));
	wlBandwidth =  websGetVar(wp, T("wlBandwidth"), T("auto")); 
	
	if(strcmp(nvram_safe_get(WLN0_HIDE_SSID),hidessid) != 0)
	{
		_SET_VALUE(WLN0_HIDE_SSID, hidessid);
		wl_temp = 1;
#ifdef __CONFIG_ALINKGW__
		broadcast_flag = 1;
#endif
	}
	if(strcmp(nvram_safe_get(WLN0_CHANNEL1),sz11bChannel) != 0)
	{
		/*add wzs ,channel 10~13 sideband should be "upper".*/
		_SET_VALUE(WLN0_CHANNEL1, sz11bChannel);
		if(atoi(sz11bChannel) <= 4 && atoi(sz11bChannel) >= 1)
			_SET_VALUE(WLN0_HT_EXTCHA1, "lower");
		else if(atoi(sz11bChannel) <= 13 && atoi(sz11bChannel) >= 10)
			_SET_VALUE(WLN0_HT_EXTCHA1, "upper");
		wl_temp = 1;
#ifdef __CONFIG_ALINKGW__
		channel_band_flag = 1;
#endif
	}

	//add (for wl bandwidth)
	char ht_bw_value[8]={0},obss_value[8]={0};
	if(0 ==strcmp(wlBandwidth, "0"))//20
	{
		//_SET_VALUE(WLN0_HT_EXTCHA1, "none");
		_SET_VALUE(WLN0_HT_BW_FAKE_AUTO,"0");
		strcpy(ht_bw_value,"0");
		strcpy(obss_value,"0");
	}
	else if (0 ==strcmp(wlBandwidth, "1"))//40
	{
		strcpy(ht_bw_value,"1");
		strcpy(obss_value,"0");
		_SET_VALUE(WLN0_HT_BW_FAKE_AUTO,"0");
	}
#ifdef AUTO_WIDTH_KEEP_FORCE
	else//fake 20/40
	{
		strcpy(ht_bw_value,"0");
		strcpy(obss_value,"0");
		_SET_VALUE(WLN0_HT_BW_FAKE_AUTO,"1");

	}
#else
	else//20/40
	{
		_SET_VALUE(WLN0_HT_BW_FAKE_AUTO,"1");
		strcpy(ht_bw_value,"1");
		strcpy(obss_value,"1");
	}
#endif

	if(strcmp(nvram_safe_get(WLN0_HT_BW1),ht_bw_value) != 0 ||
		strcmp(nvram_safe_get(WLN0_OBSS1),obss_value) != 0)
	{
		_SET_VALUE(WLN0_HT_BW1, ht_bw_value);
		_SET_VALUE(WLN0_OBSS1, obss_value);
		wl_temp = 1;
#ifdef __CONFIG_ALINKGW__
		channel_band_flag = 1;
#endif
	}
	//end add (for wl bandwidth)
	
	if(strcmp(old_pwd,"") == 0 && strcmp(new_pwd1,"") == 0)
	{
		
	}
	else if(strcmp(old_pwd,"") != 0 && strcmp(new_pwd1,"") == 0)
	{
		
		if(strcmp(nvram_safe_get(HTTP_PASSWD),old_pwd) == 0)
		{
			
			_SET_VALUE(HTTP_PASSWD, new_pwd1);
			_SET_VALUE(HTTP_PASSWD_TIP1, "1");
			_SET_VALUE("http_defaultpwd1", "0");
			strncpy(g_Pass,new_pwd1,sizeof(g_Pass));
			pwd_temp = 1;
		}
		else
		{
			
			_SET_VALUE(HTTP_PASSWD_TIP1, "0");
			pwd_temp = 0;		
			
		}
	}
	else if(strcmp(old_pwd,"") == 0 && strcmp(new_pwd1,"") != 0)
	{
		_SET_VALUE(HTTP_PASSWD, new_pwd1);
		_SET_VALUE(HTTP_PASSWD_TIP1, "1");
		_SET_VALUE("http_defaultpwd1", "1");
		strncpy(g_Pass,new_pwd1,sizeof(g_Pass));
		pwd_temp = 1;
	}
	else
	{
		if(strcmp(nvram_safe_get(HTTP_PASSWD),old_pwd) == 0)
		{
			_SET_VALUE(HTTP_PASSWD, new_pwd1);
			_SET_VALUE(HTTP_PASSWD_TIP1, "1");
			_SET_VALUE("http_defaultpwd1", "1");
			strncpy(g_Pass,new_pwd1,sizeof(g_Pass));
			pwd_temp = 1;
		}
		else
		{
			
			_SET_VALUE(HTTP_PASSWD_TIP1, "0");
			
			pwd_temp = 0;		
			
		}
	}

	
	if(strcmp(nvram_safe_get(HTTP_PASSWD_TIP1),"1")==0 || (strcmp(new_pwd1,"")==0 && strcmp(old_pwd,"")==0))
	{
		char_t  *lan_ip;
		lan_ip = websGetVar(wp, T("lanip"), T(""));
		if(strcmp(lan_ip , ""))
		{
			char_t  *lan_mask,*value;
			char_t	*go ="error.asp";
			char_t dhcp_ip_start[20],dhcp_ip_end[20];
			unsigned int dhcp_ip[4],lan_ip2[4];
			char_t old_lan_ip[20],old_lanmask[20],wan_ip[20],wan_mask[20];
			struct in_addr;

			lan_mask = websGetVar(wp,T("LANMASK"),T("255.255.255.0"));

			strcpy(old_lan_ip,_GET_VALUE(_LAN0_IP,value));
			strcpy(old_lanmask,_GET_VALUE(_LAN0_NETMASK,value));

			_SET_VALUE(_LAN0_IP,lan_ip);
			_SET_VALUE(_LAN0_NETMASK,lan_mask);

			if(get_wan_type() == STATICMODE)
			{
				strcpy(wan_ip,_GET_VALUE(_WAN0_IPADDR,value));
				strcpy(wan_mask,_GET_VALUE(_WAN0_NETMASK,value));
				if (CGI_same_net_with_lan(inet_addr(wan_ip),inet_addr(wan_mask)))
				{
					_SET_VALUE(_LAN0_IP,old_lan_ip);
					_SET_VALUE(_LAN0_NETMASK,old_lanmask);

					strncat(result,"ip_conflict ",strlen("ip_conflict "));
					websWrite(wp, result);
					websDone(wp, 200);
					//SetErrorArg(LAN_WAN_IP_ERROR,go);
					//websRedirect(wp, go);
					return;
				}
			}

			
			if(CGI_same_net_with_lan(SYS_lan_ip,SYS_lan_mask) == 0){
				//change dhcp settings
				strcpy(dhcp_ip_start,_GET_VALUE(_LAN0_DHCPD_START,value));
				strcpy(dhcp_ip_end,_GET_VALUE(_LAN0_DHCPD_END,value));

				sscanf(lan_ip, "%d.%d.%d.%d", &lan_ip2[0], &lan_ip2[1], &lan_ip2[2], &lan_ip2[3]);
				
				sscanf(dhcp_ip_start, "%d.%d.%d.%d", &dhcp_ip[0], &dhcp_ip[1], &dhcp_ip[2], &dhcp_ip[3]);
				sprintf(dhcp_ip_start,"%d.%d.%d.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],dhcp_ip[3]);

				sscanf(dhcp_ip_end, "%d.%d.%d.%d", &dhcp_ip[0], &dhcp_ip[1], &dhcp_ip[2], &dhcp_ip[3]);
				sprintf(dhcp_ip_end,"%d.%d.%d.%d",lan_ip2[0], lan_ip2[1], lan_ip2[2],dhcp_ip[3]);

				_SET_VALUE(_LAN0_DHCPD_START,dhcp_ip_start);
				_SET_VALUE(_LAN0_DHCPD_END,dhcp_ip_end);
				
				modify_filter_virtual_server(lan_ip);
			}
			
			_SET_VALUE("err_check","0");
			reboot_temp = 1;		
		}
	}

	char_t *auto_reboot_en = NULL;
	int restart_check_temp = 0;

	auto_reboot_en = websGetVar(wp, T("automaticEn"), "1");

	//Device Management
	if (0 == strcmp(auto_reboot_en, "1"))
	{
		if(1 != nvram_match("restart_enable", "enable"))
		{
			restart_check_temp = 1;
			_SET_VALUE("restart_enable", "enable");
		}
	}
	else
	{
		if(1 != nvram_match("restart_enable", "disable"))
		{
			restart_check_temp = 1;
			_SET_VALUE("restart_enable", "disable");
		}
	}

	
	_COMMIT();

	if(restart_check_temp)
	{
		restart_check_main_loop();
	}
	
	if(pwd_temp == 0)
	{		
		//add by z10312 
		strncat(result,"pwdError ",strlen("pwdError "));
		
	}
	else if(pwd_temp == 1)
	{		
		for (i=0; i<MAX_USER_NUM; i++)
		{
			if(strncmp(loginUserInfo[i].ip, wp->ipaddr,32) == 0) 
			{
				memset(loginUserInfo[i].ip, 0, IP_SIZE);				
				loginUserInfo[i].time = 0;
				break;
			}
		}
	
		//add by z10312 
		strncat(result,"pwdOk ",strlen("pwdOk "));
		
	}
	if(reboot_temp)
	{		
		strncat(result,"ip ",strlen("ip "));
		websWrite(wp, result);
		sys_reboot();
	}
	else if(restart_temp)
	{
		websWrite(wp, result);
		_RESTART_ALL();			
	}
	else if(wl_temp)
	{
		websWrite(wp, result);
		_RESTART();
	}
	else
	{
		websWrite(wp, result);		
	}
#ifdef __CONFIG_ALINKGW__
	if(g_alinkgw_enable && channel_band_flag)
	{
		report_wifichannel_state();
	}

	if( g_alinkgw_enable && broadcast_flag)
	{
		report_wifiset_state();
	}
	
#endif
		
	printf (" %s %d  result=%s= \n", __FUNCTION__, __LINE__, result);
	websDone(wp, 200);
}
/***************************************************************/

#ifdef __CONFIG_QUICK_SET__		


static void fromgetWanConnectStatus(webs_t wp, char_t *path, char_t *query)
{
	char	date_net[10] = {0};
	char	date_line[10] = {0};
	char	date_wanType[10] = {0};
	char	date_ssid[50] = {0};
	char	value[500] = {0};
	char	err_check[10] = {0};
	int 	wan_type = 0;
	
	wan_type = wan_link_status();
	sprintf(date_line , "%s" , wan_type ==0 ? "no" : "ok");


	if(wan_type){
		if(network_tpye == 3 ){
			sprintf(date_net , "wait" );
			sprintf(date_wanType , "dhcp" );
			
		}else if(network_tpye == 2 ){
			sprintf(date_net , "ok" );
			sprintf(date_wanType , "pppoe" );

		}else if(network_tpye == 1 ){
			sprintf(date_net , "ok" );
			sprintf(date_wanType , "dhcp" );

		}else if(network_tpye == 0 ){
			sprintf(date_net , "ok" );
			sprintf(date_wanType , "static" );

		}else if(network_tpye == 4 ){
			sprintf(date_net , "ok" );
			sprintf(date_wanType , "dhcp" );
		}
	}
	else{
		sprintf(date_net , "wait" );
		sprintf(date_wanType , "dhcp" );
	}

	sprintf(date_ssid , "%s", nvram_safe_get("wl0_ssid") );

	strncat(value, "{", 1);	
	
	string_cat(value,"net",date_net);
	strncat(value, ",", 1);
	
	string_cat(value,"line",date_line);
	strncat(value, ",", 1);
	
	string_cat(value,"wanType",date_wanType);
	strncat(value, ",", 1);
	
	string_cat(value,"ssid",date_ssid);
	strncat(value, ",", 1);

	if(iplookflag(inet_addr(wp->ipaddr)) == 1 ){
		string_cat(value,"transmitter","wireless");
	}else{
		string_cat(value,"transmitter","wired");
	}
	strncat(value, ",", 1);

	
	char check[5]={0};
	snprintf(err_check , sizeof(err_check) , "%s" , nvram_safe_get("err_check"));
	if(strcmp(err_check,"11") == 0){
		sprintf(check,"%d",11);
	}
	else{
		sprintf(check,"%d",0);
	}
	string_cat(value,"error-info",check);
	strncat(value, ",", 1);


	string_cat(value,"lan-ip",nvram_safe_get(_LAN0_IP));
	
	strncat(value, "}", 1);	

	strncat(value, "\n", 1);

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, value);
	websDone(wp, 200);	

}



static void fromsaveQuickSetData(webs_t wp, char_t *path, char_t *query)
{

	char_t *go, *con_type;
	con_type = websGetVar(wp, T("wanType"), T(""));
	go = websGetVar(wp, T("GO"), T("")); 
	int sys_reboot = 0,wl_reboot = 0,wan_clone = 0;
	char*  time_zone = websGetVar(wp, T("timeZone"), T(""));
	int    i = 0 ;
	char   value_timezone[15] = {0};
	char	c_timezone;
	int		i_timezone;
	

	if(iplookflag(inet_addr(wp->ipaddr)) == 0)
	{
		char pc_mac[20];
		int netWorkSta = get_wan_onln_connstatus();
		((struct in_addr*) pc_mac)->s_addr=inet_addr(wp->ipaddr);
		if(arpioctl(SIOCGARPRT, pc_mac, NULL) == 0)
		{	
			char save_pc_mac[20]={0};
			sprintf(save_pc_mac,"%02X:%02X:%02X:%02X:%02X:%02X",
						pc_mac[4]&0XFF,
						pc_mac[5]&0XFF,
						pc_mac[6]&0XFF,
						pc_mac[7]&0XFF,
						pc_mac[8]&0XFF,
						pc_mac[9]&0XFF);
			if(strcmp(nvram_safe_get(_WAN0_HWADDR),save_pc_mac) != 0)
			{
				_SET_VALUE(_WAN0_HWADDR,save_pc_mac);
				wan_clone = 1;
			}
		}
	}
#if 1
	if(time_zone[0] != 0){
		
		//add by z10312 20150526,  
		float ftime_zone;
		ftime_zone = atof(time_zone);
		i_timezone = ftime_zone * 3600;
		
		for(i = 0 ; i < TIME_ZONES_NUMBER ; i++ ){
			if(time_zones[i].tz_offset == i_timezone ){
				sprintf(value_timezone , "%d" , time_zones[i].index );
				_SET_VALUE(_SYS_TZONE,value_timezone);
				break;
			}
		}
	}
#endif	
	
	
	int tm,old_wan_mode;
	char val[128];
	char dns3[128];
	old_wan_mode = get_wan_type();

	
	if(strcmp(con_type,"dhcp") == 0){
		char_t	*DynStatic_mtu;
		if( old_wan_mode != 2 )
		{
			sys_reboot = 1;
			_GET_VALUE(_DHCP_WAN0_MTU, DynStatic_mtu);
			_SET_VALUE(_WAN0_MTU,DynStatic_mtu);
		}
	}
	else if(strcmp(con_type,"pppoe") == 0){
	
		char_t *user_id, *pwd, *mtu, *ac, *sev, *conmode;
		char_t *idle_time, *hour_s, *min_s, *hour_e, *min_e;
		char_t *v12_time;

		char *xkjs_user_id,*xkjs_pwd;
		char tmp_xkjs_user_id[64];
		char tmp_xkjs_pwd[64];
			
		_SET_VALUE("plugplay_flag","n");//gong add
		user_id = websGetVar(wp, T("adslusername"), T("")); 
		pwd = websGetVar(wp, T("adslpwd"), T("")); 
		ac = websGetVar(wp, T("AC"), T(""));
		sev = websGetVar(wp, T("SVC"), T(""));
		conmode = websGetVar(wp, T("PCM"), T("0"));
		idle_time = websGetVar(wp, T("PIDL"), T("60"));
			
		hour_s = websGetVar(wp, T("hour1"), T("0"));
		min_s = websGetVar(wp, T("minute1"), T("0"));
		hour_e = websGetVar(wp, T("hour2"), T("0"));
		min_e = websGetVar(wp, T("minute2"), T("0"));
		v12_time = websGetVar(wp,T("v12_time"),T("0"));

		if(strcmp(nvram_safe_get(_WAN0_PPPOE_USERNAME),user_id) != 0 
			|| strcmp(nvram_safe_get(_WAN0_PPPOE_PASSWD),pwd) != 0)
		{
			sys_reboot = 1;
			
			_SAFE_GET_VALUE(_WAN0_PPPOE_USERNAME, xkjs_user_id);
			slprintf(tmp_xkjs_user_id, sizeof(tmp_xkjs_user_id), "%.*v", strlen(xkjs_user_id),xkjs_user_id);
			_SAFE_GET_VALUE(_WAN0_PPPOE_PASSWD, xkjs_pwd);
			slprintf(tmp_xkjs_pwd, sizeof(tmp_xkjs_pwd), "%.*v", strlen(xkjs_pwd),xkjs_pwd);
			if(strcmp(user_id,tmp_xkjs_user_id) == 0)
				;
			else
				_SET_VALUE(_WAN0_PPPOE_USERNAME,user_id);
			if(strcmp(pwd,tmp_xkjs_pwd) == 0)
				;
			else
				_SET_VALUE(_WAN0_PPPOE_PASSWD,pwd);
			_GET_VALUE(_PPPOE_WAN0_MTU,mtu);
			_SET_VALUE(_WAN0_PPPOE_MTU,mtu);
			_SET_VALUE(_WAN0_PPPOE_MRU,mtu);
			_SET_VALUE(_WAN0_PPPOE_AC,ac);
			_SET_VALUE(_WAN0_PPPOE_SERVICE,sev);
			_SET_VALUE(_WAN0_PPPOE_DEMAND,conmode);
			if(atoi(conmode) == PPPOE_TRAFFIC)
				_SET_VALUE(_WAN0_PPPOE_IDLETIME,idle_time);

			if(atoi(conmode) == PPPOE_BY_TIME){
				tm = atoi(hour_s)*3600 + atoi(min_s)*60;
				sprintf(val,"%d",tm);
				
				_SET_VALUE(_WAN0_PPPOE_ST,val);

				tm = atoi(hour_e)*3600 + atoi(min_e)*60;
				sprintf(val,"%d",tm);

				_SET_VALUE(_WAN0_PPPOE_ET,val);
			}
#if defined(CONFIG_CHINA_NET_CLIENT)
			_SET_VALUE(_WAN0_PPPOE_XKJX_TIME,v12_time);
#endif
			if(old_wan_mode  != 3){
				sys_reboot = 1;
			}
		}
	}else if(strcmp(con_type,"static") == 0){
		char_t	*DynStatic_mtu;
		if( old_wan_mode != 1 )
		{
			sys_reboot = 1;
			_GET_VALUE(_DHCP_WAN0_MTU, DynStatic_mtu);
			_SET_VALUE(_WAN0_MTU,DynStatic_mtu);
		}

	}


	if(sys_reboot == 1){
		_SET_VALUE("err_check","0");
		 _SET_VALUE(_WAN0_PROTO, con_type);
		_SET_VALUE("config_index","1");
	
		_SET_VALUE("wan0_check","0");
		_SET_VALUE("mode_need_switch","no");
		_SET_VALUE("wan0_connect","Connecting");
		network_tpye = 4;
	}
	

	char *ssid,*pwd,*old_ssid,*old_pwd;
	ssid = websGetVar(wp, T("ssid"), T(""));
	pwd = websGetVar(wp, T("wlpwd"), T("")); 

	char SSID_mode[16] = {0};	
	get_wl0_mode(SSID_mode);

	if(strcmp(SSID_mode,"ap") == 0)
	{
		
		//copy_wl_index_to_unindex("0");
		old_ssid = nvram_safe_get("wl0_ssid");
		old_pwd = nvram_safe_get("wl0_wpa_psk");
	}else{
		
		//copy_wl_index_to_unindex("0.1"); 
		old_ssid = nvram_safe_get("wl0.1_ssid");
		old_pwd = nvram_safe_get("wl0.1_wpa_psk");
	}
	
	if(strcmp(ssid,old_ssid) != 0 || strcmp(pwd,old_pwd) != 0)
	{
		char *wlunit, *wps_enable;
		
		//char SSID_mode[16] = {0}; 
		//get_wl0_mode(SSID_mode);
		if(strcmp(SSID_mode,"ap") == 0)
		{
			
			copy_wl_index_to_unindex("0");
		}else{
			
			copy_wl_index_to_unindex("0.1"); 
		}
		

		_GET_VALUE(WLN0_WPS_ENABLE,wps_enable);
		if(strcmp(wps_enable,"enabled") != 0)
		{
			if(strcmp(pwd,"") != 0)
			{
				_SET_VALUE(WLN0_SSID0, ssid);
				_SET_VALUE(WLN0_WEP, DISABLE);
				_SET_VALUE(WLN0_AUTH_MODE, OPEN);
				_SET_VALUE(WLN0_SECURITY_TYPE, "psk psk2"); 
				_SET_VALUE(WLN0_WPA_PSK1, pwd);
				_SET_VALUE(WLN0_ENCRYP_TYPE,"aes");
				_SET_VALUE(WLN0_REKEY_INTERVAL, "3600");
			}
			else
			{
				_SET_VALUE(WLN0_SSID0, ssid);
				_SET_VALUE(WLN0_AUTH_MODE, OPEN);
				_SET_VALUE(WLN0_WEP, DISABLE);
				_SET_VALUE(WLN0_SECURITY_TYPE,"");
				_SET_VALUE(WLN0_ENCRYP_TYPE,"");
				_SET_VALUE(WLN0_WPA_PSK1,"");
				_SET_VALUE(WLN0_ENCRYP_TYPE, "");
			}
			
			_GET_VALUE(WLN0_UNIT,wlunit);
			wl_unit(wlunit,1);	
			
			_SET_VALUE("err_check","0");
			_SET_VALUE("config_index","1");
			_SET_VALUE("wan0_check","0");
		}
		wl_reboot = 1;
	}

	set_oob_status(ssid, pwd);
change_tag = 1;
	//hqw add 
	i = 0;
	char SSID1_mode[16] = {0};	
	int wanLinkSta = get_wan_linkstatus();
	get_wl0_mode(SSID1_mode);
	if(wanLinkSta != 0 && !strcmp(SSID1_mode,"ap"))
	{
		while(check_tag != 1 && i < 10)
		{
			i++;
			cyg_thread_delay(20);
		}
	}
	//end
	change_tag = 0;
	nvram_set(_RESTORE_QUICK_SET,"0");
	
	_COMMIT();
	if(sys_reboot == 1 || wan_clone == 1)
	{
		stop_wan();
		if (wan_link_status() || wan_clone == 1)
		{
			start_wan();
		}
	}
	if(wl_reboot == 1)
	{
		_RESTART();
	}
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	if(2 ==  get_wan_onln_connstatus()){
		websWrite(wp, CONFIG_INTERNETOK);
	}else{
		websWrite(wp, CONFIG_SUCCESS);
	}
	websDone(wp, 200);	
}

#endif

