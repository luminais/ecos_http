/* $Id: upnpevents.c,v 1.2 2009/04/03 11:44:40 bradhuang Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2008 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#ifdef __ECOS
#include <network.h>
#endif

#include <stdio.h>
#include <string.h>
#ifndef __ECOS
#include <syslog.h>
#endif 
#include <sys/queue.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#ifndef __ECOS
#include <sys/sysinfo.h> 
#endif
//#include "config.h"
#include "upnpevents.h"
#include "miniupnpdpath.h"
#include "upnpglobalvars.h"
#include "upnpdescgen.h"
#ifdef 	ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif

#ifdef	ECOS_DBG_STAT
extern int	dbg_igd_index;
extern int	dbg_igd_index2;

#endif

#ifdef ENABLE_EVENTS
/*enum subscriber_service_enum {
 EWanCFG = 1,
 EWanIPC,
 EL3F
};*/

/* stuctures definitions */
struct subscriber {
	LIST_ENTRY(subscriber) entries;
	struct upnp_event_notify * notify;
	time_t timeout;
	uint32_t seq;
	/*enum { EWanCFG = 1, EWanIPC, EL3F } service;*/
	enum subscriber_service_enum service;
	char uuid[42];
	char callback[];
};

struct upnp_event_notify {
	LIST_ENTRY(upnp_event_notify) entries;
    int s;  /* socket */
    enum subscriber_service_state_enum state;
    struct subscriber * sub;
    char * buffer;
    int buffersize;
	int tosend;
    int sent;
	const char * path;
	char addrstr[16];
	char portstr[8];
};

/* prototypes */
static void
upnp_event_create_notify(struct subscriber * sub);

/* Subscriber list */
LIST_HEAD(listhead, subscriber) subscriberlist = { NULL };

/* notify list */
LIST_HEAD(listheadnotif, upnp_event_notify) notifylist = { NULL };

/* create a new subscriber */
static struct subscriber *
newSubscriber(const char * eventurl, const char * callback, int callbacklen)
{
	struct subscriber * tmp;
	if(!eventurl || !callback || !callbacklen)
		return NULL;
	tmp = calloc(1, sizeof(struct subscriber)+callbacklen+1);
	if(!tmp)
		return NULL;
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, sizeof(struct subscriber));
#endif

	if(strcmp(eventurl, WANCFG_EVENTURL)==0)
		tmp->service = EWanCFG;
	else if(strcmp(eventurl, WANIPC_EVENTURL)==0)
		tmp->service = EWanIPC;
//	
//	else if(strcmp(eventurl, L3F_EVENTURL)==0)
//		tmp->service = EL3F;
//
	else {
		free(tmp);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct subscriber));
#endif
		return NULL;
	}
	memcpy(tmp->callback, callback, callbacklen);
	tmp->callback[callbacklen] = '\0';
	/* make a dummy uuid */
	/* TODO: improve that */
	strncpy(tmp->uuid, uuidvalue, sizeof(tmp->uuid));
	tmp->uuid[sizeof(tmp->uuid)-1] = '\0';
	snprintf(tmp->uuid+37, 5, "%04lx", rand() & 0xffff);
	return tmp;
}

/* creates a new subscriber and adds it to the subscriber list
 * also initiate 1st notify */
const char *
upnpevents_addSubscriber(const char * eventurl,
                         const char * callback, int callbacklen,
                         int timeout)
{
	struct subscriber * tmp;
#ifndef __ECOS
	struct sysinfo system_info;
#endif 
	/*static char uuid[42];*/
	/* "uuid:00000000-0000-0000-0000-000000000000"; 5+36+1=42bytes */
	//syslog(LOG_DEBUG, "addSubscriber(%s, %.*s, %d)",  eventurl, callbacklen, callback, timeout);
	//printf("addSubscriber(%s, %.*s, %d)\n", eventurl, callbacklen, callback, timeout);
	/*strncpy(uuid, uuidvalue, sizeof(uuid));
	uuid[sizeof(uuid)-1] = '\0';*/
	tmp = newSubscriber(eventurl, callback, callbacklen);
	if(!tmp)
		return NULL;
	if(timeout && timeout != 0xFFFFFFFF){
#ifndef __ECOS
		sysinfo(&system_info);
		tmp->timeout = system_info.uptime + timeout;
#else
		tmp->timeout = time(NULL) + timeout;
#endif
	}else{
		tmp->timeout = 0xFFFFFFFF;
	}
	LIST_INSERT_HEAD(&subscriberlist, tmp, entries);
	upnp_event_create_notify(tmp);
	return tmp->uuid;
}

