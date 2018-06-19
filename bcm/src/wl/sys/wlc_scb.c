/*
 * Common interface to the 802.11 Station Control Block (scb) structure
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_scb.c,v 1.227.10.17 2011-01-07 22:52:13 Exp $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <proto/wpa.h>
#include <sbconfig.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#include <wlioctl.h>
#include <epivers.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wl_export.h>
#include <wlc_ap.h>
#include <wlc_rate_sel.h>
#include <wlc_assoc.h>

#define SCB_MAX_CUBBY		16	/* max number of cubbies in container */
#define SCB_MAGIC 0x0505a5a5

#define INTERNAL_SCB		0x00000001
#define USER_SCB		0x00000002

#define	SCBHASHINDEX(hash, id)	((id[3] ^ id[4] ^ id[5]) % (hash))

/* structure for storing per-cubby client info */
typedef struct cubby_info {
	scb_cubby_init_t	fn_init;	/* fn called during scb malloc */
	scb_cubby_deinit_t	fn_deinit;	/* fn called during scb free */
	scb_cubby_dump_t 	fn_dump;	/* fn called during scb dump */
	void			*context;	/* context to be passed to all cb fns */
} cubby_info_t;

/* structure for storing public and private global scb module state */
struct scb_module {
	wlc_pub_t	*pub;				/* public part of wlc */
	struct scb	*scb;				/* station control block link list */
	uint16		nscb;				/* total number of allocated scbs */
	struct scb 	**scbhash[MAXBANDS];			/* station control block hash */
	uint		scbtotsize;			/* total scb size including container */
	uint 		ncubby;				/* current num of cubbies */
	cubby_info_t	*cubby_info;	/* cubby client info */
	uint8		nscbhash;
#ifdef SCBFREELIST
	struct scb      *free_list;			/* Free list of SCBs */
#endif
};

/* station control block - one per remote MAC address */
struct scb_info {
	struct scb 	*scbpub;		/* public portion of scb */
	struct scb_info *hashnext;	/* pointer to next scb under same hash entry */
	struct scb_info	*next;		/* pointer to next allocated scb */
	struct wlcband	*band;		/* pointer to our associated band */
#ifdef MACOSX
	struct scb_info *hashnext_copy;
	struct scb_info *next_copy;
#endif
};

/* Helper macro for txpath in scb */
/* A feature in Tx path goes through following states:
 * Unregisterd -> Registered [Global state]
 * Registerd -> Configured -> Active -> Configured [Per-scb state]
 */

/* Set the next feature of given feature */
#define SCB_TXMOD_SET(scb, fid, _next_fid) { \
	scb->tx_path[fid].next_tx_fn = wlc->txmod_fns[_next_fid].tx_fn; \
	scb->tx_path[fid].next_handle = wlc->txmod_fns[_next_fid].ctx; \
	}
static void wlc_scb_hash_add(wlc_info_t *wlc, struct scb *scb, int bandunit);
static void wlc_scb_hash_del(wlc_info_t *wlc, struct scb *scbd, int bandunit);

static struct scb *wlc_scbvictim(wlc_info_t *wlc);
static struct scb *wlc_scb_getnext(struct scb *scb);
static int wlc_scbinit(wlc_info_t *wlc, struct wlcband *band, struct scb_info *scbinfo,
	uint32 scbflags);
static void wlc_scb_reset(scb_module_t *scbstate, struct scb_info *scbinfo);
static struct scb_info *wlc_scb_allocmem(scb_module_t *scbstate);
static void wlc_scb_freemem(scb_module_t *scbstate, struct scb_info *scbinfo);

#ifdef MFP
static void scb_sa_query_timeout(void *arg);
#endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_dump_scb(wlc_info_t *wlc, struct bcmstrbuf *b);
/* Dump the active txpath for the current SCB */
static int wlc_scb_txpath_dump(wlc_info_t *wlc, struct scb *scb, struct bcmstrbuf *b);
/* SCB Flags Names Initialization */
static const bcm_bit_desc_t scb_flags[] =
{
	{SCB_NONERP, "NonERP"},
	{SCB_LONGSLOT, "LgSlot"},
	{SCB_SHORTPREAMBLE, "ShPre"},
	{SCB_8021XHDR, "1X"},
	{SCB_WPA_SUP, "WPASup"},
	{SCB_DEAUTH, "DeA"},
	{SCB_WMECAP, "WME"},
#ifdef WLAFTERBURNER
	{SCB_ABCAP, "ABCAP"},
#endif /* WLAFTERBURNER */
	{SCB_BRCM, "BRCM"},
	{SCB_WDS_LINKUP, "WDSLinkUP"},
	{SCB_LEGACY_AES, "LegacyAES"},
	{SCB_LEGACY_CRAM, "LegacyCRAM"},
	{SCB_MYAP, "MyAP"},
	{SCB_PENDING_PROBE, "PendingProbe"},
	{SCB_AMSDUCAP, "AMSDUCAP"},
	{SCB_BACAP, "BACAP"},
	{SCB_HTCAP, "HT"},
	{SCB_RECV_PM, "RECV_PM"},
	{SCB_AMPDUCAP, "AMPDUCAP"},
	{SCB_IS40, "40MHz"},
	{SCB_NONGF, "NONGFCAP"},
	{SCB_APSDCAP, "APSDCAP"},
	{SCB_PENDING_FREE, "PendingFree"},
	{SCB_PENDING_PSPOLL, "PendingPSPoll"},
	{SCB_RIFSCAP, "RIFSCAP"},
	{SCB_HT40INTOLERANT, "40INTOL"},
	{SCB_WMEPS, "WMEPSOK"},
	{SCB_COEX_MGMT, "OBSSCoex"},
	{SCB_IBSS_PEER, "IBSS Peer"},
	{SCB_STBCCAP, "STBC"},
#ifdef WLBTAMP
	{SCB_11ECAP, "11e"},
#endif
	{0, NULL}
};
static const bcm_bit_desc_t scb_flags2[] =
{
	{SCB2_SGI20_CAP, "SGI20"},
	{SCB2_SGI40_CAP, "SGI40"},
	{SCB2_LDPCCAP, "LDPC"},
	{0, NULL}
};
static const bcm_bit_desc_t scb_states[] =
{
	{AUTHENTICATED, "AUTH"},
	{ASSOCIATED, "ASSOC"},
	{PENDING_AUTH, "AUTH_PEND"},
	{PENDING_ASSOC, "ASSOC_PEND"},
	{AUTHORIZED, "AUTH_8021X"},
	{TAKEN4IBSS, "IBSS"},
	{0, NULL}
};
#endif /* BCMDBG || BCMDBG_DUMP */

#ifdef SCBFREELIST
static void wlc_scbfreelist_free(scb_module_t *scbstate);
#endif

#define SCBINFO(_scb) (_scb ? (struct scb_info *)((_scb)->scb_priv) : NULL)

#ifdef MACOSX

#ifdef panic
#undef panic
void panic(const char *str, ...);
#endif

#define SCBSANITYCHECK(_scb)  { \
		if (((_scb) != NULL) &&				\
		    ((((_scb))->magic != SCB_MAGIC) ||	\
		     (SCBINFO(_scb)->hashnext != SCBINFO(_scb)->hashnext_copy) || \
		     (SCBINFO(_scb)->next != SCBINFO(_scb)->next_copy)))	\
			panic("scbinfo corrupted: magic: 0x%x hn: %p hnc: %p n: %p nc: %p\n", \
			      ((_scb))->magic, SCBINFO(_scb)->hashnext, \
			      SCBINFO(_scb)->hashnext_copy,		\
			      SCBINFO(_scb)->next, SCBINFO(_scb)->next_copy);	\
	}

#define SCBFREESANITYCHECK(_scb)  { \
		if (((_scb) != NULL) &&				\
		    ((((_scb))->magic != ~SCB_MAGIC) || \
		     (SCBINFO(_scb)->next != SCBINFO(_scb)->next_copy)))	\
			panic("scbinfo corrupted: magic: 0x%x hn: %p hnc: %p n: %p nc: %p\n", \
			      ((_scb))->magic, SCBINFO(_scb)->hashnext, \
			      SCBINFO(_scb)->hashnext_copy,		\
			      SCBINFO(_scb)->next, SCBINFO(_scb)->next_copy);	\
	}

#else

#define SCBSANITYCHECK(_scbinfo)	do {} while (0)
#define SCBFREESANITYCHECK(_scbinfo)	do {} while (0)

#endif /* MACOSX */


