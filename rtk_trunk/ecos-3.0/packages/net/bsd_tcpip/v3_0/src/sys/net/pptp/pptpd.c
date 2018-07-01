#include <sys/param.h>
#include <sys/mbuf.h>
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

#include <sys/time.h>
		  
#include "cyg/ppp/pppd.h"
#include "pptp_ctrl.h"
#include "pptp_msg.h"
#include "pptp_options.h"

struct PptpInfo {
	struct in_addr	 local_addr;	/* self IP address */
	struct in_addr	   peer_addr;  /* Peer IP addresses allowed */
	in_port_t	self_port;	/* self port */
	in_port_t	peer_port;	/* Peer port required (or zero) */
//	struct ifnet *ifp;
	char username[32];
	char passwd[32];
	char intfname[32];
	unsigned int idle_time_limit;
	unsigned int demand_mode;	//1: demand 0:other
	unsigned short mtu;
	int mppc_enable;
	int mppe_enable;
};

struct ppp_pptp_ctrl
{
	struct socket * so;
	cyg_ppp_handle_t ppp_handle;	
	struct callout 	 reply_timer;		 /* reply timer */	
	struct callout 	 start_conn_timer;		 /* reply timer */	
};

#define FREE_M(m)							\
	do {								\
		if ((m)) {						\
			m_freem((m));					\
			(m) = NULL;					\
		}							\
	} while (0)
	
#define ERROUT(x)	do { error = (x); goto done; } while (0)
#define PPTPD_THREAD_PRIORITY 14
#define PPTPD_THREAD_STACK_SIZE 0x00002000
int PPTP_REPLY_TIME  = 60 ;
int PPTP_START_CONN_TIME  = 10 ;
int pptp_runing_flag = 0;
cyg_uint8  pptpd_stack[PPTPD_THREAD_STACK_SIZE];
cyg_handle_t pptpd_thread;
cyg_thread  pptpd_thread_object;
PPTP_CONN * conn = NULL;
struct ppp_pptp_ctrl pptp_ctrl;
struct PptpInfo pptp_info;
int ppp_link_down =0;
int pptp_func_off =0;
int pptp_daemon_started=0;
int pptp_closing=0;

static const u_char addrctrl[] = { 0xff, 0x03 };
#if defined (__SVR4) && defined (__sun)
struct in_addr localbind = { INADDR_ANY };
#else
struct in_addr localbind = { INADDR_NONE };
#endif
struct in_addr server_ip;	

#ifndef HZ
#define HZ 100
#endif

void* getWanRealIf()
{
	return (void *)(ifunit(pptp_info.intfname));
}

extern 	int pptp_kernel_debug ;
static void
ksocket_pptp_incoming(struct socket *so, void *arg, int waitflag);

void ppp_pptp_reply_timeout(void *arg)
{
	struct ppp_pptp_ctrl *const ctrl = arg;
    /* check connection state */
    if (conn->conn_state != CONN_ESTABLISHED) {
        if (conn->conn_state == CONN_WAIT_STOP_REPLY) 
            /* hard close. */
            pptp_conn_destroy(conn);
        else /* soft close */
            pptp_conn_close(conn, PPTP_STOP_NONE);
    }
    /* Keep Alives and check echo status */
    if (conn->ka_state == KA_OUTSTANDING) {
#if 0		
        /* no response to keep alive */
        diag_printf("closing control connection due to missing echo reply");
		pptp_conn_close(conn, PPTP_STOP_NONE);
#endif		
    } else { /* ka_state == NONE */ /* send keep-alive */
		/* Remove event */
		callout_stop(&ctrl->reply_timer);
		
		/* Restart idle timer */	
		callout_reset(&ctrl->reply_timer,PPTP_REPLY_TIME * HZ,ppp_pptp_reply_timeout,ctrl);
		
		/* Send request echo packet */
		struct pptp_echo_rqst rqst = {
			PPTP_HEADER_CTRL(PPTP_ECHO_RQST), hton32(conn->ka_id) };
		pptp_send_ctrl_packet(conn, &rqst, sizeof(rqst));
		
		conn->ka_state = KA_OUTSTANDING;
    }
}

void call_reply_timer()
{	
	extern	void ppp_pptp_reply_timeout(void *arg);
	/* Remove event */
	callout_stop(&(pptp_ctrl.reply_timer));
	
	/* Restart idle timer */		
	callout_reset(&(pptp_ctrl.reply_timer),PPTP_REPLY_TIME * HZ, 
		ppp_pptp_reply_timeout, &pptp_ctrl);
}

