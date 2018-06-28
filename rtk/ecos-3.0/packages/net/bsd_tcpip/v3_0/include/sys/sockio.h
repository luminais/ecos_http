//==========================================================================
//
//      include/sys/sockio.h
//
//==========================================================================
// ####BSDCOPYRIGHTBEGIN####                                    
// -------------------------------------------                  
// This file is part of eCos, the Embedded Configurable Operating System.
//
// Portions of this software may have been derived from FreeBSD 
// or other sources, and if so are covered by the appropriate copyright
// and license included herein.                                 
//
// Portions created by the Free Software Foundation are         
// Copyright (C) 2002 Free Software Foundation, Inc.            
// -------------------------------------------                  
// ####BSDCOPYRIGHTEND####                                      
//==========================================================================

/*-
 * Copyright (c) 1982, 1986, 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *
 *	@(#)sockio.h	8.1 (Berkeley) 3/28/94
 * $FreeBSD: src/sys/sys/sockio.h,v 1.14.2.4 2001/07/24 19:10:19 brooks Exp $
 */

#ifndef	_SYS_SOCKIO_H_
#define	_SYS_SOCKIO_H_

#include <sys/ioccom.h>

/* Socket ioctl's. */
#define	SIOCSHIWAT	 _IOW('s',  0, int)		/* set high watermark */
#define	SIOCGHIWAT	 _IOR('s',  1, int)		/* get high watermark */
#define	SIOCSLOWAT	 _IOW('s',  2, int)		/* set low watermark */
#define	SIOCGLOWAT	 _IOR('s',  3, int)		/* get low watermark */
#define	SIOCATMARK	 _IOR('s',  7, int)		/* at oob mark? */
#define	SIOCSPGRP	 _IOW('s',  8, int)		/* set process group */
#define	SIOCGPGRP	 _IOR('s',  9, int)		/* get process group */

#define	SIOCADDRT	 _IOW('r', 10, struct ecos_rtentry)	/* add route */
#define	SIOCDELRT	 _IOW('r', 11, struct ecos_rtentry)	/* delete route */
#define	SIOCGETVIFCNT	_IOWR('r', 15, struct sioc_vif_req)/* get vif pkt cnt */
#define	SIOCGETSGCNT	_IOWR('r', 16, struct sioc_sg_req) /* get s,g pkt cnt */

/*
 * @ Source Code, from BCM project, Ported by zhuhuan @
 * Function name: NULL
 * Description: The following code is used for ARP ioctl 
 * Date: 2015.12.18
 */
// +++++
#define	SIOSGARPRT	 _IOR('r', 17, struct ecos_arpentry)	/* arpioctl */
#define	SIOCGARPRT	 _IOR('r', 18, struct ecos_arpentry)	/* arpioctl */
#define	SIOCDELARPRT _IOW('r', 19, struct ecos_arpentry)	/* arpioctl */
#define SIOCGARPNU	 _IOW('r', 20, struct ecos_arpentry)	/* arpioctl */
// +++++

#define	SIOCSIFADDR	 _IOW('i', 12, struct ifreq)	/* set ifnet address */
#define	OSIOCGIFADDR	_IOWR('i', 13, struct ifreq)	/* get ifnet address */
#define	SIOCGIFADDR	_IOWR('i', 33, struct ifreq)	/* get ifnet address */
#define	SIOCSIFDSTADDR	 _IOW('i', 14, struct ifreq)	/* set p-p address */
#define	OSIOCGIFDSTADDR	_IOWR('i', 15, struct ifreq)	/* get p-p address */
#define	SIOCGIFDSTADDR	_IOWR('i', 34, struct ifreq)	/* get p-p address */
#define	SIOCSIFFLAGS	 _IOW('i', 16, struct ifreq)	/* set ifnet flags */
#define	SIOCGIFFLAGS	_IOWR('i', 17, struct ifreq)	/* get ifnet flags */
#define	OSIOCGIFBRDADDR	_IOWR('i', 18, struct ifreq)	/* get broadcast addr */
#define	SIOCGIFBRDADDR	_IOWR('i', 35, struct ifreq)	/* get broadcast addr */
#define	SIOCSIFBRDADDR	 _IOW('i', 19, struct ifreq)	/* set broadcast addr */
#define	OSIOCGIFCONF	_IOWR('i', 20, struct ifconf)	/* get ifnet list */
#define	SIOCGIFCONF	_IOWR('i', 36, struct ifconf)	/* get ifnet list */
#define	OSIOCGIFNETMASK	_IOWR('i', 21, struct ifreq)	/* get net addr mask */
#define	SIOCGIFNETMASK	_IOWR('i', 37, struct ifreq)	/* get net addr mask */
#define	SIOCSIFNETMASK	 _IOW('i', 22, struct ifreq)	/* set net addr mask */
#define	SIOCGIFMETRIC	_IOWR('i', 23, struct ifreq)	/* get IF metric */
#define	SIOCSIFMETRIC	 _IOW('i', 24, struct ifreq)	/* set IF metric */
#define	SIOCDIFADDR	 _IOW('i', 25, struct ifreq)	/* delete IF addr */
#define	SIOCAIFADDR	 _IOW('i', 26, struct ifaliasreq)/* add/chg IF alias */

