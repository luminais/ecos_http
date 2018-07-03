/*function about management*/

#include <network.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include "http.h"
#include "apmib.h"
#include "sys_utility.h"
#include "sys_init.h"
#include "multipart.h"
#include "common.h"
int Reboot_Wait=0;
extern unsigned char last_host[4];
extern char* BRIDGE_IF;
/*NTP BLOCK*/

#if defined(ECOS_MEM_CHAIN_UPGRADE)
extern int formupload_upgrade_flag;
#endif

int  ntpHandler(char *postData, int len, char *tmpBuf, int fromWizard)
{
	int enabled=0, ntpServerIdx ;
	struct in_addr ipAddr ;
	char *tmpStr ;
//Brad add for daylight save	
	int dlenabled=0;
//Brad add end	
	if (fromWizard) {
		tmpStr = get_cstream_var(postData,len, ("enabled"), "");  
		if(!strcmp(tmpStr, "ON"))
			enabled = 1 ;
		else 
			enabled = 0 ;

		if ( apmib_set( MIB_NTP_ENABLED, (void *)&enabled) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_ntp;
		}
//Brad add for daylight save		
		tmpStr = get_cstream_var(postData,len, ("dlenabled"), "");  
		if(!strcmp(tmpStr, "ON"))
			dlenabled = 1 ;
		else 
			dlenabled = 0 ;

		if ( apmib_set( MIB_DAYLIGHT_SAVE, (void *)&dlenabled) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_ntp;
		}
//Brad add end		
	}	
	else
		enabled = 1;
	if(enabled){
		tmpStr = get_cstream_var(postData,len,("ntpServerId"), "");  
		if(tmpStr[0]){
			ntpServerIdx = tmpStr[0] - '0' ;
			if ( apmib_set(MIB_NTP_SERVER_ID, (void *)&ntpServerIdx) == 0) {
				strcpy(tmpBuf, ("Set Time Zone error!"));
				goto setErr_ntp;
			}
		}
		tmpStr = get_cstream_var(postData,len, ("timeZone"), "");  
		if(tmpStr[0]){
			if ( apmib_set(MIB_NTP_TIMEZONE, (void *)tmpStr) == 0) {
					strcpy(tmpBuf, ("Set Time Zone error!"));
				goto setErr_ntp;
		}
		}

		tmpStr =get_cstream_var(postData,len, ("ntpServerIp1"), "");  
		if(tmpStr[0]){
			inet_aton(tmpStr, &ipAddr);
			if ( apmib_set(MIB_NTP_SERVER_IP1, (void *)&ipAddr) == 0) {
				strcpy(tmpBuf, ("Set NTP server error!"));
				goto setErr_ntp;
			} 
			}
		tmpStr = get_cstream_var(postData,len,("ntpServerIp2"), "");  
		if(tmpStr[0]){
			inet_aton(tmpStr, &ipAddr);
			if ( apmib_set(MIB_NTP_SERVER_IP2,(void *) &ipAddr ) == 0) {
				strcpy(tmpBuf, ("Set NTP server IP error!"));
				goto setErr_ntp;
			}
		}
	}
	return 0 ;	
setErr_ntp:
	return -1 ;
	
}
void formRebootCheck(char *postData, int len)
{
	char *tmpBuf="<h2>Changed setting successfully!</h2><br><br><font size=\"2\">The Router is booting.<br>Do not turn off or reboot the Device during this time.<br></font>";
	apmib_update_web(CURRENT_SETTING);
	reboot_wait_redirect(tmpBuf);
	return;
}

void formNtp(char *postData, int len)
{
	char *submitUrl,*strVal, *tmpStr;
	char tmpBuf[100];
	int enabled=0;
//Brad add for daylight save	
	int dlenabled=0;
//Brad add end	
#ifndef NO_ACTION
//	int pid;
#endif
	int time_value=0;
	int cur_year=0;
	
	submitUrl = get_cstream_var(postData,len, "submit-url", "");   // hidden page
	strVal = get_cstream_var(postData,len, ("save"), "");   

	if(strVal[0]){	
		tmpStr = get_cstream_var(postData,len,("timeZone"), ""); 
		if(tmpStr[0]){
#ifdef DAYLIGHT_SAVING_TIME
//			diag_printf("the timezone is %s\n", tmpStr);
			extern void get_start_end_time_point(char *);
			get_start_end_time_point(tmpStr);
#endif
			if ( apmib_set(MIB_NTP_TIMEZONE, (void *)tmpStr) == 0) {
					strcpy(tmpBuf, ("Set Time Zone error!"));
				goto setErr_end;
			}
			/*Ecos handle timezone*/
			//cyg_libc_time_setzoneoffsets(0,3600*atoi(tmpStr));
			/*end */
		}
		
		struct tm tm_time;
		time_t tm;
		memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
		tm_time.tm_sec = 0;
		tm_time.tm_min = 0;
		tm_time.tm_hour = 0;
		tm_time.tm_isdst = -1;  /* Be sure to recheck dst. */
		strVal = get_cstream_var(postData,len, ("year"), "");	
		cur_year= atoi(strVal);
		tm_time.tm_year = atoi(strVal) - 1900;
		strVal = get_cstream_var(postData,len,("month"), "");	
		tm_time.tm_mon = atoi(strVal)-1;
		strVal = get_cstream_var(postData,len, ("day"), "");	
		tm_time.tm_mday = atoi(strVal);
		strVal = get_cstream_var(postData,len,("hour"), "");	
		tm_time.tm_hour = atoi(strVal);
		strVal = get_cstream_var(postData,len, ("minute"), "");	
		tm_time.tm_min = atoi(strVal);
		strVal = get_cstream_var(postData,len, ("second"), "");	
		tm_time.tm_sec = atoi(strVal);
		tm = mktime_rewrite(&tm_time);
		if(tm < 0){
			sprintf(tmpBuf, "set Time Error\n");
			goto setErr_end;
		}
//		diag_printf("%s:%d the hour is %d\n",__FILE__,__LINE__,tm_time.tm_hour);
#ifdef DAYLIGHT_SAVING_TIME
		Cyg_libc_time_dst states;
		time_t stdOffset, dstOffset;
		struct tm *p_tm_time;
		states=cyg_libc_time_getzoneoffsets(&stdOffset,&dstOffset);
		if(states == CYG_LIBC_TIME_DSTON)
		{
			extern int get_dst_offset(const struct tm*, int);
			extern int get_dst_time_flag();
//			diag_printf("the dst flag is %d\n", get_dst_time_flag());
			if(get_dst_offset(&tm_time, 2) == 3600)
			{
				if(stdOffset == dstOffset)
					tm = tm - 3600;
				else
					tm = tm - 7200;
				
				p_tm_time = localtime(&tm);
				
				tm_time.tm_year = p_tm_time->tm_year;
				tm_time.tm_mon = p_tm_time->tm_mon;
				tm_time.tm_mday = p_tm_time->tm_mday;
				tm_time.tm_hour = p_tm_time->tm_hour;
				tm_time.tm_min = p_tm_time->tm_min;
				tm_time.tm_sec = p_tm_time->tm_sec;

//				diag_printf("%s:%d the hour is %d\n",__FILE__,__LINE__,p_tm_time->tm_hour);
				if(cyg_libc_time_settime(tm+3600) < 0){
					sprintf(tmpBuf, "set Time Error\n");
					goto setErr_end;
				}
			}
			else
			{
				if(cyg_libc_time_settime(tm) < 0){
					sprintf(tmpBuf, "set Time Error\n");
					goto setErr_end;
				}
			}
			
		}
		else
		{
#endif
			if(cyg_libc_time_settime(tm) < 0){
				sprintf(tmpBuf, "set Time Error\n");
				goto setErr_end;
			}
#ifdef DAYLIGHT_SAVING_TIME
		}
#endif
			
#if 0		
		if(stime(&tm) < 0){
			sprintf(tmpBuf, "set Time Error\n");
			goto setErr_end;
		}
//#else
		if(cyg_libc_time_settime(tm) < 0){
			sprintf(tmpBuf, "set Time Error\n");
			goto setErr_end;
		}	
#endif
		apmib_set( MIB_SYSTIME_YEAR, (void *)&cur_year);
		time_value = tm_time.tm_mon;
		apmib_set( MIB_SYSTIME_MON, (void *)&time_value);
		time_value = tm_time.tm_mday;
		apmib_set( MIB_SYSTIME_DAY, (void *)&time_value);
		time_value = tm_time.tm_hour;
		apmib_set( MIB_SYSTIME_HOUR, (void *)&time_value);
		time_value = tm_time.tm_min;
		apmib_set( MIB_SYSTIME_MIN, (void *)&time_value);
		time_value = tm_time.tm_sec;
		apmib_set( MIB_SYSTIME_SEC, (void *)&time_value);


		tmpStr = get_cstream_var(postData,len,("enabled"), "");  
		if(!strcmp(tmpStr, "ON"))
			enabled = 1 ;
		else 
			enabled = 0 ;
		if ( apmib_set( MIB_NTP_ENABLED, (void *)&enabled) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_end;
		}
#if 1

//Brad add for daylight save		
		tmpStr = get_cstream_var(postData,len, ("dlenabled"), "");  
		if(!strcmp(tmpStr, "ON"))
			dlenabled = 1 ;
		else 
			dlenabled = 0 ;
		if ( apmib_set( MIB_DAYLIGHT_SAVE, (void *)&dlenabled) == 0) {
			strcpy(tmpBuf, ("Set dl enabled flag error!"));
			goto setErr_end;
		}
//Brad add end		
#endif
	}
	if (enabled == 0)		
		goto  set_ntp_end;
	
	if(ntpHandler(postData,len, tmpBuf, 0) < 0)
		goto setErr_end ;
	
set_ntp_end:
	
	apmib_update_web(CURRENT_SETTING);
#ifdef HAVE_SYSTEM_REINIT
	wait_redirect("Apply Changes",12,submitUrl);
	sleep(1);
#ifdef CONFIG_ECOS_AP_SUPPORT
	kick_reinit_m(SYS_AP_LAN_M);
#else
	kick_reinit_m(SYS_WAN_M);
#endif
#else
	OK_MSG(submitUrl);
#endif

	save_cs_to_file();

	return;

setErr_end:
	ERR_MSG(tmpBuf);
}
void get_redirect_ip(char* redirect_ip)
{
	char redirect_ip_buf[30]={0};
	struct in_addr intaddr;
	unsigned char lan_ip[4];
	unsigned char from_lan=0;
	unsigned int  mask, gateway, dns1, dns2;
	
	int web_wan_access_enabled = 0;
#ifdef HOME_GATEWAY
	//apmib_update_web(CURRENT_SETTING);	// update configuration to flash
	apmib_get(MIB_WEB_WAN_ACCESS_ENABLED,(void *)&web_wan_access_enabled);
	if(web_wan_access_enabled)
	{
		getInAddr(BRIDGE_IF, IP_ADDR, (void *)&intaddr );
		memcpy(lan_ip,&intaddr,4);
		if(memcmp(lan_ip,last_host,4) ==0)
		{
			from_lan=1;
		}
	}
	else
#endif
		from_lan=1;
	
	if(from_lan)
	{
		int lan_type;                		  
		apmib_get(MIB_DHCP, (void *)&lan_type);

		if(lan_type==DHCP_LAN_CLIENT)
			get_WanInfo("/etc/lan_info","eth0",(void *)redirect_ip_buf, &mask, &gateway, &dns1, &dns2);
		else
			apmib_get( MIB_IP_ADDR,  (void *)redirect_ip_buf);  		
	 }
#ifdef HOME_GATEWAY
	else
#endif
	{
		int wan_type;
		
		apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
		if(wan_type==STATIC_IP)
		{
			apmib_get( MIB_WAN_IP_ADDR,  (void *)redirect_ip_buf);
		}
		else if(wan_type==DHCP_CLIENT)
		{
			get_WanInfo("/etc/wan_info","eth1",(void *)redirect_ip_buf, &mask, &gateway, &dns1, &dns2);
		}
		else if(wan_type == PPPOE || wan_type == PPTP || wan_type == L2TP)
		{
			get_WanInfo("/etc/wan_info","ppp0",(void *)redirect_ip_buf, &mask, &gateway, &dns1, &dns2);
		}
	}		
	sprintf(redirect_ip,"%s",inet_ntoa(*((struct in_addr *)redirect_ip_buf)) );
}
void reboot_wait_redirect(char * tmpBuf)
{
	char redirect_ip[36]={0};

	Reboot_Wait = 25;
#ifdef CONFIG_RTL_8367R
	Reboot_Wait+=5;
#endif
	get_redirect_ip(redirect_ip);
#ifdef BRIDGE_REPEATER		
#ifdef CONFIG_RTL_DNS_TRAP
	{
		int op_mode;
		apmib_get(MIB_OP_MODE,(void *)&op_mode);
		if(op_mode==BRIDGE_MODE)
		{
			char dnsBuf[32]={0};
			apmib_get(MIB_DOMAIN_NAME,(void*)dnsBuf);
			sprintf(redirect_ip,"%s.net",dnsBuf);
			Reboot_Wait+=25;
		}
	}
#endif
#endif
	OK_MSG_FW(tmpBuf, Reboot_Wait,redirect_ip);	
	//printf("lanip=%s,wait=%d\n",redirect_ip,Reboot_Wait);
	extern int update_system_time_flash(void);
	update_system_time_flash();
	apmib_update_web(CURRENT_SETTING);
	sleep(1);
	RunSystemCmd(NULL_FILE,"reboot",NULL_STR);
	return;
}
void wait_redirect(char* msg,int wait_seconds, char*submitUrl)
{
	char redirect_ip[30]={0};
	get_redirect_ip(redirect_ip);
	//diag_printf("#####redirect url=%s:%s\n",redirect_ip,submitUrl);
	OK_MSG_WAIT(msg,wait_seconds,redirect_ip,submitUrl);
}