void ppp_pptp_start_conn_timeout(void *arg)
{

	struct ppp_pptp_ctrl *const ctrl = arg;

    if (!conn && conn->conn_state == CONN_WAIT_CTL_REPLY) {
		/* Remove event */
		callout_stop(&ctrl->start_conn_timer);
		
		/* Restart idle timer */	
		callout_reset(&ctrl->start_conn_timer,PPTP_START_CONN_TIME * HZ,ppp_pptp_start_conn_timeout,ctrl);

		/* Send start request ctrl acket */
        struct pptp_start_ctrl_conn packet = {
            PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RQST),
            hton16(PPTP_VERSION), 0, 0, 
            hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
            hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION), 
            PPTP_HOSTNAME, PPTP_VENDOR
        };
		pptp_send_ctrl_packet(conn, &packet, sizeof(packet));		
    }	
}

void call_start_conn_timer()
{	
	extern	void ppp_pptp_reply_timeout(void *arg);
	/* Remove event */
	callout_stop(&(pptp_ctrl.start_conn_timer));
	
	/* Restart idle timer */		
	callout_reset(&(pptp_ctrl.start_conn_timer),PPTP_START_CONN_TIME * HZ, 
		ppp_pptp_start_conn_timeout, &pptp_ctrl);	
}
/*
	address :remote server ip
	port: server port	
	pptp client bind local ip.
	pptp client need connect to server <ip:port>
*/
void pptp_ksocket(struct ppp_pptp_ctrl *ctrl,struct in_addr *address,unsigned int port)
{
	/*create socket*/
	int ret;
	struct sockaddr_in addr;
	struct sockaddr_in local_addr;

	struct proc *p = curproc ? curproc : &proc0;	/* XXX broken */
	ret=socreate(AF_INET,&ctrl->so,SOCK_RAW,PPTP_PROTO,p);	
		
	if(ret < 0) {
		diag_printf("socreate error\n");
	}

	//raw socket must call sobind function,the packet can rx success!!!
	memset(&local_addr, 0 ,sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_len = sizeof(local_addr);
	local_addr.sin_port = 0;
	local_addr.sin_addr.s_addr = localbind.s_addr; 
	ret =sobind(ctrl->so, (struct sockaddr *)(&local_addr),p);
	if(ret < 0 )
	{
		diag_printf("sobind error\n");
		soclose(ctrl->so);
	}	
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(addr);
	addr.sin_port = 0;
	addr.sin_addr.s_addr = address->s_addr; 	
	ret=soconnect(ctrl->so,(struct sockaddr *)(&addr),p);
	if(ret < 0) {	   
		diag_printf("soconnect error\n");
		soclose(ctrl->so);
	}
	ctrl->so->so_upcallarg = (void *)ctrl;
	ctrl->so->so_upcall = ksocket_pptp_incoming;
	ctrl->so->so_rcv.sb_flags |= SB_UPCALL; 	
}

/*
 * Receive incoming data on our hook.  Send it out the socket.
 */
static int
ksocket_pptp_senddata(struct socket * so, struct mbuf *m)
{
	struct proc *p = curproc ? curproc : &proc0;	/* XXX broken */
	int error;
	struct sockaddr addr;
	error = (*so->so_proto->pr_usrreqs->pru_sosend)(so, 0, 0, m, 0, MSG_DONTWAIT, p);
	return (error);
}
/*
 *	pptp tx wrapper ,called by ppp driver tx (need hook with ppp tx )
 */
int ksocket_pptp_senddata_wrapper(struct socket * so, struct mbuf *m)
{
  /*  encape the gre header ,then socket tx to protocol stack
   *  pppd----> pppctrlsend --> pptpsend-->kernel socket send
   *  so is stored in pptp_ctrl->so.when pppctrlsend need get value from pptp_ctrl
   */
	short len;
	int error;
	struct mbuf *m0;
	len=0;

	for (m0 = m; m0!= 0; m0= m0->m_next)
		len += m0->m_len;
	/*
	 * Remove PPP address and control fields, if any.
	 * For example, ng_ppp(4) always sends LCP packets
	 * with address and control fields as required by
	 * generic PPP.
	 */
	#if 0
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
	#endif
	/*we really want to be aligned on MH_DATA
	  *M_PKTHDR will be clean by M_PREPEND.
	  */	
	if(m->m_flags & M_PKTHDR) {
	}
	else {
		MGETHDR(m0,M_DONTWAIT,MT_HEADER);
		if (m0 == NULL) {
			m_freem(m);
			ERROUT(ENOBUFS);
		}
		/*make head space for gre header*/
		m0->m_data += (sizeof(struct pptp_gre_header));
		m0->m_next=m;
		m0->m_len=0;
		m=m0;
	}
#if 1

	#if 0
	if(!(m->m_flags &  M_PKTHDR))
	{
		m->m_flags |= M_PKTHDR;
		/*since we using M_PKTHDR, we need to clear all to avoid free memory error*/	
		memset(&m->m_pkthdr,0,sizeof(struct pkthdr));
		diag_printf("%s %d ********\n",__FUNCTION__,__LINE__);
	}
	#endif
//	diag_printf("%s %d m->m_len %d\n\n\n",__FUNCTION__,__LINE__,len);
	m->m_pkthdr.len = len;

	/*	
		encap the GRE header.
		need transfer the m address to function call.
		because the function call modify the m address
	*/	
	if(!encap_gre_header(&m,len))
	{		
		diag_printf("%s %d encap gre header fail\n",__FUNCTION__,__LINE__);
		m_freem(m);
		ERROUT(ENOBUFS);
	}
	
	#if 0
	if(!(m->m_flags &  M_PKTHDR))
	{
		m->m_flags |= M_PKTHDR;
		/*since we using M_PKTHDR, we need to clear all to avoid free memory error*/	
		memset(&m->m_pkthdr,0,sizeof(struct pkthdr));
		diag_printf("%s %d ********\n",__FUNCTION__,__LINE__);
	}
	#endif
	if(so == NULL)
	{
		diag_printf("%s.%d.socket null\n",__FUNCTION__,__LINE__);
		m_freem(m);
		ERROUT(ENOBUFS);		
	}
	return ksocket_pptp_senddata(so,m);
done:
	return error;  
#endif	
}

int pptp_handle_out(struct ppp_pptp_ctrl* sess, struct mbuf *m)
{	
	ksocket_pptp_senddata_wrapper(sess->so,m);
	return 0;
}

int pptp_data_handle_in(struct ppp_pptp_ctrl  *sess, struct mbuf *m)
{
	int ret;	
	ret=isIPPkt(m);
	/*we push two for ppp addr and ctrl. through addr & ctrl is not needed*/
	if(ret == 1){
		M_PREPEND(m,2,M_DONTWAIT);
		mtod(m, u_char *)[0]=0xff;
		mtod(m, u_char *)[1]=0x03;
	}
	if(ret == 2) {
		/*Set ppp protocl to 0x0021. since the ppp_inproc only dealwith STD ppp header*/
		M_PREPEND(m,3,M_DONTWAIT);
		mtod(m, u_char *)[0]=0xff;
		mtod(m, u_char *)[1]=0x03;
		mtod(m, u_char *)[2]=0;
	}
	if(0 == ret){
		//other control packet without ff03 bits
		M_PREPEND(m,2,M_DONTWAIT);
		mtod(m, u_char *)[0]=0xff;
		mtod(m, u_char *)[1]=0x03;
	}
	ret=deliver_ppp_data(sess->ppp_handle,m);	
	return ret;
	
}

/*
 *	raw ip socket, may receive the ip header+ gre header
 *  strip the ip header + gre header 	
 * 
 */
static int pptp_rcv_data(struct ppp_pptp_ctrl *ctrl, struct mbuf *m)
{
	int error = -1;
	/*
	 * Remove PPP address and control fields, if any.
	 * For example, ng_ppp(4) always sends LCP packets
	 * with address and control fields as required by
	 * generic PPP. PPPoE is an exception to the rule.
	 */
 
	if (m->m_len >= 2) {
		if (m->m_len < 2 && !(m = m_pullup(m, 2)))
			ERROUT(ENOBUFS);
		#if 1
		if (bcmp(mtod(m, u_char *), addrctrl, 2) == 0)
		{
				m_adj(m, 2);
		}	
		#endif		
	}
	if(ctrl->ppp_handle)
		error=pptp_data_handle_in(ctrl,m);
	else{
		diag_printf("%s,%d, ppp_handle null,error!!!\n",__FUNCTION__,__LINE__);
		FREE_M(m);
	}
done:
	return error;
}

void pptp_rcvdata_lower(struct ppp_pptp_ctrl *arg,struct mbuf *m)
{
	#define PACKET_MAX 100
	unsigned char buffer[PACKET_MAX + 64];
	struct pptp_gre_header *header;
	int status, ip_len = 0;
	u_int32_t seq;
	int total_len = 20 + sizeof(struct pptp_gre_header);
	unsigned char *pdata = NULL;
				
	if (m->m_len < 2 && (m = m_pullup(m, 2)) == NULL) {
		diag_printf("[%s],%d -mbuf error\n",__FUNCTION__,__LINE__);
		return;
	}
	// calculate whether is ip protocol header
	pdata = mtod(m, uint8_t *);
	if((pdata[0] & 0xF0) == 0x40)
		ip_len = (pdata[0] & 0xF) * 4;
	
	if(m->m_len < ip_len && (m = m_pullup(m,ip_len)) == NULL)
	{
		diag_printf("[%s],%d -mbuf error\n",__FUNCTION__,__LINE__);
		return;		
	}

	m_adj(m, ip_len);	//strip the ip header

#if 0
	if(m->m_len < sizeof(struct pptp_gre_header) && 
		(m = m_pullup(m,sizeof(struct pptp_gre_header))) == NULL)
	{
		diag_printf("[%s],%d -mbuf error\n",__FUNCTION__,__LINE__);
		return;				
	}
#endif

	//header = (struct pptp_gre_header *)(mtod(m, uint8_t *));
	/*
		handle the gre header , need some check ,
		gre header filter handle , use the original handle
	*/
	
	if(!decap_gre_header(m))
	{
		return ;	//error ,can't delieve packet to ppp moduel to handle
	}
//	m_adj(m, sizeof(struct pptp_gre_header));	//strip
	/*
		(1) ppp ctrl packet , put the ctrl packet to pppd queue
		(2) ip data packet ,put the data packet to ip queue
	*/
#if 0
	{
		int i = 0;
		
		diag_printf("\n");
		for(i = 0 ;i < 32; ++i)
		{
			diag_printf("%x ",((mtod(m,unsigned char *))[i]));
			if(i ==15)
				diag_printf("\n");
		}
		diag_printf("\n");
	}
#endif

	pptp_rcv_data(&pptp_ctrl,m);
}

/*
 * When incoming data is appended to the socket, we get notified here.
 */
static void
ksocket_pptp_incoming(struct socket *so, void *arg, int waitflag)
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
			
			pptp_rcvdata_lower(arg,m);
		}
	} while (error == 0 && m != NULL);
	splx(s);
}

