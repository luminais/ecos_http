/*
 * L2TP main loop
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: avp.c,v 1.8 2010-07-18 08:58:14 Exp $
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

struct avp_ctrl {
	char *msg;		/* AVP message */
	int mandatory;		/* Mandatory ? */
	int hide;		/* Can AVP be hidden? */
	uint16_t min_len;	/* payload length */
	uint16_t max_len;	/* Maximum PAYLOAD length */
};

struct avp_ctrl avp_table[] = {
	/* Message          mandatory, hide min_len max_len */
	{"Message Type",            1,	0,       2,              2}, /* 0 */
	{"Result Code",             1,	0,       2, MAX_PACKET_LEN}, /* 1 */
	{"Protocol Version",        1,	0,       2,              2}, /* 2 */
	{"Framing Capabilities",    1,	1,       4,              4}, /* 3 */
	{"Bearer Capabilities",     1,	1,       4,              4}, /* 4 */
	{"Tie Breaker",             1,	0,       8,              8}, /* 5 */
	{"Firmware Revision",       0,	1,       2,              2}, /* 6 */
	{"Host Name",               1,	0,       0, MAX_PACKET_LEN}, /* 7 */
	{"Vendor Name",             0,	1,       0, MAX_PACKET_LEN}, /* 8 */
	{"Assigned Tunnel ID",      1,	1,       2,              2}, /* 9 */
	{"Receive Window Size",     1,	0,       2,              2}, /* 10 */
	{"Challenge",               1,	1,       0, MAX_PACKET_LEN}, /* 11 */
	{"Q.931 Cause Code",        1,	0,       3, MAX_PACKET_LEN}, /* 12 */
	{"Challenge Response",      1,	1,      16,             16}, /* 13 */
	{"Assigned Session ID",     1,	1,       2,              2}, /* 14 */
	{"Call Serial Number",      1,	1,       4,              4}, /* 15 */
	{"Minimum BPS",             1,	1,       4,              4}, /* 16 */
	{"Maximum BPS",             1,	1,       4,              4}, /* 17 */
	{"Bearer Type",             1,	1,       4,              4}, /* 18 */
	{"Framing Type",            1,	1,       4,              4}, /* 19 */
	{"Unknown",                 1,	0,       0, MAX_PACKET_LEN}, /* 20 */
	{"Called Number",           1,	1,       0, MAX_PACKET_LEN}, /* 21 */
	{"Calling Number",          1,	1,       0, MAX_PACKET_LEN}, /* 22 */
	{"Sub-Address",             1,	1,       0, MAX_PACKET_LEN}, /* 23 */
	{"TX Connect Speed",        1,	1,       4,              4}, /* 24 */
	{"Physical Channel ID",     1,	1,       4,              4}, /* 25 */
	{"Intial Received Confreq", 1,	1,       0, MAX_PACKET_LEN}, /* 26 */
	{"Last Sent Confreq",       1,	1,       0, MAX_PACKET_LEN}, /* 27 */
	{"Last Received Confreq",   1,	1,       0, MAX_PACKET_LEN}, /* 28 */
	{"Proxy Authen Type",       1,	1,       2,              2}, /* 29 */
	{"Proxy Authen Name",       1,	1,       0, MAX_PACKET_LEN}, /* 30 */
	{"Proxy Authen Challenge",  1,	1,       0, MAX_PACKET_LEN}, /* 31 */
	{"Proxy Authen ID",         1,	1,       2,              2}, /* 32 */
	{"Proxy Authen Response",   1,	1,       0, MAX_PACKET_LEN}, /* 33 */
	{"Call Errors",             1,	1,      26,             26}, /* 34 */
	{"ACCM",                    1,	1,      10,             10}, /* 35 */
	{"Random Vector",           1,	0,       0, MAX_PACKET_LEN}, /* 36 */
	{"Private Group ID",        1,	1,       0, MAX_PACKET_LEN}, /* 37 */
	{"RX Connect Speed",        1,	1,       4,              4}, /* 38 */
	{"Sequencing Required",     1,	0,       0,              0}  /* 39 */
};

