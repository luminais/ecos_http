/*
 * WIDE Project DHCP Implementation
 * Copyright (c) 1995, 1996 Akihiro Tominaga
 * Copyright (c) 1995, 1996 WIDE Project
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided the following conditions
 * are satisfied,
 *
 * 1. Both the copyright notice and this permission notice appear in
 *    all copies of the software, derivative works or modified versions,
 *    and any portions thereof, and that both notices appear in
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by WIDE Project and
 *      its contributors.
 * 3. Neither the name of WIDE Project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE DEVELOPER ``AS IS'' AND WIDE
 * PROJECT DISCLAIMS ANY LIABILITY OF ANY KIND FOR ANY DAMAGES
 * WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. ALSO, THERE
 * IS NO WARRANTY IMPLIED OR OTHERWISE, NOR IS SUPPORT PROVIDED.
 *
 * Feedback of the results generated from any improvements or
 * extensions made to this software would be much appreciated.
 * Any such feedback should be sent to:
 * 
 *  Akihiro Tominaga
 *  WIDE Project
 *  Keio University, Endo 5322, Kanagawa, Japan
 *  (E-mail: dhcp-dist@wide.ad.jp)
 *
 * WIDE project has the rights to redistribute these changes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_var.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <net/route.h>

#include <cyg/kernel/kapi.h>

#include "dhcp.h"
#include "common.h"
#include "common_subr.h"
#include "hash.h"
#include "dhcps.h"
#include "syslog.h"
#include "../system/sys_init.h"

#ifdef CONFIG_RTL_819X
#include "database.h"
#endif

#define DHCPLEN(UDP)  (ntohs(UDP->uh_ulen) - UDPHL)  /* get DHCP message size from
							struct udp */
#define OPTLEN(TAGP)   (*(((char *)TAGP) + 1)) /* get length of DHCP option */
#define OPTBODY(TAGP)  (((char *)TAGP) + 2)    /* get content of DHCP option */

/* handle the word alignment */
#define  GETHS(PTR)   (*((u_char *)PTR)*256 + *(((u_char *)PTR)+1))
#define  GETHL(PTR)   (*((u_char *)PTR)*256*256*256 + *(((u_char *)PTR)+1)*256*256 +\
		       *(((u_char *)PTR)+2)*256 + *(((u_char *)PTR)+3))

#define MEMORIZE      90  /* server avoid to reuse the address between 90 secs */
#define E_NOMORE      (-2)    /* Error code "There is no more space" */

#define WITH_CID      1
#define WITH_REQIP    2
#define FREE          3

/*
 *  global variable
 */
static int overload;      /* flag for option overload */
static int off_options;   /* offset of DHCP options field */
static int off_extopt;    /* offset of extended options */
static int off_file;      /* offset of DHCP file field */
static int off_sname;     /* offset of DHCP sname field */
static int maxoptlen;     /* maximum offset of DHCP options */
static int timeoutflag;   /* used in icmp_check */
static int debug;
static int rdhcplen;

static char *rbufp = NULL;                 /* receive buffer */
static char *sbufp = NULL;
static struct iovec sbufvec[2];            /* send buffer */
static struct msg snd;
static struct if_info *if_list = NULL;      /* interfaces list */

static int flag_addclient=0;
static unsigned long addclient_ip=0;
unsigned char magic_c[] = RFC1048_MAGIC;  /* magic cookie which defined in
					     RFC1048 */
FILE *addrpool_dbfp;
FILE *binding_dbfp;
FILE *relay_dbfp;
FILE *dump_fp;

#ifdef SUPPORT_MAC_FILTER
FILE *mac_dbfp;
#endif

char binding_db[MAXPATHLEN];
char addrpool_db[MAXPATHLEN];
char relay_db[MAXPATHLEN];

#ifdef SUPPORT_MAC_FILTER
char mac_db[MAXPATHLEN];
#endif

int nbind;
struct msg rcv;
struct bpf_hdr *rbpf;

char **argv_orig;

#ifdef SUPPORT_MAC_FILTER
char allowed_haddr[MAX_CLIENTS][6];
int  nmacaddr;
#endif

cyg_uint8  dhcpd_stack[DHCPD_THREAD_STACK_SIZE];
cyg_handle_t  dhcpd_thread;
cyg_thread  dhcpd_thread_object;

char dhcpd_running=0;    // Boolean (running loop)
char dhcpd_started=0;	

static char inf_name[]="eth0";

static cyg_uint8 dhcpd_restart_flag=0;
static cyg_uint8 dhcpd_quitting=0;

static cyg_uint8 lanDnsFromWanDhcpFlag=0;
struct hash_tbl cidhashtable;
struct hash_tbl iphashtable;
struct hash_tbl nmhashtable;
struct hash_tbl relayhashtable;
struct hash_member *bindlist;
struct hash_member *reslist;


#if 0
static int set_default_route(char *intf)
{
	int s = -1;
	struct ifreq if_req;
	
	struct sockaddr_in *addrp;
	struct ecos_rtentry route;
	
	int retcode = FALSE;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) 
	{
		perror("socket");
		diag_printf("create socket fails!\n");
		goto out;
	}

	strcpy(if_req.ifr_name, intf);
	if (ioctl(s, SIOCGIFADDR, &if_req)) 
	{ /* get ifnet address */
		perror("SIOCSIFADDR");
		diag_printf("get ifnet address fails!\n");
		goto out;
	}

#if 1
	addrp = (struct sockaddr_in *) calloc(1, sizeof(struct sockaddr_in));
	memset(addrp, 0, sizeof(*addrp));
	// Set up routing
	addrp->sin_family = AF_INET;
	addrp->sin_port = 0;
	addrp->sin_len = sizeof(*addrp);  // Size of address

	/* the broadcast address is 255.255.255.255 */
//	memset(&addrp->sin_addr, 255, sizeof(addrp->sin_addr));
	memset(&route, 0, sizeof(route));
//	memcpy(&route.rt_dst, addrp, sizeof(*addrp));
	
	memcpy(&route.rt_gateway, &(if_req.ifr_addr), sizeof(struct sockaddr));

	addrp->sin_addr.s_addr = INADDR_ANY;
	memcpy(&route.rt_dst, addrp, sizeof(*addrp));
	memcpy(&route.rt_genmask, addrp, sizeof(*addrp));

	route.rt_dev = intf;
	route.rt_flags = RTF_UP|RTF_GATEWAY;
	route.rt_metric = 0;
#if 0
	if (ioctl(s, SIOCADDRT, &route)) { /* add route */
		if (errno != EEXIST) {
		perror("SIOCADDRT 3");
		diag_printf("add route fails!\n");
		goto out;
		}
	}	
#endif

	memset(&route, 0, sizeof(route));	
	memcpy(&route.rt_gateway, &(if_req.ifr_addr), sizeof(struct sockaddr));

#if 0
	if (ioctl(s, SIOCGIFNETMASK, &if_req)) 
	{ /* set net addr mask */
		perror("SIOCSIFNETMASK");
		diag_printf("get net addr mask fails!\n");
		goto out;
	}
#endif
	
	inet_aton("192.168.1.0", &addrp->sin_addr);
	memcpy(&route.rt_dst, addrp, sizeof(*addrp));
	
	inet_aton("255.255.255.0", &addrp->sin_addr);
	memcpy(&route.rt_genmask, addrp, sizeof(*addrp));
	
	route.rt_dev = intf;
	route.rt_flags = RTF_UP | RTF_HOST;
	route.rt_metric = 0;

	if (ioctl(s, SIOCADDRT, &route)) { /* add route */
		if (errno != EEXIST) {
		perror("SIOCADDRT 3");
		diag_printf("add route fails!\n");
		goto out;
		}
	}	
	
#endif

	retcode = TRUE;
	out:
	if (s != -1) 
	close(s);

	return retcode;
}
#endif

#ifdef KLD_ENABLED
int IsclientExist(char *filename, char *mac_addr, char *host_name, int flag)
{
	if(filename==NULL)
		return -1;
	
	FILE *fp=NULL;
	if((fp=fopen(filename, "r"))==NULL)
		return -1;

	char tmpbuf[64], tmphstnam[64];
	char *phname=NULL;
	int retval=0, find=0;
	while(fgets(tmpbuf, sizeof(tmpbuf), fp))
	{		
		if((phname=strstr(tmpbuf, mac_addr))!=NULL)
		{	
			while(*phname!='\0' && *phname!=':')
				phname++;
			phname++;
			
			strcpy(tmphstnam, phname);		
			int len=strlen(tmphstnam);
			tmphstnam[len-1]='\0';		
			find=1;
		}
	}
	if(find==1)
	{
		if(flag==1)
		{
			strcpy(host_name, tmphstnam);
			retval=1;
		}
		else
		{
			if(strcmp(host_name, tmphstnam)==0)
				retval=1;
		}
	}	
	fclose(fp);
	return retval;
}

void writeHostNameMacToFile(char *filename, char *host_name, char *mac_addr)
{
	int write_flag=0;
	int need_flag=0;
	char tmpbuf[64];
	if(!isFileExist(filename))
	{
		need_flag=1;
		write_flag=1;
	}
	else if(IsclientExist(filename, mac_addr, host_name, 0))
	{
		need_flag=0;
		write_flag=0;
	}
	else
	{
		need_flag=1;
		write_flag=2;
	}

	if(need_flag)
	{
		sprintf(tmpbuf, "%s:%s\n", mac_addr, host_name);
		write_line_to_file(filename, write_flag, tmpbuf);	
	}
	return;	
}
#endif
int get_dhcpd_running_state(void)
{
	return dhcpd_running;
}

#ifdef CONFIG_RTL_819X
static int rp_call_bp=0;
static int bp_call_rp=0;

void free_bp(hash_datum *bptr)
{
	struct dhcp_binding *bp=(struct dhcp_binding *)bptr;
	if(bp==NULL)
		return ;
	
	if(bp->cid.id)
	{
		free(bp->cid.id);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, bp->cid.idlen);
#endif
		bp->cid.id=NULL;
	}
	
	if(bp->haddr.haddr)
	{
		free(bp->haddr.haddr);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, bp->haddr.hlen);
#endif
		bp->haddr.haddr=NULL;
	}

	if(bp->res_name)
	{
		free(bp->res_name);		
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, strlen(bp->res_name)+1);
#endif
		bp->res_name=NULL;
	}

	if(rp_call_bp==0 && bp->res!=NULL)
	{
		bp_call_rp=1;
		free_rp(bp->res);
		bp->res=NULL;
		bp_call_rp=0;
	}
	free(bp);
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct dhcp_binding));
#endif
	bp=NULL;	
	return ;
}

void free_rp(hash_datum *rptr)
{
	struct dhcp_resource *rp=(struct dhcp_resource *)rptr;
	
	if(rp==NULL)
		return ;
	
	int i;
	
	static int first_free=0;
	
	if(rp->entryname!=NULL)
	{
		free(rp->entryname);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, strlen(rp->entryname)+1);
#endif
		rp->entryname=NULL;
	}

#if 1
	if(rp->sname!=NULL)
	{
		free(rp->sname);
		rp->sname=NULL;
	}

	if(rp->file!=NULL)
	{
		free(rp->file);
		rp->file=NULL;
	}

	if(rp->hostname!=NULL)
	{
		free(rp->hostname);
		rp->hostname=NULL;
	}

	if(rp->dns_domain!=NULL)
	{
		free(rp->dns_domain);
		rp->dns_domain=NULL;
	}

	if(rp->siaddr!=NULL)
	{
		free(rp->siaddr);
		rp->siaddr=NULL;
	}
#endif

	if(rp->ip_addr!=NULL)
	{
		free(rp->ip_addr);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof( struct in_addr));
#endif
		rp->ip_addr=NULL;
	}

	if(first_free==0 && rp->subnet_mask!=NULL)
	{
		free(rp->subnet_mask);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof( struct in_addr));
#endif
		rp->subnet_mask=NULL;
	}

#if 1
	if(rp->brdcast_addr!=NULL)
	{
		free(rp->brdcast_addr);
		rp->brdcast_addr=NULL;
	}
#endif

	if(first_free==0 && rp->router!=NULL)
	{
		for(i=0;i<rp->router->num;i++)
		{
			if(rp->router->addr!=NULL)
			{
				free(rp->router->addr);
#ifdef ECOS_DBG_STAT
				dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof( struct in_addr));
#endif
				rp->router->addr=NULL;
			}
		}
		free(rp->router);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct in_addrs));
#endif
		rp->router=NULL;
	}
	
#if 1
	if(rp->time_server!=NULL)
	{
		for(i=0;i<rp->time_server->num;i++)
		{
			if(rp->time_server->addr!=NULL)
			{
				free(rp->time_server->addr);
				rp->time_server->addr=NULL;
			}
		}
		free(rp->time_server);
		rp->time_server=NULL;
	}

	if(rp->name_server!=NULL)
	{
		for(i=0;i<rp->name_server->num;i++)
		{
			if(rp->name_server->addr!=NULL)
			{
				free(rp->name_server->addr);
				rp->name_server->addr=NULL;
			}
		}
		free(rp->name_server);
		rp->name_server=NULL;
	}
#endif

	if(first_free==0 && rp->dns_server!=NULL)
	{
		for(i=0;i<rp->dns_server->num;i++)
		{
			if(rp->dns_server->addr!=NULL)
			{
				free(rp->dns_server->addr);
#ifdef ECOS_DBG_STAT
				dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct in_addr));
#endif
				rp->dns_server->addr=NULL;
			}
		}
		free(rp->dns_server);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct in_addrs));
#endif

		rp->dns_server=NULL;
	}
	
#if 1
	if(rp->policy_filter!=NULL)
	{
		for(i=0;i<rp->policy_filter->num;i++)
		{
			if(rp->policy_filter->addr1!=NULL)
			{
				free(rp->policy_filter->addr1);
				rp->policy_filter->addr1=NULL;
			}
			if(rp->policy_filter->addr2!=NULL)
			{
				free(rp->policy_filter->addr2);
				rp->policy_filter->addr2=NULL;
			}
		}
		free(rp->policy_filter);
		rp->policy_filter=NULL;
	}
		
	if(rp->static_route!=NULL)
	{
		for(i=0;i<rp->static_route->num;i++)
		{
			if(rp->static_route->addr1!=NULL)
			{
				free(rp->static_route->addr1);
				rp->static_route->addr1=NULL;
			}
			if(rp->static_route->addr2!=NULL)
			{
				free(rp->static_route->addr2);
				rp->static_route->addr2=NULL;
			}
		}
		free(rp->static_route);
		rp->static_route=NULL;
	}
#endif

	if(bp_call_rp==0 && rp->binding!=NULL)
	{
		rp_call_bp=1;
		free_bp(rp->binding);
		rp->binding=NULL;
		rp_call_bp=0;
	}
	free(rp);
	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct dhcp_resource));
#endif
	rp=NULL;
	first_free++;
	return ;
}

void free_hash_table(struct hash_tbl *htbl, void (*free_data)())
{
	if(htbl==NULL)
		return;
	
	int i;
	struct hash_member *tmphmbr=NULL;
	
	for(i=0;i<HASHTBL_SIZE;i++)
	{
		while((htbl->head)[i]!=NULL)
		{
			if(((htbl->head)[i])->data!=NULL)
			{
				if(free_data!=NULL)
					(*free_data)(((htbl->head)[i])->data);
				
				((htbl->head)[i])->data=NULL;
			}
			tmphmbr=((htbl->head)[i]);
			(htbl->head)[i]=((htbl->head)[i])->next;
			//diag_printf("[%d]:%p\n", i, *tmphmbr);
			free(tmphmbr);	
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct hash_member));
#endif
			tmphmbr=NULL;
		}
	}
	return ;
}

void free_hash_member(struct hash_member *hmbr, void (*free_data)())
{
	if(hmbr==NULL)
		return ;
	struct hash_member *tmphmbr=NULL;
	while(hmbr!=NULL)
	{
		tmphmbr=hmbr;
		hmbr=hmbr->next;
		
		if(tmphmbr->data!=NULL)
		{
			if(free_data!=NULL)
				(*free_data)(tmphmbr->data);
			
			tmphmbr->data=NULL;
		}
		free(tmphmbr);
	
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct hash_member));
#endif
		tmphmbr=NULL;
	}
	return ;
}

void free_dhcpd_res()
{		
	free_hash_member(reslist, free_rp);
	
	free_hash_table(&nmhashtable, NULL);
	
	free_hash_table(&iphashtable, NULL);
	
//	free_hash_member(bindlist, free_bp);
	free_hash_member(bindlist, NULL);
	free_hash_table(&cidhashtable, NULL);		
	free_hash_table(&relayhashtable, NULL);		
		
	reslist=NULL;
	bindlist=NULL;	
	return ;
}

void dhcpd_restart(cyg_uint8 flag)
{
	dhcpd_restart_flag=flag;
	return ;
}

