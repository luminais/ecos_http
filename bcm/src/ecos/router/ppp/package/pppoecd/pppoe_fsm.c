
/*
 * PPPOE OS independent code
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pppoe_fsm.c,v 1.8 2010-10-23 11:00:10 Exp $
 */
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>

#include <pppd.h>
#include <pppoe_var.h>

#include <arpa/inet.h>
#include <stdlib.h>

#ifdef __CONFIG_TENDA_HTTPD_V3__
#include <bcmnvram.h>
#include <shutils.h>
#endif
#include <sys/md5.h>
#define senderr(e) { error = (e); goto bad;}

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

//第一次用户使用路由器时检测上网环境使用，add by wwk20120929
int recv_pado_packet_tag = 0;
int pppoe_pado_tag = 0;


//add by ldm
int num_padi=0;

static void
suppress_candidate(struct pppoe_softc *sc)
{
	int i;
	struct suppress *oldest = NULL;
	struct suppress *blank = NULL;
	struct suppress *slog = sc->slog;

	for (i = 0; i < NUM_SUPPRESS; i++, slog++) {
		switch (slog->state) {
		case SUPPRESS_CANDIDATE:
			if (memcmp(slog->dst, sc->dhost, 6) == 0) {
				/* Found old one */
				return;
			}
			break;
		case SUPPRESS_OFF:
			if (blank == NULL)
				blank = slog;
			break;
		case SUPPRESS_ON:
			if (oldest == NULL || oldest->stime > slog->stime)
				oldest = slog;
			break;
		default:
			break;
		}
	}

	/* Check entry */
	if (blank == NULL && oldest == NULL)
		return;
	if (blank == NULL)
		blank = oldest;

	/* Set the condidate */
	memset(blank, 0, sizeof(*blank));
	blank->state = SUPPRESS_CANDIDATE;
	blank->stime = time(0);
	memcpy(blank->dst, sc->dhost, 6);
	return;
}

static void
suppress_check(struct pppoe_softc *sc)
{
	int i;
	struct suppress *slog = sc->slog;
	time_t now = time(0);

	for (i = 0; i < NUM_SUPPRESS; i++, slog++) {
		if (slog->state == SUPPRESS_CANDIDATE &&
		    memcmp(slog->dst, sc->dhost, 6) == 0) {
			/*
			 * If time period from up to down is too short,	
			 * take it as ppp configuration error.
			 * Block this entry for a while.
			 */
			if ((now - slog->stime) < 3) {
				slog->state = SUPPRESS_ON;
				slog->stime = now;
			}
			else {
				slog->state = SUPPRESS_OFF;
			}
			break;
		}
	}
	return;
}

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

/*
 * Return the location where the next
 * tag can be put.
 */
static __inline struct pppoe_tag *
next_tag(struct pppoe_hdr *ph)
{
	return (struct pppoe_tag *)(((char *)&ph->tag[0]) + ntohs(ph->length));
}

/*
 * Look for a tag of a specific type
 * Don't trust any length the other
 * end says.  but assume we already
 * sanity checked ph->length.
 */
static struct pppoe_tag *
get_tag(struct pppoe_hdr *ph, unsigned short idx)
{
	char *end = (char *)next_tag(ph);
	char *ptn;
	struct pppoe_tag *pt = &ph->tag[0];

	/*
	* Keep processing tags while a tag header will still fit.
	*/
	while ((char*)(pt + 1) <= end) {
		/*
		* If the tag data would go past the end of the packet, abort.
		*/
		ptn = (((char *)(pt + 1)) + ntohs(pt->tag_len));
		if (ptn > end)
			return NULL;

		if (ntohs(pt->tag_type) == idx)
			return pt;

		pt = (struct pppoe_tag *)ptn;
	}

	return NULL;
}

/* Random seed for cookie generation */
#define ETH_ALEN 6
#define SEED_LEN 16
#define MD5_LEN 16
#define COOKIE_LEN (MD5_LEN + sizeof(pid_t)) /* Cookie is 16-byte MD5 + PID of server */

