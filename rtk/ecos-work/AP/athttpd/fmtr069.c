#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys_utility.h>
#include <sys_init.h>

#include "asp.h"
#include "asp_form.h"
#include "fmget.h"
//#include "boa.h"
//#include "apform.h"
#include "utility.h"
#include "multipart.h"
#include "asp.h"


#define CWMP_PPPOE_STR  "pppoe"
#define CWMP_DHCP_STR   "dhcp"
#define CWMP_STATIC_IP_STR      "static_ip"
#define CWMP_ACS_ADDR "/var/system/cwmp_ACS_Addr"
#define req_get_cstream_var(wp, format, args...) get_cstream_var(postData, len, format, ##args)
#define req_format_write(wp, format, args...) __req_format_write(format, ##args)
#define _CWMP_WITH_SSL_ 1
#define REBOOT_TO_DISABLE 1

#ifdef ECOS_CWMP_WITH_SSL
extern unsigned char cacert_pem[];
extern unsigned char client_pem[];
#endif

static int __req_format_write(char *format, ...) 
{
        int len; 
        char tmpbuf[TMP_BUF_SIZE];
    	va_list args;

    if (!format)
        return 0;

    va_start(args, format);
    len = vsnprintf(tmpbuf, sizeof(tmpbuf), format, args);
    va_end(args);

        if (len < 0) 
                return len;

        return cyg_httpd_write_chunked(tmpbuf, len);
}

enum {LINK_INIT=0, LINK_NO, LINK_DOWN, LINK_WAIT, LINK_UP};
#define CONFIG_DIR      "/var/cwmp_config"
#define CA_FNAME        CONFIG_DIR"/cacert.pem"
#define CERT_FNAME      CONFIG_DIR"/client.pem"
#define strACSURLWrong  "ACS's URL can't be empty!"
#define strSSLWrong "CPE does not support SSL! URL should not start with 'https://'!"
#define strSetACSURLerror "Set ACS's URL error!"
#define strSetUserNameerror "Set User Name error!"
#define strSetPasserror "Set Password error!"
#define strSetInformEnableerror "Set Inform Enable error!"
#define strSetInformIntererror "Set Inform Interval error!"
#define strSetConReqUsererror "Set Connection Request UserName error!"
#define strSetConReqPasserror "Set Connection Request Password error!"
#define strSetCWMPFlagerror "Set CWMP_FLAG error!"
#define strGetCWMPFlagerror "Get CWMP_FLAG error!"
#define strUploaderror "Upload error!"
#define strMallocFail "malloc failure!"
#define strArgerror "Insufficient args\n"
#define strSetCerPasserror  "Set CPE Certificat's Password error!"
#define BUF_SIZE 4096

//const char IFCONFIG[] = "/bin/ifconfig";
//const char IPTABLES[] = "/bin/iptables";
#if 0
const char ARG_T[] = "-t";
const char FW_DEL[] = "-D";
const char FW_PREROUTING[] = "PREROUTING";
const char ARG_I[] = "-i";
const char LANIF[] = "br0";
const char ARG_TCP[] = "TCP";
const char ARG_UDP[] = "UDP";
const char FW_DPORT[] = "--dport";
const char RMACC_MARK[] = "0x1000";
const char FW_ADD[] = "-A";
#endif

//#define RECONNECT_MSG(url) { \
        req_format_write(wp, ("<html><body><blockquote><h4>Change setting successfully!" \
                "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body></html>"), url);\
}

void RECONNECT_MSG(char *url) 
{ 
	char tmpbuf[BUF_SIZE];
	int len;
	MUST_REBOOT = 1;
	if(!url) url="/wizard.htm";


	cyg_httpd_start_chunked("html");
	web_write_chunked("<html>\n");

	mutil_lang_support();
	web_write_chunked("<body><blockquote><h4>Change setting successfully!\n");
	web_write_chunked("<form>\n"); \
	web_write_chunked("<input type=button value=\" OK \" OnClick=window.location.replace(\"%s\")>",url); \
	web_write_chunked("</form></blockquote></body>\n"); \
	web_write_chunked("</html>");
#ifdef CSRF_SECURITY_PATCH
	extern void log_boaform(char *form);
	log_boaform("formRebootCheck");
#endif
	cyg_httpd_end_chunked();	
}

