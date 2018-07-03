/*
 * PPTP Client
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pptp_ecos.c,v 1.9 2010-08-06 16:09:49 Exp $
 */
#include <config.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pppd.h>
#include <pptp_var.h>
#include <stdio.h>
#include <stdlib.h>
#include <netconf.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <stdlib.h>

#define	PPTP_LOCK()	cyg_scheduler_lock()
#define	PPTP_UNLOCK()	cyg_scheduler_unlock()

/* Conexts for pptp */
#define STACK_SIZE	8192

struct pptp_ctx {
	struct pptp_softc sc;

	int unit;
	char pppname[16];

	char tname[64];
	cyg_handle_t handle;
	cyg_thread thread;
	cyg_uint8 stack[STACK_SIZE];
};

static struct pptp_ctx ctx;
static struct pptp_ctx *pptp_ctx = NULL;

/*
 * The follwong functions are called by
 * thread out side the pptp thread,
 * have to special take care of the
 * context lock.
 */
static struct pptp_softc *
pptp_getsoftcbyname(char *pppname)
{
	struct pptp_softc *sc = NULL;

	if (pptp_ctx != 0 && strcmp(pptp_ctx->pppname, pppname) == 0)
		sc = &pptp_ctx->sc;

	return sc;
}

int
pptp_osl_ppp_open(struct pptp_softc *sc)
{
	struct pptp_param *param = &sc->param;
	int pid = 0;
	struct ppp_conf conf;

	/* Attach the ppp configuration structure */
	memset(&conf, 0, sizeof(conf));

	strcpy(conf.pppname, param->pppname);
	strcpy(conf.user, param->username);
	strcpy(conf.passwd, param->password);

	conf.unit           = param->unit;
	conf.mtu            = param->mtu;
	conf.mru            = param->mru;
	conf.idle           = param->idle_time;
	conf.defaultroute   = param->dflrt;
	conf.netmask        = param->netmask;
	conf.my_ip          = param->my_ip;
#ifdef MPPE
	conf.mppe           = param->mppe;
#endif
	/* Start ppp daemon */
	pid = ppp_start(&conf);
	
	return pid;
}

int
pptp_osl_ppp_close(struct pptp_softc *sc)
{
	if (sc->ppp_pid) {
		/* Stop ppp */
		ppp_stop(sc->param.pppname);
	
		/* Wait until the ppp shutdown completely */
		while (oslib_waitpid(sc->ppp_pid, NULL) != 0)
			pptp_msleep(100);

		/* Set to null pid */
		sc->ppp_pid = 0;
	}
	return 0;
}

int
pptp_osl_ppp_state(struct pptp_softc *sc)
{
	if (sc->ppp_pid && (oslib_waitpid(sc->ppp_pid, NULL) != 0))
		return 1;
	else
		return 0;
}

/* 
 * Read configuration from NVRAM to
 * pptp parameter structure.
 */
void
pptp_osl_init_param(struct pptp_softc *sc)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *value = NULL;
	struct pptp_param *param = &sc->param;
	int unit = atoi(&param->pppname[3]);

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	/* Reset all the configuration */
	param->unit = unit;
	if ((value = nvram_get(strcat_r(prefix, "pptp_username", tmp))) != NULL)
		strcpy(param->username, value);
	if ((value = nvram_get(strcat_r(prefix, "pptp_passwd", tmp))) != NULL)
		strcpy(param->password, value);

	if ((value = nvram_get(strcat_r(prefix, "pptp_server_name", tmp))) != NULL)
		strcpy(param->server_name, value);

	if ((value = nvram_get(strcat_r(prefix, "wan_hostname", tmp))) != NULL)
		strcpy(param->hostname, value);
	if ((value = nvram_get(strcat_r(prefix, "pptp_demand", tmp))) != NULL)
		param->demand = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "pptp_keepalive", tmp))) != NULL)
		param->keepalive = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "pptp_idletime", tmp))) != NULL)
		param->idle_time = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "pptp_mtu", tmp))) != NULL)
		param->mtu = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "pptp_mru", tmp))) != NULL)
		param->mru = atoi(value);
