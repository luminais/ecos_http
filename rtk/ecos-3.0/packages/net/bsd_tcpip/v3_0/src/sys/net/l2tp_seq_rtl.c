
/*
 * Copyright (c) 2001-2002 Packet Design, LLC.
 * All rights reserved.
 * 
 * Subject to the following obligations and disclaimer of warranty,
 * use and redistribution of this software, in source or object code
 * forms, with or without modifications are expressly permitted by
 * Packet Design; provided, however, that:
 * 
 *    (i)  Any and all reproductions of the source or object code
 *         must include the copyright notice above and the following
 *         disclaimer of warranties; and
 *    (ii) No rights are granted, in any manner or form, to use
 *         Packet Design trademarks, including the mark "PACKET DESIGN"
 *         on advertising, endorsements, or otherwise except as such
 *         appears in the above copyright notice or in the software.
 * 
 * THIS SOFTWARE IS BEING PROVIDED BY PACKET DESIGN "AS IS", AND
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, PACKET DESIGN MAKES NO
 * REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED, REGARDING
 * THIS SOFTWARE, INCLUDING WITHOUT LIMITATION, ANY AND ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR NON-INFRINGEMENT.  PACKET DESIGN DOES NOT WARRANT, GUARANTEE,
 * OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF, OR THE RESULTS
 * OF THE USE OF THIS SOFTWARE IN TERMS OF ITS CORRECTNESS, ACCURACY,
 * RELIABILITY OR OTHERWISE.  IN NO EVENT SHALL PACKET DESIGN BE
 * LIABLE FOR ANY DAMAGES RESULTING FROM OR ARISING OUT OF ANY USE
 * OF THIS SOFTWARE, INCLUDING WITHOUT LIMITATION, ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE, OR CONSEQUENTIAL
 * DAMAGES, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, LOSS OF
 * USE, DATA OR PROFITS, HOWEVER CAUSED AND UNDER ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF PACKET DESIGN IS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Archie Cobbs <archie@freebsd.org>
 */
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
		  
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <net/netisr.h>
#include <net/intrq.h>
		  
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
		  
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>
		  
#include <cyg/ppp/ppp_io.h>
#include <cyg/ppp/ppp.h>
		  
#include <net/ethernet.h>
#include <net/pppoe.h>
#include <net/if_arp.h>
		  
#include "cyg/ppp/pppd.h"

#include "l2tp_rtl.h"
#include "l2tp_avp_rtl.h"
#include "l2tp_seq_rtl.h"
#include "l2tp_ctrl_rtl.h"

#define L2TP_COPY_MBUF		m_copypacket
#define ERROUT(x)	do { error = (x); goto done; } while (0)
#define L2TP_SEQ_CHECK(x)	do { } while (0)
#define NG_FREE_M(m)							\
	do {								\
		if ((m)) {						\
			m_freem((m));					\
			(m) = NULL;					\
		}							\
	} while (0)

#define mtx_lock cyg_mutex_lock
#define mtx_unlock  cyg_mutex_unlock
static const u_char addrctrl[] = { 0xff, 0x03 };
static void ng_l2tp_seq_rack_timeout(struct ppp_l2tp_ctrl * priv);

/*####################################################*/
/*
 * Transmit a control stream packet, payload optional.
 * The transmit sequence number is not incremented.
 */
