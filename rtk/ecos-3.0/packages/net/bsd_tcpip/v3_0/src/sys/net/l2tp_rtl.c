
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
#include <sys/socketvar.h>
		  
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

#define MB_PHYS   "PHYS"

 static void
 ksocket_incoming(struct socket *so, void *arg, int waitflag);

 void l2tp_ksocket(struct ppp_l2tp_ctrl *ctrl,struct in_addr *address,unsigned int port)
 {
	 /*create socket*/
	 int ret;
	 struct sockaddr_in addr;
	 struct proc *p = curproc ? curproc : &proc0;	 /* XXX broken */
	 ret=socreate(AF_INET,&ctrl->so,SOCK_DGRAM,IPPROTO_UDP,p);	 
	 if(ret < 0) {	 	
		 diag_printf("socreate error\n");
	 }
	 memset(&addr, 0, sizeof(addr));
	 addr.sin_family = AF_INET;
	 addr.sin_len = sizeof(addr);
	 addr.sin_port = htons(port);
	 addr.sin_addr.s_addr = address->s_addr;	 
	 ret=soconnect(ctrl->so,(struct sockaddr *)(&addr),p);
	 if(ret < 0) {	 	
		 diag_printf("bind error\n");
		 soclose(ctrl->so);
	 }	 
	 ctrl->so->so_upcallarg = (void *)ctrl;
	 ctrl->so->so_upcall = ksocket_incoming;
	 ctrl->so->so_rcv.sb_flags |= SB_UPCALL; 	 
 }

 
 /*
  * Receive incoming data on our hook.	Send it out the socket.
  */
 static int
 ksocket_senddata(struct socket *const so, struct mbuf *m)
 {
	 struct proc *p = curproc ? curproc : &proc0;	 /* XXX broken */
	 int error;
	 error = (*so->so_proto->pr_usrreqs->pru_sosend)(so, 0, 0, m, 0, MSG_DONTWAIT, p);
	 return (error);
 }


 int ksocket_senddata_wrapper(struct socket *const so, struct mbuf *m)
 {
 	return ksocket_senddata(so,m);
 }
 /*
  * When incoming data is appended to the socket, we get notified here.
  */
 static void
 ksocket_incoming(struct socket *so, void *arg, int waitflag)
 {
	 struct mbuf *m;
	 struct uio auio;
	 int s, flags, error;
	 s = splnet();
 
 
	 auio.uio_resid = 1000000000;
	 flags = MSG_DONTWAIT;
	 do {
		 if ((error = (*so->so_proto->pr_usrreqs->pru_soreceive)
			   (so, (struct sockaddr **)0, &auio, &m,
			   (struct mbuf **)0, &flags)) == 0
			 && m != NULL) {
			 struct mbuf *n;
 
			 /* Don't trust the various socket layers to get the
				packet header and length correct (eg. kern/15175) */
			 for (n = m, m->m_pkthdr.len = 0; n; n = n->m_next)
				 m->m_pkthdr.len += n->m_len;
			 
			 l2tp_rcvdata_lower(arg,m);
		 }
	 } while (error == 0 && m != NULL);
	 splx(s);
 }

static L2tpInfo	 l2tp;

 /**** copy from mpd ***/
   /* L2TP control callbacks */
  static ppp_l2tp_ctrl_connected_t	ppp_l2tp_ctrl_connected_cb;
  static ppp_l2tp_ctrl_terminated_t	ppp_l2tp_ctrl_terminated_cb;
  static ppp_l2tp_ctrl_destroyed_t	ppp_l2tp_ctrl_destroyed_cb;
  static ppp_l2tp_initiated_t		ppp_l2tp_initiated_cb;
  static ppp_l2tp_connected_t		ppp_l2tp_connected_cb;
  static ppp_l2tp_terminated_t		ppp_l2tp_terminated_cb;
  static ppp_l2tp_set_link_info_t	ppp_l2tp_set_link_info_cb;

  static const struct ppp_l2tp_ctrl_cb ppp_l2tp_server_ctrl_cb = {
	ppp_l2tp_ctrl_connected_cb,
	ppp_l2tp_ctrl_terminated_cb,
	ppp_l2tp_ctrl_destroyed_cb,
	ppp_l2tp_initiated_cb,
	ppp_l2tp_connected_cb,
	ppp_l2tp_terminated_cb,
	ppp_l2tp_set_link_info_cb,
	NULL,
  };

