/*this file implemented the pppoe module for ecos 
   HF*/

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


//#define PPPOE_DEBUG

#ifdef PPPOE_DEBUG
#define AAA diag_printf("pppoe: %s\n", __FUNCTION__ );
#define BBB diag_printf("-%d-", __LINE__ );
#define DEBUG_PRT diag_printf 
static void mem_dump(unsigned char *buf, int len)
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
	mem_dump(m->m_data,( m->m_len > 32 ) ?  32: m->m_len);	
}
void mdump_m(struct mbuf *m)
{
	struct mbuf *m0;
	for(m0=m;m0!=NULL;m0=m0->m_next)
		dump_m(m0);
}
#define dump_m1 dump_m
#else
#define AAA
#define BBB
#define DEBUG_PRT 
#define dump_m
#endif

#define	LEAVE(x) do { error = x; goto quit; } while(0)

static int nonstandard;

/*related to ecos*/
static int pppoe_enabled;
struct ifqueue	pppoeintrq={0, 0, 0, 100};

struct sess_con gSession[MAX_PPPOE_NUM];


struct ether_header eh_prototype =
	{{0xff,0xff,0xff,0xff,0xff,0xff},
	 {0x00,0x00,0x00,0x00,0x00,0x00},
	 ETHERTYPE_PPPOE_DISC};


static void pppoe_ticker(void *arg);
static void sendpacket(sessp sp);
static void pppoe_start(sessp sp);
static void pppoe_send_data(sessp sp,struct mbuf *m);


/*************************************************************************
 * Some basic utilities  from the Linux version with author's permission.*
 * Author:	Michal Ostrowski <mostrows@styx.uwaterloo.ca>		 *
 ************************************************************************/



#if 0
/*
 * Generate a new session id
 * XXX find out the FreeBSD locking scheme.
 */
static u_int16_t
get_new_sid(node_p node)
{
	static int pppoe_sid = 10;
	sessp sp;
	hook_p	hook;
	u_int16_t val; 
	priv_p privp = node->private;

AAA
restart:
	val = pppoe_sid++;
	/*
	 * Spec says 0xFFFF is reserved.
	 * Also don't use 0x0000
	 */
	if (val == 0xffff) {
		pppoe_sid = 20;
		goto restart;
	}

	/* Check it isn't already in use */
	LIST_FOREACH(hook, &node->hooks, hooks) {
		/* don't check special hooks */
		if ((hook->private == &privp->debug_hook)
		||  (hook->private == &privp->ethernet_hook)) 
			continue;
		sp = hook->private;
		if (sp->Session_ID == val)
			goto restart;
	}

	return val;
}
#endif

/*
 * Return the location where the next tag can be put 
 */
static __inline struct pppoe_tag*
next_tag(struct pppoe_hdr* ph)
{
	return (struct pppoe_tag*)(((char*)&ph->tag[0]) + ntohs(ph->length));
}

/*
 * Look for a tag of a specific type
 * Don't trust any length the other end says.
 * but assume we already sanity checked ph->length.
 */
static struct pppoe_tag*
get_tag(struct pppoe_hdr* ph, u_int16_t idx)
{
	char *end = (char *)next_tag(ph);
	char *ptn;
	struct pppoe_tag *pt = &ph->tag[0];
	/*
	 * Keep processing tags while a tag header will still fit.
	 */
AAA
	while((char*)(pt + 1) <= end) {
	    /*
	     * If the tag data would go past the end of the packet, abort.
	     */
	    ptn = (((char *)(pt + 1)) + ntohs(pt->tag_len));
	    if(ptn > end)
		return NULL;

	    if(pt->tag_type == idx)
		return pt;

	    pt = (struct pppoe_tag*)ptn;
	}
	return NULL;
}

/**************************************************************************
 * inlines to initialise or add tags to a session's tag list,
 **************************************************************************/
/*
 * Initialise the session's tag list
 */
static void
init_tags(sessp sp)
{
AAA
	if(sp->neg == NULL) {
		printf("pppoe: asked to init NULL neg pointer\n");
		return;
	}
	sp->neg->numtags = 0;
}

static void
insert_tag(sessp sp, struct pppoe_tag *tp)
{
	int	i;
	negp neg;

AAA
	if((neg = sp->neg) == NULL) {
		diag_printf("pppoe: asked to use NULL neg pointer\n");
		return;
	}
	if ((i = neg->numtags++) < NUMTAGS) {
		neg->tags[i] = tp;
	} else {
		diag_printf("pppoe: asked to add too many tags to packet\n");
		neg->numtags--;
	}
}

/*
 * Parse an incoming packet to see if any tags should be copied to the
 * output packet. Don't do any tags that have been handled in the main
 * state machine.
 */
static struct pppoe_tag* 
scan_tags(sessp	sp, struct pppoe_hdr* ph)
{
	char *end = (char *)next_tag(ph);
	char *ptn;
	struct pppoe_tag *pt = &ph->tag[0];
	/*
	 * Keep processing tags while a tag header will still fit.
	 */
AAA
	while((char*)(pt + 1) <= end) {
		/*
		 * If the tag data would go past the end of the packet, abort.
		 */
		ptn = (((char *)(pt + 1)) + ntohs(pt->tag_len));
		if(ptn > end)
			return NULL;

		switch (pt->tag_type) {
		case	PTT_RELAY_SID:
			insert_tag(sp, pt);
			break;
		case	PTT_EOL:
			return NULL;
		case	PTT_SRV_NAME:
		case	PTT_AC_NAME:
		case	PTT_HOST_UNIQ:
		case	PTT_AC_COOKIE:
		case	PTT_VENDOR:
		case	PTT_SRV_ERR:
		case	PTT_SYS_ERR:
		case	PTT_GEN_ERR:
			break;
		}
		pt = (struct pppoe_tag*)ptn;
	}
	return NULL;
}


