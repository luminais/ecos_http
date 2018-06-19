/*
 * wlc_auth.c -- driver-resident authenticator.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_auth.c,v 1.40.10.11 2011-01-14 14:40:09 Exp $
 */

#include <wlc_cfg.h>

#if !defined(BCMAUTH_PSK)
#error "BCMAUTH_PSK must be defined"
#endif

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <proto/eap.h>
#include <proto/eapol.h>
#include <bcmwpa.h>
#include <bcmcrypto/passhash.h>
#include <bcmcrypto/prf.h>
#include <bcmcrypto/sha1.h>
#include <proto/802.11.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlc_security.h>
#include <wlc_scb.h>
#include <wlc_wpa.h>
#include <wlc_sup.h>
#include <wlc_auth.h>
#ifdef WLBTAMP
#include <proto/802.11_bta.h>
#endif /* WLBTAMP */

#define AUTH_WPA2_RETRY		3	/* number of retry attempts */
#define AUTH_WPA2_RETRY_TIMEOUT	1000	/* 1 sec retry timeout */

/* PRF() expects to write its result sloppily. */
#define PRF_RESULT_LEN	80

#define GMK_LEN			32
#define KEY_COUNTER_LEN		32

#define AUTH_FLAG_GTK_PLUMBED	0x1		/* GTK has been plumbed into h/w */
#define AUTH_FLAG_PMK_PRESENT	0x2		/* auth->psk contains pmk */

/* GTK plumbing index values */
#define GTK_INDEX_1	1
#define GTK_INDEX_2	2

/* Toggle GTK index.  Indices 1 - 3 are usable; spec recommends 1 and 2. */
#define GTK_NEXT_INDEX(auth)	((auth)->gtk_index == GTK_INDEX_1 ? GTK_INDEX_2 : GTK_INDEX_1)

/* GTK plumbing index values */
#define IGTK_ID_1	4
#define IGTK_ID_2	5

/* Toggle GTK index.  Indices 1 - 3 are usable; spec recommends 1 and 2. */
#define IGTK_NEXT_ID(auth)	((auth)->igtk.id == IGTK_ID_1 ? IGTK_ID_2 : IGTK_ID_1)

#define BIP 1

/* Authenticator top-level structure hanging off bsscfg */
struct authenticator {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_bsscfg_t *bsscfg;		/* pointer to auth's bsscfg */
	wlc_auth_info_t *auth_info;	/* pointer to parent module */
	struct scb *scb_auth_in_prog;	/* pointer to SCB actively being authorized */

	uint16 flags;			/* operation flags */

	/* mixed-mode WPA/WPA2 is not supported */
	int auth_type;			/* authenticator discriminator */

	/* global passphrases used for cobbling pairwise keys */
	ushort psk_len;			/* len of pre-shared key */
	uchar  psk[WSEC_MAX_PSK_LEN];	/* saved pre-shared key */

	/* group key stuff */
	uint8 gtk[TKIP_KEY_SIZE];		/* group transient key */
	uint8 gtk_index;
	uint8 gtk_install_index;
	ushort gtk_len;		/* Group (mcast) key length */
	uint8 global_key_counter[KEY_COUNTER_LEN];	/* global key counter */
	uint8 initial_gkc[KEY_COUNTER_LEN];		/* initial GKC value */
	uint8 gnonce[EAPOL_WPA_KEY_NONCE_LEN];	/* AP's group key nonce */
	uint8 gmk[GMK_LEN];			/* group master key */
	uint8 gtk_rsc[8];

#ifdef MFP
	/* integrated group key stuff */
	wsec_igtk_info_t igtk;
#endif
};

/* skeletion structure since authenticator_t hangs off the bsscfg, rather than wlc */
struct wlc_auth_info {
	wlc_info_t *wlc;
	int scb_handle;			/* scb cubby for per-STA data */
};

/* scb cubby */
typedef struct scb_auth {
	uint16 flags;			/* operation flags */
	wpapsk_t *wpa;			/* volatile, initialized in set_auth */
	wpapsk_info_t *wpa_info;		/* persistent wpa related info */
} scb_auth_t;

struct auth_cubby {
	scb_auth_t *cubby;
};

/* iovar table */
enum {
	_IOV_AUTHENTICATOR_DUMMY	/* avoid empty enum */
};

static const bcm_iovar_t auth_iovars[] = {
	{NULL, 0, 0, 0, 0}
};

#define SCB_AUTH_INFO(auth, scb) ((SCB_CUBBY((scb), (auth)->scb_handle)))
#define SCB_AUTH_CUBBY(auth, scb) (((struct auth_cubby *)SCB_AUTH_INFO(auth, scb))->cubby)

#define AUTH_IN_PROGRESS(auth) (((auth)->scb_auth_in_prog != NULL))

static bool wlc_auth_wpapsk_start(authenticator_t *auth, uint8 *sup_ies, uint sup_ies_len,
                                  uint8 *auth_ies, uint auth_ies_len, struct scb *scb);
static bool wlc_wpa_auth_sendeapol(authenticator_t *auth, uint16 flags,
                                   wpa_msg_t msg, struct scb *scb);
static bool wlc_wpa_auth_recveapol(authenticator_t *auth, eapol_header_t *eapol,
                                   bool encrypted, struct scb *scb);

static int wlc_auth_insert_gtk(authenticator_t *auth, eapol_header_t *eapol, uint16 *data_len);
static void wlc_auth_gen_gtk(authenticator_t *auth, wlc_bsscfg_t *bsscfg);
static void wlc_auth_initialize_gkc(authenticator_t *auth);
static void wlc_auth_incr_gkc(authenticator_t *auth);
static void wlc_auth_initialize_gmk(authenticator_t *auth);
static int wlc_auth_set_ssid(authenticator_t *auth, uchar ssid[], int ssid_len, struct scb *scb);

#ifdef MFP
#define MFP_IGTK 1
static void wlc_auth_gen_igtk(authenticator_t *auth, wlc_bsscfg_t *bsscfg);
#endif
#ifdef MFP
static void wlc_auth_initialize_ipn(authenticator_t *auth);
#endif

/* scb stuff */
static int wlc_auth_scb_init(void *context, struct scb *scb);
static void wlc_auth_scb_deinit(void *context, struct scb *scb);
static int wlc_auth_prep_scb(authenticator_t *auth, struct scb *scb);
static void wlc_auth_cleanup_scb(wlc_info_t *wlc, struct scb *scb);

/* module stuff */
static int wlc_auth_down(void *handle);
static int wlc_auth_watchdog(void *handle);
static int wlc_auth_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
#ifdef BCMDBG
static int wlc_auth_dump(authenticator_t *auth, struct bcmstrbuf *b);
#endif /* BCMDBG */

static void wlc_auth_endassoc(authenticator_t *auth);
static bool wlc_auth_wpa_encr_gtk(authenticator_t *auth, wpapsk_t *wpa, uint type, uint8 *body,
                                  uint16 *encrypted_len);

void initialize_gmk(authenticator_t *auth);

/* Break a lengthy passhash algorithm into smaller pieces. It is necessary
 * for dongles with under-powered CPUs.
 */
static void
wlc_auth_wpa_passhash_timer(void *arg)
{
	authenticator_t *auth = (authenticator_t *)arg;
	wlc_info_t *wlc = auth->wlc;
	struct scb *scb = auth->scb_auth_in_prog;
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", wlc->pub->unit,  __FUNCTION__));
		return;
	}
	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa_info = auth_cubby->wpa_info;

	if (wpa_info == NULL) {
		WL_ERROR(("wl%d: %s: wpa_info is null\n", wlc->pub->unit,  __FUNCTION__));
		return;
	}

	if (do_passhash(&wpa_info->passhash_states, 256) == 0) {
		WL_WSEC(("wl%d: %s: Passhash is done\n", wlc->pub->unit, __FUNCTION__));
		get_passhash(&wpa_info->passhash_states, wpa_info->pmk, PMK_LEN);
		wpa_info->pmk_len = PMK_LEN;
			wlc_auth_join_complete(auth, &scb->ea, FALSE);
		return;
	}

	WL_WSEC(("wl%d: %s: passhash is in progress\n", wlc->pub->unit, __FUNCTION__));
	wl_add_timer(wlc->wl, wpa_info->passhash_timer, 0, 0);
}