int access_from_lan_or_wan()// 1: from lan;0:from wan
{
	struct in_addr intaddr;
	unsigned char lan_ip[4];
	int from_lan=0;
	int web_wan_access_enabled = 0;
#ifdef HOME_GATEWAY
	apmib_get(MIB_WEB_WAN_ACCESS_ENABLED,(void *)&web_wan_access_enabled);
	if(web_wan_access_enabled)
	{
		getInAddr(BRIDGE_IF, IP_ADDR, (void *)&intaddr );
		memcpy(lan_ip,&intaddr,4);
		if(memcmp(lan_ip,last_host,4) ==0)
		{
			from_lan=1;
		}
	}
	else
#endif
		from_lan=1;
	return from_lan;
}
void wait_redirect_home(char* msg,int wait_seconds)
{
	char redirect_ip[30]={0};
	get_redirect_ip(redirect_ip);
	//diag_printf("#####redirect url=%s:%s\n",redirect_ip,submitUrl);
#ifdef HOME_GATEWAY
	int from_lan_or_wan = access_from_lan_or_wan();
	if(from_lan_or_wan == 1)//access web server form lan
	{
		OK_MSG_FW(msg,wait_seconds,redirect_ip);
	}
	else                    //access web server form wan
	{
		//add port number after ip address
		unsigned short web_wan_access_port;
		apmib_get(MIB_WEB_WAN_ACCESS_PORT,(void *)&web_wan_access_port);
		int ip_length = strlen(redirect_ip);
		sprintf(redirect_ip+ip_length,":%d",web_wan_access_port);
		
		OK_MSG_FW(msg,wait_seconds,redirect_ip);
	}
#else
	OK_MSG_FW(msg,wait_seconds,redirect_ip);
#endif
}

void formSaveConfig(char *postData, int len)
{
	char *strRequest=NULL;
	char tmpBuf[256]={0};
	
	char *submitUrl = NULL;
	CONFIG_DATA_T type=0;
	//printf("%s:%d len=%d postData=%s\n",__FUNCTION__,__LINE__,len,postData);

	strRequest = get_cstream_var(postData,len,("save"), "");
	if (strRequest[0])
		type |= CURRENT_SETTING;
	if (type) {
		send_redirect_perm("./config.dat");
		return;
		//cyg_httpd_send_file("config.dat");
		//strcpy(tmpBuf, "How to get config.dat?!\n");
		//goto back;
	}
	strRequest=get_cstream_var(postData,len,"reset","");
	if (strRequest[0]) {//reset have value
		extern int run_clicmd(char *command_buf);
		run_clicmd("flash reset");
		sprintf(tmpBuf, "%s","Reloaded settings successfully!<br><br>The Router is booting.<br>Do not turn off or reboot the Device during this time.<br>");
		reboot_wait_redirect(tmpBuf);
		return;
		//ERR_MSG(tmpBuf);
		
	}

back:
	ERR_MSG(tmpBuf);
	return;
}


void formUploadConfig(char *postData, int len)
{
	MULTIPART_T mime;	
	COMPRESS_MIB_HEADER_Tp pCompHeader=NULL;
	char*binData=NULL;
	int i=0, dataLen=0;	
	char tmpBuf[256]={0};
	char lan_ip_buf[30]={0}, lan_ip[30]={0};
	parse_MIME(&mime,postData,len);
	//dump_MIME(&mime);
	for(i=0;i<mime.entry_count;i++)
		if(strncmp(mime.entry[i].name,"binary",mime.entry[i].name_len)==0)
		{
			binData=mime.entry[i].value;
			break;
		}
	if(!binData)
	{
		printf("wrong file!!\n");
		return;
	}
	if(memcmp(binData,COMP_HS_SIGNATURE,COMP_SIGNATURE_LEN)==0)
	{		
		pCompHeader = (COMPRESS_MIB_HEADER_Tp)binData;		
		dataLen=DWORD_SWAP(pCompHeader->compLen)+sizeof(COMPRESS_MIB_HEADER_T);
		_apmib_updateFlash(HW_SETTING,binData,dataLen,0,0);
		binData+=dataLen;
		//diag_printf("%s:%d dataLen=%d\n",__FILE__,__LINE__,dataLen);
	}
	if(memcmp(binData,COMP_DS_SIGNATURE,COMP_SIGNATURE_LEN)==0)
	{
		pCompHeader = (COMPRESS_MIB_HEADER_Tp)binData;
		dataLen=DWORD_SWAP(pCompHeader->compLen)+sizeof(COMPRESS_MIB_HEADER_T);
		_apmib_updateFlash(DEFAULT_SETTING,binData,dataLen,0,0);
		binData+=dataLen;
		//diag_printf("%s:%d dataLen=%d\n",__FILE__,__LINE__,dataLen);
	}
	if(memcmp(binData,COMP_CS_SIGNATURE,COMP_SIGNATURE_LEN)==0)
	{
		pCompHeader = (COMPRESS_MIB_HEADER_Tp)binData;
		dataLen=DWORD_SWAP(pCompHeader->compLen)+sizeof(COMPRESS_MIB_HEADER_T);
		_apmib_updateFlash(CURRENT_SETTING,binData,dataLen,0,0);
		binData+=dataLen;
		//diag_printf("%s:%d dataLen=%d\n",__FILE__,__LINE__,dataLen);
		apmib_reinit();
	}
	//diag_printf("%s:%d \n",__FILE__,__LINE__);

	if(dataLen==0)
	{//bin have no config signature
		sprintf(tmpBuf,"invalid setting file!");
		ERR_MSG(tmpBuf);
		return;
	}
	sprintf(tmpBuf, ("%s"), "Update successfully!<br><br>Update in progressing.<br>Do not turn off or reboot the Device during this time.<br>");
	
	reboot_wait_redirect(tmpBuf);
	
	return;
}

