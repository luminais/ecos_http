/* $Id: ipfwrdr.c,v 1.13 2012/03/05 20:36:19 nanard Exp $ */
/*
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2009 Jardel Weyrich
 * (c) 2011-2012 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution
 */
#ifdef __ECOS
#include <network.h>
#endif


#include <sys/param.h>
#include <sys/types.h>

/*
This is a workaround for <sys/uio.h> troubles on FreeBSD, HPUX, OpenBSD.
Needed here because on some systems <sys/uio.h> gets included by things
like <sys/socket.h>
*/
#ifndef _KERNEL
#  define ADD_KERNEL
#  define _KERNEL
#  define KERNEL
#endif

#include <sys/uio.h>
#ifdef ADD_KERNEL
#  undef _KERNEL
#  undef KERNEL
#endif

#include <sys/time.h>
#include <sys/socket.h>
#ifndef __ECOS
#include <sys/syslog.h>
#endif
#include <sys/ioctl.h>
#include <net/if.h>

#include <netinet/in.h>
#ifndef __ECOS
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#endif
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <stdlib.h>

#include <stddef.h>
#include <stdio.h>
//#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "../upnpglobalvars.h"
#ifndef __ECOS
#include <netinet/ip_fw.h>
//#include "ipfwaux.h"
#endif
#ifdef 	ECOS_DBG_STAT
#include "../../system/sys_utility.h"
#endif
#ifdef	ECOS_DBG_STAT
extern int	dbg_igd_index;
#endif


#include "ipfwrdr.h"

#include "../upnpglobalvars.h"
#include "../../common/common.h"
#define REDIRECT_TYPE 1
#define FILTER_TYPE 2
#define BACKUP_UPNP_IPFW_RULES_FILENAME	"/tmp/upnp_ipfw_rules.bak"

static int ipfw_num =IPFW_INDEX_MINIIGD;
char _IPFW[]="ipfw";
char _ADD[]="add";
char _DEL[]="delete";
char _ALLOW[]= "allow";
char _FWD[]="fwd";
char _FROM[]="from";
char _TO[]="to";
char _ANY[]="any";

#ifndef __ECOS

/* init and shutdown functions */

int init_redirect(void) {
	return ipfw_exec(IP_FW_INIT, NULL, 0);
}

void shutdown_redirect(void) {
	ipfw_exec(IP_FW_TERM, NULL, 0);
}
#endif
#if defined(HAVE_NAPT)
#if defined(HAVE_NATD)
extern void ParseOption (const char* option, const char* parms);
#else
extern void rtl_parseNaptOption (const char* option, const char* parms);
#endif
#endif
extern int miniigd_delete_redirect_rule(int proto,int eport,char *iaddr,unsigned short iport);



/* ipfw cannot store descriptions and timestamp for port mappings so we keep
 * our own list in memory */
struct mapping_desc_time {
	struct mapping_desc_time * next;
	unsigned int timestamp;
	unsigned short eport;
	short proto;
	int ipfw_no;
	unsigned short iport;
	unsigned char type;
	char iaddr[20];
	char desc[];
};
static struct mapping_desc_time * mappings_list = NULL;

int ipfw_validate_proto(int value,char* proto)
{
	switch(value){
		case IPPROTO_TCP:
			if(proto)
				sprintf(proto,"%s","tcp");
			break;
		case IPPROTO_UDP:
			if(proto)
				sprintf(proto,"%s","udp");
			break;
		default:
			return -1;
		}
	return 0;
}
/* add an element to the port mappings descriptions & timestamp list */
static int
add_desc_time(unsigned short eport, int proto,
              const char * desc,const char* iaddr, unsigned short iport,
              int ipfw_no,unsigned int timestamp,unsigned char type)
{
	struct mapping_desc_time * tmp;
	size_t l;
	if(!desc)
		desc = "miniupnpd";
	l = strlen(desc) + 1;
	tmp = malloc(sizeof(struct mapping_desc_time) + l);
	if(tmp) {
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, sizeof(struct mapping_desc_time));
#endif
		/* fill the element and insert it as head of the list */
//diag_printf("%s:%d add eport=%d proto=%d desc=%s iaddr=%s iport=%d ipfw_no=%d timestamp=%d type=%d\n",__FUNCTION__,__LINE__,eport,proto,desc,iaddr,iport,ipfw_no,timestamp,type);
		tmp->next = mappings_list;
		tmp->timestamp = timestamp;
		tmp->eport = eport;
		tmp->proto = (short)proto;
		tmp->ipfw_no = ipfw_no;
		tmp->type = type;
		tmp->iport = iport;
		if(iaddr)
			strcpy(tmp->iaddr,iaddr);
		if(desc)
		memcpy(tmp->desc, desc, l);
		mappings_list = tmp;
		return 0;
	}
	return -1;
}