/*
 * Make up a packet, using the tags filled out for the session.
 *
 * Assume that the actual pppoe header and ethernet header 
 * are filled out externally to this routine.
 * Also assume that neg->wh points to the correct 
 * location at the front of the buffer space.
 */
static void
make_packet(sessp sp) {
	struct pppoe_full_hdr *wh = NULL;
	struct pppoe_tag **tag;
	char *dp;
	int count;
	int tlen;
	u_int16_t length = 0;
	if(sp && sp->neg && sp->neg->pkt)
		wh = &sp->neg->pkt->pkt_header;
AAA
	if (sp && ((sp->neg == NULL) || (sp->neg->m == NULL))) {
		printf("pppoe: make_packet called from wrong state\n");
	}
	dp = (char *)wh->ph.tag;
	for (count = 0, tag = sp->neg->tags;
	    ((count < sp->neg->numtags) && (count < NUMTAGS)); 
	    tag++, count++) {
		tlen = ntohs((*tag)->tag_len) + sizeof(**tag);
		if ((length + tlen) > (ETHER_MAX_LEN - 4 - sizeof(*wh))) {
			printf("pppoe: tags too long\n");
			sp->neg->numtags = count;
			break;	/* XXX chop off what's too long */
		}
		bcopy((char *)*tag, (char *)dp, tlen);
		length += tlen;
		dp += tlen;
	}
 	wh->ph.length = htons(length);
	sp->neg->m->m_len = length + sizeof(*wh);
	sp->neg->m->m_pkthdr.len = length + sizeof(*wh);
}


int start_a_ppp(sessp sp)
{
	cyg_ppp_options_t options;
    	cyg_ppp_options_init( &options );
#ifdef PPPOE_DEBUG		
	options.debug = 1;
	options.kdebugflag = 1;
#endif

	#if 0
	extern int	demand;	
	demand = sp->demand_mode;
	#endif
	if(sp->demand_mode)
		options.idle_time_limit = sp->idle_time_limit;
	else
		options.idle_time_limit = 0;
	strcpy(options.user,sp->username);
	strcpy(options.passwd,sp->passwd);
	options.mtu = sp->mtu;
	// options.flowctl = CYG_PPP_FLOWCTL_SOFTWARE;
	sp->ppp_handle = cyg_ppp_up_rtl("/dev/ser1", &options );
	//cyg_ppp_wait_up(sp->ppp_handle);
	AAA
	return 1;
}

int stop_a_ppp(sessp sp)
{
	if(sp->ppp_handle == NULL) {
		/*remove timer */
		untimeout(pppoe_ticker, sp);
		diag_printf("no ppp to close\n");
		return 0;
	}
#ifdef CONFIG_RTL_FAST_PPPOE	
	clear_pppoe_info(NULL,NULL,sp->Session_ID,0,0,NULL,NULL);
#endif
#if defined (CONFIG_RTL_PPPOE_DIRECT_REPLY)
		clear_magicNum(); 
#endif
	cyg_ppp_down(sp->ppp_handle );
 	cyg_ppp_wait_down(sp->ppp_handle );
	sp->state = PPPOE_DEAD;
	send_PADT(sp);
 	cyg_thread_delay( 200 );
	sp->ppp_handle=NULL;
	return 0;
}

/*HF Borrow sp->pkt_hdr for send PADT*/
int send_PADT(sessp sp)
{
	struct mbuf *m;
	struct pppoe_full_hdr *wh;
	MGETHDR(m,M_NOWAIT,MT_DATA);
	if(m == NULL)
	{
		diag_printf("no M\n");
		return -1;
	}
	if(sp->state != PPPOE_DEAD)
		return -1;

	sp->pkt_hdr.eh.ether_type=htons(ETHERTYPE_PPPOE_DISC);
	sp->pkt_hdr.ph.ver = 0x1;
	sp->pkt_hdr.ph.type = 0x1;
	sp->pkt_hdr.ph.code=PADT_CODE;
	sp->pkt_hdr.ph.length = htons((short)(0));

	/*sid not change. no tag info*/
	
	wh = mtod(m, struct pppoe_full_hdr *);
	bcopy(&sp->pkt_hdr, wh, sizeof(*wh));
	m->m_len= sizeof(struct pppoe_full_hdr);
	m->m_pkthdr.len = 0;
	pppoe_send_data(sp,m);
	return 0;
}

int deliver_ppp_data(cyg_ppp_handle_t handle, struct mbuf *m);

static const u_char addrctrl[] = { 0xff, 0x03 };

int send_to_ppp(struct sess_con *psess,struct mbuf *m)
{
	int error;
	int ret;
	if(psess->ppp_handle == NULL)
		return -1;
	AAA
	error=0;
	dump_m(m);

	/*m need to FIX. since m is from skb with pppoe header*/
	m->m_data += sizeof(struct pppoe_full_hdr);
	m->m_len -= sizeof(struct pppoe_full_hdr);
	m->m_pkthdr.len -= sizeof(struct pppoe_full_hdr);
       
	/*
	 * Remove PPP address and control fields, if any.
	 * For example, ng_ppp(4) always sends LCP packets
	 * with address and control fields as required by
	 * generic PPP. PPPoE is an exception to the rule.
	 */
	if (m->m_len >= 2) {
		if (m->m_len < 2 && !(m = m_pullup(m, 2)))
			LEAVE(ENOBUFS);
		if (bcmp(mtod(m, u_char *), addrctrl, 2) == 0)
		{
			m_adj(m, 2);
		}
	}
	
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
		
		error=deliver_ppp_data(psess->ppp_handle,m);
	}
	else {
		
		error=deliver_ppp_ctrl(psess->ppp_handle,m);
	}
	