scb_module_t *
BCMATTACHFN(wlc_scb_attach)(wlc_pub_t *pub, wlc_info_t *wlc)
{
	scb_module_t *scbstate;
	int len, i;
	uint8 nscbhash;

	nscbhash = ((pub->tunables->maxscb + 7)/8); /* # scb hash buckets */
	len = sizeof(scb_module_t) + (sizeof(cubby_info_t) * SCB_MAX_CUBBY)
	      + (sizeof(struct scb *) * MAXBANDS * nscbhash);
	if ((scbstate = MALLOC(pub->osh, len)) == NULL)
		return NULL;

	bzero((char *)scbstate, len);
	scbstate->pub = pub;
	scbstate->cubby_info = (cubby_info_t *)((uintptr)scbstate + sizeof(scb_module_t));

	scbstate->nscbhash = nscbhash; /* # scb hash buckets */

	for (i = 0; i < MAXBANDS; i++) {
		scbstate->scbhash[i] = (struct scb **)((uintptr)scbstate->cubby_info +
		                           (sizeof(cubby_info_t) * SCB_MAX_CUBBY) +
		                           (i * scbstate->nscbhash * sizeof(struct scb *)));
	}
	scbstate->scbtotsize = sizeof(struct scb);
	scbstate->scbtotsize += sizeof(int) * MA_WINDOW_SZ; /* sizeof rssi_window */
	scbstate->scbtotsize += sizeof(struct tx_path_node) * TXMOD_LAST;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)

	wlc_dump_register(pub, "scb", (dump_fn_t)wlc_dump_scb, (void *)wlc);
#endif

	return scbstate;
}

void
BCMATTACHFN(wlc_scb_detach)(scb_module_t *scbstate)
{
	int len;

	if (!scbstate)
		return;

#ifdef SCBFREELIST
	wlc_scbfreelist_free(scbstate);
#endif

	ASSERT(scbstate->nscb == 0);

	len = sizeof(scb_module_t) + (sizeof(cubby_info_t) * SCB_MAX_CUBBY)
	      + (sizeof(struct scb *) * MAXBANDS * scbstate->nscbhash);
	MFREE(scbstate->pub->osh, scbstate, len);
}

/* Methods for iterating along a list of scb */

/* Direct access to the next */
static struct scb *
wlc_scb_getnext(struct scb *scb)
{
	if (scb) {
		SCBSANITYCHECK(scb);
		return (SCBINFO(scb)->next ? SCBINFO(scb)->next->scbpub : NULL);
	}
	return NULL;
}

/* Initialize an iterator keeping memory of the next scb as it moves along the list */
void
wlc_scb_iterinit(scb_module_t *scbstate, struct scb_iter *scbiter)
{
	ASSERT(scbiter != NULL);
	SCBSANITYCHECK(scbstate->scb);

	/* Prefetch next scb, so caller can free an scb before going on to the next */
	scbiter->next = scbstate->scb;
}

/* move the iterator */
struct scb *
wlc_scb_iternext(struct scb_iter *scbiter)
{
	struct scb *scb;

	ASSERT(scbiter != NULL);
	if ((scb = scbiter->next)) {
		SCBSANITYCHECK(scb);
		scbiter->next = (SCBINFO(scb)->next ? SCBINFO(scb)->next->scbpub : NULL);
	} else
		scbiter->next = NULL;
	return  scb;
}


/*  
 * Accessors, nagative values are errors.
 */

int
BCMATTACHFN(wlc_scb_cubby_reserve)(wlc_info_t *wlc, uint size, scb_cubby_init_t fn_init,
	scb_cubby_deinit_t fn_deinit, scb_cubby_dump_t fn_dump, void *context)
{
	uint offset;
	scb_module_t *scbstate = wlc->scbstate;
	cubby_info_t *cubby_info;

	ASSERT(scbstate->nscb == 0);
	ASSERT((scbstate->scbtotsize % PTRSZ) == 0);
	ASSERT(scbstate->ncubby < SCB_MAX_CUBBY);

	if (scbstate->ncubby >= SCB_MAX_CUBBY) {
		ASSERT(0);
		return -1;
	}

	/* housekeeping info is stored in scb_module struct */
	cubby_info = &scbstate->cubby_info[scbstate->ncubby++];
	cubby_info->fn_init = fn_init;
	cubby_info->fn_deinit = fn_deinit;
	cubby_info->fn_dump = fn_dump;
	cubby_info->context = context;

	/* actual cubby data is stored at the end of scb's */
	offset = scbstate->scbtotsize;

	/* roundup to pointer boundary */
	scbstate->scbtotsize = ROUNDUP(scbstate->scbtotsize + size, PTRSZ);

	return offset;
}

struct wlcband *
wlc_scbband(struct scb *scb)
{
	return SCBINFO(scb)->band;
}


#ifdef SCBFREELIST
static
struct scb_info *wlc_scbget_free(scb_module_t *scbstate)
{
	struct scb_info *ret = NULL;
	if (scbstate->free_list == NULL)
		return NULL;
	ret = SCBINFO(scbstate->free_list);
	SCBFREESANITYCHECK(ret->scbpub);
	scbstate->free_list = (ret->next ? ret->next->scbpub : NULL);
#ifdef MACOSX
	ret->next_copy = NULL;
#endif

	ret->next = NULL;
	return ret;
}

static
void wlc_scbadd_free(scb_module_t *scbstate, struct scb_info *ret)
{
	SCBFREESANITYCHECK(scbstate->free_list);
	ret->next = SCBINFO(scbstate->free_list);
	scbstate->free_list = ret->scbpub;
#ifdef MACOSX
	ret->scbpub->magic = ~SCB_MAGIC;
	ret->next_copy = ret->next;
#endif
}

static
void wlc_scbfreelist_free(scb_module_t *scbstate)
{
	struct scb_info *ret = NULL;
	ret = SCBINFO(scbstate->free_list);
	while (ret) {
#ifdef MACOSX
		SCBFREESANITYCHECK(ret->scbpub);
#endif
		scbstate->free_list = (ret->next ? ret->next->scbpub : NULL);
		wlc_scb_freemem(scbstate, ret);
		ret = scbstate->free_list ? SCBINFO(scbstate->free_list) : NULL;
	}
}
#endif /* MACOSX */

void
wlc_internalscb_free(wlc_info_t *wlc, struct scb *scb)
{
	scb->permanent = FALSE;
	wlc_scbfree(wlc, scb);
}

static void
wlc_scb_reset(scb_module_t *scbstate, struct scb_info *scbinfo)
{
	struct scb *scbpub = scbinfo->scbpub;

	bzero((char*)scbinfo, sizeof(struct scb_info));
	scbinfo->scbpub = scbpub;
	bzero(scbpub, scbstate->scbtotsize);
	scbpub->scb_priv = (void *) scbinfo;
	/* init substructure pointers */
	scbpub->rssi_window = (int *)((char *)scbpub + sizeof(struct scb));
	scbpub->tx_path = (struct tx_path_node *)
	                ((char *)scbpub->rssi_window + (sizeof(int)*MA_WINDOW_SZ));
}

static struct scb_info *
wlc_scb_allocmem(scb_module_t *scbstate)
{
	struct scb_info *scbinfo = NULL;
	struct scb *scbpub;

	scbinfo = MALLOC(scbstate->pub->osh, sizeof(struct scb_info));
	if (!scbinfo) {
		WL_ERROR(("wl%d: %s: Internalscb alloc failure\n", scbstate->pub->unit,
		          __FUNCTION__));
		return NULL;
	}
	scbpub = MALLOC(scbstate->pub->osh, scbstate->scbtotsize);
	if (!scbpub) {
		wlc_scb_freemem(scbstate, scbinfo);
		WL_ERROR(("wl%d: %s: Internalscb alloc failure\n", scbstate->pub->unit,
		          __FUNCTION__));
		return NULL;
	}
	scbinfo->scbpub = scbpub;
	wlc_scb_reset(scbstate, scbinfo);

	return scbinfo;
}

struct scb *
wlc_internalscb_alloc(wlc_info_t *wlc, const struct ether_addr *ea, struct wlcband *band)
{
	struct scb_info *scbinfo = NULL;
	scb_module_t *scbstate = wlc->scbstate;
	int bcmerror = 0;

#ifdef SCBFREELIST
	/* If not found on freelist then allocate a new one */
	if ((scbinfo = wlc_scbget_free(scbstate)) == NULL) {
#endif
		scbinfo = wlc_scb_allocmem(scbstate);
		if (!scbinfo)
			return NULL;
#ifdef SCBFREELIST
	}
#endif

	wlc_scb_reset(scbstate, scbinfo);

	bcmerror = wlc_scbinit(wlc, band, scbinfo, INTERNAL_SCB);
	if (bcmerror) {
		wlc_internalscb_free(wlc, scbinfo->scbpub);
		WL_ERROR(("wl%d: %s failed with err %d\n", wlc->pub->unit, __FUNCTION__, bcmerror));
		return NULL;
	}
	scbinfo->scbpub->permanent = TRUE;

	bcopy(ea, &scbinfo->scbpub->ea, sizeof(struct ether_addr));

	wlc_scb_set_bsscfg(scbinfo->scbpub, wlc->cfg);

	return (scbinfo->scbpub);
}

