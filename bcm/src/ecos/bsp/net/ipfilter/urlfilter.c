/*
 * URL filter for eCos platform
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: urlfilter.c,v 1.2 2010-07-30 06:38:09 Exp $
 *
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/malloc.h>
#include <ip_compat.h>
#include <urlf.h>

static struct urlfilter *urlfil_list = 0;
static int urlfil_dafault_action = 0;
static int check_https = CHECK_HTTPS;


extern int (*urlfilter_checkp)(struct ifnet *ifp, char *eh, struct mbuf *m);

#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	extern int url_filter_mode;
#else
	static char cmpstr[1500];
#endif

/* Return the line with CRLF */
inline char *
get_line(char *data, int *len)
{
    register int i;
    register char c;

    for (i=0; i < *len; ++i )
    {
		c = data[i];
		if (c == '\n' || c == '\r')
		{
	    	++i;
	    	if ((c == '\r') && (i < *len) && (data[i] == '\n'))
	    	{
				++i;
			}

			*len -= i ;
	   		return &(data[i]);
	    }
	}

    return (char*)0;
}


#ifdef __CONFIG_TENDA_HTTPD_NORMAL__

#define HTTP_HOST 			"Host: "
#define HTTP_REFERER 		"Referer: "

#define URL_MAX_LEN 	256
/*
	https过滤，通过匹配 handshake握手包[Client Hello]  进行过滤
	返回值0  允许；1  拦截
*/
int https_url_match( struct ip *ip , int length)
{
	char *data;
	int data_len;
	struct tcphdr *tcp;
	int i = 0 ;
	int url_len = 0 ;
	struct parentCtl_devices_list  *ptr;
	struct parentCtl_url_list  *uptr;
	int find_sta = 0;
	
	tcp = (struct tcphdr *)((char *)ip + (ip->ip_hl << 2));
	data = (char *)tcp + (tcp->th_off << 2);
	data_len = length - (tcp->th_off << 2) - (ip->ip_hl << 2);
	if (data_len < 10)
	{
		return URLF_PASS;
	}
	/*0x16 https handshake*//*0x01 Client Hello */
	if(data[0] != 0x16 || data[5] != 1)
	{
		return URLF_PASS ;
	}

	for(ptr = gParentCtlConfig.devlist;ptr != NULL; ptr = ptr->next)
	{
		if(ip->ip_src.s_addr == ptr->ip)
		{
			find_sta = 1;
			break;
		}
		
	}
	if(find_sta)
	{
		for(uptr = gParentCtlConfig.urllist;uptr != NULL; uptr = uptr->next)
		{
			url_len = strlen(uptr->url);
			for(i = 0 ; i < data_len ; i++)
			{
				if(data[i] == *(uptr->url)  && memcmp(uptr->url, data+i, url_len) == 0)
				{
					/*find url , match*/
					return urlfil_dafault_action^URLF_BLOCK;
				}
			}
		}
		/*not find url , return default action.*/
		return urlfil_dafault_action;
	}
	else
	{
		/*not find sta , return pass.*/
		return URLF_PASS;
	}
}

/*
	从http 报文 查找对于字段
	返回值
		大于0:	 查找成功，返回查找的字符串长度，并保存到str
*/
static int search_http_string(const char *data, int data_len, const char *pattern, char *str)
{
	if(data == NULL || pattern == NULL || str == NULL)
		return -1;
	int i , j, str_len;
	int plen = strlen(pattern);
	for(i = 0 ; i < data_len ; i++)
	{
		if(data[i] == *(pattern)  && memcmp(pattern, data+i, plen) == 0)
		{
			for (j=i+plen; j<data_len; j++) {	
				if (data[j] == '\r' || data[j] == '\n') {
					str_len = j - (i+plen);
					if(str_len == 0)
						break;
					if(str_len >= URL_MAX_LEN)
					{
						strncpy(str, data+i+plen, URL_MAX_LEN-1);
						return URL_MAX_LEN-1;
					}
					else
					{
						strncpy(str, data+i+plen, str_len);
						return str_len;
					}
					
				}
			}
		}
	}
	return -1;
}
/*
	url 规则匹配
*/
typedef enum {
	MATCH_ERR = 0 ,
	MATCH_OK,
} match_ret;

