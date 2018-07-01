/*
* Copyright c                  Realtek Semiconductor Corporation, 2013  
* All rights reserved.
* 
* Program : spi(stateful packet inspection) firewall support
* Abstract : 
* Author : lynn_xu 
*/

/* System include files */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <sys/time.h>

#include <sys/param.h>
#undef  ip_id

/* BSD network include files */
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <pkgconf/system.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <pkgconf/system.h>
#include <pkgconf/net_freebsd_stack.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>


#include <rtl/rtl_alias.h>
#include <rtl/rtl_queue.h>

//#define SPI_FIREWALL_DEBUG

#define SPI_CHECK_POSITION_IP_LAYER     1
#define SPI_CHECK_POSITION_FAST_PATH    2
#define SPI_CHECK_POSITION_FAST_SKB     3

#define SPI_DIR_ORIGINAL                0
#define SPI_DIR_REPLY                   1

/* But this is what stacks really send out. */
#define TCPOLEN_TSTAMP_ALIGNED		12
#define TCPOLEN_WSCALE_ALIGNED		4
#define TCPOLEN_SACKPERM_ALIGNED	4
#define TCPOLEN_SACK_BASE		2
#define TCPOLEN_SACK_BASE_ALIGNED	4
#define TCPOLEN_SACK_PERBLOCK		8
#define TCPOLEN_MD5SIG_ALIGNED		20
#define TCPOLEN_MSS_ALIGNED		4

#define	SPI_FIREWALL_TABLE_LIST_MAX	    1024
#define	SPI_FIREWALL_TABLE_ENTRY_MAX	1024
#define SPI_FIREWALL_IDLE               0
#define SPI_FIREWALL_INUSE              1
#define SPI_FIREWALL_STATE_UNCONNECTED_TIMEOUT (2*60*HZ)
#define SPI_FIREWALL_CONNECTED_TIME_OUT (60*60*HZ)
#define SPI_FIREWALL_STATE_CLOSE_TIMEOUT (60*HZ)
#define SPI_FIREWALL_TIMEOUT_CHECK		(hz)

#define SPI_FIREWALL_STATE_UNCONNECTED 1
#define SPI_FIREWALL_STATE_CONNECTED   2
#define SPI_FIREWALL_STATE_CLOSE       3

/* This is exposed to userspace (ctnetlink) */
enum tcp_conntrack {
	TCP_CONNTRACK_NONE,
	TCP_CONNTRACK_SYN_SENT,
	TCP_CONNTRACK_SYN_RECV,
	TCP_CONNTRACK_ESTABLISHED,
	TCP_CONNTRACK_FIN_WAIT,
	TCP_CONNTRACK_CLOSE_WAIT,
	TCP_CONNTRACK_LAST_ACK,
	TCP_CONNTRACK_TIME_WAIT,
	TCP_CONNTRACK_CLOSE,
	TCP_CONNTRACK_LISTEN,
	TCP_CONNTRACK_MAX,
	TCP_CONNTRACK_IGNORE
};

/* What TCP flags are set from RST/SYN/FIN/ACK. */
enum tcp_bit_set {
	TCP_SYN_SET,
	TCP_SYNACK_SET,
	TCP_FIN_SET,
	TCP_ACK_SET,
	TCP_RST_SET,
	TCP_NONE_SET,
};

/* Window scaling is advertised by the sender */
#define IP_CT_TCP_FLAG_WINDOW_SCALE		0x01

/* SACK is permitted by the sender */
#define IP_CT_TCP_FLAG_SACK_PERM		0x02

/* This sender sent FIN first */
#define IP_CT_TCP_FLAG_CLOSE_INIT		0x04

/* Be liberal in window checking */
#define IP_CT_TCP_FLAG_BE_LIBERAL		0x08

/* Has unacknowledged data */
#define IP_CT_TCP_FLAG_DATA_UNACKNOWLEDGED	0x10

/* The field td_maxack has been set */
#define IP_CT_TCP_FLAG_MAXACK_SET		0x20


typedef unsigned long long	uint64;
typedef long long		    int64;
typedef unsigned int	    uint32;
typedef int			        int32;
typedef uint32              ipaddr_t;
typedef unsigned short	    uint16;
typedef short			    int16;
typedef unsigned char	    uint8;
typedef char			    int8;


