/*
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wl_rte.c,v 1.360.2.3 2010-10-26 18:31:52 Exp $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <epivers.h>
#include <bcmendian.h>
#include <proto/ethernet.h>
#include <bcmdevs.h>
#include <wlioctl.h>

#include <proto/802.11.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_lmac.h>
#include <bcmsrom_fmt.h>
#include <bcmsrom.h>
#ifdef MSGTRACE
#include <msgtrace.h>
#endif

#ifdef WLLMACPROTO
#include <wlc_lmac_proto.h>
#endif /* WLLMACPROTO */

#include <wl_export.h>

#include <wl_oid.h>
#include <wlc_led.h>

#ifdef WLPFN
#include <wl_pfn.h>
#endif 	/* WLPFN */
#include <wl_toe.h>
#include <wl_arpoe.h>
#include <wl_keep_alive.h>
#include <wlc_pkt_filter.h>

#ifdef CONFIG_WLU
#include "../exe/wlu_cmd.h"
#endif	/* CONFIG_WLU */

#ifdef WLC_LOW_ONLY
#include <bcm_xdr.h>
#include <bcm_rpc_tp.h>
#include <bcm_rpc.h>
#include <wlc_rpc.h>

#include <wlc_channel.h>
#endif

#ifdef BCMDBG
#include <bcmutils.h>
#endif

#include <dngl_bus.h>
#include <dngl_wlhdr.h>

#define WL_IFTYPE_BSS	1
#define WL_IFTYPE_WDS	2

#ifdef BCMASSERT_LOG
#include <bcm_assert_log.h>
#endif

typedef struct hndrte_timer wl_timer;

struct wl_if {
	struct wlc_if *wlcif;
	hndrte_dev_t *dev;		/* virtual device */
};

typedef struct wl_info {
	uint		unit;		/* device instance number */
	wlc_pub_t	*pub;		/* pointer to public wlc state */
	void		*wlc;		/* pointer to private common os-independent data */
	wlc_hw_info_t	*wlc_hw;
	hndrte_dev_t	*dev;		/* primary device */
	bool		link;		/* link state */
	uint8		hwfflags;	/* host wake up filter flags */
	uint8		active_overlay;	/* ioctl overlay region in use */
	hndrte_stats_t	stats;
	wl_oid_t	*oid;		/* oid handler state */
	hndrte_timer_t  dpcTimer;	/* 0 delay timer used to schedule dpc */
#ifdef WLPFN
	wl_pfn_info_t	*pfn;		/* pointer to prefered network data */
#endif /* WLPFN */
	wl_toe_info_t	*toei;		/* pointer to toe specific information */
	wl_arp_info_t	*arpi;		/* pointer to arp agent offload info */
	wl_keep_alive_info_t	*keep_alive_info;	/* pointer to keep-alive offload info */
	wlc_pkt_filter_info_t	*pkt_filter_info;	/* pointer to packet filter info */
#ifdef WLC_LOW_ONLY
	rpc_info_t 	*rpc;		/* RPC handle */
	rpc_tp_info_t	*rpc_th;	/* RPC transport handle */
	wlc_rpc_ctx_t	rpc_dispatch_ctx;
	bool dpc_stopped;	/* stop wlc_dpc() flag */
	bool dpc_requested;	/* request to wlc_dpc() */
#if defined(HNDRTE_PT_GIANT) && defined(DMA_TX_FREE)
	hndrte_lowmem_free_t lowmem_free_info;
#endif
#endif /* WLC_LOW_ONLY */
} wl_info_t;


#define WL_IF(wl, dev)	(((hndrte_dev_t *)(dev) == ((wl_info_t *)(wl))->dev) ? \
			 NULL : \
			 *(wl_if_t **)((hndrte_dev_t *)(dev) + 1))

#ifdef WLC_LOW_ONLY

/* Minimal memory requirement to do wlc_dpc. This is critical for BMAC as it cannot
 * lose frames
 * This value is based on perceived DPC need. Note that it accounts for possible
 * fragmentation  where sufficient memory does not mean getting contiguous allocation
 */

#define MIN_DPC_MEM	((RXBND + 6)* 2048)

#endif /* WLC_LOW_ONLY */

/* host wakeup filter flags */
#define HWFFLAG_UCAST	1		/* unicast */
#define HWFFLAG_BCAST	2		/* broadcast */

/* iovar table */
enum {
	IOV_HWFILTER,		/* host wakeup filter */
	IOV_DEEPSLEEP,		/* Deep sleep mode */
#if defined(DONGLEOVERLAYS) && defined(DONGLEOVERLAYTEST)
	IOV_OVERLAYTEST,	/* test overlays */
	IOV_OVERLAYTEST2,	/* test overlays */
#endif
	IOV_LAST 		/* In case of a need to check max ID number */
};

static const bcm_iovar_t wl_iovars[] = {
	{"hwfilter", IOV_HWFILTER, 0, IOVT_BUFFER, 0},
	{"deepsleep", IOV_DEEPSLEEP, 0, IOVT_BOOL, 0},
#if defined(DONGLEOVERLAYS) && defined(DONGLEOVERLAYTEST)
	{"overlaytest", IOV_OVERLAYTEST, 0, IOVT_UINT32, 0},
	{"overlaytest2", IOV_OVERLAYTEST2, 0, IOVT_UINT32, 0},
#endif
	{NULL, 0, 0, 0, 0}
};

#ifdef CONFIG_WLU
/* forward prototype */
static void do_wl_cmd(uint32 arg, uint argc, char *argv[]);
#endif /* CONFIG_WLU */
#ifdef BCMDBG
static void do_wlmsg_cmd(uint32 arg, uint argc, char *argv[]);
#endif
#ifdef WLC_HIGH
static void wl_statsupd(wl_info_t *wl);
#endif
static void wl_timer_main(hndrte_timer_t *t);
#ifdef WLC_HIGH
static int wl_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                      void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
#endif

/* Driver entry points */
void *wl_probe(hndrte_dev_t *dev, void *regs, uint bus,
	uint16 devid, uint coreid, uint unit);
static void wl_free(wl_info_t *wl, osl_t *osh);
static void wl_isr(hndrte_dev_t *dev);
static void _wl_dpc(hndrte_timer_t *timer);
static void wl_dpc(wl_info_t *wl);
static int wl_open(hndrte_dev_t *dev);
static int wl_send(hndrte_dev_t *src, hndrte_dev_t *dev, struct lbuf *lb);
static int wl_close(hndrte_dev_t *dev);

#ifndef WLC_LOW_ONLY
static int wl_ioctl(hndrte_dev_t *dev, uint32 cmd, void *buf, int len, int *used, int *needed,
	int set);