static match_ret url_match_rule(const char *url)
{
	if(url == NULL)
		return MATCH_ERR;
	int i = 0, len = 0;
	struct parentCtl_url_list  *uptr = NULL;
	int url_len = strlen(url);
	for(uptr = gParentCtlConfig.urllist;uptr != NULL; uptr = uptr->next)
	{
		len = strlen(uptr->url);
		for (i=0; (i+len)<=url_len; i++) {
			if (url[i] == *(uptr->url) && memcmp(uptr->url, url+i, len) == 0) {
			//	printf("%s match %s ok\n",url, uptr->url);
				return MATCH_OK;
			}
		}
	}
//	printf("%s match err.\n",url);
	return MATCH_ERR;
}

int
urlfilter_match(struct ifnet *ifp, char *head, struct mbuf *m)
{
	struct in_ifaddr *ia;
	struct ip *ip;
	struct tcphdr *tcp;
	char *data;
	int pk_len;
	char *path = 0;
	struct parentCtl_devices_list  *ptr;
	int find_sta = 0;
	char host[URL_MAX_LEN] = {0};
	char referer[URL_MAX_LEN] = {0};
	
	if(head == NULL)
		return URLF_PASS;

	if ((((struct ifnet *)m->m_pkthdr.rcvif)->if_fltflags & IFFLT_NAT) == 0)
		return URLF_PASS;

	if (m->m_pkthdr.len < 40)
		return URLF_PASS;
	/* Check destination IP, if it is to the interface IP, pass it */
	ip = mtod(m, struct ip *);

#if defined(__OpenBSD__)
	for (ia = in_ifaddr.tqh_first; ia; ia = ia->ia_list.tqe_next) {
		if (ip->ip_dst.s_addr == ia->ia_addr.sin_addr.s_addr)
			return URLF_PASS;
	}
#else
	TAILQ_FOREACH(ia, &in_ifaddrhead, ia_link)
		if (IA_SIN(ia)->sin_addr.s_addr == ip->ip_dst.s_addr)
			return URLF_PASS;
#endif

	/* Web page should be TCP packet */
	if (ip->ip_p != IPPROTO_TCP)
		return URLF_PASS;//urlfil_dafault_action;#roy modify
	
	tcp = (struct tcphdr *)((char *)ip + (ip->ip_hl << 2));

	/*https 过滤*/
	if(ntohs(tcp->th_dport) == 443 )
	{
		return https_url_match(ip , m->m_pkthdr.len);
	}

	if(tcp->th_flags & TH_SYN || ntohs(tcp->th_dport) != 80 )
		return URLF_PASS;

	data = (char *)tcp + (tcp->th_off << 2);
	pk_len = m->m_pkthdr.len - (tcp->th_off << 2) - (ip->ip_hl << 2);
	
	if (pk_len < 10)
		return URLF_PASS;
	
	switch (data[0])
	{
	case 'G':
		if (memcmp(data, "GET ", 4) == 0) {
			pk_len -= 4;
			data += 4;
			path = data;
		}
		break;

	case 'P':
		if (memcmp(data, "POST ", 5) == 0) {
			pk_len -= 5;
			data += 5;
			path = data;
		}
		break;

	case 'H':
		if (memcmp(data, "HEAD ", 5) == 0) {
			pk_len -= 5;
			data += 5;
			path = data;
		}
		break;

	default:
		break;
	}

	if (path == 0)
		return URLF_PASS;

	/* Now check ip */
	for(ptr = gParentCtlConfig.devlist;ptr != NULL; ptr = ptr->next)
	{
		if(ip->ip_src.s_addr == ptr->ip)
		{
			find_sta = 1;
			break;
		}
	}
	
	if( find_sta == 0 )
		return URLF_PASS;

	/* Get Host: */
	if(search_http_string(data, pk_len, HTTP_HOST, host) <= 0)
		return urlfil_dafault_action;
	
	if(url_match_rule(host) == MATCH_OK)
		return urlfil_dafault_action^URLF_BLOCK;
	else
	{
		if(url_filter_mode == URLF_MODE_PASS)
		{	/* Get Referer: */
			if(search_http_string(data, pk_len, HTTP_REFERER, referer) > 0)
			{
				if(url_match_rule(referer) == MATCH_OK)
					return urlfil_dafault_action^URLF_BLOCK;
				else
					return urlfil_dafault_action;
			}
		}
	}
	return urlfil_dafault_action;
}

