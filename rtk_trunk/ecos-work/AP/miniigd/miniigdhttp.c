/* $Id: upnphttp.c,v 1.2 2008/03/02 10:16:43 jasonwang Exp $ */
/* Project :  miniupnp
 * Website :  http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * Author :   Thomas Bernard
 * Copyright (c) 2005 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * LICENCE file included in this distribution.
 * */
#ifdef __ECOS
#include <network.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifndef __ECOS
#include <syslog.h>
#endif
#include "miniigdhttp.h"
#include "upnpdescgen.h"
#include "miniupnpdpath.h"
#include "miniigdsoap.h"
#ifdef 	ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif

#if defined(ENABLE_EVENTS)
#include "upnpevents.h"
#endif
#ifdef	ECOS_DBG_STAT
extern int	dbg_igd_index;
extern int	dbg_igd_index1;

#endif

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

struct upnphttp * MiniigdNew_upnphttp(int s)
{
	struct upnphttp * ret;
	if(s<0)
		return NULL;
	ret = (struct upnphttp *)malloc(sizeof(struct upnphttp));
	if(ret == NULL)
		return NULL;
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, sizeof(struct upnphttp));
#endif
	memset(ret, 0, sizeof(struct upnphttp));
	ret->socket = s;
	return ret;
}

void MiniigdCloseSocket_upnphttp(struct upnphttp * h)
{
	if(close(h->socket)==0){
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index1, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
	h->socket = -1;
	}
	else
		diag_printf("miniigd can't close http accept socket!\n");
	h->state = 100;
}

void MiniigdDelete_upnphttp(struct upnphttp * h)
{
	if(h)
	{
		if(h->socket >= 0){
			if(close(h->socket)==0){
#ifdef	ECOS_DBG_STAT
			dbg_stat_add(dbg_igd_index1, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		
			}else
				diag_printf("miniigd can't close http accept socket!\n");
		}
		if(h->req_buf){			
			free(h->req_buf);
#ifdef	ECOS_DBG_STAT
			dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,h->req_buflen);
#endif
		}
		if(h->res_buf){			
			free(h->res_buf);
#ifdef	ECOS_DBG_STAT
			dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,h->res_buf_alloclen);
#endif
		}
		free(h);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct upnphttp));
#endif

	}
}

/* parse HttpHeaders of the REQUEST */
static void ParseHttpHeaders(struct upnphttp * h)
{
	char * line;
	char * colon;
	char * p;
	int n;
	line = h->req_buf;
	/* TODO : check if req_buf, contentoff are ok */
	while(line < (h->req_buf + h->req_contentoff))
	{
		colon = strchr(line, ':');
		if(colon)
		{
			if(strncasecmp(line, "Content-Length", 14)==0)
			{
				p = colon;
				while(*p < '0' || *p > '9')
					p++;
				h->req_contentlen = atoi(p);
				/*printf("*** Content-Lenght = %d ***\n", h->req_contentlen);
				printf("    readbufflen=%d contentoff = %d\n",
					h->req_buflen, h->req_contentoff);*/
			}
			else if(strncasecmp(line, "SOAPAction", 10)==0)
			{
				p = colon;
				n = 0;
				while(*p == ':' || *p == ' ' || *p == '\t')
					p++;
				while(p[n]>=' ')
				{
					n++;
				}
				if((p[0] == '"' && p[n-1] == '"')
				  || (p[0] == '\'' && p[n-1] == '\''))
				{
					p++; n -= 2;
				}
				h->req_soapAction = p;
				h->req_soapActionLen = n;
			}
#ifdef ENABLE_EVENTS
			else if(strncasecmp(line, "Callback", 8)==0)
			{
				p = colon;
				while(*p != '<' && *p != '\r' )
					p++;
					n = 0;
				while(p[n] != '>' && p[n] != '\r' )
					n++;
					h->req_Callback = p + 1;
					h->req_CallbackLen = MAX(0, n - 1);
			}
			else if(strncasecmp(line, "SID", 3)==0)
			{
				p = colon + 1;
				while(isspace(*p)){
					if(*p == '\r'){
						break;
					}
					p++;
				}
					n = 0;
				while(!isspace(p[n])){
					n++;
				}
				if(n > 0)
				h->req_SID = p;
				else
					h->req_SID = NULL;
				h->req_SIDLen = n;
			}
			/* Timeout: Seconds-nnnn */
			/* TIMEOUT
			Recommended. Requested duration until subscription expires,
			either number of seconds or infinite. Recommendation
			by a UPnP Forum working committee. Defined by UPnP vendor.
			 Consists of the keyword "Second-" followed (without an
			intervening space) by either an integer or the keyword "infinite". */
			else if(strncasecmp(line, "Timeout", 7)==0)
			{
				p = colon + 1;
				while(isspace(*p))
					p++;
				if(strncasecmp(p, "Second-", 7)==0) {
					if(strncasecmp(p+7, "infinite", 8)==0){
						h->req_Timeout = 0xFFFFFFFF;
					}else
					h->req_Timeout = atoi(p+7);
					}
			}
#endif
		}
		while(!(line[0] == '\r' && line[1] == '\n'))
			line++;
		line += 2;
	}
}

