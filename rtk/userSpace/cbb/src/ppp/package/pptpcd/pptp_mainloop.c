/*
 * Mainloop of the PPTP client
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pptp_mainloop.c,v 1.6 2010-07-20 02:12:33 Exp $
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/pptp_gre.h>
#include <pptp_var.h>
#include <stdio.h>

/*
 * Shutdown pptp
 */
static void
pptp_shutdown(struct pptp_softc *sc)
{
	PPTP_LOG("PPTP shutdown!!");

	pptp_fsm_down(sc);
	return;
}

/* Do interface initialization */
static void
pptp_init(struct pptp_softc *sc, char *pppname)
{
	PPTP_LOG("PPTP init");

	memset(sc, 0, sizeof(*sc));

	strcpy(sc->param.pppname, pppname);
	sc->devfd = -1;
	sc->tunnel_sock = -1;

	/* Read pptp parameter */
	pptp_osl_init_param(sc);

	if (sc->param.demand == 0)
		sc->param.idle_time = 0;

	pptp_fsm_up(sc);
	return;
}

/* 
 * The main entrance of pptp main loop,
 * continues until shutdown happens.
 */
void pptp_mainloop(struct pptp_softc *sc, char *pppname)
{
	pptp_init(sc, pppname);

	/* Process event */
	while (1) {
		if (sc->flag & PPTP_SHUTDOWN)
			break;

		/* Check reconnect request */
		if (sc->flag & PPTP_RECONNECT) {
			pptp_fsm_up(sc);
			sc->flag &= ~PPTP_RECONNECT;
		}

		/* Check disconnect request */
		if (sc->flag & PPTP_DISCONNECT) {
			pptp_fsm_down(sc);
			sc->flag &= ~PPTP_DISCONNECT;
		}

		/* Main state machine */
		pptp_fsm_handler(sc);
	}

	pptp_shutdown(sc);
	return;
}

/*
 * The following functions are apis for
 * pptp_ecos.c
 */
void
pptp_signal_reconnect(struct pptp_softc *sc)
{
	sc->flag |= PPTP_RECONNECT;
}

void
pptp_signal_disconnect(struct pptp_softc *sc)
{
	sc->flag |= PPTP_DISCONNECT;
}

void
pptp_signal_terminate(struct pptp_softc *sc)
{
	sc->flag |= PPTP_SHUTDOWN;
}
