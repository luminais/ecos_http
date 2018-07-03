/*
 * L2 tunnel functions
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: tunnel.c,v 1.15 2010-08-30 10:12:13 Exp $
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <l2tp_var.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/md5.h>

struct msg_table
{
	char *msg;
	int type;
	void (*handler_fun) __P((l2tp_tunnel *));
};

static uint16_t l2tp_id = 1;
extern int gethostname(char *name, size_t len);
static void lt_HandleSCCRP(l2tp_tunnel *tunnel);
static void lt_HandleStopCCN(l2tp_tunnel *tunnel);

static struct msg_table ctrlmsg_table[] =
{
	{"SCCRP",   CTRL_MSG_SCCRP,   lt_HandleSCCRP},
	{"StopCCN", CTRL_MSG_StopCCN, lt_HandleStopCCN},
	{"ICRP",    CTRL_MSG_ICRP,    ls_HandleICRP},
	{"CDN",     CTRL_MSG_CDN,     ls_HandleCDN},
	{"",        -1,               NULL}
};

static int
lt_SeqCheck(uint16_t a, uint16_t b, int flag)
{
	if (flag == 0)
		return ((((a) > (b) && (a) - (b) < 32768) || ((a) < (b) && (b) - (a) > 32768)));
	else
		return ((((a) < (b) && (b) - (a) < 32768) || ((a) > (b) && (a) - (b) > 32768)));
}

static void
lt_AuthGenResponse(uint16_t msg_type, char const *secret,
	uint8_t const *challenge, size_t chal_len, uint8_t buf[16])
{
	MD5_CTX ctx;
	uint8_t id = (uint8_t)msg_type;

	MD5Init(&ctx);
	MD5Update(&ctx, &id, 1);
	MD5Update(&ctx, (uint8_t *) secret, strlen(secret));
	MD5Update(&ctx, challenge, chal_len);
	MD5Final(buf, &ctx);

	L2TP_LOG("auth_gen_response(secret=%s) -> "
		"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
		secret,
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
		buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
}

void
lt_TxPacketInit(struct l2tphdr *l2tp, uint16_t msg_type, uint16_t tid, uint16_t sid)
{
	uint16_t type;

	memset(l2tp, 0, sizeof(*l2tp));

	l2tp->version = 2;
	l2tp->bits = TBIT | LBIT | SBIT;
	l2tp->tid = tid;
	l2tp->sid = sid;
	if (msg_type != CTRL_MSG_ZLB) {
		type = htons(msg_type);
		la_AddAvp(l2tp->avp, AVP_MESSAGE_TYPE, sizeof(type), &type);
	}
	return;
}

static int
lt_Send(int sock, struct l2tphdr *l2tp, struct sockaddr_in const *to)
{
	socklen_t len = sizeof(struct sockaddr_in);
	uint16_t total_len = l2tp->length;

	if (LENGTH_BIT(l2tp->bits))
		l2tp->length = htons(l2tp->length);

	l2tp->tid = htons(l2tp->tid);
	l2tp->sid = htons(l2tp->sid);
	if (SEQUENCE_BIT(l2tp->bits)) {
		l2tp->Ns = htons(l2tp->Ns);
		l2tp->Nr = htons(l2tp->Nr);
	}

	return sendto(sock, l2tp, total_len, 0, (struct sockaddr const *)to, len);
}

/* Send a zero legnth packet out */
void
lt_SendZLB(l2tp_tunnel *tunnel)
{
	struct l2tphdr *l2tp = &tunnel->txpkt;

	lt_TxPacketInit(l2tp, CTRL_MSG_ZLB, tunnel->ptid, 0);

	l2tp->Nr = tunnel->Nr;
	l2tp->Ns = tunnel->Ns;
	l2tp->length = L2TPHDR_LEN;
	lt_Send(tunnel->sock, l2tp, &tunnel->peer_addr);
}

/*
 * Sends a control message.
 */
