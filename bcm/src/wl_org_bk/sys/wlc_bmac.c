/*
 * Common (OS-independent) portion of
 * Broadcom 802.11bang Networking Device Driver
 *
 * BMAC portion of common driver. The external functions should match wlc_bmac_stubs.c
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_bmac.c,v 1.530.2.188 2011-02-10 02:07:49 Exp $
 */

#ifndef WLC_LOW
#error "This file needs WLC_LOW"
#endif

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <proto/802.11.h>
#include <bcmwifi.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <sbconfig.h>
#include <sbchipc.h>
#include <pcicfg.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_channel.h>
#include <wlc_pio.h>
#include <bcmsrom.h>
#include  <wlc_rm.h>
#ifdef WLC_LOW_ONLY
#include <bcmnvram.h>
#endif
#ifdef WLC_HIGH
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#endif /* WLC_HIGH */

/* BMAC_NOTE: a WLC_HIGH compile include of wlc.h adds in more structures and type
 * dependencies. Need to include these to files to allow a clean include of wlc.h
 * with WLC_HIGH defined.
 * At some point we may be able to skip the include of wlc.h and instead just
 * define a stub wlc_info and band struct to allow rpc calls to get the rpc handle.
 */

#include <wlc.h>


#include <wlc_bmac.h>
#include <wlc_phy_shim.h>
#include <wlc_phy_hal.h>
#include <wlc_led.h>
#include <wl_export.h>
#include "d11ucode.h"
#include <bcmotp.h>
#include  <wlc_stf.h>
#ifdef WLC_HIGH
#ifdef WLP2P
#include <wlc_p2p.h>
#endif
#endif /* WLC_HIGH */

/* BMAC_NOTE: With WLC_HIGH defined, some fns in this file make calls to high level
 * functions defined in the headers below. We should be eliminating those calls and
 * will be able to delete these include lines.
 */
#ifdef WLC_HIGH
#ifdef CRAM
#include <wlc_cram.h>
#endif
#include <wlc_ap.h>
#include <wlc_scan.h>
#include <wlc_antsel.h>
#include <wllmacctl.h>
#include <wlc_lmac.h>
#endif /* WLC_HIGH */

#ifdef WLDIAG
#include <wlc_diag.h>
#endif
#include <pcie_core.h>
#ifdef ROUTER_COMA
#include <hndchipc.h>
#include <hndjtagdefs.h>
#endif
#if defined(BCMDBG) || defined(WLTEST)
#include <bcmsrom.h>
#endif


#ifdef AP
#include <wlc_apps.h>
#endif

#include <wlc_extlog.h>
#include <wlc_alloc.h>

#ifdef WL_MULTIQUEUE
#define WL_MQ(x) do {} while (0)
#endif /* WL_MULTIQUEUE */

#ifdef WLC_HIGH
#ifndef WLC_PHYTXERR_THRESH
#define WLC_PHYTXERR_THRESH 20
#endif

uint wlc_phytxerr_thresh = WLC_PHYTXERR_THRESH;
#endif /* WLC_HIGH */

#define	TIMER_INTERVAL_WATCHDOG_BMAC	1000	/* watchdog timer, in unit of ms */
#define	TIMER_INTERVAL_RPC_AGG_WATCHDOG_BMAC	10 /* rpc agg watchdog timer, in unit of ms */

#define	SYNTHPU_DLY_PHY_US_QT	100	/* QT(no radio) synthpu_dly time in us */

#define	SYNTHPU_DLY_APHY_US	3700	/* a phy synthpu_dly time in us */
#define	SYNTHPU_DLY_BPHY_US	1050	/* b/g phy synthpu_dly time in us, default */
#define	SYNTHPU_DLY_LPPHY_US	300	/* lpphy synthpu_dly time in us */
#define	SYNTHPU_DLY_NPHY_US	2048	/* n phy REV3 synthpu_dly time in us, default */
#define	SYNTHPU_DLY_HTPHY_US	2800	/* HT phy REV0 synthpu_dly time in us, default */


typedef struct _btc_flags_ucode {
	uint8	idx;
	uint16	mask;
} btc_flags_ucode_t;

#define BTC_FLAGS_SIZE 9
#define BTC_FLAGS_MHF3_START 1
#define BTC_FLAGS_MHF3_END   6

const btc_flags_ucode_t btc_ucode_flags[BTC_FLAGS_SIZE] = {
	{MHF2, MHF2_BTCPREMPT},
	{MHF3, MHF3_BTCX_DEF_BT},
	{MHF3, MHF3_BTCX_ACTIVE_PROT},
	{MHF3, MHF3_BTCX_SIM_RSP},
	{MHF3, MHF3_BTCX_PS_PROTECT},
	{MHF3, MHF3_BTCX_SIM_TX_LP},
	{MHF3, MHF3_BTCX_ECI},
	{MHF5, MHF5_BTCX_LIGHT},
	{MHF5, MHF5_BTCX_PARALLEL}
};

#ifndef BMAC_DUP_TO_REMOVE
#define WLC_RM_WAIT_TX_SUSPEND		4 /* Wait Tx Suspend */
#define	ANTCNT			10		/* vanilla M_MAX_ANTCNT value */
#ifdef WLAFTERBURNER
#define	ANTCNT_AB		5		/* afterburner M_MAX_ANTCNT value */
#endif
#endif	/* BMAC_DUP_TO_REMOVE */

#define DMAREG(wlc_hw, direction, fifonum)	(D11REV_LT(wlc_hw->corerev, 11) ? \
	((direction == DMA_TX) ? \
		(void*)(uintptr)&(wlc_hw->regs->fifo.f32regs.dmaregs[fifonum].xmt) : \
		(void*)(uintptr)&(wlc_hw->regs->fifo.f32regs.dmaregs[fifonum].rcv)) : \
	((direction == DMA_TX) ? \
		(void*)(uintptr)&(wlc_hw->regs->fifo.f64regs[fifonum].dmaxmt) : \
		(void*)(uintptr)&(wlc_hw->regs->fifo.f64regs[fifonum].dmarcv)))

/*
 * The following table lists the buffer memory allocated to xmt fifos in HW.
 * the size is in units of 256bytes(one block), total size is HW dependent
 * ucode has default fifo partition, sw can overwrite if necessary
 *
 * This is documented in twiki under the topic UcodeTxFifo. Please ensure
 * the twiki is updated before making changes.
 */

#define XMTFIFOTBL_STARTREV	4	/* Starting corerev for the fifo size table */

static uint16 xmtfifo_sz[][NFIFO] = {
	{ 14, 14, 14, 14, 14, 2 }, 	/* corerev 4: 3584, 3584, 3584, 3584, 3584, 512 */
	{ 9, 13, 10, 8, 13, 1 }, 	/* corerev 5: 2304, 3328, 2560, 2048, 3328, 256 */
	{ 9, 13, 10, 8, 13, 1 }, 	/* corerev 6: 2304, 3328, 2560, 2048, 3328, 256 */
	{ 9, 13, 10, 8, 13, 1 }, 	/* corerev 7: 2304, 3328, 2560, 2048, 3328, 256 */
	{ 9, 13, 10, 8, 13, 1 }, 	/* corerev 8: 2304, 3328, 2560, 2048, 3328, 256 */
#if defined(MBSS)
	/* Fifo sizes are different for ucode with this support */
	{ 9, 14, 10, 9, 14, 6 }, 	/* corerev 9: 2304, 3584, 2560, 2304, 3584, 1536 */
#else
	{ 10, 14, 11, 9, 14, 2 }, 	/* corerev 9: 2560, 3584, 2816, 2304, 3584, 512 */
#endif
	{ 10, 14, 11, 9, 14, 2 }, 	/* corerev 10: 2560, 3584, 2816, 2304, 3584, 512 */
#ifdef MACOSX
	/* Give more bandwidth to BK traffic as Apple wants to Aggregate */
	{ 30, 47, 22, 14, 8, 1 }, 	/* corerev 11: 5632, 12032, 5632, 3584, 2048, 256 */
	{ 30, 47, 22, 14, 8, 1 }, 	/* corerev 12: 5632, 12032, 5632, 3584, 2048, 256 */
#else
	{ 9, 58, 22, 14, 14, 5 }, 	/* corerev 11: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 58, 22, 14, 14, 5 }, 	/* corerev 12: 2304, 14848, 5632, 3584, 3584, 1280 */
#endif
	{ 10, 14, 11, 9, 14, 5 }, 	/* corerev 13: 2560, 3584, 2816, 2304, 3584, 1280 */
	{ 10, 14, 11, 9, 14, 2 }, 	/* corerev 14: 2560, 3584, 2816, 2304, 3584, 512 */
	{ 10, 14, 11, 9, 14, 2 }, 	/* corerev 15: 2560, 3584, 2816, 2304, 3584, 512 */
#ifdef MACOSX
	/* Give more bandwidth to BK traffic as Apple wants to Aggregate */
	{ 98, 159, 160, 21, 8, 1 },	/* corerev 16: 25088, 40704, 40960, 5376, 2048, 256 */
#else /* MACOSX */
#ifdef WLLPRS
	{ 20, 176, 192, 21, 17, 5 },	/* corerev 16: 5120, 45056, 49152, 5376, 4352, 1280 */
#else /* WLLPRS */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 16: 5120, 49152, 49152, 5376, 4352, 1280 */
#endif /* WLLPRS */
#endif /* MACOSX */
#ifdef WLLPRS
	{ 20, 176, 192, 21, 17, 5 },	/* corerev 17: 5120, 45056, 49152, 5376, 4352, 1280 */
#else /* WLLPRS */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 17: 5120, 49152, 49152, 5376, 4352, 1280 */
#endif /* WLLPRS */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 18: 5120, 49152, 49152, 5376, 4352, 1280 */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 19: 5120, 49152, 49152, 5376, 4352, 1280 */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 20: 5120, 49152, 49152, 5376, 4352, 1280 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 21: 2304, 14848, 5632, 3584, 3584, 1280 */
#ifdef WLLPRS
	{ 9, 42, 22, 14, 14, 5 }, 	/* corerev 22: 2304, 10752, 5632, 3584, 3584, 1280 */
#else /* WLLPRS */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 22: 2304, 14848, 5632, 3584, 3584, 1280 */
#endif /* WLLPRS */
#ifdef MACOSX
	/* Give more bandwidth to BK traffic as Apple wants to Aggregate */
	{ 98, 159, 160, 21, 8, 1 },	/* corerev 23: 25088, 40704, 40960, 5376, 2048, 256 */
#else
	{ 20, 192, 192, 21, 17, 5 },    /* corerev 23: 5120, 49152, 49152, 5376, 4352, 1280 */
#endif
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 24: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 25: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 150, 223, 223, 21, 17, 5 },	/* corerev 26: 38400, 57088, 57088, 5376, 4352, 1280 */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 27: 5120, 49152, 49152, 5376, 4352, 1280 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 28: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 29: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 192, 22, 14, 14, 5 },	/* corerev 30: 2304, 49152, 5632, 3584, 3584, 1280 */
	{ 9, 58, 22, 14, 14, 5 }	/* corerev 31: 2304, 14848, 5632, 3584, 3584, 1280 */
};

/* corerev 26 host agg fifo size: 38400, 57088, 57088, 5376, 4352, 1280 */
static uint16 xmtfifo_sz_hostagg[] = { 150, 223, 223, 21, 17, 5 };
/* corerev 26 hw agg fifo size: 25088, 65280, 62208, 5376, 4352, 1280 */
static uint16 xmtfifo_sz_hwagg[] = { 98, 255, 243, 21, 17, 5 };

/* WLP2P Support */
#ifdef WLP2P
#ifdef DONGLEBUILD
#define P2P_ENAB_HW(wlc_hw)	1
#elif defined WLC_LOW_ONLY
#define P2P_ENAB_HW(wlc_hw)	1
#else
#define P2P_ENAB_HW(wlc_hw)	(wlc_hw->_p2p_hw)
#endif
#else
#define P2P_ENAB_HW(wlc_hw)	0
#endif /* WLP2P */

typedef struct bmac_pmq_entry {
	struct ether_addr ea;		/* station address */
	uint8 switches;
	uint8 ps_on;
	struct bmac_pmq_entry *next;
} bmac_pmq_entry_t;


#define BMAC_PMQ_SIZE 16
#define BMAC_PMQ_MIN_ROOM 5

struct bmac_pmq {
	bmac_pmq_entry_t *entry;
	int active_entries; /* number of entries still to receive akcs from high driver  */
	uint8 tx_draining; /* total number of entries */
	uint8 pmq_read_count; /* how many entries have been read since the last clear */
	uint8 pmq_size;
};


static void wlc_clkctl_clk(wlc_hw_info_t *wlc, uint mode);
static void wlc_coreinit(wlc_hw_info_t *wlc_hw);

/* used by wlc_bmac_wakeucode_init() */
static void wlc_write_inits(wlc_hw_info_t *wlc_hw, const d11init_t *inits);
static void wlc_ucode_write(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes);
static void wlc_ucode_write_byte(wlc_hw_info_t *wlc_hw, const uint8 ucode[], const uint nbytes);
static void wlc_ucode_download(wlc_hw_info_t *wlc);
static void wlc_ucode_txant_set(wlc_hw_info_t *wlc_hw);

/* used by wlc_dpc() */
static bool wlc_bmac_dotxstatus(wlc_hw_info_t *wlc, tx_status_t *txs, uint32 s2);
static bool wlc_bmac_txstatus_corerev4(wlc_hw_info_t *wlc);
static bool wlc_bmac_txstatus(wlc_hw_info_t *wlc, bool bound, bool *fatal);
static bool wlc_bmac_recv(wlc_hw_info_t *wlc_hw, uint fifo, bool bound);
#if defined(STA) && defined(BCMDBG)
static void wlc_bmac_dma_lpbk(wlc_hw_info_t *wlc_hw, bool enable);
#endif

#ifdef WLLED
static void wlc_bmac_led_hw_init(wlc_hw_info_t *wlc_hw);
#endif

/* used by wlc_down() */
static void wlc_flushqueues(wlc_hw_info_t *wlc_hw);

static void wlc_write_mhf(wlc_hw_info_t *wlc_hw, uint16 *mhfs);
static void wlc_mctrl_reset(wlc_hw_info_t *wlc_hw);
static void wlc_corerev_fifofixup(wlc_hw_info_t *wlc_hw);

static void wlc_bmac_btc_param_init(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_btc_btcflag2ucflag(wlc_hw_info_t *wlc_hw);
static bool wlc_bmac_btc_param_to_shmem(wlc_hw_info_t *wlc_hw, uint32 *pval);
static bool wlc_bmac_btc_flags_ucode(uint8 val, uint8 *idx, uint16 *mask);
static void wlc_bmac_btc_flags_upd(wlc_hw_info_t *wlc_hw, bool set_clear, uint16, uint8, uint16);
static void wlc_bmac_btc_gpio_enable(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_btc_gpio_disable(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_btc_gpio_configure(wlc_hw_info_t *wlc_hw);
#ifdef WLC_LOW_ONLY
static void wlc_bmac_dngl_wd_upd(wlc_hw_info_t *wlc_hw);
#endif
#ifdef STA
static void wlc_bmac_btc_update_predictor(wlc_hw_info_t *wlc_hw);
#endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_bmac_btc_dump(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b);
#endif

/* Low Level Prototypes */
#ifdef AP
#ifdef WLC_LOW_ONLY
static void wlc_bmac_pmq_remove(wlc_hw_info_t *wlc, bmac_pmq_entry_t *pmq_entry);
static bmac_pmq_entry_t * wlc_bmac_pmq_find(wlc_hw_info_t *wlc, struct ether_addr *ea);
static bmac_pmq_entry_t * wlc_bmac_pmq_add(wlc_hw_info_t *wlc, struct ether_addr *ea);
static void wlc_bmac_pa_war_set(wlc_hw_info_t *wlc_hw, bool enable);
#endif
static void wlc_bmac_pmq_delete(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_pmq_init(wlc_hw_info_t *wlc);
static bool wlc_bmac_processpmq(wlc_hw_info_t *wlc, bool bounded);
static void wlc_bmac_clearpmq(wlc_hw_info_t *wlc);
#endif /* AP */
static uint16 wlc_bmac_read_objmem(wlc_hw_info_t *wlc_hw, uint offset, uint32 sel);
static void wlc_bmac_write_objmem(wlc_hw_info_t *wlc_hw, uint offset, uint16 v, uint32 sel);
static bool wlc_bmac_attach_dmapio(wlc_hw_info_t *wlc_hw, uint j, bool wme);
static void wlc_bmac_detach_dmapio(wlc_hw_info_t *wlc_hw);
static void wlc_ucode_pcm_write(wlc_hw_info_t *wlc_hw, const uint32 pcm[], const uint nbytes);
static void wlc_ucode_bsinit(wlc_hw_info_t *wlc_hw);
static bool wlc_validboardtype(wlc_hw_info_t *wlc);
static bool wlc_isgoodchip(wlc_hw_info_t* wlc_hw);
static char* wlc_get_macaddr(wlc_hw_info_t *wlc_hw);
static void wlc_mhfdef(wlc_hw_info_t *wlc_hw, uint16 *mhfs, uint16 mhf2_init);
static void wlc_mctrl_write(wlc_hw_info_t *wlc_hw);
static void wlc_ucode_mute_override_set(wlc_hw_info_t *wlc_hw);
static void wlc_ucode_mute_override_clear(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_ifsctl1_regshm(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val);
#if defined(STA) && defined(WLRM)
static uint16 wlc_bmac_read_ihr(wlc_hw_info_t *wlc_hw, uint offset);
#endif
static uint32 wlc_wlintrsoff(wlc_hw_info_t *wlc_hw);
static void wlc_wlintrsrestore(wlc_hw_info_t *wlc_hw, uint32 macintmask);
#ifdef BCMDBG
static bool wlc_intrs_enabled(wlc_hw_info_t *wlc_hw);
#endif /* BCMDBG */
static void wlc_gpio_init(wlc_hw_info_t *wlc_hw);
static void wlc_write_hw_bcntemplate0(wlc_hw_info_t *wlc_hw, void *bcn, int len);
static void wlc_write_hw_bcntemplate1(wlc_hw_info_t *wlc_hw, void *bcn, int len);
static void wlc_bmac_bsinit(wlc_hw_info_t *wlc_hw, chanspec_t chanspec);
static uint32 wlc_setband_inact(wlc_hw_info_t *wlc_hw, uint bandunit);
static void wlc_bmac_setband(wlc_hw_info_t *wlc_hw, uint bandunit, chanspec_t chanspec);
static void wlc_bmac_update_slot_timing(wlc_hw_info_t *wlc_hw, bool shortslot);
#ifdef WL11N
static void wlc_upd_ofdm_pctl1_table(wlc_hw_info_t *wlc_hw);
static uint16 wlc_bmac_ofdm_ratetable_offset(wlc_hw_info_t *wlc_hw, uint8 rate);
#endif
#if (defined(BCMNVRAMR) || defined(BCMNVRAMW)) && defined(WLTEST)
static int wlc_bmac_cissource(wlc_hw_info_t *wlc_hw);
#endif 

#ifdef WLC_LOW_ONLY

/* debug/trace */
#ifdef BCMDBG
uint wl_msg_level = WL_ERROR_VAL;
#else
uint wl_msg_level = 0;
#endif
uint wl_msg_level2 = 0;

#endif /* WLC_LOW_ONLY */
#ifdef WLC_LOW_ONLY
#if defined(BCMPKTPOOL)
extern struct pktpool pktpool_shared;
#endif 
#endif /* WLC_LOW_ONLY */

/* === Low Level functions === */

#ifdef WLC_LOW_ONLY

wlc_pub_t *
wlc_pub(void *wlc)
{
	return ((wlc_info_t *)wlc)->pub;
}

void *
BCMATTACHFN(wlc_attach)(void *wl, uint16 vendor, uint16 device, uint unit, bool piomode,
                      osl_t *osh, void *regsva, uint bustype, void *btparam, uint *perr)
{
	wlc_info_t *wlc;
	uint err = 0;

	WL_TRACE(("wl%d: wlc_attach: vendor 0x%x device 0x%x\n", unit, vendor, device));


	/* allocate wlc_info_t state and its substructures */
	if ((wlc = (wlc_info_t*) wlc_attach_malloc(osh, unit, &err, device)) == NULL)
		goto fail;
	wlc->osh = osh;

	wlc->core = wlc->corestate;
	wlc->wl = wl;
	wlc->pub->_piomode = piomode;

	/* do the low level hw attach steps */
	err = wlc_bmac_attach(wlc->hw, vendor, device, unit, piomode, osh, regsva, bustype,
		btparam);
	if (err)
		goto fail;

	wlc->band = wlc->bandstate[IS_SINGLEBAND_5G(wlc->hw->deviceid) ?
		BAND_5G_INDEX : BAND_2G_INDEX];

	if (!(wlc->hw->wdtimer = wl_init_timer(wlc->wl, wlc_bmac_watchdog, wlc, "watchdog"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for wdtimer failed\n", unit, __FUNCTION__));
		err = 39;
		goto fail;
	}

	if (!(wlc->hw->rpc_agg_wdtimer = wl_init_timer(wlc->wl, wlc_bmac_rpc_agg_watchdog,
		wlc, "rpc_agg_watchdog"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for rpc_agg_wdtimer failed\n", unit,
			__FUNCTION__));
		err = 39;
		goto fail;
	}

	if (perr)
		*perr = 0;
	return ((void*)wlc);

fail:
	WL_ERROR(("wl%d: wlc_attach: failed with err %d\n", unit, err));
	wlc_detach_mfree(wlc, osh);

	if (perr)
		*perr = err;
	return (NULL);
}

/*
 * Do we need a separate low implementation for this, or just wlc_bmac_detach()
 * that is called from high level wlc_detach()?
 */
uint
BCMATTACHFN(wlc_detach)(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw;

	if (wlc == NULL)
		return 0;

	wlc_hw = wlc->hw;

	/* free timer state */
	if (wlc_hw->wdtimer) {
		wl_free_timer(wlc->wl, wlc_hw->wdtimer);
		wlc_hw->wdtimer = NULL;
	}

	/* free timer state */
	if (wlc_hw->rpc_agg_wdtimer) {
		wl_free_timer(wlc->wl, wlc_hw->rpc_agg_wdtimer);
		wlc_hw->rpc_agg_wdtimer = NULL;
	}

	wlc_bmac_detach(wlc_hw);

	wlc_detach_mfree(wlc, wlc->osh);
	return 0;
}

void
BCMINITFN(wlc_reset)(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;

	WL_TRACE(("wl%d: wlc_reset\n", wlc_hw->unit));

	wlc_bmac_reset(wlc_hw);
}

int
wlc_ioctl(wlc_info_t *wlc, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	return 0;
}

int
wlc_iovar_op(wlc_info_t *wlc, const char *name,
	void *params, int p_len, void *arg, int len,
             bool set, struct wlc_if *wlcif)
{
	return 0;
}
#endif /* WLC_LOW_ONLY */

void
wlc_bmac_set_shortslot(wlc_hw_info_t *wlc_hw, bool shortslot)
{
	wlc_hw->shortslot = shortslot;

	if (BAND_2G(wlc_hw->band->bandtype) && wlc_hw->up) {
		wlc_bmac_suspend_mac_and_wait(wlc_hw);
		wlc_bmac_update_slot_timing(wlc_hw, shortslot);
		wlc_bmac_enable_mac(wlc_hw);
	}
}

/*
 * Update the slot timing for standard 11b/g (20us slots)
 * or shortslot 11g (9us slots)
 * The PSM needs to be suspended for this call.
 */
static void
wlc_bmac_update_slot_timing(wlc_hw_info_t *wlc_hw, bool shortslot)
{
	osl_t *osh;
	d11regs_t *regs;

	osh = wlc_hw->osh;
	regs = wlc_hw->regs;

	if (shortslot) {
		/* 11g short slot: 11a timing */
		W_REG(osh, &regs->ifs_slot, 0x0207);	/* APHY_SLOT_TIME */
		wlc_bmac_write_shm(wlc_hw, M_DOT11_SLOT, APHY_SLOT_TIME);
	} else {
		/* 11g long slot: 11b timing */
		W_REG(osh, &regs->ifs_slot, 0x0212);	/* BPHY_SLOT_TIME */
		wlc_bmac_write_shm(wlc_hw, M_DOT11_SLOT, BPHY_SLOT_TIME);
	}
}

/* Helper functions for full ROM chips */
static CONST d11init_t*
WLBANDINITFN(wlc_get_lcn0bsinitvals24_addr)(void)
{
	return (d11lcn0bsinitvals24);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_lcn0initvals24_addr)(void)
{
	return (d11lcn0initvals24);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_lcn0bsinitvals25_addr)(void)
{
	return (d11lcn0bsinitvals25);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_lcn0initvals25_addr)(void)
{
	return (d11lcn0initvals25);
}

static void
WLBANDINITFN(wlc_ucode_bsinit)(wlc_hw_info_t *wlc_hw)
{
	/* init microcode host flags */
	wlc_write_mhf(wlc_hw, wlc_hw->band->mhfs);

	/* do band-specific ucode IHR, SHM, and SCR inits */
	if (D11REV_IS(wlc_hw->corerev, 31)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ht0bsinitvals29);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 30)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n16bsinitvals30);
	} else if (D11REV_IS(wlc_hw->corerev, 29)) {
		if (WLCISHTPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ht0bsinitvals29);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 26)) {
		if (WLCISHTPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 26))
				wlc_write_inits(wlc_hw, d11ht0bsinitvals26);
			else if (D11REV_IS(wlc_hw->corerev, 29))
				wlc_write_inits(wlc_hw, d11ht0bsinitvals29);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 25) || D11REV_IS(wlc_hw->corerev, 28)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11n0bsinitvals25);
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_lcn0bsinitvals25_addr());
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 24)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11n0bsinitvals24);
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_lcn0bsinitvals24_addr());
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_GE(wlc_hw->corerev, 22)) {
		if (WLCISNPHY(wlc_hw->band)) {
			/* ucode only supports rev23(43224b0) with rev16 ucode */
			if (D11REV_IS(wlc_hw->corerev, 23))
				wlc_write_inits(wlc_hw, d11n0bsinitvals16);
			else
				wlc_write_inits(wlc_hw, d11n0bsinitvals22);
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 22))
				wlc_write_inits(wlc_hw, d11sslpn4bsinitvals22);
			else
				WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
		}
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 21)) {
		if (WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11sslpn3bsinitvals21);
		}
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 21\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 20) && WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11sslpn1bsinitvals20);
	} else if (D11REV_GE(wlc_hw->corerev, 16)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n0bsinitvals16);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11sslpn0bsinitvals16);
		else if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0bsinitvals16);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 15)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0bsinitvals15);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 15\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 14)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0bsinitvals14);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 14\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 13)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0bsinitvals13);
		else if (WLCISGPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11b0g0bsinitvals13);
		else if (WLCISAPHY(wlc_hw->band) &&
			(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_2G_PHY))
			wlc_write_inits(wlc_hw, d11a0g1bsinitvals13);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 13\n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 11)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n0bsinitvals11);
		else
			WL_ERROR(("wl%d: corerev >= 11 && ! NPHY\n", wlc_hw->unit));
#if defined(MBSS)
	} else if (D11REV_IS(wlc_hw->corerev, 9)) {
		/* Only ucode for corerev 9 has this support */
		if (WLCISAPHY(wlc_hw->band)) {
			if (si_core_sflags(wlc_hw->sih, 0, 0) & SISF_2G_PHY)
				wlc_write_inits(wlc_hw, d11a0g1bsinitvals9);
			else
				wlc_write_inits(wlc_hw, d11a0g0bsinitvals9);
		} else
			wlc_write_inits(wlc_hw, d11b0g0bsinitvals9);
#endif 
	} else if (D11REV_GE(wlc_hw->corerev, 5)) {
		if (WLCISAPHY(wlc_hw->band)) {
			if (si_core_sflags(wlc_hw->sih, 0, 0) & SISF_2G_PHY)
				wlc_write_inits(wlc_hw, d11a0g1bsinitvals5);
			else
				wlc_write_inits(wlc_hw, d11a0g0bsinitvals5);
		} else
			wlc_write_inits(wlc_hw, d11b0g0bsinitvals5);
	} else if (D11REV_IS(wlc_hw->corerev, 4)) {
		if (WLCISAPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11a0g0bsinitvals4);
		else
			wlc_write_inits(wlc_hw, d11b0g0bsinitvals4);
	} else
		WL_ERROR(("wl%d: %s: corerev %d is invalid", wlc_hw->unit,
			__FUNCTION__, wlc_hw->corerev));
}

/* switch to new band but leave it inactive */
static uint32
WLBANDINITFN(wlc_setband_inact)(wlc_hw_info_t *wlc_hw, uint bandunit)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	uint32 macintmask;
	uint32 tmp;

	WL_TRACE(("wl%d: wlc_setband_inact\n", wlc_hw->unit));

	ASSERT(bandunit != wlc_hw->band->bandunit);
	ASSERT(si_iscoreup(wlc_hw->sih));
	ASSERT((R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC) == 0);

	/* disable interrupts */
	macintmask = wl_intrsoff(wlc->wl);

	/* radio off -- NPHY radios don't require to be turned off and on on a band switch */
	if (!(WLCISNPHY(wlc_hw->band) && NREV_GE(wlc_hw->band->phyrev, 3)))
		wlc_phy_switch_radio(wlc_hw->band->pi, OFF);


	ASSERT(wlc_hw->clk);

	if (D11REV_LT(wlc_hw->corerev, 17))
		tmp = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);

	wlc_bmac_core_phy_clk(wlc_hw, OFF);

	wlc_setxband(wlc_hw, bandunit);

	return (macintmask);
}

static void
WLBANDINITFN(wlc_bmac_core_phyclk_abg_switch)(wlc_hw_info_t *wlc_hw)
{
	uint cnt = 0;
	uint32 fsbit = WLCISAPHY(wlc_hw->band) ? SICF_FREF : 0;

	/* toggle SICF_FREF */

	ASSERT(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA);
	while (cnt++ < 5) {
		if ((BUSTYPE(wlc_hw->sih->bustype) == PCMCIA_BUS) &&
			(D11REV_IS(wlc_hw->corerev, 9))) {
			si_core_cflags_wo(wlc_hw->sih, SICF_FREF, fsbit);
			OSL_DELAY(2 * MIN_SLOW_CLK); /* 2 slow clock tick (worst case 32khz) */
			/* readback to ensure write completion */
			si_core_cflags(wlc_hw->sih, 0, 0);
		} else {
			si_core_cflags(wlc_hw->sih, SICF_FREF, fsbit);
			OSL_DELAY(2 * MIN_SLOW_CLK); /* 2 slow clock tick (worst case 32khz) */
		}
		SPINWAIT(!(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA), 2 * FREF_DELAY);

		if (D11REV_GE(wlc_hw->corerev, 8) ||
		    (si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA))
			break;

		si_core_cflags(wlc_hw->sih, SICF_FREF, WLCISAPHY(wlc_hw->band) ? 0 : SICF_FREF);
		OSL_DELAY(FREF_DELAY);
	}
	ASSERT(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA);
}

/* Process received frames */
/*
 * Return TRUE if more frames need to be processed. FALSE otherwise.
 * Param 'bound' indicates max. # frames to process before break out.
 */
static bool BCMFASTPATH
wlc_bmac_recv(wlc_hw_info_t *wlc_hw, uint fifo, bool bound)
{
	void *p;
	void *head = NULL;
	void *tail = NULL;
	uint n = 0;
	uint bound_limit = bound ? wlc_hw->wlc->pub->tunables->rxbnd : -1;
	uint32 tsf_h, tsf_l;
	wlc_d11rxhdr_t *wlc_rxhdr = NULL;

#if defined(WLC_LOW_ONLY)
#ifndef BCMPKTPOOL
	uint32 mem_avail;
	/* always bounded for bmac */
	bound_limit = wlc_hw->wlc->pub->tunables->rxbnd;
	/* check how many rxbufs we can afford to refill */
	mem_avail = OSL_MEM_AVAIL();
	if (mem_avail < wlc_hw->mem_required_def) {
		if (mem_avail > wlc_hw->mem_required_lower) {
			bound_limit = bound_limit - (bound_limit >> 2);
		}
		else if (mem_avail > wlc_hw->mem_required_least) {
			bound_limit = (bound_limit >> 1);
		} else { /* memory pool is stresed,  come back later */
			return TRUE;
		}
	}
#else
	uint pktpool_len;

	/* always bounded for bmac */
	bound_limit = wlc_hw->wlc->pub->tunables->rxbnd;
	pktpool_len = (uint) (pktpool_avail(&pktpool_shared) & 0xFFFF);
	if (bound_limit > pktpool_len)
	{
		WL_TRACE(("%s: bound_limit = %d pktpool_len = %d\n",
		__FUNCTION__, bound_limit, pktpool_len));
		bound_limit = (pktpool_len - (pktpool_len >> 2));
	}
#endif /* BCMPKTPOOL */	
#endif /* defined(WLC_LOW_ONLY) */

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));
	/* gather received frames */
	while ((p = (PIO_ENAB(wlc_hw->wlc->pub) ?
		wlc_pio_rx(wlc_hw->pio[fifo]) : dma_rx(wlc_hw->di[fifo])))) {

		if (!tail)
			head = tail = p;
		else {
			PKTSETLINK(tail, p);
			tail = p;
		}

		/* !give others some time to run! */
		if (++n >= bound_limit)
			break;
	}

	/* get the TSF REG reading */
	wlc_bmac_read_tsf(wlc_hw, &tsf_l, &tsf_h);

	/* prefetch the headers */
	if (head != NULL)
		WLPREFHDRS(PKTDATA(wlc_hw->osh, head), PREFSZ);

	/* post more rbufs */
	if (!PIO_ENAB(wlc_hw->wlc->pub))
		dma_rxfill(wlc_hw->di[fifo]);

	/* process each frame */
	while ((p = head) != NULL) {
		head = PKTLINK(head);
		PKTSETLINK(p, NULL);

		/* prefetch the headers */
		if (head != NULL)
			WLPREFHDRS(PKTDATA(wlc_hw->osh, head), PREFSZ);

		/* record the tsf_l in wlc_rxd11hdr */
		wlc_rxhdr = (wlc_d11rxhdr_t *)PKTDATA(wlc_hw->osh, p);
		wlc_rxhdr->tsf_l = htol32(tsf_l);

		/* compute the RSSI from d11rxhdr and record it in wlc_rxd11hr */
		wlc_phy_rssi_compute(wlc_hw->band->pi, wlc_rxhdr);

		wlc_recv(wlc_hw->wlc, p);
	}

	wlc_hw->wlc->pub->processed += n;

	return (n >= bound_limit);
}

#ifdef WLP2P
/* low level p2p interrupt processing
 */
static void
wlc_p2p_bmac_int_proc(wlc_hw_info_t *wlc_hw)
{
	uint b, i;
	uint8 p2p_interrupts[M_P2P_BSS_MAX];
	uint32 p2p_shm_base = wlc_bmac_read_shm(wlc_hw, M_P2P_BLK_PTR) << 1;
	uint32 tsf_l, tsf_h;

	memset(p2p_interrupts, 0, sizeof(uint8) * M_P2P_BSS_MAX);

	/* collect and clear p2p interrupts */
	for (b = 0; b < M_P2P_BSS_MAX; b ++) {

		for (i = 0; i < M_P2P_I_BLK_SZ; i ++) {
			uint loc = p2p_shm_base + M_P2P_I(b, i);

			/* any P2P event/interrupt? */
			if (wlc_bmac_read_shm(wlc_hw, loc) == 0)
				continue;

			/* ACK */
			wlc_bmac_write_shm(wlc_hw, loc, 0);

			/* store */
			p2p_interrupts[b] |= (1 << i);
		}
	}

	wlc_bmac_read_tsf(wlc_hw, &tsf_l, &tsf_h);
	wlc_p2p_int_proc(wlc_hw->wlc, p2p_interrupts, tsf_l, tsf_h);
}
#endif /* WLP2P */

/* second-level interrupt processing
 *   Return TRUE if another dpc needs to be re-scheduled. FALSE otherwise.
 *   Param 'bounded' indicates if applicable loops should be bounded.
 */
bool BCMFASTPATH
wlc_dpc(wlc_info_t *wlc, bool bounded)
{
	uint32 macintstatus;
	wlc_hw_info_t *wlc_hw = wlc->hw;
	d11regs_t *regs = wlc_hw->regs;
	bool fatal = FALSE;
	static int phy_err_cnt = 0;

	wlc->pub->processed = 0;

#ifdef WLC_HIGH
	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return FALSE;
	}
#endif

	/* grab and clear the saved software intstatus bits */
	macintstatus = wlc->macintstatus;
	wlc->macintstatus = 0;

	WL_TRACE(("wl%d: wlc_dpc: macintstatus 0x%x\n", wlc_hw->unit, macintstatus));

#ifdef STA
	if (macintstatus & MI_BT_PRED_REQ)
		wlc_bmac_btc_update_predictor(wlc_hw);
#endif

#ifdef WLC_HIGH
	if (macintstatus & (MI_TBTT | MI_DTIM_TBTT)) {
#ifdef RADIO_PWRSAVE
		/* Enter power save mode */
		wlc_radio_pwrsave_enter_mode(wlc, ((macintstatus & MI_DTIM_TBTT) != 0));
#endif /* RADIO_PWRSAVE */
#ifdef MBSS
		if (MBSS_ENAB(wlc->pub)) {
			if (MBSS_ENAB16(wlc->pub))
				(void)wlc_ap_mbss16_tbtt(wlc, macintstatus);
			else
				(void)wlc_ap_mbss4_tbtt(wlc, macintstatus);
		}
#endif /* MBSS */
	}

	if (macintstatus & MI_PRQ) {
		/* Process probe request FIFO */
#ifdef MBSS
#ifdef WLC_HIGH
		/* Only used by the soft PRQ */
		ASSERT(MBSS_ENAB4(wlc->pub));
		if (!MBSS_ENAB16(wlc->pub) && wlc_prq_process(wlc, bounded)) {
			wlc->macintstatus |= MI_PRQ;
		}
#endif /* WLC_HIGH */
#else
		ASSERT(0 && "PRQ Interrupt in non-MBSS");
#endif /* MBSS */
	}

	/* BCN template is available */
	/* ZZZ: Use AP_ACTIVE ? */
	if (AP_ENAB(wlc->pub) && (!APSTA_ENAB(wlc->pub) || wlc->aps_associated) &&
	    (macintstatus & MI_BCNTPL)) {
		WL_APSTA_BCN(("wl%d: wlc_dpc: template -> wlc_update_beacon()\n",
			wlc_hw->unit));
		wlc_update_beacon(wlc);
	}
#endif /* WLC_HIGH */

	/* PMQ entry addition */
	if (macintstatus & MI_PMQ) {
#ifdef AP
		if (wlc_bmac_processpmq(wlc_hw, bounded))
			wlc->macintstatus |= MI_PMQ;
#endif
	}

	/* tx status */
	if (macintstatus & MI_TFS) {
		if (wlc_bmac_txstatus(wlc_hw, bounded, &fatal))
			wlc->macintstatus |= MI_TFS;
#ifdef WLC_HIGH
		if (fatal)
			goto fatal;
#endif
	}

#ifdef WLC_HIGH
	if (macintstatus & (MI_TBTT | MI_DTIM_TBTT))
		wlc_tbtt(wlc, regs);
#endif

	/* ATIM window end */
	if (macintstatus & MI_ATIMWINEND) {
		WL_TRACE(("wlc_isr: end of ATIM window\n"));

		OR_REG(wlc_hw->osh, &regs->maccommand, wlc->qvalid);
		wlc->qvalid = 0;
	}

	/* phy tx error */
	if (macintstatus & MI_PHYTXERR) {
		if (D11REV_IS(wlc_hw->corerev, 11) || D11REV_IS(wlc_hw->corerev, 12))
			WL_INFORM(("wl%d: PHYTX error, CNT = %d\n", wlc_hw->unit, wlc_hw->phyerr_cnt));
		else
			WL_ERROR(("wl%d: PHYTX error,  CNT = %d\n", wlc_hw->unit , wlc_hw->phyerr_cnt));
		WLCNTINCR(wlc->pub->_cnt->txphyerr);
		wlc_hw->phyerr_cnt++;
		phy_err_cnt++;
		
		if( phy_err_cnt >= 3 ) {
			WL_ERROR(("wl%d: HAMMERING info, CNT = %d\n", wlc_hw->unit , wlc_hw->phyerr_cnt ));
			wl_init(wlc->wl);
			phy_err_cnt = 0;
		}
	} else {
		wlc_hw->phyerr_cnt = 0;
	}
#ifdef WLC_HIGH
	if (WLCISSSLPNPHY(wlc_hw->band) && (wlc_hw->phyerr_cnt >= wlc_phytxerr_thresh)) {
		WL_ERROR(("wl%d: HAMMERING!\n", wlc_hw->unit));
		wl_init(wlc->wl);
	}
#endif /* WLC_HIGH */
	/* received data or control frame, MI_DMAINT is indication of RX_FIFO interrupt */
	if (macintstatus & MI_DMAINT) {
		if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
		    wlc_hw->dma_lpbk) {
			wlc_recover_pkts(wlc_hw->wlc, TX_DATA_FIFO);
			dma_rxfill(wlc_hw->di[RX_FIFO]);
		} else if (wlc_bmac_recv(wlc_hw, RX_FIFO, bounded)) {
			wlc->macintstatus |= MI_DMAINT;
		}
	}

	/* TX FIFO suspend/flush completion */
	if (macintstatus & MI_TXSTOP) {
		if (wlc_bmac_tx_fifo_suspended(wlc_hw, TX_DATA_FIFO)) {
#ifdef WLC_HIGH
			/* handle a scan request for tx suspend */
			wlc_scan_fifo_suspend_complete(wlc->scan);
#ifdef STA
#ifdef WLRM
			/* handle a radio measurement request for tx suspend */
			if (wlc->rm_info->rm_state->step == WLC_RM_WAIT_TX_SUSPEND)
				wl_add_timer(wlc->wl, wlc->rm_info->rm_timer, 0, 0);
#endif
			/* move to next quiet state if both DATA and CTL fifos are suspended */
			if ((QUIET_STATE(wlc) & WAITING_FOR_FLUSH_COMPLETE) &&
			    wlc_bmac_tx_fifo_suspended(wlc_hw, TX_CTL_FIFO)) {
				wlc->quiet->_state &= ~WAITING_FOR_FLUSH_COMPLETE;
				wlc_start_quiet2(wlc);
			}
#endif /* STA */
#endif /* WLC_HIGH */
		}
	}

	/* noise sample collected */
	if (macintstatus & MI_BG_NOISE) {
		WL_NONE(("wl%d: got background noise samples\n", wlc_hw->unit));
		wlc_phy_noise_sample_intr(wlc_hw->band->pi);
	}

#if defined(STA) && defined(WLRM)
	if (macintstatus & MI_CCA) {		/* CCA measurement complete */
		WL_INFORM(("wl%d: CCA measurement interrupt\n", wlc_hw->unit));
		wlc_bmac_rm_cca_int(wlc_hw);
	}
#endif

	if (macintstatus & MI_GP0) {
#ifdef BCMDBG
#ifdef WLC_HIGH
		/* BMAC_NOTE: maybe make this a WLC_HIGH_API ? */
		wlc_dump_ucode_fatal(wlc_hw->wlc);
#endif
#else
		WL_ERROR(("wl%d: PSM microcode watchdog fired at %d (seconds). Resetting.\n",
			wlc_hw->unit, wlc_hw->now));
#endif /* BCMDBG */

#ifdef WLC_HIGH
		if (!((CHIPID(wlc_hw->sih->chip) == BCM4321_CHIP_ID) &&
		      (CHIPREV(wlc_hw->sih->chiprev) == 0))) {
			WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
				0, R_REG(wlc_hw->osh, &regs->psmdebug), "psmdebug");
			WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
				0, R_REG(wlc_hw->osh, &regs->phydebug), "phydebug");
			WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
				0, R_REG(wlc_hw->osh, &regs->psm_brc), "psm_brc");

			ASSERT("PSM Watchdog" && 0);
		}

		WLCNTINCR(wlc->pub->_cnt->psmwds);

		/* big hammer */
		wl_init(wlc->wl);
#endif /* WLC_HIGH */
	}

#ifdef WLC_HIGH
	/* gptimer timeout */
	if (macintstatus & MI_TO) {
		wlc_hwtimer_isr(wlc->gptimer);
	}

#ifdef STA
	if (macintstatus & MI_RFDISABLE) {
		uint32 rfd = R_REG(wlc_hw->osh, &regs->phydebug) & PDBG_RFD;

		WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_RFDISABLE_ID, WL_LOG_LEVEL_ERR,
			0, rfd, NULL);

		WL_ERROR(("wl%d: MAC Detected a change on the RF Disable Input 0x%x\n",
			wlc_hw->unit, rfd));

		WLCNTINCR(wlc->pub->_cnt->rfdisable);
		/* delay the cleanup to wl_down in IBSS case */
		if (!LMAC_ENAB(wlc->pub) && rfd) {
			int idx;
			wlc_bsscfg_t *bsscfg;
			FOREACH_BSS(wlc, idx, bsscfg) {
				if (!BSSCFG_STA(bsscfg) || !bsscfg->BSS || !bsscfg->enable)
					continue;
				WL_APSTA_UPDN(("wl%d: wlc_dpc: rfdisable -> wlc_bsscfg_disable()\n",
					wlc_hw->unit));
				wlc_bsscfg_disable(wlc, bsscfg);
			}
		}
	}
#endif /* STA */
#endif /* WLC_HIGH */

#ifdef WLP2P
	if (P2P_ENAB_HW(wlc_hw) &&
	     (macintstatus & MI_P2P))
		wlc_p2p_bmac_int_proc(wlc_hw);
#endif

#ifdef WLC_HIGH
	/* send any enq'd tx packets. Just makes sure to jump start tx */
	if (!pktq_empty(&wlc->active_queue->q))
		wlc_send_q(wlc, wlc->active_queue);

	ASSERT(wlc_ps_check(wlc));
#endif	/* WLC_HIGH */

#ifdef WLC_LOW_ONLY

#define MACINTMASK_BMAC \
	(MI_TFS | MI_ATIMWINEND |  MI_PHYTXERR | MI_DMAINT | MI_BG_NOISE | MI_CCA | MI_GP0 | \
	 MI_PMQ | MI_P2P)

	macintstatus &= ~MACINTMASK_BMAC;
	if (macintstatus != 0) {
		wlc_high_dpc(wlc, macintstatus);
	}
#endif /* WLC_LOW_ONLY */

	/* make sure the bound indication and the implementation are in sync */
	ASSERT(bounded == TRUE || wlc->macintstatus == 0);

#if defined(BCMDBG) && !defined(WLC_LOW_ONLY)
	wlc_update_perf_stats(wlc, WLC_PERF_STATS_DPC);
#endif

	/* it isn't done and needs to be resched if macintstatus is non-zero */
	return (wlc->macintstatus != 0);

#ifdef WLC_HIGH
fatal:
	wl_init(wlc->wl);
	return (wlc->macintstatus != 0);
#endif
}

#ifdef AP	    /* PMQ stuff */

#if defined(WLC_LOW_ONLY)
/* tries to find an entry for ea. If none and "add" is true,
 * add and initialize the new
*/
static  bmac_pmq_entry_t *
wlc_bmac_pmq_find(wlc_hw_info_t *wlc_hw, struct ether_addr *ea)
{
	bmac_pmq_entry_t * res = wlc_hw->bmac_pmq->entry;

	/* There should not be THAT many stations in PS ON mode which needs
	   tx pending drain at the same time.
	   do a simple search for now. We will see if we need to optimize it later.
	*/
	if (res == NULL)
		return res;
	do {
		if (!memcmp(&res->ea, ea, sizeof(struct ether_addr))) {
			/* move the head to the entry we found. For high
			   traffic, this is the most likely to comme next.
			 */
			wlc_hw->bmac_pmq->entry = res;
			return res;
		}
		res = res->next;
	} while (res && res != wlc_hw->bmac_pmq->entry);

	/* If we are here, we didn't find it */
	ASSERT(res);

	return NULL;
}

static  bmac_pmq_entry_t *
wlc_bmac_pmq_add(wlc_hw_info_t *wlc_hw, struct ether_addr *ea)
{
	bmac_pmq_entry_t * res;

	res = (bmac_pmq_entry_t *)MALLOC(wlc_hw->osh, sizeof(bmac_pmq_t));
	if (!res) {
		WL_ERROR(("wl%d: BPMQ error. Out of memory !!\n",
		          wlc_hw->unit));
		return NULL;
	}
	if (! wlc_hw->bmac_pmq->entry) {
		wlc_hw->bmac_pmq->entry = res;
		res->next = res;
	}
	else {
		res->next = wlc_hw->bmac_pmq->entry->next;
		wlc_hw->bmac_pmq->entry->next = res;
		wlc_hw->bmac_pmq->entry = res;
	}
	memcpy(&res->ea, ea, sizeof(struct ether_addr));
	res->switches = 0;
	res->ps_on = 0;
	return res;
}

static void
wlc_bmac_pmq_remove(wlc_hw_info_t *wlc_hw, bmac_pmq_entry_t *pmq_entry)
{
	bmac_pmq_entry_t * res = wlc_hw->bmac_pmq->entry;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	/* There should not be THAT many stations in PS ON mode which needs
	   tx pending drain at the same time.
	   do a simple search for now. We will see if we need to optimize it later.
	*/

	ASSERT(res);

	while (res && res->next != pmq_entry && res->next != wlc_hw->bmac_pmq->entry) {
		res = res->next;
	}

	ASSERT(res->next == pmq_entry);
	/* last element */
	if (res == res->next)
		wlc_hw->bmac_pmq->entry = NULL;
	else {
		res->next = pmq_entry->next;
		if (pmq_entry == wlc_hw->bmac_pmq->entry)
			wlc_hw->bmac_pmq->entry = res;
	}

	WL_PS((" BPMQ removed %s from table \n", bcm_ether_ntoa(&pmq_entry->ea, eabuf)));
	MFREE(wlc_hw->osh, pmq_entry, sizeof(bmac_pmq_entry_t));

}

#endif /* WLC_LOW_ONLY */

/* called by high driver when it detects a switch which is normally already in the bmac queue.
 * For full driver, this function will be called directly as a result of a call to
 * wlc_apps_process_ps_switch,
 *  with no delay, before the end of the wlc_bmac_processpmq loop. No state is necessary.
 *  In case of bmac-high split, it will be called after the high received the rpc
 *  call to wlc_apps_process_ps_switch, AFTER
 *  the end of  the wlc_bmac_processpmq loop.
 *  We need to keep state of what has been received by the high driver.
 */

void BCMFASTPATH
wlc_bmac_process_ps_switch(wlc_hw_info_t *wlc_hw, struct ether_addr *ea, int8 ps_flags)
{
	/*
	   ps_on's highest bits are used like this :
	   - TX_FIFO_FLUSHED : there is no more packets pending
	   - MSG_MAC_INVALID  this is not really a switch, just a tx fifo
	   empty  indication. Mac address
	   is not present in the message.
	   - STA_REMOVED : the scb for this mac has been removed by the high driver or
	   is not associated.
	*/
#ifdef WLC_LOW_ONLY
	/*
	   for bmac-high split, manage a local pmq state to detect when the high driver
	   has received all relevant information and the ucode pmq can be cleared.
	   NOTE : it seems unlikely that we will ever get the PMQ full because the high
	   driver doesn't report switches fast enough to keep up with bmac.
	   However, if it happens, we would need to either stop reading the PMQ
	   waiting to be in sync, or have a bmac suppression level.
	   To keep in mind.
	*/
	bmac_pmq_entry_t *entry;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	if (!(ps_flags & MSG_MAC_INVALID)) {

		if ((entry = wlc_bmac_pmq_find(wlc_hw, ea))) {
			WL_PS((" HIGH %s switched to PS state %d switches %d active %d \n",
			       bcm_ether_ntoa(ea, eabuf), ps_flags,
			       entry->switches, wlc_hw->bmac_pmq->active_entries));

			/* if the STA is removed, remove forcefully and decrement active entries
			 * if switches are not already 0
			 */
			if (ps_flags & STA_REMOVED) {
				if (entry->switches)
					wlc_hw->bmac_pmq->active_entries--;
				wlc_bmac_pmq_remove(wlc_hw, entry);
			}
			else {
				/* if getting to 0, decrement the number of active entries.
				   Ideally, this should take the 0x80 bit into account for each
				   STA.
				*/
				entry->switches--;
				if (entry->switches == 0) {
					wlc_hw->bmac_pmq->active_entries--;
					/* if we are in ps off state, or the scb has been removed,
					   this entry can be re-used
					*/
					if ((!entry->ps_on)) {
						wlc_bmac_pmq_remove(wlc_hw, entry);
					}
				}
			}
		}
	}
#endif /* WLC_LOW_ONLY */

	/* no more packet pending and no more non-acked switches ... clear the PMQ */
#ifdef WLC_HIGH
	/* for high driver, only clear when coming from tx status  */
	if ((ps_flags & TX_FIFO_FLUSHED) && (ps_flags & MSG_MAC_INVALID))
#else
	if (ps_flags & TX_FIFO_FLUSHED)
#endif
	{
		wlc_hw->bmac_pmq->tx_draining = 0;
		if (wlc_hw->bmac_pmq->active_entries == 0 &&
		    wlc_hw->bmac_pmq->pmq_read_count) {
			WL_PS(("PS : clearing ucode PMQ \n"));
			wlc_bmac_clearpmq(wlc_hw);
		}
	}
}

static void
wlc_bmac_pmq_init(wlc_hw_info_t *wlc_hw)
{

	wlc_hw->bmac_pmq = (bmac_pmq_t *)MALLOC(wlc_hw->osh, sizeof(bmac_pmq_t));
	if (!wlc_hw->bmac_pmq) {
		WL_ERROR(("BPMQ error. Out of memory !!\n"));
		return;
	}
	memset(wlc_hw->bmac_pmq, 0, sizeof(bmac_pmq_t));
	wlc_hw->bmac_pmq->pmq_size = BMAC_PMQ_SIZE;
}

static void
wlc_bmac_pmq_delete(wlc_hw_info_t *wlc_hw)
{
#ifdef WLC_LOW_ONLY
	bmac_pmq_entry_t * entry;
	bmac_pmq_entry_t * nxt;
#endif
	if (!wlc_hw->bmac_pmq)
		return;
#ifdef WLC_LOW_ONLY
	entry = wlc_hw->bmac_pmq->entry;
	while (entry) {
		nxt = entry->next;
		MFREE(wlc_hw->osh, entry, sizeof(bmac_pmq_entry_t));
		if (nxt == wlc_hw->bmac_pmq->entry)
			break;
		entry = nxt;
	}
#endif
	MFREE(wlc_hw->osh, wlc_hw->bmac_pmq, sizeof(bmac_pmq_t));
	wlc_hw->bmac_pmq = NULL;
}


static bool BCMFASTPATH
wlc_bmac_processpmq(wlc_hw_info_t *wlc_hw, bool bounded)
{
	volatile uint32 *pmqhostdata;
	uint32 pmqdata;
	d11regs_t *regs = wlc_hw->regs;
	uint32 pat_hi, pat_lo;
	struct ether_addr eaddr;
	bmac_pmq_t *pmq = wlc_hw->bmac_pmq;
	int8 ps_on;
	bool pmq_need_resched = FALSE;

#ifdef WLC_LOW_ONLY
	uint8 new_switch = 0;
	bmac_pmq_entry_t *pmq_entry = NULL;
#endif /* WLC_LOW_ONLY */
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	pmqhostdata = (volatile uint32 *)&regs->pmqreg.pmqhostdata;

	/* read entries until empty or pmq exeeding limit */
	while ((pmqdata = R_REG(wlc_hw->osh, pmqhostdata)) & PMQH_NOT_EMPTY) {
		/* if we exceed the per invocation pmq entry processing limit,
		 * reschedule again (only if bounded) to process the remaining pmq entries at a
		 * later time.
		 */
		if (bounded &&
		    (pmq->pmq_read_count >= pmq->pmq_size - BMAC_PMQ_MIN_ROOM)) {
			pmq_need_resched = TRUE;
			break;
		}

		pat_lo = R_REG(wlc_hw->osh, &regs->pmqpatl);
		pat_hi = R_REG(wlc_hw->osh, &regs->pmqpath);
		eaddr.octet[5] = (pat_hi >> 8)  & 0xff;
		eaddr.octet[4] =  pat_hi	& 0xff;
		eaddr.octet[3] = (pat_lo >> 24) & 0xff;
		eaddr.octet[2] = (pat_lo >> 16) & 0xff;
		eaddr.octet[1] = (pat_lo >> 8)  & 0xff;
		eaddr.octet[0] =  pat_lo	& 0xff;

		pmq->pmq_read_count++;

		if (ETHER_ISMULTI(eaddr.octet)) {
			WL_ERROR(("wl%d: wlc_bmac_processpmq:"
				" skip entry with mc/bc address %s\n",
				wlc_hw->unit, bcm_ether_ntoa(&eaddr, eabuf)));
			continue;
		}

		ps_on = (pmqdata & PMQH_PMON) ? 1: 0;

#ifdef WLC_LOW_ONLY
		/* in case of a split driver, evaluate if this is a real switch
		   before calling the high
		*/
		new_switch = 0;
		if ((pmq_entry = wlc_bmac_pmq_find(wlc_hw, &eaddr))) {
			/* is it a switch ? */
			if (pmq_entry->ps_on == 1 - ps_on) {
				pmq_entry->switches++;
				if (pmq_entry->switches == 1)
					pmq->active_entries++;
				pmq_entry->ps_on = ps_on;
				new_switch = 1;
				WL_PS(("BPMQ : sta %s switched to PS %d\n",
				       bcm_ether_ntoa(&eaddr, eabuf), ps_on));
			}
		}
		/* add a new entry only if it is in the ON state */
		else if (ps_on) {
			if ((pmq_entry = wlc_bmac_pmq_add(wlc_hw, &eaddr))) {
				WL_PS(("BPMQ : sta %s added with PS %d\n",
				       bcm_ether_ntoa(&eaddr, eabuf), ps_on));
				pmq_entry->ps_on = ps_on;
				pmq_entry->switches = 1;
				pmq->active_entries++;
				new_switch = 1;
			}
		}
		if (new_switch) {
			if (ps_on)
				pmq->tx_draining = 1;
			wlc_apps_process_ps_switch(wlc_hw->wlc, &eaddr, ps_on);
		}
#else
		wlc_apps_process_ps_switch(wlc_hw->wlc, &eaddr, ps_on);
#endif /* WLC_LOW_ONLY */
	}

	if (!pmq->active_entries && !pmq->tx_draining) {
		wlc_bmac_clearpmq(wlc_hw);
	}

	return pmq_need_resched;
}


/* Read and drain all the PMQ entries while not EMPTY.
 * When PMQ handling is enabled (MCTL_DISCARD_PMQ in maccontrol is clear),
 * one PMQ entry per packet received from a STA is created with corresponding 'ea' as key.
 * AP reads the entry and handles the PowerSave mode transitions of a STA by
 * comparing the PMQ entry with current PS-state of the STA. If PMQ entry is same as the
 * driver state, it's ignored, else transition is handled.
 *
 * With MBSS code, ON PMQ entries are also added for BSS configs; they are
 * ignored by the SW.
 *
 * Note that PMQ entries remain in the queue for the ucode to search until
 * an explicit delete of the entries is done with PMQH_DEL_MULT (or DEL_ENTRY).
 */

static void
wlc_bmac_clearpmq(wlc_hw_info_t *wlc_hw)
{
	volatile uint16 *pmqctrlstatus;
	d11regs_t *regs = wlc_hw->regs;

	if (!wlc_hw->bmac_pmq->pmq_read_count)
		return;
	pmqctrlstatus = (volatile uint16 *)&regs->pmqreg.w.pmqctrlstatus;
	/* Clear the PMQ entry unless we are letting the data fifo drain
	 * when txstatus indicates unlocks the data fifo we clear
	 * the PMQ of any processed entries
	 */

	W_REG(wlc_hw->osh, pmqctrlstatus, PMQH_DEL_MULT);
	wlc_hw->bmac_pmq->pmq_read_count = 0;
}

#endif /* AP */


void
wlc_bmac_txfifo(wlc_hw_info_t *wlc_hw, uint fifo, void *p,
	bool commit, uint16 frameid, uint8 txpktpend)
{
	d11txh_t *txh;
#if SSLPNCONF && defined(WLC_LOW_ONLY)
	uint8 *plcp;
	uint16 phyctl;
	int16 mcs_adjustment;
	int8 mcs_pwr_this_rate, legacy_pwr_this_rate;
#endif

	txh = (d11txh_t *)PKTDATA(wlc_hw->osh, p);
#if SSLPNCONF && defined(WLC_LOW_ONLY)
	plcp = (uint8 *)(txh + 1);
	if (WLCISSSLPNPHY(wlc_hw->band)) {
		if (IS_SINGLE_STREAM(plcp[0] & MIMO_PLCP_MCS_MASK)) {
			legacy_pwr_this_rate =	wlc_phy_get_tx_power_offset(wlc_hw->band->pi,
				(MIMO_PLCP_MCS_MASK & plcp[0]));
			mcs_pwr_this_rate =  wlc_phy_get_tx_power_offset_by_mcs(wlc_hw->band->pi,
				(MIMO_PLCP_MCS_MASK & plcp[0]));

			mcs_adjustment = legacy_pwr_this_rate - mcs_pwr_this_rate;

			if (mcs_adjustment > 31)
				mcs_adjustment = 31;

			if (mcs_adjustment < -32)
				mcs_adjustment = -32;
			phyctl = htol16(txh->PhyTxControlWord);
			phyctl = (phyctl & ~0xfc00) | ((mcs_adjustment << 10) & 0xfc00);
			txh->PhyTxControlWord = htol16(phyctl);
		}
	}
#endif /* WLC_LOW_ONLY */

#if defined(DMA_TX_FREE)
	int is_ampdu_status = 0;
	int is_ampdu_intermediate = 0;

	if (WLPKTTAG(p)->flags & WLF_AMPDU_MPDU) {
		uint16 mcl;

		txh = (d11txh_t *)PKTDATA(wlc_hw->osh, p);
		mcl = ltoh16(txh->MacTxControlLow);
		mcl = (mcl & TXC_AMPDU_MASK);

		if (mcl == (TXC_AMPDU_LAST << TXC_AMPDU_SHIFT))
			is_ampdu_status = 1;
		else if (mcl != (TXC_AMPDU_NONE << TXC_AMPDU_SHIFT))
			is_ampdu_intermediate = 1;
	}

	/* if this is not an intermediate status that we toss,
	 * add an entry to the txstatus_ampdu_flags bit fifo
	 */
	if (!is_ampdu_intermediate) {
		uint head_idx;

		head_idx = wlc_hw->txstatus_ampdu_flags[fifo].head;

		/* set the bit depending on whether or not the status
		 * is a long AMPDU status or not.
		 */
		if (is_ampdu_status)
			setbit(wlc_hw->txstatus_ampdu_flags[fifo].vec, head_idx);
		else
			clrbit(wlc_hw->txstatus_ampdu_flags[fifo].vec, head_idx);

		wlc_hw->txstatus_ampdu_flags[fifo].head = (head_idx + 1) % WLC_TXSTATUS_SZ;
	}
#endif /* DMA_TX_FREE */

	/* bump up pending count */
	if (commit) {
		TXPKTPENDINC(wlc_hw->wlc, fifo, txpktpend);
		WL_TRACE(("wlc_bmac_txfifo, pktpend inc %d to %d\n", txpktpend,
			TXPKTPENDGET(wlc_hw->wlc, fifo)));
	}

	if (!PIO_ENAB(wlc_hw->wlc->pub)) {
		/* Commit BCMC sequence number in the SHM frame ID location */
		if (frameid != INVALIDFID) {
			wlc_bmac_write_shm(wlc_hw, M_BCMC_FID, frameid);
		}

		if (dma_txfast(wlc_hw->di[fifo], p, commit) < 0) {
			WL_ERROR(("wlc_bmac_txfifo: fatal, toss frames !!!\n"));
			if (commit)
				TXPKTPENDDEC(wlc_hw->wlc, fifo, txpktpend);
		}
	} else
		wlc_pio_tx(wlc_hw->pio[fifo], p);
}

#ifdef WLC_LOW_ONLY
static void
wlc_bmac_dngl_wd_upd(wlc_hw_info_t *wlc_hw)
{
	if (!wlc_hw->dngl_wd)
		return;

	if (wlc_hw->dngl_wd_exptime > 0) {
		si_watchdog_ms(wlc_hw->sih, 2000);
		--wlc_hw->dngl_wd_exptime;
	} else {
		/* force trigger */
		si_watchdog_ms(wlc_hw->sih, 1);
	}
}

int
wlc_bmac_dngl_wd_set(wlc_hw_info_t *wlc_hw, bool dngl_wd, uint32 dngl_wd_exptime)
{
	if (dngl_wd) {
		si_watchdog_ms(wlc_hw->sih, 2000);
		wlc_hw->dngl_wd = TRUE;
		wlc_hw->dngl_wd_exptime = dngl_wd_exptime;
	} else {
		wlc_hw->dngl_wd = FALSE;
		wlc_hw->dngl_wd_exptime = 0;
		si_watchdog_ms(wlc_hw->sih, 0);
	}

	return 0;
}
#endif /* WLC_LOW_ONLY */
/* common low-level watchdog code */
void
wlc_bmac_watchdog(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	WL_TRACE(("wl%d: wlc_bmac_watchdog\n", wlc_hw->unit));

	if (!wlc_hw->up)
		return;

	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));

	/* increment second count */
	wlc_hw->now++;
#ifdef WLC_LOW_ONLY
	wlc_bmac_dngl_wd_upd(wlc_hw);
#endif
	/* Check for FIFO error interrupts */
	wlc_bmac_fifoerrors(wlc_hw);

	/* make sure RX dma has buffers */
	if (!PIO_ENAB(wlc->pub)) {
		dma_rxfill(wlc_hw->di[RX_FIFO]);
		if (D11REV_IS(wlc_hw->corerev, 4)) {
			dma_rxfill(wlc_hw->di[RX_TXSTATUS_FIFO]);
		}
	}

	wlc_phy_watchdog(wlc_hw->band->pi);

	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));
}

/* bmac rpc agg watchdog code */
void
wlc_bmac_rpc_agg_watchdog(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	WL_TRACE(("wl%d: wlc_bmac_rpc_agg_watchdog\n", wlc_hw->unit));

	if (!wlc_hw->up)
		return;
#ifdef WLC_LOW_ONLY
	/* maintenance */
	wlc_bmac_rpc_watchdog(wlc);
#endif
}

void
wlc_bmac_set_chanspec(wlc_hw_info_t *wlc_hw, chanspec_t chanspec, bool mute, txppr_t *txpwr)
{
	bool fastclk;
	uint bandunit;

	WL_TRACE(("wl%d: wlc_bmac_set_chanspec 0x%x\n", wlc_hw->unit, chanspec));

	/* request FAST clock if not on */
	if (!(fastclk = wlc_hw->forcefastclk))
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

	wlc_hw->chanspec = chanspec;

	/* Switch bands if necessary */
	if (NBANDS_HW(wlc_hw) > 1) {
		bandunit = CHSPEC_WLCBANDUNIT(chanspec);
		if (wlc_hw->band->bandunit != bandunit) {
			/* wlc_bmac_setband disables other bandunit,
			 *  use light band switch if not up yet
			 */
			if (wlc_hw->up) {
				wlc_phy_chanspec_radio_set(wlc_hw->bandstate[bandunit]->pi,
					chanspec);
				wlc_bmac_setband(wlc_hw, bandunit, chanspec);
			} else {
				wlc_setxband(wlc_hw, bandunit);
			}
		}
	}

	wlc_phy_initcal_enable(wlc_hw->band->pi, !mute);

	if (!wlc_hw->up) {
		if (wlc_hw->clk)
			wlc_phy_txpower_limit_set(wlc_hw->band->pi, txpwr, chanspec);
		wlc_phy_chanspec_radio_set(wlc_hw->band->pi, chanspec);
	} else {
		/* Bandswitch above may end up changing the channel so avoid repetition */
		if (chanspec != wlc_phy_chanspec_get(wlc_hw->band->pi))
			wlc_phy_chanspec_set(wlc_hw->band->pi, chanspec);
		wlc_phy_txpower_limit_set(wlc_hw->band->pi, txpwr, chanspec);

		/* Update muting of the channel */
		wlc_bmac_mute(wlc_hw, mute, 0);
	}
	if (!fastclk)
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);
}

int
wlc_bmac_revinfo_get(wlc_hw_info_t *wlc_hw, wlc_bmac_revinfo_t *revinfo)
{
	si_t *sih = wlc_hw->sih;
	uint idx;

	revinfo->vendorid = wlc_hw->vendorid;
	revinfo->deviceid = wlc_hw->deviceid;

	revinfo->boardrev = wlc_hw->boardrev;
	revinfo->corerev = wlc_hw->corerev;
	revinfo->sromrev = wlc_hw->sromrev;
	revinfo->chiprev = sih->chiprev;
	revinfo->chip = sih->chip;
	revinfo->chippkg = sih->chippkg;
	revinfo->boardtype = sih->boardtype;
	revinfo->boardvendor = sih->boardvendor;
	revinfo->bustype = sih->bustype;
	revinfo->buscoretype = sih->buscoretype;
	revinfo->buscorerev = sih->buscorerev;
	revinfo->issim = sih->issim;

	revinfo->nbands = NBANDS_HW(wlc_hw);

	for (idx = 0; idx < NBANDS_HW(wlc_hw); idx++) {
		wlc_hwband_t *band;

		/* So if this is a single band 11a card, use band 1 */
		if (IS_SINGLEBAND_5G(wlc_hw->deviceid))
			idx = BAND_5G_INDEX;

		band = wlc_hw->bandstate[idx];
		revinfo->band[idx].bandunit = band->bandunit;
		revinfo->band[idx].bandtype = band->bandtype;
		revinfo->band[idx].phytype = band->phytype;
		revinfo->band[idx].phyrev = band->phyrev;
		revinfo->band[idx].radioid = band->radioid;
		revinfo->band[idx].radiorev = band->radiorev;
		revinfo->band[idx].abgphy_encore = band->abgphy_encore;
		revinfo->band[idx].anarev = 0;

	}
	return 0;
}

int
wlc_bmac_state_get(wlc_hw_info_t *wlc_hw, wlc_bmac_state_t *state)
{
	state->machwcap = wlc_hw->machwcap;
	state->preamble_ovr = (uint32)wlc_phy_preamble_override_get(wlc_hw->band->pi);

	return 0;
}

#ifdef BCMDBG_POOL
static void
wlc_pktpool_dbg_cb(pktpool_t *pool, void *arg)
{
	wlc_hw_info_t *wlc_hw = arg;
	pktpool_stats_t pstats;

	if (wlc_hw == NULL)
		return;

	if (wlc_hw->up == FALSE)
		return;

	WL_ERROR(("wl: post=%d rxactive=%d txactive=%d txpend=%d\n",
		NRXBUFPOST,
		dma_rxactive(wlc_hw->di[RX_FIFO]),
		dma_txactive(wlc_hw->di[1]),
		dma_txpending(wlc_hw->di[1])));

	pktpool_stats_dump(pool, &pstats);
	WL_ERROR(("pool len=%d\n", pktpool_len(pool)));
	WL_ERROR(("txdh:%d txd11:%d enq:%d rxdh:%d rxd11:%d rxfill:%d idle:%d\n",
		pstats.txdh, pstats.txd11, pstats.enq,
		pstats.rxdh, pstats.rxd11, pstats.rxfill, pstats.idle));
}
#endif /* BCMDBG_POOL */

static void
wlc_pktpool_avail_cb(pktpool_t *pool, void *arg)
{
	wlc_hw_info_t *wlc_hw = arg;

	if (wlc_hw == NULL)
		return;

	if (wlc_hw->up == FALSE)
		return;

	if (!PIO_ENAB(wlc_hw->wlc->pub) && wlc_hw->di[RX_FIFO])
		dma_rxfill(wlc_hw->di[RX_FIFO]);
}

static bool
BCMATTACHFN(wlc_bmac_attach_dmapio)(wlc_hw_info_t *wlc_hw, uint j, bool wme)
{
	uint i;
	char name[8];
	/* ucode host flag 2 needed for pio mode, independent of band and fifo */
	uint16 pio_mhf2 = 0;
	wlc_info_t *wlc = wlc_hw->wlc;
	uint unit = wlc_hw->unit;
	wlc_tunables_t *tune = wlc->pub->tunables;

	/* name and offsets for dma_attach */
	snprintf(name, sizeof(name), "wl%d", unit);

	/* init core's pio or dma channels */
	if (PIO_ENAB(wlc->pub)) {
		if (wlc_hw->pio[0] == 0) {
			for (i = 0; i < NFIFO; i++) {
				wlc_hw->pio[i] = wlc_pio_attach(wlc->pub, wlc, i, &pio_mhf2);
				if (wlc_hw->pio[i] == NULL) {
					WL_ERROR(("wlc_attach: pio_attach failed\n"));
					return FALSE;
				}
			}
		}
	} else if (wlc_hw->di[0] == 0) {	/* Init FIFOs */
		uint addrwidth;
		int dma_attach_err = 0;
		osl_t *osh = wlc_hw->osh;

		/* Find out the DMA addressing capability and let OS know
		 * All the channels within one DMA core have 'common-minimum' same
		 * capability
		 */
		addrwidth = dma_addrwidth(wlc_hw->sih, DMAREG(wlc_hw, DMA_TX, 0));
		OSL_DMADDRWIDTH(osh, addrwidth);

		if (!wl_alloc_dma_resources(wlc_hw->wlc->wl, addrwidth)) {
			WL_ERROR(("wl%d: wlc_attach: alloc_dma_resources failed\n", unit));
			return FALSE;
		}

		/*
		 * FIFO 0
		 * TX: TX_AC_BK_FIFO (TX AC Background data packets)
		 * RX: RX_FIFO (RX data packets)
		 */
		ASSERT(TX_AC_BK_FIFO == 0);
		ASSERT(RX_FIFO == 0);
		wlc_hw->di[0] = dma_attach(osh, name, wlc_hw->sih,
			(wme ? DMAREG(wlc_hw, DMA_TX, 0) : NULL), DMAREG(wlc_hw, DMA_RX, 0),
			(wme ? tune->ntxd : 0), tune->nrxd, tune->rxbufsz, -1, tune->nrxbufpost,
			wlc_hw->wlc->hwrxoff, &wl_msg_level);
		dma_attach_err |= (NULL == wlc_hw->di[0]);

#if defined(WLC_LOW_ONLY)
		/* calculate memory required to replenish  rx buffer */
		wlc_hw->mem_required_def = (wlc_hw->wlc->pub->tunables->rxbufsz + BCMEXTRAHDROOM) *
			(wlc_hw->wlc->pub->tunables->rxbnd);
		wlc_hw->mem_required_lower = wlc_hw->mem_required_def -
			(wlc_hw->mem_required_def >> 2) +
			+ wlc_hw->wlc->pub->tunables->dngl_mem_restrict_rxdma;
		wlc_hw->mem_required_least = (wlc_hw->mem_required_def >> 1) +
			wlc_hw->wlc->pub->tunables->dngl_mem_restrict_rxdma;
		wlc_hw->mem_required_def += wlc_hw->wlc->pub->tunables->dngl_mem_restrict_rxdma;
#endif /* defined(WLC_LOW_ONLY) */


		/*
		 * FIFO 1
		 * TX: TX_AC_BE_FIFO (TX AC Best-Effort data packets)
		 *   (legacy) TX_DATA_FIFO (TX data packets)
		 * RX: UNUSED
		 */
		ASSERT(TX_AC_BE_FIFO == 1);
		ASSERT(TX_DATA_FIFO == 1);
		wlc_hw->di[1] = dma_attach(osh, name, wlc_hw->sih,
			DMAREG(wlc_hw, DMA_TX, 1), NULL, tune->ntxd, 0, 0, -1, 0, 0, &wl_msg_level);
		dma_attach_err |= (NULL == wlc_hw->di[1]);

#ifdef WME
		/*
		 * FIFO 2
		 * TX: TX_AC_VI_FIFO (TX AC Video data packets)
		 * RX: UNUSED
		 */
		ASSERT(TX_AC_VI_FIFO == 2);
		wlc_hw->di[2] = dma_attach(osh, name, wlc_hw->sih,
			DMAREG(wlc_hw, DMA_TX, 2), NULL, tune->ntxd, 0, 0, -1, 0, 0, &wl_msg_level);
		dma_attach_err |= (NULL == wlc_hw->di[2]);
#endif /* WME */
		/*
		 * FIFO 3
		 * TX: TX_AC_VO_FIFO (TX AC Voice data packets)
		 *   (legacy) TX_CTL_FIFO (TX control & mgmt packets)
		 * RX: RX_TXSTATUS_FIFO (transmit-status packets)
		 *	for corerev < 5 only
		 */
		ASSERT(TX_AC_VO_FIFO == 3);
		ASSERT(TX_CTL_FIFO == 3);
		if (D11REV_IS(wlc_hw->corerev, 4)) {
			ASSERT(RX_TXSTATUS_FIFO == 3);
			wlc_hw->di[3] = dma_attach(osh, name, wlc_hw->sih,
				DMAREG(wlc_hw, DMA_TX, 3), DMAREG(wlc_hw, DMA_RX, 3), tune->ntxd,
				tune->nrxd, sizeof(tx_status_t), -1, tune->nrxbufpost, 0,
				&wl_msg_level);
			dma_attach_err |= (NULL == wlc_hw->di[3]);
		} else {
			wlc_hw->di[3] = dma_attach(osh, name, wlc_hw->sih,
				DMAREG(wlc_hw, DMA_TX, 3), NULL, tune->ntxd, 0, 0, -1, 0, 0,
				&wl_msg_level);
			dma_attach_err |= (NULL == wlc_hw->di[3]);
		}
/* Cleaner to leave this as if with AP defined */
#ifdef AP
		/*
		 * FIFO 4
		 * TX: TX_BCMC_FIFO (TX broadcast & multicast packets)
		 * RX: UNUSED
		 */

		ASSERT(TX_BCMC_FIFO == 4);
		wlc_hw->di[4] = dma_attach(osh, name, wlc_hw->sih,
			DMAREG(wlc_hw, DMA_TX, 4), NULL, tune->ntxd, 0, 0, -1, 0, 0, &wl_msg_level);
		dma_attach_err |= (NULL == wlc_hw->di[4]);
#endif /* AP */

#if defined(MBSS)
		/*
		 * FIFO 5: TX_ATIM_FIFO
		 * TX: MBSS: but used for beacon/probe resp pkts
		 * TX: WLNINTENDO: TX Nintendo Nitro protocol packets
		 * RX: UNUSED
		 */
		wlc_hw->di[5] = dma_attach(osh, name, wlc_hw->sih,
			DMAREG(wlc_hw, DMA_TX, 5), NULL, tune->ntxd, 0, 0, -1, 0, 0, &wl_msg_level);
		dma_attach_err |= (NULL == wlc_hw->di[5]);
#endif 

		if (dma_attach_err) {
			WL_ERROR(("wl%d: wlc_attach: dma_attach failed\n", unit));
			return FALSE;
		}

		/* get pointer to dma engine tx flow control variable */
		for (i = 0; i < NFIFO; i++)
			if (wlc_hw->di[i])
				wlc_hw->txavail[i] =
					(uint*)dma_getvar(wlc_hw->di[i], "&txavail");
	}

	/* initial ucode host flags */
	wlc_mhfdef(wlc_hw, wlc_hw->band->mhfs, pio_mhf2);

	return TRUE;
}

static void
BCMATTACHFN(wlc_bmac_detach_dmapio)(wlc_hw_info_t *wlc_hw)
{
	uint j;

	for (j = 0; j < NFIFO; j++) {
		if (!PIO_ENAB(wlc_hw->wlc->pub)) {
			if (wlc_hw->di[j]) {
				dma_detach(wlc_hw->di[j]);
				wlc_hw->di[j] = NULL;
			}
		} else {
			if (wlc_hw->pio[j]) {
				wlc_pio_detach(wlc_hw->pio[j]);
				wlc_hw->pio[j] = NULL;
			}
		}
	}
}

static uint
wlc_bmac_dma_avoidance_cnt(wlc_hw_info_t *wlc_hw)
{
	uint i, total = 0;
	/* get total DMA avoidance counts */
	for (i = 0; i < NFIFO; i++)
		if (wlc_hw->di[i])
			total += dma_avoidance_cnt(wlc_hw->di[i]);

	return (total);
}

#ifdef WLC_LOW_ONLY
bool
wlc_chipmatch(uint16 vendor, uint16 device)
{
	if (vendor != VENDOR_BROADCOM) {
		WL_ERROR(("wlc_chipmatch: unknown vendor id %04x\n", vendor));
		return (FALSE);
	}

	if (device == BCM4306_D11G_ID)
		return (TRUE);
	if (device == BCM4306_D11G_ID2)
		return (TRUE);
	if (device == BCM4303_D11B_ID)
		return (TRUE);
	if (device == BCM4306_D11A_ID)
		return (TRUE);
	if (device == BCM4306_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4318_D11G_ID)
		return (TRUE);
	if (device == BCM4318_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4318_D11A_ID)
		return (TRUE);
	if (device == BCM4311_D11G_ID)
		return (TRUE);
	if (device == BCM4311_D11A_ID)
		return (TRUE);
	if (device == BCM4311_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4328_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4328_D11G_ID)
		return (TRUE);
	if (device == BCM4328_D11A_ID)
		return (TRUE);
	if (device == BCM4325_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4325_D11G_ID)
		return (TRUE);
	if (device == BCM4325_D11A_ID)
		return (TRUE);
	if ((device == BCM4321_D11N_ID) || (device == BCM4321_D11N2G_ID) ||
		(device == BCM4321_D11N5G_ID))
		return (TRUE);
	if ((device == BCM4322_D11N_ID) || (device == BCM4322_D11N2G_ID) ||
		(device == BCM4322_D11N5G_ID))
		return (TRUE);
	if (device == BCM4322_CHIP_ID)	/* 4322/43221 without OTP/SROM */
		return (TRUE);
	if (device == BCM43221_D11N2G_ID)
		return (TRUE);
	if (device == BCM43231_D11N2G_ID)
		return (TRUE);
	if ((device == BCM43222_D11N_ID) || (device == BCM43222_D11N2G_ID) ||
		(device == BCM43222_D11N5G_ID))
		return (TRUE);
	if ((device == BCM43224_D11N_ID) || (device == BCM43225_D11N2G_ID) ||
		(device == BCM43421_D11N_ID) || (device == BCM43224_D11N_ID_VEN1))
		return (TRUE);
	if ((device == BCM43226_D11N_ID))
		return (TRUE);
	if ((device == BCM6362_D11N_ID))
		return (TRUE);

	if (device == BCM4329_D11N2G_ID)
		return (TRUE);
	if (device == BCM4315_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4315_D11G_ID)
		return (TRUE);
	if (device == BCM4315_D11A_ID)
		return (TRUE);
	if ((device == BCM4319_D11N_ID) || (device == BCM4319_D11N2G_ID) ||
		(device == BCM4319_D11N5G_ID))
		return (TRUE);
	if ((device == BCM4716_CHIP_ID) || (device == BCM4748_CHIP_ID))
		return (TRUE);
	if (device == BCM4313_D11N2G_ID)
		return (TRUE);
	if (device == BCM4336_D11N_ID)
		return (TRUE);
	if (device == BCM4330_D11N_ID)
		return (TRUE);
	if ((device == BCM43236_D11N_ID) || (device == BCM43236_D11N2G_ID) ||
		(device == BCM43236_D11N5G_ID))
		return (TRUE);
	if ((device == BCM4331_D11N_ID) || (device == BCM4331_D11N2G_ID) ||
		(device == BCM4331_D11N5G_ID))
		return (TRUE);
	if (device == BCM43131_D11N2G_ID)
		return (TRUE);
	if ((device == BCM43227_D11N2G_ID) || (device == BCM43228_D11N_ID) ||
		(device == BCM43228_D11N5G_ID) || (device == BCM43217_D11N2G_ID))
		return (TRUE);
	if ((device == BCM43237_D11N_ID) || (device == BCM43237_D11N5G_ID))
		return (TRUE);

	WL_ERROR(("wlc_chipmatch: unknown device id %04x\n", device));
	return (FALSE);
}
#endif /* WLC_LOW_ONLY */

void
wlc_bmac_ampdu_set(wlc_hw_info_t *wlc_hw, uint8 mode)
{
	if ((D11REV_IS(wlc_hw->corerev, 26) || D11REV_IS(wlc_hw->corerev, 29))) {
		if (mode == AMPDU_AGG_HW)
			memcpy(wlc_hw->xmtfifo_sz, xmtfifo_sz_hwagg, sizeof(xmtfifo_sz_hwagg));
		else
			memcpy(wlc_hw->xmtfifo_sz, xmtfifo_sz_hostagg, sizeof(xmtfifo_sz_hostagg));
	}
}

#if defined(AP) && defined(WLC_LOW_ONLY)
static void
wlc_bmac_pa_war_set(wlc_hw_info_t *wlc_hw, bool enable)
{
	int polarity = 0;
	int pa_gpio_pin = GPIO_PIN_NOTDEFINED;
	static bool war_on = 0;

	if (enable && war_on) {
		return;
	}
	war_on = enable;

	if ((pa_gpio_pin = getgpiopin(NULL, "pa_mask_low", GPIO_PIN_NOTDEFINED)) !=
	    GPIO_PIN_NOTDEFINED) {
		polarity = enable ? 0 : 1;
	} else if ((pa_gpio_pin = getgpiopin(NULL, "pa_mask_high", GPIO_PIN_NOTDEFINED)) !=
		GPIO_PIN_NOTDEFINED) {
		polarity = enable ? 1 : 0;
	}

	if (pa_gpio_pin != GPIO_PIN_NOTDEFINED) {
		int pa_enable = 1 << pa_gpio_pin;

		if (wlc_hw->boardflags2 & (BFL2_ANAPACTRL_2G | BFL2_ANAPACTRL_5G)) {
			/* External PA is controlled using ANALOG PA ctrl lines.
			 * Enable PARLDO_pwrup (bit 12).
			 */
			si_pmu_regcontrol(wlc_hw->sih, 0, 0x3000, 0x1000);
		} else {
			/* External PA is controlled using DIGITAL PA ctrl lines */
			si_corereg(wlc_hw->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, chipcontrol), 0x44, 0x44);
		}
		si_gpioout(wlc_hw->sih, pa_enable, polarity, GPIO_DRV_PRIORITY);
		si_gpioouten(wlc_hw->sih, pa_enable, pa_enable, GPIO_DRV_PRIORITY);
	}
}
#endif /* AP && WLC_LOW_ONLY */

/* low level attach
 *    run backplane attach, init nvram
 *    run phy attach
 *    initialize software state for each core and band
 *    put the whole chip in reset(driver down state), no clock
 */
int
BCMATTACHFN(wlc_bmac_attach)(wlc_hw_info_t *wlc_hw, uint16 vendor, uint16 device, uint unit,
	bool piomode, osl_t *osh, void *regsva, uint bustype, void *btparam)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	d11regs_t *regs;
	char *macaddr = NULL;
	char *vars;
	uint err = 0;
	uint j;
	bool wme = FALSE;
	shared_phy_params_t sha_params;

	WL_TRACE(("wl%d: wlc_bmac_attach: vendor 0x%x device 0x%x\n", unit, vendor, device));

	ASSERT(sizeof(wlc_d11rxhdr_t) <= WL_HWRXOFF);

#ifdef WME
	wme = TRUE;
#endif /* WME */

#ifdef WLC_SPLIT
	wlc->rpc = btparam;
	wlc->hw->rpc = btparam;
#endif
	wlc_hw = wlc->hw;
	wlc_hw->wlc = wlc;
	wlc_hw->unit = unit;
	wlc_hw->osh = osh;
	wlc_hw->_piomode = piomode;

	/* populate wlc_hw_info_t with default values  */
	wlc_bmac_info_init(wlc_hw);

	/*
	 * Do the hardware portion of the attach.
	 * Also initialize software state that depends on the particular hardware
	 * we are running.
	 */
	wlc_hw->sih = si_attach((uint)device, osh, regsva, bustype, btparam,
	                        &wlc_hw->vars, &wlc_hw->vars_size);
	if (wlc_hw->sih == NULL) {
		WL_ERROR(("wl%d: wlc_bmac_attach: si_attach failed\n", unit));
		err = 11;
		goto fail;
	}
	vars = wlc_hw->vars;


	/*
	 * Get vendid/devid nvram overwrites, which could be different
	 * than those the BIOS recognizes for devices on PCMCIA_BUS,
	 * SDIO_BUS, and SROMless devices on PCI_BUS.
	 */
#ifdef BCMBUSTYPE
	bustype = BCMBUSTYPE;
#endif
	if (bustype != SI_BUS)
	{
		char *var;

		if ((var = getvar(vars, "vendid"))) {
			vendor = (uint16)bcm_strtoul(var, NULL, 0);
			WL_ERROR(("Overriding vendor id = 0x%x\n", vendor));
		}
		if ((var = getvar(vars, "devid"))) {
			uint16 devid = (uint16)bcm_strtoul(var, NULL, 0);
			if (devid != 0xffff) {
				WL_ERROR(("Overriding device id = 0x%d from %x\n", devid, device));
				device = devid;
				WL_ERROR(("Overriding device id = 0x%x\n", device));
			}
		}


		/* verify again the device is supported */
		if (!wlc_chipmatch(vendor, device)) {
			WL_ERROR(("wl%d: wlc_bmac_attach: Unsupported vendor/device (0x%x/0x%x)\n",
				unit, vendor, device));
			err = 12;
			goto fail;
		}
	}

	wlc_hw->vendorid = vendor;
	wlc_hw->deviceid = device;

	wlc_hw->band = wlc_hw->bandstate[IS_SINGLEBAND_5G(wlc_hw->deviceid) ?
		BAND_5G_INDEX : BAND_2G_INDEX];
	wlc->band = wlc->bandstate[IS_SINGLEBAND_5G(wlc_hw->deviceid) ?
		BAND_5G_INDEX : BAND_2G_INDEX];

	/* set bar0 window to point at D11 core */
	wlc_hw->regs = (d11regs_t *)si_setcore(wlc_hw->sih, D11_CORE_ID, 0);
	wlc_hw->corerev = si_corerev(wlc_hw->sih);

	regs = wlc_hw->regs;

	wlc->regs = wlc_hw->regs;

	/* validate chip, chiprev and corerev */
	if (!wlc_isgoodchip(wlc_hw)) {
		err = 13;
		goto fail;
	}

	/* initialize power control registers */
	si_clkctl_init(wlc_hw->sih);

	/* request fastclock and force fastclock for the rest of attach
	 * bring the d11 core out of reset.
	 *   For PMU chips, the first wlc_clkctl_clk is no-op since core-clk is still FALSE;
	 *   But it will be called again inside wlc_corereset, after d11 is out of reset.
	 */
	wlc_clkctl_clk(wlc_hw, CLK_FAST);
	wlc_bmac_corereset(wlc_hw, WLC_USE_COREFLAGS);

	if (!wlc_bmac_validate_chip_access(wlc_hw)) {
		WL_ERROR(("wl%d: wlc_bmac_attach: validate_chip_access failed\n", unit));
		err = 14;
		goto fail;
	}

	/* get the board rev, used just below */
	j = getintvar(vars, "boardrev");
	/* promote srom boardrev of 0xFF to 1 */
	if (j == BOARDREV_PROMOTABLE)
		j = BOARDREV_PROMOTED;
	wlc_hw->boardrev = (uint16)j;
	if (!wlc_validboardtype(wlc_hw)) {
		WL_ERROR(("wl%d: wlc_bmac_attach: Unsupported Broadcom board type (0x%x)"
			" or revision level (0x%x)\n",
			unit, wlc_hw->sih->boardtype, wlc_hw->boardrev));
		err = 15;
		goto fail;
	}
	wlc_hw->sromrev = (uint8)getintvar(vars, "sromrev");
	/* srom9 introduced ppr, which requires corerev >= 24 */
	if (wlc_hw->sromrev >= 9) {
		WL_ERROR(("wlc_bmac_attach: srom9 ppr requires corerev >=24"));
		ASSERT(D11REV_GE(wlc_hw->corerev, 24));
	}
	wlc_hw->boardflags = (uint32)getintvar(vars, "boardflags");
	wlc_hw->boardflags2 = (uint32)getintvar(vars, "boardflags2");

	/* some branded-boards boardflags srom programmed incorrectly */
	if ((wlc_hw->sih->boardvendor == VENDOR_APPLE) &&
		(wlc_hw->sih->boardtype == 0x4e) && (wlc_hw->boardrev >= 0x41))
		wlc_hw->boardflags |= BFL_PACTRL;

	if (D11REV_LE(wlc_hw->corerev, 4) || (wlc_hw->boardflags & BFL_NOPLLDOWN))
		wlc_bmac_pllreq(wlc_hw, TRUE, WLC_PLLREQ_SHARED);

	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) && (si_pci_war16165(wlc_hw->sih)))
		wlc->war16165 = TRUE;

#if defined(DBAND)
	/* check device id(srom, nvram etc.) to set bands */
	if ((wlc_hw->deviceid == BCM4306_D11DUAL_ID) ||
	    (wlc_hw->deviceid == BCM4318_D11DUAL_ID) ||
	    (wlc_hw->deviceid == BCM4311_D11DUAL_ID) ||
	    (wlc_hw->deviceid == BCM4321_D11N_ID) ||
	    (wlc_hw->deviceid == BCM4322_D11N_ID) ||
	    (wlc_hw->deviceid == BCM4328_D11DUAL_ID) ||
	    (wlc_hw->deviceid == BCM4325_D11DUAL_ID) ||
	    (wlc_hw->deviceid == BCM43222_D11N_ID) ||
	    (wlc_hw->deviceid == BCM43224_D11N_ID) ||
	    (wlc_hw->deviceid == BCM43224_D11N_ID_VEN1) ||
	    (wlc_hw->deviceid == BCM43421_D11N_ID) ||
	    (wlc_hw->deviceid == BCM43226_D11N_ID) ||
	    (wlc_hw->deviceid == BCM43236_D11N_ID) ||
	    (wlc_hw->deviceid == BCM6362_D11N_ID) ||
	    (wlc_hw->deviceid == BCM4331_D11N_ID) ||
	    (wlc_hw->deviceid == BCM43237_D11N_ID) ||
	    (wlc_hw->deviceid == BCM4315_D11DUAL_ID) ||
	    (wlc_hw->deviceid == BCM43228_D11N_ID) ||
	    (wlc_hw->deviceid == BCM4319_D11N_ID) ||
	    (wlc_hw->deviceid == BCM4319_D11N5G_ID) ||
	    0) {
		/* Dualband boards */
		wlc_hw->_nbands = 2;
	} else
#endif /* DBAND */
		wlc_hw->_nbands = 1;

#if NCONF
	if ((wlc_hw->sih->chip == BCM43221_CHIP_ID) ||
	    (wlc_hw->sih->chip == BCM43231_CHIP_ID) ||
	    (wlc_hw->sih->chip == BCM43225_CHIP_ID) ||
	    (wlc_hw->sih->chip == BCM43131_CHIP_ID) ||
	    (wlc_hw->sih->chip == BCM43217_CHIP_ID) ||
	    (wlc_hw->sih->chip == BCM43227_CHIP_ID)) {
		wlc_hw->_nbands = 1;
	}
#endif /* NCONF */

#ifdef WLC_HIGH
	/* BMAC_NOTE: remove init of pub values when wlc_attach() unconditionally does the
	 * init of these values
	 */
	wlc->vendorid = wlc_hw->vendorid;
	wlc->deviceid = wlc_hw->deviceid;
	wlc->pub->sih = wlc_hw->sih;
	wlc->pub->corerev = wlc_hw->corerev;
	wlc->pub->sromrev = wlc_hw->sromrev;
	wlc->pub->boardrev = wlc_hw->boardrev;
	wlc->pub->boardflags = wlc_hw->boardflags;
	wlc->pub->boardflags2 = wlc_hw->boardflags2;
	wlc->pub->_nbands = wlc_hw->_nbands;
#endif

	WL_ERROR(("wlc_bmac_attach, deviceid 0x%x nbands %d\n", wlc_hw->deviceid, wlc_hw->_nbands));

	wlc_hw->physhim = wlc_phy_shim_attach(wlc_hw, wlc->wl, wlc);

	if (wlc_hw->physhim == NULL) {
		WL_ERROR(("wl%d: wlc_bmac_attach: wlc_phy_shim_attach failed\n", unit));
		err = 25;
		goto fail;
	}

	/* pass all the parameters to wlc_phy_shared_attach in one struct */
	sha_params.osh = osh;
	sha_params.sih = wlc_hw->sih;
	sha_params.physhim = wlc_hw->physhim;
	sha_params.unit = unit;
	sha_params.corerev = wlc_hw->corerev;
	sha_params.vars = vars;
	sha_params.vid = wlc_hw->vendorid;
	sha_params.did = wlc_hw->deviceid;
	sha_params.chip = wlc_hw->sih->chip;
	sha_params.chiprev = wlc_hw->sih->chiprev;
	sha_params.chippkg = wlc_hw->sih->chippkg;
	sha_params.sromrev = wlc_hw->sromrev;
	sha_params.boardtype = wlc_hw->sih->boardtype;
	sha_params.boardrev = wlc_hw->boardrev;
	sha_params.boardvendor = wlc_hw->sih->boardvendor;
	sha_params.boardflags = wlc_hw->boardflags;
	sha_params.boardflags2 = wlc_hw->boardflags2;
	sha_params.bustype = wlc_hw->sih->bustype;
	sha_params.buscorerev = wlc_hw->sih->buscorerev;

	/* alloc and save pointer to shared phy state area */
	wlc_hw->phy_sh = wlc_phy_shared_attach(&sha_params);
	if (!wlc_hw->phy_sh) {
		err = 16;
		goto fail;
	}

	/* initialize software state for each core and band */
	for (j = 0; j < NBANDS_HW(wlc_hw); j++) {
		/*
		 * band0 is always 2.4Ghz
		 * band1, if present, is 5Ghz
		 */

		/* So if this is a single band 11a card, use band 1 */
		if (IS_SINGLEBAND_5G(wlc_hw->deviceid))
			j = BAND_5G_INDEX;

		wlc_setxband(wlc_hw, j);

		wlc_hw->band->bandunit = j;
		wlc_hw->band->bandtype = j ? WLC_BAND_5G : WLC_BAND_2G;
		wlc->band->bandunit = j;
		wlc->band->bandtype = j ? WLC_BAND_5G : WLC_BAND_2G;
		wlc->core->coreidx = si_coreidx(wlc_hw->sih);

		if (D11REV_GE(wlc_hw->corerev, 13)) {
			wlc_hw->machwcap = R_REG(wlc_hw->osh, &regs->machwcap);
			if ((D11REV_IS(wlc_hw->corerev, 26) && wlc_hw->sih->chiprev == 0) ||
			    (D11REV_IS(wlc_hw->corerev, 29)) || (D11REV_IS(wlc_hw->corerev, 31)))
				wlc_hw->machwcap &= ~MCAP_TKIPMIC;
			wlc_hw->machwcap_backup = wlc_hw->machwcap;
		}

		/* init tx fifo size */
		ASSERT((wlc_hw->corerev - XMTFIFOTBL_STARTREV) < ARRAYSIZE(xmtfifo_sz));
		wlc_hw->xmtfifo_sz = xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)];

		wlc_bmac_ampdu_set(wlc_hw, AMPDU_AGGMODE_HOST);

		/* Get a phy for this band */
		WL_NONE(("wl%d: %s: bandunit %d bandtype %d coreidx %d\n", unit,
		         __FUNCTION__, wlc_hw->band->bandunit, wlc_hw->band->bandtype,
		         wlc->core->coreidx));
		if ((wlc_hw->band->pi = wlc_phy_attach(wlc_hw->phy_sh, (void *)(uintptr)regs,
			wlc_hw->band->bandtype, vars)) == NULL) {
			WL_ERROR(("wl%d: wlc_bmac_attach: wlc_phy_attach failed\n", unit));
			err = 17;
			goto fail;
		}

		wlc_phy_machwcap_set(wlc_hw->band->pi, wlc_hw->machwcap);

		wlc_phy_get_phyversion(wlc_hw->band->pi, &wlc_hw->band->phytype,
			&wlc_hw->band->phyrev, &wlc_hw->band->radioid, &wlc_hw->band->radiorev);
		wlc_hw->band->abgphy_encore = wlc_phy_get_encore(wlc_hw->band->pi);
		wlc->band->abgphy_encore = wlc_phy_get_encore(wlc_hw->band->pi);
		wlc_hw->band->core_flags = wlc_phy_get_coreflags(wlc_hw->band->pi);

		/* verify good phy_type & supported phy revision */
		if (WLCISAPHY(wlc_hw->band)) {
			if (ACONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISGPHY(wlc_hw->band)) {
			if (GCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISNPHY(wlc_hw->band)) {
			if (NCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISLPPHY(wlc_hw->band)) {
			if (LPCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			if (SSLPNCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			if (LCNCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISHTPHY(wlc_hw->band)) {
			if (HTCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else {
bad_phy:
			WL_ERROR(("wl%d: wlc_bmac_attach: unsupported phy type/rev (%d/%d)\n",
				unit, wlc_hw->band->phytype, wlc_hw->band->phyrev));
			err = 18;
			goto fail;
		}

good_phy:
		WL_ERROR(("wl%d: %s: chiprev %d corerev %d "
			"cccap 0x%x maccap 0x%x band %sG, phy_type %d phy_rev %d\n",
			unit, __FUNCTION__, wlc_hw->sih->chiprev,
			wlc_hw->corerev, wlc_hw->sih->cccaps, wlc_hw->machwcap,
			BAND_2G(wlc_hw->band->bandtype) ? "2.4" : "5",
			wlc_hw->band->phytype, wlc_hw->band->phyrev));

		/* BMAC_NOTE: wlc->band->pi should not be set below and should be done in the
		 * high level attach. However we can not make that change until all low level access
		 * is changed to wlc_hw->band->pi. Instead do the wlc->band->pi init below, keeping
		 * wlc_hw->band->pi as well for incremental update of low level fns, and cut over
		 * low only init when all fns updated.
		 */
		wlc->band->pi = wlc_hw->band->pi;
		wlc->band->phytype = wlc_hw->band->phytype;
		wlc->band->phyrev = wlc_hw->band->phyrev;
		wlc->band->radioid = wlc_hw->band->radioid;
		wlc->band->radiorev = wlc_hw->band->radiorev;

		/* default contention windows size limits */
		wlc_hw->band->CWmin = APHY_CWMIN;
		wlc_hw->band->CWmax = PHY_CWMAX;

		if (!wlc_bmac_attach_dmapio(wlc_hw, j, wme)) {
			err = 19;
			goto fail;
		}
	}

	if (!PIO_ENAB(wlc_hw->wlc->pub) &&
		(BCM4331_CHIP_ID == CHIPID(wlc_hw->sih->chip) &&
		si_pcie_get_request_size(wlc_hw->sih) > 128)) {
		uint i;
		for (i = 0; i < NFIFO; i++) {
			if (wlc_hw->di[i])
				dma_ctrlflags(wlc_hw->di[i], DMA_CTRL_DMA_AVOIDANCE_WAR,
					DMA_CTRL_DMA_AVOIDANCE_WAR);
		}
	}


	/* initialize the rxbnd */
	wlc->pub->tunables->rxbnd = RXBND;

	/* set default 2-wire or 3-wire setting */
	wlc_bmac_btc_wire_set(wlc_hw, WL_BTC_DEFWIRE);

	wlc_hw->btc->btcx_aa = (uint8)getintvar(vars, "aa2g");
	/* set BT Coexistence default mode */
	if (WLCISSSLPNPHY(wlc_hw->band))
		wlc_bmac_btc_mode_set(wlc_hw, WL_BTC_ENABLE);
	else
		wlc_bmac_btc_mode_set(wlc_hw, WL_BTC_DEFAULT);

#ifdef DONGLEBUILD
	/* 2 stage reclaim
	 *  download the ucode during attach, reclaim the ucode after attach
	 *    along with other rattach stuff, unconditionally on dongle
	 *  the second stage reclaim happens after up conditioning on reclaim flag
	 */
	wlc_ucode_download(wlc_hw);
#endif

	/* disable core to match driver "down" state */
	wlc_coredisable(wlc_hw);

	/* Match driver "down" state */
	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
		si_pci_down(wlc_hw->sih);

	/* register sb interrupt callback functions */
#ifdef BCMDBG
	si_register_intr_callback(wlc_hw->sih, (void *)wlc_wlintrsoff,
		(void *)wlc_wlintrsrestore, (void *)wlc_intrs_enabled, wlc_hw);
#else
	si_register_intr_callback(wlc_hw->sih, (void *)wlc_wlintrsoff,
		(void *)wlc_wlintrsrestore, NULL, wlc_hw);
#endif /* BCMDBG */

	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));

	/* turn off pll and xtal to match driver "down" state */
	wlc_bmac_xtal(wlc_hw, OFF);

	/* *********************************************************************
	 * The hardware is in the DOWN state at this point. D11 core
	 * or cores are in reset with clocks off, and the board PLLs
	 * are off if possible.
	 *
	 * Beyond this point, wlc->sbclk == FALSE and chip registers
	 * should not be touched.
	 *********************************************************************
	 */

	/* init etheraddr state variables */
	if ((macaddr = wlc_get_macaddr(wlc_hw)) == NULL) {
		WL_ERROR(("wl%d: wlc_bmac_attach: macaddr not found\n", unit));
		err = 21;
		goto fail;
	}
	bcm_ether_atoe(macaddr, &wlc_hw->etheraddr);
	if (ETHER_ISBCAST((char*)&wlc_hw->etheraddr) ||
		ETHER_ISNULLADDR((char*)&wlc_hw->etheraddr)) {
		WL_ERROR(("wl%d: wlc_bmac_attach: bad macaddr %s\n", unit, macaddr));
		err = 22;
		goto fail;
	}

#ifdef WLC_LOW_ONLY
	bcopy(&wlc_hw->etheraddr, &wlc_hw->orig_etheraddr, ETHER_ADDR_LEN);
#endif

	WL_INFORM(("wl%d: %s: board 0x%x macaddr: %s\n", unit, __FUNCTION__,
		wlc_hw->sih->boardtype, macaddr));

#ifdef WLLED
	if ((wlc_hw->ledh = wlc_bmac_led_attach(wlc_hw)) == NULL) {
		WL_ERROR(("wl%d: wlc_bmac_attach: wlc_bmac_led_attach() failed.\n", unit));
		err = 23;
		goto fail;
	}
#endif
#ifdef AP
	wlc_bmac_pmq_init(wlc_hw);
#endif

#if defined(AP) && defined(WLC_LOW_ONLY)
	wlc_bmac_pa_war_set(wlc_hw, FALSE);
#endif

#ifdef MBSS
	/* This needs to be the same with the one in wlc.c */
	if (D11REV_ISMBSS16(wlc_hw->corerev)) {
		wlc->max_ap_bss = wlc->pub->tunables->maxucodebss;

		/* 4313 has total fifo space of 128 blocks. if we enable
		 * all 16 MBSSs we will not be left with enough fifo space to
		 * support max thru'put. so we only allow configuring/enabling
		 * max of 4 BSSs. Rest of the space is distributed acorss
		 * the tx fifos.
		 */
		if (D11REV_IS(wlc_hw->corerev, 24)) {
			wlc->max_ap_bss = 4;
			wlc->mbss_ucidx_mask = wlc->max_ap_bss - 1;
		}
	}
#endif /* MBSS */

#ifdef WLEXTLOG
#ifdef WLC_LOW_ONLY
	if ((wlc_hw->extlog = wlc_extlog_attach(wlc_hw->osh, NULL)) == NULL) {
		WL_ERROR(("wl%d: wlc_bmac_attach: wlc_extlog_attach() failed.\n", unit));
		err = 24;
		goto fail;
	}
#endif
#endif /* WLEXTLOG */

	/* Register to be notified when pktpool is available which can
	 * happen outside this scope from bus side.
	 */
	if (POOL_ENAB(wlc_hw->wlc->pub)) {
		pktpool_avail_register(wlc_hw->wlc->pub->pktpool, wlc_pktpool_avail_cb, wlc_hw);
#ifdef BCMDBG_POOL
		pktpool_dbg_register(wlc_hw->wlc->pub->pktpool, wlc_pktpool_dbg_cb, wlc_hw);
#endif
	}

	return err;

fail:
	WL_ERROR(("wl%d: wlc_bmac_attach: failed with err %d\n", unit, err));
	return err;
}

/*
 * Initialize wlc_info default values ...
 * may get overrides later in this function
 *  BMAC_NOTES, move low out and resolve the dangling ones
 */
void
#ifdef WLC_LOW_ONLY
wlc_bmac_info_init(wlc_hw_info_t *wlc_hw)
#else
BCMATTACHFN(wlc_bmac_info_init)(wlc_hw_info_t *wlc_hw)
#endif /* WLC_LOW_ONLY */
{
	wlc_info_t *wlc = wlc_hw->wlc;

	/* set default sw macintmask value */
	wlc->defmacintmask = DEF_MACINTMASK;
#ifdef WLP2P
	wlc->defmacintmask |= MI_P2P;
#endif

	/* various 802.11g modes */
	wlc_hw->shortslot = FALSE;

	wlc_hw->SFBL = RETRY_SHORT_FB;
	wlc_hw->LFBL = RETRY_LONG_FB;

	/* default mac retry limits */
	wlc_hw->SRL = RETRY_SHORT_DEF;
	wlc_hw->LRL = RETRY_LONG_DEF;
	wlc_hw->chanspec = CH20MHZ_CHSPEC(1);
}

/*
 * low level detach
 */
int
BCMATTACHFN(wlc_bmac_detach)(wlc_hw_info_t *wlc_hw)
{
	uint i;
	wlc_hwband_t *band;
	wlc_info_t *wlc = wlc_hw->wlc;
	int callbacks;

	callbacks = 0;

	if (wlc_hw->sih) {
		/* detach interrupt sync mechanism since interrupt is disabled and per-port
		 * interrupt object may has been freed. this must be done before sb core switch
		 */
		si_deregister_intr_callback(wlc_hw->sih);

		if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
			si_pci_sleep(wlc_hw->sih);
	}

	wlc_bmac_detach_dmapio(wlc_hw);

	band = wlc_hw->band;
	for (i = 0; i < NBANDS_HW(wlc_hw); i++) {
		/* So if this is a single band 11a card, use band 1 */
		if (IS_SINGLEBAND_5G(wlc_hw->deviceid))
			i = BAND_5G_INDEX;

		if (band->pi) {
			/* Detach this band's phy */
			wlc_phy_detach(band->pi);
			band->pi = NULL;
		}
		band = wlc_hw->bandstate[OTHERBANDUNIT(wlc)];
	}

	/* Free shared phy state */
	wlc_phy_shared_detach(wlc_hw->phy_sh);

	wlc_phy_shim_detach(wlc_hw->physhim);

	/* free vars */
	if (wlc_hw->vars) {
		MFREE(wlc_hw->osh, wlc_hw->vars, wlc_hw->vars_size);
		wlc_hw->vars = NULL;
	}

	if (wlc_hw->sih) {
		si_detach(wlc_hw->sih);
		wlc_hw->sih = NULL;
	}

#ifdef WLLED
	if (wlc_hw->ledh) {
		callbacks += wlc_bmac_led_detach(wlc_hw);
		wlc_hw->ledh = NULL;
	}
#endif
#ifdef AP
	wlc_bmac_pmq_delete(wlc_hw);
#endif

#ifdef WLEXTLOG
#ifdef WLC_LOW_ONLY
	if (wlc_hw->extlog) {
		callbacks += wlc_extlog_detach(wlc_hw->extlog);
		wlc_hw->extlog = NULL;
	}
#endif
#endif /* WLEXTLOG */

	return callbacks;

}

void
BCMINITFN(wlc_bmac_reset)(wlc_hw_info_t *wlc_hw)
{
	WL_TRACE(("wl%d: wlc_bmac_reset\n", wlc_hw->unit));

	WLCNTINCR(wlc_hw->wlc->pub->_cnt->reset);

	/* reset the core */
	if (!DEVICEREMOVED(wlc_hw->wlc))
		wlc_bmac_corereset(wlc_hw, WLC_USE_COREFLAGS);

	/* purge the pio queues or dma rings */
	wlc_flushqueues(wlc_hw);

	wlc_reset_bmac_done(wlc_hw->wlc);
}

void
BCMINITFN(wlc_bmac_init)(wlc_hw_info_t *wlc_hw, chanspec_t chanspec, bool mute,
	uint32 defmacintmask)
{
	uint32 macintmask;
	bool fastclk;
	wlc_info_t *wlc = wlc_hw->wlc;

	WL_TRACE(("wl%d: wlc_bmac_init\n", wlc_hw->unit));

#if defined(WLC_LOW_ONLY) && defined(MBSS)
	wlc->defmacintmask |= defmacintmask;
#else
	UNUSED_PARAMETER(defmacintmask);
#endif
	/* request FAST clock if not on */
	if (!(fastclk = wlc_hw->forcefastclk))
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

	/* disable interrupts */
	macintmask = wl_intrsoff(wlc->wl);

	/* set up the specified band and chanspec */
	wlc_setxband(wlc_hw, CHSPEC_WLCBANDUNIT(chanspec));
	wlc_phy_chanspec_radio_set(wlc_hw->band->pi, chanspec);

	/* do one-time phy inits and calibration */
	wlc_phy_cal_init(wlc_hw->band->pi);

	/* core-specific initialization */
	wlc_coreinit(wlc_hw);

	/* suspend the tx fifos and mute the phy for preism cac time */
	if (mute)
		wlc_bmac_mute(wlc_hw, ON, PHY_MUTE_FOR_PREISM);

	if (WLCISHTPHY(wlc_hw->band)) {
		wlc_phy_switch_radio(wlc_hw->band->pi, OFF);
	}
	/* band-specific inits */
	wlc_bmac_bsinit(wlc_hw, chanspec);

	/* restore macintmask */
	wl_intrsrestore(wlc->wl, macintmask);

	/* seed wake_override with WLC_WAKE_OVERRIDE_MACSUSPEND since the mac is suspended
	 * and wlc_bmac_enable_mac() will clear this override bit.
	 */
	mboolset(wlc_hw->wake_override, WLC_WAKE_OVERRIDE_MACSUSPEND);

	/*
	 * initialize mac_suspend_depth to 1 to match ucode initial suspended state
	 */
	wlc_hw->mac_suspend_depth = 1;

	/* restore the clk */
	if (!fastclk)
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);
}

int
BCMINITFN(wlc_bmac_set_epa_default_state)(wlc_hw_info_t *wlc_hw)
{
#define GPIO_5	(1<<5)
	uint32 mask = GPIO_5, val = GPIO_5;

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));
	if (wlc_hw->sih->boardtype != BCM94331X19)
		return (-1);

	si_gpiopull(wlc_hw->sih, GPIO_PULLUP, mask, 0);
	si_gpiopull(wlc_hw->sih, GPIO_PULLDN, mask, val);

	return 0;
}

int
BCMINITFN(wlc_bmac_up_prep)(wlc_hw_info_t *wlc_hw)
{
	uint coremask;

	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	ASSERT(wlc_hw->wlc->pub->hw_up && wlc_hw->wlc->macintmask == 0);

	/*
	 * Enable pll and xtal, initialize the power control registers,
	 * and force fastclock for the remainder of wlc_up().
	 */
	wlc_bmac_xtal(wlc_hw, ON);
	si_clkctl_init(wlc_hw->sih);
	wlc_clkctl_clk(wlc_hw, CLK_FAST);

	/*
	 * Configure pci/pcmcia here instead of in wlc_attach()
	 * to allow mfg hotswap:  down, hotswap (chip power cycle), up.
	 */
	coremask = (1 << wlc_hw->wlc->core->coreidx);

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
		si_pci_setup(wlc_hw->sih, coremask);
	else if (BUSTYPE(wlc_hw->sih->bustype) == PCMCIA_BUS) {
		wlc_hw->regs = (d11regs_t*)si_setcore(wlc_hw->sih, D11_CORE_ID, 0);
		wlc_hw->wlc->regs = wlc_hw->regs;
		si_pcmcia_init(wlc_hw->sih);
	}
	ASSERT(si_coreid(wlc_hw->sih) == D11_CORE_ID);

	/*
	 * Need to read the hwradio status here to cover the case where the system
	 * is loaded with the hw radio disabled. We do not want to bring the driver up in this case.
	 */
	if (wlc_bmac_radio_read_hwdisabled(wlc_hw)) {
		/* put SB PCI in down state again */
		if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
			si_pci_down(wlc_hw->sih);
		wlc_bmac_xtal(wlc_hw, OFF);
		return BCME_RADIOOFF;
	}

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
		si_pci_up(wlc_hw->sih);


	/* reset the d11 core */
	wlc_bmac_corereset(wlc_hw, WLC_USE_COREFLAGS);

	return 0;
}

int
BCMINITFN(wlc_bmac_up_finish)(wlc_hw_info_t *wlc_hw)
{
	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	wlc_hw->up = TRUE;
	wlc_phy_hw_state_upd(wlc_hw->band->pi, TRUE);

	/* FULLY enable dynamic power control and d11 core interrupt */
	wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);
	ASSERT(wlc_hw->wlc->macintmask == 0);
	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));
	wl_intrson(wlc_hw->wlc->wl);

#ifdef WLC_LOW_ONLY
	/* start one second watchdog timer */
	wl_add_timer(wlc_hw->wlc->wl, wlc_hw->wdtimer, TIMER_INTERVAL_WATCHDOG_BMAC, TRUE);
	/* start 10 msec rpc watchdog timer */
	wl_add_timer(wlc_hw->wlc->wl, wlc_hw->rpc_agg_wdtimer,
		TIMER_INTERVAL_RPC_AGG_WATCHDOG_BMAC, TRUE);
#endif /* WLC_LOW_ONLY */
#if NCONF
	wlc_bmac_ifsctl_edcrs_set(wlc_hw,
#ifdef WLAFTERBURNER
	wlc_hw->wlc->afterburner,
#else
	FALSE,
#endif
	WLCISHTPHY(wlc_hw->wlc->band));
#endif /* NCONF */
	return 0;
}

int
BCMINITFN(wlc_bmac_set_ctrl_ePA)(wlc_hw_info_t *wlc_hw)
{
	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (!wlc_hw->clk) {
		WL_ERROR(("wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__));
		return -1;
	}
	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		if ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) {
			WL_INFORM(("wl%d: %s: set mux pin to ePA\n", wlc_hw->unit, __FUNCTION__));
			/* force muxed pin to control ePA */
			si_chipcontrl_epa4331(wlc_hw->sih, TRUE);
		}
	}

	return 0;
}

int
BCMUNINITFN(wlc_bmac_down_prep)(wlc_hw_info_t *wlc_hw)
{
	bool dev_gone;
	uint callbacks = 0;

	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (!wlc_hw->up)
		return callbacks;

	dev_gone = DEVICEREMOVED(wlc_hw->wlc);

	/* disable interrupts */
	if (dev_gone)
		wlc_hw->wlc->macintmask = 0;
	else {
		/* now disable interrupts */
		wl_intrsoff(wlc_hw->wlc->wl);

		/* ensure we're running on the pll clock again */
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

		/* Disable GPIOs related to BTC returning the control to chipcommon */
		if (!wlc_hw->noreset)
			wlc_bmac_btc_gpio_disable(wlc_hw);
	}

#ifdef WLC_LOW_ONLY
	/* cancel the watchdog timer */
	if (!wl_del_timer(wlc_hw->wlc->wl, wlc_hw->wdtimer))
		callbacks++;

	/* cancel the rpc agg watchdog timer */
	if (!wl_del_timer(wlc_hw->wlc->wl, wlc_hw->rpc_agg_wdtimer))
		callbacks++;
#endif /* WLC_LOW_ONLY */

	/* down phy at the last of this stage */
	callbacks += wlc_phy_down(wlc_hw->band->pi);

	return callbacks;
}

void
BCMUNINITFN(wlc_bmac_hw_down)(wlc_hw_info_t *wlc_hw)
{
	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {

		if ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) {
			WL_INFORM(("wl%d: %s: set mux pin to SROM\n", wlc_hw->unit, __FUNCTION__));
			/* restore all muxed pins back to default state */
			si_chipcontrl_epa4331(wlc_hw->sih, FALSE);
		}

		si_pci_down(wlc_hw->sih);
	}
	wlc_bmac_xtal(wlc_hw, OFF);
}

int
BCMUNINITFN(wlc_bmac_down_finish)(wlc_hw_info_t *wlc_hw)
{
	uint callbacks = 0;
	bool dev_gone;

	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (!wlc_hw->up)
		return callbacks;

	wlc_hw->up = FALSE;
	wlc_phy_hw_state_upd(wlc_hw->band->pi, FALSE);

	dev_gone = DEVICEREMOVED(wlc_hw->wlc);

	if (dev_gone) {
		wlc_hw->sbclk = FALSE;
		wlc_hw->clk = FALSE;
		wlc_phy_hw_clk_state_upd(wlc_hw->band->pi, FALSE);

		/* reclaim any posted packets */
		wlc_flushqueues(wlc_hw);
	} else {

		/* Reset and disable the core */
		if (si_iscoreup(wlc_hw->sih)) {
			if (R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC)
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			callbacks += wl_reset(wlc_hw->wlc->wl);
			wlc_coredisable(wlc_hw);
		}

		/* turn off primary xtal and pll */
		if (!wlc_hw->noreset)
			wlc_bmac_hw_down(wlc_hw);
	}

	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));

	return callbacks;
}

void
wlc_bmac_wait_for_wake(wlc_hw_info_t *wlc_hw)
{
	if (D11REV_IS(wlc_hw->corerev, 4)) /* no slowclock */
		OSL_DELAY(5);
	else {
		/* delay before first read of ucode state */
		if ((WLCISGPHY(wlc_hw->band)) && (D11REV_IS(wlc_hw->corerev, 5))) {
#if defined(DSLCPE_DELAY)
			OSL_YIELD_EXEC(wlc_hw->osh, 2000);
#else
			OSL_DELAY(2000);
#endif
		} else {
			OSL_DELAY(40);
		}

		/* wait until ucode is no longer asleep */
		SPINWAIT((wlc_bmac_read_shm(wlc_hw, M_UCODE_DBGST) == DBGST_ASLEEP),
		         wlc_hw->wlc->fastpwrup_dly);
	}

	ASSERT(wlc_bmac_read_shm(wlc_hw, M_UCODE_DBGST) != DBGST_ASLEEP);
}

void
wlc_bmac_hw_etheraddr(wlc_hw_info_t *wlc_hw, struct ether_addr *ea)
{
	bcopy(&wlc_hw->etheraddr, ea, ETHER_ADDR_LEN);
}

void
wlc_bmac_set_hw_etheraddr(wlc_hw_info_t *wlc_hw, struct ether_addr *ea)
{
	bcopy(ea, &wlc_hw->etheraddr, ETHER_ADDR_LEN);
}

int
wlc_bmac_bandtype(wlc_hw_info_t *wlc_hw)
{
	return (wlc_hw->band->bandtype);
}

void *
wlc_cur_phy(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	return ((void *)wlc_hw->band->pi);
}

/* control chip clock to save power, enable dynamic clock or force fast clock */
static void
wlc_clkctl_clk(wlc_hw_info_t *wlc_hw, uint mode)
{
	if (PMUCTL_ENAB(wlc_hw->sih)) {
		/* new chips with PMU, CCS_FORCEHT will distribute the HT clock on backplane,
		 *  but mac core will still run on ALP(not HT) when it enters powersave mode,
		 *      which means the FCA bit may not be set.
		 *      should wakeup mac if driver wants it to run on HT.
		 */

		if (wlc_hw->clk) {
			if (mode == CLK_FAST) {
				OR_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, CCS_FORCEHT);

				OSL_DELAY(64);

				SPINWAIT(((R_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st) &
				           CCS_HTAVAIL) == 0), PMU_MAX_TRANSITION_DLY);
				ASSERT(R_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st) &
					CCS_HTAVAIL);
			} else {
				if ((wlc_hw->sih->pmurev == 0) &&
				    (R_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st) &
				     (CCS_FORCEHT | CCS_HTAREQ)))
					SPINWAIT(((R_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st) &
					           CCS_HTAVAIL) == 0), PMU_MAX_TRANSITION_DLY);
				AND_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, ~CCS_FORCEHT);
			}
		}
		wlc_hw->forcefastclk = (mode == CLK_FAST);
	} else {
		bool wakeup_ucode;

		/* old chips w/o PMU, force HT through cc,
		 * then use FCA to verify mac is running fast clock
		 */

		wakeup_ucode = D11REV_LT(wlc_hw->corerev, 9);

		if (wlc_hw->up && wakeup_ucode)
			wlc_ucode_wake_override_set(wlc_hw, WLC_WAKE_OVERRIDE_CLKCTL);

		wlc_hw->forcefastclk = si_clkctl_cc(wlc_hw->sih, mode);

		if (D11REV_LT(wlc_hw->corerev, 11)) {
			/* ucode WAR for old chips */
			if (wlc_hw->forcefastclk)
				wlc_bmac_mhf(wlc_hw, MHF1, MHF1_FORCEFASTCLK, MHF1_FORCEFASTCLK,
				        WLC_BAND_ALL);
			else
				wlc_bmac_mhf(wlc_hw, MHF1, MHF1_FORCEFASTCLK, 0, WLC_BAND_ALL);
		}

		/* check fast clock is available (if core is not in reset) */
		if (D11REV_GT(wlc_hw->corerev, 4) && wlc_hw->forcefastclk && wlc_hw->clk)
			ASSERT(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA);

		/* keep the ucode wake bit on if forcefastclk is on
		 * since we do not want ucode to put us back to slow clock
		 * when it dozes for PM mode.
		 * Code below matches the wake override bit with current forcefastclk state
		 * Only setting bit in wake_override instead of waking ucode immediately
		 * since old code (wlc.c 1.4499) had this behavior. Older code set
		 * wlc->forcefastclk but only had the wake happen if the wakup_ucode work
		 * (protected by an up check) was executed just below.
		 */
		if (wlc_hw->forcefastclk)
			mboolset(wlc_hw->wake_override, WLC_WAKE_OVERRIDE_FORCEFAST);
		else
			mboolclr(wlc_hw->wake_override, WLC_WAKE_OVERRIDE_FORCEFAST);

		/* ok to clear the wakeup now */
		if (wlc_hw->up && wakeup_ucode)
			wlc_ucode_wake_override_clear(wlc_hw, WLC_WAKE_OVERRIDE_CLKCTL);
	}
}

/* set initial host flags value */
static void
BCMINITFN(wlc_mhfdef)(wlc_hw_info_t *wlc_hw, uint16 *mhfs, uint16 mhf2_init)
{
	bzero(mhfs, sizeof(uint16) * MHFMAX);

	mhfs[MHF2] |= mhf2_init;

	if (WLCISGPHY(wlc_hw->band) && GREV_IS(wlc_hw->band->phyrev, 1))
		mhfs[MHF1] |= MHF1_DCFILTWAR;

	if (WLCISGPHY(wlc_hw->band) && (wlc_hw->boardflags & BFL_PACTRL) &&
		(wlc_hw->ucode_dbgsel != 0))
		mhfs[MHF1] |= MHF1_PACTL;

	/* enable CCK power boost in ucode but not for 4318/20 */
	if (WLCISGPHY(wlc_hw->band) && (wlc_hw->band->phyrev < 3)) {
		if (mhfs[MHF1] & MHF1_PACTL)
			WL_ERROR(("wl%d: Cannot support simultaneous MHF1_OFDMPWR & MHF1_CCKPWR\n",
				wlc_hw->unit));
		else
			mhfs[MHF1] |= MHF1_CCKPWR;
	} else {
		/* WAR for pin mux between ePA & SROM for 4331 12x9 package */
		bool is_4331_12x9 = FALSE;
		is_4331_12x9 = (CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID);
		is_4331_12x9 &= ((wlc_hw->sih->chippkg == 9 || wlc_hw->sih->chippkg == 0xb));
		if (is_4331_12x9)
			mhfs[MHF1] |= MHF1_4331EPA_MUX;
	}

	/* prohibit use of slowclock on multifunction boards */
	if (wlc_hw->boardflags & BFL_NOPLLDOWN)
		mhfs[MHF1] |= MHF1_FORCEFASTCLK;

	if ((wlc_hw->band->radioid == BCM2050_ID) && (wlc_hw->band->radiorev < 6))
		mhfs[MHF2] |= MHF2_SYNTHPUWAR;

	if (WLCISNPHY(wlc_hw->band) && NREV_LT(wlc_hw->band->phyrev, 2)) {
		mhfs[MHF2] |= MHF2_NPHY40MHZ_WAR;
		mhfs[MHF1] |= MHF1_IQSWAP_WAR;
	}

	/* set host flag to enable ucode for srom9: tx power offset based on txpwrctrl word */
	if (WLCISNPHY(wlc_hw->band) && (wlc_hw->sromrev >= 9)) {
		mhfs[MHF2] |= MHF2_PPR_HWPWRCTL;
	}
}

/* set or clear ucode host flag bits
 * it has an optimization for no-change write
 * it only writes through shared memory when the core has clock;
 * pre-CLK changes should use wlc_write_mhf to get around the optimization
 *
 *
 * bands values are: WLC_BAND_AUTO <--- Current band only
 *                   WLC_BAND_5G   <--- 5G band only
 *                   WLC_BAND_2G   <--- 2G band only
 *                   WLC_BAND_ALL  <--- All bands
 */
void
wlc_bmac_mhf(wlc_hw_info_t *wlc_hw, uint8 idx, uint16 mask, uint16 val, int bands)
{
	uint16 save;
	uint16 addr[MHFMAX] = {M_HOST_FLAGS1, M_HOST_FLAGS2, M_HOST_FLAGS3, M_HOST_FLAGS4,
		M_HOST_FLAGS5};
	wlc_hwband_t *band;

	ASSERT((val & ~mask) == 0);
	ASSERT(idx < MHFMAX);
	ASSERT(ARRAYSIZE(addr) == MHFMAX);

	switch (bands) {
		/* Current band only or all bands,
		 * then set the band to current band
		 */
	case WLC_BAND_AUTO:
	case WLC_BAND_ALL:
		band = wlc_hw->band;
		break;
	case WLC_BAND_5G:
		band = wlc_hw->bandstate[BAND_5G_INDEX];
		break;
	case WLC_BAND_2G:
		band = wlc_hw->bandstate[BAND_2G_INDEX];
		break;
	default:
		ASSERT(0);
		band = NULL;
	}

	if (band) {
		save = band->mhfs[idx];
		band->mhfs[idx] = (band->mhfs[idx] & ~mask) | val;

		/* optimization: only write through if changed, and
		 * changed band is the current band
		 */
		if (wlc_hw->clk && (band->mhfs[idx] != save) && (band == wlc_hw->band))
			wlc_bmac_write_shm(wlc_hw, addr[idx], (uint16)band->mhfs[idx]);
	}

	if (bands == WLC_BAND_ALL) {
		wlc_hw->bandstate[0]->mhfs[idx] = (wlc_hw->bandstate[0]->mhfs[idx] & ~mask) | val;
		wlc_hw->bandstate[1]->mhfs[idx] = (wlc_hw->bandstate[1]->mhfs[idx] & ~mask) | val;
	}
}

uint16
wlc_bmac_mhf_get(wlc_hw_info_t *wlc_hw, uint8 idx, int bands)
{
	wlc_hwband_t *band;
	ASSERT(idx < MHFMAX);

	switch (bands) {
	case WLC_BAND_AUTO:
		band = wlc_hw->band;
		break;
	case WLC_BAND_5G:
		band = wlc_hw->bandstate[BAND_5G_INDEX];
		break;
	case WLC_BAND_2G:
		band = wlc_hw->bandstate[BAND_2G_INDEX];
		break;
	default:
		ASSERT(0);
		band = NULL;
	}

	if (!band)
		return 0;

	return band->mhfs[idx];
}

static void
wlc_write_mhf(wlc_hw_info_t *wlc_hw, uint16 *mhfs)
{
	uint8 idx;
	uint16 addr[] = {M_HOST_FLAGS1, M_HOST_FLAGS2, M_HOST_FLAGS3,
		M_HOST_FLAGS4, M_HOST_FLAGS5};

	ASSERT(ARRAYSIZE(addr) == MHFMAX);

	for (idx = 0; idx < MHFMAX; idx++) {
		wlc_bmac_write_shm(wlc_hw, addr[idx], mhfs[idx]);
	}
}

/* edcrs getting stuck on top of frame WAR */
static void
wlc_bmac_ifsctl1_regshm(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val)
{
	osl_t *osh;
	d11regs_t *regs;
	uint32 w;

	osh = wlc_hw->osh;
	regs = wlc_hw->regs;
	w = (R_REG(osh, &regs->ifs_ctl1) & ~mask) | val;
	W_REG(osh, &regs->ifs_ctl1, w);
	wlc_bmac_write_shm(wlc_hw, M_IFSCTL1, (uint16)w);
}


/* set the maccontrol register to desired reset state and
 * initialize the sw cache of the register
 */
static void
wlc_mctrl_reset(wlc_hw_info_t *wlc_hw)
{
	/* IHR accesses are always enabled, PSM disabled, HPS off and WAKE on */
	wlc_hw->maccontrol = 0;
	wlc_hw->suspended_fifos = 0;
	wlc_hw->wake_override = 0;
	wlc_hw->mute_override = 0;
	wlc_bmac_mctrl(wlc_hw, ~0, MCTL_IHR_EN | MCTL_WAKE);
}

/* set or clear maccontrol bits */
void
wlc_bmac_mctrl(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val)
{
	uint32 maccontrol;
	uint32 new_maccontrol;

	ASSERT((val & ~mask) == 0);

	maccontrol = wlc_hw->maccontrol;
	new_maccontrol = (maccontrol & ~mask) | val;

	/* if the new maccontrol value is the same as the old, nothing to do */
	if (new_maccontrol == maccontrol)
		return;

	/* something changed, cache the new value */
	wlc_hw->maccontrol = new_maccontrol;

	/* write the new values with overrides applied */
	wlc_mctrl_write(wlc_hw);
}

/* write the software state of maccontrol and overrides to the maccontrol register */
static void
wlc_mctrl_write(wlc_hw_info_t *wlc_hw)
{
	uint32 maccontrol = wlc_hw->maccontrol;

	/* OR in the wake bit if overridden */
	if (wlc_hw->wake_override)
		maccontrol |= MCTL_WAKE;

	/* set AP and INFRA bits for mute if needed */
	if (wlc_hw->mute_override) {
		maccontrol &= ~(MCTL_AP);
		maccontrol |= MCTL_INFRA;
	}

	W_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol, maccontrol);
}

void
wlc_ucode_wake_override_set(wlc_hw_info_t *wlc_hw, uint32 override_bit)
{
	ASSERT((wlc_hw->wake_override & override_bit) == 0);

	if (wlc_hw->wake_override || (wlc_hw->maccontrol & MCTL_WAKE)) {
		mboolset(wlc_hw->wake_override, override_bit);
		return;
	}

	mboolset(wlc_hw->wake_override, override_bit);

	wlc_mctrl_write(wlc_hw);
	wlc_bmac_wait_for_wake(wlc_hw);

	return;
}

void
wlc_ucode_wake_override_clear(wlc_hw_info_t *wlc_hw, uint32 override_bit)
{
	ASSERT(wlc_hw->wake_override & override_bit);

	mboolclr(wlc_hw->wake_override, override_bit);

	if (wlc_hw->wake_override || (wlc_hw->maccontrol & MCTL_WAKE))
		return;

	wlc_mctrl_write(wlc_hw);

	return;
}

/* When driver needs ucode to stop beaconing, it has to make sure that
 * MCTL_AP is clear and MCTL_INFRA is set
 * Mode           MCTL_AP        MCTL_INFRA
 * AP                1              1
 * STA               0              1 <--- This will ensure no beacons
 * IBSS              0              0
 */
static void
wlc_ucode_mute_override_set(wlc_hw_info_t *wlc_hw)
{
	wlc_hw->mute_override = 1;

	/* if maccontrol already has AP == 0 and INFRA == 1 without this
	 * override, then there is no change to write
	 */
	if ((wlc_hw->maccontrol & (MCTL_AP | MCTL_INFRA)) == MCTL_INFRA)
		return;

	wlc_mctrl_write(wlc_hw);

	return;
}

/* Clear the override on AP and INFRA bits */
static void
wlc_ucode_mute_override_clear(wlc_hw_info_t *wlc_hw)
{
	if (wlc_hw->mute_override == 0)
		return;

	wlc_hw->mute_override = 0;

	/* if maccontrol already has AP == 0 and INFRA == 1 without this
	 * override, then there is no change to write
	 */
	if ((wlc_hw->maccontrol & (MCTL_AP | MCTL_INFRA)) == MCTL_INFRA)
		return;

	wlc_mctrl_write(wlc_hw);
}

#ifdef WLC_LOW_ONLY
/* Low Level API functions for reg read/write called by a high level driver */
uint32
wlc_reg_read(wlc_info_t *wlc, void *r, uint size)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	void* addr = (int8*)wlc_hw->regs + ((int8*)r - (int8*)0);
	uint32 v;

	if (size == 1)
		v = R_REG(wlc_hw->osh, (uint8*)addr);
	else if (size == 2)
		v = R_REG(wlc_hw->osh, (uint16*)addr);
	else
		v = R_REG(wlc_hw->osh, (uint32*)addr);

	return v;
}

void
wlc_reg_write(wlc_info_t *wlc, void *r, uint32 v, uint size)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	void* addr = (int8*)wlc_hw->regs + ((int8*)r - (int8*)0);

	if (size == 1)
		W_REG(wlc_hw->osh, (uint8*)addr, v);
	else if (size == 2)
		W_REG(wlc_hw->osh, (uint16*)addr, v);
	else
		W_REG(wlc_hw->osh, (uint32*)addr, v);
}
#endif /* WLC_LOW_ONLY */

/*
 * Write a MAC address to the rcmta structure
 */
void
wlc_bmac_set_rcmta(wlc_hw_info_t *wlc_hw, int idx, const struct ether_addr *addr)
{
	d11regs_t *regs = wlc_hw->regs;
	volatile uint16* objdata16 = (volatile uint16*)(uintptr)&regs->objdata;
	uint32 mac_hm;
	uint16 mac_l;
	osl_t *osh;

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	ASSERT(wlc_hw->corerev > 4);

	mac_hm = (addr->octet[3] << 24) | (addr->octet[2] << 16) | (addr->octet[1] << 8) |
		addr->octet[0];
	mac_l = (addr->octet[5] << 8) | addr->octet[4];

	osh = wlc_hw->osh;

	W_REG(osh, &regs->objaddr, (OBJADDR_RCMTA_SEL | (idx * 2)));
	(void)R_REG(osh, &regs->objaddr);
	W_REG(osh, &regs->objdata, mac_hm);
	W_REG(osh, &regs->objaddr, (OBJADDR_RCMTA_SEL | ((idx * 2) + 1)));
	(void)R_REG(osh, &regs->objaddr);
	W_REG(osh, objdata16, mac_l);
}

/*
 * Write a MAC address to the given match reg offset in the RXE match engine.
 */
void
wlc_bmac_set_addrmatch(wlc_hw_info_t *wlc_hw, int match_reg_offset, const struct ether_addr *addr)
{
	d11regs_t *regs;
	uint16 mac_l;
	uint16 mac_m;
	uint16 mac_h;
	osl_t *osh;

	WL_TRACE(("wl%d: wlc_bmac_set_addrmatch\n", wlc_hw->unit));

	ASSERT((match_reg_offset < RCM_SIZE) || (wlc_hw->corerev == 4));

	regs = wlc_hw->regs;
	mac_l = addr->octet[0] | (addr->octet[1] << 8);
	mac_m = addr->octet[2] | (addr->octet[3] << 8);
	mac_h = addr->octet[4] | (addr->octet[5] << 8);


	osh = wlc_hw->osh;

	/* enter the MAC addr into the RXE match registers */
	W_REG(osh, &regs->rcm_ctl, RCM_INC_DATA | match_reg_offset);
	W_REG(osh, &regs->rcm_mat_data, mac_l);
	W_REG(osh, &regs->rcm_mat_data, mac_m);
	W_REG(osh, &regs->rcm_mat_data, mac_h);

}

void
wlc_bmac_write_template_ram(wlc_hw_info_t *wlc_hw, int offset, int len, void *buf)
{
	d11regs_t *regs;
	uint32 word;
	bool be_bit;
#ifdef IL_BIGENDIAN
	volatile uint16 *dptr = NULL;
#endif /* IL_BIGENDIAN */
	osl_t *osh;

	WL_TRACE(("wl%d: wlc_bmac_write_template_ram\n", wlc_hw->unit));

	regs = wlc_hw->regs;
	osh = wlc_hw->osh;

	ASSERT(ISALIGNED(offset, sizeof(uint32)));
	ASSERT(ISALIGNED(len, sizeof(uint32)));
	ASSERT((offset & ~0xffff) == 0);

	W_REG(osh, &regs->tplatewrptr, offset);

#ifdef IL_BIGENDIAN
	if (BUSTYPE(wlc_hw->sih->bustype) == PCMCIA_BUS)
		dptr = (uint16*)&regs->tplatewrdata;
#endif /* IL_BIGENDIAN */

	/* if MCTL_BIGEND bit set in mac control register,
	 * the chip swaps data in fifo, as well as data in
	 * template ram
	 */
	be_bit = (R_REG(osh, &regs->maccontrol) & MCTL_BIGEND) != 0;

	while (len > 0) {
		bcopy((uint8*)buf, &word, sizeof(uint32));

		if (be_bit)
			word = hton32(word);
		else
			word = htol32(word);

#ifdef IL_BIGENDIAN
		if (BUSTYPE(wlc_hw->sih->bustype) == PCMCIA_BUS &&
		    D11REV_IS(wlc_hw->corerev, 4)) {
			W_REG(osh, (dptr + 1), (uint16)((word >> NBITS(uint16)) & 0xffff));
			W_REG(osh, dptr, (uint16)(word & 0xffff));
		} else
#endif /* IL_BIGENDIAN */
			W_REG(osh, &regs->tplatewrdata, word);

		buf = (uint8*)buf + sizeof(uint32);
		len -= sizeof(uint32);
	}
}

void
wlc_bmac_set_cwmin(wlc_hw_info_t *wlc_hw, uint16 newmin)
{
	osl_t *osh;

	osh = wlc_hw->osh;
	wlc_hw->band->CWmin = newmin;

	W_REG(osh, &wlc_hw->regs->objaddr, OBJADDR_SCR_SEL | S_DOT11_CWMIN);
	(void)R_REG(osh, &wlc_hw->regs->objaddr);
	W_REG(osh, &wlc_hw->regs->objdata, newmin);
}

void
wlc_bmac_set_cwmax(wlc_hw_info_t *wlc_hw, uint16 newmax)
{
	osl_t *osh;

	osh = wlc_hw->osh;
	wlc_hw->band->CWmax = newmax;

	W_REG(osh, &wlc_hw->regs->objaddr, OBJADDR_SCR_SEL | S_DOT11_CWMAX);
	(void)R_REG(osh, &wlc_hw->regs->objaddr);
	W_REG(osh, &wlc_hw->regs->objdata, newmax);
}

void
wlc_bmac_bw_set(wlc_hw_info_t *wlc_hw, uint16 bw)
{
	uint32 tmp;

	wlc_phy_bw_state_set(wlc_hw->band->pi, bw);

	ASSERT(wlc_hw->clk);
	if (D11REV_LT(wlc_hw->corerev, 17))
		tmp = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);

	wlc_bmac_phy_reset(wlc_hw);
	wlc_phy_init(wlc_hw->band->pi, wlc_phy_chanspec_get(wlc_hw->band->pi));

	/* restore the clk */
}

static void
wlc_write_hw_bcntemplate0(wlc_hw_info_t *wlc_hw, void *bcn, int len)
{
	d11regs_t *regs = wlc_hw->regs;

	wlc_bmac_write_template_ram(wlc_hw, T_BCN0_TPL_BASE, (len + 3) & ~3, bcn);
	/* write beacon length to SCR */
	ASSERT(len < 65536);
	wlc_bmac_write_shm(wlc_hw, M_BCN0_FRM_BYTESZ, (uint16)len);
	/* mark beacon0 valid */
	OR_REG(wlc_hw->osh, &regs->maccommand, MCMD_BCN0VLD);
}

static void
wlc_write_hw_bcntemplate1(wlc_hw_info_t *wlc_hw, void *bcn, int len)
{
	d11regs_t *regs = wlc_hw->regs;

	wlc_bmac_write_template_ram(wlc_hw, T_BCN1_TPL_BASE, (len + 3) & ~3, bcn);
	/* write beacon length to SCR */
	ASSERT(len < 65536);
	wlc_bmac_write_shm(wlc_hw, M_BCN1_FRM_BYTESZ, (uint16)len);
	/* mark beacon1 valid */
	OR_REG(wlc_hw->osh, &regs->maccommand, MCMD_BCN1VLD);
}

/* mac is assumed to be suspended at this point */
void
wlc_bmac_write_hw_bcntemplates(wlc_hw_info_t *wlc_hw, void *bcn, int len, bool both)
{
	d11regs_t *regs = wlc_hw->regs;

	if (both) {
		wlc_write_hw_bcntemplate0(wlc_hw, bcn, len);
		wlc_write_hw_bcntemplate1(wlc_hw, bcn, len);
	} else {
		/* bcn 0 */
		if (!(R_REG(wlc_hw->osh, &regs->maccommand) & MCMD_BCN0VLD))
			wlc_write_hw_bcntemplate0(wlc_hw, bcn, len);
		/* bcn 1 */
		else if (!(R_REG(wlc_hw->osh, &regs->maccommand) & MCMD_BCN1VLD))
			wlc_write_hw_bcntemplate1(wlc_hw, bcn, len);
		else	/* one template should always have been available */
			ASSERT(0);
	}
}

static void
WLBANDINITFN(wlc_bmac_upd_synthpu)(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	uint16 v;

	wlc = wlc; /* avoid unused variable warning */ 
	/* update SYNTHPU_DLY */


	if (ISSIM_ENAB(wlc_hw->sih)) {
		v = SYNTHPU_DLY_PHY_US_QT;
	} else {

		/* for LPPHY synthpu delay is very small as PMU handles xtal/pll */
		if (WLCISLPPHY(wlc_hw->band)) {
			v = LPREV_GE(wlc_hw->band->phyrev, 2) ?
				SYNTHPU_DLY_LPPHY_US : SYNTHPU_DLY_BPHY_US;
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_LPPHY_US;
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_LPPHY_US;
		} else if (WLCISAPHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_APHY_US;
		} else if (WLCISNPHY(wlc_hw->band) && (NREV_GE(wlc_hw->band->phyrev, 3))) {
			v = SYNTHPU_DLY_NPHY_US;
		} else if (WLCISHTPHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_HTPHY_US;
		} else {
			v = SYNTHPU_DLY_BPHY_US;
		}
	}

	if ((wlc_hw->band->radioid == BCM2050_ID) && (wlc_hw->band->radiorev == 8)) {
		if (v < 2400)
			v = 2400;
	}

	wlc_bmac_write_shm(wlc_hw, M_SYNTHPU_DLY, v);
}

/* band-specific init */
static void
WLBANDINITFN(wlc_bmac_bsinit)(wlc_hw_info_t *wlc_hw, chanspec_t chanspec)
{

	WL_TRACE(("wl%d: wlc_bmac_bsinit: bandunit %d\n", wlc_hw->unit, wlc_hw->band->bandunit));

	/* sanity check */
	if (PHY_TYPE(R_REG(wlc_hw->osh, &wlc_hw->regs->phyversion)) != PHY_TYPE_LCNXN)
		ASSERT((uint)PHY_TYPE(R_REG(wlc_hw->osh, &wlc_hw->regs->phyversion)) ==
		       wlc_hw->band->phytype);

	wlc_ucode_bsinit(wlc_hw);

	wlc_phy_init(wlc_hw->band->pi, chanspec);


	wlc_ucode_txant_set(wlc_hw);

	/* cwmin is band-specific, update hardware with value for current band */
	wlc_bmac_set_cwmin(wlc_hw, wlc_hw->band->CWmin);
	wlc_bmac_set_cwmax(wlc_hw, wlc_hw->band->CWmax);

	wlc_bmac_update_slot_timing(wlc_hw,
		BAND_5G(wlc_hw->band->bandtype) ? TRUE : wlc_hw->shortslot);

	/* write phytype and phyvers */
	wlc_bmac_write_shm(wlc_hw, M_PHYTYPE, (uint16)wlc_hw->band->phytype);
	wlc_bmac_write_shm(wlc_hw, M_PHYVER, (uint16)wlc_hw->band->phyrev);

#ifdef WL11N
	/* initialize the txphyctl1 rate table since shmem is shared between bands */
	wlc_upd_ofdm_pctl1_table(wlc_hw);
#endif

	if (D11REV_IS(wlc_hw->corerev, 4) && WLCISAPHY(wlc_hw->band))
		wlc_bmac_write_shm(wlc_hw, M_SEC_DEFIVLOC, 0x1d);

	wlc_bmac_upd_synthpu(wlc_hw);

	/* Configure BTC GPIOs as bands change */
	if (BAND_5G(wlc_hw->band->bandtype))
		wlc_bmac_mhf(wlc_hw, MHF5, MHF5_BTCX_DEFANT, MHF5_BTCX_DEFANT, WLC_BAND_ALL);
	else
		wlc_bmac_mhf(wlc_hw, MHF5, MHF5_BTCX_DEFANT, 0, WLC_BAND_ALL);
	wlc_bmac_btc_gpio_enable(wlc_hw);
}

void
wlc_bmac_core_phy_clk(wlc_hw_info_t *wlc_hw, bool clk)
{
	WL_TRACE(("wl%d: wlc_bmac_core_phy_clk: clk %d\n", wlc_hw->unit, clk));

	wlc_hw->phyclk = clk;

	if (OFF == clk) {	/* clear gmode bit, put phy into reset */

		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_FGC | SICF_GMODE),
			(SICF_PRST | SICF_FGC));
		OSL_DELAY(1);
		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_FGC), SICF_PRST);
		OSL_DELAY(1);

	} else {		/* take phy out of reset */

		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_FGC), SICF_FGC);
		OSL_DELAY(1);
		si_core_cflags(wlc_hw->sih, (SICF_FGC), 0);
		OSL_DELAY(1);

	}
}

/* Perform a soft reset of the PHY PLL */
void
wlc_bmac_core_phypll_reset(wlc_hw_info_t *wlc_hw)
{
	WL_TRACE(("wl%d: wlc_bmac_core_phypll_reset\n", wlc_hw->unit));

	si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_addr), ~0, 0);
	OSL_DELAY(1);
	si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data), 0x4, 0);
	OSL_DELAY(1);
	si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data), 0x4, 4);
	OSL_DELAY(1);
	si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data), 0x4, 0);
	OSL_DELAY(1);
}

/* light way to turn on phy clock without reset for NPHY and HTPHY only
 *  refer to wlc_bmac_core_phy_clk for full version
 */
void
wlc_bmac_phyclk_fgc(wlc_hw_info_t *wlc_hw, bool clk)
{
	/* support(necessary for NPHY and HTPHY) only */
	if (!WLCISNPHY(wlc_hw->band) && !WLCISHTPHY(wlc_hw->band))
		return;

	if (ON == clk)
		si_core_cflags(wlc_hw->sih, SICF_FGC, SICF_FGC);
	else
		si_core_cflags(wlc_hw->sih, SICF_FGC, 0);

}

void
wlc_bmac_macphyclk_set(wlc_hw_info_t *wlc_hw, bool clk)
{
	if (ON == clk)
		si_core_cflags(wlc_hw->sih, SICF_MPCLKE, SICF_MPCLKE);
	else
		si_core_cflags(wlc_hw->sih, SICF_MPCLKE, 0);
}

void
wlc_bmac_phy_reset(wlc_hw_info_t *wlc_hw)
{
	wlc_phy_t *pih = wlc_hw->band ? wlc_hw->band->pi: NULL;
	uint32 phy_bw_clkbits;
	bool phy_in_reset = FALSE;

	WL_TRACE(("wl%d: wlc_bmac_phy_reset\n", wlc_hw->unit));

	if (pih == NULL)
		return;

	phy_bw_clkbits = wlc_phy_clk_bwbits(wlc_hw->band->pi);

	/* Specfic reset sequence required for NPHY rev 3 and 4 */
	if (WLCISNPHY(wlc_hw->band) && NREV_GE(wlc_hw->band->phyrev, 3) &&
		NREV_LE(wlc_hw->band->phyrev, 4)) {
		/* Set the PHY bandwidth */
		si_core_cflags(wlc_hw->sih, SICF_BWMASK, phy_bw_clkbits);

		OSL_DELAY(1);

		/* Perform a soft reset of the PHY PLL */
		wlc_bmac_core_phypll_reset(wlc_hw);

		/* reset the PHY */
		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_PCLKE),
			(SICF_PRST | SICF_PCLKE));
		phy_in_reset = TRUE;
	} else if (WLCISSSLPNPHY(wlc_hw->band)) {
		if ((SSLPNREV_IS(wlc_hw->band->phyrev, 2)) ||
			(SSLPNREV_IS(wlc_hw->band->phyrev, 4))) {

			/* For sslpnphy 40MHz bw program the pll */
			si_core_cflags(wlc_hw->sih, SICF_BWMASK, phy_bw_clkbits);

			if (phy_bw_clkbits == SICF_BW40) {
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL1,
					0xff000000, 0x09000000);
				OSL_DELAY(5);
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL2,
					0x0000ffff, 0x0000120C);
				OSL_DELAY(5);
			} else if (phy_bw_clkbits == SICF_BW20) {
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL1,
					0xff000000, 0x12000000);
				OSL_DELAY(5);
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL2,
					0x0000ffff, 0x00001212);
				OSL_DELAY(5);
			} else if (phy_bw_clkbits == SICF_BW10) {
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL1,
					0xff000000, 0x24000000);
				OSL_DELAY(5);
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL2,
					0x0000ffff, 0x00002424);
				OSL_DELAY(5);
			}

			/* update the pll settings now */
			si_pmu_pllupd(wlc_hw->sih);
			OSL_DELAY(5);

			si_pmu_chipcontrol(wlc_hw->sih, PMU1_PLL0_CHIPCTL0, 0x40000000, 0x40000000);
			OSL_DELAY(5);
			si_pmu_chipcontrol(wlc_hw->sih, PMU1_PLL0_CHIPCTL0, 0x40000000, 0);
		} else if (SSLPNREV_IS(wlc_hw->band->phyrev, 3)) {
			si_corereg(wlc_hw->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, min_res_mask), (1<<6), (0<< 6));
				OSL_DELAY(100);
			si_corereg(wlc_hw->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, max_res_mask), (1<<6), (0<< 6));
				OSL_DELAY(100);

			if (phy_bw_clkbits == SICF_BW40) {
				si_pmu_pllcontrol(wlc_hw->sih,
					PMU7_PLL_PLLCTL7, 0xff000000, 0x06000000);
				OSL_DELAY(100);
				si_pmu_pllcontrol(wlc_hw->sih,
					PMU7_PLL_PLLCTL8, 0x0000ffff, 0x00000c08);
				OSL_DELAY(100);
			} else if (phy_bw_clkbits == SICF_BW20) {
				si_pmu_pllcontrol(wlc_hw->sih,
					PMU7_PLL_PLLCTL7, 0xff000000, 0x0c000000);
				OSL_DELAY(100);
				si_pmu_pllcontrol(wlc_hw->sih,
					PMU7_PLL_PLLCTL8, 0x0000ffff, 0x00000c0c);
				OSL_DELAY(100);
			} else if (phy_bw_clkbits == SICF_BW10) {
				si_pmu_pllcontrol(wlc_hw->sih,
					PMU7_PLL_PLLCTL7, 0xff000000, 0x18000000);
				OSL_DELAY(100);
				si_pmu_pllcontrol(wlc_hw->sih,
					PMU7_PLL_PLLCTL8, 0x0000ffff, 0x00001818);
				OSL_DELAY(100);
			}

			si_pmu_pllcontrol(wlc_hw->sih,
				PMU7_PLL_PLLCTL11, 0xffffff00, 0x22222200);
			OSL_DELAY(100);

			/* update the pll settings now */
			si_pmu_pllupd(wlc_hw->sih);
			OSL_DELAY(100);

			si_corereg(wlc_hw->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, min_res_mask), (1<<6), (1<< 6));
				OSL_DELAY(100);
			si_corereg(wlc_hw->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, max_res_mask), (1<<6), (1<< 6));
				OSL_DELAY(100);
		}

		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_PCLKE | SICF_BWMASK),
			(SICF_PRST | SICF_PCLKE | phy_bw_clkbits));
			OSL_DELAY(100);
	} else {

		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_PCLKE | SICF_BWMASK),
			(SICF_PRST | SICF_PCLKE | phy_bw_clkbits));
	}

	OSL_DELAY(2);
	wlc_bmac_core_phy_clk(wlc_hw, ON);

	if (pih)
		wlc_phy_anacore(pih, ON);
}

/* switch to and initialize new band */
static void
WLBANDINITFN(wlc_bmac_setband)(wlc_hw_info_t *wlc_hw, uint bandunit, chanspec_t chanspec)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	uint32 macintmask;

	ASSERT(NBANDS_HW(wlc_hw) > 1);
	ASSERT(bandunit != wlc_hw->band->bandunit);

	/* Enable the d11 core before accessing it */
	if (!si_iscoreup(wlc_hw->sih)) {
		si_core_reset(wlc_hw->sih, 0, 0);
		ASSERT(si_iscoreup(wlc_hw->sih));
		wlc_mctrl_reset(wlc_hw);
	}

	macintmask = wlc_setband_inact(wlc_hw, bandunit);

	if (!wlc_hw->up)
		return;

	/* FREF: switch the pll frequency reference for abg phy */
	if ((WLCISAPHY(wlc_hw->band) || WLCISGPHY(wlc_hw->band)) &&
		D11REV_GT(wlc_hw->corerev, 4)) {
		wlc_bmac_core_phyclk_abg_switch(wlc_hw);
	}

	wlc_bmac_core_phy_clk(wlc_hw, ON);

	/* band-specific initializations */
	wlc_bmac_bsinit(wlc_hw, chanspec);

	/*
	 * If there are any pending software interrupt bits,
	 * then replace these with a harmless nonzero value
	 * so wlc_dpc() will re-enable interrupts when done.
	 */
	if (wlc->macintstatus)
		wlc->macintstatus = MI_DMAINT;

	/* restore macintmask */
	wl_intrsrestore(wlc->wl, macintmask);

	/* ucode should still be suspended.. */
	ASSERT((R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC) == 0);
}

/* low-level band switch utility routine */
void
WLBANDINITFN(wlc_setxband)(wlc_hw_info_t *wlc_hw, uint bandunit)
{
	WL_TRACE(("wl%d: wlc_setxband: bandunit %d\n", wlc_hw->unit, bandunit));

	wlc_hw->band = wlc_hw->bandstate[bandunit];

	/* BMAC_NOTE: until we eliminate need for wlc->band refs in low level code */
	wlc_hw->wlc->band = wlc_hw->wlc->bandstate[bandunit];

	/* set gmode core flag */
	if (wlc_hw->sbclk && !wlc_hw->noreset) {
		si_core_cflags(wlc_hw->sih, SICF_GMODE, ((bandunit == 0) ? SICF_GMODE : 0));
	}
}

static bool
BCMATTACHFN(wlc_isgoodchip)(wlc_hw_info_t *wlc_hw)
{
	/* reject some 4306 package/device combinations */
	if ((CHIPID(wlc_hw->sih->chip) == BCM4306_CHIP_ID) &&
	    (CHIPREV(wlc_hw->sih->chiprev) > 2)) {
		/* 4309 is recognized by a pkg option */
		if (((wlc_hw->deviceid == BCM4306_D11A_ID) ||
		     (wlc_hw->deviceid == BCM4306_D11DUAL_ID)) &&
		    (wlc_hw->sih->chippkg != BCM4309_PKG_ID))
			return FALSE;
	}
	/* reject 4311 A0 device */
	if ((CHIPID(wlc_hw->sih->chip) == BCM4311_CHIP_ID) &&
	    (CHIPREV(wlc_hw->sih->chiprev) == 0)) {
		WL_ERROR(("4311A0 is not supported\n"));
		return FALSE;
	}

	/* reject unsupported corerev */
	if (!VALID_COREREV(wlc_hw->corerev)) {
		WL_ERROR(("unsupported core rev %d\n", wlc_hw->corerev));
		return FALSE;
	}

	return TRUE;
}

static bool
BCMATTACHFN(wlc_validboardtype)(wlc_hw_info_t *wlc_hw)
{
	bool goodboard = TRUE;
	uint boardtype = wlc_hw->sih->boardtype;
	uint boardrev = wlc_hw->boardrev;

	if (boardrev == 0)
		goodboard = FALSE;
	else if (boardrev > 0xff) {
		uint brt = (boardrev & 0xf000) >> 12;
		uint b0 = (boardrev & 0xf00) >> 8;
		uint b1 = (boardrev & 0xf0) >> 4;
		uint b2 = boardrev & 0xf;

		if ((brt > 2) || (brt == 0) || (b0 > 9) || (b0 == 0) || (b1 > 9) || (b2 > 9))
			goodboard = FALSE;
	}

	if (wlc_hw->sih->boardvendor != VENDOR_BROADCOM)
		return goodboard;

	if ((boardtype == BCM94306MP_BOARD) || (boardtype == BCM94306CB_BOARD)) {
		if (boardrev < 0x40)
			goodboard = FALSE;
	} else if (boardtype == BCM94309MP_BOARD) {
		goodboard = FALSE;
	} else if (boardtype == BCM94309G_BOARD) {
		if (boardrev < 0x51)
			goodboard = FALSE;
	}
	return goodboard;
}

static char *
BCMINITFN(wlc_get_macaddr)(wlc_hw_info_t *wlc_hw)
{
	const char *varname = "macaddr";
	char *macaddr;

	/* If macaddr exists, use it (Sromrev4, CIS, ...). */
	if ((macaddr = getvar(wlc_hw->vars, varname)) != NULL)
		return macaddr;

#ifndef BCMSMALL
	/*
	 * Take care of our legacy: MAC addresses can not change
	 * during sw upgrades!
	 * 4309B0 dualband:  il0macaddr
	 * other  dualband:  et1macaddr
	 * uniband-A cards:  et1macaddr
	 * else:             il0macaddr
	 */
	if ((CHIPID(wlc_hw->sih->chip) == BCM4306_CHIP_ID) &&
	    (CHIPREV(wlc_hw->sih->chiprev) == 2) && (NBANDS_HW(wlc_hw) > 1))
		varname = "il0macaddr";
	else if (NBANDS_HW(wlc_hw) > 1)
		varname = "et1macaddr";
	else if (WLCISAPHY(wlc_hw->band))
		varname = "et1macaddr";
	else
		varname = "il0macaddr";

	if ((macaddr = getvar(wlc_hw->vars, varname)) == NULL) {
		WL_ERROR(("wl%d: wlc_get_macaddr: macaddr getvar(%s) not found\n",
			wlc_hw->unit, varname));
	}
#endif /* !BCMSMALL */

	return macaddr;
}

/*
 * Return TRUE if radio is disabled, otherwise FALSE.
 * hw radio disable signal is an external pin, users activate it asynchronously
 * this function could be called when driver is down and w/o clock
 * it operates on different registers depending on corerev and boardflag.
 */
bool
wlc_bmac_radio_read_hwdisabled(wlc_hw_info_t* wlc_hw)
{
	bool v, clk, xtal;
	uint32 resetbits = 0, flags = 0;

	xtal = wlc_hw->sbclk;
	if (!xtal)
		wlc_bmac_xtal(wlc_hw, ON);

	/* may need to take core out of reset first */
	clk = wlc_hw->clk;
	if (!clk) {
		if (D11REV_LE(wlc_hw->corerev, 11))
			resetbits |= SICF_PCLKE;

		/*
		 * corerev >= 18, mac no longer enables phyclk automatically when driver accesses
		 * phyreg throughput mac. This can be skipped since only mac reg is accessed below
		 */
		if (D11REV_GE(wlc_hw->corerev, 18))
			flags |= SICF_PCLKE;

		/* AI chip doesn't restore bar0win2 on hibernation/resume, need sw fixup */
		if ((CHIPID(wlc_hw->sih->chip) == BCM43224_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43225_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43421_CHIP_ID))
			wlc_hw->regs = (d11regs_t *)si_setcore(wlc_hw->sih, D11_CORE_ID, 0);
		si_core_reset(wlc_hw->sih, flags, resetbits);
		wlc_mctrl_reset(wlc_hw);
	}

	v = ((R_REG(wlc_hw->osh, &wlc_hw->regs->phydebug) & PDBG_RFD) != 0);

	/* put core back into reset */
	if (!clk)
		si_core_disable(wlc_hw->sih, 0);

	if (!xtal)
		wlc_bmac_xtal(wlc_hw, OFF);

	return (v);
}

/* Initialize just the hardware when coming out of POR or S3/S5 system states */
void
BCMINITFN(wlc_bmac_hw_up)(wlc_hw_info_t *wlc_hw)
{
	if (wlc_hw->wlc->pub->hw_up)
		return;

	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
		si_ldo_war(wlc_hw->sih, wlc_hw->sih->chip);

	/*
	 * Enable pll and xtal, initialize the power control registers,
	 * and force fastclock for the remainder of wlc_up().
	 */
	wlc_bmac_xtal(wlc_hw, ON);
	si_clkctl_init(wlc_hw->sih);
	wlc_clkctl_clk(wlc_hw, CLK_FAST);

	/* Init BTC related GPIOs to clean state on power up as well. This must
	 * be done here as even if radio is disabled, driver needs to
	 * make sure that output GPIO is lowered
	 */
	wlc_bmac_btc_gpio_disable(wlc_hw);

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		/* HW up(initial load, post hibernation resume), core init/fixup */

		si_pci_fixcfg(wlc_hw->sih);

		/* AI chip doesn't restore bar0win2 on hibernation/resume, need sw fixup */
		if ((CHIPID(wlc_hw->sih->chip) == BCM43224_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43225_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43421_CHIP_ID))
			wlc_hw->regs = (d11regs_t *)si_setcore(wlc_hw->sih, D11_CORE_ID, 0);

		if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID)
			si_clk_pmu_htavail_set(wlc_hw->sih, FALSE);
	}

#ifdef WLLED
	wlc_bmac_led_hw_init(wlc_hw);
#endif

	/* Inform phy that a POR reset has occurred so it does a complete phy init */
	wlc_phy_por_inform(wlc_hw->band->pi);

	wlc_hw->ucode_loaded = FALSE;
	wlc_hw->wlc->pub->hw_up = TRUE;
	/* 4313 EPA fix. In case of standalone btd, GPIO is used to turn ON the ePA */ 
	/* In case if BT Combo (epa/ipa), GPIO is used for controling the BT s/w */
	if ((CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID)) {
		if ((wlc_hw->boardflags & BFL_FEM)) {
			if (!(wlc_hw->boardrev >= 0x1250 && (wlc_hw->boardflags & BFL_FEM_BT)))
				si_epa_4313war(wlc_hw->sih);
			else
				si_btcombo_p250_4313_war(wlc_hw->sih);
		} else {
			if (wlc_hw->boardflags & BFL_FEM_BT)
				si_btcombo_p250_4313_war(wlc_hw->sih);
		}
	}

#if defined(AP) && defined(WLC_LOW_ONLY)
	wlc_bmac_pa_war_set(wlc_hw, TRUE);
#endif

}

static bool
wlc_dma_rxreset(wlc_hw_info_t *wlc_hw, uint fifo)
{
	hnddma_t *di = wlc_hw->di[fifo];
	osl_t *osh;

	if (D11REV_LT(wlc_hw->corerev, 12)) {
		bool rxidle = TRUE;
		uint16 rcv_frm_cnt = 0;

		osh = wlc_hw->osh;

		W_REG(osh, &wlc_hw->regs->rcv_fifo_ctl, fifo << 8);
		SPINWAIT((!(rxidle = dma_rxidle(di))) &&
		         ((rcv_frm_cnt = R_REG(osh, &wlc_hw->regs->rcv_frm_cnt)) != 0), 50000);

		if (!rxidle && (rcv_frm_cnt != 0))
			WL_ERROR(("wl%d: %s: rxdma[%d] not idle && rcv_frm_cnt(%d) not zero\n",
			          wlc_hw->unit, __FUNCTION__, fifo, rcv_frm_cnt));
#if defined(DSLCPE_DELAY)
		OSL_YIELD_EXEC(osh, 2000);
#else
		OSL_DELAY(2000);
#endif
	}

	return (dma_rxreset(di));
}

/* d11 core reset
 *   ensure fask clock during reset
 *   reset dma
 *   reset d11(out of reset)
 *   reset phy(out of reset)
 *   clear software macintstatus for fresh new start
 * one testing hack wlc_hw->noreset will bypass the d11/phy reset
 */
void
BCMINITFN(wlc_bmac_corereset)(wlc_hw_info_t *wlc_hw, uint32 flags)
{
	d11regs_t *regs;
	uint i;
	bool fastclk;
	uint32 resetbits = 0;

	if (flags == WLC_USE_COREFLAGS)
		flags = (wlc_hw->band->pi ? wlc_hw->band->core_flags : 0);

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	regs = wlc_hw->regs;

	/* request FAST clock if not on  */
	if (!(fastclk = wlc_hw->forcefastclk))
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

	/* reset the dma engines except if core is in reset (first time thru or bigger hammer) */
	if (si_iscoreup(wlc_hw->sih)) {
		if (!PIO_ENAB(wlc_hw->wlc->pub)) {
			for (i = 0; i < NFIFO; i++)
				if ((wlc_hw->di[i]) && (!dma_txreset(wlc_hw->di[i]))) {
					WL_ERROR(("wl%d: %s: dma_txreset[%d]: cannot stop dma\n",
					          wlc_hw->unit, __FUNCTION__, i));
				}

			if ((wlc_hw->di[RX_FIFO]) && (!wlc_dma_rxreset(wlc_hw, RX_FIFO))) {
				WL_ERROR(("wl%d: %s: dma_rxreset[%d]: cannot stop dma\n",
				          wlc_hw->unit, __FUNCTION__, RX_FIFO));
			}
			if (D11REV_IS(wlc_hw->corerev, 4) && wlc_hw->di[RX_TXSTATUS_FIFO] &&
			    (!wlc_dma_rxreset(wlc_hw, RX_TXSTATUS_FIFO))) {
				WL_ERROR(("wl%d: %s: dma_rxreset[%d]: cannot stop dma\n",
				          wlc_hw->unit, __FUNCTION__, RX_TXSTATUS_FIFO));
			}
		} else {
			for (i = 0; i < NFIFO; i++)
				if (wlc_hw->pio[i])
					wlc_pio_reset(wlc_hw->pio[i]);
		}
	}
	/* if noreset, just stop the psm and return */
	if (wlc_hw->noreset) {
		wlc_hw->wlc->macintstatus = 0;	/* skip wl_dpc after down */
		wlc_bmac_mctrl(wlc_hw, MCTL_PSM_RUN | MCTL_EN_MAC, 0);
		return;
	}

	if (D11REV_LE(wlc_hw->corerev, 11))
		resetbits |= SICF_PCLKE;

	/*
	 * corerev >= 18, mac no longer enables phyclk automatically when driver accesses phyreg
	 * throughput mac, AND phy_reset is skipped at early stage when band->pi is invalid
	 * need to enable PHY CLK
	 */
	if (D11REV_GE(wlc_hw->corerev, 18))
		flags |= SICF_PCLKE;

	/* reset the core
	 * In chips with PMU, the fastclk request goes through d11 core reg 0x1e0, which
	 *  is cleared by the core_reset. have to re-request it.
	 *  This adds some delay and we can optimize it by also requesting fastclk through
	 *  chipcommon during this period if necessary. But that has to work coordinate
	 *  with other driver like mips/arm since they may touch chipcommon as well.
	 */
	wlc_hw->clk = FALSE;
	si_core_reset(wlc_hw->sih, flags, resetbits);
	wlc_hw->clk = TRUE;
	if (wlc_hw->band && wlc_hw->band->pi)
		wlc_phy_hw_clk_state_upd(wlc_hw->band->pi, TRUE);

	wlc_mctrl_reset(wlc_hw);

	if (PMUCTL_ENAB(wlc_hw->sih))
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

	wlc_bmac_phy_reset(wlc_hw);

	/* turn on PHY_PLL */
	wlc_bmac_core_phypll_ctl(wlc_hw, TRUE);

	/* clear sw intstatus */
	wlc_hw->wlc->macintstatus = 0;

	/* restore the clk setting */
	if (!fastclk)
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);
}

/* Search mem rw utilities */

#ifdef MBSS
bool
wlc_bmac_ucodembss_hwcap(wlc_hw_info_t *wlc_hw)
{
	/* add up template space here */
	int templ_ram_sz, fifo_mem_used, i, stat;
	uint blocks = 0;

	for (fifo_mem_used = 0, i = 0; i < NFIFO; i++) {
		stat = wlc_bmac_xmtfifo_sz_get(wlc_hw, i, &blocks);
		if (stat != 0) return FALSE;
		fifo_mem_used += blocks;
	}

	templ_ram_sz = ((wlc_hw->machwcap & MCAP_TXFSZ_MASK) >> MCAP_TXFSZ_SHIFT) * 2;

	if ((templ_ram_sz - fifo_mem_used) < (int)MBSS_TPLBLKS(wlc_hw->wlc->max_ap_bss)) {
#ifdef WLC_HIGH
		WL_ERROR(("wl%d: %s: Insuff mem for MBSS: templ memblks %d fifo memblks %d\n",
			wlc_hw->wlc->pub->unit, __FUNCTION__, templ_ram_sz, fifo_mem_used));
#endif /* WLC_HIGH */
		return FALSE;
	}

	return TRUE;
}
#endif /* MBSS */

/* If the ucode that supports corerev 5 is used for corerev 9 and above,
 * txfifo sizes needs to be modified(increased) since the newer cores
 * have more memory.
 */
static void
BCMINITFN(wlc_corerev_fifofixup)(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs = wlc_hw->regs;
	uint16 fifo_nu;
	uint16 txfifo_startblk = TXFIFO_START_BLK, txfifo_endblk;
	uint16 txfifo_def, txfifo_def1;
	uint16 txfifo_cmd;
	osl_t *osh;

	if (D11REV_LT(wlc_hw->corerev, 9))
		goto exit;

	/* Re-assign the space for tx fifos to allow BK aggregation */
	if (D11REV_IS(wlc_hw->corerev, 28)) {
		uint16 xmtsz[] = { 30, 47, 22, 14, 8, 1 };

		memcpy(xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)],
		       xmtsz, sizeof(xmtsz));
	} else if (D11REV_IS(wlc_hw->corerev, 16) || D11REV_IS(wlc_hw->corerev, 17)) {
		uint16 xmtsz[] = { 98, 159, 160, 21, 8, 1 };

		memcpy(xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)],
		       xmtsz, sizeof(xmtsz));
	}

	/* tx fifos start at TXFIFO_START_BLK from the Base address */
#ifdef MBSS
	if (D11REV_ISMBSS16(wlc_hw->corerev)) {
#if defined(WLLPRS) || defined(WLC_HIGH)
		wlc_info_t *wlc = wlc_hw->wlc;
#endif

		/* 4313 has total fifo space of 128 blocks. if we enable
		 * all 16 MBSSs we will not be left with enough fifo space to
		 * support max thru'put. so we only allow configuring/enabling
		 * max of 4 BSSs. Rest of the space is distributed acorss
		 * the tx fifos.
		 * 43228 has 256 blocks of fifo space. reducing BE fifo size from 
		 * 192 to 127 for 16 MBSS support.
		 */
		if (D11REV_IS(wlc_hw->corerev, 24)) {
			uint16 xmtsz[] = { 9, 47, 22, 14, 14, 5 };

			memcpy(xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)],
			       xmtsz, sizeof(xmtsz));
		} else if (D11REV_IS(wlc_hw->corerev, 30)) {
			uint16 xmtsz[] = { 9, 127, 22, 14, 14, 5 };

			memcpy(xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)],
				xmtsz, sizeof(xmtsz));
		}

#ifdef WLLPRS
		/* To support legacy prs of size > 256bytes, reduce the no. of
		 * bss supported to 8.
		 */
		if (D11REV_IS(wlc_hw->corerev, 16) || D11REV_IS(wlc_hw->corerev, 17) ||
			D11REV_IS(wlc_hw->corerev, 22)) {

			wlc->max_ap_bss = 8;
			wlc->pub->tunables->maxucodebss = 8;
			wlc->mbss_ucidx_mask = wlc->max_ap_bss - 1;
		}

		/* tell ucode the lprs size is 0x80 * 4bytes. */
		wlc_write_shm(wlc, SHM_MBSS_BC_FID2, 0x80);
#endif /* WLLPRS */

#ifdef WLC_HIGH
		if (MBSS_ENAB(wlc->pub)) {
#endif
			if (wlc_bmac_ucodembss_hwcap(wlc_hw)) {
				ASSERT(wlc_hw->wlc->max_ap_bss > 0);
				txfifo_startblk = MBSS_TXFIFO_START_BLK(wlc_hw->wlc->max_ap_bss);
			}
#ifdef WLC_HIGH
		}
#endif
	} else
#endif /* MBSS */
	txfifo_startblk = TXFIFO_START_BLK;

	osh = wlc_hw->osh;

	/* sequence of operations:  reset fifo, set fifo size, reset fifo */
	for (fifo_nu = 0; fifo_nu < NFIFO; fifo_nu++) {

		txfifo_endblk = txfifo_startblk + wlc_hw->xmtfifo_sz[fifo_nu];
		txfifo_def = (txfifo_startblk & 0xff) |
			(((txfifo_endblk - 1) & 0xff) << TXFIFO_FIFOTOP_SHIFT);
		txfifo_def1 = ((txfifo_startblk >> 8) & 0x3) |
			((((txfifo_endblk - 1) >> 8) & 0x3) << TXFIFO_FIFOTOP_SHIFT);
		txfifo_cmd = TXFIFOCMD_RESET_MASK | (fifo_nu << TXFIFOCMD_FIFOSEL_SHIFT);

		W_REG(osh, &regs->xmtfifocmd, txfifo_cmd);
		W_REG(osh, &regs->xmtfifodef, txfifo_def);
		if (D11REV_GE(wlc_hw->corerev, 16))
			W_REG(osh, &regs->xmtfifodef1, txfifo_def1);

		W_REG(osh, &regs->xmtfifocmd, txfifo_cmd);

		txfifo_startblk += wlc_hw->xmtfifo_sz[fifo_nu];
	}
exit:
	/* need to propagate to shm location to be in sync since ucode/hw won't do this */
	wlc_bmac_write_shm(wlc_hw, M_FIFOSIZE0, wlc_hw->xmtfifo_sz[TX_AC_BE_FIFO]);
	wlc_bmac_write_shm(wlc_hw, M_FIFOSIZE1, wlc_hw->xmtfifo_sz[TX_AC_VI_FIFO]);
	wlc_bmac_write_shm(wlc_hw, M_FIFOSIZE2, ((wlc_hw->xmtfifo_sz[TX_AC_VO_FIFO] << 8) |
		wlc_hw->xmtfifo_sz[TX_AC_BK_FIFO]));
	wlc_bmac_write_shm(wlc_hw, M_FIFOSIZE3, ((wlc_hw->xmtfifo_sz[TX_ATIM_FIFO] << 8) |
		wlc_hw->xmtfifo_sz[TX_BCMC_FIFO]));
}

/* d11 core init
 *   reset PSM
 *   download ucode/PCM
 *   let ucode run to suspended
 *   download ucode inits
 *   config other core registers
 *   init dma/pio
 */
static void
BCMINITFN(wlc_coreinit)(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	d11regs_t *regs;
	uint32 sflags;
	uint bcnint_us;
	uint i = 0;
	bool fifosz_fixup = FALSE;
	osl_t *osh;
	int err = 0;
	uint16 buf[NFIFO];

	regs = wlc_hw->regs;
	osh = wlc_hw->osh;

	WL_TRACE(("wl%d: wlc_coreinit\n", wlc_hw->unit));

	/* reset PSM */
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_PSM_JMP_0 | MCTL_WAKE));

	/* make sure 2-wire or 3-wire decision has been made */
	ASSERT((wlc_hw->btc->wire >= WL_BTC_2WIRE) || (wlc_hw->btc->wire <= WL_BTC_4WIRE));

	/* Configure selected BTC mode */
	wlc_bmac_btc_mode_set(wlc_hw, wlc_hw->btc->mode);

	if (wlc_hw->boardflags2 & BFL2_BTC3WIRE) {
		if ((CHIPID(wlc->pub->sih->chip) == BCM4313_CHIP_ID) ||
		    (CHIPID(wlc->pub->sih->chip) == BCM4331_CHIP_ID) ||
		    (CHIPID(wlc->pub->sih->chip) == BCM43431_CHIP_ID))
			si_btc_enable_chipcontrol(wlc_hw->sih);

		if (CHIPID(wlc->pub->sih->chip) == BCM4313_CHIP_ID) {
			if (wlc_bmac_btc_mode_get(wlc_hw))
				wlc_phy_btclock_war(wlc_hw->band->pi, wlc_hw->btclock_tune_war);
		}
	}

	/* Pin muxing changes for BT coex operation in LCNXNPHY */	
	if (((CHIPID(wlc->pub->sih->chip) == BCM43131_CHIP_ID) ||
		(CHIPID(wlc->pub->sih->chip) == BCM43217_CHIP_ID) ||
		(CHIPID(wlc->pub->sih->chip) == BCM43227_CHIP_ID) ||
		(CHIPID(wlc->pub->sih->chip) == BCM43228_CHIP_ID) ||
		(CHIPID(wlc->pub->sih->chip) == BCM43428_CHIP_ID)) &&
		(wlc_hw->boardflags2 & BFL2_BTC3WIRE)) {
		si_btc_enable_chipcontrol(wlc_hw->sih);
		si_pmu_chipcontrol(wlc_hw->sih, PMU1_PLL0_CHIPCTL1, 0x10, 0x10);
	}

	if (wlc_hw->boardflags & BFL_BTCOEX) {
		if (wlc_hw->boardflags2 & BFL2_BTC3WIRE)
			si_seci_init(wlc_hw->sih, SECI_MODE_LEGACY_3WIRE_WLAN);
		else
			si_seci_init(wlc_hw->sih, SECI_MODE_SECI);
	}
#ifndef DONGLEBUILD
	wlc_ucode_download(wlc_hw);
#endif
	/*
	 * FIFOSZ fixup
	 * 1) core5-9 use ucode 5 to save space since the PSM is the same
	 * 2) newer chips, driver wants to controls the fifo allocation
	 */
	if (D11REV_GE(wlc_hw->corerev, 4))
		fifosz_fixup = TRUE;

	/* write the PCM ucode for cores supporting AES (via the PCM) */
	if (D11REV_IS(wlc_hw->corerev, 4))
		wlc_ucode_pcm_write(wlc_hw, d11pcm4, d11pcm4sz);
	else if (D11REV_LT(wlc_hw->corerev, 11))
		wlc_ucode_pcm_write(wlc_hw, d11pcm5, d11pcm5sz);

	/* let the PSM run to the suspended state, set mode to BSS STA */
	W_REG(osh, &regs->macintstatus, -1);
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_INFRA | MCTL_PSM_RUN | MCTL_WAKE));

	/* wait for ucode to self-suspend after auto-init */
	SPINWAIT(((R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD) == 0), 1000 * 1000);
	if ((R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD) == 0)
		WL_ERROR(("wl%d: wlc_coreinit: ucode did not self-suspend!\n", wlc_hw->unit));


	wlc_gpio_init(wlc_hw);

#ifdef WL11N
	/* REV8+: mux out 2o3 control lines when 3 antennas are available */
	if (wlc_hw->antsel_avail) {
		if ((wlc_hw->sih->chip == BCM43234_CHIP_ID) ||
		    (wlc_hw->sih->chip == BCM43235_CHIP_ID) ||
		    (wlc_hw->sih->chip == BCM43236_CHIP_ID) ||
		    (wlc_hw->sih->chip == BCM43238_CHIP_ID)) {
			si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
				CCTRL43236_ANT_MUX_2o3, CCTRL43236_ANT_MUX_2o3);
		} else if (wlc_hw->sih->chip == BCM43237_CHIP_ID) {
			si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
				CCTRL43237_ANT_MUX_2o3, CCTRL43237_ANT_MUX_2o3);
		} else if (((CHIPID(wlc_hw->sih->chip)) == BCM5357_CHIP_ID) ||
		           ((CHIPID(wlc_hw->sih->chip)) == BCM4749_CHIP_ID) ||
		           ((CHIPID(wlc_hw->sih->chip)) == BCM53572_CHIP_ID)) {
			si_pmu_chipcontrol(wlc_hw->sih, 1, CCTRL5357_ANT_MUX_2o3,
				CCTRL5357_ANT_MUX_2o3);
		}
	}
#endif	/* WL11N */

	sflags = si_core_sflags(wlc_hw->sih, 0, 0);

	/* init IHR, SHM, and SCR */
	if (D11REV_IS(wlc_hw->corerev, 31)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ht0initvals29);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 30)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n16initvals30);
	} else if (D11REV_IS(wlc_hw->corerev, 29)) {
		if (WLCISHTPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ht0initvals29);
		} else
			WL_ERROR(("wl%d: unsupported phy in corerev 26 \n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 26)) {
		if (WLCISHTPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 26))
				wlc_write_inits(wlc_hw, d11ht0initvals26);
			else if (D11REV_IS(wlc_hw->corerev, 29))
				wlc_write_inits(wlc_hw, d11ht0initvals29);
		} else
			WL_ERROR(("wl%d: unsupported phy in corerev 26 \n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 25) || D11REV_IS(wlc_hw->corerev, 28)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11n0initvals25);
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_lcn0initvals25_addr());
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 24)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11n0initvals24);
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_lcn0initvals24_addr());
		} else
			WL_ERROR(("wl%d: unsupported phy in corerev 24 \n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 22)) {
		if (WLCISNPHY(wlc_hw->band)) {
			/* ucode only supports rev23(43224b0) with rev16 ucode */
			if (D11REV_IS(wlc_hw->corerev, 23))
				wlc_write_inits(wlc_hw, d11n0initvals16);
			else
				wlc_write_inits(wlc_hw, d11n0initvals22);
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 22))
				wlc_write_inits(wlc_hw, d11sslpn4initvals22);
			else
				WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 21)) {
		if (WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11sslpn3initvals21);
		}
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 20)) {
		if (WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11sslpn1initvals20);
			WL_ERROR(("wl%d: supported phy in corerev 20\n", wlc_hw->unit));
		}
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 20\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 19)) {
		if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11sslpn2initvals19);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 19\n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 16)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n0initvals16);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11sslpn0initvals16);
		else if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0initvals16);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 15)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0initvals15);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 15\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 14)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0initvals14);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 14\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 13)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0initvals13);
		else if (WLCISGPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11b0g0initvals13);
		else if (WLCISAPHY(wlc_hw->band) && (sflags & SISF_2G_PHY))
			wlc_write_inits(wlc_hw, d11a0g1initvals13);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 13\n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 11)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n0initvals11);
		else
			WL_ERROR(("wl%d: corerev 11 or 12 && ! NPHY\n", wlc_hw->unit));
#if defined(MBSS)
	} else if (D11REV_IS(wlc_hw->corerev, 9)) {
		if (WLCISAPHY(wlc_hw->band)) {
			if (sflags & SISF_2G_PHY)
				wlc_write_inits(wlc_hw, d11a0g1initvals9);
			else
				wlc_write_inits(wlc_hw, d11a0g0initvals9);
		} else
			wlc_write_inits(wlc_hw, d11b0g0initvals9);
#endif
	} else if (D11REV_IS(wlc_hw->corerev, 4)) {
		if (WLCISAPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11a0g0initvals4);
		else
			wlc_write_inits(wlc_hw, d11b0g0initvals4);
	} else {
		if (WLCISAPHY(wlc_hw->band)) {
			if (sflags & SISF_2G_PHY)
				wlc_write_inits(wlc_hw, d11a0g1initvals5);
			else
				wlc_write_inits(wlc_hw, d11a0g0initvals5);
		} else
			wlc_write_inits(wlc_hw, d11b0g0initvals5);
	}

	/* For old ucode, txfifo sizes needs to be modified(increased) for Corerev >= 9 */
	if (fifosz_fixup == TRUE) {
		wlc_corerev_fifofixup(wlc_hw);
	}

	/* check txfifo allocations match between ucode and driver */
	buf[TX_AC_BE_FIFO] = wlc_bmac_read_shm(wlc_hw, M_FIFOSIZE0);
	if (buf[TX_AC_BE_FIFO] != wlc_hw->xmtfifo_sz[TX_AC_BE_FIFO]) {
		i = TX_AC_BE_FIFO;
		err = -1;
	}
	buf[TX_AC_VI_FIFO] = wlc_bmac_read_shm(wlc_hw, M_FIFOSIZE1);
	if (buf[TX_AC_VI_FIFO] != wlc_hw->xmtfifo_sz[TX_AC_VI_FIFO]) {
		i = TX_AC_VI_FIFO;
	        err = -1;
	}
	buf[TX_AC_BK_FIFO] = wlc_bmac_read_shm(wlc_hw, M_FIFOSIZE2);
	buf[TX_AC_VO_FIFO] = (buf[TX_AC_BK_FIFO] >> 8) & 0xff;
	buf[TX_AC_BK_FIFO] &= 0xff;
	if (buf[TX_AC_BK_FIFO] != wlc_hw->xmtfifo_sz[TX_AC_BK_FIFO]) {
		i = TX_AC_BK_FIFO;
	        err = -1;
	}
	if (buf[TX_AC_VO_FIFO] != wlc_hw->xmtfifo_sz[TX_AC_VO_FIFO]) {
		i = TX_AC_VO_FIFO;
		err = -1;
	}
	buf[TX_BCMC_FIFO] = wlc_bmac_read_shm(wlc_hw, M_FIFOSIZE3);
	buf[TX_ATIM_FIFO] = (buf[TX_BCMC_FIFO] >> 8) & 0xff;
	buf[TX_BCMC_FIFO] &= 0xff;
	if (buf[TX_BCMC_FIFO] != wlc_hw->xmtfifo_sz[TX_BCMC_FIFO]) {
		i = TX_BCMC_FIFO;
		err = -1;
	}
	if (buf[TX_ATIM_FIFO] != wlc_hw->xmtfifo_sz[TX_ATIM_FIFO]) {
		i = TX_ATIM_FIFO;
		err = -1;
	}
	if (err != 0) {
		WL_ERROR(("wlc_coreinit: txfifo mismatch: ucode size %d driver size %d index %d\n",
			buf[i], wlc_hw->xmtfifo_sz[i], i));
		/* DO NOT ASSERT corerev < 4 even there is a mismatch
		 * shmem, since driver don't overwrite those chip and
		 * ucode initialize data will be used.
		 */
		if (D11REV_GE(wlc_hw->corerev, 4))
			ASSERT(0);
	}

#ifdef WLAMPDU_HW
	for (i = 0; i < AC_COUNT; i++) {
		wlc_hw->xmtfifo_frmmax[i] =
		        (wlc_hw->xmtfifo_sz[i] * 256 - 1300) / MAX_MPDU_SPACE;
		WL_INFORM(("%s: fifo sz blk %d entries %d\n",
			__FUNCTION__, wlc_hw->xmtfifo_sz[i], wlc_hw->xmtfifo_frmmax[i]));
	}
#endif	/* WLAMPDU_HW */

	/* make sure we can still talk to the mac */
	ASSERT(R_REG(osh, &regs->maccontrol) != 0xffffffff);


	/* band-specific inits done by wlc_bsinit() */

#ifdef MBSS
	/* Set search engine ssid lengths to zero */
	if (D11REV_ISMBSS16(wlc_hw->corerev) && (wlc_bmac_ucodembss_hwcap(wlc_hw) == TRUE)) {
		uint32 start, swplen, idx;

		swplen = 0;
		for (idx = 0; idx < (uint) wlc->pub->tunables->maxucodebss; idx++) {
			start = SHM_MBSS_SSIDSE_BASE_ADDR + (idx * SHM_MBSS_SSIDSE_BLKSZ);
			wlc_bmac_copyto_objmem(wlc_hw, start, &swplen,
				SHM_MBSS_SSIDLEN_BLKSZ, OBJADDR_SRCHM_SEL);
		}
	}
#endif /* MBSS */

	/* Set up frame burst size and antenna swap threshold init values */
#ifdef WLAFTERBURNER
	wlc_bmac_mhf(wlc_hw, MHF1, MHF1_AFTERBURNER,
	     (wlc->afterburner ? MHF1_AFTERBURNER : 0), WLC_BAND_ALL);
	wlc_bmac_write_shm(wlc_hw, M_MBURST_SIZE,
	                  (wlc->afterburner? MAXTXFRAMEBURST_AB : MAXTXFRAMEBURST));
	wlc_bmac_write_shm(wlc_hw, M_MAX_ANTCNT, (wlc->afterburner? ANTCNT_AB : ANTCNT));
#else
	wlc_bmac_write_shm(wlc_hw, M_MBURST_SIZE, MAXTXFRAMEBURST);
	wlc_bmac_write_shm(wlc_hw, M_MAX_ANTCNT, ANTCNT);
#endif /* WLAFTERBURNER */

	/* enable one rx interrupt per received frame */
	W_REG(osh, &regs->intrcvlazy[0], (1 << IRL_FC_SHIFT));
	if (D11REV_IS(wlc_hw->corerev, 4))
		W_REG(osh, &regs->intrcvlazy[3], (1 << IRL_FC_SHIFT));

	/* set the station mode (BSS STA) */
	wlc_bmac_mctrl(wlc_hw,
	          (MCTL_INFRA | MCTL_DISCARD_PMQ | MCTL_AP),
	          (MCTL_INFRA | MCTL_DISCARD_PMQ));

	if (PIO_ENAB(wlc->pub)) {
		/* set fifo mode for each VALID rx fifo */
		wlc_rxfifo_setpio(wlc_hw);

		for (i = 0; i < NFIFO; i++)
			if (wlc_hw->pio[i])
				wlc_pio_init(wlc_hw->pio[i]);

		/*
		 * For D11 corerev less than 8, the h/w does not store the pad bytes in the
		 * Rx Data FIFO
		*/
		if (D11REV_LT(wlc_hw->corerev, 8))
			wlc_bmac_write_shm(wlc_hw, M_RX_PAD_DATA_OFFSET, 0);

#ifdef IL_BIGENDIAN
		/* enable byte swapping */
		wlc_bmac_mctrl(wlc_hw, MCTL_BIGEND, MCTL_BIGEND);
#endif /* IL_BIGENDIAN */
	}

	/* BMAC_NOTE: Could this just be a ucode init? */
	if (D11REV_ISMBSS4(wlc_hw->corerev)) {
		uint offset = SHM_MBSS_BC_FID0;
		int idx = 0;
		for (idx = 0; idx < wlc->pub->tunables->maxucodebss4; idx++) {
			wlc_bmac_write_shm(wlc_hw, offset, INVALIDFID);
			offset += 2;
		}
	}

	/* set up Beacon interval */
	bcnint_us = 0x8000 << 10;
	W_REG(osh, &regs->tsf_cfprep, (bcnint_us << CFPREP_CBI_SHIFT));
	W_REG(osh, &regs->tsf_cfpstart, bcnint_us);
	W_REG(osh, &regs->macintstatus, MI_GP1);

	/* write interrupt mask */
	W_REG(osh, &regs->intctrlregs[RX_FIFO].intmask, DEF_RXINTMASK);
	if (D11REV_IS(wlc_hw->corerev, 4))
		W_REG(osh, &regs->intctrlregs[RX_TXSTATUS_FIFO].intmask, DEF_RXINTMASK);

	/* allow the MAC to control the PHY clock (dynamic on/off) */
	wlc_bmac_macphyclk_set(wlc_hw, ON);

	/* program dynamic clock control fast powerup delay register */
	if (D11REV_GT(wlc_hw->corerev, 4)) {
		wlc->fastpwrup_dly = si_clkctl_fast_pwrup_delay(wlc_hw->sih);
		W_REG(osh, &regs->scc_fastpwrup_dly, wlc->fastpwrup_dly);
	}

	/* tell the ucode the corerev */
	wlc_bmac_write_shm(wlc_hw, M_MACHW_VER, (uint16)wlc_hw->corerev);

	/* tell the ucode MAC capabilities */
	if (D11REV_GE(wlc_hw->corerev, 13)) {
		wlc_bmac_write_shm(wlc_hw, M_MACHW_CAP_L, (uint16)(wlc_hw->machwcap & 0xffff));
		wlc_bmac_write_shm(wlc_hw, M_MACHW_CAP_H,
			(uint16)((wlc_hw->machwcap >> 16) & 0xffff));
	}

	/* write retry limits to SCR, this done after PSM init */
	W_REG(osh, &regs->objaddr, OBJADDR_SCR_SEL | S_DOT11_SRC_LMT);
	(void)R_REG(osh, &regs->objaddr);
	W_REG(osh, &regs->objdata, wlc_hw->SRL);
	W_REG(osh, &regs->objaddr, OBJADDR_SCR_SEL | S_DOT11_LRC_LMT);
	(void)R_REG(osh, &regs->objaddr);
	W_REG(osh, &regs->objdata, wlc_hw->LRL);

	/* write rate fallback retry limits */
	wlc_bmac_write_shm(wlc_hw, M_SFRMTXCNTFBRTHSD, wlc_hw->SFBL);
	wlc_bmac_write_shm(wlc_hw, M_LFRMTXCNTFBRTHSD, wlc_hw->LFBL);

	if (D11REV_GE(wlc_hw->corerev, 16)) {
		AND_REG(osh, &regs->ifs_ctl, 0x0FFF);
		W_REG(osh, &regs->ifs_aifsn, EDCF_AIFSN_MIN);
	}

	/* dma or pio initializations */
	if (!PIO_ENAB(wlc->pub)) {
		wlc->txpend16165war = 0;

		/* init the tx dma engines */
		for (i = 0; i < NFIFO; i++) {
			if (wlc_hw->di[i])
				dma_txinit(wlc_hw->di[i]);
		}

		/* init the rx dma engine(s) and post receive buffers */
		dma_rxinit(wlc_hw->di[RX_FIFO]);
		dma_rxfill(wlc_hw->di[RX_FIFO]);
		if (D11REV_IS(wlc_hw->corerev, 4)) {
			dma_rxinit(wlc_hw->di[RX_TXSTATUS_FIFO]);
			dma_rxfill(wlc_hw->di[RX_TXSTATUS_FIFO]);
		}
	} else {
		for (i = 0; i < NFIFO; i++) {
			uint tmp = 0;
			if (wlc_pio_txdepthget(wlc_hw->pio[i]) == 0) {
				wlc_pio_txdepthset(wlc_hw->pio[i], (buf[i] << 8));

				tmp = wlc_pio_txdepthget(wlc_hw->pio[i]);
				if ((D11REV_LE(wlc_hw->corerev, 7) ||
				     D11REV_GE(wlc_hw->corerev, 11)) && tmp)
					wlc_pio_txdepthset(wlc_hw->pio[i], tmp - 4);
			}
		}
	}

	if ((CHIPID(wlc_hw->sih->chip) == BCM4716_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM4748_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM47162_CHIP_ID)) {
		/* The value to be written into these registers is (2^26)/(freq)MHz */
		/* MAC clock frequency for 4716 is 125MHz */
		W_REG(osh, &regs->tsf_clk_frac_l, 0x3127);
		W_REG(osh, &regs->tsf_clk_frac_h, 0x8);
	}
	/* initialize btc_params and btc_flags */
	wlc_bmac_btc_param_init(wlc_hw);
}

/* This function is used for changing the tsf frac register
 * If spur avoidance mode is off, the mac freq will be 80/120/160Mhz
 * If spur avoidance mode is on1, the mac freq will be 82/123/164Mhz
 * If spur avoidance mode is on2, the mac freq will be 84/126/168Mhz
 * Formula is 2^26/freq(MHz)
 */

void
wlc_bmac_switch_macfreq(wlc_hw_info_t *wlc_hw, uint8 spurmode)
{
	d11regs_t *regs;
	osl_t *osh;
	regs = wlc_hw->regs;
	osh = wlc_hw->osh;

	/* ??? better keying, corerev, phyrev ??? */
	if ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) {
		if (spurmode == WL_SPURAVOID_ON2) { /* 168MHz */
			W_REG(osh, &regs->tsf_clk_frac_l, 0x1862);
			W_REG(osh, &regs->tsf_clk_frac_h, 0x6);
		} else if (spurmode == WL_SPURAVOID_ON1) { /* 164MHz */
			W_REG(osh, &regs->tsf_clk_frac_l, 0x3E70);
			W_REG(osh, &regs->tsf_clk_frac_h, 0x6);
		} else { /* 160MHz */
			W_REG(osh, &regs->tsf_clk_frac_l, 0x6666);
			W_REG(osh, &regs->tsf_clk_frac_h, 0x6);
		}
	} else if ((CHIPID(wlc_hw->sih->chip) == BCM43222_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43420_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43111_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43112_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43224_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43225_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43421_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43226_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43131_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43217_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43227_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43228_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43428_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43237_CHIP_ID) ||
		(wlc_hw->sih->chip == BCM43234_CHIP_ID) ||
		(wlc_hw->sih->chip == BCM43235_CHIP_ID) ||
		(wlc_hw->sih->chip == BCM43236_CHIP_ID) ||
		(wlc_hw->sih->chip == BCM43238_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM6362_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM5357_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4749_CHIP_ID)) {
		if (spurmode == WL_SPURAVOID_ON2) {	/* 126Mhz */
			W_REG(osh, &regs->tsf_clk_frac_l, 0x2082);
			W_REG(osh, &regs->tsf_clk_frac_h, 0x8);
		} else if (spurmode == WL_SPURAVOID_ON1) {	/* 123Mhz */
			W_REG(osh, &regs->tsf_clk_frac_l, 0x5341);
			W_REG(osh, &regs->tsf_clk_frac_h, 0x8);
		} else {	/* 120Mhz */
			W_REG(osh, &regs->tsf_clk_frac_l, 0x8889);
			W_REG(osh, &regs->tsf_clk_frac_h, 0x8);
		}
	} else if (WLCISLCNPHY(wlc_hw->band)) {
		if (spurmode == WL_SPURAVOID_ON1) {	/* 82Mhz */
			W_REG(osh, &regs->tsf_clk_frac_l, 0x7CE0);
			W_REG(osh, &regs->tsf_clk_frac_h, 0xC);
		} else {	/* 80Mhz */
			W_REG(osh, &regs->tsf_clk_frac_l, 0xCCCD);
			W_REG(osh, &regs->tsf_clk_frac_h, 0xC);
		}
	}
}

/* Initialize GPIOs that are controlled by D11 core */
static void
BCMINITFN(wlc_gpio_init)(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs;
	uint32 gc, gm;
	osl_t *osh;

	regs = wlc_hw->regs;
	osh = wlc_hw->osh;

	/* use GPIO select 0 to get all gpio signals from the gpio out reg */
	wlc_bmac_mctrl(wlc_hw, MCTL_GPOUT_SEL_MASK, 0);



	/*
	 * Common GPIO setup:
	 *	G0 = LED 0 = WLAN Activity
	 *	G1 = LED 1 = WLAN 2.4 GHz Radio State
	 *	G2 = LED 2 = WLAN 5 GHz Radio State
	 *	G4 = radio disable input (HI enabled, LO disabled)
	 * Boards that support BT Coexistence:
	 *	G7 = BTC
	 *	G8 = BTC
	 * Boards with chips that have fewer gpios and support BT Coexistence:
	 *	G4 = BTC
	 *	G5 = BTC
	 */

	gc = gm = 0;

	/* Set/clear GPIOs for BTC */
	if (wlc_hw->btc->gpio_out != 0)
		wlc_bmac_btc_gpio_enable(wlc_hw);

#ifdef WL11N
	/* Allocate GPIOs for mimo antenna diversity feature */
	if (WLANTSEL_ENAB(wlc)) {
		if (wlc_hw->antsel_type == ANTSEL_2x3 || wlc_hw->antsel_type == ANTSEL_1x2_CORE1 ||
			wlc_hw->antsel_type == ANTSEL_1x2_CORE0) {
			/* Enable antenna diversity, use 2x3 mode */
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_ANTSEL_EN, MHF3_ANTSEL_EN, WLC_BAND_ALL);
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_ANTSEL_MODE, MHF3_ANTSEL_MODE,
				WLC_BAND_ALL);

			/* init superswitch control */
			wlc_phy_antsel_init(wlc_hw->band->pi, FALSE);

		} else if (wlc_hw->antsel_type == ANTSEL_2x4) {
			ASSERT((gm & BOARD_GPIO_12) == 0);
			gm |= gc |= (BOARD_GPIO_12 | BOARD_GPIO_13);
			/* The board itself is powered by these GPIOs (when not sending pattern)
			* So set them high
			*/
			OR_REG(osh, &regs->psm_gpio_oe, (BOARD_GPIO_12 | BOARD_GPIO_13));
			OR_REG(osh, &regs->psm_gpio_out, (BOARD_GPIO_12 | BOARD_GPIO_13));

			/* Enable antenna diversity, use 2x4 mode */
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_ANTSEL_EN, MHF3_ANTSEL_EN, WLC_BAND_ALL);
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_ANTSEL_MODE, 0, WLC_BAND_ALL);

			/* Configure the desired clock to be 4Mhz */
			wlc_bmac_write_shm(wlc_hw, M_ANTSEL_CLKDIV, ANTSEL_CLKDIV_4MHZ);
		}
	}
#endif /* WL11N */
	/* gpio 9 controls the PA.  ucode is responsible for wiggling out and oe */
	if (wlc_hw->boardflags & BFL_PACTRL)
		gm |= gc |= BOARD_GPIO_PACTRL;

	if (((wlc_hw->sih->boardtype == BCM94322MC_SSID) ||
	     (wlc_hw->sih->boardtype == BCM94322HM_SSID)) &&
	    ((wlc_hw->boardrev & 0xfff) >= 0x200) &&
	    ((CHIPID(wlc_hw->sih->chip) == BCM4322_CHIP_ID) &&
	     (CHIPREV(wlc_hw->sih->chiprev) == 0))) {

		gm |= gc |= BOARD_GPIO_12;
		OR_REG(osh, &regs->psm_gpio_oe, (BOARD_GPIO_12));
		AND_REG(osh, &regs->psm_gpio_out, ~BOARD_GPIO_12);
	}

	/* gpio 14(Xtal_up) and gpio 15(PLL_powerdown) are controlled in PCI config space */


	WL_INFORM(("wl%d: gpiocontrol mask 0x%x value 0x%x\n", wlc_hw->unit, gm, gc));

	/* apply to gpiocontrol register */
	si_gpiocontrol(wlc_hw->sih, gm, gc, GPIO_DRV_PRIORITY);
}

static void
BCMATTACHFN(wlc_ucode_download)(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc;
	wlc = wlc_hw->wlc;

	if (wlc_hw->ucode_loaded)
		return;

#if defined(WLP2P)
	if (P2P_ENAB_HW(wlc_hw)) {
		const uint32 *ucode32 = NULL;
		const uint8 *ucode8 = NULL;
		uint nbytes = 0;

		if (WLCISHTPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 26)) {
				ucode32 = d11ucode_p2p26_mimo;
				nbytes = d11ucode_p2p26_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 29)) {
				ucode32 = d11ucode_p2p29_mimo;
				nbytes = d11ucode_p2p29_mimosz;
			}
		} else if (WLCISNPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 31)) {
				ucode32 = d11ucode_p2p29_mimo;
				nbytes = d11ucode_p2p29_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 30)) {
				ucode32 = d11ucode_p2p30_mimo;
				nbytes = d11ucode_p2p30_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 25) ||
				D11REV_IS(wlc_hw->corerev, 28)) {
				ucode32 = d11ucode_p2p25_mimo;
				nbytes = d11ucode_p2p25_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 24)) {
				ucode32 = d11ucode_p2p24_mimo;
				nbytes = d11ucode_p2p24_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 22)) {
				ucode32 = d11ucode_p2p22_mimo;
				nbytes = d11ucode_p2p22_mimosz;
			} else if (D11REV_GE(wlc_hw->corerev, 16)) {
				/* ucode only supports rev23(43224b0) with rev16 ucode */
				ucode32 = d11ucode_p2p16_mimo;
				nbytes = d11ucode_p2p16_mimosz;
			}
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 25)) {
				ucode8 = d11ucode_p2p25_lcn;
				nbytes = d11ucode_p2p25_lcnsz;
			} else if (D11REV_IS(wlc_hw->corerev, 24)) {
				ucode8 = d11ucode_p2p24_lcn;
				nbytes = d11ucode_p2p24_lcnsz;
			}
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			if (D11REV_GE(wlc_hw->corerev, 24)) {
				ucode8 = d11ucode_p2p20_sslpn;
				nbytes = d11ucode_p2p20_sslpnsz;
			} else if (D11REV_GE(wlc_hw->corerev, 16)) {
				ucode8 = d11ucode_p2p16_sslpn;
				nbytes = d11ucode_p2p16_sslpnsz;
			}
		} else if (WLCISLPPHY(wlc_hw->band)) {
			if (D11REV_GE(wlc_hw->corerev, 16)) {
				ucode32 = d11ucode_p2p16_lp;
				nbytes = d11ucode_p2p16_lpsz;
			}
			else if (D11REV_IS(wlc_hw->corerev, 15)) {
				ucode32 = d11ucode_p2p15;
				nbytes = d11ucode_p2p15sz;
			}
		}

		if (ucode32 != NULL)
			wlc_ucode_write(wlc_hw, ucode32, nbytes);
		else if (ucode8 != NULL)
			wlc_ucode_write_byte(wlc_hw, ucode8, nbytes);
		else {
			WL_ERROR(("%s: wl%d: unsupported phy %d in corerev %d for P2P\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->band->phytype,
			          wlc_hw->corerev));
			return;
		}
	}
	else
#endif /* WLP2P */
	if (D11REV_IS(wlc_hw->corerev, 31)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode29_mimo, d11ucode29_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 30)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode30_mimo, d11ucode30_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 29)) {
		if (WLCISHTPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode29_mimo, d11ucode29_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 26)) {
		if (WLCISHTPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode26_mimo, d11ucode26_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 25) || D11REV_IS(wlc_hw->corerev, 28)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode25_mimo, d11ucode25_mimosz);
		else if (WLCISLCNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode25_lcn, d11ucode25_lcnsz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 24)) {
		if (WLCISLCNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode24_lcn,
			                     d11ucode24_lcnsz);
		else if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode24_mimo, d11ucode24_mimosz);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode20_sslpn,
			                     d11ucode20_sslpnsz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 23)) {
		/* ucode only supports rev23(43224b0) with rev16 ucode */
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode16_mimo, d11ucode16_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 22)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode22_mimo, d11ucode22_mimosz);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode22_sslpn, d11ucode22_sslpnsz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 21)) {
		if (WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_ucode_write_byte(wlc_hw, d11ucode21_sslpn,
				d11ucode21_sslpnsz);
		}
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 21\n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 20) && WLCISSSLPNPHY(wlc_hw->band))
		wlc_ucode_write_byte(wlc_hw, d11ucode20_sslpn, d11ucode20_sslpnsz);
	else if (D11REV_IS(wlc_hw->corerev, 19) && WLCISSSLPNPHY(wlc_hw->band)) {
#ifdef BCMECICOEX
		wlc_ucode_write_byte(wlc_hw, d11ucode19_sslpn, d11ucode19_sslpnsz);
#else
		wlc_ucode_write_byte(wlc_hw, d11ucode19_sslpn_nobt, d11ucode19_sslpn_nobtsz);
#endif
	}
	else if (D11REV_GE(wlc_hw->corerev, 16)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode16_mimo, d11ucode16_mimosz);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode16_sslpn, d11ucode16_sslpnsz);
		else if (WLCISLPPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode16_lp, d11ucode16_lpsz);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
	}
#ifdef BTC2WIRE
	else if (D11REV_IS(wlc_hw->corerev, 15) && (wlc_hw->btc->wire == WL_BTC_2WIRE))
		wlc_ucode_write(wlc_hw, d11ucode_2w15, d11ucode_2w15sz);
#endif /* BTC2WIRE */
	else if (D11REV_IS(wlc_hw->corerev, 15))
		wlc_ucode_write(wlc_hw, d11ucode15, d11ucode15sz);
	else if (D11REV_IS(wlc_hw->corerev, 14))
		wlc_ucode_write(wlc_hw, d11ucode14, d11ucode14sz);
#ifdef BTC2WIRE
	else if (D11REV_IS(wlc_hw->corerev, 13) && (wlc_hw->btc->wire == WL_BTC_2WIRE))
		wlc_ucode_write(wlc_hw, d11ucode_2w13, d11ucode_2w13sz);
#endif /* BTC2WIRE */
	else if (D11REV_IS(wlc_hw->corerev, 13))
		wlc_ucode_write(wlc_hw, d11ucode13, d11ucode13sz);
#ifdef BTC2WIRE
	else if (D11REV_GE(wlc_hw->corerev, 11) && (wlc_hw->btc->wire == WL_BTC_2WIRE))
		wlc_ucode_write(wlc_hw, d11ucode_2w11, d11ucode_2w11sz);
#endif /* BTC2WIRE */
	else if (D11REV_GE(wlc_hw->corerev, 11))
		wlc_ucode_write(wlc_hw, d11ucode11, d11ucode11sz);
#if defined(MBSS)
	/* ucode for corerev 9 has this support */
	else if (D11REV_IS(wlc_hw->corerev, 9))
		wlc_ucode_write(wlc_hw, d11ucode9, d11ucode9sz);
#endif 
	else if (D11REV_GE(wlc_hw->corerev, 5))
		wlc_ucode_write(wlc_hw, d11ucode5, d11ucode5sz);
	else if (D11REV_IS(wlc_hw->corerev, 4))
		wlc_ucode_write(wlc_hw, d11ucode4, d11ucode4sz);
	else
		WL_ERROR(("wl%d: %s: corerev %d is invalid\n", wlc_hw->unit,
			__FUNCTION__, wlc_hw->corerev));

	wlc_hw->ucode_loaded = TRUE;
}

static void
wlc_ucode_write(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes)
{
	osl_t *osh;
	d11regs_t *regs = wlc_hw->regs;
	uint i;
	uint count;

	osh = wlc_hw->osh;

	WL_TRACE(("wl%d: wlc_ucode_write\n", wlc_hw->unit));

	ASSERT(ISALIGNED(nbytes, sizeof(uint32)));

	count = (nbytes/sizeof(uint32));

	W_REG(osh, &regs->objaddr, (OBJADDR_AUTO_INC | OBJADDR_UCM_SEL));
	(void)R_REG(osh, &regs->objaddr);
	for (i = 0; i < count; i++)
		W_REG(osh, &regs->objdata, ucode[i]);
}

static void
BCMATTACHFN(wlc_ucode_write_byte)(wlc_hw_info_t *wlc_hw, const uint8 ucode[], const uint nbytes)
{
	osl_t *osh;
	d11regs_t *regs = wlc_hw->regs;
	uint i;
	uint32 ucode_word;

	osh = wlc_hw->osh;

	WL_TRACE(("wl%d: wlc_ucode_write\n", wlc_hw->unit));

	W_REG(osh, &regs->objaddr, (OBJADDR_AUTO_INC | OBJADDR_UCM_SEL));
	for (i = 0; i < nbytes; i += 7) {
		ucode_word = ucode[i+3] << 24;
		ucode_word = ucode_word | (ucode[i+4] << 16);
		ucode_word = ucode_word | (ucode[i+5] << 8);
		ucode_word = ucode_word | (ucode[i+6] << 0);
		W_REG(osh, &regs->objdata, ucode_word);

		ucode_word = ucode[i+0] << 16;
		ucode_word = ucode_word | (ucode[i+1] << 8);
		ucode_word = ucode_word | (ucode[i+2] << 0);
		W_REG(osh, &regs->objdata, ucode_word);
	}
}

static void
BCMINITFN(wlc_ucode_pcm_write)(wlc_hw_info_t *wlc_hw, const uint32 pcm[], const uint nbytes)
{
	uint i;
	osl_t *osh;
	d11regs_t *regs = wlc_hw->regs;

	WL_TRACE(("wl%d: wlc_ucode_pcm_write\n", wlc_hw->unit));

	ASSERT(ISALIGNED(nbytes, sizeof(uint32)));

	osh = wlc_hw->osh;

	W_REG(osh, &regs->objaddr,
	      (OBJADDR_IHR_SEL | ((WEP_PCMADDR - PIHR_BASE)/sizeof(uint16))));
	(void)R_REG(osh, &regs->objaddr);
	W_REG(osh, &regs->objdata, (PCMADDR_INC | PCMADDR_UCM_SEL));
	W_REG(osh, &regs->objaddr,
	      (OBJADDR_IHR_SEL | ((WEP_PCMDATA - PIHR_BASE)/sizeof(uint16))));
	(void)R_REG(osh, &regs->objaddr);
	for (i = 0; i < (nbytes/sizeof(uint32)); i++)
		W_REG(osh, &regs->objdata, pcm[i]);
}

static void
wlc_write_inits(wlc_hw_info_t *wlc_hw, const d11init_t *inits)
{
	int i;
	osl_t *osh;
	volatile uint8 *base;

	WL_TRACE(("wl%d: wlc_write_inits\n", wlc_hw->unit));

	osh = wlc_hw->osh;
	base = (volatile uint8*)wlc_hw->regs;

	for (i = 0; inits[i].addr != 0xffff; i++) {
		ASSERT((inits[i].size == 2) || (inits[i].size == 4));

		if (inits[i].size == 2)
			W_REG(osh, (uint16*)(uintptr)(base+inits[i].addr), inits[i].value);
		else if (inits[i].size == 4)
			W_REG(osh, (uint32*)(uintptr)(base+inits[i].addr), inits[i].value);
	}
}

#ifdef WOWL
/* External API to write the ucode to avoid exposing the details */

#define BOARD_GPIO_3_WOWL 0x8 /* bit mask of 3rd pin */
#ifdef WLC_LOW_ONLY
static bool
wlc_bmac_wowl_config_hw(wlc_hw_info_t *wlc_hw)
{
	/* configure the gpio etc to inform host to wake up etc */

	WL_TRACE(("wl: %s: corerev = 0x%x boardtype = 0x%x\n",  __FUNCTION__,
		wlc_hw->corerev, wlc_hw->sih->boardtype));

	if (!wlc_hw->clk) {
		WL_ERROR(("wl: %s: No hw clk \n",  __FUNCTION__));
		return FALSE;
	}

	if (D11REV_IS(wlc_hw->corerev, 24) ||
		(D11REV_IS(wlc_hw->corerev, 16) &&
		(BUSTYPE(wlc_hw->sih->bustype) == SI_BUS))) {

		si_gpiocontrol(wlc_hw->sih, BOARD_GPIO_3_WOWL, 0, GPIO_DRV_PRIORITY);

		si_gpioouten(wlc_hw->sih, BOARD_GPIO_3_WOWL, BOARD_GPIO_3_WOWL,
			GPIO_DRV_PRIORITY);

		/* drive the output to 1 and ucode will drive to 0 ACTIVE_LOW */
		si_gpioout(wlc_hw->sih, BOARD_GPIO_3_WOWL, BOARD_GPIO_3_WOWL,
			GPIO_DRV_PRIORITY);

		OR_REG(osh, &wlc_hw->regs->psm_gpio_oe, BOARD_GPIO_3_WOWL);
		OR_REG(osh, &wlc_hw->regs->psm_gpio_out, BOARD_GPIO_3_WOWL);

		/* give the control to ucode */
		si_gpiocontrol(wlc_hw->sih, BOARD_GPIO_3_WOWL, BOARD_GPIO_3_WOWL,
			GPIO_DRV_PRIORITY);
	}

	return TRUE;
}

#else

static bool
wlc_bmac_wowl_config_hw(wlc_hw_info_t *wlc_hw)
{
	/* configure the gpio etc to inform host to wake up etc */

	WL_TRACE(("wl: %s: corerev = 0x%x boardtype = 0x%x\n",  __FUNCTION__,
		wlc_hw->corerev, wlc_hw->sih->boardtype));

	if (!wlc_hw->clk) {
		WL_ERROR(("wl: %s: No hw clk \n",  __FUNCTION__));
		return FALSE;
	}

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		if ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) {
				WL_INFORM(("wl%d: %s: set mux pin to SROM\n",
					wlc_hw->unit, __FUNCTION__));
				/* force muxed pin to control SROM */
				si_chipcontrl_epa4331(wlc_hw->sih, FALSE);
				/* Apply WAR to enable 2G ePA and force muxed pin to SROM */
				si_chipcontrl_epa4331_wowl(wlc_hw->sih, TRUE);
		}
	}

	return TRUE;
}
#endif /* WLC_LOW_ONLY */

int
wlc_bmac_wowlucode_init(wlc_hw_info_t *wlc_hw)
{
#ifndef WLC_LOW_ONLY
	wlc_bmac_wowl_config_hw(wlc_hw);
#endif

	if (!wlc_hw->clk) {
		WL_ERROR(("wl: %s: No hw clk \n",  __FUNCTION__));
		return BCME_ERROR;
	}

	/* Reset ucode. PSM_RUN is needed because current PC is not going to be 0 */
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_PSM_JMP_0 | MCTL_PSM_RUN));

	return BCME_OK;
}

int
wlc_bmac_wowlucode_start(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs;
	regs = wlc_hw->regs;

	/* let the PSM run to the suspended state, set mode to BSS STA */
	W_REG(wlc_hw->osh, &regs->macintstatus, -1);
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_INFRA | MCTL_PSM_RUN | MCTL_WAKE));

	/* wait for ucode to self-suspend after auto-init */
	SPINWAIT(((R_REG(wlc_hw->osh, &regs->macintstatus) & MI_MACSSPNDD) == 0), 1000 * 1000);
	if ((R_REG(wlc_hw->osh, &regs->macintstatus) & MI_MACSSPNDD) == 0) {
		WL_ERROR(("wl%d: wlc_coreinit: ucode did not self-suspend!\n", wlc_hw->unit));
		return BCME_ERROR;
	}

	return BCME_OK;
}

int
wlc_bmac_write_inits(wlc_hw_info_t *wlc_hw, void *inits, int len)
{

	wlc_write_inits(wlc_hw, inits);

	return BCME_OK;
}

int
wlc_bmac_wakeucode_dnlddone(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs;
	regs = wlc_hw->regs;

	/* tell the ucode the corerev */
	wlc_bmac_write_shm(wlc_hw, M_MACHW_VER, (uint16)wlc_hw->corerev);

	/* overwrite default long slot timing */
	if (wlc_hw->shortslot)
		wlc_bmac_update_slot_timing(wlc_hw, wlc_hw->shortslot);

	/* write rate fallback retry limits */
	wlc_bmac_write_shm(wlc_hw, M_SFRMTXCNTFBRTHSD, wlc_hw->SFBL);
	wlc_bmac_write_shm(wlc_hw, M_LFRMTXCNTFBRTHSD, wlc_hw->LFBL);

	/* Restore the hostflags */
	wlc_write_mhf(wlc_hw, wlc_hw->band->mhfs);

	/* make sure we can still talk to the mac */
	ASSERT(R_REG(wlc_hw->osh, &regs->maccontrol) != 0xffffffff);

	wlc_bmac_mctrl(wlc_hw, MCTL_DISCARD_PMQ, MCTL_DISCARD_PMQ);

	wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);

	wlc_bmac_upd_synthpu(wlc_hw);

#ifdef WLC_LOW_ONLY
	wlc_bmac_wowl_config_hw(wlc_hw);
#endif

	return BCME_OK;
}


#endif /* WOWL */


#ifdef SAMPLE_COLLECT
/* Load sample collect ucode
 * Ucode inits the SHM and all MAC regs
 * can support all PHY types, implement NPHY for now.
 */
static void
wlc_ucode_sample_init_rev(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes)
{
	d11regs_t *regs = wlc_hw->regs;

	if (WLCISNPHY(wlc_hw->band) && NREV_GE(wlc_hw->band->phyrev, 7)) {
	  /* Restart the ucode (recover from wl out) */
		wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_PSM_RUN | MCTL_EN_MAC));
		return;
	}

	/* Reset ucode. PSM_RUN is needed because current PC is not going to be 0 */
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_PSM_JMP_0 | MCTL_PSM_RUN));

	/* Load new d11ucode */
	wlc_ucode_write(wlc_hw, ucode, nbytes);

	/* let the PSM run to the suspended state, set mode to BSS STA */
	W_REG(wlc_hw->osh, &regs->macintstatus, -1);
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_INFRA | MCTL_PSM_RUN | MCTL_WAKE));

	/* wait for ucode to self-suspend after auto-init */
	SPINWAIT(((R_REG(wlc_hw->osh, &regs->macintstatus) & MI_MACSSPNDD) == 0), 1000 * 1000);
	if ((R_REG(wlc_hw->osh, &regs->macintstatus) & MI_MACSSPNDD) == 0)
		WL_ERROR(("wl%d: wlc_coreinit: ucode did not self-suspend!\n", wlc_hw->unit));

	/* make sure we can still talk to the mac */
	ASSERT(R_REG(wlc_hw->osh, &regs->maccontrol) != 0xffffffff);
}

void
wlc_ucode_sample_init(wlc_hw_info_t *wlc_hw)
{
	if (D11REV_LT(wlc_hw->corerev, 16)) {
		WL_ERROR(("wlc_ucode_sample_init: this corerev is not support\n"));
	} else {
		wlc_ucode_sample_init_rev(wlc_hw, d11sampleucode16, d11sampleucode16sz);
	}
}

#endif	/* SAMPLE_COLLECT */

static void
wlc_ucode_txant_set(wlc_hw_info_t *wlc_hw)
{
	uint16 phyctl;
	uint16 phytxant = wlc_hw->bmac_phytxant;
	uint16 mask = PHY_TXC_ANT_MASK;


	/* set the Probe Response frame phy control word */
	phyctl = wlc_bmac_read_shm(wlc_hw, M_CTXPRS_BLK + C_CTX_PCTLWD_POS);
	phyctl = (phyctl & ~mask) | phytxant;
	wlc_bmac_write_shm(wlc_hw, M_CTXPRS_BLK + C_CTX_PCTLWD_POS, phyctl);

	/* set the Response (ACK/CTS) frame phy control word */
	phyctl = wlc_bmac_read_shm(wlc_hw, M_RSP_PCTLWD);
	phyctl = (phyctl & ~mask) | phytxant;
	wlc_bmac_write_shm(wlc_hw, M_RSP_PCTLWD, phyctl);
}

void
wlc_bmac_txant_set(wlc_hw_info_t *wlc_hw, uint16 phytxant)
{
	/* update sw state */
	wlc_hw->bmac_phytxant = phytxant;

	/* push to ucode if up */
	if (!wlc_hw->up)
		return;
	wlc_ucode_txant_set(wlc_hw);

}

uint16
wlc_bmac_get_txant(wlc_hw_info_t *wlc_hw)
{
#ifdef WLC_HIGH
	return (uint16)wlc_hw->wlc->stf->txant;
#else
	return 0;
#endif
}

void
wlc_bmac_antsel_type_set(wlc_hw_info_t *wlc_hw, uint8 antsel_type)
{
	wlc_hw->antsel_type = antsel_type;

	/* Update the antsel type for phy module to use */
	wlc_phy_antsel_type_set(wlc_hw->band->pi, antsel_type);
}

void
wlc_bmac_fifoerrors(wlc_hw_info_t *wlc_hw)
{
	bool fatal = FALSE;
	uint unit;
	uint intstatus, idx;
	d11regs_t *regs = wlc_hw->regs;

	unit = wlc_hw->unit;

	for (idx = 0; idx < NFIFO; idx++) {
		/* read intstatus register and ignore any non-error bits */
		intstatus = R_REG(wlc_hw->osh, &regs->intctrlregs[idx].intstatus) & I_ERRORS;
		if (!intstatus)
			continue;

		WL_TRACE(("wl%d: wlc_bmac_fifoerrors: intstatus%d 0x%x\n", unit, idx, intstatus));

		if (intstatus & I_RO) {
			WL_ERROR(("wl%d: fifo %d: receive fifo overflow\n", unit, idx));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->rxoflo);
			fatal = TRUE;
		}

		if (intstatus & I_PC) {
			WL_ERROR(("wl%d: fifo %d: descriptor error\n", unit, idx));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->dmade);
			fatal = TRUE;
		}

		if (intstatus & I_PD) {
#if defined(MACOSX)
			printf("wl%d: fifo %d: data error\n", unit, idx);
#else
			WL_ERROR(("wl%d: fifo %d: data error\n", unit, idx));
#endif
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->dmada);
			fatal = TRUE;
		}

		if (intstatus & I_DE) {
			WL_ERROR(("wl%d: fifo %d: descriptor protocol error\n", unit, idx));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->dmape);
			fatal = TRUE;
		}

		if (intstatus & I_RU) {
			WL_ERROR(("wl%d: fifo %d: receive descriptor underflow\n", unit, idx));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->rxuflo[idx]);
		}

		if (intstatus & I_XU) {
			WL_ERROR(("wl%d: fifo %d: transmit fifo underflow\n", idx, unit));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->txuflo);
			fatal = TRUE;
		}

#ifdef BCMDBG
		{
			/* dump dma rings to console */
			const int FIFOERROR_DUMP_SIZE = 8192;
			char *tmp;
			struct bcmstrbuf b;
			if (fatal && !PIO_ENAB(wlc_hw->wlc->pub) && wlc_hw->di[idx] &&
			    (tmp = MALLOC(wlc_hw->osh, FIFOERROR_DUMP_SIZE))) {
				bcm_binit(&b, tmp, FIFOERROR_DUMP_SIZE);
				dma_dump(wlc_hw->di[idx], &b, TRUE);
				printbig(tmp);
				MFREE(wlc_hw->osh, tmp, FIFOERROR_DUMP_SIZE);
			}
		}


#endif /* BCMDBG */

		if (fatal) {
			WLC_EXTLOG(wlc_hw->wlc, LOG_MODULE_COMMON, FMTSTR_FATAL_ERROR_ID,
				WL_LOG_LEVEL_ERR, 0, intstatus, NULL);
			wlc_fatal_error(wlc_hw->wlc);	/* big hammer */
			break;
		}
		else
			W_REG(wlc_hw->osh, &regs->intctrlregs[idx].intstatus, intstatus);
	}
}

void
wlc_intrson(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	ASSERT(wlc->defmacintmask);
	wlc->macintmask = wlc->defmacintmask;
	W_REG(wlc_hw->osh, &wlc_hw->regs->macintmask, wlc->macintmask);
}


/* callback for siutils.c, which has only wlc handler, no wl
 * they both check up, not only because there is no need to off/restore d11 interrupt
 *  but also because per-port code may require sync with valid interrupt.
 */

static uint32
wlc_wlintrsoff(wlc_hw_info_t *wlc_hw)
{
	if (!wlc_hw->up)
		return 0;

	return wl_intrsoff(wlc_hw->wlc->wl);
}

static void
wlc_wlintrsrestore(wlc_hw_info_t *wlc_hw, uint32 macintmask)
{
	if (!wlc_hw->up)
		return;

	wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
}

#ifdef BCMDBG
static bool
wlc_intrs_enabled(wlc_hw_info_t *wlc_hw)
{
	return (wlc_hw->wlc->macintmask != 0);
}
#endif /* BCMDBG */

uint32
wlc_intrsoff(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint32 macintmask;

	if (!wlc_hw->clk)
		return 0;

	macintmask = wlc->macintmask;	/* isr can still happen */

	W_REG(wlc_hw->osh, &wlc_hw->regs->macintmask, 0);
	(void)R_REG(wlc_hw->osh, &wlc_hw->regs->macintmask);	/* sync readback */
	OSL_DELAY(1); /* ensure int line is no longer driven */
	wlc->macintmask = 0;

	/* return previous macintmask; resolve race between us and our isr */
	return (wlc->macintstatus ? 0 : macintmask);
}

void
wlc_intrsrestore(wlc_info_t *wlc, uint32 macintmask)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	if (!wlc_hw->clk)
		return;

	wlc->macintmask = macintmask;
	W_REG(wlc_hw->osh, &wlc_hw->regs->macintmask, wlc->macintmask);
}

void
wlc_bmac_mute(wlc_hw_info_t *wlc_hw, bool on, mbool flags)
{
	struct ether_addr null_ether_addr = {{0, 0, 0, 0, 0, 0}};

	if (on) {
		/* suspend tx fifos */
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_DATA_FIFO);
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_CTL_FIFO);
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_AC_BK_FIFO);
#ifdef WME
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_AC_VI_FIFO);
#endif /* WME */
#ifdef AP
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_BCMC_FIFO);
#endif /* AP */
#if defined(MBSS)
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_ATIM_FIFO);
#endif 

		/* zero the address match register so we do not send ACKs */
		wlc_bmac_set_addrmatch(wlc_hw, RCM_MAC_OFFSET, &null_ether_addr);
	} else {
		/* resume tx fifos */
		if (!wlc_hw->wlc->tx_suspended) {
			wlc_bmac_tx_fifo_resume(wlc_hw, TX_DATA_FIFO);
		}
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_CTL_FIFO);
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_AC_BK_FIFO);
#ifdef WME
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_AC_VI_FIFO);
#endif /* WME */
#ifdef AP
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_BCMC_FIFO);
#endif /* AP */
#if defined(MBSS)
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_ATIM_FIFO);
#endif 

		/* Restore address */
		wlc_bmac_set_addrmatch(wlc_hw, RCM_MAC_OFFSET, &wlc_hw->etheraddr);
	}

	wlc_phy_mute_upd(wlc_hw->band->pi, on, flags);

	if (on)
		wlc_ucode_mute_override_set(wlc_hw);
	else
		wlc_ucode_mute_override_clear(wlc_hw);
}

void
wlc_bmac_set_deaf(wlc_hw_info_t *wlc_hw, bool user_flag)
{
	wlc_phy_set_deaf(wlc_hw->band->pi, user_flag);
}

#if defined(WLTEST)
void
wlc_bmac_clear_deaf(wlc_hw_info_t *wlc_hw, bool user_flag)
{
	wlc_phy_clear_deaf(wlc_hw->band->pi, user_flag);
}
#endif

void
wlc_bmac_filter_war_upd(wlc_hw_info_t *wlc_hw, bool set)
{
	wlc_phy_set_filt_war(wlc_hw->band->pi, set);
}

int
wlc_bmac_xmtfifo_sz_get(wlc_hw_info_t *wlc_hw, uint fifo, uint *blocks)
{
	if (fifo >= NFIFO)
		return BCME_RANGE;

	*blocks = wlc_hw->xmtfifo_sz[fifo];

	return 0;
}

int
wlc_bmac_xmtfifo_sz_set(wlc_hw_info_t *wlc_hw, uint fifo, uint16 blocks)
{
	if (fifo >= NFIFO || blocks > 299)
		return BCME_RANGE;

	wlc_hw->xmtfifo_sz[fifo] = blocks;

#ifdef WLAMPDU_HW
	if (fifo < AC_COUNT) {
		wlc_hw->xmtfifo_frmmax[fifo] =
			(wlc_hw->xmtfifo_sz[fifo] * 256 - 1300)	/ MAX_MPDU_SPACE;
		WL_INFORM(("%s: fifo sz blk %d entries %d\n",
			__FUNCTION__, wlc_hw->xmtfifo_sz[fifo], wlc_hw->xmtfifo_frmmax[fifo]));
	}
#endif

	return 0;
}

/* wlc_bmac_tx_fifo_suspended:
 * Check the MAC's tx suspend status for a tx fifo.
 *
 * When the MAC acknowledges a tx suspend, it indicates that no more
 * packets will be transmitted out the radio. This is independent of
 * DMA channel suspension---the DMA may have finished suspending, or may still
 * be pulling data into a tx fifo, by the time the MAC acks the suspend
 * request.
 */
bool
wlc_bmac_tx_fifo_suspended(wlc_hw_info_t *wlc_hw, uint tx_fifo)
{
	/* check that a suspend has been requested and is no longer pending */
	if (!PIO_ENAB(wlc_hw->wlc->pub)) {
		/*
		 * for DMA mode, the suspend request is set in xmtcontrol of the DMA engine,
		 * and the tx fifo suspend at the lower end of the MAC is acknowledged in the
		 * chnstatus register.
		 * The tx fifo suspend completion is independent of the DMA suspend completion and
		 *   may be acked before or after the DMA is suspended.
		 */
		if (dma_txsuspended(wlc_hw->di[tx_fifo]) &&
		    (R_REG(wlc_hw->osh, &wlc_hw->regs->chnstatus) &
			(1<<tx_fifo)) == 0)
			return TRUE;
	} else {
		if (wlc_pio_txsuspended(wlc_hw->pio[tx_fifo]))
			return TRUE;
	}

	return FALSE;
}

void
wlc_bmac_tx_fifo_suspend(wlc_hw_info_t *wlc_hw, uint tx_fifo)
{
	uint8 fifo = 1 << tx_fifo;

	/* Two clients of this code, 11h Quiet period and scanning. */

	/* only suspend if not already suspended */
	if ((wlc_hw->suspended_fifos & fifo) == fifo)
		return;

	/* force the core awake only if not already */
	if (wlc_hw->suspended_fifos == 0)
		wlc_ucode_wake_override_set(wlc_hw, WLC_WAKE_OVERRIDE_TXFIFO);

	wlc_hw->suspended_fifos |= fifo;

	if (!PIO_ENAB(wlc_hw->wlc->pub)) {
		if (wlc_hw->di[tx_fifo]) {
			/* Suspending AMPDU transmissions in the middle can cause underflow
			 * which may result in mismatch between ucode and driver
			 * so suspend the mac before suspending the FIFO
			 */
			if (WLC_PHY_11N_CAP(wlc_hw->band))
				wlc_bmac_suspend_mac_and_wait(wlc_hw);

			dma_txsuspend(wlc_hw->di[tx_fifo]);

			if (WLC_PHY_11N_CAP(wlc_hw->band))
				wlc_bmac_enable_mac(wlc_hw);
		}
	} else {
		wlc_pio_txsuspend(wlc_hw->pio[tx_fifo]);
	}
}

void
wlc_bmac_tx_fifo_resume(wlc_hw_info_t *wlc_hw, uint tx_fifo)
{
	/* BMAC_NOTE: WLC_TX_FIFO_ENAB is done in wlc_dpc() for DMA case but need to be done
	 * here for PIO otherwise the watchdog will catch the inconsistency and fire
	 */
	/* Two clients of this code, 11h Quiet period and scanning. */
	if (!PIO_ENAB(wlc_hw->wlc->pub)) {
		if (wlc_hw->di[tx_fifo])
			dma_txresume(wlc_hw->di[tx_fifo]);
	} else {
		wlc_pio_txresume(wlc_hw->pio[tx_fifo]);
#ifdef WLC_HIGH
		WLC_TX_FIFO_ENAB(wlc_hw->wlc, tx_fifo);
#endif
	}

	/* allow core to sleep again */
	if (wlc_hw->suspended_fifos == 0)
		return;
	else {
		wlc_hw->suspended_fifos &= ~(1 << tx_fifo);
		if (wlc_hw->suspended_fifos == 0)
			wlc_ucode_wake_override_clear(wlc_hw, WLC_WAKE_OVERRIDE_TXFIFO);
	}
}

#ifdef WL_MULTIQUEUE
static void wlc_bmac_service_txstatus(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_flush_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap);
static void wlc_bmac_enable_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap);
#ifdef WLC_LOW_ONLY
static void wlc_bmac_clear_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap);
#endif

/* Enable new method of suspend and flush.
 * Requires minimum ucode BOM 622.1.
 */ 
#define NEW_SUSPEND_FLUSH_UCODE 1
void
wlc_bmac_tx_fifo_sync(wlc_hw_info_t *wlc_hw, uint fifo_bitmap, uint8 flag)
{
#ifdef NEW_SUSPEND_FLUSH_UCODE
	/* halt any tx processing by ucode */
	wlc_bmac_suspend_mac_and_wait(wlc_hw);

	/* clear the hardware fifos */
	wlc_bmac_flush_tx_fifos(wlc_hw, fifo_bitmap);

	/* process any frames that made it out before the suspend */
	wlc_bmac_service_txstatus(wlc_hw);

	/* allow ucode to run again */
	wlc_bmac_enable_mac(wlc_hw);
#else
	bool suspend;

	/* enable MAC only if currently suspended */
	suspend = (0 == (R_REG(wlc_hw->wlc->osh, &wlc_hw->wlc->regs->maccontrol) & MCTL_EN_MAC));
	if (suspend)
		wlc_bmac_enable_mac(wlc_hw);

	/* clear the hardware fifos */
	wlc_bmac_flush_tx_fifos(wlc_hw, fifo_bitmap);

	/* put MAC back into suspended state if required */
	if (suspend)
		wlc_bmac_suspend_mac_and_wait(wlc_hw);

	/* process any frames that made it out before the suspend */
	wlc_bmac_service_txstatus(wlc_hw);
#endif /* NEW_SUSPEND_FLUSH_UCODE */

#ifdef WLC_LOW_ONLY
	/* clear the hardware fifos for split driver */
	wlc_bmac_clear_tx_fifos(wlc_hw, fifo_bitmap);
#endif /* WLC_LOW_ONLY */

	/* signal to the upper layer that the fifos are flushed
	 * and any tx packet statuses have been returned
	 */
	wlc_tx_fifo_sync_complete(wlc_hw->wlc, fifo_bitmap, flag);

	/* reenable the fifos once the completion has been signaled */
	wlc_bmac_enable_tx_fifos(wlc_hw, fifo_bitmap);
}

static void
wlc_bmac_service_txstatus(wlc_hw_info_t *wlc_hw)
{
	bool fatal = FALSE;

	wlc_bmac_txstatus(wlc_hw, FALSE, &fatal);
}

static void
wlc_bmac_flush_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap)
{
	uint i;
	uint chnstatus;
	uint count;
	osl_t *osh;
	uint fbmp;

	osh = wlc_hw->osh;

	if (!PIO_ENAB(wlc_hw->wlc->pub)) {
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->di[i] == NULL)
				continue;

			dma_txflush(wlc_hw->di[i]);
		}

		/* DBG print */
		chnstatus = R_REG(osh, &wlc_hw->regs->chnstatus);
		WL_MQ(("MQ: %s: post flush req chanstatus 0x%x\n", __FUNCTION__,
		       chnstatus));

		/* DBG print */
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			uint status;
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->di[i] == NULL)
				continue;

			if (D11REV_LT(wlc_hw->corerev, 11)) {
				dma32regs_t *d32regs = &wlc_hw->regs->fifo.f32regs.dmaregs[i].xmt;
				status = ((R_REG(osh, &d32regs->status) & XS_XS_MASK) >>
				          XS_XS_SHIFT);
			} else {
				dma64regs_t *d64regs = &wlc_hw->regs->fifo.f64regs[i].dmaxmt;
				status = ((R_REG(osh, &d64regs->status0) & D64_XS0_XS_MASK) >>
				          D64_XS0_XS_SHIFT);
			}
			WL_MQ(("MQ: %s: post flush req dma %d status %u\n", __FUNCTION__,
			       i, status));
		} /* _fifo_bitmap & 0x01 */

		/* wait for flush complete */
		count = 0;
		while (((chnstatus = R_REG(osh, &wlc_hw->regs->chnstatus)) & 0xFF00) &&
		       (count < (80 * 1000))) {
			OSL_DELAY(10);
			count += 10;
		}
		if (chnstatus & 0xFF00) {
			WL_ERROR(("MQ: %s: waited %d us but still fifo-suspend: chanstatus 0x%x\n",
			       __FUNCTION__, count, chnstatus));
		} else {
			WL_MQ(("MQ: %s: waited %d us for success chanstatus 0x%x\n",
			       __FUNCTION__, count, chnstatus));
		}

		/* Clear the dma flush command */
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->di[i] == NULL)
				continue;

			dma_txflush_clear(wlc_hw->di[i]);
		}

		/* DBG print */
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			uint status;

			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->di[i] == NULL)
				continue;

			if (D11REV_LT(wlc_hw->corerev, 11)) {
				dma32regs_t *d32regs = &wlc_hw->regs->fifo.f32regs.dmaregs[i].xmt;
				status = ((R_REG(osh, &d32regs->status) & XS_XS_MASK) >>
				          XS_XS_SHIFT);
			} else {
				dma64regs_t *d64regs = &wlc_hw->regs->fifo.f64regs[i].dmaxmt;
				status = ((R_REG(osh, &d64regs->status0) & D64_XS0_XS_MASK) >>
				          D64_XS0_XS_SHIFT);
			}
			WL_MQ(("MQ: %s: post flush wait dma %d status %u\n", __FUNCTION__,
			       i, status));
		} /* for */
	} else {
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->pio[i])
				wlc_pio_reset(wlc_hw->pio[i]);
		} /* for */
	} /* else */
}

static void
wlc_bmac_enable_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap)
{
	uint i;
	uint fbmp;

	if (!PIO_ENAB(wlc_hw->wlc->pub)) {
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->di[i] == NULL)
				continue;

			dma_txreset(wlc_hw->di[i]);
			dma_txinit(wlc_hw->di[i]);
		} /* for */
	} else {
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->pio[i])
				wlc_pio_reset(wlc_hw->pio[i]);
		} /* for */
	} /* else */
}

#ifdef WLC_LOW_ONLY
static void
wlc_bmac_clear_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap)
{
	uint i;
	uint fbmp;

	/* clear dma fifo */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0) /* not the right fifo to process */
			continue;

		if (!PIO_ENAB((wlc_hw->wlc)->pub)) {
			if (wlc_hw->di[i] == NULL)
				continue;

			dma_txreclaim(wlc_hw->di[i], HNDDMA_RANGE_ALL);
			TXPKTPENDCLR(wlc_hw->wlc, i);
#if defined(DMA_TX_FREE)
			wlc_hw->txstatus_ampdu_flags[i].head = 0;
			wlc_hw->txstatus_ampdu_flags[i].tail = 0;
#endif
		} else {
			if (wlc_hw->pio[i] == NULL)
				continue;

			/* include reset the counter */
			wlc_pio_txreclaim(wlc_hw->pio[i]);
		}
	}
}
#endif /* WLC_LOW_ONLY */
#endif /* WL_MULTIQUEUE */

/*
 * Read and clear macintmask and macintstatus and intstatus registers.
 * This routine should be called with interrupts off
 * Return:
 *   -1 if DEVICEREMOVED(wlc) evaluates to TRUE;
 *   0 if the interrupt is not for us, or we are in some special cases;
 *   device interrupt status bits otherwise.
 */
static INLINE uint32
wlc_intstatus(wlc_info_t *wlc, bool in_isr)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	d11regs_t *regs = wlc_hw->regs;
	uint32 macintstatus;
	uint32 intstatus_rxfifo, intstatus_txsfifo;
	osl_t *osh;

	osh = wlc_hw->osh;

	/* macintstatus includes a DMA interrupt summary bit */
	macintstatus = R_REG(osh, &regs->macintstatus);

	WL_TRACE(("wl%d: macintstatus: 0x%x\n", wlc_hw->unit, macintstatus));

	/* detect cardbus removed, in power down(suspend) and in reset */
	if (DEVICEREMOVED(wlc))
		return -1;

	/* DEVICEREMOVED succeeds even when the core is still resetting,
	 * handle that case here.
	 */
	if (macintstatus == 0xffffffff)
		return 0;

	/* defer unsolicited interrupts */
	macintstatus &= (in_isr ? wlc->macintmask : wlc->defmacintmask);

	/* if not for us */
	if (macintstatus == 0)
		return 0;

	/* interrupts are already turned off for CFE build
	 * Caution: For CFE Turning off the interrupts again has some undesired
	 * consequences
	 */
#if !defined(_CFE_)
	/* turn off the interrupts */
	W_REG(osh, &regs->macintmask, 0);
	(void)R_REG(osh, &regs->macintmask);	/* sync readback */
	wlc->macintmask = 0;
#endif /* !defined(_CFE_) */

#ifdef WLC_HIGH
	/* BMAC_NOTE: this code should not be in the ISR code, it should be in DPC.
	 * We can not modify wlc state except for very restricted items.
	 */
	if (macintstatus & MI_BT_RFACT_STUCK) {
		/* disable BT Coexist and WAR detection when stuck pin is detected
		 * This has to be done before clearing the macintstatus
		 */
		WL_ERROR(("Disabling BTC mode (BT_RFACT stuck detected)\n"));
		wlc_hw->btc->stuck_detected = TRUE;
		wlc_bmac_btc_mode_set(wlc_hw, WL_BTC_DISABLE);
		wlc_bmac_btc_stuck_war50943(wlc_hw, FALSE);
	}
#endif

	/* clear device interrupts */
	W_REG(osh, &regs->macintstatus, macintstatus);

	/* MI_DMAINT is indication of non-zero intstatus */
	if (macintstatus & MI_DMAINT) {
		if (D11REV_IS(wlc_hw->corerev, 4)) {
			intstatus_rxfifo = R_REG(osh, &regs->intctrlregs[RX_FIFO].intstatus);
			intstatus_txsfifo = R_REG(osh,
			                          &regs->intctrlregs[RX_TXSTATUS_FIFO].intstatus);
			WL_TRACE(("wl%d: intstatus_rxfifo 0x%x, intstatus_txsfifo 0x%x\n",
			          wlc_hw->unit, intstatus_rxfifo, intstatus_txsfifo));

			/* defer unsolicited interrupt hints */
			intstatus_rxfifo &= DEF_RXINTMASK;
			intstatus_txsfifo &= DEF_RXINTMASK;

			/* MI_DMAINT bit in macintstatus is indication of RX_FIFO interrupt */
			/* clear interrupt hints */
			if (intstatus_rxfifo)
				W_REG(osh,
				      &regs->intctrlregs[RX_FIFO].intstatus, intstatus_rxfifo);
			else
				macintstatus &= ~MI_DMAINT;

			/* MI_TFS bit in macintstatus is encoding of RX_TXSTATUS_FIFO interrupt */
			if (intstatus_txsfifo) {
				W_REG(osh, &regs->intctrlregs[RX_TXSTATUS_FIFO].intstatus,
				      intstatus_txsfifo);
				macintstatus |= MI_TFS;
			}
#if defined(DMA_TX_FREE)
			/* do not support DMA_TX_FREE for corerev 4 */
			ASSERT(0);
#endif
		} else {
#if !defined(DMA_TX_FREE)
			/*
			 * For corerevs >= 5, only fifo interrupt enabled is I_RI in RX_FIFO.
			 * If MI_DMAINT is set, assume it is set and clear the interrupt.
			 */
			W_REG(osh, &regs->intctrlregs[RX_FIFO].intstatus, DEF_RXINTMASK);
#else
			int i;

			/* clear rx and tx interrupts */
			for (i = 0; i < NFIFO; i++) {
				if (i == RX_FIFO)
					W_REG(osh, &regs->intctrlregs[i].intstatus,
					      DEF_RXINTMASK | I_XI);
				else if (wlc_hw->di[i] && dma_txactive(wlc_hw->di[i]))
					W_REG(osh, &regs->intctrlregs[i].intstatus, I_XI);
			}
#endif /* DMA_TX_FREE */
		}
	}

	return macintstatus;
}

/* Update wlc->macintstatus and wlc->intstatus[]. */
/* Return TRUE if they are updated successfully. FALSE otherwise */
bool
wlc_intrsupd(wlc_info_t *wlc)
{
	uint32 macintstatus;

	ASSERT(wlc->macintstatus != 0);

	/* read and clear macintstatus and intstatus registers */
	macintstatus = wlc_intstatus(wlc, FALSE);

	/* device is removed */
	if (macintstatus == 0xffffffff)
		return FALSE;

	/* update interrupt status in software */
	wlc->macintstatus |= macintstatus;

	return TRUE;
}

/*
 * First-level interrupt processing.
 * Return TRUE if this was our interrupt, FALSE otherwise.
 * *wantdpc will be set to TRUE if further wlc_dpc() processing is required,
 * FALSE otherwise.
 */
bool BCMFASTPATH
wlc_isr(wlc_info_t *wlc, bool *wantdpc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint32 macintstatus;

	*wantdpc = FALSE;

	if (!wlc_hw->up || !wlc->macintmask)
		return (FALSE);

	/* read and clear macintstatus and intstatus registers */
	macintstatus = wlc_intstatus(wlc, TRUE);

	if (macintstatus == 0xffffffff)
		WL_ERROR(("DEVICEREMOVED detected in the ISR code path.\n"));

	/* it is not for us */
	if (macintstatus == 0)
		return (FALSE);

	*wantdpc = TRUE;

#ifdef WLC_HIGH
#if defined(MBSS)
	/* BMAC_NOTE: This mechanism to bail out of sending beacons in
	 * wlc_ap.c:wlc_ap_sendbeacons() does not seem like a good idea across a bus with
	 * non-negligible reg-read time. The time values in the code have
	 * wlc_ap_sendbeacons() bail out if the delay between the isr time and it is >
	 * 4ms. But the check is done in wlc_ap_sendbeacons() with a reg read that might
	 * take on order of 0.3ms.  Should mbss only be supported with beacons in
	 * templates instead of beacons from host driver?
	 */
	if (MBSS_ENAB(wlc->pub) && (macintstatus & (MI_DTIM_TBTT | MI_TBTT))) {
		/* Record latest TBTT/DTIM interrupt time for latency calc */
		wlc->last_tbtt_us = R_REG(wlc_hw->osh, &wlc_hw->regs->tsf_timerlow);
	}
#endif /* MBSS */
#endif	/* WLC_HIGH */

	/* save interrupt status bits */
	ASSERT(wlc->macintstatus == 0);
	wlc->macintstatus = macintstatus;

#if defined(BCMDBG) && !defined(WLC_LOW_ONLY)
	wlc_update_isr_stats(wlc, macintstatus);
#endif
	return (TRUE);

}

/* process tx completion events for corerev < 5 */
static bool
wlc_bmac_txstatus_corerev4(wlc_hw_info_t *wlc_hw)
{
	void *status_p;
	tx_status_cr4_t *txscr4;
	tx_status_t txs;
	osl_t *osh;
	bool fatal = FALSE;
	uint32 tsf_time;
	d11regs_t *regs;

	regs = wlc_hw->regs;

	WL_TRACE(("wl%d: wlc_txstatusrecv\n", wlc_hw->unit));

	osh = wlc_hw->osh;

	/* To avoid overhead time is read only once for the whole while loop
	 * since time accuracy is not a concern for now.
	 */
	tsf_time = R_REG(osh, &regs->tsf_timerlow);

	while (!fatal && (PIO_ENAB(wlc_hw->wlc->pub) ?
	                  (status_p = wlc_pio_rx(wlc_hw->pio[RX_TXSTATUS_FIFO])) :
	                  (status_p = dma_rx(wlc_hw->di[RX_TXSTATUS_FIFO])))) {

		txscr4 = (tx_status_cr4_t *)PKTDATA(osh, status_p);
		/* MAC uses little endian only */
		ltoh16_buf((void*)txscr4, sizeof(tx_status_cr4_t));

		/* shift low bits for tx_status_t status compatibility */
		txscr4->status = (txscr4->status & ~TXS_COMPAT_MASK)
			| (((txscr4->status & TXS_COMPAT_MASK) << TXS_COMPAT_SHIFT));

		txs.framelen = 		txscr4->framelen;
		txs.frameid = 		txscr4->frameid;
		txs.status = 		txscr4->status;
		txs.sequence = 		txscr4->sequence;
		txs.phyerr = 		txscr4->phyerr;
		txs.ackphyrxsh = 	txscr4->ackphyrxsh;

		txs.lasttxtime = tsf_time;

		fatal = wlc_bmac_dotxstatus(wlc_hw, &txs, 0);

		PKTFREE(osh, status_p, FALSE);
	}

	if (fatal)
		return TRUE;

	/* post more rbufs */
	if (!PIO_ENAB(wlc_hw->wlc->pub))
		dma_rxfill(wlc_hw->di[RX_TXSTATUS_FIFO]);

	return FALSE;
}

static bool BCMFASTPATH
wlc_bmac_dotxstatus(wlc_hw_info_t *wlc_hw, tx_status_t *txs, uint32 s2)
{
#ifdef WL_MULTIQUEUE
	if (wlc_hw->wlc->txfifo_detach_pending)
		WL_MQ(("MQ: %s: sync processing of txstatus\n", __FUNCTION__));
#endif /* WL_MULTIQUEUE */

	/* discard intermediate indications for ucode with one legitimate case:
	 *   e.g. if "useRTS" is set. ucode did a successful rts/cts exchange, but the subsequent
	 *   tx of DATA failed. so it will start rts/cts from the beginning (resetting the rts
	 *   transmission count)
	 */
	if (!(txs->status & TX_STATUS_AMPDU) && (txs->status & TX_STATUS_INTERMEDIATE)) {
		return FALSE;
	}

	return wlc_dotxstatus(wlc_hw->wlc, txs, s2);
}

/* process tx completion events in BMAC
 * Return TRUE if more tx status need to be processed. FALSE otherwise.
 */
static bool BCMFASTPATH
wlc_bmac_txstatus(wlc_hw_info_t *wlc_hw, bool bound, bool *fatal)
{
	bool morepending = FALSE;
	wlc_info_t *wlc = wlc_hw->wlc;

	WL_TRACE(("wl%d: wlc_bmac_txstatus\n", wlc_hw->unit));

	if (D11REV_IS(wlc_hw->corerev, 4)) {
		/* to retire soon */
		*fatal = wlc_bmac_txstatus_corerev4(wlc->hw);

		if (*fatal)
			return 0;
	} else {
		/* corerev >= 5 */
		d11regs_t *regs;
		osl_t *osh;
		tx_status_t txs;
		uint32 s1, s2;
		uint n = 0;
		/* Param 'max_tx_num' indicates max. # tx status to process before break out. */
		uint max_tx_num = bound ? wlc->pub->tunables->txsbnd : -1;
		uint32 tsf_time;

		regs = wlc_hw->regs;
		osh = wlc_hw->osh;

		/* To avoid overhead time is read only once for the whole while loop
		 * since time accuracy is not a concern for now.
		 */
		tsf_time = R_REG(osh, &regs->tsf_timerlow);

		while (!(*fatal) && (s1 = R_REG(osh, &regs->frmtxstatus)) & TXS_V) {

			if (s1 == 0xffffffff) {
				WL_ERROR(("wl%d: %s: dead chip\n",
					wlc_hw->unit, __FUNCTION__));
				ASSERT(s1 != 0xffffffff);
				return morepending;
			}

			s2 = R_REG(osh, &regs->frmtxstatus2);

			txs.status = s1 & TXS_STATUS_MASK;
			txs.frameid = (s1 & TXS_FID_MASK) >> TXS_FID_SHIFT;
			txs.sequence = s2 & TXS_SEQ_MASK;
			txs.phyerr = (s2 & TXS_PTX_MASK) >> TXS_PTX_SHIFT;
			txs.lasttxtime = tsf_time;

			*fatal = wlc_bmac_dotxstatus(wlc_hw, &txs, s2);

			/* !give others some time to run! */
			if (++n >= max_tx_num)
				break;
		}

		if (*fatal)
			return 0;

#ifdef WLLMAC
		if (LMAC_ENAB(wlc->pub))
			wlc_lmac_txstatus_sendup(wlc->lmac_info, FALSE);
#endif
		if (n >= max_tx_num)
			morepending = TRUE;
	}

#ifdef WLC_HIGH
	if (wlc->active_queue != NULL && !pktq_empty(&wlc->active_queue->q))
		wlc_send_q(wlc, wlc->active_queue);
#endif

	return morepending;
}

#if defined(STA) && defined(WLRM)
static uint16
wlc_bmac_read_ihr(wlc_hw_info_t *wlc_hw, uint offset)
{
	d11regs_t *regs = wlc_hw->regs;
	volatile uint16* objdata_lo = (volatile uint16*)(uintptr)&regs->objdata;
	uint16 v;

	W_REG(wlc_hw->osh, &regs->objaddr, OBJADDR_IHR_SEL | offset);
	(void)R_REG(wlc_hw->osh, &regs->objaddr);
	v = (uint16)R_REG(wlc_hw->osh, objdata_lo);

	return v;
}
#endif  /* STA && WLRM */

#if (defined(STA) && defined(WLRM)) || defined(AP)
void
wlc_bmac_write_ihr(wlc_hw_info_t *wlc_hw, uint offset, uint16 v)
{
	d11regs_t *regs = wlc_hw->regs;
	volatile uint16* objdata_lo = (volatile uint16*)(uintptr)&regs->objdata;

	W_REG(wlc_hw->osh, &regs->objaddr, OBJADDR_IHR_SEL | offset);
	(void)R_REG(wlc_hw->osh, &regs->objaddr);
	W_REG(wlc_hw->osh, objdata_lo, v);
}
#endif  /* STA && WLRM || AP */

void
wlc_bmac_suspend_mac_and_wait(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	d11regs_t *regs = wlc_hw->regs;
	uint32 mc, mi;
	osl_t *osh;

	wlc = wlc; /* avoid unused variable warning */

	WL_TRACE(("wl%d: wlc_bmac_suspend_mac_and_wait: bandunit %d\n", wlc_hw->unit,
		wlc_hw->band->bandunit));

	/*
	 * Track overlapping suspend requests
	 */
	wlc_hw->mac_suspend_depth++;
	if (wlc_hw->mac_suspend_depth > 1)
		return;

	osh = wlc_hw->osh;

#ifdef STA
	/* force the core awake */
	wlc_ucode_wake_override_set(wlc_hw, WLC_WAKE_OVERRIDE_MACSUSPEND);

	if ((wlc_hw->btc->wire >= WL_BTC_3WIRE) &&
	    D11REV_LT(wlc_hw->corerev, 13)) {
		si_gpiocontrol(wlc_hw->sih, wlc_hw->btc->gpio_mask, 0, GPIO_DRV_PRIORITY);
	}
#endif /* STA */

	mc = R_REG(osh, &regs->maccontrol);

#ifdef WLC_HIGH	    /* BMAC: skip DEVICEREMOVED, not needed in low level driver. */
	if (mc == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}
#endif
	ASSERT(!(mc & MCTL_PSM_JMP_0));
	ASSERT(mc & MCTL_PSM_RUN);
	ASSERT(mc & MCTL_EN_MAC);

	mi = R_REG(osh, &regs->macintstatus);
#ifdef WLC_HIGH	    /* BMAC: skip DEVICEREMOVED, not needed in low level driver. */
	if (mi == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}
#endif
	ASSERT(!(mi & MI_MACSSPNDD));

	wlc_bmac_mctrl(wlc_hw, MCTL_EN_MAC, 0);

	SPINWAIT(!(R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD), WLC_MAX_MAC_SUSPEND);

	if (!(R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD)) {
		WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_SUSPEND_MAC_FAIL_ID, WL_LOG_LEVEL_ERR,
			0, R_REG(osh, &regs->psmdebug), NULL);
		WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
			0, R_REG(osh, &regs->phydebug), "phydebug");
		WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
			0, R_REG(osh, &regs->psm_brc), "psm_brc");
		WL_ERROR(("wl%d: wlc_bmac_suspend_mac_and_wait: waited %d uS and "
			"MI_MACSSPNDD is still not on.\n",
			wlc_hw->unit, WLC_MAX_MAC_SUSPEND));
		WL_ERROR(("wl%d: psmdebug 0x%08x, phydebug 0x%08x, psm_brc 0x%04x\n",
		          wlc_hw->unit, R_REG(osh, &regs->psmdebug),
		          R_REG(osh, &regs->phydebug), R_REG(osh, &regs->psm_brc)));
	}

	mc = R_REG(osh, &regs->maccontrol);
#ifdef WLC_HIGH	    /* BMAC: skip DEVICEREMOVED, not needed in low level driver. */
	if (mc == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}
#endif
	ASSERT(!(mc & MCTL_PSM_JMP_0));
	ASSERT(mc & MCTL_PSM_RUN);
	ASSERT(!(mc & MCTL_EN_MAC));
	wlc_bmac_mctrl(wlc_hw, MCTL_PSM_RUN, 0);
}


void
wlc_bmac_enable_mac(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	d11regs_t *regs = wlc_hw->regs;
	uint32 mc, mi;
	osl_t *osh;

	wlc = wlc; /* avoid unused variable warning */
	WL_TRACE(("wl%d: wlc_enable_mac: bandunit %d\n", wlc_hw->unit, wlc->band->bandunit));

	/*
	 * Track overlapping suspend requests
	 */
	ASSERT(wlc_hw->mac_suspend_depth > 0);
	wlc_hw->mac_suspend_depth--;
	if (wlc_hw->mac_suspend_depth > 0)
		return;

	osh = wlc_hw->osh;

	mc = R_REG(osh, &regs->maccontrol);
	ASSERT(!(mc & MCTL_PSM_JMP_0));
	ASSERT(!(mc & MCTL_EN_MAC));
	/* ASSERT(mc & MCTL_PSM_RUN); */

	wlc_bmac_mctrl(wlc_hw, (MCTL_EN_MAC | MCTL_PSM_RUN), (MCTL_EN_MAC | MCTL_PSM_RUN));
	W_REG(osh, &regs->macintstatus, MI_MACSSPNDD);

	mc = R_REG(osh, &regs->maccontrol);
	ASSERT(!(mc & MCTL_PSM_JMP_0));
	ASSERT(mc & MCTL_EN_MAC);
	ASSERT(mc & MCTL_PSM_RUN);

	mi = R_REG(osh, &regs->macintstatus);
	ASSERT(!(mi & MI_MACSSPNDD));

#ifdef STA
	wlc_ucode_wake_override_clear(wlc_hw, WLC_WAKE_OVERRIDE_MACSUSPEND);

	if ((wlc_hw->btc->wire >= WL_BTC_3WIRE) &&
	    D11REV_LT(wlc_hw->corerev, 13)) {
		si_gpiocontrol(wlc_hw->sih, wlc_hw->btc->gpio_mask, wlc_hw->btc->gpio_mask,
		               GPIO_DRV_PRIORITY);
	}
#endif /* STA */

#if defined(MBSS) && defined(WLC_HIGH) && defined(WLC_LOW)
	/* The PRQ fifo is reset on a mac suspend/resume; reset the SW read ptr */
	wlc->prq_rd_ptr = wlc->prq_base;
#endif /* MBSS && WLC_HIGH && WLC_LOW */
}

void
wlc_bmac_ifsctl_edcrs_set(wlc_hw_info_t *wlc_hw, bool abie, bool isht)
{
	if (!(WLCISNPHY(wlc_hw->band) && (D11REV_GE(wlc_hw->corerev, 16))) &&
	    !WLCISHTPHY(wlc_hw->band))
		return;

#ifdef WLAFTERBURNER
	/* EDCRS cannot be enabled with afterburner on */
	if (abie) {
		wlc_bmac_ifsctl1_regshm(wlc_hw, IFS_CTL1_EDCRS, 0);
		return;
	}
#endif

	if (isht) {
		if (WLCISNPHY(wlc_hw->band) &&
			NREV_LT(wlc_hw->band->phyrev, 3)) {
			wlc_bmac_ifsctl1_regshm(wlc_hw, IFS_CTL1_EDCRS, 0);
		}
	} else {
		/* enable EDCRS for non-11n association */
		wlc_bmac_ifsctl1_regshm(wlc_hw, IFS_CTL1_EDCRS, IFS_CTL1_EDCRS);
	}
	if (WLCISHTPHY(wlc_hw->band) ||
	    (WLCISNPHY(wlc_hw->band) && NREV_GE(wlc_hw->band->phyrev, 3))) {
		if (CHSPEC_IS20(wlc_hw->chanspec)) {
			/* 20 mhz, use 20U ED only */
			wlc_bmac_ifsctl1_regshm(wlc_hw,
				(IFS_CTL1_EDCRS | IFS_CTL1_EDCRS_20L | IFS_CTL1_EDCRS_40),
				IFS_CTL1_EDCRS);
		} else {
			/* 40 mhz, use 20U 20L and 40 ED */
			wlc_bmac_ifsctl1_regshm(wlc_hw,
				(IFS_CTL1_EDCRS | IFS_CTL1_EDCRS_20L | IFS_CTL1_EDCRS_40),
				(IFS_CTL1_EDCRS | IFS_CTL1_EDCRS_20L | IFS_CTL1_EDCRS_40));
		}
	}

#ifdef WLC_LOW_ONLY
	if (CHIPID(wlc_hw->sih->chip) == BCM4322_CHIP_ID ||
		CHIPID(wlc_hw->sih->chip) == BCM43231_CHIP_ID) {

		wlc_phy_edcrs_lock(wlc_hw->band->pi, !isht);
	}
#endif
}

#ifdef WL11N
static void
wlc_upd_ofdm_pctl1_table(wlc_hw_info_t *wlc_hw)
{
	uint8 rate;
	uint8 rates[8] = {
		WLC_RATE_6M, WLC_RATE_9M, WLC_RATE_12M, WLC_RATE_18M,
		WLC_RATE_24M, WLC_RATE_36M, WLC_RATE_48M, WLC_RATE_54M
	};

	uint16 rate_phyctl1[8] = {0x0002, 0x0202, 0x0802, 0x0a02, 0x1002, 0x1202, 0x1902, 0x1a02};

	uint16 entry_ptr;
	uint16 pctl1, phyctl;
	uint i;
	bool btcx_ack_ant1 = ((wlc_hw->boardflags2 & BFL2_BT_SHARE_ANT0) == BFL2_BT_SHARE_ANT0) &&
		wlc_bmac_btc_mode_get(wlc_hw);

	if (!WLC_PHY_11N_CAP(wlc_hw->band))
		return;

	/* walk the phy rate table and update the entries */
	for (i = 0; i < ARRAYSIZE(rates); i++) {
		rate = rates[i];

		entry_ptr = wlc_bmac_ofdm_ratetable_offset(wlc_hw, rate);

		/* read the SHM Rate Table entry OFDM PCTL1 values */
		pctl1 = wlc_bmac_read_shm(wlc_hw, entry_ptr + M_RT_OFDM_PCTL1_POS);

		/* modify the MODE & code_rate value */
		if (D11REV_IS(wlc_hw->corerev, 31) && WLCISNPHY(wlc_hw->band)) {
			/* corerev31 uses corerev29 ucode, where PHY_CTL_1 inits is for HTPHY
			 * fix it to OFDM rate
			 */
			pctl1 &= (PHY_TXC1_MODE_MASK | PHY_TXC1_BW_MASK);
			pctl1 |= (rate_phyctl1[i] & 0xFFC0);
		}

		if (D11REV_IS(wlc_hw->corerev, 29) &&
			WLCISHTPHY(wlc_hw->band) &&
			AMPDU_HW_ENAB(wlc_hw->wlc->pub)) {
			pctl1 &= ~PHY_TXC1_BW_MASK;
			if (CHSPEC_WLC_BW(wlc_hw->chanspec) == WLC_40_MHZ)
				pctl1 |= PHY_TXC1_BW_40MHZ_DUP;
			else
				pctl1 |= PHY_TXC1_BW_20MHZ;
		}

		/* modify the STF value */
		pctl1 &= ~PHY_TXC1_MODE_MASK;
		if (btcx_ack_ant1)
			pctl1 |= (PHY_TXC1_MODE_SISO << PHY_TXC1_MODE_SHIFT);
		else
			pctl1 |= (wlc_hw->hw_stf_ss_opmode << PHY_TXC1_MODE_SHIFT);

		/* Update the SHM Rate Table entry OFDM PCTL1 values */
		wlc_bmac_write_shm(wlc_hw, entry_ptr + M_RT_OFDM_PCTL1_POS, pctl1);
	}

	if (btcx_ack_ant1) {
		/* set the Response (ACK/CTS) frame phy control word */
		phyctl = wlc_bmac_read_shm(wlc_hw, M_RSP_PCTLWD);
		phyctl = (phyctl & ~PHY_TXC_ANT_MASK) | PHY_TXC_ANT_1;
		wlc_bmac_write_shm(wlc_hw, M_RSP_PCTLWD, phyctl);
	}
}

static uint16
wlc_bmac_ofdm_ratetable_offset(wlc_hw_info_t *wlc_hw, uint8 rate)
{
	uint i;
	uint8 plcp_rate = 0;
	struct plcp_signal_rate_lookup {
		uint8 rate;
		uint8 signal_rate;
	};
	/* OFDM RATE sub-field of PLCP SIGNAL field, per 802.11 sec 17.3.4.1 */
	const struct plcp_signal_rate_lookup rate_lookup[] = {
		{WLC_RATE_6M,  0xB},
		{WLC_RATE_9M,  0xF},
		{WLC_RATE_12M, 0xA},
		{WLC_RATE_18M, 0xE},
		{WLC_RATE_24M, 0x9},
		{WLC_RATE_36M, 0xD},
		{WLC_RATE_48M, 0x8},
		{WLC_RATE_54M, 0xC}
	};

	for (i = 0; i < ARRAYSIZE(rate_lookup); i++) {
		if (rate == rate_lookup[i].rate) {
			plcp_rate = rate_lookup[i].signal_rate;
			break;
		}
	}

	/* Find the SHM pointer to the rate table entry by looking in the
	 * Direct-map Table
	 */
	return (2*wlc_bmac_read_shm(wlc_hw, M_RT_DIRMAP_A + (plcp_rate * 2)));
}

void
wlc_bmac_band_stf_ss_set(wlc_hw_info_t *wlc_hw, uint8 stf_mode)
{
	wlc_hw->hw_stf_ss_opmode = stf_mode;

	if (wlc_hw->clk)
		wlc_upd_ofdm_pctl1_table(wlc_hw);
}

void
wlc_bmac_txbw_update(wlc_hw_info_t *wlc_hw)
{
	if (wlc_hw->clk)
		wlc_upd_ofdm_pctl1_table(wlc_hw);

}
#endif /* WL11N */

void BCMFASTPATH
wlc_bmac_read_tsf(wlc_hw_info_t* wlc_hw, uint32* tsf_l_ptr, uint32* tsf_h_ptr)
{
	d11regs_t *regs = wlc_hw->regs;

	/* read the tsf timer low, then high to get an atomic read */
	*tsf_l_ptr = R_REG(wlc_hw->osh, &regs->tsf_timerlow);
	*tsf_h_ptr = R_REG(wlc_hw->osh, &regs->tsf_timerhigh);

	return;
}

bool
#ifdef WLDIAG
wlc_bmac_validate_chip_access(wlc_hw_info_t *wlc_hw)
#else
BCMATTACHFN(wlc_bmac_validate_chip_access)(wlc_hw_info_t *wlc_hw)
#endif
{
	d11regs_t *regs;
	uint32 w, val;
	volatile uint16 *reg16;
	osl_t *osh;

	WL_TRACE(("wl%d: validate_chip_access\n", wlc_hw->unit));

	regs = wlc_hw->regs;
	osh = wlc_hw->osh;

	/* Validate dchip register access */

	W_REG(osh, &regs->objaddr, OBJADDR_SHM_SEL | 0);
	(void)R_REG(osh, &regs->objaddr);
	w = R_REG(osh, &regs->objdata);

	/* Can we write and read back a 32bit register? */
	W_REG(osh, &regs->objaddr, OBJADDR_SHM_SEL | 0);
	(void)R_REG(osh, &regs->objaddr);
	W_REG(osh, &regs->objdata, (uint32)0xaa5555aa);

	W_REG(osh, &regs->objaddr, OBJADDR_SHM_SEL | 0);
	(void)R_REG(osh, &regs->objaddr);
	val = R_REG(osh, &regs->objdata);
	if (val != (uint32)0xaa5555aa) {
		WL_ERROR(("wl%d: validate_chip_access: SHM = 0x%x, expected 0xaa5555aa\n",
			wlc_hw->unit, val));
		return (FALSE);
	}

	W_REG(osh, &regs->objaddr, OBJADDR_SHM_SEL | 0);
	(void)R_REG(osh, &regs->objaddr);
	W_REG(osh, &regs->objdata, (uint32)0x55aaaa55);

	W_REG(osh, &regs->objaddr, OBJADDR_SHM_SEL | 0);
	(void)R_REG(osh, &regs->objaddr);
	val = R_REG(osh, &regs->objdata);
	if (val != (uint32)0x55aaaa55) {
		WL_ERROR(("wl%d: validate_chip_access: SHM = 0x%x, expected 0x55aaaa55\n",
			wlc_hw->unit, val));
		return (FALSE);
	}

	W_REG(osh, &regs->objaddr, OBJADDR_SHM_SEL | 0);
	(void)R_REG(osh, &regs->objaddr);
	W_REG(osh, &regs->objdata, w);

	if (D11REV_LT(wlc_hw->corerev, 11)) {
		/* if 32 bit writes are split into 16 bit writes, are they in the correct order
		 * for our interface, low to high
		 */
		reg16 = (volatile uint16*)(uintptr)&regs->tsf_cfpstart;

		/* write the CFPStart register low half explicitly, starting a buffered write */
		W_REG(osh, reg16, 0xAAAA);

		/* Write a 32 bit value to CFPStart to test the 16 bit split order.
		 * If the low 16 bits are written first, followed by the high 16 bits then the
		 * 32 bit value 0xCCCCBBBB should end up in the register.
		 * If the order is reversed, then the write to the high half will trigger a buffered
		 * write of 0xCCCCAAAA.
		 * If the bus is 32 bits, then this is not much of a test, and the reg should
		 * have the correct value 0xCCCCBBBB.
		 */
		W_REG(osh, &regs->tsf_cfpstart, 0xCCCCBBBB);

		/* verify with the 16 bit registers that have no side effects */
		val = R_REG(osh, &regs->tsf_cfpstrt_l);
		if (val != (uint)0xBBBB) {
			WL_ERROR(("wl%d: validate_chip_access: tsf_cfpstrt_l = 0x%x, expected"
				" 0x%x\n",
				wlc_hw->unit, val, 0xBBBB));
			return (FALSE);
		}
		val = R_REG(osh, &regs->tsf_cfpstrt_h);
		if (val != (uint)0xCCCC) {
			WL_ERROR(("wl%d: validate_chip_access: tsf_cfpstrt_h = 0x%x, expected"
				" 0x%x\n",
				wlc_hw->unit, val, 0xCCCC));
			return (FALSE);
		}

	}

	/* clear CFPStart */
	W_REG(osh, &regs->tsf_cfpstart, 0);

	w = R_REG(osh, &regs->maccontrol);
	if ((w != (MCTL_IHR_EN | MCTL_WAKE)) &&
	    (w != (MCTL_IHR_EN | MCTL_GMODE | MCTL_WAKE))) {
		WL_ERROR(("wl%d: validate_chip_access: maccontrol = 0x%x, expected 0x%x or 0x%x\n",
		          wlc_hw->unit, w, (MCTL_IHR_EN | MCTL_WAKE),
		          (MCTL_IHR_EN | MCTL_GMODE | MCTL_WAKE)));
		return (FALSE);
	}

	return (TRUE);
}

#define PHYPLL_WAIT_US	100000

void
wlc_bmac_core_phypll_ctl(wlc_hw_info_t* wlc_hw, bool on)
{
	d11regs_t *regs;
	osl_t *osh;
	uint32 tmp;
	uint32 req_bits;
	uint32 avail_bits;

	WL_TRACE(("wl%d: wlc_bmac_core_phypll_ctl\n", wlc_hw->unit));

	regs = wlc_hw->regs;
	osh = wlc_hw->osh;

	if (D11REV_LE(wlc_hw->corerev, 16) || D11REV_IS(wlc_hw->corerev, 20))
		return;

	if (wlc_hw->sih->chip == BCM4313_CHIP_ID) {
		req_bits = CCS_ERSRC_REQ_D11PLL | CCS_ERSRC_REQ_PHYPLL | CCS_ERSRC_REQ_HT;
		avail_bits = CCS_ERSRC_AVAIL_HT;
	} else {
		req_bits = CCS_ERSRC_REQ_D11PLL | CCS_ERSRC_REQ_PHYPLL;
		avail_bits = CCS_ERSRC_AVAIL_D11PLL | CCS_ERSRC_AVAIL_PHYPLL;
	}

	if (on) {
		OR_REG(osh, &regs->clk_ctl_st, req_bits);
		SPINWAIT((R_REG(osh, &regs->clk_ctl_st) & avail_bits) != avail_bits,
		         PHYPLL_WAIT_US);

		tmp = R_REG(osh, &regs->clk_ctl_st);
		if ((tmp & avail_bits) != avail_bits) {
			WL_ERROR(("%s: turn on PHY PLL failed\n", __FUNCTION__));
			ASSERT(0);
		}
	} else {
		/* Since the PLL may be shared, other cores can still be requesting it;
		 * so we'll deassert the request but not wait for status to comply.
		 */
		AND_REG(osh, &regs->clk_ctl_st, ~req_bits);
	}
}

void
wlc_coredisable(wlc_hw_info_t* wlc_hw)
{
	bool dev_gone;

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	ASSERT(!wlc_hw->up);

	dev_gone = DEVICEREMOVED(wlc_hw->wlc);

	if (dev_gone)
		return;

	if (wlc_hw->noreset)
		return;

	/* radio off */
	wlc_phy_switch_radio(wlc_hw->band->pi, OFF);

	/* turn off analog core */
	wlc_phy_anacore(wlc_hw->band->pi, OFF);

	/* turn off PHYPLL to save power */
	wlc_bmac_core_phypll_ctl(wlc_hw, FALSE);

	/* No need to set wlc->pub->radio_active = OFF
	 * because this function needs down capability and
	 * radio_active is designed for BCMNODOWN.
	 */

	/* remove gpio controls */
	if (wlc_hw->ucode_dbgsel)
		si_gpiocontrol(wlc_hw->sih, ~0, 0, GPIO_DRV_PRIORITY);

	wlc_hw->clk = FALSE;
	si_core_disable(wlc_hw->sih, 0);
	wlc_phy_hw_clk_state_upd(wlc_hw->band->pi, FALSE);
}

/* power both the pll and external oscillator on/off */
void
wlc_bmac_xtal(wlc_hw_info_t* wlc_hw, bool want)
{
	WL_TRACE(("wl%d: wlc_bmac_xtal: want %d\n", wlc_hw->unit, want));

	/* dont power down if plldown is false or we must poll hw radio disable */
	if (!want && wlc_hw->pllreq)
		return;

	if (wlc_hw->sih)
		si_clkctl_xtal(wlc_hw->sih, XTAL|PLL, want);

	wlc_hw->sbclk = want;
	if (!wlc_hw->sbclk) {
		wlc_hw->clk = FALSE;
		if (wlc_hw->band && wlc_hw->band->pi)
			wlc_phy_hw_clk_state_upd(wlc_hw->band->pi, FALSE);
	}
}

static void
wlc_flushqueues(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	uint i;

	if (!PIO_ENAB(wlc->pub)) {
		wlc->txpend16165war = 0;

		/* free any posted tx packets */
		for (i = 0; i < NFIFO; i++)
			if (wlc_hw->di[i]) {
				dma_txreclaim(wlc_hw->di[i], HNDDMA_RANGE_ALL);
				TXPKTPENDCLR(wlc, i);
				WL_TRACE(("wlc_flushqueues: pktpend fifo %d cleared\n", i));
#if defined(DMA_TX_FREE)
				WL_TRACE(("wlc_flushqueues: ampdu_flags cleared, head %d tail %d\n",
				          wlc_hw->txstatus_ampdu_flags[i].head,
				          wlc_hw->txstatus_ampdu_flags[i].tail));
				wlc_hw->txstatus_ampdu_flags[i].head = 0;
				wlc_hw->txstatus_ampdu_flags[i].tail = 0;
#endif
			}

		/* free any posted rx packets */
		dma_rxreclaim(wlc_hw->di[RX_FIFO]);
		if (D11REV_IS(wlc_hw->corerev, 4))
			dma_rxreclaim(wlc_hw->di[RX_TXSTATUS_FIFO]);
	} else {
		for (i = 0; i < NFIFO; i++) {
			if (wlc_hw->pio[i]) {
				/* include reset the counter */
				wlc_pio_txreclaim(wlc_hw->pio[i]);
			}
		}
		/* For PIO, no rx sw queue to reclaim */
	}
}

#ifdef STA
#ifdef WLRM
/* start a CCA measurement for the given number of microseconds */
void
wlc_bmac_rm_cca_measure(wlc_hw_info_t *wlc_hw, uint32 us)
{
	uint32 gpt_ticks;

	/* convert dur in TUs to 1/8 us units for GPT */
	gpt_ticks = us << 3;

	/* config GPT 2 to decrement by TSF ticks */
	wlc_bmac_write_ihr(wlc_hw, TSF_GPT_2_STAT, TSF_GPT_USETSF);
	/* set GPT 2 to the measurement duration */
	wlc_bmac_write_ihr(wlc_hw, TSF_GPT_2_CTR_L, (gpt_ticks & 0xffff));
	wlc_bmac_write_ihr(wlc_hw, TSF_GPT_2_CTR_H, (gpt_ticks >> 16));
	/* tell ucode to start the CCA measurement */
	OR_REG(wlc_hw->osh, &wlc_hw->regs->maccommand, MCMD_CCA);

	return;
}

void
wlc_bmac_rm_cca_int(wlc_hw_info_t *wlc_hw)
{
	uint32 cca_idle;
	uint32 cca_idle_us;
	uint32 gpt2_h, gpt2_l;

	gpt2_l = wlc_bmac_read_ihr(wlc_hw, TSF_GPT_2_VAL_L);
	gpt2_h = wlc_bmac_read_ihr(wlc_hw, TSF_GPT_2_VAL_H);
	cca_idle = (gpt2_h << 16) | gpt2_l;

	/* convert GTP 1/8 us units to us */
	cca_idle_us = (cca_idle >> 3);

	wlc_rm_cca_complete(wlc_hw->wlc, cca_idle_us);
}
#endif /* WLRM */
#endif /* STA */

/* set the PIO mode bit in the control register for the rxfifo */
void
wlc_rxfifo_setpio(wlc_hw_info_t *wlc_hw)
{
	if (D11REV_LT(wlc_hw->corerev, 11)) {
		fifo32_t *fiforegs;

		fiforegs = &wlc_hw->regs->fifo.f32regs;
		W_REG(wlc_hw->osh, &fiforegs->dmaregs[RX_FIFO].rcv.control, RC_FM);
		if (D11REV_IS(wlc_hw->corerev, 4))
			W_REG(wlc_hw->osh,
				&fiforegs->dmaregs[RX_TXSTATUS_FIFO].rcv.control, RC_FM);
	} else {
		fifo64_t *fiforegs;

		fiforegs = &wlc_hw->regs->fifo.f64regs[RX_FIFO];
		W_REG(wlc_hw->osh, &fiforegs->dmarcv.control, D64_RC_FM);
	}
}

uint16
wlc_bmac_read_shm(wlc_hw_info_t *wlc_hw, uint offset)
{
	return  wlc_bmac_read_objmem(wlc_hw, offset, OBJADDR_SHM_SEL);
}

void
wlc_bmac_write_shm(wlc_hw_info_t *wlc_hw, uint offset, uint16 v)
{
	wlc_bmac_write_objmem(wlc_hw, offset, v, OBJADDR_SHM_SEL);
}

/* Set a range of shared memory to a value.
 * SHM 'offset' needs to be an even address and
 * Buffer length 'len' must be an even number of bytes
 */
void
wlc_bmac_set_shm(wlc_hw_info_t *wlc_hw, uint offset, uint16 v, int len)
{
	int i;

	/* offset and len need to be even */
	ASSERT((offset & 1) == 0);
	ASSERT((len & 1) == 0);

	if (len <= 0)
		return;

	for (i = 0; i < len; i += 2) {
		wlc_bmac_write_objmem(wlc_hw, offset + i, v, OBJADDR_SHM_SEL);
	}
}

static uint16
wlc_bmac_read_objmem(wlc_hw_info_t *wlc_hw, uint offset, uint32 sel)
{
	d11regs_t *regs = wlc_hw->regs;
	volatile uint16* objdata_lo = (volatile uint16*)(uintptr)&regs->objdata;
	volatile uint16* objdata_hi = objdata_lo + 1;
	uint16 v;

	ASSERT((offset & 1) == 0);

	W_REG(wlc_hw->osh, &regs->objaddr, sel | (offset >> 2));
	(void)R_REG(wlc_hw->osh, &regs->objaddr);
	if (offset & 2) {
		v = R_REG(wlc_hw->osh, objdata_hi);
	} else {
		v = R_REG(wlc_hw->osh, objdata_lo);
	}

	return v;
}

static void
wlc_bmac_write_objmem(wlc_hw_info_t *wlc_hw, uint offset, uint16 v, uint32 sel)
{
	d11regs_t *regs = wlc_hw->regs;
	volatile uint16* objdata_lo = (volatile uint16*)(uintptr)&regs->objdata;
	volatile uint16* objdata_hi = objdata_lo + 1;

	ASSERT((offset & 1) == 0);

	W_REG(wlc_hw->osh, &regs->objaddr, sel | (offset >> 2));
	(void)R_REG(wlc_hw->osh, &regs->objaddr);
	if (offset & 2) {
		W_REG(wlc_hw->osh, objdata_hi, v);
	} else {
		W_REG(wlc_hw->osh, objdata_lo, v);
	}
}

/* Copy a buffer to shared memory of specified type .
 * SHM 'offset' needs to be an even address and
 * Buffer length 'len' must be an even number of bytes
 * 'sel' selects the type of memory
 */
void
wlc_bmac_copyto_objmem(wlc_hw_info_t *wlc_hw, uint offset, const void* buf, int len, uint32 sel)
{
	uint16 v;
	const uint8* p = (const uint8*)buf;
	int i;

	/* offset and len need to be even */
	ASSERT((offset & 1) == 0);
	ASSERT((len & 1) == 0);

	if (len <= 0)
		return;

	for (i = 0; i < len; i += 2) {
		v = p[i] | (p[i+1] << 8);
		wlc_bmac_write_objmem(wlc_hw, offset + i, v, sel);
	}
}

/* Copy a piece of shared memory of specified type to a buffer .
 * SHM 'offset' needs to be an even address and
 * Buffer length 'len' must be an even number of bytes
 * 'sel' selects the type of memory
 */
void
wlc_bmac_copyfrom_objmem(wlc_hw_info_t *wlc_hw, uint offset, void* buf, int len, uint32 sel)
{
	uint16 v;
	uint8* p = (uint8*)buf;
	int i;

	/* offset and len need to be even */
	ASSERT((offset & 1) == 0);
	ASSERT((len & 1) == 0);

	if (len <= 0)
		return;

	for (i = 0; i < len; i += 2) {
		v = wlc_bmac_read_objmem(wlc_hw, offset + i, sel);
		p[i] = v & 0xFF;
		p[i+1] = (v >> 8) & 0xFF;
	}
}

void
wlc_bmac_copyfrom_vars(wlc_hw_info_t *wlc_hw, char ** buf, uint *len)
{
	WL_TRACE(("wlc_bmac_copyfrom_vars, nvram vars totlen=%d\n", wlc_hw->vars_size));
	if (wlc_hw->vars) {
		*buf = wlc_hw->vars;
		*len = wlc_hw->vars_size;
	}
#ifdef WLC_LOW_ONLY
	else {
		/* no per device vars, return the global one */
		nvram_get_global_vars(buf, len);
	}
#endif
}

void
wlc_bmac_retrylimit_upd(wlc_hw_info_t *wlc_hw, uint16 SRL, uint16 LRL)
{
	wlc_hw->SRL = SRL;
	wlc_hw->LRL = LRL;

	/* write retry limit to SCR, shouldn't need to suspend */
	if (wlc_hw->up) {
		W_REG(wlc_hw->osh, &wlc_hw->regs->objaddr, OBJADDR_SCR_SEL | S_DOT11_SRC_LMT);
		(void)R_REG(wlc_hw->osh, &wlc_hw->regs->objaddr);
		W_REG(wlc_hw->osh, &wlc_hw->regs->objdata, wlc_hw->SRL);
		W_REG(wlc_hw->osh, &wlc_hw->regs->objaddr, OBJADDR_SCR_SEL | S_DOT11_LRC_LMT);
		(void)R_REG(wlc_hw->osh, &wlc_hw->regs->objaddr);
		W_REG(wlc_hw->osh, &wlc_hw->regs->objdata, wlc_hw->LRL);
	}
}

void
wlc_bmac_set_noreset(wlc_hw_info_t *wlc_hw, bool noreset_flag)
{
	wlc_hw->noreset = noreset_flag;
}

void
wlc_bmac_set_ucode_loaded(wlc_hw_info_t *wlc_hw, bool ucode_loaded)
{
	wlc_hw->ucode_loaded = ucode_loaded;
}

void
wlc_bmac_p2p_set(wlc_hw_info_t *wlc_hw, bool enable)
{
	wlc_hw->_p2p_hw = enable;
}

void
wlc_bmac_pllreq(wlc_hw_info_t *wlc_hw, bool set, mbool req_bit)
{
	ASSERT(req_bit);

	if (set) {
		if (mboolisset(wlc_hw->pllreq, req_bit))
			return;

		mboolset(wlc_hw->pllreq, req_bit);

		if (mboolisset(wlc_hw->pllreq, WLC_PLLREQ_FLIP)) {
			if (!wlc_hw->sbclk) {
				wlc_bmac_xtal(wlc_hw, ON);
			}
		}
	}
	else {
		if (!mboolisset(wlc_hw->pllreq, req_bit))
			return;

		mboolclr(wlc_hw->pllreq, req_bit);

		if (mboolisset(wlc_hw->pllreq, WLC_PLLREQ_FLIP)) {
			if (wlc_hw->sbclk) {
				wlc_bmac_xtal(wlc_hw, OFF);
			}
		}
	}

	return;
}

void
wlc_bmac_set_clk(wlc_hw_info_t *wlc_hw, bool on)
{
	if (on) {
		/* power up pll and oscillator */
		wlc_bmac_xtal(wlc_hw, ON);

		/* enable core(s), ignore bandlocked
		 * Leave with the same band selected as we entered
		 */
		wlc_bmac_corereset(wlc_hw, WLC_USE_COREFLAGS);
	} else {
		/* if already down, must skip the core disable */
		if (wlc_hw->clk) {
			/* disable core(s), ignore bandlocked */
			wlc_coredisable(wlc_hw);
		}
			/* power down pll and oscillator */
		wlc_bmac_xtal(wlc_hw, OFF);
	}
}

#ifdef BCMASSERT_SUPPORT
bool
wlc_bmac_taclear(wlc_hw_info_t *wlc_hw, bool ta_ok)
{
	return (!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, !ta_ok));
}
#endif

#ifdef WLLED
/* may touch sb register inside */
void
wlc_bmac_led_hw_deinit(wlc_hw_info_t *wlc_hw, uint32 gpiomask_cache)
{
	/* BMAC_NOTE: split mac should not worry about pci cfg access to disable GPIOs. */
	bool xtal_set = FALSE;

	if (!wlc_hw->sbclk) {
		wlc_bmac_xtal(wlc_hw, ON);
		xtal_set = TRUE;
	}

	/* opposite sequence of wlc_led_init */
	if (wlc_hw->sih) {
		si_gpioout(wlc_hw->sih, gpiomask_cache, 0, GPIO_DRV_PRIORITY);
		si_gpioouten(wlc_hw->sih, gpiomask_cache, 0, GPIO_DRV_PRIORITY);
		si_gpioled(wlc_hw->sih, gpiomask_cache, 0);
	}

	if (xtal_set)
		wlc_bmac_xtal(wlc_hw, OFF);
}

void
wlc_bmac_led_hw_mask_init(wlc_hw_info_t *wlc_hw, uint32 mask)
{
	wlc_hw->led_gpio_mask = mask;
}

static void
wlc_bmac_led_hw_init(wlc_hw_info_t *wlc_hw)
{
	uint32 mask = wlc_hw->led_gpio_mask, val = 0;
	struct bmac_led *led;
	bmac_led_info_t *li = wlc_hw->ledh;


	if (!wlc_hw->sbclk)
		return;

	/* designate gpios driving LEDs . Make sure that we have the control */
	si_gpiocontrol(wlc_hw->sih, mask, 0, GPIO_DRV_PRIORITY);
	si_gpioled(wlc_hw->sih, mask, mask);

	/* Begin with LEDs off */
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (!led->activehi)
			val |= (1 << led->pin);
	}
	val = val & mask;

	if (!(wlc_hw->boardflags2 & BFL2_TRISTATE_LED)) {
		li->gpioout_cache = si_gpioout(wlc_hw->sih, mask, val, GPIO_DRV_PRIORITY);
		si_gpioouten(wlc_hw->sih, mask, mask, GPIO_DRV_PRIORITY);
	} else {
		si_gpioout(wlc_hw->sih, mask, !val, GPIO_DRV_PRIORITY);
		li->gpioout_cache = si_gpioouten(wlc_hw->sih, mask, 0, GPIO_DRV_PRIORITY);
		/* for tristate leds, clear gpiopullup/gpiopulldown registers to
		 * allow the tristated gpio to float
		 */
		if (wlc_hw->sih->ccrev >= 20) {
			si_gpiopull(wlc_hw->sih, GPIO_PULLDN, mask, 0);
			si_gpiopull(wlc_hw->sih, GPIO_PULLUP, mask, 0);
		}
	}

	li->gpiomask_cache = mask;
}

/* called by the led_blink_timer at every li->led_blink_time interval */
static void
wlc_bmac_led_blink_timer(bmac_led_info_t *li)
{
	struct bmac_led *led;
#if OSL_SYSUPTIME_SUPPORT
	uint32 now = OSL_SYSUPTIME();
	/* Timer event can come early, and the LED on/off state change will be missed until the
	 * next li->led_blink_time cycle. Thus, the LED on/off state could be extended. To adjust
	 * for this situation, LED time may need to restart at the end of the current
	 * li->led_blink_time cycle
	 */
	wlc_hw_info_t *wlc_hw = (wlc_hw_info_t *)li->wlc_hw;
	uint time_togo;
	uint restart_time = 0;
	uint time_passed;

	/* blink each pin at its respective blinkrate */
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (led->msec_on || led->msec_off) {
			bool change_state = FALSE;
			uint factor;

			time_passed = now - led->timestamp;

			/* Currently off */
			if ((led->next_state) || (led->restart)) {
				if (time_passed > led->msec_off)
					change_state = TRUE;
				else {
					time_togo = led->msec_off - time_passed;
					factor = (led->msec_off > 1000) ? 20 : 10;
					if (time_togo < li->led_blink_time) {
						if (time_togo < led->msec_off/factor ||
							time_togo < LED_BLINK_TIME) {
							if (li->led_blink_time - time_togo >
								li->led_blink_time/10)
								change_state = TRUE;
						}
						else {
							if (!restart_time)
								restart_time = time_togo;
							else if (time_togo < restart_time)
								restart_time = time_togo;
						}
					}
				}

				/* Blink on */
				if (led->restart || change_state) {
					wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
					             (1<<led->pin), (1<<led->pin), led->activehi);
					led->next_state = OFF;
					led->timestamp = now;
					led->restart = FALSE;
				}
			}
			/* Currently on */
			else {
				if (time_passed > led->msec_on)
					change_state = TRUE;
				else {
							time_togo = led->msec_on - time_passed;
					if (time_togo < li->led_blink_time) {
						factor = (led->msec_on > 1000) ? 20 : 10;
						if (time_togo < led->msec_on/factor ||
							time_togo < LED_BLINK_TIME) {
							if (li->led_blink_time - time_togo >
								li->led_blink_time/10)
								change_state = TRUE;
						}
						else {
							if (!restart_time)
								restart_time = time_togo;
							else if (time_togo < restart_time)
								restart_time = time_togo;
						}
					}
				}

				/* Blink off  */
				if (change_state) {
					wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
					             (1<<led->pin), 0, led->activehi);
					led->next_state = ON;
					led->timestamp = now;
				}
			}
		}
	}

	if (restart_time) {
#ifdef BCMDBG
		WL_TRACE(("restart led blink timer in %dms\n", restart_time));
#endif
		wl_del_timer(wlc_hw->wlc->wl, li->led_blink_timer);
		wl_add_timer(wlc_hw->wlc->wl, li->led_blink_timer, restart_time, 0);
		li->blink_start = TRUE;
		li->blink_adjust = TRUE;
		}
	else if (li->blink_adjust) {
#ifdef BCMDBG
		WL_TRACE(("restore led_blink_time to %d\n", li->led_blink_time));
#endif
		wlc_bmac_led_blink_event(wlc_hw, TRUE);
		li->blink_start = TRUE;
		li->blink_adjust = FALSE;
	}
#else
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (led->blinkmsec) {
			if (led->blinkmsec > (int32) led->msec_on) {
				wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
				             (1<<led->pin), 0, led->activehi);
			} else {
				wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
				             (1<<led->pin), (1<<led->pin), led->activehi);
			}
			led->blinkmsec -= LED_BLINK_TIME;
			if (led->blinkmsec <= 0)
				led->blinkmsec = led->msec_on + led->msec_off;
		}
	}
#endif /* (OSL_SYSUPTIME_SUPPORT) */
}

static void
wlc_bmac_timer_led_blink(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
#ifdef WLC_HIGH
		wl_down(wlc->wl);
#endif
		return;
	}

	wlc_bmac_led_blink_timer(wlc_hw->ledh);
}


bmac_led_info_t *
BCMATTACHFN(wlc_bmac_led_attach)(wlc_hw_info_t *wlc_hw)
{
	bmac_led_info_t *bmac_li;
	bmac_led_t *led;
	int i;
	char name[32];
	char *var;
	uint val;

	if ((bmac_li = (bmac_led_info_t *)MALLOC
			(wlc_hw->osh, sizeof(bmac_led_info_t))) == NULL) {
		printf("wlc_bmac_led_attach: out of memory, malloced %d bytes",
			MALLOCED(wlc_hw->osh));
		goto fail;
	}
	bzero((char *)bmac_li, sizeof(bmac_led_info_t));

	led = &bmac_li->led[0];
	for (i = 0; i < WL_LED_NUMGPIO; i ++) {
		led->pin = i;
		led->activehi = TRUE;
#if OSL_SYSUPTIME_SUPPORT
		/* current time, in ms, for computing LED blink duration */
		led->timestamp = OSL_SYSUPTIME();
		led->next_state = ON; /* default to turning on */
#endif
		led ++;
	}

	/* look for led gpio/behavior nvram overrides */
	for (i = 0; i < WL_LED_NUMGPIO; i++) {
		led = &bmac_li->led[i];

		snprintf(name, sizeof(name), "ledbh%d", i);

		if ((var = getvar(wlc_hw->vars, name)) == NULL) {
			snprintf(name, sizeof(name), "wl0gpio%d", i);
			if ((var = getvar(wlc_hw->vars, name)) == NULL) {
				continue;
			}
		}

		val = bcm_strtoul(var, NULL, 0);

		/* silently ignore old card srom garbage */
		if ((val & WL_LED_BEH_MASK) >= WL_LED_NUMBEHAVIOR)
			continue;

		led->pin = i;	/* gpio pin# == led index# */
		led->activehi = (val & WL_LED_AL_MASK)? FALSE : TRUE;
	}

	bmac_li->wlc_hw = wlc_hw;
	if (!(bmac_li->led_blink_timer = wl_init_timer
			(wlc_hw->wlc->wl, wlc_bmac_timer_led_blink, wlc_hw->wlc,
	                                          "led_blink"))) {
		printf("wl%d: wlc_led_attach: wl_init_timer for led_blink_timer failed\n",
			wlc_hw->unit);
		goto fail;
	}

#if !OSL_SYSUPTIME_SUPPORT
	bmac_li->led_blink_time = LED_BLINK_TIME;
#endif

	return bmac_li;

fail:
	if (bmac_li) {
		MFREE(wlc_hw->osh, bmac_li, sizeof(bmac_led_info_t));
	}
	return NULL;

}

int
BCMATTACHFN(wlc_bmac_led_detach)(wlc_hw_info_t *wlc_hw)
{
	bmac_led_info_t *li = wlc_hw->ledh;
	int callbacks = 0;

	if (li) {
		if (li->led_blink_timer) {
			if (!wl_del_timer(wlc_hw->wlc->wl, li->led_blink_timer))
				callbacks++;
			wl_free_timer(wlc_hw->wlc->wl, li->led_blink_timer);
			li->led_blink_timer = NULL;
		}

		MFREE(wlc_hw->osh, li, sizeof(bmac_led_info_t));
	}

	return callbacks;
}

static void
wlc_bmac_led_blink_off(bmac_led_info_t *li)
{
	struct bmac_led *led;

	/* blink each pin at its respective blinkrate */
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (led->msec_on || led->msec_off) {
			wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
				(1<<led->pin), 0, led->activehi);
#if OSL_SYSUPTIME_SUPPORT
			led->restart = TRUE;
#endif
		}
	}
}

int
wlc_bmac_led_blink_event(wlc_hw_info_t *wlc_hw, bool blink)
{
	bmac_led_info_t *li = (bmac_led_info_t *)(wlc_hw->ledh);

	if (blink) {
		wl_del_timer(wlc_hw->wlc->wl, li->led_blink_timer);
		wl_add_timer(wlc_hw->wlc->wl, li->led_blink_timer, li->led_blink_time, 1);
		li->blink_start = TRUE;
	} else {
		if (!wl_del_timer(wlc_hw->wlc->wl, li->led_blink_timer))
			return 1;
		li->blink_start = FALSE;
		wlc_bmac_led_blink_off(li);
	}
	return 0;
}

void
wlc_bmac_led_set(wlc_hw_info_t *wlc_hw, int indx, uint8 activehi)
{
	bmac_led_t *led = &wlc_hw->ledh->led[indx];

	led->activehi = activehi;

	return;
}

void
wlc_bmac_led_blink(wlc_hw_info_t *wlc_hw, int indx, uint16 msec_on, uint16 msec_off)
{
	bmac_led_t *led = &wlc_hw->ledh->led[indx];
#if OSL_SYSUPTIME_SUPPORT
	bmac_led_info_t *li = (bmac_led_info_t *)(wlc_hw->ledh);
	uint num_leds_set = 0;
	uint led_blink_rates[WL_LED_NUMGPIO];
	uint tmp, a, b, i;
	led_blink_rates[0] = 1000; /* 1 sec, default timer */
#endif

	led->msec_on = msec_on;
	led->msec_off = msec_off;

#if !OSL_SYSUPTIME_SUPPORT
	led->blinkmsec = msec_on + msec_off;
#else
	if ((led->msec_on != msec_on) || (led->msec_off != msec_off)) {
		led->restart = TRUE;
	}

	/* recompute to an optimized blink rate timer interval */
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (!(led->msec_on || led->msec_off)) {
			led->restart = TRUE;
			continue;
		}

		/* compute the GCF of this particular LED's on+off rates */
		b = led->msec_off;
		a = led->msec_on;
		while (b != 0) {
			tmp = b;
			b = a % b;
			a = tmp;
		}

		led_blink_rates[num_leds_set++] = a;
	}

	/* compute the GCF across all LEDs, if more than one */
	a = led_blink_rates[0];

	for (i = 1; i < num_leds_set; i++) {
		b = led_blink_rates[i];
		while (b != 0) {
			tmp = b;
			b = a % b;
			a = tmp; /* A is the running GCF */
		}
	}

	li->led_blink_time = MAX(a, LED_BLINK_TIME);

	if (num_leds_set) {
		if ((li->blink_start) && !li->blink_adjust) {
			wlc_bmac_led_blink_event(wlc_hw, FALSE);
			wlc_bmac_led_blink_event(wlc_hw, TRUE);
		}
	}

#endif /* !(OSL_SYSUPTIME_SUPPORT) */
	return;
}

void
wlc_bmac_blink_sync(wlc_hw_info_t *wlc_hw, uint32 led_pins)
{
#if OSL_SYSUPTIME_SUPPORT
	bmac_led_info_t *li = wlc_hw->ledh;
	int i;

	for (i = 0; i < WL_LED_NUMGPIO; i++) {
		if (led_pins & (0x1 << i)) {
			li->led[i].restart = TRUE;
		}
	}
#endif

	return;
}

/* turn gpio bits on or off */
void
wlc_bmac_led(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val, bool activehi)
{
	bmac_led_info_t *li = wlc_hw->ledh;
	bool off = (val != mask);

	ASSERT((val & ~mask) == 0);

	if (!wlc_hw->sbclk)
		return;

	if (!activehi)
		val = ((~val) & mask);

	/* Tri-state the GPIO if the board flag is set */
	if (wlc_hw->boardflags2 & BFL2_TRISTATE_LED) {
		if ((!activehi && ((val & mask) == (li->gpioout_cache & mask))) ||
		    (activehi && ((val & mask) != (li->gpioout_cache & mask))))
			li->gpioout_cache = si_gpioouten(wlc_hw->sih, mask, off ? 0 : mask,
			                                 GPIO_DRV_PRIORITY);
	}
	else
		/* prevent the unnecessary writes to the gpio */
		if ((val & mask) != (li->gpioout_cache & mask))
			/* Traditional GPIO behavior */
			li->gpioout_cache = si_gpioout(wlc_hw->sih, mask, val,
			                               GPIO_DRV_PRIORITY);
}
#endif /* WLLED */

int
wlc_bmac_iovars_dispatch(wlc_hw_info_t *wlc_hw, uint32 actionid, uint16 type,
	void *params, uint p_len, void *arg, int len, int val_size)
{
	int err = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	bool bool_val2;

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;
	bool_val2 = (int_val2 != 0) ? TRUE : FALSE;

	WL_TRACE(("%s(): actionid=%d, p_len=%d, len=%d\n", __FUNCTION__, actionid, p_len, len));

	switch (actionid) {
#ifdef WLDIAG
	case IOV_GVAL(IOV_BMAC_DIAG): {
		uint32 result;
		uint32 diagtype;

		/* recover diagtype to run */
		bcopy((char *)params, (char *)(&diagtype), sizeof(diagtype));
		err = wlc_diag(wlc_hw->wlc, diagtype, &result);
		bcopy((char *)(&result), arg, sizeof(diagtype)); /* copy result to be buffer */
		break;
	}
#endif /* WLDIAG */

#ifdef WLLED
	case IOV_GVAL(IOV_BMAC_SBGPIOTIMERVAL):
		*ret_int_ptr = si_gpiotimerval(wlc_hw->sih, 0, 0);
		break;
	case IOV_SVAL(IOV_BMAC_SBGPIOTIMERVAL):
		si_gpiotimerval(wlc_hw->sih, ~0, int_val);
		break;
#endif /* WLLED */

#if defined(WLTEST)
	case IOV_SVAL(IOV_BMAC_SBGPIOOUT): {
		uint32 mask; /* GPIO pin mask */
		uint32 val;  /* GPIO value to program */
		mask = ((uint32*)params)[0];
		val = ((uint32*)params)[1];

		/* WARNING: This is unconditionally assigning the GPIOs to Chipcommon */
		/* Make it override all other priorities */
		si_gpiocontrol(wlc_hw->sih, mask, 0, GPIO_HI_PRIORITY);
		si_gpioouten(wlc_hw->sih, mask, mask, GPIO_HI_PRIORITY);
		si_gpioout(wlc_hw->sih, mask, val, GPIO_HI_PRIORITY);
		break;
	}

	case IOV_GVAL(IOV_BMAC_SBGPIOOUT): {
		uint32 gpio_cntrl;
		uint32 gpio_out;
		uint32 gpio_outen;

		if (len < (int) (sizeof(uint32) * 3))
			return BCME_BUFTOOSHORT;

		gpio_cntrl = si_gpiocontrol(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		gpio_out = si_gpioout(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		gpio_outen = si_gpioouten(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);

		((uint32*)arg)[0] = gpio_cntrl;
		((uint32*)arg)[1] = gpio_out;
		((uint32*)arg)[2] = gpio_outen;
		break;
	}

	case IOV_SVAL(IOV_BMAC_CCGPIOCTRL):
		si_gpiocontrol(wlc_hw->sih, ~0, int_val, GPIO_HI_PRIORITY);
		break;
	case IOV_GVAL(IOV_BMAC_CCGPIOCTRL):
		*ret_int_ptr = si_gpiocontrol(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		break;

	case IOV_SVAL(IOV_BMAC_CCGPIOOUT):
		si_gpioout(wlc_hw->sih, ~0, int_val, GPIO_HI_PRIORITY);
		break;
	case IOV_GVAL(IOV_BMAC_CCGPIOOUT):
		*ret_int_ptr = si_gpioout(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		break;
	case IOV_SVAL(IOV_BMAC_CCGPIOOUTEN):
		si_gpioouten(wlc_hw->sih, ~0, int_val, GPIO_HI_PRIORITY);
		break;
	case IOV_GVAL(IOV_BMAC_CCGPIOOUTEN):
		*ret_int_ptr = si_gpioouten(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		break;

#ifdef WLTEST
	case IOV_SVAL(IOV_BMAC_BOARDFLAGS): {
		if ((wlc_hw->sih->chip == BCM4322_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43231_CHIP_ID)) {
			wlc_hw->boardflags = int_val;

			/* wlc_hw->sih->boardflags is not updated because it's in
			 * read-only region and not used by 4323/43231
			 */
			/* wlc_hw->sih->boardflags = int_val; */

			/* some branded-boards boardflags srom programmed incorrectly */
			if ((wlc_hw->sih->boardvendor == VENDOR_APPLE) &&
			    (wlc_hw->sih->boardtype == 0x4e) && (wlc_hw->boardrev >=
			                                         0x41))
				wlc_hw->boardflags |= BFL_PACTRL;

			if (D11REV_LE(wlc_hw->corerev, 4) || (wlc_hw->boardflags &
			                                      BFL_NOPLLDOWN))
				wlc_bmac_pllreq(wlc_hw, TRUE, WLC_PLLREQ_SHARED);
		}
		break;
	}

	case IOV_SVAL(IOV_BMAC_BOARDFLAGS2): {
		if ((wlc_hw->sih->chip == BCM4322_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43231_CHIP_ID)) {
			int orig_band = wlc_hw->band->bandunit;

			wlc_hw->boardflags2 = int_val;

			/* Update A-band spur WAR */
			wlc_setxband(wlc_hw, WLC_BAND_5G);
			wlc_phy_boardflag_upd(wlc_hw->band->pi);
			wlc_setxband(wlc_hw, orig_band);
		}
		break;
	}
#endif /* WLTEST */
#endif 

	case IOV_GVAL(IOV_BMAC_CCGPIOIN):
		*ret_int_ptr = si_gpioin(wlc_hw->sih);
		break;

	case IOV_GVAL(IOV_BMAC_WPSGPIO): {
		char *var;

		if ((var = getvar(wlc_hw->vars, "wpsgpio")))
			*ret_int_ptr = (uint32)bcm_strtoul(var, NULL, 0);
		else {
			*ret_int_ptr = -1;
			err = BCME_NOTFOUND;
		}

		break;
	}

	case IOV_GVAL(IOV_BMAC_WPSLED): {
		char *var;

		if ((var = getvar(wlc_hw->vars, "wpsled")))
			*ret_int_ptr = (uint32)bcm_strtoul(var, NULL, 0);
		else {
			*ret_int_ptr = -1;
			err = BCME_NOTFOUND;
		}
		break;
	}

	case IOV_GVAL(IOV_BMAC_BTCLOCK_TUNE_WAR):
		*ret_int_ptr = wlc_hw->btclock_tune_war;
		break;

	case IOV_SVAL(IOV_BMAC_BTCLOCK_TUNE_WAR):
		wlc_hw->btclock_tune_war = bool_val;
		break;


#if (defined(BCMNVRAMR) || defined(BCMNVRAMW)) && defined(WLTEST)
	case IOV_GVAL(IOV_BMAC_OTPDUMP): {
		void *oh;
		uint32 macintmask;
		bool wasup;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
			si_otp_power(wlc_hw->sih, TRUE);

		if ((oh = otp_init(wlc_hw->sih)) == NULL) {
			err = BCME_NOTFOUND;
		} else if (otp_dump(oh, int_val, (char *)arg, len) <= 0) {
			err = BCME_BUFTOOSHORT;
		}

		if (!wasup)
			si_otp_power(wlc_hw->sih, FALSE);

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}

	case IOV_GVAL(IOV_BMAC_OTPSTAT): {
		void *oh;
		uint32 macintmask;
		bool wasup;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
			si_otp_power(wlc_hw->sih, TRUE);

		if ((oh = otp_init(wlc_hw->sih)) == NULL) {
			err = BCME_NOTFOUND;
		} else if (otp_dumpstats(oh, int_val, (char *)arg, len) <= 0) {
			err = BCME_BUFTOOSHORT;
		}

		if (!wasup)
			si_otp_power(wlc_hw->sih, FALSE);

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}

#endif 

	case IOV_GVAL(IOV_BMAC_PCIEADVCORRMASK):
			if ((BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS) ||
			    (wlc_hw->sih->buscoretype != PCIE_CORE_ID)) {
			err = BCME_UNSUPPORTED;
			break;
		}

#ifdef WLC_HIGH
		*ret_int_ptr = si_pciereg(wlc_hw->sih, PCIE_ADV_CORR_ERR_MASK,
			0, 0, PCIE_CONFIGREGS);
#endif
		break;


	case IOV_SVAL(IOV_BMAC_PCIEADVCORRMASK):
	        if ((BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS) ||
	            (wlc_hw->sih->buscoretype != PCIE_CORE_ID)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		/* Set all errors if -1 or else mask off undefined bits */
		if (int_val == -1)
			int_val = ALL_CORR_ERRORS;

		int_val &= ALL_CORR_ERRORS;
#ifdef WLC_HIGH
		si_pciereg(wlc_hw->sih, PCIE_ADV_CORR_ERR_MASK, 1, int_val,
			PCIE_CONFIGREGS);
#endif
		break;

	case IOV_GVAL(IOV_BMAC_PCIEASPM): {
		/* this command is to hide the details, but match the lcreg
		   #define PCIE_CLKREQ_ENAB		0x100
		   #define PCIE_ASPM_L1_ENAB		2
		   #define PCIE_ASPM_L0s_ENAB		1
		*/
		uint8 clkreq = 0;
		uint32 aspm = 0;
#ifdef WLC_HIGH
		clkreq = si_pcieclkreq(wlc_hw->sih, 0, 0);
		aspm = si_pcielcreg(wlc_hw->sih, 0, 0);
#endif
		*ret_int_ptr = ((clkreq & 0x1) << 8) | (aspm & 0x3);
		break;
	}

	case IOV_SVAL(IOV_BMAC_PCIEASPM): {
#ifdef WLC_HIGH
		si_pcielcreg(wlc_hw->sih, 3, (uint)(int_val & 0x3));
		si_pcieclkreq(wlc_hw->sih, 1, ((int_val & 0x100) >> 8));
#endif
		break;
	}
#ifdef BCMDBG
	case IOV_GVAL(IOV_BMAC_PCIECLKREQ):
		*ret_int_ptr = si_pcieclkreq(wlc_hw->sih, 0, 0);
		break;

	case IOV_SVAL(IOV_BMAC_PCIECLKREQ):
		if (int_val < AUTO || int_val > ON) {
			err = BCME_RANGE;
			break;
		}

		/* For AUTO, disable clkreq and then rest of the
		 * state machine will take care of it
		 */
		if (int_val == AUTO)
			si_pcieclkreq(wlc_hw->sih, 1, 0);
		else
			si_pcieclkreq(wlc_hw->sih, 1, (uint)int_val);
		break;

	case IOV_GVAL(IOV_BMAC_PCIELCREG):
		*ret_int_ptr = si_pcielcreg(wlc_hw->sih, 0, 0);
		break;

	case IOV_SVAL(IOV_BMAC_PCIELCREG):
		si_pcielcreg(wlc_hw->sih, 3, (uint)int_val);
		break;

#ifdef STA
	case IOV_SVAL(IOV_BMAC_DMALPBK):
		if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS &&
		    !PIO_ENAB(wlc_hw->wlc->pub)) {
			if (wlc_hw->dma_lpbk == bool_val)
				break;
			wlc_bmac_dma_lpbk(wlc_hw, bool_val);
			wlc_hw->dma_lpbk = bool_val;
		} else
			err = BCME_UNSUPPORTED;

		break;
#endif /* STA */
#endif /* BCMDBG */

	case IOV_SVAL(IOV_BMAC_PCIEREG):
		if (p_len < (int)sizeof(int_val) * 2) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val < 0) {
			err = BCME_BADARG;
			break;
		}
		si_pciereg(wlc_hw->sih, int_val, 1, int_val2, PCIE_PCIEREGS);
		break;

	case IOV_GVAL(IOV_BMAC_PCIEREG):
		if (p_len < (int)sizeof(int_val)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val < 0) {
			err = BCME_BADARG;
			break;
		}
		*ret_int_ptr = si_pciereg(wlc_hw->sih, int_val, 0, 0, PCIE_PCIEREGS);
		break;


#ifdef ROUTER_COMA
	case IOV_SVAL(IOV_BMAC_JTAGUREG): {
		si_t *sih = wlc_hw->sih;
		int savecidx;
		void *jh;

		if (sih->ccrev < 31) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (p_len < (int)sizeof(int_val) * 2) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		/* hnd_jtagm_init does a setcore to chipc */
		savecidx = si_coreidx(sih);
		wlc_intrsoff(wlc_hw->wlc);
		if ((jh = hnd_jtagm_init(sih, 0, FALSE)) != NULL) {
			jtag_scan(sih, jh, LV_IR_SIZE, LV_UREG_IR(int_val), 0,
				LV_DR_SIZE, int_val2, NULL, FALSE);
			hnd_jtagm_disable(sih, jh);
		} else
			err = BCME_ERROR;

		si_setcoreidx(sih, savecidx);
		wlc_intrson(wlc_hw->wlc);
		break;
	}

	case IOV_GVAL(IOV_BMAC_JTAGUREG): {
		si_t *sih = wlc_hw->sih;
		int savecidx;
		void *jh;

		if (p_len < (int)sizeof(int_val)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		/* hnd_jtagm_init does a setcore to chipc */
		savecidx = si_coreidx(sih);
		wlc_intrsoff(wlc_hw->wlc);
		if ((jh = hnd_jtagm_init(sih, 0, FALSE)) != NULL) {
			*ret_int_ptr = jtag_scan(sih, jh, LV_IR_SIZE, LV_UREG_ROIR(int_val),
			                         0, LV_DR_SIZE, 0, NULL, FALSE);
			hnd_jtagm_disable(sih, jh);
		} else
			err = BCME_ERROR;

		si_setcoreidx(sih, savecidx);
		wlc_intrson(wlc_hw->wlc);
		break;
	}

	case IOV_SVAL(IOV_BMAC_COMA): {
		si_t *sih = wlc_hw->sih;
		int savecidx;
		extern void si_router_coma(si_t *sihptr, int reset, int delay);

		if (((CHIPID(sih->chip)) != BCM5357_CHIP_ID) &&
			((CHIPID(sih->chip)) != BCM53572_CHIP_ID) &&
			((CHIPID(sih->chip)) != BCM4749_CHIP_ID)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (sih->ccrev < 31) {
			err = BCME_UNSUPPORTED;
			break;
		}

		savecidx = si_coreidx(sih);
		/* Should we do a wlc_out? */
		wlc_intrsoff(wlc_hw->wlc);
		/* Turn off radio, analog core, phypll, etc. */
		wlc_bmac_down_finish(wlc_hw);

		si_router_coma(sih, int_val, int_val2);

		si_setcoreidx(sih, savecidx);
		wlc_intrson(wlc_hw->wlc);
		break;
	}
#endif	/* ROUTER_COMA */

	case IOV_SVAL(IOV_BMAC_PCIESERDESREG): {
		int32 int_val3;
		if (p_len < (int)sizeof(int_val) * 3) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val < 0 || int_val2 < 0) {
			err = BCME_BADARG;
			break;
		}
		if (BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS) {
			err = BCME_UNSUPPORTED;
			break;
		}

		bcopy((void*)((uintptr)params + 2 * sizeof(int_val)), &int_val3, sizeof(int_val));
		/* write dev/offset/val to serdes */
		si_pcieserdesreg(wlc_hw->sih, int_val, int_val2, 1, int_val3);
		break;
	}

	case IOV_GVAL(IOV_BMAC_PCIESERDESREG): {
		if (p_len < (int)sizeof(int_val) * 2) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val < 0 || int_val2 < 0) {
			err = BCME_BADARG;
			break;
		}

		*ret_int_ptr = si_pcieserdesreg(wlc_hw->sih, int_val, int_val2, 0, 0);
		break;
	}


#if defined(WLTEST)
	case IOV_SVAL(IOV_BMAC_PLLRESET): {
		uint32 macintmask;
		wlc_info_t *wlc = wlc_hw->wlc;
		if (wlc_hw->up)
		{
			/* disable interrupts */
			macintmask = wl_intrsoff(wlc->wl);
			err = si_pll_reset(wlc_hw->sih);
			/* restore macintmask */
			wl_intrsrestore(wlc->wl, macintmask);
			wlc_phy_resetcntrl_regwrite(wlc_hw->band->pi);
		}
		else
		{
			err = BCME_UNSUPPORTED;
		}
		break;
	}
#ifdef BCMNVRAMW
	case IOV_SVAL(IOV_BMAC_OTPW):
	case IOV_SVAL(IOV_BMAC_NVOTPW): {
		void *oh;
		uint32 macintmask;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (actionid == IOV_SVAL(IOV_BMAC_OTPW)) {
			err = otp_write_region(wlc_hw->sih, OTP_HW_RGN,
			                       (uint16 *)params, p_len / 2);
		} else {
			bool wasup;

			if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
				si_otp_power(wlc_hw->sih, TRUE);

			oh = otp_init(wlc_hw->sih);
			if (oh != NULL)
				err = otp_nvwrite(oh, (uint16 *)params, p_len / 2);
			else
				err = BCME_NOTFOUND;

			if (!wasup)
				si_otp_power(wlc_hw->sih, FALSE);
		}

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}

	case IOV_SVAL(IOV_BMAC_CISVAR): {
		uint32 macintmask;
		bool wasup;
		int region;
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);
		if (!(wasup = si_is_otp_powered(wlc_hw->sih))) {
			si_otp_power(wlc_hw->sih, TRUE);
			if (!si_is_otp_powered(wlc_hw->sih)) {
				err = BCME_NOTFOUND;
				break;
			}
		}
#ifdef 	BCMSDIODEV_ENABLED
		region = OTP_HW_RGN;
#else
		region = OTP_SW_RGN;
#endif /* BCMSDIODEV_ENABLED */

		/* for OTP wrvar */
		err = otp_cis_append_region(wlc_hw->sih, region, (char*)params,
		                            p_len);
		if (!wasup)
			si_otp_power(wlc_hw->sih, FALSE);

		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}

	case IOV_GVAL(IOV_BMAC_OTPLOCK): {
		uint32 macintmask;
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		*ret_int_ptr = otp_lock(wlc_hw->sih);

		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}
#endif /* BCMNVRAMW */

#if defined(BCMNVRAMR) || defined(BCMNVRAMW)
	case IOV_GVAL(IOV_BMAC_OTP_RAW_READ):
	{
		uint32 macintmask;
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);
		if (si_is_otp_disabled(wlc_hw->sih)) {
			WL_INFORM(("OTP do not exist\n"));
			err = BCME_NOTFOUND;
		} else {
			bool wasup;
			uint32 i, offset, data = 0;
			uint16 tmp;
			void * oh;
			if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
				si_otp_power(wlc_hw->sih, TRUE);

			oh = otp_init(wlc_hw->sih);
			if (oh == NULL)
				err = BCME_NOTFOUND;
			else  {
				offset = (*(uint32 *)params);
				offset *= 16;
				for (i = 0; i < 16; i++) {
					tmp = otp_read_bit(oh, i + offset);
					data |= (tmp << i);
				}
				*ret_int_ptr = data;
				WL_TRACE(("OTP_RAW_READ, offset %x:%x\n", offset, data));
			}
			if (!wasup)
				si_otp_power(wlc_hw->sih, FALSE);
		}
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;
	}

	case IOV_GVAL(IOV_BMAC_CIS_SOURCE): {
		if ((*ret_int_ptr = wlc_bmac_cissource(wlc_hw)) == BCME_ERROR)
			err = BCME_ERROR;
		break;
	}
#endif	/* defined(BCMNVRAMR) || defined (BCMNVRAMW) */
	case IOV_GVAL(IOV_BMAC_DEVPATH): {
		si_devpath(wlc_hw->sih, (char *)arg, SI_DEVPATH_BUFSZ);
		break;
	}

#endif 

	case IOV_GVAL(IOV_BMAC_SROM): {
		srom_rw_t *s = (srom_rw_t *)arg;
		bool was_enabled;
		uint32 macintmask;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (si_is_sprom_available(wlc_hw->sih)) {
			if (!(was_enabled = si_is_sprom_enabled(wlc_hw->sih)))
				si_sprom_enable(wlc_hw->sih, TRUE);
			if (srom_read(wlc_hw->sih, wlc_hw->sih->bustype,
			              (void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
			              s->byteoff, s->nbytes, s->buf, FALSE))
				err = BCME_ERROR;
			if (!was_enabled)
				si_sprom_enable(wlc_hw->sih, FALSE);
#if defined(BCMNVRAMR) || defined(BCMNVRAMW)
		} else if (!si_is_otp_disabled(wlc_hw->sih)) {
#if defined(WLTEST)
			err = otp_read_region(wlc_hw->sih, OTP_HW_RGN, s->buf,
			                      &s->nbytes);
#else
			err = BCME_UNSUPPORTED;
#endif
#endif /* BCMNVRAMR || BCMNVRAMW */
		} else
			err = BCME_NOTFOUND;


		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;
	}

#if (defined(BCMDBG) || defined(WLTEST))
	case IOV_SVAL(IOV_BMAC_SROM): {
		srom_rw_t *s = (srom_rw_t *)params;
		bool was_enabled;
		uint32 macintmask;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (si_is_sprom_available(wlc_hw->sih)) {
			if (!(was_enabled = si_is_sprom_enabled(wlc_hw->sih)))
				si_sprom_enable(wlc_hw->sih, TRUE);
			if (srom_write(wlc_hw->sih, wlc_hw->sih->bustype,
			               (void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
			               s->byteoff, s->nbytes, s->buf))
				err = BCME_ERROR;
			if (!was_enabled)
				si_sprom_enable(wlc_hw->sih, FALSE);
		} else if (!si_is_otp_disabled(wlc_hw->sih)) {
			/* srwrite to SROM format OTP */
			err = srom_otp_write_region_crc(wlc_hw->sih, s->nbytes, s->buf,
			                                TRUE);
		} else
			err = BCME_NOTFOUND;

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;
	}

	case IOV_GVAL(IOV_BMAC_SRCRC): {
		srom_rw_t *s = (srom_rw_t *)params;

		*ret_int_ptr = (uint8)srom_otp_write_region_crc(wlc_hw->sih, s->nbytes,
		                                                s->buf, FALSE);
		break;
	}

	case IOV_GVAL(IOV_BMAC_NVRAM_SOURCE): {
		/* 0 for SROM; 1 for OTP */
		*ret_int_ptr = si_is_sprom_available(wlc_hw->sih) ? 0 : 1;
		break;
	}
#endif 



	case IOV_GVAL(IOV_BMAC_CUSTOMVAR1): {
		char *var;

		if ((var = getvar(wlc_hw->vars, "customvar1")))
			*ret_int_ptr = (uint32)bcm_strtoul(var, NULL, 0);
		else
			*ret_int_ptr = 0;

		break;
	}

	case IOV_GVAL(IOV_BMAC_NOISE_METRIC):
		*ret_int_ptr = (int32)wlc_hw->noise_metric;
		break;
	case IOV_SVAL(IOV_BMAC_NOISE_METRIC):

		if ((uint16)int_val > NOISE_MEASURE_KNOISE) {
			err = BCME_UNSUPPORTED;
			break;
		}

		wlc_hw->noise_metric = (uint16)int_val;

		if ((wlc_hw->noise_metric & NOISE_MEASURE_KNOISE) == NOISE_MEASURE_KNOISE)
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_KNOISE, MHF3_KNOISE, WLC_BAND_ALL);
		else
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_KNOISE, 0, WLC_BAND_ALL);

		break;

	case IOV_GVAL(IOV_BMAC_AVIODCNT):
		*ret_int_ptr = wlc_bmac_dma_avoidance_cnt(wlc_hw);
		break;

#ifdef BCMDBG
	case IOV_SVAL(IOV_BMAC_FILT_WAR):
		wlc_phy_set_filt_war(wlc_hw->band->pi, bool_val);
		break;

	case IOV_GVAL(IOV_BMAC_FILT_WAR):
		*ret_int_ptr = wlc_phy_get_filt_war(wlc_hw->band->pi);
		break;
#endif /* BCMDBG */

	default:
		WL_ERROR(("%s(): undefined BMAC IOVAR: %d\n", __FUNCTION__, actionid));

		break;

	}

	return err;

}

int
wlc_bmac_phy_iovar_dispatch(wlc_hw_info_t *wlc_hw, uint32 actionid, uint16 type,
	void *p, uint plen, void *a, int alen, int vsize)
{
	return wlc_phy_iovar_dispatch(wlc_hw->band->pi, actionid, type, p, plen, a, alen, vsize);
}

void
wlc_bmac_dump(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b, wlc_bmac_dump_id_t dump_id)
{
	bool single_phy, a_only;
	bool ta_ok;

	ta_ok = FALSE;
	single_phy = (wlc_hw->bandstate[0]->pi == wlc_hw->bandstate[1]->pi) ||
		(wlc_hw->bandstate[1]->pi == NULL);

	a_only = (wlc_hw->bandstate[0]->pi == NULL);

	switch (dump_id) {
#if defined(BCMDBG_DUMP)
	case BMAC_DUMP_GPIO_ID:
		si_gpiodump(wlc_hw->sih, b);

		break;

	case BMAC_DUMP_SI_ID:
		si_dump(wlc_hw->sih, b);
		break;

	case BMAC_DUMP_SIREG_ID:
		si_dumpregs(wlc_hw->sih, b);

		break;

	case BMAC_DUMP_SICLK_ID:
		si_clkctl_dump(wlc_hw->sih, b);

		break;

	case BMAC_DUMP_CCREG_ID:
		si_ccreg_dump(wlc_hw->sih, b);
		ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, FALSE) || TRUE);

		break;

	case BMAC_DUMP_PCIEREG_ID:
		si_dump_pcieregs(wlc_hw->sih, b);

		break;
#endif 

#if defined(BCMDBG) || defined(BCMDBG_DUMP)

	case BMAC_DUMP_BTC_ID:
		wlc_bmac_btc_dump(wlc_hw, b);
		break;


	case BMAC_DUMP_PHY_PHYCAL_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_phycal(wlc_hw->bandstate[0]->pi, b);
		if (!single_phy || a_only)
			wlc_phydump_phycal(wlc_hw->bandstate[1]->pi, b);
		break;

	case BMAC_DUMP_PHY_ACI_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_aci(wlc_hw->bandstate[0]->pi, b);
		if (!single_phy || a_only)
			wlc_phydump_aci(wlc_hw->bandstate[1]->pi, b);
		break;

	case BMAC_DUMP_PHY_PAPD_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_papd(wlc_hw->bandstate[0]->pi, b);
		if (!single_phy || a_only)
			wlc_phydump_papd(wlc_hw->bandstate[1]->pi, b);
		break;

	case BMAC_DUMP_PHY_NOISE_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_noise(wlc_hw->bandstate[0]->pi, b);
		if (!single_phy || a_only)
			wlc_phydump_noise(wlc_hw->bandstate[1]->pi, b);
		break;

	case BMAC_DUMP_PHY_STATE_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_state(wlc_hw->bandstate[0]->pi, b);
		if (!single_phy || a_only)
			wlc_phydump_state(wlc_hw->bandstate[1]->pi, b);

		break;

	case BMAC_DUMP_PHY_MEASLO_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_measlo(wlc_hw->bandstate[0]->pi, b);
			if (!single_phy || a_only)
			wlc_phydump_measlo(wlc_hw->bandstate[1]->pi, b);
		break;

	case BMAC_DUMP_PHY_LNAGAIN_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_lnagain(wlc_hw->bandstate[0]->pi, b);
			if (!single_phy || a_only)
			wlc_phydump_lnagain(wlc_hw->bandstate[1]->pi, b);
		break;

	case BMAC_DUMP_PHY_INITGAIN_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_initgain(wlc_hw->bandstate[0]->pi, b);
		if (!single_phy || a_only)
			wlc_phydump_initgain(wlc_hw->bandstate[1]->pi, b);
		break;

	case BMAC_DUMP_PHY_HPF1TBL_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_hpf1tbl(wlc_hw->bandstate[0]->pi, b);
		if (!single_phy || a_only)
			wlc_phydump_hpf1tbl(wlc_hw->bandstate[1]->pi, b);
		break;

	case BMAC_DUMP_PHY_LPPHYTBL0_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_lpphytbl0(wlc_hw->bandstate[0]->pi, b);
		if (!single_phy || a_only)
			wlc_phydump_lpphytbl0(wlc_hw->bandstate[1]->pi, b);
		break;

	case BMAC_DUMP_PHY_CHANEST_ID:
		if (wlc_hw->bandstate[0]->pi)
			wlc_phydump_chanest(wlc_hw->bandstate[0]->pi, b);
		if (!single_phy || a_only)
			wlc_phydump_chanest(wlc_hw->bandstate[1]->pi, b);
		break;

#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */

	default:
		break;
	}

	ASSERT(wlc_bmac_taclear(wlc_hw, ta_ok) || !ta_ok);
	return;
}

#if (defined(BCMNVRAMR) || defined(BCMNVRAMW)) && defined(WLTEST)
static int
wlc_bmac_cissource(wlc_hw_info_t *wlc_hw)
{
	int ret = 0;

	switch (si_cis_source(wlc_hw->sih)) {
	case CIS_OTP:
		ret = WLC_CIS_OTP;
		break;
	case CIS_SROM:
		ret = WLC_CIS_SROM;
		break;
	case CIS_DEFAULT:
		ret = WLC_CIS_DEFAULT;
		break;
	default:
		ret = BCME_ERROR;
		break;
	}

	return ret;
}

#ifdef BCMNVRAMW
int
wlc_bmac_ciswrite(wlc_hw_info_t *wlc_hw, cis_rw_t *cis, uint16 *tbuf, int len)
{
	int err = 0;

	WL_TRACE(("%s\n", __FUNCTION__));

	if (len < (int)cis->nbytes)
		return BCME_BUFTOOSHORT;

	switch (si_cis_source(wlc_hw->sih)) {
	case CIS_OTP: {
		uint32 macintmask;
		int region;
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (wlc_hw->sih->chip == BCM4319_CHIP_ID) {
#ifdef 	BCMSDIODEV_ENABLED
			region = OTP_HW_RGN;
#else
			region = OTP_SW_RGN;
#endif /* BCMSDIODEV_ENABLED */
		} else if ((wlc_hw->sih->chip == BCM4322_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43231_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43234_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43235_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43236_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43238_CHIP_ID))
			region = OTP_SW_RGN;
		else if (wlc_hw->sih->chip == BCM43237_CHIP_ID)
			region = OTP_HW_RGN;
		else /* including 43221 */
			region = OTP_HW_RGN;

		err = otp_write_region(wlc_hw->sih, region, tbuf, cis->nbytes / 2);

		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;

	}

	case CIS_SROM: {
		bool was_enabled;

		if (!(was_enabled = si_is_sprom_enabled(wlc_hw->sih)))
			si_sprom_enable(wlc_hw->sih, TRUE);
		if (srom_write(wlc_hw->sih, wlc_hw->sih->bustype,
			(void *)(uintptr)wlc_hw->wlc->regs, wlc_hw->osh,
			cis->byteoff, cis->nbytes, tbuf))
			err = BCME_ERROR;
		if (!was_enabled)
			si_sprom_enable(wlc_hw->sih, FALSE);
		break;
	}

	case CIS_DEFAULT:
	default:
		err = BCME_NOTFOUND;
		break;
	}

	return err;
}
#endif /* def BCMNVRAMW */

int
wlc_bmac_cisdump(wlc_hw_info_t *wlc_hw, cis_rw_t *cis, uint16 *tbuf, int len)
{
	int err = 0;
	uint32 macintmask;

	WL_TRACE(("%s\n", __FUNCTION__));

	macintmask = wl_intrsoff(wlc_hw->wlc->wl);
	cis->source = WLC_CIS_OTP;
	cis->byteoff = 0;

	switch (si_cis_source(wlc_hw->sih)) {
	case CIS_SROM: {
		bool was_enabled;

		cis->source = WLC_CIS_SROM;
		cis->byteoff = 0;
		cis->nbytes = cis->nbytes ? ROUNDUP(cis->nbytes, 2) : SROM_MAX;
		if (len < (int)cis->nbytes) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (!(was_enabled = si_is_sprom_enabled(wlc_hw->sih)))
			si_sprom_enable(wlc_hw->sih, TRUE);
		if (srom_read(wlc_hw->sih, wlc_hw->sih->bustype,
			(void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
			0, cis->nbytes, tbuf, FALSE)) {
			err = BCME_ERROR;
		}
		if (!was_enabled)
			si_sprom_enable(wlc_hw->sih, FALSE);

		break;
	}

	case CIS_OTP: {
		int region;

		cis->nbytes = len;
		if (wlc_hw->sih->chip == BCM4319_CHIP_ID) {
#ifdef 	BCMSDIODEV_ENABLED
			region = OTP_HW_RGN;
#else
			region = OTP_SW_RGN;
#endif /* BCMSDIODEV_ENABLED */
		} else if ((wlc_hw->sih->chip == BCM4322_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43231_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43234_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43235_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43236_CHIP_ID) ||
			(wlc_hw->sih->chip == BCM43238_CHIP_ID))
			region = OTP_SW_RGN;
		else if (wlc_hw->sih->chip == BCM43237_CHIP_ID)
			region = OTP_HW_RGN;
		else /* including 43221 */
			region = OTP_HW_RGN;

		err = otp_read_region(wlc_hw->sih, region, tbuf, &cis->nbytes);
		cis->nbytes *= 2;

		/* Not programmed is ok */
		if (err == BCME_NOTFOUND)
			err = 0;

		break;
	}

	case CIS_DEFAULT:
	case BCME_NOTFOUND:
	default:
		err = BCME_NOTFOUND;
		cis->source = 0;
		cis->byteoff = 0;
		cis->nbytes = 0;
		break;
	}

	wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

	return err;
}
#endif 

#if (defined(WLTEST) || defined(WLPKTENG))
#define MACSTATOFF(name) ((uint)((char *)(&wlc_hw->wlc->pub->_cnt->name) - \
	(char *)(&wlc_hw->wlc->pub->_cnt->txallfrm)))
int
wlc_bmac_pkteng(wlc_hw_info_t *wlc_hw, wl_pkteng_t *pkteng, void* p)
{
	wlc_phy_t *pi = wlc_hw->band->pi;
	uint32 cmd;
	bool is_sync;
	uint16 pkteng_mode;
	uint err = BCME_OK;

	cmd = pkteng->flags & WL_PKTENG_PER_MASK;
	is_sync = (pkteng->flags & WL_PKTENG_SYNCHRONOUS) ? TRUE : FALSE;

	switch (cmd) {
	case WL_PKTENG_PER_RX_START:
	case WL_PKTENG_PER_RX_WITH_ACK_START:
	{
#ifdef WLC_HIGH
		uint32 pktengrxducast_start = 0;
#endif /* WLC_HIGH */

		/* Reset the counters */
		wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_LO, 0);
		wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_HI, 0);
		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, TRUE);
		if (is_sync) {
#ifdef WLC_HIGH
			/* get counter value before start of pkt engine */
			wlc_ctrupd(wlc_hw->wlc, OFFSETOF(macstat_t, pktengrxducast),
				MACSTATOFF(pktengrxducast));
			pktengrxducast_start = WLCNTVAL(wlc_hw->wlc->pub->_cnt->pktengrxducast);
#else
			/* BMAC_NOTE: need to split wlc_ctrupd before supporting this in bmac */
			ASSERT(0);
#endif /* WLC_HIGH */
		}

		pkteng_mode = (cmd == WL_PKTENG_PER_RX_START) ?
			M_PKTENG_MODE_RX: M_PKTENG_MODE_RX_WITH_ACK;

		wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, pkteng_mode);

		/* set RA match reg with dest addr */
		wlc_bmac_set_addrmatch(wlc_hw, RCM_MAC_OFFSET, &pkteng->dest);

#ifdef WLC_HIGH
		/* wait for counter for synchronous receive with a maximum total delay */
		if (is_sync) {
			/* loop delay in msec */
			uint32 delay_msec = 1;
			/* avoid calculation in loop */
			uint32 delay_usec = delay_msec * 1000;
			uint32 total_delay = 0;
			uint32 delta;
			do {
				OSL_DELAY(delay_usec);
				total_delay += delay_msec;
				wlc_ctrupd(wlc_hw->wlc, OFFSETOF(macstat_t, pktengrxducast),
					MACSTATOFF(pktengrxducast));
				if (WLCNTVAL(wlc_hw->wlc->pub->_cnt->pktengrxducast)
					> pktengrxducast_start) {
					delta = WLCNTVAL(wlc_hw->wlc->pub->_cnt->pktengrxducast) -
						pktengrxducast_start;
				}
				else {
					/* counter overflow */
					delta = (~pktengrxducast_start + 1) +
						WLCNTVAL(wlc_hw->wlc->pub->_cnt->pktengrxducast);
				}
			} while (delta < pkteng->nframes && total_delay < pkteng->delay);

			wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, FALSE);
			/* implicit rx stop after synchronous receive */
			wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, 0);
			wlc_bmac_set_addrmatch(wlc_hw, RCM_MAC_OFFSET, &wlc_hw->etheraddr);
		}
#endif /* WLC_HIGH */

		break;
	}

	case WL_PKTENG_PER_RX_STOP:
		WL_INFORM(("Pkteng RX Stop Called\n"));
		wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, 0);
		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, FALSE);
		/* Restore match address register */
		wlc_bmac_set_addrmatch(wlc_hw, RCM_MAC_OFFSET, &wlc_hw->etheraddr);

		break;

	case WL_PKTENG_PER_TX_START:
	case WL_PKTENG_PER_TX_WITH_ACK_START:
	{
		uint16 val = M_PKTENG_MODE_TX;

		WL_INFORM(("Pkteng TX Start Called\n"));

		if ((pkteng->delay < 15) || (pkteng->delay > 1000)) {
			err = BCME_RANGE;
			break;
		}

		ASSERT(p != NULL);

		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, TRUE);
		wlc_bmac_suspend_mac_and_wait(wlc_hw);

		if (WLCISSSLPNPHY(wlc_hw->band) || WLCISLCNPHY(wlc_hw->band)) {
			wlc_phy_set_deaf(pi, (bool)1);
			/* set nframes */
			if (pkteng->nframes) {
				wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_LO,
					(pkteng->nframes & 0xffff));
				wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_HI,
					((pkteng->nframes>>16) & 0xffff));
				val |= M_PKTENG_FRMCNT_VLD;
			}

		} else {
			/*
			 * mute the rx side for the regular TX.
			 * tx_with_ack mode makes the ucode update rxdfrmucastmbss count
			 */
			if (cmd == WL_PKTENG_PER_TX_START)
				wlc_phy_set_deaf(pi, (bool)1);
			else
				wlc_phy_clear_deaf(pi, (bool)1);

			/* set nframes */
			if (pkteng->nframes) {
				wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_LO,
					(pkteng->nframes & 0xffff));
				wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_HI,
					((pkteng->nframes>>16) & 0xffff));
				val |= M_PKTENG_FRMCNT_VLD;
			}
		}

		wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, val);

		/* we write to M_MFGTEST_IFS the IFS required in 1/8us factor */
		/* 10 : for factoring difference b/w Tx.crs and energy in air */
		/* 44 : amount of time spent after TX_RRSP to frame start */
		/* IFS */
		wlc_bmac_write_shm(wlc_hw, M_PKTENG_IFS, (pkteng->delay - 10)*8 - 44);

		wlc_bmac_enable_mac(wlc_hw);

		/* Do the low part of wlc_txfifo() */
		wlc_bmac_txfifo(wlc_hw, TX_DATA_FIFO, p, TRUE, INVALIDFID, 1);

		/* wait for counter for synchronous transmit */
		if (is_sync) {
			int i;
			do {
				OSL_DELAY(1000);
				i = wlc_bmac_read_shm(wlc_hw, M_PKTENG_CTRL);
			} while (i & M_PKTENG_MODE_TX);

			/* implicit tx stop after synchronous transmit */
			wlc_phy_clear_deaf(pi, (bool)1);
			wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, FALSE);
		}

		break;
	}

	case WL_PKTENG_PER_TX_STOP:
		ASSERT(p == NULL);

		wlc_bmac_suspend_mac_and_wait(wlc_hw);

		WL_INFORM(("Pkteng TX Stop Called\n"));
		wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, 0);
		wlc_phy_clear_deaf(pi, (bool)1);
		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, FALSE);
		wlc_bmac_enable_mac(wlc_hw);
		break;

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

#endif 

#ifdef WLC_HIGH
/* Lower down relevant GPIOs like LED/BTC when going down w/o
 * doing PCI config cycles or touching interrupts
 */
void
wlc_gpio_fast_deinit(wlc_hw_info_t *wlc_hw)
{
	if ((wlc_hw == NULL) || (wlc_hw->sih == NULL))
		return;

	/* Only chips with internal bus or PCIE cores or certain PCI cores
	 * are able to switch cores w/o disabling interrupts
	 */
	if (!((BUSTYPE(wlc_hw->sih->bustype) == SI_BUS) ||
	      ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
	       ((wlc_hw->sih->buscoretype == PCIE_CORE_ID) ||
	        (wlc_hw->sih->buscorerev >= 13)))))
		return;

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

#ifdef WLLED
	if (wlc_hw->wlc->ledh)
		wlc_led_deinit(wlc_hw->wlc->ledh);
#endif

	wlc_bmac_btc_gpio_disable(wlc_hw);

	return;
}

#endif /* WLC_HIGH */

#if defined(STA) && defined(BCMDBG)
static void
wlc_bmac_dma_lpbk(wlc_hw_info_t *wlc_hw, bool enable)
{
	if (BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS ||
	    PIO_ENAB(wlc_hw->wlc->pub))
		return;

	if (enable) {
		wlc_bmac_suspend_mac_and_wait(wlc_hw);
		dma_fifoloopbackenable(wlc_hw->di[TX_DATA_FIFO]);
	} else {
		dma_txreset(wlc_hw->di[TX_DATA_FIFO]);
		wlc_bmac_enable_mac(wlc_hw);
	}
}
#endif /* defined(STA) && defined(BCMDBG) */

bool
wlc_bmac_radio_hw(wlc_hw_info_t *wlc_hw, bool enable, bool skip_anacore)
{
	/* Do not access Phy registers if core is not up */
	if (si_iscoreup(wlc_hw->sih) == FALSE)
		return FALSE;

	if (enable) {
		if (PMUCTL_ENAB(wlc_hw->sih)) {
			AND_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, ~CCS_FORCEHWREQOFF);
			si_pmu_radio_enable(wlc_hw->sih, TRUE);
		}

		/* need to skip for 5356 in case of radio_pwrsave feature. */
		if (!skip_anacore)
			wlc_phy_anacore(wlc_hw->band->pi, ON);
		wlc_phy_switch_radio(wlc_hw->band->pi, ON);

		/* resume d11 core */
		wlc_bmac_enable_mac(wlc_hw);
	}
	else {
		/* suspend d11 core */
		wlc_bmac_suspend_mac_and_wait(wlc_hw);

		wlc_phy_switch_radio(wlc_hw->band->pi, OFF);
		/* need to skip for 5356 in case of radio_pwrsave feature. */
		if (!skip_anacore)
			wlc_phy_anacore(wlc_hw->band->pi, OFF);

		if (PMUCTL_ENAB(wlc_hw->sih)) {
			si_pmu_radio_enable(wlc_hw->sih, FALSE);
			OR_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, CCS_FORCEHWREQOFF);
		}
	}

	return TRUE;
}

void
wlc_bmac_minimal_radio_hw(wlc_hw_info_t *wlc_hw, bool enable)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	if (D11REV_GE(wlc_hw->corerev, 13) && PMUCTL_ENAB(wlc_hw->sih)) {

		if (enable == TRUE) {
			AND_REG(wlc->osh, &wlc->regs->clk_ctl_st, ~CCS_FORCEHWREQOFF);
			si_pmu_radio_enable(wlc_hw->sih, TRUE);
		} else {
			si_pmu_radio_enable(wlc_hw->sih, FALSE);
			OR_REG(wlc->osh, &wlc->regs->clk_ctl_st, CCS_FORCEHWREQOFF);
		}
	}
}

bool
wlc_bmac_si_iscoreup(wlc_hw_info_t *wlc_hw)
{
	return si_iscoreup(wlc_hw->sih);
}

#ifdef WLC_LOW_ONLY
/* Note: We need a copy of rate_info[] for BMAC over LPPHY */
/* Rate info per rate: It tells whether a rate is ofdm or not and its phy_rate value */
const uint8 rate_info[WLC_MAXRATE + 1] = {
	/*  0     1     2     3     4     5     6     7     8     9 */
/*   0 */ 0x00, 0x00, 0x0a, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  10 */ 0x00, 0x37, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0x00,
/*  20 */ 0x00, 0x00, 0x6e, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  30 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8e, 0x00, 0x00, 0x00,
/*  40 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x89, 0x00,
/*  50 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  60 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  70 */ 0x00, 0x00, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  80 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*  90 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
/* 100 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c
};

#endif /* WLC_LOW_ONLY */

uint16
wlc_bmac_rate_shm_offset(wlc_hw_info_t *wlc_hw, uint8 rate)
{
	uint16 table_ptr;
	uint8 phy_rate, indx;

	/* get the phy specific rate encoding for the PLCP SIGNAL field */
	/* XXX4321 fixup needed ? */
	if (IS_OFDM(rate))
		table_ptr = M_RT_DIRMAP_A;
	else
		table_ptr = M_RT_DIRMAP_B;

	/* for a given rate, the LS-nibble of the PLCP SIGNAL field is
	 * the index into the rate table.
	 */
	phy_rate = rate_info[rate] & RATE_MASK;
	indx = phy_rate & 0xf;

	/* Find the SHM pointer to the rate table entry by looking in the
	 * Direct-map Table
	 */
	return (2*wlc_bmac_read_shm(wlc_hw, table_ptr + (indx * 2)));
}

#ifdef WLEXTLOG
#ifdef WLC_LOW_ONLY
void
wlc_bmac_extlog_cfg_set(wlc_hw_info_t *wlc_hw, wlc_extlog_cfg_t *cfg)
{
	wlc_extlog_info_t *extlog = (wlc_extlog_info_t *)wlc_hw->extlog;

	extlog->cfg.module = cfg->module;
	extlog->cfg.level = cfg->level;
	extlog->cfg.flag = cfg->flag;

	return;
}
#endif /* WLC_LOW_ONLY */
#endif /* WLEXTLOG */

#ifdef PHYCAL_CACHING
void
wlc_bmac_set_phycal_cache_flag(wlc_hw_info_t *wlc_hw, bool state)
{
	wlc_phy_cal_cache_set(wlc_hw->band->pi, state);
}

bool
wlc_bmac_get_phycal_cache_flag(wlc_hw_info_t *wlc_hw)
{
	return wlc_phy_cal_cache_get(wlc_hw->band->pi);
}
#endif /* PHYCAL_CACHING */

void
wlc_bmac_set_txpwr_percent(wlc_hw_info_t *wlc_hw, uint8 val)
{
	wlc_phy_txpwr_percent_set(wlc_hw->band->pi, val);
}


static uint32
cca_read_counter(wlc_hw_info_t* wlc_hw, int lo_off, int hi_off)
{
	uint16 high, tmp_high, low;

	high = wlc_bmac_read_shm(wlc_hw, hi_off);
	low = wlc_bmac_read_shm(wlc_hw, lo_off);
	tmp_high = wlc_bmac_read_shm(wlc_hw, hi_off);
	if (high != tmp_high) {
		high = wlc_bmac_read_shm(wlc_hw, hi_off);
		low = wlc_bmac_read_shm(wlc_hw, lo_off);
	}
	return (high << 16) | low;
}

void
cca_stats_read(wlc_hw_info_t *wlc_hw, cca_ucode_counts_t *cca_counts)
{
	uint32 tsf_h;
	/* Read shmem */
	cca_counts->txdur = cca_read_counter(wlc_hw, M_CCA_TXDUR_L, M_CCA_TXDUR_H);
	cca_counts->ibss = cca_read_counter(wlc_hw, M_CCA_INBSS_L, M_CCA_INBSS_H);
	cca_counts->obss = cca_read_counter(wlc_hw, M_CCA_OBSS_L, M_CCA_OBSS_H);
	cca_counts->noctg = cca_read_counter(wlc_hw, M_CCA_NOCTG_L, M_CCA_NOCTG_H);
	cca_counts->nopkt = cca_read_counter(wlc_hw, M_CCA_NOPKT_L, M_CCA_NOPKT_H);
	cca_counts->PM = cca_read_counter(wlc_hw, M_MAC_DOZE_L, M_MAC_DOZE_H);
#ifdef ISID_STATS
	cca_counts->crsglitch = wlc_bmac_read_shm(wlc_hw, M_UCODE_MACSTAT +
		OFFSETOF(macstat_t, rxcrsglitch));
	cca_counts->badplcp = wlc_bmac_read_shm(wlc_hw, M_UCODE_MACSTAT +
		OFFSETOF(macstat_t, rxbadplcp));
#endif /* ISID_STATS */
	wlc_bmac_read_tsf(wlc_hw, &cca_counts->usecs, &tsf_h);
}

int
wlc_bmac_cca_stats_read(wlc_hw_info_t *wlc_hw, cca_ucode_counts_t *cca_counts)
{
	cca_stats_read(wlc_hw, cca_counts);
	return 0;
}

void
wlc_bmac_antsel_set(wlc_hw_info_t *wlc_hw, uint32 antsel_avail)
{
	wlc_hw->antsel_avail = antsel_avail;
}



/* BTC stuff BEGIN */
static void
BCMINITFN(wlc_bmac_btc_param_init)(wlc_hw_info_t *wlc_hw)
{
	uint16 indx;
	char   buf[15];

	/* cache the pointer to the BTCX shm block, which won't change after coreinit */
	wlc_hw->btc->bt_shm_addr = 2 * wlc_bmac_read_shm(wlc_hw, M_BTCX_BLK_PTR);

	if (0 == wlc_hw->btc->bt_shm_addr)
		return;

	/* go through all btc_params, if existed in nvram, overwrite shared memory */
	for (indx = 0; indx <= M_BTCX_MAX_INDEX; indx++)
	{
		snprintf(buf, sizeof(buf), "btc_params%d", indx);
		if (getvar(wlc_hw->vars, buf) != NULL) {
			wlc_bmac_write_shm(wlc_hw, wlc_hw->btc->bt_shm_addr + indx * 2,
				(uint16)getintvar(wlc_hw->vars, buf));
		}
	}
	/* go through btc_flags list in nvram and initialize them */
	if (getvar(wlc_hw->vars, "btc_flags") != NULL) {
		 wlc_hw->btc->flags = (uint16)getintvar(wlc_hw->vars, "btc_flags");
		 wlc_bmac_btc_btcflag2ucflag(wlc_hw);
	}
}

static void
wlc_bmac_btc_btcflag2ucflag(wlc_hw_info_t *wlc_hw)
{
	int indx;
	int btc_flags = wlc_hw->btc->flags;
	uint16 btc_mhf = (btc_flags & 1) ? MHF2_BTCPREMPT : 0;

	wlc_bmac_mhf(wlc_hw, MHF2, MHF2_BTCPREMPT, btc_mhf, WLC_BAND_2G);
	btc_mhf = 0;
	for (indx = BTC_FLAGS_MHF3_START; indx <= BTC_FLAGS_MHF3_END; indx++)
		if (btc_flags & (1 << indx))
			btc_mhf |= btc_ucode_flags[indx].mask;

	btc_mhf &= ~(MHF3_BTCX_ACTIVE_PROT | MHF3_BTCX_PS_PROTECT);
	wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_DEF_BT | MHF3_BTCX_SIM_RSP |
		MHF3_BTCX_ECI | MHF3_BTCX_SIM_TX_LP, btc_mhf, WLC_BAND_2G);

	/* Ucode needs ECI indication in all bands */
	if ((btc_mhf & ~MHF3_BTCX_ECI) == 0)
		wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_ECI, btc_mhf & MHF3_BTCX_ECI, WLC_BAND_AUTO);
	btc_mhf = 0;
	for (indx = BTC_FLAGS_MHF3_END + 1; indx < BTC_FLAGS_SIZE; indx++)
		if (btc_flags & (1 << indx))
			btc_mhf |= btc_ucode_flags[indx].mask;

	wlc_bmac_mhf(wlc_hw, MHF5, MHF5_BTCX_LIGHT | MHF5_BTCX_PARALLEL,
		btc_mhf, WLC_BAND_2G);
}

#ifdef STA
static void
wlc_bmac_btc_update_predictor(wlc_hw_info_t *wlc_hw)
{
	uint32 tsf;
	uint16 bt_period, bt_last_l, bt_last_h, bt_shm_addr;
	uint32 bt_last, bt_next;
	d11regs_t *regs = wlc_hw->regs;

	bt_shm_addr = wlc_hw->btc->bt_shm_addr;
	if (0 == bt_shm_addr)
		return;

	/* Make sure period is known */
	bt_period = wlc_bmac_read_shm(wlc_hw, wlc_hw->btc->bt_shm_addr + M_BTCX_PRED_PER);

	if (0 == bt_period)
		return;

	tsf = R_REG(wlc_hw->osh, &regs->tsf_timerlow);

	/* Avoid partial read */
	do {
		bt_last_l = wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_LAST_SCO);
		bt_last_h = wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_LAST_SCO_H);
	} while (bt_last_l != wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_LAST_SCO));
	bt_last = ((uint32)bt_last_h << 16) | bt_last_l;

	/* Calculate next expected BT slot time */
	bt_next = bt_last + ((((tsf - bt_last) / bt_period) + 1) * bt_period);
	wlc_bmac_write_shm(wlc_hw, bt_shm_addr + M_BTCX_NEXT_SCO, (uint16)(bt_next & 0xffff));
}
#endif /* STA */

/*
 * Bluetooth/WLAN coexistence parameters are exposed for some customers.
 * Rather than exposing all of shared memory, an index that is range-checked
 * is translated to an address.
 */
static bool
wlc_bmac_btc_param_to_shmem(wlc_hw_info_t *wlc_hw, uint32 *pval)
{
	if (*pval > M_BTCX_MAX_INDEX)
		return FALSE;

	if (0 == wlc_hw->btc->bt_shm_addr)
		return FALSE;

	*pval = wlc_hw->btc->bt_shm_addr + (2 * (*pval));
	return TRUE;
}

static bool
wlc_bmac_btc_flags_ucode(uint8 val, uint8 *idx, uint16 *mask)
{
	/* Check that the index is valid */
	if (val >= ARRAYSIZE(btc_ucode_flags))
		return FALSE;

	*idx = btc_ucode_flags[val].idx;
	*mask = btc_ucode_flags[val].mask;

	return TRUE;
}

int
wlc_bmac_btc_period_get(wlc_hw_info_t *wlc_hw, uint16 *btperiod, bool *btactive)
{
	uint16 bt_period, bt_shm_addr, bt_per_count;
	uint32 tmp;
	d11regs_t *regs = wlc_hw->regs;
	bt_shm_addr = wlc_hw->btc->bt_shm_addr;

#define BTCX_PER_THRESHOLD 4
#define BTCX_BT_ACTIVE_THRESHOLD 5

	if (0 == bt_shm_addr)
		tmp = 0;

	else if (0 == (bt_period = wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_PRED_PER)))

		tmp = 0;

	else {
		if ((bt_per_count = wlc_bmac_read_shm(wlc_hw,
			bt_shm_addr + M_BTCX_PRED_PER_COUNT)) <= BTCX_PER_THRESHOLD)
			tmp = 0;
		else
			tmp = bt_period;
	}
	*btperiod = wlc_hw->btc->bt_period = (uint16)tmp;

	if (R_REG(wlc_hw->osh, &regs->maccontrol) & MCTL_PSM_RUN) {
		tmp = R_REG(wlc_hw->osh, &regs->btcx_cur_rfact_timer);
		/* code below can be optimized for speed; however, we choose not
		 * to do that to achieve better readability 
		 */
		if (wlc_hw->btc->bt_active) {
			/* active state : switch to inactive when reading 0xffff */
			if (tmp == 0xffff) {
				wlc_hw->btc->bt_active = FALSE;
				wlc_hw->btc->bt_active_asserted_cnt = 0;
			}
		} else {
			/* inactive state : switch to active when bt_active asserted for
			 * more than a certain times 
			 */
			if (tmp == 0xffff)
				wlc_hw->btc->bt_active_asserted_cnt = 0;
			/* concecutive asserts, now declare bt is active */
			else if (++wlc_hw->btc->bt_active_asserted_cnt >= BTCX_BT_ACTIVE_THRESHOLD)
				wlc_hw->btc->bt_active = TRUE;
		}
	}

	*btactive = wlc_hw->btc->bt_active;

	return BCME_OK;
}

int
wlc_bmac_btc_mode_set(wlc_hw_info_t *wlc_hw, int btc_mode)
{
	uint16 btc_mhfs[MHFMAX];
	bool ucode_up = FALSE;

	if (btc_mode > WL_BTC_DEFAULT)
		return BCME_BADARG;

	/* Make sure 2-wire or 3-wire decision has been made */
	ASSERT((wlc_hw->btc->wire >= WL_BTC_2WIRE) || (wlc_hw->btc->wire <= WL_BTC_4WIRE));

	 /* Determine the default mode for the device */
	if (btc_mode == WL_BTC_DEFAULT) {
		if (BCMECISECICOEX_ENAB_BMAC(wlc_hw) || (wlc_hw->boardflags2 & BFL2_BTC3WIRE)) {
			btc_mode = WL_BTC_FULLTDM;
			/* default to hybrid mode for combo boards with 2 or more antennas */
			if (!BCMECISECICOEX_ENAB_BMAC(wlc_hw) && wlc_hw->btc->btcx_aa > 2)
				btc_mode = WL_BTC_HYBRID;
		}
		else
			btc_mode = WL_BTC_DISABLE;
	}

	/* Do not allow an enable without hw support */
	if (btc_mode != WL_BTC_DISABLE) {
		if ((wlc_hw->btc->wire >= WL_BTC_3WIRE) && D11REV_GE(wlc_hw->corerev, 13) &&
			!(wlc_hw->machwcap_backup & MCAP_BTCX))
			return BCME_BADOPTION;
	}

	/* Initialize ucode flags */
	bzero(btc_mhfs, sizeof(btc_mhfs));
	wlc_hw->btc->flags = 0;

	if (wlc_hw->up)
		ucode_up = (R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC);

	if (btc_mode != WL_BTC_DISABLE) {
		btc_mhfs[MHF1] |= MHF1_BTCOEXIST;
		if (wlc_hw->btc->wire == WL_BTC_2WIRE) {
			/* BMAC_NOTES: sync the state with HIGH driver ??? */
			/* Make sure 3-wire coex is off */
			wlc_hw->machwcap &= ~MCAP_BTCX;
			if (wlc_hw->boardflags & BFL_BTC2WIRE_ALTGPIO) {
				btc_mhfs[MHF2] |= MHF2_BTC2WIRE_ALTGPIO;
				wlc_hw->btc->gpio_mask =
					BOARD_GPIO_BTCMOD_OUT | BOARD_GPIO_BTCMOD_IN;
				wlc_hw->btc->gpio_out = BOARD_GPIO_BTCMOD_OUT;
			} else {
				btc_mhfs[MHF2] &= ~MHF2_BTC2WIRE_ALTGPIO;
			}
		} else {
			if (D11REV_GE(wlc_hw->corerev, 13)) {
				wlc_hw->machwcap |= MCAP_BTCX;
				/* by default we use PS protection unless overriden. */
				if (btc_mode == WL_BTC_HYBRID)
					wlc_hw->btc->flags |= WL_BTC_FLAG_SIM_RSP;
				else if (btc_mode == WL_BTC_LITE) {
					wlc_hw->btc->flags |= WL_BTC_FLAG_LIGHT;
					wlc_hw->btc->flags |= WL_BTC_FLAG_SIM_RSP;
				} else if (btc_mode == WL_BTC_PARALLEL)
					wlc_hw->btc->flags |= WL_BTC_FLAG_PARALLEL;
				else
					wlc_hw->btc->flags |=
						(WL_BTC_FLAG_PS_PROTECT | WL_BTC_FLAG_ACTIVE_PROT);

				if (BCMECISECICOEX_ENAB_BMAC(wlc_hw)) {
					wlc_hw->btc->flags |= WL_BTC_FLAG_ECI;
				} else {
					if (wlc_hw->btc->wire == WL_BTC_4WIRE)
						btc_mhfs[MHF3] |= MHF3_BTCX_EXTRA_PRI;
					else
						wlc_hw->btc->flags |= WL_BTC_FLAG_PREMPT;
				}
			} else { /* 3-wire over GPIO */
				wlc_hw->btc->flags |= (WL_BTC_FLAG_ACTIVE_PROT |
					WL_BTC_FLAG_SIM_RSP);
				wlc_hw->btc->gpio_mask = BOARD_GPIO_BTC3W_OUT | BOARD_GPIO_BTC3W_IN;
				wlc_hw->btc->gpio_out = BOARD_GPIO_BTC3W_OUT;
			}
		}

	} else {
		btc_mhfs[MHF1] &= ~MHF1_BTCOEXIST;
		if (D11REV_GE(wlc_hw->corerev, 13))
			wlc_hw->machwcap &= ~MCAP_BTCX;
	}

	wlc_hw->btc->mode = btc_mode;
	wlc_phy_machwcap_set(wlc_hw->band->pi, wlc_hw->machwcap);

	/* Set the MHFs only in 2G band
	 * If we are on the other band, update the sw cache for the
	 * 2G band.
	 */
	if (wlc_hw->up && ucode_up)
		wlc_bmac_suspend_mac_and_wait(wlc_hw);

	wlc_bmac_mhf(wlc_hw, MHF1, MHF1_BTCOEXIST, btc_mhfs[MHF1], WLC_BAND_2G);
	wlc_bmac_mhf(wlc_hw, MHF2, MHF2_BTC2WIRE_ALTGPIO, btc_mhfs[MHF2],
		WLC_BAND_2G);
	wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_EXTRA_PRI, btc_mhfs[MHF3], WLC_BAND_2G);
	wlc_bmac_btc_btcflag2ucflag(wlc_hw);

	if (wlc_hw->up && ucode_up) {
		wlc_bmac_enable_mac(wlc_hw);
	}

	return BCME_OK;
}

int
wlc_bmac_btc_mode_get(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->btc->mode;
}

int
wlc_bmac_btc_wire_get(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->btc->wire;
}

int
wlc_bmac_btc_wire_set(wlc_hw_info_t *wlc_hw, int btc_wire)
{
	/* Has to be down. Enforced through iovar flag */
	ASSERT(!wlc_hw->up);

	if (btc_wire > WL_BTC_4WIRE)
		return BCME_BADARG;

	/* default to 4-wire ucode if 3-wire boardflag is set or
	 * - M93 or ECI is enabled
	 * else default to 2-wire
	 */
	if (btc_wire == WL_BTC_DEFWIRE) {
		/* Use the boardflags to finally fix the setting for
		 * boards with correct flags
		 */
		if (BCMECISECICOEX_ENAB_BMAC(wlc_hw))
			wlc_hw->btc->wire = WL_BTC_3WIRE;
		else if (wlc_hw->boardflags2 & BFL2_BTC3WIRE)
			wlc_hw->btc->wire = WL_BTC_4WIRE;
		else
			wlc_hw->btc->wire = WL_BTC_2WIRE;
		/* some boards may not have the 3-wire boardflag */
		if (D11REV_IS(wlc_hw->corerev, 12) &&
		    ((wlc_hw->sih->boardvendor == VENDOR_APPLE) &&
		     ((wlc_hw->sih->boardtype == BCM943224M93) ||
		      (wlc_hw->sih->boardtype == BCM943224M93A))))
			wlc_hw->btc->wire = WL_BTC_3WIRE;
	}
	else
		wlc_hw->btc->wire = btc_wire;
	/* flush ucode_loaded so the ucode download will happen again to pickup the right ucode */
	wlc_hw->ucode_loaded = FALSE;

	wlc_bmac_btc_gpio_configure(wlc_hw);

	return BCME_OK;
}

int
wlc_bmac_btc_flags_get(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->btc->flags;
}


static void
wlc_bmac_btc_flags_upd(wlc_hw_info_t *wlc_hw, bool set_clear, uint16 val, uint8 idx, uint16 mask)
{
	if (set_clear) {
		wlc_hw->btc->flags |= val;
		wlc_bmac_mhf(wlc_hw, idx, mask, mask, WLC_BAND_2G);
	} else {
		wlc_hw->btc->flags &= ~val;
		wlc_bmac_mhf(wlc_hw, idx, mask, 0, WLC_BAND_2G);
	}
}

int
wlc_bmac_btc_flags_idx_get(wlc_hw_info_t *wlc_hw, int int_val)
{
	uint8 idx = 0;
	uint16 mask = 0;

	if (!wlc_bmac_btc_flags_ucode((uint8)int_val, &idx, &mask))
		return 0xbad;

	return (wlc_bmac_mhf_get(wlc_hw, idx, WLC_BAND_2G) & mask) ? 1 : 0;
}

int
wlc_bmac_btc_flags_idx_set(wlc_hw_info_t *wlc_hw, int int_val, int int_val2)
{
	uint8 idx = 0;
	uint16 mask = 0;

	if (!wlc_bmac_btc_flags_ucode((uint8)int_val, &idx, &mask))
		return BCME_BADARG;

	if (int_val2)
		wlc_bmac_btc_flags_upd(wlc_hw, TRUE, (uint16)(int_val2 << int_val), idx, mask);
	else
		wlc_bmac_btc_flags_upd(wlc_hw, FALSE, (uint16)(1 << int_val), idx, mask);

	return BCME_OK;
}

void
wlc_bmac_btc_stuck_war50943(wlc_hw_info_t *wlc_hw, bool enable)
{
	if (enable) {
		wlc_hw->btc->stuck_detected = FALSE;
		wlc_hw->btc->stuck_war50943 = TRUE;
		wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_DELL_WAR, MHF3_BTCX_DELL_WAR, WLC_BAND_ALL);
	} else {
		wlc_hw->btc->stuck_war50943 = FALSE;
		wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_DELL_WAR, 0, WLC_BAND_ALL);
	}
}

int
wlc_bmac_btc_params_set(wlc_hw_info_t *wlc_hw, int int_val, int int_val2)
{
	if (!wlc_bmac_btc_param_to_shmem(wlc_hw, (uint32*)&int_val))
		return BCME_BADARG;

	wlc_bmac_write_shm(wlc_hw, (uint16)int_val, (uint16)int_val2);
	return BCME_OK;
}

int
wlc_bmac_btc_params_get(wlc_hw_info_t *wlc_hw, int int_val)
{
	if (!wlc_bmac_btc_param_to_shmem(wlc_hw, (uint32*)&int_val))
		return 0xbad;

	return wlc_bmac_read_shm(wlc_hw, (uint16)int_val);
}

void
wlc_bmac_btc_rssi_threshold_get(wlc_hw_info_t *wlc_hw, uint8 *prot, uint8 *ampdut, uint8 *ampdur)
{
	uint16 bt_shm_addr = wlc_hw->btc->bt_shm_addr;

	if (0 == bt_shm_addr)
		return;

	*prot =	(uint8)wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_PROT_RSSI_THRESH);
	*ampdut = (uint8)wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_AMPDUTX_RSSI_THRESH);
	*ampdur = (uint8)wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_AMPDURX_RSSI_THRESH);
}

/* configure 3/4 wire coex gpio for newer chips */
void
wlc_bmac_btc_gpio_configure(wlc_hw_info_t *wlc_hw)
{

	if (wlc_hw->btc->wire >= WL_BTC_3WIRE) {
		uint32 gm = 0;
		switch ((CHIPID(wlc_hw->sih->chip))) {
		case BCM43224_CHIP_ID:
		case BCM43421_CHIP_ID:
			if (wlc_hw->boardflags & BFL_FEM_BT)
				gm = GPIO_BTC4W_OUT_43224_SHARED;
			else
				gm = GPIO_BTC4W_OUT_43224;
			break;
		case BCM43225_CHIP_ID:
			gm = GPIO_BTC4W_OUT_43225;
			break;
		case BCM4312_CHIP_ID:
			gm = GPIO_BTC4W_OUT_4312;
			break;
		case BCM4313_CHIP_ID:
			gm = GPIO_BTC4W_OUT_4313;
			break;
		case BCM4331_CHIP_ID:
			if (wlc_hw->boardflags & BFL_FEM_BT)
				gm = GPIO_BTC4W_OUT_4331_SHARED;
			break;
		};

		wlc_hw->btc->gpio_mask = wlc_hw->btc->gpio_out = gm;
	}
}
/* Lower BTC GPIO through ChipCommon when BTC is OFF or D11 MAC is in reset or on powerup */
void
wlc_bmac_btc_gpio_disable(wlc_hw_info_t *wlc_hw)
{
	uint32 gm, go;
	si_t *sih;
	bool xtal_set = FALSE;

	if (!wlc_hw->sbclk) {
		wlc_bmac_xtal(wlc_hw, ON);
		xtal_set = TRUE;
	}

	/* Proceed only if BTC GPIOs had been configured */
	if (wlc_hw->btc->gpio_mask == 0)
		return;

	sih = wlc_hw->sih;

	gm = wlc_hw->btc->gpio_mask;
	go = wlc_hw->btc->gpio_out;

	/* Set the control of GPIO back and lower only GPIO OUT pins and not the ones that
	 * are supposed to be IN
	 */
	si_gpiocontrol(sih, gm, 0, GPIO_DRV_PRIORITY);
	/* configure gpio to input to float pad */
	si_gpioouten(sih, gm, 0, GPIO_DRV_PRIORITY);
	/* a HACK to enable internal pulldown for 4313 */
	if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID)
		si_gpiopull(wlc_hw->sih, GPIO_PULLDN, gm, 0x40);

	si_gpioout(sih, go, 0, GPIO_DRV_PRIORITY);

	if (wlc_hw->clk)
		AND_REG(wlc_hw->osh, &wlc_hw->regs->psm_gpio_oe, ~wlc_hw->btc->gpio_out);

	/* BMAC_NOTE: PCI_BUS check here is actually not relevant; there is nothing PCI
	 * bus specific here it was only meant to be compile time optimization. Now it's
	 * true that it may not anyway be applicable to 4323, but need to see if there are
	 * any more places like this
	 */
	/* On someboards, which give GPIOs to UART via strapping,
	 * GPIO_BTC_OUT is not directly controlled by gpioout on CC
	 */
	if ((BUSTYPE(sih->bustype) == PCI_BUS) && (gm & BOARD_GPIO_BTC_OUT))
		si_btcgpiowar(sih);

	if (xtal_set)
		wlc_bmac_xtal(wlc_hw, OFF);

}

/* Set BTC GPIO through ChipCommon when BTC is ON */
static void
wlc_bmac_btc_gpio_enable(wlc_hw_info_t *wlc_hw)
{
	uint32 gm, go, gi;
	si_t *sih;

	ASSERT(wlc_hw->clk);

	/* Proceed only if GPIO-based BTC is configured */
	if (wlc_hw->btc->gpio_mask == 0)
		return;


	sih = wlc_hw->sih;

	gm = wlc_hw->btc->gpio_mask;
	go = wlc_hw->btc->gpio_out;
	gi = (~wlc_hw->btc->gpio_out) & wlc_hw->btc->gpio_mask;

	OR_REG(wlc_hw->osh, &wlc_hw->regs->psm_gpio_oe, wlc_hw->btc->gpio_out);
	/* Clear OUT enable from GPIOs that the driver expects to be IN */
	si_gpioouten(sih, gi, 0, GPIO_DRV_PRIORITY);

	if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID)
		si_gpiopull(wlc_hw->sih, GPIO_PULLDN, gm, 0);
	si_gpiocontrol(sih, gm, gm, GPIO_DRV_PRIORITY);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_bmac_btc_dump(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "BTC---\n");
	bcm_bprintf(b, "btc_mode %d btc_wire %d btc_flags %d "
		"btc_gpio_mask %d btc_gpio_out %d btc_stuck_detected %d btc_stuck_war50943 %d "
		"bt_shm_add %d bt_period %d bt_active %d\n",
		wlc_hw->btc->mode, wlc_hw->btc->wire, wlc_hw->btc->flags, wlc_hw->btc->gpio_mask,
		wlc_hw->btc->gpio_out, wlc_hw->btc->stuck_detected, wlc_hw->btc->stuck_war50943,
		wlc_hw->btc->bt_shm_addr, wlc_hw->btc->bt_period, wlc_hw->btc->bt_active);
}
#endif	/* defined(BCMDBG) || defined(BCMDBG_DUMP) */

/* BTC stuff END */

#ifdef STA
/* Change PCIE War override for some platforms */
void
wlc_bmac_pcie_war_ovr_update(wlc_hw_info_t *wlc_hw, uint8 aspm)
{
	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
		(wlc_hw->sih->buscoretype == PCIE_CORE_ID))
		si_pcie_war_ovr_update(wlc_hw->sih, aspm);
}

void
wlc_bmac_pcie_power_save_enable(wlc_hw_info_t *wlc_hw, bool enable)
{
	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
		(wlc_hw->sih->buscoretype == PCIE_CORE_ID))
		si_pcie_power_save_enable(wlc_hw->sih, enable);
}
#endif /* STA */
