/*
 * PPTP protocol include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pptp.h,v 1.1 2010-07-15 11:33:22 Exp $
 */
#ifndef _PPTP_PROTOCOL_H_
#define _PPTP_PROTOCOL_H_

#define PPTP_PORT			1723
#define PPTP_MAGIC			0x1a2b3c4d
#define PPTP_PROTO_VERS			0x0100

enum {
	PPTP_StartControlConnRequest = 1,
	PPTP_StartControlConnReply = 2,
	PPTP_StopControlConnRequest = 3,
	PPTP_StopControlConnReply = 4,
	PPTP_EchoRequest = 5,
	PPTP_EchoReply = 6,
	PPTP_OutCallRequest = 7,
	PPTP_OutCallReply = 8,
	PPTP_InCallRequest = 9,
	PPTP_InCallReply = 10,
	PPTP_InCallConn = 11,
	PPTP_CallClearRequest = 12,
	PPTP_CallDiscNotify = 13,
	PPTP_WanErrorNotify = 14,
	PPTP_SetLinkInfo = 15
};

/* message structures */

#define PPTP_HOSTNAME_LEN		64
#define PPTP_VENDOR_LEN			64
#define PPTP_PHONE_LEN			64
#define PPTP_SUBADDR_LEN		64
#define PPTP_STATS_LEN			128

#define PPTP_CTRL_MSG_TYPE		1

#define PPTP_FRAMECAP_ASYNC		0x01
#define PPTP_FRAMECAP_SYNC		0x02
#define PPTP_FRAMECAP_ANY		0x03

#define PPTP_BEARCAP_ANALOG		0x01
#define PPTP_BEARCAP_DIGITAL		0x02
#define PPTP_BEARCAP_ANY		0x03

struct pptpMsgHead {
	u_int16_t length;
	u_int16_t msgType;
	u_int32_t magic;
	u_int16_t type;
	u_int16_t resv0;
};

struct pptpStartCtrlConnRequest {
	struct pptpMsgHead header;
	u_int16_t vers;
	u_int16_t resv0;
	u_int32_t frameCap;
	u_int32_t bearCap;
	u_int16_t maxChan;
	u_int16_t firmware;
	char host[PPTP_HOSTNAME_LEN];
	char vendor[PPTP_VENDOR_LEN];
};

struct pptpStartCtrlConnReply {
	struct pptpMsgHead header;
	u_int16_t vers;
	u_int8_t result;
	u_int8_t err;
	u_int32_t frameCap;
	u_int32_t bearCap;
	u_int16_t maxChan;
	u_int16_t firmware;
	char host[PPTP_HOSTNAME_LEN];
	char vendor[PPTP_VENDOR_LEN];
};

struct pptpStopCtrlConnRequest {
	struct pptpMsgHead header;
	u_int8_t reason;
	u_int8_t resv0;
	u_int16_t resv1;
};

#define PPTP_SCCR_REAS_NONE		1	/* general */
#define PPTP_SCCR_REAS_PROTO		2	/* can't support protocol version */
#define PPTP_SCCR_REAS_LOCAL		3	/* local shutdown */

struct pptpStopCtrlConnReply {
	struct pptpMsgHead header;
	u_int8_t result;
	u_int8_t err;
	u_int16_t resv0;
};

struct pptpEchoRequest {
	struct pptpMsgHead header;
	u_int32_t id;
};

struct pptpEchoReply {
	struct pptpMsgHead header;
	u_int32_t id;
	u_int8_t result;
	u_int8_t err;
	u_int16_t resv0;
};

#define PPTP_RECV_WIN		64
#define PPTP_PPD		1
#define PPTP_MINBPS		2400
#define PPTP_MAXBPS		10000000

struct pptpOutCallRequest {
	struct pptpMsgHead header;
	u_int16_t cid;
	u_int16_t serno;
	u_int32_t minBps;
	u_int32_t maxBps;
	u_int32_t bearType;
	u_int32_t frameType;
	u_int16_t recvWin;
	u_int16_t ppd;
	u_int16_t numLen;
	u_int16_t resv0;
	char phone[PPTP_PHONE_LEN];
	char subaddr[PPTP_SUBADDR_LEN];
};

struct pptpOutCallReply {
	struct pptpMsgHead header;
	u_int16_t cid;
	u_int16_t peerCid;
	u_int8_t result;
	u_int8_t err;
	u_int16_t cause;
	u_int32_t speed;
	u_int16_t recvWin;
	u_int16_t ppd;
	u_int32_t channel;
};

struct pptpInCallRequest {
	struct pptpMsgHead header;
	u_int16_t cid;
	u_int16_t serno;
	u_int32_t bearType;
	u_int32_t channel;
	u_int16_t dialedLen;
	u_int16_t dialingLen;
	char dialed[PPTP_PHONE_LEN];
	char dialing[PPTP_PHONE_LEN];
	char subaddr[PPTP_SUBADDR_LEN];
};

struct pptpInCallReply {
	struct pptpMsgHead header;
	u_int16_t cid;
	u_int16_t peerCid;
	u_int8_t result;
	u_int8_t err;
	u_int16_t recvWin;
	u_int16_t ppd;
	u_int16_t resv0;
};

struct pptpInCallConn {
	struct pptpMsgHead header;
	u_int16_t peerCid;
	u_int16_t resv0;
	u_int32_t speed;
	u_int16_t recvWin;
	u_int16_t ppd;
	u_int32_t frameType;
};

struct pptpCallClearRequest {
	struct pptpMsgHead header;
	u_int16_t cid;
	u_int16_t resv0;
};

struct pptpCallDiscNotify {
	struct pptpMsgHead header;
	u_int16_t cid;
	u_int8_t result;
	u_int8_t err;
	u_int16_t cause;
	u_int16_t resv0;
	char stats[PPTP_STATS_LEN];
};

struct pptpWanErrorNotify {
	struct pptpMsgHead header;
	u_int16_t cid;
	u_int16_t resv0;
	u_int32_t crc;
	u_int32_t frame;
	u_int32_t hdw;
	u_int32_t ovfl;
	u_int32_t timeout;
	u_int32_t align;
};

struct pptpSetLinkInfo {
	struct pptpMsgHead header;
	u_int16_t cid;
	u_int16_t resv0;
	u_int32_t sendAccm;
	u_int32_t recvAccm;
};

#endif	/* _PPTP_PROTOCOL_H_ */