static unsigned int spi_firewall_enable = 0;

struct ip_ct_tcp_state {
	u_int32_t	td_end;		/* max of seq + len */
	u_int32_t	td_maxend;	/* max of ack + max(win, 1) */
	u_int32_t	td_maxwin;	/* max(win) */
	u_int32_t	td_maxack;	/* max of ack */
	u_int8_t	td_scale;	/* window scale factor */
	u_int8_t	flags;		/* per direction options */
};

struct ip_ct_tcp
{
	struct ip_ct_tcp_state seen[2];	/* connection parameters per direction */
	u_int8_t	state;		/* state of the connection (enum tcp_conntrack) */
	/* For detecting stale connections */
	u_int8_t	last_dir;	/* Direction of the last packet (enum ip_conntrack_dir) */
	u_int8_t	retrans;	/* Number of retransmitted packets */
	u_int8_t	last_index;	/* Index of the last packet */
	u_int32_t	last_seq;	/* Last sequence number seen in dir */
	u_int32_t	last_ack;	/* Last sequence number seen in opposite dir */
	u_int32_t	last_end;	/* Last seq + len */
	u_int16_t	last_win;	/* Last window advertisement seen in dir */
};

/* --- spi firewall Table Structures --- */
struct spi_firewall_List_entry
{
	ipaddr_t inip; 
	//ipaddr_t outip;
	ipaddr_t rmtip;    
	uint16 inport; 
	uint16 rmtport;   
	unsigned long last_used;    
	struct ip_ct_tcp tcp;
    //uint16 outport; 
    //uint8	protocol;
    uint8 status;
    uint8 flags;
	CTAILQ_ENTRY(spi_firewall_List_entry) ct_link;
	CTAILQ_ENTRY(spi_firewall_List_entry) tqe_link;
};

struct spi_firewall_table
{
	CTAILQ_HEAD(spi_firewall_list_entry_head, spi_firewall_List_entry) *list;
};

CTAILQ_HEAD(spi_firewall_list_inuse_head, spi_firewall_List_entry) spi_firewall_list_inuse;
CTAILQ_HEAD(spi_firewall_list_free_head, spi_firewall_List_entry) spi_firewall_list_free;

static struct spi_firewall_table *spifirewall_table=NULL;
struct callout spi_firewall_timer;

static inline uint32 spi_firewall_hashfn(ipaddr_t saddr, uint16 sport, ipaddr_t daddr, uint16 dport)
{
	uint32 h = saddr ^ daddr;

	h ^= (h>>16) ^ sport ^ dport;
	return h & (SPI_FIREWALL_TABLE_LIST_MAX - 1);

}

static inline void free_spi_firewall_entry(struct spi_firewall_List_entry *entry)
{
	uint32 hash = spi_firewall_hashfn(entry->inip, entry->inport, entry->rmtip, entry->rmtport);	
	
	entry->status= SPI_FIREWALL_IDLE;
	CTAILQ_REMOVE(&spifirewall_table->list[hash], entry, ct_link);  
    CTAILQ_REMOVE(&spi_firewall_list_inuse, entry, tqe_link);
	CTAILQ_INSERT_TAIL(&spi_firewall_list_free, entry, tqe_link);		
}
inline unsigned long rtl_get_spi_firewall_expiretime(uint8 flags)
{
	unsigned long expires = 0;

    if (flags == SPI_FIREWALL_STATE_UNCONNECTED)
        expires = SPI_FIREWALL_STATE_UNCONNECTED_TIMEOUT;
    else if (flags == SPI_FIREWALL_STATE_CONNECTED)
        expires = SPI_FIREWALL_CONNECTED_TIME_OUT;
    else if (flags == SPI_FIREWALL_STATE_CLOSE)
        expires = SPI_FIREWALL_STATE_CLOSE_TIMEOUT;
    else
        expires = SPI_FIREWALL_STATE_CLOSE_TIMEOUT;        
    
	return expires;
}	
static inline void spi_firewall_cache_timeout(void)
{
    unsigned long now = (unsigned long)jiffies;
    struct spi_firewall_List_entry *spi_ep;

    CTAILQ_FOREACH(spi_ep, &spi_firewall_list_inuse, tqe_link)
    {
        if (((spi_ep->last_used+rtl_get_spi_firewall_expiretime(spi_ep->flags)) < now)&&(spi_ep->status == SPI_FIREWALL_INUSE))
        {
            free_spi_firewall_entry(spi_ep); 
        }
    }

    callout_reset(&spi_firewall_timer, SPI_FIREWALL_TIMEOUT_CHECK, spi_firewall_cache_timeout, 0);
}