/* remove an element to the port mappings descriptions & timestamp list */
static void
del_desc_time(unsigned short eport, int proto,unsigned char type)
{
	struct mapping_desc_time * e;
	struct mapping_desc_time * * p;
	p = &mappings_list;
	e = *p;
	while(e) {
		if(e->eport == eport && e->proto == (short)proto && e->type == type) {
			*p = e->next;
			free(e);
#ifdef	ECOS_DBG_STAT
			dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct mapping_desc_time));
#endif
			return;
		} else {
			p = &e->next;
			e = *p;
		}
	}
}

/* go through the list and find the description and timestamp */
int
get_desc_time(unsigned short eport, int proto,unsigned char type,
					char *iaddr,unsigned short *iport, char *desc,int *index)
{
	struct mapping_desc_time * e;

	for(e = mappings_list; e; e = e->next) {
		if(e->eport == eport && e->proto == (short)proto && e->type == type) {
			*index = e ->ipfw_no;
			if(iaddr)
				strcpy(iaddr,e->iaddr);
			if(iport)
				*iport = e->iport;
			if(desc)
				strcpy(desc,e->desc);
			return 0;
		}
	}
	return -1;
}
int get_desc_time_by_index(
	int index,
	unsigned short * eport,
	char * iaddr,
	char * desc,
	unsigned short * iport,
	int * proto)
{
	struct mapping_desc_time *e;
	int i = 0;
	for(e = mappings_list; e; e = e->next) {
		if(i == index){
			if(eport)
				*eport = e->eport;
			if(proto)
				*proto = e->proto;
			if(iaddr)
				strcpy(iaddr,e->iaddr);
			if(iport)
				*iport = e->iport;
			if(desc)
				strcpy(desc,e->desc);
			return 0;
		}
		else
			i++;
	}
	return -1;
	
}

/* --- */
int add_redirect_rule2(
	const char * ifname,
	unsigned short eport,
	const char * iaddr,
	unsigned short iport,
	int proto,
	const char * desc,unsigned int timestamp)
{
	char prot[10];
	char port[10];
	char ipfw_index[10];
	char *param = NULL;
	
	if(ipfw_validate_proto(proto,prot) < 0)
		return -1;
	if(add_desc_time(eport,proto,desc,iaddr,iport,ipfw_num,timestamp,REDIRECT_TYPE)<0)
		return -1;
	param = malloc(100);
	if(param == NULL)
		return -1;
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 100);
#endif
	sprintf(param,"%s %s:%d %s:%d",prot,iaddr,iport,ext_ip_addr,eport);
#if defined(HAVE_NAPT)

#if defined(HAVE_NATD)
	ParseOption ("redirect_port", param);
#else
	rtl_parseNaptOption ("redirect_port",  param);
#endif
#endif
	sprintf(ipfw_index,"%d",ipfw_num++); 
	
	if(ipfw_num ==IPFW_INDEX_MINIIGD+IPFW_SECTOR_SIZE*2)
		ipfw_num = IPFW_INDEX_MINIIGD;
	sprintf(port,"%d",iport);

	char *iaddr_netmask = (char *)malloc(20);
	sprintf(iaddr_netmask,"%s/32",iaddr);
		
	RunSystemCmd(NULL,_IPFW,_ADD,ipfw_index,_ALLOW,prot,_FROM,_ANY,_TO,iaddr_netmask,port,"in","via",ext_if_name,"");
//diag_printf(" ipfw add %s allow %s from any to %s %s in via %s %s:%d\n",ipfw_index,prot,iaddr_netmask,port,ext_if_name,__FUNCTION__,__LINE__);
	//RunSystemCmd(NULL,_IPFW,_ADD,ipfw_index,_FWD,addr_port,prot,_FROM,_ANY,_TO,"me",export,"");

	free(iaddr_netmask);
	iaddr_netmask = NULL;
	free(param);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 100);
#endif

	return 0;
}

/* get_redirect_rule()
 * return value : 0 success (found)
 * -1 = error or rule not found */
int get_redirect_rule(
	const char * ifname,
	unsigned short eport,
	int proto,
	char * iaddr,
	int iaddrlen,
	unsigned short * iport,
	char * desc)
{
	int index;
	if(ipfw_validate_proto(proto,NULL) < 0)
		return -1;
	if(get_desc_time(eport,proto,REDIRECT_TYPE,iaddr,iport,desc,&index) < 0)
		return -1;
	return 0;
}

