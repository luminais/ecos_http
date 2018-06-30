/*function extra for asp functions*/

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/kernel/kapi.h>           // Kernel API.
#include <cyg/infra/diag.h>            // For diagnostic printing.
#include <network.h>
#include <time.h>

#include <cyg/hal/hal_tables.h>
#include <cyg/fileio/fileio.h>
#include <stdio.h>                     // sprintf().
#include <stdlib.h>


#include "asp.h"
#include "fmget.h"
#include "http.h"
#include "socket.h"
#include "asp_form.h"

#define BUF_SIZE 256

int wlan_num;
#ifdef MBSSID
int vwlan_num=0;
int mssid_idx=0;
#endif

char WLAN_IF[20];
unsigned int MUST_REBOOT=0;

/*
  *ASP Function
  */
asp_name_t root_asp[] = {
	{"getInfo",getInfo},		
	{"getIndex",getIndex},
	{"getLangInfo",getLangInfo},
#if defined(HAVE_FIREWALL)
	{"urlFilterList",urlFilterList},	
	{"ipFilterList",ipFilterList},
	{"macFilterList",macFilterList},
	{"portFilterList",portFilterList},
	{"portFwList", portFwList},
#endif
#ifndef HAVE_NOWIFI
	{"getModeCombobox",getModeCombobox},	
//#if defined(MBSSID)
	{"getVirtualIndex",getVirtualIndex},
	{"getVirtualInfo",getVirtualInfo},
//#endif
	{"wlAcList",wlAcList},
	{"wlWdsList",wlWdsList},
	{"wdsList",wdsList},
	{"getScheduleInfo",getScheduleInfo},
	{"wirelessClientList",wirelessClientList},
	{"wlSiteSurveyTbl",wlSiteSurveyTbl},
#endif
#ifdef CONFIG_RTK_MESH
#ifdef _MESH_ACL_ENABLE_
	{"wlMeshAcList", wlMeshAcList},
#endif
	{"wlMeshNeighborTable", wlMeshNeighborTable},
	{"wlMeshRoutingTable", wlMeshRoutingTable},
	{"wlMeshProxyTable", wlMeshProxyTable},
	{"wlMeshRootInfo", wlMeshRootInfo},
	{"wlMeshPortalTable", wlMeshPortalTable},
#endif

#ifdef QOS_BY_BANDWIDTH
	{"ipQosList",ipQosList},
#endif
	{"dhcpClientList",dhcpClientList},
	{"dhcpRsvdIp_List",dhcpRsvdIp_List},
//	{"portFwList", portFwList},
	{"sysCmdLog",sysCmdLog},
#if defined(HAVE_FIREWALL)
#ifdef CONFIG_RTL_VLAN_SUPPORT
	{"vlanList",vlanList},
	{"netIfList",netIfList},
	{"showVlanTagTbl",showVlanTagTbl},
#endif
#endif
#ifdef ROUTE_SUPPORT
	{"staticRouteList",staticRouteList},
	{"kernelRouteList",kernelRouteList},
#endif
#if defined(WLAN_PROFILE)	
	{"wlProfileList", wlProfileList},
	{"wlProfileTblList", wlProfileTblList},
	{"getWlProfileInfo", getWlProfileInfo},
#endif //#if defined(WLAN_PROFILE)
#ifdef SYSLOG_SUPPORT
	{"sysLogList",sysLogList},
#endif
#ifdef CONFIG_IPV6
	{"getIPv6Info", getIPv6Info},
// TODO: ipv6 wan
//	{"getIPv6WanInfo", getIPv6WanInfo},
	{"getIPv6Status", getIPv6Status},
	{"getIPv6BasicInfo", getIPv6BasicInfo},	
#endif
#ifdef CONFIG_TR069
	{"TR069ConPageShow", TR069ConPageShow},
#endif 
#if defined(HTTP_FILE_SERVER_SUPPORTED)
	{"dump_directory_index", dump_directory_index},
	{"Check_directory_status", Check_directory_status},
	{"Upload_st", Upload_st},
#endif
	{NULL, NULL}
};


/*
  * ASP Form
  */
