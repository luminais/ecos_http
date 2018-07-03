/*
 * L2TP ecos osl main
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: l2tp_ecos.c,v 1.11 2010-08-06 16:10:08 Exp $
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <pppd.h>
#include <l2tp_var.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netconf.h>
#include <shutils.h>
#include <bcmnvram.h>

/* Conexts for l2tp */
#define STACK_SIZE	24*1024

struct l2tp_ctx {
	struct l2tp_softc sc;

	int unit;
	char pppname[16];

	char tname[64];
	cyg_handle_t handle;
	cyg_thread thread;
	cyg_uint8 stack[STACK_SIZE];
};

static struct l2tp_ctx ctx;
static struct l2tp_ctx *l2tp_ctx = NULL;


/*
 * The follwong functions are called by
 * thread out side the l2tp thread,
 * have to special take care of the
 * context lock.
 */
static struct l2tp_softc *
l2tp_getsoftcbyname(char *pppname)
{
	struct l2tp_softc *sc = NULL;

	if (l2tp_ctx != 0 && strcmp(l2tp_ctx->pppname, pppname) == 0)
		sc = &l2tp_ctx->sc;

	return sc;
}

int
l2tp_osl_ppp_open(struct l2tp_softc *sc)
{
	struct l2tp_param *param = &sc->param;
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
#ifdef MPPE
	conf.mppe           = param->mppe;
#endif
	/* Start ppp daemon */
	pid = ppp_start(&conf);
	return pid;
}

int
l2tp_osl_ppp_close(struct l2tp_softc *sc)
{
	if (sc->ppp_pid) {
		/* Stop ppp */
		ppp_stop(sc->param.pppname);

		/* Wait until the ppp shutdown completely */
		while (oslib_waitpid(sc->ppp_pid, NULL) != 0)
			l2tp_msleep(100);

		/* Set to null pid */
		sc->ppp_pid = 0;
	}

	return 0;
}

int
l2tp_osl_ppp_state(struct l2tp_softc *sc)
{
	if (sc->ppp_pid && (oslib_waitpid(sc->ppp_pid, NULL) != 0))
		return 1;
	else
		return 0;
}

int
l2tp_osl_ifaddr(char *ifname, struct in_addr *ipaddr)
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
	if (ioctl(s, SIOCGIFADDR, &ifr) != 0)
		printf("%s:%s ioctl(SIOCGIFADDR) failed!\n", __FUNCTION__, ifname);
	else {
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		*ipaddr = sin->sin_addr;
	}
	close(s);
	return 0;
}

/* 
 * Read configuration from NVRAM to
 * l2tp parameter structure.
 */
void
l2tp_osl_init_param(struct l2tp_softc *sc)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *value = NULL;
	struct l2tp_param *param = &sc->param;
	int unit = atoi(&param->pppname[3]);

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	/* Reset all the configuration */
	param->unit = unit;
	if ((value = nvram_get(strcat_r(prefix, "l2tp_username", tmp))) != NULL)
		strcpy(param->username, value);
	if ((value = nvram_get(strcat_r(prefix, "l2tp_passwd", tmp))) != NULL)
		strcpy(param->password, value);
	if ((value = nvram_get(strcat_r(prefix, "wan_hostname", tmp))) != NULL)
		strcpy(param->hostname, value);
	if ((value = nvram_get(strcat_r(prefix, "l2tp_demand", tmp))) != NULL)
		param->demand = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "l2tp_keepalive", tmp))) != NULL)
		param->keepalive = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "l2tp_idletime", tmp))) != NULL)
		param->idle_time = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "l2tp_mtu", tmp))) != NULL)
		param->mtu = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "l2tp_mru", tmp))) != NULL)
		param->mru = atoi(value);

	if ((value = nvram_get(strcat_r(prefix, "l2tp_server_name", tmp))) != NULL)
		strcpy(param->server_name, value);
#ifdef	MPPE
	if ((value = nvram_get(strcat_r(prefix, "l2tp_mppe", tmp))) != NULL)
		param->mppe = atoi(value);
#endif

	/* Do fixed up */
	if (param->mtu < 512 || param->mtu > 1452)
		param->mtu = 1452;
	/* Do fixed up */
	if (param->mru < 512 || param->mru > 1452)
		param->mru = 1452;

	/* Set device name */
	if ((value = nvram_get(strcat_r(prefix, "ifname", tmp))) != NULL)	//hqe add coverity
		strcpy(param->tunnel_ifname, value);
	return;
}

/*
 * Daemon of L2TP module.
 * This function calls the portable
 * main loop of the l2tp functions.
 */
static void
l2tp_main(struct l2tp_ctx *ctx)
{
	struct l2tp_softc *sc = &ctx->sc;

	/* Enter os independent main loop */
	l2tp_mainloop(sc, ctx->pppname);

	/* Free this context */
	l2tp_ctx = NULL;
	return;
}

/*
 * Functions to raise the L2TP daemon,
 * called by application main entry and
 * the mointor task.
 */
void
l2tp_start(char *pppname)
{
	int unit;

	if (l2tp_ctx)
		return;

	unit = atoi(pppname+3);
	if (unit < 0 || unit >= NUM_PPP)
		return;

	/* Allocate one */
	l2tp_ctx = &ctx;
	memset(&l2tp_ctx->sc, 0, sizeof(l2tp_ctx->sc));

	/* Setup data */
	strcpy(l2tp_ctx->pppname, pppname);
	sprintf(l2tp_ctx->tname, "l2tp%d", unit);
	l2tp_ctx->unit = unit;

	/* Create thread */
	cyg_thread_create(
		8,
		(cyg_thread_entry_t *)l2tp_main,
		(cyg_addrword_t)l2tp_ctx,
		l2tp_ctx->tname,
		l2tp_ctx->stack,
		sizeof(l2tp_ctx->stack),
		&l2tp_ctx->handle,
		&l2tp_ctx->thread);
	cyg_thread_resume(l2tp_ctx->handle);

	/* Make sure thread is running */
	cyg_thread_delay(10);
	return;
}

/*
 * Functions to cease the L2TP daemon,
 * called by application main entry and
 * the mointor task.
 */
void
l2tp_stop(char *pppname)
{
	struct l2tp_softc *sc;
	int pid;
	char tname[64];
	int unit = atoi(&pppname[3]);

	/* Signal terminate request */
	sc = l2tp_getsoftcbyname(pppname);
	if (sc != 0)
		l2tp_signal_terminate(sc);

	/* Wait pid */
	sprintf(tname, "l2tp%d", unit);
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
l2tp_connect(char *pppname)
{
	struct l2tp_softc *sc;

	/* Signal reconnect request */
	sc = l2tp_getsoftcbyname(pppname);
	if (sc != 0)
		l2tp_signal_reconnect(sc);
}

void
l2tp_disconnect(char *pppname)
{
	struct l2tp_softc *sc;

	/* Signal reconnect request */
	sc = l2tp_getsoftcbyname(pppname);
	if (sc != 0)
		l2tp_signal_disconnect(sc);
}