int delete_redirect_rule(
	const char * ifname,
	unsigned short eport,
	int proto)
{
	int value =0;
	char ipfw_index[10];
	char iaddr[20];
	unsigned short iport = 0;
	if(ipfw_validate_proto(proto,NULL) < 0)
		return -1;
	if(get_desc_time( eport,proto,REDIRECT_TYPE,iaddr,&iport,NULL,&value) < 0)
		return -1;
	sprintf(ipfw_index,"%d",value);
	/*add delete rediect rule*/
	if(miniigd_delete_redirect_rule(proto,eport,iaddr,iport) < 0){
		//printf("miniigd delete redirect rule failed !\n");
		return -1;
		}
	RunSystemCmd(NULL,_IPFW,_DEL,ipfw_index,"");
	del_desc_time(eport,proto,REDIRECT_TYPE);
	return 0;
}

int add_filter_rule2(
	const char * ifname,
	const char * iaddr,
	unsigned short eport,
	int proto,
	const char * desc)
{
    /*eport is iport!*/
	/*and delete filter rule is eport, so add it when add redirect rule*/
	#if 0
	char ipfw_index[10];
	char port[10];
	char prot[10];

	if(ipfw_validate_proto(proto,prot) < 0)
		return -1;
	if(add_desc_time(eport,proto,desc,iaddr,0,ipfw_num,time(NULL),FILTER_TYPE) <0)
		return -1;
	sprintf(ipfw_index,"%d",ipfw_num++);
	if(ipfw_num ==IPFW_INDEX_MINIIGD+IPFW_SECTOR_SIZE*2)
		ipfw_num = IPFW_INDEX_MINIIGD;
	sprintf(port,"%d",eport);
	
	//RunSystemCmd(NULL,_IPFW,_ADD,ipfw_index,_ALLOW,prot,_FROM,iaddr,_TO,_ANY,port,"");
	#endif
	return 0;

}

int delete_filter_rule(
	const char * ifname,
	unsigned short eport,
	int proto)
{
	/*eport is eport*/
	/*delete it when delete redirect rule*/
	#if 0
	int value;
	char ipfw_index[10];
	if(ipfw_validate_proto(proto,NULL) < 0)
		return -1;
	if(get_desc_time( eport,proto,FILTER_TYPE,NULL,NULL,&value) < 0)
		return -1;
	sprintf(ipfw_index,"%d",value);
	RunSystemCmd(NULL,_IPFW,_DEL,ipfw_index,"");
	del_desc_time(eport,proto,FILTER_TYPE);
	#endif
	return 0;
}

int get_redirect_rule_by_index(
	int index,
	char * ifname,
	unsigned short * eport,
	char * iaddr,
	int iaddrlen,
	unsigned short * iport,
	int * proto,
	char * desc)
{
	if (index < 0)
		return -1;
	if(get_desc_time_by_index(index,eport,iaddr,desc,iport,proto) < 0)
		return -1;
	return 0;
}
int clear_expired_redirect_rule()
{
	char ipfw_index[10];
	int n = 0;
	unsigned int uptime;
	struct mapping_desc_time *e,**p;
	p = &mappings_list;
	e = *p;
	uptime = time(NULL);
	while(e){
		if(e->timestamp < uptime){
			*p = e->next;
			sprintf(ipfw_index,"%d",e ->ipfw_no);
			RunSystemCmd(NULL,_IPFW,_DEL,ipfw_index,"");
			if(miniigd_delete_redirect_rule(e->proto,e->eport,e->iaddr,e->iport) < 0)
			{
				return -1;
			}
			free(e);
#ifdef	ECOS_DBG_STAT
			dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct mapping_desc_time));
#endif
			n++;
			e = *p;
		}
		else{
			p = &e->next;
			e = *p;
		}
	}
	return n;
	
}
void save_mappings()
{
	FILE *fp;
	struct mapping_desc_time * e;
	size_t len;
	
	
	unsigned char rule[256],desc[64];

	if( !(fp = fopen(BACKUP_UPNP_IPFW_RULES_FILENAME,"w")) )
		return -1;	
	
	for(e = mappings_list; e; e = e->next) {		
		sprintf(rule,"timestamp=%d eport=%d proto=%d ipfw_no=%d iport=%d type=%d iaddr=%s desc=%s\n",
			e->timestamp,e->eport,e->proto,e->ipfw_no,e->iport,e->type,e->iaddr,e->desc);
		fputs(rule,fp);
	}

	fclose(fp);
	return 0;
}