#else

/* change liuchengchi URL HTTPS filter 过滤 */
int https_url_match( struct ip *ip , int length , 	char *matched_url  )
{

	char *data;
	int data_len;
	int url_index = 0 ;
	int sip;
	struct tcphdr *tcp;
	struct urlfilter *url_en;
	int i = 0 ;
	int j = 0 ;
	int url_len = 0 ;
	
	if (urlfil_list == 0)
	{
		return urlfil_dafault_action;
	}
	tcp = (struct tcphdr *)((char *)ip + (ip->ip_hl << 2));
	data = (char *)tcp + (tcp->th_off << 2);
	data_len = length - (tcp->th_off << 2) - (ip->ip_hl << 2);
	if (data_len < 10)
	{
		return 0;
	}
	if(data[url_index] != 0x16)
	{
		return 0 ;
	}

	sip = ntohl(ip->ip_src.s_addr);
	for(url_en = urlfil_list; url_en ; url_en = url_en->next)
	{
		if (url_en->sip > sip || url_en->eip < sip )
		{
			continue ;
		}

		url_len = strlen(url_en->url) ;
		for(i = 0 ; i < data_len ; i++)
		{
			if(i + url_len > data_len)
			{
				break ;
			}

			if(data[i] == url_en->url[j] )
			{
				j++ ;
				if(j == url_len)
				{
#ifdef CONFIG_URLFLT_LOG				
					strcpy(matched_url , url_en->url);
#endif
					return 1 ;
				}
			}
			else
			{
				j = 0 ;
			}
		}
		
	}
	return 0 ;
}