static void
wlc_auth_retry_timer(void *arg)
{
	uint16 flags;
	authenticator_t *auth = (authenticator_t *)arg;
	struct scb *scb = auth->scb_auth_in_prog;
	scb_auth_t *auth_cubby;
	wpapsk_t *wpa;

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return;
	}

	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa = auth_cubby->wpa;

	if (wpa == NULL) {
		WL_ERROR(("wl%d: %s: wpa is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return;
	}

	WL_WSEC(("wl%d: retry timer fired in state %d\n",
		auth->wlc->pub->unit, wpa->state));

#ifdef MFP
	if (SCB_SHA256(scb))
		flags = WPA_KEY_DESC_V3;
	else
#endif
	flags = WSEC_TKIP_ENABLED(auth->bsscfg->wsec)? WPA_KEY_DESC_V1 : WPA_KEY_DESC_V2;
	switch (wpa->state) {
	case WPA_AUTH_PTKINITNEGOTIATING:
		if (wpa->retries++ >= AUTH_WPA2_RETRY) {
			wlc_auth_endassoc(auth);
			break;
		}
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		wlc_wpa_auth_sendeapol(auth, flags, PMSG1, scb);
		break;

	case WPA_AUTH_PTKINITDONE:
		if (wpa->retries++ >= AUTH_WPA2_RETRY) {
			wlc_auth_endassoc(auth);
			break;
		}
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		wlc_wpa_auth_sendeapol(auth, flags, PMSG3, scb);
		break;

	case WPA_AUTH_REKEYESTABLISHED:
		if (wpa->retries++ >= AUTH_WPA2_RETRY) {
			wlc_auth_endassoc(auth);
			break;
		}
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		wlc_wpa_auth_sendeapol(auth, flags, GMSG1, scb);
		break;

	default:
		break;
	}
}

/* return 0 when succeeded, 1 when passhash is in progress, -1 when failed */
int
wlc_auth_set_ssid(authenticator_t *auth, uchar ssid[], int len, struct scb* scb)
{
	wlc_info_t *wlc;
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;

	if (auth == NULL) {
		WL_WSEC(("wlc_auth_set_ssid: called with NULL auth\n"));
		return -1;
	}

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return -1;
	}

	wlc = auth->wlc;
	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa_info = auth_cubby->wpa_info;

	if (auth->psk_len == 0) {
		WL_WSEC(("%s: called with NULL psk\n", __FUNCTION__));
		return 0;
	} else if (wpa_info->pmk_len != 0) {
		WL_WSEC(("%s: called with non-NULL pmk\n", __FUNCTION__));
		return 0;
	}
	return wlc_wpa_cobble_pmk(wpa_info, (char *)auth->psk, auth->psk_len, ssid, len);
}

/* Allocate authenticator context, squirrel away the passed values,
 * and return the context handle.
 */
wlc_auth_info_t *
BCMATTACHFN(wlc_auth_attach)(wlc_info_t *wlc)
{
	wlc_auth_info_t *auth_info;

	WL_TRACE(("wl%d: wlc_auth_attach\n", wlc->pub->unit));

	if (!(auth_info = (wlc_auth_info_t *)MALLOC(wlc->osh, sizeof(wlc_auth_info_t)))) {
		WL_ERROR(("wl%d: wlc_auth_attach: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)auth_info, sizeof(wlc_auth_info_t));


	auth_info->scb_handle =
	        wlc_scb_cubby_reserve(wlc, sizeof(struct auth_cubby), wlc_auth_scb_init,
	                              wlc_auth_scb_deinit, NULL, (void*) auth_info);

	if (auth_info->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_scb_cubby_reserve() failed\n", wlc->pub->unit));
		goto err;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, auth_iovars, "auth",
		auth_info, wlc_auth_doiovar, wlc_auth_watchdog, NULL, wlc_auth_down)) {
		WL_ERROR(("wl%d: auth wlc_module_register() failed\n", wlc->pub->unit));
		goto err;
	}

	auth_info->wlc = wlc;

#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "auth", (dump_fn_t)wlc_auth_dump, (void *)auth_info);
#endif
	return auth_info;

err:
	wlc_auth_detach(auth_info);
	return NULL;
}

authenticator_t *
wlc_authenticator_attach(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	authenticator_t *auth;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	if (!(auth = (authenticator_t *)MALLOC(wlc->osh, sizeof(authenticator_t)))) {
		WL_ERROR(("wl%d: wlc_authenticator_attach: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)auth, sizeof(authenticator_t));
	auth->wlc = wlc;
	auth->bsscfg = cfg;
	auth->auth_info = wlc->authi;
	auth->gtk_index = GTK_INDEX_1;
#ifdef MFP
	auth->igtk.id = IGTK_ID_1;
#endif

	return auth;
}

/* detach the authenticator_t struct from the wlc_auth_info_t struct */
/* exists to preserve scb handle, which is needed to deinit the scbs */

void
wlc_authenticator_detach(authenticator_t *auth)
{
	if (auth == NULL)
		return;

	MFREE(auth->wlc->osh, auth, sizeof(authenticator_t));
	auth = NULL;
}

/* down and clean the authenticator structure which hangs off bsscfg */
void
wlc_authenticator_down(authenticator_t *auth)
{
	if (auth == NULL)
		return;

	wlc_key_remove_all(auth->wlc, auth->bsscfg);
	auth->flags &= ~AUTH_FLAG_GTK_PLUMBED;
	auth->gtk_index = GTK_INDEX_1;
	auth->gtk_install_index = 0;
	auth->gtk_len = 0;
#ifdef MFP
	auth->igtk.id = IGTK_ID_1;
	auth->igtk.len = 0;
	memset(auth->igtk.key, 0, sizeof(auth->igtk.key));
	auth->igtk.ipn_lo = 0;
	auth->igtk.ipn_hi = 0;
#endif
	memset(auth->gtk, 0, sizeof(auth->gtk));
	memset(auth->global_key_counter, 0, sizeof(auth->global_key_counter));
	memset(auth->initial_gkc, 0, sizeof(auth->initial_gkc));
	memset(auth->gtk_rsc, 0, sizeof(auth->gtk_rsc));
	memset(auth->gmk, 0, sizeof(auth->gmk));
	memset(auth->gnonce, 0, sizeof(auth->gnonce));
}


/* Toss authenticator context */
void
BCMATTACHFN(wlc_auth_detach)(wlc_auth_info_t *auth_info)
{
	if (!auth_info)
		return;

	WL_TRACE(("wl%d: wlc_auth_detach\n", auth_info->wlc->pub->unit));

	wlc_module_unregister(auth_info->wlc->pub, "auth", auth_info);
	MFREE(auth_info->wlc->osh, auth_info, sizeof(wlc_auth_info_t));
	auth_info = NULL;
}

/* JRK TBD */
static int
wlc_auth_scb_init(void *context, struct scb *scb)
{
	wlc_auth_info_t *auth_info = (wlc_auth_info_t *)context;
	struct auth_cubby *cubby_info = SCB_AUTH_INFO(auth_info, scb);

	ASSERT(cubby_info);

	cubby_info->cubby = (scb_auth_t *)MALLOC(auth_info->wlc->osh, sizeof(scb_auth_t));

	if (cubby_info->cubby == NULL)
		return BCME_NOMEM;

	bzero(cubby_info->cubby, sizeof(scb_auth_t));
	return BCME_OK;
}

static void
wlc_auth_scb_deinit(void *context, struct scb *scb)
{
	struct auth_cubby *cubby_info;
	int idx;
	wlc_auth_info_t *auth_info = (wlc_auth_info_t *)context;
	wlc_bsscfg_t *bsscfg;

	wlc_auth_cleanup_scb(auth_info->wlc, scb);

	FOREACH_BSS(auth_info->wlc, idx, bsscfg) {
		authenticator_t *auth = (authenticator_t *)bsscfg->authenticator;
		if (auth && auth->scb_auth_in_prog == scb)
			auth->scb_auth_in_prog = NULL;
	}

	cubby_info = SCB_AUTH_INFO(auth_info, scb);

	if ((cubby_info != NULL) && (cubby_info->cubby != NULL)) {
		MFREE(auth_info->wlc->osh, cubby_info->cubby, sizeof(scb_auth_t));
		cubby_info->cubby = NULL;
	}

	return;
}

static int
wlc_auth_down(void *handle)
{
	return 0;
}

static int
wlc_auth_watchdog(void *handle)
{
	return 0;
}

static int
wlc_auth_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	return 0;
}

int
wlc_auth_set_pmk(authenticator_t *auth, wsec_pmk_t *pmk)
{
	if (auth == NULL || pmk == NULL) {
		WL_WSEC(("wl%d: %s: missing required parameter\n",
		         auth->wlc->pub->unit, __FUNCTION__));
		return BCME_BADARG;
	}

	auth->flags &= ~AUTH_FLAG_PMK_PRESENT;

	if (pmk->flags & WSEC_PASSPHRASE) {
		if (pmk->key_len < WSEC_MIN_PSK_LEN ||
		    pmk->key_len > WSEC_MAX_PSK_LEN)
			return BCME_BADARG;
		else if (pmk->key_len == WSEC_MAX_PSK_LEN) {
			wpapsk_info_t psk_info; /* temp variable to hold the cobbled pmk */
			if (0 == wlc_wpa_cobble_pmk(&psk_info, (char *)pmk->key,
			                             pmk->key_len, NULL, 0))
			{
				bcopy(&psk_info.pmk, auth->psk, PMK_LEN);
				auth->flags |= AUTH_FLAG_PMK_PRESENT;
				return BCME_OK;
			}
		}
	}
	else if (pmk->key_len == PMK_LEN)
		auth->flags |= AUTH_FLAG_PMK_PRESENT;
	else
		return BCME_BADARG;

	bcopy((char*)pmk->key, auth->psk, pmk->key_len);
	auth->psk_len = pmk->key_len;

	return BCME_OK;
}

static bool
wlc_wpa_auth_recveapol(authenticator_t *auth, eapol_header_t *eapol, bool encrypted,
                       struct scb *scb)
{
	uint16 flags;
	wlc_info_t *wlc = auth->wlc;
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	wpapsk_t *wpa;
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;
	uint16 key_info, key_len, wpaie_len;

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return FALSE;
	}

	WL_WSEC(("wl%d: %s: received EAPOL_WPA_KEY packet.\n",
	         wlc->pub->unit, __FUNCTION__));

	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa = auth_cubby->wpa;
	wpa_info = auth_cubby->wpa_info;

	if (wpa == NULL || wpa_info == NULL) {
		WL_ERROR(("wl%d: %s: wpa or wpa_info is null\n",
			auth->wlc->pub->unit,  __FUNCTION__));
		return FALSE;
	}

#ifdef MFP
	if (SCB_SHA256(scb))
		flags = WPA_KEY_DESC_V3;
	else
#endif
	flags = WSEC_TKIP_ENABLED(auth->bsscfg->wsec)? WPA_KEY_DESC_V1 : WPA_KEY_DESC_V2;

	switch (wpa->WPA_auth) {
	case WPA2_AUTH_PSK:
	case WPA_AUTH_PSK:
		break;
	default:
		WL_ERROR(("wl%d: wlc_wpa_auth_recveapol: unexpected...\n",
			wlc->pub->unit));
		ASSERT(0);
		return FALSE;
	}

	key_info = ntoh16_ua(&body->key_info);
	key_len = ntoh16_ua(&body->key_len);

	/* check for replay */
	if (wpa_array_cmp(MAX_ARRAY, body->replay, wpa->replay, EAPOL_KEY_REPLAY_LEN) ==
	    wpa->replay) {
#ifdef BCMDBG
		uchar *g = body->replay, *s = wpa->replay;
		WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: ignoring replay "
			"(got %02x%02x%02x%02x%02x%02x%02x%02x"
			" last saw %02x%02x%02x%02x%02x%02x%02x%02x)\n",
			wlc->pub->unit,
			g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7],
			s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]));
#endif /* BCMDBG */
		return TRUE;
	}

	switch (wpa->state) {
	case WPA_AUTH_PTKINITNEGOTIATING:

		WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: processing message 2\n",
			wlc->pub->unit));

		if (((key_info & PMSG2_REQUIRED) != PMSG2_REQUIRED) ||
		    ((key_info & PMSG2_PROHIBITED) != 0)) {
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: incorrect key_info 0x%x\n",
				wlc->pub->unit, key_info));
			return TRUE;
		}

		/* Save snonce and produce PTK */
		bcopy((char *)body->nonce, wpa->snonce, sizeof(wpa->anonce));

