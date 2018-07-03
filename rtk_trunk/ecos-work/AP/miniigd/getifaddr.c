/* project: miniupnp
 * webpage: http://miniupnp.free.fr/
 * (c) 2006 Thomas Bernard
 * this software is subject to the conditions detailed in the
 * LICENCE file provided in this distribution */
/* $Id: getifaddr.c,v 1.2 2007-08-31 11:36:38 chien_hsiang Exp $ */
#ifdef __ECOS
#include <network.h>
#endif
#include <string.h>
#ifndef __ECOS
#include <syslog.h>
#endif
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#ifdef 	ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif
#ifdef	ECOS_DBG_STAT
extern int	dbg_igd_index;
#endif

#define syslog(x, fmt, args...);

int
getifaddr(const char * ifname, char * buf, int len)
{
	/* SIOCGIFADDR struct ifreq *  */
	int s;
	struct ifreq ifr;
	int ifrlen;
	struct sockaddr_in * addr;
	ifrlen = sizeof(ifr);
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if(s<0)
	{
		syslog(LOG_ERR, "socket(PF_INET, SOCK_DGRAM, 0): %m");
		return -1;
	}
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif
       // syslog(LOG_INFO,"name1=%s, name2=%s SIZE=%d\n",ifr.ifr_name,ifname,IFNAMSIZ);
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
       // syslog(LOG_INFO,"Xname1=%s, name2=%s SIZE=%d\n",ifr.ifr_name,ifname,IFNAMSIZ);
	if(ioctl(s, SIOCGIFADDR, &ifr, &ifrlen) < 0)
	{
		syslog(LOG_ERR, "ioctl(s, SIOCGIFADDR, ...): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
	}
	addr = (struct sockaddr_in *)&ifr.ifr_addr;
	if(!inet_ntop(AF_INET, &addr->sin_addr, buf, len))
	{
		syslog(LOG_ERR, "inet_ntop FAILED");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
	}
	close(s);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
	return 0;
}

