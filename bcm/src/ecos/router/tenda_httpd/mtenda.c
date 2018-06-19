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
#include  "cJSON.h"


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


#ifdef __CONFIG_TENDA_WLJB__
#define W311R_ECOS_SV "V11.13.01.01_T"
#else
#define W311R_ECOS_SV "V11.13.01.01"
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

char g_User[64] = {0},g_Pass[64] = {0};

int dns_redirect_dag = 0;//add for redirect to advance.asp by ldm 20121127

extern int check_tag;

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

//start: add by z10312  合入 厂测按键测试&LED灯测试 接口2015-01-27
extern void set_all_led_on( void );
extern void set_all_led_off( void );
//end: add by z10312  合入 厂测按键测试&LED灯测试 接口2015-01-27



//extern uint32 wps_gen_pin(char *devPwd, int devPwd_len);
/* ARP ioctl structure */
struct arp_arpentry {
	struct in_addr addr;
	char enaddr[ETHER_ADDR_LEN];
};

//roy+++

#define TPI_MAC_STRING_LEN	18	
#define TPI_IP_STRING_LEN	16		
#define TPI_IFNAME_LEN		16			

#define TPI_BUFLEN_64       64    //!< buffer length 64

typedef struct client_info{
        int in_use;                                //now in use
        unsigned char mac[TPI_MAC_STRING_LEN];     //mac address
        unsigned char ip[TPI_IP_STRING_LEN];       //ip address
	unsigned char hostname[TPI_BUFLEN_64];     //hostname
	int l2type;                                //layer2 type: wired or wireless 
	time_t time; 
	time_t interval_time;
	int limitenable;
}arp_client_info;


typedef struct static_dhcp_list
{
    int index;		//标识nvram索引
    char mac[TPI_MAC_STRING_LEN];
    char ip[TPI_IP_STRING_LEN];
    struct static_dhcp_list *next;
}Static_dhcp_list, *pStatic_dhcp_list;

struct static_dhcp_table
{
    int count;
    struct static_dhcp_list *gStatic_dhcp_list;
};


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
	{13,   "GMT-04:00" , "WPT+04:00"  ,-4*3600},       
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
static void	fromVirSerDMZ(webs_t wp, char_t *path, char_t *query);
static void 	fromVirSerUpnp(webs_t wp, char_t *path, char_t *query);
static void	fromVirSerSeg(webs_t wp, char_t *path, char_t *query);
static void	fromSysToolTime(webs_t wp, char_t *path, char_t *query);
static void 	fromalgform(webs_t wp, char_t *path, char_t *query);
static void	fromsetWanSpeed(webs_t wp, char_t *path, char_t *query);//huangxiaoli modify
static void 	formGetNAT(webs_t wp, char_t *path, char_t *query);
static void 	formSetNAT(webs_t wp, char_t *path, char_t *query);

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
static void 	fromGetSysTools(webs_t wp, char_t *path, char_t *query);
static void 	fromSetSystem(webs_t wp, char_t *path, char_t *query);
static void 	fromSysReboot(webs_t wp, char_t *path, char_t *query);
static void 	fromSysRestore(webs_t wp, char_t *path, char_t *query);

/****************************hqw add for F307*********************************************/
static void 	fromNetWorkSetupInit(webs_t wp, char_t *path, char_t *query);
static void 	fromNetWorkSetupSave(webs_t wp, char_t *path, char_t *query);
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
//产品归一化新增接口
static void	fromSetWizard(webs_t wp, char_t *path, char_t *query);
static void	fromGetWizard(webs_t wp, char_t *path, char_t *query);
static void fromSetWan(webs_t wp, char_t *path, char_t *query);
static void fromSetMacClone(webs_t wp, char_t *path, char_t *query);
static void fromGetWan(webs_t wp, char_t *path, char_t *query);
static void fromGetStatus(webs_t wp, char_t *path, char_t *query);
static void fromGetWifi(webs_t wp, char_t *path, char_t *query);
static void fromSetWifi(webs_t wp, char_t *path, char_t *query);
static void fromSetWps(webs_t wp, char_t *path, char_t *query);
static void fromGetIsHasLoginPwd(webs_t wp, char_t *path, char_t *query);
static void fromLoginOut(webs_t wp, char_t *path, char_t *query);
static int 	internet_error_check_function( void );
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
	websFormDefine(T("AdvSetLanip"), fromAdvSetLanip);
	websFormDefine(T("WizardHandle"), fromWizardHandle);
	websFormDefine(T("AdvSetMacClone"), fromAdvSetMacClone);
	websFormDefine(T("AdvSetWan"), fromAdvSetWan);
	websFormDefine(T("VirSerDMZ"), fromVirSerDMZ);
	websFormDefine(T("VirSerUpnp"), fromVirSerUpnp);
	websFormDefine(T("VirSerSeg"), fromVirSerSeg);
	websFormDefine(T("SysToolTime"), fromSysToolTime);
	websFormDefine(T("algform"), fromalgform);
	websFormDefine(T("setWanSpeed"),fromsetWanSpeed);
	websFormDefine(T("getNAT"), formGetNAT);
	websFormDefine(T("setNAT"), formSetNAT);
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
	websFormDefine(T("getSysTools"), fromGetSysTools);
	websFormDefine(T("setSystem"), fromSetSystem);
	websFormDefine(T("sysReboot"), fromSysReboot);
	websFormDefine(T("sysRestore"), fromSysRestore);
    /**************************hqw add for F307**********************************/
	websFormDefine(T("NetWorkSetupInit"), fromNetWorkSetupInit);
	websFormDefine(T("NetWorkSetupSave"), fromNetWorkSetupSave);
#ifdef __CONFIG_QUICK_SET__	
    /*************************add liuchengchi 20114-11-08***********************************/
	websFormDefine(T("getWanConnectStatus"),fromgetWanConnectStatus);
	websFormDefine(T("saveQuickSetData"),fromsaveQuickSetData);		
    /*************************************************************************/
#endif	

#ifdef __CONFIG_AL_SECURITY__
	websFormDefine(T("InsertWhite"), formalInsertWhite);
#endif
//产品归一化新增接口	
	websFormDefine(T("setWizard"), fromSetWizard);
	websFormDefine(T("getWizard"), fromGetWizard);
	websFormDefine(T("setWAN"), fromSetWan);
	websFormDefine(T("getWAN"), fromGetWan);
	websFormDefine(T("getStatus"), fromGetStatus);
	websFormDefine(T("setWifi"), fromSetWifi);
	websFormDefine(T("getWifi"), fromGetWifi);
	websFormDefine(T("setWifiWps"), fromSetWps);
	websFormDefine(T("setMacClone"), fromSetMacClone );
	
	websFormDefine(T("getIsHasLoginPwd"), fromGetIsHasLoginPwd);
	websFormDefine(T("loginOut"), fromLoginOut);

}

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

