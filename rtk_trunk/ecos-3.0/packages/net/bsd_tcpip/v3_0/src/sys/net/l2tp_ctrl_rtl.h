
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
 
 /* Retransmit timeout parameters */
#define L2TP_REXMIT_MAX		8
#define L2TP_REXMIT_MAX_TO	10
 
#define L2TP_CHALLENGE_LEN	16
 
#define L2TP_CONNECT_SPEED	10000000		/* XXX hardcoded */
#define L2TP_FRAMING_TYPE	L2TP_FRAMING_SYNC	/* XXX hardcoded */
 
 /* Idle timeout for sending 'HELLO' message */
#define L2TP_IDLE_TIMEOUT	60
 
 /* Reply timeout for messages */
#define L2TP_REPLY_TIMEOUT	60
 
 /* Linger time for dying tunnels and sessions */
#define L2TP_CTRL_DEATH_TIMEOUT	1//11
#define L2TP_SESS_DEATH_TIMEOUT	1//11

#define L2TP_TUNNEL_NUM 4
#define L2TP_SESS_NUM 4

#if 0
#define LOG_ERR		LG_ERR
#define LOG_WARNING	LG_PHYS2
#define LOG_NOTICE	LG_PHYS2
#define LOG_INFO	LG_PHYS3
#define LOG_DEBUG	LG_PHYS3
 #else
 #define LOG_ERR		
