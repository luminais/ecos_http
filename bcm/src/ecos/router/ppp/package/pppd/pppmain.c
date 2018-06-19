/*
 * pppmain.c, Modified from ppp main.c
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: pppmain.c,v 1.3 2010-07-19 08:34:32 Exp $
 */
/*
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <net/ppp_defs.h>
#include <net/ppp-comp.h>
#include <net/if_ppp.h>
#include "pppd.h"
#include "fsm.h"
#include "lcp.h"
#include "chap-new.h"
#include "upap.h"
#include "ipcp.h"
#include "ccp.h"
#include "magic.h"

#include <arpa/inet.h>

#include <syslog.h>
#include <ecos_oslib.h>
#include <stdlib.h>

#include <bcmnvram.h>

#define PPP_FLAG_HUNGUP		0x01	/* Hungup */
#define PPP_FLAG_INTERRUPT	0x02	/* INTERRUPT */
#define PPP_FLAG_TERMINATE	0x04	/* Terminate */
#define PPP_FLAG_TIMEOUT	0x08	/* Timeout */
#define PPP_FLAG_INPUT		0x10	/* Input available */
#define PPP_FLAG_ADJ_IDLE	0x20	/* Adjust idle time-out */

#define	MINUTE_TICKS		(1*100)	/* 1 second */
#define PPP_MSG_WAIT_TIME	(cyg_current_time()+MINUTE_TICKS)

struct	callout {
    struct timeval c_time;		/* time at which to call routine */
    void *c_arg;			/* argument to routine */
    void (*c_func) __P((void *));	/* routine */
    struct callout *c_next;
};

/* prototypes */
static void calltimeout(struct ppp *sc);
static void get_input(struct ppp *sc);
static void cleanup(struct ppp *sc);
static void cleanup_db(struct ppp *sc);

static int ppp_flag[NUM_PPP];


/*
 * PPP Data Link Layer "protocol" table.
 * One entry per supported protocol.
 * The last entry must be NULL.
 */
struct protent *protocols[] = {
	&lcp_protent,
	&pap_protent,
	&chap_protent,
	&ipcp_protent,
#ifdef PPP_COMPRESS
	&ccp_protent,
#endif
	NULL
};

#define PPPBUFSIZE		(PPP_MTU * 2)

/* Do initialization */
static void
ppp_init(struct ppp *sc)
{
	int i = 0;
	struct protent *protp;

	/* Setup IFNAME env variable */
	script_setenv("IFNAME", sc->pppname, 0);

	new_phase(PHASE_INITIALIZE);

	/* Open raw socket */
	ppp_flag[sc->unit] = 0;

	if (establish_ppp(sc->unit) != 0) {
		syslog(LOG_USER|LOG_INFO, "Establish ppp io failed!");
		goto errout;
	}

	sc->callout = 0;

	/* PPP ifup */
	sifup(sc->unit);

	/*
	 * Initialize magic number generator now so that protocols may
	 * use magic numbers in initialization.
	 */
	magic_init();

	/*
	 * Initialize each protocol.
	 */
	for (i = 0; (protp = protocols[i]) != NULL; ++i)
		(*protp->init)(sc->unit);

	/*
	 * Initialize options
	 */
	init_options(sc->unit);

	auth_check_options();

	/*
	 * Create evnet group, start opening the connection, and  wait for
	 * incoming event (reply, timeout, etc.).
	 */
	syslog(LOG_USER|LOG_INFO, "start lcp stage");

	lcp_lowerup(sc->unit);
	lcp_open(sc->unit);		/* Start protocol */

	new_phase(PHASE_ESTABLISH);
	return;

errout:
	ppp_flag[sc->unit] = PPP_FLAG_TERMINATE;
	return;
}


/* 
 * The main entrance of ppp main loop,
 * continues until shutdown happens.
 */
void
ppp_mainloop(struct ppp *sc)
{
	ppp_init(sc);
	
	while (sc->Phase != PHASE_DEAD) {
		/* Handle events */
		switch (ppp_flag[sc->unit]) {
		case PPP_FLAG_TERMINATE:
			goto shutdown;

		default:
			break;
		}

		calltimeout(sc);

		get_input(sc);
	}

shutdown:
	cleanup(sc);
}