static int internet_error_check_function( void )
{
	char wan_mode_check_verdict[20] = {0};	
	int check_result = 100 ;
	int wanLink = 0;
	int errorInfo = 0;
	int netCheck = DHCPMODE;
	
	/*network status*/
	wan_link_check();
	int internetStat = get_wan_onln_connstatus();		
	int conStat = get_wan_connstatus();
	int wanConnTyp = get_wan_type();
	int conType = wanConnTyp ;	
	int wanLinkSta = get_wan_linkstatus();
	int netWorkSta = get_wan_onln_connstatus();

	snprintf(wan_mode_check_verdict,sizeof(wan_mode_check_verdict) , "%s",nvram_safe_get("wan_mode_check_verdict"));

	if(strcmp(wan_mode_check_verdict , "pppoe") == 0 ){
		netCheck = PPPOEMODE;
	}else if(strcmp(wan_mode_check_verdict , "static") == 0 ){
		netCheck = STATICMODE;
	}else{
		netCheck = DHCPMODE; 
	}
	
	char SSID1_mode[16] = {0};	
	get_wl0_mode(SSID1_mode);
	
	if(wanLinkSta == 0 && !strcmp(SSID1_mode,"ap")) 	
		wanLink = 0;
	else		
		wanLink = 1;

	/*wan pppoe check*/
	char wan_ppp_chk[8] = {0};
	sprintf(wan_ppp_chk,"%s",nvram_safe_get("err_check"));
	if(strcmp(wan_ppp_chk,"11") == 0)
		errorInfo = 11;
	else
		errorInfo = 4;

	if(wanLinkSta == 0 && !strcmp(SSID1_mode,"ap"))
	{
		errorInfo = 0;
	}
	else if(wanConnTyp == 3)			//pppoe
	{
		if(!strcmp(wan_ppp_chk,"5"))
				errorInfo = 3;
		else if(!strcmp(wan_ppp_chk,"7"))
				errorInfo = 1;
		else if(!strcmp(wan_ppp_chk,"2"))
				errorInfo = 2;
		else if(!strcmp(wan_ppp_chk,"3"))
				errorInfo = 4;
	}	

	if(internetStat != 2 )	//~{N4A*Mx~}
	{
		if(conType == DHCPMODE) 		//dhcp
		{
			if(errorInfo == 11 && conStat != 2 ){
				check_result =  122;
			}
			else if(netCheck == PPPOEMODE){ 
				//should be pppoe
				check_result =  126;				
			}
			else if(conStat != 2 && netCheck != PPPOEMODE) {
			
				//DHCP~{?M;'6KN^7(;qH!5=~}IP~{#,~}WAN~{R2N4Tx<l2b5=~}PPPOE~{7~NqFw~}
				if(internetStat == 1)
				{
					check_result =   127;
				}	
			} else if(conStat == 2 && netCheck != PPPOEMODE) {
					//DHCP~{?M;'6K;qH!5=AK~}IP
					check_result =   128;
			}	
		}else { 
		
			if(conType == PPPOEMODE) {

				if(netCheck == STATICMODE)
				{
					check_result =   127;
				}else if(netCheck == DHCPMODE){
					//DHCP~{D#J=2;LaJ>~}
					check_result =   999;
				}
		
				if (conStat != 2) 
				{					
					if (errorInfo == 1)
						check_result =   112;
					else if(errorInfo == 2)
						check_result =   123;
					else if(errorInfo == 3) 						
						check_result =   124;
				} else if(conStat == 2) {
					check_result =   125;
				}

			}
		}
	}

	if(netWorkSta == 2){
		return 100;
	}
	else if(wanLinkSta == 0){
		return 121;
	}
	else if(wan_mode_check_verdict[0] == '\0' ){		//~{V;SP;V843v3'2E4+JdUo6O4zBk#,WwN*D,HO9JUO4&@m~},~{2;OTJ>4mNsT-Rr~}
		return 999;
	}
	return check_result ;

}

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


	wan_status = get_wan_connstatus();
	wan_onln_status = get_wan_onln_connstatus();
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
	else if(DHCPMODE == wanmod)//DHCP模式
	{
		char_t  *DynStaticMtu;
		
		DynStaticMtu = websGetVar(wp, T("dynamicMTU"), T("1500"));

		 _SET_VALUE(_WAN0_MTU,DynStaticMtu);
	}
	else if(PPPOEMODE == wanmod)//PPPOE 模式
	{//pppoe
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

		//wan模式
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
		//wan模式
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

		//wan??~{D#J=~}
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


/*
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
		//
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
	{//pppoe
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
		
		
	     //	_SET_VALUE(_WAN0_PPPOE_USERNAME,user_id);
	     //	_SET_VALUE(_WAN0_PPPOE_PASSWD,pwd);
	//	_SET_VALUE(_WAN0_PPPOE_MTU,mtu);
	//	_SET_VALUE(_WAN0_PPPOE_MRU,mtu);//important for mtu
	//	_SET_VALUE(_WAN0_PPPOE_AC,ac);
	//	_SET_VALUE(_WAN0_PPPOE_SERVICE,sev);
		//conmode:0,auto;1,traffic;2,hand;3,time
	//	_SET_VALUE(_WAN0_PPPOE_DEMAND,conmode);
		//to be added idle_time
		if(atoi(conmode) == PPPOE_TRAFFIC)
	//		_SET_VALUE(_WAN0_PPPOE_IDLETIME,idle_time);

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
//		_SET_VALUE(_WAN0_PPPOE_XKJX_TIME,v12_time);
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

		//wan??~{D#J=~}
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
		//wan??~{D#J=~}
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

		//wan??~{D#J=~}
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
//	_SET_VALUE("err_check","0");
//	set_wan_str(wanmod);
	if(strcmp(mac,"NULL") != 0)
//		_SET_VALUE(_WAN0_HWADDR,mac);

#if 0//huangxiaoli modify	
	set_wan_mac(wp->ipaddr);
#endif
//	_SET_VALUE("config_index","1");

//	_SET_VALUE("wan0_check","0");
	//_SET_VALUE("onlncheck","no");
//	_SET_VALUE("mode_need_switch","no");
	
//	_RESTART_ALL();
	
	//cyg_thread_delay(100);
//#ifdef __CONFIG_TENDA_MULTI__
//	websRedirect(wp, T("/system_status.asp"));
//#else
//	websRedirect(wp, T("/wan_connected.asp"));
//#endif
}
*/


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

static int prase_webstr2virtualServer(char *old_value, char *new_value)
{
	/*   
	*web string:   192.168.0.100;53;7008
	*                   192.168.0.21;21;22;Both
	*nvram value: 7008-7008>192.168.0.100:53-53,tcp,on
	        		       56-56>192.168.0.215:56-56,tcp/udp,on
	*/
	char *arglists[4] = {NULL};
	if (! (str2arglist(old_value, arglists, ';', 4) == 4)) {
		return 0;
	}
	
	if (!strcmp(arglists[3], "TCP"))
	{
		strcpy(arglists[3], "tcp");	
	}
	else if (!strcmp(arglists[3], "UDP"))
	{
		strcpy(arglists[3], "udp");
	}
	else
	{
		strcpy(arglists[3], "tcp/udp");
	}
	
	
sprintf(new_value, "%s-%s>%s:%s-%s,%s,%s",
			arglists[2], arglists[2], arglists[0], arglists[1], arglists[1], arglists[3], "on");

	printf("===%s===new_value:%s\n",__FUNCTION__, new_value);
		
				
	return 1;		
}

static void clear_all_virtual_server()
{
	int i = 0;
	
	for (i = 0; i < VTS_MAX_NUM_1; ++i)
	{
		_SET_VALUE(_FW_FORWARD_PORT(i), "");
	}
}
extern struct static_dhcp_table static_dhcp_config_get(void);
static void formGetNAT(webs_t wp, char_t *path, char_t *query)
{
	int i = 0 , j = 0;
	int in_port = 0;
	int ex_port = 0;  
	char ip[32] = {0};
	char porto[32] = {0};
	char *out = NULL;
	cJSON *obj = NULL;
	cJSON *root = NULL;
	cJSON *array = NULL;
	char *value = NULL;
	char *arglists[9]= {NULL};
	char ddns_value[256] = {0};
	struct client_info  *clients_list = NULL ;
	int client_num = 0;
	char *host_name = NULL;
	char static_value[256] = {0};
	struct static_dhcp_table	Static_dhcp_table;
	struct static_dhcp_list		*static_dhcp_list_node;


	root = cJSON_CreateObject();
	
	//LAN
	_GET_VALUE(_LAN0_IP, value);
	cJSON_AddStringToObject(root, "lanIP", value);
	
	_GET_VALUE(_LAN0_NETMASK, value);
	cJSON_AddStringToObject(root, "lanMask", value);

	_GET_VALUE(_LAN0_DHCPD_START, value);
	cJSON_AddStringToObject(root, "lanStartIP", value);
	_GET_VALUE(_LAN0_DHCPD_END, value);
	cJSON_AddStringToObject(root, "lanEndIP", value);
	_GET_VALUE(_LAN0_DHCPD_EN, value);
	cJSON_AddStringToObject(root, "lanDhcpEn", (0 == strcmp(value, "dhcp"))?"true":"false");


	clients_list = (struct client_info *)malloc(255 * sizeof(arp_client_info));
	if(clients_list != NULL )
	{
		memset(clients_list, 0x0 , sizeof(arp_client_info) * 255);
		client_num = get_all_client_info(clients_list , 255);
	} 

	cJSON_AddItemToObject(root, "staticList", array = cJSON_CreateArray());
	Static_dhcp_table = static_dhcp_config_get();
	static_dhcp_list_node = Static_dhcp_table.gStatic_dhcp_list;
	for ( i = 1; i <= Static_dhcp_table.count; ++i )
	{
		if(static_dhcp_list_node == NULL ){
			break;
		}
		
		host_name = get_remark(static_dhcp_list_node->mac);
		/*mac 转换为小写字符串格式*/
		qosMacToLower(static_dhcp_list_node->mac);
		
		if( host_name == NULL ){
			for( j = 0 ; j < client_num ; j++){
				/*mac 转换为小写字符串格式*/
				qosMacToLower(clients_list[j].mac);
				if(strcmp(clients_list[j].mac ,(static_dhcp_list_node->mac) ) == 0 ){
					host_name = &(clients_list[j].hostname[0]);
				}
			}
		}
		cJSON_AddItemToArray(array, obj = cJSON_CreateObject());
		cJSON_AddStringToObject(obj, "staticIP", (static_dhcp_list_node->ip));
		cJSON_AddStringToObject(obj, "staticMac", (static_dhcp_list_node->mac));
		cJSON_AddStringToObject(obj, "staticRemark", host_name);
		static_dhcp_list_node = static_dhcp_list_node->next;

	}
	free(clients_list);

	//Port Forwarding
	cJSON_AddItemToObject(root, "portList", array = cJSON_CreateArray());
	for ( i = 0; i < VTS_MAX_NUM_1; ++i)
	{
		/*56-56>192.168.0.215:56-56,tcp/udp,on
		 *"53-80>192.168.0.100:53-80,tcp,on"
		 *Parse wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1[:,]proto[:,]enable[:,]desc 
		 */
		_GET_VALUE(_FW_FORWARD_PORT(i), value);
		if (strlen(value) < 12){
			continue;
		}
		
		if (4 != sscanf(value, "%d-%*d>%[^:]:%d-%*d,%[^,],%*s", &ex_port, ip, &in_port, porto)) {
			continue;
		}
		cJSON_AddItemToArray(array, obj = cJSON_CreateObject());
		cJSON_AddStringToObject(obj, _VIRTUAL_SRV_IP, ip);
		cJSON_AddNumberToObject(obj, _VIRTUAL_SRV_IN_PORT, in_port);
		cJSON_AddNumberToObject(obj, _VIRTUAL_SRV_EX_PORT, ex_port);
		
		if (!strcmp(porto, "tcp/udp"))
		{
			strcpy(porto, "both");
		}
		cJSON_AddStringToObject(obj, _VIRTUAL_SRV_PORTO, porto);
	}

	//DDNS
	cJSON_AddItemToObject(root, "ddns", obj = cJSON_CreateObject());

	_GET_VALUE(_DDNS_ENABLE, value);
	if (0 != strcmp(value, "0"))
		cJSON_AddStringToObject(obj, "ddnsEn", "true");
	else
		cJSON_AddStringToObject(obj, "ddnsEn", "false");

	switch(atoi(value))
	{
		case 1:
			cJSON_AddStringToObject(obj, "ddnsServiceName", "oray.net");
			break;
		case 2:
			cJSON_AddStringToObject(obj, "ddnsServiceName", "dyndns.org");
			break;
		case 3:
			cJSON_AddStringToObject(obj, "ddnsServiceName", "88ip.com");
			break;
		case 4:
			cJSON_AddStringToObject(obj, "ddnsServiceName", "freedns.afraid.org");
			break;
		case 5:
			cJSON_AddStringToObject(obj, "ddnsServiceName", "zoneedit.com");
			break;
		case 6:
			cJSON_AddStringToObject(obj, "ddnsServiceName", "no-ip.com");
			break;
		case 7:
			cJSON_AddStringToObject(obj, "ddnsServiceName", "3322.org");
			break;
		default:
			cJSON_AddStringToObject(obj, "ddnsServiceName", "dyndns.org");
			break;
	}
	
	_GET_VALUE(_DDNS_SET1,value);
	strncpy(ddns_value,value,sizeof(ddns_value));
	if(str2arglist(ddns_value, arglists, ';', 9) == 9)
	{
		cJSON_AddStringToObject(obj, "ddnsUser", arglists[3]);
		cJSON_AddStringToObject(obj, "ddnsPwd", arglists[4]);
		cJSON_AddStringToObject(obj, "ddnsServer", arglists[2]);
	}
	else
	{
		cJSON_AddStringToObject(obj, "ddnsUser", "");
		cJSON_AddStringToObject(obj, "ddnsPwd", "");
		cJSON_AddStringToObject(obj, "ddnsServer", "");
	}
	
	_GET_VALUE(_DDNS_STATUS, value);
	cJSON_AddStringToObject(obj, "ddnsStatus", value);
	
	//DMZ Host
	cJSON_AddItemToObject(root, "dmz", obj = cJSON_CreateObject());
	
	_GET_VALUE(_FW_DMZ_IPADDR_EN, value);
	if (0 == strcmp(value, "1"))
		cJSON_AddStringToObject(obj, "dmzEn", "true");
	else
		cJSON_AddStringToObject(obj, "dmzEn", "false");
	
	_GET_VALUE(_FW_DMZ_IPADDR, value);
	cJSON_AddStringToObject(obj, "dmzHostIP", value);

	//UPNP
	_GET_VALUE(_FW_UPNP_EN, value);
	if (0 == strcmp(value, "1"))
		cJSON_AddStringToObject(root, "upnpEn", "true");
	else
		cJSON_AddStringToObject(root, "upnpEn", "false");

	out = cJSON_Print(root);
	cJSON_Delete(root);
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWriteLongString(wp, out);
	//websWrite(wp,T("%s"),out);
	free(out);
	websDone(wp, 200);

	return ;
}

extern int sscanfArglistConfig(const char *value, const char key , char **argc, int count);
extern void freeArglistConfig(char **argc, int count);
extern int static_dhcp_config_delete_all( void );
static void formSetNAT(webs_t wp, char_t *path, char_t *query)
{
	char_t *portList, *ddnsEn, *ddnsServiceName, *ddnsUser, *ddnsPwd, *ddnsHostName;
	char_t *dmzEn, *dmzHostIP, *upnpEn;
	char value[128] = {0};
	char_t *serName = NULL;
	char listBuf[512] = {0};
	char *arglist[20] = {NULL};
	char *argc[6] = {0}; 
	char old_value[64] = {0};
	char new_value[64] = {0};
	int i = 0;
	char *p = NULL;
	char *staticList ;
	int  count = 0;
	
	
	portList = websGetVar(wp, "portList", "");
	ddnsEn = websGetVar(wp, "ddnsEn", "");
	ddnsServiceName = websGetVar(wp, "ddnsServiceName", "");
	ddnsUser = websGetVar(wp, "ddnsUser", "");
	ddnsPwd = websGetVar(wp, "ddnsPwd", "");
	ddnsHostName = websGetVar(wp, "ddnsServer", "");
	dmzEn = websGetVar(wp, "dmzEn", "");
	dmzHostIP = websGetVar(wp, "dmzHostIP", "");
	upnpEn = websGetVar(wp, "upnpEn", "");
	staticList = websGetVar(wp, "staticList", "");

	
	p = staticList;
	
	str2arglist(p, arglist, '\n', 20);


	static_dhcp_config_delete_all();

	for( i = 0 ; i < DHCPD_STATIC_LEASE_NU ; i++){
		if (arglist[i] == NULL || strlen(arglist[i]) < 10)
			continue;
		
		count = sscanfArglistConfig(arglist[i],'\t' ,  argc, 3);

		if(count == 3){
			qosMacToLower(argc[1]);
			static_dhcp_config_add(argc[1],argc[0]);
			add_remark(argc[1],argc[2]);
		}
		freeArglistConfig(argc,count);
	}

	
	//Port Forwarding
	/*
	 *   web  string:  "192.168.0.100;53;7008~192.168.0.10.;21;65535~..."
	 *   nvram value: "7008-7008>192.168.0.100:53-53,tcp/udp,on","65535-65535>192.168.0.100:21-21,tcp/udp,on"......
	 */
	clear_all_virtual_server();
	
	i = 0;
	strcpy(listBuf, portList);
	
	for (p = strtok(listBuf, "~"); p; p = strtok(NULL, "~"))
	{
		if (i >= VTS_MAX_NUM_1)
			break;
		
		if (strlen(p) > 10)
		{
			printf("---->>>%s\n", p);
			strcpy(old_value, p);
			if (!prase_webstr2virtualServer(old_value, new_value))
				continue;

			_SET_VALUE(_FW_FORWARD_PORT(i),new_value);

			++i;
		}
	}
	
	//DDNS
	if (0 == strcmp(ddnsEn, "true")) 
	{
		if (0 == strcmp(ddnsServiceName, "dyndns.org"))
		{
			serName= DYNDNS_NAME;
			_SET_VALUE(_DDNS_ISP,"2");
			_SET_VALUE(_DDNS_ENABLE,"2");
		}
		else if (0 == strcmp(ddnsServiceName, "no-ip.com"))
		{
			serName = NOIP_NAME;
			_SET_VALUE(_DDNS_ISP,"6");
			_SET_VALUE(_DDNS_ENABLE,"6");
		}
		else if (0 == strcmp(ddnsServiceName, "3322.org"))
		{
			serName = QDNS_NAME;
			_SET_VALUE(_DDNS_ISP,"7");
			_SET_VALUE(_DDNS_ENABLE,"7");
		}
		else
		{
			;
		}

		/*1;dyndns;dyndnsyue.com;admin;admin;;;;60*/
		memset(value,0,sizeof(value));
		snprintf(value,sizeof(value),"%s;%s;%s;%s;%s;%s;%s;%s;%s",
			"1", serName, ddnsHostName, ddnsUser, ddnsPwd, "", "", "", "60");
		
		_SET_VALUE(_DDNS_SET1,value);
		_SET_VALUE(_DDNS_HOST_NAME,ddnsHostName);
	} 
	else 
	{
		_SET_VALUE(_DDNS_ENABLE,"0");
	}

	//DMZ
	if (0 == strcmp(dmzEn, "true"))
		_SET_VALUE(_FW_DMZ_IPADDR_EN, "1");
	else
		_SET_VALUE(_FW_DMZ_IPADDR_EN, "0");
	_SET_VALUE(_FW_DMZ_IPADDR, dmzHostIP);

	//UPNP
	if (0 == strcmp(upnpEn, "true"))
		_SET_VALUE(_FW_UPNP_EN, "1");
	else
		_SET_VALUE(_FW_UPNP_EN, "0");

	_COMMIT();

	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWrite(wp, T("%s"), "{\"errCode\":\"0\"}");
	websDone(wp, 200);
	
	_RESTART_ALL();	

	return ;
}


void get_lan_dns(char *dns1_old, char * dns2_old)
{

	char str_temp[128];
	strncpy(str_temp, nvram_safe_get(_LAN0_DNS), sizeof(str_temp));
	if( strstr(str_temp, " "))
	{
		sscanf(str_temp, "%s %s", dns1_old, dns2_old);
	}
	else 
	{
		sscanf(str_temp, "%s", dns1_old);
		dns2_old[0] = 0;
	}
	//printf(" %s %d dns1_old=%s=  dns2_old=%s=\n", __FUNCTION__, __LINE__, dns1_old, dns2_old);
}

void normal_get_wifi_mode(char *wifimode, int len)
{
	
	if(!wifimode)
	{
		return;
	}
	if( nvram_match(WLN0_N_MODE, "-1")) //11bgn
	{
		strncpy(wifimode, "bgn", len);
	}
	else if (nvram_match(WLN0_G_MODE, "1"))//11bg
	{
		strncpy(wifimode, "bg", len);
	}
	else if (nvram_match(WLN0_G_MODE, "0"))//11b
	{
		strncpy(wifimode, "b", len);
	}
	else if(nvram_match(WLN0_G_MODE, "2"))//11g
	{
		strncpy(wifimode, "g", len);
	}
	
}
static void fromGetSysTools(webs_t wp, char_t *path, char_t *query)
{
	int wanType = 0;
	char *out = NULL;
	char *value = NULL;
	cJSON *root = NULL;
	cJSON *obj = NULL;
	char str_temp[128] = {0};
	char dns1[32], dns2[32];
	
	root = cJSON_CreateObject();

	//WAN
	cJSON_AddItemToObject(root, "wan", obj = cJSON_CreateObject());

	_GET_VALUE(_WAN0_PPPOE_AC, value);
	if (0 == strcmp(value, ""))
		value = "default";
	cJSON_AddStringToObject(obj, "wanServerName", value);
	
	_GET_VALUE(_WAN0_PPPOE_SERVICE, value);
	if (0 == strcmp(value, ""))
		value = "default";
	cJSON_AddStringToObject(obj, "wanServiceName", value);
	
	_GET_VALUE(_WAN0_PROTO, value);
	cJSON_AddStringToObject(obj, "wanType", value);

	_GET_VALUE(_WAN0_IPADDR,value);
	cJSON_AddStringToObject(obj, "wanIP", value);
	
	_GET_VALUE(_WAN0_NETMASK,value);
	cJSON_AddStringToObject(obj, "wanMask", value);

	/*wan mtu*/
	wanType = get_wan_type();
	switch(wanType)
	{
		case PPPOEMODE:
			_GET_VALUE(_WAN0_PPPOE_MTU, value);
			break;
		case PPPOEMODE2:
			_GET_VALUE(_WAN0_PPPOE2_MTU, value);
			break;
		case L2TPMODE:
			_GET_VALUE(_WAN0_l2TP_MTU, value);
			break;
		case PPTPMODE:
		case PPTPMODE2:
			_GET_VALUE(_WAN0_PPTP_MTU, value);
			break;
		case STATICMODE:
		case DHCPMODE:
		case C8021XMODE:
		default:
			_GET_VALUE(_WAN0_MTU, value);
			break;
	}
	cJSON_AddStringToObject(obj, "wanMTU", value);
	cJSON_AddStringToObject(obj, "wanMTUCurrent", value);

	/*wan speed*/
	memset(str_temp, 0x0, sizeof(str_temp));
	_GET_VALUE("wan_speed", value);
	if(!strcmp(value,"0"))
		strcpy(str_temp,"Auto");
	else if(!strcmp(value,"1"))
		strcpy(str_temp,"-10");
	else if(!strcmp(value,"2"))
		strcpy(str_temp,"10");
	else if(!strcmp(value,"3"))
		strcpy(str_temp,"-100");
	else if(!strcmp(value,"4"))
		strcpy(str_temp,"100");
	cJSON_AddStringToObject(obj, "wanSpeed", str_temp);

	_GET_VALUE(_WAN0_IFNAME, value);
	int speed = ifr_link_speed(value);
	if(speed  == -1)
		strcpy(str_temp,"Auto");
	else if(speed  == 0)
		strcpy(str_temp,"-10");
	else if(speed  == 1)
		strcpy(str_temp,"10");
	else if(speed  == 2)
		strcpy(str_temp,"-100");
	else if(speed  == 3)
		strcpy(str_temp,"100");
	cJSON_AddStringToObject(obj, "wanSpeedCurrent", str_temp);

	//MAC
	cJSON_AddItemToObject(root, "mac", obj = cJSON_CreateObject());

	
	//add by z10312  优化适配mac 克隆部分 20150605	
	char *macClone = NULL;
	_SAFE_GET_VALUE(_MACCLONEMODE, macClone);
	cJSON_AddStringToObject(obj, "macClone", macClone);

	/*factory mac*/
	char fmmac[6]={0,0,0,0,0,0};
	iflib_getifhwaddr("br0",fmmac);
	sprintf(str_temp,"%02X:%02X:%02X:%02X:%02X:%02X",
				fmmac[0]&0XFF,
				fmmac[1]&0XFF,
				fmmac[2]&0XFF,
				fmmac[3]&0XFF,
				fmmac[4]&0XFF,
				fmmac[5]&0XFF);	
	cJSON_AddStringToObject(obj, "macRouter", str_temp);

	/*PC mac*/
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
	cJSON_AddStringToObject(obj, "macHost", mm);

	/*current mac*/ 
	char *u = NULL;
	_SAFE_GET_VALUE("wl0_mode", u);
	if(strcmp(u,"sta")==0)
		_SAFE_GET_VALUE("wl0.1_hwaddr", value);
	else
		_SAFE_GET_VALUE(_WAN0_HWADDR, value);	
	cJSON_AddStringToObject(obj, "macCurrentWan", value);	
	
	//WIFI
	cJSON_AddItemToObject(root, "wifi", obj = cJSON_CreateObject());
	
	/*wifi mode*/
	memset(str_temp, 0x0, sizeof(str_temp));
	normal_get_wifi_mode(str_temp, sizeof(str_temp));
	cJSON_AddStringToObject(obj, "wifiMode", str_temp);
	
	/*wifi channel*/
	_GET_VALUE(WLN0_CHANNEL1, value);
	if (0 == strcmp(value, "0"))
		cJSON_AddStringToObject(obj, "wifiChannel", "Auto");
	else
		cJSON_AddStringToObject(obj, "wifiChannel", value);
	
	memset(str_temp, 0x0, sizeof(str_temp));
	get_cur_channel(str_temp);
	cJSON_AddStringToObject(obj, "wifiChannelCurrent", str_temp);////////mark
	
	
	/*wifi bandwidth*/	
	_GET_VALUE(WLN0_HT_BW1, value);
	if(!strcmp(value,"1"))
	{
		cJSON_AddStringToObject(obj, "wifiBandwidth", "40");
	}
	else if(!strcmp(value,"0"))
	{
	
		_GET_VALUE(WLN0_HT_BW_FAKE_AUTO, value);
		if(!strcmp(value,"1"))
		{
			cJSON_AddStringToObject(obj, "wifiBandwidth", "Auto");//页面显示为20/40 
		}
		else
		{
			cJSON_AddStringToObject(obj, "wifiBandwidth", "20");
		}
	}
	
	memset(str_temp, 0x0, sizeof(str_temp));
	get_cur_bandwidth(str_temp);
	cJSON_AddStringToObject(obj, "wifiBandwidthCurrent", str_temp);////////mark


	//LAN
	cJSON_AddItemToObject(root, "lan", obj = cJSON_CreateObject());

	char *lan_ip = NULL;
	_GET_VALUE(_LAN0_IP, lan_ip);
	cJSON_AddStringToObject(obj, "lanIP", lan_ip);
		
	_GET_VALUE(_LAN0_NETMASK, value);
	cJSON_AddStringToObject(obj, "lanMask", value);
	_GET_VALUE(_LAN0_DHCPD_EN, value);
	if (0 == strcmp(value, "dhcp"))
		cJSON_AddStringToObject(obj, "dhcpEn", "true");
	else
		cJSON_AddStringToObject(obj, "dhcpEn", "false");
							
								
	//根据归一化风格适配 lan dns模块
	get_lan_dns(dns1, dns2);
	cJSON_AddStringToObject(obj, "lanDns1", dns1);
	cJSON_AddStringToObject(obj, "lanDns2", dns2);
	
	
	//Remote WEB
	cJSON_AddItemToObject(root, "remoteWeb", obj = cJSON_CreateObject());
	
	_GET_VALUE(_SYS_RM_EN, value);
	if (0 == strcmp(value, "0"))
		cJSON_AddStringToObject(obj, "remoteWebEn", "false");
	else
		cJSON_AddStringToObject(obj, "remoteWebEn", "true");
	
	_GET_VALUE(_SYS_RM_IP, value);
	cJSON_AddStringToObject(obj, "remoteWebIP", value);

	if (0 == strcmp(value, "") || strlen(value) < 7)
		cJSON_AddStringToObject(obj, "remoteWebType", "any");
	else
		cJSON_AddStringToObject(obj, "remoteWebType", "specified");
	
	_GET_VALUE(_SYS_RM_PORT, value);
	cJSON_AddStringToObject(obj, "remoteWebPort", value);

	//Date & Time
	cJSON_AddItemToObject(root, "sysTime", obj = cJSON_CreateObject());
	
	_GET_VALUE(_SYS_TZONE, value);
	cJSON_AddStringToObject(obj, "sysTimeZone", value);

	struct tm *cur_time_tm;
	time_t  cur_time_t = time(0);
	cur_time_tm = localtime(&cur_time_t);
	memset(str_temp, 0x0, sizeof(str_temp));
	snprintf(str_temp,sizeof(str_temp),"%d-%02d-%02d %02d:%02d:%02d" , 1900+cur_time_tm->tm_year ,cur_time_tm->tm_mon+1 , 
		cur_time_tm->tm_mday, cur_time_tm->tm_hour , cur_time_tm->tm_min , cur_time_tm->tm_sec);
	cJSON_AddStringToObject(obj, "sysTimecurrentTime", str_temp);

	_GET_VALUE(_SYS_NTPTYPE,value);
	cJSON_AddStringToObject(obj, "sysTimeSntpType", value);

	if(get_sntp_update_time_success() != 1){	
		cJSON_AddStringToObject(obj,"internetState","false");
	}else{
		cJSON_AddStringToObject(obj,"internetState","true");
	}

	//Firmware
	cJSON_AddItemToObject(root, "firmware", obj = cJSON_CreateObject());
	
	//web password 
	char  haspwd[16];
	if (strcmp(nvram_safe_get("http_defaultpwd1"), "0"))
	{
		sprintf(haspwd, "true");							
	}
	else
	{
		sprintf(haspwd, "false");	
	}
	cJSON_AddStringToObject(obj, "hasPwd", haspwd);
	
	
	memset(str_temp, 0x0, sizeof(str_temp));
#if defined(__CONFIG_WEB_VERSION__)
	sprintf(str_temp, "%s_%s", W311R_ECOS_SV,__CONFIG_WEB_VERSION__);
#else
	sprintf(str_temp, "%s", W311R_ECOS_SV);
#endif
	cJSON_AddStringToObject(obj, "firmwareVision", str_temp);

	//auto reboot
	_GET_VALUE("restart_enable", value);
	if (0 == strcmp(value, "enable"))
		cJSON_AddStringToObject(obj, "firmwareAutoMaintenanceEn", "true");
	else
		cJSON_AddStringToObject(obj, "firmwareAutoMaintenanceEn", "false");

	out = cJSON_Print(root);
	cJSON_Delete(root);
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWriteLongString(wp, out);
	//websWrite(wp,T("%s"),out);
	free(out);
	websDone(wp, 200);

	return ;
}

static void fromSetSystem(webs_t wp, char_t *path, char_t *query)
{
	char_t *new_password,*old_password;
	char_t *wan_type, *wan_server_name, *wan_service_name;
	char_t *wan_mtu, *wan_mac,  *macClone, *wan_speed;
	char_t *wifiMode, *wifiChannel, *wifiBandwidth;
	char_t *dhcpEn, lanDns1[32], lanDns2[32], dns1_old[32],dns2_old[32], *dns_temp;
	char_t *remoteWebEn, *remoteWebType, *remoteWebIP, *remoteWebPort;
	char_t *auto_reboot_en, *timeZone;
	
	int pwd_temp = -1;
	int i;
	int wan_proto = 0;
	int speed;
	char str_temp[128] = {0};
	char *value = NULL;
	int reboot_temp = 0;
	int errCode = 0;
	int restart_temp = 0;
	int wl_temp = 0;
	
	wan_type = websGetVar(wp, T("wanType"), "");/////////////////mark 
	wan_mtu = websGetVar(wp, T("wanMTU"), "");
	wan_speed = websGetVar(wp, T("wanSpeed"), "");
	dhcpEn = websGetVar(wp, T("dhcpEn"), "");

	timeZone = websGetVar(wp, T("sysTimeZone"), "57");
	auto_reboot_en = websGetVar(wp, T("firmwareAutoMaintenanceEn"), "false");


	
	//add by z10312 20150608  优化设置密码部分 
	new_password = websGetVar(wp, T("newPwd"), "");
	old_password = websGetVar(wp, T("oldPwd"), "");
		
	if(strcmp(old_password,"") == 0 && strcmp(new_password,"") == 0)
	{
		
	}
	else if(strcmp(old_password,"") != 0 && strcmp(new_password,"") == 0)
	{
		
		if(strcmp(nvram_safe_get(HTTP_PASSWD), old_password) == 0)
		{
			_SET_VALUE(HTTP_PASSWD, new_password);
			_SET_VALUE("http_defaultpwd1", "0");
			strncpy(g_Pass,new_password,sizeof(g_Pass));
			pwd_temp = 1;  //这种情况属于删除密码, 页面 errCode 返回 0
		}
		else
		{
			pwd_temp = 0;
		}
	}
	else if(strcmp(old_password,"") == 0 && strcmp(new_password,"") != 0)
	{
		_SET_VALUE(HTTP_PASSWD, new_password);
		pwd_temp = 1;
		_SET_VALUE("http_defaultpwd1", "1");
		strncpy(g_Pass,new_password,sizeof(g_Pass));
		errCode = 101; //修改密码成功
	}
	else
	{
		if(strcmp(nvram_safe_get(HTTP_PASSWD),old_password) == 0)
		{
			_SET_VALUE(HTTP_PASSWD, new_password);
			pwd_temp = 1;
			errCode = 101; //修改密码成功
			_SET_VALUE("http_defaultpwd1", "1");
			strncpy(g_Pass,new_password,sizeof(g_Pass));
		}
		else
		{
			pwd_temp = 0;
		}
	}					
	
	
	if(pwd_temp == 0)
	{		
		errCode = 2;
		goto webdone;
	}
	else if(pwd_temp == 1)  //修改密码成功
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
	}
	
	
	
	//add by z10312  优化适配mac 克隆部分 20150605	
	wan_mac = websGetVar(wp, T("wanMAC"), T("")); 
	if(strcmp(wan_mac,"") != 0 && strcmp(wan_mac,nvram_safe_get(_WAN0_HWADDR)) != 0)
	{
		_SET_VALUE(_WAN0_HWADDR,wan_mac);
		restart_temp = 1;  
	}	
	macClone = websGetVar(wp, T("macClone"), T("")); 
	if(strcmp(macClone,"") != 0 && strcmp(macClone,nvram_safe_get(_MACCLONEMODE)) != 0)
	{
		_SET_VALUE(_MACCLONEMODE,macClone);
		
	}
	
	
	
	
	//Wan Parameters  ,页面会做拨号模式判断
	wan_server_name = websGetVar(wp, T("wanServerName"), "");/////mark
	wan_service_name = websGetVar(wp, T("wanServiceName"), "");////mark
	if ( !nvram_match(_WAN0_PPPOE_AC,  wan_server_name))
	{
		_SET_VALUE(_WAN0_PPPOE_AC, wan_server_name);
		restart_temp = 1;
	}
	if ( !nvram_match(_WAN0_PPPOE_SERVICE,  wan_service_name))
	{
		_SET_VALUE(_WAN0_PPPOE_SERVICE, wan_service_name);
		restart_temp = 1;
	}	
		
		
	/*wan mtu*/
	wan_proto = get_wan_type();
	switch(wan_proto)
	{
		case PPPOEMODE:
			_SET_VALUE(_PPPOE_WAN0_MTU, wan_mtu);
			_SET_VALUE(_WAN0_PPPOE_MTU, wan_mtu);
			_SET_VALUE(_WAN0_PPPOE_MRU, wan_mtu);
			break;
		case PPPOEMODE2:
			_SET_VALUE(_WAN0_PPPOE2_MTU, wan_mtu);
			_SET_VALUE(_WAN0_PPPOE2_MRU, wan_mtu);
			_SET_VALUE(_WAN0_PPPOE2_MTU2, wan_mtu);
			break;
		case L2TPMODE:
			_SET_VALUE(_WAN0_l2TP_MTU, wan_mtu);
			_SET_VALUE(_WAN0_l2TP_MRU, wan_mtu);
			break;
		case PPTPMODE:
		case PPTPMODE2:
			_SET_VALUE(_WAN0_PPTP_MTU, wan_mtu);
			_SET_VALUE(_WAN0_PPTP_MRU, wan_mtu);
			break;
		case STATICMODE:
			_SET_VALUE(_STATIC_WAN0_MTU, wan_mtu);
		case DHCPMODE:
			_SET_VALUE(_DHCP_WAN0_MTU, wan_mtu);
		case C8021XMODE:
		default:
			_SET_VALUE(_WAN0_MTU, wan_mtu);
			break;
	}
	



	/*wan speed*/
	if(!strcmp(wan_speed,"Auto"))
		strcpy(str_temp,"0");
	else if(!strcmp(wan_speed,"-10"))
		strcpy(str_temp,"1");
	else if(!strcmp(wan_speed,"10"))
		strcpy(str_temp,"2");
	else if(!strcmp(wan_speed,"-100"))
		strcpy(str_temp,"3");
	else if(!strcmp(wan_speed,"100"))
		strcpy(str_temp,"4");
	_SET_VALUE("wan_speed", str_temp);
	speed = atoi(str_temp);
	ifr_set_link_speed2(speed);

	
	
	//Wireless Parameters
	/*wifi mode*/
	wifiMode = websGetVar(wp, T("wifiMode"), "");
	wifiChannel = websGetVar(wp, T("wifiChannel"), T("0"));
	wifiBandwidth = websGetVar(wp, T("wifiBandwidth"), "");
	if( !strcmp(wifiChannel, "Auto"))
	{
		wifiChannel = "0";
	}
	
	/*wifi channel*/	
	if(strcmp(nvram_safe_get(WLN0_CHANNEL1), wifiChannel) != 0)
	{
		/*add wzs ,channel 10~13 sideband should be "upper".*/
		_SET_VALUE(WLN0_CHANNEL1, wifiChannel);
		if(atoi(wifiChannel) <= 4 && atoi(wifiChannel) >= 1)
			_SET_VALUE(WLN0_HT_EXTCHA1, "lower");
		else if(atoi(wifiChannel) <= 13 && atoi(wifiChannel) >= 10)
			_SET_VALUE(WLN0_HT_EXTCHA1, "upper");
		wl_temp = 1;
	}	
		
	/*wifi bandwidth*/
	char ht_bw_value[8]={0},obss_value[8]={0};
	if(0 ==strcmp(wifiBandwidth, "20"))//20
	{
		_SET_VALUE(WLN0_HT_BW_FAKE_AUTO,"0");
		strcpy(ht_bw_value,"0");
		strcpy(obss_value,"0");
	}
	else if (0 ==strcmp(wifiBandwidth, "40"))//40
	{
		strcpy(ht_bw_value,"1");
		strcpy(obss_value,"0");
	}
	else//fake 20/40   auto
	{
		_SET_VALUE(WLN0_HT_BW_FAKE_AUTO,"1");
		strcpy(ht_bw_value,"0");
		strcpy(obss_value,"0");
	}

	if(strcmp(nvram_safe_get(WLN0_HT_BW1),ht_bw_value) != 0 ||
		strcmp(nvram_safe_get(WLN0_OBSS1),obss_value) != 0)
	{
		_SET_VALUE(WLN0_HT_BW1, ht_bw_value);
		_SET_VALUE(WLN0_OBSS1, obss_value);
		wl_temp = 1;
	}
		
	
	/* wifi mode */
	_SET_VALUE(WLN0_BASICRATE, "12");
	memset(str_temp, 0x0, sizeof(str_temp));	
	normal_get_wifi_mode(str_temp, sizeof(str_temp));
	if (strcmp(wifiMode, str_temp))
	{
		wl_temp = 1;
		if(0 == strcmp(wifiMode, "bg"))
		{//11bg
			_SET_VALUE(WLN0_G_MODE, "1");	
			_SET_VALUE(WLN0_N_MODE, "0");	
			_SET_VALUE(WLN0_PLCPHDR, "long"); 
		}else if(0 == strcmp(wifiMode, "b"))
		{//11b only
			_SET_VALUE(WLN0_G_MODE, "0");	
			_SET_VALUE(WLN0_N_MODE, "0");	
			_SET_VALUE(WLN0_PLCPHDR, "long");
		}else if(0 == strcmp(wifiMode, "g"))
		{//11g only
			_SET_VALUE(WLN0_G_MODE, "2");
			_SET_VALUE(WLN0_N_MODE, "0");
			_SET_VALUE(WLN0_BASICRATE, "default"); 
			_SET_VALUE(WLN0_PLCPHDR, "short");
		}else if(0 == strcmp(wifiMode, "bgn"))
		{//11bgn	
			_SET_VALUE(WLN0_N_MODE, "-1");
			_SET_VALUE(WLN0_G_MODE, "1");	
			_SET_VALUE(WLN0_PLCPHDR, "long");
		}
	
	}
	
	//LAN Parameters
	char_t  *lan_ip, *go,*lan_mask;
	char_t dhcp_ip_start[20],dhcp_ip_end[20];
	unsigned int dhcp_ip[4],lan_ip2[4];
	char_t old_lan_ip[20],old_lanmask[20];
	
	
	strcpy(old_lan_ip,_GET_VALUE(_LAN0_IP,value));
	strcpy(old_lanmask,_GET_VALUE(_LAN0_NETMASK,value));
	lan_ip = websGetVar(wp, T("lanIP"), "");  //mask 为固定的 255.255.255.0
	
	if(strcmp(lan_ip, "") && strcmp(old_lan_ip, lan_ip))
	{	
		
		_SET_VALUE(_LAN0_IP,lan_ip );  	
		//跟原来 ip&mac 不是在同个 网段,需修改dhcp配置
		if(CGI_same_net_with_lan(SYS_lan_ip,SYS_lan_mask) == 0)
		{
			
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
			
			//set filter
			modify_filter_virtual_server(lan_ip);
		}
		
		reboot_temp = 1;
		errCode = 100;
	}
			
			
	//适配dhcp server 模块 
	dhcpEn = websGetVar(wp,T("dhcpEn"),T(""));
	if (strcmp(nvram_safe_get(_LAN0_DHCPD_EN),"static") == 0 && strcmp(dhcpEn,"true")==0)
	{
		set_dhcpd_en(1);
		wl_temp = 1;
	}	
	else if (strcmp(nvram_safe_get(_LAN0_DHCPD_EN),"dhcp") == 0 && strcmp(dhcpEn,"false")==0)
	{	
		set_dhcpd_en(0);
		wl_temp = 1;
	}	

	//适配 lan-dns模块
	memset(str_temp, 0x0, sizeof(str_temp));	
	get_lan_dns(dns1_old, dns2_old);
	strncpy(lanDns1, websGetVar(wp,T("lanDns1"),T("")), sizeof(lanDns1));
	strncpy(lanDns2, websGetVar(wp,T("lanDns2"),T("")), sizeof(lanDns2));
	
	
	// 由于页面没有做校验, 先更新 下发的dns 值。
	if (!strcmp(lanDns1,""))//页面下发为空
	{	
		
		if(!nvram_match(_LAN0_IP, old_lan_ip) ) //ip更新, 如果之前配置了 route 对应的ip,则dns需更新
		{	
		
			if (!strcmp(dns1_old, old_lan_ip))
			{
				strncpy(lanDns1, nvram_safe_get(_LAN0_IP), sizeof(lanDns1));		
			}	
			else if (!strcmp(dns2_old, old_lan_ip))
			{
				strncpy(lanDns2, nvram_safe_get(_LAN0_IP), sizeof(lanDns2));		
			}
		}
	}	
	else   //页面下发不为空
	{	
			
		if(!nvram_match(_LAN0_IP, old_lan_ip) ) //ip更新, 如果之前配置了 route 对应的ip,则dns需更新
		{
			if (!strcmp(lanDns1, old_lan_ip))
			{
				strncpy(lanDns1, nvram_safe_get(_LAN0_IP), sizeof(lanDns1));		
			}	
			else if (!strcmp(lanDns2, old_lan_ip))  
			{			
				strncpy(lanDns2, nvram_safe_get(_LAN0_IP), sizeof(lanDns2));
			}
		}
	}
			
	if (strcmp(lanDns1,""))
	{
		
		if (strcmp(lanDns2,""))
		{
			sprintf(str_temp, "%s %s", lanDns1, lanDns2);
		}
		else
		{
			sprintf(str_temp, "%s", lanDns1);	
		}
		
		if (strcmp(str_temp, nvram_safe_get(_LAN0_DNS)) )
		{
			_SET_VALUE(_LAN0_DNS, str_temp);
		}	
	}			
					
					
	//Remote Web Management
	remoteWebEn = websGetVar(wp, T("remoteWebEn"), "");
	remoteWebType = websGetVar(wp, T("remoteWebType"), "");
	remoteWebIP = websGetVar(wp, T("remoteWebIP"), "");
	remoteWebPort = websGetVar(wp, T("remoteWebPort"), "");
	
	if ( !nvram_match(_SYS_RM_PORT, remoteWebPort))
	{
		restart_temp = 1;
		_SET_VALUE(_SYS_RM_PORT, remoteWebPort);
	}
	
	if (!strcmp(remoteWebType, "any"))  //页面没有对ip 做模式下方校验
	{
		remoteWebIP = "";
	}
	
	if ( !nvram_match(_SYS_RM_IP, remoteWebIP))
	{
		restart_temp = 1;
		_SET_VALUE(_SYS_RM_IP, remoteWebIP);
	}

	if ( !strcmp(remoteWebEn, "false"))
	{
		remoteWebEn = "0";
	}
	else 
	{
		remoteWebEn = "1";
	}
	
	if ( !nvram_match(_SYS_RM_EN, remoteWebEn))
	{
		
		restart_temp = 1;
		_SET_VALUE(_SYS_RM_EN, remoteWebEn);
		if (!strcmp(remoteWebEn, "false"))  //disable后, port初始化为8080
		{
			_SET_VALUE(_SYS_RM_PORT, "8080");		
		}
	}
	
	
	//Date & Time
	_SET_VALUE(_SYS_TZONE, timeZone);
	
	//Device Management
	if (0 == strcmp(auto_reboot_en, "true"))
		_SET_VALUE("restart_enable", "enable");
	else
		_SET_VALUE("restart_enable", "disable");

	_COMMIT();

	
    webdone:	
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWrite(wp, T("{\"errCode\":\"%d\"}"), errCode);
	websDone(wp, 200);
	
	if(reboot_temp)
	{		
		sys_reboot();
	}
	else if(restart_temp)
	{
		_RESTART_ALL();			
	}
	else if(wl_temp)
	{
		_RESTART();
	}
}