int start_pptp_ppp(struct PptpInfo *info)
{
	cyg_ppp_handle_t ppp_handle;
	cyg_ppp_options_t options;
    cyg_ppp_options_init( &options );
	
	if(info->demand_mode)
		options.idle_time_limit = info->idle_time_limit;
	else
		options.idle_time_limit = 0;
	
	strcpy(options.user,info->username);
	strcpy(options.passwd,info->passwd);	
	options.mtu = info->mtu;
	options.mppc_Enable = info->mppc_enable ;
	options.mppe_Enable = info->mppe_enable ;
	
	register_ppp_tx_cb(&pptp_ctrl,pptp_handle_out);
	pptp_ctrl.ppp_handle = cyg_ppp_up_rtl("/dev/ser1", &options );
	pptp_closing=0;
	return 0;
}
#if 0
void ppp_pptp_terminated()
{ 
  if(pptp_ctrl.ppp_handle)
  {
		/*stop ppp*/		
		pptp_sessoin_close(conn);	//send pptp request clear
		unregister_ppp_tx_cb(&pptp_ctrl,pptp_handle_out);
		cyg_ppp_down(pptp_ctrl.ppp_handle );		
		pptp_runing_flag = 0 ;
  }
}
#endif

void pptp_stop_link()
{
	if(pptp_ctrl.ppp_handle == NULL)
		return 0;

	if(conn->inet_sock)
		pptp_sessoin_close(conn); //send pptp request clear packet!!!	
	return ;
}

