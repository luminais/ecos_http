/*
 * Modified by Akihiro Tominaga. (tomy@sfc.wide.ad.jp)
 */
/*
 * Copyright (c) 1990, 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
//#include <sys/proc.h>

#include <sys/socket.h>
#include <net/if.h>

#ifdef ECOS_DBG_STAT
#include "../system/sys_utility.h"

extern int dbg_dhcpd_index;
#endif


#ifdef lint
static char rcsid[] =
    "@(#) $Header: myetheraddr.c,v 1.6 92/03/17 18:43:30 leres Exp $ (LBL)";
#endif


#include <net/if_var.h>
#include <sys/sysctl.h>
#include <net/route.h>
#include <net/if_dl.h>

#include "syslog.h"

/* Determine if "bits" is set in "flag" */
#define ALLSET(flag, bits) (((flag) & (bits)) == (bits))


int
getmac(ifname, result)
  char *ifname;
  char *result;
{
  int flags;
  struct  if_msghdr *ifm, *nextifm;
  struct  ifa_msghdr *ifam;
  struct  sockaddr_dl *sdl;
  char    *buf, *lim, *next;
  size_t needed;
  int mib[6];

  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;
  mib[3] = AF_INET;
  mib[4] = NET_RT_IFLIST;
  mib[5] = 0;

  if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
    syslog(LOG_ERR, "iflist-sysctl-estimate error in getmac(): %m");
    return(-1);
  }     
  if ((buf = malloc(needed)) == NULL) {
    syslog(LOG_ERR, "malloc error in getmac(): %m");
    return(-1);
  }
  if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
    syslog(LOG_ERR, "iflist-sysctl error in getmac(): %m");                
    return(-1);
  }
  lim = buf + needed;

  next = buf;
  while (next < lim) {

    ifm = (struct if_msghdr *)next;
                
    if (ifm->ifm_type == RTM_IFINFO) {
      sdl = (struct sockaddr_dl *)(ifm + 1);
      flags = ifm->ifm_flags;
    } else {
      syslog(LOG_ERR, "out of sync parsing NET_RT_IFLIST");
      syslog(LOG_ERR, "expected %d, got %d", RTM_IFINFO, ifm->ifm_type);
      syslog(LOG_ERR, "msglen = %d", ifm->ifm_msglen);
      syslog(LOG_ERR, "buf:%p, next:%p, lim:%p", buf, next, lim);
      return (-1);
    }

    next += ifm->ifm_msglen;
    ifam = NULL;
    while (next < lim) {

      nextifm = (struct if_msghdr *)next;

      if (nextifm->ifm_type != RTM_NEWADDR)
        break;

      if (ifam == NULL)
        ifam = (struct ifa_msghdr *)nextifm;

        next += nextifm->ifm_msglen;
    }

    /* Only look at configured, broadcast interfaces */
    if (!ALLSET(flags, IFF_UP | IFF_BROADCAST))
      continue;

    /* Only look at the specified interface */
    if (strlen(ifname) != sdl->sdl_nlen)
      continue; /* not same len */
    if (strncmp(ifname, sdl->sdl_data, sdl->sdl_nlen) != 0)
      continue; /* not same name */

    /* found */
    bcopy(LLADDR(sdl), result, 6);
    free(buf);
	
#ifdef ECOS_DBG_STAT
	  dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, needed);
#endif

    return(0);
  }
  /* not found */
  free(buf); 
  
#ifdef ECOS_DBG_STAT
	   dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, needed);
#endif

  errno = 0;
  syslog(LOG_ERR, "getmac() failed");
  return(-1);
}