form_name_t root_form[] = {
	{"formWizard",formWizard},
	{"formtest",formtest},
	{"formRebootCheck",formRebootCheck},
	{"formTcpipSetup",formTcpipSetup},
	{"formReflashClientTbl",formReflashClientTbl},
	{"formStaticDHCP",formStaticDHCP},
#ifdef HOME_GATEWAY
	{"formWanTcpipSetup",formWanTcpipSetup},
#endif
#ifdef HAVE_RTLDD
	{"formDdns",formDdns},
#endif
	{"formNtp",formNtp},
	{"formSaveConfig",formSaveConfig},
#if defined(HAVE_FIREWALL)
	{"formFilter",formFilter},
	{"formDMZ", formDMZ},
    {"formPortFw", formPortFw},
#endif
#ifdef QOS_BY_BANDWIDTH
	{"formIpQoS",formIpQoS},
#endif
#ifdef HAVE_NOWIFI
#else
#if defined(CONFIG_RTL_8881A_SELECTIVE)
	{"formWlanBand2G5G",formWlanBand2G5G},
#endif
	{"formWlanSetup",formWlanSetup},	
	{"formAdvanceSetup",formAdvanceSetup},
	{"formWlEncrypt",formWlEncrypt},
#if defined(MBSSID)
	{"formWlanMultipleAP",formWlanMultipleAP},
#endif
	{"formWlAc",formWlAc},
	{"formWdsEncrypt",formWdsEncrypt},
	{"formWlWds",formWlWds},
#ifdef CONFIG_RTK_MESH
	{"formMeshSetup", formMeshSetup},
	{"formMeshProxy", formMeshProxy},
	{"formMeshStatus", formMeshStatus},
#ifdef _MESH_ACL_ENABLE_
	{"formMeshACLSetup", formMeshACLSetup},
#endif
#endif
#ifdef HAVE_WLAN_SCHEDULE
	{"formSchedule",formSchedule},
#endif
	{"formWsc",formWsc},
	{"formWirelessTbl",formWirelessTbl},
	{"formWlSiteSurvey",formWlSiteSurvey},
#endif
	{"formUploadConfig",formUploadConfig},
	{"formUpload",formUpload},
	{"formOpMode", formOpMode},

/*ASP From added by winfred_wang*/
	{"formPasswordSetup",formPasswordSetup},
#ifdef SYSLOG_SUPPORT
	{"formSysLog",formSysLog},
#endif
	{"formLogout",formLogout},
	{"formDosCfg",formDosCfg},
//	{"formDMZ", formDMZ},
//	{"formPortFw", formPortFw},
	{"formStats", formStats},
	{"formSysCmd",formSysCmd},
#if defined(HAVE_FIREWALL)
#ifdef CONFIG_RTL_VLAN_SUPPORT
	{"formVlanAdd",formVlanAdd},
	{"formVlan",formVlan},
	{"formNetIf",formNetIf},
#endif
#endif
	{"formLangSel",formLangSel},
#ifdef ROUTE_SUPPORT
	{"formRouteRip",formRouteRip},
	{"formRouteAdd",formRouteAdd},
	{"formRouteDel",formRouteDel},
#endif
#if defined(WLAN_PROFILE)	
	{"formSiteSurveyProfile", formSiteSurveyProfile},	
#endif //#if defined(WLAN_PROFILE)		
#ifdef CONFIG_IPV6
	{"formRadvd", formRadvd},
#ifdef CONFIG_IPV6_WAN_RA
	{"formRadvd_wan", formRadvd_wan},
#endif
// TODO: dns v6
//	{"formDnsv6", formDnsv6},
// TODO: dhcp v6
//	{"formDhcpv6s", formDhcpv6s},
	{"formIPv6Addr", formIPv6Addr},
// TODO: ipv6 wan
//	{"formIpv6Setup", formIpv6Setup},
// TODO: tunnel6	
//	{"formTunnel6", formTunnel6},	
#endif
#ifdef CONFIG_TR069
	{"formTR069Config", formTR069Config},
#endif
#ifdef ECOS_CWMP_WITH_SSL
        {"formTR069CPECert", formTR069CPECert},
        {"formTR069CACert", formTR069CACert},
#endif
#if defined(HTTP_FILE_SERVER_SUPPORTED)
	{"formusbdisk_uploadfile", formusbdisk_uploadfile},
#endif
	{NULL, NULL}
};
#ifdef CSRF_SECURITY_PATCH
#include <time.h>