/*############ppp related  code ############################*/
int l2tp_data_handle_in(struct ppp_l2tp_sess  *sess, struct mbuf *m)
{
	int ret;
	ret=isIPPkt(m);
	
	if(ret) {
		/*we push two for ppp addr and ctrl. through addr & ctrl is not needed*/
		if(ret == 1)
			M_PREPEND(m,2,M_DONTWAIT);
		if(ret == 2) {			
			/*Set ppp protocl to 0x0021. since the ppp_inproc only dealwith STD ppp header*/
			M_PREPEND(m,3,M_DONTWAIT);
			mtod(m, u_char *)[2]=0;
		}	
		
		ret=deliver_ppp_data(sess->ppp_handle,m);
	}
	else {		
		ret=deliver_ppp_ctrl(sess->ppp_handle,m);
	}	
	return ret;
}

int l2tp_data_handle_out(struct ppp_l2tp_sess *sess, struct mbuf *m)
{
	return l2tp_xmit_data_wrapper(sess->ctrl, sess, m);
}

int start_l2tp_ppp(struct ppp_l2tp_sess *sp, L2tpInfo pi)
{
	cyg_ppp_options_t options;
    	cyg_ppp_options_init( &options );
#ifdef PPPOE_DEBUG		
	options.debug = 1;
	options.kdebugflag = 1;
#endif

	options.idle_time_limit = (pi->demand_mode)?(pi->idle_time_limit):0;
	strcpy(options.user,pi->username);
	strcpy(options.passwd,pi->passwd);
	options.mtu = pi->mtu;
	// options.flowctl = CYG_PPP_FLOWCTL_SOFTWARE;
	register_ppp_tx_cb(sp,l2tp_data_handle_out);
	sp->ppp_handle = cyg_ppp_up_rtl("/dev/ser1", &options );
	//cyg_ppp_wait_up(sp->ppp_handle);
	//AAA
	return 0;
}

void stop_l2tp_ctrl(struct ppp_l2tp_ctrl *ctrl)
{
	if(ctrl != NULL) {
		ppp_l2tp_ctrl_shutdown(ctrl,L2TP_RESULT_CLEARED,0,NULL);
	}	
}

void l2tp_session_ppp_close(struct ppp_l2tp_ctrl *ctrl)
{
	struct ppp_l2tp_sess *sess;
	TAILQ_FOREACH(sess,&ctrl->sess_head,sess_link)
	{
		if (sess != NULL) {
			if(sess->ppp_handle) {		
				cyg_ppp_down(sess->ppp_handle );
				cyg_ppp_wait_down(sess->ppp_handle );
				sess->ppp_handle =NULL;
			}
		}	
	}	
}

void L2tp_close(void)
{
	int i;
	struct ppp_l2tp_ctrl *l2tp_ctrl_array=get_l2tp_ctrl_array();
	for(i=0;i<L2TP_TUNNEL_NUM;i++)
	{
		if(l2tp_ctrl_array[i].used && (l2tp_ctrl_array[i].closing == 0))
		{
			l2tp_ctrl_array[i].closing=1;
			l2tp_session_ppp_close(&l2tp_ctrl_array[i]);
			stop_l2tp_ctrl(&l2tp_ctrl_array[i]);
		}		
	}
}


  /***/

  char *u_addrtoa(struct in_addr *addr, char *dst, size_t size)
  {
	  dst[0]=0;
	  inet_ntoa_r(*addr,dst);
	  return dst;
  }


/**************/
 static L2tpInfo