struct scb *
wlc_userscb_alloc(wlc_info_t *wlc, struct wlcband *band)
{
	scb_module_t *scbstate = wlc->scbstate;
	struct scb_info *scbinfo = NULL;
	struct scb *oldscb;
	int bcmerror;

	if (scbstate->nscb < wlc->pub->tunables->maxscb) {
		scbinfo = wlc_scb_allocmem(scbstate);
		if (!scbinfo) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		}
	}
	if (!scbinfo) {
		/* free the oldest entry */
		if (!(oldscb = wlc_scbvictim(wlc))) {
			WL_ERROR(("wl%d: %s: no SCBs available to reclaim\n",
			          wlc->pub->unit, __FUNCTION__));
			return NULL;
		}
		if (!wlc_scbfree(wlc, oldscb)) {
			WL_ERROR(("wl%d: %s: Couldn't free a victimized scb\n",
			          wlc->pub->unit, __FUNCTION__));
			return NULL;
		}
		ASSERT(scbstate->nscb < wlc->pub->tunables->maxscb);
		/* allocate memory for scb */
		if (!(scbinfo = wlc_scb_allocmem(scbstate))) {
			WL_ERROR(("wl%d: %s: out of memory(FAIL), malloced %d bytes\n",
			          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			return NULL;
		}
	}

	scbstate->nscb++;

	SCBSANITYCHECK((scbstate)->scb);

	/* update scb link list */
	wlc_scb_reset(scbstate, scbinfo);
	scbinfo->next = SCBINFO(scbstate->scb);

#ifdef MACOSX
	scbinfo->next_copy = scbinfo->next;
#endif

	scbstate->scb = scbinfo->scbpub;

	bcmerror = wlc_scbinit(wlc, band, scbinfo, USER_SCB);
	if (bcmerror) {
		WL_ERROR(("wl%d: %s failed with err %d\n", wlc->pub->unit, __FUNCTION__, bcmerror));
		wlc_scbfree(wlc, scbinfo->scbpub);
		return NULL;
	}

	wlc_scb_set_bsscfg(scbinfo->scbpub, wlc->cfg);

	return (scbinfo->scbpub);
}

static int
wlc_scbinit(wlc_info_t *wlc, struct wlcband *band, struct scb_info *scbinfo, uint32 scbflags)
{
	struct scb *scb = NULL;
	scb_module_t *scbstate = wlc->scbstate;
	cubby_info_t *cubby_info;
	uint i;
	int bcmerror = 0;

	scb = scbinfo->scbpub;
	ASSERT(scb != NULL);

	scb->used = wlc->pub->now;
	scbinfo->band = band;

	for (i = 0; i < NUMPRIO; i++)
		scb->seqctl[i] = 0xFFFF;
	scb->seqctl_nonqos = 0xFFFF;

#ifdef MACOSX
	scb->magic = SCB_MAGIC;
#endif

	if (scbflags & INTERNAL_SCB)
		scb->flags2 |= SCB2_INTERNAL;

	for (i = 0; i < scbstate->ncubby; i++) {
		cubby_info = &scbstate->cubby_info[i];
		if (cubby_info->fn_init) {
			bcmerror = cubby_info->fn_init(cubby_info->context, scb);
			if (bcmerror) {
				WL_ERROR(("wl%d: %s: Cubby failed\n",
				          wlc->pub->unit, __FUNCTION__));
				return bcmerror;
			}
		}
	}

#ifdef MFP
	if (!(scb->sa_query_timer = wl_init_timer(wlc->wl, scb_sa_query_timeout, scb, "sa_query")))
		bcmerror = BCME_ERROR;
#endif /* MFP */

	return bcmerror;
}

static void
wlc_scb_freemem(scb_module_t *scbstate, struct scb_info *scbinfo)
{
	if (scbinfo->scbpub)
		MFREE(scbstate->pub->osh, scbinfo->scbpub, scbstate->scbtotsize);
	MFREE(scbstate->pub->osh, scbinfo, sizeof(struct scb_info));
}

bool
wlc_scbfree(wlc_info_t *wlc, struct scb *scbd)
{
	struct scb_info *remove = SCBINFO(scbd);
	struct scb_info *scbinfo;
	scb_module_t *scbstate = wlc->scbstate;
	cubby_info_t *cubby_info;
	uint i;
	uint8 prio;

#if defined(BCMDBG) || defined(WLMSG_WSEC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_WSEC */

	if (scbd->permanent)
		return FALSE;

	/* Return if SCB is already being deleted else mark it */
	if (scbd->flags & SCB_PENDING_FREE)
		return FALSE;
	else
		scbd->flags |= SCB_PENDING_FREE;

#ifdef MFP
	if (scbd->sa_query_timer) {
		wl_free_timer(wlc->wl, scbd->sa_query_timer);
		scbd->sa_query_timer = NULL;
	}
#endif /* MFP */

	/* free the per station key if one exists */
	if (scbd->key) {
		WL_WSEC(("wl%d: %s: deleting pairwise key for %s\n", wlc->pub->unit,
		        __FUNCTION__, bcm_ether_ntoa(&scbd->ea, eabuf)));
		ASSERT(!bcmp((char*)&scbd->key->ea, (char*)&scbd->ea, ETHER_ADDR_LEN));
		wlc_key_scb_delete(wlc, scbd);

	}

	for (i = 0; i < scbstate->ncubby; i++) {
		cubby_info = &scbstate->cubby_info[i];
		if (cubby_info->fn_deinit)
			cubby_info->fn_deinit(cubby_info->context, scbd);
	}

#ifdef AP
	/* free any leftover authentication state */
	if (scbd->challenge) {
		MFREE(wlc->osh, scbd->challenge, 2 + scbd->challenge[1]);
		scbd->challenge = NULL;
	}
	/* free WDS state */
	if (scbd->wds != NULL) {
		if (scbd->wds->wlif) {
			wlc_if_event(wlc, WLC_E_IF_DEL, scbd->wds);
			wl_del_if(wlc->wl, scbd->wds->wlif);
			scbd->wds->wlif = NULL;
		}
		wlc_wlcif_free(wlc, wlc->osh, scbd->wds);
		scbd->wds = NULL;
	}
	/* free wpaie if stored */
	if (scbd->wpaie) {
		MFREE(wlc->osh, scbd->wpaie, scbd->wpaie_len);
		scbd->wpaie_len = 0;
		scbd->wpaie = NULL;
	}
#endif /* AP */

	/* free any frame reassembly buffer */
	for (prio = 0; prio < NUMPRIO; prio++) {
		if (scbd->fragbuf[prio]) {
			PKTFREE(wlc->osh, scbd->fragbuf[prio], FALSE);
			scbd->fragbuf[prio] = NULL;
			scbd->fragresid[prio] = 0;
		}
	}

	scbd->state = 0;

	if (!SCB_INTERNAL(scbd)) {
		if (!ETHER_ISMULTI(scbd->ea.octet)) {
			wlc_scb_hash_del(wlc, scbd, remove->band->bandunit);
		}
		/* delete it from the link list */
		scbinfo = SCBINFO(scbstate->scb);
		if (scbinfo == remove) {
			scbstate->scb = wlc_scb_getnext(scbd);
		} else {
			while (scbinfo) {
				SCBSANITYCHECK(scbinfo->scbpub);
				if (scbinfo->next == remove) {
					scbinfo->next = remove->next;
#ifdef MACOSX
					scbinfo->next_copy = scbinfo->next;
#endif
					break;
				}
				scbinfo = scbinfo->next;
			}
			ASSERT(scbinfo != NULL);
		}
#ifdef AP
		/* mark the aid unused */
		if (scbd->aid) {
			ASSERT(AID2AIDMAP(scbd->aid) < wlc->pub->tunables->maxscb);
			clrbit(scbd->bsscfg->aidmap, AID2AIDMAP(scbd->aid));
		}
#endif /* AP */
		/* update total allocated scb number */
		scbstate->nscb--;
	}

#ifdef SCBFREELIST
	wlc_scbadd_free(scbstate, remove);
#else
	/* free scb memory */
	wlc_scb_freemem(scbstate, remove);
#endif

	return TRUE;
}


/* free all scbs, unless permanent. Force indicates reclaim permanent as well */

void
wlc_scbclear(struct wlc_info *wlc, bool force)
{
	struct scb_iter scbiter;
	struct scb *scb;

	if (wlc->scbstate == NULL)
		return;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (force)
			scb->permanent = FALSE;
		wlc_scbfree(wlc, scb);
	}
	if (force)
		ASSERT(wlc->scbstate->nscb == 0);
}