void
genCookie(unsigned char const *peerEthAddr,
	  unsigned char const *myEthAddr,
	  unsigned char const *seed,
	  unsigned char *cookie)
{
    struct MD5Context ctx;
    pid_t pid = 5555;

    MD5Init(&ctx);
    MD5Update(&ctx, peerEthAddr, ETH_ALEN);
    MD5Update(&ctx, myEthAddr, ETH_ALEN);
    MD5Update(&ctx, seed, SEED_LEN);
    MD5Final(cookie, &ctx);
    memcpy(cookie+MD5_LEN, &pid, sizeof(pid));
}
static int
make_packet_server(struct pppoe_softc *sc, unsigned char code,unsigned char *uniq,int len_tag_gong)
{
	struct pppoe_full_hdr *pkt;
	struct ether_header *eh;
	struct pppoe_hdr *ph;
	struct pppoe_tag *tag;
	struct pppoe_tag *peer;
	int tag_data_len;
	unsigned short len;

	pkt = (struct pppoe_full_hdr *)sc->pktbuf;
	memset(pkt, 0, sizeof(*pkt));

	/* Build ether header */
	eh = &pkt->eh;
	memcpy(eh->ether_dhost, sc->dhost, 6);
	memcpy(eh->ether_shost, sc->shost, 6);
	eh->ether_type = htons(ETHERTYPE_PPPOE_DISC);

	/* Build pppoe header */
	ph = &pkt->ph;
	ph->ver = 0x1;
	ph->type = 0x1;
	ph->code = code;

	//设置session id为固定的0x1234
	if (code == PADS_CODE)
	{
		ph->sid = 0x1234;
	}

	/* Build TAGs */
	len = 0;
	tag = ph->tag;
	
	if (code == PADO_CODE)
	{
		/* Generate ac name*/
		unsigned char ACName[256];
		memcpy(ACName,"tenda_wwk",sizeof("tenda_wwk"));
		tag_data_len = strlen(ACName);
		tag->tag_type = htons(PTT_AC_NAME);
		tag->tag_len = htons(tag_data_len);
		memcpy(tag->tag_data, ACName, tag_data_len);
		len += sizeof(*tag) + tag_data_len;
				
		tag = (struct pppoe_tag *)((char *)ph->tag + len);

		// Generate a cookie 
		unsigned char CookieSeed[SEED_LEN];
		tag->tag_type = htons(PTT_AC_COOKIE);
		tag->tag_len = htons(COOKIE_LEN);
		int i;
		 for (i=2; i<SEED_LEN; i++) {
			 CookieSeed[i] = (rand() >> (i % 9)) & 0xFF;
		}
		 genCookie(sc->shost, sc->dhost, CookieSeed, tag->tag_data);
		len += sizeof(*tag) + COOKIE_LEN;
	
		tag = (struct pppoe_tag *)((char *)ph->tag + len);
	}

		
	/* Insert host unique */
	tag_data_len = len_tag_gong;
	tag->tag_type = htons(PTT_HOST_UNIQ);
	tag->tag_len = htons(tag_data_len);
	memcpy(tag->tag_data, uniq, tag_data_len);

	len += sizeof(*tag) + tag_data_len;
	tag = (struct pppoe_tag *)((char *)ph->tag + len);
	

	/* Insert service name */
	tag_data_len = strlen(sc->service_name);
	tag->tag_type = htons(PTT_SRV_NAME);
	tag->tag_len = htons(tag_data_len);
	if (tag_data_len)
		memcpy(tag->tag_data, sc->service_name, tag_data_len);

	len += sizeof(*tag) + tag_data_len;

	/* Setup protocol length */
	ph->length = htons(len);
	return (sizeof(*pkt) + len);
}

/*
 * Make an PAD packet
 */
