/*
 * PPTP include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pptp_var.h,v 1.1 2010-07-15 11:32:22 Exp $
 */

#ifndef __PPTP_VAR_H__
#define __PPTP_VAR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pptp.h>

#define PPTP_TIMER_INTERVAL		60

#define	PPTP_RECONNECT			0x0002
#define	PPTP_DISCONNECT			0x0004
#define	PPTP_SHUTDOWN			0x0008

#ifndef IFNAMSIZ
#define IFNAMSIZ	16
#endif

struct pptp_param {
	int  unit;
	char pppname[IFNAMSIZ];
	char tunnel_ifname[IFNAMSIZ];
	char username[256];
	char password[256];
	u_long my_ip;
	u_long netmask;
	char server_name[256];
	int idle_time;
	int demand;
	int keepalive;
	int mtu;
	int mru;
#ifdef	MPPE
	int mppe;
#endif
	int dflrt;
	char hostname[256];
};

struct pptp_call {
	u_int16_t call_id;
	u_int16_t peer_call_id;
	u_int16_t call_sn;
	u_int32_t speed;
};

#define PPTP_RX_BUFSIZ	(2*1024)

struct pptp_tunnel {
	u_int32_t keepalive_sent;
	u_int32_t keepalive_id;

	u_int16_t vers;
	u_int16_t firmware;
	char host[PPTP_HOSTNAME_LEN];
	char vendor[PPTP_VENDOR_LEN];

	struct pptp_call *call;
	u_int16_t call_sn;

	size_t read_len;
};

enum {
	PPTP_STATE_NONE = 0,
	PPTP_STATE_WAIT,
	PPTP_STATE_INIT,
	PPTP_STATE_CONNECT,
	PPTP_STATE_CALL_CONNECTED,
	PPTP_STATE_GRE,
	PPTP_STATE_PPP,
	PPTP_STATE_CLOSE
};

enum {
	PPTP_TUNNEL_STATE_IDLE,
	PPTP_TUNNEL_STATE_WAITSTARTREPLY,
	PPTP_TUNNEL_STATE_WAITSTOPREPLY,
	PPTP_TUNNEL_STATE_CONNECTED,
	PPTP_TUNNEL_STATE_DESTROY
};

enum {
	PPTP_CALL_STATE_IDLE,
	PPTP_CALL_STATE_WAITREPLY,
	PPTP_CALL_STATE_CONNECTED,
	PPTP_CALL_STATE_WAITDISC
};

struct pptp_softc {
	/* Configuration */
	struct pptp_tunnel *tunnel;

	struct pptp_param param;

	int flag;
	int devfd;

	int state;
	time_t state_time;
	struct in_addr server_ip;
	struct in_addr tunnel_ip;

	int tunnel_sock;
	int tunnel_state;
	int call_state;
	u_int16_t call_id;
	u_int16_t peer_call_id;

	int ppp_pid;
};

void pptp_fsm_timer(struct pptp_softc *sc);
int  pptp_fsm_up(struct pptp_softc *sc);
void pptp_fsm_down(struct pptp_softc *sc);
void pptp_fsm_handler(struct pptp_softc *sc);

/*
 * Functions called by OSL main
 */
void pptp_mainloop(struct pptp_softc *sc, char *pppname);
void pptp_signal_reconnect(struct pptp_softc *sc);
void pptp_signal_disconnect(struct pptp_softc *sc);
void pptp_signal_terminate(struct pptp_softc *sc);

/*
 * External OSL functions declaration
 */
extern	int  pptp_osl_ppp_open(struct pptp_softc *sc);
extern	int  pptp_osl_ppp_close(struct pptp_softc *sc);
extern	void pptp_osl_init_param(struct pptp_softc *sc);
extern	int  pptp_osl_ifaddr(char *ifname, struct in_addr *ipaddr);
extern  int  pptp_osl_ppp_state(struct pptp_softc *sc);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#if defined(linux)
#include <pptp_linux_osl.h>
#elif defined(__ECOS)
#include <pptp_ecos_osl.h>
#else
#error "Unsupported OSL requested"
#endif

#endif	/* __PPTP_VAR_H__ */