void clear_mappings()
{
	struct mapping_desc_time *e,*p;
	char ipfw_index[10];
	p = mappings_list;
	e = p;
	while(e) {
		p = e->next;
		sprintf(ipfw_index,"%d",e ->ipfw_no);
		RunSystemCmd(NULL,_IPFW,_DEL,ipfw_index,"");
		miniigd_delete_redirect_rule(e->proto,e->eport,e->iaddr,e->iport);
		free(e);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, sizeof(struct mapping_desc_time));
#endif
		e = p;
	}
	mappings_list = NULL;
	ipfw_num = IPFW_INDEX_MINIIGD;;
}
void clear_and_reset()
{
	char ipfw_index[10];
	char prot[10];
	char port[10];
		
	char *param;
	struct mapping_desc_time *e,*p;
	param = malloc(100);
	if(param == NULL){
		clear_mappings();
		return ;
	}
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 100);
#endif
	p = mappings_list;
	e = p;
	while(e) {
		ipfw_validate_proto(e->proto,prot);
		sprintf(param,"%s %s:%d %s:%d",prot,e->iaddr,e->iport,ext_ip_addr,e->eport);
#if defined(HAVE_NAPT)
#if defined(HAVE_NATD)
		ParseOption ("redirect_port", param);
#else
		rtl_parseNaptOption ("redirect_port",  param);
#endif
#endif
		sprintf(ipfw_index,"%d",e->ipfw_no); 
		sprintf(port,"%d",e->iport);
		RunSystemCmd(NULL,_IPFW,_ADD,ipfw_index,_ALLOW,prot,_FROM,_ANY,_TO,e->iaddr,port,"in","via",ext_if_name,"");
		//RunSystemCmd(NULL,_IPFW,_ADD,ipfw_index,_FWD,addr_port,prot,_FROM,_ANY,_TO,"me",export,"");
		p = e->next;
		e = p;
	}
	free(param);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 100);
#endif
	return;
}

/* upnp_get_portmappings_in_range()
 * return a list of all "external" ports for which a port
 * mapping exists */
unsigned short *
get_portmappings_in_range(unsigned short startport,
                          unsigned short endport,
                          int proto,
                          unsigned int * number)
{
	unsigned short * array = NULL;
	unsigned int capacity = 128;

	if (ipfw_validate_protocol(proto) < 0)
		return NULL;
	struct mapping_desc_time * e;
	array = calloc(capacity, sizeof(unsigned short));
	if(!array) {
		//syslog(LOG_ERR, "get_portmappings_in_range() : calloc error");
                goto error;
	}
	*number = 0;
	for(e = mappings_list; e; e = e->next) {
		if( e->proto == (short)proto && e->eport <= endport && startport <= e->eport) {
			if(*number >= capacity) {
				capacity += 128;
				unsigned short * tmp;
				tmp = (unsigned short *)realloc(array, sizeof(unsigned short)*capacity);
				if(!tmp) {
					free(array);
					array =NULL;
					//syslog(LOG_ERR, "get_portmappings_in_range() : realloc(%lu) error", sizeof(unsigned short)*capacity);
					*number = 0;
					goto error;
				}
				array = tmp;
			}
			array[*number] = e->eport;
			(*number)++;
		
		}
	}
error:
	return array;
}
void load_mappings()
{
	int index=0;
	char *ptr=NULL;
	char line[256], token[16], value[64];
	FILE *fp=NULL;
	struct stat status;
	struct mapping_desc_time e;

	if( !stat(BACKUP_UPNP_IPFW_RULES_FILENAME, &status) ){
		if( (fp = fopen(BACKUP_UPNP_IPFW_RULES_FILENAME,"r")) ){
			/* tmp = fopen("/tmp/recover","w"); */
			while( fgets(line,sizeof(line),fp) ){
				sscanf(line,"timestamp=%d eport=%d proto=%d ipfw_no=%d iport=%d type=%d iaddr=%s desc=%s",
					&e.timestamp,&e.eport,&e.proto,&e.ipfw_no,&e.iport,&e.type,e.iaddr,e.desc);
				add_redirect_rule2(NULL,e.eport,e.iaddr,e.iport,e.proto,e.desc,e.timestamp);

				//add_desc_time(e.eport,e.proto,e.desc,e.iaddr,e.iport,e.ipfw_no,e.timestamp,e.type);
				
			}
			fclose(fp);
		}
		else{
			
		}
		/* fclose(tmp); */
		remove(BACKUP_UPNP_IPFW_RULES_FILENAME);
		//clear_and_reset();
	}	
}

