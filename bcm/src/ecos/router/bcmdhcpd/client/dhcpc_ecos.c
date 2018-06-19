/*
 * DHCP client Ecos main entry.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpc_ecos.c,v 1.10 2010-08-04 09:22:16 Exp $
 *
 */
#include <dhcpc.h>
#include <dhcpc_osl.h>

#include <stdio.h>
#include <stdlib.h>
#include <ecos_oslib.h>
#include <bcmnvram.h>
#include <shutils.h>

#define	DHCPC_LOCK()	cyg_scheduler_lock()
#define	DHCPC_UNLOCK()	cyg_scheduler_unlock()

#define WAN_PREFIX(unit, prefix)	snprintf(prefix, sizeof(prefix), "wan%d_", unit)

extern int loipc(char *cmd, int sport, int dport, char *resp, int *rlen);

/* Context for dhcpc */
#ifndef	__CONFIG_DHCPC_NUM__
#define	__CONFIG_DHCPC_NUM__	3
#endif

#define NUM_DHCPC	__CONFIG_DHCPC_NUM__
#define STACK_SIZE	10240

struct dhcpc_ctx {
	int used;
	char ifname[16];
	struct dhcpc_config config;

	char tname[64];
	cyg_handle_t handle;
	cyg_thread thread;
	cyg_uint8 stack[STACK_SIZE];
};

static struct dhcpc_ctx dhcpc_ctx[NUM_DHCPC];


/* Allocate the context */
static struct dhcpc_ctx *dhcpc_ctx_alloc(char *ifname)
{
	struct dhcpc_ctx *ctx = 0;
	int i;

	DHCPC_LOCK();

	/* ifname exactly matching */
	for (i = 0; i < NUM_DHCPC; i++) {
		if (strcmp(dhcpc_ctx[i].ifname, ifname) == 0) {
			ctx = &dhcpc_ctx[i];
			break;
		}
	}

	if (ctx) {
		if (!ctx->used) {
			/* Resume this one */
			ctx->used = 1;
		}
		else {
			/* Must not run for the second time */
			printf("dhcpc_%s has already started.\n", ifname);
			ctx = NULL;
		}
	}
	else {
		/* This is the fist time running of this interface */
		for (i = 0; i < NUM_DHCPC; i++) {
			if (dhcpc_ctx[i].ifname[0] == 0) {
				ctx = &dhcpc_ctx[i];
				memset(ctx, 0, sizeof(*ctx));
				strncpy(ctx->ifname, ifname, sizeof(ctx->ifname)-1);
				ctx->used = 1;
				break;
			}
		}
	}

	DHCPC_UNLOCK();
	return ctx;
}

/*
 * Daemon of DHCPC module.
 * This function calls the portable
 * main loop of the dhcpc functions.
 */
static void
dhcpc_main(struct dhcpc_ctx *ctx)
{
	/* Enter os independent main loop */
	dhcpc_mainloop(&ctx->config);

	ctx->used = 0;
	return;
}

/*
 * Functions to raise the DHCPC daemon,
 * called by application main entry and
 * the mointor task.
 */
void
dhcpc_start(char *ifname, char *script, char *hostname)
{
	printf("%s,%d,%s,%s,%s\n",__FUNCTION__,__LINE__,ifname,script,hostname);
	struct dhcpc_ctx *ctx;
	struct dhcpc_config *config;

	/* Allocate one */
	ctx = dhcpc_ctx_alloc(ifname);
	if (ctx) {
		/* Setup param */
		sprintf(ctx->tname, "dhcpc_%s", ifname);

		config = &ctx->config;

		memset(config, 0, sizeof(*config));
		strcpy(config->ifname, ifname);
		strcpy(config->script, script);
		strcpy(config->hostname, hostname);
		config->req_ip.s_addr = 0;
		config->mtu = 0;

		/* Create thread */	
		cyg_thread_create(
			8,
			(cyg_thread_entry_t *)dhcpc_main,
			(cyg_addrword_t)ctx,
			ctx->tname,
			ctx->stack,
			sizeof(ctx->stack),
			&ctx->handle,
			&ctx->thread);
		cyg_thread_resume(ctx->handle);

		/* Make sure param is copied */
		cyg_thread_delay(10);
printf("function[%s] , line[%d] , start end \n" , __FUNCTION__ , __LINE__);
	}

	return;
}

/*
 * Note: This function send message to
 *       thread dhcpc_xxx.  Should only
 *       be called by CLI or monitor
 *       thread.
 */
static int
dhcpc_raise_event(char *ifname, char *event)
{
	/* Issue stop event */
	int ifidx ;

	ifidx = if_nametoindex(ifname);
	if (ifidx == 0)
		return -1;

	/* Send signal to dhcpc */
	loipc(event, 0, DHCPC_IPC_PORT+ifidx, 0, 0);
	return 0;
}

void
dhcpc_stop(char *ifname)
{
	int pid;
	char tname[64];

	/* Get dhcpc pid */
	sprintf(tname, "dhcpc_%s", ifname);
	pid = oslib_getpidbyname(tname);
	if (pid == 0)
		return;

	/* Send stop signal to dhcpc */
	dhcpc_raise_event(ifname, "FREE");

	/* Wait until thread exit */
	while (oslib_waitpid(pid, NULL) != 0)
		cyg_thread_delay(1);

	return;
}

/* External functions */
void
dhcpc_renew(char *ifname)
{
	dhcpc_raise_event(ifname, "RENEW");
	return;
}

void
dhcpc_release(char *ifname)
{
	dhcpc_raise_event(ifname, "RELEASE");
	return;
}

int dhcpc_renew1(void)
{
	char *wan_ifname;
	int unit;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";

	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	WAN_PREFIX(unit, prefix);

	if (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp")) {
		wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		dhcpc_renew(wan_ifname);
	}

	return 0;
}

int dhcpc_release1(void)
{
	char *wan_ifname;
	int unit;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";

	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	WAN_PREFIX(unit, prefix);

	if (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp")) {
		wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		dhcpc_release(wan_ifname);
	}
	return 0;
}

