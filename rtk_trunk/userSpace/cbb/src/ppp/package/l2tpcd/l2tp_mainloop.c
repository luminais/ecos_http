/*
 * L2TP main loop
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: l2tp_mainloop.c,v 1.9 2010-07-19 08:38:21 Exp $
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
#include <net/ifl2tp.h>
#include <l2tp_var.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef	__ECOS
#define NO_DRAND48
#define	srandom(a)	arc4random()
#endif


/*
 * Shutdown l2tp
 */
static void
l2tp_shutdown(struct l2tp_softc *sc)
{
	L2TP_LOG("L2TP down!!\n");

	l2tp_fsm_down(sc);
	return;
}

/* Do interface initialization */
static void
l2tp_init(struct l2tp_softc *sc, char *pppname)
{
	L2TP_LOG("L2TP init\n");

	srandom(time(0));

	memset(sc, 0, sizeof(*sc));

	strcpy(sc->param.pppname, pppname);
	sc->devfd = -1;
	sc->tunnel.sock = -1;

	/* Read l2tp parameter */
	l2tp_osl_init_param(sc);

	if (sc->param.demand == 0)
		sc->param.idle_time = 0;

	l2tp_fsm_up(sc);
	return;
}

/* 
 * The main entrance of l2tp main loop,
 * continues until shutdown happens.
 */
void l2tp_mainloop(struct l2tp_softc *sc, char *pppname)
{
	/* Update the reconnect mode */
	l2tp_init(sc, pppname);

	/* Process event */
	while (1) {
		if (sc->flag & L2TP_SHUTDOWN)
			break;

		/* Check reconnect request */
		if (sc->flag & L2TP_RECONNECT) {
			l2tp_fsm_up(sc);
			sc->flag &= ~L2TP_RECONNECT;
		}

		/* Check disconnect request */
		if (sc->flag & L2TP_DISCONNECT) {
			l2tp_fsm_down(sc);
			sc->flag &= ~L2TP_DISCONNECT;
		}

		/* Main state machine */
		l2tp_fsm_handler(sc);
	}
	l2tp_shutdown(sc);
	return;
}

/*
 * The following functions are apis for
 * l2tp_ecos.c
 */
void
l2tp_signal_reconnect(struct l2tp_softc *sc)
{
	sc->flag |= L2TP_RECONNECT;
}

void
l2tp_signal_disconnect(struct l2tp_softc *sc)
{
	sc->flag |= L2TP_DISCONNECT;
}

void
l2tp_signal_terminate(struct l2tp_softc *sc)
{
	sc->flag |= L2TP_SHUTDOWN;
}