static int
make_packet(struct pppoe_softc *sc, unsigned char code)
{
	struct pppoe_full_hdr *pkt;
	struct ether_header *eh;
	struct pppoe_hdr *ph;
	struct pppoe_tag *tag;
	struct pppoe_tag *peer;
	int tag_data_len;
	unsigned short len;

	pkt = (struct pppoe_full_hdr *)sc->pktbuf;
	memset(pkt, 0, sizeof(*pkt));

	/* Build ether header */
	eh = &pkt->eh;
	if (code == PADI_CODE)
		memcpy(eh->ether_dhost, "\xff\xff\xff\xff\xff\xff", 6);
	else
		memcpy(eh->ether_dhost, sc->dhost, 6);
	memcpy(eh->ether_shost, sc->shost, 6);
	eh->ether_type = htons(ETHERTYPE_PPPOE_DISC);

	/* Build pppoe header */
	ph = &pkt->ph;
	ph->ver = 0x1;
	ph->type = 0x1;
	ph->code = code;

	/* Build TAGs */
	len = 0;
	tag = ph->tag;

	if (code == PADT_CODE) {
		/* Insert SINGOFF */
		tag_data_len = strlen(PADT_SIGNOFF);

		tag->tag_type = htons(PTT_GEN_ERR);
		tag->tag_len = htons(tag_data_len);
		memcpy(tag->tag_data, PADT_SIGNOFF, tag_data_len);

		len += sizeof(*tag) + tag_data_len;
	}
	else {
		/* Insert host unique */
		tag_data_len = PPPOE_HOST_UNIQ_LEN;

		tag->tag_type = htons(PTT_HOST_UNIQ);
		tag->tag_len = htons(tag_data_len);
		memcpy(tag->tag_data, sc->unique, tag_data_len);

		len += sizeof(*tag) + tag_data_len;
		tag = (struct pppoe_tag *)((char *)ph->tag + len);

		if (code == PADR_CODE) {
			/* Insert ac cookie */
			if ((peer = sc->peer_cookie_tag) != 0) {
				tag_data_len = ntohs(peer->tag_len);

				tag->tag_type = peer->tag_type;
				tag->tag_len = peer->tag_len;
				memcpy(tag->tag_data, peer->tag_data, tag_data_len);

				len += sizeof(*tag) + tag_data_len;
				tag = (struct pppoe_tag *)((char *)ph->tag + len);
			}

			/* Insert ac name */
			if ((peer = sc->peer_ac_tag) != 0) {
				tag_data_len = ntohs(peer->tag_len);

				tag->tag_type = peer->tag_type;
				tag->tag_len = peer->tag_len;
				memcpy(tag->tag_data, peer->tag_data, tag_data_len);

				len += sizeof(*tag) + tag_data_len;
				tag = (struct pppoe_tag *)((char *)ph->tag + len);
			}
			/* Insert Session ID . By wxy*/
			if ((peer = sc->peer_sid_tag) != 0) {
				tag_data_len = ntohs(peer->tag_len);

				tag->tag_type = peer->tag_type;
				tag->tag_len = peer->tag_len;
				memcpy(tag->tag_data, peer->tag_data, tag_data_len);

				len += sizeof(*tag) + tag_data_len;
				tag = (struct pppoe_tag *)((char *)ph->tag + len);
			}
			
		}

		/* Insert service name */
		tag_data_len = strlen(sc->service_name);

		tag->tag_type = htons(PTT_SRV_NAME);
		tag->tag_len = htons(tag_data_len);
		if (tag_data_len)
			memcpy(tag->tag_data, sc->service_name, tag_data_len);

		len += sizeof(*tag) + tag_data_len;
	}

	/* Setup protocol length */
	ph->length = htons(len);

	return (sizeof(*pkt) + len);
}

/*
 * Send PADR packet
 */
void
pppoe_send_padt(struct pppoe_softc *sc)
{
	unsigned short len;

	/* Build PADT packet */
	len = make_packet(sc, PADT_CODE);

	write(sc->dev_fd, sc->pktbuf, len);
	pppoe_msleep(1000);
}

/*
 * Send PADR packet
 */
void
pppoe_send_padr(struct pppoe_softc *sc)
{
	unsigned short len;

	PPPOE_LOG("%s send PADR", sc->param.pppname);

	/* Build PADR packet */
	len = make_packet(sc, PADR_CODE);

	write(sc->dev_fd, sc->pktbuf, len);
	return;
}

/* Send PADI packet */
int pppoe_send_padi_num = 0;
int pppoe0_send_padi_num = 0;