#define EXPIRE_TIME					300	// in sec
#define MAX_TBL_SIZE				3

struct goform_entry {
	int	valid;		
	char name[80];
	time_t time;
};

struct goform_entry  security_tbl[MAX_TBL_SIZE] = {\
		{0}, {0}, {0}, {0}};

void log_boaform(char *form)
{
	int i, oldest_entry=-1;
	time_t t, oldest_time=	-1;
	//diag_printf("%s:%d logBoaForm=%s\n",__FILE__,__LINE__,form);

	for (i=0; i<MAX_TBL_SIZE; i++) {
		if (!security_tbl[i].valid ||
				(security_tbl[i].valid && 
					(time(&t) - security_tbl[i].time) > EXPIRE_TIME) ||
					(security_tbl[i].valid && !strcmp(form, security_tbl[i].name))) {	
			break;					
		}	
		else {
			if (security_tbl[i].valid) {
				if (oldest_entry == -1 || security_tbl[i].time < oldest_time) {
					oldest_entry = i;
					oldest_time = security_tbl[i].time;
				}				
			}			
		}		
	}

	if ((i < MAX_TBL_SIZE) || (i == MAX_TBL_SIZE && oldest_entry != -1)) {		
		if (i == MAX_TBL_SIZE)
			i = oldest_entry;
		
		strcpy(security_tbl[i].name, form);

		security_tbl[i].time = time(&t);
		security_tbl[i].valid = 1;		
	}
#if 0
	for(i=0;i<MAX_TBL_SIZE;i++)
	{		
		diag_printf("%s:%d valid=%d name=%s\n",__FILE__,__LINE__,security_tbl[i].valid,security_tbl[i].name);
	}
#endif
}

void delete_boaform(char *form)
{
	int i;

	for (i=0; i<MAX_TBL_SIZE; i++) {
		if (security_tbl[i].valid && !strcmp(form, security_tbl[i].name)) {
			security_tbl[i].valid = 0;
			//diag_printf("%s:%d valid=%d name=%s\n",__FILE__,__LINE__,security_tbl[i].valid,security_tbl[i].name);
			break;
		}		
	}
}

int is_valid_boaform(char *form) 
{
	int i, valid=0;
	time_t t;
	
	//diag_printf("%s:%d  form=%s\n",__FILE__,__LINE__,form);
	for (i=0; i<MAX_TBL_SIZE; i++) {
		if (security_tbl[i].valid && !strcmp(form, security_tbl[i].name)) {
			if	((time(&t) - security_tbl[i].time) > EXPIRE_TIME) {					
				security_tbl[i].valid = 0;
				break;
			}				
			valid = 1;
			//diag_printf("%s:%d	form=%s is valid\n",__FILE__,__LINE__,form);
			break;
		}
	}	
	return valid;
}

int is_any_log()
{
	int i;
	for (i=0; i<MAX_TBL_SIZE; i++) {
		if (security_tbl[i].valid)
			return 1;
	}
	return 0;
}
#endif // CSRF_SECURITY_PATCH

/*
  * Form Function
  */
void formtest(char *postData, int len)
{
	char *buf;
	printf("%s %d\n",__FUNCTION__,__LINE__);
	buf=get_cstream_var(postData,len,"testname","");
	printf("%s %d testname(%s)\n",__FUNCTION__,__LINE__,buf);
	return;
}

/*
  *misc function
  */

void mutil_lang_support(){
	char* argv_char[1] = {NULL};
	argv_char[0] = (char *)malloc(sizeof("charset"));
	memcpy(argv_char[0],"charset",sizeof"charset");
	web_write_chunked("<meta http-equiv=\"Content-Type\" content=\"text/html;charset=");
	getLangInfo(1,argv_char);
	web_write_chunked("\">\n");
	if(argv_char[0])
		free(argv_char[0]);
	argv_char[0] = NULL;

	char* argv_lang[1] = {NULL};
	argv_lang[0] = (char *)malloc(sizeof("lang"));
	memcpy(argv_lang[0],"lang",sizeof"lang");
	web_write_chunked("<SCRIPT language=Javascript src=\"");
	getLangInfo(1,argv_lang);
	web_write_chunked("\">");
	web_write_chunked("</SCRIPT>\n");
	if(argv_lang[0])
		free(argv_lang[0]);
	argv_lang[0] = NULL;

}

