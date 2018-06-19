/*
 * PPPOE main
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pppoe_ecos.c,v 1.5 2010-10-23 10:57:42 Exp $
 */
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pppd.h>
#include <pppoe_var.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <shutils.h>
#include <bcmnvram.h>

#define	PPPOE_LOCK()	cyg_scheduler_lock()
#define	PPPOE_UNLOCK()	cyg_scheduler_unlock()


/* Conexts for pppoe */
#define STACK_SIZE	8192

struct pppoe_ctx {
	struct pppoe_softc sc;						//pppoe strcture

	int unit;
	char pppname[16];

	char tname[64];
	cyg_handle_t handle;						//int 型
	cyg_thread thread;                                                  //线程结构
	cyg_uint8 stack[STACK_SIZE];					//char
};

static struct pppoe_ctx pppoe_ctx[NUM_PPP];
static int pppoe_ctx_used[NUM_PPP];

/* Forwarding declaration */
static struct pppoe_ctx *pppoe_ctx_find(char *pppname);
static struct pppoe_ctx *pppoe_ctx_alloc(char *pppname);
static void pppoe_ctx_free(struct pppoe_ctx *ctx);

/*
 * The follwong functions are called by
 * thread out side the pppoe thread,
 * have to special take care of the
 * context lock.
 */
/*
 * Find the pppoe context with given
 * pppname.
 * Note: the caller should do
 *       PPPOE_LOCK()/PPPOE_UNLOCK()
 *       by itself.
 */
struct pppoe_ctx *
pppoe_ctx_find(char *pppname)
{
	struct pppoe_ctx *ctx = 0;
	int unit;

	unit = atoi(pppname+3);
	if (unit < 0 || unit >= NUM_PPP)
		return 0;

	/* Check it */
	//if (pppoe_ctx_used[unit] == 1)
	//if (pppoe_ctx_used[unit] == 2)//gong modify for pppoe server
   if (pppoe_ctx_used[unit] == 3)//hqw modify
		ctx = &pppoe_ctx[unit];

	return ctx;
}

static struct pppoe_softc *
pppoe_getsoftcbyname(char *pppname)
{
	struct pppoe_softc *sc = 0;
	struct pppoe_ctx *ctx = pppoe_ctx_find(pppname);

	if (ctx != 0)
		sc = &ctx->sc;

	return sc;
}

/* Allocate/free context */
static struct pppoe_ctx *
pppoe_ctx_alloc(char *pppname)
{
	struct pppoe_ctx *ctx = 0;
	int unit;

	unit = atoi(pppname+3);
	
	if (unit < 0 || unit >= NUM_PPP)
		return 0;

	/* Allocate one */
	PPPOE_LOCK();
	
	/*if (pppoe_ctx_used[unit] == 0) {
		pppoe_ctx_used[unit] = 1;*/
	//gong modify for pppoe server	
	if (pppoe_ctx_used[unit] == 0 || pppoe_ctx_used[unit] == 1 || pppoe_ctx_used[unit] == 2) {
		pppoe_ctx_used[unit] = 3;
		ctx = &pppoe_ctx[unit];

		/* Setup data */
		strcpy(ctx->pppname, pppname);
		sprintf(ctx->tname, "pppoe%d", unit);
		ctx->unit = unit;
	}
	PPPOE_UNLOCK();
	
	return ctx;
}

static void
pppoe_ctx_free(struct pppoe_ctx *ctx)
{
	/* Free it */
	PPPOE_LOCK();
	pppoe_ctx_used[ctx->unit] = 0;
	PPPOE_UNLOCK();

	return;
}

/*
 * Functions below are the osl dependent
 * implementated.  The prefix of them is
 * 'pppoe_osl_' to avoid naming pollution.
 * They are called in the pppoe thread.
 */
int
pppoe_osl_ppp_open(struct pppoe_softc *sc)
{
	struct pppoe_param *param = &sc->param;
	int pid = 0;
	struct ppp_conf conf;

	/* Attach the ppp configuration structure */
	memset(&conf, 0, sizeof(conf));

	strcpy(conf.pppname, param->pppname);
	strcpy(conf.ifname, param->ethname);
	strcpy(conf.user, param->username);
	strcpy(conf.passwd, param->password);

	conf.unit           = param->unit;
	conf.mtu            = param->mtu;
	conf.mru            = param->mru;
	conf.idle           = param->idle_time;

	/* Start ppp daemon */
	pid = ppp_start(&conf);

	PPPOE_LOG("%s(%s): open successfully.", __func__, sc->param.pppname);
	return pid;
}

int
pppoe_osl_ppp_close(struct pppoe_softc *sc)
{
	if (sc->ppp_pid) {
		/* Stop ppp */
		ppp_stop(sc->param.pppname);

		/* Wait until the ppp shutdown completely */
		while (oslib_waitpid(sc->ppp_pid, NULL) != 0)
			pppoe_msleep(100);

		/* Set to null pid */
		sc->ppp_pid = 0;
	}

	PPPOE_LOG("%s(%s): close completedly.", __func__, sc->param.pppname);
	return 0;
}

int
pppoe_osl_ppp_state(struct pppoe_softc *sc)
{
	if (sc->ppp_pid && (oslib_waitpid(sc->ppp_pid, NULL) != 0))
		return 1;
	else
		return 0;
}

