/*
 * Separate alloc/free module for wlc_xxx.c files. Decouples 
 * the code that does alloc/free from other code so data
 * structure changes don't affect ROMMED code as much.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_alloc.h,v 1.6 2010-01-23 03:35:46 Exp $
 */

extern void *wlc_calloc(osl_t *osh, uint unit, uint size);

extern wlc_info_t *wlc_attach_malloc(osl_t *osh, uint unit, uint *err, uint devid);
extern void wlc_detach_mfree(wlc_info_t *wlc, osl_t *osh);

struct wlc_bsscfg;
extern struct wlc_bsscfg *wlc_bsscfg_malloc(osl_t *osh, uint unit);
extern void wlc_bsscfg_mfree(osl_t *osh, struct wlc_bsscfg *cfg);