void
lt_SendCtlMessage(l2tp_tunnel *tunnel)
{
	struct l2tphdr *l2tp = &tunnel->txpkt;

	l2tp->length = CTRL_HDR_LEN + la_AvpLen(l2tp->avp);

	/* Update next sequence number to send */
	l2tp->Ns = tunnel->Ns;
	tunnel->Ns++;

	/* Update next sequence number expected to receive */
	l2tp->Nr = tunnel->Nr;

	lt_Send(tunnel->sock, l2tp, &tunnel->peer_addr);
}

/* Send Hello */
void
lt_SendHello(l2tp_tunnel *tunnel)
{
	lt_TxPacketInit(&tunnel->txpkt, CTRL_MSG_HELLO, tunnel->ptid, 0);
	lt_SendCtlMessage(tunnel);
}

/* Send Start-Control-Connection-Request */
static int
lt_SendSCCRQ(l2tp_tunnel *tunnel)
{
	struct l2tphdr *l2tp = &tunnel->txpkt;
	char *avp = l2tp->avp;

	uint16_t sval;
	uint32_t wval;
	uint8_t challenge[16];
	uint16_t len;

	/* Setup tx packet */
	lt_TxPacketInit(l2tp, CTRL_MSG_SCCRQ, 0, 0);

	/* Add the AVP's */
	/* Protocol version */
	sval = htons(0x0100);
	la_AddAvp(avp, AVP_PROTOCOL_VERSION, sizeof(sval), &sval);

	/* Framing capabilities -- sync framing */
	wval = htonl(1);
	la_AddAvp(avp, AVP_FRAMING_CAPABILITIES, sizeof(wval), &wval);

	/* Bearer capabilities */
	wval = htons(0);
	la_AddAvp(avp, AVP_BEARER_CAPABILITIES, sizeof(wval), &wval);

	/* Firmware version */
	sval = htons(1);
	la_AddAvp(avp, AVP_FIRMWARE_REVISION, sizeof(sval), &sval);

	/* Host name */
	len = strlen(tunnel->hostname);
	la_AddAvp(avp, AVP_HOST_NAME, len, tunnel->hostname);

	/* Vendor name */
	la_AddAvp(avp, AVP_VENDOR_NAME, strlen(VENDOR_STR), VENDOR_STR);

	/* Assigned tunnel ID */
	sval = htons(tunnel->tid);
	la_AddAvp(avp, AVP_ASSIGNED_TUNNEL_ID, sizeof(sval), &sval);

	/* Receive window size */
	sval = htons(tunnel->rcv_win_size);
	la_AddAvp(avp, AVP_RECEIVE_WINDOW_SIZE, sizeof(sval), &sval);

	/* Challenge */
	if (tunnel->secret_len) {
		char *buf = challenge;
		size_t size = sizeof(challenge);

		while (size--)
			*buf++ = random() & 0xFF;

		la_AddAvp(avp, AVP_CHALLENGE, size, challenge);

		/* Compute and save expected response */
		lt_AuthGenResponse(CTRL_MSG_SCCRP, tunnel->secret,
			challenge, sizeof(challenge), tunnel->expected_response);
	}

	lt_SendCtlMessage(tunnel);
	return 1;
}

/* Start-Control-Connection-Connected */
static int
lt_SendSCCCN(l2tp_tunnel *tunnel)
{
	struct l2tphdr *l2tp = &tunnel->txpkt;
	char *avp = l2tp->avp;

	/* Send Start-Control-Connection-Connected */
	lt_TxPacketInit(l2tp, CTRL_MSG_SCCCN, tunnel->ptid, 0);

	/* Add response */
	if (tunnel->secret_len) {
		la_AddAvp(avp, AVP_CHALLENGE_RESPONSE, MD5LEN, tunnel->response);
	}

	lt_SendCtlMessage(tunnel);
	return 1;
}