static int
l2tp_xmit_ctrl(struct ppp_l2tp_ctrl * priv, struct mbuf *m, u_int16_t ns)
{
	struct l2tp_seq *const seq = &priv->seq;
	uint8_t *p;
	u_int16_t session_id = 0;
	int error = -1;

	mtx_lock(&seq->mtx);

	/* Stop ack timer: we're sending an ack with this packet.
	   Doing this before to keep state predictable after error. */
	if (callout_active(&seq->xack_timer))
		callout_deactivate(&seq->xack_timer);

	seq->xack = seq->nr;

	mtx_unlock(&seq->mtx);

	/* If no mbuf passed, send an empty packet (ZLB) */
	if (m == NULL) {

		/* Create a new mbuf for ZLB packet */
		MGETHDR(m, M_DONTWAIT, MT_DATA);
		if (m == NULL) {
			priv->stats.memoryFailures++;
			return (ENOBUFS);
		}
		m->m_len = m->m_pkthdr.len = 12;
		m->m_pkthdr.rcvif = NULL;
		priv->stats.xmitZLBs++;
	} else {

		/* Strip off session ID */
		if (m->m_len < 2 && (m = m_pullup(m, 2)) == NULL) {
			priv->stats.memoryFailures++;
			return (ENOBUFS);
		}
		session_id = (mtod(m, u_int8_t *)[0] << 8) + mtod(m, u_int8_t *)[1];

		/* Make room for L2TP header */
		M_PREPEND(m, 10, M_DONTWAIT);	/* - 2 + 12 = 10 */
		if (m == NULL) {
			priv->stats.memoryFailures++;
			return (ENOBUFS);
		}
	}

	/* Fill in L2TP header */
	p = mtod(m, u_int8_t *);
	p[0] = L2TP_CTRL_HDR >> 8;
	p[1] = L2TP_CTRL_HDR & 0xff;
	p[2] = m->m_pkthdr.len >> 8;
	p[3] = m->m_pkthdr.len & 0xff;
	p[4] = priv->peer_id >> 8;
	p[5] = priv->peer_id & 0xff;
	p[6] = session_id >> 8;
	p[7] = session_id & 0xff;
	p[8] = ns >> 8;
	p[9] = ns & 0xff;
	p[10] = seq->nr >> 8;
	p[11] = seq->nr & 0xff;

	/* Update sequence number info and stats */
	priv->stats.xmitPackets++;
	priv->stats.xmitOctets += m->m_pkthdr.len;

	/* Send packet */
	error = ksocket_senddata_wrapper(priv->so, m);
	return (error);
}

/*
 * Handle an outgoing control frame.
 */
static int
l2tp_send_ctrl(struct ppp_l2tp_ctrl * priv, struct mbuf *m)
{
	int error;
	int i;
	u_int16_t	ns;
	struct l2tp_seq *const seq = &priv->seq;

	/* Sanity check */
	L2TP_SEQ_CHECK(&priv->seq);

	/* Packet should have session ID prepended */
	if (m->m_pkthdr.len < 2) {
		priv->stats.xmitInvalid++;
		m_freem(m);
		ERROUT(EINVAL);
	}

	/* Check max length */
	if (m->m_pkthdr.len >= 0x10000 - 14) {
		priv->stats.xmitTooBig++;
		m_freem(m);
		ERROUT(-1);
	}

	mtx_lock(&seq->mtx);

	/* Find next empty slot in transmit queue */
	for (i = 0; i < L2TP_MAX_XWIN && seq->xwin[i] != NULL; i++);
	if (i == L2TP_MAX_XWIN) {
		mtx_unlock(&seq->mtx);
		priv->stats.xmitDrops++;
		m_freem(m);
		ERROUT(ENOBUFS);
	}
	seq->xwin[i] = m;

	/* If peer's receive window is already full, nothing else to do */
	if (i >= seq->cwnd) {
		mtx_unlock(&seq->mtx);
		ERROUT(0);
	}

	/* Start retransmit timer if not already running */
	if (!callout_active(&seq->rack_timer))
		callout_reset(&seq->rack_timer, hz, ng_l2tp_seq_rack_timeout, priv);
	
	ns = seq->ns++;
	
	mtx_unlock(&seq->mtx);

	/* Copy packet */
	if ((m = L2TP_COPY_MBUF(m, M_DONTWAIT)) == NULL) {
		priv->stats.memoryFailures++;
		ERROUT(ENOBUFS);
	}

	/* Send packet and increment xmit sequence number */
	error = l2tp_xmit_ctrl(priv, m, ns);
done:
	/* Done */
	L2TP_SEQ_CHECK(&priv->seq);
	return (error);
}

