/*pppoe header file ,part is from freebsd 
  HF*/
#ifndef __MY_PPPOE_H__
#define __MY_PPPOE_H__

/* Number of active sessions we can handle */
#define	PPPOE_NUM_SESSIONS		16 /* for now */
#define	PPPOE_SERVICE_NAME_SIZE		64 /* for now */
#define NUMTAGS 20 /* number of tags we are set up to work with */
#define NG_NODELEN	15	/* max node name len (16 with null) */

#define MAX_PPPOE_NUM 2

/********************************************************************
 * Constants and definitions specific to pppoe
 ********************************************************************/
#define PPPOE_TIMEOUT_LIMIT 64
#define PPPOE_OFFER_TIMEOUT 16
#define PPPOE_INITIAL_TIMEOUT 2

/* Codes to identify message types */
#define PADI_CODE	0x09
#define PADO_CODE	0x07
#define PADR_CODE	0x19
#define PADS_CODE	0x65
#define PADT_CODE	0xa7

/* Tag identifiers */
#if BYTE_ORDER == BIG_ENDIAN
#define PTT_EOL		(0x0000)
#define PTT_SRV_NAME	(0x0101)
#define PTT_AC_NAME	(0x0102)
#define PTT_HOST_UNIQ	(0x0103)
#define PTT_AC_COOKIE	(0x0104)
#define PTT_VENDOR 	(0x0105)
#define PTT_RELAY_SID	(0x0106)
#define PTT_SRV_ERR     (0x0201)
#define PTT_SYS_ERR  	(0x0202)
#define PTT_GEN_ERR  	(0x0203)

#define ETHERTYPE_PPPOE_DISC	0x8863	/* pppoe discovery packets     */
#define ETHERTYPE_PPPOE_SESS	0x8864	/* pppoe session packets       */
#define ETHERTYPE_PPPOE_STUPID_DISC 0x3c12 /* pppoe discovery packets 3com? */
#define ETHERTYPE_PPPOE_STUPID_SESS 0x3c13 /* pppoe session packets   3com? */
#else
#define PTT_EOL		(0x0000)
#define PTT_SRV_NAME	(0x0101)
#define PTT_AC_NAME	(0x0201)
#define PTT_HOST_UNIQ	(0x0301)
#define PTT_AC_COOKIE	(0x0401)
#define PTT_VENDOR 	(0x0501)
#define PTT_RELAY_SID	(0x0601)
#define PTT_SRV_ERR     (0x0102)
#define PTT_SYS_ERR  	(0x0202)
#define PTT_GEN_ERR  	(0x0302)

#define ETHERTYPE_PPPOE_DISC	0x6388	/* pppoe discovery packets     */
#define ETHERTYPE_PPPOE_SESS	0x6488	/* pppoe session packets       */
#define ETHERTYPE_PPPOE_STUPID_DISC 0x123c /* pppoe discovery packets 3com? */
#define ETHERTYPE_PPPOE_STUPID_SESS 0x133c /* pppoe session packets   3com? */
#endif


#define PPPOE_FREE_M(m)							\
	do {								\
		if ((m)) {						\
			m_freem((m));					\
			(m) = NULL;					\
		}							\
	} while (0)

struct pppoe_tag {
	u_int16_t tag_type;
	u_int16_t tag_len;
	char tag_data[0];
}__attribute ((packed));

struct pppoe_hdr{
	u_int8_t ver:4;
	u_int8_t type:4;
	u_int8_t code;
	u_int16_t sid;
	u_int16_t length;
	struct pppoe_tag tag[0];
}__attribute__ ((packed));


struct pppoe_full_hdr {
	struct  ether_header eh;
	struct pppoe_hdr ph;
}__attribute__ ((packed));

union	packet {
	struct pppoe_full_hdr	pkt_header;
	u_int8_t	bytes[2048];
};

struct datatag {
        struct pppoe_tag hdr;
	u_int8_t        data[PPPOE_SERVICE_NAME_SIZE];
};     