void free_sbuf()
{
	if(sbufp!=NULL)
	{
		free(sbufp);
		
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, ETHERHL + IPHL + UDPHL + DFLTDHCPLEN + QWOFF);
#endif
		sbufp=NULL;
		sbufvec[0].iov_base=NULL;
		snd.ether=NULL;
		snd.ip=NULL;
		snd.udp=NULL;
		snd.dhcp=NULL;
	}
	return ;
}

void close_if(struct if_info * ifinfo)
{
	if(ifinfo==NULL)
		return ;	

	if(ifinfo->fd>0)
	{
		close(ifinfo->fd);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
	}
	
	if(ifinfo->subnetmask!=NULL)
	{
		free(ifinfo->subnetmask);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct in_addr));
#endif
		ifinfo->subnetmask=NULL;
	}
	
	if(ifinfo->ipaddr!=NULL)
	{
		free(ifinfo->ipaddr);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct in_addr));
#endif
		ifinfo->ipaddr=NULL;
	}

	if(ifinfo->buf!=NULL)
	{
		free(ifinfo->buf);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, ifinfo->buf_size);
#endif
		ifinfo->buf=NULL;
	}

	free(ifinfo);	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct if_info));
#endif
	ifinfo=NULL;	
	return ;
}

void close_file()
{
	if(addrpool_dbfp!=NULL)
	{
		fclose(addrpool_dbfp);	
		
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_FILE, DBG_ACTION_DEL, 0);
#endif
		addrpool_dbfp=NULL;
	}
	
	if(binding_dbfp!=NULL)
	{
		fclose(binding_dbfp);
		
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_FILE, DBG_ACTION_DEL, 0);
#endif
		binding_dbfp=NULL;
	}
	
	if(dump_fp!=NULL)
	{
		fclose(dump_fp);
		
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_FILE, DBG_ACTION_DEL, 0);
#endif
		dump_fp=NULL;
	}
	return;
}
#endif

int start_dhcpd(cyg_addrword_t data)
{
	int n = 0;
	struct if_info *ifp = NULL;          /* pointer to interface */
	char *option = NULL;                 /* command line option */
	char msgtype;                        /* DHCP message type */
	struct sockaddr_in my_addr, any_addr;
	struct ifreq ifreq;	

DHCPD_RESTART:

	bzero(&ifreq, sizeof(ifreq));

	debug = 0;                           /* debug flag */
//	srand(getpid());
	dhcpd_running=1;

	if ((ifp = (struct if_info *) calloc(1, sizeof(struct if_info))) == NULL) 
	{
		perror("calloc error in main()");
		exit(1);
	}
	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, sizeof(struct if_info));
#endif
	
//	diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
	
	ifp->next = if_list;
	if_list = ifp;
	strcpy(ifp->name, inf_name);

	if (if_list == NULL) 
		usage();
	
	init_db();               /* initialize databases */
	read_addrpool_db();      /* read address pool database */
//	read_bind_db();          /* read binding database */
#ifdef SUPPORT_RELAY
	read_relay_db();         /* read relay agents database */
#endif

#ifdef SUPPORT_MAC_FILTER
	read_mac_db();           /* read MAC address database */
#endif

#ifdef CONFIG_RTL_819X
	close_file();	
#endif

//	set_sighand();           /* Set up signal handlers */
	set_srvport();           /* determine dhcp port */

	ifp = if_list;
	
	while (ifp != NULL) 
	{
		if (open_if(ifp, inf_name) < 0) 
		{
			exit(1);
		}		
		ifp = ifp->next;
	}
	/* buffer allocation for sending */
	alloc_sbuf();	
	/****************************
	* Main loop                *
	* Process incoming message *
	****************************/
//	set_default_route(inf_name);
	if(dhcpd_restart_flag == 2 || dhcpd_restart_flag == 0)
	{
		kick_event(LAN_EVENT); //set LAN_EVENT
		//set_event_flag(&lan_event_flag, 1);
	}
	dhcpd_restart_flag = 0;
	while (1) 
	{
		garbage_collect();

#ifdef CONFIG_RTL_819X
		if(dhcpd_restart_flag>0 || dhcpd_quitting==1)
		{
			free_dhcpd_res();
//			dhcpd_restart_flag=0;
			free_sbuf();			
			
//			for(ifp=if_list; ifp!=NULL; ifp=ifp->next)
			close_if(if_list);
			
			ifp=if_list=NULL;	
			
//			close_file();

			nbind=0;

			if(dhcpd_quitting==1)
			{
				dhcpd_quitting=0;
				return 0;
			}
				
			goto DHCPD_RESTART;
		}
#endif		
		/* select and read from interfaces */
		if ((ifp = read_interfaces(if_list, &n)) == NULL) 
		{
			continue;
		}
		 
		rcv.dhcp = (struct dhcp *) ifp->buf;
		rdhcplen = n;

		if (check_pkt2(rdhcplen)) 
		{
			/* process the packet */
			msgtype = 0;
			if ((option = pickup_opt(rcv.dhcp, rdhcplen, DHCP_MSGTYPE)) != NULL)
			{
				msgtype = *OPTBODY(option);
			}
			
//			diag_printf("%s:%d	msgtype=%d\n", __FUNCTION__,__LINE__,msgtype);
			
			if (msgtype < BOOTP || msgtype > DHCPINFORM)
				syslog(LOG_INFO, "unknown message");
			else if (process_msg[(int) msgtype] != NULL)
				(*process_msg[(int) msgtype])(ifp);
		}
	}
  	return(0);
}


/*********************************
 * print out usage. Never return *  
 *********************************/
static void
usage()
{
  fprintf(stderr, "Usage: dhcps ifname [ifname] ...\n");

  exit(1);
}


/*
 * print out version  
 */
static void
version()
{
  fprintf(stderr, "WIDE DHCP server version %s\n", WIDEDHCP_VERS);
  exit(0);
}


void
finish()
{
  dump_bind_db();
  exit(0);
}


void
__dhcpd_orig_restart()
{
  dump_bind_db();

  if (start_dhcpd(0)< 0) 
  {
    syslog(LOG_ERR, "execv error in restart()");
  }
}


/*
 * final check the packet
 */
static int
check_pkt2(len)
  int len;
{
  if (len < MINDHCPLEN || rcv.dhcp->op != BOOTREQUEST)
    return(FALSE);

#if 0
  if (rcv.dhcp->giaddr.s_addr == 0)    /* it should be read from bpf or nit */
    return(FALSE);
#endif

  if (bcmp(rcv.dhcp->options, magic_c, MAGIC_LEN) != 0)
    return(FALSE);

  return(TRUE);
}


/*
 * final check the packet
 */
#ifndef sun
static int
check_pkt1(ifp)
  struct if_info *ifp;
{
  if (ntohs(rcv.ip->ip_len) < MINDHCPLEN + UDPHL + IPHL)
    return(FALSE);
  if (rcv.ip->ip_dst.s_addr != 0xffffffff &&
      rcv.ip->ip_dst.s_addr != 0 &&   /* old BOOTP client */
      rcv.ip->ip_dst.s_addr != ifp->ipaddr->s_addr)
    return(FALSE);
  if (ntohs(rcv.udp->uh_ulen) < MINDHCPLEN + UDPHL)
    return(FALSE);
  if (check_ipsum(rcv.ip) == 0)
    return(FALSE);
  if (check_udpsum(rcv.ip, rcv.udp) == 0)
    return(FALSE);
  if (rcv.dhcp->op != BOOTREQUEST ||
      rcv.dhcp->giaddr.s_addr != 0)  /* it should be read from normal socket */
    return(FALSE);
  if (bcmp(rcv.dhcp->options, magic_c, MAGIC_LEN) != 0 &&
      GETHL(rcv.dhcp->options) != 0)  /* old BOOTP client */
    return(FALSE);

  return(TRUE);
}
#endif /* not sun */


/*
 * initialize databases
 */
static void
init_db()
{
  if (addrpool_db[0] == '\0') strcpy(addrpool_db, ADDRPOOL_DB);
  if ((addrpool_dbfp = fopen(addrpool_db, "r")) == NULL) {
    syslog(LOG_ERR, "Cannot open the resource database \"%s\"", addrpool_db);
    exit(1);
  }

#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_FILE, DBG_ACTION_ADD, 0);
#endif

#ifdef SUPPORT_RELAY
  if (relay_db[0] == '\0') strcpy(relay_db, RELAY_DB);
  if ((relay_dbfp = fopen(relay_db, "r")) == NULL) {
    syslog(LOG_ERR, "Cannot open the relay agent database \"%s\"", relay_db);
    exit(1);
  }
  
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_FILE, DBG_ACTION_ADD, 0);
#endif
#endif

#ifdef SUPPORT_MAC_FILTER
  if (mac_db[0] == '\0') strcpy(mac_db, MAC_DB);
  if ((mac_dbfp = fopen(mac_db, "r")) == NULL) {
    syslog(LOG_ERR, "Cannot open the MAC address database \"%s\"", mac_db);
    exit(1);
  }

#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_FILE, DBG_ACTION_ADD, 0);
#endif
#endif

  if (binding_db[0] == '\0') strcpy(binding_db, BINDING_DB);
  if ((binding_dbfp = fopen(binding_db, "r+")) == NULL &&
      (binding_dbfp = fopen(binding_db, "w+")) == NULL) {
    syslog(LOG_ERR, "Cannot open the binding database \"%s\"", binding_db);
    exit(1);
  }

#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_FILE, DBG_ACTION_ADD, 0);
#endif

  unlink(ADDRPOOL_DUMP);
  if ((dump_fp = fopen(ADDRPOOL_DUMP, "w+")) == NULL) {
    syslog(LOG_ERR, "Cannot open the resource dump file \"%s\"",ADDRPOOL_DUMP);
    exit(1);
  }

#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_FILE, DBG_ACTION_ADD, 0);
#endif
  return;
}


/*
 * allocate buffer for sending
 */
static void
alloc_sbuf()
{
  sbufvec[0].iov_len = ETHERHL + IPHL + UDPHL + DFLTDHCPLEN;

  if ((sbufp = calloc(1, sbufvec[0].iov_len + QWOFF)) == NULL) {
    syslog(LOG_ERR, "calloc error send buffer in alloc_buf()");
    exit(1);
  }
 
#ifdef ECOS_DBG_STAT
  dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, ETHERHL + IPHL + UDPHL + DFLTDHCPLEN + QWOFF);
#endif

  sbufvec[0].iov_base = (void *)&sbufp[QWOFF];
  snd.ether = (struct ether_header *) &sbufp[QWOFF];
  snd.ip = (struct ip *) &sbufp[QWOFF + ETHERHL];
  snd.udp = (struct udphdr *) &sbufp[QWOFF + ETHERHL + IPHL];
  snd.dhcp = (struct dhcp *) &sbufp[QWOFF + ETHERHL + IPHL + UDPHL];

  return;
}


/*
 * return strings which include the haddr in the form of "type:haddr...."
 */
char *
chaddrtos(chaddr)
  struct chaddr *chaddr;
{
  int i, fin;
  char tmp[4];
  static char result[MAX_HLEN*2+32];     /* it seems enough */

  char mac_addr[32];
  
  bzero(result, sizeof(result));
  if ((fin = chaddr->hlen) > MAX_HLEN)
    fin = MAX_HLEN;


sprintf(mac_addr, "%.2x", chaddr->haddr[0] & 0xff);
for(i=1; i<fin; i++)
{
	sprintf(tmp, "%.2x", chaddr->haddr[i] & 0xff);
	strcat(mac_addr, tmp);
}

//diag_printf("%s:%d mac_addr=%s\n",__FUNCTION__,__LINE__,mac_addr);

#ifdef KLD_ENABLED
char host_name[32];
memset(host_name, 0, sizeof(host_name));

if(IsclientExist("/etc/hostname_mac", mac_addr, host_name, 1)==1)
	sprintf(result, "%s:0x", host_name);
else
	sprintf(result, "%s:0x", "none");
#else
sprintf(result, "%d:0x", chaddr->htype);
#endif

strcat(result, mac_addr);

#if 0
  for (i = 0; i < fin; i++) {
    sprintf(tmp, "%.2x:", chaddr->haddr[i] & 0xff);
    strcat(result, tmp);
  }
#endif

//  diag_printf("%s:%d result=%s\n",__FUNCTION__,__LINE__,result);


  return(result);
}


/*
 * return strings which include the cid in the form of "type:id...."
 */
char *
cidtos(cid, withsubnet)
  struct client_id *cid;
  int withsubnet;
{
  int i = 0;
  static char result[MAXOPT*2+sizeof("255.255.255.255")+3];   /* it seems enough */
  char tmp[sizeof("255.255.255.255")+1];

  sprintf(result, "%d:0x", cid->idtype);
  for (i = 0; i < cid->idlen; i++) {
    sprintf(tmp, "%.2x", cid->id[i] & 0xff);
    strcat(result, tmp);
  }
  if (withsubnet) {
    sprintf(tmp, ":%s", inet_ntoa(cid->subnet));
    strcat(result, tmp);
  }

  return(result);
}


#ifndef CONFIG_RTL_819X
/*
 * set signal handlers
 */
static void
set_sighand()
{
  sigset_t sigset;

  sigemptyset(&sigset);
  sigaddset(&sigset, SIGHUP);
  if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
    syslog(LOG_ERR, "sigprocmask error in set_sighand");
    exit(1);
  }

  if ((int) signal(SIGINT, finish) < 0) {
    syslog(LOG_ERR, "cannot set signal handler(SIGINT)");
    exit(1);
  }
  if ((int) signal(SIGTERM, finish) < 0) {
    syslog(LOG_ERR, "cannot set signal handler(SIGTERM)");
    exit(1);
  }
  if ((int) signal(SIGUSR1, dump_bind_db) < 0) {
    syslog(LOG_ERR, "cannot set signal handler(SIGUSR1)");
    exit(1);
  }
  if ((int) signal(SIGUSR2, dump_addrpool_db) < 0) {
    syslog(LOG_ERR, "cannot set signal handler(SIGUSR2)");
    exit(1);
  }
  if (debug == 0) {
    if ((int) signal(SIGHUP, __dhcpd_orig_restart) < 0) {
      syslog(LOG_ERR, "cannot set signal handler(SIGHUP)");
      exit(1);
    }
  }

  return;
}


/*
 * become daemon
 */
static void
become_daemon()
{
  int n = 0;

  /*
   * go into background and disassociate from controlling terminal
   */
  if(fork() != 0) exit(0);
  for (n = 0; n < 64; n++)
    close(n);
  chdir("/");
#ifdef SIGTTOU
  signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
  signal(SIGTTIN, SIG_IGN);
#endif
  setsid();
}
#endif

/*
 * return requested lease in host byte order
 */
static u_Long
get_reqlease(msg, length)
  struct dhcp *msg;
  int length;
{
  char *option = NULL;
  u_Long retval = 0;

  if ((option = pickup_opt(msg, length, LEASE_TIME)) != NULL) {
    retval = GETHL(OPTBODY(option));
  }

  return(retval);
}


/*
 * get client identifier (if there is no cid, substitute chaddr)
 */
static void
get_cid(msg, length, cid)
  struct dhcp *msg;
  int length;
  struct client_id *cid;
{
  char *option = NULL;

  if ((option = pickup_opt(msg, length, CLIENT_ID)) != NULL) {
    cid->idlen = ((int) OPTLEN(option)) - 1;  /* -1 == identifier type */
    cid->id = (OPTBODY(option) + 1);
    cid->idtype = *OPTBODY(option);
  } else {      /* haddr is used to substitute for client identifier */
    cid->idlen = msg->hlen;
    cid->idtype = msg->htype;
    cid->id = msg->chaddr;
  }

  return;
}


/*
 * get maximum length of option field
 */
static int
get_maxoptlen(msg, length)
  struct dhcp *msg;
  int length;
{
  char *option = NULL;
  int retval = DFLTOPTLEN;

  if ((option = pickup_opt(msg, length, DHCP_MAXMSGSIZE)) != NULL) {
    retval = GETHS(OPTBODY(option)) - IPHL - UDPHL - DFLTDHCPLEN + DFLTOPTLEN;
  }

  /* XXX assume default mtu */
  if (retval - DFLTOPTLEN + DFLTDHCPLEN + UDPHL + IPHL > ETHERMTU) {
    retval = ETHERMTU - IPHL - UDPHL - DFLTDHCPLEN + DFLTOPTLEN;
  }

  return(retval);
}


/*
 * get subnet number
 */
static int
get_subnet(msg, length, subn, ifp)
  struct dhcp *msg;
  int length;
  struct in_addr *subn;
  struct if_info *ifp;
{
  char *option = NULL;
  struct relay_acl *acl = NULL;
  struct dhcp_resource *res = NULL;