void formUpload(char *postData, int len)
{
	int k;
	int value;
	MULTIPART_T mime;
	int i=0,dataLen=0;
	char updateValid=0;
	//int checkSum=0;
	char tmpBuf[256]={0};
	char* binData=NULL;
	IMG_HEADER_Tp pImgHeader=NULL;

	parse_MIME(&mime,postData,len);
//	dump_MIME(&mime);
	
	
	for(i=0;i<mime.entry_count;i++){		
		if(strncmp(mime.entry[i].name,"binary",mime.entry[i].name_len)==0)
		{
			binData=mime.entry[i].value;
			break;
		}
	}

	for(i=0;i<mime.entry_count;i++){
#if defined(ECOS_MEM_CHAIN_UPGRADE)	
		if(formupload_upgrade_flag == 1){
			if(mime.entry[i].name){
				free(mime.entry[i].name);
				mime.entry[i].name = NULL;
			}
		}		
#endif
	}
	

	if(!binData)
	{
		diag_printf("wrong file!!\n");
		return;
	}

#if defined(ECOS_MEM_CHAIN_UPGRADE)
	if(formupload_upgrade_flag == 1)
		value = mem_chain_upgrade_memcmp(binData,WEB_HEADER,SIGNATURE_LEN);
	else
		value = memcmp(binData,WEB_HEADER,SIGNATURE_LEN);	
#else
	value = memcmp(binData,WEB_HEADER,SIGNATURE_LEN);
#endif

	if(value == 0)
	{
#if defined(ECOS_MEM_CHAIN_UPGRADE)
		if(formupload_upgrade_flag == 1){
			pImgHeader =(IMG_HEADER_Tp)mem_chain_upgrade_mem_str_convert(binData,sizeof(IMG_HEADER_T));	
			if(	pImgHeader == NULL){
				sprintf(tmpBuf, "<b>do not have enough space to malloc in heap</b><br>");
				goto UPLOAD_FAIL;
			}
		}
		else
			pImgHeader=(IMG_HEADER_Tp)binData;
#else
		pImgHeader=(IMG_HEADER_Tp)binData;
#endif

#ifndef JFFS2_SUPPORT
		dataLen = DWORD_SWAP(pImgHeader->len)+sizeof(IMG_HEADER_T);		
#else		
		if(dataLen%2)
			dataLen = DWORD_SWAP(pImgHeader->len)-1;//checksum
		else
			dataLen = DWORD_SWAP(pImgHeader->len)-2;//checksum and pad	

#endif
		if(dataLen>len || dataLen>CODE_IMAGE_OFFSET-WEB_PAGE_OFFSET)
		{			
			sprintf(tmpBuf, "<b>image invalid!oversize len=%d avaliable %d</b><br>",dataLen,CODE_IMAGE_OFFSET-WEB_PAGE_OFFSET);
			goto UPLOAD_FAIL;
		}
//write to flash

#if defined(ECOS_MEM_CHAIN_UPGRADE)
		if(formupload_upgrade_flag == 1){
			if(!mem_chain_upgrade_CHECKSUM_OK(binData+sizeof(IMG_HEADER_T),DWORD_SWAP(pImgHeader->len)))
			{
				sprintf(tmpBuf, "<b>Image web checksum mismatched! len=0x%x</b><br>", DWORD_SWAP(pImgHeader->len));
				goto UPLOAD_FAIL;
			}
		}else{
			if(!CHECKSUM_OK(binData+sizeof(IMG_HEADER_T),DWORD_SWAP(pImgHeader->len)))
			{
				sprintf(tmpBuf, "<b>Image web checksum mismatched! len=0x%x</b><br>", DWORD_SWAP(pImgHeader->len));
				goto UPLOAD_FAIL;
			}
		}
#else
		if(!CHECKSUM_OK(binData+sizeof(IMG_HEADER_T),DWORD_SWAP(pImgHeader->len)))
		{
			sprintf(tmpBuf, "<b>Image web checksum mismatched! len=0x%x</b><br>", DWORD_SWAP(pImgHeader->len));
			goto UPLOAD_FAIL;
		}
#endif
		
		diag_printf("burning web.bin!\n");
#ifdef JFFS2_SUPPORT
		binData+=sizeof(IMG_HEADER_T);
#endif
		//printf("%s:%d signature=%s startAddr=%p burnAddr=%p len=0x%x \n",__FILE__,__LINE__,
		//pImgHeader->signature,pImgHeader->startAddr,pImgHeader->burnAddr,dataLen);

#if defined(ECOS_MEM_CHAIN_UPGRADE)
		if(formupload_upgrade_flag == 1){
			if(mem_chain_support_rtk_flash_write(binData,DWORD_SWAP(pImgHeader->burnAddr),dataLen)==0)
			{
				sprintf(tmpBuf, "<b>Image fw write to flash fail!</b><br>");
				goto UPLOAD_FAIL;
			}
		}else{
			if(rtk_flash_write(binData,DWORD_SWAP(pImgHeader->burnAddr),dataLen)==0)
			{
				sprintf(tmpBuf, "<b>Image fw write to flash fail!</b><br>");
				goto UPLOAD_FAIL;
			}
		}	

#else
		if(rtk_flash_write(binData,DWORD_SWAP(pImgHeader->burnAddr),dataLen)==0)
		{
			sprintf(tmpBuf, "<b>Image fw write to flash fail!</b><br>");
			goto UPLOAD_FAIL;
		}
#endif

		binData+=dataLen;
		updateValid=1;
#if defined(ECOS_MEM_CHAIN_UPGRADE)
		if(formupload_upgrade_flag == 1){
			if(pImgHeader){
				free(pImgHeader);
				pImgHeader = NULL;
			}
		}
		
#endif

#ifdef CONFIG_IRES_WEB_ADVANCED_SUPPORT
	free_web_ptr();
	clear_web_size();
#ifdef CONFIG_RESERVE_WEB_CACHE
	free_buf_ptr();
#endif
#endif
	}	

#if defined(ECOS_MEM_CHAIN_UPGRADE)
	if(formupload_upgrade_flag == 1)
		value = mem_chain_upgrade_memcmp(binData,FW_HEADER,SIGNATURE_LEN);
	else
		value = memcmp(binData,FW_HEADER,SIGNATURE_LEN);
#else
	value = memcmp(binData,FW_HEADER,SIGNATURE_LEN);
#endif

	if(value == 0)
	{
#if defined(ECOS_MEM_CHAIN_UPGRADE)
		if(formupload_upgrade_flag == 1){
			pImgHeader =(IMG_HEADER_Tp)mem_chain_upgrade_mem_str_convert(binData,sizeof(IMG_HEADER_T));
			if(	pImgHeader == NULL){
				sprintf(tmpBuf, "<b>do not have enough space to malloc in heap</b><br>");
				goto UPLOAD_FAIL;
			}
		}
		else
			pImgHeader=(IMG_HEADER_Tp)binData;
#else
		pImgHeader=(IMG_HEADER_Tp)binData;
#endif
		dataLen = DWORD_SWAP(pImgHeader->len)+sizeof(IMG_HEADER_T);

		if(dataLen+binData-postData>len)
		{			
			sprintf(tmpBuf, "<b>image invalid!</b><br>");
			goto UPLOAD_FAIL;
		}
		
//write to flash
#if defined(ECOS_MEM_CHAIN_UPGRADE)
		if(formupload_upgrade_flag == 1){
			if(!mem_chain_upgrade_fwChecksumOk(binData+sizeof(IMG_HEADER_T),DWORD_SWAP(pImgHeader->len)))
			{
				sprintf(tmpBuf, "<b>Image fw checksum mismatched! len=0x%x</b><br>", DWORD_SWAP(pImgHeader->len));
				goto UPLOAD_FAIL;
			}
		}else{
			if(!fwChecksumOk(binData+sizeof(IMG_HEADER_T),DWORD_SWAP(pImgHeader->len)))
			{
				sprintf(tmpBuf, "<b>Image fw checksum mismatched! len=0x%x</b><br>", DWORD_SWAP(pImgHeader->len));
				goto UPLOAD_FAIL;
			}
		}
		
#else
		if(!fwChecksumOk(binData+sizeof(IMG_HEADER_T),DWORD_SWAP(pImgHeader->len)))
		{
			sprintf(tmpBuf, "<b>Image fw checksum mismatched! len=0x%x</b><br>", DWORD_SWAP(pImgHeader->len));
			goto UPLOAD_FAIL;
		}
#endif

		diag_printf("burning ecos.bin!\n");
		//printf("%s:%d signature=%s startAddr=%p burnAddr=%p len=%d checkSum=%d\n",__FILE__,__LINE__,
		//pImgHeader->signature,pImgHeader->startAddr,pImgHeader->burnAddr,pImgHeader->len,checkSum);

#if defined(ECOS_MEM_CHAIN_UPGRADE)
		if(formupload_upgrade_flag == 1){
			if(mem_chain_support_rtk_flash_write(binData,DWORD_SWAP(pImgHeader->burnAddr),dataLen)==0)
			{
				sprintf(tmpBuf, "<b>Image fw write to flash fail!</b><br>");
				goto UPLOAD_FAIL;
			}
		}else{
			if(rtk_flash_write(binData,DWORD_SWAP(pImgHeader->burnAddr),dataLen)==0)
			{
				sprintf(tmpBuf, "<b>Image fw write to flash fail!</b><br>");
				goto UPLOAD_FAIL;
			}
		}	

#else
		if(rtk_flash_write(binData,DWORD_SWAP(pImgHeader->burnAddr),dataLen)==0)
		{
			sprintf(tmpBuf, "<b>Image fw write to flash fail!</b><br>");
			goto UPLOAD_FAIL;
		}
#endif
		binData+=dataLen;
		updateValid=1;

#if defined(ECOS_MEM_CHAIN_UPGRADE)
		if(formupload_upgrade_flag == 1){
			if(pImgHeader){
				free(pImgHeader);
				pImgHeader = NULL;
			}
		}

#endif
	}
	if(!updateValid)
	{
		sprintf(tmpBuf, "<b>image invalid!</b><br>");
		goto UPLOAD_FAIL;
	}
	sprintf(tmpBuf, ("%s"), "Update successfully!<br><br>Update in progressing.<br>Do not turn off or reboot the Device during this time.<br>");

	reboot_wait_redirect(tmpBuf);
	return;
UPLOAD_FAIL:	
	ERR_MSG(tmpBuf);
}