static inline int32 add_spi_firewall_entry(struct spi_firewall_List_entry *spi_ep)
{
	uint32 hash = spi_firewall_hashfn(spi_ep->inip, spi_ep->inport, spi_ep->rmtip, spi_ep->rmtport);
	struct spi_firewall_List_entry *entry;
		
	if(CTAILQ_EMPTY(&spi_firewall_list_free))
	{
		return FAILED;
	}
				
	entry = CTAILQ_FIRST(&spi_firewall_list_free);
    #if 1
	entry->inip = spi_ep->inip;
    entry->inport = spi_ep->inport;
	//entry->outip = spi_ep->outip;
	//entry->outport = spi_ep->outport;
    entry->rmtip = spi_ep->rmtip;
	entry->rmtport = spi_ep->rmtport;
    //entry->protocol = spi_ep->protocol;    
	entry->status = SPI_FIREWALL_INUSE;
    entry->flags = spi_ep->flags; 
    
    memcpy(&entry->tcp, &spi_ep->tcp, sizeof(struct ip_ct_tcp));    
    #endif

	CTAILQ_REMOVE(&spi_firewall_list_free, entry, tqe_link);    
    CTAILQ_INSERT_TAIL(&spi_firewall_list_inuse, entry, tqe_link);   
	CTAILQ_INSERT_TAIL(&spifirewall_table->list[hash], entry, ct_link);

    entry->last_used = jiffies;

	return SUCCESS;
}

static inline struct spi_firewall_List_entry *find_spi_firewall_entry(struct spi_firewall_List_entry *iterm)
{
	uint32 hash = 0;
	struct spi_firewall_List_entry *entry = NULL;

    if (iterm == NULL)
        return NULL;

    {
        //hash = spi_firewall_hashfn(iterm->inip, iterm->inport, iterm->rmtip, iterm->rmtport);
        CTAILQ_FOREACH(entry, &spi_firewall_list_inuse, tqe_link)
    	{
    		if (((entry->inip == iterm->inip) &&
    			(entry->inport == iterm->inport) &&
    			(entry->rmtip == iterm->rmtip) &&
    			(entry->rmtport== iterm->rmtport) &&
    			(entry->status == SPI_FIREWALL_INUSE))||
    			((entry->inip == iterm->rmtip) &&
    			(entry->inport == iterm->rmtport) &&
    			(entry->rmtip == iterm->inip) &&
    			(entry->rmtport== iterm->inport) &&
    			(entry->status == SPI_FIREWALL_INUSE)))
    		{
    			return (entry);
    		}
    	}
    }    
	
	return NULL;
}

/*
 * The next routines deal with comparing 32 bit unsigned ints
 * and worry about wraparound (automatic with unsigned arithmetic).
 */

static inline int before(__u32 seq1, __u32 seq2)
{
        return (int32)(seq1-seq2) < 0;
}
#define after(seq2, seq1) 	before(seq1, seq2)

static inline uint32 segment_seq_plus_len(uint32 seq,
					 uint16 totallen,
					 uint32 headerlen,
					 uint8 flags)
{
	return (seq + totallen - headerlen + (( (flags & TH_SYN) == TH_SYN) ? 1 : 0) + (( (flags & TH_FIN) == TH_FIN) ? 1 : 0));
}

/*
 * Simplified tcp_parse_options routine for spi firewall
 */
static inline void tcp_options(const struct tcphdr *tcph,
			struct ip_ct_tcp_state *state)
{
	unsigned char *ptr = NULL;
	int length = 0;

    if (tcph == NULL || state == NULL)
        return;
    
    length = (tcph->th_off << 2) - sizeof(struct tcphdr);
	ptr = (unsigned char *)(tcph + 1);

	if (!length || ptr == NULL)
		return;

	state->td_scale =
	state->flags = 0;