/* Send Stop-Control-Connection-Notification to peer */
void
lt_SendStopCCN(l2tp_tunnel *tunnel, int result_code, int error_code)
{
	struct l2tphdr *l2tp = &tunnel->txpkt;
	char *avp = l2tp->avp;
	char buf[256];
	uint16_t u16;
	uint16_t len;

	/* Build the buffer for the result-code AVP */
	buf[0] = result_code / 256;
	buf[1] = result_code & 255;
	buf[2] = error_code / 256;
	buf[3] = error_code & 255;

	sprintf(buf+4, ERRMSG);
	buf[255] = 0;

	L2TP_LOG("lt_SendStopCCN(%d/%d, %d, %d, %s)\n",
	         (int)tunnel->tid, (int)tunnel->ptid, result_code, error_code, ERRMSG);

	len = 4 + strlen(buf+4);

	/* Init the control header */
	lt_TxPacketInit(l2tp, CTRL_MSG_StopCCN, tunnel->ptid, 0);

	/* Add assigned tunnel ID */
	u16 = htons(tunnel->tid);
	la_AddAvp(avp, AVP_ASSIGNED_TUNNEL_ID, sizeof(u16), &u16);

	/* Add result code */
	la_AddAvp(avp, AVP_RESULT_CODE, len, buf);

	/* Change state */
	tunnel->state = TUNNEL_SENT_STOP_CCN;

	lt_SendCtlMessage(tunnel);
}

/* Extract control connection management AVPs base on the SCCCN received */
static int
lt_ExtractAvps(l2tp_tunnel *tunnel)
{
	struct l2tphdr *l2tp = (struct l2tphdr *)&tunnel->rxpkt; 
	struct l2tp_avphdr *avphdr;
	char *data;
	int length;
	int mandatory;
	int found_response = 0;
	uint16_t u16;

	/* Extract AVP's */
	while (1) {
		if (tunnel->rx_avpoff >= l2tp->length)
			return 0;

		if ((avphdr = la_HandleAvp(tunnel)) == NULL) {
			lt_SendStopCCN(tunnel, RESULT_GENERAL_ERROR, ERROR_BAD_VALUE);
			return -1;
		}

		length = ALENGTH(avphdr->length);
		mandatory = AMBIT(avphdr->length)? 1 : 0;
		data = (char *)(avphdr+1);

		/* Unknown vendor?  Ignore, unless mandatory */
		if (avphdr->vendorid != VENDOR_IETF) {
			if (!mandatory)
				continue;

			L2TP_SET_ERRMSG(
				"Unknown mandatory attribute (vendor=%d, type=%d) in %s",
				(int)avphdr->vendorid,
				(int)avphdr->type,
				ctrlmsg_table[tunnel->rx_msg_type].msg);

			lt_SendStopCCN(tunnel, RESULT_GENERAL_ERROR,
				ERROR_UNKNOWN_AVP_WITH_M_BIT);
			return -1;
		}

		switch (avphdr->type) {
		case AVP_PROTOCOL_VERSION:
			memcpy(&u16, data, sizeof(u16));
			if (htons(u16) != 0x0100) {
				L2TP_SET_ERRMSG("Unsupported protocol version");
				lt_SendStopCCN(tunnel, RESULT_UNSUPPORTED_VERSION, 0x0100);
				return -1;
			}
			break;

		case AVP_HOST_NAME:
			if (length >= MAX_HOSTNAME)
				length = MAX_HOSTNAME-1;

			memcpy(tunnel->peer_hostname, data, length);
			tunnel->peer_hostname[length+1] = 0;
			break;

		case AVP_ASSIGNED_TUNNEL_ID:
			memcpy(&u16, data, sizeof(u16));
			tunnel->ptid = htons(u16);
			if (!tunnel->ptid) {
				L2TP_LOG("Invalid assigned-tunnel-ID of zero");
				tunnel->session.state = SESSION_IDLE;
				return -1;
			}
			break;

		case AVP_RECEIVE_WINDOW_SIZE:
			memcpy(&u16, data, sizeof(u16));
			tunnel->peer_rcv_win_size = htons(u16);
			break;

		case AVP_CHALLENGE:
			if (tunnel->secret_len) {
				lt_AuthGenResponse(((tunnel->rx_msg_type == CTRL_MSG_SCCRQ) ?
					CTRL_MSG_SCCRP : CTRL_MSG_SCCCN),
					tunnel->secret, data, length, tunnel->response);
			}
			break;

		case AVP_CHALLENGE_RESPONSE:
			if (tunnel->secret_len) {
				if (memcmp(data, tunnel->expected_response, MD5LEN)) {
					L2TP_SET_ERRMSG("Incorrect challenge response");
					lt_SendStopCCN(tunnel, RESULT_NOAUTH, ERROR_BAD_VALUE);
					return -1;
				}
			}
			found_response = 1;
			break;

		case AVP_TIE_BREAKER:
		case AVP_FRAMING_CAPABILITIES:
		case AVP_BEARER_CAPABILITIES:
		case AVP_FIRMWARE_REVISION:
		case AVP_VENDOR_NAME:
		default:
			break;
		}
	}

	if (tunnel->secret_len && !found_response &&
	    tunnel->rx_msg_type != CTRL_MSG_SCCRQ) {
		L2TP_SET_ERRMSG("Missing challenge-response");
		lt_SendStopCCN(tunnel, RESULT_NOAUTH, 0);
		return -1;
	}

	return 0;
}