int l2tp_send_ctrl_wrapper(struct ppp_l2tp_ctrl * priv, struct mbuf *m)
{
	return l2tp_send_ctrl(priv,m);
}


/*
 * Handle an ack timeout. We have an outstanding ack that we
 * were hoping to piggy-back, but haven't, so send a ZLB.
 */
static void
ng_l2tp_seq_xack_timeout(struct ppp_l2tp_ctrl * priv)
{
	struct l2tp_seq *const seq = &priv->seq;

	/* Make sure callout is still active before doing anything */
	if (callout_pending(&seq->xack_timer) ||
	    (!callout_active(&seq->xack_timer)))
		return;

	/* Sanity check */
	L2TP_SEQ_CHECK(seq);

	/* Send a ZLB */
	l2tp_xmit_ctrl(priv, NULL, seq->ns);

	/* callout_deactivate() is not needed here 
	    as ng_uncallout() was called by ng_l2tp_xmit_ctrl() */

	/* Sanity check */
	L2TP_SEQ_CHECK(seq);
}
void ng_l2tp_seq_failure(struct ppp_l2tp_ctrl * priv)
{
	diag_printf("retry over times\n");
}


/* 
 * Handle a transmit timeout. The peer has failed to respond
 * with an ack for our packet, so retransmit it.
 */
static void
ng_l2tp_seq_rack_timeout(struct ppp_l2tp_ctrl * priv)
{
	struct l2tp_seq *const seq = &priv->seq;
	struct mbuf *m;
	u_int delay;

	/* Make sure callout is still active before doing anything */
	if (callout_pending(&seq->rack_timer) ||
	    (!callout_active(&seq->rack_timer)))
		return;

	/* Sanity check */
	L2TP_SEQ_CHECK(seq);

	priv->stats.xmitRetransmits++;

	/* Have we reached the retransmit limit? If so, notify owner. */
	if (seq->rexmits++ >= priv->rexmit_max)
		ng_l2tp_seq_failure(priv);
#if 0
	/* Restart timer, this time with an increased delay */
	delay = (seq->rexmits > 12) ? (1 << 12) : (1 << seq->rexmits);
	if (delay > priv->rexmit_max_to)
		delay = priv->rexmit_max_to;
#else
	delay = 5;
#endif
	callout_reset(&seq->rack_timer, hz * delay, ng_l2tp_seq_rack_timeout, priv);

	/* Do slow-start/congestion algorithm windowing algorithm */
	seq->ns = seq->rack;
	seq->ssth = (seq->cwnd + 1) / 2;
	seq->cwnd = 1;
	seq->acks = 0;

	/* Retransmit oldest unack'd packet */
	if ((m = L2TP_COPY_MBUF(seq->xwin[0], M_DONTWAIT)) == NULL)
		priv->stats.memoryFailures++;
	else
		l2tp_xmit_ctrl(priv, m, seq->ns++);

	/* callout_deactivate() is not needed here 
	    as ng_callout() is getting called each time */

	/* Sanity check */
	L2TP_SEQ_CHECK(seq);
}


/*
 * Handle receipt of an acknowledgement value (Nr) from peer.
 */