/*
 * Get input for network protocol
 */
static void
get_input(struct ppp *sc)
{
	int len, i;
	u_char *p;
	u_short protocol;
	struct protent *protp;

	/* point to beginning of packet buffer */
	p = sc->inpacket_buf;

	len = read_packet(sc->unit, sc->inpacket_buf);
	if (len < 0)
		return;

	if (len < PPP_HDRLEN) {
		MAINDEBUG(("io(): Received short packet."));
		return;
	}

	/* Skip address and control */
	p += 2;
	GETSHORT(protocol, p);
	len -= PPP_HDRLEN;

	
	if(sc->unit == 1)//gong
	{
		lcp_fsm[sc->unit].state = OPENED;//直接略过前面几个链路协商过程??
	}
		
	/*
	 * Toss all non-LCP packets unless LCP is OPEN.
	 */
	if (protocol != PPP_LCP && lcp_fsm[sc->unit].state != OPENED) {
		MAINDEBUG(("get_input: Received non-LCP packet when LCP not open."));
		return;
	}

	/*
	 * Until we get past the authentication phase, toss all packets
	 * except LCP, LQR and authentication packets.
	 */
	if (sc->Phase <= PHASE_AUTHENTICATE &&
	    !(protocol == PPP_LCP || protocol == PPP_LQR ||
	    protocol == PPP_PAP || protocol == PPP_CHAP)) {
		/* Print debug message */
		MAINDEBUG(("get_input: discarding proto 0x%x in phase %d",
			protocol, sc->Phase));
		return;
	}

	/*
	 * Upcall the proper protocol input routine.
	 */
	for (i = 0; (protp = protocols[i]) != NULL; ++i) {		
		if (protp->protocol == protocol && protp->enabled_flag) {
			(*protp->input)(sc->unit, p, len);
			return;
		}
	}

	/* Send protocol reject */
	lcp_sprotrej(sc->unit, p - PPP_HDRLEN, len + PPP_HDRLEN);
	return;
}

/*
 * die - like quit, clean up state and 
 * exit with the specified status..
 */
void
die(int unit)
{
	ppp_flag[unit] = PPP_FLAG_TERMINATE;
}

/*
 * cleanup - restore anything which needs to be restored before we exit
 */
static void
cleanup(struct ppp *sc)
{
	struct callout **copp, *freep;

	MAINDEBUG((" Exiting."));

	lcp_close(sc->unit, "User request");	/* Close connection */

	sifdown(sc->unit);

	/* Free the time lists before we deleting timer */
	for (copp = &(sc->callout); *copp;) {
		freep = *copp;
		*copp = freep->c_next;
		(void) free((char *) freep);
	}

	sc->callout = NULL;

	/* Disestablish ppp device */
	disestablish_ppp(sc->unit);

	/* Do env script cleanup */
	cleanup_db(sc);
}

/*
 * timeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of milliseconds, NOT hz (as in
 * the kernel).
 */
void
ppp_timeout(func, arg, secs, usecs)
	void (*func) __P((void *));
	void *arg;
	int secs, usecs;
{
	struct ppp *sc = pppsc();
	struct callout *newp, *p, **pp;
	struct timeval timenow;		/* Current time */

	/*
	 * Allocate timeout.
	 */
	if ((newp = (struct callout *) malloc(sizeof(struct callout))) == NULL) {
		MAINDEBUG(("Out of memory in timeout()!"));
		ppp_flag[sc->unit] = PPP_FLAG_TERMINATE;
		return;
	}

	newp->c_arg = arg;
	newp->c_func = func;
	gettimeofday(&timenow, NULL);
	newp->c_time.tv_sec = timenow.tv_sec + secs;
	newp->c_time.tv_usec = timenow.tv_usec + usecs;
	if (newp->c_time.tv_usec >= 1000000) {
		newp->c_time.tv_sec += newp->c_time.tv_usec / 1000000;
		newp->c_time.tv_usec %= 1000000;
	}

	/*
	 * Find correct place and link it in.
	 */
	for (pp = &sc->callout; (p = *pp); pp = &p->c_next) {
		if (newp->c_time.tv_sec < p->c_time.tv_sec ||
		    (newp->c_time.tv_sec == p->c_time.tv_sec &&
		    newp->c_time.tv_usec < p->c_time.tv_usec))
			break;
	}

	newp->c_next = p;
	*pp = newp;
}


