/*
 * AP Channel/Chanspec Selection interface.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_apcs.h,v 1.5 2010-01-07 19:32:08 Exp $
 */

#ifndef _wlc_apcs_h_
#define _wlc_apcs_h_

#ifdef AP
extern int wlc_cs_scan_start(wlc_info_t *wlc, wl_uint32_list_t *request, bool bw40,
	bool active, int bandtype, uint8 reason,
	void (*cb)(void *arg, int status), void *arg);
extern void wlc_cs_scan_timer(wlc_info_t *wlc);
#else
#define wlc_cs_scan_start(wlc, request, bw40, active, bandtype, reason, cb, arg) BCME_BADARG
#define wlc_cs_scan_timer(wlc) 0xdeadbeef

#endif /* AP */
#endif	/* _wlc_apcs_h_ */