static void
l2tp_seq_recv_nr(struct ppp_l2tp_ctrl * priv, u_int16_t nr)
{
	struct l2tp_seq *const seq = &priv->seq;
	struct mbuf	*xwin[L2TP_MAX_XWIN];	/* partial local copy */
	int		nack;
	int		i, j;
	uint16_t	ns;

	mtx_lock(&seq->mtx);

	/* Verify peer's ACK is in range */
	if ((nack = L2TP_SEQ_DIFF(nr, seq->rack)) <= 0) {
		mtx_unlock(&seq->mtx);
		return;				/* duplicate ack */
	}
	if (L2TP_SEQ_DIFF(nr, seq->ns) > 0) {
		mtx_unlock(&seq->mtx);
		priv->stats.recvBadAcks++;	/* ack for packet not sent */
		return;
	}
	KASSERT(nack <= L2TP_MAX_XWIN,
	    ("%s: nack=%d > %d", __func__, nack, L2TP_MAX_XWIN));

	/* Update receive ack stats */
	seq->rack = nr;
	seq->rexmits = 0;

	/* Free acknowledged packets and shift up packets in the xmit queue */
	for (i = 0; i < nack; i++)
		m_freem(seq->xwin[i]);
	memmove(seq->xwin, seq->xwin + nack,
	    (L2TP_MAX_XWIN - nack) * sizeof(*seq->xwin));
	memset(seq->xwin + (L2TP_MAX_XWIN - nack), 0,
	    nack * sizeof(*seq->xwin));

	/*
	 * Do slow-start/congestion avoidance windowing algorithm described
	 * in RFC 2661, Appendix A. Here we handle a multiple ACK as if each
	 * ACK had arrived separately.
	 */
	if (seq->cwnd < seq->wmax) {

		/* Handle slow start phase */
		if (seq->cwnd < seq->ssth) {
			seq->cwnd += nack;
			nack = 0;
			if (seq->cwnd > seq->ssth) {	/* into cg.av. phase */
				nack = seq->cwnd - seq->ssth;
				seq->cwnd = seq->ssth;
			}
		}

		/* Handle congestion avoidance phase */
		if (seq->cwnd >= seq->ssth) {
			seq->acks += nack;
			while (seq->acks >= seq->cwnd) {
				seq->acks -= seq->cwnd;
				if (seq->cwnd < seq->wmax)
					seq->cwnd++;
			}
		}
	}

	/* Stop xmit timer */
	if (callout_active(&seq->rack_timer))
		callout_deactivate(&seq->rack_timer);

	/* If transmit queue is empty, we're done for now */
	if (seq->xwin[0] == NULL) {
		mtx_unlock(&seq->mtx);
		return;
	}

	/* Start restransmit timer again */
	callout_reset(&seq->rack_timer, hz, ng_l2tp_seq_rack_timeout, priv);

	/*
	 * Send more packets, trying to keep peer's receive window full.
	 * Make copy of everything we need before lock release.
	 */
	ns = seq->ns;
	j = 0;
	while ((i = L2TP_SEQ_DIFF(seq->ns, seq->rack)) < seq->cwnd
	    && seq->xwin[i] != NULL) {
		xwin[j++] = seq->xwin[i];
		seq->ns++;
	}

	mtx_unlock(&seq->mtx);

	/*
	 * Send prepared.
	 * If there is a memory error, pretend packet was sent, as it
	 * will get retransmitted later anyway.
	 */
	for (i = 0; i < j; i++) {
		struct mbuf 	*m;
		if ((m = L2TP_COPY_MBUF(xwin[i], M_DONTWAIT)) == NULL)
			priv->stats.memoryFailures++;
		else
			l2tp_xmit_ctrl(priv, m, ns);
		ns++;
	}
}
#if 0
static void mem_dump_i(unsigned char *buf, int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		if(((i+1) % 16) == 0)
			diag_printf("\n");
		diag_printf("%02x ",buf[i]);
	}
	diag_printf("\n");
}

void dump_m(struct mbuf *m)
{
	diag_printf("m->m_len %d\n",m->m_len);
	diag_printf("m->m_flags %x\n",m->m_flags);
	diag_printf("m->m_type %x\n",m->m_type);
	diag_printf("m->m_data %x\n",m->m_data);
	mem_dump_i(m->m_data,( m->m_len > 64 ) ?  64: m->m_len);	
}
#endif

