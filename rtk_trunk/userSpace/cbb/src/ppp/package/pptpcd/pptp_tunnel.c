/*
 * PPTP tunnel module.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pptp_tunnel.c,v 1.7 2010-07-23 16:04:37 Exp $
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pppd.h>
#include <net/pptp_gre.h>
#include <pptp_var.h>
#include <stdlib.h>

static int  first = 1;
static char pptp_read_buf[PPTP_RX_BUFSIZ];

int pptp_tunnel_hangup(struct pptp_softc *sc);
int pptp_tunnel_gre_close(struct pptp_softc *sc);

struct msg_table
{
	u_int16_t type;
	int (*handler_fun) __P((struct pptp_softc *, struct pptpMsgHead *));
	int len;
};

static int pptp_handle_StopControlConnRequest(struct pptp_softc *sc, struct pptpMsgHead *header);
static int pptp_handle_StartControlConnReply(struct pptp_softc *sc, struct pptpMsgHead *header);
static int pptp_handle_StopControlConnReply(struct pptp_softc *sc, struct pptpMsgHead *header);
static int pptp_handle_EchoRequest(struct pptp_softc *sc, struct pptpMsgHead *header);
static int pptp_handle_EchoReply(struct pptp_softc *sc, struct pptpMsgHead *header);
static int pptp_handle_OutCallReply(struct pptp_softc *sc, struct pptpMsgHead *header);
static int pptp_handle_CallClearRequest(struct pptp_softc *sc, struct pptpMsgHead *header);
static int pptp_handle_CallDiscNotify(struct pptp_softc *sc, struct pptpMsgHead *header);
static int pptp_handle_SetLinkInfo(struct pptp_softc *sc, struct pptpMsgHead *header);

struct msg_table ctrlmsg_table[] =
{
	{PPTP_StartControlConnRequest,	NULL, 					sizeof(struct pptpStartCtrlConnRequest)},
	{PPTP_StartControlConnReply,	pptp_handle_StartControlConnReply, 	sizeof(struct pptpStartCtrlConnReply)},
	{PPTP_StopControlConnRequest,	pptp_handle_StopControlConnRequest,	sizeof(struct pptpStopCtrlConnRequest)},
	{PPTP_StopControlConnReply,	pptp_handle_StopControlConnReply, 	sizeof(struct pptpStopCtrlConnReply)},
	{PPTP_EchoRequest,		pptp_handle_EchoRequest,		sizeof(struct pptpEchoRequest)},
	{PPTP_EchoReply,		pptp_handle_EchoReply,			sizeof(struct pptpEchoReply)},
	{PPTP_OutCallRequest,		NULL,					sizeof(struct pptpOutCallRequest)},
	{PPTP_OutCallReply,		pptp_handle_OutCallReply,		sizeof(struct pptpOutCallReply)},
	{PPTP_InCallRequest,		NULL,					sizeof(struct pptpInCallRequest)},
	{PPTP_InCallReply,		NULL,					sizeof(struct pptpInCallReply)},
	{PPTP_InCallConn,		NULL,					sizeof(struct pptpInCallConn)},
	{PPTP_CallClearRequest,		pptp_handle_CallClearRequest,		sizeof(struct pptpCallClearRequest)},
	{PPTP_CallDiscNotify,		pptp_handle_CallDiscNotify,		sizeof(struct pptpCallDiscNotify)},
	{PPTP_WanErrorNotify,		NULL,					sizeof(struct pptpWanErrorNotify)},
	{PPTP_SetLinkInfo,		pptp_handle_SetLinkInfo,		sizeof(struct pptpSetLinkInfo)},
	{-1,				NULL,					0}
};

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

int
pptp_tunnel_send(struct pptp_softc *sc, void *packet, size_t len)
{
	int n;

	n = write(sc->tunnel_sock, packet, len);
	if (n < 0) {
		if (errno != EAGAIN && errno != EINTR) {
			sc->tunnel_state = PPTP_TUNNEL_STATE_DESTROY;
		}
		return 0;
	}

	return 0;
}

int
pptp_tunnel_call(struct pptp_softc *sc)
{
	struct pptp_tunnel *tunnel = sc->tunnel;
	struct pptp_call *call;
	struct pptpOutCallRequest request;

	call = (struct pptp_call *)malloc(sizeof(struct pptp_call));
	if (call == NULL) {
		PPTP_LOG("pptp_tunnel_call: malloc failed");
		return -1;
	}

	call->call_id = 0;
	call->peer_call_id = 0;
	call->call_sn = tunnel->call_sn++;
	sc->call_state = PPTP_CALL_STATE_IDLE;

	memset(&request, 0, sizeof(request));
	request.header.length = htons(sizeof(struct pptpOutCallRequest));
	request.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
	request.header.magic = htonl(PPTP_MAGIC);
	request.header.type = htons(PPTP_OutCallRequest);
	request.header.resv0 = 0;
	request.cid = call->call_id;
	request.serno = call->call_sn;
	request.minBps = htonl(PPTP_MINBPS);
	request.maxBps = htonl(PPTP_MAXBPS);
	request.bearType = htonl(PPTP_BEARCAP_ANALOG);
	request.frameType = htonl(PPTP_FRAMECAP_ASYNC);
	request.recvWin = htons(PPTP_RECV_WIN);

	if (pptp_tunnel_send(sc, &request, sizeof(request)) == 0) {
		sc->call_state = PPTP_CALL_STATE_WAITREPLY;
		sc->state_time = time(0) + PPTP_TIMER_INTERVAL;

		tunnel->call = call;
		return 0;
	}
	else {
		free(call);
		return -1;
	}
}

int
pptp_tunnel_call_clear(struct pptp_softc *sc)
{
	struct pptp_tunnel *tunnel = sc->tunnel;
	struct pptp_call *call = tunnel->call;
	struct pptpCallClearRequest request;

	if (sc->call_state == PPTP_CALL_STATE_IDLE)
		return 0;

	request.header.length = htons(sizeof(struct pptpCallClearRequest));
	request.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
	request.header.magic = htonl(PPTP_MAGIC);
	request.header.type = htons(PPTP_CallClearRequest);
	request.header.resv0 = 0;
	request.cid = htons(call->call_id);
	request.resv0 = 0;

	pptp_tunnel_send(sc, &request, sizeof(request));

	sc->call_state = PPTP_CALL_STATE_WAITDISC;
	sc->state_time = time(0) + PPTP_TIMER_INTERVAL;
	return 0;
}

int
pptp_tunnel_call_release(struct pptp_softc *sc)
{
	struct pptp_tunnel *tunnel = sc->tunnel;

	if (tunnel->call)
		free(tunnel->call);
	tunnel->call = 0;
	return 0;
}

int
pptp_tunnel_keepalive(struct pptp_softc *sc)
{
	struct pptpEchoRequest request;

	request.header.length = htons(sizeof(struct pptpEchoRequest));
	request.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
	request.header.magic = htonl(PPTP_MAGIC);
	request.header.type = htons(PPTP_EchoRequest);
	request.header.resv0 = 0;
	++sc->tunnel->keepalive_id;
	request.id = sc->tunnel->keepalive_id;

	if (pptp_tunnel_send(sc, &request, sizeof(request)) == 0)
		sc->tunnel->keepalive_sent++;

	return 0;
}

int
pptp_tunnel_connect(struct pptp_softc *sc)
{
	struct pptp_param *param = &sc->param;
	struct pptp_tunnel *tunnel;
	struct hostent *hostinfo;
	struct sockaddr_in peer;
	int error;
	struct pptpStartCtrlConnRequest request;
	int reuse = 1;

	if (sc->tunnel_sock < 0) {
		/* connect to peer */
		hostinfo = gethostbyname(param->server_name);
		if (hostinfo && (hostinfo->h_length == 4)) {
			memcpy(&sc->server_ip, hostinfo->h_addr_list[0], 4);
		}
		else {
			//PPTP_LOG("PPTP: error looking up %s", param->server_name);
			memset(&sc->server_ip, 0, 4);
#if 0//roy modify,2010/11/04
			return -1;
#else
//server ip is ip address
			inet_aton(param->server_name,&sc->server_ip);
#endif			
		}
		
		PPTP_LOG("PPTP: looking up server %s", param->server_name);
		
		if ((sc->tunnel_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			PPTP_LOG("PPTP_CTRLCONN: socket failed (%s)", strerror(errno));
			return -1;
		}
	
		if (setsockopt(sc->tunnel_sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
			PPTP_LOG("PPTP_CTRLCONN: setsockopt SO_REUSEPORT failed");
			goto err_out;
		}
	
		peer.sin_family = AF_INET;
		peer.sin_port = htons(PPTP_PORT);
		peer.sin_addr = sc->server_ip;
		error = connect(sc->tunnel_sock, (struct sockaddr *)&peer, sizeof(peer));
		if (error < 0 && error != EWOULDBLOCK) {
			PPTP_LOG("PPTP_CTRLCONN: connect failed (%s)", strerror(errno));
			goto err_out;
		}
	}
	if (sc->tunnel == NULL) {
		tunnel = (struct pptp_tunnel *)malloc(sizeof(struct pptp_tunnel));
		if (tunnel == NULL) {
			PPTP_LOG("PPTP_CTRLCONN: malloc");
			goto err_out;
		}
		sc->tunnel = tunnel;
	}
	else
		tunnel = sc->tunnel;

	memset(tunnel, 0, sizeof(*tunnel));
	tunnel->keepalive_id = 1;
	if (strlen(param->hostname) >= 64)
		strncpy(tunnel->host, param->hostname, sizeof(tunnel->host)-1);
	else
		strcpy(tunnel->host, param->hostname);

	request.header.length = htons(sizeof(struct pptpStartCtrlConnRequest));
	request.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
	request.header.magic = htonl(PPTP_MAGIC);
	request.header.type = htons(PPTP_StartControlConnRequest);
	request.header.resv0 = 0;
	request.vers = htons(PPTP_PROTO_VERS);
	request.resv0 = 0;
	request.frameCap = PPTP_FRAMECAP_ASYNC;
	request.bearCap = PPTP_BEARCAP_ANALOG;
	request.maxChan = 0;
	request.firmware = 0;
	memset(request.host, 0, sizeof(request.host));
	memcpy(request.host, tunnel->host, strlen(tunnel->host));
	memset(request.vendor, 0, sizeof(request.vendor));

	sc->tunnel_state = PPTP_TUNNEL_STATE_IDLE;

	if (pptp_tunnel_send(sc, &request, sizeof(request)) == 0) {
		sc->tunnel_state = PPTP_TUNNEL_STATE_WAITSTARTREPLY;
		sc->state_time = time(0) + PPTP_TIMER_INTERVAL;
	}
	else 
		goto err_out;

	first = 1;
	sc->state = PPTP_STATE_CONNECT;

	return 0;
err_out:
	if (sc->tunnel_sock >= 0)
		close(sc->tunnel_sock);
	if (sc->tunnel)
		free(sc->tunnel);
	sc->tunnel_sock = -1;
	sc->tunnel = NULL;
	return -1;
}