/* renew a subscription (update the timeout) */
int
renewSubscription(const char * sid, int sidlen, int timeout)
{
	struct subscriber * sub;
#ifndef __ECOS
	struct sysinfo system_info;
#endif
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		if(memcmp(sid, sub->uuid, 41)) {
			if(sub->timeout != 0xFFFFFFFF){
#ifndef __ECOS
				sysinfo(&system_info);
				sub->timeout = (timeout ? system_info.uptime + timeout : 0);
#else
				sub->timeout = (timeout ? (time(NULL) + timeout) : 0);
#endif
			}
			return 0;
		}
	}
	return -1;
}

int
upnpevents_removeSubscriber(const char * sid, int sidlen)
{
	struct subscriber * sub;
	if(!sid)
		return -1;
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		if(memcmp(sid, sub->uuid, 41)) {
			if(sub->notify) {
				sub->notify->sub = NULL;
			}
			LIST_REMOVE(sub, entries);
#ifdef	ECOS_DBG_STAT
			dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct subscriber));
#endif

			free(sub);
			return 0;
		}
	}
	return -1;
}




void upnpevents_removeSubscriber_shutdown(void)
{
	struct subscriber *sub,*next;
	next = NULL;
	sub = subscriberlist.lh_first;
	while(sub != NULL){
		next = sub->entries.le_next;
		if(sub->notify) {
			sub->notify->sub = NULL;
		}
		LIST_REMOVE(sub, entries);
		free(sub);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct subscriber));
#endif
		sub = next;
	}

}


/* notifies all subscriber of a number of port mapping change
 * or external ip address change */
void
upnp_event_var_change_notify(enum subscriber_service_enum service)
{
	struct subscriber * sub;
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		if(sub->service == service && sub->notify == NULL)
			upnp_event_create_notify(sub);
	}
}

/* create and add the notify object to the list */
static void
upnp_event_create_notify(struct subscriber * sub)
{
	struct upnp_event_notify * obj;
	int flags;
	obj = calloc(1, sizeof(struct upnp_event_notify));
	if(!obj) {
		//syslog(LOG_ERR, "%s: calloc(): %m", "upnp_event_create_notify");
		return;
	}
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, sizeof(struct upnp_event_notify));
#endif
	obj->sub = sub;
	obj->state = ECreated;
	obj->s = socket(PF_INET, SOCK_STREAM, 0);
	if(obj->s<0) {
		//syslog(LOG_ERR, "%s: socket(): %m", "upnp_event_create_notify");
		goto error;
	}
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index2, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif
#if 0
	if((flags = fcntl(obj->s, F_GETFL, 0)) < 0) {
		//syslog(LOG_ERR, "%s: fcntl(..F_GETFL..): %m","upnp_event_create_notify");
		goto error;
	}
	if(fcntl(obj->s, F_SETFL, flags | O_NONBLOCK) < 0) {
		//syslog(LOG_ERR, "%s: fcntl(..F_SETFL..): %m", "upnp_event_create_notify");
		goto error;
	}
#endif
	if(sub)
		sub->notify = obj;
	else
		goto error;
	LIST_INSERT_HEAD(&notifylist, obj, entries);
	return;
error:
	if(obj->s >= 0){
		if(close(obj->s)==0){
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index2, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
			obj->s = -1;
		}
	}
	free(obj);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct upnp_event_notify));
#endif

}

