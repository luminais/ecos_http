/* $Id: iptcrdr.c,v 1.2 2007-08-31 11:37:03 chien_hsiang Exp $ */
/* MiniUPnP project
 * website : http://miniupnp.free.fr/
 * (c) 2006 Thomas Bernard
 * This software is subjects to the conditions detailed in
 * the LICENCE file included in the distribution */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include "libiptc/libiptc.h"
#include "iptables.h"
#include <linux/netfilter_ipv4/ip_nat.h>
//#include "../../../linux-2.6.30/include/linux/netfilter_ipv4/ip_nat.h"
#include "iptcrdr.h"

#define syslog(x, fmt, args...);

#ifdef MULTI_PPPOE
#define PppdeviceNumber 4
struct IpInfo
{
	char subnet[20];
	char pppIfaceName[32];
};
struct IpInfo pppInfos[PppdeviceNumber];
char ppp_order[4][32];

#endif
/* chain name to use, both in the nat table
 * and the filter table */
static const char miniupnpd_chain[] = "MINIUPNPD";

/* convert an ip address to string */
static int snprintip(char * dst, size_t size, uint32_t ip)
{
	return snprintf(dst, size,
	       "%u.%u.%u.%u", ip >> 24, (ip >> 16) & 0xff,
	       (ip >> 8) & 0xff, ip & 0xff);
}

int
add_redirect_rule2(const char * ifname, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
				   const char * desc)
{
	return addnatrule(proto, eport, iaddr, iport);
}

int
add_filter_rule2(const char * ifname, const char * iaddr,
                 unsigned short eport, int proto, const char * desc)
 
{
	return add_filter_rule(proto, iaddr, eport);
}

/* get_redirect_rule() 
 * returns -1 if the rule is not found */
int
get_redirect_rule(const char * ifname, unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc)
{
	int r = -1;
	iptc_handle_t h;
	const struct ipt_entry * e;
	const struct ipt_entry_target * target;
	const struct ip_nat_multi_range * mr;
	const struct ipt_entry_match *match;

	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "get_redirect_rule_by_index() : "
		                "iptc_init() failed : %s",
		       iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(miniupnpd_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_chain);
	}
	else
	{
		for(e = iptc_first_rule(miniupnpd_chain, &h);
		    e;
			e = iptc_next_rule(e, &h))
		{
			
			if(proto==e->ip.proto)
			{
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					if(eport != info->dpts[0])
						continue;
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					if(eport != info->dpts[0])
						continue;
				}
				target = (void *)e + e->target_offset;
				mr = (const struct ip_nat_multi_range *)&target->data[0];
				snprintip(iaddr, iaddrlen, ntohl(mr->range[0].min_ip));
				*iport = ntohs(mr->range[0].min.all);
				if(desc!=NULL)
				  strncpy(desc, "miniupnpd", 16);
				r = 0;
				break;
			}
		}
	}
	iptc_free(&h);
	return r;
}

/* get_redirect_rule_by_index() 
 * return -1 when the rule was not found */
int
get_redirect_rule_by_index(int index,
                           char * ifname, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc)
{
	int r = -1;
	int i = 0;
	iptc_handle_t h;
	const struct ipt_entry * e;
	const struct ipt_entry_target * target;
	const struct ip_nat_multi_range * mr;
	const struct ipt_entry_match *match;

	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "get_redirect_rule_by_index() : "
		                "iptc_init() failed : %s",
		       iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(miniupnpd_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_chain);
	}
	else
	{
		for(e = iptc_first_rule(miniupnpd_chain, &h);
		    e;
			e = iptc_next_rule(e, &h))
		{
			if(i==index)
			{
				*proto = e->ip.proto;
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					*eport = info->dpts[0];
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					*eport = info->dpts[0];
				}
				target = (void *)e + e->target_offset;
				mr = (const struct ip_nat_multi_range *)&target->data[0];
				snprintip(iaddr, iaddrlen, ntohl(mr->range[0].min_ip));
				*iport = ntohs(mr->range[0].min.all);
				strncpy(desc, "miniupnpd", 16);
				r = 0;
				break;
			}
			i++;
		}
	}
	iptc_free(&h);
	return r;
}