  if (msg->ciaddr.s_addr != 0) {
    if ((option = pickup_opt(msg, length, SUBNET_MASK)) != NULL) {
      subn->s_addr = msg->ciaddr.s_addr & htonl(GETHL(OPTBODY(option)));
      return(0);
    }
    else if ((res = (struct dhcp_resource *)
	      hash_find(&iphashtable, (char *) &msg->ciaddr.s_addr,
			sizeof(u_Long), resipcmp, &msg->ciaddr)) == NULL) {
      errno = 0;
      syslog(LOG_INFO, "Can't find corresponding resource in get_subnet()");
    } else {
      subn->s_addr = msg->ciaddr.s_addr & res->subnet_mask->s_addr;
//      diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
      return(0);
    }
  }
  if (msg->giaddr.s_addr != 0) {
    if ((option = pickup_opt(msg, length, SUBNET_MASK)) != NULL) {
      subn->s_addr = msg->giaddr.s_addr & htonl(GETHL(OPTBODY(option)));
      return(0);
    }
    else if ((acl = (struct relay_acl *)
	      hash_find(&relayhashtable, (char *) &msg->giaddr,
			sizeof(struct in_addr), relayipcmp,
			&msg->giaddr)) == NULL) {
      errno = 0;
      syslog(LOG_INFO, "relayed packet come from invalid relay agent(%s).",
	     inet_ntoa(msg->giaddr));
      return(-1);
    } else {
      subn->s_addr = (acl->relay.s_addr & acl->subnet_mask.s_addr);
      return(0);
    }
  }
//  diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
  subn->s_addr = ifp->ipaddr->s_addr & ifp->subnetmask->s_addr;
  return(0);
}


/*
 * get subnet mask
 */
static int
get_snmk(msg, length, subn, ifp)
  struct dhcp *msg;
  int length;
  struct in_addr *subn;
  struct if_info *ifp;
{
  struct relay_acl *acl = NULL;

  if (msg->giaddr.s_addr != 0) {
    if ((acl = (struct relay_acl *)
	 hash_find(&relayhashtable, (char *) &msg->giaddr,
		   sizeof(struct in_addr), relayipcmp,
		   &msg->giaddr)) == NULL) {
      errno = 0;
      syslog(LOG_INFO, "relayed packet come from invalid relay agent(%s).",
	     inet_ntoa(msg->giaddr));
      return(-1);
    } else {
      subn->s_addr = acl->subnet_mask.s_addr;
      return(0);
    }
  }

  subn->s_addr = ifp->subnetmask->s_addr;
  return(0);
}


/*
 * return true if resource is available
 */
static int
available_res(res, cid, curr_epoch)
  struct dhcp_resource *res;
  struct client_id *cid;
  time_t curr_epoch;
{
  return(res->binding == NULL ||
	 (res->binding->expire_epoch != 0xffffffff &&
	  res->binding->expire_epoch < curr_epoch &&
	  res->binding->temp_epoch < curr_epoch) ||
	 cidcmp(&res->binding->cid, cid));
}


static int
cidcopy(src, dst)
  struct client_id *src;
  struct client_id *dst;
{
  dst->subnet.s_addr = src->subnet.s_addr;
  dst->idtype = src->idtype;
  dst->idlen = src->idlen;
  if (dst->id != NULL) 
  {
  	free(dst->id);	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, dst->idlen);
#endif
  }
  if ((dst->id = (char *) calloc(1, src->idlen)) == NULL) {
    syslog(LOG_WARNING, "calloc error in cidcopy()");
    return(-1);
  }
 
#ifdef ECOS_DBG_STAT
   dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, src->idlen);
#endif

  bcopy(src->id, dst->id, src->idlen);

  return(0);
}


/*
 * choose lease duration
 */
static int
choose_lease(reqlease, curr_epoch, offer_res)
  u_Long reqlease;
  time_t curr_epoch;
  struct dhcp_resource *offer_res;
{
  u_Long offer_lease = 0;

  if (isset(offer_res->valid, S_CLIENT_ID)) {
    offer_lease = 0xffffffff;
  }
  else if (reqlease != 0) {
    if (reqlease <= offer_res->max_lease)
      offer_lease = reqlease;
    else
      offer_lease = offer_res->max_lease;
  }
  else if (offer_res->binding == NULL) {
    offer_lease = offer_res->default_lease;
  } else {
    if (offer_res->binding->expire_epoch == 0xffffffff) {
      offer_lease = 0xffffffff;
    }
    else if ((offer_res->binding->expire_epoch > curr_epoch) &&
	     (offer_res->binding->expire_epoch - curr_epoch >
	      offer_res->default_lease / 2)) {
      offer_lease = offer_res->binding->expire_epoch - curr_epoch;
    } else {
      offer_lease = offer_res->default_lease;
    }
  }

  return(offer_lease);
}


static void clean_sbuf()
{
  if (sbufvec[0].iov_base != NULL)
    bzero(sbufvec[0].iov_base, sbufvec[0].iov_len);
  if (sbufvec[1].iov_base != NULL)
    bzero(sbufvec[1].iov_base, sbufvec[1].iov_len);
  return;
}


/*
 * choose resource with client identifier
 */
static struct dhcp_resource *
select_wcid(msgtype, cid, curr_epoch)
  int msgtype;
  struct client_id *cid;
  time_t curr_epoch;
{
  struct dhcp_binding *binding = NULL;

  if ((binding = (struct dhcp_binding *)
       hash_find(&cidhashtable, cid->id, cid->idlen, bindcidcmp, cid)) != NULL) {
    /*
     * Is the resource used ?
     */
    if (available_res(binding->res, cid, curr_epoch)) {
      if (icmp_check(msgtype, binding->res->ip_addr) == GOOD) {
	return(binding->res);
      } else {
	turnoff_bind(binding);
	return(NULL);
      }
    }
  }

  return(NULL);
}


/*
 * choose resource with requested IP
 */
static struct dhcp_resource *
select_wreqip(msgtype, cid, curr_epoch)
  int msgtype;
  struct client_id *cid;
  time_t curr_epoch;
{
  char *option = NULL;
  char tmp[sizeof("255.255.255.255")];
  struct dhcp_resource *res = NULL;
  struct in_addr reqip;

  bzero(tmp, sizeof(tmp));
  bzero(&reqip, sizeof(reqip));

  option = pickup_opt(rcv.dhcp, rdhcplen, REQUEST_IPADDR);
  if(option!=NULL || flag_addclient>0)
//  if ((option = pickup_opt(rcv.dhcp, rdhcplen, REQUEST_IPADDR)) != NULL) 
  {
  	flag_addclient=0;
  	if(option!=NULL)
    		reqip.s_addr = htonl(GETHL(OPTBODY(option)));
	else		
		reqip.s_addr= addclient_ip;
	
    	if ((res = (struct dhcp_resource *)hash_find(&iphashtable, (char *) &reqip.s_addr,
			 sizeof(u_Long), resipcmp, &reqip)) == NULL) 
	{
      errno = 0;
      syslog(LOG_INFO, "No such IP addr %s in address pool", inet_ntoa(reqip));
      return(NULL);
    } else {      /* find out */
      /* check the subnet number */
      if (cid->subnet.s_addr != (res->ip_addr->s_addr & res->subnet_mask->s_addr)) {
	strcpy(tmp, inet_ntoa(reqip));
	errno = 0;
	syslog(LOG_INFO,
	       "DHCP%s(from cid:\"%s\"): invalid requested IP addr %s (different subnet)",
	       (msgtype == DHCPDISCOVER) ? "DISCOVER" : "REQUEST",
	       cidtos(cid, 1), tmp);
	return(NULL);
      }

      /* is it manual allocation ? */
      if (isset(res->valid, S_CLIENT_ID)) {

	/* is there corresponding binding ? */
	if (res->binding == NULL) {
	  strcpy(tmp, inet_ntoa(reqip));
	  errno = 0;
	  syslog(LOG_INFO,
		 "DHCP%s(from cid:\"%s\"): No corresponding binding for %s",
		 (msgtype == DHCPDISCOVER) ? "DISCOVER" : "REQUEST",
		 cidtos(cid, 1), tmp);
	  return(NULL);
	}

	/* for the client ? */
	if (cidcmp(&res->binding->cid, cid) != TRUE) {
	  strcpy(tmp, inet_ntoa(reqip));
	  errno = 0;
	  syslog(LOG_INFO,
		 "DHCP%s(from cid:\"%s\"): requested IP addr %s is not for the client",
		 (msgtype == DHCPDISCOVER) ? "DISCOVER" : "REQUEST",
		 cidtos(cid, 1), tmp);
	  return(NULL);
	}

	/* is there no host which has the same IP ? */
	if (icmp_check(msgtype, res->ip_addr) == GOOD) {
	  return(res);
	} else {
	  turnoff_bind(res->binding);
	  return(NULL);
	}
      }

      /* is the requested IP available ? */
      else if (available_res(res, cid, curr_epoch)) {
	if (icmp_check(msgtype, res->ip_addr) == GOOD) {
	  return(res);
	} else {
	  turnoff_bind(res->binding);
	  return(NULL);
	}
      }
    }
  }
  return(NULL);
}


/*
 * choose a new address
 *         firstly, choose (never used && reqlease > max_lease),
 *         if not found, choose (LRU && reqlease > max_lease),
 *         if not found, choose ((never used || LRU) && maximum(max_lease)),
 *         if not found, choose the first available one.
 */
static struct dhcp_resource *
select_newone(msgtype, cid, curr_epoch, reqlease)
  int msgtype;
  struct client_id *cid;
  time_t curr_epoch;
  u_Long reqlease;
{
  struct dhcp_resource *res = NULL,
                       *best = NULL;
  struct hash_member *resptr = NULL;
  resptr = reslist;
  while (resptr != NULL) {
//    diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
    res = (struct dhcp_resource *) resptr->data;

    /* if it is dummy entry, skip it */
    if (res->ip_addr == NULL) {
//      diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
      resptr = resptr->next;
      continue;
    }

    /* check the resource. valid subnet?, available resource? */
    if (cid->subnet.s_addr ==
	(res->ip_addr->s_addr & res->subnet_mask->s_addr) &&
	available_res(res, cid, curr_epoch)) {
//	diag_printf("%s:%d\n", __FUNCTION__,__LINE__);

      /*
       * choose the best one
       */
      if (best == NULL) {
	if (icmp_check(msgtype, res->ip_addr) == GOOD)
	{
//	  diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
	  best = res;
	}
	else
	  turnoff_bind(res->binding);
	resptr = resptr->next;
	continue;
      }

      if (best->binding == NULL && res->binding != NULL) {
	resptr = resptr->next;
	continue;
      }
      else if (best->binding != NULL && res->binding == NULL) {
	if (icmp_check(msgtype, res->ip_addr) == GOOD)
	  best = res;
	else
	  turnoff_bind(res->binding);
	resptr = resptr->next;
	continue;
      }

      if (best->allow_bootp == FALSE && res->allow_bootp == TRUE) {
	resptr = resptr->next;
	continue;
      }
      else if (best->allow_bootp == TRUE && res->allow_bootp == FALSE) {
	if (icmp_check(msgtype, res->ip_addr) == GOOD)
	  best = res;
	else
	  turnoff_bind(res->binding);
	resptr = resptr->next;
	continue;
      }

      if ((best->binding == NULL && res->binding == NULL) ||
	  (best->binding != NULL && res->binding != NULL &&
	   best->binding->expire_epoch == res->binding->expire_epoch)) {
	if (reqlease >= res->max_lease && res->max_lease > best->max_lease) {
	  if (icmp_check(msgtype, res->ip_addr) == GOOD)
	    best = res;
	  else
	    turnoff_bind(res->binding);
	  resptr = resptr->next;
	  continue;
	}

	if (reqlease != INFINITY && reqlease <= res->max_lease &&
	    res->max_lease < best->max_lease) {
	  if (icmp_check(msgtype, res->ip_addr) == GOOD)
	    best = res;
	  else
	    turnoff_bind(res->binding);
	  resptr = resptr->next;
	  continue;
	}

	if (reqlease != INFINITY && res->max_lease >= reqlease &&
	    reqlease > best->max_lease) {
	  if (icmp_check(msgtype, res->ip_addr) == GOOD)
	    best = res;
	  else
	    turnoff_bind(res->binding);
	  resptr = resptr->next;
	  continue;
	}

	resptr = resptr->next;
	continue;
      }

      if (best->binding != NULL && res->binding != NULL &&
	  best->binding->expire_epoch > res->binding->expire_epoch) {
	if (icmp_check(msgtype, res->ip_addr) == GOOD)
	  best = res;
	else
	  turnoff_bind(res->binding);
	resptr = resptr->next;
	continue;
      } else {
	resptr = resptr->next;
	continue;
      }
    }

    resptr = resptr->next;
  }

  return(best);
}


/*
 * choose the resource
 */
static struct dhcp_resource *
choose_res(cid, curr_epoch, reqlease)
  struct client_id *cid;
  time_t curr_epoch;
  u_Long reqlease;
{
  struct dhcp_resource *res = NULL;

  /* 1. select with client identifier */
  if ((res = select_wcid(DHCPDISCOVER, cid, curr_epoch)) != NULL)
    return(res);

  /* 2. select with requested IP */
  if ((res = select_wreqip(DHCPDISCOVER, cid, curr_epoch)) != NULL)
    return(res);

  /* 3. select the address newly */
  if ((res = select_newone(DHCPDISCOVER, cid, curr_epoch, reqlease)) != NULL)
    return(res);

  errno = 0;
  syslog(LOG_WARNING, "DHCPDISCOVER: No more available address in the pool");
  return(NULL);
}
  
/*
 * update binding database, and hashtable
 */
static int
update_db(msgtype, cid, res, lease, curr_epoch)
  int msgtype;
  struct client_id *cid;
  struct dhcp_resource *res;
  u_Long lease;
  time_t curr_epoch;
{
  struct dhcp_binding *binding = NULL;

  if (res->binding != NULL && (res->binding->flag & STATIC_ENTRY) != 0)
    return(0);

  if (res->binding != NULL) {
    hash_del(&cidhashtable, res->binding->cid.id, res->binding->cid.idlen,
	     bindcidcmp, &res->binding->cid, free_bind);
  }
  if ((binding = (struct dhcp_binding *)
       calloc(1, sizeof(struct dhcp_binding))) == NULL) {
    syslog(LOG_WARNING, "calloc error in update_db()");
    return(-1);
  }
  
#ifdef ECOS_DBG_STAT
  dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, sizeof(struct dhcp_binding));
#endif

  if (cidcopy(cid, &binding->cid) != 0)
  {		
  #ifdef CONFIG_RTL_819X
  		if(binding!=NULL)
  		{
  			free(binding);
			binding=NULL;
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct dhcp_binding));
#endif			
  		}
	#endif	
    return(-1);
  }
  if (msgtype == DHCPDISCOVER) {
    binding->temp_epoch = curr_epoch + MEMORIZE;
  } else {
    if (lease == 0xffffffff) {
      binding->expire_epoch = 0xffffffff;
    } else {
      binding->expire_epoch = curr_epoch + lease;
    }
  }
  binding->res = res;
  if (binding->res_name != NULL) {
    free(binding->res_name);
	#ifdef CONFIG_RTL_819X
	binding->res_name=NULL;
	#endif
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, strlen(binding->res_name)+1);
#endif
  }
  if ((binding->res_name = (char *) calloc(1, strlen(res->entryname) + 1)) == NULL) {
    syslog(LOG_WARNING, "calloc error in update_db()");
	#ifdef CONFIG_RTL_819X
	if(binding!=NULL)
	{		
		free(binding);
		binding=NULL;
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct dhcp_binding));
#endif
	}
	#endif
    return(-1);
  }
  
#ifdef ECOS_DBG_STAT
  dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, strlen(res->entryname) + 1);
#endif

  strcpy(binding->res_name, res->entryname);
  res->binding = binding;

  /* record chaddr */
  binding->haddr.htype = rcv.dhcp->htype;
  binding->haddr.hlen = rcv.dhcp->hlen;
  if (binding->haddr.haddr != NULL) {
    free(binding->haddr.haddr);
	#ifdef CONFIG_RTL_819X
	binding->haddr.haddr=NULL;
	#endif
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, binding->haddr.hlen);
#endif
  }
  if ((binding->haddr.haddr = calloc(1, binding->haddr.hlen)) == NULL) {
    syslog(LOG_WARNING, "calloc error in update_db()");
	#if 0 //def CONFIG_RTL_819X
	if (binding->res_name != NULL) {
	    free(binding->res_name);
		binding->res_name=NULL;
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, strlen(res->entryname) + 1);
#endif
  	}

	if(binding!=NULL)
	{
		free(binding);
		binding=NULL;
		
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct dhcp_binding));
#endif	
	}
	#endif
    return(-1);
  }
 
#ifdef ECOS_DBG_STAT
  dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, binding->haddr.hlen);
#endif	

  bcopy(rcv.dhcp->chaddr, binding->haddr.haddr, rcv.dhcp->hlen);

  if ((hash_ins(&cidhashtable, binding->cid.id, binding->cid.idlen, bindcidcmp,
		&binding->cid, binding) < 0)) {
    errno = 0;
    syslog(LOG_WARNING,
	   "hash_ins() with client identifier failed in update_db()");
		#if 0 //def CONFIG_RTL_819X	
		if(binding->haddr.haddr!=NULL)
		{
			free(binding->haddr.haddr);
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, binding->haddr.hlen);
#endif
			binding->haddr.haddr=NULL;
		}
		
		if (binding->res_name != NULL) {
		    free(binding->res_name);
#ifdef ECOS_DBG_STAT
		    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, strlen(binding->res_name)+1);