int
pptp_tunnel_stop(struct pptp_softc *sc, u_int8_t reason)
{
	struct pptp_tunnel *tunnel = sc->tunnel;
	struct pptpStopCtrlConnRequest request;

	request.header.length = htons(sizeof(struct pptpStopCtrlConnRequest));
	request.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
	request.header.magic = htonl(PPTP_MAGIC);
	request.header.type = htons(PPTP_StopControlConnRequest);
	request.header.resv0 = 0;

	request.reason = reason;
	request.resv0 = 0;
	request.resv1 = 0;

	if (sc->tunnel_state == PPTP_TUNNEL_STATE_IDLE ||
	    sc->tunnel_state == PPTP_TUNNEL_STATE_WAITSTOPREPLY) {
		return 0;
	}

	if (tunnel->call)
		pptp_tunnel_call_clear(sc);

	pptp_tunnel_send(sc, &request, sizeof(request));

	sc->tunnel_state = PPTP_TUNNEL_STATE_WAITSTOPREPLY;
	sc->state_time = time(0) + PPTP_TIMER_INTERVAL;
	return 0;
}

int
pptp_tunnel_disconnect(struct pptp_softc *sc)
{
	struct pptp_tunnel *tunnel = sc->tunnel;

	PPTP_LOG("%s", __FUNCTION__);

	if (tunnel) {
		if (tunnel->call)
			pptp_tunnel_call_clear(sc);

		if (tunnel->call)
			pptp_msleep(10);

		pptp_tunnel_stop(sc, PPTP_SCCR_REAS_LOCAL);

		pptp_msleep(10);

		if (tunnel->call)
			pptp_tunnel_call_release(sc);

		free(tunnel);
		sc->tunnel = NULL;
	}

	pptp_tunnel_gre_close(sc);

	if (sc->tunnel_sock >= 0) {
		close(sc->tunnel_sock);
		sc->tunnel_sock = -1;
	}

	return 0;
}