int  opModeHandler(char *postData,  int len)
{
	char *tmpStr=NULL;
	int opmode=0, wanId=0;
	char tmpBuf[64]={0};

	tmpStr = get_cstream_var(postData,len, ("opMode"), "");   
	if(NULL == tmpStr) 	return -1;
	if(tmpStr[0]){
		opmode = tmpStr[0] - '0' ;
		if ( apmib_set(MIB_OP_MODE, (void *)&opmode) == 0) {
			strcpy(tmpBuf, ("Set Opmode error!"));
			goto setErr_opmode;
		}
	}

#if defined(CONFIG_SMART_REPEATER)
	if(opmode==WISP_MODE)
	{//wisp mode
#endif
		int wlan_num;
		wlan_num = getWlanNum();
		
 		tmpStr = get_cstream_var(postData,len, ("wispWanId"), "");  
		if(NULL == tmpStr) return (-1);
		if(tmpStr[0] && (wlan_num > 1)){
			wanId = tmpStr[0] - '0' ;
			
 			if(wanId > wlan_num)
				wanId =0;
 			if ( apmib_set(MIB_WISP_WAN_ID, (void *)&wanId) == 0) {
				strcpy(tmpBuf, ("Set WISP WAN Id error!"));
				goto setErr_opmode;
			}
#if defined(CONFIG_SMART_REPEATER)
			int rpt_enabled = 1;
			char wlanifStr[20];
			int wlanMode;

#ifdef CONFIG_WLANIDX_MUTEX
			int s = apmib_save_idx();
#else
			apmib_save_idx();
#endif
			if(wanId == 0)
			{
				apmib_set( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);

				rpt_enabled=0;
				apmib_set(MIB_REPEATER_ENABLED2,(void *)&rpt_enabled);
			}
			else
			{
				apmib_set( MIB_REPEATER_ENABLED2, (void *)&rpt_enabled);

				rpt_enabled=0;
				apmib_set(MIB_REPEATER_ENABLED1,(void *)&rpt_enabled);
			}

			sprintf(wlanifStr, "wlan%d", wanId); 
 			SetWlan_idx(wlanifStr);
			wlanMode = AP_MODE;
			apmib_set( MIB_WLAN_MODE, (void *)&wlanMode);

			sprintf(wlanifStr, "wlan%d-vxd0", wanId); 
			SetWlan_idx(wlanifStr);
 			wlanMode = CLIENT_MODE;
			apmib_set( MIB_WLAN_MODE, (void *)&wlanMode);
			rpt_enabled = 0;
			apmib_set( MIB_WLAN_WLAN_DISABLED, (void *)&rpt_enabled);
#ifdef CONFIG_WLANIDX_MUTEX
			apmib_revert_idx(s);
#else
			apmib_revert_idx();
#endif

#endif
		}
#if defined(CONFIG_SMART_REPEATER)
		else{//only one wlan:92c/88
			int rpt_enabled = 1;
			char wlanifStr[20]={0};
			int wlanMode;

			wanId=0;
#ifdef CONFIG_WLANIDX_MUTEX
			int s = apmib_save_idx();
#else
			apmib_save_idx();
#endif			
			sprintf(wlanifStr, "wlan%d", wanId); 
			SetWlan_idx(wlanifStr);
			apmib_set( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
 			wlanMode = AP_MODE;
			apmib_set( MIB_WLAN_MODE, (void *)&wlanMode);

			sprintf(wlanifStr, "wlan%d-vxd0", wanId); 
			SetWlan_idx(wlanifStr);
 			wlanMode = CLIENT_MODE;
			apmib_set( MIB_WLAN_MODE, (void *)&wlanMode);
			rpt_enabled = 0;
			apmib_set( MIB_WLAN_WLAN_DISABLED, (void *)&rpt_enabled);
#ifdef CONFIG_WLANIDX_MUTEX
			apmib_revert_idx(s);
#else
			apmib_revert_idx();
#endif
		}
	}
	else //opmode is gw or bridge
	{	

		int rpt_enabled=0;
	#ifdef BRIDGE_REPEATER
		if(opmode==BRIDGE_MODE)
			rpt_enabled=1;
	#endif
		apmib_set(MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
		apmib_set(MIB_REPEATER_ENABLED2,(void *)&rpt_enabled);
	}
#endif

	
	return 0;

setErr_opmode:
	return -1;

}

void formOpMode(char *postData, int len)
{
//	char *submitUrl;
	char tmpBuf[100];
//	submitUrl = get_cstream_var(postData,len, "submit-url", "");   // hidden page

	if(opModeHandler(postData, len) < 0)
			goto setErr;
	
	apmib_update_web(CURRENT_SETTING);
	sprintf(tmpBuf, ("%s"), "Set Operating Mode successfully!");
#ifdef HAVE_SYSTEM_REINIT
	wait_redirect_home(tmpBuf,10);
	sleep(1);
	kick_reinit_m(SYS_WAN_M | SYS_BRIDGE_M | SYS_WIFI_M);
#else
	reboot_wait_redirect(tmpBuf);
#endif
	save_cs_to_file();
return;

setErr:
	ERR_MSG(tmpBuf);
}
void formPasswordSetup(char *postData, int len)
{
	char *submitUrl, *strUser, *strPassword;
	char tmpBuf[100];

	strUser = get_cstream_var(postData,len, "username", "");
	strPassword = get_cstream_var(postData,len, "newpass", "");
	if ( strUser[0] && !strPassword[0] ) {
		strcpy(tmpBuf, ("ERROR: Password cannot be empty."));
		goto setErr_pass;
	}

	if ( strUser[0] ) {
		/* Check if user name is the same as supervisor name */
		if ( !apmib_get(MIB_SUPER_NAME, (void *)tmpBuf)) {
			strcpy(tmpBuf, ("ERROR: Get supervisor name MIB error!"));
			goto setErr_pass;
		}
		if ( !strcmp(strUser, tmpBuf)) {
			strcpy(tmpBuf, ("ERROR: Cannot use the same user name as supervisor."));
			goto setErr_pass;
		}
	}
	else {
		/* Set NULL account */
	}

	/* Set user account to MIB */
	if ( !apmib_set(MIB_USER_NAME, (void *)strUser) ) {
		strcpy(tmpBuf, ("ERROR: Set user name to MIB database failed."));
		goto setErr_pass;
	}

	if ( !apmib_set(MIB_USER_PASSWORD, (void *)strPassword) ) {
		strcpy(tmpBuf, ("ERROR: Set user password to MIB database failed."));
		goto setErr_pass;
	}

	/* Retrieve next page URL */
	apmib_update_web(CURRENT_SETTING);
		
	submitUrl = get_cstream_var(postData,len, "submit-url", "");   // hidden page

#ifdef LOGIN_URL
	if (strUser[0])
		submitUrl = "/login.htm";
#endif

#ifdef REBOOT_CHECK
	{
		char tmpMsg[300];
		char lan_ip_buf[30], lan_ip[30];
		
		sprintf(tmpMsg, "%s","okmsg_fw_passwd");
		apmib_get( MIB_IP_ADDR,  (void *)lan_ip_buf) ;
		sprintf(lan_ip,"%s",inet_ntoa(*((struct in_addr *)lan_ip_buf)) );
		OK_MSG_FW(tmpMsg, submitUrl,APPLY_COUNTDOWN_TIME,lan_ip);
#ifdef REBOOT_CHECK
		run_init_script_flag = 1;
#endif		
#ifndef NO_ACTION
		run_init_script("all");
#endif	
	}
#else
	setAuthUP(WEB_ROOT,strUser,strPassword);
	apmib_update_web(CURRENT_SETTING);
	reboot_wait_redirect("");
#endif
	return;

setErr_pass:
	ERR_MSG(tmpBuf);
}


void formLogout(char *postData, int len)
{
	char *logout_str, *return_url;
	logout_str = get_cstream_var(postData,len, ("logout"), "");
	if (logout_str[0]) {
#ifdef LOGIN_URL
		delete_user(wp);
	    OK_MSG("/login.htm");
	    return;
#endif		
	}

	return_url = get_cstream_var(postData,len, ("return-url"), "");

#ifdef REBOOT_CHECK
	send_redirect_perm(return_url);	
#else
       OK_MSG(return_url);
#endif

	return;
}
#define _PATH_SYSCMD_LOG "/tmp/syscmd.log"
#define LOG_MAX_LINE_SIZE 128
void formSysCmd(char *postData, int len)
{
	char  *submitUrl, *sysCmd;
	char* ptmpBuf;
	extern int run_clicmd(char *command_buf);

	ptmpBuf=(char*)malloc(64);
	if(!ptmpBuf)
	{
		printf("malloc fail!!\n");
		return;
	}
	submitUrl = get_cstream_var(postData, len, "submit-url", "");   // hidden page
	sysCmd = get_cstream_var(postData, len, "sysCmd", "");   // hidden page
	if(sysCmd[0])
	{	
		FILE *fileTmp=stdout;
		
		if((stdout=fopen(_PATH_SYSCMD_LOG,"w"))==NULL)
		{
			fprintf(stderr,"redirect output fail!\n");
			stdout=fileTmp;
			goto SYSCMD_END;
		}		
		run_clicmd(sysCmd);
		fclose(stdout);
		stdout=fileTmp;
	}
		send_redirect_perm(submitUrl);
SYSCMD_END:
	if(ptmpBuf)
	{
		free(ptmpBuf);
		ptmpBuf=NULL;
	}
	return;
}

#if defined(WLAN_PROFILE)
void formSiteSurveyProfile(char *postData, int len)
{
	char *submitUrl, *strTmp, *addProfileTmp;
	char tmpBuf[100];
	char varName[20];
	int wlan_idx=apmib_get_wlanidx();	
	int vwlan_idx=apmib_get_vwlanidx();
//displayPostDate(wp->post_data);	


	sprintf(varName, "wizardAddProfile%d", wlan_idx);
	addProfileTmp = get_cstream_var(postData, len, varName, "");
	
	if(addProfileTmp[0])	
	{
		int rptEnabled, wlan_mode;
		int ori_vwlan_idx=vwlan_idx;
		int profile_enabled_id, profile_num_id, profile_tbl_id;
		int profileEnabledVal=1;
		char iwprivCmd[128]={0};
		int entryNum;
		WLAN_PROFILE_T entry;
		int profileIdx;
		char ifname[10]={0}; //max is wlan0-vxd
		
		memset(iwprivCmd, 0x00, sizeof(iwprivCmd));
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);

		if(wlan_idx == 0)
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnabled);

		
		if( (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE) && (rptEnabled == 1))
		{
			sprintf(ifname,"wlan%d-vxd0",wlan_idx);
			vwlan_idx = NUM_VWLAN_INTERFACE;
		}
		else
		{
			sprintf(ifname,"wlan%d",wlan_idx);
			vwlan_idx = 0;
		}
		
		apmib_set_vwlanidx(vwlan_idx);
		
		if(wlan_idx == 0)
		{		
			profile_num_id = MIB_PROFILE_NUM1;
			profile_tbl_id = MIB_PROFILE_TBL1;
			profile_enabled_id = MIB_PROFILE_ENABLED1;
		}
		else
		{
			profile_num_id = MIB_PROFILE_NUM2;
			profile_tbl_id = MIB_PROFILE_TBL2;
			profile_enabled_id = MIB_PROFILE_ENABLED2;
		}

		apmib_set(profile_enabled_id, (void *)&profileEnabledVal);
		


		if(addWlProfileHandler(postData, len, tmpBuf, wlan_idx) < 0){
	printf("\r\n Add wireless profile fail__[%s-%u]\r\n",__FILE__,__LINE__);
			//strcpy(tmpBuf, ("Add wireless profile fail!"));
			//goto ss_err;
		}
		sprintf(iwprivCmd,"ap_profile_enable=%d",profileEnabledVal);		
		RunSystemCmd(NULL_FILE,"iwpriv",ifname,"set_mib",iwprivCmd,NULL_STR);
		//system(iwprivCmd);
		
		//sprintf(iwprivCmd,"iwpriv %s set_mib ap_profile_num=0",ifname);
		//system(iwprivCmd);
		//sprintf(iwprivCmd,"ap_profile_num=0",ifname);
		RunSystemCmd(NULL_FILE,"iwpriv",ifname,"set_mib","ap_profile_num=0",NULL_STR);
		
		apmib_get(profile_num_id, (void *)&entryNum);

		for(profileIdx=1; profileIdx<=entryNum;profileIdx++)
		{
			memset(iwprivCmd, 0x00, sizeof(iwprivCmd));
			memset(&entry, 0x00, sizeof(WLAN_PROFILE_T));
			*((char *)&entry) = (char)profileIdx;
			apmib_get(profile_tbl_id, (void *)&entry);
		



		
		
		
		
			//iwpriv wlan0 set_mib ap_profile_add="open-ssid",0,0
			if(entry.encryption == ENCRYPT_DISABLED)
			{
				sprintf(iwprivCmd,"ap_profile_add=\"%s\",%d,%d",entry.ssid,0,0);
				RunSystemCmd(NULL_FILE,"iwpriv",ifname,"set_mib",iwprivCmd,NULL_STR);
			}
			else if(entry.encryption == WEP64 || entry.encryption == WEP128)
			{
				char tmp1[400];
				if (entry.encryption == WEP64)			
					sprintf(tmp1,"%d,%d,%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x", 
						entry.auth,
						entry.wep_default_key,
						entry.wepKey1[0],entry.wepKey1[1],entry.wepKey1[2],entry.wepKey1[3],entry.wepKey1[4],
						entry.wepKey2[0],entry.wepKey2[1],entry.wepKey2[2],entry.wepKey2[3],entry.wepKey2[4],
						entry.wepKey3[0],entry.wepKey3[1],entry.wepKey3[2],entry.wepKey3[3],entry.wepKey3[4],
						entry.wepKey4[0],entry.wepKey4[1],entry.wepKey4[2],entry.wepKey4[3],entry.wepKey4[4]);
				else
					sprintf(tmp1,"%d,%d,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
						entry.auth,
						entry.wep_default_key,
						entry.wepKey1[0],entry.wepKey1[1],entry.wepKey1[2],entry.wepKey1[3],entry.wepKey1[4],entry.wepKey1[5],entry.wepKey1[6],entry.wepKey1[7],entry.wepKey1[8],
						entry.wepKey1[9],entry.wepKey1[10],entry.wepKey1[11],entry.wepKey1[12],
						entry.wepKey2[0],entry.wepKey2[1],entry.wepKey2[2],entry.wepKey2[3],entry.wepKey2[4],entry.wepKey2[5],entry.wepKey2[6],entry.wepKey2[7],entry.wepKey2[8],
						entry.wepKey2[9],entry.wepKey2[10],entry.wepKey2[11],entry.wepKey2[12],
						entry.wepKey3[0],entry.wepKey3[1],entry.wepKey3[2],entry.wepKey3[3],entry.wepKey3[4],entry.wepKey3[5],entry.wepKey3[6],entry.wepKey3[7],entry.wepKey3[8],
						entry.wepKey3[9],entry.wepKey3[10],entry.wepKey3[11],entry.wepKey3[12],
						entry.wepKey4[0],entry.wepKey4[1],entry.wepKey4[2],entry.wepKey4[3],entry.wepKey4[4],entry.wepKey4[5],entry.wepKey4[6],entry.wepKey4[7],entry.wepKey4[8],
						entry.wepKey4[9],entry.wepKey4[10],entry.wepKey4[11],entry.wepKey4[12]);	
				
				sprintf(iwprivCmd,"ap_profile_add=\"%s\",%d,%s,",entry.ssid,entry.encryption, tmp1);
			}
			else if(entry.encryption == 3 || entry.encryption == 4) //wpa or wpa2
			{
				char tmp1[400];
				sprintf(tmp1, "%d,%s", entry.wpa_cipher, entry.wpaPSK);
				sprintf(iwprivCmd,"ap_profile_add=\"%s\",%d,0,%s",entry.ssid,entry.encryption,tmp1 );
			}	
			RunSystemCmd(NULL_FILE,"iwpriv",ifname,"set_mib",iwprivCmd,NULL_STR);
			//system(iwprivCmd);
		}

		vwlan_idx = ori_vwlan_idx;
		apmib_set_vwlanidx(vwlan_idx);
		apmib_update_web(CURRENT_SETTING);
		
		
	}

	submitUrl = get_cstream_var(postData, len, "submit-url", "");   // hidden page
	
	strTmp = get_cstream_var(postData, len, "restartNow", ""); 
	if(strTmp[0])
	{		
		apmib_update_web(CURRENT_SETTING);
#ifdef HAVE_SYSTEM_REINIT
		sprintf(tmpBuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,wlan_idx);
		wait_redirect("Apply Changes",10,tmpBuf);
		sleep(1);
		kick_reinit_m(SYS_WIFI_M | SYS_BRIDGE_M | SYS_WIFI_APP_M);
#else
		reboot_wait_redirect("");
#endif
	}
	else
	{
		send_redirect_perm(submitUrl);
	}
	
}
#endif //#if defined(WLAN_PROFILE)