int l2tp_rcvdata_lower(struct ppp_l2tp_ctrl *priv, struct mbuf *m)
{
	static const u_int16_t req_bits[2][2] = {
		{ L2TP_DATA_0BITS, L2TP_DATA_1BITS },
		{ L2TP_CTRL_0BITS, L2TP_CTRL_1BITS },
	};
	u_int16_t tid, sid;
	u_int16_t hdr;
	u_int16_t ns, nr;
	int is_ctrl;
	int error;
	int len, plen;
	struct ppp_l2tp_sess *hpriv;

	//if ctrl/session is down,drop the packet!!!
	if(!priv || priv->closing || !(priv->used))
	{
		NG_FREE_M(m);
		ERROUT(0);
	}
	/* Remember full packet length; needed for per session accounting. */
	plen = m->m_pkthdr.len;
#if 0
	/* Update stats */
	priv->stats.recvPackets++;
	priv->stats.recvOctets += plen;
#endif
	/* Get initial header */
	if (m->m_pkthdr.len < 6) {
		priv->stats.recvRunts++;
		NG_FREE_M(m);
		ERROUT(EINVAL);
	}
	if (m->m_len < 2 && (m = m_pullup(m, 2)) == NULL) {
		priv->stats.memoryFailures++;
		//NG_FREE_ITEM(item);
		ERROUT(EINVAL);
	}
	hdr = (mtod(m, uint8_t *)[0] << 8) + mtod(m, uint8_t *)[1];
	m_adj(m, 2);

	/* Check required header bits and minimum length */
	is_ctrl = (hdr & L2TP_HDR_CTRL) != 0;
	if ((hdr & req_bits[is_ctrl][0]) != 0
	    || (~hdr & req_bits[is_ctrl][1]) != 0) {
		priv->stats.recvInvalid++;
		NG_FREE_M(m);
		ERROUT(EINVAL);
	}
	if (m->m_pkthdr.len < 4				/* tunnel, session id */
	    + (2 * ((hdr & L2TP_HDR_LEN) != 0))		/* length field */
	    + (4 * ((hdr & L2TP_HDR_SEQ) != 0))		/* seq # fields */
	    + (2 * ((hdr & L2TP_HDR_OFF) != 0))) {	/* offset field */
		priv->stats.recvRunts++;
		NG_FREE_M(m);
		ERROUT(EINVAL);
	}

	/* Get and validate length field if present */
	if ((hdr & L2TP_HDR_LEN) != 0) {
		if (m->m_len < 2 && (m = m_pullup(m, 2)) == NULL) {
			priv->stats.memoryFailures++;
			ERROUT(EINVAL);
		}
		len = (mtod(m, uint8_t *)[0] << 8) + mtod(m, uint8_t *)[1] - 4;
		m_adj(m, 2);
		if (len < 0 || len > m->m_pkthdr.len) {
			priv->stats.recvInvalid++;
			NG_FREE_M(m);
			ERROUT(EINVAL);
		}
		if (len < m->m_pkthdr.len)		/* trim extra bytes */
			m_adj(m, -(m->m_pkthdr.len - len));
	}

	/* Get tunnel ID and session ID */
	if (m->m_len < 4 && (m = m_pullup(m, 4)) == NULL) {
		priv->stats.memoryFailures++;
		ERROUT(EINVAL);
	}
	
	tid = (mtod(m, u_int8_t *)[0] << 8) + mtod(m, u_int8_t *)[1];
	sid = (mtod(m, u_int8_t *)[2] << 8) + mtod(m, u_int8_t *)[3];
	m_adj(m, 4);

	/* Check tunnel ID */
#if 0	
	if (tid != priv->conf.tunnel_id &&
	    (priv->conf.match_id || tid != 0)) {
		priv->stats.recvWrongTunnel++;
		NG_FREE_M(m);
		ERROUT(EADDRNOTAVAIL);
	}
#else
	//priv=l2tp_find_ctrl(tid);
#endif
	/* Check session ID (for data packets only) */
	if ((hdr & L2TP_HDR_CTRL) == 0) {
		hpriv = l2tp_find_session(priv, sid);
		if (hpriv == NULL) {
			priv->stats.recvUnknownSID++;
			NG_FREE_M(m);
			ERROUT(ENOTCONN);
		}
	}

	/* Get Ns, Nr fields if present */
	if ((hdr & L2TP_HDR_SEQ) != 0) {
		if (m->m_len < 4 && (m = m_pullup(m, 4)) == NULL) {
			priv->stats.memoryFailures++;
			ERROUT(EINVAL);
		}
		ns = (mtod(m, u_int8_t *)[0] << 8) + mtod(m, u_int8_t *)[1];
		nr = (mtod(m, u_int8_t *)[2] << 8) + mtod(m, u_int8_t *)[3];
		m_adj(m, 4);
	} else
		ns = nr = 0;

	/* Strip offset padding if present */
	if ((hdr & L2TP_HDR_OFF) != 0) {
		u_int16_t offset;

		/* Get length of offset padding */
		if (m->m_len < 2 && (m = m_pullup(m, 2)) == NULL) {
			priv->stats.memoryFailures++;
			ERROUT(EINVAL);
		}
		offset = (mtod(m, u_int8_t *)[0] << 8) + mtod(m, u_int8_t *)[1];

		/* Trim offset padding */
		if ((2+offset) > m->m_pkthdr.len) {
			priv->stats.recvInvalid++;
			NG_FREE_M(m);
			ERROUT(EINVAL);
		}
		m_adj(m, 2+offset);
	}

	/* Handle control packets */
	if ((hdr & L2TP_HDR_CTRL) != 0) {
		struct l2tp_seq *const seq = &priv->seq;

		/* Handle receive ack sequence number Nr */
		l2tp_seq_recv_nr(priv, nr);

		/* Discard ZLB packets */
		if (m->m_pkthdr.len == 0) {
			priv->stats.recvZLBs++;
			NG_FREE_M(m);
			ERROUT(0);
		}

		mtx_lock(&seq->mtx);
		/*
		 * If not what we expect or we are busy, drop packet and
		 * send an immediate ZLB ack.
		 */
		if (ns != seq->nr || seq->inproc) {
			if (L2TP_SEQ_DIFF(ns, seq->nr) <= 0)
				priv->stats.recvDuplicates++;
			else
				priv->stats.recvOutOfOrder++;
			mtx_unlock(&seq->mtx);
			l2tp_xmit_ctrl(priv, NULL, seq->ns);
			NG_FREE_M(m);
			ERROUT(0);
		}
		/*
		 * Until we deliver this packet we can't receive next one as
		 * we have no information for sending ack.
		 */
		seq->inproc = 1;
		mtx_unlock(&seq->mtx);

		/* Prepend session ID to packet. */
		M_PREPEND(m, 2, M_DONTWAIT);
		if (m == NULL) {
			seq->inproc = 0;
			priv->stats.memoryFailures++;
			ERROUT(ENOBUFS);
		}
		mtod(m, u_int8_t *)[0] = sid >> 8;
		mtod(m, u_int8_t *)[1] = sid & 0xff;

		/* Deliver packet to upper layers */
		error=l2tp_ctrl_handle(priv, m->m_data, m->m_len);
		
		mtx_lock(&seq->mtx);
		/* Ready to process next packet. */
		seq->inproc = 0;
		/* If packet was successfully delivered send ack. */
		if (error == 0) {
			/* Update recv sequence number */
			seq->nr++;
			
			/* Start receive ack timer, if not already running */
			if (!callout_active(&seq->xack_timer)) {
				callout_reset(&seq->xack_timer, L2TP_DELAYED_ACK, 
					ng_l2tp_seq_xack_timeout,priv);
			}
		}
		mtx_unlock(&seq->mtx);
		/*free the m*/
		NG_FREE_M(m);
		ERROUT(error);
	}

	/* Per session packet, account it. */
	//hpriv->stats.recvPackets++;
	//hpriv->stats.recvOctets += plen;

	/* Follow peer's lead in data sequencing, if configured to do so */
	if (!hpriv->control_dseq)
		hpriv->enable_dseq = ((hdr & L2TP_HDR_SEQ) != 0);

	/* Handle data sequence numbers if present and enabled */
	if ((hdr & L2TP_HDR_SEQ) != 0) {
		if (hpriv->enable_dseq
		    && L2TP_SEQ_DIFF(ns, hpriv->nr) < 0) {
			//NG_FREE_ITEM(item);	/* duplicate or out of order */
			NG_FREE_M(m);
			priv->stats.recvDataDrops++;
			ERROUT(0);
		}
		hpriv->nr = ns + 1;
	}

	/* Drop empty data packets */
	if (m->m_pkthdr.len == 0) {
		//NG_FREE_ITEM(item);
		NG_FREE_M(m);
		ERROUT(0);
	}

	if(hpriv->ppp_handle ==NULL)
	{
		NG_FREE_M(m);
		ERROUT(0);
	}
	/* Deliver data */
	error=l2tp_rcv_data(hpriv,m);
done:
	/* Done */
	L2TP_SEQ_CHECK(&priv->seq);
	return (error);
}

