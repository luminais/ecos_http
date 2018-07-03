/*
 *      Web server handler routines for management (password, save config, f/w update)
 *
 *
 */
#ifdef ROUTE_SUPPORT
#include <network.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>


#include "http.h"
#include "apmib.h"
#include "sys_init.h"
#include "sys_utility.h"
#include "fmget.h"
#include "common.h"
//#include "fmget.c"
#ifndef MSG_BUFFER_SIZE
#define MSG_BUFFER_SIZE 256
#endif

int formRouteRip(char *postData, int len)
{
	char *submitUrl=NULL;
	char *errMsgBuf=(char*)malloc(MSG_BUFFER_SIZE);	
	int val=0;

	if(!errMsgBuf)
	{
		printf("malloc fail!!\n");
		goto setErr_fmroute;
	}
	bzero(errMsgBuf,MSG_BUFFER_SIZE);
	
	if(handle_fmGetValAndSetChkbox("enabled",MIB_DYNAMICROUTE_ENABLED,&val,postData,len,errMsgBuf)<0) 
		goto setErr_fmroute;

	if(val) //rip enable
	{
		if(handle_fmGetValAndSetIntVal("nat_enabled",MIB_NAT_ENABLED,NULL,postData,len,errMsgBuf)<0) 
			goto setErr_fmroute;
		if(handle_fmGetValAndSetIntVal("rip_enable",MIB_RIP_ENABLED,NULL,postData,len,errMsgBuf)<0) 
			goto setErr_fmroute;
//#ifdef ROUTE6D_SUPPORT
		if(handle_fmGetValAndSetIntVal("rip6_enable",MIB_RIP6_ENABLED,NULL,postData,len,errMsgBuf)<0) 
			goto setErr_fmroute;
//#endif
	}

	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}

	apmib_update_web(CURRENT_SETTING);
	submitUrl = get_cstream_var(postData,len, "submit-url", "");   // hidden page
		
#ifdef HAVE_SYSTEM_REINIT	
	wait_redirect("Apply Changes", 5, submitUrl); //wait 5s for daemon restart(if needed)
	sleep(1);
	kick_reinit_m(SYS_RIP_M);
#else
	OK_MSG(submitUrl);
#endif

	return 0;

setErr_fmroute:
	ERR_MSG(errMsgBuf);
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}

	return 0;
}

//delete repeate static route entry
char delRepSticRoute_entry(STATICROUTE_Tp pEntry,char *errMsgBuf)
{
	int entryNum=0,i=0,retVal=0;
	STATICROUTE_T entry={0};
	if(!apmib_get(MIB_STATICROUTE_TBL_NUM,(void *)&entryNum))
	{
		strcpy(errMsgBuf, ("Get entry number error!"));
		return -1;
	}
	
	for (i=entryNum; i>0; i--) 
	{
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&entry)) {
			strcpy(errMsgBuf, ("Get table entry error!"));
			return -1;
		}
		if(!memcmp(entry.dstAddr,pEntry->dstAddr,4)
			&&!memcmp(entry.netmask,pEntry->netmask,4)
			&&!memcmp(entry.gateway,pEntry->gateway,4)
			&&entry.interface==pEntry->interface)
		{
			if ( !apmib_set(MIB_STATICROUTE_DEL, (void *)&entry)) {
				strcpy(errMsgBuf, ("Delete table entry error!"));
				return -1;
			}
			delStaticRoute(pEntry);
			retVal=1;
		}
	}
	return retVal;
}
char set_enable_static_route(char enable)
{
	int entryNum=0,i=0,retVal=0;
	STATICROUTE_T entry={0};
	if(!apmib_get(MIB_STATICROUTE_TBL_NUM,(void *)&entryNum))
	{
		fprintf(stderr, ("Get entry number error!\n"));
		return -1;
	}
	
	for (i=entryNum; i>0; i--) 
	{
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&entry)) {
			fprintf(stderr,("Get table entry error!\n"));
			return -1;
		}
		if(enable) 
			addStaticRoute(&entry);
		else 
			delStaticRoute(&entry);
	}
	return 0;
}