int sysCmdLog(int argc, char **argv)
{
	FILE*fp=NULL;
	int nBytesSent=0;
	char * buf=(char*)malloc(LOG_MAX_LINE_SIZE);
	if(!buf)
	{
		printf("malloc fial!");
		return nBytesSent;
	}
	if((fp=fopen(_PATH_SYSCMD_LOG,"r"))==NULL)
	{
		goto SYSCMDLOG_END;
	}
	while(fgets(buf,LOG_MAX_LINE_SIZE,fp))
	{
		nBytesSent=strlen(buf);
		cyg_httpd_write_chunked(buf,nBytesSent);
    }
    fclose(fp);
SYSCMDLOG_END:
	if(buf)
	{
		free(buf);
		buf=NULL;
	}
	return nBytesSent;
}
void formLangSel(char *postData, int len)
{
	char tmpBuf[MSG_BUFFER_SIZE]={0};
	char *submitUrl=NULL;
	if(handle_fmGetValAndSetIntVal("languageSel",MIB_WEB_LANGUAGE,NULL,postData,len, tmpBuf)<0)
	{
		return -1;
	}
	apmib_update_web(CURRENT_SETTING);
	submitUrl = get_cstream_var(postData, len, "submit-url", "");   // hidden page
	send_redirect_perm(submitUrl);
	return 0;
}