static int l2tp_rcv_data(struct ppp_l2tp_sess *hpriv, struct mbuf *m)
{
	int error;
	/*
	 * Remove PPP address and control fields, if any.
	 * For example, ng_ppp(4) always sends LCP packets
	 * with address and control fields as required by
	 * generic PPP. PPPoE is an exception to the rule.
	 */
	if (m->m_len >= 2) {
		//diag_printf("m->m_len %d\n",m->m_len);
		if (m->m_len < 2 && !(m = m_pullup(m, 2)))
			ERROUT(ENOBUFS);
		if (bcmp(mtod(m, u_char *), addrctrl, 2) == 0)
		{
			m_adj(m, 2);
		}	
	}
	error=l2tp_data_handle_in(hpriv,m);
done:
	return error;
}
/*
 * Handle an outgoing data frame.
 */
static int
l2tp_xmit_data(struct ppp_l2tp_ctrl *priv, struct ppp_l2tp_sess *hpriv, struct mbuf *m)
{
	uint8_t *p;
	u_int16_t hdr;
	int error = -1;
	int i = 2;

	//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
	/* Sanity check */
	L2TP_SEQ_CHECK(&priv->seq);

	/* Check max length */
	if (m->m_pkthdr.len >= 0x10000 - 12) {
		priv->stats.xmitDataTooBig++;;
		NG_FREE_M(m);
		//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
		ERROUT(-1);
	}

	/* Prepend L2TP header */
	M_PREPEND(m, 6
	    + (2 * (hpriv->include_length != 0))
	    + (4 * (hpriv->enable_dseq != 0)),
	    M_DONTWAIT);
	if (m == NULL) {
		priv->stats.memoryFailures++;
		
		//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
		ERROUT(ENOBUFS);
	}
	p = mtod(m, uint8_t *);
	hdr = L2TP_DATA_HDR;
	if (hpriv->include_length) {
		hdr |= L2TP_HDR_LEN;
		p[i++] = m->m_pkthdr.len >> 8;
		p[i++] = m->m_pkthdr.len & 0xff;
	}
	p[i++] = priv->peer_id >> 8;
	p[i++] = priv->peer_id & 0xff;
	p[i++] = hpriv->peer_id >> 8;
	p[i++] = hpriv->peer_id & 0xff;
	if (hpriv->enable_dseq) {
		hdr |= L2TP_HDR_SEQ;
		p[i++] = hpriv->ns >> 8;
		p[i++] = hpriv->ns & 0xff;
		p[i++] = hpriv->nr >> 8;
		p[i++] = hpriv->nr & 0xff;
		hpriv->ns++;
	}
	p[0] = hdr >> 8;
	p[1] = hdr & 0xff;

	/* Update per session stats. */
	//hpriv->stats.xmitPackets++;
	//hpriv->stats.xmitOctets += m->m_pkthdr.len;

	/* And the global one. */
	priv->stats.xmitPackets++;
	priv->stats.xmitOctets += m->m_pkthdr.len;

	/* Send packet */
	error = ksocket_senddata_wrapper(priv->so, m);
done:
	/* Done */
	L2TP_SEQ_CHECK(&priv->seq);
	return (error);
}

