/*
 * DNSMASQ protocol.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dnsmasq_proto.c,v 1.5 2010-07-26 01:55:34 Exp $
 */

#include <dnsmasq.h>
#include <ctype.h>
#include <arpa/inet.h>

#define DNS_DIRECT_EN 1//roy add,2010/10/11

#ifdef DNS_DIRECT_EN//roy add,2010/10/11
#include <router_net.h>
#endif

//在登录界面点不再提示时,这个值为1
int dns_redirect_disable = 0;
int dns_redirect_web = 0;

extern int getRedirectPPPOEStatus();
extern int getRedirectWlsecStatus();
extern char g_Pass[64];


/* Pares query domian name */
static unsigned char *
dnsmasq_parse_name(DNSHEADER *dnsheader, unsigned char *p,
	unsigned int plen, char *name)
{
	unsigned char *end = (unsigned char *)dnsheader + plen;
	char *cp = name;
	char c;
	unsigned char *qtype = NULL;
	unsigned int len;
	unsigned int alias = 0;
	unsigned int i;

	while ((len = *p++)) {
		if ((len & 0xc0) != 0xc0) {
			if (cp + len >= name + MAXDNAME-1)
				return NULL;
			if (p + len >= end)
				return NULL;

			for (i = 0; i < len; i++) {
				c = p[i];
				if (!isalnum(c) && strchr("-/_.", c) == NULL)
					return NULL;
			}

			/* Copy to name */
			memcpy(cp, p, len);

			p += len;
			cp += len;

			/* Add '.' to cat next name */
			*cp++ = '.';
		}
		else  {
			/* pointer */
			if (p >= end - 1)
				return NULL;

			/* get offset */
			len = (len & 0x3f) << 8;
			len |= *p++;
			if (len >= (unsigned int)plen)
				return NULL;

			/* Save the pointer of first hop to back */
			if (qtype == NULL)
				qtype = p;

			/* Maximum 255 alias jumps */
			alias++;
			if (alias > 255)
				return NULL;

			p = (unsigned char *)dnsheader + len;
		}

		if (p >= end)
			return NULL;
	}

	/* Terminate the last '.' */
	if (cp != name)
		*--cp = 0;

	/* Check qtype position */
	if (qtype)
		return qtype;

	return p;
}

/* Parse the query content */
int dnsmasq_parse_request(DNSHEADER *dnsheader, unsigned int qlen, char *name)
{
	unsigned char *p = (unsigned char *)(dnsheader+1);
	int qtype, qclass;

	if (ntohs(dnsheader->qdcount) != 1 || dnsheader->opcode != QUERY)
		return UNKNOW_QUERY;

	/* Check dns name */
	p = dnsmasq_parse_name(dnsheader, p, qlen, name);
	if (p == NULL)
		return UNKNOW_QUERY;

	/* Check IPv4 and HOST/ANY address request */
	GETSHORT(qtype, p);
	GETSHORT(qclass, p);
	if (qclass == C_IN && (qtype == T_A || qtype == T_ANY))
		return GOOD_QUERY;

	if(qtype == C_IPV6_IN)
		return UNKNOW_QUERY;

	return UNKNOW_TYPE;
}

static unsigned char *
skip_name(unsigned char *name, unsigned char *end)
{
	/* Loop until end, zero length */
	while (1) {
		if (name >= end)
			return NULL;

		/* name alias pointer */
		if (((*name) & 0xc0) == 0xc0) {
			name += 2;
			break;
		}
		else if (*name) {
			/* Goto next segment */
			name += (*name) + 1;
		}
		else {
			/* End, skip zero length itself. */
			name++;
			break;
		}
	}

	return name;
}