static bool _wl_rte_oid_check(wl_info_t *wl, uint32 cmd, void *buf, int len, int *used,
	int *needed, bool set, int *status);
#else
static void wl_rpc_down(void *wlh);
static void wl_rpc_resync(void *wlh);

static void wl_rpc_tp_txflowctl(hndrte_dev_t *dev, bool state, int prio);
static void wl_rpc_txflowctl(void *wlh, bool on);

#if defined(HNDRTE_PT_GIANT) && defined(DMA_TX_FREE)
static void wl_lowmem_free(void *wlh);
#endif
static void wl_rpc_bmac_dispatch(void *ctx, struct rpc_buf* buf);

static void do_wlhist_cmd(uint32 arg, uint argc, char *argv[]);
static void do_wldpcdump_cmd(uint32 arg, uint argc, char *argv[]);
#endif /* WLC_LOW_ONLY */

#ifdef DONGLEOVERLAYS
static int wl_overlay_copy(wl_info_t *wl, uint8 *buf, int len);
#ifdef DONGLEOVERLAYTEST
int wl_overlaytest_iovar(wl_info_t *wl, uint32 *val);
int wl_overlaytest2_iovar(wl_info_t *wl, uint32 *val);
#endif
#endif /* DONGLEOVERLAYS */

static hndrte_devfuncs_t wl_funcs = {
#ifndef BCMROMBUILD
	probe:		wl_probe,
#endif
	open:		wl_open,
	close:		wl_close,
	xmit:		wl_send,
#ifdef WLC_LOW_ONLY
	txflowcontrol:	wl_rpc_tp_txflowctl,
#else
	ioctl:		wl_ioctl,
#endif /* WLC_LOW_ONLY */
	poll:		wl_isr
};

hndrte_dev_t bcmwl = {
	name:		"wl",
	funcs:		&wl_funcs
};

#ifdef WLC_LOW_ONLY
#ifdef BCMASSERT_LOG
	void *g_assert_hdl = NULL;
#endif
#endif /* WLC_LOW_ONLY */

#ifdef WLC_HIGH
static int
wl_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
           void *p, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{

	wl_info_t *wl = (wl_info_t *)hdl;
	wlc_info_t *wlc = wl->wlc;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	int err = 0;
	int radio;

	/* LMAC doesn't have support for iovars */
	if (LMAC_ENAB(wl->pub))
		return 0;

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));
	bool_val = (int_val != 0) ? TRUE : FALSE;

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	switch (actionid) {
	case IOV_GVAL(IOV_HWFILTER):
		*ret_int_ptr = wl->hwfflags;
		break;

	case IOV_SVAL(IOV_HWFILTER):
		wl->hwfflags = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_DEEPSLEEP):
		if ((err = wlc_get(wlc, WLC_GET_RADIO, &radio)))
			break;
		*ret_int_ptr = (radio & WL_RADIO_SW_DISABLE) ? TRUE : FALSE;
		break;

	case IOV_SVAL(IOV_DEEPSLEEP):
		wlc_set(wlc, WLC_SET_RADIO, (WL_RADIO_SW_DISABLE << 16)
		        | (bool_val ? WL_RADIO_SW_DISABLE : 0));
		/* suspend or resume timers */
		if (bool_val)
			hndrte_suspend_timer();
		else
			hndrte_resume_timer();
		break;

#if defined(DONGLEOVERLAYS) && defined(DONGLEOVERLAYTEST)
	case IOV_GVAL(IOV_OVERLAYTEST):
	{
		uint32 val;
		if ((err = wl_overlaytest_iovar(wl, &val)))
			break;
		*ret_int_ptr = val;
		break;
	}

	case IOV_GVAL(IOV_OVERLAYTEST2):
	{
		uint32 val;
		if ((err = wl_overlaytest2_iovar(wl, &val)))
			break;
		*ret_int_ptr = val;
		break;
	}
#endif /* DONGLEOVERLAYTEST */

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

static void
BCMINITFN(_wl_init)(wl_info_t *wl)
{
	wl_reset(wl);

	wlc_init(wl->wlc);
}

void
wl_init(wl_info_t *wl)
{
	WL_TRACE(("wl%d: wl_init\n", wl->unit));

		_wl_init(wl);
}
#endif /* WLC_HIGH */

uint
BCMINITFN(wl_reset)(wl_info_t *wl)
{
	WL_TRACE(("wl%d: wl_reset\n", wl->unit));

	wlc_reset(wl->wlc);

	return 0;
}

bool
wl_alloc_dma_resources(wl_info_t *wl, uint addrwidth)
{
	return TRUE;
}

/*
 * These are interrupt on/off enter points.
 * Since wl_isr is serialized with other drive rentries using spinlock,
 * They are SMP safe, just call common routine directly,
 */
void
wl_intrson(wl_info_t *wl)
{
	wlc_intrson(wl->wlc);
}

uint32
wl_intrsoff(wl_info_t *wl)
{
	return wlc_intrsoff(wl->wlc);
}

void
wl_intrsrestore(wl_info_t *wl, uint32 macintmask)
{
	wlc_intrsrestore(wl->wlc, macintmask);
}

#ifdef WLC_HIGH
/* BMAC driver has alternative up/down etc. */
int
wl_up(wl_info_t *wl)
{
	int ret;
	wlc_info_t *wlc = (wlc_info_t *) wl->wlc;

	WL_TRACE(("wl%d: wl_up\n", wl->unit));

	if (wl->pub->up)
		return 0;


	if (wl->pub->up)
		return 0;

	/* Reset the hw to known state */
#ifdef WLLMAC
	if (LMAC_ENAB(wlc->pub))
		ret = wlc_lmac_up(wlc->lmac_info);
	else
#endif
		ret = wlc_up(wlc);

	if (ret == 0)
		ret = wl_keep_alive_up(wl->keep_alive_info);

#ifndef RSOCK
	if (wl_oid_reclaim(wl->oid))
		hndrte_reclaim();

	if (POOL_ENAB(wl->pub))
		pktpool_fill(hndrte_osh, wl->pub->pktpool, FALSE);
#endif

	return ret;
}

void
wl_down(wl_info_t *wl)
{
	WL_TRACE(("wl%d: wl_down\n", wl->unit));
	if (!wl->pub->up)
		return;

	wl_keep_alive_down(wl->keep_alive_info);
	wlc_down(wl->wlc);
	wl->pub->hw_up = FALSE;
}

void
wl_dump_ver(wl_info_t *wl, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "wl%d: %s %s version %s\n", wl->unit,
		__DATE__, __TIME__, EPI_VERSION_STR);
}