static void
upnp_event_notify_connect(struct upnp_event_notify * obj)
{
	int i;
	const char * p;
	unsigned short port;
	struct sockaddr_in addr;
	if(!obj)
		return;
	memset(&addr, 0, sizeof(addr));
	i = 0;
	if(obj->sub == NULL) {
		obj->state = EError;
		return;
	}
	p = obj->sub->callback;
	p += 7;	/* http:// */
	while(*p != '/' && *p != ':' && *p)
		obj->addrstr[i++] = *(p++);
	obj->addrstr[i] = '\0';
	if(*p == ':') {
		obj->portstr[0] = *p;
		i = 1;
		p++;
		port = (unsigned short)atoi(p);
		while(*p != '/' && *p) {
			if(i<7) obj->portstr[i++] = *p;
			p++;
		}
		obj->portstr[i] = 0;
	} else {
		port = 80;
		obj->portstr[0] = '\0';
	}
	obj->path = p;
	if(!is_ipv4str(obj->addrstr)){
		obj->state = EError;
		return;
	}
	addr.sin_family = AF_INET;
	inet_aton(obj->addrstr, &addr.sin_addr);
	addr.sin_port = htons(port);
	//syslog(LOG_DEBUG, "%s: '%s' %hu '%s'", "upnp_event_notify_connect",obj->addrstr, port, obj->path);
	obj->state = EConnecting;
	if(connect(obj->s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		if(errno != EINPROGRESS && errno != EWOULDBLOCK) {
			//syslog(LOG_ERR, "%s: connect(): %m", "upnp_event_notify_connect");
			obj->state = EError;
		}
	}
}

static void upnp_event_prepare(struct upnp_event_notify * obj)
{
	static const char notifymsg[] = 
		"NOTIFY %s HTTP/1.1\r\n"
		"Host: %s%s\r\n"
		"Content-Type: text/xml\r\n"
		"Content-Length: %d\r\n"
		"NT: upnp:event\r\n"
		"NTS: upnp:propchange\r\n"
		"SID: %s\r\n"
		"SEQ: %u\r\n"
		"Connection: close\r\n"
		"Cache-Control: no-cache\r\n"
		"\r\n"
		"%.*s\r\n";
	char * xml;
	int l;
	if(obj->sub == NULL) {
		obj->state = EError;
		return;
	}
	switch(obj->sub->service) {
	case EWanCFG:
		xml = getVarsWANCfg(&l);
		break;
	case EWanIPC:
		xml = getVarsWANIPCn(&l);
		break;
//		
//	case EL3F:
//		xml = getVarsL3F(&l);
//		break;
//
	default:
		xml = NULL;
		l = 0;
	}
	obj->buffersize = 1024;
	obj->buffer = malloc(obj->buffersize);
	if(!obj->buffer) {
		obj->state = EError;
		if(xml){
			free(xml);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 512);
#endif

			xml=NULL;
		}
		return;
	}
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, obj->buffersize);
#endif
	obj->tosend = snprintf(obj->buffer, obj->buffersize, notifymsg,
	                       obj->path, obj->addrstr, obj->portstr, l+2,
	                       obj->sub->uuid, obj->sub->seq,
	                       l, xml);
	if(xml) {
		free(xml);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 512);
#endif
		xml = NULL;
	}
	obj->state = ESending;
}

static void upnp_event_send(struct upnp_event_notify * obj)
{
	int i;
	i = send(obj->s, obj->buffer + obj->sent, obj->tosend - obj->sent, 0);
	if(i<0) {
		//syslog(LOG_NOTICE, "%s: send(): %m", "upnp_event_send");
		obj->state = EError;
		return;
	}
	else if(i != (obj->tosend - obj->sent))
		;//syslog(LOG_NOTICE, "%s: %d bytes send out of %d","upnp_event_send", i, obj->tosend - obj->sent);
	obj->sent += i;
	if(obj->sent == obj->tosend)
		obj->state = EWaitingForResponse;
}

static void upnp_event_recv(struct upnp_event_notify * obj)
{
	int n;
	n = recv(obj->s, obj->buffer, obj->buffersize, 0);
	if(n<0) {
		//syslog(LOG_ERR, "%s: recv(): %m", "upnp_event_recv");
		obj->state = EError;
		return;
	}
	//syslog(LOG_DEBUG, "%s: (%dbytes) %.*s", "upnp_event_recv",n, n, obj->buffer);
	obj->state = EFinished;
	if(obj->sub)
		obj->sub->seq++;
}

static void
upnp_event_process_notify(struct upnp_event_notify * obj)
{
	switch(obj->state) {
	case EConnecting:
		/* now connected or failed to connect */
		upnp_event_prepare(obj);
		upnp_event_send(obj);
		break;
	case ESending:
		upnp_event_send(obj);
		break;
	case EWaitingForResponse:
		upnp_event_recv(obj);
		break;
	case EFinished:
		
		if(close(obj->s)==0){
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index2, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		obj->s = -1;
		}
		break;
	case EError:
		if(close(obj ->s)== 0){
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index2, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		obj ->s = -1;
		}
	default:
		;//syslog(LOG_ERR, "upnp_event_process_notify: unknown state");
	}
}