	while (length > 0) {
		int opcode=*ptr++;
		int opsize;

		switch (opcode) {
		case TCPOPT_EOL:
			return;
		case TCPOPT_NOP:	/* Ref: RFC 793 section 3.1 */
			length--;
			continue;
		default:
			opsize=*ptr++;
			if (opsize < 2) /* "silly options" */
				return;
			if (opsize > length)
				break;	/* don't parse partial options */

			if (opcode == TCPOPT_SACK_PERMITTED
			    && opsize == TCPOLEN_SACK_PERMITTED)
				state->flags |= IP_CT_TCP_FLAG_SACK_PERM;
			else if (opcode == TCPOPT_WINDOW
				 && opsize == TCPOLEN_WINDOW) {
				state->td_scale = *(u_int8_t *)ptr;

				if (state->td_scale > 14) {
					/* See RFC1323 */
					state->td_scale = 14;
				}
				state->flags |=
					IP_CT_TCP_FLAG_WINDOW_SCALE;
			}
			ptr += opsize - 2;
			length -= opsize;
		}
	}
}

static inline void tcp_sack(const struct tcphdr *tcph, __u32 *sack)
{
	const unsigned char *ptr = NULL;
	int length = 0;
	__u32 tmp;

    if (tcph == NULL || sack == NULL)
        return;
    
    length = (tcph->th_off << 2) - sizeof(struct tcphdr);
    ptr = (unsigned char *)(tcph + 1);
	if (!length || !ptr)
		return;

	/* Fast path for timestamp-only option */
	if (length == TCPOLEN_TSTAMP_ALIGNED*4
	    && *(uint32 *)ptr == htonl((TCPOPT_NOP << 24)
				       | (TCPOPT_NOP << 16)
				       | (TCPOPT_TIMESTAMP << 8)
				       | TCPOLEN_TIMESTAMP))
		return;

	while (length > 0) {
		int opcode = *ptr++;
		int opsize, i;

		switch (opcode) {
		case TCPOPT_EOL:
			return;
		case TCPOPT_NOP:	/* Ref: RFC 793 section 3.1 */
			length--;
			continue;
		default:
			opsize = *ptr++;
			if (opsize < 2) /* "silly options" */
				return;
			if (opsize > length)
				break;	/* don't parse partial options */

			if (opcode == TCPOPT_SACK
			    && opsize >= (TCPOLEN_SACK_BASE
					  + TCPOLEN_SACK_PERBLOCK)
			    && !((opsize - TCPOLEN_SACK_BASE)
				 % TCPOLEN_SACK_PERBLOCK)) {
				for (i = 0;
				     i < (opsize - TCPOLEN_SACK_BASE);
				     i += TCPOLEN_SACK_PERBLOCK) {
					tmp = *((uint32 *)(ptr+i)+1);

					if (after(tmp, *sack))
						*sack = tmp;
				}
				return;
			}
			ptr += opsize - 2;
			length -= opsize;
		}
	}
}

/* Fixme: what about big packets? */
#define MAXACKWINCONST			66000
#define MAXACKWINDOW(sender)						\
	((sender)->td_maxwin > MAXACKWINCONST ? (sender)->td_maxwin	\
					      : MAXACKWINCONST)


static inline unsigned int get_conntrack_index(uint8 flags)
{
	if ((flags&TH_RST)==TH_RST) return TCP_RST_SET;
	else if ((flags&TH_SYN)==TH_SYN) return (((flags&TH_ACK)==TH_ACK) ? TCP_SYNACK_SET : TCP_SYN_SET);
	else if ((flags&TH_FIN)==TH_FIN) return TCP_FIN_SET;
	else if ((flags&TH_ACK)==TH_ACK) return TCP_ACK_SET;
	else return TCP_NONE_SET;
}