static struct scb *
wlc_scbvictim(wlc_info_t *wlc)
{
	uint oldest;
	struct scb *scb;
	struct scb *oldscb;
	uint now, age;
	struct scb_iter scbiter;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	wlc_bsscfg_t *bsscfg = NULL;

#ifdef AP
	/* search for an unauthenticated scb */
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (!scb->permanent && (scb->state == UNAUTHENTICATED))
			return scb;
	}
#endif /* AP */

	/* free the oldest scb */
	now = wlc->pub->now;
	oldest = 0;
	oldscb = NULL;
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		bsscfg = SCB_BSSCFG(scb);
		ASSERT(bsscfg != NULL);
		if (BSSCFG_STA(bsscfg) && bsscfg->BSS && SCB_ASSOCIATED(scb))
			continue;
		if (!scb->permanent && ((age = (now - scb->used)) >= oldest)) {
			oldest = age;
			oldscb = scb;
		}
	}
	/* handle extreme case(s): all are permanent ... or there are no scb's at all */
	if (oldscb == NULL)
		return NULL;

#ifdef AP
	bsscfg = SCB_BSSCFG(oldscb);

	if (BSSCFG_AP(bsscfg)) {
		/* if the oldest authenticated SCB has only been idle a short time then
		 * it is not a candidate to reclaim
		 */
		if (oldest < SCB_SHORT_TIMEOUT)
		    return NULL;

		/* notify the station that we are deauthenticating it */
		(void)wlc_senddeauth(wlc, &oldscb->ea, &bsscfg->BSSID,
		                     &bsscfg->cur_etheraddr,
		                     oldscb, DOT11_RC_INACTIVITY);
		wlc_deauth_complete(wlc, bsscfg, WLC_E_STATUS_SUCCESS, &oldscb->ea,
		              DOT11_RC_INACTIVITY, 0);
	}
#endif /* AP */

	WL_ASSOC(("wl%d: %s: relcaim scb %s, idle %d sec\n",  wlc->pub->unit, __FUNCTION__,
	          bcm_ether_ntoa(&oldscb->ea, eabuf), oldest));

	return oldscb;
}

/* "|" operation. */
void
wlc_scb_setstatebit(struct scb *scb, uint8 state)
{

	WL_NONE(("set state %x\n", state));
	ASSERT(scb != NULL);

	if (state & AUTHENTICATED)
	{
		scb->state &= ~PENDING_AUTH;
	}
	if (state & ASSOCIATED)
	{
		ASSERT((scb->state | state) & AUTHENTICATED);
		scb->state &= ~PENDING_ASSOC;
	}

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	if (state & AUTHORIZED)
	{
		if (!((scb->state | state) & ASSOCIATED) && !SCB_WDS(scb) &&
		    !SCB_IS_IBSS_PEER(scb)) {
			char eabuf[ETHER_ADDR_STR_LEN];
			WL_ASSOC(("wlc_scb : authorized %s is not a associated station, "
				"state = %x\n", bcm_ether_ntoa(&scb->ea, eabuf),
				scb->state));
		}
	}
#endif /* BCMDBG || WLMSG_ASSOC */

	scb->state |= state;
	WL_NONE(("wlc_scb : state = %x\n", scb->state));
}

/* "& ~" operation */
void
wlc_scb_clearstatebit(struct scb *scb, uint8 state)
{
	ASSERT(scb != NULL);
	WL_NONE(("clear state %x\n", state));
	scb->state &= ~state;
	WL_NONE(("wlc_scb : state = %x\n", scb->state));
}


/* "|" operation . idx = position of the bsscfg in the wlc array of multi ssids.
*/
void
wlc_scb_setstatebit_bsscfg(struct scb *scb, uint8 state, int idx)
{
	ASSERT(scb != NULL);
	WL_NONE(("set state : %x   bsscfg idx : %d\n", state, idx));
	if (state & ASSOCIATED)
	{

		ASSERT(SCB_AUTHENTICATED_BSSCFG(scb, idx));
		/* clear all bits (idx is set below) */
		memset(&scb->auth_bsscfg, 0, SCB_BSSCFG_BITSIZE);
		scb->state &= ~PENDING_ASSOC;
	}

	if (state & AUTHORIZED)
	{
		ASSERT(SCB_ASSOCIATED_BSSCFG(scb, idx));
	}
	setbit(&scb->auth_bsscfg, idx);
	scb->state |= state;
	WL_NONE(("wlc_scb : state = %x\n", scb->state));
}


/*
 * "& ~" operation .
 * idx = position of the bsscfg in the wlc array of multi ssids.
 */
void
wlc_scb_clearstatebit_bsscfg(struct scb *scb, uint8 state, int idx)

{
	int i;
	ASSERT(scb != NULL);
	WL_NONE(("clear state : %x   bsscfg idx : %d\n", state, idx));
	/*
	   any clear of a stable state should lead to clear a bit
	   Warning though : this implies that, if we want to switch from
	   associated to authenticated, the clear happens before the set
	   otherwise this bit will be clear in authenticated state.
	*/
	if ((state & AUTHENTICATED) || (state & ASSOCIATED) || (state & AUTHORIZED))
	{
		clrbit(&scb->auth_bsscfg, idx);
	}
	/* quik hack .. clear first ... */
	scb->state &= ~state;
	for (i = 0; i < SCB_BSSCFG_BITSIZE; i++)
	{
		/* reset if needed */
		if (scb->auth_bsscfg[i])
		{
			scb->state |= state;
			break;
		}
	}

}

/* reset all state. */
void
wlc_scb_resetstate(struct scb *scb)
{
	WL_NONE(("reset state\n"));
	ASSERT(scb != NULL);
	memset(&scb->auth_bsscfg, 0, SCB_BSSCFG_BITSIZE);
	scb->state = 0;
	WL_NONE(("wlc_scb : state = %x\n", scb->state));
}

void
wlc_scb_ratesel_init_all(wlc_info_t *wlc)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb)
		wlc_scb_ratesel_init(wlc, scb);
}

void
wlc_scb_ratesel_init_bss(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_BSSCFG(scb) == cfg)
			wlc_scb_ratesel_init(wlc, scb);
	}
}

/* initialize per-scb state utilized by rate selection */
void
wlc_scb_ratesel_init(wlc_info_t *wlc, struct scb *scb)
{
	bool is40 = FALSE;
	int8 sgi_tx = OFF;
	uint8 active_antcfg_num = 0;
	uint8 antselid_init = 0;
	int32 ac;

	if (SCB_INTERNAL(scb))
		return;
#ifdef WL11N
	if (WLANTSEL_ENAB(wlc))
		wlc_antsel_ratesel(wlc->asi, &active_antcfg_num, &antselid_init);

	is40 = ((scb->flags & SCB_IS40) ? TRUE : FALSE) &&
	        wlc->band->mimo_cap_40 && CHSPEC_IS40(wlc->chanspec);

	if (wlc->sgi_tx == AUTO) {
		if ((is40 && (scb->flags2 & SCB2_SGI40_CAP)) ||
		    (!is40 && (scb->flags2 & SCB2_SGI20_CAP)))
			sgi_tx = AUTO;

		/* Disable SGI Tx in 20MHz on IPA chips */
		if (!is40 && wlc->stf->ipaon)
			sgi_tx = OFF;
	}
#endif /* WL11N */

	for (ac = 0; ac < WME_MAX_AC(wlc, scb); ac++) {
		wlc_ratesel_init(wlc->rsi, scb, &scb->rateset, is40, sgi_tx,
#ifdef WLAFTERBURNER
			wlc->afterburner,
#endif /* WLAFTERBURNER */
			wlc_txc_get_cubby(wlc, scb),
			active_antcfg_num, antselid_init, ac);
	}
}

