#ifndef	__RTL_NETSNIPER_H__
#define	__RTL_NETSNIPER_H__

/* System include files */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <sys/queue.h>
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


typedef unsigned long long	uint64;
typedef long long		    int64;
typedef unsigned int	    uint32;
typedef int			        int32;
typedef uint32              ipaddr_t;
typedef unsigned short	    uint16;
typedef short			    int16;
typedef unsigned char	    uint8;
typedef char			    int8;

#define RTL_NETSNIPER_IMPROVE_PERFORMANCE

#define NETSNIPER_IP_BIT    (1<<0)
#define NETSNIPER_TCP_BIT   (1<<1)
#define NETSNIPER_HTTP_BIT  (1<<2)
#define NETSNIPER_NTP_BIT   (1<<3)

#define SUCCESS 0
#define FAILED -1

#define GATEWAY_MODE 0
#define WISP_MODE 2
#define CLIENT_MODE 1

#define NETSNIPER_INTERFACE_WAN 1
#define NETSNIPER_INTERFACE_LAN 2

#define NETSNIPER_FIX_IP_LAYER
#define NETSNIPER_FIX_TCP_LAYER
#define NETSNIPER_FIX_APPLICATION_LAYER

#define RTL_NETSNIPER_DEFTTL	128
#define RTL_NETSNIPER_INITIDENT	1
#define RTL_NETSNIPER_MAXIDENT	65535U

#define	IPFRAG_TABLE_LIST_MAX	32
#define	IPFRAG_TABLE_ENTRY_MAX	128

#define IP_FRAG_IDLE            0
#define IP_FRAG_FORWADING       1
#define IP_FRAG_COMPLETE        2
#define IP_FRAG_IN_KERNEL       3
#define IP_FRAG_CACHE_TIMEOUT (5 * HZ)
#define IP_FRAG_TIMEOUT_CHECK		(hz)


#define IP_FRAG_ORDERLY         1
#define IP_FRAG_DISORDER        2

#define	CONNTRACK_TABLE_LIST_MAX	1024
#define	CONNTRACK_TABLE_ENTRY_MAX	1024
#define CONNTRACK_IDLE              0
#define CONNTRACK_INUSE             1
#define RTL_NETSNIPER_INITSEQ	    1
#define RTL_NETSNIPER_MAXSEQ	    4294967295U
//#define RTL_NETSNIPER_WINSIZE       16426 
#define RTL_NETSNIPER_WINSIZE       44620 
#define RTL_NETSNIPER_WMAXSIZE      (1<<16) -1
#define RTL_NETSNIPER_MSS           1380

#define CONNTRACK_STATE_UNCONNECTED_TIMEOUT (2*60*HZ)
#define CONNTRACK_CONNECTED_TIME_OUT (60*60*HZ)
#define CONNTRACK_STATE_CLOSE_TIMEOUT (60*HZ)
#define CONNTRACK_TIMEOUT_CHECK		(hz)

#define CONNTRACK_STATE_UNCONNECTED 1
#define CONNTRACK_STATE_CONNECTED   2
#define CONNTRACK_STATE_CLOSE       3


#define HTTP_SERVER_PORT            80
#define HTTP_OS_SYSTEM              "(compatible; Windows)"

#define NTP_PORT                    123
#define NTP_EPOCH            (86400U * (365U * 70U + 17U))
#define NTP_DIFF_UTC         (8U*60U*60U)

struct ntp_packet
{
#if defined(__BIG_ENDIAN_BITFIELD) 
  unsigned char li : 2;
  unsigned char vn : 3;
  unsigned char mode : 3;
#else
  unsigned char mode : 3;
  unsigned char vn : 3;
  unsigned char li : 2;
#endif
  unsigned char stratum;
  char poll;
  char precision;
  unsigned long root_delay;
  unsigned long root_dispersion;
  unsigned long reference_identifier;
  unsigned long reference_timestamp_secs;
  unsigned long reference_timestamp_fraq;
  unsigned long originate_timestamp_secs;
  unsigned long originate_timestamp_fraq;
  unsigned long receive_timestamp_seqs;
  unsigned long receive_timestamp_fraq;
  unsigned long transmit_timestamp_secs;
  unsigned long transmit_timestamp_fraq;
};

#define LAN_TO_WAN 1
#define WAN_TO_LAN 2
#define LAN_TO_LAN 3
#define DUT_TO_LAN 4
#define DUT_TO_WAN 5
#define WAN_TO_DUT 6

#define NETSNIPER_ADJUST_CHKSUM_TWOBYTES(twobytes_mod, twobytes_org, chksum) \
	do { \
		s32 accumulate = 0; \
		accumulate += (twobytes_org); \
		accumulate -= (twobytes_mod); \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((uint16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */


#define NETSNIPER_ADJUST_CHKSUM_FOURBYTES(fourbytes_mod, fourbytes_org, chksum) \
    do { \
        s32 accumulate = 0; \
        accumulate = ((fourbytes_org) & 0xffff); \
        accumulate += (( (fourbytes_org) >> 16 ) & 0xffff); \
        accumulate -= ((fourbytes_mod) & 0xffff); \
        accumulate -= (( (fourbytes_mod) >> 16 ) & 0xffff); \
        accumulate += ntohs(chksum); \
        if (accumulate < 0) { \
            accumulate = -accumulate; \
            accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
            accumulate += accumulate >> 16; \
            chksum = htons((uint16) ~accumulate); \
        } else { \
            accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
            accumulate += accumulate >> 16; \
            chksum = htons((uint16) accumulate); \
        } \
    }while(0)   /* Checksum adjustment */

#endif	/* __RTL_NETSNIPER_H__ */

