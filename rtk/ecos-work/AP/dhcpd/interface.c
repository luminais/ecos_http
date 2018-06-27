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
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <net/if.h>
#include <net/if_var.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>

#ifdef ECOS_DBG_STAT
#include "../system/sys_utility.h"

extern int dbg_dhcpd_index;
#endif

#ifdef __alpha
#define    Long    int
#define  u_Long  u_int
#else
#define    Long   long
#define  u_Long u_long
#endif
#include "common.h"

#include "syslog.h"


#if !defined(sony_news) && !defined(__osf__)
int getmac();
#endif

/********************************
 *    open the if and setup    *
 ********************************/
int open_if(struct if_info *ifinfo, char *ifname)
{
	int n=1;
	struct ifreq ifreq;
	struct sockaddr_in server_addr;

#ifdef CONFIG_RTL_819X
	int retval=-1;
#endif

	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_len = sizeof(server_addr);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = dhcps_port; // dhcps_port have htons in set_srvport() !!!

	ifinfo->fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ifinfo->fd < 0) 
	{           
		syslog(LOG_ERR, "create socket fails!\n");
		goto out;
	}
	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif
	
	if (setsockopt(ifinfo->fd, SOL_SOCKET, SO_BROADCAST, &n, sizeof(n))) 
	{
		perror("setsockopt");
		goto out;
	}	
	if (setsockopt(ifinfo->fd, SOL_SOCKET, SO_USEBCASTADDR, &n, sizeof(n))) 
	{
		diag_printf("set setsockopt SO_USEBCASTADDR fails!\n");
		goto out;
	}
	
	if (setsockopt(ifinfo->fd, SOL_SOCKET, SO_BINDTODEVICE, ifname, strlen(ifname))) 
	{
		diag_printf("set setsockopt SO_BINDTODEVICE fails!\n");
		goto out;
	}

	if (setsockopt(ifinfo->fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n))) 
	{
		perror("setsockopt SO_REUSEADDR");
		goto out;
	}

	if (setsockopt(ifinfo->fd, SOL_SOCKET, SO_REUSEPORT, &n, sizeof(n))) 
	{
		perror("setsockopt SO_REUSEPORT");
		goto out;
	}
	
	if(bind(ifinfo->fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
	{
	        perror("bind error");
		 goto out;
	}	
	
	/*
	* Initialize the interface information. subnet, IP address and mac address
	*/
	bzero(&ifreq, sizeof(ifreq));
	
	strcpy(ifreq.ifr_name, ifinfo->name);
	if (ioctl(ifinfo->fd, SIOCGIFNETMASK, &ifreq) < 0) 
	{
		syslog(LOG_ERR, "ioctl(SIOCGIFNETMASK) in open_if()");
		goto out;
	}
	if ((ifinfo->subnetmask = (struct in_addr *) calloc(1, sizeof(struct in_addr))) == NULL) 
	{
		syslog(LOG_ERR, "calloc error in open_if()");
		goto out;
	}
	
#ifdef ECOS_DBG_STAT
       dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, sizeof(struct in_addr));
#endif	

	ifinfo->subnetmask->s_addr = ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr;
	
//	diag_printf("%s:%d  subnetmask=%s\n",__FUNCTION__,__LINE__,inet_ntoa(*(ifinfo->subnetmask)));
	if (ioctl(ifinfo->fd, SIOCGIFADDR, &ifreq) < 0) 
	{
		syslog(LOG_ERR, "ioctl(SIOCGIFADDR) in open_if()");
		goto out;
	}
	if ((ifinfo->ipaddr = (struct in_addr *) calloc(1, sizeof(struct in_addr))) == NULL) 
	{
		syslog(LOG_ERR, "calloc in open_if()");
		goto out;
	}
	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, sizeof(struct in_addr));
#endif	

	ifinfo->ipaddr->s_addr = ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr;
	
//	diag_printf("%s:%d  ipaddr=%s\n",__FUNCTION__,__LINE__,inet_ntoa(*(ifinfo->ipaddr)));
	if (ioctl(ifinfo->fd, SIOCGIFHWADDR, &ifreq) < 0) 
	{
		perror("SIOCGIFHWADDR 2");
		goto out;
	}
	bcopy(ifreq.ifr_hwaddr.sa_data, ifinfo->haddr, 6);
	
	ifinfo->buf_size=IF_BUFSIZE;
	
	if ((ifinfo->buf = calloc(1, ifinfo->buf_size)) == NULL) 
	{
		syslog(LOG_ERR, "calloc in open_if()");
		goto out;
	}	
	
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, ifinfo->buf_size);
#endif	

	ifinfo->htype = 1;  //HTYPE_ETHERNET
	ifinfo->hlen = 6;	

	retval=0;
	return retval;

out:	
#ifdef CONFIG_RTL_819X
	if(ifinfo->fd>=0)
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
#endif
	return retval;
}


/*
 * select and read from the interface
 */
struct if_info *read_interfaces(struct if_info *iflist, int *n)
{
	struct if_info *ifp;
	int nfound = 0;                      /* used in select */
	fd_set readfds;                      /* used in select */
#ifndef CONFIG_RTL_819X
	static int maxfd = 0;                /* maximum number of bpf descriptors */
	static int first = 0;
	static fd_set backup;

	/* set up for select() */
	if (first == 0) 
	{
		first = 1;
		FD_ZERO(&backup);
		ifp = iflist;
		while (ifp != NULL)
		{
			FD_SET(ifp->fd, &backup);
			if (maxfd < ifp->fd)
				maxfd = ifp->fd;
			ifp = ifp->next;
		}
	}
	readfds = backup;
#else
	int maxfd=0;
	FD_ZERO(&readfds);
	ifp = iflist;
	while (ifp != NULL)
	{
		FD_SET(ifp->fd, &readfds);
		if (maxfd < ifp->fd)
			maxfd = ifp->fd;
		ifp = ifp->next;
	}
	struct timeval tv;
	tv.tv_sec=2;
	tv.tv_usec=0;
#endif

	/* block till some packet arrive */
//	if ((nfound = select(maxfd + 1, &readfds, NULL, NULL, NULL)) < 0) 
	if (!((nfound = select(maxfd + 1, &readfds, NULL, NULL, &tv)) > 0)) 
	{
		return(NULL);
	}
//	diag_printf("%s:%d\n", __FUNCTION__,__LINE__);
	/* determine the descriptor to be read */
	ifp = iflist;
	while (ifp != NULL) 
	{
		if (FD_ISSET(ifp->fd, &readfds))
			break;
		else 
			ifp = ifp->next;
	}
	if (ifp == NULL) 
	{
		return(NULL);
	}
	
	if ((*n = read(ifp->fd, ifp->buf, ifp->buf_size)) < 0)
	{
		syslog(LOG_WARNING, "read from bpf or socket");
		return(NULL);
	}
//	diag_printf("%s:%d	n=%d\n", __FUNCTION__,__LINE__,*n);
	return(ifp);
}  
