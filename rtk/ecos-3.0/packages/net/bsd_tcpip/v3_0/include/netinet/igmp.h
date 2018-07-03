//==========================================================================
//
//      include/netinet/igmp.h
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

/*
 * Copyright (c) 1988 Stephen Deering.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Stephen Deering of Stanford University.
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
 *	@(#)igmp.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/igmp.h,v 1.10 1999/08/28 00:49:15 peter Exp $
 */

#ifndef _NETINET_IGMP_H_
#define _NETINET_IGMP_H_

#define __BIG_ENDIAN_BITFIELD

/*
 * Internet Group Management Protocol (IGMP) definitions.
 *
 * Written by Steve Deering, Stanford, May 1988.
 *
 * MULTICAST Revision: 3.5.1.2
 */

/*
 * IGMP packet format.
 */
struct igmp {
	u_char		igmp_type;	/* version & type of IGMP message  */
	u_char		igmp_code;	/* subtype for routing msgs        */
	u_short		igmp_cksum;	/* IP-style checksum               */
	struct in_addr	igmp_group;	/* group address being reported    */
};					/*  (zero for queries)             */

/* V3 group record types [grec_type] */
#define IGMPV3_MODE_IS_INCLUDE		1
#define IGMPV3_MODE_IS_EXCLUDE		2
#define IGMPV3_CHANGE_TO_INCLUDE	3
#define IGMPV3_CHANGE_TO_EXCLUDE	4
#define IGMPV3_ALLOW_NEW_SOURCES	5
#define IGMPV3_BLOCK_OLD_SOURCES	6

struct igmpv3_grec {
	u_char	grec_type;
	u_char	grec_auxwords;
	u_short	grec_nsrcs;
	struct in_addr	grec_mca;/*group address*/
	struct in_addr	grec_src[0];
};

struct igmpv3_report {
	u_char type;
	u_char resv1;
	u_short csum;
	u_short resv2;
	u_short ngrec;
	struct igmpv3_grec grec[0];
};

struct igmpv3_query {
	u_char type;
	u_char code;
	u_short csum;
	u_short group;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u_char qrv:3,
	     suppress:1,
	     resv:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	u_char resv:4,
	     suppress:1,
	     qrv:3;
#else
#error "Please fix <asm/byteorder.h>"
#endif
	u_char qqic;
	u_short nsrcs;
	struct in_addr srcs[0];
};

#define IGMP_MINLEN		     8

/*
 * Message types, including version number.
 */
#define IGMP_MEMBERSHIP_QUERY   	0x11	/* membership query         */
#define IGMP_V1_MEMBERSHIP_REPORT	0x12	/* Ver. 1 membership report */
#define IGMP_V2_MEMBERSHIP_REPORT	0x16	/* Ver. 2 membership report */
#define IGMP_V2_LEAVE_GROUP		0x17	/* Leave-group message	    */
#define IGMP_V3_REPORT 0x22

#define IGMP_DVMRP			0x13	/* DVMRP routing message    */
#define IGMP_PIM			0x14	/* PIM routing message	    */

#define IGMP_MTRACE_RESP		0x1e  /* traceroute resp.(to sender)*/
#define IGMP_MTRACE			0x1f  /* mcast traceroute messages  */

#define IGMP_MAX_HOST_REPORT_DELAY   10    /* max delay for response to     */
					   /*  query (in seconds) according */
					   /*  to RFC1112                   */


#define IGMP_TIMER_SCALE     10		/* denotes that the igmp code field */
					/* specifies time in 10th of seconds*/

/*
 * The following four defininitions are for backwards compatibility.
 * They should be removed as soon as all applications are updated to
 * use the new constant names.
 */
#define IGMP_HOST_MEMBERSHIP_QUERY	IGMP_MEMBERSHIP_QUERY
#define IGMP_HOST_MEMBERSHIP_REPORT	IGMP_V1_MEMBERSHIP_REPORT
#define IGMP_HOST_NEW_MEMBERSHIP_REPORT	IGMP_V2_MEMBERSHIP_REPORT
#define IGMP_HOST_LEAVE_MESSAGE		IGMP_V2_LEAVE_GROUP
#define IGMPV3_HOST_MEMBERSHIP_REPORT IGMP_V3_REPORT


#if defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
#define IGMP_PROXY_UP_STREAM_FLAG 0
#define IGMP_PROXY_DOWN_STREAM_FLAG 1	

#define	IGMP_ROBUSTNESS_VARIABLE		2
#define	IGMP_LAST_MEMBER_QUERY_INTERVAL		1
#define	IGMP_LAST_MEMBER_QUERY_COUNT	IGMP_ROBUSTNESS_VARIABLE
#define	IGMP_STARTUP_QUERY_INTERVAL		20

#define	IGMP_STARTUP_QUERY_COUNT		IGMP_ROBUSTNESS_VARIABLE
#define	IGMP_QUERY_INTERVAL			125

#define GENERAL_START_TIMER_TPYE	0x0
#define GENERAL_PERIODICAL_TIMER_TPYE	0x1
#define SPECIAL_TIMER_TPYE	0x10

#define ADD_MFC_FLAG  1
#define DEL_MFC_FLAG  0


struct query_timer
{
	unsigned int retry_left_time;
	unsigned long igmp_group;
	int type;
};
#endif

#endif /* _NETINET_IGMP_H_ */