void pptp_close()
{
	//set the state machine to init state.
	extern unsigned int ack_sent, ack_recv;
	extern unsigned int seq_sent, seq_recv;
	extern unsigned int tx_seq; 	/* first sequence number sent must be 1 */

	if(pptp_closing==1)
		return;
	pptp_closing=1;
	ack_sent =	ack_recv = 0;		//reset to init state.
	seq_sent = seq_recv = 0;
	tx_seq = 1;
	//clear pptp session info
	if(pptp_ctrl.ppp_handle == NULL) {
		if(pptp_ctrl.so) {
			soclose(pptp_ctrl.so);		//stop rx
			pptp_ctrl.so=NULL;
		}
		
		if(pptp_runing_flag)
			pptp_func_off =1;
		
		return 0;
	}	
	unregister_ppp_tx_cb(&pptp_ctrl,pptp_handle_out);	//stop tx!!!



	if(conn && conn->inet_sock){
		pptp_sessoin_close(conn); //send pptp request clear packet!!!	
		//conn->inet_sock = 0;
		//close(conn->inet_sock);
	}
	if(pptp_ctrl.so){
		soclose(pptp_ctrl.so);		//stop rx
		pptp_ctrl.so = NULL;
	}

#if 0
	//set the state machine to init state.
	extern unsigned int ack_sent, ack_recv;
	extern unsigned int seq_sent, seq_recv;
	extern unsigned int tx_seq; 	/* first sequence number sent must be 1 */
	ack_sent =  ack_recv = 0;		//reset to init state.
	seq_sent = seq_recv = 0;
	tx_seq = 1;
#endif
	if(pptp_runing_flag)
		pptp_func_off =1;	//force pptpd daemon exit!!!
	cyg_ppp_down(pptp_ctrl.ppp_handle);
 	cyg_ppp_wait_down(pptp_ctrl.ppp_handle);	
 	//cyg_thread_delay( 200 );
	//pptp_ctrl.ppp_handle =NULL;
	memset(&pptp_ctrl,0,sizeof(pptp_ctrl));	
	memset(&pptp_info,0,sizeof(pptp_info));
#ifdef FAST_PPTP
	//clean kernel pptp info!!!!
	clean_fastforward_info();
#endif

}


