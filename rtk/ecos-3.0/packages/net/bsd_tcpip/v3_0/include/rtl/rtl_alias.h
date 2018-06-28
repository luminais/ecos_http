#ifndef __RTL_ALIAS_H
#define __RTL_ALIAS_H

#define JIFFIES_CHECK_TIMEOUT
/* Timeouts (in seconds) for different link types */
#if defined(JIFFIES_CHECK_TIMEOUT)
#define ICMP_EXPIRE_TIME             (60 * cyg_hz)
#if defined(CONFIG_RTL_NAPT_GARBAGE_COLLECTION)
#define UDP_EXPIRE_TIME              (60 * cyg_hz)
#else
#define UDP_EXPIRE_TIME              (15 * cyg_hz)
#endif
#define PROTO_EXPIRE_TIME            (60 * cyg_hz)
#define FRAGMENT_ID_EXPIRE_TIME      (10 * cyg_hz)
#define FRAGMENT_PTR_EXPIRE_TIME     (30 * cyg_hz)
#else
#define ICMP_EXPIRE_TIME             60
#if defined(CONFIG_RTL_NAPT_GARBAGE_COLLECTION)
#define UDP_EXPIRE_TIME              60
#else
#define UDP_EXPIRE_TIME              15
#endif
#define PROTO_EXPIRE_TIME            60
#define FRAGMENT_ID_EXPIRE_TIME      10
#define FRAGMENT_PTR_EXPIRE_TIME     30
#endif

/* TCP link expire time for different cases */
/* When the link has been used and closed - minimal grace time to
   allow ACKs and potential re-connect in FTP (XXX - is this allowed?)  */
#ifndef TCP_EXPIRE_DEAD
#if defined(CONFIG_RTL_NAPT_GARBAGE_COLLECTION)
#define TCP_EXPIRE_DEAD           10
#else
#if defined(JIFFIES_CHECK_TIMEOUT)
#define TCP_EXPIRE_DEAD           (0 * cyg_hz)
#else
#define TCP_EXPIRE_DEAD           1
#endif
#endif
#endif

/* When the link has been used and closed on one side - the other side
   is allowed to still send data */
#ifndef TCP_EXPIRE_SINGLEDEAD
//#define TCP_EXPIRE_SINGLEDEAD     90
#if defined(JIFFIES_CHECK_TIMEOUT)
#define TCP_EXPIRE_SINGLEDEAD     (0 * cyg_hz)
#else
#define TCP_EXPIRE_SINGLEDEAD     5
#endif

#endif
/* When the link isn't yet up */
#ifndef TCP_EXPIRE_INITIAL
//#define TCP_EXPIRE_INITIAL       300
#if defined(JIFFIES_CHECK_TIMEOUT)
#define TCP_EXPIRE_INITIAL       (180 * cyg_hz)
#else
#define TCP_EXPIRE_INITIAL       5
#endif

#endif
/* When the link is up */
#ifndef TCP_EXPIRE_CONNECTED
//#define TCP_EXPIRE_CONNECTED   900
/*keep same as linux*/
#if defined(JIFFIES_CHECK_TIMEOUT)
#define TCP_EXPIRE_CONNECTED  (180 * cyg_hz)
#else
#define TCP_EXPIRE_CONNECTED  600
#endif
#endif


struct ack_data_record     /* used to save changes to ACK/sequence numbers */
{
    u_long ack_old;
    u_long ack_new;
    int delta;
    int active;
};

struct tcp_state           /* Information about TCP connection        */
{
    int in;                /* State for outside -> inside             */
    int out;               /* State for inside  -> outside            */
    int index;             /* Index to ACK data array                 */
    int ack_modified;      /* Indicates whether ACK and sequence numbers */
                           /* been modified                           */
};

#define N_LINK_TCP_DATA   3 /* Number of distinct ACK number changes
                               saved for a modified TCP stream */