int
pptp_tunnel_connected(struct pptp_softc *sc)
{
	struct pptp_tunnel *tunnel = sc->tunnel;

	if (!tunnel->call)
		return 0;

	return (sc->call_state == PPTP_CALL_STATE_CONNECTED);
}

int
pptp_tunnel_read(struct pptp_softc *sc)
{
	struct pptp_tunnel *tunnel = sc->tunnel;
	int n;

	if (tunnel->read_len == PPTP_RX_BUFSIZ) {
		PPTP_LOG("PPTP_CTRLCONN: buffer full");
		return 0;
	}

	n = read(sc->tunnel_sock, pptp_read_buf + tunnel->read_len,
		PPTP_RX_BUFSIZ - tunnel->read_len);
	if (n <= 0) {
		sc->state = PPTP_STATE_CLOSE;
		return 0;
	}

	tunnel->read_len += n;
	return n;
}

static int
pptp_msg_len(u_int16_t type)
{
	int i;

	for (i = 0; ctrlmsg_table[i].type != -1; i++) {
		if (ctrlmsg_table[i].type == type)
			return ctrlmsg_table[i].len;
	}
	return 0;
}

int
pptp_tunnel_parse(struct pptp_softc *sc, char **buf)
{
	struct pptp_tunnel *tunnel = sc->tunnel;
	struct pptpMsgHead *header;
	int len;
	int skip_bytes = 0;

	while ((tunnel->read_len - skip_bytes) >= sizeof(struct pptpMsgHead)) {
		header = (struct pptpMsgHead *)(pptp_read_buf+skip_bytes);
		if (ntohl(header->magic) != PPTP_MAGIC) {
			skip_bytes++;
			continue;
		}
		if (ntohs(header->resv0) != 0) {
			skip_bytes++;
			continue;
		}
		if (ntohs(header->length) < sizeof(struct pptpMsgHead)) {
			skip_bytes++;
			continue;
		}
		if (ntohs(header->length) > (tunnel->read_len - skip_bytes)) {
			break;
		}
		if (ntohs(header->msgType) == PPTP_CTRL_MSG_TYPE) {
			int msg_len = pptp_msg_len(ntohs(header->type));

			if (ntohs(header->length) != msg_len) {
				skip_bytes++;
				continue;
			}
		}

		len = ntohs(header->length);
		*buf = (char *)malloc(len);
		if (*buf == NULL) {
			PPTP_LOG("pptp_tunnel_parse: malloc failed");
			return 0;
		}

		memcpy(*buf, pptp_read_buf+skip_bytes, len);
		tunnel->read_len -= (skip_bytes+len);
		memmove(pptp_read_buf, pptp_read_buf+skip_bytes+len, tunnel->read_len);
		return len;
	}

	tunnel->read_len -= skip_bytes;
	memmove(pptp_read_buf, pptp_read_buf+skip_bytes, tunnel->read_len);

	return 0;
}

