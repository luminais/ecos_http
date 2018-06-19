/*
 *
 * Revision Details
 * ----------------
 * $Log: not supported by cvs2svn $
 *
 */

//==============================================================================
//                                INCLUDE FILES
//==============================================================================

//==============================================================================
//                                    MACROS
//==============================================================================
extern unsigned int ip_defense;

#define DEF_IP_SPOOFING		(ip_defense&0x00000001)
#define DEF_SHORT_PACKET	(ip_defense&0x00000002)
#define DEF_PING_OF_DEATH	(ip_defense&0x00000004)
#define DEF_LAND_ATTACK		(ip_defense&0x00000008)
#define DEF_SNORK_ATTACK	(ip_defense&0x00000010)
#define DEF_UDP_PORT_LOOP	(ip_defense&0x00000020)
#define DEF_TCP_NULL_SCAN	(ip_defense&0x00000040)
#define DEF_SMURF_ATTACK	(ip_defense&0x00000080)
#define DEF_SYN_FLOODING	(ip_defense&0x00000100)
#define DEF_IP_FRAG			(ip_defense&0x00000200)

#define SET_DEF_IP_SPOOFING		(ip_defense|=0x00000001)
#define SET_DEF_SHORT_PACKET	(ip_defense|=0x00000002)
#define SET_DEF_PING_OF_DEATH	(ip_defense|=0x00000004)
#define SET_DEF_LAND_ATTACK		(ip_defense|=0x00000008)
#define SET_DEF_SNORK_ATTACK	(ip_defense|=0x00000010)
#define SET_DEF_UDP_PORT_LOOP	(ip_defense|=0x00000020)
#define SET_DEF_TCP_NULL_SCAN	(ip_defense|=0x00000040)
#define SET_DEF_SMURF_ATTACK	(ip_defense|=0x00000080)
#define SET_DEF_SYN_FLOODING	(ip_defense|=0x00000100)
#define SET_DEF_IP_FRAG			(ip_defense|=0x00000200)

#define PORTSCAN_REC		3

#define	LOG_KERN		(0<<3)		/* kernel messages */
#define	LOG_USER		(1<<3)		/* random user-level messages */
#define	LOG_MAIL		(2<<3)		/* mail system */
#define	LOG_DAEMON		(3<<3)		/* system daemons */
#define	LOG_AUTH		(4<<3)		/* security/authorization messages */
#define	LOG_SYSLOG		(5<<3)		/* messages generated internally by syslogd */

//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================
struct rate_limit {
	unsigned int lasttime;
	int curpps;
	int maxpps;	/* maximum pps allowed */
};

extern struct rate_limit sync_rate;
#define SYN_FLOOD_RATE	(sync_rate)
#define SYN_FLOOD_DAFAULT_RATE		150

//==============================================================================
//                               LOCAL VARIABLES
//==============================================================================
extern unsigned int ip_defense;

//==============================================================================
//                          LOCAL FUNCTION PROTOTYPES
//==============================================================================

//==============================================================================
//                              EXTERNAL FUNCTIONS
//==============================================================================