int l2tp_xmit_data_wrapper(struct ppp_l2tp_ctrl *priv, struct ppp_l2tp_sess *hpriv, struct mbuf *m)
{
	short len;
	int error;
	struct mbuf *m0;
	len=0;

	//if l2tp ctrl/session is down,then free the packet!!!!!
	if(!priv || priv->closing || !(priv->used))
	{
		m_freem(m);
		ERROUT(ENOBUFS);
	}
	
	for (m0 = m; m0!= 0; m0= m0->m_next)
		len += m0->m_len;

	/*
	 * Remove PPP address and control fields, if any.
	 * For example, ng_ppp(4) always sends LCP packets
	 * with address and control fields as required by
	 * generic PPP. PPPoE is an exception to the rule.
	 */
	if (len >= 2) {
		//diag_printf("m->m_len %d\n",m->m_len);
		if (m->m_len < 2 && !(m = m_pullup(m, 2)))
			ERROUT(ENOBUFS);
		if (bcmp(mtod(m, u_char *), addrctrl, 2) == 0)
		{
			m_adj(m, 2);
			len-=2;
		}	
	}

	/*we really want to be aligned on MH_DATA
	  *M_PKTHDR will be clean by M_PREPEND.
	  */
	//diag_printf("%s %d len %d\n",__FUNCTION__,__LINE__);
	if(m->m_flags & M_PKTHDR) {
		//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
	}
	else {
		MGETHDR(m0,M_DONTWAIT,MT_HEADER);
		if (m0 == NULL) {
			m_freem(m);
			ERROUT(ENOBUFS);
		}
		/*make head space for l2tp header*/
		m0->m_data +=(6+ 
			(2 * (hpriv->include_length != 0))+ 
			(4 * (hpriv->enable_dseq != 0)));
		m0->m_next=m;
		m0->m_len=0;
		m=m0;
		//diag_printf("%s %d m->m_len %d\n",__FUNCTION__,__LINE__,m->m_len);
	}	

	#if 0
	if(!(m->m_flags &  M_PKTHDR))
	{
		m->m_flags |= M_PKTHDR;
		/*since we using M_PKTHDR, we need to clear all to avoid free memory error*/	
		memset(&m->m_pkthdr,0,sizeof(struct pkthdr));
	}
	#endif
	m->m_pkthdr.len = len;

	return l2tp_xmit_data(priv,hpriv,m);
done:
	return error;
}

