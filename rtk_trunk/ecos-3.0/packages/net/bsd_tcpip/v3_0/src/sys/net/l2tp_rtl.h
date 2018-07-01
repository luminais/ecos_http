
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

 /*copied from vars.h */
 
 /* Generic option configuration structure */
 
   struct optinfo
   {
	 u_int32_t	 enable;	 /* Options I want */
	 u_int32_t	 accept;	 /* Options I'll allow */
   };
   typedef struct optinfo	 *Options;
 
  #define Enable(c,x)		((c)->enable |= (1<<(x)))
  #define Disable(c,x)		((c)->enable &= ~(1<<(x)))
  #define Accept(c,x)		((c)->accept |= (1<<(x)))
  #define Deny(c,x)		((c)->accept &= ~(1<<(x)))
 
  #define Enabled(c,x)		(((c)->enable & (1<<(x))) != 0)
  #define Acceptable(c,x)	(((c)->accept & (1<<(x))) != 0)

  /*vars.h*/
  

 /*
  * DEFINITIONS
  */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	256	
#endif

#define L2TP_MTU              1600
#define L2TP_MRU		L2TP_MTU

#define L2TP_PORT		1701

#define L2TP_CALL_MIN_BPS	56000
#define L2TP_CALL_MAX_BPS	64000

#define MPD_VENDOR "MPD_RTL"

#define Enable(c,x)		((c)->enable |= (1<<(x)))
#define Disable(c,x)		((c)->enable &= ~(1<<(x)))
#define Accept(c,x)		((c)->accept |= (1<<(x)))
#define Deny(c,x)		((c)->accept &= ~(1<<(x)))

#define Enabled(c,x)		(((c)->enable & (1<<(x))) != 0)
#define Acceptable(c,x)	(((c)->accept & (1<<(x))) != 0)


   struct l2tp_server {
	 struct in_addr	 self_addr;  /* self IP address */
	 in_port_t		 self_port;  /* self port */
	 int		 refs;
	 int		 sock;		 /* server listen socket */
   };
   
   struct l2tp_tun {
	 struct in_addr	 self_addr;  /* self IP address */
	 struct in_addr	 peer_addr;  /* peer IP address */
	 char				 peer_iface[IFNAMSIZ];	 /* Peer iface */
	 u_char 	 peer_mac_addr[6];	 /* Peer MAC address */
	 in_port_t		 self_port;  /* self port */
	 in_port_t		 peer_port;  /* peer port */
	 u_char 	 connected;  /* control connection is connected */
	 u_char 	 alive; 	 /* control connection is not dying */
	 u_int		 active_sessions;/* number of calls in this sunnels */
	 struct ppp_l2tp_ctrl *ctrl;	 /* control connection for this tunnel */
   };
   
   struct l2tpinfo {
	 struct {
	 struct in_addr	 self_addr;  /* self IP address */
	 struct in_addr  	peer_addr;  /* Peer IP addresses allowed */
	 in_port_t	 self_port;  /* self port */
	 in_port_t	 peer_port;  /* Peer port required (or zero) */
	 struct optinfo  options;
	 char		 callingnum[64]; /* L2TP phone number to use */
	 char		 callednum[64];  /* L2TP phone number to use */
	 char		 hostname[MAXHOSTNAMELEN]; /* L2TP local hostname */
	 char		 secret[64]; /* L2TP tunnel secret */
	 char		 *fqdn_peer_addr;	 /* FQDN Peer address */
	 } conf;
	 u_char 	 opened;	 /* L2TP opened by phys */
	 u_char 	 incoming;	 /* Call is incoming vs. outgoing */
	 u_char 	 outcall;	 /* incall or outcall */
	 u_char 	 sync;		 /* sync or async call */
	 u_char	 rep;
	 struct l2tp_server  *server;	 /* server associated with link */
	 struct l2tp_tun *tun;		 /* tunnel associated with link */
	 struct ppp_l2tp_sess *sess;	 /* current session for this link */
	 char		 callingnum[64]; /* current L2TP phone number */
	 char		 callednum[64];  /* current L2TP phone number */

	 /*add HF*/
	 char 		username[64];
	 char		passwd[64];
	 char		wanintfname[32];
	 unsigned char demand_mode;	 
	 int		       idle_time_limit;		 /* Shut down link if idle for this long */
	 unsigned short mtu;
   };
   typedef struct l2tpinfo	 *L2tpInfo;

char *Mdup_rtl(const char *from, size_t size);
void *Malloc(char *str, int size);
void Freee(char *mem);

/*Some ecos depended code define*/
//#define Malloc(x,y) malloc((y),M_TEMP,M_DONTWAIT)
#define Mdup(x,y,z) Mdup_rtl((y),(z))
#define Mstrdup(x,y) Mstrdup_rtl((y))
//#define Freee(x)	free((x),M_TEMP)


//#define Log(x,y) diag_printf(y)
#define Log(x,y)


#define strlcpy 			strncpy
#define strlcat 			strncat
#define Perror			diag_printf

#define MD5_Init MD5Init
#define MD5_Update MD5Update
#define MD5_Final MD5Final

  /* Set menu options */
  enum {
    SET_SELFADDR,
    SET_PEERADDR,
    SET_CALLINGNUM,
    SET_CALLEDNUM,
    SET_HOSTNAME,
    SET_SECRET,
    SET_ENABLE,
    SET_DISABLE
  };

  /* Binary options */
  enum {
    L2TP_CONF_OUTCALL,		/* when originating, calls are "outgoing" */
    L2TP_CONF_HIDDEN,		/* enable AVP hidding */
    L2TP_CONF_LENGTH,		/* enable Length field in data packets */
    L2TP_CONF_DATASEQ,		/* enable sequence fields in data packets */
    L2TP_CONF_RESOLVE_ONCE	/* Only once resolve peer_addr */
  };


#if 0
static int	ppp_l2tp_ctrl_setup_1(struct ppp_l2tp_ctrl *ctrl,
			struct ppp_l2tp_avp_ptrs *ptrs);
static int	ppp_l2tp_ctrl_setup_2(struct ppp_l2tp_ctrl *ctrl,
			struct ppp_l2tp_avp_ptrs *ptrs);
static void	ppp_l2tp_ctrl_send(struct ppp_l2tp_ctrl *ctrl,
			u_int16_t session_id, enum l2tp_msg_type msgtype,
			const struct ppp_l2tp_avp_list *avps0);
static void	ppp_l2tp_ctrl_check_reply(struct ppp_l2tp_ctrl *ctrl);
static void	ppp_l2tp_ctrl_close(struct ppp_l2tp_ctrl *ctrl,
			u_int16_t result, u_int16_t error, const char *errmsg);
static void	ppp_l2tp_ctrl_death_start(struct ppp_l2tp_ctrl *ctrl);

static struct	ppp_l2tp_sess *ppp_l2tp_sess_create(struct ppp_l2tp_ctrl *ctrl,
			enum l2tp_sess_orig orig, enum l2tp_sess_side side, 
			u_int32_t serial);
static void	ppp_l2tp_sess_destroy(struct ppp_l2tp_sess **sessp);
static void	ppp_l2tp_sess_close(struct ppp_l2tp_sess *sess,
			u_int16_t result, u_int16_t error, const char *errmsg);
static int	ppp_l2tp_sess_setup(struct ppp_l2tp_sess *sess);
static int	ppp_l2tp_sess_check_liic(struct ppp_l2tp_sess *sess);
static void	ppp_l2tp_sess_check_reply(struct ppp_l2tp_sess *sess);
#endif