inline int32 rtl_add_spi_firewall_new_connect(struct ip *iph)
{
    struct spi_firewall_List_entry entry, *iterm;
    uint32 header_len = 0;
    uint8 flags = 0;   
    
    /* disable spi firewall just return */
    if (!spi_firewall_enable)
        return SUCCESS;
    
    /* sanity check */
    if (iph == NULL)
        return FAILED;
    
    memset(&entry, 0x00, sizeof(struct spi_firewall_List_entry));
    if (iph->ip_p == IPPROTO_TCP)
    {
        struct tcphdr *tcph = (struct tcphdr*)((__u32 *)iph + iph->ip_hl);

        if (tcph == NULL)
            return FAILED;
        
        flags = tcph->th_flags;
        header_len = (iph->ip_hl + tcph->th_off) << 2;
        if (((flags & TH_SYN) == TH_SYN) && (!((flags & TH_ACK) == TH_ACK)))
        {
            /* new connection add related data to cache */
            entry.flags = SPI_FIREWALL_STATE_UNCONNECTED;
            entry.inip = ntohl(iph->ip_src.s_addr);
            entry.inport = ntohs(tcph->th_sport);
            entry.rmtip = ntohl(iph->ip_dst.s_addr);
            entry.rmtport = ntohs(tcph->th_dport);
            
            iterm = find_spi_firewall_entry(&entry);
            if (iterm)
            /* if already in cache just return----retransmission syn packets */
        	{
        	    iterm->last_used = jiffies;
        		return SUCCESS;
        	}
            entry.tcp.seen[0].td_end =
			segment_seq_plus_len(ntohl(tcph->th_seq), ntohs(iph->ip_len),header_len, flags);
		    entry.tcp.seen[0].td_maxwin = ntohs(tcph->th_win);
		    if (entry.tcp.seen[0].td_maxwin == 0)
			    entry.tcp.seen[0].td_maxwin = 1;
		    entry.tcp.seen[0].td_maxend =
			entry.tcp.seen[0].td_end;

		    tcp_options(tcph, &entry.tcp.seen[0]);
		    entry.tcp.seen[1].flags = 0;

            entry.tcp.seen[1].td_end = 0;
	        entry.tcp.seen[1].td_maxend = 0;
	        entry.tcp.seen[1].td_maxwin = 1;
	        entry.tcp.seen[1].td_scale = 0;

        	/* tcp_packet will set them */
        	entry.tcp.state = TCP_CONNTRACK_NONE;
        	entry.tcp.last_index = TCP_NONE_SET;
            #ifdef SPI_FIREWALL_DEBUG
            diag_printf("%s %d: src=0x%x:%d dst=0x%x:%d sender end=%u maxend=%u maxwin=%u scale=%u \
             receiver end=%u maxend=%u maxwin=%u scale=%u\n",__FUNCTION__, 
             __LINE__,entry.inip, entry.inport, entry.rmtip, entry.rmtport,
             entry.tcp.seen[0].td_end, entry.tcp.seen[0].td_maxend, entry.tcp.seen[0].td_maxwin,
             entry.tcp.seen[0].td_scale,
             entry.tcp.seen[1].td_end, entry.tcp.seen[1].td_maxend, entry.tcp.seen[1].td_maxwin,
             entry.tcp.seen[1].td_scale);
            #endif
            if (add_spi_firewall_entry(&entry) == FAILED)
            {            
                return FAILED;
            }
        }
    }    
    
    return SUCCESS;
}

inline struct spi_firewall_List_entry * rtl_spi_firewall_find_entryinfo(struct ip *iph, struct tcphdr *tcph, uint32 *dir)
{
    struct spi_firewall_List_entry entry, *iterm = NULL;

    //entry.status = SPI_FIREWALL_STATE_UNCONNECTED;
    entry.inip = ntohl(iph->ip_src.s_addr);
    entry.inport = ntohs(tcph->th_sport);
    entry.rmtip = ntohl(iph->ip_dst.s_addr);
    entry.rmtport = ntohs(tcph->th_dport);

    iterm = find_spi_firewall_entry(&entry);
    if (iterm)
    {
        if (iterm->inip == entry.inip)
            *dir = SPI_DIR_ORIGINAL;
        else
            *dir = SPI_DIR_REPLY;
        iterm->last_used = jiffies;        
    }
    
    return iterm;
}