/* Find answer position. */
static unsigned char *
dnsmasq_find_answer_position(DNSHEADER *dnsheader, unsigned int plen)
{
	int i;
	int count = ntohs(dnsheader->qdcount);
	unsigned char *p = (unsigned char *)(dnsheader+1);
	unsigned char *end = p + plen;

	for (i = 0; i < count; i++) {
		/* Get next query */
		p = skip_name(p, end);
		if (p == NULL)
			return NULL;	/* Not a valid answer */

		/* Skip type and class */
		p += 4;
	}

	if (p > end)
		return NULL;

	/* End queries, must be answers */
	return p;
}

int
dnsmasq_find_answer(DNSHEADER *dnsheader, unsigned int plen, DNSMASQ_ENTRY *entry)
{
	int i;
	int count = ntohs(dnsheader->ancount);
	unsigned char *p = dnsmasq_find_answer_position(dnsheader, plen);
	unsigned char *end = p + plen;
	int qtype, qclass;
	unsigned long ttl;
	int len;
	char *inp;

	if (!p)
		return -1;

	for (i = 0; i < count; i++) {
		if (p >= end)
			return -1;

		/* Skip name, usually is name pointer */
		p = skip_name(p, end);
		if (p == NULL)
			return -1;

		/* Get qtype and qclass */
		GETSHORT(qtype, p);
		GETSHORT(qclass, p);

		/* Get TTL */
		GETLONG(ttl, p);

		/* Get data length and addr */
		GETSHORT(len, p);
		inp = (char *)p;

		/* Skip datalen and addr */
		p += len;

		/* Is IPV4 and HOST and data length 4 */
		if (qtype == T_A && qclass == C_IN && len == 4) {
				entry->expire = ttl;
				entry->expire += time(0);
				memcpy(&entry->answer_addr, inp, 4);
				return 0;
		}
	}

	return -1;
}

/* Build a reply packet */
static int
dnsmasq_build_reply(DNSHEADER *dnsheader, unsigned int qlen,
	struct in_addr addr, unsigned short flags)
{
	unsigned short temp = sizeof(DNSHEADER) | 0xc000;
	unsigned long ttl = 0;
	unsigned char *p = dnsmasq_find_answer_position(dnsheader, qlen);

	/* Stop when answer pointer is not correct */
	if (p == NULL)
		return -1;

	/* Rebuild response packet */
	dnsheader->qr = 1;		/* response */
	dnsheader->aa = 0;		/* authoritive */
	dnsheader->ra = 1;		/* recursion if available */
	dnsheader->tc = 0;		/* not truncated */
	dnsheader->nscount = htons(0);
	dnsheader->arcount = htons(0);
	dnsheader->ancount = htons(0);	/* no answers unless changed below */

	switch (flags) {
	case FLAG_IPV4:
		/* we know the address */
		dnsheader->rcode = NOERROR;
		dnsheader->ancount = htons(1);
		dnsheader->aa = 1;

		PUTSHORT(temp, p);
		PUTSHORT(T_A, p);
		PUTSHORT(C_IN, p);
		PUTLONG(ttl, p);
		PUTSHORT(sizeof(addr), p);
		memcpy(p, &addr, sizeof(addr));
		p += sizeof(addr);
		break;

	case FLAG_NOERR:
		/* empty domain */
		dnsheader->rcode = NOERROR;
		break;

	case FLAG_NEG:
		/* couldn't get memory */
		dnsheader->rcode = SERVFAIL;
		break;

	default:
		/* nowhere to forward to */
		dnsheader->rcode = REFUSED;
		break;
	}

	return p - (unsigned char *)dnsheader;
}

/* Create an nid */
static unsigned short
getnid(void)
{
	unsigned short nid = 0;
	DNSMASQ_ENTRY *entry;

	while (1) {
		nid = htons((unsigned short)(rand() >> 15));
		if (nid) {
			/* Make sure nobody use this nid */
			entry = dnsmasq_forward_find_by_nid(nid);
			if (entry == 0)
				break;
		}
	}

	return nid;
}

int check_webpage()
{
	if(/*getRedirectPPPOEStatus()==1 || */getRedirectWlsecStatus()==1)
		return 1;
	else
		return 0;
}