#ifdef MFP
		if (SCB_SHA256(scb)) {
			kdf_calc_ptk(&scb->ea, &auth->bsscfg->cur_etheraddr,
				wpa->anonce, wpa->snonce, wpa_info->pmk,
				wpa_info->pmk_len, wpa->eapol_mic_key, wpa->ptk_len);
		} else
#endif
		wpa_calc_ptk(&scb->ea, &auth->bsscfg->cur_etheraddr,
			wpa->anonce, wpa->snonce, wpa_info->pmk,
		             wpa_info->pmk_len, wpa->eapol_mic_key, wpa->ptk_len);

		/* check message MIC */
		if ((key_info & WPA_KEY_MIC) &&
		    !wpa_check_mic(eapol, key_info & (WPA_KEY_DESC_V1|WPA_KEY_DESC_V2),
		                   wpa->eapol_mic_key)) {
			/* 802.11-2007 clause 8.5.3.2 - silently discard MIC failure */
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: MIC failure, discarding pkt\n",
			         wlc->pub->unit));
			return TRUE;
		}

		/* check the IE */
		wpaie_len = ntoh16_ua(&body->data_len);
		if (!wpaie_len || wpaie_len != wpa->sup_wpaie_len ||
		    bcmp(body->data, wpa->sup_wpaie, wpaie_len) != 0) {
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: wpaie does not match\n",
				wlc->pub->unit));
			wlc_wpa_senddeauth(auth->bsscfg, (char *)&scb->ea,
				DOT11_RC_WPA_IE_MISMATCH);
			return TRUE;
		}

		/* clear older timer */
		wpa->retries = 0;
		wl_del_timer(auth->wlc->wl, wpa_info->retry_timer);

		/* if MIC was okay, increment counter */
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);

		/* send msg 3 */
		wlc_wpa_auth_sendeapol(auth, flags, PMSG3, scb);

		break;

	case WPA_AUTH_PTKINITDONE:

		WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: processing message 4\n",
			wlc->pub->unit));

		if (((key_info & PMSG4_REQUIRED) != PMSG4_REQUIRED) ||
		    ((key_info & PMSG4_PROHIBITED) != 0)) {
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: incorrect key_info 0x%x\n",
				wlc->pub->unit, key_info));
			return TRUE;
		}

		/* check message MIC */
		if ((key_info & WPA_KEY_MIC) &&
		    !wpa_check_mic(eapol, key_info & (WPA_KEY_DESC_V1|WPA_KEY_DESC_V2),
		                   wpa->eapol_mic_key)) {
			/* 802.11-2007 clause 8.5.3.4 - silently discard MIC failure */
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: MIC failure, discarding pkt\n",
			         wlc->pub->unit));
			return TRUE;
		}

		/* clear older timer */
		wpa->retries = 0;
		wl_del_timer(auth->wlc->wl, wpa_info->retry_timer);

		/* Plumb paired key */
		wlc_wpa_plumb_tk(wlc, auth->bsscfg, (uint8*)wpa->temp_encr_key,
			wpa->tk_len, wpa->ucipher, &scb->ea);

		if (wpa->WPA_auth == WPA2_AUTH_PSK)
			wpa->state = WPA_AUTH_KEYUPDATE;
		else if (wpa->WPA_auth == WPA_AUTH_PSK) {
			WL_WSEC(("wl%d: %s: Moving into WPA_AUTH_REKEYNEGOTIATING\n",
			         wlc->pub->unit, __FUNCTION__));
			wpa->state = WPA_AUTH_REKEYNEGOTIATING;
			/* create the GTK */
			if (!(auth->flags & AUTH_FLAG_GTK_PLUMBED)) {
				wlc_auth_initialize_gmk(auth);
				wlc_auth_gen_gtk(auth, auth->bsscfg);
				/* GTK for WPA should use either index 1 or 2. Not zero. */
				auth->gtk_index = GTK_NEXT_INDEX(auth);
				auth->gtk_install_index =
					(uint8)wlc_wpa_plumb_gtk(wlc, auth->bsscfg,
					auth->gtk, auth->gtk_len,
					auth->gtk_index, wpa->ucipher, NULL, 1);
				if (auth->gtk_install_index == (uint8) (-1)) {
					WL_ERROR(("wl%d: %s: invalid gtk_install_index \n",
						auth->wlc->pub->unit,  __FUNCTION__));
					auth->gtk_install_index = 0;
					break;
				}

#ifdef MFP
				wlc_auth_gen_igtk(auth, auth->bsscfg);
				auth->igtk.id = IGTK_NEXT_ID(auth);
#endif
				auth->flags |= AUTH_FLAG_GTK_PLUMBED;
			}
			wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
			wlc_wpa_auth_sendeapol(auth, flags, GMSG1, scb);
		}
		if (wpa->WPA_auth == WPA2_AUTH_PSK || wpa->WPA_auth == WPA_AUTH_PSK)
			wlc_ioctl(wlc, WLC_SCB_AUTHORIZE, &scb->ea, ETHER_ADDR_LEN,
			          auth->bsscfg->wlcif);

		/* can support a new authorization now */
		if (wpa->WPA_auth == WPA2_AUTH_PSK)
			wlc_auth_endassoc(auth);
		break;

	case WPA_AUTH_REKEYESTABLISHED:

		WL_WSEC(("wl%d: %s: Processing group key message 2\n", wlc->pub->unit,
		         __FUNCTION__));

		/* check the MIC */
		if ((key_info & WPA_KEY_MIC) &&
		    !wpa_check_mic(eapol, key_info & (WPA_KEY_DESC_V1|WPA_KEY_DESC_V2),
		                   wpa->eapol_mic_key)) {
			/* 802.11-2007 clause 8.5.3.4 - silently discard MIC failure */
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: GMSG1 - MIC failure, "
			         "discarding pkt\n",
			         wlc->pub->unit));
			return TRUE;
		}

		/* clear older timer */
		wpa->retries = 0;
		wl_del_timer(auth->wlc->wl, wpa_info->retry_timer);
		wpa->state = WPA_AUTH_KEYUPDATE;
		wlc_auth_endassoc(auth);
		break;

	default:
		WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: unexpected state\n",
			wlc->pub->unit));
	}

	return TRUE;
}