/*control*/
/*
 * Reset sequence number state.
 */
static void
l2tp_seq_reset(struct l2tp_seq *seq)
{
	int i;

	/* Sanity check */
	L2TP_SEQ_CHECK(seq);

	/* Stop timers */
	callout_stop(&seq->rack_timer);
	callout_stop(&seq->xack_timer);

	/* Free retransmit queue */
	for (i = 0; i < L2TP_MAX_XWIN; i++) {
		if (seq->xwin[i] == NULL)
			break;
		m_freem(seq->xwin[i]);
	}

	/* Reset node's sequence number state */
	seq->ns = 0;
	seq->nr = 0;
	seq->rack = 0;
	seq->xack = 0;
	seq->wmax = L2TP_MAX_XWIN;
	seq->cwnd = 1;
	seq->ssth = seq->wmax;
	seq->acks = 0;
	seq->rexmits = 0;
	bzero(seq->xwin, sizeof(seq->xwin));

	/* Done */
	L2TP_SEQ_CHECK(seq);
}

void l2tp_init_seq(struct l2tp_seq *seq)
{
	memset(seq, 0, sizeof(*seq));
	seq->cwnd = 1;
	seq->wmax = 64 ;
	if (seq->wmax > L2TP_MAX_XWIN)
		seq->wmax = L2TP_MAX_XWIN;
	seq->ssth = seq->wmax;
	callout_init(&seq->rack_timer);
	callout_init(&seq->xack_timer);
	cyg_mutex_init(&seq->mtx);
}


void l2tp_fini_seq(struct l2tp_seq *seq)
{
	callout_stop(&seq->rack_timer);
	callout_stop(&seq->xack_timer);	
	cyg_mutex_destroy(&seq->mtx);
	l2tp_seq_reset(seq);
}

