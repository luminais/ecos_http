/*
 * PPPOE include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pppoe_var.h,v 1.3 2010-10-23 11:00:57 Exp $
 */

#ifndef __PPPOE_VAR_H__
#define __PPPOE_VAR_H__

#include <net/pppoe.h>
#include <net/if_pppoe.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	PPPOE_NAME_SIZE			256
#define PPPOE_MAX_MTU			1492

#define PPPOE_TIMEOUT_LIMIT		(16*2)
#define PPPOE_OFFER_TIMEOUT		16
#define PPPOE_INITIAL_TIMEOUT		(2)

/* Authentication failed log */
#define NUM_SUPPRESS			16
#define	PADO_NEGLECTED_MAX		16
#define	SUPPRESS_OFF			0
#define	SUPPRESS_CANDIDATE		1
#define	SUPPRESS_ON			2

/*
 * 	PPPOE structure
 */
#define IF_NAME_LEN  16

struct pppoe_param {
	int unit;
	char pppname[IF_NAME_LEN+1];
	char ethname[IF_NAME_LEN+1];
	char username[256];
	char password[256];
	char service_name[PPPOE_NAME_SIZE];
	char ac_name[PPPOE_NAME_SIZE];
	int idle_time;
	int keepalive;
	int demand;
	int mtu;
	int mru;
};

struct suppress {
	int state;
	u_char dst[6];
	time_t stime;
	int count;
};

#define	PPPOE_RECONNECT			0x0002
#define	PPPOE_DISCONNECT		0x0004
#define	PPPOE_SHUTDOWN			0x0008

/* pppoe structure */
struct pppoe_softc {
	int flag;

	/* Configuration */
	 struct pppoe_param param;
	 unsigned char pktbuf[PPPOE_MAX_MTU];

	/* FSM context */
	int dev_fd;

	int state;
	int fsm_timeout;
	time_t fsm_time;

	unsigned short sid;
	unsigned char shost[6];
	unsigned char dhost[6];

	/* Runtime data */
	char unique[PPPOE_HOST_UNIQ_LEN];
	char service_name[PPPOE_NAME_SIZE];
	char ac_name[PPPOE_NAME_SIZE];

	char peer_ac[sizeof(struct pppoe_tag) + PPPOE_NAME_SIZE];
	char peer_cookie[sizeof(struct pppoe_tag) + PPPOE_NAME_SIZE];
	char peer_sid[sizeof(struct pppoe_tag) + PPPOE_NAME_SIZE]; //wxy
	struct pppoe_tag *peer_sid_tag;
	struct pppoe_tag *peer_ac_tag;
	struct pppoe_tag *peer_cookie_tag;

	/* Authentication log */
	int ppp_pid;
	int ppp_up_time;
	struct suppress slog[NUM_SUPPRESS];
};

/*
 * FSM Functions
 */
void pppoe_fsm_timer(struct pppoe_softc *sc);
int  pppoe_fsm_up(struct pppoe_softc *sc);
void pppoe_fsm_down(struct pppoe_softc *sc);
void pppoe_fsm_handler(struct pppoe_softc *sc);

/*
 * Functions called by OSL main
 */
void pppoe_mainloop(struct pppoe_softc *sc, char *pppname);
void pppoe_signal_reconnect(struct pppoe_softc *sc);
void pppoe_signal_disconnect(struct pppoe_softc *sc);
void pppoe_signal_terminate(struct pppoe_softc *sc);

/*
 * External OSL functions declaration
 */
extern	void pppoe_osl_init_param(struct pppoe_softc *sc);
extern	int  pppoe_osl_ppp_open(struct pppoe_softc *sc);
extern	int  pppoe_osl_ppp_close(struct pppoe_softc *sc);
extern  int  pppoe_osl_ppp_state(struct pppoe_softc *sc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#if defined(linux)
#include <pppoe_linux_osl.h>
#elif defined(__ECOS)
#include <pppoe_ecos_osl.h>
#else
#error "Unsupported OSL requested"
#endif

#endif	/* __PPPOE_VAR_H__ */