void OK_MSG1(char *msg, char *url) {
	cyg_httpd_start_chunked("html");
	web_write_chunked("<html>");

	mutil_lang_support();
	
   	web_write_chunked("<body bgColor=\"transparent\"><blockquote><h4>%s</h4>\n", msg); 
	if (url) web_write_chunked("<form><input type=button id='ok_msg1' OnClick=window.location.replace(\"%s\")></form></blockquote></body></html>", url);
	else web_write_chunked("<form><input type=button id='ok_msg1' OnClick=window.close()></form></blockquote></body></html>");
	web_write_chunked("<script>document.getElementById('ok_msg1').value=okmsg_btn;</script>");
	cyg_httpd_end_chunked();
}
#if defined(WLAN_PROFILE)
void ret_survey_page(char* pMsg,char* url,int connectOK,int wlan_id,int isWizard)
{
	if(strlen(url)==0) strcpy(url,"/wizard.htm");
	cyg_httpd_start_chunked("html");
	web_write_chunked("<html>\n<body bgColor=\"transparent\"><blockquote><h4>%s</h4>\n", pMsg);
	if(isWizard)  
		web_write_chunked("Your changes have been saved. The router must be rebooted for the changes to take effect.<br> You can reboot now, or you can continue to make other changes and reboot later.\n");
	web_write_chunked("<form action=/formSiteSurveyProfile.htm method=POST name='rebootSiteSurveyProfileForm'>");
	web_write_chunked("<input type='hidden' value='%s' name='submit-url'>",url);
	if(connectOK) web_write_chunked("<td><font size=2><b><input type=\"checkbox\" name=wizardAddProfile%d value=\"ON\">&nbsp;&nbsp;Add to Wireless Profile</b></td><br><br>\n", wlan_id);	
#ifdef HAVE_SYSTEM_REINIT
	if(!connectOK) web_write_chunked("<input id='restartLater' name='restartLater' type='submit' value='Apply Changes' onclick=\"return true\" />&nbsp;&nbsp;\n");
	if(connectOK) web_write_chunked("<input id='restartNow' name='restartNow' type='submit' value='Apply Changes' onclick=\"return true\" />&nbsp;&nbsp;\n");
#else
	if(!connectOK) web_write_chunked("<input id='restartLater' name='restartLater' type='submit' value='  OK  ' onclick=\"return true\" />&nbsp;&nbsp;\n");
	if(connectOK) web_write_chunked("<input id='restartNow' name='restartNow' type='submit' value='Reboot Now' onclick=\"return true\" />&nbsp;&nbsp;\n");
	if(connectOK) web_write_chunked("<input id='restartLater' name='restartLater' type='submit' value='Reboot Later' OnClick=window.location.replace(\"%s\");return true>\n", url);
#endif	
	web_write_chunked("</form>\n</blockquote>\n</body>\n</html>");
#ifdef CSRF_SECURITY_PATCH
		extern void log_boaform(char *form);
		log_boaform("formRebootCheck");
#endif

	cyg_httpd_end_chunked();
}
#endif
void OK_MSG(char *url) 
{ 
	char tmpbuf[BUF_SIZE];
	int len;
	MUST_REBOOT = 1;
	if(!url) url="/wizard.htm";


	cyg_httpd_start_chunked("html");
	web_write_chunked("<html>\n");

	mutil_lang_support();
	
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
		web_write_chunked("<link href=\"style_cmj.css\" rel=\"stylesheet\" type=\"text/css\">");
#endif	
	web_write_chunked("<blockquote>\n<body bgColor='transparent'>\n");
   	web_write_chunked("<script>dw(okmsg_explain)</script>\n");\
	web_write_chunked("<form action=/formRebootCheck.htm method=POST name='rebootForm'>\n"); \
	web_write_chunked("<input type='hidden' value='%s' name='submit-url'>\n",url); \
	web_write_chunked("<input id='restartNow' type='submit' onclick=\"return true\" >&nbsp;&nbsp;\n"); \
	web_write_chunked("<script>document.getElementById('restartNow').value=okmsg_reboot_now;</script>\n");\
	web_write_chunked("<input id='restartLater' type='button' OnClick=window.location.replace(\"%s\")>\n", url); \
	web_write_chunked("<script>document.getElementById('restartLater').value=okmsg_reboot_later;</script>\n");
	web_write_chunked("</form>\n</blockquote>\n</body>\n</html>");
#ifdef CSRF_SECURITY_PATCH
	extern void log_boaform(char *form);
	log_boaform("formRebootCheck");
#endif
	cyg_httpd_end_chunked();	
}