#if defined(BCMDBG) || defined(WLDUMP)
static int
wl_dump(wl_info_t *wl, struct bcmstrbuf *b)
{
	wl_dump_ver(wl, b);

	return 0;
}
#endif /* BCMDBG || WLDUMP */
#endif /* WLC_HIGH */

void
wl_monitor(wl_info_t *wl, wl_rxsts_t *rxsts, void *p)
{
#ifdef WL_MONITOR
	uint len;
	struct lbuf *mon_pkt;

	len = PKTLEN(wl->pub->osh, p) - D11_PHY_HDR_LEN + sizeof(rx_ctxt_t);

	if ((mon_pkt = PKTGET(wl->pub->osh, len, FALSE)) == NULL)
		return;

	PKTSETLEN(wl->pub->osh, mon_pkt, len - sizeof(rx_ctxt_t));

	bcopy(PKTDATA(wl->pub->osh, p) + D11_PHY_HDR_LEN,
		PKTDATA(wl->pub->osh, mon_pkt),
		len);
	wl_sendup(wl, NULL, mon_pkt, 1);
#endif /* WL_MONITOR */
}

void
wl_set_monitor(wl_info_t *wl, int val)
{
}

char *
wl_ifname(wl_info_t *wl, struct wl_if *wlif)
{
	if (wlif == NULL)
		return wl->dev->name;
	else
		return wlif->dev->name;
}

#if defined(AP) && defined(WLC_HIGH)

/* Allocate wl_if_t, hndrte_dev_t, and wl_if_t * all together */
static wl_if_t *
wl_alloc_if(wl_info_t *wl, int iftype, uint subunit, struct wlc_if *wlcif)
{
	hndrte_dev_t *dev;
	wl_if_t *wlif;
	osl_t *osh = wl->pub->osh;
	hndrte_dev_t *bus = wl->dev->chained;
	uint len;
	int ifindex;
	wl_if_t **priv;

	/* the primary device must be binded to the bus */
	if (bus == NULL) {
		WL_ERROR(("wl%d: wl_alloc_if: device not binded\n", wl->pub->unit));
		return NULL;
	}

	/* allocate wlif struct + dev struct + priv pointer */
	len = sizeof(wl_if_t) + sizeof(hndrte_dev_t) + sizeof(wl_if_t **);
	if ((wlif = MALLOC(osh, len)) == NULL) {
		WL_ERROR(("wl%d: wl_alloc_if: out of memory, malloced %d bytes\n",
		          (wl->pub)?wl->pub->unit:subunit, MALLOCED(wl->pub->osh)));
		goto err;
	}
	bzero(wlif, len);

	dev = (hndrte_dev_t *)(wlif + 1);
	priv = (wl_if_t **)(dev + 1);

	wlif->dev = dev;
	wlif->wlcif = wlcif;

	dev->funcs = &wl_funcs;
	dev->softc = wl;
	snprintf(dev->name, HNDRTE_DEV_NAME_MAX, "wl%d.%d", wl->pub->unit, subunit);

	*priv = wlif;

	/* use the return value as the i/f no. in the event to the host */
#ifdef DONGLEBUILD
	if ((ifindex = bus_ops->binddev(bus, dev)) < 1) {
		WL_ERROR(("wl%d: wl_alloc_if: bus_binddev failed\n", wl->pub->unit));
		goto err;
	}
#else
	ifindex = subunit;
#endif
	wlcif->index = (uint8)ifindex;

	return wlif;

err:
	if (wlif != NULL)
		MFREE(osh, wlif, len);
	return NULL;
}

static void
wl_free_if(wl_info_t *wl, wl_if_t *wlif)
{
	MFREE(wl->pub->osh, wlif, sizeof(wl_if_t) + sizeof(hndrte_dev_t) + sizeof(wl_if_t *));
}

struct wl_if *
wl_add_if(wl_info_t *wl, struct wlc_if *wlcif, uint unit, struct ether_addr *remote)
{
	wl_if_t *wlif;

	wlif = wl_alloc_if(wl, remote != NULL ? WL_IFTYPE_WDS : WL_IFTYPE_BSS, unit, wlcif);

	if (wlif == NULL) {
		WL_ERROR(("wl%d: wl_add_if: failed to create %s interface %d\n", wl->pub->unit,
			(remote)?"WDS":"BSS", unit));
		return NULL;
	}

	return wlif;
}

void
wl_del_if(wl_info_t *wl, struct wl_if *wlif)
{
#ifdef DONGLEBUILD
	hndrte_dev_t *bus = wl->dev->chained;

	if (bus_ops->unbinddev(bus, wlif->dev) < 1)
		WL_ERROR(("wl%d: wl_del_if: bus_unbinddev failed\n", wl->pub->unit));
#endif
	WL_TRACE(("wl%d: wl_del_if: bus_unbinddev idx %d\n", wl->pub->unit, wlif->wlcif->index));
	wl_free_if(wl, wlif);
}
#endif /* AP */

static void
wl_timer_main(hndrte_timer_t *t)
{
	ASSERT(t->context); ASSERT(t->auxfn);

	t->auxfn(t->data);
}

#undef wl_init_timer

struct wl_timer *
wl_init_timer(wl_info_t *wl, void (*fn)(void* arg), void *arg, const char *name)
{
	return (struct wl_timer *)hndrte_init_timer(wl, arg, wl_timer_main, fn);
}

void
wl_free_timer(wl_info_t *wl, struct wl_timer *t)
{
	hndrte_free_timer((hndrte_timer_t *)t);
}

void
wl_add_timer(wl_info_t *wl, struct wl_timer *t, uint ms, int periodic)
{
	ASSERT(t != NULL);
	hndrte_add_timer((hndrte_timer_t *)t, ms, periodic);
}

bool
wl_del_timer(wl_info_t *wl, struct wl_timer *t)
{
	if (t == NULL)
		return TRUE;
	return hndrte_del_timer((hndrte_timer_t *)t);
}

static const char BCMATTACHDATA(rstr_fmt_hello)[] =
	"wl%d: Broadcom BCM%04x 802.11 Wireless Controller %s\n";

/* wl_probe is always a RAM function and selects LMAC vs. full dongle (so does wlc_alloc.c).
 * It should be static but is not because BCMROMBUILD builds say 'defined but not used'.
 */
#if defined(WLLMAC) && defined(WLLMAC_ENABLED)
#define WLC_ATTACH	wlc_lmac_attach
#else
#define WLC_ATTACH	wlc_attach
#endif