int
la_AvpLen(char *avp)
{
	struct l2tp_avphdr *avphdr = (struct l2tp_avphdr *)avp;

	/* Search until avp length is 0 */
	while (avphdr->length != 0)
		avphdr = (struct l2tp_avphdr *)((char *)avphdr + ALENGTH(ntohs(avphdr->length)));

	return (int)((char *)avphdr - avp);
}

int
la_AddAvp(char *avp, uint16_t type, uint16_t len, void *val)
{
	struct l2tp_avphdr *avphdr;
	uint16_t avplen;
	char *data;
	uint16_t avpoff;

	/* max len is 1023 - 6 */
	if (len > MAX_PACKET_LEN - 1 - sizeof(*avphdr)) {
		L2TP_SET_ERRMSG("AVP length of %d too long", (int) len);
		return -1;
	}

	/* Get current avp size */
	avpoff = la_AvpLen(avp);
	avplen = len + sizeof(*avphdr);
	if (avpoff + avplen > MAX_PACKET_LEN-1) {
		L2TP_SET_ERRMSG("No room for AVP of length %d(%d)",
			(int)avplen, avpoff);
		return -1;
	}

	/* Setup avphdr */
	if (avp_table[type].mandatory)
		avplen |= MBIT;

	avphdr = (struct l2tp_avphdr *)(avp + avpoff);

	avphdr->length = htons(avplen);
	avphdr->vendorid = htons(VENDOR_IETF);
	avphdr->type = htons(type);

	data = (char *)(avphdr + 1);
	if (len > 0)
		memcpy(data, val, len);

	return 0;
}

struct l2tp_avphdr *
la_HandleAvp(l2tp_tunnel *tunnel)
{
	struct l2tphdr *l2tp = &tunnel->rxpkt;
	int avpoff = tunnel->rx_avpoff;
	struct l2tp_avphdr *avphdr = (struct l2tp_avphdr *)((char *)l2tp + avpoff);
	int length;
	int mandatory;
	int hidden;

	/* Do range check */
	if (tunnel->rx_avpoff >= l2tp->length)
		return NULL;

	avphdr->length = ntohs(avphdr->length);
	avphdr->vendorid = ntohs(avphdr->vendorid);
	avphdr->type = ntohs(avphdr->type);

	/* Set length */
	if (AZBITS(avphdr->length)) {
		L2TP_SET_ERRMSG("AVP with reserved bits set to non-zero");
		return NULL;
	}

	/* Get bits */
	mandatory = AMBIT(avphdr->length)? 1 : 0;
	hidden = AHBIT(avphdr->length)? 1 : 0;

	/* Get len */
	length = ALENGTH(avphdr->length);
	if (length < sizeof(*avphdr)) {
		L2TP_SET_ERRMSG("Received AVP of length %d (too short)", length);
		return NULL;
	}

	if (tunnel->rx_avpoff + length > l2tp->length) {
		L2TP_SET_ERRMSG("Received AVP of length %d too long for rest of datagram",
			length);
		return NULL;
	}

	/* If we see a random vector, remember it */
	if (avphdr->vendorid == VENDOR_IETF &&
	    avphdr->type == AVP_RANDOM_VECTOR) {
		if (hidden) {
			L2TP_SET_ERRMSG("Invalid random vector AVP has H bit set");
			return NULL;
		}
	}

	/* Advance to next avpoff */
	tunnel->rx_avpoff += length;

	length -= sizeof(*avphdr);
//add by roy
//if type >= avp_table size, then
	if(avphdr->type >= sizeof(avp_table)/sizeof(avp_table[0])){
		L2TP_LOG("%s: Unknown mandatory attribute (vendor=%d, type=%d)",__FUNCTION__,avphdr->vendorid,avphdr->type);
		return avphdr;
	}
//	
	if ((length < avp_table[avphdr->type].min_len) ||
	    (length > avp_table[avphdr->type].max_len)) {
		L2TP_SET_ERRMSG("Received data length %d of AVP %s is incorrect",
			length, avp_table[avphdr->type].msg);
		return NULL;
	}

	return avphdr;
}