/* set/change bsscfg */
void
wlc_scb_set_bsscfg(struct scb *scb, wlc_bsscfg_t *cfg)
{
	wlc_bsscfg_t *oldcfg = SCB_BSSCFG(scb);
	wlc_info_t *wlc = cfg->wlc;
	scb->bsscfg = cfg;

	if (oldcfg == NULL || oldcfg != cfg) {
		wlcband_t *band = wlc_scbband(scb);
		wlc_rateset_t *rs;

		/* flag the scb is used by IBSS */
		if (cfg->BSS)
			wlc_scb_clearstatebit(scb, TAKEN4IBSS);
		else {
			wlc_scb_resetstate(scb);
			wlc_scb_setstatebit(scb, TAKEN4IBSS);
		}

		/* Invalidate Tx header cache */
		wlc_txc_invalidate_cache(wlc_txc_get_cubby(wlc, scb));

		/* use current, target, or per-band default rateset? */
		if (wlc->pub->up && wlc_valid_chanspec(wlc->cmi, cfg->target_bss->chanspec))
			if (cfg->associated)
				rs = &cfg->current_bss->rateset;
			else
				rs = &cfg->target_bss->rateset;
		else
			rs = &band->defrateset;

		/*
		 * Initialize the per-scb rateset:
		 * - if we are AP, start with only the basic subset of the
		 *	network rates.  It will be updated when receive the next
		 *	probe request or association request.
		 * - if we are IBSS and gmode, special case:
		 *	start with B-only subset of network rates and probe for ofdm rates
		 * - else start with the network rates.
		 *	It will be updated on join attempts.
		 */
		/* initialize the scb rateset */
		if (BSSCFG_AP(cfg)) {
#ifdef WLP2P
			if (BSS_P2P_ENAB(wlc, cfg))
				wlc_rateset_filter(rs, &scb->rateset, FALSE, WLC_RATES_OFDM,
				                   RATE_MASK, BSS_N_ENAB(wlc, cfg));
			else
#endif
			wlc_rateset_filter(rs, &scb->rateset, TRUE, WLC_RATES_CCK_OFDM, RATE_MASK,
			                   BSS_N_ENAB(wlc, cfg));
		}
		else if (!cfg->BSS && band->gmode) {
			wlc_rateset_filter(rs, &scb->rateset, FALSE, WLC_RATES_CCK,
			                   RATE_MASK, FALSE);
			/* if resulting set is empty, then take all network rates instead */
			if (scb->rateset.count == 0)
				wlc_rateset_filter(rs, &scb->rateset, FALSE, WLC_RATES_CCK_OFDM,
				                   RATE_MASK, FALSE);
		}
		else {
#ifdef WLP2P
			if (BSS_P2P_ENAB(wlc, cfg))
				wlc_rateset_filter(rs, &scb->rateset, FALSE, WLC_RATES_OFDM,
				                   RATE_MASK, BSS_N_ENAB(wlc, cfg));
			else
#endif
			wlc_rateset_filter(rs, &scb->rateset, FALSE, WLC_RATES_CCK_OFDM,
			                   RATE_MASK, FALSE);
		}

		if (!SCB_INTERNAL(scb))
			wlc_scb_ratesel_init(wlc, scb);
	}
}

void
wlc_scb_reinit(wlc_info_t *wlc)
{
	uint prev_count;
	const wlc_rateset_t *rs;
	wlcband_t *band;
	struct scb *scb;
	struct scb_iter scbiter;
	bool cck_only;
	bool reinit_forced;

	WL_INFORM(("wl%d: %s: bandunit 0x%x phy_type 0x%x gmode 0x%x\n", wlc->pub->unit,
		__FUNCTION__, wlc->band->bandunit, wlc->band->phytype, wlc->band->gmode));

	/* sanitize any existing scb rates against the current hardware rates */
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		prev_count = scb->rateset.count;
		/* Keep only CCK if gmode == GMODE_LEGACY_B */
		band = SCBINFO(scb)->band;
		if (BAND_2G(band->bandtype) && (band->gmode == GMODE_LEGACY_B)) {
			rs = &cck_rates;
			cck_only = TRUE;
		} else {
			rs = &band->hw_rateset;
			cck_only = FALSE;
		}
		if (!wlc_rate_hwrs_filter_sort_validate(&scb->rateset, rs, FALSE,
			wlc->stf->txstreams)) {
			/* continue with default rateset.
			 * since scb rateset does not carry basic rate indication,
			 * clear basic rate bit.
			 */
			WL_RATE(("wl%d: %s: invalid rateset in scb 0x%p bandunit 0x%x "
				"phy_type 0x%x gmode 0x%x\n", wlc->pub->unit, __FUNCTION__,
				scb, band->bandunit, band->phytype, band->gmode));
#ifdef BCMDBG
			wlc_rateset_show(wlc, &scb->rateset, &scb->ea);
#endif

			wlc_rateset_default(&scb->rateset, &band->hw_rateset, band->phytype,
				band->bandtype, cck_only, RATE_MASK, BSS_N_ENAB(wlc, scb->bsscfg),
				CHSPEC_WLC_BW(scb->bsscfg->current_bss->chanspec),
				wlc->stf->txstreams);
			reinit_forced = TRUE;
		}
		else
			reinit_forced = FALSE;

		/* if the count of rates is different, then the rate state
		 * needs to be reinitialized
		 */
		if (reinit_forced || (scb->rateset.count != prev_count))
			wlc_scb_ratesel_init(wlc, scb);

		WL_RATE(("wl%d: %s: bandunit 0x%x, phy_type 0x%x gmode 0x%x. final rateset is\n",
			wlc->pub->unit, __FUNCTION__,
			band->bandunit, band->phytype, band->gmode));
#ifdef BCMDBG
		wlc_rateset_show(wlc, &scb->rateset, &scb->ea);
#endif
	}
}

static INLINE struct scb* BCMFASTPATH
_wlc_scbfind(wlc_info_t *wlc, const struct ether_addr *ea, int bandunit)
{
	int indx;
	struct scb_info *scbinfo;

	/* All callers of wlc_scbfind() should first be checking to see
	 * if the SCB they're looking for is a BC/MC address.  Because we're
	 * using per bsscfg BCMC SCBs, we can't "find" BCMC SCBs without
	 * knowing which bsscfg.
	 */
	ASSERT(!ETHER_ISMULTI(ea));

	indx = SCBHASHINDEX(wlc->scbstate->nscbhash, ea->octet);

	/* search for the scb which corresponds to the remote station ea */
	scbinfo = (wlc->scbstate->scbhash[bandunit][indx] ?
	           SCBINFO(wlc->scbstate->scbhash[bandunit][indx]) : NULL);
	for (; scbinfo; scbinfo = scbinfo->hashnext) {
		SCBSANITYCHECK(scbinfo->scbpub);

		/* pxy add, check whether they are non-NULL*/
		if ((const char*)ea &&(const char*)&(scbinfo->scbpub->ea) && 
			bcmp((const char*)ea, (const char*)&(scbinfo->scbpub->ea), ETHER_ADDR_LEN) == 0)
			break;
	}

	return (scbinfo ? scbinfo->scbpub : NULL);
}

#ifdef WLMEDIA_IPTV
bool wlc_is_scb_videotag(wlc_info_t *wlc, const struct ether_addr *ea)
{
    struct scb * scb;

        if (ETHER_ISMULTI(ea))
        return(FALSE);
    scb = _wlc_scbfind(wlc, ea, wlc->band->bandunit);
    if( (scb) && (scb == wlc->videotag_scb) ) {
        return(TRUE);
    }
    else {
        return(FALSE);
    }
}
#endif /* WLMEDIA_IPTV */

/* Find station control block corresponding to the remote id */
struct scb * BCMFASTPATH
wlc_scbfind(wlc_info_t *wlc, const struct ether_addr *ea)
{
	return _wlc_scbfind(wlc, ea, wlc->band->bandunit);
}

/*
 * Lookup station control block corresponding to the remote id.
 * If not found, create a new entry.
 */
static INLINE struct scb *
_wlc_scblookup(wlc_info_t *wlc, const struct ether_addr *ea, int bandunit)
{
	struct scb *scb;
	struct wlcband *band;
#ifdef STA
	wlc_bsscfg_t *bsscfg;
#endif

	/* Don't allocate/find a BC/MC SCB this way. */
	ASSERT(!ETHER_ISMULTI(ea));
	if (ETHER_ISMULTI(ea))
		return NULL;

	if ((scb = _wlc_scbfind(wlc, ea, bandunit)))
		return (scb);

	/* no scb match, allocate one for the desired bandunit */
	band = wlc->bandstate[bandunit];
	scb = wlc_userscb_alloc(wlc, band);
	if (!scb)
		return (NULL);

	scb->ea = *((const struct ether_addr*)ea);

	/* install it in the cache */
	wlc_scb_hash_add(wlc, scb, bandunit);

	scb->bandunit = bandunit;

#ifdef STA
	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	/* send ofdm rate probe */
	if (BSSCFG_STA(bsscfg) && !bsscfg->BSS && band->gmode && wlc->pub->up)
		wlc_rateprobe(wlc, &scb->ea, WLC_RATEPROBE_RATE);
#endif /* STA */

	return (scb);
}

