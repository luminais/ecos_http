/*
 * L2TP pass through proxy
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ip_l2tp_pxy.c,v 1.1 2010-08-27 03:45:55 Exp $
 */
#define IPF_L2TP_PROXY

struct l2tphdr {
	uint16_t msg_type;	/* Message type */
	uint16_t length;	/* Length (opt) */
	uint16_t tid;		/* Tunnel ID */
	uint16_t sid;		/* Session ID */
	uint16_t Ns;		/* Ns (opt) */
	uint16_t Nr;		/* Nr (opt) */
};                 

/* Hook function */
int
ippr_l2tp_init(void)
{
	return 0;
}

int
ippr_l2tp_new
(
	fr_info_t *fin,
	ip_t *ip,
	ap_session_t *aps,
	nat_t *nat
)
{
	KMALLOCS(aps->aps_data, u_32_t *, sizeof(u_32_t));
	memset(aps->aps_data, 0, sizeof(u_32_t));
	aps->aps_psiz = sizeof(u_32_t);
	return 0;
}

/* Process L2TP ALG */
/*modify by zzh used to compatible L2tp server in ROS*/
int
ippr_l2tp_match
(
	fr_info_t *fin,
	ap_session_t *aps,
	nat_t *nat
)
{
	struct l2tphdr l2tp;
	unsigned short	*tid;
	unsigned char *pmsg_type;
	mb_t *m;
	int off;

	memset(&l2tp, 0x0, sizeof(l2tp));
	if ((fin->fin_dlen-sizeof(udphdr_t)) < sizeof(struct l2tphdr)) {
		//printf("return here in func:%s, line:%d\n", __func__, __LINE__);
		return -1;
	}
	m = *(mb_t **)fin->fin_mp;
	off = fin->fin_hlen + sizeof(udphdr_t);
	m_copydata(m, off, sizeof(struct l2tphdr), (char *)&l2tp);
	tid = (unsigned short *)(aps->aps_data);
	pmsg_type = (unsigned char *)(&l2tp.msg_type);
	
	//printf("msg_type[0]=%d", pmsg_type[0]);
	if ((pmsg_type[0] & 0x40) == 0) {
		l2tp.tid = l2tp.length;
	}
	
	//printf("=====>func:%s, line:%d, l2tp.tid=%d, tid[fin->fin_out]=%d\n", __func__, __LINE__, 
	//	ntohs(l2tp.tid), ntohs(tid[fin->fin_out]));


	if (tid[fin->fin_out] == 0) {
		tid[fin->fin_out] = l2tp.tid;
	} else {
		if (tid[fin->fin_out] != l2tp.tid) {
			//printf("return here in func:%s, line:%d\n", __func__, __LINE__);
			return -1;
		}
	}
	return 0;
}
//end by zzh 20140603
