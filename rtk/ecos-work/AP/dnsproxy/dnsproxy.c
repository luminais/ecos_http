/* $Id: dnsproxy.c,v 1.16 2010/01/11 15:02:00 armin Exp $ */
/*
 * Copyright (c) 2003,2004,2005,2010 Armin Wolfermann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/system.h>
#include <cyg/kernel/kapi.h>           // Kernel API.
#include <cyg/kernel/ktypes.h>         // base kernel types.

//#include <config.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>  
//#include <sys/callout.h>		/* For struct callout_handle. */
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#define GLOBALS 1
#include "dnsproxy.h"
#ifdef 	ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
#include "../apmib/apmib.h"
#endif
#define RD(x) (*(x + 2) & 0x01)
#define MAX_BUFSPACE 512

static unsigned short queryid = 0;
#define QUERYID queryid++

static int sock_query;
#ifndef MAX_DNS_SERVER_NUM
#define MAX_DNS_SERVER_NUM 3
#endif
struct dnssrv_t dns_srv[MAX_DNS_SERVER_NUM];

int dns_server_count=0;
struct in_addr dns_server[MAX_DNS_SERVER_NUM];

static int dns_proxy_restart_flag=0;
static cyg_uint8 dns_proxy_quitting=0;

//static int dnsproxy_sig;
struct callout req_timer;
unsigned int authoritative_port=53;
unsigned int authoritative_timeout= 10<<6;
unsigned int recursive_port= 53;
unsigned int recursive_timeout= 90<<6;
unsigned int stats_timeout=3600;
unsigned int port=53;

char *authoritative;
char *chrootdir;
char *listenat;
char *recursive;
char *user;

unsigned long active_queries;
unsigned long all_queries;
unsigned long removed_queries;
unsigned long dropped_queries;
unsigned long answered_queries;
unsigned long dropped_answers;
unsigned long late_answers;
unsigned long hash_collisions;
unsigned long fw_queries_count=0;


//extern int event_gotsig;
//extern int (*event_sigcb)(void);
//extern struct in_addr dns_primary;
//extern struct in_addr dns_second;

static cyg_mutex_t dnsproxy_mutex;


#ifdef DEBUG
char *malloc_options = "AGZ";
#endif
#ifdef 	ECOS_DBG_STAT
extern int dbg_dns_index;
#endif
/* signal_handler -- Called by libevent if a signal arrives.
 */

void
signal_handler(int sig, short event, void *arg)
{
	fatal("exiting on signal %d", sig);
}

/* timeout */
static void
request_timeout()
{
	struct request *req ;
	struct request *tmp ;
	int i;

	if(dns_proxy_restart_flag > 0) {	
		/*restarting. do nothing*/
		cyg_callout_reset(&req_timer, recursive_timeout, request_timeout, NULL);
		return;
	}
	
	cyg_mutex_lock(&dnsproxy_mutex);
	for(i=0;i<(1 << HASHSIZE);i++){
		req=request_hash[i];
		while(req!=NULL) {	
			tmp=req->next;
			if(req->timeout){
				req->timeout--;
			}
			else{
				hash_remove_request(req);
				free(req);
			#ifdef 	ECOS_DBG_STAT
				dbg_stat_add(dbg_dns_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct request));
			#endif
				--active_queries;
				++removed_queries;
			}
			req = tmp;
		}
	}
	cyg_mutex_unlock(&dnsproxy_mutex);
	cyg_callout_reset(&req_timer, recursive_timeout, request_timeout, NULL);
}

#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
#define	DNS_CLASS_INET			1
#define	DNS_TYPE_A				1
#define	DEFAULT_TTL		(60 * 60)

#define RR_NAMESIZE 			300
#define	PACKET_DATABEGIN		12
#define	PACKET_ASSEMBLYSIZE		600
#define MASK_QR     			0x8000

#define SET_QR(x, y)        ((x) = ((x) & ~MASK_QR) | (((y) << 15) & MASK_QR))

typedef struct _header {
    unsigned short int	id;
    unsigned short      u;

    short int	qdcount;
    short int	ancount;
    short int	nscount;
    short int	arcount;	/* Till here it would fit perfectly to a real
    				 * DNS packet if we had big endian. */

    char	*here;		/* For packet parsing. */
    char	*packet;	/* The actual data packet ... */
    int		len;		/* ... with this size in bytes. */

    char	*rdata;		/* For packet assembly. */
} dnsheader_t;