static int
auth_get_group_rsc(authenticator_t *auth, uint8 *buf, int index)
{
	union {
		int index;
		uint8 rsc[EAPOL_WPA_KEY_RSC_LEN];
	} u;

	u.index = index;
	if (wlc_ioctl(auth->wlc, WLC_GET_KEY_SEQ, &u, sizeof(u), auth->bsscfg->wlcif) != 0)
		return -1;

	bcopy(u.rsc, buf, EAPOL_WPA_KEY_RSC_LEN);

	return 0;
}

static int
wlc_auth_insert_gtk(authenticator_t *auth, eapol_header_t *eapol, uint16 *data_len)
{
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	eapol_wpa2_encap_data_t *data_encap;
	uint16 len = *data_len;
	eapol_wpa2_key_gtk_encap_t *gtk_encap;

	if (auth_get_group_rsc(auth, &auth->gtk_rsc[0], auth->gtk_install_index)) {
		/* Don't use what we don't have. */
		memset(auth->gtk_rsc, 0, sizeof(auth->gtk_rsc));
	}

	/* insert GTK into eapol message */
	/*	body->key_len = htons(wpa->gtk_len); */
	/* key_len is PTK len, gtk len is implicit in encapsulation */
	data_encap = (eapol_wpa2_encap_data_t *) (body->data + len);
	data_encap->type = DOT11_MNG_PROPR_ID;
	data_encap->length = (EAPOL_WPA2_ENCAP_DATA_HDR_LEN - TLV_HDR_LEN) +
	        EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN + auth->gtk_len;
	bcopy(WPA2_OUI, data_encap->oui, DOT11_OUI_LEN);
	data_encap->subtype = WPA2_KEY_DATA_SUBTYPE_GTK;
	len += EAPOL_WPA2_ENCAP_DATA_HDR_LEN;
	gtk_encap = (eapol_wpa2_key_gtk_encap_t *) (body->data + len);
	gtk_encap->flags = (auth->gtk_index << WPA2_GTK_INDEX_SHIFT) & WPA2_GTK_INDEX_MASK;
	bcopy(auth->gtk, gtk_encap->gtk, auth->gtk_len);
	len += auth->gtk_len + EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN;

	/* copy in the gtk rsc */
	bcopy(auth->gtk_rsc, body->rsc, sizeof(body->rsc));

	/* return the adjusted data len */
	*data_len = len;

	return (auth->gtk_len + EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN + EAPOL_WPA2_ENCAP_DATA_HDR_LEN);
}

#ifdef MFP_IGTK
static int
wlc_auth_insert_igtk(authenticator_t *auth, eapol_header_t *eapol, uint16 *data_len)
{
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	eapol_wpa2_encap_data_t *data_encap;
	uint16 len = *data_len;
	eapol_wpa2_key_igtk_encap_t *igtk_encap;

	printf("%s: enter\n", __FUNCTION__);
	data_encap = (eapol_wpa2_encap_data_t *) (body->data + len);
	data_encap->type = DOT11_MNG_PROPR_ID;
	data_encap->length = (EAPOL_WPA2_ENCAP_DATA_HDR_LEN - TLV_HDR_LEN) +
	        EAPOL_WPA2_KEY_IGTK_ENCAP_HDR_LEN + auth->igtk.len;
	bcopy(WPA2_OUI, data_encap->oui, DOT11_OUI_LEN);
	data_encap->subtype = WPA2_KEY_DATA_SUBTYPE_IGTK;
	len += EAPOL_WPA2_ENCAP_DATA_HDR_LEN;
	igtk_encap = (eapol_wpa2_key_igtk_encap_t *) (body->data + len);
	igtk_encap->key_id = auth->igtk.id;
	*(uint32 *)igtk_encap->ipn = auth->igtk.ipn_lo;
	*(uint16 *)(igtk_encap->ipn + 4) = auth->igtk.ipn_hi;

	bcopy(auth->igtk.key, igtk_encap->key, auth->igtk.len);
	len += auth->igtk.len + EAPOL_WPA2_KEY_IGTK_ENCAP_HDR_LEN;

	/* return the adjusted data len */
	*data_len = len;
	printf("%s: leave igtk_len=%d\n", __FUNCTION__, auth->igtk.len);
	return (auth->igtk.len + EAPOL_WPA2_KEY_IGTK_ENCAP_HDR_LEN + EAPOL_WPA2_ENCAP_DATA_HDR_LEN);
}
#endif /* #ifdef MFP */