/* very minimalistic 404 error message */
static void Send404(struct upnphttp * h)
{
	static const char error404[] = "HTTP/1.1 404 Not found\r\n"
		"Connection: close\r\n"
		"Content-type: text/html\r\n"
		"\r\n"
		"<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>"
		"<BODY><H1>Not Found</H1>The requested URL was not found"
		" on this server.</BODY></HTML>\r\n";
	int n;
// david	
//	n = send(h->socket, error404, sizeof(error404) - 1, 0);
	n = send(h->socket, error404, sizeof(error404) - 1, 
#ifndef __ECOS
	MSG_DONTROUTE | MSG_NOSIGNAL);
#else
	MSG_DONTROUTE);
#endif
	if(n < 0)
	{
		syslog(LOG_ERR, "Send404: send: %m");
	}
	MiniigdCloseSocket_upnphttp(h);
}

/* very minimalistic 501 error message */
static void Send501(struct upnphttp * h)
{
	static const char error501[] = "HTTP/1.1 501 Not Implemented\r\n"
		"Connection: close\r\n"
		"Content-type: text/html\r\n"
		"\r\n"
		"<HTML><HEAD><TITLE>501 Not Implemented</TITLE></HEAD>"
		"<BODY><H1>Not Implemented</H1>The HTTP Method "
		"is not implemented by this server.</BODY></HTML>\r\n";
	int n;
//david	
//	n = send(h->socket, error501, sizeof(error501) - 1, 0);
	n = send(h->socket, error501, sizeof(error501) - 1, 
#ifndef __ECOS
	MSG_DONTROUTE | MSG_NOSIGNAL);
#else
	MSG_DONTROUTE);
#endif
	if(n < 0)
	{
		syslog(LOG_ERR, "Send501: send: %m");
	}
	MiniigdCloseSocket_upnphttp(h);
}

static const char * findendheaders(const char * s, int len)
{
	while(len-->0)
	{
		if(s[0]=='\r' && s[1]=='\n' && s[2]=='\r' && s[3]=='\n')
			return s;
		s++;
	}
	return NULL;
}

static void sendDummyDesc(struct upnphttp * h)
{
	static const char xml_desc[] = "<?xml version=\"1.0\"?>\n"
		"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
		" <specVersion>"
		"    <major>1</major>"
		"    <minor>0</minor>"
		"  </specVersion>"
		"  <actionList />"
		"  <serviceStateTable />"
		"</scpd>";
	MiniigdBuildResp_upnphttp(h, xml_desc, sizeof(xml_desc)-1);
	MiniigdSendResp_upnphttp(h);
	MiniigdCloseSocket_upnphttp(h);
}