void
pppoe_send_padi(struct pppoe_softc *sc)
{
	
	unsigned short len;

	PPPOE_LOG("%s send PADI", sc->param.pppname);

	/* Build PADI packet */
	len = make_packet(sc, PADI_CODE);

	write(sc->dev_fd, sc->pktbuf, len);
	if(sc->param.unit == 2)
	{
		pppoe_send_padi_num++;
	}
	if(sc->param.unit == 0)
	{
		pppoe0_send_padi_num++;
	}
	return;
}
/*
 * Send PADO packet
 */
void
pppoe_send_pado(struct pppoe_softc *sc,unsigned char *uniq,int len_tag_gong)
{
	unsigned short len;

	PPPOE_LOG("%s send PADO", sc->param.pppname);

	/* Build PADR packet */
	len = make_packet_server(sc, PADO_CODE,uniq,len_tag_gong);

	write(sc->dev_fd, sc->pktbuf, len);

	return;
}

void
pppoe_send_pads(struct pppoe_softc *sc,unsigned char *uniq,int len_tag_gong)
{
	unsigned short len;

	PPPOE_LOG("%s send PADs", sc->param.pppname);

	/* Build PADR packet */
	len = make_packet_server(sc, PADS_CODE,uniq,len_tag_gong);

	write(sc->dev_fd, sc->pktbuf, len);

	return;
}

/*
 * Divide switch case method to
 * enter_init_state,
 * enter_dwait_state,
 * enter_req_state,
 * enter_connected_state and
 * enter_zombie_state
 */
static void
enter_dwait_state(struct pppoe_softc *sc)
{
	struct ifpppoereq req;

	sc->state = PPPOE_SDWAIT;

	/* Set state to dev */
	memset(&req, 0, sizeof(req));

	req.state = PPPOE_SDWAIT;
	memcpy(req.dhost, sc->dhost, 6);
	ioctl(sc->dev_fd, PPPIOCSPPPOESTATE, &req);
}

static void
enter_init_state(struct pppoe_softc *sc)
{
	struct ifpppoereq req;

	sc->peer_ac_tag = 0;
	sc->peer_cookie_tag = 0;
		sc->peer_sid_tag = 0; //wxy
	sc->fsm_time = time(0);
	sc->fsm_timeout = PPPOE_INITIAL_TIMEOUT;
	sc->state = PPPOE_SINIT;

	/* Set state to dev */
	memset(&req, 0, sizeof(req));

	req.state = PPPOE_SINIT;
	strcpy(req.ethname, sc->param.ethname);
	memcpy(req.shost, sc->shost, 6);
	memcpy(req.unique, sc->unique, sizeof(req.unique));
	ioctl(sc->dev_fd, PPPIOCSPPPOESTATE, &req);

	/* Send PADI */
	if(sc->param.unit != 1)//gong add
	{
		pppoe_send_padi(sc);
	}
	return;
}

static void
enter_req_state(struct pppoe_softc *sc)
{
	struct ifpppoereq req;

	sc->fsm_time = time(0);
	sc->fsm_timeout = PPPOE_INITIAL_TIMEOUT;
	sc->state = PPPOE_SREQ;

	/* Set state to dev */
	memset(&req, 0, sizeof(req));

	req.state = PPPOE_SREQ;
	memcpy(req.dhost, sc->dhost, 6);
	ioctl(sc->dev_fd, PPPIOCSPPPOESTATE, &req);

	/* Send PADR */
	pppoe_send_padr(sc);
	return;
}
static void
enter_offer_state(struct pppoe_softc *sc,unsigned char *uniq,int len_tag_gong)
{
#if 0
	struct ifpppoereq req;

	sc->fsm_time = time(0);
	sc->fsm_timeout = PPPOE_INITIAL_TIMEOUT;
	sc->state = PPPOE_SREQ;

	/* Set state to dev */
	memset(&req, 0, sizeof(req));

	req.state = PPPOE_SREQ;
	memcpy(req.dhost, sc->dhost, 6);
	ioctl(sc->dev_fd, PPPIOCSPPPOESTATE, &req);
#endif
	/* Send PADO */
	pppoe_send_pado(sc,uniq,len_tag_gong);
	return;
}

static void
enter_sure_state(struct pppoe_softc *sc,unsigned char *uniq,int len_tag_gong)
{
	/* Send PADO */
	pppoe_send_pads(sc,uniq,len_tag_gong);
	return;
}

