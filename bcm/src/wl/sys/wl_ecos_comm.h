/*
 * Required functions exported by the port-specific (os-dependent) driver
 * to common (os-independent) driver code.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wl_ecos_comm.h,v 1.3.94.1 2010-12-21 02:38:57 Exp $
 */

#ifndef _wl_ecos_comm_h_
#define _wl_ecos_comm_h_

struct osl_t;
struct wlc_if;
struct wl_rxsts;
struct wlc_event;

extern bool wl_ecos_comm_wlc_chipmatch(uint16 vendor, uint16 device);
extern bool wl_ecos_comm_wlc_sendpkt(void *wlc, void *sdu, void *wlcif);
extern bool wl_ecos_comm_wlc_intrsupd(void *wlc);
extern bool wl_ecos_comm_wlc_dpc(void *wlc, bool bounded);
extern void *wl_ecos_comm_wlc_attach(void *wl, uint16 vendor, uint16 device, uint unit,
                                     bool piomode, osl_t *osh, void *regsva,
                                     uint bustype, void *btparam, uint *perr);
extern void wl_ecos_comm_wlc_detach(void *wlc);
extern void wl_ecos_comm_wlc_reset(void *wlc);
extern void wl_ecos_comm_wlc_init(void *wlc);
extern void wl_ecos_comm_wlc_intrson(void *wlc);
extern uint32 wl_ecos_comm_wlc_intrsoff(void *wlc);
extern void wl_ecos_comm_wlc_intrsrestore(void *wlc, uint32 macintmask);
extern int  wl_ecos_comm_wlc_up(void *wlc);
extern uint wl_ecos_comm_wlc_down(void *wlc);
extern int wl_ecos_comm_wlc_set(void *wlc, int cmd, int arg);
extern int wl_ecos_comm_wlc_ioctl(void *wlc, int cmd, void *arg, int len, void *wlcif);
extern bool wl_ecos_comm_wlc_isr(void *wlc, bool *wantdpc);
extern void wl_ecos_comm_wlc_intrsoff_isr(void *wlc);
extern bool wl_ecos_comm_wlc_sup_mic_error(void *wlc, bool group);

extern int wl_ecos_comm_iovar_op(void *wlc, const char *name,
        void *arg, int len, bool set, void *wlcif);
extern void *wl_ecos_comm_pub(void *wlc);
extern void *wl_ecos_comm_osh(void *pub);
extern bool wl_ecos_comm_up(void *pub);
extern uint wl_ecos_comm_get_unit(void *pub);
extern void wl_ecos_comm_set_mac(void *pub, char *addr);
extern void wl_ecos_comm_get_mac(void *pub, char *addr);
extern void wl_ecos_comm_set_allmulti(void *wlc, bool value);
extern void wl_ecos_comm_set_multicastlist(void *wlc, struct maclist *maclist, int buflen);
extern bool wl_ecos_comm_get_ap(void *pub);
extern bool wl_ecos_comm_get_associated(void *pub);

extern uint32 wl_ecos_comm_get_event_type(void *e);
extern uint8 wl_ecos_comm_get_event_flags(void *e);
extern 	struct ether_addr *wl_ecos_comm_get_event_addr(void *e);

extern void wl_ecos_comm_cntincr_cnt_txnobuf(void *pub);
extern int wl_ecos_comm_get_maxmultilist(void);
#ifdef TENDA_WLAN_DBG
/* add by jack,2015-10-9,统计dev的发包个数*/
extern void wl_ecos_comm_update_if_opackets(void *wlc,uint32 packets);
/* end by jack */
#endif
#endif	/* _wl_ecos_comm_h_ */