int check_dnsname(char *dns)
{
	char str[64] = {0};
	char head[32] = {0},/* middle[32] ={0}, */tail[32] = {0};
	char *p=NULL,*p1=NULL,*p2=NULL;
	int n = 0, ch ='.';
	
	if(strlen(dns)==0 || strlen(dns) >=64)
		return 0;
		
	strcpy(str,dns);
	p=str;
	
	while(*p!='\0'){
		if(*p == ch){
			n++;
		}
		p++	;
	}
	/*
	 * only consider www.xxx.com, revise 2013.07.20
	 */
	if (n != 2/* && n != 1*/)
		return 0;

	p1=p2=p=str;
	
	if(n == 1){
		p =strchr(str,ch);
		if((p-p1)==0 || *(p+1) =='\0')
			return 0;
		
		strncpy(head,p1,p-p1);
		strcpy(tail,p+1);
		
		if(strcmp(tail,"com")!=0 || strcmp(head,"www")==0)
			return 0;
		else{
			printf("1 point dns=%s\n",str);
			return 1;
		}
	}else{
		p=strchr(str,ch);
		if((p-p1)==0 || *(p+2)=='\0')
			return 0;

		strncpy(head,p1,p-p1);
		
		p2=p;
		p=strchr(p+1,ch);
		if((p-p2)==1 || *(p+1) == '\0')
			return 0;
		
	//	strncpy(middle,p2,p-p2-1);
		strcpy(tail,p+1);

		if(strcmp(head,"www")!=0 || strcmp(tail,"com")!=0)
			return 0;
		else{
			printf("2 point dns=%s\n",str);
			return 1;
		}
	}
	
	return 0;
}


/* Process dns server replay messages. */
void
dnsmasq_process_reply(DNSHEADER *dnsheader, int plen, int fd)
{
	struct in_addr	addr = {0};
	unsigned short	flags = 0;

	DNSMASQ_ENTRY *entry;

	if (dnsheader->qr && plen >= (int)sizeof(DNSHEADER)) {
		/* 
		 * There must have a matched cache(match id)
		 * in cache list, replace the original id
		 */
		entry = dnsmasq_forward_find_by_nid(dnsheader->id);
		if (entry) {
			/* Detach from the forward list anyway */
			dnsmasq_forward_del(entry);

			/*
			 * This dns server is most likely to be good,
			 * set it to be the active_server.
			 */
			if (dnsheader->rcode == NOERROR || dnsheader->rcode == NXDOMAIN) {
				dnsmasq_set_last_reply_server(entry->forwardto);
			}

			/* replace to original id */
			dnsheader->id = entry->oid;

			/* 
			 * pxy add 2013.06.06
			 * 
			 */
			if(0)/*add by lqz just to remove for donot dns_redirect when don not have the wl password*/
			{
				if(!strcmp(g_Pass,"") && check_webpage()==1){
					/*if user open browser */
					if(dns_redirect_web != -1 && check_dnsname(entry->dnsname)==1)
					{
						/*it need to jump to the router's web page*/
						dns_redirect_web = 1;
						flags = FLAG_IPV4;
						addr.s_addr = SYS_lan_ip;
						plen = dnsmasq_build_reply(dnsheader, (unsigned int)plen, addr, flags);				
					}
				}
			}
		//	printf("entry->dnsname=%s\n",entry->dnsname);
		//	printf("entry->answer_addr=%s\n",inet_ntoa(entry->answer_addr));
		//	printf("send to the requester=%s\n",NSTR((&entry->queryaddr)->sin_addr));
		
			/* send to the requester */
			sendto(entry->fd, dnsheader, plen, 0,
				(struct sockaddr *)&entry->queryaddr, sizeof(entry->queryaddr));

			/* entry answer */
			if (dnsmasq_find_answer(dnsheader, plen, entry) == 0 &&
				entry->flag == GOOD_QUERY) {
				dnsmasq_cache_add(entry);
			}
			else {
				dnsmasq_free(entry);
			}
		}
	}

	return;
}