/* delete_rule_and_commit() :
 * subfunction used in delete_redirect_and_filter_rules() */
static int
delete_rule_and_commit(unsigned int index, iptc_handle_t *h,
                       const char * logcaller)
{
	int r = 0;
	
	if(!iptc_delete_num_entry(miniupnpd_chain, index, h))
	{
		syslog(LOG_ERR, "%s : iptc_delete_num_entry failed : %s\n",
	    	   logcaller, iptc_strerror(errno));
		r = -1;
	}

	else if(!iptc_commit(h))
	{
		syslog(LOG_ERR, "%s : iptc_commit() error : %s\n",
	    	   logcaller, iptc_strerror(errno));
		r = -1;
	}
	
	
	return r;
}

/* delete_redirect_and_filter_rules()
 */
int
delete_redirect_and_filter_rules(unsigned short eport, int proto)
{
	int r = -1;
	unsigned index = 0;
	unsigned i = 0;
	iptc_handle_t h;
	const struct ipt_entry * e;
	const struct ipt_entry_match *match;

	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "delete_redirect_and_filter_rules() : "
		                "iptc_init() failed : %s",
		       iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(miniupnpd_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_chain);
	}
	else
	{
		for(e = iptc_first_rule(miniupnpd_chain, &h);
		    e;
			e = iptc_next_rule(e, &h))
		{
			if(proto==e->ip.proto)
			{
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					if(eport != info->dpts[0])
					{
						i++;
						continue;
					}
					
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					if(eport != info->dpts[0])
					{
						i++;
						continue;
					}
				}
				index = i;
				
				r = 0;
				break;
			}
			i++;
		}
	}
	iptc_free(&h);
	/* Now delete both rules */

	if(r==0)
	{
		h = iptc_init("nat");
		if(h && (r == 0))
		{
			r = delete_rule_and_commit(index, &h, "delete_redirect_rule");
		}
		h = iptc_init("filter");
		if(h && (r == 0))
		{
			r = delete_rule_and_commit(index, &h, "delete_filter_rule");
		}
	}
	return r;
}

/* ==================================== */
/* TODO : add the -m state --state NEW,ESTABLISHED,RELATED 
 * only for the filter rule */
static struct ipt_entry_match *
get_tcp_match(unsigned short dport)
{
	struct ipt_entry_match *match;
	struct ipt_tcp * tcpinfo;
	size_t size;
	size =   IPT_ALIGN(sizeof(struct ipt_entry_match))
	       + IPT_ALIGN(sizeof(struct ipt_tcp));
	match = calloc(1, size);
	match->u.match_size = size;
	strncpy(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN);
	tcpinfo = (struct ipt_tcp *)match->data;
	tcpinfo->spts[0] = 0;		/* all source ports */
	tcpinfo->spts[1] = 0xFFFF;
	tcpinfo->dpts[0] = dport;	/* specified destination port */
	tcpinfo->dpts[1] = dport;
	return match;
}

static struct ipt_entry_match *
get_udp_match(unsigned short dport)
{
	struct ipt_entry_match *match;
	struct ipt_udp * udpinfo;
	size_t size;
	size =   IPT_ALIGN(sizeof(struct ipt_entry_match))
	       + IPT_ALIGN(sizeof(struct ipt_udp));
	match = calloc(1, size);
	match->u.match_size = size;
	strncpy(match->u.user.name, "udp", IPT_FUNCTION_MAXNAMELEN);
	udpinfo = (struct ipt_udp *)match->data;
	udpinfo->spts[0] = 0;		/* all source ports */
	udpinfo->spts[1] = 0xFFFF;
	udpinfo->dpts[0] = dport;	/* specified destination port */
	udpinfo->dpts[1] = dport;
	return match;
}