static void fromSysReboot(webs_t wp, char_t *path, char_t *query)
{
						
	char ret_buf[32] = {0};		
									
	strcpy(ret_buf, "{\"errCode\":\"100\"}");			
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWrite(wp,T("%s"),ret_buf);
	websDone(wp, 200);
	
	sys_reboot();
}

static void fromSysRestore(webs_t wp, char_t *path, char_t *query)
{
	char *v = NULL;
	char ret_buf[32] = {0};
	
	strcpy(ret_buf, "{\"errCode\":\"100\"}");
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWrite(wp,T("%s"),ret_buf);
	websDone(wp, 200);
	
	_SET_VALUE(_RESTORE_DEFAULTS,"1");
	_SET_VALUE(_LAN0_IP,_GET_DEFAULT(_LAN0_IP,v));
		
#ifdef A5
	_SET_VALUE("vlan1ports", "1 2 3 4 5*");
	_SET_VALUE("vlan2ports", "0 5");
#endif 
		
	_COMMIT();
	
	//websRedirect(wp, T("/direct_reboot.asp"));

	sys_reboot();
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

	char *v = NULL;
	_SET_VALUE(_RESTORE_DEFAULTS,"1");
	_SET_VALUE(_LAN0_IP,_GET_DEFAULT(_LAN0_IP,v));

	pre_deal_when_restore();

	_COMMIT();
	
	websRedirect(wp, T("/direct_reboot.asp"));

	sys_reboot();
	
	
}

