/*
 *      Web server handler routines for Dynamic DNS 
 *
 *      Authors: Shun-Chin  Yang	<sc_yang@realtek.com.tw>
 *
 *      $Id
 *
 */

#include <string.h>
#include <stdlib.h>


#include "apmib.h"
#include "sys_init.h"


#define _DDNS_SCRIPT_PROG	"ddns.sh"

void formDdns(char *postData, int len)
{
	char *submitUrl;
	char tmpBuf[100];

	int enabled=0 ,ddnsType=0 ;
	char *tmpStr ;
	       
	submitUrl = get_cstream_var(postData,len,"submit-url","");   // hidden page
	
	tmpStr = get_cstream_var(postData,len, "ddnsEnabled", "");  
	if(!strcmp(tmpStr, "ON"))
		enabled = 1 ;
	else 
		enabled = 0 ;

	if ( apmib_set(MIB_DDNS_ENABLED, (void *)&enabled) == 0) {
		strcpy(tmpBuf, "Set enabled flag error!");
		goto setErr_ddns;
	}
	
	if(enabled){
		tmpStr = get_cstream_var(postData,len, "ddnsType", "");  
		if(tmpStr[0]){
		ddnsType = tmpStr[0] - '0' ;
	 		if ( apmib_set(MIB_DDNS_TYPE, (void *)&ddnsType) == 0) {
					strcpy(tmpBuf, "Set DDNS Type error!");
					goto setErr_ddns;
			}
		}
		tmpStr = get_cstream_var(postData,len, "ddnsUser", "");  
		if(tmpStr[0]){
			if ( apmib_set(MIB_DDNS_USER, (void *)tmpStr) == 0) {
					strcpy(tmpBuf, "Set DDNS User String error!");
					goto setErr_ddns;
			}
		}
		tmpStr = get_cstream_var(postData,len, "ddnsPassword", "");  
		if(tmpStr[0]){
			if ( apmib_set(MIB_DDNS_PASSWORD, (void *)tmpStr) == 0) {
					strcpy(tmpBuf, "Set DDNS Password String error!");
					goto setErr_ddns;
			}	
		}
		tmpStr = get_cstream_var(postData,len, "ddnsDomainName", "");  
		if(tmpStr[0]){
			if ( apmib_set(MIB_DDNS_DOMAIN_NAME, (void *)tmpStr) == 0) {
					strcpy(tmpBuf, "Set DDNS Password String error!");
					goto setErr_ddns;
			}	
		}		
	}

	apmib_update_web(CURRENT_SETTING);

#if 0
	run_init_script("all");
#endif

#ifdef HAVE_SYSTEM_REINIT	
	wait_redirect("Apply Changes",10,submitUrl);
	sleep(1);
	kick_reinit_m(SYS_WAN_M);
#else
	OK_MSG(submitUrl);
#endif

	save_cs_to_file();


	return;

setErr_ddns:
	ERR_MSG(tmpBuf);
}