typedef struct _rr {
    unsigned short      flags;
    char	  name[RR_NAMESIZE];
    
    unsigned int  type;
    unsigned int  class;

    unsigned long ttl;
    int		  len;
    char	  data[RR_NAMESIZE];
} rr_t;

static int get_objectname(unsigned char *msg, unsigned const char *limit, 
			  unsigned char **here, char *string, int strlen,
			  int k)
{
    unsigned int len;
    int	i;

    if ((*here>=limit) || (k>=strlen)) return(-1);
    while ((len = **here) != 0) {

	*here += 1;
	if ( *here >= limit ) return(-1);
	/* If the name is compressed (see 4.1.4 in rfc1035)  */
	if (len & 0xC0) {
	    unsigned offset;
	    unsigned char *p;

	    offset = ((len & ~0xc0) << 8) + **here;
	    if ((p = &msg[offset]) >= limit) return(-1);
	    if (p == *here-1) {
	      //log_debug("looping ptr");
	      return(-2);
	    }

	    if ((k = get_objectname(msg, limit, &p, string, RR_NAMESIZE, k))<0)
	      return(-1); /* if we cross the limit, bail out */
	    break;
	}
	else if (len < 64) {
	  /* check so we dont pass the limits */
	  if (((*here + len) > limit) || (len+k+2 > strlen)) return(-1);

	  for (i=0; i < len; i++) {
	    string[k++] = **here;
	    *here += 1;
	  }

	  string[k++] = '.';
	}
    }

    *here += 1;
    string[k] = 0;
    
    return (k);
}

static unsigned char *compile_char(dnsheader_t *x, char number)
{
    unsigned char conv = (unsigned char) number;
    memcpy(x->here, &conv, 1);
    x->here += 1;

    return (x->here);
}


static unsigned char *compile_int(dnsheader_t *x, int number)
{
    unsigned short conv = htons((unsigned short) number);
    memcpy(x->here, &conv, 2);
    x->here += 2;

    return (x->here);
}

static unsigned char *compile_long(dnsheader_t *x, long number)
{
    unsigned long conv = htonl((unsigned long) number);
    memcpy(x->here, &conv, 4);

    x->here += 4;

    return (x->here);
}

static unsigned char *compile_objectname(dnsheader_t *x)
{
    *x->here++ = 0xC0;
    *x->here++ = 0x0C;

    return (x->here);
}

static int compile_name(dnsheader_t *x, char *name)
{
    unsigned int c;
    int	i, k, n, offset;

    offset = x->here - x->packet;
    k = 0;
    while ((c = name[k]) != 0) {
	n = 0;
	while ((c = name[k+n]) != 0  &&  c != '.') {
	    n++;
	}

	if (n == 0) break;

	*x->here++ = (unsigned char) (n & 0x3F);
	for (i = 0; i < n; i++) {
	    *x->here++ = name[k++];
	}

	if (name[k] == '.') {
	    k++;
	}
    }

    *x->here++ = 0;
    return (offset);
}

static int end_rdata(dnsheader_t *x)
{
    unsigned short int conv;

    if (x->rdata != NULL) {
	int	rsize;

	rsize = x->here - (x->rdata + 2);
	conv = htons((unsigned short int) rsize);
	memcpy(x->rdata, &conv, 2);
    }
    
    x->rdata = NULL;
    return (0);
}

static int end_assembly(dnsheader_t *x)
{
    unsigned short int *pkt;

    end_rdata(x);
/*  SET_AA(x->u, 0); */

    pkt = (unsigned short int *) x->packet;
    pkt[1] = htons(x->u);
    pkt[2] = htons(x->qdcount);
    pkt[3] = htons(x->ancount);
    pkt[4] = htons(x->nscount);
    pkt[5] = htons(x->arcount);
    
    x->len = x->here - x->packet;
    return (x->len);
}



static int start_rdata(dnsheader_t *x)
{
    x->rdata = x->here;
    compile_int(x, 0);

    return (0);
}