void  reset_button(void){
	//char *v = NULL;
	_SET_VALUE(_RESTORE_DEFAULTS,"1");

	pre_deal_when_restore();
	
	_COMMIT();
	
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
	
	char *p_user_cookie,*p_cookie_tmp, *value;
	char urlbuf[256] = {0};
	char lan_ip[20]={0};
	char wan_ip[20]={0};
	char default_Pwd[64];

	char lan_wlpwd[64]={0}, wan_wlpwd[64]={0};	
	char lan_index[64]={0}, wan_index[64]={0};

	char lan_quickset[64]={0};

	char lan_login[64]={0};
	char wan_login[64]={0}; 
	char lan_login_error[64]={0};
	char wan_login_error[64]={0};
	char login_succes[64]={0}, login_error[64]={0}; 

	char http_true_passwd[256] = {0};

	a_assert(websValid(wp));
	a_assert(url && *url);
	a_assert(path && *path);

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

	if(wp->cookie)
	{
		p_user_cookie = wp->cookie;
		while ((p_cookie_tmp = strstr(p_user_cookie,"ecos_pw="))!=NULL)
		{
			pcookie = p_cookie_tmp + strlen("ecos_pw=");
			p_user_cookie = pcookie;
		}
	}

	if (strlen(urlbuf)>3)
	{
		pfiletype = strrchr(urlbuf, '.');
		if (pfiletype)
		{
			pfiletype++;
			if (!memcmp(pfiletype, "gif", 3)||!memcmp(pfiletype, "js", 2)||!memcmp(pfiletype, "css", 3)||!memcmp(pfiletype, "png", 3) ||!memcmp(pfiletype, "jpg", 3)||!memcmp(pfiletype, "xml", 3)||!memcmp(pfiletype, "ico", 3)||!memcmp(pfiletype, "ttf", 3)||!memcmp(pfiletype, "woff", 4))
				
				return 0;
		}
	}


    if ( g_Pass!=NULL && (strcmp(g_Pass,default_Pwd) == 0) && wp->url!=NULL && (strstr(wp->url,"/goform/ate")!=NULL) )
    	 return 0;

	strncpy(lan_ip,NSTR(SYS_lan_ip),sizeof(lan_ip));
	strncpy(wan_ip,NSTR(SYS_wan_ip),sizeof(wan_ip));

	snprintf(lan_login,sizeof(lan_login),"http://%s/login.html",lan_ip); 
	snprintf(wan_login,sizeof(wan_login),"http://%s/login.html",wp->host_str);
	
	snprintf(lan_login_error,sizeof(lan_login_error),"http://%s/login.html?1",lan_ip);
	snprintf(wan_login_error,sizeof(wan_login_error),"http://%s/login.html?0",wp->host_str);
	
	snprintf(lan_wlpwd,sizeof(lan_wlpwd),"http://%s/redirect_set_wlsec.asp",lan_ip);
	snprintf(lan_index,sizeof(lan_index),"http://%s/index.html",lan_ip);

	snprintf(lan_quickset,sizeof(lan_quickset),"http://%s/quickset.html",lan_ip);//add liuchengchi

	snprintf(wan_wlpwd,sizeof(wan_wlpwd),"http://%s/redirect_set_wlsec.asp",wp->host_str);
	snprintf(wan_index,sizeof(wan_index),"http://%s/index.html",wp->host_str);
	
	snprintf(login_succes,sizeof(login_succes),"http://%s/loginsuccess.html",lan_ip);
	snprintf(login_error,sizeof(login_error),"http://%s/loginerror.html",lan_ip);
	
	for (i=0; i<MAX_USER_NUM; i++)
	{
		if(!strcmp(loginUserInfo[i].ip, wp->ipaddr)) 
		{
			break;
		}
	}
	
	if(i<MAX_USER_NUM)	
	{
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
			if((!memcmp(urlbuf, "/login.html", 11) || (strlen(urlbuf)==1&&urlbuf[0]=='/')) || (!memcmp(urlbuf, "/login/Auth", 11)))
			{
				goto LOGINOK;
			} 
			else
			{
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

			
			if(!memcmp(urlbuf, "/login.html", 11) || (strlen(urlbuf)==1 && urlbuf[0]=='/') || !memcmp(urlbuf, "/login/Auth", 11))
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
				loginUserInfo[i].time = (unsigned int)cyg_current_time();
				return 0;
			}	
		}
		else{
			if(strstr(wp->url,"login")!=NULL || !memcmp(urlbuf, "/login/Auth", 11) 
				|| !memcmp(urlbuf, "/redirect_set_wlsec", 18)|| !memcmp(urlbuf, "/goform/redirectSetwlSecurity", 29))
			{
				goto RELOGIN;
			}
			else{
				websResponse(wp,302,NULL,T(lan_login));
				loginUserInfo[i].time = (unsigned int)cyg_current_time();
				return 0;
			}
		}
	} 
	else
	{
		
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
		else if (!memcmp(urlbuf, "/login.html", 11))
			return 0;
		else if  (!memcmp(urlbuf, "/loginerror.html", 16))
			return 0;
		else if (!memcmp(urlbuf, "/error.asp", 10))
			return 0;
		else if (!memcmp(urlbuf, "/login/Auth", 11))
		{
			password = websGetVar(wp, T("password"), T(""));
			userid = websGetVar(wp, T("Username"), T("admin"));
			
			if ( !strcmp(password, g_Pass))
			{
				for(i=0; i<MAX_USER_NUM; i++)
				{
					if (strlen(loginUserInfo[i].ip) == 0 || !strcmp(loginUserInfo[i].ip, wp->ipaddr))
					{
						memcpy(loginUserInfo[i].ip , wp->ipaddr, IP_SIZE);	
						
						loginUserInfo[i].time = (unsigned int)cyg_current_time();
						goto LOGINOK;
						break;
						
					}
				}
				if(i == MAX_USER_NUM) 
				{
					SetErrorArg(LOGIN_USERS_FULL,"login.html");
					websRedirect(wp, T("error.asp"));
					return 0;
				}
			}
			else
			{
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
	if(dns_redirect_web == 1){
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

			char *restore_quick_set;
			_GET_VALUE(_RESTORE_QUICK_SET,restore_quick_set);
			
			if(atoi(restore_quick_set) == 1){
				websResponseWithCookie(wp,302,NULL,T(lan_quickset), 1);
			}
			else{
				websResponseWithCookie(wp,302,NULL,T(lan_index), 1);
			}			
		}
	}
	return 0;
	
LOGINERR:
	if(strncmp(wp->host_str,wan_ip,strlen(wan_ip))==0)
		websRedirect(wp, T(wan_login_error));
	else{		
		websRedirect(wp, T(lan_login_error));
	}
	printf("Login ERROR !!!\n");
	return 0;
	
NOPASSWORD:
	if(strcmp(g_Pass, "") == 0)
	{
		for(i=0; i<MAX_USER_NUM; i++)
		{
			if (strlen(loginUserInfo[i].ip) == 0)
			{
				memcpy(loginUserInfo[i].ip , wp->ipaddr, IP_SIZE);		//?????~{C;~}?		
				//printf("5\n");
				loginUserInfo[i].time = (unsigned int)cyg_current_time();
				//printf("NOPASSWORD: urlbuf=%s\n",urlbuf);

				if((!memcmp(urlbuf, "/login.html", 11) || (strlen(urlbuf)==1&&urlbuf[0]=='/')) || (!memcmp(urlbuf, "/login/Auth", 11)))
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
static void mfg_html_info(char *buf) 	//~{R3~}????~{O"~}
{

      char *hv,*lanmac,*wanmac,*language,*ssid,*pin,*date;
	  hv = W311R_ECOS_HV;
      lanmac = mfg_get_value("et0macaddr");
	  wanmac = mfg_get_value("wan0_hwaddr");
	  ssid = mfg_get_value("wl_ssid");
	  pin = mfg_get_value("wps_device_pin");  
	  language = "";
	  date = "";
      sprintf(buf,"HV=%s;SV=%s_%s;LANMAC=%s;WANMAC=%s;LANGUAGEINFO=%s;SSID=%s;PIN=%s;DATA=%s",
	  	hv,W311R_ECOS_SV,__CONFIG_WEB_VERSION__,lanmac,wanmac,language,ssid,pin,date);
	  //printf("buf = %s\n",buf);
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
			cyg_thread_delay(20); // 0.2 s
	
			printf("ledtest  on \n");
			
			set_all_led_on();	
			break;
		}
		cyg_thread_delay(10); // 0.1 s
	}	

	return 0;
}


extern int mfg_reset_button_check_tag;
extern int mfg_wifi_button_check_tag;
extern int mfg_button_check_tag;
void mfg_mainloop(void)
{
    int mfgfd;
    int ilen;
	struct sockaddr_in local;
	struct sockaddr remote;
    char recvbuf[256], sendbuf[256];
	int			argc;
	char *argv[32];
	//char *macaddr1=NULL;
	char *bootwait=NULL;
    char cmdbuf[256];
	
    struct ether_addr *hwaddr;
    unsigned char macaddr[32];
	fd_set rfds;
	struct timeval tv={2,0};
       int retval;
    int mfgflag;

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
        if (ilen == 256 )
            recvbuf[255] = 0;

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
                //start: add by z10312  合入 厂测按键测试&LED灯测试 接口2015-01-27
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
                //end: add by z10312  合入 厂测按键测试&LED灯测试 接口2015-01-27
            break;
        case 3://wlctrl
        	//Tenda_mfg check ResetButton
        	if (0 == strcmp(argv[0], "Tenda_mfg"))
        	{
        		if (0 == strcmp(argv[1], "Check")) 
        		{
					if(0 == strcmp(argv[2], "ResetButton"))
	        		{
	        			int i = 0;
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
					else if(0 == strcmp(argv[2], "WiFiButton"))
	        		{
	        			int j = 0;
	                    for(j = 0; j < 15 ; j++)
						{
							if (1 == mfg_button_check_tag)
							{
								break;
							}
							cyg_thread_delay(1*100);
						}

						if (1 == mfg_button_check_tag)
						{
							printf("WiFiButton test OK\n");
							mfgflag = SUCCESS;
							mfg_button_check_tag = 0;
						}
						else
						{
							printf("WiFiButton test ERROR\n");
							mfgflag = ERROR;
						}
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
                    else if (strcmp(argv[3], "lan") == 0)
                    {
                        // add for A5 V1
                        mfg_set_value("vlan1ports", "0 1 2 3 4 5*");
                        mfg_set_value("vlan2ports", "5");
                        mfg_commit();
                        mfgflag = SUCCESS;
                    }//end lan
                    else if (strcmp(argv[3], "wan") == 0)
                    {
                        mfg_set_value("vlan1ports", "1 2 3 4 5*");
                        mfg_set_value("vlan2ports", "0 5");
                        mfg_commit();
                        mfgflag = SUCCESS;                      
                    } //end wan
                }//en boot_wait
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
            }//end nvram set    
            else if (strcmp(argv[0], "Tenda_mfg") == 0 && strcmp(argv[1], "check" )== 0&& strcmp(argv[2], "USB") == 0)
            {       
            }           
        break;

    default:
            mfgflag = ERROR;

        }//end switch
    if (mfgflag == SUCCESS)
    {
        //sucess
        strcpy(sendbuf, "success");
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
		  printf("?????~{K!~}???????");
    }
    ilen = strlen(sendbuf);
    sendto(mfgfd, (char *)sendbuf, ilen, 0, (struct sockaddr *)&remote, sizeof(remote));

    continue;

    }//end while

    return;

}//end main

static void fromGetWan(webs_t wp, char_t *path, char_t *query)
{
	char result[2048] = {0};    
	char value[128]={0};
	
	strncat(result, "{", 1);	
//wanType: wan~{?Z<l2b5=5D=SHk7=J=~}	
	memset(value , 0 , sizeof(value));
	snprintf(value , sizeof(value) ,  "%s" , nvram_safe_get(_WAN0_PROTO) );
	string_cat(result,"wanType",value);

//wanType: wan~{?ZA,=SW4L,~}

	strncat(result, ",", 1);
	memset(value , 0 , sizeof(value));	
	sprintf(value,"%d",internet_error_check_function());
	string_cat(result,"wanConnectStatus",value);

//PPPOE~{SC;'C{C\Bk~}
	strncat(result, ",", 1);
	memset(value , 0 , sizeof(value));	
	string_cat(result,"wanPPPoEUser",encodeSSID(nvram_safe_get(_WAN0_PPPOE_USERNAME),value));
	strncat(result, ",", 1);
	memset(value , 0 , sizeof(value));	
	string_cat(result,"wanPPPoEPwd",encodeSSID(nvram_safe_get(_WAN0_PPPOE_PASSWD),value));

	//~{75;X~}lan ip
	memset(value , 0 ,sizeof(value));
	strncat(result, ",", 1);
	sprintf(value,"%s",nvram_safe_get("lan_ipaddr"));
	string_cat(result,"lanIP",value);

	//~{75;X~}lan mask
	memset(value , 0 ,sizeof(value));
	strncat(result, ",", 1);
	sprintf(value,"%s",nvram_safe_get("lan_netmask"));
	string_cat(result,"lanMask",value);

//wan ip
		memset(value , 0 , sizeof(value));
		strncat(result, ",", 1);
		snprintf(value , sizeof(value) ,  "%s" , nvram_safe_get(_WAN0_IPADDR) );
		if(strncmp(value , "0.0.0.0" , strlen("0.0.0.0")) == 0 ){
			string_cat(result,"wanIP", "");
		}else{
			string_cat(result,"wanIP", value);
		}
		strncat(result, ",", 1);

	/*wan mask*/	
		memset(value , 0 , sizeof(value));
		snprintf(value , sizeof(value) ,  "%s" ,nvram_safe_get(_WAN0_NETMASK) );
		if(strncmp(value , "0.0.0.0" , strlen("0.0.0.0")) == 0 ){
			string_cat(result,"wanMask", "");
		}else{
			string_cat(result,"wanMask",value);
		}
		
		strncat(result, ",", 1);

	/*wan gateway*/
		memset(value , 0 , sizeof(value));
		snprintf(value , sizeof(value) ,  "%s" ,nvram_safe_get(_WAN0_GATEWAY) );
		if(strncmp(value , "0.0.0.0" , strlen("0.0.0.0")) == 0 ){
			string_cat(result,"wanGateway", "");
		}else{
			string_cat(result,"wanGateway",value);
		}
		strncat(result, ",", 1);

	/*wan dns1*/
		char *dns,dns1[30]={0},dns2[30]={0};
		_GET_VALUE(_WAN0_DNS,dns);
		sscanf(dns,"%s %s",dns1,dns2);
		string_cat(result,"wanDns1",dns1);
		strncat(result, ",", 1);
		
	/*wan dns2*/	
		string_cat(result,"wanDns2",dns2);
		//strncat(result, ",", 1);
			
	strncat(result, "}", 1);	
	strncat(result, "\n", 1);
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);

}

static void fromSetMacClone(webs_t wp, char_t *path, char_t *query)
{
	char_t  *wan_mac;

	wan_mac = websGetVar(wp, T("wanMac"), T("")); 
	if(strcmp(wan_mac,"") != 0 && strcmp(wan_mac,nvram_safe_get(_WAN0_HWADDR)) != 0)
	{
		_SET_VALUE(_WAN0_HWADDR,wan_mac);
	}	
	_SET_VALUE(_MACCLONEMODE,"clone");
		
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWrite(wp, T("{\"errCode\":\"%d\"}"), 0);
	websDone(wp, 200);
	
	_RESTART_ALL(); 
}

int change_tag = 0;
static void fromSetWan(webs_t wp, char_t *path, char_t *query)

{
	char_t *go, *con_type;
	con_type = websGetVar(wp, T("wanType"), T(""));			///*wantstr = 1 static ip, =2 DHCP, =3 PPPOE */
	//go = websGetVar(wp, T("GO"), T("")); 
	int sys_reboot = 0,wl_reboot = 0;

	_SET_VALUE(_WAN0_PROTO, con_type);
	_SET_VALUE(_WAN0_PROTO_INDEX, con_type);
	
	int tm,wan_mod,old_wan_mode;
	char val[128];
	char dns3[128];
	old_wan_mode = get_wan_type();
	if(strcmp(con_type,"") != 0)
	{	
		//wan_mod = atoi(con_type);
		if(strcmp(con_type,"static") == 0)
		{
			char_t  *wan_ip, *wan_msk, *gw,*dns1, *dns2,*static_mtu;	
			wan_ip = websGetVar(wp, T("wanIP"), T("0.0.0.0")); 
			wan_msk = websGetVar(wp, T("wanMask"), T("0.0.0.0")); 
			gw = websGetVar(wp, T("wanGateway"), T("0.0.0.0")); 
			dns1 = websGetVar(wp, T("wanDns1"), T(""));
			dns2 = websGetVar(wp, T("wanDns2"), T(""));

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
		else if(strcmp(con_type,"dhcp") == 0)//DHCP~{D#J=~}
		{
			char_t  *DynStatic_mtu;
			if((old_wan_mode - 1) != wan_mod)
			{
				sys_reboot = 1;
				_GET_VALUE(_DHCP_WAN0_MTU, DynStatic_mtu);
				_SET_VALUE(_WAN0_MTU,DynStatic_mtu);
			}
		}
		else if(strcmp(con_type,"pppoe") == 0)//PPPOE ~{D#J=~}
		{
			char_t *user_id, *pwd, *mtu, *ac, *sev, *conmode;
			char_t *idle_time, *hour_s, *min_s, *hour_e, *min_e;
			char_t *v12_time;

			char *xkjs_user_id,*xkjs_pwd;
			char tmp_xkjs_user_id[64];
			char tmp_xkjs_pwd[64];
			
			_SET_VALUE("plugplay_flag","n");//gong add
			user_id = websGetVar(wp, T("wanPPPoEUser"), T("")); 
			pwd = websGetVar(wp, T("wanPPPoEPwd"), T("")); 
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
				//?????~{G?U<~}???
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
			_SET_VALUE("config_index","1");

			_SET_VALUE("wan0_check","0");
			_SET_VALUE("mode_need_switch","no");
			_SET_VALUE("wan0_connect","Connecting");
		}
		network_tpye = 4;
	}

	 change_tag = 1;
		
	_COMMIT();
	if(sys_reboot == 1)
	{
		stop_wan();
		if (wan_link_status())
		{
			start_wan();
			printf("line=%d,start_wan service\n",__LINE__);
		}
	}

	printf("****line=%d,con_type=%s,sys_reboot=%d,wl_reboot=%d\n",__LINE__,con_type,sys_reboot,wl_reboot);	

 	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n")); 
	websWrite(wp, T("%s"), "{\"errCode\":\"0\"}"); 
	websDone(wp, 200);
	
}


#ifdef __CONFIG_STREAM_STATISTIC__
//#include "../tc/tc.h"
#define TIMES 1
#define ip_ubs(X) ((stream_ip_per[X][0])/TIMES)
#define ip_dbs(X) ((stream_ip_per[X][1])/TIMES)
#define ip_upa(X) (stream_ip[X].index->up_packets)
#define ip_dpa(X) (stream_ip[X].index->down_packets)
#define ip_uby(X) (stream_ip[X].index->up_bytes)
#define ip_dby(X) (stream_ip[X].index->down_bytes)
#define ip_uM(X)   (stream_ip[X].index->up_Mbytes)
#define ip_dM(X)   (stream_ip[X].index->down_Mbytes)

typedef struct stream_list{
	char hostname[64];
	char remark[128];
	in_addr_t  ip;
	char  mac[20];
	unsigned int type;
	unsigned int  up_byte_pers;
	unsigned int  down_byte_pers;
	unsigned int  set_pers;
	float		up_limit;
	float		down_limit;
	float		limit;
	int  access;
}stream_list_t;

typedef struct stream_statistic{
	unsigned int up_packets;
	unsigned int down_packets;
	unsigned int up_bytes;
	unsigned int down_bytes;
	unsigned int up_Mbytes;
	unsigned int down_Mbytes;
	unsigned int  up_byte_pers;
	unsigned int  down_byte_pers;
}stream_statistic_t;

typedef struct statistic_ip_index{
	stream_statistic_t *index;
}statistic_ip_index_t;

#define TC_CLIENT_NUMBER 	255
#define	NAT_OUTBOUND	0
#define	NAT_INBOUND	1

#ifdef __CONFIG_STREAM_STATISTIC__
#define STREAM_CLIENT_NUMBER TC_CLIENT_NUMBER
#define CONFIG_STREAM_INTERVAL 100
#define INBOUND NAT_INBOUND
#define OUTBOUND NAT_OUTBOUND
#endif
extern statistic_ip_index_t stream_ip[STREAM_CLIENT_NUMBER];
extern unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];
#endif

void get_stream_statistic(u_long * up_speed,u_long * down_speed)
{
	char_t pre_ip[32]={'\0'}, pr_ip[32]={'\0'};
	char_t *lanip, *p;
	unsigned int index;
	int up = 0, down = 0;
	int up1=0, down1=0;
	int up2 = 0, down2 = 0;
	int  u_kbs=0, d_kbs=0, u_kbs0=0, d_kbs0=0, u_kbs1=0, d_kbs1=0, u_kbs2 =0, d_kbs2 =0;
	char  vlaue[32]={0};
	unsigned char *mac_look_list;
	int temp=0;
	
	_GET_VALUE(_LAN0_IP, lanip);
	strcpy(pr_ip, lanip);
	p=rindex(pr_ip, '.');
	if(p) *p='\0';

	u_long   up_kbyte_pers = 0;
	u_long   down_kbyte_pers = 0;

	stream_list_t  *qosInfo = (stream_list_t *)malloc(sizeof(stream_list_t));
	for(index=0; index< STREAM_CLIENT_NUMBER; ++index)
	{
		temp=0;
		
		sprintf(pre_ip, "%s.%d", pr_ip, index+1);
		if(!strcmp(pre_ip , lanip))
		{
			continue ;
		}

		mac_look_list = lookmac(inet_addr(pre_ip));
		if( NULL == mac_look_list)
		{
			continue;
		}
		
		if(ismulticast(mac_look_list))
			continue;


		memset(qosInfo, 0x0, sizeof(qosInfo));

		(ip_ubs(index)!=0) ? (u_kbs0 = ip_ubs(index)/1000) : (u_kbs0=0);
		(ip_dbs(index)!=0) ? (d_kbs0 = ip_dbs(index)/1000) : (d_kbs0=0);
		
		(ip_ubs(index)!=0) ? (u_kbs = ip_ubs(index)%1000) : (u_kbs=0);
		(ip_dbs(index)!=0) ? (d_kbs = ip_dbs(index)%1000) : (d_kbs=0);
				
		(u_kbs!=0) ? (u_kbs1=u_kbs/100) : (u_kbs1=0);
		(d_kbs!=0) ? (d_kbs1=d_kbs/100) : (d_kbs1=0);
		(u_kbs1!=0) ? (u_kbs2=u_kbs1/10) : (u_kbs2=0);
		(d_kbs1!=0) ? (d_kbs2=d_kbs1/10) : (d_kbs2=0);
		
		(ip_uby(index)!=0) ? (up=ip_uby(index)/1000) : (up=0);
		(ip_dby(index)!=0) ? (down=ip_dby(index)/1000) : (down=0);
		
		(up!=0) ? (up1=up/100) : (up1=0);
		(down!=0) ? (down1=down/100) : (down1=0);
		(up1!=0) ? (up2=up1/10) : (up2=0);
		(down1!=0)?(down2=down1/10) : (down2=0);
		
		/*ip;uprate;downrate;sendpacket;sendbytes;recvpackets;recvbytes*/
		sprintf(vlaue,"%d.%d%d",(d_kbs0), (d_kbs1), (d_kbs2));
		qosInfo->down_byte_pers = (int)(atof(vlaue));

		memset(vlaue, 0x0, sizeof(vlaue));
		sprintf(vlaue, "%d.%d%d", (u_kbs0), (u_kbs1), (u_kbs2));
		qosInfo->up_byte_pers = (int)(atof(vlaue));

		up_kbyte_pers += (float)qosInfo->up_byte_pers;
		down_kbyte_pers += (float)qosInfo->down_byte_pers;

	}

	(*up_speed) = up_kbyte_pers;
	(*down_speed) = down_kbyte_pers;
	
	return;
}



extern int get_all_client_info( struct client_info * clients_list,	int max_client_num);
#define MAX_CLIENT_NUMBER 		255


static void fromGetStatus(webs_t wp, char_t *path, char_t *query)
{
	char result[2048] = {0};
	char value[128]={0};
	char pcMac[20];
	char mm[20]={0};
	int wanConnTyp = get_wan_type();
	struct client_info  *clients_list ;

	strncat(result, "{", 1);	
		
	sprintf(value,"%d",internet_error_check_function());

	string_cat(result,"statusInternet",value);
	strncat(result, ",", 1);
	

/*client numbers*/
	int client_num = 0;

	clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
	if(clients_list != NULL )
	{
		memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);
		client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);
	}
	sprintf(value,"%d",client_num < 1 ? 1 : client_num);
	string_cat(result,"statusOnlineNumber",value);
	strncat(result, ",", 1);
	free(clients_list);
	clients_list = NULL;

/*download and upload*/
	u_long up_speed = 0;
	u_long down_speed =0;
	get_stream_statistic(&up_speed,&down_speed);
	memset(value , 0 , sizeof(value));	
	sprintf(value, "%d", down_speed);
	string_cat(result,"statusDownSpeed",value);
	strncat(result, ",", 1);
	memset(value , 0 , sizeof(value));	
	sprintf(value, "%d", up_speed);
	string_cat(result,"statusUpSpeed",value);
	strncat(result, ",", 1);	

/*WAN~{?Z~} status*/
	strncat(result, "\"statusWAN\"", strlen("\"statusWAN\""));
	strncat(result, ":", 1);
	strncat(result, "{", 1);
	
	//wan information**************
	wan_link_check();
	int wanConnSta = get_wan_connstatus();
	

/*run time*/
	int connect_time = 0;
	connect_time = (wanConnSta==2&&(SYS_wan_conntime))?(time(0) - SYS_wan_conntime):0;
	connect_time = (connect_time < 0) ? 0 : connect_time;
	memset(value , 0 , sizeof(value));
	sprintf(value,"%d",connect_time);
	string_cat(result,"statusWanConnectTime",value);
	strncat(result, ",", 1);


/*wan protoc*/
	memset(value , 0 , sizeof(value));
	snprintf(value , sizeof(value) ,  "%s" , nvram_safe_get(_WAN0_PROTO) );
	string_cat(result,"statusWanType",value);
	strncat(result, ",", 1);

/*wan mac*/ 
	char *u, *v;
	_SAFE_GET_VALUE("wl0_mode", u);
	if(strcmp(u,"sta")==0)
		_SAFE_GET_VALUE("wl0.1_hwaddr", v);
	else
		_SAFE_GET_VALUE(_WAN0_HWADDR, v);	
	string_cat(result,"statusWanMAC",v);	
	strncat(result, ",", 1);
	
/*PC mac*/
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
	string_cat(result,"macHost",mm);	
	strncat(result, ",", 1);

/*lan ip*/
	string_cat(result,"statusLanIP",nvram_safe_get(_LAN0_IP));	
	strncat(result, ",", 1);
	
/*wan mask*/
	string_cat(result,"statusWanMask",wanConnSta==2?NSTR(SYS_wan_mask):"");	
	strncat(result, ",", 1);

/*wan ip*/
	string_cat(result,"statusWanIP",wanConnSta==2?NSTR(SYS_wan_ip):"");	
	strncat(result, ",", 1);

/*wan gateway*/
	string_cat(result,"statusWanGaterway",wanConnSta==2?NSTR(SYS_wan_gw):"");	
	strncat(result, ",", 1);

/*wan dns1*/
	char wan_dns_1[32] = {0}, wan_dns_2[32] = {0};
	if(wanConnSta == 2)
		if(wanConnTyp == PPPOEMODE ||wanConnTyp == PPTPMODE ||
			wanConnTyp == L2TPMODE||wanConnTyp==PPPOEMODE2){
			sprintf(wan_dns_1,T("%s"),SYS_dns_1);
		}else{
			get_wan0_dns(1,wan_dns_1);
		}
	else
		sprintf(wan_dns_1,"%s","");	
	string_cat(result,"statusWanDns1",wan_dns_1);	
	strncat(result, ",", 1);
	
/*wan dns2*/
	if(wanConnSta == 2)
		if(wanConnTyp == PPPOEMODE ||wanConnTyp == PPTPMODE ||
			wanConnTyp == L2TPMODE||wanConnTyp==PPPOEMODE2){
			sprintf(wan_dns_2,T("%s"),SYS_dns_2);
		}else{
			get_wan0_dns(2,wan_dns_2);
		}
	else
		sprintf(wan_dns_2,"%s","");
	string_cat(result,"statusWanDns2",wan_dns_2);	
	strncat(result, ",", 1);

/*sotf version*/
	memset(value , 0 , sizeof(value));
	sprintf(value,T("%s_%s"),W311R_ECOS_SV,__CONFIG_WEB_VERSION__);
	string_cat(result,"statusFirmwareVersion",value);	
	//strncat(result, ",", 1);


	strncat(result, "}", 1);
//end wan information		

	strncat(result, "}", 1);	
	strncat(result, "\n", 1);
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);
	
	return ;



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

	//~{=b>v~}F3~{:M~}F450~{9&BJ2;<fH]NJLb~} ,add by ll 
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