struct scb *
wlc_scblookup(wlc_info_t *wlc, const struct ether_addr *ea)
{
	return (_wlc_scblookup(wlc, ea, wlc->band->bandunit));
}

struct scb *
wlc_scblookupband(wlc_info_t *wlc, const struct ether_addr *ea, int bandunit)
{
	/* assert that the band is the current band, or we are dual band and it is the other band */
	ASSERT((bandunit == (int)wlc->band->bandunit) ||
	       (NBANDS(wlc) > 1 && bandunit == (int)OTHERBANDUNIT(wlc)));

	return (_wlc_scblookup(wlc, ea, bandunit));
}

/* Get scb from band */
struct scb * BCMFASTPATH
wlc_scbfindband(wlc_info_t *wlc, const struct ether_addr *ea, int bandunit)
{
	/* assert that the band is the current band, or we are dual band and it is the other band */
	ASSERT((bandunit == (int)wlc->band->bandunit) ||
	       (NBANDS(wlc) > 1 && bandunit == (int)OTHERBANDUNIT(wlc)));

	return (_wlc_scbfind(wlc, ea, bandunit));
}

/* Determine if any SCB associated to ap cfg
 * cfg specifies a specific ap cfg to compare to.
 * If cfg is NULL, then compare to any ap cfg.
 */
bool
wlc_scb_associated_to_ap(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb_iter scbiter;
	struct scb *scb;
	bool associated = FALSE;

	ASSERT((cfg == NULL) || BSSCFG_AP(cfg));

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_ASSOCIATED(scb) && BSSCFG_AP(scb->bsscfg)) {
			if ((cfg == NULL) || (cfg == scb->bsscfg)) {
				associated = TRUE;
			}
		}
	}

	return (associated);
}

/* Move the scb's band info.
 * Parameter description:
 *
 * wlc - global wlc_info structure
 * bsscfg - the bsscfg that is about to move to a new chanspec
 * chanspec - the new chanspec the bsscfg is moving to
 *
 */
void
wlc_scb_update_band_for_cfg(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, chanspec_t chanspec)
{
	struct scb_iter scbiter;
	struct scb *scb, *stale_scb;
	struct scb_info *scbinfo;
	int bandunit;
	bool reinit = FALSE;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (scb->bsscfg == bsscfg && SCB_ASSOCIATED(scb)) {
			scbinfo = SCBINFO(scb);
			bandunit = CHSPEC_WLCBANDUNIT(chanspec);
			if (scb->bandunit != (uint)bandunit) {
				/* We're about to move our scb to the new band.
				 * Check to make sure there isn't an scb entry for us there.
				 * If there is one for us, delete it first.
				 */
				if ((stale_scb = _wlc_scbfind(wlc, &bsscfg->BSSID, bandunit)) &&
				    (stale_scb->permanent == FALSE)) {
					WL_ASSOC(("wl%d.%d: %s: found stale scb %p on %s band, "
					          "remove it\n",
					          wlc->pub->unit, bsscfg->_idx, __FUNCTION__,
					          stale_scb,
					          (bandunit == BAND_5G_INDEX) ? "5G" : "2G"));
					/* mark the scb for removal */
					stale_scb->stale_remove = TRUE;
				}
				/* Now perform the move of our scb to the new band */

				/* first, del scb from hash table in old band */
				wlc_scb_hash_del(wlc, scb, scb->bandunit);
				/* next add scb to hash table in new band */
				wlc_scb_hash_add(wlc, scb, bandunit);
				/* update the scb's band */
				scb->bandunit = bandunit;
				scbinfo->band = wlc->bandstate[bandunit];
				reinit = TRUE;
			}
		}
	}
	/* remove stale scb's marked for removal */
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (scb->stale_remove == TRUE) {
			WL_ASSOC(("remove stale scb %p\n", scb));
			scb->stale_remove = FALSE;
			wlc_scbfree(wlc, scb);
		}
	}

	if (reinit) {
		wlc_scb_reinit(wlc);
	}
}

/* (de)authorize/(de)authenticate single station
 * 'enable' TRUE means authorize, FLASE means deauthorize/deauthenticate
 * 'flag' is AUTHORIZED or AUTHENICATED for the type of operation
 * 'rc' is the reason code for a deauthenticate packet
 */
void
wlc_scb_set_auth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb, bool enable, uint32 flag,
                 int rc)
{
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	void *pkt = NULL;

	if (enable) {
		if (flag == AUTHORIZED) {
			wlc_scb_setstatebit(scb, AUTHORIZED);
			scb->flags &= ~SCB_DEAUTH;
		} else {
			wlc_scb_setstatebit(scb, AUTHENTICATED);
		}
	} else {
		if (flag == AUTHORIZED) {

			wlc_scb_clearstatebit(scb, AUTHORIZED);
		} else {

			if (wlc->pub->up && (SCB_AUTHENTICATED(scb) || SCB_WDS(scb))) {
				pkt = wlc_senddeauth(wlc, &scb->ea, &bsscfg->BSSID,
				                     &bsscfg->cur_etheraddr,
				                     scb, (uint16)rc);
			}
			if (pkt == NULL ||
			    wlc_pkt_callback_register(wlc, wlc_deauth_sendcomplete,
			                              (void *)&scb->ea, pkt))
				WL_ERROR(("wl%d: wlc_scb_set_auth: could not register callback\n",
				          wlc->pub->unit));
		}
	}
	WL_ASSOC(("wl%d: %s: %s %s%s\n", wlc->pub->unit, __FUNCTION__,
		bcm_ether_ntoa(&scb->ea, eabuf),
		(enable ? "" : "de"),
		((flag == AUTHORIZED) ? "authorized" : "authenticated")));
}

static void
wlc_scb_hash_add(wlc_info_t *wlc, struct scb *scb, int bandunit)
{
	scb_module_t *scbstate = wlc->scbstate;
	int indx = SCBHASHINDEX(scbstate->nscbhash, scb->ea.octet);
	struct scb_info *scbinfo = (scbstate->scbhash[bandunit][indx] ?
	                            SCBINFO(scbstate->scbhash[bandunit][indx]) : NULL);

	SCBINFO(scb)->hashnext = scbinfo;
#ifdef MACOSX
	SCBINFO(scb)->hashnext_copy = SCBINFO(scb)->hashnext;
#endif

	scbstate->scbhash[bandunit][indx] = scb;
}

static void
wlc_scb_hash_del(wlc_info_t *wlc, struct scb *scbd, int bandunit)
{
	scb_module_t *scbstate = wlc->scbstate;
	int indx = SCBHASHINDEX(scbstate->nscbhash, scbd->ea.octet);
	struct scb_info *scbinfo;
	struct scb_info *remove = SCBINFO(scbd);

	/* delete it from the hash */
	scbinfo = (scbstate->scbhash[bandunit][indx] ?
	           SCBINFO(scbstate->scbhash[bandunit][indx]) : NULL);
	ASSERT(scbinfo != NULL);
	SCBSANITYCHECK(scbinfo->scbpub);
	/* special case for the first */
	if (scbinfo == remove) {
		if (scbinfo->hashnext)
		    SCBSANITYCHECK(scbinfo->hashnext->scbpub);
		scbstate->scbhash[bandunit][indx] =
		        (scbinfo->hashnext ? scbinfo->hashnext->scbpub : NULL);
	} else {
		for (; scbinfo; scbinfo = scbinfo->hashnext) {
			SCBSANITYCHECK(scbinfo->hashnext->scbpub);
			if (scbinfo->hashnext == remove) {
				scbinfo->hashnext = remove->hashnext;
#ifdef MACOSX
				scbinfo->hashnext_copy = scbinfo->hashnext;
#endif
				break;
			}
		}
		ASSERT(scbinfo != NULL);
	}
}

#ifdef AP
void
wlc_scb_wds_free(struct wlc_info *wlc)
{
	struct scb *scb;
	struct scb_iter scbiter;
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (scb->wds) {
			scb->permanent = FALSE;
			wlc_scbfree(wlc, scb);
		}
	}
}
#endif /* AP */


#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_dump_scb(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint k, i;
	struct scb *scb;
	struct scb_info *scbinfo;
	char eabuf[ETHER_ADDR_STR_LEN];
	char flagstr[64];
	char flagstr2[64];
	char statestr[64];
	struct scb_iter scbiter;
	cubby_info_t *cubby_info;
	scb_module_t *scbstate = wlc->scbstate;