/////////////////////////////////////////////////////////////////////////////
void formStats(char *postData, int len)
{
	char *submitUrl;

	submitUrl = get_cstream_var(postData, len,"submit-url", "");   // hidden page

	if (submitUrl[0])
		send_redirect_perm(submitUrl);
}

#ifdef CONFIG_RTK_MESH
void formMeshStatus(char *postData, int len)
{
	char *submitUrl;

	submitUrl = get_cstream_var(postData, len, "submit-url", "");   // hidden page

	if (submitUrl[0])
		send_redirect_perm(submitUrl);
}
#endif // CONFIG_RTK_MESH

///////////////////////////////////////////////////////////////////////////
int static wizard_fmGetValAndSetDHCP_T(char * name, int mibid, DHCP_T* pDhcp, char * postData, int len, char * errMsgBuf)
{
	char *strVal = NULL; 
	DHCP_T wan_mode;
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
	{
		if(strcmp(strVal,"fixedIp")==0) 	wan_mode=STATIC_IP;
		else if(strcmp(strVal,"autoIp")==0)	wan_mode=DHCP_CLIENT;
		else if(strcmp(strVal,"ppp")==0)	wan_mode=PPPOE;
		else if(strcmp(strVal,"pptp")==0) wan_mode=PPTP;		
		else if(strcmp(strVal,"l2tp")==0) wan_mode=L2TP;
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
        else if(strcmp(strVal,"pppoehenan")==0)	wan_mode=PPPOE_HENAN;
		else if(strcmp(strVal,"pppoenanchang")==0)wan_mode=PPPOE_NANCHANG;
		else if(strcmp(strVal,"pppoeother1")==0)	wan_mode=PPPOE_OTHER1;
		else if(strcmp(strVal,"pppoeother2")==0)	wan_mode=PPPOE_OTHER2;
		else if(strcmp(strVal,"dhcpplus")==0)wan_mode=DHCP_PLUS;
        #endif
		else
		{
			sprintf(errMsgBuf,"%s","Wrong WAN Access Type!!");	
			return -1;
		}	
		if ( !apmib_set(mibid, (void *)&wan_mode)) 
		{
			sprintf(errMsgBuf, "Set %s MIB error!",name);
			return -1;
		}
		if(pDhcp)	*pDhcp=wan_mode;
	}
	return 0;
}
int static handle_fmGetValAndSetBandVal(char * name, int mibid, int *intVal, char * postData, int len, char * errMsgBuf)
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
	 strVal= get_cstream_var(postData,len,name,"");
	 if ( strVal[0] ) 
	 {//get value
		 int tmpVal=strtol(strVal, (char**)NULL, 10);
		 tmpVal++;
		 if ( !apmib_set(mibid, (void *)&tmpVal)) 
		 {
			 sprintf(errMsgBuf, "Set %s MIB error!",name);
			 return -1;
		 }
		 if(intVal) *intVal=tmpVal;
	 }	 
	 return 0;
 }