void *
BCMATTACHFN(wl_probe)(hndrte_dev_t *dev, void *regs, uint bus, uint16 devid,
                      uint coreid, uint unit)
{
	wl_info_t *wl;
	wlc_info_t *wlc;
	osl_t *osh;
	uint err;

	/* allocate private info */
	if (!(wl = (wl_info_t *)MALLOC(NULL, sizeof(wl_info_t)))) {
		WL_ERROR(("wl%d: MALLOC failed\n", unit));
		return NULL;
	}
	bzero(wl, sizeof(wl_info_t));

	wl->unit = unit;

	osh = osl_attach(dev);

#ifdef WLC_LOW_ONLY
	wl->rpc_th = bcm_rpc_tp_attach(osh, dev);
	if (wl->rpc_th == NULL) {
		WL_ERROR(("wl%d: bcm_rpc_tp_attach failed\n", unit));
		goto fail;
	}
	bcm_rpc_tp_txflowctlcb_init(wl->rpc_th, wl, wl_rpc_txflowctl);

	wl->rpc = bcm_rpc_attach(NULL, NULL, wl->rpc_th, NULL);
	if (wl->rpc == NULL) {
		WL_ERROR(("wl%d: bcm_rpc_attach failed\n", unit));
		goto fail;
	}
#ifdef BCMASSERT_LOG
	g_assert_hdl = (void*)wl->rpc;
#endif
#endif /* WLC_LOW_ONLY */

	/* common load-time initialization */
	if (!(wlc = WLC_ATTACH(wl,			/* wl */
	                       VENDOR_BROADCOM,		/* vendor */
	                       devid,			/* device */
	                       unit,			/* unit */
	                       FALSE,			/* piomode */
	                       osh,			/* osh */
	                       regs,			/* regsva */
	                       bus,			/* bustype */
#ifdef WLC_LOW_ONLY
	                       wl->rpc,			/* BMAC, overloading, to change */
#else
			       wl,			/* sdh */
#endif
	                       &err))) {		/* perr */
		WL_ERROR(("wl%d: wlc_attach failed with error %d\n", unit, err));
		goto fail;
	}
	wl->wlc = (void *)wlc;
	wl->pub = wlc_pub((void *)wlc);
	wl->wlc_hw = wlc->hw;
	wl->dev = dev;
	wl->dpcTimer.mainfn = _wl_dpc;
	wl->dpcTimer.data = wl;
#ifdef DONGLEOVERLAYS
	wl->active_overlay = 0xff;
#endif

	snprintf(dev->name, HNDRTE_DEV_NAME_MAX, "wl%d", unit);

	/* print hello string */
	printf(rstr_fmt_hello, unit, wlc->hw->sih->chip, EPI_VERSION_STR);

#ifndef HNDRTE_POLLING
	if (hndrte_add_isr(0, coreid, unit, (isr_fun_t)wl_isr, dev, bus)) {
		WL_ERROR(("wl%d: hndrte_add_isr failed\n", unit));
		goto fail;
	}
#endif	/* HNDRTE_POLLING */

#ifdef WLC_LOW_ONLY
	wl->rpc_dispatch_ctx.rpc = wl->rpc;
	wl->rpc_dispatch_ctx.wlc = wlc;
	wl->rpc_dispatch_ctx.wlc_hw = wlc->hw;
	bcm_rpc_rxcb_init(wl->rpc, &wl->rpc_dispatch_ctx, wl_rpc_bmac_dispatch, wl,
	                  wl_rpc_down, wl_rpc_resync, NULL);

	hndrte_cons_addcmd("wlhist", do_wlhist_cmd, (uint32)wl);
	hndrte_cons_addcmd("dpcdump", do_wldpcdump_cmd, (uint32)wl);

#if defined(HNDRTE_PT_GIANT) && defined(DMA_TX_FREE)
	wl->lowmem_free_info.free_fn = wl_lowmem_free;
	wl->lowmem_free_info.free_arg = wl;

	hndrte_pt_lowmem_register(&wl->lowmem_free_info);
#endif /* HNDRTE_PT_GIANT && DMA_TX_FREE */

#else /* WLC_LOW_ONLY */

#ifdef STA
	/* algin watchdog with tbtt indication handling in PS mode */
	wl->pub->align_wd_tbtt = TRUE;
#endif

	wlc_eventq_set_ind(wlc->eventq, WLC_E_IF, TRUE);

	/* initialize OID handler state */
	if ((wl->oid = wl_oid_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_oid_attach failed\n", unit));
		goto fail;
	}

#ifdef WLPFN
	/* initialize PFN handler state */
	if ((wl->pfn = wl_pfn_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_pfn_attach failed\n", unit));
		goto fail;
	}
#endif /* WLPFN */

	/* allocate the toe info struct */
	if ((wl->toei = wl_toe_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_toe_attach failed\n", unit));
		goto fail;
	}

	/* allocate the arp info struct */
	if ((wl->arpi = wl_arp_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_arp_attach failed\n", unit));
		goto fail;
	}

	/* allocate the keep-alive info struct */
	if ((wl->keep_alive_info = wl_keep_alive_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_keep_alive_attach failed\n", unit));
		goto fail;
	}

	/* allocate the packet filter info struct */
	if ((wl->pkt_filter_info = wlc_pkt_filter_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wlc_pkt_filter_attach failed\n", unit));
		goto fail;
	}
#endif /* WLC_HIGH */

#ifdef	CONFIG_WLU
	hndrte_cons_addcmd("wl", do_wl_cmd, (uint32)wl);
#endif /* CONFIG_WLU */

#ifdef BCMDBG
	hndrte_cons_addcmd("wlmsg", do_wlmsg_cmd, (uint32)wl);
#endif
#ifdef WLC_HIGH

	/* register module */
	if (wlc_module_register(wlc->pub, wl_iovars, "wl", wl, wl_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: wlc_module_register() failed\n", unit));
		goto fail;
	}

#if defined(BCMDBG) || defined(WLDUMP)
	wlc_dump_register(wl->pub, "wl", (dump_fn_t)wl_dump, (void *)wl);
#endif

#endif /* WLC_HIGH */

#ifdef MSGTRACE
	msgtrace_init(wlc, wl->dev, (msgtrace_func_send_t)wlc_event_sendup_trace);
#endif


	return (wl);

fail:
	wl_free(wl, osh);
	return NULL;
}

static void
BCMATTACHFN(wl_free)(wl_info_t *wl, osl_t *osh)
{

#ifdef WLC_HIGH
	if (wl->pkt_filter_info)
		wlc_pkt_filter_detach(wl->pkt_filter_info);
	if (wl->keep_alive_info)
		wl_keep_alive_detach(wl->keep_alive_info);
	if (wl->arpi)
		wl_arp_detach(wl->arpi);
	if (wl->toei)
		wl_toe_detach(wl->toei);
#ifdef WLPFN
	if (wl->pfn)
		wl_pfn_detach(wl->pfn);
#endif
	if (wl->oid)
		wl_oid_detach(wl->oid);
#endif /* WLC_HIGH */

#if defined(HNDRTE_PT_GIANT) && defined(DMA_TX_FREE)
	hndrte_pt_lowmem_unregister(&wl->lowmem_free_info);
#endif

	/* common code detach */
	if (wl->wlc)
		wlc_detach(wl->wlc);

#ifdef WLC_LOW_ONLY
	/* rpc, rpc_transport detach */
	if (wl->rpc)
		bcm_rpc_detach(wl->rpc);
	if (wl->rpc_th)
		bcm_rpc_tp_detach(wl->rpc_th);
#ifdef BCMASSERT_LOG
	g_assert_hdl = NULL;
#endif
#endif /* WLC_LOG_ONLY */

	MFREE(osh, wl, sizeof(wl_info_t));
}

static void
wl_isr(hndrte_dev_t *dev)
{
	wl_info_t *wl = dev->softc;
	bool dpc;

	WL_TRACE(("wl%d: wl_isr\n", wl->unit));

	/* call common first level interrupt handler */
	if (wlc_isr(wl->wlc, &dpc)) {
		/* if more to do... */
		if (dpc) {
			wl_dpc(wl);
		}
	}
}

static void
wl_dpc(wl_info_t *wl)
{
	bool resched = 0;
	bool bounded = TRUE;

	/* call the common second level interrupt handler if we have enough memory */
	if (wl->wlc_hw->up) {
#ifdef WLC_LOW_ONLY
		if (!wl->dpc_stopped) {
			if (wl->wlc_hw->rpc_dngl_agg & BCM_RPC_TP_DNGL_AGG_DPC) {
				bcm_rpc_tp_agg_set(wl->rpc_th, BCM_RPC_TP_DNGL_AGG_DPC, TRUE);
			}

			resched = wlc_dpc(wl->wlc, bounded);

			if (wl->wlc_hw->rpc_dngl_agg & BCM_RPC_TP_DNGL_AGG_DPC) {
				bcm_rpc_tp_agg_set(wl->rpc_th, BCM_RPC_TP_DNGL_AGG_DPC, FALSE);
			}
		} else {
			WL_TRACE(("dpc_stop is set!\n"));
			wl->dpc_requested = TRUE;
			return;
		}
#else
		resched = wlc_dpc(wl->wlc, bounded);
#endif /* WLC_LOW_ONLY */
	}

	/* wlc_dpc() may bring the driver down */
	if (!wl->wlc_hw->up)
		return;

	/* re-schedule dpc or re-enable interrupts */
	if (resched) {
		if (!hndrte_add_timer(&wl->dpcTimer, 0, FALSE))
			ASSERT(FALSE);
	} else
		wlc_intrson(wl->wlc);
}

static void
_wl_dpc(hndrte_timer_t *timer)
{
	wl_info_t *wl = (wl_info_t *) timer->data;

	if (wl->wlc_hw->up) {
		wlc_intrsupd(wl->wlc);
		wl_dpc(wl);
	}
}

static int
wl_open(hndrte_dev_t *dev)
{
	wl_info_t *wl = dev->softc;
	int ret;

	WL_TRACE(("wl%d: wl_open\n", wl->unit));

	if ((ret = wlc_ioctl(wl->wlc, WLC_UP, NULL, 0, NULL)))
		return ret;

#ifdef HNDRTE_JOIN_SSID
	/*
	 * Feature useful for repetitious testing: if Make defines HNDRTE_JOIN_SSID
	 * to an SSID string, automatically join that SSID at driver startup.
	 */
	{
		wlc_info_t *wlc = wl->wlc;
		int infra = 1;
		int auth = 0;
		char *ss = HNDRTE_JOIN_SSID;
		wlc_ssid_t ssid;

		printf("Joining %s:\n", ss);
		/* set infrastructure mode */
		printf("  Set Infra\n");
		wlc_ioctl(wlc, WLC_SET_INFRA, &infra, sizeof(int), NULL);
		printf("  Set Auth\n");
		wlc_ioctl(wlc, WLC_SET_AUTH, &auth, sizeof(int), NULL);
		printf("  Set SSID %s\n", ss);
		ssid.SSID_len = strlen(ss);
		bcopy(ss, ssid.SSID, ssid.SSID_len);
		wlc_ioctl(wlc, WLC_SET_SSID, &ssid, sizeof(wlc_ssid_t), NULL);
	}
#endif /* HNDRTE_JOIN_SSID */

	return (ret);
}

#ifdef WLC_HIGH
static bool
wl_hwfilter(wl_info_t *wl, void *p)
{
	struct ether_header *eh = (struct ether_header *)PKTDATA(wl->pub->osh, p);

	if (((wl->hwfflags & HWFFLAG_UCAST) && !ETHER_ISMULTI(eh->ether_dhost)) ||
	    ((wl->hwfflags & HWFFLAG_BCAST) && ETHER_ISBCAST(eh->ether_dhost)))
		return TRUE;

	return FALSE;
}


static void *
wl_pkt_header_push(wl_info_t *wl, void *p, uint8 *wl_hdr_words)
{
	wl_header_t *h;
	osl_t *osh = wl->pub->osh;
	wlc_pkttag_t *pkttag = WLPKTTAG(p);
	int8 rssi = pkttag->rssi;

#ifdef WLLMACPROTO
	/* No wl header for LMAC */
	if (LMACPROTO_ENAB(wl->pub)) {
		*wl_hdr_words = 0;
		return p;
	}
#endif

	if (PKTHEADROOM(osh, p) < WL_HEADER_LEN) {
		void *p1;
		int plen = PKTLEN(osh, p);

		/* Alloc a packet that will fit all the data; chaining the header won't work */
		if ((p1 = PKTGET(osh, plen + WL_HEADER_LEN, TRUE)) == NULL) {
			WL_ERROR(("PKTGET pkt size %d failed\n", plen));
			PKTFREE(osh, p, TRUE);
			return NULL;
		}

		/* Transfer other fields */
		PKTSETPRIO(p1, PKTPRIO(p));
		PKTSETSUMGOOD(p1, PKTSUMGOOD(p));

		bcopy(PKTDATA(osh, p), PKTDATA(osh, p1) + WL_HEADER_LEN, plen);
		PKTFREE(osh, p, TRUE);

		p = p1;
	} else
		PKTPUSH(osh, p, WL_HEADER_LEN);

	h = (wl_header_t *)PKTDATA(osh, p);
	h->type = WL_HEADER_TYPE;
	h->version = WL_HEADER_VER;
	h->rssi = rssi;
	h->pad = 0;
	/* Return header length in words */
	*wl_hdr_words = WL_HEADER_LEN/4;

	return p;
}

static void
wl_pkt_header_pull(wl_info_t *wl, void *p)
{
	/* Currently this is a placeholder function. We don't process wl header
	   on Tx side as no meaningful fields defined for tx currently.
	 */
	PKTPULL(wl->pub->osh, p, PKTDATAOFFSET(p));
	return;
}

/*
 * The last parameter was added for the build. Caller of
 * this function should pass 1 for now.
 */
void
wl_sendup(wl_info_t *wl, struct wl_if *wlif, void *p, int numpkt)
{
	struct lbuf *lb;
	hndrte_dev_t *dev;
	hndrte_dev_t *chained;
	int ret_val;
	uint8 *buf;
	uint16 type;
	uint8 wl_hdr_words = 0;

	WL_TRACE(("wl%d: wl_sendup: %d bytes\n", wl->unit, PKTLEN(NULL, p)));

	if (wlif == NULL)
		dev = wl->dev;
	else
		dev = wlif->dev;
	chained = dev->chained;

	/* Apply TOE */
	if (TOE_ENAB(wl->pub))
		(void)wl_toe_recv_proc(wl->toei, p);

	/* Apply ARP offload */
	if (ARPOE_ENAB(wl->pub)) {
		ret_val = wl_arp_recv_proc(wl->arpi, p);
		if ((ret_val == ARP_REQ_SINK) || (ret_val == ARP_REPLY_PEER)) {
			PKTFREE(wl->pub->osh, p, FALSE);
			return;
		}
	}

	buf = PKTDATA(wl->pub->osh, p);
	type = ntoh16_ua(buf + ETHER_TYPE_OFFSET);


	if (chained) {

		/* Internally generated events have the special ether-type of
		 * ETHER_TYPE_BRCM; do not run these events through data packet filters.
		 */
		if (type != ETHER_TYPE_BRCM) {
			/* Apply packet filter */
			if ((chained->flags & RTEDEVFLAG_HOSTASLEEP) &&
			    wl->hwfflags && !wl_hwfilter(wl, p)) {
				PKTFREE(wl->pub->osh, p, FALSE);
				return;
			}

			/* Apply packet filtering. */
			if (PKT_FILTER_ENAB(wl->pub) && !LMAC_ENAB(wl->pub)) {
				if (!wlc_pkt_filter_recv_proc(wl->pkt_filter_info, p)) {
					/* Discard received packet. */
					PKTFREE(wl->pub->osh, p, FALSE);
					return;
				}
			}
		}

		if ((p = wl_pkt_header_push(wl, p, &wl_hdr_words)) == NULL) {
			return;
		}

		PKTSETDATAOFFSET(p, wl_hdr_words);
		lb = PKTTONATIVE(wl->pub->osh, p);
		if (chained->funcs->xmit(dev, chained, lb) != 0) {
			WL_ERROR(("wl_sendup: xmit failed; free pkt 0x%p\n", lb));
			lb_free(lb);
		}
	} else {
		/* only AP mode can be non chained */
		ASSERT(AP_ENAB(wl->pub));
		PKTFREE(wl->pub->osh, p, FALSE);
	}
}
#endif /* WLC_HIGH */

/* buffer received from BUS driver(e.g USB, SDIO) in dongle framework
 *   For normal driver, push it to common driver sendpkt
 *   For BMAC driver, forward to RPC layer to process
 */
#ifdef WLC_HIGH
static int
wl_send(hndrte_dev_t *src, hndrte_dev_t *dev, struct lbuf *lb)
{
	wl_info_t *wl = dev->softc;
	wl_if_t *wlif = WL_IF(wl, dev);
	struct wlc_if *wlcif = wlif != NULL ? wlif->wlcif : NULL;
	void *p;

	/* Pull wl header. Currently no information is included on transmit side */
	wl_pkt_header_pull(wl, lb);

	p = PKTFRMNATIVE(wl->pub->osh, lb);

#ifdef WLLMACPROTO
	if (LMACPROTO_ENAB(wl->pub)) {
		wlc_info_t *wlc = (wlc_info_t *)wl->wlc;
		return wlc_lmacproto_hostpkt(wlc->lmac_info, p);
	}
#endif /* WLLMACPROTO */

	WL_TRACE(("wl%d: wl_send: len %d\n", wl->unit, PKTLEN(wl->pub->osh, p)));

	/* Fix the priority if WME enabled */
	if (WME_ENAB(wl->pub) && (PKTPRIO(p) == 0))
		pktsetprio(p, FALSE);

	/* Apply ARP offload */
	if (ARPOE_ENAB(wl->pub)) {
		if (wl_arp_send_proc(wl->arpi, p) == ARP_REPLY_HOST) {
			PKTFREE(wl->pub->osh, p, TRUE);
			return TRUE;
		}
	}

	/* Apply TOE */
	if (TOE_ENAB(wl->pub))
		wl_toe_send_proc(wl->toei, p);

	if (wlc_sendpkt(wl->wlc, p, wlcif))
		return TRUE;


	return FALSE;
}
#else
static int
wl_send(hndrte_dev_t *src, hndrte_dev_t *dev, struct lbuf *lb)
{
	wl_info_t *wl = dev->softc;

	WL_TRACE(("wl%d: wl_send: len %d\n", wl->unit, lb->len));

	bcm_rpc_tp_rx_from_dnglbus(wl->rpc_th, lb);

	return FALSE;
}
#endif /* WLC_HIGH */

#ifdef WLC_HIGH
void
wl_txflowcontrol(wl_info_t *wl, struct wl_if *wlif, bool state, int prio)
{
	hndrte_dev_t *chained = wl->dev->chained;

	/* sta mode must be chained */
	if (chained && chained->funcs->txflowcontrol)
		chained->funcs->txflowcontrol(chained, state, prio);
	else
		ASSERT(AP_ENAB(wl->pub));
}

void
wl_event(wl_info_t *wl, char *ifname, wlc_event_t *e)
{
	wl_oid_event(wl->oid, e);

#ifdef WLPFN
	/* Tunnel events into PFN for analysis */
	wl_pfn_event(wl->pfn, e);
#endif /* WLPFN */

	switch (e->event.event_type) {
	case WLC_E_LINK:
	case WLC_E_NDIS_LINK:
		wl->link = e->event.flags&WLC_EVENT_MSG_LINK;
		if (wl->link)
			WL_ERROR(("wl%d: link up (%s)\n", wl->unit, ifname));
/* Getting too many */
		else
			WL_ERROR(("wl%d: link down (%s)\n", wl->unit, ifname));
		break;
#if defined(BCMSUP_PSK) && defined(STA)
	case WLC_E_MIC_ERROR: {
		wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_wlcif(wl->wlc, NULL);
		if (cfg == NULL || e->event.bsscfgidx != WLC_BSSCFG_IDX(cfg))
			break;
		wlc_sup_mic_error(cfg, (e->event.flags&WLC_EVENT_MSG_GROUP) == WLC_EVENT_MSG_GROUP);
		break;
	}
#endif
	}
}
#endif /* WLC_HIGH */

void
wl_event_sync(wl_info_t *wl, char *ifname, wlc_event_t *e)
{
}

void
wl_event_sendup(wl_info_t *wl, const wlc_event_t *e, uint8 *data, uint32 len)
{
}

#ifndef WLC_LOW_ONLY
static int
wl_ioctl(hndrte_dev_t *dev, uint32 cmd, void *buf, int len, int *used, int *needed, int set)
{
	wl_info_t *wl = dev->softc;
	wl_if_t *wlif = WL_IF(wl, dev);
	struct wlc_if *wlcif = wlif != NULL ? wlif->wlcif : NULL;
	wlc_bsscfg_t *cfg = NULL;
	int ret = 0;
	int origcmd = cmd;
	int status = 0;
	uint32 *ret_int_ptr = (uint32 *)buf;

	WL_TRACE(("wl%d: wl_ioctl: cmd 0x%x\n", wl->unit, cmd));

	cfg = wlc_bsscfg_find_by_wlcif(wl->wlc, wlcif);
	ASSERT(cfg != NULL);
	switch (cmd) {
	case RTEGHWADDR:
		ret = wlc_iovar_op(wl->wlc, "cur_etheraddr", NULL, 0, buf, len, IOV_GET, wlcif);
		break;
	case RTESHWADDR:
		ret = wlc_iovar_op(wl->wlc, "cur_etheraddr", NULL, 0, buf, len, IOV_SET, wlcif);
		break;
	case RTEGPERMADDR:
		ret = wlc_iovar_op(wl->wlc, "perm_etheraddr", NULL, 0, buf, len, IOV_GET, wlcif);
		break;
	case RTEGMTU:
		*ret_int_ptr = ETHER_MAX_DATA;
		break;
#ifdef WLC_HIGH
	case RTEGSTATS:
		wl_statsupd(wl);
		bcopy(&wl->stats, buf, MIN(len, sizeof(wl->stats)));
		break;

	case RTEGALLMULTI:
		*ret_int_ptr = cfg->allmulti;
		break;
	case RTESALLMULTI:
		cfg->allmulti = *((uint32 *) buf);
		break;
#endif /* WLC_HIGH */
	case RTEGPROMISC:
		cmd = WLC_GET_PROMISC;
		break;
	case RTESPROMISC:
		cmd = WLC_SET_PROMISC;
		break;
#ifdef WLC_HIGH
	case RTESMULTILIST: {
		int i;

		/* copy the list of multicasts into our private table */
		cfg->nmulticast = len / ETHER_ADDR_LEN;
		for (i = 0; i < cfg->nmulticast; i++)
			cfg->multicast[i] = ((struct ether_addr *)buf)[i];
		break;
	}
#endif /* WLC_HIGH */
	case RTEGUP:
		cmd = WLC_GET_UP;
		break;
	default:
		/* force call to wlc ioctl handler */
		origcmd = -1;
		break;
	}

	if (cmd != origcmd) {
#ifdef DONGLEOVERLAYS
		if (cmd == WLC_OVERLAY_IOCTL)
			return wl_overlay_copy(wl, buf, len);
#endif /* DONGLEOVERLAYS */
		if (!_wl_rte_oid_check(wl, cmd, buf, len, used, needed, set, &status))
			ret = wlc_ioctl(wl->wlc, cmd, buf, len, wlcif);
#ifdef DONGLEOVERLAYS
		if (wl->active_overlay != 0xff) {
			hndrte_overlay_invalidate(wl->active_overlay);
			wl->active_overlay = 0xff;
		}
#endif /* DONGLEOVERLAYS */
	}

	if (status)
		return status;

	return (ret);
}
#endif /* WLC_LOW_ONLY */

static int
BCMUNINITFN(wl_close)(hndrte_dev_t *dev)
{
	wl_info_t *wl = dev->softc;
	BCM_REFERENCE(wl);

	WL_TRACE(("wl%d: wl_close\n", wl->unit));

#ifdef WLC_HIGH
	/* BMAC_NOTE: ? */
	wl_down(wl);
#endif

	return 0;
}

#ifdef WLC_HIGH
static void
wl_statsupd(wl_info_t *wl)
{
	hndrte_stats_t *stats;

	WL_TRACE(("wl%d: wl_get_stats\n", wl->unit));

	stats = &wl->stats;

	/* refresh stats */
	if (wl->pub->up)
		wlc_statsupd(wl->wlc);

	stats->rx_packets = WLCNTVAL(wl->pub->_cnt->rxframe);
	stats->tx_packets = WLCNTVAL(wl->pub->_cnt->txframe);
	stats->rx_bytes = WLCNTVAL(wl->pub->_cnt->rxbyte);
	stats->tx_bytes = WLCNTVAL(wl->pub->_cnt->txbyte);
	stats->rx_errors = WLCNTVAL(wl->pub->_cnt->rxerror);
	stats->tx_errors = WLCNTVAL(wl->pub->_cnt->txerror);
	stats->rx_dropped = 0;
	stats->tx_dropped = 0;
	stats->multicast = WLCNTVAL(wl->pub->_cnt->rxmulti);
}
#endif /* WLC_HIGH */

#ifdef	CONFIG_WLU

int
wl_get(void *wlc, int cmd, void *buf, int len)
{
	return wlc_ioctl(wlc, cmd, buf, len, NULL);
}

int
wl_set(void *wlc, int cmd, void *buf, int len)
{
	return wlc_ioctl(wlc, cmd, buf, len, NULL);
}

static void
do_wl_cmd(uint32 arg, uint argc, char *argv[])
{
	wl_info_t *wl = (wl_info_t *)arg;
	cmd_t *cmd;
	int ret = 0;

	if (argc < 2)
		printf("missing subcmd\n");
	else {
		/* search for command */
		for (cmd = wl_cmds; cmd->name && strcmp(cmd->name, argv[1]); cmd++);

		/* defaults to using the set_var and get_var commands */
		if (cmd->name == NULL)
			cmd = &wl_varcmd;

		/* do command */
		ret = (*cmd->func)(wl->wlc, cmd, argv + 1);
		printf("ret=%d (%s)\n", ret, bcmerrorstr(ret));
	}
}

#endif	/* CONFIG_WLU */

#ifdef WLC_LOW_ONLY
static void
do_wlhist_cmd(uint32 arg, uint argc, char *argv[])
{
	wl_info_t *wl = (wl_info_t *)arg;

	if (strcmp(argv[1], "clear") == 0) {
		wlc_rpc_bmac_dump_txfifohist(wl->wlc_hw, FALSE);
		return;
	}

	wlc_rpc_bmac_dump_txfifohist(wl->wlc_hw, TRUE);
}

static void
do_wldpcdump_cmd(uint32 arg, uint argc, char *argv[])
{
	wl_info_t *wl = (wl_info_t *)arg;

	printf("wlc_dpc(): stopped = %d, requested = %d\n", wl->dpc_stopped, wl->dpc_requested);
	printf("\n");
}
#endif /* WLC_LOW_ONLY */
#ifdef BCMDBG
/* Mini command to control msglevel for BCMDBG builds */
static void
do_wlmsg_cmd(uint32 arg, uint argc, char *argv[])
{
	switch (argc) {
	case 3:
		/* Set both msglevel and msglevel2 */
		wl_msg_level2 = strtoul(argv[2], 0, 0);
		/* fall through */
	case 2:
		/* Set msglevel */
		wl_msg_level = strtoul(argv[1], 0, 0);
		break;
	case 1:
		/* Display msglevel and msglevel2 */
		printf("msglvl1=0x%x msglvl2=0x%x\n", wl_msg_level, wl_msg_level2);
		break;
	}
}
#endif /* BCMDBG */

#ifdef NOT_YET
static int
BCMATTACHFN(wl_module_init)(si_t *sih)
{
	uint16 id;

	WL_TRACE(("wl_module_init: add WL device\n"));

	if ((id = si_d11_devid(sih)) == 0xffff)
		id = BCM4318_D11G_ID;

	return hndrte_add_device(&bcmwl, D11_CORE_ID, id);
}

HNDRTE_MODULE_INIT(wl_module_init);

#endif /* NOT_YET */

#ifdef WLC_LOW_ONLY


#if defined(HNDRTE_PT_GIANT) && defined(DMA_TX_FREE)
static void
wl_lowmem_free(void *wlh)
{
	wl_info_t *wl = (wl_info_t*)wlh;
	wlc_hw_info_t *wlc_hw = wl->wlc_hw;
	int i;

	/* process any tx reclaims */
	for (i = 0; i < NFIFO; i++) {
		if (wlc_hw->di[i])
			dma_txreclaim(wlc_hw->di[i], HNDDMA_RANGE_TRANSFERED);
	}
}
#endif /* HNDRTE_PT_GIANT && DMA_TX_FREE */

static void
wl_rpc_tp_txflowctl(hndrte_dev_t *dev, bool state, int prio)
{
	wl_info_t *wl = dev->softc;

	bcm_rpc_tp_txflowctl(wl->rpc_th, state, prio);
}

static void
wl_rpc_down(void *wlh)
{
	wl_info_t *wl = (wl_info_t*)(wlh);

	if (wlc_bmac_down_prep(wl->wlc_hw) == 0)
		wlc_bmac_down_finish(wl->wlc_hw);
}

static void
wl_rpc_resync(void *wlh)
{
	wl_info_t *wl = (wl_info_t*)(wlh);

	/* reinit to all the default values */
	wlc_bmac_info_init(wl->wlc_hw);

	/* reload original  macaddr */
	bcopy(&wl->wlc_hw->orig_etheraddr, &wl->wlc_hw->etheraddr, ETHER_ADDR_LEN);
	ASSERT(!ETHER_ISBCAST((char*)&wl->wlc_hw->etheraddr));
	ASSERT(!ETHER_ISNULLADDR((char*)&wl->wlc_hw->etheraddr));
}

/* CLIENT dongle driver RPC dispatch routine, called by bcm_rpc_buf_recv()
 *  Based on request, push to common driver or send back result
 */
static void
wl_rpc_bmac_dispatch(void *ctx, struct rpc_buf* buf)
{
	wlc_rpc_ctx_t *rpc_ctx = (wlc_rpc_ctx_t *)ctx;

	wlc_rpc_bmac_dispatch(rpc_ctx, buf);
}

static void
wl_rpc_txflowctl(void *wlh, bool on)
{
	wl_info_t *wl = (wl_info_t *)(wlh);

	if (!wl->wlc_hw->up) {
		wl->dpc_stopped = FALSE;
		wl->dpc_requested = FALSE;
		return;
	}

	if (on) {	/* flowcontrol activated */
		if (!wl->dpc_stopped) {
			WL_TRACE(("dpc_stopped set!\n"));
			wl->dpc_stopped = TRUE;
		}
	} else {	/* flowcontrol released */

		if (!wl->dpc_stopped)
			return;

		WL_TRACE(("dpc_stopped cleared!\n"));
		wl->dpc_stopped = FALSE;

		/* if there is dpc requeset pending, run it */
		if (wl->dpc_requested) {
			wl->dpc_requested = FALSE;
			wl_dpc(wl);
		}
	}
}
#endif /* WLC_LOW_ONLY */


#ifndef WLC_LOW_ONLY
static bool
_wl_rte_oid_check(wl_info_t *wl, uint32 cmd, void *buf, int len, int *used, int *needed,
	bool set, int *status)
{
	return FALSE;
}
#endif /* WLC_LOW_ONLY */

#ifdef DONGLEOVERLAYS
int
wl_overlay_copy(wl_info_t *wl, uint8 *buf, int len)
{
	wl_ioctl_overlay_t overlay;
	int ret;
	uint8 active_overlay;

	if (len < sizeof(wl_ioctl_overlay_t))
		return BCME_BUFTOOSHORT;

	bcopy(buf, &overlay, sizeof(wl_ioctl_overlay_t));

	active_overlay = (uint8) (overlay.flags_idx & OVERLAY_IDX_MASK);
	WL_TRACE(("wl%d: %s: o.idx %d, o.offset %d, o.len %d\n", wl->pub->unit,
	          __FUNCTION__, active_overlay, overlay.offset, overlay.len));

	ret = hndrte_overlay_copy((uint32)active_overlay, buf + sizeof(wl_ioctl_overlay_t),
	                          overlay.offset, overlay.len);
	if (ret == BCME_OK)
		wl->active_overlay = active_overlay;

	return ret;
}

#ifdef DONGLEOVERLAYTEST
int
BCMOVERLAY0FN(wl_overlaytest_iovar)(wl_info_t *wl, uint32 *val)
{
	*val = 123456789;
	return BCME_OK;
}

int
BCMOVERLAY1FN(wl_overlaytest2_iovar)(wl_info_t *wl, uint32 *val)
{
	*val = 987654321;
	return BCME_OK;
}
#endif /* DONGLEOVERLAYTEST */
#endif /* DONGLEOVERLAYS */