static int
pptp_handle_StartControlConnReply(struct pptp_softc *sc, struct pptpMsgHead *header)
{
	struct pptpStartCtrlConnReply *reply = (struct pptpStartCtrlConnReply *)header;
	struct pptp_tunnel *tunnel = sc->tunnel;

	if (sc->tunnel_state != PPTP_TUNNEL_STATE_WAITSTARTREPLY)
		return 0;

	if (ntohs(reply->vers) != PPTP_PROTO_VERS)
		return PPTP_SCCR_REAS_PROTO;

	if (reply->result != 1)
		return PPTP_SCCR_REAS_PROTO;

	sc->tunnel_state = PPTP_TUNNEL_STATE_CONNECTED;

	tunnel->vers = ntohs(reply->vers);
	tunnel->firmware = ntohs(reply->firmware);
	memcpy(tunnel->host, reply->host, sizeof(tunnel->host));
	memcpy(tunnel->vendor, reply->vendor, sizeof(tunnel->vendor));

	sc->state_time = time(0) + PPTP_TIMER_INTERVAL;
	return 0;
}

static int
pptp_handle_StopControlConnRequest(struct pptp_softc *sc, struct pptpMsgHead *header)
{
	struct pptpStopCtrlConnReply reply;

	if (sc->tunnel_state == PPTP_TUNNEL_STATE_IDLE)
		return 0;

	reply.header.length = htons(sizeof(struct pptpStopCtrlConnReply));
	reply.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
	reply.header.magic = htonl(PPTP_MAGIC);
	reply.header.type = htons(PPTP_StopControlConnReply);
	reply.header.resv0 = 0;
	reply.result = 1;
	reply.err = 0;
	reply.resv0 = 0;
	if (pptp_tunnel_send(sc, &reply, sizeof(reply)) == 0)
		sc->tunnel_state = PPTP_TUNNEL_STATE_DESTROY;

	return 0;
}