L2tpInit(char *wan_intf, char *username, char * passwd, struct in_addr  *peer_addr, 
			int mtu, unsigned int connect_type,int idle_timeout,int flag)
{
    L2tpInfo	l2tp;
    /* Initialize this link */
    l2tp = (L2tpInfo) (Malloc(MB_PHYS, sizeof(*l2tp)));
    memset(l2tp,0x0,sizeof(*l2tp));
    l2tp->conf.peer_addr=*peer_addr;
    l2tp->conf.peer_port = 1701;
    l2tp->conf.fqdn_peer_addr = NULL;
    strcpy(l2tp->username,username);	
    strcpy(l2tp->passwd,passwd);	
    strcpy(l2tp->wanintfname,wan_intf);
    l2tp->mtu = mtu;
	l2tp->idle_time_limit = idle_timeout*60;
	l2tp->demand_mode = (connect_type == 1)?1:0;	//1->demand_mode ; 0 -> other
    //Enable(&l2tp->conf.options, L2TP_CONF_DATASEQ);
    Enable(&l2tp->conf.options, L2TP_CONF_RESOLVE_ONCE);
    return l2tp;
}

void L2tpFini(L2tpInfo l2tp)
{
	if(l2tp)
		Freee(l2tp);	
}

 uint32_t
 u_addrtoid(const struct in_addr *addr)
 {
	 uint32_t id;
 
	 id = ntohl(addr->s_addr);
	 
	 return (id);
 }
void l2tp_open(char *wan_intf, char *username, char * passwd, struct in_addr  *peer_addr, int mtu, 
				unsigned int connect_type,int idle_timeout, int flag)
 {
	L2tpOpen(L2tpInit(wan_intf,username,passwd, peer_addr, mtu,connect_type,idle_timeout,flag));
 }