#ifdef AP
	char ssidbuf[SSID_FMT_BUF_LEN] = "";
#endif /* AP */
	wlc_bsscfg_t *cfg;

	bcm_bprintf(b, "idx  ether_addr\n");
	k = 0;
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		scbinfo = SCBINFO(scb);

		cfg = SCB_BSSCFG(scb);
		ASSERT(cfg != NULL);

		bcm_format_flags(scb_flags, scb->flags, flagstr, 64);
		bcm_format_flags(scb_flags2, scb->flags2, flagstr2, 64);
		bcm_format_flags(scb_states, scb->state, statestr, 64);

		bcm_bprintf(b, "%3d%s %s\n", k, (scb->permanent? "*":" "),
			bcm_ether_ntoa(&scb->ea, eabuf));

		bcm_bprintf(b, "     State:0x%02x (%s) Used:%d(%d)\n",
		            scb->state, statestr, scb->used, (int)(scb->used - wlc->pub->now));
		bcm_bprintf(b, "     Band:%s",
		            ((scb->bandunit == BAND_2G_INDEX) ? BAND_2G_NAME :
		             BAND_5G_NAME));
		bcm_bprintf(b, " Flags:0x%x", scb->flags);
		if (flagstr[0] != '\0')
			bcm_bprintf(b, " (%s)", flagstr);
		bcm_bprintf(b, " Flags2:0x%x", scb->flags2);
		if (flagstr2[0] != '\0')
			bcm_bprintf(b, " (%s)", flagstr2);
		if (scb->key)
			bcm_bprintf(b, " Key:%d", scb->key->idx);
		bcm_bprintf(b, " Cfg:%d(%p)", WLC_BSSCFG_IDX(cfg), cfg);
		if (scb->flags & SCB_AMSDUCAP)
			bcm_bprintf(b, " AMSDU-MTU:%d", scb->amsdu_mtu_pref);
		bcm_bprintf(b, "\n");

		if (scb->key) {
			bcm_bprintf(b, "     Key ID:%d algo:%s length:%d data:",
				scb->key->id,
			        bcm_crypto_algo_name(scb->key->algo),
				scb->key->idx, scb->key->len);
			if (scb->key->len)
				bcm_bprintf(b, "0x");
			for (i = 0; i < scb->key->len; i++)
				bcm_bprintf(b, "%02X", scb->key->data[i]);
			for (i = 0; i < scb->key->len; i++)
				if (!bcm_isprint(scb->key->data[i]))
					break;
			if (i == scb->key->len)
				bcm_bprintf(b, " (%.*s)", scb->key->len, scb->key->data);
			bcm_bprintf(b, "\n");
		}

		wlc_dump_rateset("     rates", &scb->rateset, b);
		bcm_bprintf(b, "\n");

		if (scb->rateset.htphy_membership) {
			bcm_bprintf(b, "     membership %d(b)",
				(scb->rateset.htphy_membership & RATE_MASK));
			bcm_bprintf(b, "\n");
		}
#ifdef AP
		if (BSSCFG_AP(cfg)) {
			bcm_bprintf(b, "     AID:0x%x PS:%d Listen:%d WDS:%d(%p) RSSI:%d",
			               scb->aid, scb->PS, scb->listen, (scb->wds ? 1 : 0),
			               scb->wds, wlc_scb_rssi(scb));
			wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
			bcm_bprintf(b, " BSS %d \"%s\"\n",
			            WLC_BSSCFG_IDX(cfg), ssidbuf);
		}
#endif
		if (BSSCFG_STA(cfg)) {
			bcm_bprintf(b, "     MAXSP:%u DEFL:0x%x TRIG:0x%x DELV:0x%x\n",
			            scb->apsd.maxsplen, scb->apsd.ac_defl,
			            scb->apsd.ac_trig, scb->apsd.ac_delv);
		}

#ifdef WL11N
		if (N_ENAB(wlc->pub) && SCB_HT_CAP(scb)) {
			wlc_dump_mcsset("     HT mcsset :", &scb->rateset.mcs[0], b);
			bcm_bprintf(b,  "\n     HT capabilites 0x%04x ampdu_params 0x%02x "
			    "mimops_enabled %d mimops_rtsmode %d",
			    scb->ht_capabilities, scb->ht_ampdu_params, scb->ht_mimops_enabled,
			    scb->ht_mimops_rtsmode);
			bcm_bprintf(b,  "\n     wsec 0x%x", scb->wsec);
			bcm_bprintf(b, "\n");
		}
		wlc_dump_rclist("     rclist", scb->rclist, scb->rclen, b);
#endif  /* WL11N */

		for (i = 0; i < scbstate->ncubby; i++) {
			cubby_info = &scbstate->cubby_info[i];
			if (cubby_info->fn_dump)
				cubby_info->fn_dump(cubby_info->context, scb, b);
		}

		wlc_scb_txpath_dump(wlc, scb, b);

		k++;
	}

	return 0;
}
#endif /* BCMDBG || BCMDBG_DUMP */

void
wlc_scb_sortrates(wlc_info_t *wlc, struct scb *scb)
{
	struct scb_info *scbinfo = SCBINFO(scb);
	wlc_rate_hwrs_filter_sort_validate(&scb->rateset, &scbinfo->band->hw_rateset, FALSE,
		wlc->stf->txstreams);
}

void
BCMINITFN(wlc_scblist_validaterates)(wlc_info_t *wlc)
{
	struct scb *scb;
	struct scb_iter scbiter;
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		wlc_scb_sortrates(wlc, scb);
		if (scb->rateset.count == 0) {
			wlc_scbfree(wlc, scb);
		}
	}
}

#if defined(AP)
int
wlc_scb_rssi(struct scb *scb)
{
	int rssi = 0;
	int i;

	for (i = 0; i < MA_WINDOW_SZ; i++)
		rssi += scb->rssi_window[i];
	rssi /= MA_WINDOW_SZ;

	return (rssi);
}

void
wlc_scb_rssi_init(struct scb *scb, int rssi)
{
	int i;

	for (i = 0; i < MA_WINDOW_SZ; i++)
		scb->rssi_window[i] = rssi;
}
#endif 

/* Give then tx_fn, return the feature id from txmod_fns array.
 * If tx_fn is NULL, 0 will be returned
 * If entry is not found, it's an ERROR!
 */
