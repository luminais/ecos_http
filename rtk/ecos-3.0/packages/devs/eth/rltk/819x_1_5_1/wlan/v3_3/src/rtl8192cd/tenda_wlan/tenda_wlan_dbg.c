#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#if 0
#include "ieee802_mib.h"
#include "tenda_wlan_common.h"
#include "tenda_wlan_dbg.h"
#endif

//#include "tenda_wlan_dbg.h"


#define TD_PRINT diag_printf

#define eaistr		"%u.%u.%u.%u"
#define eaip(a)		((a)[0] & 0xff), ((a)[1] & 0xff),\
				((a)[2] & 0xff), ((a)[3] & 0xff)

typedef unsigned int __be32;
typedef unsigned short __sum16;

typedef struct {
	unsigned short	ar_hrd,	/* format of hardware address */
			ar_pro;	/* format of protocol address */
	unsigned char	ar_hln,	/* length of hardware address */
			ar_pln;	/* length of protocol address */
	unsigned short	ar_op;	/* ARP opcode (command) */
	unsigned char	ar_sha[ETH_ALEN],	/* sender hardware address */
			ar_sip[4],		/* sender IP address */
			ar_tha[ETH_ALEN],	/* target hardware address */
			ar_tip[4];		/* target IP address */
} eth_arphdr;




struct udphdr {
        __be16  source;
        __be16  dest;
        __be16  len;
        __sum16 check;
};

struct dhcp_packet {
#define DHCP_UDP_OVERHEAD    (20 + /* IP header */            \
                    8)   /* UDP header */
#define DHCP_SNAME_LEN        64
#define DHCP_FILE_LEN        128
#define DHCP_FIXED_NON_UDP    236
#define DHCP_FIXED_LEN        (DHCP_FIXED_NON_UDP + DHCP_UDP_OVERHEAD)
                        /* Everything but options. */
#define BOOTP_MIN_LEN        300

#define DHCP_MTU_MAX        1500
#define DHCP_MTU_MIN            576

#define DHCP_MAX_OPTION_LEN    (DHCP_MTU_MAX - DHCP_FIXED_LEN)
#define DHCP_MIN_OPTION_LEN     (DHCP_MTU_MIN - DHCP_FIXED_LEN)
    u_int8_t  op;        /* 0: Message opcode/type */
    u_int8_t  htype;    /* 1: Hardware addr type (net/if_types.h) */
    u_int8_t  hlen;        /* 2: Hardware addr length */
    u_int8_t  hops;        /* 3: Number of relay agent hops from client */
    u_int32_t xid;        /* 4: Transaction ID */
    u_int16_t secs;        /* 8: Seconds since client started looking */
    u_int16_t flags;    /* 10: Flag bits */
    struct in_addr ciaddr;    /* 12: Client IP address (if already in use) */
    struct in_addr yiaddr;    /* 16: Client IP address */
    struct in_addr siaddr;    /* 18: IP address of next server to talk to */
    struct in_addr giaddr;    /* 20: DHCP relay agent IP address */
    unsigned char chaddr [16];    /* 24: Client hardware address */
    char sname [DHCP_SNAME_LEN];    /* 40: Server name */
    char file [DHCP_FILE_LEN];    /* 104: Boot filename */
    unsigned char options [DHCP_MAX_OPTION_LEN];
                /* 212: Optional parameters
              (actual length dependent on MTU). */
};

struct ethhdr {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	unsigned short	h_proto;		/* packet type ID field	*/
};
#define ETH_P_ARP		0x0806	/* Address Resolution packet    */
#define ICMP_ECHOREPLY          0               /* echo reply */
#define ICMP_ECHO               8               /* echo service */




struct icmphdr {
  __u8          type;
  __u8          code;
  __sum16       checksum;
  union {
        struct {
                __be16  id;
                __be16  sequence;
        } echo;
        __be32  gateway;
        struct {
                __be16  __unused;
                __be16  mtu;
        } frag;
  } un;
};


int td_dump_8023_skb(void *_priv, void *p, char *func, void *txcfg)
{
    struct tx_insn *ptxcfg = (struct tx_insn *)txcfg;
    struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)_priv;
    struct  sk_buff *skb     = (struct sk_buff *)p;
    struct  ethhdr *eh       = (struct ethhdr *)(skb->data);
    int     len              = skb->len;
    struct  iphdr      *ih;
    unsigned char       tag[8] = {0};

    if (!(priv->tenda_mib.debug & TD_DBG_SKB_PATH_MASK)) {
        return 0;
    }

    if(len < sizeof(struct  ethhdr)) {
        TD_PRINT("%s:len(%d) is less than eh header\n",func,len);
        return 0;
    }
#ifdef CONFIG_RTL_MESH_SUPPORT
    if (ptxcfg) {
        sprintf(tag,"%02x%02x",ptxcfg->nhop_11s[4],ptxcfg->nhop_11s[5]);
        if (ptxcfg->is_11s & RELAY_11S) {
            sprintf(tag+4,"R");
        }
    }