quit:
	return error;
}

int xmit_data_from_ppp(void *sp, struct mbuf *m);
static int pppoe_connected(sessp sp)
{
	AAA
	register_ppp_tx_cb((void *)sp,xmit_data_from_ppp);
#ifdef CONFIG_RTL_FAST_PPPOE 	
	set_pppoe_info("ppp0",sp->intfname,sp->Session_ID,0,0,sp->pkt_hdr.eh.ether_shost,sp->pkt_hdr.eh.ether_dhost);
#endif
	start_a_ppp(sp);
	return 0;
}



static int pppoe_rcvdata(struct sess_con *psess,struct mbuf *m)
{
	sessp			sp = psess;
	struct pppoe_full_hdr	*wh;
	struct pppoe_hdr	*ph;
	int			error = 0;
	u_int16_t		session;
	u_int16_t		length;
	u_int8_t		code;
	struct pppoe_tag	*utag = NULL, *tag = NULL;
	struct {
		struct pppoe_tag hdr;
		union	uniq	data;
	} __attribute ((packed)) uniqtag;
	negp			neg = NULL;

	AAA

	if(sp->state == PPPOE_SNONE)
	{
		/*pppoe not started, Free m*/
		LEAVE(-1);
	}
	if( m->m_len < sizeof(*wh)) {
		m = m_pullup(m, sizeof(*wh)); /* Checks length */
		if (m == NULL) {
			printf("couldn't m_pullup\n");
			LEAVE(ENOBUFS);
		}
	}
	wh = mtod(m, struct pppoe_full_hdr *);
	ph = &wh->ph;
	session = ntohs(wh->ph.sid);
	length = ntohs(wh->ph.length);
	code = wh->ph.code; 
	DEBUG_PRT("code %x\n",code);
	switch(wh->eh.ether_type) {

	case	ETHERTYPE_PPPOE_STUPID_DISC:
		nonstandard = 1;
		eh_prototype.ether_type = ETHERTYPE_PPPOE_STUPID_DISC;
		/* fall through */
		
	case	ETHERTYPE_PPPOE_DISC:
		DEBUG_PRT("%s %d\n",__FUNCTION__,__LINE__);
		/*
		 * We need to try to make sure that the tag area
		 * is contiguous, or we could wander off the end
		 * of a buffer and make a mess. 
		 * (Linux wouldn't have this problem).
		 */
		/*XXX fix this mess */
		
		if (m->m_pkthdr.len <= MHLEN) {
			if( m->m_len < m->m_pkthdr.len) {
				m = m_pullup(m, m->m_pkthdr.len);
				if (m == NULL) {
					
					DEBUG_PRT("%s %d\n",__FUNCTION__,__LINE__);
					printf("couldn't m_pullup\n");
					LEAVE(ENOBUFS);
				}
			}
		}
		if (m->m_len != m->m_pkthdr.len) {
			/*
			 * It's not all in one piece.
			 * We need to do extra work.
			 */
			 
			DEBUG_PRT("%s %d\n",__FUNCTION__,__LINE__);
			printf("packet fragmented\n");
			LEAVE(EMSGSIZE);
		}

		switch(code) {
#if 0				
		case	PADI_CODE:
			/*
			 * We are a server:
			 * Look for a hook with the required service
			 * and send the ENTIRE packet up there.
			 * It should come back to a new hook in 
			 * PRIMED state. Look there for further
			 * processing.
			 */
			tag = get_tag(ph, PTT_SRV_NAME);
			if (tag == NULL) {
				printf("no service tag\n");
				LEAVE(ENETUNREACH);
			}
			sendhook = pppoe_match_svc(hook->node,
		    		tag->tag_data, ntohs(tag->tag_len),
				NG_MATCH_ANY);
			if (sendhook) {
				NG_SEND_DATA(error, sendhook, m, meta);
			} else {
				LEAVE(ENETUNREACH);
			}
			break;
#endif					
		case	PADO_CODE:
			/*
			 * We are a client:
			 * Use the host_uniq tag to find the 
			 * hook this is in response to.
			 * Received #2, now send #3
			 * For now simply accept the first we receive.
			 */
			 
			DEBUG_PRT("%s %d\n",__FUNCTION__,__LINE__);
			utag = get_tag(ph, PTT_HOST_UNIQ);
			if ((utag == NULL)
			|| (ntohs(utag->tag_len) != sizeof(sp))) {
				printf("no host unique field\n");
				LEAVE(ENETUNREACH);
			}

#if 0
			sendhook = pppoe_finduniq(node, utag);
			if (sendhook == NULL) {
				printf("no matching session\n");
				LEAVE(ENETUNREACH);
			}
#endif					

			/*
			 * Check the session is in the right state.
			 * It needs to be in PPPOE_SINIT.
			 */
			if (sp->state != PPPOE_SINIT) {
				printf("session in wrong state\n");
				LEAVE(ENETUNREACH);
			}
			neg = sp->neg;
			/*HF 20120829
			untimeout(pppoe_ticker, sendhook,
			    neg->timeout_handle);
			*/
			untimeout(pppoe_ticker, sp);

			/*
			 * This is the first time we hear
			 * from the server, so note it's
			 * unicast address, replacing the
			 * broadcast address .
			 */
			bcopy(wh->eh.ether_shost,
				neg->pkt->pkt_header.eh.ether_dhost,
				ETHER_ADDR_LEN);
			neg->timeout = 0;
			neg->pkt->pkt_header.ph.code = PADR_CODE;
			init_tags(sp);
			insert_tag(sp, utag);      /* Host Unique */
			if ((tag = get_tag(ph, PTT_AC_COOKIE)))
				insert_tag(sp, tag); /* return cookie */
			if ((tag = get_tag(ph, PTT_AC_NAME)))
				insert_tag(sp, tag); /* return it */
			insert_tag(sp, &neg->service.hdr); /* Service */
			scan_tags(sp, ph);
			make_packet(sp);
			sp->state = PPPOE_SREQ;
			sendpacket(sp);
			break;
#if 0				
		case	PADR_CODE:

			/*
			 * We are a server:
			 * Use the ac_cookie tag to find the 
			 * hook this is in response to.
			 */
			utag = get_tag(ph, PTT_AC_COOKIE);
			if ((utag == NULL)
			|| (ntohs(utag->tag_len) != sizeof(sp))) {
				LEAVE(ENETUNREACH);
			}

			sendhook = pppoe_finduniq(node, utag);
			if (sendhook == NULL) {
				LEAVE(ENETUNREACH);
			}

			/*
			 * Check the session is in the right state.
			 * It needs to be in PPPOE_SOFFER
			 * or PPPOE_NEWCONNECTED. If the latter,
			 * then this is a retry by the client.
			 * so be nice, and resend.
			 */
			sp = sendhook->private;
			if (sp->state == PPPOE_NEWCONNECTED) {
				/*
				 * Whoa! drop back to resend that 
				 * PADS packet.
				 * We should still have a copy of it.
				 */
				sp->state = PPPOE_SOFFER;
			}
			if (sp->state != PPPOE_SOFFER) {
				LEAVE (ENETUNREACH);
				break;
			}
			neg = sp->neg;
			/* HF 20120829
			untimeout(pppoe_ticker, sendhook,
			    neg->timeout_handle);
			*/
			untimeout(pppoe_ticker, sendhook);
			
			neg->pkt->pkt_header.ph.code = PADS_CODE;
			if (sp->Session_ID == 0)
				neg->pkt->pkt_header.ph.sid =
				    htons(sp->Session_ID
					= get_new_sid(node));
			neg->timeout = 0;
			/*
			 * start working out the tags to respond with.
			 */
			init_tags(sp);
			insert_tag(sp, &neg->ac_name.hdr); /* AC_NAME */
			if ((tag = get_tag(ph, PTT_SRV_NAME)))
				insert_tag(sp, tag);/* return service */
			if ((tag = get_tag(ph, PTT_HOST_UNIQ)))
				insert_tag(sp, tag); /* return it */
			insert_tag(sp, utag);	/* ac_cookie */
			scan_tags(sp, ph);
			make_packet(sp);
			sp->state = PPPOE_NEWCONNECTED;
			sendpacket(sp);
			/*
			 * Having sent the last Negotiation header,
			 * Set up the stored packet header to 
			 * be correct for the actual session.
			 * But keep the negotialtion stuff
			 * around in case we need to resend this last 
			 * packet. We'll discard it when we move
			 * from NEWCONNECTED to CONNECTED
			 */
			sp->pkt_hdr = neg->pkt->pkt_header;
			if (nonstandard)
				sp->pkt_hdr.eh.ether_type
					= ETHERTYPE_PPPOE_STUPID_SESS;
			else
				sp->pkt_hdr.eh.ether_type
					= ETHERTYPE_PPPOE_SESS;
			sp->pkt_hdr.ph.code = 0;
			pppoe_send_event(sp, NGM_PPPOE_SUCCESS);
			break;
#endif					
		case	PADS_CODE:
			/*
			 * We are a client:
			 * Use the host_uniq tag to find the 
			 * hook this is in response to.
			 * take the session ID and store it away.
			 * Also make sure the pre-made header is
			 * correct and set us into Session mode.
			 */
			DEBUG_PRT("%s %d\n",__FUNCTION__,__LINE__);
			utag = get_tag(ph, PTT_HOST_UNIQ);
			if ((utag == NULL)
			|| (ntohs(utag->tag_len) != sizeof(sp))) {
			
				DEBUG_PRT("%s %d\n",__FUNCTION__,__LINE__);
				LEAVE (ENETUNREACH);
				break;
			}
#if 0					
			sendhook = pppoe_finduniq(node, utag);
			if (sendhook == NULL) {
				LEAVE(ENETUNREACH);
			}
#endif
			/*
			 * Check the session is in the right state.
			 * It needs to be in PPPOE_SREQ.
			 */
			if (sp->state != PPPOE_SREQ) {
				DEBUG_PRT("%s[%d]\n",__FUNCTION__,__LINE__);
				LEAVE(ENETUNREACH);
			}
			neg = sp->neg;
			/* HF  20120829
			untimeout(pppoe_ticker, sendhook,
			    neg->timeout_handle);
			*/
			untimeout(pppoe_ticker, sp);

			neg->pkt->pkt_header.ph.sid = wh->ph.sid;
			sp->Session_ID = ntohs(wh->ph.sid);
			neg->timeout = 0;
			sp->state = PPPOE_CONNECTED;
			/*
			 * Now we have gone to Connected mode, 
			 * Free all resources needed for 
			 * negotiation.
			 * Keep a copy of the header we will be using.
			 */
			sp->pkt_hdr = neg->pkt->pkt_header;
			if (nonstandard)
				sp->pkt_hdr.eh.ether_type
					= ETHERTYPE_PPPOE_STUPID_SESS;
			else
				sp->pkt_hdr.eh.ether_type
					= ETHERTYPE_PPPOE_SESS;
			sp->pkt_hdr.ph.code = 0;

			#if 1/*HF temply we don't free it. FIX BUG*/
			m_freem(neg->m);
			FREE(sp->neg, M_NETGRAPH);
			sp->neg = NULL;
			sp->inited=0;
			#endif
			diag_printf("\n********************************************\n");
			diag_printf("[%s,%d],session_ID is:%d\n",__FUNCTION__,__LINE__,sp->Session_ID);
			diag_printf("%02X:%02X:%02X:%02X:%02X:%02X",sp->pkt_hdr.eh.ether_dhost[0],sp->pkt_hdr.eh.ether_dhost[1],sp->pkt_hdr.eh.ether_dhost[2],sp->pkt_hdr.eh.ether_dhost[3],sp->pkt_hdr.eh.ether_dhost[4],sp->pkt_hdr.eh.ether_dhost[5]);
			if(sp->save_connInfo && sp->Session_ID)
				sp->save_connInfo(sp->Session_ID,sp->pkt_hdr.eh.ether_dhost);
			#if 0
			pppoe_send_event(sp, NGM_PPPOE_SUCCESS);
			#else
			pppoe_connected(sp);
			#endif

			break;
		case	PADT_CODE:
			/*
			 * Send a 'close' message to the controlling
			 * process (the one that set us up);
			 * And then tear everything down.
			 *
			 * Find matching peer/session combination.
			 */

			/*
			  * Show Down PPPD Daemon. FIX ME !!!
			  */
			#if 0  
			sendhook = pppoe_findsession(node, wh);
			NG_FREE_DATA(m, meta); /* no longer needed */
			if (sendhook == NULL) {
				LEAVE(ENETUNREACH);
			}
			/* send message to creator */
			/* close hook */
			if (sendhook) {
				ng_destroy_hook(sendhook);
			}
			#endif
			/*HF . need to check session number. 20121120*/
			if(session == psess->Session_ID) {
				if(sp->state == PPPOE_CONNECTED) {
					diag_printf("Got PADT\n");
					//diag_printf("%s %d PADT\n",__FUNCTION__,__LINE__);
					sp->state = PPPOE_DEAD;
			#ifdef CONFIG_RTL_FAST_PPPOE		
					clear_pppoe_info(NULL,NULL,session,0,0,NULL,NULL);
			#endif
			#if defined (CONFIG_RTL_PPPOE_DIRECT_REPLY)
				clear_magicNum(); 
			#endif
					if(sp->ppp_handle == NULL) {
						diag_printf("no ppp to close\n");
						return 0;
					}	
					cyg_ppp_down(sp->ppp_handle );
					send_PADT(sp);			
					/*HF note:
	                             *if stop a ppp will call cyg_thread_delay.
	                             *since in tasklet context it will block any other
	                             *process.so do not call the stop_a_ppp.
					  */				  
					//stop_a_ppp(psess);				
				}
			}	
			break;
		default:
			LEAVE(EPFNOSUPPORT);
		}
		break;
		
	case	ETHERTYPE_PPPOE_STUPID_SESS:
	case	ETHERTYPE_PPPOE_SESS:
	#if 0	
		/*
		 * find matching peer/session combination.
		 */
		sendhook = pppoe_findsession(node, wh);
		if (sendhook == NULL) {
			LEAVE (ENETUNREACH);
			break;
		}
		sp = sendhook->private;
		m_adj(m, sizeof(*wh));
		if (m->m_pkthdr.len < length) {
			/* Packet too short, dump it */
			LEAVE(EMSGSIZE);
		}

		/* Also need to trim excess at the end */
		if (m->m_pkthdr.len > length) {
			m_adj(m, -((int)(m->m_pkthdr.len - length)));
		}
		if ( sp->state != PPPOE_CONNECTED) {
			if (sp->state == PPPOE_NEWCONNECTED) {
				sp->state = PPPOE_CONNECTED;
				/*
				 * Now we have gone to Connected mode, 
				 * Free all resources needed for 
				 * negotiation. Be paranoid about
				 * whether there may be a timeout.
				 */
				m_freem(sp->neg->m);
				/* HF 20120829
				untimeout(pppoe_ticker, sendhook,
			    		sp->neg->timeout_handle);
				*/
				untimeout(pppoe_ticker, sendhook);

				FREE(sp->neg, M_NETGRAPH);
				sp->neg = NULL;
			} else {
				LEAVE (ENETUNREACH);
				break;
			}
		}
		NG_SEND_DATA( error, sendhook, m, meta);
	#else
		DEBUG_PRT("%s %d\n",__FUNCTION__,__LINE__);
		/*HF . need to check session number. 20121120*/
		if(session == psess->Session_ID) {
			if(psess->state == PPPOE_CONNECTED)
			{
				error=send_to_ppp(psess,m);
				if(error < 0)
					goto quit;
				/*upper layer will free the m, so just return*/
				return error;
			}	
		}
		/*pass up to ppp interface*/
	#endif
		break;
	default:
		LEAVE(EPFNOSUPPORT);
	}

quit:
	
	DEBUG_PRT("%s %d\n",__FUNCTION__,__LINE__);
	PPPOE_FREE_M(m);
	return error;
} 
void
pppoe_input(struct mbuf *m)
{
	AAA
	/*back to eth header only if skb to mbuf. problem ?*/
	m->m_data -=sizeof(struct ether_header);
	m->m_len += sizeof(struct ether_header);
	m->m_pkthdr.len += sizeof(struct ether_header);
	pppoe_rcvdata(&gSession[0],m);
}