#endif
		    binding->res_name=NULL;
  		}

		if(binding!=NULL)
		{
			free(binding);
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct dhcp_binding));
#endif	
			binding=NULL;
		}
		#endif
    return(-1);
  }
  if (add_bind(binding) != 0)
  {
	#if 0 //def CONFIG_RTL_819X
  	
	if(binding->haddr.haddr!=NULL)
	{
		free(binding->haddr.haddr);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, binding->haddr.hlen);
#endif
		binding->haddr.haddr=NULL;
	}

	if (binding->res_name != NULL) 
	{
	    free(binding->res_name);
#ifdef ECOS_DBG_STAT
	    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, strlen(binding->res_name)+1);
#endif
	    binding->res_name=NULL;
	}

	if(binding!=NULL)
	{
		free(binding);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct dhcp_binding));
#endif	
		binding=NULL;
	}
	#endif
   	 return(-1);
  }

  return(0);
}


static void
turnoff_bind(binding)
  struct dhcp_binding *binding;
{
#if 0 //def CONFIG_RTL_819X
	return;
#endif
  static u_short magic = 0;
  time_t curr_epoch = 0;

  if (binding == NULL)
    return;

  if (time(&curr_epoch) == -1) {
    syslog(LOG_WARNING, "time() error in turnoff_bind()");
    return;
  }

  /* insert dummy binding to avoid choosing same IP address */
  binding->expire_epoch = binding->temp_epoch = curr_epoch + 180;
  hash_del(&cidhashtable, binding->cid.id, binding->cid.idlen, bindcidcmp,
	   &binding->cid, NULL);
  if (binding->flag & STATIC_ENTRY)
    binding->cid.subnet.s_addr = magic++;
  else
    binding->cid.subnet.s_addr = ~0; 
  
  binding->flag &= ~COMPLETE_ENTRY;
  if (hash_ins(&cidhashtable, binding->cid.id, binding->cid.idlen, bindcidcmp,
	       &binding->cid, binding) < 0) {
    syslog(LOG_WARNING,
	   "hash_ins() with client identifier failure in turnoff_bind()");
  }

  return;
}


/*
 * construct dhcp message
 */
static void
construct_msg(msgtype, res, lease, ifp)
  u_char msgtype;
  struct dhcp_resource *res;
  u_Long lease;
  struct if_info *ifp;
{
  int i = 0;
  int reqoptlen = 0;
  u_Long tmp = 0;
  char *reqopt = NULL,
       inserted[32],
       *option = NULL;
	FILE *fp;
	char domain_name[32];
	
  bzero(inserted, sizeof(inserted));
  clean_sbuf();
  snd.dhcp->op = BOOTREPLY;
  snd.dhcp->htype = rcv.dhcp->htype;
  snd.dhcp->hlen = rcv.dhcp->hlen;
  snd.dhcp->hops = 0;
  snd.dhcp->xid = rcv.dhcp->xid;
  snd.dhcp->secs = 0;
  snd.dhcp->flags = rcv.dhcp->flags;
  snd.dhcp->giaddr.s_addr = rcv.dhcp->giaddr.s_addr;
  bcopy(rcv.dhcp->chaddr, snd.dhcp->chaddr, rcv.dhcp->hlen);

  if (msgtype == DHCPACK)
    snd.dhcp->ciaddr.s_addr = rcv.dhcp->ciaddr.s_addr;

  if (msgtype != DHCPNAK) {
    snd.dhcp->yiaddr.s_addr = res->ip_addr->s_addr;
    if (isset(res->valid, S_SIADDR)) {
      snd.dhcp->siaddr.s_addr = res->siaddr->s_addr;
    } else {
      snd.dhcp->siaddr.s_addr = 0;
    }
    overload = BOTH_AREOPT;
    if (isset(res->valid, S_SNAME)) {
      strncpy(snd.dhcp->sname, res->sname, MAX_SNAME);
      snd.dhcp->sname[MAX_SNAME - 1] = '\0';  /* NULL terminate certainly */
      overload -= SNAME_ISOPT;
    }
    if (isset(res->valid, S_FILE)) {
      strncpy(snd.dhcp->file, res->file, MAX_FILE);
      snd.dhcp->file[MAX_FILE - 1] = '\0';    /* NULL terminate cerntainly */
      overload -= FILE_ISOPT;
    }
  } else {
    snd.dhcp->ciaddr.s_addr = 0;
  }

  /* insert magic cookie */
  bcopy(magic_c, snd.dhcp->options, MAGIC_LEN);
  off_options = MAGIC_LEN;
  off_extopt = 0;

  /* insert dhcp message type option */
  snd.dhcp->options[off_options++] = DHCP_MSGTYPE;
  snd.dhcp->options[off_options++] = 1;
  snd.dhcp->options[off_options++] = msgtype;

  if (msgtype == DHCPNAK) {
    SETBRDCST(snd.dhcp->flags);
    if ((option = pickup_opt(rcv.dhcp, rdhcplen, CLIENT_ID)) != NULL) {
      snd.dhcp->options[off_options++] = CLIENT_ID;
      snd.dhcp->options[off_options++] = OPTLEN(option);
      bcopy(option, &snd.dhcp->options[off_options], OPTLEN(option));
      off_options += OPTLEN(option);
    }
    return;
  }
  
  /* insert "server identifier" */
  snd.dhcp->options[off_options++] = SERVER_ID;
  snd.dhcp->options[off_options++] = 4;
  bcopy(&ifp->ipaddr->s_addr, &snd.dhcp->options[off_options], 4);
  off_options += 4;

  /* insert "subnet mask" */
  if (insert_opt(res, lease, SUBNET_MASK, inserted, PASSIVE) == E_NOMORE) {
    errno = 0;
    syslog(LOG_INFO, "No more space left to insert options to DHCP%s",
	   (msgtype == DHCPOFFER) ? "OFFER" : "ACK");
  }

  /* insert "lease duration" */
  tmp = htonl(lease);
  snd.dhcp->options[off_options++] = LEASE_TIME;
  snd.dhcp->options[off_options++] = 4;
  bcopy(&tmp, &snd.dhcp->options[off_options], 4);
  off_options += 4;

	/*insert domain name*/
	fp=fopen("/etc/domain_name","r");
	if(fp!=NULL){
		snd.dhcp->options[off_options++] = DNS_DOMAIN;
		fscanf(fp,"%s",domain_name);
		fclose(fp);
  		snd.dhcp->options[off_options++] = strlen(domain_name);
  		bcopy(domain_name, &snd.dhcp->options[off_options], strlen(domain_name));
  		off_options += strlen(domain_name);		
	}
#ifdef KLD_ENABLED
	/*insert dns server addr*/
	if(lanDnsFromWanDhcpFlag>0)
	{
		fp=fopen("/etc/wan_info","r");
		if(fp!=NULL)
		{
			char linebuf[32];
			char *pstr=NULL;
			struct in_addr dns_addr[3];
			int i=0, j;
			while(fgets(linebuf, sizeof(linebuf), fp) && i<3)	
			{
				j=0;
				while(linebuf[j]!='\n' && linebuf[j]!='\0')
					j++;
				linebuf[j]=0;
				
				if((pstr=strstr(linebuf, "dns"))!=NULL)
				{
					inet_aton(pstr+5, &dns_addr[i]);
					i++;
				}
			}			
			fclose(fp);
			if(i>0)
			{
				snd.dhcp->options[off_options++] = DNS_SERVER;
		  		snd.dhcp->options[off_options++] = i*4;
				for(j=0;j<i;j++)
				{
		  			bcopy(&dns_addr[j], &snd.dhcp->options[off_options], 4);
		  			off_options += 4;
				}
			}
		}
	}
#endif
  
  /* insert "option overload" */
  if (overload != 0) {
    snd.dhcp->options[off_options++] = OPT_OVERLOAD;
    snd.dhcp->options[off_options++] = 1;        /* 1 == option body's length */
    snd.dhcp->options[off_options++] = overload;
  }

  /* insert the requested options */
  if ((option = pickup_opt(rcv.dhcp, rdhcplen, REQ_LIST)) != NULL) {
    reqopt = OPTBODY(option);
    reqoptlen = OPTLEN(option);
    for (i = 0; i < reqoptlen; i++) {
      if (isclr(inserted, (u_char) *(reqopt + i))) {
	if (insert_opt(res, lease, (u_char) *(reqopt + i), inserted, PASSIVE) == E_NOMORE) {
	  errno = 0;
	  syslog(LOG_INFO, "No more space left to insert options to DHCP%s",
		 (msgtype == DHCPOFFER) ? "OFFER" : "ACK");
	  break;
	}
      }
    }
  }

  /* insert that is different from "Host requirement RFC" defaults */
  for (i = 0; i < LAST_OPTION; i++) {
    if (isclr(inserted, i)) {
      if (insert_opt(res, lease, i, inserted, ACTIVE) == E_NOMORE) {
	errno = 0;
	syslog(LOG_INFO, "No more space left to insert options to DHCP%s",
	       (msgtype == DHCPOFFER) ? "OFFER" : "ACK");
	break;
      }
    }
  }

  return;
}


/*
 * choose resource with ciaddr
 */
static struct dhcp_resource *
select_wciaddr(cid, curr_epoch, nosuchaddr)
  struct client_id *cid;
  time_t curr_epoch;
  int *nosuchaddr;
{
  struct dhcp_resource *res = NULL;
  char tmp[sizeof("255.255.255.255")];

  *nosuchaddr = FALSE;
  if ((res = (struct dhcp_resource *)
       hash_find(&iphashtable, (char *)&rcv.dhcp->ciaddr.s_addr,
		       sizeof(u_Long), resipcmp, &rcv.dhcp->ciaddr)) == NULL) {
    *nosuchaddr = TRUE;	
    return(NULL);
  } else {
    if (cid->subnet.s_addr != (res->ip_addr->s_addr & res->subnet_mask->s_addr)) {
      strcpy(tmp, inet_ntoa(rcv.dhcp->ciaddr));
      errno = 0;
      syslog(LOG_INFO,
	     "DHCPREQUEST(from cid:\"%s\", ciaddr:%s): it's for different subnet",
	     cidtos(cid, 1), tmp);
      return(NULL);
    }
    else if (isset(res->valid, S_CLIENT_ID)) {
      /* manual allocation must have corresponding binding */
      if (res->binding == NULL) {
	strcpy(tmp, inet_ntoa(rcv.dhcp->ciaddr));
	errno = 0;
	syslog(LOG_INFO,
	       "DHCPREQUEST(from cid:\"%s\"): No corresponding binding for %s",
	       cidtos(cid, 1), tmp);
	*nosuchaddr = TRUE;
	return(NULL);
      }
      else if (res->binding->cid.idtype != cid->idtype ||
	       res->binding->cid.idlen != cid->idlen ||
	       bcmp(res->binding->cid.id, cid->id, cid->idlen) != 0) {
	strcpy(tmp, inet_ntoa(rcv.dhcp->ciaddr));
	errno = 0;
	syslog(LOG_INFO,
	       "DHCPREQUEST(from cid:\"%s\", ciaddr:%s): it isn't for the client",
	       cidtos(cid, 1), tmp);
	return(NULL);
      }
    }
    else if (res->binding == NULL ||
	     (res->binding->expire_epoch != 0xffffffff &&
	      res->binding->expire_epoch <= curr_epoch) ||
	     res->binding->cid.idtype != cid->idtype ||
	     res->binding->cid.idlen != cid->idlen ||
	     bcmp(res->binding->cid.id, cid->id, cid->idlen) != 0) {
      strcpy(tmp, inet_ntoa(rcv.dhcp->ciaddr));
      errno = 0;
      syslog(LOG_INFO,
	     "DHCPREQUEST(from cid:\"%s\", ciaddr:%s): it isn't available",
	     cidtos(cid, 1), tmp);
      return(NULL);
    }
  }

  return(res);
}


/********************************
 * process DHCPDISCOVER message *
 * which incoming from ether    *
 ********************************/
static int
discover(ifp)
  struct if_info *ifp;
{  
  struct dhcp_resource *offer_res = NULL;
  struct client_id cid;
  u_Long offer_lease = 0,                 /* offering lease */
         reqlease = 0;                    /* requested lease duration */
  time_t curr_epoch = 0;                  /* current epoch */

  bzero(&cid, sizeof(cid));

  syslogAll_printk("DHCPD receive DISCOVER packet!\n");

  if (time(&curr_epoch) == -1) {
    //diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
    syslog(LOG_WARNING, "time() error in discover()");
    return(-1);
  }  
  
  reqlease = get_reqlease(rcv.dhcp, rdhcplen);
  maxoptlen = get_maxoptlen(rcv.dhcp, rdhcplen);
  get_cid(rcv.dhcp, rdhcplen, &cid);  

  #if 0 //def KLD_ENABLED
  
  unsigned char client_mac[16];
  int i;
  unsigned char tmp[3];
  sprintf(client_mac, "%.2x", cid.id[0] & 0xff);
  for(i=1; i<cid.idlen; i++)
  {
	sprintf(tmp, "%.2x", cid.id[i] & 0xff);
	strcat(client_mac, tmp);
  } 
  
  char *option=NULL;
  char host_name[32];
  int max_len=0;
  if ((option = pickup_opt(rcv.dhcp, rdhcplen, HOSTNAME)) != NULL) 
  {
  	max_len=OPTLEN(option)>31 ? 31 : OPTLEN(option);
  	memcpy(host_name, OPTBODY(option), max_len);
	host_name[max_len]='\0';
	writeHostNameMacToFile("/etc/hostname_mac", host_name, client_mac);
  }     
  else
  	writeHostNameMacToFile("/etc/hostname_mac", "none", client_mac);
  #endif

#ifdef SUPPORT_MAC_FILTER
  {
    int found = 0;
    int i;
    if (bcmp(allowed_haddr[0], "\0\0\0\0\0\0", 6) == 0) {
      found = 1;
    } else {
      for (i = 0; i < nmacaddr; i++) {
        if (bcmp(allowed_haddr[i], rcv.dhcp->chaddr, 6) == 0) {
	  found = 1;
	  break;
        }
      }
    }
    if (!found) {
      errno = 0;
      syslog(LOG_INFO, "DHCPDISCOVER from %s ignored",
	     haddrtos(&rcv.dhcp->chaddr));
      return(-1);
    }
  }
#endif

//  diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
  if (get_subnet(rcv.dhcp, rdhcplen, &cid.subnet, ifp) != 0)
  {
	//diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
	return(-1);
  }
  if ((offer_res = choose_res(&cid, curr_epoch, reqlease)) == NULL)
  {
//	diag_printf("%s:%d Don't allocate IP!\n", __FUNCTION__,__LINE__);
	return(-1);
  }
  offer_lease = choose_lease(reqlease, curr_epoch, offer_res);

  if (update_db(DHCPDISCOVER, &cid, offer_res, offer_lease, curr_epoch) != 0)
  {
	//diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
	return(-1);
  }
//  diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
  construct_msg(DHCPOFFER, offer_res, offer_lease, ifp);

  /* xxx insert options for the specific client */
  /* xxx insert options for the specific client-type */
  /* send DHCPOFFER from the interface
   * xxx must be able to handle the fragments, but currently not implemented
   */
//  diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
  send_dhcp(ifp, DHCPOFFER);

  return(0);
}


/*******************************
 * process DHCPREQUEST message *
 * which incoming from ether   *
 *******************************/
static int
request(ifp)
  struct if_info *ifp;
{
  int reqforme = 0, nosuchaddr = 0;
  struct dhcp_resource *res = NULL;
  struct client_id cid;
  struct in_addr reqip, netmask;
  unsigned Long offer_lease = 0,         /* offerring lease */
                reqlease = 0;            /* requested lease duration */
  char *option = NULL;
#define EPOCH      "Thu Jan  1 00:00:00 1970\n"
#define BRDCSTSTR  "255.255.255.255"
  char datestr[sizeof(EPOCH)];
  char addrstr[sizeof(BRDCSTSTR)];
  time_t curr_epoch = 0;                 /* current epoch */
  int no_srv_id=0;
  bzero(&cid, sizeof(cid));
  bcopy(EPOCH, datestr, sizeof(EPOCH));
  bcopy(BRDCSTSTR, addrstr, sizeof(BRDCSTSTR));