static dnsheader_t *begin_assembly(rr_t *query,dnsheader_t* x)
{
    /*
     * Reset the packet ...
     */

    x->id = 0;
    x->u  = 0;
    SET_QR(x->u, 1);

    x->qdcount = 1;
    x->ancount = 0;
    x->nscount = 0;
    x->arcount = 0;
    x->rdata   = NULL;

    memset(x->packet, 0, PACKET_ASSEMBLYSIZE);
    x->here = &x->packet[PACKET_DATABEGIN];

    /*
     * ... and write the original query data.
     */

    compile_name(x, query->name);
    compile_int(x, query->type);
    compile_int(x, query->class);
    
    return (x);
}
#endif


/* do_query -- when a packet arrives at our listening socket. Read the packet, create a new query, append it to the
 * queue and send it to the correct server.
 */
static void
do_query(int fd)
{
	char buf[MAX_BUFSPACE];
	int byte = 0, i;
	struct sockaddr_in fromaddr;
	unsigned int fromlen = sizeof(fromaddr);
	struct request *req;
	int fail_count = 0;
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
	unsigned char	*here,*limit;
	rr_t			*y,query;
	dnsheader_t 	*x,xs;
	int	k;
	unsigned short int conv;
	unsigned char	domainName[MAX_NAME_LEN];
	unsigned char	domainNameCom[MAX_NAME_LEN+6];
	unsigned char	domainNameNet[MAX_NAME_LEN+6];
	unsigned long	ipName;
	unsigned char	ipAddr[4];
//	char			packet[PACKET_ASSEMBLYSIZE];
	char			*packet;
	packet=(char *)malloc(PACKET_ASSEMBLYSIZE);
	if(packet==NULL)
	{
		printf("malloc failed\n");
		return;
	}
	y = &query;
	x = &xs;
	x->packet = packet;
#endif

	++all_queries;

	/* read packet from socket */
	if ((byte = recvfrom(fd, buf, sizeof(buf), 0,
			    (struct sockaddr *)&fromaddr, &fromlen)) == -1) {
		error("recvfrom failed: %s", strerror(errno));
		++dropped_queries;
		
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
		free(packet);
#endif
		return;
	}

#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
	memset(domainName,'\0',MAX_NAME_LEN);
	memset(domainNameCom,'\0',MAX_NAME_LEN+6);
	memset(domainNameNet,'\0',MAX_NAME_LEN+6);
	apmib_get(MIB_DOMAIN_NAME,(void*)domainName);
	strncpy(domainNameCom,domainName,MAX_NAME_LEN);
	strncpy(domainNameNet,domainName,MAX_NAME_LEN);
	
	y->flags = ntohs(((short int *) buf)[1]);
	limit = buf+byte;
	here = &buf[PACKET_DATABEGIN];
	if ((k=get_objectname(buf, limit, &here, y->name, sizeof(y->name),0))< 0) {
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
		free(packet);
#endif
		return(0);
	}
	y->name[k] = 0;
	strcat(domainNameCom,".com.");
	strcat(domainNameNet,".net.");
	if(!strcasecmp(y->name,domainNameCom) ||
		!strcasecmp(y->name,domainNameNet)){
		memset(ipAddr,0,4);
		apmib_get(MIB_IP_ADDR,(void*)ipAddr);
		*(int *)&ipAddr = get_conflict_ip(*(int *)&ipAddr);

		if (here + 4 > limit) {
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
			free(packet);
#endif
			return;
		}	
   		memcpy(&conv, here, sizeof(unsigned short int));
    	y->type = ntohs(conv);
    	here += 2;

   		memcpy(&conv, here, sizeof(unsigned short int));
    	y->class = ntohs(conv);
    	here += 2;

		x = begin_assembly(&query,x);
		compile_objectname(x);
		compile_int(x, DNS_TYPE_A);
		compile_int(x, DNS_CLASS_INET);
		compile_long(x, DEFAULT_TTL);
		start_rdata(x);
		compile_char(x, ipAddr[0]);
		compile_char(x, ipAddr[1]);
		compile_char(x, ipAddr[2]);
		compile_char(x, ipAddr[3]);
		end_rdata(x);

		x->ancount = 1;
		end_assembly(x);
		memcpy(buf + 2, x->packet + 2, x->len - 2);
		

		if ((byte = sendto(fd, buf, (unsigned int)x->len, 0,
			(struct sockaddr *)&fromaddr,
			sizeof(struct sockaddr_in))) == -1) {
				diag_printf("send domian name query error.\n");
		}
		
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
		free(packet);
#endif
		return;
	}

#endif
	
	if(dns_server_count==0) {
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
		free(packet);
#endif
		return ;
	}
	/* check for minimum dns packet length */
	if (byte < 12) {
		error("query too short from %s", inet_ntoa(fromaddr.sin_addr));
		++dropped_queries;
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
		free(packet);
#endif
		return;
	}

	/* allocate new request */
	if ((req = calloc(1, sizeof(struct request))) == NULL) {
		error("calloc: %s", strerror(errno));
		++dropped_queries;
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
		free(packet);
#endif
		return;
	}
#ifdef 	ECOS_DBG_STAT
		dbg_stat_add(dbg_dns_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, sizeof(struct request));
#endif

	/* fill the request structure */
	req->id = QUERYID;
	memcpy(&req->client, &fromaddr, sizeof(struct sockaddr_in));
	memcpy(&req->clientid, &buf[0], 2);
	/* where is this query coming from? */
	if (is_internal(fromaddr.sin_addr)) {
		req->recursion = RD(buf);
		DPRINTF(("Internal query RD=%d\n", req->recursion));
	} else {
		/* no recursion for foreigners */
		req->recursion = 0;
		DPRINTF(("External query RD=%d\n", RD(buf)));
	}
	/*update timer*/
	req->timeout=1;
	/* insert it into the hash table */
	
	cyg_mutex_lock(&dnsproxy_mutex);
	hash_add_request(req);	
	cyg_mutex_unlock(&dnsproxy_mutex);
	/* overwrite the original query id */
	memcpy(&buf[0], &req->id, 2);


	fail_count = 0 ;
	for(i=0; i<dns_server_count; i++)
	{
		if(dns_srv[i].sock>0)
		{
			if(sendto(dns_srv[i].sock, buf, (unsigned int)byte, 0,
				    (struct sockaddr *)&dns_srv[i].addr, sizeof(struct sockaddr_in))<0)
			{
				++fail_count;
				++dropped_queries;
				// HF add. for mem exhausted when ppp disconnected.				
				if(fail_count ==dns_server_count)
				{
					
					cyg_mutex_lock(&dnsproxy_mutex);
					hash_remove_request(req);
					cyg_mutex_unlock(&dnsproxy_mutex);
					free(req);			
				}
#ifdef 	ECOS_DBG_STAT
				dbg_stat_add(dbg_dns_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct request));
#endif			
				continue;
			}
			fw_queries_count++;
		}
	}