static void
pppoeintr(void)
{
	int s;
	struct mbuf *m;
	AAA
	while(1) {
		s = splimp();
		IF_DEQUEUE(&pppoeintrq, m);
		splx(s);
		if (m == 0)
			return;
		pppoe_input(m);
	}
}

sessp pppoe_selectSession(int session_id)
{
//	int i;
	if(session_id<MAX_PPPOE_NUM) {
		return &gSession[session_id]; 
	} else {
		return NULL;
	}	
}

int pppoe_initSession(sessp sp, char *servicename,char *acname)
{
	int error=0;
	negp neg = NULL;
	AAA

	/*
	 * set up prototype header
	 */
	MALLOC(neg, negp, sizeof(*neg), M_NETGRAPH, M_NOWAIT);

	if (neg == NULL) {
		printf("pppoe: Session out of memory\n");
		LEAVE(ENOMEM);
	}
	bzero(neg, sizeof(*neg));
	MGETHDR(neg->m, M_DONTWAIT, MT_DATA);
	if(neg->m == NULL) {
		printf("pppoe: Session out of mbufs\n");
		FREE(neg, M_NETGRAPH);
		LEAVE(ENOBUFS);
	}
	neg->m->m_pkthdr.rcvif = NULL;
	//dzh add for reserved cluster
	#ifdef PPP_RESERVED_CLUSTER_SUPPORT //dzh add for cluster
	if(PPP_MCLGET(neg->m))
		;
	else
	#endif
	MCLGET(neg->m, M_DONTWAIT);
	if ((neg->m->m_flags & M_EXT) == 0) {
		printf("pppoe: Session out of mcls\n");
		m_freem(neg->m);
		FREE(neg, M_NETGRAPH);
		LEAVE(ENOBUFS);
	}
	sp->neg = neg;
	
	//HF 20120829
	//callout_handle_init( &neg->timeout_handle);
	neg->m->m_len = sizeof(struct pppoe_full_hdr);
	neg->pkt = mtod(neg->m, union packet*);
	neg->pkt->pkt_header.eh = eh_prototype;
	neg->pkt->pkt_header.ph.ver = 0x1;
	neg->pkt->pkt_header.ph.type = 0x1;
	neg->pkt->pkt_header.ph.sid = 0x0000;
	neg->timeout = 0;

	//strncpy(sp->creator, retaddr, NG_NODELEN);
	//sp->creator[NG_NODELEN] = '\0';
	sp->inited = 1;


			
	neg->service.hdr.tag_type = PTT_SRV_NAME;
	
	if(servicename)
	{
		neg->service.hdr.tag_len =strlen(servicename);

		bcopy(servicename, neg->service.data,
		    neg->service.hdr.tag_len);
	}
	else
	{
		/*zero*/
		neg->service.hdr.tag_len=0;
	}
	neg->service_len = neg->service.hdr.tag_len;


	neg->ac_name.hdr.tag_type = PTT_AC_NAME;

	if(acname)
	{
		neg->ac_name.hdr.tag_len =strlen(acname);
		bcopy(acname, neg->ac_name.data,
		    neg->ac_name.hdr.tag_len);
	}
	else
	{
		/*zero*/
		neg->ac_name.hdr.tag_len=0;
	}
	neg->ac_name_len= neg->ac_name.hdr.tag_len;
quit:
	return(error);
}