/* Build and send an EAPOL WPA key message */
static bool
wlc_wpa_auth_sendeapol(authenticator_t *auth, uint16 flags, wpa_msg_t msg, struct scb *scb)
{
	wlc_info_t *wlc = auth->wlc;
	wpapsk_t *wpa;
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;
	uint16 len, key_desc, data_len, buf_len;
	void *p = NULL;
	eapol_header_t *eapol_hdr = NULL;
	eapol_wpa_key_header_t *wpa_key = NULL;
	uchar mic[PRF_OUTBUF_LEN];
	bool add_mic = FALSE;
	uint16 cipher = CRYPTO_ALGO_OFF;

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return FALSE;
	}

	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa = auth_cubby->wpa;
	wpa_info = auth_cubby->wpa_info;

	len = EAPOL_HEADER_LEN + EAPOL_WPA_KEY_LEN;
	switch (msg) {
	case PMSG1:		/* pair-wise msg 1 */
		if ((p = wlc_eapol_pktget(wlc, auth->bsscfg, &scb->ea, len)) == NULL)
			break;
		eapol_hdr = (eapol_header_t *) PKTDATA(wlc->osh, p);
		eapol_hdr->length = hton16(EAPOL_WPA_KEY_LEN);
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero((char *)wpa_key, EAPOL_WPA_KEY_LEN);
		hton16_ua_store((flags | PMSG1_REQUIRED), (uint8 *)&wpa_key->key_info);
		hton16_ua_store(wpa->tk_len, (uint8 *)&wpa_key->key_len);
		wlc_getrand(wlc, wpa->anonce, EAPOL_WPA_KEY_NONCE_LEN);
		bcopy(wpa->anonce, wpa_key->nonce, EAPOL_WPA_KEY_NONCE_LEN);
		/* move to next state */
		wpa->state = WPA_AUTH_PTKINITNEGOTIATING;
		WL_WSEC(("wl%d: wlc_wpa_auth_sendeapol: sending message 1\n",
			wlc->pub->unit));
		break;

	case PMSG3:		/* pair-wise msg 3 */
		if (wpa->WPA_auth == WPA2_AUTH_PSK) {
#ifdef WLBTAMP
			if (BSS_BTA_ENAB(wlc, auth->bsscfg))
				flags |= PMSG3_BRCM_REQUIRED;
			else
#endif /* WLBTAMP */
				flags |= PMSG3_WPA2_REQUIRED;
		} else if (wpa->WPA_auth == WPA_AUTH_PSK) {
			flags |= PMSG3_REQUIRED;
		}
		else
			/* nothing else supported for now */
			ASSERT(0);


		if (WSEC_TKIP_ENABLED(auth->bsscfg->wsec))
			cipher = CRYPTO_ALGO_TKIP;
		else
			cipher = CRYPTO_ALGO_AES_CCM;

		data_len = wpa->auth_wpaie_len;
		len += data_len;
		buf_len = 0;
		if ((wpa->WPA_auth == WPA2_AUTH_PSK) && (flags & WPA_KEY_ENCRYPTED_DATA)) {
			if (!(auth->flags & AUTH_FLAG_GTK_PLUMBED)) {
				/* Cobble the key and plumb it. */
				wlc_auth_initialize_gmk(auth);
				wlc_auth_gen_gtk(auth, auth->bsscfg);
				auth->gtk_install_index =
					(uint8)wlc_wpa_plumb_gtk(wlc, auth->bsscfg,
					auth->gtk, auth->gtk_len,
					auth->gtk_index, cipher, NULL, 1);
				if (auth->gtk_install_index == (uint8) (-1)) {
					WL_ERROR(("wl%d: %s: invalid gtk_install_index \n",
						auth->wlc->pub->unit,  __FUNCTION__));
					auth->gtk_install_index = 0;
					break;
				}

				auth->flags |= AUTH_FLAG_GTK_PLUMBED;
			}
#ifdef MFP
			wlc_auth_gen_igtk(auth, auth->bsscfg);
			buf_len += auth->igtk.len + EAPOL_WPA2_KEY_IGTK_ENCAP_HDR_LEN + 8
				+ EAPOL_WPA2_ENCAP_DATA_HDR_LEN;
#endif
			/* add 8+8 for extra aes bytes and possible padding */
			buf_len += auth->gtk_len + EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN + 8
				+ EAPOL_WPA2_ENCAP_DATA_HDR_LEN;
			/* The encryption result has to be 8-byte aligned */
			if (data_len % AKW_BLOCK_LEN)
				buf_len += (AKW_BLOCK_LEN - (data_len % AKW_BLOCK_LEN));
		}

		buf_len += len;

		if ((p = wlc_eapol_pktget(wlc, auth->bsscfg, &scb->ea, buf_len)) == NULL)
			break;

		eapol_hdr = (eapol_header_t *) PKTDATA(wlc->osh, p);
		eapol_hdr->length = EAPOL_WPA_KEY_LEN;
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero((char *)wpa_key, EAPOL_WPA_KEY_LEN);

		hton16_ua_store(flags, (uint8 *)&wpa_key->key_info);
		hton16_ua_store(wpa->tk_len, (uint8 *)&wpa_key->key_len);
		bcopy(wpa->anonce, wpa_key->nonce, EAPOL_WPA_KEY_NONCE_LEN);
		bcopy((char *)wpa->auth_wpaie, (char *)wpa_key->data,
			wpa->auth_wpaie_len);
		wpa_key->data_len = hton16(data_len);
		if (wpa->WPA_auth == WPA2_AUTH_PSK && (flags & WPA_KEY_ENCRYPTED_DATA)) {
			uint8 *data, *encrkey;
			rc4_ks_t *rc4key;
			bool enc_status;

			if (!(data = MALLOC(auth->wlc->osh, WPA_KEY_DATA_LEN_256))) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				auth->wlc->pub->unit, __FUNCTION__,  MALLOCED(auth->wlc->osh)));
				break;
			}
			if (!(encrkey = MALLOC(auth->wlc->osh, WPA_MIC_KEY_LEN*2))) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				auth->wlc->pub->unit, __FUNCTION__,  MALLOCED(auth->wlc->osh)));
				MFREE(auth->wlc->osh, data, WPA_KEY_DATA_LEN_256);
				break;
			}
			if (!(rc4key = MALLOC(auth->wlc->osh, sizeof(rc4_ks_t)))) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				auth->wlc->pub->unit, __FUNCTION__,  MALLOCED(auth->wlc->osh)));
				MFREE(auth->wlc->osh, data, WPA_KEY_DATA_LEN_256);
				MFREE(auth->wlc->osh, encrkey, WPA_MIC_KEY_LEN*2);
				break;
			}


			wlc_auth_insert_gtk(auth, eapol_hdr, &data_len);
			wpa_key->data_len = hton16(data_len);

#ifdef MFP_IGTK
			wlc_auth_insert_igtk(auth, eapol_hdr, &data_len);
			wpa_key->data_len = hton16(data_len);
