
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

 
 /* L2TP header format (first 2 bytes only) */
#define L2TP_HDR_CTRL		0x8000			/* control packet */
#define L2TP_HDR_LEN		0x4000			/* has length field */
#define L2TP_HDR_SEQ		0x0800			/* has ns, nr fields */
#define L2TP_HDR_OFF		0x0200			/* has offset field */
#define L2TP_HDR_PRIO		0x0100			/* give priority */
#define L2TP_HDR_VERS_MASK	0x000f			/* version field mask */
#define L2TP_HDR_VERSION	0x0002			/* version field */
	  
	  /* Bits that must be zero or one in first two bytes of header */
#define L2TP_CTRL_0BITS		0x030d			/* ctrl: must be 0 */
#define L2TP_CTRL_1BITS		0xc802			/* ctrl: must be 1 */
#define L2TP_DATA_0BITS		0x800d			/* data: must be 0 */
#define L2TP_DATA_1BITS		0x0002			/* data: must be 1 */
	  
	  /* Standard xmit ctrl and data header bits */
#define L2TP_CTRL_HDR		(L2TP_HDR_CTRL | L2TP_HDR_LEN \
						  | L2TP_HDR_SEQ | L2TP_HDR_VERSION)
#define L2TP_DATA_HDR		(L2TP_HDR_VERSION)	/* optional: len, seq */
	  
	  /* Some hard coded values */
#define L2TP_MAX_XWIN		128			/* my max xmit window */
#define L2TP_MAX_REXMIT		5			/* default max rexmit */
#define L2TP_MAX_REXMIT_TO	30			/* default rexmit to */
#define L2TP_DELAYED_ACK	((hz + 19) / 20)	/* delayed ack: 50 ms */
	  
	  /* Default data sequence number configuration for new sessions */
#define L2TP_CONTROL_DSEQ	1			/* we are the lns */
#define L2TP_ENABLE_DSEQ	1			/* enable data seq # */
	  
	  /* Compare sequence numbers using circular math */
#define L2TP_SEQ_DIFF(x, y)	((int)((int16_t)(x) - (int16_t)(y)))
 #ifndef KASSERT(expr,msg)
 #define KASSERT(expr,msg) \
        if(!(expr)) {                                   \
        diag_printf( "\033[33;41m%s:%d: assert(%s)\033[m\n", \
        __FILE__,__LINE__,#msg);               \
        }
#endif

int l2tp_rcvdata_lower(struct ppp_l2tp_ctrl *priv, struct mbuf *m);
static int l2tp_rcv_data(struct ppp_l2tp_sess *hpriv, struct mbuf *m);