void formRouteAdd(char *postData, int len)
{
	char *submitUrl=NULL,*strVal=NULL;
	STATICROUTE_T entry={0};
	int enable=0,origEnable=0;
	char *strIp=NULL, *strMask=NULL, *strGateway=NULL, *strRip=NULL, *strIface=NULL, *strMetric=NULL;
	char *errMsgBuf=(char*)malloc(MSG_BUFFER_SIZE);
	
	if(!errMsgBuf)
		return;

	strVal = get_cstream_var(postData,len, ("enabled"), "");
	strIp= get_cstream_var(postData,len, ("ipAddr"), "");
	strMask= get_cstream_var(postData,len, ("subnet"), "");
	strGateway= get_cstream_var(postData,len, ("gateway"), "");
	strIface= get_cstream_var(postData,len, ("iface"), "");
	strMetric= get_cstream_var(postData,len, ("metric"), "");
	if(!apmib_get(MIB_STATICROUTE_ENABLED,&origEnable))
	{
		strcpy(errMsgBuf,"get MIB_STATICROUTE_ENABLED fail!!\n");
		return;
	}
	if(handle_fmGetValAndSetChkbox("enabled",MIB_STATICROUTE_ENABLED,&enable,postData,len,errMsgBuf)<0)
		goto setErr_fmrouteAdd;
	if(!enable)
	{//only disable static route		
		goto setOk_fmrouteAdd;		
	}
	if(!strIp[0]&&!strMask[0]&&!strGateway[0]&&!strMetric[0])
	{//only enable static route
		goto setOk_fmrouteAdd;
	}
	/*else if(!strIp[0]||!strMask[0]||!strGateway[0]||!strIface[0]||!strMetric[0])
	{//have empty value
		strcpy(errMsgBuf, ("Invalid input!!"));
		goto setErr_fmrouteAdd;
	}*/	
	
	if(strIp[0])
	{
		 if ( !inet_aton(strIp, (struct in_addr *)&(entry.dstAddr)) ) {
			 sprintf(errMsgBuf, ("Invalid IP address value! %s"),strIp);
			 goto setErr_fmrouteAdd;
		 } 		
	}	
	if(strMask[0])
	{
		 if ( !inet_aton(strMask, (struct in_addr *)&(entry.netmask)) ) {
			 sprintf(errMsgBuf, ("Invalid IP address value! %s"),strIp);
			 goto setErr_fmrouteAdd;
		 } 		
	}	
	if(strGateway[0])
	{
		 if ( !inet_aton(strGateway, (struct in_addr *)&(entry.gateway)) ) {
			 sprintf(errMsgBuf, ("Invalid IP address value! %s"),strIp);
			 goto setErr_fmrouteAdd;
		 } 		
	}	
	if(strIface[0])
	{
		entry.interface=(unsigned char)atoi(strIface);
	}	
	if(strMetric[0])
	{
		entry.metric=(unsigned char)atoi(strMetric);
	}

	if(delRepSticRoute_entry(&entry,errMsgBuf)<0)
		goto setErr_fmrouteAdd;
	
	if ( apmib_set(MIB_STATICROUTE_ADD, (void *)&entry) == 0) {
		strcpy(errMsgBuf, ("Add table entry error!"));
		goto setErr_fmrouteAdd;
	}
	addStaticRoute(&entry);
setOk_fmrouteAdd:
	apmib_update_web(CURRENT_SETTING);
	if(origEnable && !enable)//try to disable route
		set_enable_static_route(0);
	else if(!origEnable && enable)//try to enable route
		set_enable_static_route(1);
	submitUrl = get_cstream_var(postData,len, "submit-url", "");   // hidden page
	if(submitUrl[0])
	{
		send_redirect_perm(submitUrl);	
	}else
	{
		send_redirect_perm("/wizard.htm");
	}
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	return 0;
setErr_fmrouteAdd:
	ERR_MSG(errMsgBuf);
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	return 0;

}