void OK_MSG2(char *msg, char *msg1,char* url) { 
	char tmp[200]; 
	sprintf(tmp, msg, msg1); 
	OK_MSG1(tmp, url); 
}
void OK_MSG_WAIT(char* msg,int c,char*redirect_ip,char* submit_url)
{
	
	cyg_httpd_start_chunked("html"); 
	web_write_chunked("<html>\n<head>\n");

	//mutil_lang_support();

	web_write_chunked("<script language=JavaScript>\n");
	web_write_chunked("var count = %d;\nfunction get_by_id(id)\n{\nwith(document)\n{\nreturn getElementById(id);\n}\n}\n",c);
	web_write_chunked("function do_count_down()\n{\nget_by_id(\"show_sec\").innerHTML = count;\n");
#ifdef SERVER_SSL
	web_write_chunked("if(count == 0) \n{\nlocation.href='https://%s%s';\n return false;\n}\n",redirect_ip, submit_url);
#else
	web_write_chunked("if(count == 0) \n{\nlocation.href='http://%s%s';\n return false;\n}\n",redirect_ip, submit_url);
#endif
	web_write_chunked("if (count > 0) \n{\ncount--;\nsetTimeout('do_count_down()',1000);\n}\n}\n");
	web_write_chunked("</script>\n</head>\n");
	web_write_chunked("<body onload=\"do_count_down();\" bgColor=\"transparent\">\n<blockquote>\n<h4>%s</h4>\n",msg);
	web_write_chunked("<P align=left><h4>Please wait <B><SPAN id=show_sec></SPAN></B>&nbsp;seconds ...</h4></P>\n");
	web_write_chunked("</blockquote>\n</body>\n</html>\n");
	cyg_httpd_end_chunked();
}

void OK_MSG_FW(char*msg, int c,char* ip)
{
	
	cyg_httpd_start_chunked("html"); 
	web_write_chunked("<html>\n<head>\n");
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
		web_write_chunked("<style type=\"text/css\">\
		html { font-size:12px; font-family:Microsoft YaHei,calibri,verdana;}\
		h2 {font-size:25px;font-weight:bold;color:rgb(168,192,6);}\
		.explain {color:rgb(168,192,6);font-weight:bold;}\
		</style>");
#endif	

	//mutil_lang_support();

	web_write_chunked("<script language=JavaScript>\n");
	web_write_chunked("var count = %d;\nfunction get_by_id(id)\n{\nwith(document)\n{\nreturn getElementById(id);\n}\n}\n",c);
	web_write_chunked("function do_count_down()\n{\nget_by_id(\"show_sec\").innerHTML = count;\n");
#ifdef SERVER_SSL
	web_write_chunked("if(count == 0) \n{\nparent.location.href='https://%s/home.htm?';\n return false;\n}\n", ip);
#else
	web_write_chunked("if(count == 0) \n{\nparent.location.href='http://%s/home.htm?t='+new Date().getTime();\n return false;\n}\n", ip);
#endif

	web_write_chunked("if (count > 0) \n{\ncount--;\nsetTimeout('do_count_down()',1000);\n}\n}\n");
	web_write_chunked("</script>\n</head>\n");
	
	web_write_chunked("<body onload=\"do_count_down();\" bgColor=\"transparent\">\n<blockquote>\n<h4>%s</h4>\n", msg);
	web_write_chunked("<P align=left><h4>Please wait <B><SPAN id=show_sec></SPAN></B>&nbsp;seconds ...</h4></P>\n");
	web_write_chunked("</blockquote>\n</body>\n</html>\n");
	cyg_httpd_end_chunked();
}

void ERR_MSG(char *tmpbuf)
{
	cyg_httpd_start_chunked("html"); 
	web_write_chunked("<html>");

	mutil_lang_support();
	
	web_write_chunked("<body bgColor=\"transparent\"><blockquote><h4>%s</h4>\n", tmpbuf);
	web_write_chunked("<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body></html>");
	cyg_httpd_end_chunked();
}