#endif

    if ((priv->tenda_mib.debug & TD_DBG_ARP) && (eh->h_proto == htons(ETH_P_ARP))) {
        eth_arphdr *arp = (eth_arphdr *)(eh + 1);
        char *arps[] = { "Unkown", "req", "rsp", "rreq", "rrsp" };
        TD_PRINT("[%s][%s%s][%15s][len=%d],"MACSTR" -> "MACSTR", arp %s," eaistr "-> " eaistr "\n",
            priv->dev->name,skb->dev->name,tag,func,len,MAC2STR(eh->h_source),MAC2STR(eh->h_dest),
            arps[htons(arp->ar_op)],eaip(arp->ar_sip),eaip(arp->ar_tip));
        return 0;
    }

    if(eh->h_proto != htons(ETH_P_IP)) {
        return 0;
    }

    if(len < (sizeof(eh) + sizeof(*ih))) {
        return 0;
    }

    ih     = (struct  iphdr *)(eh + 1);

    if ((priv->tenda_mib.debug & TD_DBG_DHCP) && ih->protocol == 17) {
        char *dhcparps[] = { "Unkonw0", "discover", "offer", "request", "Unkonw4" ,"ACK"};
        struct  udphdr     *udphd = (struct  udphdr *)(ih + 1);
        struct dhcp_packet *dhcphd = NULL;
        char *tmp = NULL;
        unsigned char dhcptype = 0;
        if ((udphd->source == htons(67) && udphd->dest == htons(68))
            || (udphd->source == htons(68) && udphd->dest == htons(67))) {
            dhcphd = (struct dhcp_packet *)(udphd + 1);
             tmp = dhcphd->options + 4;
             if(*tmp == 53) {
                tmp = tmp + 2;
                dhcptype = *tmp;
             }        
             if(dhcptype < 6) {
                TD_PRINT("[%s][%s%s][%15s][len=%d],"MACSTR" -> "MACSTR", dhcp %s\n",
                    priv->dev->name,skb->dev->name,tag,func,len,MAC2STR(eh->h_source),
                    MAC2STR(eh->h_dest),dhcparps[dhcptype]);
            }
        }
    } else if ((priv->tenda_mib.debug & TD_DBG_ICMP) && ih->protocol == IPPROTO_ICMP) {
        struct  icmphdr *icmp = (struct  icmphdr *)(ih + 1);
        if (icmp->type == ICMP_ECHOREPLY) {
            TD_PRINT("[%s][%s%s][%15s][len=%d],"MACSTR" -> "MACSTR",icmp reply,seq(%d:0x%x)\n",
                priv->dev->name,skb->dev->name,tag,func,len,MAC2STR(eh->h_source),MAC2STR(eh->h_dest),
                icmp->un.echo.sequence,icmp->un.echo.sequence);
        } else if (icmp->type == ICMP_ECHO) {
            TD_PRINT("[%s][%s%s][%15s][len=%d],"MACSTR" -> "MACSTR",icmp req  ,seq(%d:0x%x)\n",
                priv->dev->name,skb->dev->name,tag,func,len,MAC2STR(eh->h_source),MAC2STR(eh->h_dest),
                icmp->un.echo.sequence,icmp->un.echo.sequence);
        }
    }

    return 0;
}

void td_wlan_dbg_help(void)
{
    TD_PRINT("---%s---\n",__func__);

    TD_PRINT("  TD_DBG_RADIO_CFG  : 0x%x\n",TD_DBG_RADIO_CFG);
    TD_PRINT("  TD_DBG_BIG_HAMMER : 0x%x\n",TD_DBG_BIG_HAMMER);
    TD_PRINT("  TD_DBG_EVENT      : 0x%x\n",TD_DBG_EVENT);
    TD_PRINT("  TD_DBG_AUTH       : 0x%x\n",TD_DBG_AUTH);
    TD_PRINT("  TD_DBG_ASSOC      : 0x%x\n",TD_DBG_ASSOC);
    TD_PRINT("  TD_DBG_DISASSOC   : 0x%x\n",TD_DBG_DISASSOC);
    TD_PRINT("  TD_DBG_DEAUTH     : 0x%x\n",TD_DBG_DEAUTH);

    TD_PRINT("\n  TD_DBG_ISOLATE     : 0x%x\n",TD_DBG_ISOLATE);

    TD_PRINT("  TD_DBG_EAP        : 0x%x\n",TD_DBG_EAP);
    TD_PRINT("  TD_DBG_ICMP       : 0x%x\n",TD_DBG_ICMP);
    TD_PRINT("  TD_DBG_DHCP       : 0x%x\n",TD_DBG_DHCP);
    TD_PRINT("  TD_DBG_ARP        : 0x%x\n",TD_DBG_ARP);
    TD_PRINT("  TD_DBG_ALL        : 0x%x\n",TD_DBG_ALL);
}

void td_wlan_dbg_init(void *tenda_mib)
{
    struct tenda_priv_mib *mib = (struct tenda_priv_mib *)tenda_mib;

    mib->debug = TD_DBG_MGMT_CTRL_MASK;
}