int wizardHandleLan(char *postData, int len,char*msg)
{
	struct in_addr orig_addr,orig_mask,new_addr,new_mask;
	char tmpBuff[32];

	/*get orig address*/
	apmib_get( MIB_IP_ADDR,(void *)tmpBuff);
	orig_addr = (*((struct in_addr *)tmpBuff));
	/*get orig mask*/	
	apmib_get( MIB_SUBNET_MASK,(void *)tmpBuff);
	orig_mask = (*((struct in_addr *)tmpBuff));
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
	//do nothing for CMJ don't have lan ip setting UI in wizard.
#else
	if(handle_fmGetValAndSetIp("lan_ip",MIB_IP_ADDR,NULL,postData,len,msg)<0) return -1;

	if(handle_fmGetValAndSetIp("lan_mask",MIB_SUBNET_MASK,NULL,postData,len,msg)<0) return -1;
	{
		//set the ip range 
		//192.168.10.100 ~192.168.10.200
		//lan_ip value is 192.168.xx.254
		#define IP_RANGE_START 100
		#define IP_RANGE_END 200		
		char* strIp=get_cstream_var(postData,len,"lan_ip","");
		char* strMask=get_cstream_var(postData,len,"lan_mask","");
		struct in_addr ip_addr={0};
		struct in_addr ip_mask={0};
		struct in_addr ip_range_start={0};
		struct in_addr ip_range_end={0};		
		if(strIp[0] && strMask[0])
		{
			 if ( !inet_aton(strIp, &ip_addr) ) {
				 sprintf(msg, ("Invalid IP address value! %s"),strIp);
				 return -1;
			 }
			 
			 if ( !inet_aton(strMask, &ip_mask) ) {
				 sprintf(msg, ("Invalid IP address value! %s"),strIp);
				 return -1;
			 }			 
			 
			 ip_range_start.s_addr = (ip_addr.s_addr & ip_mask.s_addr) | htonl(IP_RANGE_START);
			 ip_range_end.s_addr = (ip_addr.s_addr & ip_mask.s_addr) | htonl(IP_RANGE_END);
			 
			 if ( !apmib_set(MIB_DHCP_CLIENT_START, (void *)&ip_range_start)) {
				 sprintf(msg, "Set IP MIB lan_ip start error!");
					 return -1;
		 	 }
			 
			 if ( !apmib_set(MIB_DHCP_CLIENT_END, (void *)&ip_range_end)) {
				 sprintf(msg, "Set IP MIB lan_ip end error!");
					 return -1;
		 	 }
			 
#ifdef HOME_GATEWAY
//if in different subnet ,we should clear the firewall info
			 if((orig_addr.s_addr & orig_mask.s_addr) != (ip_addr.s_addr & ip_mask.s_addr))
				 if (clear_firewall_info(orig_addr,orig_mask,ip_addr,ip_mask, msg)<0)
                    return -1;
#endif
		}
	}
#endif
	return 0;
}
#ifdef HOME_GATEWAY
int wizardHandleWan(char *postData, int len,char*msg)
{
	DHCP_T wan_mode;
	DNS_TYPE_T dns_mode;

	/*save old wan dhcp for reinit*/
	apmib_get(MIB_WAN_DHCP,&wan_mode);
	apmib_set(MIB_WAN_OLD_DHCP, (void *)&wan_mode);
	
	if(wizard_fmGetValAndSetDHCP_T("wanType",MIB_WAN_DHCP,&wan_mode,postData,len,msg)<0) return -1;
	switch(wan_mode)
	{
		case STATIC_IP:
			if(handle_fmGetValAndSetIp("wan_ip",MIB_WAN_IP_ADDR,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetIp("wan_mask",MIB_WAN_SUBNET_MASK,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetIp("wan_gateway",MIB_WAN_DEFAULT_GATEWAY,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetIp("dns1",MIB_DNS1,NULL,postData,len,msg)<0) return -1;
			dns_mode=DNS_MANUAL;
			if(!apmib_set(MIB_DNS_MODE,(void*)&dns_mode))
				return -1;		
			break;
		case DHCP_CLIENT:			
			dns_mode=DNS_AUTO;
			if(!apmib_set(MIB_DNS_MODE,(void*)&dns_mode))
				return -1;
			break;
		case PPPOE:
			if(handle_fmGetValAndSetStrVal("pppUserName",MIB_PPP_USER_NAME,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetStrVal("pppPassword",MIB_PPP_PASSWORD,NULL,postData,len,msg)<0) return -1;
			dns_mode=DNS_AUTO;
			if(!apmib_set(MIB_DNS_MODE,(void*)&dns_mode))
				return -1;
			break;
		case PPTP:
			if(handle_fmGetValAndSetStrVal("pptpUserName",MIB_PPTP_USER_NAME,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetStrVal("pptpPassword",MIB_PPTP_PASSWORD,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetIp("pptpIpAddr",MIB_PPTP_IP_ADDR,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetIp("pptpSubnetMask",MIB_PPTP_SUBNET_MASK,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetIp("pptpServerIpAddr",MIB_PPTP_SERVER_IP_ADDR,NULL,postData,len,msg)<0) return -1;
			#if defined(CONFIG_DYNAMIC_WAN_IP)
			if(handle_fmGetValAndSetIp("pptpDefGw",MIB_PPTP_DEFAULT_GW,NULL,postData,len,msg)<0) return -1;
			#endif
			#ifdef CONFIG_DYNAMIC_WAN_IP
			{
				int pptpDyn=0;
				char *strPptpDyn=get_cstream_var(postData,len,"wan_pptp_use_dynamic_carrier_radio","");
				if(strPptpDyn[0])
				{
					if(strcmp(strPptpDyn,"dynamicIP")==0)
						pptpDyn=0;
					else if(strcmp(strPptpDyn,"staticIP")==0)
						pptpDyn=1;
					else
					{
						printf("Invalid pptp_use_dynamic_carrier_radio mode value!");
						return -1;
					}
					if(!apmib_set(MIB_PPTP_WAN_IP_DYNAMIC,(void*)&pptpDyn))
					{
						printf("Set MIB_PPTP_WAN_IP_DYNAMIC MIB error!");
						return -1;
					}
				}		
			}
			#endif
			break;
		case L2TP:
			if(handle_fmGetValAndSetStrVal("l2tpUserName",MIB_L2TP_USER_NAME,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetStrVal("l2tpPassword",MIB_L2TP_PASSWORD,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetIp("l2tpIpAddr",MIB_L2TP_IP_ADDR,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetIp("l2tpSubnetMask",MIB_L2TP_SUBNET_MASK,NULL,postData,len,msg)<0) return -1;
			if(handle_fmGetValAndSetIp("l2tpServerIpAddr",MIB_L2TP_SERVER_IP_ADDR,NULL,postData,len,msg)<0) return -1;
			#if defined(CONFIG_DYNAMIC_WAN_IP)
			if(handle_fmGetValAndSetIp("l2tpDefGw",MIB_L2TP_DEFAULT_GW,NULL,postData,len,msg)<0) return -1;
			#endif
			#ifdef CONFIG_DYNAMIC_WAN_IP
			{
				int l2tpDyn=0;
				char *strL2tpDyn=get_cstream_var(postData,len,"wan_l2tp_use_dynamic_carrier_radio","");
				if(strL2tpDyn[0])
				{
					if(strcmp(strL2tpDyn,"dynamicIP")==0)
						l2tpDyn=0;
					else if(strcmp(strL2tpDyn,"staticIP")==0)
						l2tpDyn=1;
					else
					{
						printf("Invalid l2tp_use_dynamic_carrier_radio mode value!");
						return -1;
					}
					if(!apmib_set(MIB_L2TP_WAN_IP_DYNAMIC,(void*)&l2tpDyn))
					{
						printf("Set MIB_L2TP_WAN_IP_DYNAMIC MIB error!");
						return -1;
					}
				}		
			}
			#endif
			
			break;        
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
        case PPPOE_HENAN:
        case PPPOE_NANCHANG:
        case PPPOE_OTHER1:
        case PPPOE_OTHER2:
            //diag_printf("%s %d wan_mode = %d \n", __FUNCTION__, __LINE__, wan_mode);
			if (handle_wizardwan_ppp_netsniper(postData, len, msg, wan_mode) < 0) return -1;
			break;
        case DHCP_PLUS:            
            if (handle_wizardwan_dhcphenan_netsniper(postData, len, msg, wan_mode) < 0) return -1;
			break;
        #endif
		default:
			sprintf(msg,"invalid wan type %d",wan_mode);
			return -1;
	}
	return 0;
}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
int wizardHandle_92d_wlan(char *postData, int len,char*msg)
{
	WLANBAND2G5GMODE_TYPE_T wlBandMode;

	int i=0,intval=0;	
	if(handle_fmGetValAndSetIntVal("wlBandModeSel",MIB_WLAN_BAND2G5G_SELECT,&wlBandMode,postData,len,msg)<0) return -1;

	for(i=0;i<NUM_WLAN_INTERFACE;i++)
	{		
		if(wlBandMode==BANDMODEBOTH)
			if(handle_setWlanIntVal(DMACDPHY,MIB_WLAN_MAC_PHY_MODE,i,msg)<0) return -1;			
		else
			if(handle_setWlanIntVal(SMACSPHY,MIB_WLAN_MAC_PHY_MODE,i,msg)<0) return -1;
	}

	switch(wlBandMode)
	{	
		case BANDMODE2G:
			if(whichWlanIfIs(PHYBAND_2G)!=0)
				swapWlanMibSetting(0,1);
			if(handle_setWlanIntVal(1,MIB_WLAN_WLAN_DISABLED,1,msg)<0)
				return -1;	
			break;
		case BANDMODE5G:	
			if(whichWlanIfIs(PHYBAND_5G)!=0)
				swapWlanMibSetting(0,1);
			if(handle_setWlanIntVal(1,MIB_WLAN_WLAN_DISABLED,1,msg)<0)
				return -1;
		case BANDMODEBOTH:
			/* 92d rule, 5g must up in wlan0 */
			/* phybandcheck */
			if(whichWlanIfIs(PHYBAND_5G)!=0)
				swapWlanMibSetting(0,1);			
			break;
		default:
			break;		
	}	
	return 0;
}
#endif
#ifndef HAVE_NOWIFI
int wizardHandleWlan(char *postData, int len,char*msg)
{
	ENCRYPT_T encrypt;
	WEP_T wep;
	int format=0,keyLen=0,mode=-1,i=0,intval=0, num_wlan_interface = NUM_WLAN_INTERFACE;
	
	char nameBuf[32]={0};

#if defined(CONFIG_RTL_92D_SUPPORT) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	for(i=0;i<NUM_WLAN_INTERFACE;i++)
		if(handle_setWlanIntVal(1,MIB_WLAN_WLAN_DISABLED,i,msg)<0)
			return -1;	
	if(wizardHandle_92d_wlan(postData,len,msg)<0) 
		return -1;
#endif

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)  //&& !defined(CONFIG_CUTE_MAHJONG_RTK_UI)
	num_wlan_interface = 1;
#endif

	for(i=0;i<num_wlan_interface;i++)
	{
		apmib_set_wlanidx(i);
		if(wlanHandler(postData,len, msg, &mode, i)<0) return -1;
		sprintf(nameBuf,"method%d",i);
		if(handle_fmGetValAndSetIntVal(nameBuf,MIB_WLAN_ENCRYPT,&encrypt,postData,len,msg)<0) return -1;
		switch(encrypt)
		{
			case ENCRYPT_DISABLED:
				break;
			case ENCRYPT_WEP:	
				if(wepHandler(postData,len, msg, i) < 0 )
					return -1;			
				break;
			case ENCRYPT_WPA:
			case ENCRYPT_WPA2:
			case ENCRYPT_WPA2_MIXED:
				if(wpaHandler(postData,len, msg, i)<0)
					return -1;
				//if(handle_fmGetValAndSetIntVal("pskFormat0",MIB_WLAN_PSK_FORMAT,NULL,postData,len,msg)<0) return -1;
				//if(handle_fmGetValAndSetStrVal("pskValue0",MIB_WLAN_WPA_PSK,NULL,postData,len,msg)<0) return -1;
				break;
			case ENCRYPT_WAPI:
				break;
			default:
				break;
		}
	}	
	return 0;
}
#endif

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE) || defined (CONFIG_ECOS_AP_SUPPORT)
static int __inline__ wlan_mib_copy(CONFIG_WLAN_SETTING_T* dst, CONFIG_WLAN_SETTING_T* src)
{
	if (apmib_sem_lock() != 0) return -1;
	memcpy(dst, src, sizeof(CONFIG_WLAN_SETTING_T));
	if (apmib_sem_unlock() != 0) return -1;

	return 0;
}

int wizardHandleSiteSurvey(char *postData, int len)
{
	int val, wlan_idx = apmib_get_wlanidx();
	char *strSsid=NULL;
	apmib_get(MIB_OP_MODE, (void *)&val);

	if (get_gpio_2g5g() == 2) {
		if (wlan_idx == 0)
			wlan_idx = 1;
		else if (wlan_idx == 1)
			wlan_idx = 0;
	}

	if(val == WISP_MODE
#ifdef BRIDGE_REPEATER
	|| val== BRIDGE_MODE
#endif
	) {
		wlan_mib_copy(&pMib->wlan[wlan_idx][NUM_VWLAN_INTERFACE], &pMib->wlan[wlan_idx][0]);

		SetWlan_idx("wlan0-vxd0");
		strSsid = get_cstream_var(postData,len, "site_survey_ssid", "");
		if ( strSsid[0] ) {
			apmib_set(MIB_WLAN_SSID, (void *)strSsid);
			apmib_set(MIB_REPEATER_SSID1, (void *)strSsid);
		}
		val = 1;
		apmib_set(MIB_WLAN_MODE, (void *)&val);
		SetWlan_idx("wlan0");
	}
	return 0;
}

int  uiModeHandler(char *postData,  int len)
{
	char *mode;
	int ui_mode;

#if defined(MIB_UI_MODE)
	mode = get_cstream_var(postData, len, ("ui_mode"), "");
	if (mode[0]) {
		if (strstr(mode, "easy"))
			ui_mode = 1;
		else
			ui_mode = 0;
		apmib_set(MIB_UI_MODE,(void *)&ui_mode);
		return 1;
	}
#endif

	mode = get_cstream_var(postData, len, ("opMode"), "");
	if (mode[0]) {
		ui_mode = atoi(mode);
		if (ui_mode != BRIDGE_MODE) {
			ui_mode = DHCP_SERVER;
			apmib_set(MIB_DHCP,(void *)&ui_mode);
			ui_mode = 0;
		}
		apmib_set(MIB_UPGRADE_KIT,(void *)&ui_mode);
	}

	mode = get_cstream_var(postData, len, ("gpio_2g5g"), "");
	if (mode[0]) {
		if (get_gpio_2g5g() != atoi(mode)) {
			cyg_httpd_start_chunked("html"); 
			web_write_chunked("<html>");

			mutil_lang_support();
			
			web_write_chunked("<body><blockquote><h4>%s</h4>\n", "The Wifi Band has been changed, please press OK to reload WEB UI!!");
			web_write_chunked("<form><input type=\"button\" OnClick=window.location.replace(\"%s\") value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body></html>", "/wizard.htm");
			cyg_httpd_end_chunked();

			return 1;
		}
	}

	return 0;
}

int sync_AP_setting(void)
{
	int idx1, idx2;
	if (get_gpio_2g5g() == 5) {
		idx1 = 0;
		idx2 = 1;
	}
	else if (get_gpio_2g5g() == 2) {
		idx1 = 1;
		idx2 = 0;
	}
	else 
		return -1;

#if 0 // get ori WSC info
#ifdef WIFI_SIMPLE_CONFIG
	memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
	wps_config_info.caller_id = CALLED_FROM_WLANHANDLER;
	apmib_get(MIB_WLAN_SSID, (void *)wps_config_info.ssid); 
	apmib_get(MIB_WLAN_MODE, (void *)&wps_config_info.wlan_mode);
#endif

#ifdef WIFI_SIMPLE_CONFIG
#ifdef MBSSID
	if (apmib_get_vwlanidx() == 0)
#endif
	{
		memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
		wps_config_info.caller_id = CALLED_FROM_WPAHANDLER;
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&wps_config_info.auth);
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wps_config_info.wpa_enc);
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wps_config_info.wpa2_enc);
		apmib_get(MIB_WLAN_WPA_PSK, (void *)wps_config_info.wpaPSK);
	}
#endif

#ifdef WIFI_SIMPLE_CONFIG
#ifdef MBSSID
	if (apmib_get_vwlanidx() == 0)
#endif
	{
		memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
		wps_config_info.caller_id = CALLED_FROM_WEPHANDLER;
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&wps_config_info.auth);
		apmib_get(MIB_WLAN_WEP, (void *)&wps_config_info.wep_enc);
		apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&wps_config_info.KeyId);
		apmib_get(MIB_WLAN_WEP64_KEY1, (void *)wps_config_info.wep64Key1);
		apmib_get(MIB_WLAN_WEP64_KEY2, (void *)wps_config_info.wep64Key2);
		apmib_get(MIB_WLAN_WEP64_KEY3, (void *)wps_config_info.wep64Key3);
		apmib_get(MIB_WLAN_WEP64_KEY4, (void *)wps_config_info.wep64Key4);
		apmib_get(MIB_WLAN_WEP128_KEY1, (void *)wps_config_info.wep128Key1);
		apmib_get(MIB_WLAN_WEP128_KEY2, (void *)wps_config_info.wep128Key2);
		apmib_get(MIB_WLAN_WEP128_KEY3, (void *)wps_config_info.wep128Key3);
		apmib_get(MIB_WLAN_WEP128_KEY4, (void *)wps_config_info.wep128Key4);
	}
#endif

#endif

	/* ssid */
	//strcpy(pMib->wlan[idx2][0].ssid, pMib->wlan[idx1][0].ssid);

	/* wlan security */
	pMib->wlan[idx2][0].encrypt = pMib->wlan[idx1][0].encrypt;

	/* WEP */
	pMib->wlan[idx2][0].wep = pMib->wlan[idx1][0].wep;
	pMib->wlan[idx2][0].wepDefaultKey = pMib->wlan[idx1][0].wepDefaultKey;
	strcpy(pMib->wlan[idx2][0].wep64Key1, pMib->wlan[idx1][0].wep64Key1);
	strcpy(pMib->wlan[idx2][0].wep64Key2, pMib->wlan[idx1][0].wep64Key2);
	strcpy(pMib->wlan[idx2][0].wep64Key3, pMib->wlan[idx1][0].wep64Key3);
	strcpy(pMib->wlan[idx2][0].wep64Key4, pMib->wlan[idx1][0].wep64Key4);
	strcpy(pMib->wlan[idx2][0].wep128Key1, pMib->wlan[idx1][0].wep128Key1);
	strcpy(pMib->wlan[idx2][0].wep128Key2, pMib->wlan[idx1][0].wep128Key2);
	strcpy(pMib->wlan[idx2][0].wep128Key3, pMib->wlan[idx1][0].wep128Key3);
	strcpy(pMib->wlan[idx2][0].wep128Key4, pMib->wlan[idx1][0].wep128Key4);

	/* WPA/WPA2 */
	pMib->wlan[idx2][0].wpaAuth = pMib->wlan[idx1][0].wpaAuth;//????
	pMib->wlan[idx2][0].wpaCipher = pMib->wlan[idx1][0].wpaCipher;
	pMib->wlan[idx2][0].wpa2Cipher = pMib->wlan[idx1][0].wpa2Cipher;
	strcpy(pMib->wlan[idx2][0].wpaPSK, pMib->wlan[idx1][0].wpaPSK);

	/*wps configured status*/
	pMib->wlan[idx2][0].wscConfigured = pMib->wlan[idx2][0].wscConfigured;

	return 0;
}

SITE_SURVEY_STATUS siteSurveyStatus =SITE_SURVEY_STATUS_OFF;

//return 0:ok, 1:fail
int sitesurvey_connect(char *postData, int len, char *tmpBuf, int *connect_ok)
{
	char *ptr;
	int i,val, sec, idx;
	unsigned char res=0;

	ptr = get_cstream_var(postData, len, ("select"), "");
	if (ptr[0]) {
		idx=atoi(ptr);
		if (idx==-1) return 0;
	}
	else
		goto CONNECT_ERR0;

	/* set enc */
	ptr = get_cstream_var(postData, len, ("method0"), "");
	if (ptr[0])
		sec = atoi(ptr);
	else
		goto CONNECT_ERR0;
	siteSurveyStatus=SITE_SURVEY_STATUS_RUNNING;
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0-vxd0", "down", NULL_STR);

	if (sec == 0) //no 
	{
		sprintf(tmpBuf, "%s=%d", "encmode", 0);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);
		sprintf(tmpBuf, "%s=%d", "802_1x", 0);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);
	}
	else if (sec == 1)//wep
	{
		sprintf(tmpBuf, "%s=%d", "802_1x", 0);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);
		sitesurvey_connect_wepHandler(postData, len, tmpBuf, 0);
	}
	else if (sec == 2)//wpa-tkip
	{
		sprintf(tmpBuf, "%s=%d", "authtype", 0);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "encmode", 2);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "psk_enable", 1);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "wpa_cipher", 2);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		ptr = get_cstream_var(postData, len, ("pskValue0"), "");
		if (ptr[0]) {
			sprintf(tmpBuf, "%s=%s", "passphrase", ptr);
			RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);
		}
		else
			goto CONNECT_ERR0;

	}
	else if (sec == 4)//wpa2-psk
	{
		sprintf(tmpBuf, "%s=%d", "authtype", 0);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "encmode", 2);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "psk_enable", 2);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "wpa2_cipher", 8);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		ptr = get_cstream_var(postData, len, ("pskValue0"), "");
		if (ptr[0]) {
			sprintf(tmpBuf, "%s=%s", "passphrase", ptr);
			RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);
		}
		else
			goto CONNECT_ERR0;
	}
	else if (sec == 6)//wpa-mix
	{
		sprintf(tmpBuf, "%s=%d", "authtype", 0);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "encmode", 2);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "psk_enable", 3);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "wpa_cipher", 2);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		sprintf(tmpBuf, "%s=%d", "wpa2_cipher", 8);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

		ptr = get_cstream_var(postData, len, ("pskValue0"), "");
		if (ptr[0]) {
			sprintf(tmpBuf, "%s=%s", "passphrase", ptr);
			RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);
		}
		else
			goto CONNECT_ERR0;

	}

	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0-vxd0", "up", NULL_STR);
	if (sitesurvey_connect_test(tmpBuf, idx, sec))
		goto CONNECT_ERR0;

	printf("%s\n", tmpBuf);//bruce
	*connect_ok = 1;
	siteSurveyStatus=SITE_SURVEY_STATUS_WAITING;
	return 0;

