/*
 * L2TP FSM
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: l2tp_fsm.c,v 1.16 2010-08-30 10:09:59 Exp $
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/ifl2tp.h>
#include <l2tp_var.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>


extern int open_udp_socket(char *ipaddr, unsigned short port);

static int
get_ifflags(char *pppname)
{
	int s = -1;
	int flags = 0;
	struct ifreq ifr;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return flags;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, pppname);
	if (ioctl(s, SIOCGIFFLAGS, &ifr) >= 0)
		flags = ifr.ifr_flags;

	close(s);
	return flags;
}

/* Close ppp */
static void
l2tp_fsm_pppclose(struct l2tp_softc *sc)
{
	struct ifl2tpreq req;

	/* Stop ppp */
	l2tp_osl_ppp_close(sc);

	/* Delete ifl2tp kernel structure */
	memset(&req, 0, sizeof(req));
	strcpy(req.pppname, sc->param.pppname);
	ioctl(sc->devfd, PPPIOCIFL2TPCONN, &req);
	return;
}

/* Establish ppp */
static int
l2tp_fsm_pppopen(struct l2tp_softc *sc)
{
	struct l2tp_param *param = &sc->param;
	struct ifl2tpreq req;

	/* Add ifl2tp kernel structure */
	memset(&req, 0, sizeof(req));

	strcpy(req.pppname, param->pppname);

	/* Set L2TP information */
	req.tunnel_addr   = sc->tunnel_addr;
	req.server_addr   = sc->server_addr;
	req.server_port   = sc->tunnel.peer_addr.sin_port;
	req.assigned_tid  = sc->tunnel.ptid;
	req.assigned_sid  = sc->tunnel.session.psid;
	req.tid        = sc->tunnel.tid;
	req.sid        = sc->tunnel.session.sid;

	ioctl(sc->devfd, PPPIOCIFL2TPCONN, &req);

	/* Start ppp */
	sc->ppp_pid = l2tp_osl_ppp_open(sc);

	/* Wait until interface up */
	if (sc->ppp_pid != 0) {
		while ((get_ifflags(sc->param.pppname) & IFF_UP) == 0) {
			/* check thread status of ppp */
			if (l2tp_osl_ppp_state(sc) == 0) {
				l2tp_fsm_down(sc);
				return 1;
			}
			l2tp_msleep(100);
		}
	}
	return 0;
}

/* Start tunnel */
static int
l2tp_StartConnect(struct l2tp_softc *sc)
{
	struct l2tp_param *param = &sc->param;
	struct hostent *hostinfo;

	hostinfo = gethostbyname(param->server_name);
	if (hostinfo && (hostinfo->h_length == 4)) {
		memcpy(&sc->server_addr.s_addr, hostinfo->h_addr_list[0], 4);
	}
	else {
		L2TP_LOG("%s:Can't resolve server %s\n", __func__, param->server_name);
#if 0//roy modify,2010/11/04
			return -1;
#else
//server ip is ip address
		inet_aton(param->server_name,&sc->server_addr);
#endif			
	}

	/* Setup call */
	ls_new_session((l2tp_session *)&sc->tunnel.session);

	lt_new_tunnel((l2tp_tunnel *)&sc->tunnel);

	/* Set peer address */
	sc->tunnel.peer_addr.sin_len = sizeof(struct sockaddr_in);
	sc->tunnel.peer_addr.sin_family = AF_INET;
	sc->tunnel.peer_addr.sin_port = htons(L2TP_PORT);
	sc->tunnel.peer_addr.sin_addr = sc->server_addr;

	lt_TunnelCreate((l2tp_tunnel *)&sc->tunnel);
	return 0;
}