/* url filter matching function */
int
urlfilter_match(struct ifnet *ifp, char *head, struct mbuf *m)
{
	struct in_ifaddr *ia;
	struct ip *ip;
	struct tcphdr *tcp;
	char *data;
	int pk_len;
	char *url = 0;
	int url_len = 0;
	char *path = 0;
	int path_len = 0;
	int i, j;
	int sip;
	struct urlfilter *url_en;
	char matched_url[256] = {0} ;
	
	if(head == NULL)
		return URLF_PASS;

	if ((((struct ifnet *)m->m_pkthdr.rcvif)->if_fltflags & IFFLT_NAT) == 0)
		return URLF_PASS;

	if (m->m_pkthdr.len < 40)
		return URLF_PASS;//urlfil_dafault_action;#roy modify

	/* Check destination IP, if it is to the interface IP, pass it */
	ip = mtod(m, struct ip *);

#if defined(__OpenBSD__)
	for (ia = in_ifaddr.tqh_first; ia; ia = ia->ia_list.tqe_next) {
		if (ip->ip_dst.s_addr == ia->ia_addr.sin_addr.s_addr)
			return 0;
	}
#else
	TAILQ_FOREACH(ia, &in_ifaddrhead, ia_link)
		if (IA_SIN(ia)->sin_addr.s_addr == ip->ip_dst.s_addr)
			return 0;
#endif

	/* Web page should be TCP packet */
	if (ip->ip_p != IPPROTO_TCP)
		return URLF_PASS;//urlfil_dafault_action;#roy modify
	
	tcp = (struct tcphdr *)((char *)ip + (ip->ip_hl << 2));

/* change liuchengchi URL HTTPS filter 过滤 */
	if(ntohs(tcp->th_dport) == 443 )
	{
		if (check_https == CHECK_HTTPS)
		{
			if(https_url_match(ip , m->m_pkthdr.len , matched_url))
			{
				goto match ;
			}
		}
		else if (check_https == NOT_CHECK_HTTPS)
			return urlfil_dafault_action;
	}
//end by ll

	//if(tcp->th_flags & TH_SYN)
	if(tcp->th_flags & TH_SYN || ntohs(tcp->th_dport) != 80 )//roy modify
		return URLF_PASS;//urlfil_dafault_action;#roy modify

	/* Check empty firstly for performance */
	if (urlfil_list == 0)
		return urlfil_dafault_action;

	data = (char *)tcp + (tcp->th_off << 2);
	pk_len = m->m_pkthdr.len - (tcp->th_off << 2) - (ip->ip_hl << 2);
	
	if (pk_len < 10)
		return URLF_PASS;//urlfil_dafault_action;#roy modify
	
	switch (data[0])
	{
	case 'G':
		if (memcmp(data, "GET ", 4) == 0) {
			pk_len -= 4;
			data += 4;
			path = data;
		}
		break;

	case 'P':
		if (memcmp(data, "POST ", 5) == 0) {
			pk_len -= 5;
			data += 5;
			path = data;
		}
		break;

	case 'H':
		if (memcmp(data, "HEAD ", 5) == 0) {
			pk_len -= 5;
			data += 5;
			path = data;
		}
		break;

	default:
		break;
	}

	if (path == 0)
		return 0;

	/* Get path length */
	for (i=0; i<pk_len; i++) {
		if (data[i] == ' ')
			break;

		if (data[i] == '\r' || data[i] == '\n')
			return 0;
	}
	
	if (i==0 || i==pk_len)
		return 0;

	/* Set path_len */
	path_len = i;
	pk_len -= i;	
	data += i;

	/* Get Host: */
	while (pk_len > 0) {
		url = get_line(data, &pk_len);
		if (url == 0)
			break;

		/* Http header end */
		if ((*url == '\r') || (*url == '\n')) {
			url = 0;
			break;
		}
		
		if (url[0] == 'H' && memcmp(url, "Host:", 5) == 0) {
			url += 5;
			pk_len -= 5;

			if (*url == ' ') {
				url++;
				pk_len--;
			}

			for (i=0; i<pk_len; i++) {
       			if (url[i] == '\r' || url[i] == '\n') {
#if 0	//we don't nedd path					
					/* Append path */
					for (j=0; j<path_len; j++)
						cmpstr[i+j] = path[j];

					url_len = i+j;					
					cmpstr[url_len] = '\0';
#else
					url_len = i;
					cmpstr[i] = '\0';
#endif
					break;
				}
				else {
					cmpstr[i] = url[i];
				}
			}

			break;
		}

		data = url;
		url = 0;
	}
		
	if (url == 0 || pk_len == 0)
		return 0;
	
	/* Now check ip range */
	sip = ntohl(ip->ip_src.s_addr);
	for (url_en = urlfil_list; url_en ; url_en = url_en->next) {
		int len;
		if (url_en->sip > sip || url_en->eip < sip )
			continue;

		len = strlen(url_en->url);
		for (j=0; (j+len)<=url_len; j++) {//roy modify,2011/10/27
			if (cmpstr[j] == *(url_en->url) &&
			    memcmp(url_en->url, cmpstr+j, len) == 0) {
				goto match;
			}
		}
	}
	return urlfil_dafault_action;

match:
#ifdef CONFIG_URLFLT_LOG
	{
		char srcip[20];
		sip = ntohl(ip->ip_src.s_addr);// change liuchengchi URL HTTPS filter 过滤

		sip = ntohl(sip);
		strcpy(srcip, inet_ntoa(*(struct in_addr *)&sip));
		if ((urlfil_dafault_action^1) == 1)
		{//change liuchengchi URL HTTPS filter 过滤
			if(strlen(matched_url) != 0)
			{		
				SysLog(LOG_KERN|LOG_NOTICE|LOGM_FIREWALL, "**Drop Packet** [%s] match, %s->*", matched_url, srcip);
				memset(matched_url , 0 , sizeof(matched_url));	
			}
			else
			{
				SysLog(LOG_KERN|LOG_NOTICE|LOGM_FIREWALL, "**Drop Packet** [%s] match, %s->*", url_en->url, srcip);
			}
				
		}	//end change
	}
#endif

	return urlfil_dafault_action^URLF_BLOCK;//^1;#roy modify
}
#endif
/* Hook the url filter */
void
urlfilter_act(void)
{
	urlfilter_checkp = urlfilter_match;
}