static INLINE scb_txmod_t
wlc_scb_txmod_fid(wlc_info_t *wlc, txmod_tx_fn_t tx_fn)
{
	scb_txmod_t txmod;

	for (txmod = TXMOD_START; txmod < TXMOD_LAST; txmod++)
		if (tx_fn == wlc->txmod_fns[txmod].tx_fn)
			return txmod;

	/* Should not reach here */
	ASSERT(txmod < TXMOD_LAST);
	return txmod;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_scb_txpath_dump(wlc_info_t *wlc, struct scb *scb, struct bcmstrbuf *b)
{
	const uchar txmod_names[][20] = {
		"Start", "DPT", "CRAM", "APPS", "A-MSDU", "Block Ack", "A-MPDU", "Transmit"
	};

	scb_txmod_t fid;
	bcm_bprintf(b, "     Tx Path:");
	fid = TXMOD_START;
	do {
		bcm_bprintf(b, " -> %s",
		       txmod_names[wlc_scb_txmod_fid(wlc, scb->tx_path[fid].next_tx_fn)]);
		fid = wlc_scb_txmod_fid(wlc, scb->tx_path[fid].next_tx_fn);
	} while (fid != TXMOD_TRANSMIT && fid != 0);

	bcm_bprintf(b, "\n");
	return 0;
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* Add a feature to the path. It should not be already on the path and should be configured
 * Does not take care of evicting anybody
 */
void
wlc_scb_txmod_activate(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid)
{
	/* Numeric value designating this feature's position in tx_path */
	static const uint8 txmod_position[TXMOD_LAST] = {
		0, /* TXMOD_START */
		1, /* TXMOD_DPT */
		2, /* TXMOD_CRAM */
		4, /* TXMOD_APPS */
		2, /* TXMOD_AMSDU */
		3, /* TXMOD_BA */
		3, /* TXMOD_AMPDU */
		5, /* TXMOD_TRANSMIT */
	};

	uint curr_mod_position;
	scb_txmod_t prev, next;
	txmod_info_t curr_mod_info = wlc->txmod_fns[fid];

	ASSERT(SCB_TXMOD_CONFIGURED(scb, fid) &&
	       !SCB_TXMOD_ACTIVE(scb, fid));

	curr_mod_position = txmod_position[fid];

	prev = TXMOD_START;

	while ((next = wlc_scb_txmod_fid(wlc, scb->tx_path[prev].next_tx_fn)) != 0 &&
	       txmod_position[next] < curr_mod_position)
		prev = next;

	/* next == 0 indicate this is the first addition to the path
	 * it HAS to be TXMOD_TRANSMIT as it's the one that puts the packet in
	 * txq. If this changes, then assert will need to be removed.
	 */
	ASSERT(next != 0 || fid == TXMOD_TRANSMIT);
	ASSERT(txmod_position[next] != curr_mod_position);

	SCB_TXMOD_SET(scb, prev, fid);
	SCB_TXMOD_SET(scb, fid, next);

	/* invoke any activate notify functions now that it's in the path */
	if (curr_mod_info.activate_notify_fn)
		curr_mod_info.activate_notify_fn(curr_mod_info.ctx, scb);
}

/* Remove a fid from the path. It should be already on the path
 * Does not take care of replacing it with any other feature.
 */
void
wlc_scb_txmod_deactivate(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid)
{
	scb_txmod_t prev, next;
	txmod_info_t curr_mod_info = wlc->txmod_fns[fid];

	/* If not active, do nothing */
	if (!SCB_TXMOD_ACTIVE(scb, fid))
		return;

	/* if deactivate notify function is present, call it */
	if (curr_mod_info.deactivate_notify_fn)
		curr_mod_info.deactivate_notify_fn(curr_mod_info.ctx, scb);

	prev = TXMOD_START;

	while ((next = wlc_scb_txmod_fid(wlc, scb->tx_path[prev].next_tx_fn))
	       != fid)
		prev = next;

	SCB_TXMOD_SET(scb, prev, wlc_scb_txmod_fid(wlc, scb->tx_path[fid].next_tx_fn));
	scb->tx_path[fid].next_tx_fn = NULL;
}

#ifdef WDS
int
wlc_wds_create(wlc_info_t *wlc, struct scb *scb, uint flags)
{
	ASSERT(scb != NULL);

	/* honor the existing WDS link */
	if (scb->wds != NULL) {
		scb->permanent = TRUE;
		return BCME_OK;
	}

	if (!(flags & WDS_INFRA_BSS) && SCB_ISMYAP(scb)) {
#ifdef BCMDBG_ERR
		char eabuf[ETHER_ADDR_STR_LEN];
		WL_ERROR(("wl%d: rejecting WDS %s, associated to it as our AP\n",
		          wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
#endif /* BCMDBG_ERR */
		return BCME_ERROR;
	}

	/* allocate a wlc_if_t for the wds interface and fill it out */
	scb->wds = wlc_wlcif_alloc(wlc, wlc->osh, WLC_IFTYPE_WDS, wlc->active_queue);
	if (scb->wds == NULL) {
		WL_ERROR(("wl%d: wlc_wds_create: failed to alloc wlcif\n",
		          wlc->pub->unit));
		return BCME_NOMEM;
	}
	scb->wds->u.scb = scb;

#ifdef AP
	/* create an upper-edge interface */
	if (!(flags & WDS_INFRA_BSS)) {
		/* a WDS scb has an AID for a unique WDS interface unit number */
		if (scb->aid == 0)
			scb->aid = wlc_bsscfg_newaid(scb->bsscfg);
		scb->wds->wlif = wl_add_if(wlc->wl, scb->wds, AID2PVBMAP(scb->aid), &scb->ea);
		if (scb->wds->wlif == NULL) {
			MFREE(wlc->osh, scb->wds, sizeof(wlc_if_t));
			scb->wds = NULL;
			return BCME_NOMEM;
		}
		scb->bsscfg->wlcif->flags |= WLC_IF_LINKED;
		wlc_if_event(wlc, WLC_E_IF_ADD, scb->wds);
	}

	wlc_wds_wpa_role_set(wlc->ap, scb, WL_WDS_WPA_ROLE_AUTO);
#endif /* AP */

	/* override WDS nodes rates to the full hw rate set */
	wlc_rateset_filter(&wlc->band->hw_rateset, &scb->rateset, FALSE,
	                   WLC_RATES_CCK_OFDM, RATE_MASK, BSS_N_ENAB(wlc, scb->bsscfg));
	wlc_scb_ratesel_init(wlc, scb);

	scb->permanent = TRUE;
	scb->flags &= ~SCB_MYAP;

	return BCME_OK;
}
#endif /* WDS */
#ifdef MFP
void *
wlc_scb_send_action_sa_query(wlc_info_t *wlc, struct scb *scb, uint8 action, uint16 id)
{
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_action_sa_query *af;

	body_len = sizeof(struct dot11_action_sa_query);

	if ((p = wlc_frame_get_action(wlc, FC_ACTION, &scb->ea, &scb->bsscfg->cur_etheraddr,
	        &scb->bsscfg->BSSID, body_len, &pbody, DOT11_ACTION_CAT_SA_QUERY)) == NULL) {
		return NULL;
	}

	af = (struct dot11_action_sa_query *)pbody;
	af->category = DOT11_ACTION_CAT_SA_QUERY;
	af->action = action;
	af->id = id;

	wlc_sendmgmt(wlc, p, scb->bsscfg->wlcif->qi, scb);

	return p;
}

void
wlc_scb_start_sa_query(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb)
{
	if ((scb == NULL) || !SCB_ASSOCIATED(scb))
		return;

	if (scb->sa_query_started)
		return;
	scb->sa_query_started = TRUE;
	scb->sa_query_count = 0;
	/* Token needs to be non-zero, so burn the high bit */
	scb->sa_query_id = (uint16)(wlc->counter | 0x8000);
	/* start periodic timer */
	wl_add_timer(wlc->wl, scb->sa_query_timer, 200, TRUE);
	wlc_scb_send_action_sa_query(wlc, scb, SA_QUERY_REQUEST, scb->sa_query_id);

}

static void
wlc_scb_recv_sa_resp(wlc_info_t *wlc, struct scb *scb, struct dot11_action_sa_query *af)
{
	if (scb->sa_query_started == FALSE)
		return;
	if (af->id == scb->sa_query_id) {
		wl_del_timer(wlc->wl, scb->sa_query_timer);
		scb->sa_query_started = FALSE;
		scb->sa_query_count = 0;
		return;
	}
	/* id mismatch, continue query */
	scb->sa_query_id = (uint16)(wlc->counter | 0x8000);
	wlc_scb_send_action_sa_query(wlc, scb, SA_QUERY_REQUEST, scb->sa_query_id);
}

static void
scb_sa_query_timeout(void *arg)
{
	struct scb *scb = (struct scb *)arg;
	wlc_bsscfg_t *cfg = scb->bsscfg;
	wlc_info_t *wlc = cfg->wlc;

	scb->sa_query_count++;
	/* total time out = 25*200ms = 5 seconds */
	if (scb->sa_query_count > 25) {
		wl_del_timer(wlc->wl, scb->sa_query_timer);
		scb->sa_query_count = 0;
		scb->sa_query_started = FALSE;
		/* disassoc scb */
		wlc_senddisassoc(wlc, &scb->ea, &cfg->BSSID,
		                 &cfg->cur_etheraddr,
		                 scb, DOT11_RC_NOT_AUTH);
		wlc_scb_clearstatebit(scb, ASSOCIATED | AUTHORIZED);
		wlc_scb_disassoc_cleanup(wlc, scb);
		wlc_disassoc_complete(cfg, WLC_E_STATUS_SUCCESS, &cfg->BSSID,
			DOT11_RC_DISASSOC_LEAVING, DOT11_BSSTYPE_INFRASTRUCTURE);

	} else {
		scb->sa_query_id++;
		scb->sa_query_id |= 0x8000;
		wlc_scb_send_action_sa_query(wlc, scb, SA_QUERY_REQUEST, scb->sa_query_id);
	}
}

void
wlc_scb_sa_query(wlc_info_t *wlc, uint action_id, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	struct dot11_action_sa_query *af;

	if (scb == NULL)
		return;

	af = (struct dot11_action_sa_query *)body;
	WL_WSEC(("wl%d: Rcvd SA Query action frame with id %d\n", wlc->pub->unit, action_id));

	switch (action_id) {
		case SA_QUERY_REQUEST:
			if (SCB_ASSOCIATED(scb)) {
				wlc_scb_send_action_sa_query(wlc, scb, SA_QUERY_RESPONSE, af->id);
			}
			break;

		case SA_QUERY_RESPONSE:
			wlc_scb_recv_sa_resp(wlc, scb, af);
			break;

		default:
			WL_ERROR(("wl %d: unrecognised SA Query action frame\n", wlc->pub->unit));
	}
}


#endif /* MFP */