CONNECT_ERR0:
	printf("%s\n", tmpBuf);//bruce
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0-vxd0", "up", NULL_STR);
	ERR_MSG2(tmpBuf);
	siteSurveyStatus=SITE_SURVEY_STATUS_OFF;
	return 1;
}
#endif /* #if defined(CONFIG_CUTE_MAHJONG_SELECTABLE) */

void formWizard(char *postData, int len)
{
	char *submitUrl=NULL;
	char *errMsg=NULL;
	int connect_ok = 0, val;
	errMsg=(char*)malloc(MSG_BUFFER_SIZE);
	if(!errMsg)
	{
		printf("malloc fail!\n");
		return -1;
	}
	bzero(errMsg,MSG_BUFFER_SIZE);

#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
//check sitesurvey connect
	if(sitesurvey_connect(postData,len, errMsg, &connect_ok)) goto WIZARD_ERRO;
//ui mode
	if(uiModeHandler(postData,len)) goto WIZARD_ERRO;
#endif

//opmode
	if(opModeHandler(postData,len)<0){sprintf(errMsg,"handle opmode fail!");goto WIZARD_ERRO;}
//ntp
	if(ntpHandler(postData,len,errMsg,1)<0)	goto WIZARD_ERRO;
//lan
	if(wizardHandleLan(postData,len,errMsg)<0) goto WIZARD_ERRO;
#ifdef HOME_GATEWAY
//wan
	if(wizardHandleWan(postData,len,errMsg)<0) goto WIZARD_ERRO;
#endif
//wlan
#ifndef HAVE_NOWIFI
	if(wizardHandleWlan(postData,len,errMsg)<0) goto WIZARD_ERRO;
#endif

#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
//site survey
	if(wizardHandleSiteSurvey(postData,len)<0) goto WIZARD_ERRO;
#endif

#if defined(CONFIG_CMJ_2G5G_USE_SAME_SECURITY)
//sync AP SSID/security
	if(sync_AP_setting()<0) goto WIZARD_ERRO;
#endif

	apmib_update_web(CURRENT_SETTING);
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
	extern void update_wps_configured(int reset_flag);
	// 2 means called from CMJ sync wlan security.
	update_wps_configured(2);
#endif


#ifdef HAVE_SYSTEM_REINIT
	if (connect_ok) {
		strcpy(errMsg, ("Connect successfully!"));
		wait_redirect_home(errMsg,10);
	}
	else {
	submitUrl=get_cstream_var(postData,len,"submit-url","");
		wait_redirect_home("Apply Changes",10);
		//wait_redirect("Apply Changes",10,submitUrl);
	}
	sleep(1);
	kick_reinit_m(SYS_REINIT_ALL);
#else
	if (connect_ok) {
		strcpy(errMsg, ("Connect successfully!"));
		CONNECT_OK_MSG(errMsg);
	}
	else {
	submitUrl=get_cstream_var(postData,len,"submit-url","");
	OK_MSG(submitUrl);
	}
#endif	
	save_cs_to_file();
	return 0;
WIZARD_ERRO:
	ERR_MSG(errMsg);
	if(errMsg)
	{
		free(errMsg);
		errMsg=NULL;
	}
	return 0;
}