/* Handle Start-Control-Connection-Reply */
static void
lt_HandleSCCRP(l2tp_tunnel *tunnel)
{
	/* If we don't send SCCRQ, reject this package */
	if (tunnel->state != TUNNEL_WAIT_CTL_REPLY) {
		L2TP_SET_ERRMSG("Not expecting SCCRP");
		lt_SendStopCCN(tunnel, RESULT_FSM_ERROR, 0);
		return;
	}

	if (lt_ExtractAvps(tunnel) < 0)
		return;

	tunnel->state = TUNNEL_ESTABLISHED;

	/* Send Start-Control-Connection-Connected */
	lt_SendSCCCN(tunnel);

	/* Tell sessions tunnel has been established */
	ls_SessionOpen(tunnel);
}

/* Handle the Stop-Control-Connection-Notification from peer */
static void
lt_HandleStopCCN(l2tp_tunnel *tunnel)
{
	uint16_t tid;
	struct l2tp_avphdr *avphdr;

	/* Reset the session */
	tunnel->session.state = SESSION_IDLE;

	tunnel->state = TUNNEL_RECEIVED_STOP_CCN;

	/* Parse the AVP's */
	while (1) {
		if ((avphdr = la_HandleAvp(tunnel)) == NULL)
			break;

		if (avphdr->vendorid != VENDOR_IETF ||
		    avphdr->type != AVP_ASSIGNED_TUNNEL_ID) {
			continue;
		}

		if (ALENGTH(avphdr->length) == sizeof(*avphdr) + sizeof(tid)) {
			char *data = (char *)(avphdr + 1);

			memcpy(&tid, data, sizeof(tid));
			tunnel->ptid = ntohs(tid);
		}
	}
}

/*
 * Function to handle the l2tp packet
 * received from l2tp socket.
 */
static void
lt_HandleMessage(l2tp_tunnel *tunnel, struct sockaddr_in *from)
{
	int i;
	struct l2tphdr *l2tp = &tunnel->rxpkt;

	/* We're LAC, reject SCCRQ */
	if (tunnel->rx_msg_type == CTRL_MSG_SCCRQ) {
		L2TP_LOG("%s receive CTRL_MSG_SCCRQ\n", __FUNCTION__);
		return;
	}

	if (l2tp->tid == 0) {
		L2TP_LOG("Invalid control message - tunnel ID = 0");
		return;
	}

	/* Verify that source address is the tunnel's peer */
	if (from->sin_addr.s_addr != tunnel->peer_addr.sin_addr.s_addr) {
		L2TP_LOG("Invalid control message for tunnel %d%d - not sent from peer",
			(int)tunnel->tid, (int)tunnel->ptid);
		return;
	}

	/* Set port for replies */
	tunnel->peer_addr.sin_port = from->sin_port;

	/* If it's an old datagram, ignore it */
	if (l2tp->Ns != tunnel->Nr)
		return;

	/* Do not increment if we got ZLB */
	if (tunnel->rx_msg_type != CTRL_MSG_ZLB) {
		tunnel->ack_time = time(0);
		tunnel->Nr++;
	}
	tunnel->hello_timeout = time(0);

	/* Update peer_Nr */
	if (lt_SeqCheck(l2tp->Nr, tunnel->peer_Nr, 0))
		tunnel->peer_Nr = l2tp->Nr;

	/* Run handler */
	for (i = 0; ctrlmsg_table[i].handler_fun; i++) {
		if (ctrlmsg_table[i].type == tunnel->rx_msg_type) {
			(*ctrlmsg_table[i].handler_fun)(tunnel);
			break;
		}
	}

	if (tunnel->state == TUNNEL_SENT_STOP_CCN)
		tunnel->session.state = SESSION_IDLE;
}