void L2tpOpen(L2tpInfo pi)
 {
	 struct l2tp_tun *tun = NULL;
	 struct ppp_l2tp_sess *sess;
	 struct ppp_l2tp_avp_list *avps = NULL;
	 struct ppp_l2tp_ctrl *ctrl;
	char namebuf[64];
	char buf[32], buf2[32];
	char hostname[MAXHOSTNAMELEN];
	u_int32_t       cap;
	u_int16_t	win;

	tun =(struct l2tp_tun *)(Malloc(MB_PHYS, sizeof(*tun)));
	memset(tun,0,sizeof(*tun));

	/*let it be 0, for init SCCQ*/
	//tun->peer_addr = pi->conf.peer_addr;
	tun->peer_port = pi->conf.peer_port;
	tun->self_addr = pi->conf.self_addr;
	tun->self_port = pi->conf.self_port;
	
	/* Create vendor name AVP */
	avps = ppp_l2tp_avp_list_create();

	if (pi->conf.hostname[0] != 0) {
		strlcpy(hostname, pi->conf.hostname, sizeof(hostname));
	} else {
	#if 0
		(void)gethostname(hostname, sizeof(hostname) - 1);
		hostname[sizeof(hostname) - 1] = '\0';
	#endif
	}
	cap = htonl(L2TP_BEARER_DIGITAL|L2TP_BEARER_ANALOG);
	win = htons(8); /* XXX: this value is empirical. */
	if ((ppp_l2tp_avp_list_append(avps, 1, 0, AVP_HOST_NAME,
		  hostname, strlen(hostname)) == -1) ||
		(ppp_l2tp_avp_list_append(avps, 1, 0, AVP_VENDOR_NAME,
		  MPD_VENDOR, strlen(MPD_VENDOR)) == -1) ||
		(ppp_l2tp_avp_list_append(avps, 1, 0, AVP_BEARER_CAPABILITIES,
		  &cap, sizeof(cap)) == -1) ||
		(ppp_l2tp_avp_list_append(avps, 1, 0, AVP_RECEIVE_WINDOW_SIZE,
		  &win, sizeof(win)) == -1)) {
		Perror("L2TP: ppp_l2tp_avp_list_append");
		goto fail;
	}
	/* Create a new control connection */
	if ((tun->ctrl = ppp_l2tp_ctrl_create(&ppp_l2tp_server_ctrl_cb, u_addrtoid(&tun->peer_addr),
	    avps, 
	    pi->conf.secret, strlen(pi->conf.secret),
	    Enabled(&pi->conf.options, L2TP_CONF_HIDDEN))) == NULL) {
		Perror("ppp_l2tp_ctrl_create");
		goto fail;
	}
	ppp_l2tp_ctrl_set_cookie(tun->ctrl, tun);
	/*create socket*/
	l2tp_ksocket(tun->ctrl,&pi->conf.peer_addr,1701);
	
	/*init seq*/
	
	pi->tun = tun;
	tun->ctrl->info = pi;
	tun->active_sessions++;	
	#if 0
	Log(LG_PHYS2, ("L2TP: Control connection %p %s %u <-> %s %u initiated",
	    tun->ctrl, u_addrtoa(&tun->self_addr,buf,sizeof(buf)), tun->self_port,
	    u_addrtoa(&tun->peer_addr,buf2,sizeof(buf2)), tun->peer_port));
	#endif
	
	ppp_l2tp_ctrl_initiate(tun->ctrl);
	/* Clean up and return */
	ppp_l2tp_avp_list_destroy(&avps);

	return;

fail:
	if (tun != NULL) {
		ppp_l2tp_ctrl_destroy(&tun->ctrl);
		Freee(tun);
	}
	/*add HF*/
	if(pi)
		Freee(pi);
 }


 void start_l2tpc(struct sockaddr_in *wan_ip, int flag)
 {
 	 struct ppp_l2tp_ctrl *ctrl;
	ctrl=l2tp_new_ctrl();
	if(NULL == ctrl){
		diag_printf("Create ctrl failed\n");
		return ;
	}
 	l2tp_ksocket(ctrl,wan_ip,1701);
 	//L2tpOpen();	
	ppp_l2tp_initiate(ctrl, 1, 0, 0,ctrl->avps);
 }
 
  /*
   * This is called when a control connection gets opened.
   */
  static void
  ppp_l2tp_ctrl_connected_cb(struct ppp_l2tp_ctrl *ctrl)
  {
	  struct l2tp_tun *tun = ppp_l2tp_ctrl_get_cookie(ctrl);
	  struct ppp_l2tp_sess *sess;
	  struct ppp_l2tp_avp_list *avps = NULL;
	  struct sockaddr_dl  hwa;
	  char	  buf[32], buf2[32];
	  int k;
	  
	  L2tpInfo pi;
  
	  Log(LG_PHYS, ("L2TP: Control connection %p %s %u <-> %s %u connected",
		  ctrl, u_addrtoa(&tun->self_addr,buf,sizeof(buf)), tun->self_port,
		  u_addrtoa(&tun->peer_addr,buf2,sizeof(buf2)), tun->peer_port));
	  

	  pi = (L2tpInfo)ctrl->info;

	  if (pi->tun != tun)
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);

	  tun->connected = 1;
	  /* Create number AVPs */
	  avps = ppp_l2tp_avp_list_create();
	  if (pi->conf.callingnum[0]) {
		 if (ppp_l2tp_avp_list_append(avps, 1, 0, AVP_CALLING_NUMBER,
			  pi->conf.callingnum, strlen(pi->conf.callingnum)) == -1) {
		  Perror("ppp_l2tp_avp_list_append");
		 }
	  }
	  if (pi->conf.callednum[0]) {
		 if (ppp_l2tp_avp_list_append(avps, 1, 0, AVP_CALLED_NUMBER,
			  pi->conf.callednum, strlen(pi->conf.callednum)) == -1) {
		  Perror(" ppp_l2tp_avp_list_append");
		 }
	  }
	  if ((sess = ppp_l2tp_initiate(tun->ctrl,
			  Enabled(&pi->conf.options, L2TP_CONF_OUTCALL)?1:0, 
			  Enabled(&pi->conf.options, L2TP_CONF_LENGTH)?1:0,
			  Enabled(&pi->conf.options, L2TP_CONF_DATASEQ)?1:0,
			  avps)) == NULL) {
		  Perror("ppp_l2tp_initiate");
		  pi->sess = NULL;
		  pi->tun = NULL;
		  if(tun->active_sessions>0)
		  	tun->active_sessions--;
	  }
	  ppp_l2tp_avp_list_destroy(&avps);
	  pi->sess = sess;
	  pi->outcall = Enabled(&pi->conf.options, L2TP_CONF_OUTCALL);
	  Log(LG_PHYS, ("L2TP: %s call #%u via control connection %p initiated", 
			 (pi->outcall?"Outgoing":"Incoming"), 
		  ppp_l2tp_sess_get_serial(sess), tun->ctrl));
	  //ppp_l2tp_sess_set_cookie(sess, l);
	  if (!pi->outcall) {
		  pi->sync = 1;
		  if (pi->rep) {
		  #if 0
		  uint32_t fr;
		  avps = ppp_l2tp_avp_list_create();
		  if (RepIsSync(pi)) {
			  fr = htonl(L2TP_FRAMING_SYNC);
		  } else {
			  fr = htonl(L2TP_FRAMING_ASYNC);
			  pi->sync = 0;
		  }
		  if (ppp_l2tp_avp_list_append(avps, 1, 0, AVP_FRAMING_TYPE,
				  &fr, sizeof(fr)) == -1) {
				  Perror("ppp_l2tp_avp_list_append");
		  }
		  #endif
		  } else {
		  avps = NULL;
		  }
		  ppp_l2tp_connected(pi->sess, avps);
		  if (avps)
		  ppp_l2tp_avp_list_destroy(&avps);
  	};
  }
  
  /*
   * This is called when a control connection is terminated for any reason
   * other than a call ppp_l2tp_ctrl_destroy().
   */
  static void
  ppp_l2tp_ctrl_terminated_cb(struct ppp_l2tp_ctrl *ctrl,
	  u_int16_t result, u_int16_t error, const char *errmsg)
  {
	  struct l2tp_tun *tun = ppp_l2tp_ctrl_get_cookie(ctrl);
	  int k; 
 	   L2tpInfo pi;

	  Log(LG_PHYS, ("L2TP: Control connection %p terminated: %d (%s)", 
		  ctrl, error, errmsg));
  

	  pi = (L2tpInfo)ctrl->info;

	  if (pi->tun != tun)
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);

	  pi->sess = NULL;
	  pi->tun = NULL;
	  if(tun->active_sessions>0)
	  	tun->active_sessions--;
	  pi->callingnum[0]=0;
	  pi->callednum[0]=0;
	  tun->alive = 0;
  }
  
  /*
   * This is called before control connection is destroyed for any reason
   * other than a call ppp_l2tp_ctrl_destroy().
   */
  static void
  ppp_l2tp_ctrl_destroyed_cb(struct ppp_l2tp_ctrl *ctrl)
  {
  	
	  L2tpInfo pi;
	  struct l2tp_tun *tun = ppp_l2tp_ctrl_get_cookie(ctrl);
	  Log(LG_PHYS, ("L2TP: Control connection %p destroyed", ctrl));  

	  pi = (L2tpInfo)ctrl->info;
	  
	  /*Free tun*/
	  Freee(tun);
	  /*Free info*/
	  Freee(pi);
  }
  
  /*
   * This callback is used to report the peer's initiating a new incoming
   * or outgoing call.
   */
  static void
  ppp_l2tp_initiated_cb(struct ppp_l2tp_ctrl *ctrl,
	  struct ppp_l2tp_sess *sess, int out,
	  const struct ppp_l2tp_avp_list *avps,
	  u_char *include_length, u_char *enable_dseq)
  {
//	  struct  l2tp_tun *const tun = ppp_l2tp_ctrl_get_cookie(ctrl);
//	  struct  ppp_l2tp_avp_ptrs *ptrs = NULL;
	  L2tpInfo pi = NULL;
	  int k;
  
	  /* Convert AVP's to friendly form */
	  if ((ppp_l2tp_avp_list2ptrs(avps)) == NULL) {
		  Perror("L2TP: error decoding AVP list");
		  ppp_l2tp_terminate(sess, L2TP_RESULT_ERROR,
			  L2TP_ERROR_GENERIC, strerror(errno));
		  return;
	  }
  
	  Log(LG_PHYS, ("L2TP: %s call #%u via connection %p received", 
		  (out?"Outgoing":"Incoming"), 
		  ppp_l2tp_sess_get_serial(sess), ctrl));

  }
  
  /*
   * This callback is used to report successful connection of a remotely
   * initiated incoming call (see ppp_l2tp_initiated_t) or a locally initiated
   * outgoing call (see ppp_l2tp_initiate()).
   */
  static void
  ppp_l2tp_connected_cb(struct ppp_l2tp_sess *sess,
	  const struct ppp_l2tp_avp_list *avps)
  {  	
	L2tpInfo pi;

	pi = (L2tpInfo)sess->ctrl->info;

	Log(LG_PHYS, ("L2TP: Call #%u connected", 
	  ppp_l2tp_sess_get_serial(sess)));
	  
	start_l2tp_ppp(sess,pi);
#ifdef FAST_L2TP 
	{
		struct ifnet *ifp,*ifp2;
		ifp=ifunit("ppp0");
		if(ifp == NULL)
			diag_printf("ifp is NULL\n");
		if(pi->wanintfname[0]) {
			ifp2=rtl_getDevByName(pi->wanintfname);
			if(NULL == ifp2)
				diag_printf("ifp2 is NULL\n");
		}
		new_l2tp_session(sess, ifp, ifp2, sess->ctrl->tunnel_id, sess->session_id, sess->ctrl->peer_id, sess->peer_id);
	}
#endif
	return;
  }
  
  /*
   * This callback is called when any call, whether successfully connected
   * or not, is terminated for any reason other than explict termination
   * from the link side (via a call to either ppp_l2tp_terminate() or
   * ppp_l2tp_ctrl_destroy()).
   */
  static void
  ppp_l2tp_terminated_cb(struct ppp_l2tp_sess *sess,
	  u_int16_t result, u_int16_t error, const char *errmsg)
  {	
  
#ifdef FAST_L2TP 
  	clear_l2tp_session(sess, sess->ctrl->tunnel_id, sess->session_id);
#endif
	if(sess->ppp_handle)
	{
		/*stop ppp*/
		cyg_ppp_down(sess->ppp_handle );
		//wait will cause hang. why ??? since this funcion is called in alarm_thread. and ppp_main is 
		//also triggered by timer or input data. since wait down will block alarm_thread, the timer in pppd will
		//never be triggered. so ppp will never return when there is no input data.
	 	//cyg_ppp_wait_down(sess->ppp_handle );
		sess->ppp_handle =NULL;
	}	
	
	return;
  }
  
  /*
   * This callback called on receiving link info from peer.
   */
  void
  ppp_l2tp_set_link_info_cb(struct ppp_l2tp_sess *sess,
			  u_int32_t xmit, u_int32_t recv)
  {
  	diag_printf("%s %d\n",__FUNCTION__,__LINE__);
	return;
  }