/* L2TP FSM */
int
l2tp_fsm(struct l2tp_softc *sc)
{
	l2tp_tunnel *tunnel = (l2tp_tunnel *)&sc->tunnel;
	l2tp_session *sess = &tunnel->session;
	struct in_addr tunnel_addr;
	int s;

	fd_set fds;
	struct timeval tv = {1, 0};
	int maxfd = -1;
	int n;
	int flags;
	char rcvbuf[64];

	/* Check sc state */
	switch (sc->state) {
	case L2TP_STATE_NONE:
		l2tp_msleep(200);
		break;

	case L2TP_STATE_WAIT:
		/* Set receive select set */
		FD_ZERO(&fds);

		FD_SET(sc->devfd, &fds);
		maxfd = sc->devfd+1;

		/* Wait for socket events */
		n = select(maxfd+1, &fds, NULL, NULL, &tv);
		if (n > 0) {
			/* process dhcpd */
			if (FD_ISSET(sc->devfd, &fds)) {
				int len;

				/* Process the dhcpd incoming packet */
				len = read(sc->devfd, rcvbuf, 12);
				if (len >= sizeof("DIALUP") &&
					memcmp(rcvbuf, "DIALUP", sizeof("DIALUP")) == 0) {
					sc->state = L2TP_STATE_INIT;
				}
			}
		}
		break;

	case L2TP_STATE_INIT:
		if (l2tp_osl_ifaddr(sc->param.tunnel_ifname, &tunnel_addr) != 0 ||
		    tunnel_addr.s_addr == 0) {
			/* Stick in this state, until got */
			l2tp_msleep(200);
			break;
		}

		s = open_udp_socket(inet_ntoa(tunnel_addr), L2TP_PORT);
		if (s < 0) {
			sc->state = L2TP_STATE_CLOSE;
			break;
		}

		tunnel->sock = s;
		sc->tunnel_addr = tunnel_addr;
		sc->state = L2TP_STATE_CONNECT;
		sess->state = SESSION_INIT;
		tunnel->state = TUNNEL_IDLE;
		tunnel->create_time = time(0);
		break;

	case L2TP_STATE_CONNECT:
		/* Check session state */
		if (sess->state == SESSION_INIT) {
			if (l2tp_StartConnect(sc) != 0) {
				sc->state = L2TP_STATE_CLOSE;
				break;
			}
		}
		/* Check session state and tunnel */
		else if (sess->state == SESSION_IDLE) {
			sc->state = L2TP_STATE_CLOSE;
			break;
		}
		/* Check ppp state */
		else if (sess->state == SESSION_ESTABLISHED) {
			if (sc->ppp_pid == 0) {
				/* Start ppp */
				if (l2tp_fsm_pppopen(sc) != 0) {
					sc->state = L2TP_STATE_CLOSE;
					break;
				}
			}
			else {
				flags = get_ifflags(sc->param.pppname);
				if ((flags & IFF_UP) == 0) {
					sc->state = L2TP_STATE_CLOSE;
					break;
				}
			}
		}

		/* Read L2 control packet */
		FD_ZERO(&fds);

		FD_SET(tunnel->sock, &fds);
		maxfd = tunnel->sock;

		n = select(maxfd+1, &fds, NULL, NULL, &tv);
		if (n > 0) {
			if (FD_ISSET(tunnel->sock, &fds))
				lt_Read(tunnel, tunnel->sock);
		}
		break;

	case L2TP_STATE_CLOSE:
		l2tp_fsm_down(sc);
		break;

	default:
		break;
	}

	return 0;
}

void
l2tp_fsm_timer(struct l2tp_softc *sc)
{
	l2tp_tunnel *tunnel = &sc->tunnel;
	l2tp_session *sess = (l2tp_session *)tunnel;
	time_t now = time(0);

	/* Skip none and wait state */ 
	if (sc->state < L2TP_STATE_CONNECT)
		return;

	/* Check session state and tunnel */
	if (sess->state == SESSION_IDLE)
		return;

	if (tunnel->state == TUNNEL_RECEIVED_STOP_CCN) {
		tunnel->session.state = SESSION_IDLE;
	}
	else if ((now - tunnel->hello_timeout) > HELLO_TIME) {
		tunnel->hello_timeout = now;
		lt_SendHello(tunnel);
	}
	else if (tunnel->ack_time && (now - tunnel->ack_time) > ACK_TIME) {
		tunnel->ack_time = 0;
		lt_SendZLB(tunnel);
	}

	if (tunnel->session.state != SESSION_ESTABLISHED &&
	    (now - tunnel->create_time) > ESTABLISHE_TIME) {
		l2tp_fsm_down(sc);
	}
	return;
}

void
l2tp_fsm_handler(struct l2tp_softc *sc)
{
	l2tp_fsm(sc);
	l2tp_fsm_timer(sc);
}

int
l2tp_fsm_up(struct l2tp_softc *sc)
{
	struct ifl2tpreq req;
	char dev[128] = {0};

	L2TP_LOG("%s\n", __func__);

	if (sc->devfd < 0) {
		sprintf(dev, "/dev/net/l2tp/%s", sc->param.pppname);
		sc->devfd = open(dev, O_RDWR);
		if (sc->devfd < 0) {
			L2TP_LOG("Can't open %s!\n", dev);
			return -1;
		}
	}

	/* Demand wait or start connection */
	if (sc->param.demand && (sc->flag & L2TP_RECONNECT) == 0) {
		/* Starting from wait dialup */
		req.wait = 1;
		ioctl(sc->devfd, PPPIOCIFL2TPWAIT, &req);
		sc->state = L2TP_STATE_WAIT;
	}
	else
		sc->state = L2TP_STATE_INIT;

	return 0;
}

void
l2tp_fsm_down(struct l2tp_softc *sc)
{
	L2TP_LOG("%s\n", __func__);

	if (sc->devfd >= 0) {
		l2tp_tunnel *tunnel = &sc->tunnel;
		l2tp_session *sess = &tunnel->session;
		struct ifl2tpreq req;

		/* Close ppp if any */
		l2tp_fsm_pppclose(sc);

		if (tunnel->sock >= 0) {
			/* Close tunnel */
			if (sess->state != SESSION_IDLE) {
				/* Send stop tunnel datagram out */
				lt_StopTunnel(tunnel, "Force stop tunnel");
				l2tp_msleep(200);

				/* Reset sessions */
				tunnel->session.state = SESSION_IDLE;
			}

			/* Close socket */
			close(tunnel->sock);
			tunnel->sock = -1;
		}

		/* Unhook dialup check */
		req.wait = 0;
		ioctl(sc->devfd, PPPIOCIFL2TPWAIT, &req);

		close(sc->devfd);
		sc->devfd = -1;
	}

	/* Enter NONE state */
	sc->state = L2TP_STATE_NONE;

	/* Re-initializing if possible */
	if ((sc->flag & (L2TP_SHUTDOWN | L2TP_DISCONNECT)) == 0 &&
	    (sc->param.demand || sc->param.keepalive))
		l2tp_fsm_up(sc);

	return;
}