void formRouteDel(char *postData, int len)
{
	char *errMsgBuf=(char*)malloc(MSG_BUFFER_SIZE);
	char *submitUrl=NULL,*strVal=NULL;
	STATICROUTE_T entry={0};
	char tmpBuf[16]={0};
	int i=0,entryNum=0;
	
	if(!apmib_get(MIB_STATICROUTE_TBL_NUM,(void *)&entryNum))
	{
		strcpy(errMsgBuf, ("Get entry number error!"));
		goto setErr_fmrouteDel;
	}
	strVal=get_cstream_var(postData,len, "deleteSelRoute", "");
	if(strVal[0])
	{
		for (i=entryNum; i>0; i--) 
		{
			snprintf(tmpBuf, 16, "select%d", i);

			strVal = get_cstream_var(postData,len, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {
				*((char *)&entry) = (char)i;
				if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&entry)) {
					strcpy(errMsgBuf, ("Get table entry error!"));
					goto setErr_fmrouteDel;
				}
				if ( !apmib_set(MIB_STATICROUTE_DEL, (void *)&entry)) {
					strcpy(errMsgBuf, ("Delete table entry error!"));
					goto setErr_fmrouteDel;
				}
				delStaticRoute(&entry);
			}
		}
	}
	strVal=get_cstream_var(postData,len, "deleteAllRoute", "");
	if(strVal[0])
	{
		for (i=entryNum; i>0; i--) 
		{
			*((char *)&entry) = (char)i;
			if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&entry)) {
				strcpy(errMsgBuf, ("Get table entry error!"));
				goto setErr_fmrouteDel;
			}
			if ( !apmib_set(MIB_STATICROUTE_DEL, (void *)&entry)) {
				strcpy(errMsgBuf, ("Delete table entry error!"));
				goto setErr_fmrouteDel;
			}
			delStaticRoute(&entry);
		}		
	}
setOk_fmrouteDel:
	apmib_update_web(CURRENT_SETTING);
	submitUrl = get_cstream_var(postData,len, "submit-url", "");   // hidden page
	if(submitUrl[0])
	{
		send_redirect_perm(submitUrl);	
	}else
	{
		send_redirect_perm("/wizard.htm");
	}
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	return 0;
setErr_fmrouteDel:
	ERR_MSG(errMsgBuf);
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	return 0;

}

int staticRouteList(int argc,char**argv)
{
	int	nBytesSent=0, entryNum=0, i=0;
	STATICROUTE_T entry={0};
	char	ip[30], netmask[30], gateway[30], *tmpStr=NULL,*tmpBuf=NULL;

	if ( !apmib_get(MIB_STATICROUTE_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	tmpBuf=(char*)malloc(TMP_BUF_SIZE*4);
	if(!tmpBuf)
	{
		fprintf(stderr, "malloc fail!\n");
		return -1;
	}
	nBytesSent=snprintf(tmpBuf,TMP_BUF_SIZE*4, "<tr>"
      	"<td align=center width=\"24%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(route_tbl_destip)</script></b></font></td>\n"
      	"<td align=center width=\"23%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(route_tbl_mask)</script></b></font></td>\n"
      	"<td align=center width=\"23%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(route_tbl_gateway)</script></b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(route_tbl_metric)</script></b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(route_tbl_inter)</script></b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(route_tbl_select)</script></b></font></td></tr>\n");
    cyg_httpd_write_chunked(tmpBuf,nBytesSent);

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&entry))
		{
			if(tmpBuf) free(tmpBuf);
			return -1;
		}

		tmpStr = inet_ntoa(*((struct in_addr *)entry.dstAddr));
		strcpy(ip, tmpStr);
		tmpStr = inet_ntoa(*((struct in_addr *)entry.netmask));
		strcpy(netmask, tmpStr);
		tmpStr = inet_ntoa(*((struct in_addr *)entry.gateway));
		strcpy(gateway, tmpStr);

		nBytesSent=snprintf(tmpBuf,TMP_BUF_SIZE*4, "<tr>"
			"<td align=center width=\"24%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"23%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"23%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n",
				ip, netmask, gateway,entry.metric,(entry.interface?"WAN":"LAN"), i);
		cyg_httpd_write_chunked(tmpBuf,nBytesSent);
	}
	if(tmpBuf) free(tmpBuf);
	return nBytesSent;
}

