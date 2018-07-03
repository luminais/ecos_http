/*
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE".  Use of the mrouted program represents acceptance of
 * the terms and conditions listed in that file.
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 *
 * config.c,v 3.8.4.10 1998/01/06 01:57:41 fenner Exp
 */

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include "defs.h"


struct ifconf ifc;

#if !defined(AF_LINK)
#define	SA_LEN(sa)	sizeof(struct sockaddr)
#endif

#if !defined(SA_LEN)
#define	SA_LEN(sa)	(sa)->sa_len
#endif

#define	SALIGN	(sizeof(long) - 1)
#define	SA_RLEN(sa)	((sa)->sa_len ? (((sa)->sa_len + SALIGN) & ~SALIGN) : (SALIGN + 1))

#ifndef	ALIGNBYTES
/*
 * On systems with a routing socket, ALIGNBYTES should match the value
 * that the kernel uses when building the messages.
 */
#define	ALIGNBYTES	XXX
#endif
#ifndef	ALIGN
#define	ALIGN(p)	(((u_long)(p) + ALIGNBYTES) &~ ALIGNBYTES)
#endif
struct ifaddrs {
	struct ifaddrs  *ifa_next;
	char		*ifa_name;
	u_int		 ifa_flags;
	struct sockaddr	*ifa_addr;
	struct sockaddr	*ifa_netmask;
	struct sockaddr	*ifa_dstaddr;
	void		*ifa_data;
};
#ifndef	ifa_broadaddr
#define	ifa_broadaddr	ifa_dstaddr	/* broadcast address interface */
#endif

u_int32 igmp_down_if_addr = 0;
u_int32 igmp_down_if_mask = 0;

int config_vifs_from_kernel()
{
    int icnt = 1;  // Interface count
    int dcnt = 0;  // Data [length] count
    int ncnt = 0;  // Length of interface names
    char *buf;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
#define	IF_WORK_SPACE_SZ	4096
#else
#define	IF_WORK_SPACE_SZ	2048
#endif

    int i, sock;
#ifdef CYGPKG_NET_INET6
 //   int sock6;
//    struct in6_ifreq ifrq6;
#endif
    struct ifconf ifc;
    struct ifreq *ifr, *lifr;
    struct ifreq ifrq;
    char *data, *names;
    struct ifaddrs *ifa, *ift;
	//diag_printf("config_vifs_from_kernel,[%s]:[%d].\n",__FUNCTION__,__LINE__);
	numvifs=0;
	igmp_down_if_idx = 0xFF;
	igmp_up_if_idx= 0xFF;
	
    buf = malloc(IF_WORK_SPACE_SZ);
    if (buf == NULL)
        return (0);
#ifdef 	ECOS_DBG_STAT
		dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 0);
#endif
    ifc.ifc_buf = buf;
    ifc.ifc_len = IF_WORK_SPACE_SZ;
	
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        free(buf);
	
		#ifdef 	ECOS_DBG_STAT
			dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 0);
		#endif
		
        return (0);
    }
#ifdef 	ECOS_DBG_STAT
	dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif
    i =  ioctl(sock, SIOCGIFCONF, (char *)&ifc);

    if (i < 0) {
        close(sock); 
		#ifdef 	ECOS_DBG_STAT
			dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
		#endif

        free(buf);
		#ifdef 	ECOS_DBG_STAT
			dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 0);
		#endif

        return (0);
    }

    ifr = ifc.ifc_req;
    lifr = (struct ifreq *)&ifc.ifc_buf[ifc.ifc_len];

    while (ifr < lifr) {
        struct sockaddr *sa;

        sa = &ifr->ifr_addr;
        ++icnt;
        dcnt += SA_RLEN(sa) * 3;  /* addr, mask, brdcst */
        ncnt += sizeof(ifr->ifr_name) + 1;
		
        if (SA_LEN(sa) < sizeof(*sa))
            ifr = (struct ifreq *)(((char *)sa) + sizeof(*sa));
        else
            ifr = (struct ifreq *)(((char *)sa) + SA_LEN(sa));
    }

    if (icnt + dcnt + ncnt == 1) {
        // Nothing found
       // *pif = NULL;
        free(buf);
#ifdef 	ECOS_DBG_STAT
		dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 0);
#endif
		
        close(sock);
		
