/* $Id: upnphttp.h,v 1.2 2007-08-31 11:36:38 chien_hsiang Exp $ */ 
/* miniupnp project
 * http://miniupnp.free.fr/
 * http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard
 * This software is subject to the conditions detailed in
 * the LICENCE file included in the distribution */
#ifndef __UPNPHTTP_H__
#define __UPNPHTTP_H__
#include <sys/queue.h>

#define syslog(x, fmt, args...);

/*
 States :
  0 - Waiting for data to read
  1 - Waiting for HTTP Post Content.
 ...
 >= 100 - to be deleted
*/
enum httpCommands {
	EUnknown = 0,
	EGet,
	EPost,
	ESubscribe,
	EUnSubscribe
};

struct upnphttp {
	int socket;
	int state;
	char HttpVer[16];
	/* request */
	char * req_buf;
	int req_buflen;
	int req_contentlen;
	int req_contentoff;     /* header length */
	enum httpCommands req_command;
	char * req_soapAction;
	int req_soapActionLen;
#ifdef ENABLE_EVENTS
	char * req_Callback;	/* For SUBSCRIBE */
	int req_CallbackLen;
	int req_Timeout;
	char * req_SID;		/* For UNSUBSCRIBE */
	int req_SIDLen;
	int respflags;
#endif	
	/* response */
	char * res_buf;
	int res_buflen;
	int res_buf_alloclen;
	/*int res_contentlen;*/
	/*int res_contentoff;*/		/* header length */
	LIST_ENTRY(upnphttp) entries;
};


#define FLAG_TIMEOUT	0x01
#define FLAG_SID		0x02

#define FLAG_HTML		0x80

struct upnphttp * MiniigdNew_upnphttp(int);

void MiniigdCloseSocket_upnphttp(struct upnphttp *);

void MiniigdDelete_upnphttp(struct upnphttp *);

void MiniigdProcess_upnphttp(struct upnphttp *);

/* Build the header for the HTTP Response
 * Also allocate the buffer for body data */
void
MiniigdBuildHeader_upnphttp(struct upnphttp * h, int respcode,
                     const char * respmsg,
                     int bodylen);
/* BuildResp_upnphttp() fill the res_buf buffer with the complete
 * HTTP 200 OK response from the body passed as argument */
void MiniigdBuildResp_upnphttp(struct upnphttp *, const char *, int);

/* same but with given response code/message */
void
MiniigdBuildResp2_upnphttp(struct upnphttp * h, int respcode,
                    const char * respmsg,
                    const char * body, int bodylen);

void MiniigdSendResp_upnphttp(struct upnphttp *);

#endif