static void
enter_connected_state(struct pppoe_softc *sc, unsigned short sid)
{
	struct ifpppoereq req;

	if (sc->state == PPPOE_CONNECTED)
		return;

	/* Change to connection state */
	sc->sid = sid;
	sc->state = PPPOE_CONNECTED;

	/* Set state to dev */
	memset(&req, 0, sizeof(req));

	req.state = PPPOE_CONNECTED;
	req.sid = sid;
	ioctl(sc->dev_fd, PPPIOCSPPPOESTATE, &req);

	/* Start ppp daemon */
	sc->ppp_pid = pppoe_osl_ppp_open(sc);
	sc->ppp_up_time = time(0);

	/* Record this host in supress table */
	suppress_candidate(sc);

	/* Wait until interface up */
	if (sc->ppp_pid != 0) {
		while ((get_ifflags(sc->param.pppname) & IFF_UP) == 0) {
			/* check thread status of ppp */
			if (pppoe_osl_ppp_state(sc) == 0) {
				pppoe_fsm_down(sc);
				return;
			}
			pppoe_msleep(100);
		}
	}
	return;
}

static void
enter_zombie_state(struct pppoe_softc *sc)
{
	struct ifpppoereq req;

	if (sc->state == PPPOE_CONNECTED)
		pppoe_send_padt(sc);

	sc->state = PPPOE_SNONE;

	/* Enter zombie state */
	memset(&req, 0, sizeof(req));

	req.state = PPPOE_SNONE;
	ioctl(sc->dev_fd, PPPIOCSPPPOESTATE, &req);
	return;
}