#ifdef 	ECOS_DBG_STAT
		dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
        return (0);
    }
	ifr = ifc.ifc_req;
	while (ifr < lifr) {
		struct sockaddr * sa;

		sa = &ifr->ifr_addr;
      
		//diag_printf("ifr->ifr_name:%s,sa->sa_family:%d,[%s]:[%d].\n",ifr->ifr_name,sa->sa_family,__FUNCTION__,__LINE__);
        if (sa->sa_family == AF_INET) {
			struct sockaddr *sa_netmask = NULL;
			struct sockaddr *sa_broadcast = NULL;
			struct sockaddr *sa_dst = NULL;
			// diag_printf("ifr->ifr_name:%s,sa->sa_family:%d,[%s]:[%d].\n",ifr->ifr_name,sa->sa_family,__FUNCTION__,__LINE__);
			memset(&ifrq,0,sizeof(ifrq));
			strcpy(ifrq.ifr_name,ifr->ifr_name);
			if(ioctl( sock, SIOCGIFFLAGS, &ifrq ) < 0)
				 log(LOG_ERR, errno, "ioctl SIOCGIFFLAGS error");
			short flags; 
			flags=ifrq.ifr_flags;
		  if ((ifrq.ifr_flags & (IFF_LOOPBACK|IFF_MULTICAST)) != IFF_MULTICAST){
				//diag_printf("IF:%s,flags:%x,next.[%s]:[%d].\n",ifrq.ifr_name,ifrq.ifr_flags,__FUNCTION__,__LINE__);
				goto next;
			}	
          //ift->ifa_flags = ifrq.ifr_flags;
		  memset(&ifrq,0,sizeof(ifrq));
          strcpy(ifrq.ifr_name,ifr->ifr_name);
		  if( ioctl( sock, SIOCGIFADDR, &ifrq ) < 0)		  	
			  log(LOG_ERR, errno, "ioctl SIOCGIFADDR error");
		 //diag_printf("ifr->ifr_name:%s,addr:%x.[%s]:[%d].\n",ifr->ifr_name,ifrq.ifr_addr.sa_data,__FUNCTION__,__LINE__);
		 //diag_printf("ifr->ifr_name:%s,sa->sa_family:%d,[%s]:[%d].\n",ifr->ifr_name,sa->sa_family,__FUNCTION__,__LINE__);		
          //memcpy(&ifrq.ifr_addr, ift->ifa_addr,sizeof(struct sockaddr));

			/*
	 * Ignore any interface that is connected to the same subnet as
	 * one already installed in the uvifs array.
	 */
	u_int32 addr, mask, subnet; 
	register struct uvif *v;
    register vifi_t vifi; 
	addr = ((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr.s_addr;
 	memset(&ifrq,0,sizeof(ifrq));
    strcpy(ifrq.ifr_name,ifr->ifr_name);
	if (ioctl(sock, SIOCGIFNETMASK, &ifrq) < 0)
	    log(LOG_ERR, errno, "ioctl SIOCGIFNETMASK for %s", ifrq.ifr_name);
	mask = ((struct sockaddr_in *)&ifrq.ifr_addr)->sin_addr.s_addr;
	//mask = ((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr.s_addr;
	subnet = addr & mask;
	//diag_printf("if:%s,addr:%x,[%s]:[%d].\n",ifr->ifr_name,addr,__FUNCTION__,__LINE__);
	for (vifi = 0, v = uvifs; vifi < numvifs; ++vifi, ++v) {
	    if (strcmp(v->uv_name, ifr->ifr_name) == 0) {
		log(LOG_DEBUG, 0, "skipping %s (%s on subnet %s) (alias for vif#%u?)",
			v->uv_name, inet_fmt(addr, s1),
			inet_fmts(subnet, mask, s2), vifi);
		//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
		break;
    }
	}
	/*
	 * If there is room in the uvifs array, install this interface.
	 */
	if(strcmp(ifr->ifr_name,igmp_down_if_name)==0) {
		
		igmp_down_if_idx=numvifs;
		igmp_down_if_addr = addr;
		igmp_down_if_mask = mask;
	}	
	else if(strcmp(ifr->ifr_name,igmp_up_if_name)==0){ 
		
		igmp_up_if_idx=numvifs;
		
	}	
	
	if (numvifs == MAXVIFS) {
	    diag_printf("too many vifs, ignoring %s.\n", ifr->ifr_name);
	    break;
	}
	v  = &uvifs[numvifs];
	zero_vif(v, 0);
	
	//addr: network endian
	//v->uv_lcl_addr    = ntohl(addr);
	v->uv_lcl_addr	  = addr;
	v->uv_subnet      = subnet;
	v->uv_subnetmask  = mask;
	v->uv_subnetbcast = subnet | ~mask;
	strncpy(v->uv_name, ifr->ifr_name, IFNAMSIZ);
	v->uv_name[IFNAMSIZ-1] = '\0';
	v->uv_flags=0;
	#if 0
	if (flags & IFF_POINTOPOINT)
	   v->uv_flags |= VIFF_REXMIT_PRUNES;
	#endif
	DBG_PRINT("installing %s (%s on subnet %s) as vif #%u - rate=%d",
	    v->uv_name, inet_fmt(addr, s1), inet_fmts(subnet, mask, s2),
	    numvifs, v->uv_rate_limit);
	DBG_PRINT("if: %s,addr:%x.%x.flags:%x\n", ifr->ifr_name,addr,v->uv_lcl_addr,flags);

	++numvifs;
	
	if (!(flags & IFF_UP)) {
	    v->uv_flags |= VIFF_DOWN;
	    vifs_down = TRUE;
	}
}
next:	        
        if (SA_LEN(sa) < sizeof(*sa))
            ifr = (struct ifreq *)(((char *)sa) + sizeof(*sa));
        else
            ifr = (struct ifreq *)(((char *)sa) + SA_LEN(sa));

		
    }
	
    free(buf);
#ifdef 	ECOS_DBG_STAT
	dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 0);
#endif
	
	
#ifdef CYGPKG_NET_INET6
    //close(sock6);
#ifdef 	ECOS_DBG_STAT
	dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif

#endif

    close(sock);
#ifdef 	ECOS_DBG_STAT
	dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
	
	if(numvifs<2||((igmp_up_if_idx&0xFF)==0xFF)||((igmp_down_if_idx&0xFF)==0xFF)){
		//diag_printf("numvifs: %d,igmp_up_if_idx:%x,igmp_down_if_idx:%x\n",numvifs, igmp_up_if_idx,igmp_down_if_idx);
		return (0);
	}	
	else
    	return (1);
}