static struct ipt_entry_target *
get_dnat_target(const char * daddr, unsigned short dport)
{
	struct ipt_entry_target * target;
	struct ip_nat_multi_range * mr;
	struct ip_nat_range * range;
	size_t size;

	size =   IPT_ALIGN(sizeof(struct ipt_entry_target))
	       + IPT_ALIGN(sizeof(struct ip_nat_multi_range));
	target = calloc(1, size);
	target->u.target_size = size;
	strncpy(target->u.user.name, "DNAT", IPT_FUNCTION_MAXNAMELEN);
	/* one ip_nat_range already included in ip_nat_multi_range */
	mr = (struct ip_nat_multi_range *)&target->data[0];
	mr->rangesize = 1;
	range = &mr->range[0];
	range->min_ip = range->max_ip = inet_addr(daddr);
	range->flags |= IP_NAT_RANGE_MAP_IPS;
	range->min.all = range->max.all = htons(dport);
	range->flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
	return target;
}

static int
iptc_init_verify_and_append(const char * table, struct ipt_entry * e,
                            const char * logcaller)
{
	iptc_handle_t h;
	h = iptc_init(table);
	if(!h)
	{
		syslog(LOG_ERR, "%s : iptc_init() error : %s\n",
		       logcaller, iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(miniupnpd_chain, h))
	{
		syslog(LOG_ERR, "%s : iptc_is_chain() error : %s\n",
		       logcaller, iptc_strerror(errno));
		return -1;
	}
	if(!iptc_append_entry(miniupnpd_chain, e, &h))
	{
		syslog(LOG_ERR, "%s : iptc_append_entry() error : %s\n",
		       logcaller, iptc_strerror(errno));
		return -1;
	}
	if(!iptc_commit(&h))
	{
		syslog(LOG_ERR, "%s : iptc_commit() error : %s\n",
		       logcaller, iptc_strerror(errno));
		return -1;
	}
	return 0;
}

#ifdef MULTI_PPPOE
/*
	dzh add
	to determine which interface to sent from
	convert the ip address to int then check the ipaddr weather in the range (start---end)
*/
int getPppDevice(struct IpInfo* pppInfos, int pppNumber , char * ipaddr)
{
	char *p;
	char *delim =".";
	char *deliM ="./";
	int index ;
	unsigned int ip_integer = 0, count = 1;
	unsigned int start,end;
    p=strtok(ipaddr,delim);
	while(NULL != p)
	{
	   ip_integer+= (atoi(p)<<(8*(4-count++)));
	   p = strtok(NULL,delim);
	}
	//loop for check which subnet is rational
	for(index = 0 ; index < pppNumber ; ++index)
	{
		count = 1;
		start = end = 0;
	    p=strtok(pppInfos[index].subnet,deliM);
		while(NULL!= p)
		{
		   if(count <= 4)
				start += (atoi(p) << (8*(4-count++)));
		   else{
				end = start + (1<<(32-atoi(p)));
				if(ip_integer >= start && ip_integer <=end)
					return index;
		   }
		   p = strtok(NULL,deliM); 
		}
	}
	//cannot match the subnet,return -1
	return -1 ; //error
}

#endif
/* add nat rule 
 * iptables -t nat -A MINIUPNPD -p proto --dport eport -j DNAT --to iaddr:iport
 * */
int addnatrule(int proto, unsigned short eport,
               const char * iaddr, unsigned short iport)
{
	int r = 0;
#ifdef MULTI_PPPOE	
	int index = 0;
	int ret = -1;
	int connect_number;
	FILE * pF;
	FILE * order;
	char tmp_addr[20];
#endif	
	struct ipt_entry * e;
	struct ipt_entry_match *match = NULL;
	struct ipt_entry_target *target = NULL;

	e = calloc(1, sizeof(struct ipt_entry));
	e->ip.proto = proto;
	if(proto == IPPROTO_TCP)
	{
		match = get_tcp_match(eport);
	}
	else
	{
		match = get_udp_match(eport);
	}
	e->nfcache = NFC_IP_DST_PT;
#ifdef MULTI_PPPOE	
	if((pF = fopen("/etc/ppp/SubInfos","r+")) == NULL)
	{
		printf("Cannot open this file\n");
		goto next;
	}
	if((order=fopen("/etc/ppp/ppp_order_info","r+"))==NULL)
	{
		printf("Cannot open this file\n");
		goto next;
	}
	fscanf(pF,"%d",&connect_number);	//get the connect number
	for(index = 0 ; index < connect_number ; ++index)
	{
		int ord;
		char str[32];
		fscanf(order,"%d--%s",&ord,str);
		memcpy(ppp_order[ord-1],str,sizeof(str));
	}
	for(index = 0 ; index < connect_number ; ++index)
	{
		fscanf(pF,"%s",pppInfos[index].subnet);
		memcpy(pppInfos[index].pppIfaceName,ppp_order[index],sizeof(ppp_order[index])); 	
	}
	strcpy(tmp_addr,iaddr);			
	ret = getPppDevice(pppInfos,connect_number,tmp_addr);
	if(ret != -1)
	{
		memcpy(e->ip.iniface,pppInfos[ret].pppIfaceName,sizeof(pppInfos[ret].pppIfaceName));
		for(index = 0 ;index <IFNAMSIZ ; ++index)
			e->ip.iniface_mask[index] = 0xffU;		
	}
	close(pF);
	close(order);
next:	
#endif		
	target = get_dnat_target(iaddr, iport);
	e->nfcache |= NFC_UNKNOWN;
	e = realloc(e, sizeof(struct ipt_entry)
	               + match->u.match_size
				   + target->u.target_size);
	memcpy(e->elems, match, match->u.match_size);
	memcpy(e->elems + match->u.match_size, target, target->u.target_size);
	e->target_offset = sizeof(struct ipt_entry)
	                   + match->u.match_size;
	e->next_offset = sizeof(struct ipt_entry)
	                 + match->u.match_size
					 + target->u.target_size;
	
	r = iptc_init_verify_and_append("nat", e, "addnatrule()");
	free(target);
	free(match);
	free(e);
	return r;
}
/* ================================= */
static struct ipt_entry_target *
get_accept_target(void)
{
	struct ipt_entry_target * target = NULL;
	size_t size;
	size =   IPT_ALIGN(sizeof(struct ipt_entry_target))
	       + IPT_ALIGN(sizeof(int));
	target = calloc(1, size);
	target->u.user.target_size = size;
	strncpy(target->u.user.name, "ACCEPT", IPT_FUNCTION_MAXNAMELEN);
	return target;
}

/* add_filter_rule()
 * */
int add_filter_rule(int proto, const char * iaddr, unsigned short iport)
{
	int r = 0;
	struct ipt_entry * e;
	struct ipt_entry_match *match = NULL;
	struct ipt_entry_target *target = NULL;

	e = calloc(1, sizeof(struct ipt_entry));
	e->ip.proto = proto;
	if(proto == IPPROTO_TCP)
	{
		match = get_tcp_match(iport);
	}
	else
	{
		match = get_udp_match(iport);
	}
	e->nfcache = NFC_IP_DST_PT;
	e->ip.dst.s_addr = inet_addr(iaddr);
	e->ip.dmsk.s_addr = INADDR_NONE;
	target = get_accept_target();
	e->nfcache |= NFC_UNKNOWN;
	e = realloc(e, sizeof(struct ipt_entry)
	               + match->u.match_size
				   + target->u.target_size);
	memcpy(e->elems, match, match->u.match_size);
	memcpy(e->elems + match->u.match_size, target, target->u.target_size);
	e->target_offset = sizeof(struct ipt_entry)
	                   + match->u.match_size;
	e->next_offset = sizeof(struct ipt_entry)
	                 + match->u.match_size
					 + target->u.target_size;
	
	r = iptc_init_verify_and_append("filter", e, "add_filter_rule()");
	free(target);
	free(match);
	free(e);
	return r;
}

/* ================================ */
static int
print_match(const struct ipt_entry_match *match)
{
	printf("match %s\n", match->u.user.name);
	if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
	{
		struct ipt_tcp * tcpinfo;
		tcpinfo = (struct ipt_tcp *)match->data;
		printf("srcport = %hu:%hu dstport = %hu:%hu\n",
		       tcpinfo->spts[0], tcpinfo->spts[1],
			   tcpinfo->dpts[0], tcpinfo->dpts[1]);
	}
	else if(0 == strncmp(match->u.user.name, "udp", IPT_FUNCTION_MAXNAMELEN))
	{
		struct ipt_udp * udpinfo;
		udpinfo = (struct ipt_udp *)match->data;
		printf("srcport = %hu:%hu dstport = %hu:%hu\n",
		       udpinfo->spts[0], udpinfo->spts[1],
			   udpinfo->dpts[0], udpinfo->dpts[1]);
	}
	return 0;
}

static void
print_iface(const char * iface, const unsigned char * mask, int invert)
{
	unsigned i;
	if(mask[0] == 0)
		return;
	if(invert)
		printf("! ");
	for(i=0; i<IFNAMSIZ; i++)
	{
		if(mask[i])
		{
			if(iface[i])
				putchar(iface[i]);
		}
		else
		{
			if(iface[i-1])
				putchar('+');
			break;
		}
	}
}

static void printip(uint32_t ip)
{
	printf("%u.%u.%u.%u", ip >> 24, (ip >> 16) & 0xff,
	       (ip >> 8) & 0xff, ip & 0xff);
}

/* for debug */
/* on aura besoin de toucher a la table "filter" et "nat" */
int list_redirect_rule(const char * ifname)
{
	iptc_handle_t h;
	const struct ipt_entry * e;
	const struct ipt_entry_target * target;
	const struct ip_nat_multi_range * mr;
	const char * target_str;

	h = iptc_init("nat");
	if(!h)
	{
		printf("iptc_init() error : %s\n", iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(miniupnpd_chain, h))
	{
		printf("chain %s not found\n", miniupnpd_chain);
		return -1;
	}
	for(e = iptc_first_rule(miniupnpd_chain, &h);
		e;
		e = iptc_next_rule(e, &h))
	{
		target_str = iptc_get_target(e, &h);
		printf("===\n");
		printf("src = %s%s/%s\n", (e->ip.invflags & IPT_INV_SRCIP)?"! ":"",
		       inet_ntoa(e->ip.src), inet_ntoa(e->ip.smsk));
		printf("dst = %s%s/%s\n", (e->ip.invflags & IPT_INV_DSTIP)?"! ":"",
		       inet_ntoa(e->ip.dst), inet_ntoa(e->ip.dmsk));
		/*printf("in_if = %s  out_if = %s\n", e->ip.iniface, e->ip.outiface);*/
		printf("in_if = ");
		print_iface(e->ip.iniface, e->ip.iniface_mask,
		            e->ip.invflags & IPT_INV_VIA_IN);
		printf(" out_if = ");
		print_iface(e->ip.outiface, e->ip.outiface_mask,
		            e->ip.invflags & IPT_INV_VIA_OUT);
		printf("\n");
		printf("ip.proto = %s%d\n", (e->ip.invflags & IPT_INV_PROTO)?"! ":"",
		       e->ip.proto);
		/* display matches stuff */
		if(e->target_offset)
		{
			IPT_MATCH_ITERATE(e, print_match);
			/*printf("\n");*/
		}
		printf("target = %s\n", target_str);
		target = (void *)e + e->target_offset;
		mr = (const struct ip_nat_multi_range *)&target->data[0];
		printf("ips ");
		printip(ntohl(mr->range[0].min_ip));
		printf(" ");
		printip(ntohl(mr->range[0].max_ip));
		printf("\nports %hu %hu\n", ntohs(mr->range[0].min.all),
		          ntohs(mr->range[0].max.all));
		printf("flags = %x\n", mr->range[0].flags);
	}
	iptc_free(&h);
	return 0;
}

