/*
 * L2TP function declarations
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: l2tp_var.h,v 1.12 2010-07-19 06:35:17 Exp $
 */
#ifndef	__L2TP_H__
#define __L2TP_H___

#include <l2tp.h>

/* Maximum size of L2TP datagram we accept */
#define MAX_PACKET_LEN          1024
#define MAX_SECRET_LEN          96
#define MAX_HOSTNAME            128
#define ESTABLISHE_TIME     	15

#define MD5LEN                  16      /* Length of MD5 hash */

#define HELLO_TIME		60
#define ACK_TIME		1
#define VENDOR_STR 		"Broadcom Corporation"
#define CTRL_HDR_LEN		12

/* A session within a tunnel */
typedef struct l2tp_session_t	l2tp_session;
typedef	struct l2tp_tunnel_t	l2tp_tunnel;

/* L2TP session */
enum {
	SESSION_INIT,
	SESSION_IDLE,
	SESSION_WAIT_TUNNEL,
	SESSION_WAIT_REPLY,
	SESSION_ESTABLISHED
};

struct l2tp_session_t {
	l2tp_tunnel *tunnel;		/* Parent tunnel */
	uint16_t sid;			/* My ID */
	uint16_t psid;			/* Quitting ID */
	int state;			/* Session state */
};

/* L2TP tunnel */
enum {
	TUNNEL_IDLE,
	TUNNEL_WAIT_CTL_REPLY,
	TUNNEL_WAIT_CTL_CONN,
	TUNNEL_ESTABLISHED,
	TUNNEL_RECEIVED_STOP_CCN,
	TUNNEL_SENT_STOP_CCN
};

struct l2tp_tunnel_t {
	l2tp_session session;		/* Session */
	int sock;			/* Socket for read/send package */
	uint16_t tid;			/* Our tunnel ID */
	uint16_t ptid;			/* Peer's tunnel identifier */
	char secret[MAX_SECRET_LEN];	/* Secret for this peer */
	size_t secret_len;		/* Length of secret */
	struct sockaddr_in peer_addr;	/* Peer's address */
	uint16_t Ns;			/* Sequence of next packet to send */
	uint16_t Nr;			/* Expected sequence of next received packet */
	uint16_t peer_Nr;		/* Last packet ack'd by peer */
	uint16_t rcv_win_size;		/* Our receive window size */
	uint16_t peer_rcv_win_size;	/* Peer receive window size */
	int hello_timeout;		/* time for hello */
	int create_time;		/* tunnel create time */
	int ack_time;			/* time for send ack's ZLB */
	char peer_hostname[MAX_HOSTNAME];	/* Peer's host name */
	char hostname[MAX_HOSTNAME];	/* My hostname */
	uint8_t response[MD5LEN];	/* Our response to challenge */
	uint8_t expected_response[MD5LEN];	/* Expected resp. to challenge */
	int state;			/* Tunnel state */

	/* Receive packet */
	int rx_msg_type;
	int rx_avpoff;
	struct l2tphdr rxpkt;

	/* Transmit packet */
	struct l2tphdr txpkt;
};

/*
 * L2TP fsm
 */
enum {
	L2TP_STATE_NONE,
	L2TP_STATE_WAIT,
	L2TP_STATE_INIT,
	L2TP_STATE_CONNECT,
	L2TP_STATE_CLOSE
};

#define	L2TP_RECONNECT			0x0002
#define	L2TP_DISCONNECT			0x0004
#define	L2TP_SHUTDOWN			0x0008

#ifndef IFNAMSIZ
#define IFNAMSIZ	16
#endif

struct l2tp_param {
	int unit;
	char pppname[16];
	char tunnel_ifname[16];
	char username[256];
	char password[256];
	struct in_addr server_ip;
	int idle_time;
	int demand;
	int keepalive;
	int mtu;
	int mru;
	int dflrt;
	int fqdn;
	char server_name[256];
	char hostname[256];
#ifdef	MPPE
	int mppe;
#endif
};

struct l2tp_softc {
	l2tp_tunnel tunnel;
	struct l2tp_param param;
	int flag;
	int state;
	struct in_addr tunnel_addr;
	struct in_addr server_addr;
	int ppp_pid;
	int devfd;
	int fsm_time;
};

/* Function declaration */
void ls_new_session(l2tp_session *ses);
void lt_new_tunnel(l2tp_tunnel *tunnel);

/* tunnel.c */
void lt_TunnelCreate(l2tp_tunnel *tunnel);
void lt_SendCtlMessage(l2tp_tunnel *tunnel);
void lt_StopTunnel(l2tp_tunnel *tunnel, char const *reason);
void lt_SendZLB(l2tp_tunnel *tunnel);
void lt_SendStopCCN(l2tp_tunnel *tunnel, int result_code, int error_code);
void lt_SendHello(l2tp_tunnel *tunnel);
void lt_Read(l2tp_tunnel *tunnel, int fd);
void lt_TxPacketInit(struct l2tphdr *l2tp, uint16_t msg_type, uint16_t tid, uint16_t sid);

/* session.c */
void ls_SessionOpen(l2tp_tunnel *tunnel);
void ls_SessionClose(l2tp_tunnel *tunnel);
void ls_HandleCDN(l2tp_tunnel *tunnel);
void ls_HandleICRP(l2tp_tunnel *tunnel);

int la_AvpLen(char *avp);
int la_AddAvp(char *avp, uint16_t type, uint16_t len, void *val);
struct l2tp_avphdr *la_HandleAvp(l2tp_tunnel *tunnel);

/* FSM functions */
void l2tp_fsm_timer(struct l2tp_softc *sc);
int  l2tp_fsm_up(struct l2tp_softc *sc);
void l2tp_fsm_down(struct l2tp_softc *sc);
void l2tp_fsm_handler(struct l2tp_softc *sc);

/*
 * Functions called by OSL main
 */
void l2tp_mainloop(struct l2tp_softc *sc, char *pppname);
void l2tp_signal_reconnect(struct l2tp_softc *sc);
void l2tp_signal_disconnect(struct l2tp_softc *sc);
void l2tp_signal_terminate(struct l2tp_softc *sc);

/*
 * External OSL functions declaration
 */
extern	int  l2tp_osl_ppp_open(struct l2tp_softc *sc);
extern	int  l2tp_osl_ppp_close(struct l2tp_softc *sc);
extern  int  l2tp_osl_ppp_state(struct l2tp_softc *sc);
extern	void l2tp_osl_init_param(struct l2tp_softc *sc);
extern	int  l2tp_osl_ifaddr(char *ifname, struct in_addr *ipaddr);

#if defined(linux)
#include <l2tp_linux_osl.h>
#elif defined(__ECOS)
#include <l2tp_ecos_osl.h>
#else
#error "Unsupported OSL requested"
#endif

#endif	/* __L2TP_H_ */