  syslogAll_printk("DHCPD receive REQUEST packet!\n");
  if (time(&curr_epoch) == -1) {
    syslog(LOG_WARNING, "time() error in request()");
    return(-1);
  }  
  reqlease = get_reqlease(rcv.dhcp, rdhcplen);
  maxoptlen = get_maxoptlen(rcv.dhcp, rdhcplen);
  get_cid(rcv.dhcp, rdhcplen, &cid);
#ifdef KLD_ENABLED
unsigned char client_mac[16];
int i;
unsigned char tmp[3];
sprintf(client_mac, "%.2x", cid.id[0] & 0xff);
for(i=1; i<cid.idlen; i++)
{
  sprintf(tmp, "%.2x", cid.id[i] & 0xff);
  strcat(client_mac, tmp);
} 

//char *option=NULL;
char host_name[32];
int max_len=0;
if ((option = pickup_opt(rcv.dhcp, rdhcplen, HOSTNAME)) != NULL) 
{
  //diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
  max_len=OPTLEN(option)>31 ? 31 : OPTLEN(option);
  memcpy(host_name, OPTBODY(option), max_len);
  host_name[max_len]='\0';
  writeHostNameMacToFile("/etc/hostname_mac", host_name, client_mac);
}	  
else
{
  //diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
  writeHostNameMacToFile("/etc/hostname_mac", "none", client_mac);
}
#endif

#ifdef SUPPORT_MAC_FILTER
  {
    int found = 0;
    int i;
    if (bcmp(allowed_haddr[0], "\0\0\0\0\0\0", 6) == 0) {
      found = 1;
    } else {
      for (i = 0; i < nmacaddr; i++) {
        if (bcmp(allowed_haddr[i], rcv.dhcp->chaddr, 6) == 0) {
	  found = 1;
	  break;
        }
      }
    }
    if (!found) {
      errno = 0;
      syslog(LOG_INFO, "DHCPREQUEST from %s ignored",
	     haddrtos(&rcv.dhcp->chaddr));
      return(-1);
    }
  }
#endif
  if (get_subnet(rcv.dhcp, rdhcplen, &cid.subnet, ifp) != 0)
    return(-1);  
  
  /* check REQUEST for me ? */
  if ((option = pickup_opt(rcv.dhcp, rdhcplen, SERVER_ID)) != NULL) {
    if (htonl(GETHL(OPTBODY(option))) == ifp->ipaddr->s_addr)
      reqforme = 1;
  }  
  else
  	no_srv_id=1;
  
  if (rcv.dhcp->ciaddr.s_addr != 0) {
    if (get_snmk(rcv.dhcp, rdhcplen, &netmask, ifp) == 0) {
      if (rcv.dhcp->giaddr.s_addr != 0) {
	if ((rcv.dhcp->giaddr.s_addr & netmask.s_addr) !=
	    (rcv.dhcp->ciaddr.s_addr & netmask.s_addr))
	  goto nak;
      }
    }	
  
    if ((res = select_wciaddr(&cid, curr_epoch, &nosuchaddr)) == NULL) {	
      if (nosuchaddr == TRUE)
      {
	   return(-1);
      }
#ifndef COMPAT_RFC1541
      else
      	{
		flag_addclient=1;
		addclient_ip=rcv.dhcp->ciaddr.s_addr;
		goto ADDCLIENT;
      	}
#endif
    } else {
      goto ack;
    }
  }  
  reqip.s_addr = 0;
  if ((option = pickup_opt(rcv.dhcp, rdhcplen, REQUEST_IPADDR)) != NULL) {
    reqip.s_addr = htonl(GETHL(OPTBODY(option)));
  }

  if(no_srv_id==1 && rcv.dhcp->ciaddr.s_addr==0 && reqip.s_addr!=0)
  {
	flag_addclient=1;
	addclient_ip=reqip.s_addr;
	goto ADDCLIENT;
  }
#ifdef COMPAT_RFC1541
  if (reqip.s_addr == 0)
    reqip.s_addr = rcv.dhcp->ciaddr.s_addr;
#endif

  if (reqip.s_addr != 0) {
    if (get_snmk(rcv.dhcp, rdhcplen, &netmask, ifp) == 0) {
      if (rcv.dhcp->giaddr.s_addr != 0) {
	if ((rcv.dhcp->giaddr.s_addr & netmask.s_addr) !=
	    (reqip.s_addr & netmask.s_addr))
	  goto nak;
      } else {
	if ((ifp->ipaddr->s_addr & netmask.s_addr) !=
	    (reqip.s_addr & netmask.s_addr))
	  goto nak;
      }
    }

    res = NULL;
    res = select_wcid(DHCPREQUEST, &cid, curr_epoch);
    if (res != NULL && res->ip_addr != NULL &&
        res->ip_addr->s_addr == reqip.s_addr)
      goto ack;
    else if (reqforme == 1)
      goto nak;
    else
      return(-1);
    
    if ((res = select_wreqip(DHCPREQUEST, &cid, curr_epoch)) != NULL)
      goto ack;
    else if (reqforme == 1)
      goto nak;
  }
  return(-1);

ADDCLIENT:
  if(flag_addclient>0)
  {	
	  
	  /* 1. select with client identifier */
	  if ((res = select_wcid(DHCPREQUEST, &cid, curr_epoch)) != NULL)
	  {
	  	//diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
		if(res->ip_addr->s_addr!=addclient_ip)
		{
			//diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
			flag_addclient=0;
			goto nak;
		}
		else
	  		goto ack;
	  }
	 
	  /* 2. select with requested IP */
	  if ((res = select_wreqip(DHCPREQUEST, &cid, curr_epoch)) != NULL)
	  {
//	  	diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
	  	goto ack;
	  }
	  goto nak;
  }
 ack:
  offer_lease = choose_lease(reqlease, curr_epoch, res);
      
  if (update_db(DHCPREQUEST, &cid, res, offer_lease, curr_epoch) != 0) {
    if (reqforme == 1)
      goto nak;
    else
      return(-1);
  }

  construct_msg(DHCPACK, res, offer_lease, ifp);

  /* xxx insert options for the specific client */
  /* xxx insert options for the specific client-type */
  /* send DHCPACK from the interface
   * xxx must be able to handle the fragments, but currently not implemented
   */
  send_dhcp(ifp, DHCPACK);

  res->binding->flag |= COMPLETE_ENTRY;    /* binding is made completely */

  errno = 0;
  if (res->binding->expire_epoch == 0xffffffff) {
    syslog(LOG_INFO, "Assign %s to the client(cid is \"%s\") forever.",
	   addrstr, cidtos(&res->binding->cid, 1));
  } else {
    strcpy(datestr, ctime(&res->binding->expire_epoch));
    datestr[strlen(datestr) - 1] = '\0';
    strcpy(addrstr, inet_ntoa(*res->ip_addr));
    syslog(LOG_INFO, "Assign %s to the client(cid is \"%s\") till \"%s\".",
	   addrstr, cidtos(&res->binding->cid, 1), datestr);
  }
  return(0);

 nak:
  construct_msg(DHCPNAK, NULL, offer_lease, ifp);
  /* send DHCPNAK from the interface
   * xxx must be able to handle the fragments, but currently not implemented
   */
  send_dhcp(ifp, DHCPNAK);

  return(0);
}


/*******************************
 * process DHCPDECLINE message *
 *******************************/
static int
decline(ifp)
  struct if_info *ifp;
{
  struct dhcp_binding *binding = NULL;
  struct dhcp_resource *res = NULL;
  struct client_id cid;
  char *option = NULL,
       *msg = NULL;

  bzero(&cid, sizeof(cid));

  /* DECLINE for another server */
  if ((option = pickup_opt(rcv.dhcp, rdhcplen, SERVER_ID)) == NULL ||
      htonl(GETHL(OPTBODY(option))) != ifp->ipaddr->s_addr) {
    return(0);
  }

  get_cid(rcv.dhcp, rdhcplen, &cid);
  if (get_subnet(rcv.dhcp, rdhcplen, &cid.subnet, ifp) != 0)
    return(-1);

  /* search with haddr (haddr is same as client identifier) */
  if ((binding = (struct dhcp_binding *)
       hash_find(&cidhashtable, cid.id, cid.idlen,
		 bindcidcmp, &cid)) == NULL) {
    errno = 0;
    syslog(LOG_INFO,
	   "Receive DHCPDECLINE, but can't find binding for such client");
    return(-1);
  } else {
    res = binding->res;
  }

  /* now, lease_id is the combination of "chaddr" and "assigned address" */
  if (binding->res != NULL) {
    /* for a while, this resource turn off
       (expect that illegal host will be disconnected soon until 30 min )*/
    turnoff_bind(binding);
  } else {
    errno = 0;
    syslog(LOG_INFO, "Receive the invalid DHCPDECLINE.");
    return(0);
  }

  errno = 0;
  syslog(LOG_INFO, "Receive the DHCPDECLINE (entryname=%s IP=%s).",
	 binding->res->entryname, inet_ntoa(*binding->res->ip_addr));
  if ((option = pickup_opt(rcv.dhcp, rdhcplen, DHCP_ERRMSG)) != NULL) {
    msg = nvttostr(OPTBODY(option), (int) OPTLEN(option));
    if (msg != NULL) {
      if (msg[0] != '\0') {
	errno = 0;
	syslog(LOG_INFO, "\"%s\" is the message from client.", msg);
      }
      free(msg);
#ifdef ECOS_DBG_STAT
      dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, (int) OPTLEN(option));
#endif	

    }
  }

  return(0);
}


/*******************************
 * process DHCPRELEASE message *
 *******************************/
static int
release(ifp)
  struct if_info *ifp;
{
  struct dhcp_binding *binding = NULL;
  struct dhcp_resource *res = NULL;
  char *option = NULL;

  /* release for another server */
  if ((option = pickup_opt(rcv.dhcp, rdhcplen, SERVER_ID)) == NULL ||
      htonl(GETHL(OPTBODY(option))) != ifp->ipaddr->s_addr) {
    return(0);
  }

  /* search with ciaddr */
  if ((res = (struct dhcp_resource *)
       hash_find(&iphashtable, (char *)&rcv.dhcp->ciaddr.s_addr,
		       sizeof(u_Long), resipcmp, &rcv.dhcp->ciaddr)) == NULL) {
    errno = 0;
    syslog(LOG_INFO,
	   "Receive DHCPRELEASE(IP:%s), but can't find binding for such client",
	   inet_ntoa(rcv.dhcp->ciaddr));
    return(-1);
  } else {
    binding = res->binding;
  }

  /* now, lease_id is the combination of "chaddr" and "assigned address" */
  if (binding != NULL && binding->res != NULL &&
      binding->res == res &&
      isclr(binding->res->valid, S_CLIENT_ID) &&
      binding->haddr.htype == rcv.dhcp->htype &&
      binding->haddr.hlen == rcv.dhcp->hlen &&
      bcmp(binding->haddr.haddr, rcv.dhcp->chaddr, rcv.dhcp->hlen) == 0) {
    binding->expire_epoch = 0;
  } else {
    errno = 0;
    if (binding != NULL && binding->res != NULL &&
	isset(binding->res->valid, S_CLIENT_ID)) {
      syslog(LOG_INFO, "Receive DHCPRELEASE for static entry (IP:%s).",
	     inet_ntoa(rcv.dhcp->ciaddr));
	  
#ifdef CONFIG_RTL_819X
	binding->expire_epoch = 0;
	binding->flag &= ~(STATIC_ENTRY);
#endif
    } else {
      syslog(LOG_INFO, "Receive an inappropriate DHCPRELEASE(IP:%s).",
	     inet_ntoa(rcv.dhcp->ciaddr));
    }
    return(0);
  }

  errno = 0;
  syslog(LOG_INFO, "Receive DHCPRELEASE and release %s", inet_ntoa(rcv.dhcp->ciaddr));
  return(0);
}


static int inform(struct if_info *ifp)
{
	FILE *fp;
	char domain_name[32];
	
	clean_sbuf();
  	snd.dhcp->op = BOOTREPLY;
	snd.dhcp->htype = rcv.dhcp->htype;
	snd.dhcp->hlen = rcv.dhcp->hlen;
	snd.dhcp->hops = 0;
	snd.dhcp->xid = rcv.dhcp->xid;
	snd.dhcp->secs = 0;
	snd.dhcp->flags = rcv.dhcp->flags;
	
	snd.dhcp->giaddr.s_addr = 0;
	snd.dhcp->yiaddr.s_addr = 0;
	snd.dhcp->siaddr.s_addr = 0;

	snd.dhcp->ciaddr.s_addr = rcv.dhcp->ciaddr.s_addr;
	
	bcopy(rcv.dhcp->chaddr, snd.dhcp->chaddr, rcv.dhcp->hlen);

	
	/* insert magic cookie */
	 bcopy(magic_c, snd.dhcp->options, MAGIC_LEN);
	 off_options = MAGIC_LEN;
	 off_extopt = 0;

	 /* insert dhcp message type option */
  	snd.dhcp->options[off_options++] = DHCP_MSGTYPE;
 	snd.dhcp->options[off_options++] = 1;
  	snd.dhcp->options[off_options++] = DHCPACK;

	/* insert "server identifier" */
  	snd.dhcp->options[off_options++] = SERVER_ID;
  	snd.dhcp->options[off_options++] = 4;
  	bcopy(&ifp->ipaddr->s_addr, &snd.dhcp->options[off_options], 4);
  	off_options += 4;

	/* insert "subnet mask" */
	snd.dhcp->options[off_options++] = SUBNET_MASK;
  	snd.dhcp->options[off_options++] = 4;
  	bcopy(&ifp->subnetmask->s_addr, &snd.dhcp->options[off_options], 4);
  	off_options += 4;
	
	/* insert "router" */
	snd.dhcp->options[off_options++] = ROUTER;
  	snd.dhcp->options[off_options++] = 4;
  	bcopy(&ifp->ipaddr->s_addr, &snd.dhcp->options[off_options], 4);
  	off_options += 4;  	

	/*insert domain name*/
	fp=fopen("/etc/domain_name","r");
	if(fp!=NULL){
		snd.dhcp->options[off_options++] = DNS_DOMAIN;
		fscanf(fp,"%s",domain_name);
		fclose(fp);
  		snd.dhcp->options[off_options++] = strlen(domain_name);
  		bcopy(domain_name, &snd.dhcp->options[off_options], strlen(domain_name));
  		off_options += strlen(domain_name);		
	}
	
#if 1 //def KLD_ENABLED
	/*insert dns server addr*/
	if(lanDnsFromWanDhcpFlag>0)
	{
		fp=fopen("/etc/wan_info","r");
		if(fp!=NULL)
		{
			char linebuf[32];
			char *pstr=NULL;
			struct in_addr dns_addr[3];
			int i=0, j;
			while(fgets(linebuf, sizeof(linebuf), fp) && i<3)	
			{
				j=0;
				while(linebuf[j]!='\n' && linebuf[j]!='\0')
					j++;
				linebuf[j]=0;
				
				if((pstr=strstr(linebuf, "dns"))!=NULL)
				{
					inet_aton(pstr+5, &dns_addr[i]);
					i++;
				}
			}			
			fclose(fp);
			if(i>0)
			{
				snd.dhcp->options[off_options++] = DNS_SERVER;
				snd.dhcp->options[off_options++] = i*4;
				for(j=0;j<i;j++)
				{
					bcopy(&dns_addr[j], &snd.dhcp->options[off_options], 4);
					off_options += 4;
				}
			}
		}
	}
	else
	{
		fp=fopen("/etc/dhcpdb.pool","r");
		if(fp!=NULL)
		{
			char linebuf[512], str_dns[128];
			char *pstr=NULL, *pstart=NULL, *pend=NULL;
			int str_dns_len;
			int i=0, j;
			struct in_addr dns_addr[5];
			fgets(linebuf, sizeof(linebuf), fp);
			fclose(fp);
			
			if((pstr=strstr(linebuf, "dnsv"))!=NULL)
			{
				pstart=pstr+5;
				for(pend=pstart; pend && *pend !=':'; pend++);

				str_dns_len=pend-pstart;
				strncpy(str_dns, pstart, str_dns_len);
				str_dns[str_dns_len]=0;

				pstr=strtok(str_dns, " ");
				while(pstr!=NULL  && i<5)
				{
					inet_aton(pstr, &dns_addr[i]);
					pstr=strtok(NULL, " ");
					i++;
				}
				if(i>0)
				{
					snd.dhcp->options[off_options++] = DNS_SERVER;
					snd.dhcp->options[off_options++] = i*4;
					for(j=0;j<i;j++)
					{
						bcopy(&dns_addr[j], &snd.dhcp->options[off_options], 4);
						off_options += 4;
					}
				}				
			}			
		}
	}
#endif	

	send_dhcp(ifp, DHCPACK);
}


/************************************
 * send DHCP message from interface *
 ************************************/