/* Sends the description generated by the parameter */
static void sendXMLdesc(struct upnphttp * h, char * (f)(int *))
{
	char * desc;
	int len;
	desc = f(&len);
	if(!desc)
	{
		syslog(LOG_ERR, "XML description generation failed");
		MiniigdCloseSocket_upnphttp(h);
		return;
	}
	MiniigdBuildResp_upnphttp(h, desc, len);
	MiniigdSendResp_upnphttp(h);
	MiniigdCloseSocket_upnphttp(h);
	free(desc);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 2048);
#endif
}
#ifdef ENABLE_EVENTS
static void ProcessHTTPSubscribe_upnphttp(struct upnphttp * h, const char * path)
{
	const char * sid=NULL;
	//syslog(LOG_DEBUG, "ProcessHTTPSubscribe %s", path);
	//syslog(LOG_DEBUG, "Callback '%.*s' Timeout=%d",h->req_CallbackLen, h->req_Callback, h->req_Timeout);
	//syslog(LOG_DEBUG, "SID '%.*s'", h->req_SIDLen, h->req_SID);
	if(!h->req_Callback && !h->req_SID) {
		/* Missing or invalid CALLBACK : 412 Precondition Failed.
		 * If CALLBACK header is missing or does not contain a valid HTTP URL,
		 * the publisher must respond with HTTP error 412 Precondition Failed*/
		MiniigdBuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
		MiniigdSendResp_upnphttp(h);
		MiniigdCloseSocket_upnphttp(h);
	} else {
		/* - add to the subscriber list
		 * - respond HTTP/x.x 200 OK 
		 * - Send the initial event message */
		/* Server:, SID:; Timeout: Second-(xx|infinite) */
		if(h->req_Callback) {
			sid = upnpevents_addSubscriber(path, h->req_Callback,
			                               h->req_CallbackLen, h->req_Timeout);
			h->respflags = FLAG_TIMEOUT;
			if(sid) {
				//syslog(LOG_DEBUG, "generated sid=%s", sid);
				h->respflags |= FLAG_SID;
				h->req_SID = sid;
				h->req_SIDLen = strlen(sid);
				MiniigdBuildResp_upnphttp(h, 0, 0);
			}else{
				MiniigdBuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
			}
			
		} else {
			/* subscription renew */
			/* Invalid SID
				412 Precondition Failed. If a SID does not correspond to a known,
				un-expired subscription, the publisher must respond
				with HTTP error 412 Precondition Failed. */
			if(renewSubscription(h->req_SID, h->req_SIDLen, h->req_Timeout) < 0) {
				MiniigdBuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
			} else {
				MiniigdBuildResp_upnphttp(h, 0, 0);
			}
		}
		MiniigdSendResp_upnphttp(h);
		MiniigdCloseSocket_upnphttp(h);
	}
}

static void ProcessHTTPUnSubscribe_upnphttp(struct upnphttp * h, const char * path)
{
	//syslog(LOG_DEBUG, "ProcessHTTPUnSubscribe %s", path);
	//syslog(LOG_DEBUG, "SID '%.*s'", h->req_SIDLen, h->req_SID);
	//syslog(LOG_DEBUG, "SID '%.*s'", h->req_SIDLen, h->req_SID);
	/* Remove from the list */
	if(!h->req_SID && !h->req_SIDLen) {
		/* Missing or invalid SID : 412 Precondition Failed.*/
		MiniigdBuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
		MiniigdSendResp_upnphttp(h);
		MiniigdCloseSocket_upnphttp(h);
	}else{
		if(upnpevents_removeSubscriber(h->req_SID, h->req_SIDLen) < 0) {
			MiniigdBuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
		} else {
			MiniigdBuildResp_upnphttp(h, 0, 0);
		}
	MiniigdSendResp_upnphttp(h);
	MiniigdCloseSocket_upnphttp(h);
	}
}
#endif



/* ProcessHTTPPOST_upnphttp()
 * executes the SOAP query if it is possible */