#ifdef ECOS_DOMAIN_NAME_QUERY_SUPPORT
	free(packet);
#endif
}

/* do_answer -- Process a packet coming from our authoritative or recursive
 * server. Find the corresponding query and send answer back to querying
 * host.
 */
static void
do_answer(int fd)
{
	char buf[MAX_BUFSPACE];
	int byte = 0;
	struct request *query = NULL;
	/* read packet from socket */
	
	if ((byte = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL)) == -1) {
		error("recvfrom failed: %s", strerror(errno));
		++dropped_answers;
		return;
	}

	if(dns_server_count==0)
		return ;
	
	/* check for minimum dns packet length */
	if (byte < 12) {
		error("answer too short");
		++dropped_answers;
		return;
	}

	
	cyg_mutex_lock(&dnsproxy_mutex);
	/* find corresponding query */
	if ((query = hash_find_request(*((unsigned short *)&buf))) == NULL) {
		++late_answers;
		
		cyg_mutex_unlock(&dnsproxy_mutex);
		return;
	}	
	hash_remove_request(query);	
	cyg_mutex_unlock(&dnsproxy_mutex);
	/* restore original query id */
	memcpy(&buf[0], &query->clientid, 2);

	/* send answer back to querying host */
	
	if (sendto(sock_query, buf, (unsigned int)byte, 0,
			    (struct sockaddr *)&query->client,
			    sizeof(struct sockaddr_in)) <0) {			
		error("sendto failed: %s", strerror(errno));
		++dropped_answers;
	} else
		++answered_queries;
	
	free(query);
	query = NULL;