//int change_tag = 0;
extern int check_tag;

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
		else if(1 == wan_mod)//DHCP~{D#J=~}
		{
			char_t  *DynStatic_mtu;
			if((old_wan_mode - 1) != wan_mod)
			{
				sys_reboot = 1;
				_GET_VALUE(_DHCP_WAN0_MTU, DynStatic_mtu);
				_SET_VALUE(_WAN0_MTU,DynStatic_mtu);
			}
		}
		else if(2 == wan_mod)//PPPOE ~{D#J=~}
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

	char *ssid,*pwd,*old_ssid,*old_pwd;
	ssid = websGetVar(wp, T("ssid"), T(""));
	pwd = websGetVar(wp, T("ssid-pwd"), T(""));	

	char SSID_mode[16] = {0};	
	get_wl0_mode(SSID_mode);

	if(strcmp(SSID_mode,"ap") == 0)
	{
		old_ssid = nvram_safe_get("wl0_ssid");
		old_pwd = nvram_safe_get("wl0_wpa_psk");
	}else{
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

extern void stop_wps_when_failed(void);
extern void set_wireless_wps(webs_t wp, char_t *path, char_t *query);
#ifdef __CONFIG_WPS_LED__
extern void wps_led_test_on(void);
#endif
extern void wl_unit(char *unit,int from_secu);


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
		sscanf(time_zone, "%c%d",&c_timezone,&i_timezone);
		i_timezone = (i_timezone/100)*3600 + (i_timezone%100)*60;
		i_timezone = (c_timezone == '-' ) ? ( 0 - i_timezone) : ( i_timezone );
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

	
	if(strcmp(con_type,"dhcp") == 0){//DHCP~{D#J=~}
		char_t	*DynStatic_mtu;
		if( old_wan_mode != 2 )
		{
			sys_reboot = 1;
			_GET_VALUE(_DHCP_WAN0_MTU, DynStatic_mtu);
			_SET_VALUE(_WAN0_MTU,DynStatic_mtu);
		}
	}
	else if(strcmp(con_type,"pppoe") == 0){//PPPOE 模式
	
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
			//?????~{G?U<~}???
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
		old_ssid = nvram_safe_get("wl0_ssid");
		old_pwd = nvram_safe_get("wl0_wpa_psk");
	}else{
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

static void fromSetWizard(webs_t wp, char_t *path, char_t *query)
{
	int want;
	char_t *wantstr, *ssid1, *wlpassword;
	char *wlunit;
	
	wantstr = websGetVar(wp, T("mode"), T("dhcp"));
	ssid1 =  websGetVar(wp, T("wizardSSID"), T(""));
	wlpassword =  websGetVar(wp, T("wizardSSIDPwd"), T(""));

	/*wantstr = 1 static ip, =2 DHCP, =3 PPPOE */

	int old_wan_mode;
	old_wan_mode = get_wan_type();
	if(strcmp(wantstr, "dhcp") == 0){
		//dhcp
		char_t	*DynStatic_mtu;
		if( old_wan_mode != 2 )
		{
			_GET_VALUE(_DHCP_WAN0_MTU, DynStatic_mtu);
			_SET_VALUE(_WAN0_MTU,DynStatic_mtu);
		}
	}
	else if(strcmp(wantstr, "pppoe") == 0){  
		//pppoe
		char_t *pppoeuser, *pppoepwd;
		char *xkjs_user_id,*xkjs_pwd;
		char tmp_xkjs_user_id[64];
		char tmp_xkjs_pwd[64];

		_SET_VALUE("plugplay_flag","n");//gong add
		pppoeuser = websGetVar(wp, T("wizardPPPoEUser"), T("")); 
		pppoepwd = websGetVar(wp, T("wizardPPPoEPwd"), T("")); 
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
	else if(strcmp(wantstr, "static") == 0){
		//static ip
		char_t *wanip,*wanmsk, *gw, *dns1, *dns2,dns[40];
		wanip = websGetVar(wp, T("wizardWanIP"), T("0.0.0.0"));
		wanmsk = websGetVar(wp, T("wizardWanMask"), T("0.0.0.0"));
		gw = websGetVar(wp, T("wizardWanGateway"), T("0.0.0.0"));
		dns1 = websGetVar(wp, T("wizardWanDns1"), T(""));
		dns2 = websGetVar(wp, T("wizardWanDns2"), T(""));
		//diag_printf("[%s]:: ssid=%s, %s, %s,%s,%s,%s,\n", __FUNCTION__, 
		//			indexssid, wanip,wanmsk,	gw, dns1, dns2);
		if(CGI_same_net_with_lan(inet_addr(wanip),inet_addr(wanmsk)))
		{
		//	SetErrorArg(LAN_WAN_IP_ERROR, "wizard.asp");
			websRedirect(wp, T("error.asp"));
			return;
		}
		if (strcmp(wanip,gw)==0||strcmp(wanip,dns1)==0||strcmp(wanip,dns2)==0)
		{
		//	SetErrorArg(WAN_IP_ERROR,"wan_connected.asp");
			websRedirect(wp, T("error.asp"));
			return;
		}

		_SET_VALUE(_WAN0_IPADDR, wanip);
		_SET_VALUE(_WAN0_NETMASK, wanmsk);
		_SET_VALUE(_WAN0_GATEWAY, gw);

		sprintf(dns,"%s %s",dns1,dns2);
		 _SET_VALUE(_WAN0_DNS, dns);

				
	}	
	else if(strcmp(wantstr, "wifi") == 0){
		// wizardSSID=Tenda_123456& wizardSSIDPwd=12345678

			char_t *ssid ;
			ssid = websGetVar(wp, T("wizardSSID"), T(""));

			if(strlen(ssid) > 0) 
				_SET_VALUE(WLN0_SSID0, ssid);

			char_t *pass_phrase_str= NULL ;
			pass_phrase_str = websGetVar(wp, T("wizardSSIDPwd"), T(""));
			if(pass_phrase_str != NULL && pass_phrase_str[0] != '\0'  ){
				
				_SET_VALUE(WLN0_WEP, DISABLE);
				_SET_VALUE(WLN0_AUTH_MODE, OPEN);
				_SET_VALUE(WLN0_SECURITY_TYPE, "psk");
				_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
				confWPAGeneral( wp);
			}else {
			
				_SET_VALUE(WLN0_AUTH_MODE, OPEN);
				_SET_VALUE(WLN0_WEP, DISABLE);
				_SET_VALUE(WLN0_SECURITY_TYPE,"");
				_SET_VALUE(WLN0_ENCRYP_TYPE, "");
			}

	}

	if (strcmp(wantstr,"wifi") != 0)
	{
		_SET_VALUE(_WAN0_PROTO, wantstr);
		_SET_VALUE(_WAN0_PROTO_INDEX, wantstr);
	}
	
	_GET_VALUE(WLN0_UNIT,wlunit);
		
	wl_unit(wlunit,1);
#ifdef __CONFIG_WPS__			
	stop_wireless_wps();
#endif
#ifdef __CONFIG_WPS_LED__
	wps_led_test_off();		
#endif
	_SET_VALUE("err_check","0");
	_SET_VALUE("config_index","1");
	_SET_VALUE("wan0_check","0");
	_SET_VALUE("mode_need_switch","no");

	_SET_VALUE(_RESTORE_QUICK_SET,"0");

	_COMMIT();
	_RESTART_ALL();

 	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n")); 
	websWrite(wp, T("%s"), "{\"errCode\":\"0\"}"); 
	websDone(wp, 200);
	
	//SetErrorArg(CONFIG_SUCCESS,"index.html");
	//websRedirect(wp, T("index.html"));	
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
	char wan_mode_check_verdict[20] = {0};

	snprintf(wan_mode_check_verdict,sizeof(wan_mode_check_verdict) , "%s",nvram_safe_get("wan_mode_check_verdict"));
	
	//~{75;X~}wan~{?Z<l2b~}
	if(0 == wanLinkSta){
		sprintf(wan_detection , "disabled" );
	}else{
		if(wan_mode_check_verdict[0] == '\0'){
			sprintf(wan_detection , "detecting");
		}else{
			sprintf(wan_detection , wan_mode_check_verdict );
		}
	}
	string_cat(result,"wizardWANDetection",wan_detection);
	printf("network_tpye=%s\n",wan_mode_check_verdict);

	//~{75;X~}ssid
	strncat(result, ",", 1);
	sprintf(value,"%s",nvram_safe_get("wl0_ssid"));
	string_cat(result,"wizardSSID",value);

	//~{75;X~}lan ip
	memset(value , 0 ,sizeof(value));
	strncat(result, ",", 1);
	sprintf(value,"%s",nvram_safe_get("lan_ipaddr"));
	string_cat(result,"lanIP",value);

	//~{75;X~}lan mask
	memset(value , 0 ,sizeof(value));
	strncat(result, ",", 1);
	sprintf(value,"%s",nvram_safe_get("lan_netmask"));
	string_cat(result,"lanMask",value);

	//~{75;X~}connectInternet
	int	internetStat = get_wan_onln_connstatus();		
	memset(value , 0 ,sizeof(value));
	strncat(result, ",", 1);
	if(2 == internetStat)
		sprintf(value,"%s","true");
	else
		sprintf(value,"%s","false");
	string_cat(result,"connectInternet",value);
	
	strncat(result, "}", 1);	
	strncat(result, "\n", 1);
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);

}


extern int gWifiStatusConfig;
extern int g_cur_wl_radio_status;
#define WL_RADIO_ON		1
#define WL_RADIO_OFF		0

void wl_change_states( int status )
{
	if ( WL_RADIO_OFF == status)
	{
		g_cur_wl_radio_status = WL_RADIO_OFF;
	}
	else
	{
		g_cur_wl_radio_status = WL_RADIO_ON;
	}
	sys_restart();
}

static void fromSetWifi(webs_t wp, char_t *path, char_t *query)
{

//******************wifi enable/disable******************
	char_t *wirelessenable;
	wirelessenable = websGetVar(wp, T("wifiEn"), T("true"));
	if(strcmp(wirelessenable,"true") == 0)
		wirelessenable = "1";
	else
		wirelessenable = "0";
	_SET_VALUE(WLN0_WIRELESS_ENABLE, wirelessenable);
	if (strcmp(wirelessenable, "0") == 0){
		wl_change_states(WL_RADIO_OFF);
		#ifdef __CONFIG_WL_LED__
			wl_led_test_off();		//add by stanley 2010/11/3
		#endif
		#ifdef __CONFIG_WPS_LED__
			char_t *wps_enable;
			_GET_VALUE("wl0_wps_mode", wps_enable);
			if(strcmp(wps_enable, "enabled") == 0){
				stop_wireless_wps();	//axin add 2012.08.09
				wps_led_test_off();
				diag_printf("wps closed! \n"); 
			}
		#endif

		goto END_WIFI_SET;
		
	}
	if(strcmp(wirelessenable, "1") == 0 ){
		wl_change_states(WL_RADIO_ON);
		#ifdef __CONFIG_WL_LED__
			wl_led_test_on();						//add by stanley 2010/11/3
		#endif
		#ifdef __CONFIG_WPS_LED__
			char_t *wps_enable;
			_GET_VALUE("wl0_wps_mode", wps_enable);
	
			if(strcmp(wps_enable, "enabled") == 0){
				diag_printf("wps on! \n"); 
				wps_led_test_on();
			}	
		#endif
	}	
//******************ssid******************
	char_t *power=NULL, *ssid=NULL ,*hide_ssid=NULL;
	ssid = websGetVar(wp, T("wifiSSID"), T(""));
	power = websGetVar(wp, T("wifiPower"), T(""));
	hide_ssid = websGetVar(wp, T("wifiHideSSID"), T("false"));
	
	if(strlen(ssid) > 0) 
		_SET_VALUE(WLN0_SSID0, ssid);	

	if(strlen(power) > 0) 
		_SET_VALUE(WLN0_POWER, power);

	if(strcmp(hide_ssid,"true") == 0)
		hide_ssid = "1";
	else
		hide_ssid = "0";
	_SET_VALUE(WLN0_HIDE_SSID0, hide_ssid);

//******************security and wps******************
	char_t *security_mode,  *wpsenable,*old_wps,old_wps_str[16];
	char_t *wps_oob_status ;
	char_t *wlunit;
	
	security_mode = websGetVar(wp, T("wifiSecurityMode"), T(""));
	wpsenable = websGetVar(wp, T("wpsEn"), T("false"));
	//~{PB=gCf4+5D2NJ}1#3VSkT-@4Hm<~5DR;VB#,~}web~{Wi2;T8RbP^8D#,NRCGWv8vW*;;#,FdK{5X7=R`@`KF~}	
	if(strcmp(wpsenable,"true") == 0)
		wpsenable = "enabled";
	else
		wpsenable = "disabled";
	
	_GET_VALUE(WLN0_UNIT,wlunit);
	_GET_VALUE(WLN0_WPS_ENABLE,old_wps);
	//must do this
	strcpy(old_wps_str,old_wps);
	/*add by stanley 2010/10/14*/
	
	if( 0== strcmp(wpsenable, "enabled") ){	

		printf("****line=%d,wpsenable=%s,old_wps=%s\n",__LINE__,wpsenable,old_wps);
		
			if(strcmp(wlunit, "0") == 0)
				_SET_VALUE(WLN0_WPS_OOB, "disabled");	//MM,liuke
			else if(strcmp(wlunit, "0.1") == 0)
				_SET_VALUE("lan1_wps_oob", "disabled");
		
			//first,let's stop wps enabled former
			stop_wps_when_failed();
			
			if(strcmp(old_wps_str,"enabled") == 0){
				//set_wireless_wps( wp,  path,  query);
				;
			}else{
				nvram_set(WLN0_WPS_ENABLE, "enabled");
				nvram_set(WLN0_WPS_METHOD, "pbc");
#ifdef __CONFIG_WPS_LED__
				wps_led_test_on();
#endif				
			}

	}

	 if(0 == strcmp(wpsenable, "disabled")){	
	 	if(strcmp(old_wps_str,"enabled") == 0){
			/*add by stanley 2010/10/25*/
			//_GET_VALUE(WLN0_WPS_OOB, wps_oob_status);
			//if(strcmp(wps_oob_status, "disabled") == 0)
			//{
			set_wireless_wps( wp,  path,  query);
			_SET_VALUE(WLN0_WPS_OOB, "enabled");		//MM,~{IhVCN*~}enabled~{Tr~}WIN7~{OB2;;a5/3v~}WPS~{M<1j#,~}Modify by liuke 2011/11/23
			//}else{
			//	tenda_stop_wps_timer();		
			//}
			_SET_VALUE(WLN0_WPS_ENABLE, "disabled");	
			_SET_VALUE(WLN0_WPS_METHOD, "");
#ifdef __CONFIG_WPS_LED__
			wps_led_test_off();		//Can I move it ???
#endif	
	 	}
		if ((0 == strcmp(security_mode, "Disable"))||(0 == strcmp(security_mode, "none")) ) {	// !--- Disable Mode ---
			//0:open; 1:shared
			diag_printf("\n security mode: Disable\n");
			
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_SECURITY_TYPE,"");//modify 2010/11/04
			/*add by stanley 2010/10/14*/
			//_SET_VALUE(WLN0_WPA_PSK1,"");//modify 2010/11/04
			_SET_VALUE(WLN0_ENCRYP_TYPE, "");
			/*end*/		
		}else if(0 == strcmp(security_mode, "wpa2-psk") ){// !---  WPA2 Personal Mode ----
			char_t *pass_phrase_str;
			pass_phrase_str = websGetVar(wp, T("wifiPwd"), T(""));
			diag_printf("\n security mode: psk2\n");	
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk2");
			_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
			confWPAGeneral( wp);
			
		}else if(0 == strcmp(security_mode, "wpa&wpa2") )	{ //! ----   WPA PSK WPA2 PSK mixed
			char_t *pass_phrase_str;
			pass_phrase_str = websGetVar(wp, T("wifiPwd"), T(""));
			diag_printf("\n security mode: PSK PSK2\n");
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk psk2");
			_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
			confWPAGeneral( wp);
			
		}else if(0 == strcmp(security_mode, "wpa-psk")){	// !---  WPA Personal Mode ---
			char_t *pass_phrase_str;
			pass_phrase_str = websGetVar(wp, T("wifiPwd"), T(""));	
			diag_printf("\n security mode: PSK\n");
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk");
			_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
			confWPAGeneral( wp);	
		}
		
	}


//******************WifiControlSet******************
	char *wlctl_enable, *time_round, *time_week, *time_interval,*sche_cnt;
	char time[4][16] = {0};
	int off_time,off_sec,off_min,off_hour;
	int on_time,on_sec,on_min,on_hour;
	char week_new[16] = {0};;
	char time_str[64] = {0};
	int flag = 0;
	wlctl_enable = websGetVar(wp, T("wifiTimeEn"), T("false"));
	time_round = websGetVar(wp, T("time-round"), T("1111111"));//everyday defaultly
	time_interval = websGetVar(wp, T("wifiTimeClose"), T("00:00-00:00"));
	time_week = websGetVar(wp, T("wifiTimeDate"), T("10101010"));

	_SET_VALUE(WLN0_CTL_TIME_WEEK2, time_week);
	_SET_VALUE(WLN0_CTL_TIME_INTERVAL2, time_interval);

	char week[10]={0};
	int j=0 ,i=1;
	if(time_week[0] == '1')
	{
		strcpy(week, "1,2,3,4,5,6,7");		//~{Q!VPAK~}everyday
	}
	else
	{
		for(i=1;i<=7;i++)
		{
			if(time_week[i] == '1')
			{
				if(i == 1)
					sprintf(week,"%s%d",week,i); 
				else
				{
					sprintf(week,"%s,%d",week,i); 
				}
			}
		}
	}
	
	time_week = week ;

	if(strcmp(wlctl_enable,"true") == 0)
		wlctl_enable = "1";
	else
		wlctl_enable = "0";

	_SET_VALUE(WLN0_CTL_ENABLE, wlctl_enable);
	
	sscanf(time_interval, "%[^:]:%[^-]-%[^:]:%s", time[0], time[1], time[2], time[3]);
	
	_GET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, sche_cnt);
	if (atoi(sche_cnt) == 0)
	{
		_SET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, "1");
	}
	
	if (atoi(wlctl_enable))
	{
		_SET_VALUE(WLN0_CTL_DAY_ROUND, time_round);
		_SET_VALUE(WLN0_CTL_TIME_INTERVAL, time_interval);

		
		if((atoi(time[0]))*60 + (atoi(time[1])) < (atoi(time[2]))*60 + (atoi(time[3])))  
		{
			off_hour = atoi(time[0]);
			off_min = atoi(time[1]);
			off_sec = 0;

			on_hour =atoi(time[2]);
			on_min = atoi(time[3]);
			on_sec = 0;
			//00:00-01:00
			//UTC+08:00 0 0 0 ? * 1,2,3,4,5,6,7
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", off_sec, off_min, off_hour,time_week);
			_SET_VALUE("alilink_wlan_offtime_list1", time_str);
			//UTC+08:00 0 0 1 ? * 1,2,3,4,5,6,7
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", on_sec, on_min, on_hour,time_week);
			_SET_VALUE("alilink_wlan_ontime_list1", time_str);
			
		}
		else		//7200,86400;0,3600
		{
			sscanf(time_interval, "%[^,],%[^;];%[^,],%s", time[0], time[1], time[2], time[3]);
			off_hour = time[0];
			off_min = time[1];
			off_sec = 0;

			on_hour = time[2];
			on_min =  time[3];
			on_sec = 0;
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", off_sec, off_min, off_hour,time_week);
			_SET_VALUE("alilink_wlan_offtime_list1", time_str);

			strcpy(week_new, time_week);
			changeWeekForm2(time_week, week_new);
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", on_sec, on_min, on_hour,week_new);
			_SET_VALUE("alilink_wlan_ontime_list1", time_str);
		}
		
	}
	else
	{
		_SET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, "0");	
	}
	
	gWifiStatusConfig = 0;