#ifdef	MPPE
	if ((value = nvram_get(strcat_r(prefix, "pptp_mppe", tmp))) != NULL)
		param->mppe = atoi(value);
#endif
	/* Set device name */
	strcpy(param->tunnel_ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	return;
}

int
pptp_osl_ifaddr(char *ifname, struct in_addr *ipaddr)
{
	int s;
	struct ifreq ifr;
	struct sockaddr_in *sin;

	s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (s < 0) {
		printf("%s:%s socket failed!\n", __FUNCTION__, ifname);
		return -1;
	}

	/* Retrieve settings */
	memset(&ifr, 0, sizeof(struct ifreq));

	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, ifname);

	/* get ip */
	if (ioctl(s, SIOCGIFADDR, &ifr) != 0) {
		printf("%s:%s ioctl(SIOCGIFADDR) failed!\n", __FUNCTION__, ifname);
	} else {
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		*ipaddr = sin->sin_addr;
	}
	close(s);
	return 0;
}

/*
 * Daemon of PPTP module.
 * This function calls the portable
 * main loop of the pptp functions.
 */
static void
pptp_main(struct pptp_ctx *ctx)
{
	struct pptp_softc *sc = &ctx->sc;

	strcpy(sc->param.pppname, ctx->pppname);
	/* Enter os independent main loop */
	pptp_mainloop(sc, ctx->pppname);

	/* Free this context */
	pptp_ctx = NULL;
	return;
}

/*
 * Functions to raise the PPTP daemon,
 * called by application main entry and
 * the mointor task.
 */
void
pptp_start(char *pppname)
{
	int unit;

	/* Already start */
	if (pptp_ctx)
		return;

	unit = atoi(pppname+3);
	if (unit < 0 || unit >= NUM_PPP)
		return;

	/* Allocate one */
	pptp_ctx = &ctx;
	memset(&pptp_ctx->sc, 0, sizeof(pptp_ctx->sc));

	strcpy(pptp_ctx->pppname, pppname);
	sprintf(pptp_ctx->tname, "pptp%d", unit);
	pptp_ctx->unit = unit;

	/* Create thread */
	cyg_thread_create(
		8,
		(cyg_thread_entry_t *)pptp_main,
		(cyg_addrword_t)pptp_ctx,
		pptp_ctx->tname,
		pptp_ctx->stack,
		sizeof(pptp_ctx->stack),
		&pptp_ctx->handle,
		&pptp_ctx->thread);
	cyg_thread_resume(pptp_ctx->handle);

	/* Make sure thread is running */
	cyg_thread_delay(10);
	return;
}

/*
 * Functions to cease the PPTP daemon,
 * called by application main entry and
 * the mointor task.
 */
void
pptp_stop(char *pppname)
{
	struct pptp_softc *sc;
	int pid;
	char tname[64];
	int unit = atoi(&pppname[3]);

	/* Signal terminate request */
	PPTP_LOCK();
	sc = pptp_getsoftcbyname(pppname);
	if (sc != 0)
		pptp_signal_terminate(sc);
	PPTP_UNLOCK();

	/* Wait pid */
	sprintf(tname, "pptp%d", unit);
	pid = oslib_getpidbyname(tname);
	if (pid != 0) {
		/* Wait until thread exit */
		while (oslib_waitpid(pid, NULL) != 0)
			cyg_thread_delay(1);
	}

	return;
}

/* External functions */
void
pptp_connect(char *pppname)
{
	struct pptp_softc *sc;

	/* Signal reconnect request */
	PPTP_LOCK();
	sc = pptp_getsoftcbyname(pppname);
	if (sc != 0)
		pptp_signal_reconnect(sc);
	PPTP_UNLOCK();
}

void
pptp_disconnect(char *pppname)
{
	struct pptp_softc *sc;

	/* Signal reconnect request */
	PPTP_LOCK();
	sc = pptp_getsoftcbyname(pppname);
	if (sc != 0)
		pptp_signal_disconnect(sc);
	PPTP_UNLOCK();
}