#endif

			/* encrypt key data field */
			if (eapol_hdr->version == 2) {
				WL_WSEC(("%s(): EAPOL hdr version is 2, set key iv to 0\n",
					__FUNCTION__));
				memset((uchar*)wpa_key->iv, 0, 16);
			}
			else
				bcopy((uchar*)&auth->global_key_counter[KEY_COUNTER_LEN-16],
					(uchar*)wpa_key->iv, 16);

			enc_status = wpa_encr_key_data(wpa_key, flags, wpa->eapol_encr_key,
				NULL, data, encrkey, rc4key);
			MFREE(auth->wlc->osh, data, WPA_KEY_DATA_LEN_256);
			MFREE(auth->wlc->osh, encrkey, WPA_MIC_KEY_LEN*2);
			MFREE(auth->wlc->osh, rc4key, sizeof(rc4_ks_t));

			if (!enc_status) {
				WL_ERROR(("wl%d: %s: error encrypting key "
				          "data\n", wlc->pub->unit, __FUNCTION__));
				break;
			}
		} /* if (flags & WPA_KEY_ENCRYPTED_DATA) */
		/* encr algorithm might change the data_len, so pick up the update */
		eapol_hdr->length += ntoh16_ua((uint8 *)&wpa_key->data_len);
		eapol_hdr->length = hton16(eapol_hdr->length);
		add_mic = TRUE;
		wpa->state = WPA_AUTH_PTKINITDONE;
		WL_WSEC(("wl%d: %s: sending message 3\n",
		         wlc->pub->unit, __FUNCTION__));
		break;

	case GMSG1: /* WPA_REKEYNEGOTIATING */ {
		int key_index;
		ASSERT(wpa->WPA_auth == WPA_AUTH_PSK);
		flags |= GMSG1_REQUIRED;
		key_index = (auth->gtk_index << WPA_KEY_INDEX_SHIFT) & WPA_KEY_INDEX_MASK;
		flags |= key_index;
		WL_ERROR(("auth->gtk_index is %u, key_index %u\n", auth->gtk_index, key_index));
		data_len = 0;
		/* make sure to pktget the EAPOL frame length plus the length of the body */
		len += auth->gtk_len;

		if (flags & WPA_KEY_DESC_V2_OR_V3)
			len += 8;

		if ((p = wlc_eapol_pktget(wlc, auth->bsscfg, &scb->ea, len)) == NULL)
			break;

		eapol_hdr = (eapol_header_t *) PKTDATA(wlc->osh, p);
		eapol_hdr->length = EAPOL_WPA_KEY_LEN;
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero((char *)wpa_key, len - EAPOL_HEADER_LEN);
		hton16_ua_store(flags, (uint8 *)&wpa_key->key_info);
		hton16_ua_store(auth->gtk_len, (uint8 *)&wpa_key->key_len);
		bcopy(auth->gnonce, wpa_key->nonce, EAPOL_WPA_KEY_NONCE_LEN);
		bcopy(&auth->global_key_counter[KEY_COUNTER_LEN-16], wpa_key->iv, 16);
		bcopy(auth->gtk_rsc, wpa_key->rsc, sizeof(wpa_key->rsc));

		wlc_auth_wpa_encr_gtk(auth, wpa, flags & (WPA_KEY_DESC_V1 | WPA_KEY_DESC_V2),
		                      wpa_key->data, &data_len);

		/* encrypt key data field */
		if (eapol_hdr->version == 2) {
			WL_WSEC(("%s(): EAPOL hdr version is 2, set key iv to 0\n",
				__FUNCTION__));
			memset((uchar*)wpa_key->iv, 0, 16);
		}
		else
			bcopy(&auth->global_key_counter[KEY_COUNTER_LEN-16], wpa_key->iv, 16);

		wpa_key->data_len = hton16(data_len);
		eapol_hdr->length += data_len;
		eapol_hdr->length = hton16(eapol_hdr->length);
		add_mic = TRUE;
		wpa->state = WPA_AUTH_REKEYESTABLISHED;
		WL_WSEC(("wl%d: wlc_wpa_auth_sendeapol: sending message G1\n",
			wlc->pub->unit));
		break;
	}

	default:
		WL_WSEC(("wl%d: %s: unexpected message type %d\n",
		         wlc->pub->unit, __FUNCTION__, msg));
		break;
	}

	if (p != NULL) {
		/* do common message fields here; make and copy MIC last. */
		eapol_hdr->type = EAPOL_KEY;
		if (wpa->WPA_auth == WPA2_AUTH_PSK)
			wpa_key->type = EAPOL_WPA2_KEY;
		else
			wpa_key->type = EAPOL_WPA_KEY;
		bcopy((char *)wpa->replay, (char *)wpa_key->replay,
		      EAPOL_KEY_REPLAY_LEN);
		key_desc = flags & (WPA_KEY_DESC_V1 |  WPA_KEY_DESC_V2);
		if (add_mic) {
			if (!wpa_make_mic(eapol_hdr, key_desc, wpa->eapol_mic_key,
				mic)) {
				WL_WSEC(("wl%d: %s: MIC generation failed\n",
				         wlc->pub->unit, __FUNCTION__));
				return FALSE;
			}
			bcopy(mic, wpa_key->mic, EAPOL_WPA_KEY_MIC_LEN);
		}



#ifdef WLBTAMP
		if (BSS_BTA_ENAB(wlc, auth->bsscfg)) {
			struct ether_header *eh;
			struct dot11_llc_snap_header *lsh;

			/* 
			 * re-encap packet w/ BT-SIG LLC/SNAP header and security prot ID:
			 * Step 1: convert Ethernet to 802.3 per 802.1H
			 * Step 2: replace RFC1042 SNAP header with BT-SIG encap header
			 * Step 3: replace ether_type with BT-SIG security prot ID
			 */
			eh = (struct ether_header *)PKTDATA(wlc->osh, p);
			wlc_ether_8023hdr(wlc, wlc->osh, eh, p);

			eh = (struct ether_header *)PKTDATA(wlc->osh, p);
			lsh = (struct dot11_llc_snap_header *)&eh[1];
			bcopy(BT_SIG_SNAP_MPROT, (char *)lsh, DOT11_LLC_SNAP_HDR_LEN - 2);

			lsh->type = hton16((uint16)BTA_PROT_SECURITY);
		}
#endif /* WLBTAMP */

		wlc_sendpkt(wlc, p, auth->bsscfg->wlcif);

		/* start the retry timer */
		wl_add_timer(wlc->wl, wpa_info->retry_timer, AUTH_WPA2_RETRY_TIMEOUT, 0);


		return TRUE;
	}
	return FALSE;
}

static bool
wlc_auth_wpapsk_start(authenticator_t *auth, uint8 *sup_ies, uint sup_ies_len,
                      uint8 *auth_ies, uint auth_ies_len, struct scb *scb)
{
	wpapsk_info_t *wpa_info;
	wpapsk_t *wpa;
	scb_auth_t *auth_cubby;
	uint16 flags;
	bool ret = TRUE;

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return FALSE;
	}

	/* find the cubby */
	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa = auth_cubby->wpa;
	wpa_info = auth_cubby->wpa_info;

	wlc_wpapsk_free(auth->wlc, wpa);

	wpa->state = WPA_AUTH_INITIALIZE;
	wpa->WPA_auth = auth->bsscfg->WPA_auth;

	if (!wlc_wpapsk_start(auth->wlc, wpa, sup_ies, sup_ies_len,
		auth_ies, auth_ies_len)) {
		WL_ERROR(("wl%d: wlc_wpapsk_start() failed\n",
		        auth->wlc->pub->unit));
		return FALSE;
	}

	if ((auth->auth_type == AUTH_WPAPSK) && (wpa_info->pmk_len == 0)) {
		WL_WSEC(("wl%d: %s: no PMK material found\n",
		         auth->wlc->pub->unit, __FUNCTION__));
		return FALSE;
	}

	/* clear older timer */
	wpa->retries = 0;
	wl_del_timer(auth->wlc->wl, wpa_info->retry_timer);

	wpa->state = WPA_AUTH_PTKSTART;
#ifdef MFP
	if (SCB_SHA256(scb))
		flags = WPA_KEY_DESC_V3;
	else
#endif
	flags = WSEC_TKIP_ENABLED(auth->bsscfg->wsec)? WPA_KEY_DESC_V1 : WPA_KEY_DESC_V2;
	wlc_wpa_auth_sendeapol(auth, flags, PMSG1, scb);

	return ret;
}

bool
wlc_set_auth(authenticator_t *auth, int auth_type, uint8 *sup_ies, uint sup_ies_len,
             uint8 *auth_ies, uint auth_ies_len, struct scb *scb)
{
	bool ret = TRUE;

	if (auth == NULL) {
		WL_WSEC(("wlc_set_auth called with NULL auth context\n"));
		return FALSE;
	}

	/* sanity */
	/* ASSERT(auth->auth_type == AUTH_UNUSED); */

	if (auth_type == AUTH_WPAPSK) {
		auth->auth_type = auth_type;
		ret = wlc_auth_wpapsk_start(auth, sup_ies, sup_ies_len, auth_ies, auth_ies_len,
		                            scb);
	} else {
		WL_ERROR(("wl%d: wlc_set_auth: unexpected auth type %d\n",
		         auth->wlc->pub->unit, auth_type));
		return FALSE;
	}
	return ret;
}

/* Dispatch EAPOL to authenticator.
 * Return boolean indicating whether it should be freed or sent up.
 */
bool
wlc_auth_eapol(authenticator_t *auth, eapol_header_t *eapol_hdr, bool encrypted, struct scb *scb)
{
	if (!auth) {
		/* no unit to report if this happens */
		WL_ERROR(("wlc_auth_eapol called with NULL auth\n"));
		return FALSE;
	}

	if ((eapol_hdr->type == EAPOL_KEY) && (auth->auth_type == AUTH_WPAPSK)) {
		eapol_wpa_key_header_t *body;

		body = (eapol_wpa_key_header_t *)eapol_hdr->body;
		if (body->type == EAPOL_WPA2_KEY || body->type == EAPOL_WPA_KEY) {
			wlc_wpa_auth_recveapol(auth, eapol_hdr, encrypted, scb);
			return TRUE;
		}
	}

	return FALSE;
}