void pptp_disable(void)
{
	if(pptp_runing_flag)
		pptp_func_off=1;
}

void wrapper_pptp_ppp()
{
	start_pptp_ppp(&pptp_info);
#ifdef FAST_PPTP	
	set_pptp_wan_dev(ifunit("ppp0"),ifunit(pptp_info.intfname));
#endif
}

int open_inetsock(struct in_addr inetaddr)
{
    struct sockaddr_in dest, src;
    int s;
    dest.sin_family = AF_INET;
    dest.sin_port   = htons(PPTP_PORT);
    dest.sin_addr   = inetaddr;
	unsigned int opt = 1;
	
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return s;
    }
		
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) 
	{
		close(s);
		return -1;
	}
	
    if (localbind.s_addr != INADDR_NONE) {
        bzero(&src, sizeof(src));
        src.sin_family = AF_INET;
        src.sin_addr   = localbind;
        if (bind(s, (struct sockaddr *) &src, sizeof(src)) != 0) {
            close(s); 
			return -1;
        }
    }
    if (connect(s, (struct sockaddr *) &dest, sizeof(dest)) < 0) {
        close(s); 			
		return -1;
    }
    return s;
}

void pptp_packet_handle(int pty_fd, int gre_fd, int inet_sock)
{	
	int max_fd = 0 ;
	int retval;

	for (;;) {
	
		struct timeval tv = {0, 0};
		fd_set rfds;		
		if (conn->inet_sock > max_fd) max_fd = conn->inet_sock;
		FD_ZERO(&rfds);
		FD_SET(conn->inet_sock, &rfds);		
		tv.tv_sec  = 1;
		retval = select(max_fd + 1, &rfds, NULL, NULL, &tv);

		if(retval ==-1)	//all socket fd is zero!!!
		{
			break;
		}
		/*TCP control socket ,handle control/manage packet*/
		if (retval > 0 && FD_ISSET(conn->inet_sock, &rfds)) 
		{
			void *buffer; size_t size;
			int r=0;
			r = pptp_read_some(conn->inet_sock , conn);
			if (r < 0)	//read error
			{	
				pptp_close();
				break;
			}
			while (r >= 0 && pptp_check_packet(conn, &buffer, &size)) {
				r = pptp_dispatch_packet(conn, buffer, size);
				//free(buffer);
			}
		}
		//break the for loop,exit the daemon!!!
		#if 1
		if(pptp_func_off)
		{		
			break;			
		}
		#endif
	}
	#if 0
	if(conn->inet_sock >=0){
		close(conn->inet_sock);
		conn->inet_sock = -1 ;
	}
	#endif
}

