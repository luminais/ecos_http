#include <sys/types.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <stdio.h>

#include "pptp_msg.h"

#define ASSERT(x) assert(x)
#define FREE_M(m)							\
	do {								\
		if ((m)) {						\
			m_freem((m));					\
			(m) = NULL;					\
		}							\
	} while (0)
	
/* test for a 32 bit counter overflow */
#define WRAPPED( curseq, lastseq) \
    ((((curseq) & 0xffffff00) == 0) && \
     (((lastseq) & 0xffffff00 ) == 0xffffff00))

//static u_int32_t ack_sent, ack_recv;
//static u_int32_t seq_sent, seq_recv;
unsigned int ack_sent, ack_recv;
unsigned int seq_sent, seq_recv;
unsigned int tx_seq = 1; /* first sequence number sent must be 1 */

u_int16_t pptp_gre_call_id, pptp_gre_peer_call_id;

void print_packet(int fd, void *pack, unsigned int len)
{
#if 0
    unsigned char *b = (unsigned char *)pack;
    unsigned int i,j;
    FILE *out = fdopen(fd, "w");
    fprintf(out,"-- begin packet (%u) --\n", len);
    for (i = 0; i < len; i += 16) {
        for (j = 0; j < 8; j++)
            if (i + 2 * j + 1 < len)
                fprintf(out, "%02x%02x ", 
                        (unsigned int) b[i + 2 * j],
                        (unsigned int) b[i + 2 * j + 1]);
            else if (i + 2 * j < len)
                fprintf(out, "%02x ", (unsigned int) b[i + 2 * j]);
        fprintf(out, "\n");
    }
    fprintf(out, "-- end packet --\n");
    fflush(out);
#endif	
}


int sync_gre_tx_2_daemon()
{
    /* send packet with payload */
   // header->flags |= hton8(PPTP_GRE_FLAG_S);
   // header->seq    = hton32(tx_seq);
    if (ack_sent != seq_recv) 	/* send ack with this message */
	{ 
        ack_sent = seq_recv;
    }
    seq_sent = tx_seq; 
	tx_seq++;
	return 1;
}

/*
*	fastpath sync to pptpd daemon!!!
*/
void sync_gre_rx_2_daemon(unsigned char *pbuf)
{
	struct pptp_gre_header *header;
    u_int32_t seq;
	header = (struct pptp_gre_header *)(pbuf);
	
    if (PPTP_GRE_IS_A(ntoh8(header->ver))) { 
        u_int32_t ack = (PPTP_GRE_IS_S(ntoh8(header->flags)))?
            header->ack:header->seq; /* ack in different place if S = 0 */
        ack = ntoh32( ack);
        if (ack > ack_recv) ack_recv = ack;
        if (WRAPPED(ack,ack_recv)) ack_recv = ack;
	}
	seq = ntoh32(header->seq);
    if ((seq == seq_recv + 1)) {
        seq_recv = seq;
    }
}


/*	
	encap the GRE header.
	need transfer the m address to function call.
	because the function call modify the m address
*/	
int encap_gre_header(struct mbuf **m , int len)
{
//    static u_int32_t seq = 1; /* first sequence number sent must be 1 */
    unsigned int header_len;
	struct pptp_gre_header *header;
	int ack = -1;
	
	ack = (ack_sent != seq_recv) ?1:0;
	M_PREPEND(*m,sizeof(struct pptp_gre_header)-sizeof(header->ack)*(1-ack),M_DONTWAIT);	//default own ack 
	header = (struct pptp_gre_header *)(mtod(*m, uint8_t *));

    /* fill GRE header for packet */
    header->flags       = hton8 (PPTP_GRE_FLAG_K);
   	header->ver         = hton8 (PPTP_GRE_VER);
    header->protocol    = hton16(PPTP_GRE_PROTO);
    header->payload_len = hton16(len);
    header->call_id     = hton16(pptp_gre_peer_call_id);
		
    /* send packet with payload */
    header->flags |= hton8(PPTP_GRE_FLAG_S);
    header->seq    = hton32(tx_seq);
    if (ack_sent != seq_recv) { /* send ack with this message */
        header->ver |= hton8(PPTP_GRE_FLAG_A);
        header->ack  = hton32(seq_recv);
        ack_sent = seq_recv;
        header_len = sizeof(header);
    } else { /* don't send ack */
        header_len = sizeof(header) - sizeof(header->ack);
    }
    seq_sent = tx_seq; 
#ifdef FAST_PPTP
	sync_gre_tx_2_fastpptp(tx_seq);
#endif
	tx_seq++;

	return 1;
}