//#define UPLOAD_MSG(url) { \
        req_format_write(wp, ("<html><body><blockquote><h4>Upload a file successfully!" \
                "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body></html>"), url);\
}

void UPLOAD_MSG(char *url) 
{ 
	char tmpbuf[BUF_SIZE];
	int len;
	MUST_REBOOT = 1;
	if(!url) url="/wizard.htm";


	cyg_httpd_start_chunked("html");
	web_write_chunked("<html>\n");

	mutil_lang_support();
	web_write_chunked("<body><blockquote><h4>Upload successfully!\n");
	web_write_chunked("<form>\n"); \
	web_write_chunked("<input type=button value=\" OK \" OnClick=window.location.replace(\"%s\")>",url); \
	web_write_chunked("</form></blockquote></body>\n"); \
	web_write_chunked("</html>");
#ifdef CSRF_SECURITY_PATCH
	extern void log_boaform(char *form);
	log_boaform("formRebootCheck");
#endif
	cyg_httpd_end_chunked();	
}
#define CWMPPID  "/var/run/cwmp.pid"
extern cyg_handle_t    tr069_thread;

#if 0
void off_tr069(void)
{
/*
	printf("off_tr069\n");
	int ret;
        cyg_handle_t handle1;
        cyg_handle_t handle2;
	cyg_thread_info info1;
	cyg_thread_info info2;

	ret = get_thread_info_by_name("cwmpClient", &info1);
	handle1 = info1.handle;
	if(&handle1 != NULL){
		//cyg_thread_kill(handle1);	
		ret = cyg_thread_delete(handle1);	
		printf("delete cwmpClient, ret=%d\n", ret);
	}

        get_thread_info_by_name("cwmp_webclient", &info2);
	handle2 = info2.handle;
	if(&handle2 != NULL){
		//cyg_thread_kill(handle2);	
		ret = cyg_thread_delete(handle2);	
		printf("delete cwmp_webclient, ret=%d\n", ret);
	}

	sleep(3);

#if 0
	unsigned int cwmp_flag_in_mib=0;
        apmib_get(MIB_CWMP_FLAG, &cwmp_flag_in_mib);
	printf("off_tr069 1: MIB_CWMP_FLAG = %d\n", cwmp_flag_in_mib);
        cwmp_flag_in_mib = 0x1;
        apmib_set(MIB_CWMP_FLAG, &cwmp_flag_in_mib);
	printf("off_tr069 2: MIB_CWMP_FLAG = %d\n", cwmp_flag_in_mib);
        apmib_get(MIB_CWMP_FLAG, &cwmp_flag_in_mib);
	printf("off_tr069 3: MIB_CWMP_FLAG = %d\n", cwmp_flag_in_mib);
	printf("=================REBOOT=====================\n");

	reboot_wait_redirect("reboot_cwmp");
#endif
	printf("out off_tr069\n");
*/
	return;
}
#endif

int startCWMP(unsigned char urlChanged)
{
        char strPort[16];
        char gwIpAddr[32]={0};
        char wanName[16];
        char pppName[16];
        char cmdBuffer[64];
        unsigned int conreq_port=0;
        unsigned int cwmp_flag;
        unsigned int i,wan_num=0;
#if defined(MULTI_WAN_SUPPORT)  
        WANIFACE_T *p,wan_entity;
#endif  
        /*add a wan port to pass */
        //system("sysconf firewall");

        /*start the cwmpClient program*/
        apmib_get(MIB_CWMP_FLAG, (void *)&cwmp_flag);
        if( cwmp_flag&CWMP_FLAG_AUTORUN ){
#ifdef ECOS_CWMP_WITH_SSL
	       	FILE *fd = fopen("/var/cacert.pem", "r+");	
		strcat(cacert_pem, "\0");
		int len = strlen(cacert_pem);
		//printf("len of cacert_pem=%d\n", len);	
		//int ret=fwrite(cacert_pem, 1, len, fd);
#endif

               	kick_event(FIREWARE_EVENT);
               	va_cmd( "cwmpClient", 0, 0 ); 
	}

        return 0;
}