#ifdef 	ECOS_DBG_STAT
	dbg_stat_add(dbg_dns_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct request));
#endif

}

void restart_dns_proxy(int flag)
{
	dns_proxy_restart_flag=flag;	
}

void free_dns_hash_table()
{
	int i;
	struct request *phead=NULL, *pnode=NULL, *ptmp=NULL;
	cyg_mutex_lock(&dnsproxy_mutex);
	for(i=0; i<(1 << HASHSIZE); i++)
	{
		phead=request_hash[i];
		if(phead)
		{			
			pnode=phead->next;
			while(pnode!=NULL)
			{
				ptmp=pnode;
				pnode=pnode->next;
				free(ptmp);
			}
			free(phead);	
			request_hash[i]=NULL;
		}
	}	
	cyg_mutex_unlock(&dnsproxy_mutex);
}

void parse_resolv_file_to_array()
{
	int i;
	FILE *fp=NULL;
	unsigned char dns_buf[32];
	
	for(i=0; i<MAX_DNS_SERVER_NUM; i++)
		memset(&dns_server[i], 0, sizeof(struct in_addr));
	
	dns_server_count=0;			
	fp=fopen("/etc/resolv.conf","r");	

	if(fp!=NULL)
	{
		unsigned char *pchar=NULL;
		while(fgets(dns_buf, sizeof(dns_buf), fp))
		{
			if(dns_server_count>=MAX_DNS_SERVER_NUM)
				break;

			if(strlen(dns_buf)<10 || strlen(dns_buf)> 31 || strstr(dns_buf, "nameserver")==NULL)
				break;
			
			if(strstr(dns_buf, "0.0.0.0")!=NULL)
				continue;
			
			dns_buf[strlen(dns_buf)-1]='\0';
			
			//for(pchar=dns_buf; *pchar!='\n' && *pchar!='\0'; pchar++) ;
			//*pchar='\0';
			if((pchar=strchr(dns_buf, ' '))!=NULL)
			{
				pchar++;
				inet_aton(pchar, &dns_server[dns_server_count]);
				dns_server_count++;				
			}
		}
		fclose(fp);
	}
	else
	{
		//diag_printf("%s-->%d,open file /etc/resolv.conf fail!\n",__FUNCTION__,__LINE__);
		dns_server_count=0;
	}
}
/* main -- dnsproxy main function
 */