/*
 * untimeout - Unschedule a timeout.
 */
void
ppp_untimeout(func, arg)
	void (*func) __P((void *));
	void *arg;
{
	struct ppp *sc = pppsc();
	struct callout **copp, *freep;

	/*
	 * Find first matching timeout and remove it from the list.
	 */
	for (copp = &sc->callout; (freep = *copp); copp = &freep->c_next)
		if (freep->c_func == func && freep->c_arg == arg) {
			*copp = freep->c_next;
			free((char *) freep);
			break;
	}
}

/*
 * calltimeout - Call any timeout routines which are now due.
 */
static void
calltimeout(sc)
	struct ppp *sc;
{
	struct callout *p;
	struct timeval timenow;		/* Current time */

	while (sc->callout != NULL) {
		p = sc->callout;

		gettimeofday(&timenow, NULL);

		if (!(p->c_time.tv_sec < timenow.tv_sec ||
		    (p->c_time.tv_sec == timenow.tv_sec &&
		    p->c_time.tv_usec <= timenow.tv_usec))) {
			/* no, it's not time yet */
			break;
		}

		sc->callout = p->c_next;
		(*p->c_func)(p->c_arg);

		free((char *)p);
	}
}

/*
 * novm - log an error message saying we ran out of memory, and die.
 */
void
novm(msg)
	char *msg;
{
	fatal("Virtual memory exhausted allocating %s\n", msg);
}

/*
 * script_setenv - set an environment variable value to be used
 * for scripts that we run (e.g. ip-up, auth-up, etc.)
 */
void
script_setenv(var, value, iskey)
	char *var, *value;
	int iskey;
{
	struct ppp *sc = pppsc();

	size_t varl = strlen(var);
	size_t vl = varl + strlen(value) + 2;
	int i;
	char *p, *newstring;

	newstring = (char *)malloc(vl);
	if (newstring == 0)
		return;

	snprintf(newstring, vl, "%s=%s", var, value);

	/* check if this variable is already set */
	if (sc->script_env != 0) {
		for (i = 0; (p = sc->script_env[i]) != 0; ++i) {
			if (strncmp(p, var, varl) == 0 && p[varl] == '=') {
				free(p);
				sc->script_env[i] = newstring;
				return;
			}
		}
	}
	else {
		/* no space allocated for script env. ptrs. yet */
		i = 0;
		sc->script_env = (char **)malloc(16 * sizeof(char *));
		if (sc->script_env == NULL)
			return;

		sc->s_env_nalloc = 16;
	}

	/* reallocate script_env with more space if needed */
	if (i + 1 >= sc->s_env_nalloc) {
		int new_n = i + 17;
		char **newenv = (char **)realloc((void *)sc->script_env, new_n * sizeof(char *));
		if (newenv == 0)
			return;

		sc->script_env = newenv;
		sc->s_env_nalloc = new_n;
	}

	/* Update script env */
	sc->script_env[i] = newstring;
	sc->script_env[i+1] = 0;
}

/*
 * script_unsetenv - remove a variable from the environment
 * for scripts.
 */
void
script_unsetenv(var)
	char *var;
{
	struct ppp *sc = pppsc();

	int vl = strlen(var);
	int i;
	char *p;

	if (sc->script_env == 0)
		return;
	for (i = 0; (p = sc->script_env[i]) != 0; ++i) {
		if (strncmp(p, var, vl) == 0 && p[vl] == '=') {
			free(p);
			while ((sc->script_env[i] = sc->script_env[i+1]) != 0)
				++i;
			break;
		}
	}
}

static void
cleanup_db(struct ppp *sc)
{
	int i;
	char *p;

	if (sc->script_env == 0)
		return;
	for (i = 0; (p = sc->script_env[i]) != 0; ++i)
		free(p);
}

/*
 * update_link_stats - get stats at link termination.
 */
void
update_link_stats(u)
	int u;
{
}

void
new_phase(p)
    int p;
{
	struct ppp *sc = pppsc();

	/* Change to new state */    
	sc->Phase = p;
}