void ERR_MSG2(char *tmpbuf)
{
	cyg_httpd_start_chunked("html"); 
	web_write_chunked("<html>");

	mutil_lang_support();

	web_write_chunked("<body bgColor=\"transparent\"><blockquote><h4>%s</h4>\n", tmpbuf);
	//web_write_chunked("<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body></html>");
	web_write_chunked("<form><input type=\"button\" OnClick=window.location.replace(\"/wizard.htm\") value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body></html>");
	cyg_httpd_end_chunked();
}

void CONNECT_OK_MSG(char *tmpbuf)
{ 
	MUST_REBOOT = 1;

	cyg_httpd_start_chunked("html");
	web_write_chunked("<html>\n");

	mutil_lang_support();
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
	web_write_chunked("<link href=\"style_cmj.css\" rel=\"stylesheet\" type=\"text/css\">");
#endif	
	web_write_chunked("<body bgColor=\"transparent\"><blockquote><h2>%s</h2><font size=\"2\">Your changes have been saved. The router must be rebooted for the changes to take effect.<br> You can reboot now, or you can continue to make other changes and reboot later.\n</font>", tmpbuf);
	web_write_chunked("<form action=/formRebootCheck.htm method=POST name='rebootForm'>\n"); \
	web_write_chunked("<input type='hidden' value='%s' name='submit-url'>\n","/wizard.htm"); \
	web_write_chunked("<input id='restartNow' type='submit' onclick=\"return true\" >&nbsp;&nbsp;\n"); \
	web_write_chunked("<script>document.getElementById('restartNow').value=okmsg_reboot_now;</script>\n");\
	web_write_chunked("<input id='restartLater' type='button' OnClick=window.location.replace(\"%s\")>\n", "/wizard.htm"); \
	web_write_chunked("<script>document.getElementById('restartLater').value=okmsg_reboot_later;</script>\n");
	web_write_chunked("</form>\n</blockquote>\n</body>\n</html>");
#ifdef CSRF_SECURITY_PATCH
	extern void log_boaform(char *form);
	log_boaform("formRebootCheck");
#endif
	cyg_httpd_end_chunked();	
}