inline int32 tcp_in_window(const uint8 *data)
{
	struct ip_ct_tcp_state *sender = NULL;
	struct ip_ct_tcp_state *receiver = NULL;
	uint32 seq, ack, sack, end, win, swin, dir = 0, header_len = 0;
	int32 res;
    struct ip *iph = NULL;
    struct tcphdr *tcph =  NULL;
    struct spi_firewall_List_entry *entry = NULL;
    uint32 index = 0;
    uint16 type = 0;
    uint8 flags = 0;

    /* disable spi firewall just return */
    if (!spi_firewall_enable)
        return true;
    
    /* sanity check */
    if (data == NULL)
        return false;
    
    iph = (struct ip *)(data);
    tcph = (struct tcphdr*)((__u32 *)iph + iph->ip_hl);
    if (tcph == NULL)
        return false; 
    
    /* ONLY process tcp protocol */   
    if (iph->ip_p != IPPROTO_TCP)
        return true;           
    flags = tcph->th_flags;
    header_len = (iph->ip_hl + tcph->th_off) << 2;   
    index = get_conntrack_index(flags);

   /* if entry not existed just return */
   entry = rtl_spi_firewall_find_entryinfo(iph, tcph, &dir);
   if(!entry)
        return true;
   /* update entry flags */
   if (index == TCP_SYNACK_SET || index == TCP_SYN_SET)
        entry->flags = SPI_FIREWALL_STATE_UNCONNECTED;
   else if (index == TCP_RST_SET || index == TCP_FIN_SET)
        entry->flags = SPI_FIREWALL_STATE_CLOSE;
   else if (entry->flags != SPI_FIREWALL_STATE_CLOSE)
        entry->flags = SPI_FIREWALL_STATE_CONNECTED;
	/*
	 * Get the required data from the packet.
	 */
	seq = ntohl(tcph->th_seq);
	ack = sack = ntohl(tcph->th_ack);
	win = ntohs(tcph->th_win);
	end = segment_seq_plus_len(seq, ntohs(iph->ip_len), header_len, flags);

    sender = &entry->tcp.seen[dir];
	receiver = &entry->tcp.seen[!dir];
    
	if (receiver->flags & IP_CT_TCP_FLAG_SACK_PERM)
        tcp_sack(tcph, &sack);

	if (sender->td_end == 0) {
		/*
		 * Initialize sender data.
		 */
		if (((flags&TH_SYN)==TH_SYN) && ((flags&TH_ACK)==TH_ACK)) {
			/*
			 * Outgoing SYN-ACK in reply to a SYN.
			 */
			sender->td_end =
			sender->td_maxend = end;
			sender->td_maxwin = (win == 0 ? 1 : win);

			tcp_options(tcph, sender);
			/*
			 * RFC 1323:
			 * Both sides must send the Window Scale option
			 * to enable window scaling in either direction.
			 */
			if (!(sender->flags & IP_CT_TCP_FLAG_WINDOW_SCALE
			      && receiver->flags & IP_CT_TCP_FLAG_WINDOW_SCALE))
				sender->td_scale =
				receiver->td_scale = 0;
		} else {
			/*
			 * We are in the middle of a connection,
			 * its history is lost for us.
			 * Let's try to use the data from the packet.
			 */
			sender->td_end = end;
			sender->td_maxwin = (win == 0 ? 1 : win);
			sender->td_maxend = end + sender->td_maxwin;
		}
	} else if (((entry->tcp.state == TCP_CONNTRACK_SYN_SENT
		     && dir == SPI_DIR_ORIGINAL)
		   || (entry->tcp.state == TCP_CONNTRACK_SYN_RECV
		     && dir == SPI_DIR_REPLY))
		   && after(end, sender->td_end)) {
		/*
		 * RFC 793: "if a TCP is reinitialized ... then it need
		 * not wait at all; it must only be sure to use sequence
		 * numbers larger than those recently used."
		 */
		sender->td_end =
		sender->td_maxend = end;
		sender->td_maxwin = (win == 0 ? 1 : win);

		tcp_options(tcph, sender);
	}

	if (!((flags&TH_ACK)==TH_ACK)) {
		/*
		 * If there is no ACK, just pretend it was set and OK.
		 */
		ack = sack = receiver->td_end;
	} else if (((flags & (TH_ACK|TH_RST)) ==(TH_ACK|TH_RST))
		   && (ack == 0)) {
		/*
		 * Broken TCP stacks, that set ACK in RST packets as well
		 * with zero ack value.
		 */
		ack = sack = receiver->td_end;
	}

	if (seq == end
	    && (!((flags&TH_RST)==TH_RST)
		|| (seq == 0 && entry->tcp.state == TCP_CONNTRACK_SYN_SENT)))
		/*
		 * Packets contains no data: we assume it is valid
		 * and check the ack value only.
		 * However RST segments are always validated by their
		 * SEQ number, except when seq == 0 (reset sent answering
		 * SYN.
		 */
		seq = end = sender->td_end;
        #ifdef SPI_FIREWALL_DEBUG 
        diag_printf("%s %d: src=0x%x:%d dst0x%x:%d seq=%u ack=%u sack =%u win=%u end=%u\n",
        __FUNCTION__, __LINE__, ntohl(iph->ip_src), ntohs(tcph->th_sport), 
        ntohl(iph->ip_dst), ntohs(tcph->th_dport) ,seq, ack, sack, win, end);
        diag_printf("%s %d: sender end=%u maxend=%u maxwin=%u scale=%u \
        receiver end=%u maxend=%u maxwin=%u scale=%u\n",__FUNCTION__, __LINE__,
        sender->td_end, sender->td_maxend, sender->td_maxwin,
        sender->td_scale,
        receiver->td_end, receiver->td_maxend, receiver->td_maxwin,
        receiver->td_scale);

        diag_printf("%s %d: I=%d II=%d III=%d IV=%d\n",__FUNCTION__, __LINE__,
        before(seq, sender->td_maxend + 1),
        after(end, sender->td_end - receiver->td_maxwin - 1),
        before(sack, receiver->td_end + 1),
        after(sack, receiver->td_end - MAXACKWINDOW(sender) - 1));
        #endif

	if (before(seq, sender->td_maxend + 1) &&
	    after(end, sender->td_end - receiver->td_maxwin - 1) &&
	    before(sack, receiver->td_end + 1) &&
	    after(sack, receiver->td_end - MAXACKWINDOW(sender) - 1)) {
		/*
		 * Take into account window scaling (RFC 1323).
		 */
		if (!((flags&TH_SYN)==TH_SYN))
			win <<= sender->td_scale;

		/*
		 * Update sender data.
		 */
		swin = win + (sack - ack);
		if (sender->td_maxwin < swin)
			sender->td_maxwin = swin;
		if (after(end, sender->td_end)) {
			sender->td_end = end;
			sender->flags |= IP_CT_TCP_FLAG_DATA_UNACKNOWLEDGED;
		}
		if (((flags&TH_ACK)==TH_ACK)) {
			if (!(sender->flags & IP_CT_TCP_FLAG_MAXACK_SET)) {
				sender->td_maxack = ack;
				sender->flags |= IP_CT_TCP_FLAG_MAXACK_SET;
			} else if (after(ack, sender->td_maxack))
             {                
				sender->td_maxack = ack;
             }
		}

		/*
		 * Update receiver data.
		 */
		if (after(end, sender->td_maxend))
			receiver->td_maxwin += end - sender->td_maxend;
		if (after(sack + win, receiver->td_maxend - 1)) {
			receiver->td_maxend = sack + win;
			if (win == 0)
				receiver->td_maxend++;
		}
		if (ack == receiver->td_end)
			receiver->flags &= ~IP_CT_TCP_FLAG_DATA_UNACKNOWLEDGED;

		/*
		 * Check retransmissions.
		 */
		if (index == TCP_ACK_SET) {
			if (entry->tcp.last_dir == dir
			    && entry->tcp.last_seq == seq
			    && entry->tcp.last_ack == ack
			    && entry->tcp.last_end == end
			    && entry->tcp.last_win == win)
				entry->tcp.retrans++;
			else {
				entry->tcp.last_dir = dir;
				entry->tcp.last_seq = seq;
				entry->tcp.last_ack = ack;
				entry->tcp.last_end = end;
				entry->tcp.last_win = win;
				entry->tcp.retrans = 0;
			}
		}
		res = true;
	} else {
		res = false;
	}
    #ifdef SPI_FIREWALL_DEBUG
	diag_printf("%s %d: res=%u sender end=%u maxend=%u maxwin=%u "
		 "receiver end=%u maxend=%u maxwin=%u\n",__FUNCTION__, __LINE__,
		 res, sender->td_end, sender->td_maxend, sender->td_maxwin,
		 receiver->td_end, receiver->td_maxend, receiver->td_maxwin);
    #endif

	return res;
}