int pppoe_finiSession(sessp sp)
{
	int error=0;
	negp neg = NULL;
	AAA

	neg=sp->neg;
	/*
	  * Free M
	  */
	if(neg && neg->m) {
		MFREE(neg->m,neg->m);
	}

	/*
	  *Free neg
	  */
	 if(neg)
	 {
	 	FREE(neg,M_NETGRAPH);
	 }

	 /*
         * Clean
	  */
	  memset(sp,0x0,sizeof(*sp));

quit:
	return(error);
}

static void pppoe_send_data(sessp sp,struct mbuf *m)
{
	AAA
	ether_output_frame(sp->ifp,m);
}


int xmit_data_from_ppp(void *sp, struct mbuf *m)
{
	return deliver_data_from_ppp(m);
}
int deliver_data_from_ppp(struct mbuf *m)
{
	/*raw ppp need to add header*/
	sessp sp;
	short len;
	int error = -1;
	struct mbuf *m0;
	struct pppoe_full_hdr *wh;

	sp=&gSession[0];

	len=0;

	/*if  pppoe session dead, no need to TX*/
	if(sp && sp->state == PPPOE_DEAD)	{
		m_freem(m);
		LEAVE(-1);
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
		DEBUG_PRT("%s %d  \n",__FUNCTION__,__LINE__);
		//diag_printf("m->m_len %d\n",m->m_len);
		if (m->m_len < 2 && !(m = m_pullup(m, 2)))
			LEAVE(ENOBUFS);
		if (bcmp(mtod(m, u_char *), addrctrl, 2) == 0)
		{
			DEBUG_PRT("%s %d \n",__FUNCTION__,__LINE__);
			m_adj(m, 2);
			len-=2;
		}	
	}

	/*
	 * Bang in a pre-made header, and set the length up
	 * to be correct. Then send it to the ethernet driver.
	 * But first correct the length.
	 */
	sp->pkt_hdr.ph.length = htons((short)(len));

	/*we really want to be aligned on MH_DATA
	  *M_PKTHDR will be clean by M_PREPEND.
	  */
	  
	if(m->m_flags & M_PKTHDR) {
		//diag_printf("M_LEADINGSPACE(m):%d\n",M_LEADINGSPACE(m));
		M_PREPEND(m, sizeof(*wh), M_DONTWAIT);
			if (m == NULL) {
			LEAVE(ENOBUFS);
		}
	}
	else {
		MGETHDR(m0,M_DONTWAIT,MT_HEADER);
		if (m0 == NULL) {
			m_freem(m);
			LEAVE(ENOBUFS);
		}
		MH_ALIGN(m0, sizeof(*wh));
		m0->m_len = sizeof(*wh);
		m0->m_next=m;
		m=m0;
	}	

	wh = mtod(m, struct pppoe_full_hdr *);
	bcopy(&sp->pkt_hdr, wh, sizeof(*wh));

	if(!(m->m_flags &  M_PKTHDR))
	{
		m->m_flags |= M_PKTHDR;
		/*since we using M_PKTHDR, we need to clear all to avoid free memory error*/	
		memset(&m->m_pkthdr,0,sizeof(struct pkthdr));
	}
	m->m_pkthdr.len = len;
	//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
	pppoe_send_data(sp,m);

quit:
	return error;
}