/* pppoe state machine */
static int
pppoe_fsm(struct pppoe_softc *sc, struct ether_header *eh, char *buf, int len)
{
	int i;

	struct pppoe_hdr *ph = (struct pppoe_hdr *)buf;
	struct pppoe_tag *utag = 0;
	struct pppoe_tag *cookietag = 0;
	struct pppoe_tag *sidtag = 0;
	struct pppoe_tag *actag = 0;

	unsigned char code;
	unsigned short sid;
	unsigned char ac_name[64+1];
	unsigned char uniq[100];
	int len_tag;

	/* Check state */
	sid = ntohs(ph->sid);
	code = ph->code;
	switch (code) {
	case PADO_CODE:
		PPPOE_LOG("%s receives PADO", sc->param.pppname);
		
		/*pxy add*/
		if(!strcmp(sc->param.pppname, "ppp2")){
			sc->flag |= PPPOE_SHUTDOWN;
			sc->state |= PPPOE_CLOSE;
			printf("---receives----PADO-------\n");
			pppoe_pado_tag = 1;//hqw add for pppoe 
			return 0;
		}
		pppoe_pado_tag = 1;
		num_padi=0;/*set sent padi number*/
		/*
		* Check the session is in the right state.
		* It needs to be in PPPOE_SINIT.
		*/
		if (sc->state != PPPOE_SINIT) {
			PPPOE_LOG("%s is not in PPPOE_SINIT state", sc->param.pppname);
			return ENETUNREACH;
		}

#ifdef  __CONFIG_TENDA_MULTI__
		/* Check failed log */
		for (i = 0; i < NUM_SUPPRESS; i++) {
			if (sc->slog[i].state == SUPPRESS_ON &&
			    memcmp(sc->slog[i].dst, eh->ether_shost, 6) == 0) {
				/* If in failed log table, exit */
				if (++sc->slog[i].count > PADO_NEGLECTED_MAX ||
				    (time(0) - sc->slog[i].stime) >= 60) {
					/* Reconsider this one.
					 * Redial every 1 minute.
					 */
					sc->slog[i].state = SUPPRESS_OFF;
					//printf("----enter PADO_NEGLECTED_MAX\n");
					break;
				}
				return ENETUNREACH;
			}
		}
#endif 
		

		/* Check host unique */
		utag = get_tag(ph, PTT_HOST_UNIQ);
		if (utag == NULL ||
		    ntohs(utag->tag_len) != PPPOE_HOST_UNIQ_LEN ||
		    memcmp(utag->tag_data, sc->unique, PPPOE_HOST_UNIQ_LEN) != 0) {
			PPPOE_LOG("Host unique does not match");
			return ENETUNREACH;
		}

		/* Check AC name */
		ac_name[0] = 0;
		actag = get_tag(ph, PTT_AC_NAME);
		if (actag) {
			int len = ntohs(actag->tag_len);

			memcpy(ac_name, actag->tag_data, len);
			ac_name[len] = 0;
		}
#if 1//tenda modify,dont check AC Name,20110225
		if (sc->ac_name[0] != 0 &&
		    strcmp((char *)ac_name, sc->ac_name) != 0) {
			PPPOE_LOG("Incoming AC_NAME = %s not matched", ac_name);
			return ENETUNREACH;
		}
#endif
		/*
		* This is the first time we hear
		* from the server, so note it's
		* unicast address, replacing the
		* broadcast address .
		*/
		memcpy(sc->dhost, eh->ether_shost, 6);

		sc->peer_ac_tag = 0;
		sc->peer_cookie_tag = 0;
		sc->peer_sid_tag = 0;
		
		/* Retrieve ac_tag */
		if (actag) {
			sc->peer_ac_tag = (struct pppoe_tag *)sc->peer_ac;
			sc->peer_ac_tag->tag_type = actag->tag_type;
			sc->peer_ac_tag->tag_len = actag->tag_len;
			memcpy(sc->peer_ac_tag->tag_data, actag->tag_data,
				ntohs(actag->tag_len));
		}

		if ((cookietag = get_tag(ph, PTT_AC_COOKIE)) != 0) {
			sc->peer_cookie_tag = (struct pppoe_tag *)sc->peer_cookie;
			sc->peer_cookie_tag->tag_type = cookietag->tag_type;
			sc->peer_cookie_tag->tag_len = cookietag->tag_len;
			memcpy(sc->peer_cookie_tag->tag_data, cookietag->tag_data,
				ntohs(cookietag->tag_len));
		}
		/* Retrieve session id */
		if ((sidtag = get_tag(ph, PTT_RELAY_SESSION_ID)) != 0) {
			sc->peer_sid_tag = (struct pppoe_tag *)sc->peer_sid;
			sc->peer_sid_tag->tag_type = sidtag->tag_type;
			sc->peer_sid_tag->tag_len = sidtag->tag_len;
			memcpy(sc->peer_sid_tag->tag_data, sidtag->tag_data,
				ntohs(sidtag->tag_len));
		}
		/* Enter SREQ state */
		PPPOE_LOG("Send PADR, Enter PPPOE_SREQ");		
		enter_req_state(sc);
		//rec PADO packet tag,add by wwk 20120929
		recv_pado_packet_tag = 1;
		break;

	case PADS_CODE:
		PPPOE_LOG("%s receive PADS", sc->param.pppname);

		/*
		* Check the session is in the right state.
		* It needs to be in PPPOE_SREQ.
		*/
		if (sc->state != PPPOE_SREQ) {
			PPPOE_LOG("%s is not in PPPOE_SREQ state", sc->param.pppname);
			return ENETUNREACH;
		}

		/* Check host unique */
		utag = get_tag(ph, PTT_HOST_UNIQ);
		if (utag == NULL ||
		    ntohs(utag->tag_len) != PPPOE_HOST_UNIQ_LEN ||
		    memcmp(utag->tag_data, sc->unique, PPPOE_HOST_UNIQ_LEN) != 0) {
			PPPOE_LOG("Host unique not matched");
			return ENETUNREACH;
		}

		/* Got new session ID */
		PPPOE_LOG("Enter PPPOE_CONNECTED, session_id = %d", sid);
		enter_connected_state(sc, sid);
	
		break;

	case PADT_CODE:
		/*
		* Send a 'close' message to the controlling
		* process (the one that set us up);
		* And then tear everything down.
		*/

		PPPOE_LOG("%s receive PADT(SID:%d)", sc->param.pppname, ntohs(ph->sid));

		/*
		* Find matching peer/session combination.
		*/
		if (sc->state == PPPOE_CONNECTED &&
		    sc->sid == sid &&
		    memcmp(sc->dhost, eh->ether_shost, 6) == 0) {
			/* Disconnect this session */
			pppoe_fsm_down(sc);
		}
		break;

	case PADI_CODE:                                            //wwk
		utag = get_tag(ph, PTT_HOST_UNIQ);
		if ( utag!=NULL )
		{
			len_tag = ntohs(utag->tag_len);
			memcpy(uniq, utag->tag_data, len_tag);
			uniq[len_tag] = 0;
		}

		memcpy(sc->dhost, eh->ether_shost, 6);
		enter_offer_state(sc,uniq,len_tag);
		break;
	case PADR_CODE:
		utag = get_tag(ph, PTT_HOST_UNIQ);
		if ( utag!=NULL )
		{
			len_tag = ntohs(utag->tag_len);

			memcpy(uniq, utag->tag_data, len_tag);
			uniq[len_tag] = 0;
		}
		memcpy(sc->dhost, eh->ether_shost, 6);
		enter_sure_state(sc,uniq,len_tag);

		PPPOE_LOG("Enter PPPOE_CONNECTED, session_id = %d", 33);
		//设置固定session id为:0x1234
		enter_connected_state(sc, 0x1234);

		break;
	default:
		return EPFNOSUPPORT;
	}

	return 0;
}