int TR069ConPageShow(int argc, char **argv)
{
        int nBytesSent=0;
        char *name;
        //unsigned int cwmp_flag;

        //printf("get parameter=%s\n", argv[0]);
        name = argv[0];
        if (name == NULL) {
                fprintf(stderr, strArgerror);
                return -1;
        }

#ifdef ECOS_CWMP_WITH_SSL
        if ( !strcmp(name, ("ShowMNGCertTable")) )
                return ShowMNGCertTable();
#endif

	return nBytesSent;
}


void formTR069Config(char* postData, int len)
{
	//printf("enter formTR069Config\n");
	char	*strData;
	char tmpBuf[100];
	char orig_acsUserName[64]={0};
	char orig_acsPassword[64]={0};
	char new_acsUserName[64]={0};
	char new_acsPassword[64]={0};
	char orig_ConReqUserName[64]={0};
	char orig_ConReqPassword[64]={0};
	char new_ConReqUserName[64]={0};
	char new_ConReqPassword[64]={0};
	unsigned char vChar;
	unsigned int cwmp_flag;
	int vInt;
	// Mason Yu
	char changeflag=0;
	unsigned char acsurlchangeflag=0;
	unsigned int informEnble;
	unsigned int informInterv;
	char cwmp_flag_value=1;
	char tmpStr[256+1];
	char origACSURL[256+1];
	char NewACSURL[256+1];
	int cur_port;
	char isDisConReqAuth=0;

//displayPostDate(wp->post_data);
	apmib_get( MIB_CWMP_ACS_URL, (void *)origACSURL);
#ifdef ECOS_CWMP_WITH_SSL
	//CPE Certificat Password
	strData = req_get_cstream_var(wp, ("CPE_Cert"), (""));
	if( strData[0] )
	{
		strData = req_get_cstream_var(wp, ("certpw"), (""));

		changeflag = 1;
		if ( !apmib_set( MIB_CWMP_CERT_PASSWORD, (void *)strData))
		{
			strcpy(tmpBuf, strSetCerPasserror);
			goto setErr_tr069;
		}
		else
			goto end_tr069;
	}
#endif

	strData = req_get_cstream_var(wp, ("url"), (""));
	//if ( strData[0] )
	{
		if ( strlen(strData)==0 )
		{
			strcpy(tmpBuf, (strACSURLWrong));
			goto setErr_tr069;
		}
#ifndef ECOS_CWMP_WITH_SSL
		if ( strstr(strData, "https://") )
		{
			strcpy(tmpBuf, (strSSLWrong));
			goto setErr_tr069;
		}
#endif
		if ( !apmib_set( MIB_CWMP_ACS_URL, (void *)strData))
		{
			strcpy(tmpBuf, (strSetACSURLerror));
			goto setErr_tr069;
		}
	}

	apmib_get( MIB_CWMP_ACS_URL, (void *)NewACSURL);
	if(strcmp(origACSURL, NewACSURL)){
		changeflag=1;
		acsurlchangeflag=1;
	}



	apmib_get( MIB_CWMP_ACS_PASSWORD, (void *)orig_acsUserName);
	apmib_get( MIB_CWMP_ACS_USERNAME, (void *)orig_acsPassword);

	apmib_get( MIB_CWMP_CONREQ_USERNAME, (void *)orig_ConReqUserName);
	apmib_get( MIB_CWMP_CONREQ_PASSWORD, (void *)orig_ConReqPassword);
	
	
	strData = req_get_cstream_var(wp, ("username"), (""));
	//if ( strData[0] )
	{
		if ( !apmib_set( MIB_CWMP_ACS_USERNAME, (void *)strData)) {
			strcpy(tmpBuf, (strSetUserNameerror));
			goto setErr_tr069;
		}
	}

	strData = req_get_cstream_var(wp, ("password"), (""));
	//if ( strData[0] )
	{
		if ( !apmib_set( MIB_CWMP_ACS_PASSWORD, (void *)strData)) {
			strcpy(tmpBuf, (strSetPasserror));
			goto setErr_tr069;
		}
	}

	strData = req_get_cstream_var(wp, ("enable"), (""));
	if ( strData[0] ) {
		informEnble = (strData[0]=='0')? 0:1;
		apmib_get( MIB_CWMP_INFORM_ENABLE, (void*)&vInt);
		if(vInt != informEnble){
			//int allow=1;
			changeflag = 1;
			if ( !apmib_set( MIB_CWMP_INFORM_ENABLE, (void *)&informEnble)) {
				strcpy(tmpBuf, (strSetInformEnableerror));
				goto setErr_tr069;
			}
		}
	}

	strData = req_get_cstream_var(wp, ("interval"), (""));
	if ( strData[0] ) {
		informInterv = atoi(strData);
		
		if(informEnble == 1){
			apmib_get( MIB_CWMP_INFORM_INTERVAL, (void*)&vInt);

			if(vInt != informInterv){
				changeflag = 1;
				if ( !apmib_set( MIB_CWMP_INFORM_INTERVAL, (void *)&informInterv)) {
					strcpy(tmpBuf, (strSetInformIntererror));
					goto setErr_tr069;
				}
			}
		}
	}

#ifdef _TR069_CONREQ_AUTH_SELECT_
	strData = req_get_cstream_var(wp, ("disconreqauth"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG2, (void *)&cwmp_flag ) )
		{
			changeflag = 1;

			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG2_DIS_CONREQ_AUTH);
			else{
				cwmp_flag = cwmp_flag | CWMP_FLAG2_DIS_CONREQ_AUTH;
				isDisConReqAuth = 1;
			}

			if ( !apmib_set( MIB_CWMP_FLAG2, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}
#endif

	//if connection reuqest auth is enabled, don't handle conreqname & conreqpw to keep the old values
	if(!isDisConReqAuth)
	{
		strData = req_get_cstream_var(wp, ("conreqname"), (""));
		//if ( strData[0] )
		{
			if ( !apmib_set( MIB_CWMP_CONREQ_USERNAME, (void *)strData)) {
				strcpy(tmpBuf, (strSetConReqUsererror));
				goto setErr_tr069;
			}
		}

		strData = req_get_cstream_var(wp, ("conreqpw"), (""));
		//if ( strData[0] )
		{
			if ( !apmib_set( MIB_CWMP_CONREQ_PASSWORD, (void *)strData)) {
				strcpy(tmpBuf, (strSetConReqPasserror));
				goto setErr_tr069;
			}
		}
	}//if(isDisConReqAuth)

	strData = req_get_cstream_var(wp, ("conreqpath"), (""));
	//if ( strData[0] )
	{
		apmib_get( MIB_CWMP_CONREQ_PATH, (void *)tmpStr);
		if (strcmp(tmpStr,strData)!=0){
			changeflag = 1;
			if ( !apmib_set( MIB_CWMP_CONREQ_PATH, (void *)strData)) {
				strcpy(tmpBuf, ("Set Connection Request Path error!"));
				goto setErr_tr069;
			}
		}
	}

	strData = req_get_cstream_var(wp, ("conreqport"), (""));
	if ( strData[0] ) {
		cur_port = atoi(strData);
		apmib_get( MIB_CWMP_CONREQ_PORT, (void *)&vInt);
		if ( vInt != cur_port ) {
			changeflag = 1;
			if ( !apmib_set( MIB_CWMP_CONREQ_PORT, (void *)&cur_port)) {
				strcpy(tmpBuf, ("Set Connection Request Port error!"));
				goto setErr_tr069;
			}
		}
	}

/*for debug*/
	strData = req_get_cstream_var(wp, ("dbgmsg"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_DEBUG_MSG);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_DEBUG_MSG;

			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}

#ifdef ECOS_CWMP_WITH_SSL
	strData = req_get_cstream_var(wp, ("certauth"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_CERT_AUTH);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_CERT_AUTH;

			changeflag = 1;
			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}
#endif

	strData = req_get_cstream_var(wp, ("sendgetrpc"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_SENDGETRPC);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_SENDGETRPC;

			if ( !apmib_set(MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}

	strData = req_get_cstream_var(wp, ("skipmreboot"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_SKIPMREBOOT);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_SKIPMREBOOT;

			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}

	strData = req_get_cstream_var(wp, ("delay"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_DELAY);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_DELAY;

			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}
	strData = req_get_cstream_var(wp, ("autoexec"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			int onoff_tr069 = 0;
			if(strData[0]=='0') {
				if ( cwmp_flag & CWMP_FLAG_AUTORUN )
					changeflag = 1;

				cwmp_flag = cwmp_flag & (~CWMP_FLAG_AUTORUN);
				cwmp_flag_value = 0;
			}else {
				if ( !(cwmp_flag & CWMP_FLAG_AUTORUN) )
					changeflag = 1;

				cwmp_flag = cwmp_flag | CWMP_FLAG_AUTORUN;
				cwmp_flag_value = 1;
			}

			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
			
			onoff_tr069 = (cwmp_flag & CWMP_FLAG_AUTORUN)==0?0:1;
			//printf("==============================\n");
			apmib_set( MIB_CWMP_ENABLED, (void *)&onoff_tr069);
			//printf("==============================\n");
			
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}
/*end for debug*/
end_tr069:
	//printf("0\n");
	apmib_get( MIB_CWMP_ACS_PASSWORD, (void *)new_acsUserName);
	apmib_get( MIB_CWMP_ACS_USERNAME, (void *)new_acsPassword);
	
	//printf("1\n");
	if(orig_acsUserName[0] && orig_acsPassword[0] && new_acsUserName[0] && new_acsPassword[0]) {
		if((strcmp(orig_acsUserName, new_acsUserName)) || (strcmp(orig_acsPassword, new_acsPassword)))
			changeflag=1;
	}

	//printf("2\n");
	apmib_get( MIB_CWMP_CONREQ_USERNAME, (void *)new_ConReqUserName);
	apmib_get( MIB_CWMP_CONREQ_PASSWORD, (void *)new_ConReqPassword);
	if(orig_ConReqUserName[0] && orig_ConReqPassword[0] && new_ConReqUserName[0] && new_ConReqPassword[0]) {
		if((strcmp(orig_ConReqUserName, new_ConReqUserName)) || (strcmp(orig_ConReqPassword, new_ConReqPassword)))
			changeflag=1;
	}
	

	//printf("3\n");

	if ( changeflag ) {
		apmib_update_web(CURRENT_SETTING);
		char *submitUrl = get_cstream_var(postData,len,  ("submit-url"), "");
#ifdef HAVE_SYSTEM_REINIT
		{
			wait_redirect("Apply Changes", 5,submitUrl);
			sleep(1);
			kick_reinit_m(SYS_TR069_M);
		}
#else
		OK_MSG(submitUrl);
#endif
	}


// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	//printf("4\n");
	Commit();
#endif

	//printf("5\n");
	apmib_update_web(CURRENT_SETTING);
	
	//printf("6\n");
	strData = req_get_cstream_var(wp, ("submit-url"), (""));
	//printf("strData=%s\n", strData);
	RECONNECT_MSG(strData);// display reconnect msg to remote
	//OK_MSG(strData);
	//printf("7\n");
	return;



setErr_tr069:
	ERR_MSG(tmpBuf);
}

/*******************************************************/
/*show extra fileds at tr069config.htm*/
/*******************************************************/
#ifdef ECOS_CWMP_WITH_SSL
void formTR069CPECert(char *postData, int len)
{
	// not used
	return ;
} //end formTR069CPECert

void formTR069CACert(char *postData, int len)
{
#if 0
        off_tr069();

        if (startCWMP(0) == -1)
        {
                strcpy(tmpBuf, ("Start tr069 Fail *****"));
                printf("Start tr069 Fail *****\n");
                goto setErr_tr069ca;
        }

#else
	char *strData;
	//printf("len=%d, in fmtr069.c\n", len);
	MULTIPART_T mime;	
	COMPRESS_MIB_HEADER_Tp pCompHeader=NULL;
	char *binData=NULL;
	int  i=0, dataLen=0;	
	unsigned char *endPos=NULL;
	unsigned char *curr=NULL;
	int count=0;
	char tmpBuf[256]={0};
	char lan_ip_buf[30]={0}, lan_ip[30]={0};
	parse_MIME(&mime,postData,len);	
        
	for(i=0;i<mime.entry_count;i++)
	{
		if(strncmp(mime.entry[i].name,"binary",mime.entry[i].name_len)==0){
			binData=mime.entry[i].value;
			break;
		}
	}
	if(!binData)
	{
		diag_printf("wrong file!!\n");
		return;
	}

	const unsigned char endCertificate[] = "END CERTIFICATE";
	endPos = strstr(binData, endCertificate);	
	curr=binData;
	if(endPos!=NULL)
	{
		while(curr!=endPos){
			curr++;
			count++;
		}
	}else
		diag_printf("Illegal input file!!!!\n");
	//printf("cacert_pem=%p\n", cacert_pem);
	//printf("cacert_pem[0]=%d\n", cacert_pem[0]);
	//printf("count=%d\n", count); //except "END CERTIFICATE-----\n"
	
	strncpy(cacert_pem, binData, count+strlen(endCertificate)+6); //6 is for "-----\n"
	//for(count=0; cacert_pem[count]!='\0'; count++)
	//	printf("%c", cacert_pem[count]);	
	//printf("\n");

	strData = req_get_cstream_var(wp, ("submit-url"), ("/tr069config.htm"));
        UPLOAD_MSG(strData);// display reconnect msg to remote
        return;

#endif
#if 0
setErr_tr069ca:
        ERR_MSG(tmpBuf);
#endif
} //end formTR069CACert

//int ShowMNGCertTable(request *wp)
int ShowMNGCertTable(void)
{
        int nBytesSent=0;
        char buffer[256]="";
	int wp;

        apmib_get(MIB_CWMP_CERT_PASSWORD,buffer);

        nBytesSent += req_format_write(wp, ("\n"
                "<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n"
                "  <tr><hr size=1 noshade align=top></tr>\n"
                "  <tr>\n"
                "      <td width=\"30%%\"><font size=2><b>Certificat Management:</b></td>\n"
                "      <td width=\"70%%\"><b></b></td>\n"
                "  </tr>\n"
                "\n"));
	
        nBytesSent += req_format_write(wp, ("\n"
                "  <tr>\n"
                "      <td width=\"30%%\"><font size=2><b>CA Certificat:</b></td>\n"
                "      <td width=\"70%%\"><font size=2>\n"
                "           <form action=/formTR069CACert.htm method=POST enctype=\"multipart/form-data\" name=\"ca_cert\">\n"
                "           <input type=\"file\" name=\"binary\" size=24>&nbsp;&nbsp;\n"
                "           <input type=\"submit\" value=\"Upload\" name=\"load\">\n"
                "           </form>\n"
                "      </td>\n"
                "  </tr>\n"
                "\n"));

        nBytesSent += req_format_write(wp, ("\n"
                "</table>\n"
                "\n"));


        return nBytesSent;	
} //end ShowMNGCertTable
#endif