//end WifiControlSet

END_WIFI_SET:

	_GET_VALUE(WLN0_UNIT,wlunit);	
	wl_unit(wlunit,1);

	_COMMIT();
	_RESTART();
	
 	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n")); 
	websWrite(wp, T("%s"), "{\"errCode\":\"0\"}"); 
	websDone(wp, 200);
	
	//cyg_thread_delay(200);
	if(strcmp(wirelessenable, "1") == 0 )
		wl_restart_check_main_loop();

	return ;

}


static void fromGetWifi(webs_t wp, char_t *path, char_t *query)
{

	char result[2048] = {0};    
	char value[128]={0};
	
	strncat(result, "{", 1);	
	char SSID1_mode[16] = {0};	
	get_wl0_mode(SSID1_mode);
	if((strcmp(SSID1_mode,"ap") !=0)){
		copy_wl_index_to_unindex("0.1");
	}	
	else{
		copy_wl_index_to_unindex("0");
	}

//wifiEn	
	if(strcmp(nvram_safe_get(WLN0_WIRELESS_ENABLE),"0") == 0)
		snprintf(value , sizeof(value) ,  "%s" , "false" );
	else
		snprintf(value , sizeof(value) ,  "%s" , "true" );
	string_cat(result,"wifiEn",value);

//wifiHideSSID
	strncat(result, ",", 1);
	memset(value , 0 , sizeof(value));	

	if(strcmp(nvram_safe_get(WLN0_HIDE_SSID0),"0") == 0)
		snprintf(value , sizeof(value) ,  "%s" , "false" );
	else
		snprintf(value , sizeof(value) ,  "%s" , "true" );
	string_cat(result,"wifiHideSSID",value);

//wifiSSID
	strncat(result, ",", 1);
	memset(value , 0 , sizeof(value));	
	sprintf(value,"%s",nvram_safe_get(WLN0_SSID0));
	string_cat(result,"wifiSSID",value);

//wifiSecurityMode
	memset(value , 0 , sizeof(value));	
	if(strcmp(nvram_safe_get(WLN0_SECURITY_TYPE),"psk") == 0)
		snprintf(value , sizeof(value) ,  "%s" , "wpa-psk" );
	else if(strcmp(nvram_safe_get(WLN0_SECURITY_TYPE),"psk2") == 0)
		snprintf(value , sizeof(value) ,  "%s" , "wpa2-psk" );
	else if(strcmp(nvram_safe_get(WLN0_SECURITY_TYPE),"psk psk2") == 0)
		snprintf(value , sizeof(value) ,  "%s" , "wpa&wpa2" );
	else if(strcmp(nvram_safe_get(WLN0_SECURITY_TYPE),"") == 0)
		snprintf(value , sizeof(value) ,  "%s" , "none" );
	strncat(result, ",", 1);


	string_cat(result,"wifiSecurityMode",value);

//wifiPwd
	strncat(result, ",", 1);
	memset(value , 0 , sizeof(value));	
	sprintf(value,"%s",nvram_safe_get(WLN0_WPA_PSK1));
	string_cat(result,"wifiPwd",value);

//wifiPower
	strncat(result, ",", 1);
	memset(value , 0 , sizeof(value));	
	sprintf(value,"%s",nvram_safe_get(WLN0_POWER));
	string_cat(result,"wifiPower",value);

//wifiTime
	strncat(result, ",", 1);
	strncat(result, "\"wifiTime\"", strlen("\"wifiTime\""));
	strncat(result, ":", 1);
	strncat(result, "{", 1);
	//wifiTimeEn
		memset(value , 0 , sizeof(value));	
		if(strcmp(nvram_safe_get(WLN0_CTL_ENABLE),"0") == 0)
			snprintf(value , sizeof(value) ,  "%s" , "false" );
		else
			snprintf(value , sizeof(value) ,  "%s" , "true" );
		string_cat(result,"wifiTimeEn",value);
	//wifiTimeClose
		strncat(result, ",", 1);
		memset(value , 0 , sizeof(value));	
		sprintf(value,"%s",nvram_safe_get(WLN0_CTL_TIME_INTERVAL2));
		string_cat(result,"wifiTimeClose",value);
	//wifiTimeDate
		strncat(result, ",", 1);
		memset(value , 0 , sizeof(value));	
		sprintf(value,"%s",nvram_safe_get(WLN0_CTL_TIME_WEEK2));
		string_cat(result,"wifiTimeDate",value);
	strncat(result, "}", 1);	

//wifiWPS
	strncat(result, ",", 1);
	strncat(result, "\"wifiWPS\"", strlen("\"wifiWPS\""));
	strncat(result, ":", 1);
	strncat(result, "{", 1);

	//wpsEn
		memset(value , 0 , sizeof(value));	
		if(strcmp(nvram_safe_get(WLN0_WPS_ENABLE),"disabled") == 0)
			snprintf(value , sizeof(value) ,  "%s" , "false" );
		else
			snprintf(value , sizeof(value) ,  "%s" , "true" );
		string_cat(result,"wpsEn",value);
	//wpsPIN
		strncat(result, ",", 1);
		memset(value , 0 , sizeof(value));	
		sprintf(value,"%s",nvram_safe_get("wps_device_pin"));
		string_cat(result,"wpsPIN",value);

	strncat(result, "}", 1);	

	strncat(result, "}", 1);	
	strncat(result, "\n", 1);
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);

}