/*
 * Check the pppoe state.
 * Send PADI/PADR if timed-out.
 */
void
pppoe_fsm_timer(struct pppoe_softc *sc)
{
	time_t now = time(0);
	int flags;

	/* Check session state */
	switch (sc->state) {
	/*pxy add,*/	
	case PPPOE_CLOSE:
		if (!strcmp(sc->param.pppname, "ppp2") && (sc->flag |= PPPOE_SHUTDOWN)){
			printf("%s,%d: PPPOE_CLOSE\n",__FUNCTION__,__LINE__);
			break;
		}
	case PPPOE_SINIT:
	case PPPOE_SREQ:
		if ((now - sc->fsm_time) > sc->fsm_timeout) {
			/*
			* Set the next timeout with
			* power of 2.
			*/
			sc->fsm_time = now;
			if ((sc->fsm_timeout <<= 1) > PPPOE_TIMEOUT_LIMIT) {
				/* revert to SINIT mode */
				if (sc->state == PPPOE_SREQ) {
					enter_init_state(sc);
					break;
				}
				else
					sc->fsm_timeout = PPPOE_INITIAL_TIMEOUT;
			}

			/* Send retry packet out */
			if (sc->state == PPPOE_SINIT){
				if(sc->param.unit == 0)
				{
					num_padi=num_padi+1;/*count  send padi number*/
				}
				if(sc->param.unit != 1)//gong add pppoe server不发包
				{
					pppoe_send_padi(sc);
				}
				if(num_padi>=3){
					nvram_set("err_check","5");
					}
				}	
			else
				pppoe_send_padr(sc);
		}
		break;

	case PPPOE_CONNECTED:
		/* Check if ppp interface is down */
		flags = get_ifflags(sc->param.pppname);
		if ((flags & IFF_UP) == 0) {
			pppoe_fsm_down(sc);
		}
		break;

	default:
		break;
	}

	return;
}

/*
 * Iterate per session to process
 * the state machine of each one.
 */
void
pppoe_fsm_handler(struct pppoe_softc *sc)
{
	fd_set fds;
	struct timeval tv = {1, 0};
	int maxfd;
	int n;

	if (sc->dev_fd < 0) {
		pppoe_msleep(10+20);
		return;
	}
	/* Set receive select set */
	FD_ZERO(&fds);

	FD_SET(sc->dev_fd, &fds);
	maxfd = sc->dev_fd+1;

	/* Wait for socket events */
	n = select(maxfd+1, &fds, NULL, NULL, &tv);
	if (n > 0) {
		/* process dhcpd */
		if (FD_ISSET(sc->dev_fd, &fds)) {
			char *buf = (char *)sc->pktbuf;
			struct ether_header *eh;
			int len;

			/* Process the dhcpd incoming packet */
			len = read(sc->dev_fd, buf, PPPOE_MAX_MTU);
			if (len >= sizeof("DIALUP") &&
			    memcmp(buf, "DIALUP", sizeof("DIALUP")) == 0) {
				/* Enter init state */
				enter_init_state(sc);
				return;
			}
			else if (len >= sizeof(struct pppoe_full_hdr)) {
				/*
				* Check protocol, only handle the
				* PPPOE DISCOVERY PACKET
				*/
				eh = (struct ether_header *)buf;
				if (ntohs(eh->ether_type) == ETHERTYPE_PPPOE_DISC) {
					len -= sizeof(*eh);
					buf += sizeof(*eh);

					/* Do state machine. */
					pppoe_fsm(sc, eh, buf, len);
				}
			}
		}
	}
	/* Do timeout check */
	pppoe_fsm_timer(sc);
}