static void ProcessHTTPPOST_upnphttp(struct upnphttp * h)
{
	if((h->req_buflen - h->req_contentoff) >= h->req_contentlen)
	{
		if(h->req_soapAction)
		{
			/* we can process the request */
//			syslog(LOG_INFO, "SOAPAction: %.*s",
//		    	   h->req_soapActionLen, h->req_soapAction);
			_ExecuteSoapAction(h, 
				h->req_soapAction,
				h->req_soapActionLen);
		}
		else
		{
			static const char err400str[] =
				"<html><body>Bad request</body></html>";
			syslog(LOG_INFO, "No SOAPAction in HTTP headers");
			MiniigdBuildResp2_upnphttp(h, 400, "Bad Request",
			                    err400str, sizeof(err400str) - 1);
			MiniigdSendResp_upnphttp(h);
			MiniigdCloseSocket_upnphttp(h);
		}
	}
	else
	{
		/* waiting for remaining data */
		h->state = 1;
	}
}

/* Parse and process Http Query 
 * called once all the HTTP headers have been received. */
static void ProcessHttpQuery_upnphttp(struct upnphttp * h)
{
	char HttpCommand[16];
	char HttpUrl[128];
	char * HttpVer;
	char * p;
	int i;
	p = h->req_buf;
	if(!p){
		h->state = 100;
		return;
	}
	for(i = 0; i<15 && *p != ' ' && *p != '\r'; i++)
		HttpCommand[i] = *(p++);
	HttpCommand[i] = '\0';
	while(*p==' ')
		p++;
	for(i = 0; i<127 && *p != ' ' && *p != '\r'; i++)
		HttpUrl[i] = *(p++);
	HttpUrl[i] = '\0';
	while(*p==' ')
		p++;
	HttpVer = h->HttpVer;
	for(i = 0; i<15 && *p != '\r'; i++)
		HttpVer[i] = *(p++);
	HttpVer[i] = '\0';
//	syslog(LOG_INFO, "HTTP REQUEST : %s %s (%s)",
//	       HttpCommand, HttpUrl, HttpVer);
	ParseHttpHeaders(h);
	if(strcmp("POST", HttpCommand) == 0)
	{
		h->req_command = EPost;
		ProcessHTTPPOST_upnphttp(h);
	}
	else if(strcmp("GET", HttpCommand) == 0)
	{
		h->req_command = EGet;
		if(strcmp(ROOTDESC_PATH, HttpUrl) == 0)
		{
			sendXMLdesc(h, genRootDesc);
		}
		else if(strcmp(WANIPC_PATH, HttpUrl) == 0)
		{
			sendXMLdesc(h, genWANIPCn);
		}
		else if(strcmp(WANCFG_PATH, HttpUrl) == 0)
		{
			sendXMLdesc(h, genWANCfg);
		}
		else if(strcmp(DUMMY_PATH, HttpUrl) == 0)
		{
			sendDummyDesc(h);
		}
		else
		{
			syslog(LOG_NOTICE, "%s not found, responding ERROR 404", HttpUrl);
			Send404(h);
		}
	}
#ifdef ENABLE_EVENTS
	else if(strcmp("SUBSCRIBE", HttpCommand) == 0)
	{
		if(strcmp("/dummy", HttpUrl) == 0){
		Send501(h);
		}else{
		h->req_command = ESubscribe;
		ProcessHTTPSubscribe_upnphttp(h, HttpUrl);
	}
	}
	else if(strcmp("UNSUBSCRIBE", HttpCommand) == 0)
	{
		if(strcmp("/dummy", HttpUrl) == 0){
			Send501(h);
		}else{
		h->req_command = EUnSubscribe;
		ProcessHTTPUnSubscribe_upnphttp(h, HttpUrl);
	}
	}
#else
	else if(strcmp("SUBSCRIBE", HttpCommand) == 0)
	{
		//syslog(LOG_NOTICE, "SUBSCRIBE not implemented yet");
		Send501(h);
	}
#endif
	else
	{
		syslog(LOG_NOTICE, "Unsupported HTTP Command %s", HttpCommand);
		Send501(h);
	}
}