#define LOG_WARNING	
#define LOG_NOTICE	
#define LOG_INFO
#define LOG_DEBUG	
 #endif

 #ifndef assert
 #define assert(expr) \
        if(!(expr)) {                                   \
        diag_printf( "\033[33;41m%s:%d: assert(%s)\033[m\n", \
        __FILE__,__LINE__,#expr);               \
        }
#endif

 /* Control message types */
 enum l2tp_msg_type {
	 SCCRQ		 =1,
	 SCCRP		 =2,
	 SCCCN		 =3,
	 StopCCN	 =4,
	 HELLO		 =6,
	 OCRQ		 =7,
	 OCRP		 =8,
	 OCCN		 =9,
	 ICRQ		 =10,
	 ICRP		 =11,
	 ICCN		 =12,
	 CDN	 =14,
	 WEN	 =15,
	 SLI	 =16
 };
 
 /* Control connection states */
 enum l2tp_ctrl_state {
	 CS_IDLE = 0,
	 CS_WAIT_CTL_REPLY,
	 CS_WAIT_CTL_CONNECT,
	 CS_ESTABLISHED,
	 CS_DYING
 };
 
 /* Session states */
 enum l2tp_sess_state {
	 SS_WAIT_REPLY = 1,
	 SS_WAIT_CONNECT,
	 SS_WAIT_ANSWER,
	 SS_ESTABLISHED,
	 SS_DYING
 };
 
 /* Session origination */
 enum l2tp_sess_orig {
	 ORIG_LOCAL  =0,
	 ORIG_REMOTE =1
 };
 
 /* Session side */
 enum l2tp_sess_side {
	 SIDE_LNS	 =0,
	 SIDE_LAC	 =1
 };
 
 
 /* Session statistics struct. */
 struct ng_l2tp_session_stats {
	 u_int64_t xmitPackets; 	 /* number of packets xmit */
	 u_int64_t xmitOctets;		 /* number of octets xmit */
	 u_int64_t recvPackets; 	 /* number of packets received */
	 u_int64_t recvOctets;		 /* number of octets received */
 };
 
 /* Statistics struct */
 struct ng_l2tp_stats {
	 u_int32_t xmitPackets; 	 /* number of packets xmit */
	 u_int32_t xmitOctets;		 /* number of octets xmit */
	 u_int32_t xmitZLBs;	 /* ack-only packets transmitted */
	 u_int32_t xmitDrops;		 /* xmits dropped due to full window */
	 u_int32_t xmitTooBig;		 /* ctrl pkts dropped because too big */
	 u_int32_t xmitInvalid; 	 /* ctrl packets with no session ID */
	 u_int32_t xmitDataTooBig;	 /* data pkts dropped because too big */
	 u_int32_t xmitRetransmits;  /* retransmitted packets */
	 u_int32_t recvPackets; 	 /* number of packets rec'd */
	 u_int32_t recvOctets;		 /* number of octets rec'd */
	 u_int32_t recvRunts;		 /* too short packets rec'd */
	 u_int32_t recvInvalid; 	 /* invalid packets rec'd */
	 u_int32_t recvWrongTunnel;  /* packets rec'd with wrong tunnel id */
	 u_int32_t recvUnknownSID;	 /* pkts rec'd with unknown session id */
	 u_int32_t recvBadAcks; 	 /* ctrl pkts rec'd with invalid 'nr' */
	 u_int32_t recvOutOfOrder;	 /* out of order ctrl pkts rec'd */
	 u_int32_t recvDuplicates;	 /* duplicate ctrl pkts rec'd */
	 u_int32_t recvDataDrops;	 /* dup/out of order data pkts rec'd */
	 u_int32_t recvZLBs;	 /* ack-only packets rec'd */
	 u_int32_t memoryFailures;	 /* times we couldn't allocate memory */
 };
 

 
 /************************************************************************
		 CONTROL -> LINK CALLBACK FUNCTIONS
 ************************************************************************/
 
 /*
  * This is called when a control connection gets connected state
  *
  * Arguments:
  *  ctrl	 Control connection
  */
 typedef void	 ppp_l2tp_ctrl_connected_t(struct ppp_l2tp_ctrl *ctrl);
 
 /*
  * This is called when a control connection is terminated for any reason
  * other than a call ppp_l2tp_ctrl_destroy().
  *
  * Arguments:
  *  ctrl	 Control connection
  *  result  Result code (never zero)
  *  error	 Error code (if result == L2TP_RESULT_GENERAL, else zero)
  *  errmsg  Error message string
  */
 typedef void	 ppp_l2tp_ctrl_terminated_t(struct ppp_l2tp_ctrl *ctrl,
			 u_int16_t result, u_int16_t error, const char *errmsg);
 
 /*
  * This is called just before control connection is destroyed for any reason
  * other than a call ppp_l2tp_ctrl_destroy().
  *
  * Arguments:
  *  ctrl	 Control connection
  */
 typedef void	 ppp_l2tp_ctrl_destroyed_t(struct ppp_l2tp_ctrl *ctrl);
 
 /*
  * This callback is used to report the peer's initiating a new incoming
  * or outgoing call.
  *
  * In the case of an incoming call, when/if the call eventually connects,
  * the ppp_l2tp_connected_t callback will be called by the control side.
  *
  * In the case of an outgoing call, the link side is responsible for calling
  * ppp_l2tp_connected() when/if the call is connected.
  *
  * This callback may choose to call ppp_l2tp_terminate() (to deny the call)
  * or ppp_l2tp_connected() (to accept it immediately if outgoing) before
  * returning.
  *
  * The link side is expected to plumb the netgraph session hook at this time.
  *
  * Arguments:
  *  ctrl	 Associated control connection
  *  sess	 Session structure
  *  out Non-zero to indicate outgoing call
  *  avps	 AVP's contained in the associated Outgoing-Call-Request
  * 	 or Incoming-Call-Request control message.
  */
 typedef void	 ppp_l2tp_initiated_t(struct ppp_l2tp_ctrl *ctrl,
			 struct ppp_l2tp_sess *sess, int out,
			 const struct ppp_l2tp_avp_list *avps,
			 u_char *include_length, u_char *enable_dseq);
 
 /*
  * This callback is used to report successful connection of a remotely
  * initiated incoming call (see ppp_l2tp_initiated_t) or a locally initiated
  * outgoing call (see ppp_l2tp_initiate()).  That is, we are acting as
  * the LNS for this session.
  *
  * Arguments:
  *  sess	 Session structure
  *  avps	 AVP's contained in the associated Outgoing-Call-Connected
  * 	 or Incoming-Call-Connected control message.
  */
 typedef void	 ppp_l2tp_connected_t(struct ppp_l2tp_sess *sess,
			 const struct ppp_l2tp_avp_list *avps);
 
 /*
  * This callback is called when any call, whether successfully connected
  * or not, is terminated for any reason other than explict termination
  * from the link side (via a call to either ppp_l2tp_terminate() or
  * ppp_l2tp_ctrl_destroy()).
  *
  * Arguments:
  *  sess	 Session structure
  *  result  Result code (never zero)
  *  error	 Error code (if result == L2TP_RESULT_GENERAL, else zero)
  *  errmsg  Error message string
  */
 typedef void	 ppp_l2tp_terminated_t(struct ppp_l2tp_sess *sess,
			 u_int16_t result, u_int16_t error, const char *errmsg);
 
 /*
  * This callback is used when the remote side sends a Set-Link-Info
  * message. It is optional and may be NULL if not required.
  *
  * Arguments:
  *  sess	 Session structure
  *  xmit	 LAC's send ACCM
  *  recv	 LAC's receive ACCM
  */
 typedef void	 ppp_l2tp_set_link_info_t(struct ppp_l2tp_sess *sess,
			 u_int32_t xmit, u_int32_t recv);
 
 /*
  * This callback is used when the remote side sends a WAN-Error-Notify
  * message. It is optional and may be NULL if not required.
  *
  * Arguments:
  *  sess	 Session structure
  *  crc CRC errors
  *  frame	 Framing errors
  *  overrun Hardware overrun errors
  *  buffer  Buffer overrun errors
  *  timeout Timeout errors
  *  align	 Alignment errors
  */
 typedef void	 ppp_l2tp_wan_error_notify_t(struct ppp_l2tp_sess *sess,
			 u_int32_t crc, u_int32_t frame, u_int32_t overrun,
			 u_int32_t buffer, u_int32_t timeout, u_int32_t align);
 
 /* Callback structure provided by the link side */
 struct ppp_l2tp_ctrl_cb {
	 ppp_l2tp_ctrl_connected_t	 *ctrl_connected;
	 ppp_l2tp_ctrl_terminated_t  *ctrl_terminated;
	 ppp_l2tp_ctrl_destroyed_t	 *ctrl_destroyed;
	 ppp_l2tp_initiated_t		 *initiated;
	 ppp_l2tp_connected_t		 *connected;
	 ppp_l2tp_terminated_t		 *terminated;
	 ppp_l2tp_set_link_info_t	 *set_link_info;
	 ppp_l2tp_wan_error_notify_t *wan_error_notify;
 };


 /*
  * Control message handler function types.
  *
  * Messages are either session-specific, or apply to the entire tunnel.
  */
 typedef int l2tp_ctrl_msg_handler_t(struct ppp_l2tp_ctrl *ctrl,
			 const struct ppp_l2tp_avp_list *avps,
			 struct ppp_l2tp_avp_ptrs *ptrs);
 typedef int l2tp_sess_msg_handler_t(struct ppp_l2tp_sess *sess,
			 const struct ppp_l2tp_avp_list *avps,
			 struct ppp_l2tp_avp_ptrs *ptrs);
 
 /* Descriptor for one control message */
 struct l2tp_msg_info {
	 const char 	 *name;
	 enum l2tp_msg_type  type;
	 l2tp_ctrl_msg_handler_t *ctrl_handler;
	 l2tp_sess_msg_handler_t *sess_handler;
	 int		 valid_states[12];
	 int		 valid_orig[3];
	 int		 valid_side[3];
	 int		 req_avps[AVP_MAX + 1];
 };
 
 
 struct l2tp_seq {
	 u_int16_t		 ns;	 /* next xmit seq we send */
	 u_int16_t		 nr;	 /* next recv seq we expect */
	 u_int16_t		 inproc;	 /* packet is in processing */
	 u_int16_t		 rack;		 /* last 'nr' we rec'd */
	 u_int16_t		 xack;		 /* last 'nr' we sent */
	 u_int16_t		 wmax;		 /* peer's max recv window */
	 u_int16_t		 cwnd;		 /* current congestion window */
	 u_int16_t		 ssth;		 /* slow start threshold */
	 u_int16_t		 acks;		 /* # consecutive acks rec'd */
	 u_int16_t		 rexmits;	 /* # retransmits sent */
	 struct callout 	 rack_timer; /* retransmit timer */
	 struct callout 	 xack_timer; /* delayed ack timer */
	 struct mbuf	 *xwin[L2TP_MAX_XWIN];	 /* transmit window */
	 cyg_mutex_t   	 mtx;			 /* seq mutex */
 };
 
  
 /* Session */
 struct ppp_l2tp_sess {
	 struct ppp_l2tp_ctrl	 *ctrl; 		 /* associated ctrl */
	 enum l2tp_sess_state	 state; 		 /* session state */
	 enum l2tp_sess_orig orig;			 /* who originated it? */
	 enum l2tp_sess_side side;			 /* are we lac or lns? */
	 u_int32_t		 serial;		 /* call serial number */
	 struct ppp_l2tp_avp_list *my_avps; 	 /* avps in [IO]CR[QP] */
	 struct ppp_l2tp_avp_list *peer_avps;		 /* avps in [IO]CCN */
	 u_int16_t		 session_id;	 /* session id */
	 u_int16_t		 peer_id;		 /* peer session id */
	// ng_ID_t		 node_id;		 /* tee node id */
	 void			 *link_cookie;		 /* opaque link cookie */
	 u_int16_t		 result;		 /* close result code */
	 u_int16_t		 error; 		 /* close error code */
	 char			 *errmsg;		 /* close error msg */
	 struct callout 	 reply_timer;		 /* reply timer */
	 struct callout 	 notify_timer; 	 /* link notify timer */
	 struct callout 	 close_timer;		 /* close timer */
	 struct callout 	 death_timer;		 /* death timer */
	 u_char 		 link_responded;	 /* link notified up */
	 u_char 		 peer_responded;	 /* got icrp from lns */
	 u_char 		 dseq_required; 	 /* data seq. req'd */
	 u_char 		 link_notified; 	 /* link notified down */
	 u_char 		 peer_notified; 	 /* peer notified down */
	 u_char 		 include_length;	 /* enable Length field in data packets */
	 u_char 		 enable_dseq;		 /* enable sequence fields in data packets */
	 TAILQ_ENTRY(ppp_l2tp_sess) sess_link;
	 unsigned char used;
	 unsigned char demand_mode;
	 int 			 idle_time_limit;	 
	 cyg_ppp_handle_t ppp_handle;
	/*as seq.c asked*/
 	unsigned char	control_dseq;
	//unsigned char enable_dseq;
 	int	nr;
	int   ns;
 };
 
 /* Control connection */
 struct ppp_l2tp_ctrl {
	 enum l2tp_ctrl_state	 state; 		 /* control state */
	 const struct ppp_l2tp_ctrl_cb *cb; 	 /* link callbacks */
	 u_int32_t		 tunnel_id;
	 u_int32_t		 peer_id;		 /* peer unique id */
	 u_char 		 *secret;		 /* shared secret */
	 u_int			 seclen;		 /* share secret len */
	 u_char 		 chal[L2TP_CHALLENGE_LEN]; /* our L2TP challenge */
	 struct ppp_l2tp_avp_list *avps;		 /* avps for SCCR[QP] */
	 struct callout 	 idle_timer;		 /* ctrl idle timer */
	 struct callout 	 reply_timer;		 /* reply timer */
	 struct callout 	 close_timer;		 /* close timer */
	 struct callout 	 death_timer;		 /* death timer */
	 //struct callout 	 ctrl_event;		 /* ctrl socket event */
	 //struct callout 	 data_event;		 /* data socket event */ 
	 void			 *link_cookie;		 /* opaque link cookie */
	 u_int16_t		 result;		 /* close result code */
	 u_int16_t		 error; 		 /* close error code */
	 u_int32_t		 peer_bearer;		 /* peer bearer types */
	 u_int32_t		 peer_framing;		 /* peer framing types */
	 u_int			 active_sessions;	 /* # of sessns */
	 char			 *errmsg;		 /* close error msg */
	 u_char 		 link_notified; 	 /* link notified down */
	 u_char 		 peer_notified; 	 /* peer notified down */
	 u_char 		 hide_avps; 	 /* enable AVPs hiding */
	 char			 self_name[MAXHOSTNAMELEN]; /* L2TP local hostname */
	 char			 peer_name[MAXHOSTNAMELEN]; /* L2TP remote hostname */
	 unsigned char	 used;
	 unsigned char 	closing;
	 struct l2tp_seq	seq;	 
	 struct ng_l2tp_stats	 stats; 	 /* node statistics */
	 TAILQ_HEAD(l2tp_sess_head,ppp_l2tp_sess) sess_head;
	 struct socket *so;
	 /*conf related*/
	 int rexmit_max;
	 int rexmit_max_to;

	 /*info*/
	 L2tpInfo info;
 };

 
 /***
 
 Notes
 
	 - "link_notified" means the link side has been notified that
	   the control connection or session has gone down.
	 - "peer_notified" means the peer has been notified that
	   the control connection or session has gone down.
	 - "link_responded" and "peer_responded" are only used for
	   outgoing calls when we are the LAC; they indicate acceptance
	   from the link side and the remote peer, respectively. Both
	   must be true before we send the OCCN.
	 - "sess->my_avps" are the AVP's included my ICRQ or OCRQ. In case
	   of ICRQ, these get overwritten by AVP's included in ICCN.
	 - "sess->peer_avps" are the AVPS included peer's ICCN or OCCN
 
 ***/
 static l2tp_ctrl_msg_handler_t  ppp_l2tp_handle_SCCRQ;
 static l2tp_ctrl_msg_handler_t  ppp_l2tp_handle_SCCRP;
 static l2tp_ctrl_msg_handler_t  ppp_l2tp_handle_SCCCN;
 static l2tp_ctrl_msg_handler_t  ppp_l2tp_handle_StopCCN;
 static l2tp_ctrl_msg_handler_t  ppp_l2tp_handle_HELLO;
 static l2tp_ctrl_msg_handler_t  ppp_l2tp_handle_OCRQ;
 static l2tp_ctrl_msg_handler_t  ppp_l2tp_handle_ICRQ;
 
 static l2tp_sess_msg_handler_t  ppp_l2tp_handle_OCRP;
 static l2tp_sess_msg_handler_t  ppp_l2tp_handle_xCCN;
 static l2tp_sess_msg_handler_t  ppp_l2tp_handle_ICRP;
 static l2tp_sess_msg_handler_t  ppp_l2tp_handle_CDN;
 static l2tp_sess_msg_handler_t  ppp_l2tp_handle_WEN;
 static l2tp_sess_msg_handler_t  ppp_l2tp_handle_SLI;


typedef  void pevent_handler_t(void *arg);

static pevent_handler_t		ppp_l2tp_idle_timeout;
static pevent_handler_t		ppp_l2tp_unused_timeout;
static pevent_handler_t		ppp_l2tp_ctrl_do_close;
static pevent_handler_t		ppp_l2tp_ctrl_death_timeout;

static pevent_handler_t		ppp_l2tp_sess_notify;
static pevent_handler_t		ppp_l2tp_sess_do_close;
static pevent_handler_t		ppp_l2tp_sess_death_timeout;


static struct ppp_l2tp_sess *
ppp_l2tp_sess_create(struct ppp_l2tp_ctrl *ctrl,
	enum l2tp_sess_orig orig, enum l2tp_sess_side side, u_int32_t serial);


static void
ppp_l2tp_ctrl_send(struct ppp_l2tp_ctrl *ctrl, u_int16_t session_id,
	enum l2tp_msg_type msgtype, const struct ppp_l2tp_avp_list *avps0);

static void
ppp_l2tp_ctrl_close(struct ppp_l2tp_ctrl *ctrl,
	u_int16_t result, u_int16_t error, const char *errmsg);

#define	 gL2TPto 10