void
wlc_auth_join_complete(authenticator_t *auth, struct ether_addr *ea, bool initialize)
{
	wlc_info_t *wlc = auth->wlc;
	wlc_bsscfg_t *bsscfg = auth->bsscfg;
	struct scb *scb =
	        wlc_scbfindband(wlc, ea, CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec));
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;
	uint auth_ies_len = BCN_TMPL_LEN;
	uint8 auth_ies[BCN_TMPL_LEN];
	bool stat = 0;

	if (!scb) {
#ifdef BCMDBG_ERR
		char eabuf[ETHER_ADDR_STR_LEN];
		WL_ERROR(("wl%d: wlc_auth_join_complete: scb not found for ea %s\n",
		          wlc->pub->unit, bcm_ether_ntoa(ea, eabuf)));
#endif
		return;
	}

	/* can only support one STA authenticating at a time */
	if (initialize && AUTH_IN_PROGRESS(auth)) {
		WL_ERROR(("wl%d: %s: Authorization blocked for current authorization in progress\n",
		         auth->wlc->pub->unit, __FUNCTION__));
		return;
	}

	if (initialize)
		wlc_auth_prep_scb(auth, scb);

	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa_info = auth_cubby->wpa_info;

	wlc_auth_initialize_gkc(auth);
#ifdef MFP
	wlc_auth_initialize_ipn(auth);
#endif

	/* init per scb WPA_auth */
	scb->WPA_auth = bsscfg->WPA_auth;

	auth->scb_auth_in_prog = scb;

	/* auth->psk is pmk */
	if (auth->flags & AUTH_FLAG_PMK_PRESENT) {
		bcopy(auth->psk, wpa_info->pmk, PMK_LEN);
		wpa_info->pmk_len = PMK_LEN;
	}

	/* kick off authenticator */
	if (wpa_info->pmk_len == PMK_LEN) {
		wlc_bcn_prb_body(wlc, FC_PROBE_RESP, SCB_BSSCFG(scb),
		                 auth_ies, (int *)&auth_ies_len, FALSE);
		stat = wlc_set_auth(auth, AUTH_WPAPSK,
		                    scb->wpaie, scb->wpaie_len,
		                    (uint8 *)auth_ies + sizeof(struct dot11_bcn_prb),
		                    auth_ies_len - sizeof(struct dot11_bcn_prb),
		                    auth->scb_auth_in_prog);
		if (!stat) {
			WL_ERROR(("wl%d: %s: 4-way handshake config problem\n",
			          wlc->pub->unit, __FUNCTION__));
		}
	}
	/* derive pmk from psk */
	else {
			wlc_auth_set_ssid(bsscfg->authenticator,
			                  (uchar *)&bsscfg->SSID, bsscfg->SSID_len, scb);
	}
}