/*
 * timeouts come here.
 */
static void
pppoe_ticker(void *arg)
{
	int s = splnet();
	sessp	sp = arg;
	negp	neg = sp->neg;
	int	error = 0;
	struct mbuf *m0 = NULL;

AAA
	switch(sp->state) {
		/*
		 * resend the last packet, using an exponential backoff.
		 * After a period of time, stop growing the backoff,
		 * and either leave it, or revert to the start.
		 */
	case	PPPOE_SINIT:
	case	PPPOE_SREQ:
		/* timeouts on these produce resends */
		m0 = m_copypacket(sp->neg->m, M_DONTWAIT);
		pppoe_send_data(sp, m0);

		untimeout(pppoe_ticker, sp);
		timeout(pppoe_ticker,sp, neg->timeout * hz);

		if ((neg->timeout <<= 1) > PPPOE_TIMEOUT_LIMIT) {
			if (sp->state == PPPOE_SREQ) {
				/* revert to SINIT mode */
				pppoe_start(sp);
			} else {
				neg->timeout = PPPOE_TIMEOUT_LIMIT;
			}
		}
		break;
#if 0		
	case	PPPOE_PRIMED:
	case	PPPOE_SOFFER:
		/* a timeout on these says "give up" */
		ng_destroy_hook(hook);
		break;
#endif		
	default:
		/* timeouts have no meaning in other states */
		printf("pppoe: unexpected timeout\n");
	}
	splx(s);
}