#define	SIOCALIFADDR	_IOW('i', 27, struct if_laddrreq) /* add IF addr */
#define	SIOCGLIFADDR	_IOWR('i', 28, struct if_laddrreq) /* get IF addr */
#define	SIOCDLIFADDR	_IOW('i', 29, struct if_laddrreq) /* delete IF addr */

#define	SIOCADDMULTI	 _IOW('i', 49, struct ifreq)	/* add m'cast addr */
#define	SIOCDELMULTI	 _IOW('i', 50, struct ifreq)	/* del m'cast addr */
#define	SIOCGIFMTU	_IOWR('i', 51, struct ifreq)	/* get IF mtu */
#define	SIOCSIFMTU	 _IOW('i', 52, struct ifreq)	/* set IF mtu */
#define	SIOCGIFPHYS	_IOWR('i', 53, struct ifreq)	/* get IF wire */
#define	SIOCSIFPHYS	 _IOW('i', 54, struct ifreq)	/* set IF wire */
#define	SIOCSIFMEDIA	_IOWR('i', 55, struct ifreq)	/* set net media */
#define	SIOCGIFMEDIA	_IOWR('i', 56, struct ifmediareq) /* get net media */

#define	SIOCSIFPHYADDR   _IOW('i', 70, struct ifaliasreq) /* set gif addres */
#define	SIOCGIFPSRCADDR	_IOWR('i', 71, struct ifreq)	/* get gif psrc addr */
#define	SIOCGIFPDSTADDR	_IOWR('i', 72, struct ifreq)	/* get gif pdst addr */
#define	SIOCDIFPHYADDR	 _IOW('i', 73, struct ifreq)	/* delete gif addrs */
#define	SIOCSLIFPHYADDR	 _IOW('i', 74, struct if_laddrreq) /* set gif addrs */
#define	SIOCGLIFPHYADDR	_IOWR('i', 75, struct if_laddrreq) /* get gif addrs */

#define	SIOCSIFGENERIC	 _IOW('i', 57, struct ifreq)	/* generic IF set op */
#define	SIOCGIFGENERIC	_IOWR('i', 58, struct ifreq)	/* generic IF get op */
#ifdef CONFIG_RTL_819X
#define	SIOCGIFSTATUS	_IOWR('i', 59, struct ifreq)	/* get IF status */
#else
#define	SIOCGIFSTATUS	_IOWR('i', 59, struct ifstat)	/* get IF status */
#endif
#define	SIOCSIFLLADDR	_IOW('i', 60, struct ifreq)	/* set link level addr */

#define SIOCIFCREATE	_IOWR('i', 122, struct ifreq)	/* create clone if */
#define SIOCIFDESTROY	 _IOW('i', 121, struct ifreq)	/* destroy clone if */
#define SIOCIFGCLONERS	_IOWR('i', 120, struct if_clonereq) /* get cloners */

#define FIONBIO          _IOW('s', 80, int)             /* set non-blocking I/O */
#define FIOASYNC         _IOW('s', 81, int)             /* set async I/O */
#define FIONREAD         _IOR('s', 82, int)             /* get number of avail chars */

#define SIOCGIFHWADDR   _IOWR('i',100, struct ifreq)    /* get MAC address */
#define SIOCSIFHWADDR    _IOW('i',101, struct ifreq)    /* set MAC address */
/* NB these two take a struct ifreq followed by the useful data */
#define SIOCGIFSTATSUD  _IOWR('i',102, struct ifreq)    /* get uptodate if stats */
#define SIOCGIFSTATS    _IOWR('i',103, struct ifreq)    /* get interface stats */

