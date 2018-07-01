//==========================================================================
//
//      include/sys/malloc.h
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

#ifndef _SYS_MALLOC_H_
#define _SYS_MALLOC_H_

#ifndef _KERNEL
//#warn This file is only suitable for kernel level code in a network stack!
#endif

/*
 * flags to malloc
 */
#define	M_WAITOK	0x0000
#define	M_NOWAIT	0x0001
#define M_ZERO          0x0008    // bzero (clear) allocated area


#if defined(__ECOS) && defined(_KERNEL) &&defined(CONFIG_RTL_819X)
#ifdef CYGPKG_HAL_MIPS_RLX
externC __attribute__((nomips16)) void *cyg_net_malloc(u_long size, int type, int flags);
externC __attribute__((nomips16)) void cyg_net_free(caddr_t addr, int type);
#else
externC void *cyg_net_malloc(u_long size, int type, int flags);
externC void cyg_net_free(caddr_t addr, int type);
#endif
#define	MALLOC(space, cast, size, type, flags) \
	(space) = (cast)cyg_net_malloc((u_long)(size), (int)type, flags)
#define malloc(size, type, flags) cyg_net_malloc((u_long)size, (int)type, flags)
#define	FREE(addr, type) cyg_net_free((caddr_t)(addr), (int)type)
#define free(addr, type) FREE(addr, (int)type)
#endif

// Memory types
#define M_DEVBUF        3
#define M_PCB           (void *)4       /* protocol control block */
#define M_RTABLE        5       /* routing tables */
#define M_IFADDR        9       /* interface address */
#define M_IFMADDR       55      /* link-level multicast address */
#define M_IPMADDR       66      /* link-level multicast address */
#define M_IGMP          99      /* gateway info */
#define M_SONAME        98
#define M_IPMOPTS       97
#define M_TSEGQ         96
#define M_ACCF          95      /* accept filter data */
#define M_TEMP          94      /* misc temp buffers */
#define M_IPFLOW        93
#define M_NETADDR       92
#define M_IP6OPT        91
#define M_IP6NDP        90
#define M_MRTABLE       89
#define M_FTABLE        88
#define M_SYSCTLOID    100     
#define M_SYSCTL       101     
#define M_SECA         102
#if defined(CONFIG_RTL_819X)	//jwj:20120612
#define M_IPFW		103
#define M_NETGRAPH      110					//hf_shi 20120828
#if defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
#define M_NAPT		111
#endif
#ifdef __ECOS
#define M_WIFI		112
#endif
#if 1//defined(HAVE_NAT_ALG)
#define M_ALG		113
#endif
#endif

#endif // _SYS_MALLOC_H_