int decap_gre_header(struct mbuf *m )
{
	struct pptp_gre_header *header;
    int status, ip_len = 0;
    static int first = 1;
    unsigned int headersize;
    unsigned int payload_len;
    u_int32_t seq;

	header = (struct pptp_gre_header *)(mtod(m, uint8_t *));
	
    /* verify packet */
    if (    /* version should be 1 */
            ((ntoh8(header->ver) & 0x7F) != PPTP_GRE_VER) ||
            /* PPTP-GRE protocol for PPTP */
            (ntoh16(header->protocol) != PPTP_GRE_PROTO)||
            /* flag C should be clear   */
            PPTP_GRE_IS_C(ntoh8(header->flags)) ||
            /* flag R should be clear   */
            PPTP_GRE_IS_R(ntoh8(header->flags)) ||
            /* flag K should be set     */
            (!PPTP_GRE_IS_K(ntoh8(header->flags))) ||
            /* routing and recursion ctrl = 0  */
            ((ntoh8(header->flags)&0xF) != 0)) {
        /* if invalid, discard this packet */
        diag_printf("Discarding GRE: %X %X %X %X %X %X", 
                ntoh8(header->ver)&0x7F, ntoh16(header->protocol), 
                PPTP_GRE_IS_C(ntoh8(header->flags)),
                PPTP_GRE_IS_R(ntoh8(header->flags)), 
                PPTP_GRE_IS_K(ntoh8(header->flags)),
                ntoh8(header->flags) & 0xF);
		goto release;
    }
	
    /* silently discard packets not for this call */
    if (ntoh16(header->call_id) != pptp_gre_call_id) goto release;
	
    /* test if acknowledgement present */
    if (PPTP_GRE_IS_A(ntoh8(header->ver))) { 
        u_int32_t ack = (PPTP_GRE_IS_S(ntoh8(header->flags)))?
            header->ack:header->seq; /* ack in different place if S = 0 */
        ack = ntoh32( ack);
        if (ack > ack_recv) ack_recv = ack;
        /* also handle sequence number wrap-around  */
        if (WRAPPED(ack,ack_recv)) ack_recv = ack;
	}
	
    /* test if payload present */
    if (!PPTP_GRE_IS_S(ntoh8(header->flags)))
        goto release; /* ack, but no payload */
    headersize  = sizeof(struct pptp_gre_header);
    payload_len = ntoh16(header->payload_len);
    seq         = ntoh32(header->seq);
    /* no ack present? */
    if (!PPTP_GRE_IS_A(ntoh8(header->ver))) headersize -= sizeof(header->ack);
  
    /* check for expected sequence number */
    if ( first || (seq == seq_recv + 1)) { /* wrap-around safe */
      //diag_printf("accepting packet %d", seq);
        //stats.rx_accepted++;
        first = 0;
        seq_recv = seq;
    }

#ifdef FAST_PPTP
	//sync daemon gre rx info to fastpath
	sync_gre_rx_2_fastpptp(seq);	
#endif
    /* out of order, check if the number is too low and discard the packet. 
     * (handle sequence number wrap-around, and try to do it right) 
	 *			Need modify!!!!!!!!!!!!!!!!!!!!!!!
     */
#if 0
	else if ( seq < seq_recv + 1 || WRAPPED(seq_recv, seq) ) {
	    diag_printf("discarding duplicate or old packet %d (expecting %d)",
	        seq, seq_recv + 1);
        stats.rx_underwin++;
    /* sequence number too high, is it reasonably close? */
    } else if ( seq < seq_recv + MISSING_WINDOW ||
                WRAPPED(seq, seq_recv + MISSING_WINDOW) ) {
		stats.rx_buffered++;
            diag_printf("%s packet %d (expecting %d, lost or reordered)",
                disable_buffer ? "accepting" : "buffering",
                seq, seq_recv+1);
        if ( disable_buffer ) {
            seq_recv = seq;
            stats.rx_lost += seq - seq_recv - 1;
        } else {
          //  pqueue_add(seq, buffer + ip_len + headersize, payload_len);
	}
    /* no, packet must be discarded */
    } else {
        diag_printf("discarding bogus packet %d (expecting %d)", 
		 		seq, seq_recv + 1);
        stats.rx_overwin++;
    }
#endif

	//strip gre header
	m_adj(m,headersize);
    return 1;
release:
	FREE_M(m);
	return 0;

}

/*********************dzh end *************************/