static void fromSetWps(webs_t wp, char_t *path, char_t *query)

{
	char_t *security_mode=NULL,  *wpsenable=NULL,*wifienable=NULL,*action=NULL;
	char_t *wlunit;
	
	//security_mode = websGetVar(wp, T("wifiSecurityMode"), T(""));
	//wpsenable = websGetVar(wp, T("wpsEn"), T("false"));
	//~{PB=gCf4+5D2NJ}1#3VSkT-@4Hm<~5DR;VB#,~}web~{Wi2;T8RbP^8D#,NRCGWv8vW*;;#,FdK{5X7=R`@`KF~}

	action = websGetVar(wp, T("action"), T("pbc"));

	_GET_VALUE(WLN0_WPS_ENABLE,wpsenable);
	_GET_VALUE(WLN0_WIRELESS_ENABLE,wifienable);
	
	_GET_VALUE(WLN0_UNIT,wlunit);

	printf("wpsenable=%s,wifienable=%s,action=%s,\n",wpsenable,wifienable,action);

	if( (0== strcmp(wpsenable, "enabled")) && (0== strcmp(wifienable, "1")) && (0== strcmp(action, "pbc")) )
	{	
		if(strcmp(wlunit, "0") == 0)
			_SET_VALUE(WLN0_WPS_OOB, "disabled");	
		else if(strcmp(wlunit, "0.1") == 0)
			_SET_VALUE("lan1_wps_oob", "disabled");

		printf("start pbc wps.................\n");
		//first,let's stop wps enabled former
		stop_wps_when_failed();
			
		set_wireless_wps( wp,  path,  query);

		wl_unit(wlunit,1);
		_COMMIT();

		cyg_thread_delay(100);
	}

 	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n")); 
	websWrite(wp, T("%s"), "{\"errCode\":\"0\"}"); 
	websDone(wp, 200);
			
	return;
	
}


static void fromGetIsHasLoginPwd  (webs_t wp, char_t *path, char_t *query)
{
	char *out = NULL;
	cJSON *root = NULL;
	char  haspwd[16] = {0};
	
	root = cJSON_CreateObject();
	
	if (strcmp(nvram_safe_get("http_defaultpwd1"), "0")){
		sprintf(haspwd, "true");							
	}
	else{
		sprintf(haspwd, "false");	
	}
	
	cJSON_AddStringToObject(root, "hasLoginPwd", haspwd);

	out = cJSON_Print(root);
	cJSON_Delete(root);
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWriteLongString(wp, out);
	free(out);
	websDone(wp, 200);

	return ;

}



static void fromLoginOut (webs_t wp, char_t *path, char_t *query)
{

	int i = 0;

	for(i=0; i<MAX_USER_NUM; i++){
		if(!strcmp(loginUserInfo[i].ip, wp->ipaddr)){
			memset(loginUserInfo[i].ip, 0x0, IP_SIZE);
			loginUserInfo[i].time = 0;
			break;
		}	
	}
	
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n")); 
	websWrite(wp, T("%s"), "{\"errCode\":\"0\"}"); 
	websDone(wp, 200);
	return ;
}



