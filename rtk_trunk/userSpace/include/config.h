/*
 * Configuration file for all modules
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: config.h,v 1.4 2010-06-25 15:56:46 Exp $
 */
#ifndef	__CONFIG_H__
#define	__CONFIG_H__

#include <autoconf.h>
#include <kconfig.h>

#ifdef __CONFIG_PPP__
#define NUM_PPP CONFIG_PPP_NUM_SESSIONS
#endif

#define SHA1Init(a)		SHA1Reset(a)
#define SHA1Update(a,b,c)	SHA1Input(a,b,c)
#define SHA1Final(a,b)		SHA1Result(b,a)
#define SHA1_CTX		SHA1Context
#define SHA1_DIGEST_LENGTH	SHA1HashSize

#define SHA1_Init(a)		SHA1Reset(a)
#define SHA1_Update(a,b,c)	SHA1Input(a,b,c)
#define SHA1_Final(a,b)		SHA1Result(b,a)
#define SHA1_CTX		SHA1Context
#define SHA1_SIGNATURE_SIZE	SHA1HashSize

#endif	/* __CONFIG_H__ */