struct tcp_dat
{
    struct tcp_state state;
    struct ack_data_record ack[N_LINK_TCP_DATA];
    int fwhole;             /* Which firewall record is used for this hole? */
};

struct udp_dat
{
    int fwhole;             /* Which firewall record is used for this hole? */
};

struct server              /* LSNAT server pool (circular list) */
{
    struct in_addr addr;
    u_short port;
    struct server *next;
};

#if defined(HAVE_NAT_ALG) && defined(HAVE_NAT_ALG_H323)
struct h323_info {
	union {
		/* RAS connection timeout */
		u_int32_t timeout;

		/* Next TPKT length (for separate TPKT header and data) */
		u_int16_t tpkt_len[2];
	};
};
#endif

#define CONFIG_RTL_BYPASS_PKT
#define RTL_BYPASS_PKT_NUM 15


struct alias_link                /* Main data structure */
{
    struct in_addr src_addr;     /* Address and port information        */
    struct in_addr dst_addr;
    struct in_addr alias_addr;
    struct in_addr proxy_addr;
    u_short src_port;
    u_short dst_port;
    u_short alias_port;
    u_short proxy_port;
    struct server *server;

    int link_type;               /* Type of link: TCP, UDP, ICMP, proto, frag */

/* values for link_type */
#define LINK_ICMP                     IPPROTO_ICMP
#define LINK_UDP                      IPPROTO_UDP
#define LINK_TCP                      IPPROTO_TCP
#define LINK_FRAGMENT_ID              (IPPROTO_MAX + 1)
#define LINK_FRAGMENT_PTR             (IPPROTO_MAX + 2)
#define LINK_ADDR                     (IPPROTO_MAX + 3)
#define LINK_PPTP                     (IPPROTO_MAX + 4)

    int flags;                   /* indicates special characteristics   */

/* flag bits */
#define LINK_UNKNOWN_DEST_PORT     0x01
#define LINK_UNKNOWN_DEST_ADDR     0x02
#define LINK_PERMANENT             0x04
#define LINK_PARTIALLY_SPECIFIED   0x03 /* logical-or of first two bits */
#define LINK_UNFIREWALLED          0x08
#define LINK_LAST_LINE_CRLF_TERMED 0x10

    int timestamp;               /* Time link was last accessed         */
    int expire_time;             /* Expire time for link                */

    int sockfd;                  /* socket descriptor                   */

    LIST_ENTRY(alias_link) list_out; /* Linked list of pointers for     */
    LIST_ENTRY(alias_link) list_in;  /* input and output lookup tables  */
#if defined(CONFIG_RTL_NAPT_GARBAGE_COLLECTION)
    LIST_ENTRY(alias_link) list_type; 
#endif

#if defined(CONFIG_RTL_NAPT_TCP_GARBAGE_COLLECTION)
    LIST_ENTRY(alias_link) list_tcp; 
#endif

    union                        /* Auxiliary data                      */
    {
        char *frag_ptr;
        struct in_addr frag_addr;
        struct tcp_dat *tcp;
        struct udp_dat *udp;
    } data;

//#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)	//jwj20120716: add dir and assured bit
    u_int8_t dir;
    u_int8_t assured;	
    u_int8_t force_del;
//#endif
#if defined(CONFIG_RTL_819X)
    unsigned short	total_len;
    unsigned short	sum_len;
#endif

#if defined(HAVE_NAT_ALG) && defined(HAVE_NAT_ALG_H323)
    struct h323_info h323info;    
#endif
#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)||defined(CONFIG_RTL_HARDWARE_NAT)
	unsigned int count;
#endif
#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
	bool rtl_add_fastpath;
#endif
#if defined(CONFIG_RTL_HARDWARE_NAT)
	bool rtl_add_hwnat;
#endif

#if defined(CONFIG_RTL_BYPASS_PKT)
    int bypass_cnt;
#endif
};

#endif	//__RTL_ALIAS_H