static void
sendpacket(sessp sp)
{
	int	error = 0;
	struct mbuf *m0 = NULL;
	negp	neg = sp->neg;

AAA
	switch(sp->state) {
	case	PPPOE_LISTENING:
	case	PPPOE_SNONE:
	case 	PPPOE_DEAD:
	case	PPPOE_CONNECTED:
		printf("pppoe: sendpacket: unexpected state\n");
		break;

#if 0 /*server function*/
	case	PPPOE_NEWCONNECTED:
		/* send the PADS without a timeout - we're now connected */
		m0 = m_copypacket(sp->neg->m, M_DONTWAIT);
		NG_SEND_DATA( error, privp->ethernet_hook, m0, dummy);
		break;

	case	PPPOE_PRIMED:
		/* No packet to send, but set up the timeout */
		/* HF 20120829
		neg->timeout_handle = timeout(pppoe_ticker,
					hook, PPPOE_OFFER_TIMEOUT * hz);
		*/
		timeout(pppoe_ticker,hook, PPPOE_OFFER_TIMEOUT * hz);
		break;

	case	PPPOE_SOFFER:
		/*
		 * send the offer but if they don't respond
		 * in PPPOE_OFFER_TIMEOUT seconds, forget about it.
		 */
		m0 = m_copypacket(sp->neg->m, M_DONTWAIT);
		NG_SEND_DATA( error, privp->ethernet_hook, m0, dummy);
		/* HF 20120829
		neg->timeout_handle = timeout(pppoe_ticker,
					hook, PPPOE_OFFER_TIMEOUT * hz)
		*/
		timeout(pppoe_ticker,hook, PPPOE_OFFER_TIMEOUT * hz);
		break;
#endif

	case	PPPOE_SINIT:
	case	PPPOE_SREQ:
		m0 = m_copypacket(sp->neg->m, M_DONTWAIT);
		pppoe_send_data(sp,m0);
		untimeout(pppoe_ticker, sp);
		timeout(pppoe_ticker, sp,(hz * PPPOE_INITIAL_TIMEOUT));		
		neg->timeout = PPPOE_INITIAL_TIMEOUT;
		break;

	default:
		error = EINVAL;
		printf("pppoe: timeout: bad state\n");
	}
	/* return (error); */
}



/*
 * Start a client into the first state. A separate function because
 * it can be needed if the negotiation times out.
 */