/*
 * Up pppoe session fsm
 */
static int
get_ifmac(unsigned char *ifname, unsigned char ifmac[6])
{
	int s;
	struct ifreq ifr;

	/* Check interface address */
	if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, (char *)ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFHWADDR, &ifr) != 0 ||
	    memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", 6) == 0) {
		close(s);
		return -1;
	}

	/* Retrieve the ifmac */
	memcpy(ifmac, ifr.ifr_hwaddr.sa_data, 6);

	close(s);
	return 0;
}

/* Bringup the fsm of a pppoe session */
int
pppoe_fsm_up(struct pppoe_softc *sc)
{
	struct pppoe_param *param = &sc->param;
	unsigned char ifmac[6];
	char dev_name[64];

	PPPOE_LOG("%s(%s)", __func__, param->pppname);
	if (sc->dev_fd < 0) {
		/* Get physical MAC addr */
		if (get_ifmac((unsigned char *)param->ethname, ifmac) != 0) {
			PPPOE_LOG("%s cannot get %s mac!", param->pppname, param->ethname);
			return -1;
		}

		sprintf(dev_name, "/dev/net/pppoe/%s", param->pppname);
		sc->dev_fd = open(dev_name, O_RDWR);
		if (sc->dev_fd < 0) {
			PPPOE_LOG("Cannot open %s!\n", dev_name);
			return -1;
		}
		/* Initialize this pppoe structure */
		memcpy(sc->shost, ifmac, 6);
		memcpy(sc->unique, ifmac, 6);
		sc->unique[6] = 0;
		sc->unique[7] = param->unit;
		sc->unique[8] = 5;//gong  mark
		sc->unique[9] = 2;
		sc->unique[10] = 6;
		sc->unique[11] = 0;

		strcpy(sc->service_name, param->service_name);
		strcpy(sc->ac_name, param->ac_name);
	}

	/* Demand wait or start connection */
	if (sc->param.demand && (sc->flag & PPPOE_RECONNECT) == 0)
	{
		enter_dwait_state(sc);
	}
	else
	{
		enter_init_state(sc);
	}

	return 0;
}

/* Shutdown the fsm of this pppoe session */
void
pppoe_fsm_down(struct pppoe_softc *sc)
{
	PPPOE_LOG("%s(%s)", __func__, sc->param.pppname);

#ifdef __CONFIG_TENDA_HTTPD_V3__
	int demand = 0 , unit = 0;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";
#endif
	if(sc->param.unit == 1)//gong 
	{
		enter_zombie_state(sc);
		return ;
		
	}
	else
	{	if (sc->dev_fd >= 0) {
		/* Close ppp */
		pppoe_osl_ppp_close(sc);

		suppress_check(sc);

		/* Enter INIT state */
		enter_zombie_state(sc);

		/* Close device */
		close(sc->dev_fd);
		sc->dev_fd = -1;
		}
	}

	/* Re-initializing if possible */
	if(sc->param.unit != 2)
	{
		if ((sc->flag & (PPPOE_SHUTDOWN | PPPOE_DISCONNECT)) == 0 &&
		    (sc->param.demand || sc->param.keepalive)) {
			/* Up state machine again */
#ifdef __CONFIG_TENDA_HTTPD_V3__			
			if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
				unit = 0;
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			demand = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_demand", tmp)));
			if(demand != 2){
				pppoe_fsm_up(sc);
			}else{
				
				cyg_thread_exit();
			}
#else
				pppoe_fsm_up(sc);
#endif
		}
	}
	return;
}