/*** utility functions****/
char *Mdup_rtl(const char *from, size_t size)
{
   char *to;
  if ((to = (u_char *)malloc(size,M_TEMP,M_DONTWAIT)) == NULL)
    return NULL;
  memcpy(to, from, size);
  return to;
}  
char *Mstrdup_rtl(char *from)
{
	char *to;
	int size;
	size=strlen(from);
	
  	if ((to = (u_char *)malloc(size,M_TEMP,M_DONTWAIT)) == NULL)
    		return NULL;
	strcpy(to,from);
	return to;
}

void *Malloc(char *str, int size)
{
	void *mem;
	mem = malloc(size,M_TEMP,M_DONTWAIT);
	if(NULL == mem)
		return NULL;
	memset(mem,0x0,size);
	return mem;
}

void Freee(char *mem)
{
	if(mem)
		free(mem,M_TEMP);
}


/*
 * Decode ASCII message
 */
void
ppp_util_ascify(char *buf, size_t bsiz, const char *data, size_t len)
{
	char *bp;
	int i;

	for (bp = buf, i = 0; i < len; i++) {
		const char ch = (char)data[i];

		if (bsiz < 3)
			break;
		switch (ch) {
		case '\t':
			*bp++ = '\\';
			*bp++ = 't';
			bsiz -= 2;
			break;
		case '\n':
			*bp++ = '\\';
			*bp++ = 'n';
			bsiz -= 2;
			break;
		case '\r':
			*bp++ = '\\';
			*bp++ = 'r';
			bsiz -= 2;
			break;
		default:
			if (isprint(ch & 0x7f)) {
				*bp++ = ch;
				bsiz--;
			} else {
				*bp++ = '^';
				*bp++ = '@' + (ch & 0x1f);
				bsiz -= 2;
			}
			break;
		}
	}
	*bp = '\0';
}