void MiniigdProcess_upnphttp(struct upnphttp * h)
{
	char *buf;
	int n;
	if(!h)
		return;
	buf = (char *)malloc(2048);
	if(buf == NULL){
		syslog(LOG_ERR, "MiniigdProcess_upnphttp: out of memory!");
		h->state = 100;
		return;
	}
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD,2048);
#endif
	memset(buf, 0, 2048);
	switch(h->state)
	{
	case 0:		
		n = recv(h->socket, buf, 2048, MSG_DONTWAIT);
		if(n<0)
		{
			syslog(LOG_ERR, "recv (state0): %m");
			h->state = 100;
		}
		else if(n==0)
		{
			syslog(LOG_WARNING, "connection closed inexpectedly");
			h->state = 100;
		}
		else
		{
			const char * endheaders;
			char *tmp = NULL;
			/*printf("== PACKET RECEIVED (%d bytes) ==\n", n);
			fwrite(buf, 1, n, stdout);	// debug
			printf("== END OF PACKET RECEIVED ==\n");*/
			/* if 1st arg of realloc() is null,
			 * realloc behaves the same as malloc() */
			
			tmp = (char *)realloc(h->req_buf, n + h->req_buflen + 1);
			if(tmp==NULL){
				h->state = 100;
				if(h ->req_buf){
					free(h ->req_buf);
#ifdef	ECOS_DBG_STAT
					dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,h ->req_buflen);
#endif
				}
				break;
			}
#ifdef	ECOS_DBG_STAT
			dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD,n);
#endif
			h->req_buf = tmp;
			memcpy(h->req_buf + h->req_buflen, buf, n);

			h->req_buflen += n;
			h->req_buf[h->req_buflen] = '\0';
			/* search for the string "\r\n\r\n" */
			endheaders = findendheaders(h->req_buf, h->req_buflen);
			if(endheaders)
			{
				h->req_contentoff = endheaders - h->req_buf + 4;
				ProcessHttpQuery_upnphttp(h);
			}
		}
		break;
	case 1:
		n = recv(h->socket, buf, 2048, MSG_DONTWAIT);
		if(n<0)
		{
			syslog(LOG_ERR, "recv (state1): %m");
			h->state = 100;
		}
		else if(n==0)
		{
			syslog(LOG_WARNING, "connection closed inexpectedly");
			h->state = 100;
		}
		else
		{
			/*fwrite(buf, 1, n, stdout);*/	/* debug */
			char *tmpe = NULL;
			tmpe = (char *)realloc(h->req_buf, n + h->req_buflen);
			if(tmpe==NULL){
				h->state = 100;
				if(h->req_buf){
					free(h->req_buf);
#ifdef	ECOS_DBG_STAT
					dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,h->req_buflen);
#endif
				}
				break;
			}
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD,n);
#endif
			h->req_buf = tmpe;
			memcpy(h->req_buf + h->req_buflen, buf, n);
			h->req_buflen += n;
			if((h->req_buflen - h->req_contentoff) >= h->req_contentlen)
			{
				ProcessHTTPPOST_upnphttp(h);
			}
		}
		break;
	default:
		syslog(LOG_WARNING, "unexpected state (%d)", h->state);
		h->state = 100;
		break;
	}
	free(buf);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,2048);
#endif
}
#ifdef ENABLE_EVENTS
static const char httpresphead[] =
	"%s %d %s\r\n"
	"Content-Type: %s\r\n"
	"Connection: close\r\n"
	"Content-Length: %d\r\n"
	"Server: miniupnpd/1.0 UPnP/1.0\r\n"
	;	/*"\r\n";*/
#else
static const char httpresphead[] =
	"%s %d %s\r\n"
	"Content-Type: text/xml; charset=\"utf-8\"\r\n"
	"Connection: close\r\n"
	"Content-Length: %d\r\n"
	"Server: miniupnpd/1.0 UPnP/1.0\r\n"
	"Ext:\r\n"
	"\r\n";