#define SIOCBRDGADD     _IOWR('i', 160, struct ifbreq)   /* add bridge ifs */
#define SIOCBRDGDEL     _IOWR('i', 161, struct ifbreq)   /* del bridge ifs */
#define SIOCBRDGGIFFLGS _IOWR('i', 162, struct ifbreq)   /* get brdg if flags */
#define SIOCBRDGSIFFLGS _IOWR('i', 163, struct ifbreq)   /* set brdg if flags */
#define SIOCBRDGSCACHE  _IOWR('i', 164, struct ifbcachereq) /* set cache size */
#define SIOCBRDGGCACHE  _IOWR('i', 165, struct ifbcachereq) /* get cache size */
#define SIOCBRDGADDS     _IOW('i', 165, struct ifbreq)   /* add span port */
#define SIOCBRDGIFS     _IOWR('i', 166, struct ifbreq)   /* get member ifs */
#define SIOCBRDGDELS     _IOW('i', 166, struct ifbreq)   /* del span port */
#define SIOCBRDGRTS     _IOWR('i', 167, struct ifbaconf) /* get addresses */
#define SIOCBRDGSADDR   _IOWR('i', 168, struct ifbareq)  /* set addr flags */
#define SIOCBRDGSTO     _IOWR('i', 169, struct ifbcachetoreq) /* cache timeout */
#define SIOCBRDGGTO     _IOWR('i', 170, struct ifbcachetoreq) /* cache timeout */
#define SIOCBRDGDADDR   _IOWR('i', 171, struct ifbareq)  /* delete addr */
#define SIOCBRDGFLUSH   _IOWR('i', 172, struct ifbreq)   /* flush addr cache */

#define SIOCBRDGARL     _IOWR('i', 177, struct ifbrlreq) /* add bridge rule */
#define SIOCBRDGFRL     _IOWR('i', 178, struct ifbrlreq) /* flush brdg rules */
#define SIOCBRDGGRL     _IOWR('i', 179, struct ifbrlconf)/* get bridge rules */

#define SIOCBRDGGPRI    _IOWR('i', 180, struct ifbrparam) /* get priority */
#define SIOCBRDGSPRI     _IOW('i', 180, struct ifbrparam) /* set priority */
#define SIOCBRDGGHT     _IOWR('i', 181, struct ifbrparam) /* get hello time */
#define SIOCBRDGSHT      _IOW('i', 181, struct ifbrparam) /* set hello time */
#define SIOCBRDGGFD     _IOWR('i', 182, struct ifbrparam) /* get forward delay */
#define SIOCBRDGSFD      _IOW('i', 182, struct ifbrparam) /* set forward delay */
#define SIOCBRDGGMA     _IOWR('i', 183, struct ifbrparam) /* get max age */
#define SIOCBRDGSMA      _IOW('i', 183, struct ifbrparam) /* set max age */
#define SIOCBRDGSIFPRIO _IOWR('i', 184, struct ifbreq)   /* set if priority */
#define SIOCBRDGSIFCOST _IOWR('i', 185, struct ifbreq)   /* set if cost */
//#ifdef BCM47XX
#if 1	//llm modify
#define SIOCGIFNAME	_IOWR('i', 133, struct ifreq)
#define SIOCGIFLINK	_IOWR('i', 134, struct ifreq)
#define SIOCSIFLINK	 _IOW('i', 135, struct ifreq)
#define SIOCGIFBAUD	_IOWR('i', 136, struct ifreq)
#define SIOCSIFBAUD	 _IOW('i', 137, struct ifreq)
#define SIOCGIFSPEED	_IOWR('i', 138, struct ifreq)
#define SIOCSIFSPEED	 _IOW('i', 139, struct ifreq)
#define SIOCGIFDATA	_IOWR('i', 140, struct ifreq)   /* get if_data */
#define SIOCSIFFLTFLAGS	 _IOW('i', 141, struct ifreq)   /* set ifnet flags */
#define SIOCGIFFLTFLAGS	_IOWR('i', 142, struct ifreq)   /* get ifnet flags */
#define SIOCSETQOS	 _IOW('i', 143, struct ifreq)
#define SIOCETHTOOL	 _IOW('i', 145, struct ifreq)
#define SIOCDEVPRIVATE	_IOWR('i', 146, struct ifreq)
#endif /* BCM47XX */
#endif /* !_SYS_SOCKIO_H_ */
