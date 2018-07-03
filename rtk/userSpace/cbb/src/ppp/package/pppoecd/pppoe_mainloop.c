/*
 * PPPOE daemon
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pppoe_mainloop.c,v 1.6 2010-07-15 11:18:26 Exp $
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <unistd.h>
#include <pppoe_var.h>
#include <stdio.h>
#include <stdlib.h>
#include "sys_module.h"
#include "pi_common.h"
#include "wifi.h"
#ifndef nvram_safe_get
#define nvram_safe_get(name) (nvram_get(name) ? : "")
#endif

#ifdef __CONFIG_CHINA_NET_CLIENT__
	extern int cur_pppoe_times;
#endif

extern void send_padt_first_tag_init();
/*
 * Shutdown pppoe
 */
static void
pppoe_shutdown(struct pppoe_softc *sc)
{
	PPPOE_LOG("%s(%s)", __func__, sc->param.pppname);

	pppoe_fsm_down(sc);
	
	send_padt_first_tag_init();
#ifdef __CONFIG_PPPOE_SERVER__
	synchro_config_start_time(0,SET_SYBCHRO_TIME);
#endif
	return;
}

extern int num_padi;

/* Do interface initialization */
static void
pppoe_init(struct pppoe_softc *sc, char *pppname)
{
	char *clean_start = (char *)&sc->param;
	int clean_size = (int)((char *)&sc->slog - clean_start);

	PPPOE_LOG("%s(%s)", __func__, pppname);//ppp0
	num_padi=0;
	
#ifdef __CONFIG_CHINA_NET_CLIENT__
    /*lrl 初始化拨号次数*/
    if(0 == strncmp(pppname,"ppp0",4))
		cur_pppoe_times = 0;
#endif

	/*
	 * 1. Cleanup sc structure.
	 * 2. Keep flag to avoid shutting dwon
	 * rice condition.
	 */
	sc->flag &= PPPOE_SHUTDOWN;
	memset(clean_start, 0, clean_size);
	strcpy(sc->param.pppname, pppname);
	sc->dev_fd = -1;

	/* Read pppoe parameter */
	pppoe_osl_init_param(sc);//hqw填充PPPOE数据包

#if 1//tenda add,we set demand ourself in rc.c,so disable here
	sc->param.demand = 0;
#endif

	if (sc->param.demand == 0)
		sc->param.idle_time = 0;
	pppoe_fsm_up(sc);
	return;
}

/* 
 * The main entrance of pppoe main loop,
 * continues until shutdown happens.
 */
void pppoe_mainloop(struct pppoe_softc *sc, char *pppname)
{
	char msg_param[PI_BUFLEN_256] = {0};
	int old_mode = 0;
	/* Enter init state */
	memset(sc, 0, sizeof(*sc));
	pppoe_init(sc, pppname);

	/* Process event */
	while (1) {
        /*add by liangia*/
		if (sc->flag & PPPOE_SHUTDOWN)
		{
			break;
		}

		/* Check reconnect request */
		if (sc->flag & PPPOE_RECONNECT) {
			pppoe_fsm_up(sc);
			sc->flag &= ~PPPOE_RECONNECT;
		}

		/* Check disconnect request */
		if (sc->flag & PPPOE_DISCONNECT) {
			pppoe_fsm_down(sc);			
			send_padt_first_tag_init();
			sc->flag &= ~PPPOE_DISCONNECT;
		}
		/* Main state machine */
#ifdef __CONFIG_PPPOE_SERVER__
/*lq 判断超时和跳过快速设置页面*/
		if(sc->param.unit == 1)
		{
		    /*lq如果同步成功，则退出*/
			if(get_synchro_status() == SYNCHRO_SUCESS)
			{
				break;
			}
			if(sc->param.synchro_type == MANUAL_SYNCHRO)
			{
				unsigned long long sec;
				sec = cyg_current_time();
				sec = sec  / 100;
				if((sec - sc->param.start_time) > sc->param.time_out)
					break;
			}
			else
			{
				if(strcmp(nvram_safe_get("restore_quick_set"),"0") == 0)
					break;
			}
		}
#endif
		pppoe_fsm_handler(sc);
	}
	
	pppoe_shutdown(sc);
#ifdef __CONFIG_PPPOE_SERVER__
	extern void change_to_router_mode(int old_mode);
 	/*lq如果同步成功，则退出，并重启wan口*/
	if(sc->param.unit == 1 && get_synchro_status() == SYNCHRO_SUCESS)
	{
		sprintf(msg_param, "op=%d", OP_RESTART);
		msg_send(MODULE_RC, RC_WAN_MODULE, msg_param);
	}
#endif
	return;
}

/*
 * The following functions are apis for
 * pppoe_ecos.c
 */
void
pppoe_signal_reconnect(struct pppoe_softc *sc)
{
	sc->flag |= PPPOE_RECONNECT;
}

void
pppoe_signal_disconnect(struct pppoe_softc *sc)
{
	sc->flag |= PPPOE_DISCONNECT;
}

void
pppoe_signal_terminate(struct pppoe_softc *sc)
{
	sc->flag |= PPPOE_SHUTDOWN;
}