/* Called when a packet arrives on the UDP socket */
void
lt_Read(l2tp_tunnel *tunnel, int fd)
{
	struct sockaddr_in from;
	socklen_t socklen = sizeof(struct sockaddr_in);
	int len;
	struct l2tphdr *l2tp = &tunnel->rxpkt;
	struct l2tp_avphdr *avphdr;
	char *data;
	uint16_t msg_type;

	len = recvfrom(fd, l2tp, sizeof(*l2tp), 0, (struct sockaddr *)&from, &socklen);
	if (len <= CTRL_HDR_LEN)
		return;

	/* Handle control frame */
	if (TYPE_BIT(l2tp->bits) == 0 ||
		LENGTH_BIT(l2tp->bits) == 0 ||
		SEQUENCE_BIT(l2tp->bits) == 0 ||
		OFFSET_BIT(l2tp->bits) != 0 ||
		PRIORITY_BIT(l2tp->bits) != 0)
		return;

	/* Check version, only support L2TP (ver = 2) */
	if (VERSION_MASK(l2tp->version) != 2)
		return;

	/*
	* Fix the byte order of the header
	*/
	l2tp->length = ntohs(l2tp->length);
	l2tp->tid = ntohs(l2tp->tid);
	l2tp->sid = ntohs(l2tp->sid);
	l2tp->Ns = ntohs(l2tp->Ns);
	l2tp->Nr = ntohs(l2tp->Nr);

	if (l2tp->length > len)
		return;

	/* Reset receive avp offset */
	tunnel->rx_avpoff = L2TPHDR_LEN;

	if (l2tp->length == L2TPHDR_LEN) {
		tunnel->rx_msg_type = CTRL_MSG_ZLB;
	}
	else {
		if ((avphdr = la_HandleAvp(tunnel)) == NULL)
			return;

		if (!AMBIT(avphdr->length) ||
		    AHBIT(avphdr->length) ||
		    ALENGTH(avphdr->length) != sizeof(*avphdr)+sizeof(msg_type) ||
		    avphdr->type != AVP_MESSAGE_TYPE ||
		    avphdr->vendorid != VENDOR_IETF) {
			return;
		}

		data = (char *)(avphdr + 1);

		memcpy(&msg_type, data, sizeof(msg_type));
		tunnel->rx_msg_type = ntohs(msg_type);
	}

	/* Handle control packet */
	lt_HandleMessage(tunnel, &from);
}

void lt_new_tunnel(l2tp_tunnel *tunnel)
{
	/* Setup parameters */
	tunnel->rcv_win_size = 4;
	tunnel->peer_rcv_win_size = 1;

	tunnel->tid = l2tp_id++;
	tunnel->ptid = 0;
	tunnel->Ns = 0;
	tunnel->Nr = 0;
	gethostname(tunnel->hostname, MAX_HOSTNAME);

	tunnel->hello_timeout = time(0);
	tunnel->ack_time = 0;
}

/*
 * Send Start-Control-Connection-Request to peer
 * while creating.
 */
void
lt_TunnelCreate(l2tp_tunnel *tunnel)
{
	lt_SendSCCRQ(tunnel);
	tunnel->state = TUNNEL_WAIT_CTL_REPLY;
}

/*
 * Send stop notification to peer, if
 * the tunnel does not enter the STOP
 * state.
 */
void
lt_StopTunnel(l2tp_tunnel *tunnel, char const *reason)
{
	if (tunnel->state != TUNNEL_RECEIVED_STOP_CCN &&
	    tunnel->state != TUNNEL_SENT_STOP_CCN) {
		L2TP_SET_ERRMSG(reason);
		lt_SendStopCCN(tunnel, RESULT_SHUTTING_DOWN, 0);
	}
}