static void
wlc_auth_gen_gtk(authenticator_t *auth, wlc_bsscfg_t *bsscfg)
{
	unsigned char data[256], prf_buff[PRF_RESULT_LEN];
	unsigned char prefix[] = "Group key expansion";
	int data_len = 0;

	/* Select a mcast cipher: only support wpa for now, otherwise change alg field */
	switch (WPA_MCAST_CIPHER(bsscfg->wsec, 0)) {
	case WPA_CIPHER_TKIP:
		WL_WSEC(("%s: TKIP\n", __FUNCTION__));
		auth->gtk_len = TKIP_TK_LEN;
		break;
	case WPA_CIPHER_AES_CCM:
		WL_WSEC(("%s: AES\n",  __FUNCTION__));
		auth->gtk_len = AES_TK_LEN;
		break;
	default:
		WL_WSEC(("%s: not supported multicast cipher\n", __FUNCTION__));
		return;
	}
	WL_WSEC(("%s: gtk_len %d\n", __FUNCTION__, auth->gtk_len));

	/* create the the data portion */
	bcopy((char*)&bsscfg->cur_etheraddr, (char*)&data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy(auth->global_key_counter, auth->gnonce, EAPOL_WPA_KEY_NONCE_LEN);
	wlc_auth_incr_gkc(auth);
	bcopy((char*)&auth->gnonce, (char*)&data[data_len], EAPOL_WPA_KEY_NONCE_LEN);
	data_len += EAPOL_WPA_KEY_NONCE_LEN;

	/* generate the GTK */
	fPRF(auth->gmk, sizeof(auth->gmk), prefix, strlen((char *)prefix),
	    data, data_len, prf_buff, auth->gtk_len);
	memcpy(auth->gtk, prf_buff, auth->gtk_len);

	/* The driver clears the IV when it gets a new key, so
	 * clearing RSC should be consistent with that, right?
	 */
	memset(auth->gtk_rsc, 0, sizeof(auth->gtk_rsc));

	WL_WSEC(("%s: done\n", __FUNCTION__));
}


/* generate the initial global_key_counter */
static void
wlc_auth_initialize_gkc(authenticator_t *auth)
{
	wlc_info_t *wlc = auth->wlc;
	unsigned char buff[32], prf_buff[PRF_RESULT_LEN];
	unsigned char prefix[] = "Init Counter";

	wlc_getrand(wlc, &buff[0], 16);
	wlc_getrand(wlc, &buff[16], 16);

	/* Still not exactly right, but better. */
	fPRF(buff, sizeof(buff), prefix, strlen((char *)prefix),
	    (unsigned char *) &auth->bsscfg->cur_etheraddr, ETHER_ADDR_LEN,
	    prf_buff, KEY_COUNTER_LEN);
	memcpy(auth->global_key_counter, prf_buff, KEY_COUNTER_LEN);
	memcpy(auth->initial_gkc, auth->global_key_counter, KEY_COUNTER_LEN);
}

static void
wlc_auth_incr_gkc(authenticator_t *auth)
{
	wpa_incr_array(auth->global_key_counter, KEY_COUNTER_LEN);

	/* if key counter is now equal to the original one, reset it */
	if (!bcmp(auth->global_key_counter, auth->initial_gkc, KEY_COUNTER_LEN))
		wlc_auth_initialize_gmk(auth);
}

static void
wlc_auth_initialize_gmk(authenticator_t *auth)
{
	wlc_info_t *wlc = auth->wlc;
	unsigned char *gmk = (unsigned char *)auth->gmk;

	wlc_getrand(wlc, &gmk[0], 16);
	wlc_getrand(wlc, &gmk[16], 16);
}

#ifdef MFP
static void
wlc_auth_gen_igtk(authenticator_t *auth, wlc_bsscfg_t *bsscfg)
{
	unsigned char data[256], prf_buff[PRF_RESULT_LEN];
	unsigned char prefix[] = "Group key expansion";
	int data_len = 0;

	printf("%s: enter\n", __FUNCTION__);
	auth->igtk.len = AES_TK_LEN;
	WL_WSEC(("%s: igtk_len %d\n", __FUNCTION__, auth->igtk.len));

	/* create the the data portion */
	bcopy((char*)&bsscfg->cur_etheraddr, (char*)&data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	*(uint32 *)&data[data_len] = auth->igtk.ipn_lo;
	*(uint16 *)&data[data_len+sizeof(uint32)] = auth->igtk.ipn_hi;
	data_len += 6;
	auth->igtk.ipn_lo++;
	if (auth->igtk.ipn_lo == 0)
		auth->igtk.ipn_hi++;

	/* generate the GTK */
	fPRF(auth->gmk, sizeof(auth->gmk), prefix, strlen((char *)prefix),
	    data, data_len, prf_buff, AES_TK_LEN);
	memcpy(auth->igtk.key, prf_buff, AES_TK_LEN);
	/* assign to bsscfg */
	memcpy(&bsscfg->igtk, &auth->igtk, sizeof(wsec_igtk_info_t));

	WL_WSEC(("%s: done\n", __FUNCTION__));
}


/* generate the initial global_key_counter */
static void
wlc_auth_initialize_ipn(authenticator_t *auth)
{
	wlc_info_t *wlc = auth->wlc;
	unsigned char buff[32], prf_buff[PRF_RESULT_LEN];
	unsigned char prefix[] = "Init Counter";

	printf("%s: enter\n", __FUNCTION__);
	wlc_getrand(wlc, &buff[0], 16);
	wlc_getrand(wlc, &buff[16], 16);

	/* Still not exactly right, but better. */
	fPRF(buff, sizeof(buff), prefix, strlen((char *)prefix),
	    (unsigned char *) &auth->bsscfg->cur_etheraddr, ETHER_ADDR_LEN,
	    prf_buff, KEY_COUNTER_LEN);
	memcpy(&auth->igtk.ipn_lo, prf_buff, sizeof(uint32));
	memcpy(&auth->igtk.ipn_hi, prf_buff+sizeof(uint32), sizeof(uint16));
	WL_WSEC(("%s: done\n", __FUNCTION__));
}

#endif /* MFP */

#ifdef BCMDBG
static int
wlc_auth_dump(authenticator_t *auth, struct bcmstrbuf *b)
{
	return 0;
}
#endif /* BCMDBG */

static int
wlc_auth_prep_scb(authenticator_t *auth, struct scb *scb)
{
	wlc_auth_info_t *auth_info = auth->auth_info;
	struct auth_cubby *cubby_info = SCB_AUTH_INFO(auth_info, scb);

	if (!(cubby_info->cubby->wpa = MALLOC(auth->wlc->osh, sizeof(wpapsk_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          auth->wlc->pub->unit, __FUNCTION__,  MALLOCED(auth->wlc->osh)));
		goto err;
	}
	bzero(cubby_info->cubby->wpa, sizeof(wpapsk_t));

	if (!(cubby_info->cubby->wpa_info = MALLOC(auth->wlc->osh, sizeof(wpapsk_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          auth->wlc->pub->unit, __FUNCTION__, MALLOCED(auth->wlc->osh)));
		goto err;
	}
	bzero(cubby_info->cubby->wpa_info, sizeof(wpapsk_info_t));
	cubby_info->cubby->wpa_info->wlc = auth->wlc;

	if (!(cubby_info->cubby->wpa_info->passhash_timer =
	      wl_init_timer(auth->wlc->wl, wlc_auth_wpa_passhash_timer, auth,
	                    "passhash"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for passhash timer "
		          "failed\n", auth->wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	if (!(cubby_info->cubby->wpa_info->retry_timer =
	      wl_init_timer(auth->wlc->wl, wlc_auth_retry_timer, auth, "auth_retry"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for retry timer failed\n",
		          auth->wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	return BCME_OK;

err:
	if (cubby_info->cubby->wpa_info) {
		if (cubby_info->cubby->wpa_info->passhash_timer) {
			wl_free_timer(auth->wlc->wl, cubby_info->cubby->wpa_info->passhash_timer);
			cubby_info->cubby->wpa_info->passhash_timer = NULL;
		}
		if (cubby_info->cubby->wpa_info->retry_timer) {
			wl_free_timer(auth->wlc->wl, cubby_info->cubby->wpa_info->retry_timer);
			cubby_info->cubby->wpa_info->retry_timer = NULL;
		}
		MFREE(auth->wlc->osh, cubby_info->cubby->wpa_info, sizeof(wpapsk_info_t));
		cubby_info->cubby->wpa_info = NULL;
	}

	if (cubby_info->cubby->wpa) {
		MFREE(auth->wlc->osh, cubby_info->cubby->wpa, sizeof(wpapsk_t));
		cubby_info->cubby->wpa = NULL;
	}
	return BCME_NOMEM;
}

static void
wlc_auth_endassoc(authenticator_t *auth)
{
	WL_TRACE(("Wl%d: %s: ENTER\n", auth->wlc->pub->unit, __FUNCTION__));

	if (!auth->scb_auth_in_prog)
		return;

	wlc_auth_cleanup_scb(auth->wlc, auth->scb_auth_in_prog);
	auth->scb_auth_in_prog = NULL;
}

static void
wlc_auth_cleanup_scb(wlc_info_t *wlc, struct scb *scb)
{
	struct auth_cubby *cubby_info;

	WL_TRACE(("wl%d: %s: Freeing SCB data at 0x%p\n", wlc->pub->unit, __FUNCTION__, scb));
	cubby_info = SCB_AUTH_INFO(wlc->authi, scb);

	if ((cubby_info == NULL) || (cubby_info->cubby == NULL))
		return;

	if (cubby_info->cubby->wpa) {
		wlc_wpapsk_free(wlc, cubby_info->cubby->wpa);
		MFREE(wlc->osh, cubby_info->cubby->wpa, sizeof(wpapsk_t));
		cubby_info->cubby->wpa = NULL;
	}

	if (cubby_info->cubby->wpa_info) {
		if (cubby_info->cubby->wpa_info->passhash_timer) {
			wl_free_timer(wlc->wl, cubby_info->cubby->wpa_info->passhash_timer);
			cubby_info->cubby->wpa_info->passhash_timer = NULL;
		}
		if (cubby_info->cubby->wpa_info->retry_timer) {
			wl_free_timer(wlc->wl, cubby_info->cubby->wpa_info->retry_timer);
			cubby_info->cubby->wpa_info->retry_timer = NULL;
		}
		MFREE(wlc->osh, cubby_info->cubby->wpa_info, sizeof(wpapsk_info_t));
		cubby_info->cubby->wpa_info = NULL;
	}
}

static bool
wlc_auth_wpa_encr_gtk(authenticator_t *auth, wpapsk_t *wpa, uint type, uint8 *body,
                      uint16 *encrypted_len)
{
	unsigned char *data = NULL, *encrkey = NULL;
	rc4_ks_t* rc4key = NULL;
	uint16 len = auth->gtk_len;
	bool ret = TRUE;
	if (((encrkey = (unsigned char *)MALLOC(auth->wlc->osh, 32 * sizeof(char))) == NULL) ||
	    ((data = (unsigned char *)MALLOC(auth->wlc->osh, 256 * sizeof(char))) == NULL) ||
	    ((rc4key = (rc4_ks_t *)MALLOC(auth->wlc->osh, sizeof(rc4_ks_t))) == NULL)) {
		ret = FALSE;
		goto err;
	}

	/* encrypt the gtk using RC4 */
	switch (type) {
	case WPA_KEY_DESC_V1:
		/* create the iv/ptk key */
		bcopy(&auth->global_key_counter[KEY_COUNTER_LEN-16], encrkey, 16);
		bcopy(wpa->eapol_encr_key, &encrkey[16], 16);

		/* copy the gtk into the encryption buffer */
		bcopy(auth->gtk, body, len);
		/* encrypt the gtk using RC4 */
		prepare_key(encrkey, 32, rc4key);
		rc4(data, 256, rc4key); /* dump 256 bytes */
		rc4(body, len, rc4key);
		break;
	case WPA_KEY_DESC_V2:
	case WPA_KEY_DESC_V3:
		if (auth->gtk_len < 16) {
			bzero(&auth->gtk[len], 16 - len);
			len = 16;
		} else if (len % AKW_BLOCK_LEN) {
			bzero(&auth->gtk[len], AKW_BLOCK_LEN - (len % AKW_BLOCK_LEN));
			len += AKW_BLOCK_LEN - (len % AKW_BLOCK_LEN);
		}
		if (aes_wrap(sizeof(wpa->eapol_encr_key), wpa->eapol_encr_key,
		             len, auth->gtk, body)) {
			WL_ERROR(("wl%d: %s: encrypt failed\n", auth->wlc->pub->unit,
			          __FUNCTION__));
			ret = FALSE;
			goto err;
		}

		/* adjust length here ?? this is taken from NAS, but why? */
		len += 8;
		break;
	default:
		WL_ERROR(("wl%d: %s: Unknown WPA key state %u\n", auth->wlc->pub->unit,
		          __FUNCTION__, type));
		ret = FALSE;
		goto err;
	}

	/* tell the calling func how long the encrypted data is */
	*encrypted_len = len;
err:
	if (data !=  NULL)
		MFREE(auth->wlc->osh, data, 256 * sizeof(char));
	if (encrkey != NULL)
		MFREE(auth->wlc->osh, encrkey, 32 * sizeof(char));
	if (rc4key != NULL)
		MFREE(auth->wlc->osh, rc4key, sizeof(rc4_ks_t));
	return ret;
}