int init_spi_firewall_table(int spi_firewall_tbl_list_max, int spi_firewall_tbl_entry_max)
{
	int i;

	spifirewall_table = (struct spi_firewall_table *)kmalloc(sizeof(struct spi_firewall_table), GFP_ATOMIC);
	if (spifirewall_table == NULL) {
		diag_printf("MALLOC Failed! (spi firewall Table) \n");
		return FAILED;
	}
	CTAILQ_INIT(&spi_firewall_list_inuse);
	CTAILQ_INIT(&spi_firewall_list_free);

	spifirewall_table->list=(struct spi_firewall_list_entry_head *)kmalloc(spi_firewall_tbl_list_max*sizeof(struct spi_firewall_list_entry_head), GFP_ATOMIC);
	if (spifirewall_table->list == NULL) {
		diag_printf("MALLOC Failed! (spi firewall Table List) \n");
		return FAILED;
	}

	for (i=0; i<spi_firewall_tbl_list_max; i++) {
		CTAILQ_INIT(&spifirewall_table->list[i]);
	}

	for (i=0; i<spi_firewall_tbl_entry_max; i++) {
		struct spi_firewall_List_entry *entry = (struct spi_firewall_List_entry *)kmalloc(sizeof(struct spi_firewall_List_entry), GFP_ATOMIC);
		if (entry == NULL) {
			diag_printf("MALLOC Failed! (spi firewall Table Entry) \n");
			return FAILED;
		}
		CTAILQ_INSERT_TAIL(&spi_firewall_list_free, entry, tqe_link);
	}

    callout_init(&spi_firewall_timer);
	callout_reset(&spi_firewall_timer, SPI_FIREWALL_TIMEOUT_CHECK , spi_firewall_cache_timeout, 0);

	return SUCCESS;
}