/* Process dnsmasq query messages. */
void
dnsmasq_process_query
(
	DNSHEADER *dnsheader,
	int plen,
	int fd,
	struct in_addr ipaddr,
	struct sockaddr_in *queryaddr
)
{
	DNSMASQ_HOST	*host;
	DNSMASQ_SERVER	*reply_server;
	DNSMASQ_ENTRY	*entry = 0;
	struct in_addr	addr = {0};
	unsigned short	flags = 0;
	unsigned short	parse_result = 0;
	char dname[MAXDNAME];
	time_t now = time(0);

	/* Init to null string */
	dname[0] = 0;

	/* Make sure this is a query packet */
	if (dnsheader->qr != 0)
		return;

	/*
	 * return UNKNOW_QUERY: dnsname is not recognizable.
	 *			forward it, not find by addr, not cache it
	 *
	 * return GOOD_QUERY: 	dnsname is recognizable, type and class is valid.
	 *			check localhost,host,cache. forward it.
	 *
	 * return UNKNOW_TYPE: 	dnsname is recognizable, type and class is not valid.
	 *			forward it, not cache it
	 */
	parse_result = dnsmasq_parse_request(dnsheader, (unsigned int)plen, dname);

	//gong add for DN Login
	//hqw add for登陆跳转到主页开始为t.d
	#ifndef __CONFIG_AUTO_CONN__
	if(strcmp(dname,"tendawifi.com") == 0 || strcmp(dname,"www.tendawifi.com") == 0 )
		goto dns_redirect; 
	#endif
	/* 
	 * If it is a good request, that means we are
	 * able the trust the domain name requested,
	 * find the localhost and hosts firstly.
	 */
	if (parse_result == GOOD_QUERY)
	{
		char *localhost;

		localhost = dnsmasq_get_localhost();
		if (localhost[0] != 0 && strcasecmp(localhost, dname) == 0) {
			flags = FLAG_IPV4;
			addr = ipaddr;
		}
		else if ((host = dnsmasq_host_find(dname)) != NULL) {
			/*
			 * Find the nostname in the hosts table.
			 * If found, set flags and address.
			 */
			flags = FLAG_IPV4;
			addr = host->addr;
		}
		else {
			/*
			 * Find the cache list, if cache is not timed-out yet,
			 * return the ipaddr of the dnsname.
			 */
			if ((entry = (DNSMASQ_ENTRY *)dnsmasq_cache_find(dname)) != NULL) {
				flags = FLAG_IPV4;
				addr = entry->answer_addr;
			}
		}
	}

	/*
	 * The addr is not found, that is the localhost and hosts checking failed.
	 * So, we check the forwarding database for the next sequence.
	 */
	reply_server = dnsmasq_get_last_reply_server();

	if (addr.s_addr == 0 && reply_server != 0)
	{
		/* Search for any pending forwarding entry */
		if (parse_result != UNKNOW_QUERY)
			entry = dnsmasq_forward_find_by_addr(queryaddr->sin_addr, dname);
		else
			entry = 0;

		/*
		 * If the old entry is found, that means the previous
		 * request was not reply properly. Search for the next
		 * dns server.
		 */
		if (entry) {
			dnsmasq_forward_del(entry);

			/* Update the sockaddr_in for new peer port */
			entry->queryaddr = *queryaddr;
			entry->oid = dnsheader->id;

			/* Set dns server to next one */
			entry->forwardto = dnsmasq_server_next(entry->forwardto);

			/* Masquerade the input id to nid */
			dnsheader->id = entry->nid;
		}
		else {
			/* Allocate a new one */
			entry = dnsmasq_new();
			if (entry)
			{
				strcpy(entry->dnsname, dname);
				entry->queryaddr = *queryaddr;
				entry->oid = dnsheader->id;
				entry->nid = getnid();
				dnsheader->id = entry->nid;

				entry->fd = fd;
				entry->forwardto = reply_server;
			}
		}

		/*
		 * check for send errors here (no route to host)
		 * if we fail to send to all nameservers, send back an error
		 * packet straight away (helps modem users when offline).
		 */
		if (entry) {
			DNSMASQ_SERVER *first = entry->forwardto;

			entry->flag = parse_result;

			if (entry->flag == UNKNOW_QUERY)
				entry->expire = now + QUICK_TIMEOUT;
			else
				entry->expire = now + QUERY_TIMEOUT;

			/* Add to forward list */
			dnsmasq_forward_add(entry);

			/*
			 * Try to send to the dns server starting from
			 * the active server, until all are failed.
			 */
			do {
				struct sockaddr_in sin;

				memset(&sin, 0, sizeof(sin));
				sin.sin_len = sizeof(sin);
				sin.sin_family = AF_INET;
				sin.sin_port = htons(NAMESERVER_PORT);
				sin.sin_addr = entry->forwardto->addr;

				if (sendto(entry->forwardto->fd, (char *)dnsheader, plen, 0,
					(struct sockaddr *)&sin, sizeof(sin)) != -1) {
					/*
					 * Give another dns server a chance to
					 * serve the next request.
					 */
					reply_server = dnsmasq_server_next(entry->forwardto);
					dnsmasq_set_last_reply_server(reply_server);
					/*
					 * pxy discribe 2013.06.06 
					 * once wan get ip, then return
					 */
				//	printf("\t dname=%s \n",dname);
					return;
				}

				/* Go to the next dns server */
				entry->forwardto = dnsmasq_server_next(entry->forwardto);
			}
			while (entry->forwardto != first);

			/* could not send on, prepare to return */
			dnsheader->id = entry->oid;
			entry->nid = 0;
#ifdef DNS_DIRECT_EN//roy add,2010/10/11
			if(dns_redirect_disable == 0){
				/*WAN口断线或者有线却没有IP时会走到这里*/
				//dnsmasq_forward_del(entry);
				goto dns_redirect;
			}
#endif
			return;
		}
		else {
			/* Can't allocate dnsmasq entry */
			DNSMASQ_DBG("%s: %d, Can't allocate dnsmasq entry\n",__FUNCTION__,__LINE__);
			return;
		}
	}
//roy+++,当WAN口一次都没起来过时,reply_server为空,不会执行上面的if语句
#ifdef DNS_DIRECT_EN//roy add,2010/10/11
	if(! reply_server){
		if(dns_redirect_disable == 0){
			goto dns_redirect;
		}
	}
#endif
//+++	
	/* Should not reply if not found in cache,
	 * and dns servers not configured.
	 */
	if (addr.s_addr == 0)
		return;

	/* Send back to querist */
	plen = dnsmasq_build_reply(dnsheader, (unsigned int)plen, addr, flags);
	if (plen > 0) {
		sendto(fd, (char *)dnsheader, plen, 0,
			(struct sockaddr *)queryaddr, sizeof(struct sockaddr_in));
		DNSMASQ_DBG("%s: reply sendto %s\n",__FUNCTION__,NSTR(queryaddr->sin_addr));
	}
	return;
#ifdef DNS_DIRECT_EN//roy add,2010/10/11
dns_redirect:
	DNSMASQ_DBG("%s_%d: dns_redirect\n",__FUNCTION__,__LINE__);
	if(dns_redirect_disable == 0){
		/* Send back to querist */
		flags = FLAG_IPV4;
		addr.s_addr = SYS_lan_ip;
		plen = dnsmasq_build_reply(dnsheader, (unsigned int)plen, addr, flags);
		if (plen > 0) {
			sendto(fd, (char *)dnsheader, plen, 0,
				(struct sockaddr *)queryaddr, sizeof(struct sockaddr_in));
		}
	}
	return;
#endif
}