static int send_dhcp(struct if_info *ifp, int msgtype)
{
	int i = 0, buflen = 0;
//	struct iovec bufvec[2];
	struct sockaddr_in dst;
//	struct msghdr msg;

	bzero(&dst, sizeof(dst));

	if (overload & FILE_ISOPT) 
	{
		snd.dhcp->file[off_file] = END;
	}
	if (overload & SNAME_ISOPT) 
	{
		snd.dhcp->sname[off_sname] = END;
	}
	if (off_options < DFLTOPTLEN) 
	{
		snd.dhcp->options[off_options] = END;
	} 
	
#if 0
	else if (off_extopt > 0 && off_extopt < maxoptlen - DFLTOPTLEN &&
			sbufvec[1].iov_base != NULL) 
	{
		*((char *)(sbufvec[1].iov_base) + off_extopt++) = END;
	}

	if (off_extopt < sbufvec[1].iov_len) 
	{
		sbufvec[1].iov_len = off_extopt;
	}
#endif

	/* if the message was not received from bpf or nit,
	send reply from normal socket */
#if 0
	if (rcv.dhcp->giaddr.s_addr != 0 ||
	(rcv.dhcp->ciaddr.s_addr != 0 &&
	(rcv.dhcp->ciaddr.s_addr & ifp->subnetmask->s_addr) !=
	(ifp->ipaddr->s_addr & ifp->subnetmask->s_addr))) 
#endif

	snd.dhcp->flags = rcv.dhcp->flags;

//	if(rcv.dhcp->giaddr.s_addr != 0 || rcv.dhcp->ciaddr.s_addr != 0)
	{
		dst.sin_family = AF_INET;
		//dst.sin_port = htons(dhcpc_port);
		dst.sin_port = dhcpc_port; // dhcpc_port have htons in set_srvport() !!!
		dst.sin_len=sizeof(dst);
		
		if(snd.dhcp->flags & htons(0x8000))
		{
			dst.sin_addr.s_addr= INADDR_BROADCAST;
			snd.dhcp->flags |= htons(0x8000);
		}		
		else if (rcv.dhcp->ciaddr.s_addr != 0) 
		{
//			dst.sin_port = dhcpc_port;
			bcopy(&rcv.dhcp->ciaddr, &dst.sin_addr, sizeof(u_Long));
		}
		else if (rcv.dhcp->giaddr.s_addr != 0) 
		{
//			dst.sin_port = dhcps_port;
			bcopy(&rcv.dhcp->giaddr, &dst.sin_addr, sizeof(u_Long));
		}
		else
		{
#if 0
//			dst.sin_port=dhcpc_port;
			if(snd.dhcp->yiaddr.s_addr != 0 && !ISBRDCST(rcv.dhcp->flags))
			{
				diag_printf("%s:%d--yiaddr=%s\n", __FUNCTION__,__LINE__, inet_ntoa(snd.dhcp->yiaddr));
				bcopy(&snd.dhcp->yiaddr.s_addr, &dst.sin_addr.s_addr, sizeof(u_Long));				
			}
			else
#endif
				dst.sin_addr.s_addr= INADDR_BROADCAST;
				snd.dhcp->flags |= htons(0x8000);
		}
#if 0
		bufvec[0].iov_base = (void *) snd.dhcp;
		buflen = bufvec[0].iov_len = DFLTDHCPLEN;
		bufvec[1].iov_base = sbufvec[1].iov_base;
		if (bufvec[1].iov_base == NULL) 
		{
			bufvec[1].iov_len = 0;
		}
		else 
		{
			bufvec[1].iov_len = sbufvec[1].iov_len;
		}
		buflen += bufvec[1].iov_len;
		diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
		if (setsockopt(if_list->fd, SOL_SOCKET, SO_SNDBUF, &buflen,
		sizeof(buflen)) < 0) 
		{
			syslog(LOG_WARNING, "setsockopt() in send_dhcp()");
			return(-1);
		}
		diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
		bzero(&msg, sizeof(msg));
		msg.msg_name = (caddr_t) &dst;
		msg.msg_namelen = sizeof(dst);
		msg.msg_iov = bufvec;
		if (bufvec[1].iov_base == NULL)
			msg.msg_iovlen = 1;
		else
			msg.msg_iovlen = 2;
		if (sendmsg(if_list->fd, &msg, 0) < 0) 
		{
			syslog(LOG_WARNING, "send DHCP message failed");
			return(-1);
		}
#endif
		if(sendto(ifp->fd, snd.dhcp, MINDHCPLEN+off_options, 0, (struct sockaddr *)&dst, sizeof(dst)) < 0) 
		{
			diag_printf("%s:%d--send reply packet fails!\n", __FUNCTION__,__LINE__);
			return 1;
		}
		if(msgtype==DHCPOFFER)
			syslogAll_printk("DHCPD send OFFER packet!\n");
		else if(msgtype==DHCPACK)
			syslogAll_printk("DHCPD send ACK packet!\n");
		else if(msgtype==DHCPNAK)
			syslogAll_printk("DHCPD send NAK packet!\n");
		return(0);
	}

#if 0
	int dlfd;
	if ((dlfd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
		DEBUG(LOG_ERR, "socket call failed: %s", strerror(errno));
		return -1;
	}
	
	/* fill pseudo header to calculating checksum */
	bcopy(&ifp->ipaddr->s_addr, &snd.ip->ip_src, sizeof(u_Long));
	if (snd.dhcp->yiaddr.s_addr != 0 && !ISBRDCST(rcv.dhcp->flags)) 
	{
		bcopy(&snd.dhcp->yiaddr.s_addr, &snd.ip->ip_dst.s_addr, sizeof(u_Long));
	} 
	else 
	{
		snd.ip->ip_dst.s_addr = 0xffffffff;   /* default dst */
	}
	snd.udp->uh_sport = dhcps_port;
	snd.udp->uh_dport = dhcpc_port;
	snd.udp->uh_ulen = htons(off_extopt + DFLTDHCPLEN + UDPHL);
	snd.udp->uh_sum = get_udpsum(snd.ip, snd.udp);

	snd.ip->ip_v = IPVERSION;
	snd.ip->ip_hl = IPHL >> 2;
	snd.ip->ip_tos = 0;
	snd.ip->ip_len = htons(off_extopt + DFLTDHCPLEN + UDPHL + IPHL);
	snd.ip->ip_id = snd.udp->uh_sum;
	snd.ip->ip_off = htons(IP_DF);    /* XXX */
	snd.ip->ip_ttl = 0x20;            /* XXX */
	snd.ip->ip_p = IPPROTO_UDP;
	snd.ip->ip_sum = get_ipsum(snd.ip);

	if (rcv.dhcp->htype == ETHER) 
	{
		if (ISBRDCST(rcv.dhcp->flags) || (snd.dhcp->yiaddr.s_addr == 0)) 
		{
			for (i = 0; i < 6; i++) 
			{
				snd.ether->ether_dhost[i] = 0xff;
			}
		}
		else 
		{
			for (i = 0; i < 6; i++) 
			{
				snd.ether->ether_dhost[i] = rcv.dhcp->chaddr[i];
			}
		}
	}
	else 
	{
		for (i = 0; i < 6; i++) 
		{
			snd.ether->ether_dhost[i] = rcv.ether->ether_shost[i];
		}
	}

	for (i = 0; i < 6; i++) 
	{
		snd.ether->ether_shost[i] = ifp->haddr[i];
	}
	snd.ether->ether_type = htons(ETHERTYPE_IP);

	if (sbufvec[1].iov_base == NULL) 
	{
		if (ether_write(ifp->fd, sbufvec[0].iov_base, sbufvec[0].iov_len) < 0) 
		{
			syslog(LOG_WARNING, "send DHCP message failed");
			return(-1);
		}
	} 
	else 
	{
		if (ether_writev(ifp->fd, sbufvec, 2) < 0) 
		{
			syslog(LOG_WARNING, "send DHCP message failed");
			return(-1);
		}
	}
	return(0);  
#endif
}


#ifndef NOBOOTP
/*
 * return true if resource is available for bootp
 */
static int
available_forbootp(res, cid, curr_epoch)
  struct dhcp_resource *res;
  struct client_id *cid;
  time_t curr_epoch;
{
  if (res->allow_bootp == FALSE)
    return(FALSE);

  if (res->binding == NULL ||
      cidcmp(&res->binding->cid, cid) ||
      (res->binding->expire_epoch != 0xffffffff &&
       res->binding->expire_epoch < curr_epoch)) {
    return(TRUE);
  }

  return(FALSE);
}


/*
 * choose the address newly for bootp client
 */
static struct dhcp_resource *
choose_forbootp(cid, curr_epoch)
  struct client_id *cid;
  time_t curr_epoch;
{
  struct dhcp_resource *res = NULL, *offer = NULL;
  struct hash_member *resptr = NULL;

  resptr = reslist;
  while (resptr != NULL) {
    res = (struct dhcp_resource *) resptr->data;

    /* if it is dummy entry, skip it */
    if (res->ip_addr == NULL) {
      resptr = resptr->next;
      continue;
    }

    /* check the resource. valid subnet?, available resource? */
    if (cid->subnet.s_addr == (res->ip_addr->s_addr & res->subnet_mask->s_addr) &&
	available_forbootp(res, cid, curr_epoch)) {

      if (rcv.dhcp->ciaddr.s_addr != 0) {
	offer = res;
	break;
      }
      /* specify DHCPDISCOVER as dummy */
      else if (icmp_check(DHCPDISCOVER, res->ip_addr) == GOOD) {
	offer = res;
	break;
      } else {
	turnoff_bind(res->binding);
      }

    }

    resptr = resptr->next;
  }
  if (offer == NULL) {
    errno = 0;
    syslog(LOG_WARNING, "BOOTP: No more available address in the pool");
  }
  return(offer);
}


/*
 * construct BOOTP REPLY
 */
static void
construct_bootp(res)
  struct dhcp_resource *res;
{
  int i = 0;
  char inserted[32];

  bzero(inserted, sizeof(inserted));

  clean_sbuf();
  overload = 0;
  snd.dhcp->op = BOOTREPLY;
  snd.dhcp->htype = rcv.dhcp->htype;
  snd.dhcp->hlen = rcv.dhcp->hlen;
  snd.dhcp->hops = 0;
  snd.dhcp->xid = rcv.dhcp->xid;
  snd.dhcp->secs = 0;
  if (rcv.dhcp->giaddr.s_addr != 0) {
    snd.dhcp->flags = rcv.dhcp->flags;
  } else {
    snd.dhcp->flags = 0;
  }
  snd.dhcp->giaddr.s_addr = rcv.dhcp->giaddr.s_addr;
  bcopy(rcv.dhcp->chaddr, snd.dhcp->chaddr, rcv.dhcp->hlen);

  snd.dhcp->yiaddr.s_addr = res->ip_addr->s_addr;
  if (isset(res->valid, S_SIADDR)) {
    snd.dhcp->siaddr.s_addr = res->siaddr->s_addr;
  } else {
    snd.dhcp->siaddr.s_addr = 0;
  }
  if (isset(res->valid, S_SNAME)) {
    strncpy(snd.dhcp->sname, res->sname, MAX_SNAME);
    snd.dhcp->sname[MAX_SNAME - 1] = '\0';  /* NULL terminate cerntainly */
  }
  if (isset(res->valid, S_FILE)) {
    strncpy(snd.dhcp->file, res->file, MAX_FILE);
    snd.dhcp->file[MAX_FILE - 1] = '\0';    /* NULL terminate cerntainly */
  }

  /* insert magic cookie */
  bcopy(magic_c, snd.dhcp->options, MAGIC_LEN);
  off_options = MAGIC_LEN;
  off_extopt = 0;

  /* insert "subnet mask" */
  if (insert_opt(res, 0xffffffff, SUBNET_MASK, inserted, PASSIVE) == E_NOMORE) {
    errno = 0;
    syslog(LOG_INFO, "No more space left to insert options to BOOTP");
  }

  /* insert that is different from "Host requirement RFC" defaults */
  for (i = 0; i < LAST_OPTION; i++) {
    if (isclr(inserted, i)) {
      if (insert_opt(res, 0xffffffff, i, inserted, PASSIVE) == E_NOMORE) {
	errno = 0;
	syslog(LOG_INFO, "No more space left to insert options to BOOTP");
	break;
      }
    }
  }

  return;
}


/*************************
 * process BOOTP message *
 *************************/
/* argsused */
static int
bootp(ifp)
  struct if_info *ifp;
{
  char addrstr[sizeof(BRDCSTSTR)];
  struct client_id cid;
  struct dhcp_binding *binding = NULL;
  struct dhcp_resource *res = NULL;
  time_t curr_epoch = 0;

  bzero(&cid, sizeof(cid));
  bcopy(BRDCSTSTR, addrstr, sizeof(BRDCSTSTR));

#ifdef NOBOOTP
  return(0);
#endif

  if (time(&curr_epoch) == -1) {
    syslog(LOG_WARNING, "time() error in bootp()");
    return(-1);
  }
  get_cid(rcv.dhcp, rdhcplen, &cid);
  if (get_subnet(rcv.dhcp, rdhcplen, &cid.subnet, ifp) != 0)
    return(-1);
  maxoptlen = BOOTPOPTLEN;

  /* search with haddr (haddr is same as client identfier) */
  if ((binding = (struct dhcp_binding *)
       hash_find(&cidhashtable, cid.id, cid.idlen, bindcidcmp, &cid)) != NULL) {
    if (cidcmp(&binding->cid, &cid))
      res = binding->res;
  }
  if (res == NULL && (res = choose_forbootp(&cid, curr_epoch)) == NULL)
    return(-1);

  if (update_db(BOOTP, &cid, res, 0xffffffff, curr_epoch) != 0)
    return(-1);

  /* binding is made completely */
  res->binding->flag |= (COMPLETE_ENTRY | BOOTP_ENTRY);

  construct_bootp(res);
  send_bootp(ifp);
  strcpy(addrstr, inet_ntoa(*res->ip_addr));
  errno = 0;
  syslog(LOG_INFO, "Reply to the bootp client(IP:%s, cid:\"%s\").",
	 addrstr, cidtos(&res->binding->cid, 1));

  return(0);
}


/*************************************
 * send BOOTP message from interface *
 *************************************/
static int
send_bootp(ifp)
  struct if_info *ifp;
{
  int i = 0, buflen = 0;
  struct sockaddr_in dstaddr;

  bzero(&dstaddr, sizeof(dstaddr));

  if (off_options < BOOTPOPTLEN) {
    snd.dhcp->options[off_options] = END;
  }

  /* if received message was relayed from relay agent,
     send reply from normal socket */
  if (rcv.dhcp->giaddr.s_addr != 0) {
    dstaddr.sin_family = AF_INET;
    if (rcv.dhcp->ciaddr.s_addr == 0 ||
        rcv.dhcp->ciaddr.s_addr != snd.dhcp->yiaddr.s_addr) {
      dstaddr.sin_port = dhcps_port;
      bcopy(&rcv.dhcp->giaddr, &dstaddr.sin_addr, sizeof(u_Long));
    } else {
      dstaddr.sin_port = dhcpc_port;
      bcopy(&snd.dhcp->yiaddr, &dstaddr.sin_addr, sizeof(u_Long));
    }

    buflen = DFLTBOOTPLEN;
    if (setsockopt(ifp->fd, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(buflen)) < 0) {
      syslog(LOG_WARNING, "setsockopt() in send_bootp()");
      return(-1);
    }

    if (ifp == if_list) {
      if (sendto(ifp->fd, snd.dhcp, buflen, 0, (struct sockaddr *)&dstaddr,
		 sizeof(dstaddr)) < 0) {
	syslog(LOG_WARNING, "send BOOTP message failed");
	return(-1);
      }
    } else {
      if (write(ifp->fd, snd.dhcp, buflen) < 0) {
	syslog(LOG_WARNING, "send BOOTP message failed");
	return(-1);
      }
    }

    return(0);
  }

  /* if directly received packet.... */

  /* fill pseudo header to calculating checksum */
  bcopy(&ifp->ipaddr->s_addr, &snd.ip->ip_src, sizeof(u_Long));
  if (ISBRDCST(rcv.dhcp->flags) || rcv.dhcp->ciaddr.s_addr != snd.dhcp->yiaddr.s_addr) {
    snd.ip->ip_dst.s_addr = 0xffffffff;   /* default dst */
  } else {
    bcopy(&snd.dhcp->yiaddr.s_addr, &snd.ip->ip_dst.s_addr, sizeof(u_Long));
  }
  snd.udp->uh_sport = dhcps_port;
  snd.udp->uh_dport = dhcpc_port;
  snd.udp->uh_ulen = htons(DFLTBOOTPLEN + UDPHL);
  snd.udp->uh_sum = get_udpsum(snd.ip, snd.udp);

  snd.ip->ip_v = IPVERSION;
  snd.ip->ip_hl = IPHL >> 2;
  snd.ip->ip_tos = 0;
  snd.ip->ip_len = htons(DFLTBOOTPLEN + UDPHL + IPHL);
  snd.ip->ip_id = snd.udp->uh_sum;
  snd.ip->ip_off = htons(IP_DF);    /* XXX */
  snd.ip->ip_ttl = 0x20;            /* XXX */
  snd.ip->ip_p = IPPROTO_UDP;
  snd.ip->ip_sum = get_ipsum(snd.ip);

  if (ISBRDCST(rcv.dhcp->flags) || (rcv.dhcp->ciaddr.s_addr != snd.dhcp->yiaddr.s_addr)) {
    for (i = 0; i < 6; i++) {
#ifdef sun
      snd.ether->ether_shost.ether_addr_octet[i] = ifp->haddr[i];
      snd.ether->ether_dhost.ether_addr_octet[i] = 0xff;
#else
      snd.ether->ether_shost[i] = ifp->haddr[i];
      snd.ether->ether_dhost[i] = 0xff;
#endif
    }
  }
  else {
    for (i = 0; i < 6; i++) {
#ifdef sun
      snd.ether->ether_shost.ether_addr_octet[i] = ifp->haddr[i];
      snd.ether->ether_dhost.ether_addr_octet[i] = rcv.dhcp->chaddr[i];
#else
      snd.ether->ether_shost[i] = ifp->haddr[i];
      snd.ether->ether_dhost[i] = rcv.dhcp->chaddr[i];
#endif
    }
  }
  snd.ether->ether_type = htons(ETHERTYPE_IP);

  buflen = DFLTBOOTPLEN + UDPHL + IPHL + ETHERHL;
  if (ether_write(ifp->fd, (char *)snd.ether, buflen) < 0) {
    syslog(LOG_WARNING, "send BOOTP message failed");
    return(-1);
  }

  return(0);
}
#endif /* NOBOOTP */


/*
 * insert the IP address
 */
/* ARGSUSED */
static int
ins_ip(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  struct in_addr *addr = NULL;
  char option[6];
  int symbol = 0;
  int retval = 0;

  bzero(option, sizeof(option));

  switch(tagnum) {
  case SUBNET_MASK:
    symbol = S_SUBNET_MASK, addr = res->subnet_mask;
    break;
	
#ifndef CONFIG_RTL_819X
  case SWAP_SERVER:
    symbol = S_SWAP_SERVER, addr = res->swap_server;
    break;
#endif

  case BRDCAST_ADDR:
    symbol = S_BRDCAST_ADDR, addr = res->brdcast_addr;
    break;

#ifndef CONFIG_RTL_819X
  case ROUTER_SOLICIT:
    symbol = S_ROUTER_SOLICIT, addr = res->router_solicit;
    break;
#endif

  default:
    return(-1);
  }
    
  if ((flag == PASSIVE && isset(res->valid, symbol)) ||
      (flag == ACTIVE && isset(res->active, symbol))) {
    option[0] = tagnum;
    option[1] = 4;
    bcopy(addr, &option[2], 4);
    if ((retval = insert_it(option)) == 0)
      setbit(inserted, tagnum);
  }
  return(retval);
}


/*
 * insert IP addresses
 */
/* ARGSUSED */
static int
ins_ips(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  struct in_addrs *addr = NULL;
  char option[254];
  int symbol = 0;
  int retval = 0;
  int i = 0;

  bzero(option, sizeof(option));

  switch(tagnum) {
  case ROUTER:
    symbol = S_ROUTER, addr = res->router;
    break;

#ifndef CONFIG_RTL_819X
  case TIME_SERVER:
    symbol = S_TIME_SERVER, addr = res->time_server;
    break;
#endif

  case NAME_SERVER:
    symbol = S_NAME_SERVER, addr = res->name_server;
    break;
  case DNS_SERVER:
    symbol = S_DNS_SERVER, addr = res->dns_server;
    break;

#ifndef CONFIG_RTL_819X
  case LOG_SERVER:
    symbol = S_LOG_SERVER, addr = res->log_server;
    break;
  case COOKIE_SERVER:
    symbol = S_COOKIE_SERVER, addr = res->cookie_server;
    break;
  case LPR_SERVER:
    symbol = S_LPR_SERVER, addr = res->lpr_server;
    break;
  case IMPRESS_SERVER:
    symbol = S_IMPRESS_SERVER, addr = res->impress_server;
    break;
  case RLS_SERVER:
    symbol = S_RLS_SERVER, addr = res->rls_server;
    break;
  case NIS_SERVER:
    symbol = S_NIS_SERVER, addr = res->nis_server;
    break;
  case NTP_SERVER:
    symbol = S_NTP_SERVER, addr = res->ntp_server;
    break;
  case NBN_SERVER:
    symbol = S_NBN_SERVER, addr = res->nbn_server;
    break;
  case NBDD_SERVER:
    symbol = S_NBDD_SERVER, addr = res->nbdd_server;
    break;
  case XFONT_SERVER:
    symbol = S_XFONT_SERVER, addr = res->xfont_server;
    break;
  case XDISPLAY_MANAGER:
    symbol = S_XDISPLAY_MANAGER, addr = res->xdisplay_manager;
    break;
  case NISP_SERVER:
    symbol = S_NISP_SERVER, addr = res->nisp_server;
    break;
  case MOBILEIP_HA:
    symbol = S_MOBILEIP_HA, addr = res->mobileip_ha;
    break;
  case SMTP_SERVER:
    symbol = S_SMTP_SERVER, addr = res->smtp_server;
    break;
  case POP3_SERVER:
    symbol = S_POP3_SERVER, addr = res->pop3_server;
    break;
  case NNTP_SERVER:
    symbol = S_NNTP_SERVER, addr = res->nntp_server;
    break;
  case DFLT_WWW_SERVER:
    symbol = S_DFLT_WWW_SERVER, addr = res->dflt_www_server;
    break;
  case DFLT_FINGER_SERVER:
    symbol = S_DFLT_FINGER_SERVER, addr = res->dflt_finger_server;
    break;
  case DFLT_IRC_SERVER:
    symbol = S_DFLT_IRC_SERVER, addr = res->dflt_irc_server;
    break;
  case STREETTALK_SERVER:
    symbol = S_STREETTALK_SERVER, addr = res->streettalk_server;
    break;
  case STDA_SERVER:
    symbol = S_STDA_SERVER, addr = res->stda_server;
    break;
#endif

  default:
    return(-1);
  }

  if ((flag == PASSIVE && isset(res->valid, symbol)) ||
      (flag == ACTIVE && isset(res->active, symbol))) {
    option[0] = tagnum;
    option[1] = addr->num * 4;
    for (i = 0; i < addr->num; i++)
      bcopy(&addr->addr[i].s_addr, &option[i * 4 + 2], 4);
    if ((retval = insert_it(option)) == 0)
      setbit(inserted, tagnum);
  }
  return(retval);
}


/*
 * insert pairs of the IP address
 */
/* ARGSUSED */
static int
ins_ippairs(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  struct ip_pairs *pair = NULL;
  char option[254];
  int symbol = 0;
  int retval = 0;
  int i = 0;

  bzero(option, sizeof(option));

  switch(tagnum) {
  case POLICY_FILTER:
    symbol = S_POLICY_FILTER, pair = res->policy_filter;
    break;
  case STATIC_ROUTE:
    symbol = S_STATIC_ROUTE, pair = res->static_route;
    break;
  default:
    return(-1);
  }

  if ((flag == PASSIVE && isset(res->valid, symbol)) ||
      (flag == ACTIVE && isset(res->active, symbol))) {
    option[0] = tagnum;
    option[1] = pair->num * 8;
    for (i = 0; i < pair->num; i++) {
      bcopy(&pair->addr1[i].s_addr, &option[i * 8 + 2], 4);
      bcopy(&pair->addr2[i].s_addr, &option[i * 8 + 6], 4);
    }
    if ((retval = insert_it(option)) == 0)
      setbit(inserted, tagnum);
  }
  return(retval);
}


/*
 * insert long
 */
/* ARGSUSED */
static int
ins_long(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  Long *num = NULL;
  char option[6];
  int symbol = 0;
  int retval = 0;

  bzero(option, sizeof(option));

  switch(tagnum) {
  case TIME_OFFSET:
    symbol = S_TIME_OFFSET, num = &res->time_offset;
    break;
	
#ifndef CONFIG_RTL_819X
  case MTU_AGING_TIMEOUT:
    symbol = S_MTU_AGING_TIMEOUT, num = (Long *) &res->mtu_aging_timeout;
    break;
  case ARP_CACHE_TIMEOUT:
    symbol = S_ARP_CACHE_TIMEOUT, num = (Long *) &res->arp_cache_timeout;
    break;
  case KEEPALIVE_INTER:
    symbol = S_KEEPALIVE_INTER, num = (Long *) &res->keepalive_inter;
    break;
#endif
  default:
    return(-1);
  }
    
  if ((flag == PASSIVE && isset(res->valid, symbol)) ||
      (flag == ACTIVE && isset(res->active, symbol))) {
    option[0] = tagnum;
    option[1] = 4;
    bcopy(num, &option[2], 4);
    if ((retval = insert_it(option)) == 0)
      setbit(inserted, tagnum);
  }
  return(retval);
}


/*
 * insert short
 */
/* ARGSUSED */
static int
ins_short(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  short *num = NULL;
  char option[4];
  int symbol = 0;
  int retval = 0;

  bzero(option, sizeof(option));

  switch(tagnum) {
  case BOOTSIZE:
    symbol = S_BOOTSIZE, num = (short *) &res->bootsize;
    break;
  case MAX_DGRAM_SIZE:
    symbol = S_MAX_DGRAM_SIZE, num = (short *) &res->max_dgram_size;
    break;
  case IF_MTU:
    symbol = S_IF_MTU, num = (short *) &res->intf_mtu;
    break;
  default:
    return(-1);
  }
    
  if ((flag == PASSIVE && isset(res->valid, symbol)) ||
      (flag == ACTIVE && isset(res->active, symbol))) {
    option[0] = tagnum;
    option[1] = 2;
    bcopy(num, &option[2], 2);
    if ((retval = insert_it(option)) == 0)
      setbit(inserted, tagnum);
  }
  return(retval);
}


/*
 * insert octet
 */
/* ARGSUSED */
static int
ins_octet(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  char num = 0;
  char option[3];
  int symbol = 0;
  int retval = 0;

  bzero(option, sizeof(option));

  switch(tagnum) {
  	
  case IP_FORWARD:
    symbol = S_IP_FORWARD, num = res->ip_forward;
    break;
	
#ifndef CONFIG_RTL_819X
  case NONLOCAL_SRCROUTE:
    symbol = S_NONLOCAL_SRCROUTE, num = res->nonlocal_srcroute;
    break;
#endif

  case DEFAULT_IP_TTL:
    symbol = S_DEFAULT_IP_TTL, num = res->default_ip_ttl;
    break;

#ifndef CONFIG_RTL_819X
  case ALL_SUBNET_LOCAL:
    symbol = S_ALL_SUBNET_LOCAL, num = res->all_subnet_local;
    break;
  case MASK_DISCOVER:
    symbol = S_MASK_DISCOVER, num = res->mask_discover;
    break;
  case MASK_SUPPLIER:
    symbol = S_MASK_SUPPLIER, num = res->mask_supplier;
    break;
  case ROUTER_DISCOVER:
    symbol = S_ROUTER_DISCOVER, num = res->router_discover;
    break;
  case TRAILER:
    symbol = S_TRAILER, num = res->trailer;
    break;
  case ETHER_ENCAP:
    symbol = S_ETHER_ENCAP, num = res->ether_encap;
    break;
#endif

  case DEFAULT_TCP_TTL:
    symbol = S_DEFAULT_TCP_TTL, num = res->default_tcp_ttl;
    break;

#ifndef CONFIG_RTL_819X
  case KEEPALIVE_GARBA:
    symbol = S_KEEPALIVE_GARBA, num = res->keepalive_garba;
    break;
  case NB_NODETYPE:
    symbol = S_NB_NODETYPE, num = res->nb_nodetype;
    break;
#endif

  default:
    return(-1);
  }
    
  if ((flag == PASSIVE && isset(res->valid, symbol)) ||
      (flag == ACTIVE && isset(res->active, symbol))) {
    option[0] = tagnum;
    option[1] = 1;
    option[2] = num;
    if ((retval = insert_it(option)) == 0)
      setbit(inserted, tagnum);
  }
  return(retval);
}


/*
 * insert string
 */
/* ARGSUSED */
static int
ins_str(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  char *str = NULL;
  char option[258];
  int symbol = 0;
  int retval = 0;
  int i = 0;

  bzero(option, sizeof(option));

  switch(tagnum) {
  case HOSTNAME:
    symbol = S_HOSTNAME, str = res->hostname;
    break;
	
#ifndef CONFIG_RTL_819X
  case MERIT_DUMP:
    symbol = S_MERIT_DUMP, str = res->merit_dump;
    break;
#endif

  case DNS_DOMAIN:
    symbol = S_DNS_DOMAIN, str = res->dns_domain;
    break;
	
#ifndef CONFIG_RTL_819X
  case ROOT_PATH:
    symbol = S_ROOT_PATH, str = res->root_path;
    break;
  case EXTENSIONS_PATH:
    symbol = S_EXTENSIONS_PATH, str = res->extensions_path;
    break;
  case NIS_DOMAIN:
    symbol = S_NIS_DOMAIN, str = res->nis_domain;
    break;
  case NB_SCOPE:
    symbol = S_NB_SCOPE, str = res->nb_scope;
    break;
  case NISP_DOMAIN:
    symbol = S_NISP_DOMAIN, str = res->nisp_domain;
    break;
#endif

  default:
    return(-1);
  }
    
  if ((flag == PASSIVE && isset(res->valid, symbol)) ||
      (flag == ACTIVE && isset(res->active, symbol))) {
    option[0] = tagnum;
    option[1] = ((i = strlen(str)) > MAXOPT) ? MAXOPT : i;
    bcopy(str, &option[2], option[1]);
    if ((retval = insert_it(option)) == 0)
      setbit(inserted, tagnum);
  }
  return(retval);
}


/*
 * insert dhcp_t1 or dhcp_t2
 */
static int
ins_dht(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  char option[6];
  int symbol = 0;
  int retval = 0;
  Long num = 0;

  bzero(option, sizeof(option));

  switch(tagnum) {
  case DHCP_T1:
    symbol = S_DHCP_T1;
    num = htonl(lease * (res->dhcp_t1 + (rand() & 0x03)) / 1000);
    break;
  case DHCP_T2:
    symbol = S_DHCP_T2;
    num = htonl(lease * (res->dhcp_t2 + (rand() & 0x03)) / 1000);
    break;
  default:
    return(-1);
  }
    
  if ((flag == PASSIVE && isset(res->valid, symbol)) ||
      (flag == ACTIVE && isset(res->active, symbol))) {
    option[0] = tagnum;
    option[1] = 4;
    bcopy(&num, &option[2], option[1]);
    if ((retval = insert_it(option)) == 0)
      setbit(inserted, tagnum);
  }
  return(retval);
}


/*
 * insert mtu_plateau_table
 */

#ifndef CONFIG_RTL_819X
static int
ins_mtpt(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  char option[256];
  int retval = 0;
  int i = 0;

  bzero(option, sizeof(option));

  if (tagnum != MTU_PLATEAU_TABLE) {
    return(-1);
  }

  if ((flag == PASSIVE && isset(res->valid, S_MTU_PLATEAU_TABLE)) ||
      (flag == ACTIVE && isset(res->active, S_MTU_PLATEAU_TABLE))) {
    option[0] = tagnum;
    option[1] = res->mtu_plateau_table->num * 2;
    for (i = 0; i < res->mtu_plateau_table->num; i++)
      bcopy(&res->mtu_plateau_table->shorts[i], &option[i * 2 + 2], 2);
    if ((retval = insert_it(option)) == 0)
      setbit(inserted, tagnum);
  }
  return(retval);
}
#endif


/*
 * Insert the specified option
 */
static int
insert_opt(res, lease, tagnum, inserted, flag)
  struct dhcp_resource *res;
  u_Long lease;
  int tagnum;
  char *inserted;
  char flag;
{
  if (tagnum < PAD || tagnum > LAST_OPTION || ins_opt[tagnum] == NULL)
    return(-1);

  return((*ins_opt[tagnum])(res, lease, tagnum, inserted, flag));
}


static int
insert_it(opt)
  char *opt;
{
  char len = 0;
  int done = 0;

  len = opt[1] + 2;   /* 2 == tag number and length field */
  if (off_options + len < maxoptlen && off_options + len < DFLTOPTLEN) {
    bcopy(opt, &snd.dhcp->options[off_options], len);
    off_options += len;
    return(0);
  }
  else if ((overload & FILE_ISOPT) != 0 && off_file + len < MAX_FILE) {
    bcopy(opt, &snd.dhcp->file[off_file], len);
    off_file += len;
    return(0);
  }
  else if ((overload & SNAME_ISOPT) != 0 && off_sname + len < MAX_SNAME) {
    bcopy(opt, &snd.dhcp->sname[off_sname], len);
    off_sname += len;
    return(0);
  }
  else if (len < maxoptlen - off_options - off_extopt) {
    if (maxoptlen > DFLTOPTLEN) {
      if (sbufvec[1].iov_base == NULL) {
	if ((sbufvec[1].iov_base = calloc(1, maxoptlen - DFLTOPTLEN)) == NULL) {
	  syslog(LOG_WARNING, "calloc error in insert_it()");
	  return(-1);
	}
      }
      else if (sbufvec[1].iov_len < maxoptlen - DFLTOPTLEN) {
	free(sbufvec[1].iov_base);
	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sbufvec[1].iov_len);
#endif	

	if ((sbufvec[1].iov_base = calloc(1, maxoptlen - DFLTOPTLEN)) == NULL) {
	  syslog(LOG_WARNING, "calloc error in insert_it()");
	  return(-1);
	}
      }
	  
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, maxoptlen - DFLTOPTLEN);
#endif	

      sbufvec[1].iov_len = maxoptlen - DFLTOPTLEN;

      done = DFLTOPTLEN - off_options;
      bcopy(opt, &snd.dhcp->options[off_options], done);
      len -= done;
      off_options += done; /* invalid offset, So, to access
					    here will cause fatal error */
      bcopy(&opt[done], ((char *)(sbufvec[1].iov_base) + off_extopt), len);
      off_extopt += len;
      return(0);
    }
  }

  if ((off_options + off_extopt >= maxoptlen) &&
      ((overload & FILE_ISOPT) == 0 || off_file >= MAX_FILE) &&
      ((overload & SNAME_ISOPT) == 0 || off_sname >= MAX_SNAME)) {
    return(E_NOMORE);
  }

  return(-1);
}