int  init_spi_firewall(void)
{
    uint32 ret = 0;
    
	ret=init_spi_firewall_table(SPI_FIREWALL_TABLE_LIST_MAX,SPI_FIREWALL_TABLE_ENTRY_MAX);
	if(ret!=0) {
		diag_printf("init_spi_firewall Failed!\n");
	}
    
    return SUCCESS;

}

void  spi_firewall_exit(void)
{    
    return;
}

void spi_firewall_dump_entry_num(void)
{

    int num;
    struct spi_firewall_List_entry *ep;
    
	num = 0;
	CTAILQ_FOREACH(ep, &spi_firewall_list_inuse, tqe_link) {
		num++;
	}

	diag_printf("spi firewall entry num: %d\n", num);

    return;
}

void rtl_show_spifirewall_entry(void)
{
    
    struct spi_firewall_List_entry *ep;

    diag_printf("spi firewall %s\n", spi_firewall_enable!=0?"Enabled":"Disabled");
	spi_firewall_dump_entry_num();
	diag_printf("entrys:\n");

	CTAILQ_FOREACH(ep, &spi_firewall_list_inuse, tqe_link) {
            diag_printf("~conn: [%s] in=0x%08X:%-5u rmt=0x%08X:%-5u flags=0x%08X \n\
            original:end=%10u maxend=%10u maxwin=%10u maxack=%10u scale=%05u flags=%05u\n\
            reply   :end=%10u maxend=%10u maxwin=%10u maxack=%10u scale=%05u flags=%05u\n",
            "TCP",ep->inip, ep->inport, ep->rmtip, 
            ep->rmtport,ep->flags, ep->tcp.seen[0].td_end, 
            ep->tcp.seen[0].td_maxend, ep->tcp.seen[0].td_maxwin, 
            ep->tcp.seen[0].td_maxack, ep->tcp.seen[0].td_scale, 
            ep->tcp.seen[0].flags, ep->tcp.seen[1].td_end, 
            ep->tcp.seen[1].td_maxend, ep->tcp.seen[1].td_maxwin, 
            ep->tcp.seen[1].td_maxack, ep->tcp.seen[1].td_scale, 
            ep->tcp.seen[1].flags);
	}
    
    return;

}
inline void rtl_spifirewall_onoff(unsigned int value)
{
	spi_firewall_enable = value;

    return;
}