static int
pptp_handle_StopControlConnReply(struct pptp_softc *sc, struct pptpMsgHead *header)
{
	if (sc->tunnel_state == PPTP_TUNNEL_STATE_IDLE)
		return 0;

	sc->tunnel_state = PPTP_TUNNEL_STATE_DESTROY;
	return 0;
}

static int
pptp_handle_EchoRequest(struct pptp_softc *sc, struct pptpMsgHead *header)
{
	struct pptpEchoRequest *request = (struct pptpEchoRequest *)header;
	struct pptpEchoReply reply;

	reply.header.length = htons(sizeof(struct pptpEchoReply));
	reply.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
	reply.header.magic = htonl(PPTP_MAGIC);
	reply.header.type = htons(PPTP_EchoReply);
	reply.header.resv0 = 0;
	reply.id = request->id;
	reply.result = 1;
	reply.err = 0;
	reply.resv0 = 0;

	pptp_tunnel_send(sc, &reply, sizeof(reply));
	return 0;
}

static int
pptp_handle_EchoReply(struct pptp_softc *sc, struct pptpMsgHead *header)
{
	struct pptpEchoReply *reply = (struct pptpEchoReply *)header;
	struct pptp_tunnel *tunnel = sc->tunnel;

	if (reply->id == tunnel->keepalive_id) {
		tunnel->keepalive_sent = 0;
	}
	return 0;
}

static int
pptp_handle_OutCallReply(struct pptp_softc *sc, struct pptpMsgHead *header)
{
	struct pptpOutCallReply *reply = (struct pptpOutCallReply *)header;
	struct pptp_tunnel *tunnel = sc->tunnel;
	struct pptp_call *call = tunnel->call;
	struct pptpSetLinkInfo link_info;

	if (ntohs(reply->peerCid) != call->call_id)
		return 0;

	if (sc->call_state != PPTP_CALL_STATE_WAITREPLY)
		return 0;

	if (reply->result != 1) {
		pptp_tunnel_call_release(sc);
		sc->call_state = PPTP_CALL_STATE_IDLE;
	}
	else {
		link_info.header.length = htons(sizeof(struct pptpSetLinkInfo));
		link_info.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
		link_info.header.magic = htonl(PPTP_MAGIC);
		link_info.header.type = htons(PPTP_SetLinkInfo);
		link_info.header.resv0 = 0;
		link_info.cid = reply->cid;
		link_info.resv0 = 0;
		link_info.sendAccm = 0xffffffff;
		link_info.recvAccm = 0xffffffff;

		pptp_tunnel_send(sc, &link_info, sizeof(link_info));

		sc->call_state = PPTP_CALL_STATE_CONNECTED;

		call->call_id = ntohs(reply->cid);
		call->speed = ntohl(reply->speed);

		sc->state_time = time(0) + PPTP_TIMER_INTERVAL;
	}
	return 0;
}

