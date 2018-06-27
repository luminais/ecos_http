/*
 * Modified by Akihiro Tominaga (tomy@sfc.wide.ad.jp)
 */
/*
 * Copyright (c) 1984, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Sun Microsystems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)arp.c       8.2 (Berkeley) 1/2/94";
#endif /* not lint */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>

#include <net/if_var.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "syslog.h"

#ifdef ECOS_DBG_STAT
#include "../system/sys_utility.h"

extern int dbg_dhcpd_index;
#endif


static int s;
static int pid;
struct sockaddr_inarp blank_sin = {sizeof(blank_sin), AF_INET}, sin_m;
#ifdef RADISH
struct sockaddr_in so_mask = {sizeof(so_mask), 0, 0, { 0xffffffff }};
#else

#ifdef CONFIG_RTL_819X
static struct sockaddr_in so_mask = {8, 0, 0, { 0xffffffff }};
#else
struct sockaddr_in so_mask = {8, 0, 0, { 0xffffffff }};
#endif

#endif
struct sockaddr_dl sdl_m;

#ifdef CONFIG_RTL_819X
static struct  {
#else
struct  {
#endif
  struct  rt_msghdr m_rtm;
  char    m_space[512];
} m_rtmsg;

void dhcpd_rtmsg(int cmd)
{
  int rlen;
  register struct rt_msghdr *rtm = &m_rtmsg.m_rtm;
  register char *cp = m_rtmsg.m_space;
  static int seq = 0;
  register int l;

  if (cmd == RTM_DELETE)
    goto doit;

  bzero((char *) &m_rtmsg, sizeof(m_rtmsg));
  rtm->rtm_version = RTM_VERSION;

  if (cmd == RTM_GET)
    rtm->rtm_addrs |= RTA_DST;

#define NEXTADDR(w, s, l) \
  if (rtm->rtm_addrs & (w)) {\
     bcopy((char *)&s, cp, sizeof(s)); cp += (l);\
  }

  NEXTADDR(RTA_DST, sin_m, sizeof(sin_m));
  NEXTADDR(RTA_GATEWAY, sdl_m, sdl_m.sdl_len);
  NEXTADDR(RTA_GATEWAY, so_mask, sizeof(so_mask));

  rtm->rtm_msglen = cp - (char *)&m_rtmsg;
doit:
  l = rtm->rtm_msglen;
  rtm->rtm_seq = ++seq;
  rtm->rtm_type = cmd;
  if ((rlen = write(s, (char *)&m_rtmsg, l)) < 0) {
    if (errno != ESRCH || cmd != RTM_DELETE) {
      syslog(LOG_ERR, "write error in rtmsg(%s)", (cmd == RTM_DELETE) ? "RTM_DELETE" : "RTM_GET");
      return;
    }
  }

  do {
    l = read(s, (char *)&m_rtmsg, sizeof(m_rtmsg));
  } while (l > 0 && (rtm->rtm_seq != seq || rtm->rtm_pid != pid));

  return;
}
  

/*
 * Delete an arp entry
 */
void
delarp(target)
  struct in_addr *target;
{
  register struct sockaddr_inarp *sin = &sin_m;
  register struct rt_msghdr *rtm = &m_rtmsg.m_rtm;
  struct sockaddr_dl *sdl;

  sin_m = blank_sin;
  sin->sin_addr.s_addr = target->s_addr;

  /* open routing socket */
  if((s = socket(PF_ROUTE, SOCK_RAW, 0)) < 0)
  {
	return;
  }
  
#ifdef ECOS_DBG_STAT
 dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

  /* get pid */
  pid = getpid();

tryagain:
  dhcpd_rtmsg(RTM_GET);
  sin = (struct sockaddr_inarp *)(rtm + 1);
  sdl = (struct sockaddr_dl *)(sin->sin_len + (char *)sin);
  if (sin->sin_addr.s_addr == sin_m.sin_addr.s_addr) {
    if (sdl->sdl_family == AF_LINK && (rtm->rtm_flags & RTF_LLINFO) &&
	!(rtm->rtm_flags & RTF_GATEWAY))
      switch (sdl->sdl_type) {
      case IFT_ETHER:
      case IFT_FDDI:
      case IFT_ISO88023:
      case IFT_ISO88024:
      case IFT_ISO88025:
	goto delete;
      }
  }
  if (sin_m.sin_other & SIN_PROXY) {
    close(s);
	
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

    return;
  } else {
    sin_m.sin_other = SIN_PROXY;
    goto tryagain;
  }
 delete:
  if (sdl->sdl_family != AF_LINK) {
    close(s);
	
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

    return;
  }

  dhcpd_rtmsg(RTM_DELETE);

  close(s);
  
#ifdef ECOS_DBG_STAT
 dbg_stat_add(dbg_dhcpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
  return;
}

