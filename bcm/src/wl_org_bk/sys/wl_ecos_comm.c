/*
 * eCos-specific portion of Broadcom 802.11
 * Networking Adapter Device Driver.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wl_ecos_comm.c,v 1.10.94.1 2010-12-21 02:38:57 Exp $
 */
#include <stdio.h>
#include <typedefs.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>


#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#include <net/ethernet.h>
#define __INCif_etherh
#include <wlioctl.h>
#undef __INCif_etherh

#include <wlc_cfg.h>
#include <wlc_pub.h>
#include <wl_dbg.h>
#include <wl_export.h>

#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>

bool
wl_ecos_comm_wlc_chipmatch(uint16 vendor, uint16 device)
{
	return (wlc_chipmatch(vendor, device));
}

bool
wl_ecos_comm_wlc_sendpkt(void *wlc, void *sdu, void *wlcif)
{
	return (wlc_sendpkt(wlc, sdu, wlcif));
}

bool
wl_ecos_comm_wlc_intrsupd(void *wlc)
{
	return (wlc_intrsupd(wlc));
}

bool
wl_ecos_comm_wlc_dpc(void *wlc, bool bounded)
{
	return (wlc_dpc(wlc, bounded));

}

void*
wl_ecos_comm_wlc_attach(void *wl, uint16 vendor, uint16 device, uint unit, bool piomode,
                        osl_t *osh, void *regsva, uint bustype, void *btparam, uint *perr)
{
	return (wlc_attach(wl, vendor, device, unit, piomode, osh, regsva, bustype, btparam, perr));
}

void
wl_ecos_comm_wlc_detach(void *wlc)
{
	wlc_detach(wlc);
}

void
wl_ecos_comm_wlc_reset(void *wlc)
{
	wlc_reset(wlc);
}

void
wl_ecos_comm_wlc_init(void *wlc)
{
	wlc_init(wlc);
}

void
wl_ecos_comm_wlc_intrson(void *wlc)
{
	wlc_intrson(wlc);
}

uint32
wl_ecos_comm_wlc_intrsoff(void *wlc)
{
	return (wlc_intrsoff(wlc));
}

void
wl_ecos_comm_wlc_intrsrestore(void *wlc, uint32 macintmask)
{
	wlc_intrsrestore(wlc, macintmask);
}

int
wl_ecos_comm_wlc_up(void *wlc)
{
	return (wlc_up(wlc));
}

uint
wl_ecos_comm_wlc_down(void *wlc)
{
	return (wlc_down(wlc));
}

int
wl_ecos_comm_wlc_set(void *wlc, int cmd, int arg)
{
	return (wlc_set(wlc, cmd, arg));
}

int
wl_ecos_comm_wlc_ioctl(void *wlc, int cmd, void *arg, int len, void *wlcif)
{
	return (wlc_ioctl(wlc, cmd, arg, len, wlcif));
}

bool
wl_ecos_comm_wlc_isr(void *wlc, bool *wantdpc)
{
	return (wlc_isr(wlc, wantdpc));
}

/* intrs off called by isr, faster, no interrupt protection needed */
void
wlc_intrsoff_isr(wlc_info_t *wlc)
{
	if (!wlc->clk)
		return;

	W_REG(wlc->pub.osh, &wlc->regs->macintmask, 0);

	(void)R_REG(wlc->pub.osh, &wlc->regs->macintmask);	/* sync readback */
	wlc->macintmask = 0;
}

void
wl_ecos_comm_wlc_intrsoff_isr(void *wlc)
{
	wlc_intrsoff_isr(wlc);
}

bool
wl_ecos_comm_wlc_sup_mic_error(void *wlc, bool group)
{
#if defined(STA) && defined(BCMSUP_PSK)
	return (wlc_sup_mic_error(wlc, group));
#endif
}

void *
wl_ecos_comm_pub(void *wlc)
{
	return (void *)wlc_pub(wlc);
}

int
wl_ecos_comm_iovar_op(void *wlc, const char *name,
        void *arg, int len, bool set, void *wlcif)
{
	return wlc_iovar_op(wlc, name, NULL, 0, arg, len, set, wlcif);
}

/* ==================================== */
/* ==================================== */
void *
wl_ecos_comm_osh(void *pub)
{
	return (((wlc_pub_t *)pub)->osh);
}

bool
wl_ecos_comm_up(void *pub)
{
	return (((wlc_pub_t *)pub)->up);
}

uint
wl_ecos_comm_get_unit(void *pub)
{
	return (((wlc_pub_t *)pub)->unit);
}

void
wl_ecos_comm_set_mac(void *pub, char *addr)
{
	bcopy(addr, (char*)&(((wlc_pub_t *)pub)->cur_etheraddr), ETHER_ADDR_LEN);
}

void
wl_ecos_comm_get_mac(void *pub, char *addr)
{
	bcopy((char*)&(((wlc_pub_t *)pub)->cur_etheraddr), addr, ETHER_ADDR_LEN);
}

void
wl_ecos_comm_set_allmulti(void *wlc, bool value)
{
	wlc_iovar_setint(wlc, "allmulti", value);
}

void wl_ecos_comm_set_multicastlist(void *wlc, struct maclist *maclist, int buflen)
{
	wlc_iovar_op(wlc, "mcast_list", NULL, 0, maclist, buflen, IOV_SET, NULL);
}

bool
wl_ecos_comm_get_ap(void *pub)
{
	return (((wlc_pub_t *)pub)->_ap);
}

bool
wl_ecos_comm_get_associated(void *pub)
{
	return (((wlc_pub_t *)pub)->associated);
}

uint32
wl_ecos_comm_get_event_type(void *e)
{
	return (((wlc_event_t *)e)->event.event_type);
}

uint8
wl_ecos_comm_get_event_flags(void *e)
{
	return (((wlc_event_t *)e)->event.flags);
}

struct ether_addr*
wl_ecos_comm_get_event_addr(void *e)
{
	return (((wlc_event_t *)e)->addr);
}

void
wl_ecos_comm_cntincr_cnt_txnobuf(void *pub)
{
	WLCNTINCR(((wlc_pub_t *)pub)->_cnt->txnobuf);
}

int
wl_ecos_comm_get_maxmultilist(void)
{
	return (MAXMULTILIST);
}