static int
pptp_handle_CallClearRequest(struct pptp_softc *sc, struct pptpMsgHead *header)
{
	struct pptpCallClearRequest *request = (struct pptpCallClearRequest *)header;
	struct pptp_tunnel *tunnel = sc->tunnel;
	struct pptp_call *call = tunnel->call;
	struct pptpCallDiscNotify notify;

	if (call->call_id == ntohs(request->cid)) {
		if (pptp_tunnel_hangup(sc) != 0)
			return 0;

		memset(&notify, 0, sizeof(notify));
		notify.header.length = htons(sizeof(struct pptpCallDiscNotify));
		notify.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
		notify.header.magic = htonl(PPTP_MAGIC);
		notify.header.type = htons(PPTP_CallDiscNotify);
		notify.header.resv0 = 0;
		notify.cid = request->cid;
		notify.result = 1;

		pptp_tunnel_send(sc, &notify, sizeof(notify));
		pptp_tunnel_call_release(sc);
	}
	return 0;
}

static int
pptp_handle_CallDiscNotify(struct pptp_softc *sc, struct pptpMsgHead *header)
{
	struct pptpCallDiscNotify *notify = (struct pptpCallDiscNotify *)header;
	struct pptp_tunnel *tunnel = sc->tunnel;
	struct pptp_call *call = tunnel->call;

	if (call->call_id == ntohs(notify->cid)) {
		if (sc->call_state == PPTP_CALL_STATE_WAITDISC)
			pptp_tunnel_call_release(sc);
		else
			pptp_tunnel_hangup(sc);
	}
	return 0;
}

static int
pptp_handle_SetLinkInfo(struct pptp_softc *sc, struct pptpMsgHead *header)
{
	struct pptpSetLinkInfo *packet = (struct pptpSetLinkInfo *)header;
	struct pptp_tunnel *tunnel = sc->tunnel;
	struct pptp_call *call = tunnel->call;
	struct pptpSetLinkInfo linfo;

	if (ntohs(packet->cid) != call->peer_call_id)
		return 0;

	if (ntohl(packet->sendAccm) == 0 && ntohl(packet->recvAccm) == 0)
		return 0;

	linfo.header.length = htons(sizeof(struct pptpSetLinkInfo));
	linfo.header.msgType = htons(PPTP_CTRL_MSG_TYPE);
	linfo.header.magic = htonl(PPTP_MAGIC);
	linfo.header.type = htons(PPTP_SetLinkInfo);
	linfo.header.resv0 = 0;
	linfo.cid = htons(call->call_id);
	linfo.resv0 = 0;
	linfo.sendAccm = 0xffffffff;
	linfo.recvAccm = 0xffffffff;

	pptp_tunnel_send(sc, &linfo, sizeof(linfo));
	return 0;
}


int
pptp_tunnel_handler(struct pptp_softc *sc, char *buf, int len)
{
	struct pptpMsgHead *header = (struct pptpMsgHead *)buf;
	u_int8_t error = 0;
	int i = 0;
	u_int16_t type = ntohs(header->type);

	if (ntohs(header->msgType) != PPTP_CTRL_MSG_TYPE) {
		return -1;
	}

	for (i = 0; ctrlmsg_table[i].type != -1; i++) {
		if (ctrlmsg_table[i].type == type && 
			ctrlmsg_table[i].handler_fun) {
			error = ctrlmsg_table[i].handler_fun(sc, header);
			break;
		}
	}

	if (error) {
		pptp_tunnel_stop(sc, error);
		return -1;
	}

	return 0;
}

int
pptp_tunnel_process(struct pptp_softc *sc)
{
	char *buf;
	int len;

	pptp_tunnel_read(sc);

	while ((len = pptp_tunnel_parse(sc, &buf)) != 0) {
		pptp_tunnel_handler(sc, buf, len);
		free(buf);
	}
	return 0;
}

int
pptp_tunnel_open(struct pptp_softc *sc)
{
	fd_set fdset;
	int max_fd;
	struct timeval tv = {1, 0};
	int n;

	if (!pptp_tunnel_connected(sc)) {
		FD_ZERO(&fdset);
		FD_SET(sc->tunnel_sock, &fdset);
		max_fd = sc->tunnel_sock + 1;

		n = select(max_fd, &fdset, 0, 0, &tv);
		if (n < 0) {
			pptp_tunnel_disconnect(sc);
			sc->state = PPTP_STATE_CLOSE;
			return -1;
		}
		if (n == 0)
			return 0;

		if (FD_ISSET(sc->tunnel_sock, &fdset)) {
			pptp_tunnel_process(sc);
			FD_CLR(sc->tunnel_sock, &fdset);
		}

		if (first && sc->tunnel_state == PPTP_TUNNEL_STATE_CONNECTED) {
			if (pptp_tunnel_call(sc) == 0)
				first = 0;
		}
	}
	else {
		/* Complete the outcall reply */
		sc->call_id = sc->tunnel->call->call_id;
		sc->peer_call_id = sc->tunnel->call->peer_call_id;

		sc->state = PPTP_STATE_CALL_CONNECTED;
	}

	return 0;
}