/*
 * compare client identifier
 */
static int
cidcmp(cid1, cid2)
  struct client_id *cid1;
  struct client_id *cid2;
{
  return (cid1->subnet.s_addr == cid2->subnet.s_addr &&
	  cid1->idtype == cid2->idtype && cid1->idlen == cid2->idlen &&
	  bcmp(cid1->id, cid2->id, cid1->idlen) == 0);
}

/* copy from dhcpc */
static int arp_check(char *ifname, int type, struct in_addr *sip, struct in_addr *tip, u_char *enaddr)
 {	 
	 //struct in_addr sip;
	 //sip.s_addr=htonl(INADDR_ANY);
	 
	 struct arpcom *ac; 	
	 struct ifnet *pifnet=NULL;  
	 pifnet=rtl_getIfpByName(ifname);
	 
	 if(pifnet==NULL)
	 {		 
		 //diag_printf("%s:%d ####\n",__FUNCTION__,__LINE__);
		 return 1;
	 }
	 
	 ac=(struct arpcom *)pifnet;	   
	 
	 extern void rtl_arpRequest(struct arpcom *ac, int type, struct in_addr *sip, struct in_addr *tip, u_char *enaddr);
	 rtl_arpRequest(ac, type, sip, tip, enaddr);			 
 
	 sleep(1);
	 
	 if(rtl_checkArpReply(type))
	 	return 0;
	 
	 return 1;
 }