/* 
 * Read configuration from NVRAM to
 * pppoe parameter structure.
 */
void
pppoe_osl_init_param(struct pppoe_softc *sc)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *value = NULL;
	struct pppoe_param *param = &sc->param;
	int unit = atoi(&param->pppname[3]);


//	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	/* Reset all the configuration */
	param->unit = unit;
	if (unit == 2){
		unit = 0;
	}
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	if ((value = nvram_get(strcat_r(prefix, "pppoe_username", tmp))) != NULL)
		strcpy(param->username, value);
	if ((value = nvram_get(strcat_r(prefix, "pppoe_passwd", tmp))) != NULL)
		strcpy(param->password, value);
	if ((value = nvram_get(strcat_r(prefix, "pppoe_service", tmp))) != NULL)
		strcpy(param->service_name, value);
	if ((value = nvram_get(strcat_r(prefix, "pppoe_ac", tmp))) != NULL)
		strcpy(param->ac_name, value);
	if ((value = nvram_get(strcat_r(prefix, "pppoe_idletime", tmp))) != NULL)
		param->idle_time = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "pppoe_demand", tmp))) != NULL)
		param->demand = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "pppoe_keepalive", tmp))) != NULL)
		param->keepalive = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "pppoe_mtu", tmp))) != NULL)
		param->mtu = atoi(value);
	if ((value = nvram_get(strcat_r(prefix, "pppoe_mru", tmp))) != NULL)
		param->mru = atoi(value);


	/* Set device name */
	if(unit == 1)
	{
		strcpy(param->ethname, nvram_get("lan_ifname"));
	}
	else
	{	
		if ((value = nvram_get(strcat_r(prefix, "ifname", tmp))) != NULL)
			strcpy(param->ethname, value);
	}
	return;
}

/*
 * Daemon of PPPOE module.
 * This function calls the portable
 * main loop of the pppoe functions.
 */
static void
pppoe_main(struct pppoe_ctx *ctx)
{
	struct pppoe_softc *sc = &ctx->sc;
	
#ifdef __CONFIG_TENDA_HTTPD_V3__
	int demand = 0 , unit = 0,manage_flag = 0;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";

	/* Enter os independent main loop */
	
	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	demand = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_demand", tmp)));
	manage_flag = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_manage_flag", tmp)));
	
	if(demand == 2 && manage_flag <= 0){
		return ;
	}
	nvram_set(strcat_r(prefix, "pppoe_manage_flag", tmp) , "0");
	nvram_commit();
#endif	
	pppoe_mainloop(sc, ctx->pppname);

	/* Free this context */
	pppoe_ctx_free(ctx);

	PPPOE_LOG("%s(%s):Exit", __func__, ctx->pppname);
	return;
}

/*
 * Functions to raise the PPPOE daemon,
 * called by application main entry and
 * the mointor task.
 */

void
pppoe_start(char *pppname)
{
	//struct pppoe_ctx *ctx[2];
	static struct pppoe_ctx *ctx[3] = {0};
	int unit;
	unit = atoi(pppname+3);

	if (unit < 0 || unit >= NUM_PPP) 
	{
		printf (" %s %d  PPPOE ERROR  unit=%d=\n", __FUNCTION__, __LINE__, unit);
		return ;
	}
	
	/* Allocate one */
#ifdef __CONFIG_TENDA_HTTPD_V3__
	if(pppoe_ctx_used[unit] == 3 && ctx[unit] != NULL){
		cyg_thread_delete(ctx[unit]->handle);
		pppoe_ctx_free(ctx[unit]);
	}
#endif
	ctx[unit] = pppoe_ctx_alloc(pppname);
	
	if (ctx[unit]) {
		/* Create thread */
		cyg_thread_create(
			8,
			(cyg_thread_entry_t *)pppoe_main,
			(cyg_addrword_t)ctx[unit],
			ctx[unit]->tname,
			ctx[unit]->stack,
			sizeof(ctx[unit]->stack),
			&ctx[unit]->handle,
			&ctx[unit]->thread);
		cyg_thread_resume(ctx[unit]->handle);

		/* Make sure thread is running */
		cyg_thread_delay(10);
	}
	return;
}

/*
 * Functions to cease the PPPOE daemon,
 * called by application main entry and
 * the mointor task.
 */
void
pppoe_stop(char *pppname)
{
	int pid;
	char tname[64];
	int unit = atoi(&pppname[3]);
	struct pppoe_softc *sc;

	/* Signal terminate request */
	PPPOE_LOCK();
	sc = pppoe_getsoftcbyname(pppname);
	if (sc != 0)
	{
		pppoe_signal_terminate(sc);

	}
	PPPOE_UNLOCK();

	/* Wait pid */
	sprintf(tname, "pppoe%d", unit);
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
pppoe_connect(char *pppname)
{
	struct pppoe_softc *sc;

	/* Signal reconnect request */
	PPPOE_LOCK();
	sc = pppoe_getsoftcbyname(pppname);
	if (sc != 0)
		pppoe_signal_reconnect(sc);
	PPPOE_UNLOCK();
}

void
pppoe_disconnect(char *pppname)
{
	struct pppoe_softc *sc;

	/* Signal reconnect request */
	PPPOE_LOCK();
	sc = pppoe_getsoftcbyname(pppname);
	if (sc != 0)
		pppoe_signal_disconnect(sc);
	PPPOE_UNLOCK();
}