int
pptp_tunnel_gre_open(struct pptp_softc *sc)
{
	struct pptp_param *param = &sc->param;
	struct ifpptpreq req;

	/* Attach the pptp gre kernel structure */
	memset(&req, 0, sizeof(req));
	strcpy(req.pppname, param->pppname);
	req.tunnel_ip = sc->tunnel_ip;
	req.server_ip = sc->server_ip;
	req.call_id = sc->call_id;
	req.peer_call_id = sc->peer_call_id;

	ioctl(sc->devfd, PPPIOCPPTPCONN, &req);

	/* Enter GRE state */
	PPTP_LOG("PPTP_STATE_GRE");
	sc->state = PPTP_STATE_GRE;
	return 0;
}

int
pptp_tunnel_gre_close(struct pptp_softc *sc)
{
	struct ifpptpreq req;

	/* Detach the pptp gre from kernel structure */
	memset(&req, 0, sizeof(req));
	strcpy(req.pppname, sc->param.pppname);
	
	ioctl(sc->devfd, PPPIOCPPTPCONN, &req);

	/* Enter close state */
	sc->state = PPTP_STATE_CLOSE;
	return 0;
}

int
pptp_tunnel_hangup(struct pptp_softc *sc)
{
	sc->state = PPTP_STATE_CLOSE;
	return 0;
}

/*
 * The followings are pptp fsm functions to
 * hooke to a generic pptp daemon model
 */
/* Timeout function */
void
pptp_fsm_timer(struct pptp_softc *sc)
{
	time_t now = time(0);

	if ((now - sc->state_time) < 0)
		return;

	if (sc->tunnel) {
		/* Check ctrl state */
		if (sc->tunnel_state == PPTP_TUNNEL_STATE_WAITSTOPREPLY) {
			sc->tunnel_state = PPTP_TUNNEL_STATE_DESTROY;
		}
		else if (sc->tunnel_state != PPTP_TUNNEL_STATE_CONNECTED) {
			pptp_tunnel_stop(sc, PPTP_SCCR_REAS_NONE);
		}

		/* Check ctrl echo */
		if (sc->tunnel && sc->tunnel->keepalive_sent > 2) {
			pptp_tunnel_stop(sc, PPTP_SCCR_REAS_NONE);
		}
		else {
			pptp_tunnel_keepalive(sc);
		}

		/* Check call state */
		if (sc->call_state == PPTP_CALL_STATE_WAITREPLY) {
			pptp_tunnel_call_clear(sc);
		}
		else if (sc->call_state == PPTP_CALL_STATE_WAITDISC) {
			pptp_tunnel_call_release(sc);
		}
	}

	/* Reschedule time */
	sc->state_time = now + PPTP_TIMER_INTERVAL;
	return;
}

