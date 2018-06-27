#include <network.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#ifdef SYSLOG_SUPPORT
#include <cyg/infra/diag.h>
#endif

#include "http.h"
#include "apmib.h"
#include "sys_utility.h"
#include "multipart.h"
#ifdef SYSLOG_SUPPORT
#ifndef LOG_MAX_LINE_SIZE
#define LOG_MAX_LINE_SIZE 128
#endif
#define LOG_MAX_SHOW_BUFFER_SIZE 1024

int outputFileLastBuff(char* fileName,int outputSize,char*buff,int buffSize)
{
	int file_tmp=-1;
	char chVar=0;
	int intVar=0,i=0;
	struct stat fileStat={0};

	if(!fileName||!buff) return -1;
	file_tmp=open(fileName,O_RDONLY,0644);
	if(file_tmp<0)
	{
		//fprintf(stderr,"open log file fail!Can't show system log %s!\n",fileName);
		return 0;
	}
	fstat(file_tmp,&fileStat);
	if(fileStat.st_size<=0)
		goto outputFileLastBuff_END;

	if(fileStat.st_size>outputSize)
		lseek(file_tmp,-outputSize,SEEK_END);

	do{
		if(read(file_tmp,&chVar,1)==0)
			goto outputFileLastBuff_END;
	}while(chVar!='\n');

	bzero(buff,buffSize);
	i=0;
	do{
		intVar=read(file_tmp,&chVar,1);
		buff[i++]=chVar;
		if(chVar=='\n' || intVar==0)		
		{
			cyg_httpd_write_chunked(buff,i);
			bzero(buff,buffSize);
			i=0;
		}
			
	}while(intVar!=0);
outputFileLastBuff_END:
	close(file_tmp);
	return 0;
}
int sysLogList( int argc, char **argv)
{
	FILE*fp=NULL;
	int nBytesSent=0;
	char *buf=(char*)malloc(LOG_MAX_LINE_SIZE);
	char tempChar=0;
	struct stat fileStat={0};
	struct stat fileStatBak={0};
	if(!buf)
	{
		printf("malloc fial!");
		return nBytesSent;
	}

	stat(SYSLOG_FILE_NAME,&fileStat);
	stat(SYSLOG_FILE_BAK_NAME,&fileStatBak);
	if(fileStat.st_size<LOG_MAX_SHOW_BUFFER_SIZE && fileStatBak.st_size>0)//need to show bak file log
		outputFileLastBuff(SYSLOG_FILE_BAK_NAME,LOG_MAX_SHOW_BUFFER_SIZE-fileStat.st_size,buf,LOG_MAX_LINE_SIZE);

	outputFileLastBuff(SYSLOG_FILE_NAME,LOG_MAX_SHOW_BUFFER_SIZE,buf,LOG_MAX_LINE_SIZE);
	
#if 0
	if((fp=fopen(SYSLOG_FILE_NAME,"r"))==NULL)
	{
		goto SYSLOG_END;
	}
	bzero(buf,LOG_MAX_LINE_SIZE);
	while((tempChar=fgetc(fp))!=EOF)
	{
		if(strlen(buf)>=LOG_MAX_LINE_SIZE-1)
		{
			fprintf(stderr,"buf over size!\n");
			goto SYSLOG_END;
		}
		buf[strlen(buf)]=tempChar;
		if(tempChar=='\n')		
		{
			//diag_printf("%s:%d buf=%s\n",__FILE__,__LINE__,buf);
			nBytesSent=strlen(buf);
			cyg_httpd_write_chunked(buf,nBytesSent);
			bzero(buf,LOG_MAX_LINE_SIZE);
		}		
    }
    fclose(fp);
#endif
SYSLOG_END:
	if(buf)
	{
		free(buf);
		buf=NULL;
	}
	return nBytesSent;
}



void formSysLog(char *postData, int len)
{
	char *submitUrl, *tmpStr;
	char tmpBuf[100];
	int enabled=0, rt_enabled;
	struct in_addr ipAddr ;
	
	submitUrl = get_cstream_var(postData,len,"submit-url","");   // hidden page
	//printf("submitUrl is %s.\n",submitUrl);

	tmpStr = get_cstream_var(postData,len,"clear","");
	if(tmpStr[0]){
		unlink(SYSLOG_FILE_NAME);
		unlink(SYSLOG_FILE_BAK_NAME);
		send_redirect_perm(submitUrl);
		return;
	}

/*
 *	NOTE: If variable enabled (MIB_SCRLOG_ENABLED) bitmask modify(bitmap),
 *	 	Please modify driver rtl8190 reference variable (dot1180211sInfo.log_enabled in linux-2.4.18/drivers/net/rtl8190/8190n_cfg.h) 
 */
	apmib_get(MIB_SCRLOG_ENABLED, (void *)&enabled);
	
	tmpStr = get_cstream_var(postData,len,"logEnabled",""); 
	if(!strcmp(tmpStr, "ON")) {
		enabled |= SYSLOG_ENABLE;

		tmpStr = get_cstream_var(postData,len,"syslogEnabled","");
		if(!strcmp(tmpStr, "ON"))
			enabled |= SYSLOG_ALL_ENABLE;		
		else
			enabled &= ~SYSLOG_ALL_ENABLE;
		
		tmpStr = get_cstream_var(postData,len,"wlanlogEnabled","");
		if(!strcmp(tmpStr, "ON")) 
			enabled |= SYSLOG_WLANLOG_ENABLE;	
		else
			enabled &= ~SYSLOG_WLANLOG_ENABLE;
		
#ifdef HOME_GATEWAY
#ifdef DOS_SUPPORT
		tmpStr = get_cstream_var(postData,len,"doslogEnabled","");
		if(!strcmp(tmpStr, "ON")) 
			enabled |= SYSLOG_DOSLOG_ENABLE;		
		else
			enabled &= ~SYSLOG_DOSLOG_ENABLE;		
#endif
#endif

#ifdef CONFIG_RTK_MESH
		tmpStr = get_cstream_var(postData,len,"meshlogEnabled","");
		if(!strcmp(tmpStr, "ON")) 
			enabled |= SYSLOG_MESHLOG_ENABLE;	
		else
			enabled &= ~SYSLOG_MESHLOG_ENABLE;
#endif

	}
	else
		enabled &= ~SYSLOG_ENABLE;						

	if ( apmib_set(MIB_SCRLOG_ENABLED, (void *)&enabled) == 0) {
		strcpy(tmpBuf, ("Set log enable error!"));
		goto setErr;
	}
	
	if(enabled & 1){
		tmpStr = get_cstream_var(postData,len,"rtLogEnabled","");
		if(!strcmp(tmpStr, "ON"))
			rt_enabled= 1;
		else
			rt_enabled= 0;
		if ( apmib_set(MIB_REMOTELOG_ENABLED, (void *)&rt_enabled) == 0) {
			strcpy(tmpBuf, ("Set remote log enable error!"));
			goto setErr;
		}

		tmpStr = get_cstream_var(postData,len,"rtlogServer","");
		if(tmpStr[0]){
			inet_aton(tmpStr, &ipAddr);
			if ( apmib_set(MIB_REMOTELOG_SERVER, (void *)&ipAddr) == 0) {
				strcpy(tmpBuf, ("Set remote log server error!"));
				goto setErr;
			}
		}
	}
	
	apmib_update_web(CURRENT_SETTING);
	
	set_log();
	send_redirect_perm(submitUrl);
	//OK_MSG(submitUrl);

	save_cs_to_file();

	return;

setErr:
	ERR_MSG(tmpBuf);
}
#endif

	