void send_redirect_perm(char* url)
{
	char *ptr;
	/*assume url is like http://x.x.x.x/xxx.htm*/
	ptr=strrchr(httpstate.url,'/');
	if(NULL == ptr)
	{
		diag_printf("not foud /\n");
		return ;
	}
	strcpy((ptr-1),url);
	cyg_httpd_send_error(CYG_HTTPD_STATUS_MOVED_TEMPORARILY);
}

 int  apmib_update_web(int type)
{
	int ret;

	ret = apmib_update(type);

	return ret;
}

 /************************************************************
 ** get value form webpage and set value to mib,
 ** if ip_val!=NULL set ip_val as output
 *************************************************************/
 int handle_fmGetValAndSetIp(char*name,int mibid,void* ip_val,char *postData, int len,char* errMsgBuf)
 {
	 char*strIp = NULL;
	 struct in_addr ip_addr={0};
	 if(!errMsgBuf)
	 {
		diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
		return (-1);
	 }
	 if(!name||!postData) 
	 {
		 strcpy(errMsgBuf, ("Invalid input value!"));
		 return -1;
	 }
	 strIp = get_cstream_var(postData,len,name,"");
	 if(strIp[0])
	 {//get value
		 if ( !inet_aton(strIp, &ip_addr) ) {
			 sprintf(errMsgBuf, ("Invalid IP address value! %s"),strIp);
			 return -1;
		 }
 
		 if ( !apmib_set(mibid, (void *)&ip_addr)) {
				 sprintf(errMsgBuf, "Set IP MIB %s error!",name);
			 return -1;
		 }
		 if(ip_val)
			 memcpy(ip_val,&ip_addr,sizeof(ip_addr));
		return 1;
	 }
	 
	 return 0;
 }
 
 int handle_fmGetValAndSetChkbox(char*name,int mibid,int* chkbox_val,char *postData, int len,char* errMsgBuf)
 {
	 char*strVal = NULL;
	 int intVal=0;	 
	 if(!errMsgBuf)
	 {
		diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
		return (-1);
	 }	 
	 if(!name||!postData)
	 {
		 strcpy(errMsgBuf, ("Invalid input value!"));
		 return -1;
	 }
	 strVal = get_cstream_var(postData,len, name, "");
	 if ( !strcmp(strVal, "ON"))
		 intVal = 1;
	 else
		 intVal = 0;
	 //printf("%s:%d name=%s val=%d\n",__FILE__,__LINE__,name,intVal);
	 if ( !apmib_set(mibid, (void *)&intVal)) {
		 sprintf(errMsgBuf, "Set %s mib error!",name);
		 return -1;
	 }
	 if(chkbox_val)
		 memcpy(chkbox_val,&intVal,sizeof(intVal));
	 return 0;
 }
 int handle_fmGetValAndSetMac(char*name,int mibid,void* mac_val,char *postData, int len,char* errMsgBuf)
 {
	 char*strMac=NULL; 
	 char tmpbuf[12]={0};
	 if(!errMsgBuf)
	 {
		diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
		return (-1);
	 }	 
	 if(!name||!postData)
	 {
		 strcpy(errMsgBuf, ("Invalid input value!"));
		 return -1;
	 }
	 strMac = get_cstream_var(postData,len,name,"");
	 if(strMac[0])
	 {//get value
		 if (strlen(strMac)!=12 || !string_to_hex(strMac, tmpbuf, 12)) {
			 strcpy(errMsgBuf, ("Error! Invalid MAC address."));
			 return -1;
		 }
		 if ( !apmib_set(mibid, (void *)tmpbuf)) {
			 sprintf(errMsgBuf, "Set %s mib error!",name);
			 return -1;
		 }
		 if(mac_val)
			 memcpy(mac_val,tmpbuf,6);

		if(!strcmp(name, "lan_macAddr"))
	 	{

			 int i;
			 int j;
#ifdef CONFIG_WLANIDX_MUTEX
			 int s = apmib_save_idx();
#else
			apmib_save_idx();
#endif
			 if( !memcmp(strMac,"000000000000",12))
			 {
			
				 for(i=0;i<NUM_WLAN_INTERFACE;i++)
				 {
#ifdef UNIVERSAL_REPEATER				 
					 //set wlan0-vxd0 MAC for ecos wlan0 mac don't affect wlan0-vxd mac.
					 unsigned char vxd_interface[12];
					 sprintf(vxd_interface, "wlan%d-vxd0", i);
					 SetWlan_idx(vxd_interface);
					 apmib_set(MIB_WLAN_WLAN_MAC_ADDR, (void *)tmpbuf);
#endif
					 apmib_set_wlanidx(i);
					 for(j=0;j<NUM_VWLAN_INTERFACE;j++)
					 {
						 apmib_set_vwlanidx(j);
						 if ( !apmib_set(MIB_WLAN_WLAN_MAC_ADDR, (void *)tmpbuf)) {
							 strcpy(errMsgBuf, ("Set MIB_WLAN_WLAN_MAC_ADDR mib error!"));
							 return -1;
						 }
					 }
				 }

			 }
			 else
			 {
				 //remove mac[5]++ for eth0 and wlan0 should use the same mac
#ifndef CONFIG_SAME_LAN_MAC
                 tmpbuf[5]++;
#endif				
				 for(i=0;i<NUM_WLAN_INTERFACE;i++)
				 {
#ifdef UNIVERSAL_REPEATER				 
					//set wlan0-vxd0 MAC for ecos wlan0 mac don't affect wlan0-vxd mac.
					unsigned char vxd_interface[12];
					sprintf(vxd_interface, "wlan%d-vxd0", i);
					SetWlan_idx(vxd_interface);
					apmib_set(MIB_WLAN_WLAN_MAC_ADDR, (void *)tmpbuf);
#endif
					 apmib_set_wlanidx(i);
					 for(j=0;j<NUM_VWLAN_INTERFACE;j++)
					 {
						 apmib_set_vwlanidx(j);						 
						 if ( !apmib_set(MIB_WLAN_WLAN_MAC_ADDR, (void *)tmpbuf)) {
							 strcpy(errMsgBuf, ("Set MIB_WLAN_WLAN_MAC_ADDR mib error!"));
							 return -1;
						 }
						 tmpbuf[5]++;
				 	}
					 tmpbuf[5]-=NUM_VWLAN_INTERFACE;
					 tmpbuf[5]+=0x10;
				 }
			 }
#ifdef CONFIG_WLANIDX_MUTEX
			apmib_revert_idx(s);
#else
			apmib_revert_idx();
#endif
		 }
		 return 1;
	 }
	 
	 return 0;
 }
 int handle_fmGetValAndSetIntVal(char * name, int mibid, int *intVal, char * postData, int len, char * errMsgBuf)
 {
	 char * strVal = NULL;
	 if(!errMsgBuf)
	 {
		diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
		return (-1);
	 }

	 if(!name||!postData)
	 {
		 strcpy(errMsgBuf, ("Invalid input value!"));
		 return -1;
	 }
	 strVal = get_cstream_var(postData,len,name,"");
	 if ( strVal[0] ) 
	 {//get value
		 int tmpVal=strtol(strVal, (char**)NULL, 10);
		 if ( !apmib_set(mibid, (void *)&tmpVal)) 
		 {
			 sprintf(errMsgBuf, "Set %s MIB error!",name);
			 return -1;
		 }
		 if(intVal) *intVal=tmpVal;
		 return 1;
	 }	 
	 return 0;
 }