/* State machine here */
int
pptp_fsm(struct pptp_softc *sc)
{
	fd_set fds;
	int maxfd;
	struct timeval tv = {1, 0};
	int n;
	int flags;
	char rcvbuf[64];
	struct in_addr tunnel_ip;

	switch (sc->state) {
	case PPTP_STATE_NONE:
		pptp_msleep(200);
		break;

	case PPTP_STATE_WAIT:
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
				len = read(sc->devfd, rcvbuf, sizeof(rcvbuf));
				if (len >= sizeof("DIALUP") &&
					memcmp(rcvbuf, "DIALUP", sizeof("DIALUP")) == 0) {
					sc->state = PPTP_STATE_INIT;
				}
			}
		}
		break;

	case PPTP_STATE_INIT:
		/* Make sure tunnel ip got */
		tunnel_ip.s_addr = 0;
		if (pptp_osl_ifaddr(sc->param.tunnel_ifname, &tunnel_ip) != 0 ||
		    tunnel_ip.s_addr == 0) {
			/* Stick in this state, until got */
			pptp_msleep(200);
			break;
		}

		sc->tunnel_ip = tunnel_ip;

		if (pptp_tunnel_connect(sc) != 0)
			sc->state = PPTP_STATE_CLOSE;

		pptp_msleep(1000);
		break;

	case PPTP_STATE_CONNECT:
		pptp_tunnel_open(sc);
		break;

	case PPTP_STATE_CALL_CONNECTED:
		pptp_tunnel_gre_open(sc);
		break;

	case PPTP_STATE_GRE:
		/* Should start the PPP task */
		sc->ppp_pid = pptp_osl_ppp_open(sc);
		sc->state = PPTP_STATE_PPP;

		/* Wait until interface up */
		if (sc->ppp_pid != 0) {
			while ((get_ifflags(sc->param.pppname) & IFF_UP) == 0) {
				/* check thread status of ppp */
				if (pptp_osl_ppp_state(sc) == 0){
					sc->state = PPTP_STATE_CLOSE;
					break;
				}
				pptp_msleep(100);
			}
		}
		break;

	case PPTP_STATE_PPP:
		if (sc->tunnel_state == PPTP_TUNNEL_STATE_DESTROY ||
		    sc->tunnel_sock == -1) {
			sc->state = PPTP_STATE_CLOSE;
			break;
		}
		else {
			flags = get_ifflags(sc->param.pppname);
			if ((flags & IFF_UP) == 0) {
				sc->state = PPTP_STATE_CLOSE;
				break;
			}
		}

		/* handle ctrlconn and pptp_gre sockets */
		FD_ZERO(&fds);
		FD_SET(sc->tunnel_sock, &fds);
		maxfd = sc->tunnel_sock + 1;

		n = select(maxfd, &fds, 0, 0, &tv);
		if (n > 0) {
			/* pptp_ctrlconn data */
			if (FD_ISSET(sc->tunnel_sock, &fds)) {
				pptp_tunnel_process(sc);
				FD_CLR(sc->tunnel_sock, &fds);
			}
		}
		break;

	case PPTP_STATE_CLOSE:
	default:
		/* Disconnect this session */
		pptp_fsm_down(sc);
		return -1;
	}

	return 0;
}

void
pptp_fsm_handler(struct pptp_softc *sc)
{
	pptp_fsm(sc);
	pptp_fsm_timer(sc);
}

int
pptp_fsm_up(struct pptp_softc *sc)
{
	struct ifpptpreq req;
	char dev[128] = {0};

	PPTP_LOG("PPTP fsm up");

	if (sc->devfd < 0) {
		sprintf(dev, "/dev/net/pptp/%s", sc->param.pppname);
		sc->devfd = open(dev, O_RDWR);
		if (sc->devfd < 0) {
			PPTP_LOG("Can't open %s!", dev);
			return -1;
		}
	}

	sc->state_time = time(0);

	/* Demand wait or start connection */
	if (sc->param.demand && (sc->flag & PPTP_RECONNECT) == 0) {
		/* Starting from wait dialup */
		req.wait = 1;
		ioctl(sc->devfd, PPPIOCPPTPWAIT, &req);
		sc->state = PPTP_STATE_WAIT;
	}
	else {
		sc->state = PPTP_STATE_INIT;
	}

	return 0;
}

void
pptp_fsm_down(struct pptp_softc *sc)
{
	struct ifpptpreq req;

	PPTP_LOG("PPTP fsm down");

	if (sc->devfd >= 0) {
		/* Shutdown ppp process */
		pptp_osl_ppp_close(sc);

		/* Close pptp ctrl and gre */
		pptp_tunnel_disconnect(sc);

		/* Close dialup wait */
		req.wait = 0;
		ioctl(sc->devfd, PPPIOCPPTPWAIT, &req);

		close(sc->devfd);
		sc->devfd = -1;
	}

	/* Enter NONE state */
	sc->state = PPTP_STATE_NONE;

	/* Re-initializing if possible */
	if ((sc->flag & (PPTP_SHUTDOWN | PPTP_DISCONNECT)) == 0 &&
	    (sc->param.demand || sc->param.keepalive)) {
		pptp_fsm_up(sc);
	}
}
