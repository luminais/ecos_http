/*
 * IP filter fast-check definitions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: ip_filfast.h,v 1.2 2010-07-30 06:38:09 Exp $
 *
 */
#ifndef	__IP_FILFAST_H__
#define __IP_FILFAST_H__

#define	SIOCSFILFAST	_IOW('f', 60, int)
#define	SIOCSFASTNAT	_IOW('f', 61, int)
#define	SIOCSMACFIL	_IOW('f', 62, int)
#define	SIOCADMACFR	_IOW('f', 63, struct macfilter *)
#define	SIOCRMMACFR	_IOW('f', 64, struct macfilter *)
#define	SIOCFLMACFR	 _IO('f', 65)
#define	SIOCSURLFIL	_IOW('f', 66, int)
#define	SIOCADURLFR	_IOW('f', 67, struct urlfilter *)
#define	SIOCRMURLFR	_IOW('f', 68, struct urlfilter *)
#define	SIOCFLURLFR	 _IO('f', 69)

#ifdef __CONFIG_TENDA_MULTI__
#define	SIOCSFIL_QUEBB	_IOW('f', 70, int)
#endif
#define 	SIOCSWAN2LANFIL _IOW('f', 71, int)
#define	SIOCADWAN2LAN_IP	_IOW('f', 72, int)
#define	SIOCADWAN2LAN_MASK	_IOW('f', 73, int)


//#define  __CONFIG_AL_SECURITY__ 1//define by ll
#ifdef __CONFIG_AL_SECURITY__
#define SIOCALSECURITY	_IOW('f', 74, int)
#endif
//add by ll
#define	SIOCSNISFASTCHECK	_IOW('f', 74, int)
#define	SIOCSURLRECORDMODE	_IOW('f', 75, int)
//end by ll

//luminais mark
#define SIOCJSINJECTCHECK _IOW('f', 76, int)
//luminais

#ifdef _KERNEL
int filfast_ioctl(int cmd, caddr_t data);
#endif

#endif	/* __IP_FILFAST_H__ */