/*
	schedule policy to retry 
*/
int schedule_Policy()
{
	sleep(5);
	return 1;
}
void __free_Malloc_memory()
{
	/*free read/write buffer*/
	if(!conn)	return ;
	if (conn->read_buffer	!= NULL) 
	{	
		free(conn->read_buffer);
		conn->read_buffer = NULL;
	}
	if (conn->write_buffer != NULL) 
	{	
		free(conn->write_buffer);
		conn->write_buffer = NULL;
	}
	if (conn != NULL ) 
	{
		free(conn);
		conn = NULL;
	}
}


int pptpd_main(cyg_addrword_t data)
{	
    int gre_fd = -1, inet_sock = -1 , pty_fd = -1;
	
	/********tcp socket, rx/tx pptp control/manage packet********/
start_socket:
    if ((inet_sock = open_inetsock(server_ip)) < 0)
    {
		//diag_printf("Could not open control socket, Wait for retry\n");
		/*give other task a chance to run*/
		sleep(1);
		#if 0
		if(schedule_Policy())
			goto start_socket;
		else
		#endif
			goto __release_NON;
	}
	
start_negotiation:
	/*	start the pptp connect (client) ***************************/
	if ((conn = pptp_conn_open(inet_sock, 1, NULL)) == NULL) {
		
		 //diag_printf("Could not open connection. Wait for retry\n");		 
		 sleep(1);
		 #if 0
		 close(inet_sock); 		 
		 if(schedule_Policy())
			goto start_negotiation;
		 else
		 #endif	
		 	goto __release_SOCKET;
	}	
	
	pptp_packet_handle(pty_fd,gre_fd,inet_sock);

__release_ALL:
	__free_Malloc_memory();
__release_SOCKET:
	close(inet_sock);
__release_NON:
	return 0;
}

/*let pptpd always exist*/
int pptpd_daemon(cyg_addrword_t data)
{
	pptp_daemon_started=1;
	while(1)
	{
		if(0==pptp_func_off) {		
			pptp_runing_flag=1;
			pptpd_main(data);
			pptp_runing_flag=0;
		} else {
			/*only sleep here*/
			sleep(2);
		}
	}
}

int create_pptpd(/*unsigned int argc, unsigned char *argv[]*/)
{
	diag_printf("PPTPD startup\n");
	{
		cyg_thread_create(PPTPD_THREAD_PRIORITY,
		pptpd_daemon,
		0,
		"pptpd",
		pptpd_stack,
		sizeof(pptpd_stack),
		&pptpd_thread,
		&pptpd_thread_object);
		cyg_thread_resume(pptpd_thread);
		return(0);
	}
}

int start_pptp(char *wan_iface ,char *username, char * passwd, struct in_addr* local_addr ,
	struct in_addr* peer_addr, int mtu, unsigned int connect_type,int idle_timeout, int flag,int mppc_flag,int mppe_flag)
{
	static int count = 0 ;
	memset(&pptp_info,0,sizeof(pptp_info));
	strcpy(pptp_info.intfname,wan_iface);
	strcpy(pptp_info.username,username);
	strcpy(pptp_info.passwd,passwd);
	pptp_info.local_addr =  *local_addr;	
	pptp_info.peer_addr=  *peer_addr;
	pptp_info.demand_mode = (connect_type == 1)?1:0;	//1->demand_mode ; 0 -> other
	pptp_info.idle_time_limit = idle_timeout*60;
	pptp_info.mtu = mtu;
	pptp_info.mppc_enable = (mppc_flag ? 1:0);
	pptp_info.mppe_enable = (mppe_flag ? 1:0);

#ifdef FAST_PPTP
	set_mppc_mppe_flag(mppc_flag | mppe_flag);
#endif
	//pptp kernel gre socket 
	server_ip = *peer_addr;
	localbind = *local_addr;

	//ppp_link_down = 0;		//start pptp runing flag!!!	
	//pptp_runing_flag = 1;	//set running flag
	//create kernel gre socket
	pptp_ksocket(&pptp_ctrl,&server_ip,0);	

	if(pptp_daemon_started) {
		while(pptp_runing_flag)
		{
			//diag_printf("pptp wait ...\n\n");
			if(++count >= 12)
			{
				count =0;
				return 1;		
			}
			sleep(1);
		}
		pptp_func_off=0;
		return 1;
	}	
	create_pptpd();
	return 1;
}