union uniq {
	char bytes[sizeof(void *)];
	void * pointer;
	};

/*
 * States for the session state machine.
 * These have no meaning if there is no hook attached yet.
 */
typedef enum pppoe_state {
    PPPOE_SNONE=0,	               /* [both] Initial state */
    PPPOE_LISTENING,	        /* [Daemon] Listening for discover initiation pkt */
    PPPOE_SINIT,	                      /* [Client] Sent discovery initiation */
    PPPOE_PRIMED,	               /* [Server] Awaiting PADI from daemon */
    PPPOE_SOFFER,	               /* [Server] Sent offer message  (got PADI)*/
    PPPOE_SREQ,		               /* [Client] Sent a Request */
    PPPOE_NEWCONNECTED,	 /* [Server] Connection established, No data received */
    PPPOE_CONNECTED,	        /* [Both] Connection established, Data received */
    PPPOE_DEAD		               /* [Both] */
} PPPOE_STATE;


/*
 * Information we store for each hook on each node for negotiating the 
 * session. The mbuf and cluster are freed once negotiation has completed.
 * The whole negotiation block is then discarded.
 */

struct sess_neg {
	struct mbuf 		*m; /* holds cluster with last sent packet */
	union	packet		*pkt; /* points within the above cluster */
	//HF 20120829
	//struct callout_handle	timeout_handle;   /* see timeout(9) */
	u_int			timeout; /* 0,1,2,4,8,16 etc. seconds */
	u_int			numtags;
	struct pppoe_tag	*tags[NUMTAGS];
	u_int			service_len;
	u_int			ac_name_len;

	struct datatag		service;
	struct datatag		ac_name;
};
typedef struct sess_neg *negp;

/*
 * Session information that is needed after connection.
 */
struct sess_con {
	u_int16_t		Session_ID;
	PPPOE_STATE		state;
	char			creator[NG_NODELEN + 1]; /* who to notify */
	struct pppoe_full_hdr	pkt_hdr;	/* used when connected */
	negp			neg;		/* used when negotiating */
	/*struct sess_con	*hash_next;*/	/* not yet used */
	struct ifnet *ifp;
	unsigned char  inited;
	unsigned char  idx;	
	cyg_ppp_handle_t ppp_handle;
	char username[32];
	char passwd[32];
	char intfname[32];
	unsigned int idle_time_limit;
	unsigned int demand_mode;	//1: demand 0:other
	unsigned short mtu;
	int (*save_connInfo)(u_int16_t Session_ID,u_char*ether_servMac);//save connInfo for power off disconnect 
};

typedef struct sess_con *sessp;

#if 0
typedef struct pppoe_session {
	int sockfd;
	PPPOE_STATE state;
	int retry_timeout;
	int retry_timeout_long;
	int retry_limits;
	int retry_count;
	unsigned char myethaddr[ETH_ALEN];
	unsigned char peerethaddr[ETH_ALEN];
	unsigned short session_id;
	char *intfname;
	char *serviceName;
	char *acName;
	int useHostUniq;
	timeout_entry retry_timer;
	struct pppoe_tag cookie;
	struct pppoe_tag relayId;
	/*packet justify*/
	int printACNames;
	int acNameOK;
	int serviceNameOK;
	int existACName;
	int existServiceName;
} PPPOE_SESS_T , *PPPOE_SESS_Tp;
#endif

/*
 * Define the order in which we will place tags in packets
 * this may be ignored
 */
/* for PADI */
#define TAGI_SVC 0
#define TAGI_HUNIQ 1
/* for PADO */
#define TAGO_ACNAME 0
#define TAGO_SVC 1
#define TAGO_COOKIE 2
#define TAGO_HUNIQ 3
/* for PADR */
#define TAGR_SVC 0
#define TAGR_HUNIQ 1
#define TAGR_COOKIE 2
/* for PADS */
#define TAGS_ACNAME 0
#define TAGS_SVC 1
#define TAGS_COOKIE 2
#define TAGS_HUNIQ 3
/* for PADT */

#endif