static void
pppoe_start(sessp sp)
{
	struct {
		struct pppoe_tag hdr;
		union	uniq	data;
	} __attribute ((packed)) uniqtag;

	AAA

	/* 
	 * kick the state machine into starting up
	 */
	 
	sp->state = PPPOE_SINIT;
	/* reset the packet header to broadcast */
	sp->neg->pkt->pkt_header.eh = eh_prototype;
	sp->neg->pkt->pkt_header.ph.code = PADI_CODE;
	/*HF add. only sid be 0, serve will feedback*/
	sp->neg->pkt->pkt_header.ph.sid=0;
	uniqtag.hdr.tag_type = PTT_HOST_UNIQ;
	uniqtag.hdr.tag_len = htons((u_int16_t)sizeof(uniqtag.data));
	uniqtag.data.pointer = sp;
 	init_tags(sp);	
 	insert_tag(sp, &sp->neg->service.hdr);
 	insert_tag(sp, &uniqtag.hdr);
 	make_packet(sp);
 	sendpacket(sp);
	
 }

#if 0
void pppoe_stop(char *intname, unsigned int flag)
{
	AAA
}
#endif
#define IFP2AC(IFP) ((struct arpcom *)IFP)

#if 0
int ppp_dail_on_demand(int id)
{
	//setup ppp0 ip ,route ,flags
	//when lan pc has packet to wan ,cause the pppoe dial up 
	sessp sp;
	
	AAA
	/*Start PPPOE DISCOVERY*/
	sp=pppoe_selectSession(id);	

//	extern int setup_ppp_device();
	if(!setup_ppp_device())
	{
		diag_printf("setup ppp device error\n");
		return 0;
	}
	return 1;
}

int pppoe_dail_trigger()
{
	//setup ppp0 ip ,route ,flags
	//when lan pc has packet to wan ,cause the pppoe dial up 
	
	extern	void kick_event(unsigned int event);
 	#define WAN_PPP_DIAL_EVENT	1<<17 
	kick_event(WAN_PPP_DIAL_EVENT);
}
#endif


void pppoe_start_on_intf(char *intname,int id,unsigned int flag, char *servicename, char *acname, 
	char *username, char *passwd,unsigned int  idle_timeout,unsigned int  connect_type, unsigned int mtu,void * save_connInfo)
{
	sessp sp;
	
	AAA
	/*Start PPPOE DISCOVERY*/
	sp=pppoe_selectSession(id);
	if(NULL == sp) {
		DEBUG_PRT("all session used\n");
		return -1;
	}
 	if(0== sp->inited)
		pppoe_initSession(sp,servicename,acname);

	if(save_connInfo)
 		sp->save_connInfo=save_connInfo;
	sp->idx = id;
	sp->ifp= ifunit(intname);
 	if(sp->ifp  == NULL)
		return -1;
 	if(username)
		strncpy(sp->username,username,32);
	if(passwd)
		strncpy(sp->passwd,passwd,32);
	
 	//if_getlladdr(sp->ifp,eh_prototype.ether_shost,ETH_ALEN);
	if(strcmp(sp->intfname,intname))
	{
		get_mac_address(intname,eh_prototype.ether_shost);
		strncpy(sp->intfname,intname,32);
	}	
	sp->idle_time_limit = idle_timeout*60;
	if(connect_type ==1)	//demand mode
		sp->demand_mode = 1;
	else
		sp->demand_mode = 0;

	sp->mtu = mtu;
 	//memcpy(eh_prototype.ether_shost,((struct eth_drv_sc *)(sp->ifp->if_softc)->sc_arpcom).ac_enaddr,ETH_ALEN);
	pppoe_enabled = 1;
	
	pppoe_start(sp);
 }

int pppoe_stop_on_intf(char *inttnmae, int id)
{	
	sessp sp;
	sp=pppoe_selectSession(id);
	stop_a_ppp(sp);
	return 1;
}

int pppoe_stop_link(int id)
{
	sessp sp;
	sp=pppoe_selectSession(id);	
	if(sp == NULL) return 0;
	if(sp->ppp_handle == NULL) {
		diag_printf("no ppp to close\n");
		return 0;
	}
	sp->state = PPPOE_DEAD;
	send_PADT(sp);
// 	cyg_thread_delay( 200 );
	return 1;
}

/*call by application
  *@intname is interface run on
  *@flag       for pppoe options
  */
int pppoe_init(unsigned int flag)
{
	sessp sp;
	AAA
	diag_printf("%s %d #######\n",__FUNCTION__,__LINE__);
	memset(gSession,0,sizeof(struct sess_con)*MAX_PPPOE_NUM);
	/*register NETISR*/
	register_netisr(NETISR_PPPOE, pppoeintr);
	diag_printf("%s %d #######\n",__FUNCTION__,__LINE__);
	return 1;
}

int pppoe_disabe(void)
{
	pppoe_enabled=0;
	return 1;
}

int is_pppoe_enabled(void)
{
	return pppoe_enabled;
}

#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)
int rtl_setAsicPppEntry(int id)
{
	int ret = -1;
	sessp sp;
	
#define	IF_PPPOE 2
	
	sp = pppoe_selectSession(id);

	if (sp){
		rtl865x_attachMasterNetif("ppp0", sp->intfname);
		rtl865x_addPpp("ppp0" , sp->pkt_hdr.eh.ether_dhost, sp->Session_ID, IF_PPPOE);	
		ret = 0;
	}

	return ret;
}

int rtl_delAsicPppEntry(int id)
{
	int ret = -1;
	sessp sp;
	
	sp = pppoe_selectSession(id);

	if (sp){
		rtl865x_detachMasterNetif("ppp0");
		rtl865x_delPpp(sp->Session_ID);
		ret = 0;
	}

	return ret;
}
#endif

