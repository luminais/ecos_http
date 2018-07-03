/*asp_form.h hodling the protype of asp function and form fucntions*/
#ifndef __ASP_FORM_H__
#define __ASP_FORM_H__


#define DEF_WLAN_PREFIX "wlan"

#include "apmib.h"

extern asp_name_t root_asp[] ;
extern form_name_t root_form[] ;
extern char WLAN_IF[20];

extern int wlan_num;
#ifdef MBSSID
extern int vwlan_num;
extern int mssid_idx;
#endif

extern unsigned int MUST_REBOOT;

/*ASP function*/
int wlAcList(int argc, char ** argv);
int wlWdsList(int argc, char **argv);
int wdsList(int argc, char **argv);
int getScheduleInfo(int argc, char **argv);
int wirelessClientList(int argc, char **argv);
int wlSiteSurveyTbl(int argc, char **argv);
int dhcpClientList(int argc, char **argv);
int dhcpRsvdIp_List(int argc, char **argv);
int portFwList(int argc, char **argv);
int sysCmdLog(int argc, char **argv);
#ifdef CONFIG_TR069
int TR069ConPageShow(int argc, char **argv);
#endif
#ifdef CONFIG_RTL_VLAN_SUPPORT
int vlanList(int argc, char **argv);
int netIfList(int argc,char**argv);
int showVlanTagTbl(int argc,char**argv);
#endif
#ifdef ROUTE_SUPPORT
int staticRouteList(int argc,char**argv);
int kernelRouteList(int argc,char**argv);
#endif
#if defined(WLAN_PROFILE)
extern int wlProfileTblList(int argc, char **argv);
extern int wlProfileList(int argc, char **argv);
extern void getWlProfileInfo(int argc, char **argv);

#endif //#if defined(WLAN_PROFILE)
#ifdef SYSLOG_SUPPORT
extern void sysLogList(int argc, char **argv);
#endif

#ifdef CONFIG_IPV6
uint32 getIPv6Info(int argc, char **argv);
uint32 getIPv6WanInfo(int argc, char **argv);
int getIPv6Status(int argc, char **argv);
int getIPv6BasicInfo(int argc, char **argv);
#endif

#if defined(HTTP_FILE_SERVER_SUPPORTED)
int dump_directory_index(int argc, char **argv);
int Check_directory_status(int argc, char **argv);
int Upload_st(int argc, char **argv);
#endif

/*ASP form*/
void formWizard(char *postData, int len);
void formDdns(char *postData, int len);
void formtest(char *postData, int len);
void formRebootCheck(char *postData, int len);
void formTcpipSetup(char *postData, int len);
void formReflashClientTbl(char *postData, int len);
void formStaticDHCP(char *postData, int len);
void formWanTcpipSetup(char *postData, int len);
void formNtp(char *postData, int len);

void formDMZ(char *postData, int len);
void formPortFw(char *postData, int len);
void formStats(char *postData, int len);
void formSysCmd(char *postData, int len);
#ifdef CONFIG_TR069
void formTR069Config(char *postData, int len);
#endif
#ifdef ECOS_CWMP_WITH_SSL
void formTR069CPECert(char *postData, int len);
void formTR069CACert(char *postData, int len);
#endif
#ifdef CONFIG_RTL_VLAN_SUPPORT
void formVlanAdd(char *postData, int len);
void formVlan(char *postData, int len);
void formNetIf(char *postData, int len);
#endif
void formLangSel(char *postData, int len);

/*ASP from added by winfred_wang*/
void formPasswordSetup(char *postData, int len);
#ifdef SYSLOG_SUPPORT
void formSysLog(char *postData, int len);
#endif
void formLogout(char *postData, int len);
void formDosCfg(char *postData, int len);

#if defined(WLAN_PROFILE)	
extern void formSiteSurveyProfile(char *postData, int len);
#endif //#if defined(WLAN_PROFILE)	

#ifdef ROUTE_SUPPORT
void formRouteRip(char *postData, int len);
void formRouteAdd(char *postData, int len);
void formRouteDel(char *postData, int len);
#endif

#ifdef HAVE_FIREWALL
void formFilter(char *wp, int len);
#endif

#ifdef QOS_BY_BANDWIDTH
void formIpQoS(char *wp, int len);
int ipQosList(int argc, char **argv);
#endif

#ifdef CONFIG_IPV6
void formRadvd(char *postData, int len);
#ifdef CONFIG_IPV6_WAN_RA
void formRadvd_wan(char *postData, int len);
#endif
void formDnsv6(char *postData, int len);
void formDhcpv6s(char *postData, int len);
void formIPv6Addr(char *postData, int len);
void formIpv6Setup(char *postData, int len);
void formTunnel6(char *postData, int len);
#endif
#if defined(CONFIG_RTL_8881A_SELECTIVE)
void formWlanBand2G5G(char *postData, int len);
#endif
void formSaveConfig(char *postData, int len);
void formWlanSetup(char *postData, int len);
void formAdvanceSetup(char * postData,int len);
void formWlEncrypt(char * postData,int len);
void formWlanMultipleAP(char *postData, int len);
void formWlAc(char *postData, int len);
void formWdsEncrypt(char *postData, int length);
void formWlWds(char *postData, int len);
#ifdef HAVE_WLAN_SCHEDULE
void formSchedule(char *postData, int length);
#endif
void formWsc(char *postData, int length);
void formWirelessTbl(char *postData, int length);
void formWlSiteSurvey(char *postData, int length);
void formUploadConfig(char *postData, int len);
void formUpload(char *postData, int len);
void formOpMode(char *postData, int len);
#ifdef CONFIG_RTK_MESH
#ifdef _MESH_ACL_ENABLE_
int  wlMeshAcList(char *postData, int len);
void formMeshACLSetup(char *postData, int len);
#endif
int  wlMeshNeighborTable(char *postData, int len);
int  wlMeshRoutingTable(char *postData, int len);
int  wlMeshProxyTable(char *postData, int len);
int  wlMeshRootInfo(char *postData, int len);
int  wlMeshPortalTable(char *postData, int len);
void formMeshSetup(char *postData, int len);
int  formMeshStatus(char *postData, int len);
void formMeshProxy(char *postData, int len);
#endif

#if defined(HTTP_FILE_SERVER_SUPPORTED)
void formusbdisk_uploadfile(char *postData, int len);
#endif

int  apmib_update_web(int type);
/*misc*/
void OK_MSG1(char *msg, char *url) ;
void OK_MSG(char *url);
void ERR_MSG(char *tmpbuf);

void send_redirect_perm(char* url);

int handle_fmGetValAndSetIp(char*name,int mibid,void* ip_val,char *postData, int len,char* errMsgBuf);
int handle_fmGetValAndSetChkbox(char*name,int mibid,int* chkbox_val,char *postData, int len,char* errMsgBuf);
int handle_fmGetValAndSetMac(char*name,int mibid,void* mac_val,char *postData, int len,char* errMsgBuf);
int handle_fmGetValAndSetIntVal(char * name, int mibid, int *intVal, char * postData, int len, char * errMsgBuf);
int handle_fmGetValAndSetStrVal(char * name, int mibid, char *strValue, char * postData, int len, char * errMsgBuf);
int handle_setWlanIntVal(int intVal,int mibid,int wlanIdx, char * errMsgBuf);
#endif
