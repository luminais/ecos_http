/*
 * Wake-on-Wireless related header file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_wowl.h,v 1.3 2009-05-21 00:34:38 Exp $
*/


#ifndef _wlc_wowl_h_
#define _wlc_wowl_h_

#ifdef WOWL
extern wowl_info_t *wlc_wowl_attach(wlc_info_t *wlc);
extern void wlc_wowl_detach(wowl_info_t *wowl);
extern bool wlc_wowl_cap(struct wlc_info *wlc);
extern bool wlc_wowl_enable(wowl_info_t *wowl);
extern uint32 wlc_wowl_clear(wowl_info_t *wowl);
#ifdef WIN7
extern void wlc_wowl_set_wpa_m1(wowl_info_t *wowl);
extern void wlc_wowl_set_eapol_id(wowl_info_t *wowl);
extern int wlc_wowl_set_key_info(wowl_info_t *wowl, void *kek, int kek_len,	void* kck,
	int kck_len, void *replay_counter, int replay_counter_len, ulong offload_id);
extern int wlc_wowl_remove_key_info(wowl_info_t *wowl, ulong offload_id);
extern int wlc_wowl_get_key_info(wowl_info_t *wowl, void *replay_counter, int *len);
#endif /* WIN7 */
#else	/* stubs */
#define wlc_wowl_attach(a) (wowl_info_t *)0x0dadbeef
#define	wlc_wowl_detach(a) do {} while (0)
#define wlc_wowl_cap(a) FALSE
#define wlc_wowl_enable(a) FALSE
#define wlc_wowl_clear(b) (0)
#ifdef WIN7
#define wlc_wowl_set_wpa_m1(a)
#define wlc_wowl_set_eapol_id(a)
#define wlc_wowl_set_key_info(a, b, c, d, e, f, g, h) (0)
#define wlc_wowl_remove_key_info(a, b) (0)
#define wlc_wowl_get_key_info(a, b, c) (0)
#endif /* WIN7 */
#endif /* WOWL */

/* number of WOWL patterns supported */
#define MAXPATTERNS(wlc) \
	(wlc_wowl_cap(wlc) ? (D11REV_GE((wlc)->pub->corerev, 15) ? 8 : 4) : 0)

#endif /* _wlc_wowl_h_ */