#endif
/*
		"<?xml version=\"1.0\"?>\n"
		"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
		"<s:Body>"

		"</s:Body>"
		"</s:Envelope>";
*/
/* with response code and response message
 * also allocate enough memory */
void
MiniigdBuildHeader_upnphttp(struct upnphttp * h, int respcode,
                     const char * respmsg,
                     int bodylen)
{
	int templen;
	if(!h->res_buf)
	{
		templen = sizeof(httpresphead) + 128 + bodylen;
		h->res_buf = (char *)malloc(templen);
		if(!h->res_buf)
			return;
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, templen);
#endif

		h->res_buf_alloclen = templen;
	}
#ifdef ENABLE_EVENTS	
	h->res_buflen = snprintf(h->res_buf, h->res_buf_alloclen,
	                         httpresphead, h->HttpVer,
	                         respcode, respmsg, 
	                         (h->respflags&FLAG_HTML)?"text/html":"text/xml",
	                         bodylen);
	/* Additional headers */

	if(h->respflags & FLAG_SID) {
		h->res_buflen += snprintf(h->res_buf + h->res_buflen,
		                          h->res_buf_alloclen - h->res_buflen,
		                          "SID: %s\r\n", h->req_SID);
	}
	if(h->respflags & FLAG_TIMEOUT) {
		h->res_buflen += snprintf(h->res_buf + h->res_buflen,
		                          h->res_buf_alloclen - h->res_buflen,
		                          "Timeout: Second-");
		if(h->req_Timeout) {
			h->res_buflen += snprintf(h->res_buf + h->res_buflen,
			                          h->res_buf_alloclen - h->res_buflen,
			                          "%d\r\n", h->req_Timeout);
		} else {
			h->res_buflen += snprintf(h->res_buf + h->res_buflen,
			                          h->res_buf_alloclen - h->res_buflen,
			                          "infinite\r\n");
		}
	}
	h->res_buf[h->res_buflen++] = '\r';
	h->res_buf[h->res_buflen++] = '\n';
#else
	h->res_buflen = snprintf(h->res_buf, h->res_buf_alloclen,httpresphead, h->HttpVer, respcode, respmsg, bodylen);
#endif

	if(h->res_buf_alloclen < (h->res_buflen + bodylen))
	{
		char* tmp = NULL;
		tmp = (char *)realloc(h->res_buf, (h->res_buflen + bodylen));
		if(tmp == NULL){
			if(h->res_buf){
				free(h->res_buf);
#ifdef	ECOS_DBG_STAT
				dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, h->res_buf_alloclen);
#endif
			}
			return;
		}
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, bodylen);
#endif
		h->res_buf = tmp;
		h->res_buf_alloclen = h->res_buflen + bodylen;
	}
}

void
MiniigdBuildResp2_upnphttp(struct upnphttp * h, int respcode,
                    const char * respmsg,
                    const char * body, int bodylen)
{
	MiniigdBuildHeader_upnphttp(h, respcode, respmsg, bodylen);
	if(h->req_buf==NULL)
		return;
	memcpy(h->res_buf + h->res_buflen, body, bodylen);
	h->res_buflen += bodylen;
}

/* responding 200 OK ! */
void MiniigdBuildResp_upnphttp(struct upnphttp * h,
                        const char * body, int bodylen)
{
	MiniigdBuildResp2_upnphttp(h, 200, "OK", body, bodylen);
}

void MiniigdSendResp_upnphttp(struct upnphttp * h)
{
	int n;
//david	
//	n = send(h->socket, h->res_buf, h->res_buflen, 0);
	n = send(h->socket, h->res_buf, h->res_buflen, 
#ifndef __ECOS
	MSG_DONTROUTE | MSG_NOSIGNAL);
#else
	MSG_DONTROUTE);
#endif	
	if(n<0)
	{
		syslog(LOG_ERR, "send(res_buf): %m");
	}
	else if(n < h->res_buflen)
	{
		syslog(LOG_ERR, "send(res_buf): %d bytes sent (out of %d)",
						n, h->res_buflen);
	}
}