/* 
 * do icmp echo request
 * if get reply, return BAD
 */
static int
icmp_check(msgtype, ip)
  int msgtype;
  struct in_addr *ip;
{
  struct protoent *protocol = NULL;
  struct sockaddr_in dst, from;
  struct icmp *sicmp = NULL, *ricmp = NULL;
  struct ip *ipp = NULL;
  char rcvbuf[1024], sndbuf[ICMP_MINLEN];
  int rlen = 0, fromlen = 0, i = 0;
  int sockfd = 0;
  u_short pid = 0;

#ifdef NOICMPCHK
  return(GOOD);
#endif

  if (msgtype != DHCPDISCOVER)
    return(GOOD);

  #define DHCPD_SEND_ARP 1

  if(!arp_check(inf_name, DHCPD_SEND_ARP, if_list->ipaddr, ip, if_list->haddr))
  	return(BAD); 
  else 
  	return(GOOD); 
#if 0
//  diag_printf("%s:%d\n", __FUNCTION__,__LINE__);

  bzero(&dst, sizeof(dst));
  bzero(&from, sizeof(from));
  bzero(sndbuf, sizeof(sndbuf));
  bzero(rcvbuf, sizeof(rcvbuf));

  sicmp = (struct icmp *) sndbuf;
  pid = (short) getpid() & 0xffff;

  if ((protocol = getprotobyname("icmp")) == NULL) {
    errno = 0;
    syslog(LOG_WARNING, "Can't get protocol \"icmp\" in icmp_check()");
//    diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
    return(GOOD);
  }

  if ((sockfd = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0) {
    syslog(LOG_WARNING, "socket error in icmp_check()");
//    diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
    return(GOOD);
  }
  
#ifdef ECOS_DBG_STAT
   dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

#ifndef CONFIG_RTL_819X
delarp(ip);
#endif

  i = 1;
  if (ioctl(sockfd, FIONBIO, &i) < 0) {
    syslog(LOG_WARNING, "ioctl(FIONBIO) in icmp_check()");
	#ifdef CONFIG_RTL_819X
	if(sockfd>0)
	{
		close(sockfd); 
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
	}
	#endif
//    diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
    return(GOOD);
  }

  dst.sin_family = AF_INET;
  dst.sin_addr.s_addr = ip->s_addr;

  sicmp->icmp_type = ICMP_ECHO;
  sicmp->icmp_code = 0;
  sicmp->icmp_cksum = 0;
  sicmp->icmp_id = pid;
  sicmp->icmp_seq = 0;

  sicmp->icmp_cksum = dhcpd_cksum((u_short *) sndbuf, sizeof(sndbuf) / sizeof(u_short));
  fromlen = sizeof(from);
  
//  timeoutflag = 0;

#ifndef CONFIG_RTL_819X
  if ((int) signal(SIGALRM, timeout) < 0) {
    syslog(LOG_WARNING, "cannot set signal handler(SIGALRM)");
    close(sockfd);
	
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
    diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
    return(GOOD);
  }
//  ualarm(500000, 0);         /* wait 500 msec */
alarm(1);
#endif

  i = sendto(sockfd, sndbuf, sizeof(sndbuf), 0, (struct sockaddr *) &dst, sizeof(dst));
  if (i < 0 || i != sizeof(sndbuf)) {
    syslog(LOG_WARNING, "Can't send icmp echo request");
	#ifdef CONFIG_RTL_819X	
	if(sockfd>0)
	{
		close(sockfd); 
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
	}
	#endif
//    diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
    return(GOOD);
  }

#ifdef CONFIG_RTL_819X
	int	timeout = 1;
	time_t prevTime;
	fd_set fdset;
	struct timeval	tm;
	
	tm.tv_usec=0;
	time(&prevTime);
	
	while(timeout>0)
	{
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset);
		tm.tv_sec=timeout;
		
		if (select(sockfd + 1, &fdset, NULL, NULL, &tm) < 0) 
		{
			goto out;
		} 
		else if (FD_ISSET(sockfd, &fdset)) 
		{
			if((rlen = recvfrom(sockfd, rcvbuf, sizeof(rcvbuf), 0, (struct sockaddr *)&from, &fromlen))>0)		
			{
				ipp = (struct ip *) rcvbuf;
				if (rlen >= (ipp->ip_hl << 2) + ICMP_MINLEN)
				{
					ricmp = (struct icmp *) (rcvbuf + (ipp->ip_hl << 2));
					if (ricmp->icmp_type == ICMP_ECHOREPLY)
					{
						if (ricmp->icmp_id == pid)
							break;
					}
				}			
			}			 
		}
		timeout -= (time(NULL) - prevTime);
		time(&prevTime);
	}
#endif
  
#if 0
  while (1) { /* repeat until receive reply or timeout */
    rlen = recvfrom(sockfd, rcvbuf, sizeof(rcvbuf), 0,
		    (struct sockaddr *)&from, &fromlen);
    if (timeoutflag == 1) {
      break;
    }
    if (rlen < 0) {
#ifndef CONFIG_RTL_819X
      	   continue;
#else
	   goto out;
#endif
     }

    ipp = (struct ip *) rcvbuf;
    if (rlen < (ipp->ip_hl << 2) + ICMP_MINLEN) {
#ifndef CONFIG_RTL_819X
          continue;
#else
	   goto out;
#endif
    }
    ricmp = (struct icmp *) (rcvbuf + (ipp->ip_hl << 2));
    if (ricmp->icmp_type != ICMP_ECHOREPLY) {
#ifndef CONFIG_RTL_819X
       continue;
#else
	goto out;
#endif
    }
    if (ricmp->icmp_id == pid) {
      break;
    }
  }
#endif
  
#ifdef CONFIG_RTL_819X
out:
#endif
  close(sockfd);
  
#ifdef ECOS_DBG_STAT
   dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

//  if (ricmp != NULL && ricmp->icmp_id == pid && timeoutflag != 1) {
     if (ricmp != NULL && ricmp->icmp_id == pid) {
    errno = 0;
    syslog(LOG_WARNING, "Maybe there is an illegal host which IP is %s",
	   inet_ntoa(*ip));
//    diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
    return(BAD);
  }
//  diag_printf("%s:%d--icmp_check over!\n", __FUNCTION__,__LINE__);
  return(GOOD);
#endif
}


/*
 * timeout handling. only set timeoutflag to 1
 */
static void
timeout()
{
  timeoutflag = 1;
  return;
}


/*
 * free dhcp_binding
 */
static int
free_bind(hash_m)
  struct hash_member *hash_m;
{
  struct dhcp_binding *bp = NULL, *cbp = NULL;
  struct hash_member *current = NULL, *previous = NULL;

  bp = (struct dhcp_binding *) hash_m->data;
  previous = current = bindlist;
  while (current != NULL) {
    cbp = (struct dhcp_binding *) current->data;
    if (cbp != NULL && cidcmp(&bp->cid, &cbp->cid) == TRUE)
      break;
    else {
      previous = current;
      current = current->next;
    }
  }

  if (current == NULL) {
    if (previous != NULL) {
      previous->next = NULL;
    }
  } else {
    if (current == bindlist) {
      bindlist = current->next;
    } else {
      if (previous != NULL) {
	previous->next = current->next;
      }
    }
    free(current);
	#ifdef CONFIG_RTL_819X
	current=NULL;
	#endif
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct hash_member));
#endif
  }

  if (bp->res_name != NULL) 
  {
  	free(bp->res_name);
	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, strlen(bp->res_name)+1);
#endif
  }
  if (bp->cid.id != NULL) 
  {
  	free(bp->cid.id);
	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, bp->cid.idlen);
#endif
  }
  if (bp->haddr.haddr != NULL)
  {
  	free(bp->haddr.haddr);
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, bp->haddr.hlen);
#endif
  }
  if (bp->res != NULL) {
    bp->res->binding = NULL;
    bp->res = NULL;
  }
  free(bp);
 
#ifdef ECOS_DBG_STAT
  dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct dhcp_binding));
#endif

  #ifdef CONFIG_RTL_819X
  bp=NULL;
  #endif
  free(hash_m);
  
#ifdef ECOS_DBG_STAT
  dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, sizeof(struct hash_member));
#endif

  #ifdef CONFIG_RTL_819X
  hash_m=NULL;
  #endif
  nbind--;

//  diag_printf("%s:%d nbind=%d\n",__FUNCTION__,__LINE__, nbind);
  return(0);
}


/*
 * garbage collection. (timer driven. called once per an hour. )
 */
static void
garbage_collect()
{
#define MTOB(X)   ((struct dhcp_binding *)((X)->data))

  struct hash_member *bindptr = NULL,
                     *oldest = NULL;
  #ifdef CONFIG_RTL_819X
  struct hash_member *nextBindptr = NULL;
  #endif	
  struct dhcp_binding *tmpptr;
  time_t curr_epoch = 0;
  static time_t prev_epoch = 0;

  if (time(&curr_epoch) == -1) {
    return;
  }
  if ((unsigned long)curr_epoch - (unsigned long)prev_epoch < GC_INTERVAL)
    return;

//  diag_printf("%s:%d nbind=%d\n",__FUNCTION__,__LINE__, nbind);
  
  prev_epoch = curr_epoch;
//  dump_bind_db();

  bindptr = bindlist;
  while (bindptr != NULL) {
  	
 // 	diag_printf("%s:%d #######\n",__FUNCTION__,__LINE__);
	
    tmpptr = MTOB(bindptr);
	#if defined (CONFIG_RTL_819X)
	nextBindptr= bindptr->next;
	#endif
 //   if (tmpptr!=NULL && (tmpptr->flag & COMPLETE_ENTRY) == 0 &&
//	tmpptr->temp_epoch < curr_epoch) {
	 if ((tmpptr!=NULL ) && (((unsigned long)(tmpptr->expire_epoch) <= (unsigned long)curr_epoch) || ((tmpptr->flag & COMPLETE_ENTRY) == 0)) && (nbind>0)) {

//	 diag_printf("%s:%d #######\n",__FUNCTION__,__LINE__);

      if (tmpptr->flag & STATIC_ENTRY) {  
	tmpptr->expire_epoch = 0xffffffff;
//	diag_printf("%s:%d #######\n",__FUNCTION__,__LINE__);
	hash_del(&cidhashtable, tmpptr->cid.id, tmpptr->cid.idlen, bindcidcmp,
		 &tmpptr->cid, NULL);
	tmpptr->cid.subnet.s_addr ^= ~0;
	tmpptr->flag |= COMPLETE_ENTRY;
	if (hash_ins(&cidhashtable, tmpptr->cid.id, tmpptr->cid.idlen,
		     bindcidcmp, &tmpptr->cid, tmpptr) < 0) {
	  syslog(LOG_WARNING, "hash_ins() failure in garbage_collect()");
	}
      } else {
//      	diag_printf("%s:%d #######\n",__FUNCTION__,__LINE__);
	hash_del(&cidhashtable, tmpptr->cid.id, tmpptr->cid.idlen, bindcidcmp,
		 &tmpptr->cid, free_bind);
      }

    }
	#if defined (CONFIG_RTL_819X)
	bindptr = nextBindptr;
	#else
    bindptr = bindptr->next;
	#endif
  }

#ifdef MAX_NBIND
  while (nbind > MAX_NBIND) {
    bindptr = bindlist;
    while (bindptr != NULL) {
	#if defined (CONFIG_RTL_819X)
	 nextBindptr= bindptr->next;
	 #endif
      tmpptr = MTOB(bindptr);
      if (tmpptr!=NULL && (tmpptr->flag & COMPLETE_ENTRY) != 0 &&
	  tmpptr->expire_epoch != 0xffffffff &&
	  tmpptr->expire_epoch < curr_epoch &&
	  (oldest == NULL ||
	   tmpptr->expire_epoch < MTOB(oldest)->expire_epoch)) {
	oldest = bindptr;
      }
	  #if defined (CONFIG_RTL_819X)
      bindptr = nextBindptr;
	  #else
      bindptr = bindptr->next;
	  #endif
    }

    if (oldest == NULL) {
      return;
    } else {
      tmpptr = MTOB(oldest);
	  if(tmpptr!=NULL)
     		 hash_del(&cidhashtable, tmpptr->cid.id, tmpptr->cid.idlen, bindcidcmp,
	       &tmpptr->cid, free_bind);
      oldest = NULL;
    }
  }
#endif

  return;
}


/*
 * convert NVT ASCII to strings
 * (actually, only remove null characters)
 */
static char *
nvttostr(nvtstr, length)
  char *nvtstr;
  int length;
{
  char *msg = NULL;
  register int i = 0;
  register char *tmp = NULL;

  if ((msg = calloc(1, length + 1)) == NULL) {
    syslog(LOG_WARNING, "calloc error in nvttostr()");
    return(msg);
  }
  
#ifdef ECOS_DBG_STAT
  dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, length + 1);
#endif	

  tmp = msg;

  for (i = 0; i < length; i++) {
    if (nvtstr[i] != NULL) {
      *tmp = nvtstr[i];
      tmp++;
    }
  }

  msg[length] = '\0';
  return(msg);
}

#if defined(CONFIG_8881A_UPGRADE_KIT)
void dhcpd_force_restart(void)
{
	cyg_thread_resume(dhcpd_thread);
	dhcpd_started=1;
	dhcpd_restart(1);
}
#endif

#ifdef HAVE_SYSTEM_REINIT
void kill_dhcpd()
{
	if(dhcpd_started && dhcpd_running)
	{
		dhcpd_quitting = 1;
		while(dhcpd_quitting)
			cyg_thread_delay(10);
		
		cyg_thread_kill(dhcpd_thread);
		cyg_thread_delete(dhcpd_thread);
		dhcpd_running =0;
		dhcpd_started=0;
	}
}
#endif

__externC int dhcpd_startup(cyg_uint8 flag)
{
	diag_printf("enter dhcpd_startup\n");	
		
	if (dhcpd_started==1)
	{
		diag_printf("DHCPD has already been startup\n");
		return(-1);
	}	
	lanDnsFromWanDhcpFlag=flag;
	if (dhcpd_running==0)
	{
		cyg_thread_create(DHCPD_THREAD_PRIORITY,
		start_dhcpd,
		0,
		"dhcpd eth0",
		dhcpd_stack,
		sizeof(dhcpd_stack),
		&dhcpd_thread,
		&dhcpd_thread_object);
		
		diag_printf("Starting DHCPD thread\n");
		cyg_thread_resume(dhcpd_thread);
		dhcpd_started=1;
		return(0);
	}
	else
	{
		diag_printf("DHCPD is already running\n");
		return(-1);
	}
}