char get_staticRouteEntry(STATICROUTE_Tp pEntry,char*dst,char*gw,char*msk,char*iface)
{
	int entryNum=0,i=0;
	char lanIface[16]={0},wanIface[16]={0},ifaceId=-1;
	if(!apmib_get(MIB_STATICROUTE_TBL_NUM,(void *)&entryNum))
	{
		fprintf(stderr, ("Get entry number error!"));
		return 0;
	}
	getInterfaces(lanIface,wanIface);
	//diag_printf("%s:%d lanif=%s wanif=%s iface=%s\n",__FILE__,__LINE__,lanIface,wanIface,iface);
	if(!strcmp(iface,lanIface))
		ifaceId=0;
	else if(!strcmp(iface,wanIface))
		ifaceId=1;
	else
		return 0;
	for (i=entryNum; i>0; i--) 
	{
		*((char *)pEntry) = (char)i;
		if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)pEntry)) {
			fprintf(stderr, ("Get table entry error!"));
			return 0;
		}
		if(!strcmp(inet_ntoa(*((struct in_addr *)(pEntry->dstAddr))),dst)
			&&!strcmp(inet_ntoa(*((struct in_addr *)(pEntry->gateway))),gw)
			&&!strcmp(inet_ntoa(*((struct in_addr *)(pEntry->netmask))),msk)
			&&pEntry->interface==ifaceId)
		{
			return 1;
		}
	}
	return 0;
}
#ifndef ROUTER_BUFFER_SIZE
#define ROUTER_BUFFER_SIZE 1024*4
#endif
int kernelRouteList(int argc,char**argv)
{
	int	nBytesSent=0;
	char *tmpBuf=(char*)malloc(TMP_BUF_SIZE*4);
	char *outBuf=(char*)malloc(ROUTER_BUFFER_SIZE);
	STATICROUTE_T entry={0};

	if(!tmpBuf||!outBuf)
	{
		fprintf(stderr, "malloc fail!\n");
		return -1;
	}
	
	nBytesSent=snprintf(tmpBuf, TMP_BUF_SIZE*4, "<tr>"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(routetbl_dst)</script></b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(routetbl_gw)</script></b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(routetbl_mask)</script></b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(routetbl_flag)</script></b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(routetbl_iface)</script></b></font></td>"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(routetbl_type)</script></b></font></td></tr>\n");
	cyg_httpd_write_chunked(tmpBuf, nBytesSent);

	char shellcmd[11];
	strcpy(shellcmd, "show route");
	//diag_printf("shellcmd: %s\n", shellcmd);
	if(getShellOutput(shellcmd, outBuf, ROUTER_BUFFER_SIZE)>0)
	{
		char * tmpStr=outBuf;
		char * token=NULL;
		char dst[40]={0};
		char gw[40]={0};
		char msk[16]={0};
		char flgs[9]={0};
		char iface[9]={0};
		char lanif[9]={0};
		char wanif[9]={0};
		char entry_type=0;
		char ifaceTypeWan=0;
		tmpStr+=strlen("Routing tables\n")+1;
		tmpStr+=strlen("Destination     Gateway         Mask            Flags    Interface\n")+1;
		if(token=strstr(tmpStr, "Interface statistics"))
			token[0]='\0';
		else
			outBuf[ROUTER_BUFFER_SIZE-1]='\0';
		token=strtok(tmpStr,"\n");
		while(token!=NULL)
		{
			sscanf(token,"%40s %40s %15s %8s %8s ",&dst,&gw,&msk,&flgs,&iface);
			if(get_staticRouteEntry(&entry,dst,gw,msk,iface))
				entry_type=1;
			else
				entry_type=0;
			getInterfaces(lanif,wanif);
			if(!strcmp(wanif,iface))
				ifaceTypeWan=1;
			else
				ifaceTypeWan=0;
			nBytesSent = snprintf(tmpBuf,TMP_BUF_SIZE*4, "<tr>"
							"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%-16s</td>\n"
							"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%-16s</td>\n"
							"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%-16s</td>\n"
							"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
							"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
							"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n",
						dst, gw, msk, flgs,(ifaceTypeWan?"WAN":"LAN") ,(entry_type?"Static":"Dynamic"));
			cyg_httpd_write_chunked(tmpBuf,nBytesSent);
			token=strtok(NULL,"\n");
		}	
		
	}else
	{	
		fprintf(stderr,"get route fail!!\n");
	}
	
	if(tmpBuf)
		free(tmpBuf);
	if(outBuf)
		free(outBuf);

	return nBytesSent;
}
#endif //ROUTE_SUPPORT