void upnpevents_selectfds(fd_set *readset, fd_set *writeset, int * max_fd)
{
	struct upnp_event_notify * obj;
	
	for(obj = notifylist.lh_first; obj != NULL; obj = obj->entries.le_next) {
		//syslog(LOG_DEBUG, "upnpevents_selectfds: %p %d %d",obj, obj->state, obj->s);
		if(obj->s >= 0) {
			switch(obj->state) {
			case ECreated:
				upnp_event_notify_connect(obj);
				if(obj->state != EConnecting)
					break;
			case EConnecting:
			case ESending:
				FD_SET(obj->s, writeset);
				if(obj->s > *max_fd)
					*max_fd = obj->s;
				break;
			case EWaitingForResponse:
				FD_SET(obj->s, readset);
				if(obj->s > *max_fd)
					*max_fd = obj->s;
				break;
			default:
				obj->state = EError;
				break;
			}
		}
	}
	
}

void upnpevents_processfds(fd_set *readset, fd_set *writeset)
{
	struct upnp_event_notify * obj;
	struct upnp_event_notify * next;
	struct subscriber * sub;
	struct subscriber * subnext;
	time_t curtime;
#ifndef __ECOS
	struct sysinfo system_info;
#endif
	for(obj = notifylist.lh_first; obj != NULL; obj = obj->entries.le_next) {
		//printf("%s: %p %d %d %d %d","upnpevents_processfds\n", obj, obj->state, obj->s,FD_ISSET(obj->s, readset), FD_ISSET(obj->s, writeset));
		if(obj->s >= 0) {
			if(FD_ISSET(obj->s, readset) || FD_ISSET(obj->s, writeset)){
				upnp_event_process_notify(obj);
				}
		}
	}     
	obj = notifylist.lh_first;
	while(obj != NULL) {
		next = obj->entries.le_next;
		if(obj->state == EError || obj->state == EFinished) {
			if(obj->s >= 0) {
				if(close(obj->s) == 0){
#ifdef	ECOS_DBG_STAT
				dbg_stat_add(dbg_igd_index2, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
				obj->s = -1;
				}
			}
			if(obj->sub)
				obj->sub->notify = NULL;
			/* remove also the subscriber from the list if there was an error */
			if(obj->state == EError && obj->sub) {
				LIST_REMOVE(obj->sub, entries);
				free(obj->sub);
#ifdef	ECOS_DBG_STAT
				dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,sizeof(struct subscriber));
#endif
			}
			if(obj->buffer) {
				free(obj->buffer);
#ifdef	ECOS_DBG_STAT
				dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,obj->buffersize);
#endif
			}
			LIST_REMOVE(obj, entries);
			free(obj);
#ifdef	ECOS_DBG_STAT
			dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,sizeof(struct upnp_event_notify));
#endif
		}
		obj = next;
	}
	/* remove timeouted subscribers */
#ifndef __ECOS
	sysinfo(&system_info);
	curtime =system_info.uptime;
#else
	time(&curtime);
#endif
	for(sub = subscriberlist.lh_first; sub != NULL; ) {
		subnext = sub->entries.le_next;
		if(sub->timeout != 0xFFFFFFFF){
		if(sub->timeout && curtime > sub->timeout && sub->notify == NULL) {
				//printf("remove timeouted subscribers:sid=%s\n", sub->uuid);
			LIST_REMOVE(sub, entries);
			free(sub);
#ifdef	ECOS_DBG_STAT
			dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL,sizeof(struct upnp_event_notify));
#endif
		}
		}
		sub = subnext;
	}
}

#ifdef USE_MINIUPNPDCTL
void write_events_details(int s) {
	int n;
	char buff[80];
	struct upnp_event_notify * obj;
	struct subscriber * sub;
	write(s, "Events details\n", 15);
	for(obj = notifylist.lh_first; obj != NULL; obj = obj->entries.le_next) {
		n = snprintf(buff, sizeof(buff), " %p sub=%p state=%d s=%d\n",
		             obj, obj->sub, obj->state, obj->s);
		write(s, buff, n);
	}
	write(s, "Subscribers :\n", 14);
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		n = snprintf(buff, sizeof(buff), " %p timeout=%d seq=%u service=%d\n",
		             sub, sub->timeout, sub->seq, sub->service);
		write(s, buff, n);
		n = snprintf(buff, sizeof(buff), "   notify=%p %s\n",
		             sub->notify, sub->uuid);
		write(s, buff, n);
		n = snprintf(buff, sizeof(buff), "   %s\n",
		             sub->callback);
		write(s, buff, n);
	}
}
#endif

#endif

