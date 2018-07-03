/*
 * L2TP session implementation.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: session.c,v 1.9 2010-07-18 08:58:14 Exp $
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
#include <stdio.h>
#include <stdlib.h>

static uint32_t call_serial_number = 0;


/* Send Incoming-Call-Request */
void
ls_SendICRQ(l2tp_tunnel *tunnel)
{
	l2tp_session *sess = &tunnel->session;
	struct l2tphdr *l2tp = &tunnel->txpkt;
	char *avp = l2tp->avp;
	uint16_t sval;
	uint32_t val;

	/* Build the packet */
	lt_TxPacketInit(l2tp, CTRL_MSG_ICRQ, tunnel->ptid, 0);

	/* assigned session ID */
	sval = htons(sess->sid);
	la_AddAvp(avp, AVP_ASSIGNED_SESSION_ID, sizeof(sval), &sval);

	/* Call serial number */
	val = htonl(call_serial_number);
	call_serial_number++;
	la_AddAvp(avp, AVP_CALL_SERIAL_NUMBER, sizeof(val), &val);

	lt_SendCtlMessage(tunnel);
}

/* Send Incoming-Call-Connected */
void
ls_SendICCN(l2tp_tunnel *tunnel)
{
	l2tp_session *sess = &tunnel->session;
	struct l2tphdr *l2tp = &tunnel->txpkt;
	char *avp = l2tp->avp;
	uint32_t u32;

	/* Build the packet */
	lt_TxPacketInit(l2tp, CTRL_MSG_ICCN, tunnel->ptid, sess->psid);

	/* Connect speed */
	u32 = htonl(10000000);
	la_AddAvp(avp, AVP_TX_CONNECT_SPEED, sizeof(u32), &u32);

	/* Framing Type */
	u32 = htonl(1);
	la_AddAvp(avp, AVP_FRAMING_TYPE, sizeof(u32), &u32);

	lt_SendCtlMessage(tunnel);
}

/* Send Call-Disconnect-Notify */
void
ls_SendCDN(l2tp_tunnel *tunnel, uint16_t result_code, uint16_t error_code, char *msg)
{
	l2tp_session *sess = &tunnel->session;
	struct l2tphdr *l2tp = &tunnel->txpkt;
	char *avp = l2tp->avp;

	struct l2tp_avp_result_code *code;
	char buf[256];
	uint16_t len;
	uint16_t sval;

	/* Build the buffer for the result-code AVP */
	code = (struct l2tp_avp_result_code *)buf;
	code->result_code = htons(result_code);
	code->error_code = htons(error_code);

	sprintf(buf+4, msg);
	buf[255] = 0;

	L2TP_LOG("session_send_CDN(%d/%d, %d/%d): %s\n",
		(int)tunnel->tid, (int)tunnel->ptid,
		(int)sess->sid, (int)sess->psid, buf+4);
	len = 4 + strlen(buf+4);

	/* Build the packet */
	lt_TxPacketInit(l2tp, CTRL_MSG_CDN, tunnel->ptid, sess->psid);

	/* Add assigned session ID */
	sval = htons(sess->sid);
	la_AddAvp(avp, AVP_ASSIGNED_SESSION_ID, sizeof(sval), &sval);

	/* Add result code */
	la_AddAvp(avp, AVP_RESULT_CODE, len, buf);

	/* Clean up */
	sess->state = SESSION_IDLE;

	lt_SendCtlMessage(tunnel);

	/* Close session */
	ls_SessionClose(tunnel);
}

void
ls_new_session(l2tp_session *sess)
{
	static uint16_t id = 1;

	sess->tunnel = (l2tp_tunnel *)sess;
	sess->sid = id++;
	sess->psid = 0;
	sess->state = SESSION_WAIT_TUNNEL;
	return;
}

/* Open the session */
void
ls_SessionOpen(l2tp_tunnel *tunnel)
{
	l2tp_session *sess = &tunnel->session;


	if (sess->state != SESSION_WAIT_TUNNEL)
		return;

	L2TP_LOG("Session (%d/%d, %d/%d) tunnel open\n",
		(int)tunnel->tid, (int)tunnel->ptid,
		(int)sess->sid, (int)sess->psid);

	/* Send Incoming-Call-Request while session opening */
	ls_SendICRQ(tunnel);

	/* Waiting Incoming-Call-Reply */
	sess->state = SESSION_WAIT_REPLY;
}

/* Close the session */
void
ls_SessionClose(l2tp_tunnel *tunnel)
{
	l2tp_session *sess = &tunnel->session;

	/* Release this session */
	sess->state = SESSION_IDLE;

	/* Tear down tunnel */
	L2TP_SET_ERRMSG("Last session has closed");
	lt_SendStopCCN(tunnel, RESULT_GENERAL_REQUEST, 0);
}

/* Handle Call-Disconnect-Notify */
void
ls_HandleCDN(l2tp_tunnel *tunnel)
{
	ls_SessionClose(tunnel);
}

/* Handle Incoming-Call-Reply */
void
ls_HandleICRP(l2tp_tunnel *tunnel)
{
	l2tp_session *sess = &tunnel->session;
	char msg[256];
	struct l2tp_avphdr *avphdr;
	char *data;
	uint16_t sr;

	char *state_names[] = {"idle",
				"wait-tunnel",
				"wait-reply",
				"wait-connect",
				"established"
			  	};

	/* Get assigned session ID */
	while (1) {
		if ((avphdr = la_HandleAvp(tunnel)) == NULL) {
			L2TP_LOG("No assigned session-ID in ICRP");
			return;
		}

		if (avphdr->vendorid == VENDOR_IETF &&
		    avphdr->type == AVP_ASSIGNED_SESSION_ID) {
			break;
		}
	}

	/* Set assigned session ID */
	data = (char *)(avphdr + 1);

	memcpy(&sr, data, sizeof(sr));
	sess->psid = ntohs(sr);
	if (!sess->psid) {
		L2TP_LOG("Invalid assigned session-ID in ICRP");
		return;
	}

	/* Drop any packet if we are not waiting the Incoming-Call-Reply */
	if (sess->state != SESSION_WAIT_REPLY) {
		sprintf(msg, "Received ICRP for session in state %s", state_names[sess->state]);
		ls_SendCDN(tunnel, RESULT_FSM_ERROR, 0, msg);
		return;
	}

	/* Inform connected */
	ls_SendICCN(tunnel);

	/* Set session state */
	sess->state = SESSION_ESTABLISHED;
}
