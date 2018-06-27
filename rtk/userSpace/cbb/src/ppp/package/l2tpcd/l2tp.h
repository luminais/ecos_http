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
 * $Id: l2tp.h,v 1.7 2010-07-18 08:58:14 Exp $
 */
#ifndef	__L2TP_H__
#define __L2TP_H___

#define	L2TP_PORT	1701
#define MAX_AVP_LEN	1024

/* L2TP header */
struct l2tphdr {
	uint8_t	bits;
	uint8_t version;
	uint16_t length;
	uint16_t tid;
	uint16_t sid;
	uint16_t Ns;
	uint16_t Nr;
	uint8_t avp[MAX_AVP_LEN];
};
#define	L2TPHDR_LEN	(sizeof(struct l2tphdr) - MAX_AVP_LEN)

/* Bit definitions */
#define TBIT		0x80
#define LBIT      	0x40
#define SBIT     	0x08
#define OBIT       	0x02
#define PBIT     	0x01
#define RBITS    	0x34
#define VMASK     	0x0F
#define VRESERVED 	0xF0

#define TYPE_BIT(var) 		(var & TBIT)	/* Determins if control or not */
#define LENGTH_BIT(var)		(var & LBIT) 	/* Length bit present */
#define SEQUENCE_BIT(var)	(var & SBIT) 	/* Sequence bit */
#define OFFSET_BIT(var)		(var & OBIT)   	/* Offset bit */
#define PRIORITY_BIT(var)	(var & PBIT)    /* Priority bit */
#define RESERVED_BITS(var)	(var & RBITS)   /* REserved */
#define VERSION_MASK(var)	(var & VMASK)
#define VERSION_RESERVED(var)	(var & VRESERVED)

/* L2TP AVP header */
struct l2tp_avphdr {
	uint16_t length;
	uint16_t vendorid;
	uint16_t type;
} __attribute__((packed));

struct l2tp_avp_result_code {
	uint16_t result_code;
	uint16_t error_code;
	char error_message[0];
} __attribute__((packed));

/*
 * Macros to extract information from length field of AVP
 */

/* Mandatory bit: If this is set on an unknown AVP, we MUST terminate */
#define AMBIT(len) (len & 0x8000)

/* Hidden bit: Specifies information hiding */
#define AHBIT(len) (len & 0x4000)

/* Reserved bits:  We must drop anything with any of these set. */
#define AZBITS(len) (len & 0x3C00)

#define ALENGTH(len) (len & 0x03FF)     /* Length:  Lenth of AVP */

#define MBIT 0x8000             /* for setting */
#define HBIT 0x4000             /* Set on hidden avp's */

#define MANDATORY                       1
#define NOT_MANDATORY                   0
#define HIDDEN                          1
#define NOT_HIDDEN                      0
#define VENDOR_IETF                     0

#define AVP_MESSAGE_TYPE                0
#define AVP_RESULT_CODE                 1
#define AVP_PROTOCOL_VERSION            2
#define AVP_FRAMING_CAPABILITIES        3
#define AVP_BEARER_CAPABILITIES         4
#define AVP_TIE_BREAKER                 5
#define AVP_FIRMWARE_REVISION           6
#define AVP_HOST_NAME                   7
#define AVP_VENDOR_NAME                 8
#define AVP_ASSIGNED_TUNNEL_ID          9
#define AVP_RECEIVE_WINDOW_SIZE         10
#define AVP_CHALLENGE                   11
#define AVP_Q931_CAUSE_CODE             12
#define AVP_CHALLENGE_RESPONSE          13
#define AVP_ASSIGNED_SESSION_ID         14
#define AVP_CALL_SERIAL_NUMBER          15
#define AVP_MINIMUM_BPS                 16
#define AVP_MAXIMUM_BPS                 17
#define AVP_BEARER_TYPE                 18
#define AVP_FRAMING_TYPE                19
#define AVP_CALLED_NUMBER               21
#define AVP_CALLING_NUMBER              22
#define AVP_SUB_ADDRESS                 23
#define AVP_TX_CONNECT_SPEED            24
#define AVP_PHYSICAL_CHANNEL_ID         25
#define AVP_INITIAL_RECEIVED_CONFREQ    26
#define AVP_LAST_SENT_CONFREQ           27
#define AVP_LAST_RECEIVED_CONFREQ       28
#define AVP_PROXY_AUTHEN_TYPE           29
#define AVP_PROXY_AUTHEN_NAME           30
#define AVP_PROXY_AUTHEN_CHALLENGE      31
#define AVP_PROXY_AUTHEN_ID             32
#define AVP_PROXY_AUTHEN_RESPONSE       33
#define AVP_CALL_ERRORS                 34
#define AVP_ACCM                        35
#define AVP_RANDOM_VECTOR               36
#define AVP_PRIVATE_GROUP_ID            37
#define AVP_RX_CONNECT_SPEED            38
#define AVP_SEQUENCING_REQUIRED         39

#define HIGHEST_AVP                     39

/* Control Connection Management */
#define CTRL_MSG_SCCRQ 	1               /* Start-Control-Connection-Request */
#define CTRL_MSG_SCCRP 	2               /* Start-Control-Connection-Reply */
#define CTRL_MSG_SCCCN 	3               /* Start-Control-Connection-Connected */
#define CTRL_MSG_StopCCN 4               /* Stop-Control-Connection-Notification */
/* 5 is reserved */
#define CTRL_MSG_HELLO	6               /* Hello */
/* Call Management */
#define CTRL_MSG_OCRQ	7               /* Outgoing-Call-Request */
#define CTRL_MSG_OCRP	8               /* Outgoing-Call-Reply */
#define CTRL_MSG_OCCN	9               /* Outgoing-Call-Connected */
#define CTRL_MSG_ICRQ	10              /* Incoming-Call-Request */
#define CTRL_MSG_ICRP	11              /* Incoming-Call-Reply */
#define CTRL_MSG_ICCN	12              /* Incoming-Call-Connected */
/* 13 is reserved */
#define CTRL_MSG_CDN	14              /* Call-Disconnect-Notify */
/* Error Reporting */
#define CTRL_MSG_WEN	15              /* WAN-Error-Notify */
/* PPP Sesssion Control */
#define CTRL_MSG_SLI	16              /* Set-Link-Info */
#define CTRL_MSG_MAX_MSG 16

#define CTRL_MSG_ZLB                     32767

/* Result and error codes */
#define RESULT_GENERAL_REQUEST          1
#define RESULT_GENERAL_ERROR            2
#define RESULT_CHANNEL_EXISTS           3
#define RESULT_NOAUTH                   4
#define RESULT_UNSUPPORTED_VERSION      5
#define RESULT_SHUTTING_DOWN            6
#define RESULT_FSM_ERROR                7

#define ERROR_OK                        0
#define ERROR_NO_CONTROL_CONNECTION     1
#define ERROR_BAD_LENGTH                2
#define ERROR_BAD_VALUE                 3
#define ERROR_OUT_OF_RESOURCES          4
#define ERROR_INVALID_SESSION_ID        5
#define ERROR_VENDOR_SPECIFIC           6
#define ERROR_TRY_ANOTHER               7
#define ERROR_UNKNOWN_AVP_WITH_M_BIT    8

#endif	/* __L2TP_H_ */