/* Unhook the url filter */
void
urlfilter_inact(void)
{
	urlfilter_checkp = 0;
}

/* Delete an entry from the url table */
int
urlfilter_del(struct urlfilter *entry)
{
//roy +++,2010/10/13
	struct urlfilter *pre_en,*cur_en;

	cur_en = urlfil_list;
	if(!cur_en || !entry)
		return 0;

	if(cur_en->sip == entry->sip && cur_en->eip == entry->eip && strcmp(cur_en->url,entry->url) == 0){
		//is the head node
		urlfil_list = cur_en->next;
		free(cur_en, M_PFIL);
	}else{
		//find next
		pre_en = cur_en;
		cur_en = cur_en->next;
		while (cur_en) {
			if(cur_en->sip == entry->sip && cur_en->eip == entry->eip && strcmp(cur_en->url,entry->url) == 0){
				pre_en->next = cur_en->next;
				free(cur_en, M_PFIL);
				break;;
			}
			pre_en = cur_en;
			cur_en = cur_en->next;
		}
	}
//+++
	/* Not implement yet */
	return 0;
}

/* Add an entry to the url filter table */
int
urlfilter_add(struct urlfilter *entry)
{
	struct urlfilter *new_en;

	if (!(new_en = malloc(sizeof(struct urlfilter), M_PFIL, M_NOWAIT)))
		return -1;
	memcpy(new_en, entry, sizeof(struct urlfilter));
	new_en->next = urlfil_list;
	urlfil_list = new_en;
	return 0;
}

/* Flush all the url filter entries */
void
urlfilter_flush(void)
{
	struct urlfilter *entry, *next_en;

	entry = urlfil_list;
	urlfil_list = NULL;
	while (entry) {
		next_en = entry->next;
		free(entry, M_PFIL);
		entry = next_en;
	}
}
//roy+++,2010/09/20
int urlfilter_set_mode(int mode)
{
	int error = 0;

	switch (mode) {
	case URLF_MODE_DISABLED:
		urlfilter_inact();
		#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		url_filter_mode = URLF_MODE_DISABLED;
		#endif
		break;
	case URLF_MODE_DENY:
		urlfilter_checkp = urlfilter_match;
		urlfilter_flush();
		//如果是仅禁止,那么不在列表中的就是PASS
		urlfil_dafault_action = URLF_PASS;
		check_https = CHECK_HTTPS;/* change liuchengchi URL HTTPS filter 过滤 */
		#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		url_filter_mode = URLF_MODE_DENY;
		#endif
		break;
	case URLF_MODE_PASS:
		urlfilter_checkp = urlfilter_match;
		urlfilter_flush();
		//如果是仅允许,那么不在列表中的就是BLOCK
		urlfil_dafault_action = URLF_BLOCK;
		check_https = NOT_CHECK_HTTPS;/* change liuchengchi URL HTTPS filter 过滤 */
		#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		url_filter_mode = URLF_MODE_PASS;
		#endif
		break;
	default:
		error = EINVAL;
		break;
	}
	return error;
}
//+++