int
cyg_dns_proxy_daemon(int argc, char *argv[])
{
	struct sockaddr_in addr;
	int maxsock;
	fd_set fdmask, fds;
	struct timeval     tout;
	int ret, i;
		
	listenat = strdup("0.0.0.0");
	/* Create and bind query socket */
	if ((sock_query = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		fatal("unable to create socket: %s", strerror(errno));
#ifdef 	ECOS_DBG_STAT
	dbg_stat_add(dbg_dns_index, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_addr.s_addr = inet_addr(listenat);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	if (bind(sock_query, (struct sockaddr *)&addr, sizeof(addr)) != 0)
		fatal("unable to bind socket: %s", strerror(errno));

	
	for (i = 0; i<MAX_DNS_SERVER_NUM; i++) 
	{
		memset(&dns_srv[i], 0, sizeof(dns_srv[i]));
		memset(&dns_server[i], 0, sizeof(dns_server[i]));
	
		if ((dns_srv[i].sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
		{
			fatal("unable to create socket: %s", strerror(errno));
			continue;
		}
		dns_srv[i].addr.sin_family = AF_INET;
		//dns_srv[i].addr.sin_addr.s_addr=dns_server[i].s_addr;		
		dns_srv[i].addr.sin_port   = htons(port);		
	}

	cyg_mutex_init(&dnsproxy_mutex);

	/* Create and bind answer socket */
	
	
RESTART:
	diag_printf("%s-->%d,DNSPROXY Start......\n",__FUNCTION__,__LINE__);
	parse_resolv_file_to_array();	

	for (i = 0; i<dns_server_count; i++) 
	{
		//dns_srv[i].addr.sin_family = AF_INET;
		dns_srv[i].addr.sin_addr.s_addr=dns_server[i].s_addr;		
		//dns_srv[i].addr.sin_port   = htons(port);		
	}
	
	FD_ZERO(&fdmask);
	FD_SET(sock_query,   &fdmask);
	maxsock=sock_query;	

	for(i = 0; i<dns_server_count; i++) 
	{
		if(dns_srv[i].sock>0 && dns_srv[i].addr.sin_addr.s_addr>0)
		{
			if (maxsock < dns_srv[i].sock) 
				maxsock = dns_srv[i].sock;
			FD_SET(dns_srv[i].sock, &fdmask);
		}
	}
	maxsock++;
	while(1) {
		if(dns_proxy_restart_flag>0 || dns_proxy_quitting>0)	
		{	
			break;	
		}
		tout.tv_sec	= 1; /* 1 second */
		tout.tv_usec = 0;
		//fds = fdmask;
		FD_COPY(&fdmask,&fds);
		ret = select(maxsock, &fds, 0, 0, &tout);
		/* Handle errors */
	   	if (ret < 0) {
	    	fatal("select returned %s", strerror(errno));			
	    	continue;
		}
		else if (ret == 0) {
	    	continue;  /* nothing to do */
		}
		
		/* Fill sockaddr_in structs for both servers */
		if (FD_ISSET(sock_query, &fds)) 
		{					
	   		do_query(sock_query);						
		}
		
		for(i=0; i<dns_server_count; i++)
		{
			if(FD_ISSET(dns_srv[i].sock,  &fds))
			{						
				do_answer(dns_srv[i].sock);
			}
		}		
	}	
	

	if(dns_proxy_restart_flag>0 || dns_proxy_quitting>0)
	{
		free_dns_hash_table();
		
		if(dns_proxy_restart_flag>0)
		{
			dns_proxy_restart_flag=0;
			goto RESTART;
		}

		if(dns_proxy_quitting>0)
		{			
			cyg_callout_stop(&req_timer);
		}		
	}	
	
	for(i=0; i<MAX_DNS_SERVER_NUM; i++)
	{
		if(dns_srv[i].sock>0)
			close(dns_srv[i].sock);
	}	
	
	cyg_mutex_destroy(&dnsproxy_mutex);
	close(sock_query);
	
	if(dns_proxy_quitting>0)
		dns_proxy_quitting=0;
	
	return 0;
}

static cyg_uint8 cyg_dns_proxy_init=0;

cyg_thread   cyg_dns_thread_object;
cyg_handle_t cyg_dns_thread_handle;
cyg_uint8    cyg_dns_thread_stack[CYGNUM_DNS_PROXY_THREADOPT_STACKSIZE];

int get_dnsproxy_status()
{
	return cyg_dns_proxy_init;
}

#ifdef HAVE_SYSTEM_REINIT
void kill_dnsproxy()
{
	if(cyg_dns_proxy_init)
	{
		diag_printf("%s %d\n",__FUNCTION__,__LINE__);
		dns_proxy_quitting = 1;
		while(dns_proxy_quitting)
			cyg_thread_delay(5);
		
		cyg_thread_kill(cyg_dns_thread_handle);
		cyg_thread_delete(cyg_dns_thread_handle);
		cyg_dns_proxy_init =0;
		dns_proxy_restart_flag=0;
	}
}
#endif

void
cyg_dns_proxy_start(void)
{
    if (cyg_dns_proxy_init)
        return;
    cyg_dns_proxy_init = 1;
	
    memset(cyg_dns_thread_stack,0,CYGNUM_DNS_PROXY_THREADOPT_STACKSIZE);
    cyg_thread_create(CYGNUM_DNS_PROXY_THREADOPT_PRIORITY,
                      cyg_dns_proxy_daemon,
                      (cyg_addrword_t)0,
                      "DNSproxy Thread",
                      (void *)cyg_dns_thread_stack,
                      CYGNUM_DNS_PROXY_THREADOPT_STACKSIZE,
                      &cyg_dns_thread_handle,
                      &cyg_dns_thread_object);
    cyg_thread_resume(cyg_dns_thread_handle);
	// start the system timer
	cyg_callout_init(&req_timer);
	cyg_callout_reset(&req_timer, recursive_timeout, request_timeout, NULL);
	
}