int checkHostName(char *str)//check hostName'format,it should contain a-zA-Z0-9 only!!
{
	int i, len=strlen(str);
	for (i=0; i<len; i++) 
	{
		if((str[i]>='0' && str[i]<='9')||(str[i]>='a' && str[i]<='z')||(str[i]>='A' && str[i]<='Z'))
			continue;
		return 0;
	}
	return 1;
}
 int handle_fmGetValAndSetStrVal(char * name, int mibid, char *strValue, char * postData, int len, char * errMsgBuf)
 {
	 char *strVal= NULL; 
	 if(!errMsgBuf)
	 {
		diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
		return (-1);
	 }

	 if(!name||!postData)
	 {
		 strcpy(errMsgBuf, ("Invalid input value!"));
		 return -1;
	 }
	 strVal = get_cstream_var(postData,len,name,"");

	if(strcmp(name,"hostName")==0)//check hostName'format,it should contain a-zA-Z0-9 only!!
	{
	 	if(checkHostName(strVal)==0)
	 	{
			sprintf(errMsgBuf, "Invalid %s Name! Please enter characters in A(a)~Z(z) or 0-9 without spacing.",name);
			return -1;				
	 	}	
	}
	// if(strVal[0])
	 {//get value
		 if (!isValidName(strVal)) 
		 {
			 sprintf(errMsgBuf, "Invalid %s Name! Please enter characters in A(a)~Z(z) or 0-9 without spacing.",name);
			 return -1; 			 
		 }			 
		 if ( !apmib_set(mibid, (void *)strVal)) 
		 {
			 sprintf(errMsgBuf, "Set %s MIB error!",name);
			 return -1;
		 }
		 if(strValue) strcpy(strValue,strVal);
	 }
	 return 0;
 }
 int handle_fmGetValAndSetSsidVal(char * name, int mibid, char *strValue, char * postData, int len, char * errMsgBuf)
 {
 	
	 char *strVal =NULL;
	 if(!errMsgBuf)
	 {
		diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
		return (-1);
	 }	 
	 if(!name||!postData)
	 {
		 strcpy(errMsgBuf, ("Invalid input value!"));
		 return -1;
	 }
	 
	 strVal=get_cstream_var(postData,len,name,"");
	 if(strVal[0])
	 {//get value
		 if (!isValidSsidName(strVal)) 
		 {
			 sprintf(errMsgBuf, "Invalid %s Name! Please enter characters in A(a)~Z(z) or 0-9 without spacing.",name);
			 return -1; 			 
		 }			 
		 if ( !apmib_set(mibid, (void *)strVal)) 
		 {
			 sprintf(errMsgBuf, "Set %s MIB error!",name);
			 return -1;
		 }
		 if(strValue) strcpy(strValue,strVal);
		 return 1;
	 }
	 return 0;
 }
int handle_setWlanIntVal(int intVal,int mibid,int wlanIdx, char * errMsgBuf)
{
	if(!errMsgBuf)
	{
		printf("Invalid input value!\n");
		return -1;
	}
	apmib_set_wlanidx(wlanIdx);
	if ( !apmib_set(mibid, (void *)&intVal)) 
	{
		 sprintf(errMsgBuf, "Set %d MIB error!",mibid);
		 return -1;
	}
	return 0;
}

