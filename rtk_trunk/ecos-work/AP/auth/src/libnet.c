
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>

#include "config.h"

#include "libnet.h"
#include <net/if.h>
#ifdef __ECOS
//#include <netinet/if_ether.h>
#include <net/if_arp.h>
#else
#if (__GLIBC__)
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#else
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#endif
#endif

#include "bpf.h"
/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM	0		/* from KA9Q: NET/ROM pseudo	*/
#define ARPHRD_ETHER 	1		/* Ethernet 10Mbps		*/
#define	ARPHRD_EETHER	2		/* Experimental Ethernet	*/
#define	ARPHRD_AX25	3		/* AX.25 Level 2		*/
#define	ARPHRD_PRONET	4		/* PROnet token ring		*/
#define	ARPHRD_CHAOS	5		/* Chaosnet			*/
#define	ARPHRD_IEEE802	6		/* IEEE 802.2 Ethernet/TR/TB	*/
#define	ARPHRD_ARCNET	7		/* ARCnet			*/
#define	ARPHRD_APPLETLK	8		/* APPLEtalk			*/
#define ARPHRD_DLCI	15		/* Frame Relay DLCI		*/
#define ARPHRD_ATM	19		/* ATM 				*/
#define ARPHRD_METRICOM	23		/* Metricom STRIP (new IANA id)	*/
#define	ARPHRD_IEEE1394	24		/* IEEE 1394 IPv4 - RFC 2734	*/
#define ARPHRD_EUI64	27		/* EUI-64                       */
#define ARPHRD_INFINIBAND 32		/* InfiniBand			*/

/* Dummy types for non ARP hardware */
#define ARPHRD_SLIP	256
#define ARPHRD_CSLIP	257
#define ARPHRD_SLIP6	258
#define ARPHRD_CSLIP6	259
#define ARPHRD_RSRVD	260		/* Notional KISS type 		*/
#define ARPHRD_ADAPT	264
#define ARPHRD_ROSE	270
#define ARPHRD_X25	271		/* CCITT X.25			*/
#define ARPHRD_HWX25	272		/* Boards with X.25 in firmware	*/
#define ARPHRD_CAN	280		/* Controller Area Network      */
#define ARPHRD_PPP	512
#define ARPHRD_CISCO	513		/* Cisco HDLC	 		*/
#define ARPHRD_HDLC	ARPHRD_CISCO
#define ARPHRD_LAPB	516		/* LAPB				*/
#define ARPHRD_DDCMP    517		/* Digital's DDCMP protocol     */
#define ARPHRD_RAWHDLC	518		/* Raw HDLC			*/

#define ARPHRD_TUNNEL	768		/* IPIP tunnel			*/
#define ARPHRD_TUNNEL6	769		/* IP6IP6 tunnel       		*/
#define ARPHRD_FRAD	770             /* Frame Relay Access Device    */
#define ARPHRD_SKIP	771		/* SKIP vif			*/
#define ARPHRD_LOOPBACK	772		/* Loopback device		*/
#define ARPHRD_LOCALTLK 773		/* Localtalk device		*/
#define ARPHRD_FDDI	774		/* Fiber Distributed Data Interface */
#define ARPHRD_BIF      775             /* AP1000 BIF                   */
#define ARPHRD_SIT	776		/* sit0 device - IPv6-in-IPv4	*/
#define ARPHRD_IPDDP	777		/* IP over DDP tunneller	*/
#define ARPHRD_IPGRE	778		/* GRE over IP			*/
#define ARPHRD_PIMREG	779		/* PIMSM register interface	*/
#define ARPHRD_HIPPI	780		/* High Performance Parallel Interface */
#define ARPHRD_ASH	781		/* Nexus 64Mbps Ash		*/
#define ARPHRD_ECONET	782		/* Acorn Econet			*/
#define ARPHRD_IRDA 	783		/* Linux-IrDA			*/
/* ARP works differently on different FC media .. so  */
#define ARPHRD_FCPP	784		/* Point to point fibrechannel	*/
#define ARPHRD_FCAL	785		/* Fibrechannel arbitrated loop */
#define ARPHRD_FCPL	786		/* Fibrechannel public loop	*/
#define ARPHRD_FCFABRIC	787		/* Fibrechannel fabric		*/
	/* 787->799 reserved for fibrechannel media types */
#define ARPHRD_IEEE802_TR 800		/* Magic type ident for TR	*/
#define ARPHRD_IEEE80211 801		/* IEEE 802.11			*/
#define ARPHRD_IEEE80211_PRISM 802	/* IEEE 802.11 + Prism2 header  */
#define ARPHRD_IEEE80211_RADIOTAP 803	/* IEEE 802.11 + radiotap header */

#define ARPHRD_PHONET	820		/* PhoNet media type		*/
#define ARPHRD_PHONET_PIPE 821		/* PhoNet pipe header		*/
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
extern struct eth_drv_sc rltk819x_wlan1_sc0;
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID
extern struct eth_drv_sc rltk819x_wlan1_vap_sc0;
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 2
extern struct eth_drv_sc rltk819x_wlan1_vap_sc1;
#endif
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
extern struct eth_drv_sc rltk819x_wlan1_vap_sc2;
extern struct eth_drv_sc rltk819x_wlan1_vap_sc3;
#endif
#endif
#endif

extern struct eth_drv_sc rltk819x_wlan_sc0;
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID
extern struct eth_drv_sc rltk819x_wlan0_vap_sc0;
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 2
extern struct eth_drv_sc rltk819x_wlan0_vap_sc1;
#endif
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
extern struct eth_drv_sc rltk819x_wlan0_vap_sc2;
extern struct eth_drv_sc rltk819x_wlan0_vap_sc3;
#endif
#endif

struct libnet_link_int *
libnet_open_link_interface(char *device, char *ebuf)
{
    register struct libnet_link_int *l;
    struct ifreq ifr;


    l = (struct libnet_link_int *)malloc(sizeof (*l));
    if (l == NULL)
    {
        sprintf(ebuf, "malloc: %s", strerror(errno));
        return (NULL);
    }
    memset(l, 0, sizeof (*l));


    //l->fd = socket(PF_INET, SOCK_PACKET, htons(ETH_P_ALL));
    // sc_yang , modify to avoid every packet copy
#ifdef __ECOS
	l->fd = socket(PF_INET, SOCK_RAW, 0);
#else
    l->fd = socket(PF_INET, SOCK_PACKET, 0);
#endif
    if (l->fd == -1)
    {
        sprintf(ebuf, "socket: %s", strerror(errno));
        goto bad;
    }


    memset(&ifr, 0, sizeof (ifr));
    strncpy(ifr.ifr_name, device, sizeof (ifr.ifr_name));
    if (ioctl(l->fd, SIOCGIFHWADDR, &ifr) < 0 )
    {
        sprintf(ebuf, "SIOCGIFHWADDR: %s", strerror(errno));
        goto bad;
    }	
    switch (ifr.ifr_hwaddr.sa_family)
    {
        case ARPHRD_ETHER:
		case ARPHRD_EETHER:
        case ARPHRD_METRICOM:

            l->linktype = DLT_EN10MB;
            l->linkoffset = 0xe;
            break;
        case ARPHRD_SLIP:
        case ARPHRD_CSLIP:
        case ARPHRD_SLIP6:
        case ARPHRD_CSLIP6:
        case ARPHRD_PPP:
            l->linktype = DLT_RAW;
            break;
        default:
            sprintf(ebuf, "unknown physical layer type 0x%x",
                ifr.ifr_hwaddr.sa_family);
        goto bad;
    }
    return (l);

bad:
    if (l->fd >= 0)
    {
        close(l->fd);
    }
    free(l);
    return (NULL);
}
int
libnet_write_link_layer(struct libnet_link_int *l, const char *device,
            u_char *buf, int len)
{


	if(strcmp(device,"wlan0")==0){
		rltk819x_send_wlan(&rltk819x_wlan_sc0, buf, len);
	}
	
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID
	else if(strcmp(device,"wlan0-va0")==0){
		rltk819x_send_wlan(&rltk819x_wlan0_vap_sc0, buf, len);
	}
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 2
	else if(strcmp(device,"wlan0-va1")==0){
		rltk819x_send_wlan(&rltk819x_wlan0_vap_sc1, buf, len);
	}
#endif
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
	else if(strcmp(device,"wlan0-va2")==0){
		rltk819x_send_wlan(&rltk819x_wlan0_vap_sc2, buf, len);
	}
	else if(strcmp(device,"wlan0-va3")==0){
		rltk819x_send_wlan(&rltk819x_wlan0_vap_sc3, buf, len);
	}
#endif
#endif
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN

	else if(strcmp(device,"wlan1")==0){
		rltk819x_send_wlan(&rltk819x_wlan1_sc0, buf, len);
	}
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID
	else if(strcmp(device,"wlan1-va0")==0){
		rltk819x_send_wlan(&rltk819x_wlan1_vap_sc0, buf, len);
	}
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 2
	else if(strcmp(device,"wlan1-va1")==0){
		rltk819x_send_wlan(&rltk819x_wlan1_vap_sc1, buf, len);
	}
#endif
#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
	else if(strcmp(device,"wlan1-va2")==0){
		rltk819x_send_wlan(&rltk819x_wlan1_vap_sc2, buf, len);
	}
	else if(strcmp(device,"wlan1-va3")==0){
		rltk819x_send_wlan(&rltk819x_wlan1_vap_sc3, buf, len);
	}
#endif
#endif
#endif
	return len;

#if 0 
	int c;

    struct sockaddr sa;

    memset(&sa, 0, sizeof (sa));

    strncpy(sa.sa_data, device, sizeof (sa.sa_data));

    c = sendto(l->fd, buf, len, 0, (struct sockaddr *)&sa, sizeof (sa));
    if (c != len)
    {
#if (__DEBUG)
        libnet_error(LIBNET_ERR_WARNING,
            "write_link_layer: %d bytes written (%s)\n", c,
            strerror(errno));
#endif
    }
    return (c);
#endif
}


int
libnet_close_link_interface(struct libnet_link_int *l)
{
    if (close(l->fd) == 0)
    {
        return (1);
    }
    else
    {
        return (-1);
    }
}

#if 1
struct ether_addr *
libnet_get_hwaddr(struct libnet_link_int *l, const char *device, char *ebuf)
{
    int fd;
    struct ifreq ifr;
    struct ether_addr *eap;
    /*
     *  XXX - non-re-entrant!
     */
    static struct ether_addr ea;

    /*
     *  Create dummy socket to perform an ioctl upon.
     */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        sprintf(ebuf, "get_hwaddr: %s", strerror(errno));
        return (NULL);
    }

    memset(&ifr, 0, sizeof(ifr));
    eap = &ea;
    strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

    if (ioctl(fd, SIOCGIFHWADDR, (char *)&ifr) < 0)
    {
        close(fd);
        sprintf(ebuf, "get_hwaddr: %s", strerror(errno));
        return (NULL);
    }
    memcpy(eap, &ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
    close(fd);
    return (eap);
}
#endif


#ifndef COMPACK_SIZE
int
libnet_in_cksum(u_short *addr, int len)
{
    int sum;
    int nleft;
    u_short ans;
    u_short *w;

    sum = 0;
    ans = 0;
    nleft = len;
    w = addr;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1)
    {
        *(u_char *)(&ans) = *(u_char *)w;
        sum += ans;
    }
    return (sum);
}
#endif


#ifndef COMPACK_SIZE
int
libnet_do_checksum(u_char *buf, int protocol, int len)
{
    struct libnet_ip_hdr *iph_p;
    int ip_hl;
    int sum;

    sum = 0;
    iph_p = (struct libnet_ip_hdr *)buf;
    ip_hl = iph_p->ip_hl << 2;

    /*
     *  Dug Song came up with this very cool checksuming implementation
     *  eliminating the need for explicit psuedoheader use.  Check it out.
     */
    switch (protocol)
    {
        /*
         *  Style note: normally I don't advocate declaring variables inside
         *  blocks of control, but it makes good sense here. -- MDS
         */
        case IPPROTO_TCP:
        {
            struct libnet_tcp_hdr *tcph_p =
                (struct libnet_tcp_hdr *)(buf + ip_hl);

#if (STUPID_SOLARIS_CHECKSUM_BUG)
            tcph_p->th_sum = tcph_p->th_off << 2;
            return (1);
#endif /* STUPID_SOLARIS_CHECKSUM_BUG */

            tcph_p->th_sum = 0;
            sum = libnet_in_cksum((u_short *)&iph_p->ip_src, 8);
            sum += ntohs(IPPROTO_TCP + len);
            sum += libnet_in_cksum((u_short *)tcph_p, len);
            tcph_p->th_sum = LIBNET_CKSUM_CARRY(sum);
            break;
        }
        case IPPROTO_UDP:
        {
            struct libnet_udp_hdr *udph_p =
                (struct libnet_udp_hdr *)(buf + ip_hl);

            udph_p->uh_sum = 0;
            sum = libnet_in_cksum((u_short *)&iph_p->ip_src, 8);
            sum += ntohs(IPPROTO_UDP + len);
            sum += libnet_in_cksum((u_short *)udph_p, len);
            udph_p->uh_sum = LIBNET_CKSUM_CARRY(sum);
            break;
        }
        case IPPROTO_ICMP:
        {
            struct libnet_icmp_hdr *icmph_p =
                (struct libnet_icmp_hdr *)(buf + ip_hl);

            icmph_p->icmp_sum = 0;
            sum = libnet_in_cksum((u_short *)icmph_p, len);
            icmph_p->icmp_sum = LIBNET_CKSUM_CARRY(sum);
            break;
        }
        case IPPROTO_IGMP:
        {
            struct libnet_igmp_hdr *igmph_p =
                (struct libnet_igmp_hdr *)(buf + ip_hl);

            igmph_p->igmp_sum = 0;
            sum = libnet_in_cksum((u_short *)igmph_p, len);
            igmph_p->igmp_sum = LIBNET_CKSUM_CARRY(sum);
            break;
        }
        case IPPROTO_OSPF:
        {
            struct libnet_ospf_hdr *oh_p =
                (struct libnet_ospf_hdr *)(buf + ip_hl);

            u_char *payload = (u_char *)(buf + ip_hl + LIBNET_AUTH_H + 
                        sizeof(oh_p));
            u_char *tbuf = (u_char *)malloc(sizeof(oh_p) + sizeof(payload));
            if (tbuf == NULL)
            {
                return (-1);
            }
            oh_p->ospf_cksum = 0;
            sum += libnet_in_cksum((u_short *)tbuf, sizeof(tbuf));
            oh_p->ospf_cksum = LIBNET_CKSUM_CARRY(sum);
            free(tbuf);
            break;
        }
        case IPPROTO_OSPF_LSA:
        {
            /*
             *  Reworked fletcher checksum taken from RFC 1008.
             */
            int c0, c1;
            struct libnet_lsa_hdr *lsa_p = (struct libnet_lsa_hdr *)buf;
            u_char *p, *p1, *p2, *p3;

            c0 = 0;
            c1 = 0;

            lsa_p->lsa_cksum[0] = 0;
            lsa_p->lsa_cksum[1] = 0;    /* zero out checksum */

            p = buf;
            p1 = buf;
            p3 = buf + len;             /* beginning and end of buf */

            while (p1 < p3)
            {
                p2 = p1 + LIBNET_MODX;
                if (p2 > p3)
                {
                    p2 = p3;
                }
  
                for (p = p1; p < p2; p++)
                {
                    c0 += (*p);
                    c1 += c0;
                }

                c0 %= 255;
                c1 %= 255;      /* modular 255 */
 
                p1 = p2;
            }

            lsa_p->lsa_cksum[0] = (((len - 17) * c0 - c1) % 255);
            if (lsa_p->lsa_cksum[0] <= 0)
            {
                lsa_p->lsa_cksum[0] += 255;
            }

            lsa_p->lsa_cksum[1] = (510 - c0 - lsa_p->lsa_cksum[0]);
            if (lsa_p->lsa_cksum[1] > 255)
            {
                lsa_p->lsa_cksum[1] -= 255;
            }
            break;
        }
        case IPPROTO_IP:
        {
            iph_p->ip_sum = 0;
            sum = libnet_in_cksum((u_short *)iph_p, len);
            iph_p->ip_sum = LIBNET_CKSUM_CARRY(sum);
            break;
        }
        case IPPROTO_VRRP:
        {
            struct libnet_vrrp_hdr *vrrph_p =
                (struct libnet_vrrp_hdr *)(buf + ip_hl);

            vrrph_p->vrrp_sum = 0;
            sum = libnet_in_cksum((u_short *)vrrph_p, len);
            vrrph_p->vrrp_sum = LIBNET_CKSUM_CARRY(sum);
            break;
        }
        default:
        {
#if (__DEBUG)
            libnet_error(LN_ERR_CRITICAL, "do_checksum: UNSUPP protocol %d\n",
                    protocol);
#endif
            return (-1);
        }
    }
    return (1);
}


u_short
libnet_ip_check(u_short *addr, int len)
{
    int sum;

    sum = libnet_in_cksum(addr, len);
    return (LIBNET_CKSUM_CARRY(sum));
}

#endif
u_long
libnet_get_ipaddr(struct libnet_link_int *l, const char *device, char *ebuf)
{
    struct ifreq ifr;
    register struct sockaddr_in *sin;
    int fd;

    /*
     *  Create dummy socket to perform an ioctl upon.
     */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        sprintf(ebuf, "socket: %s", strerror(errno));
        return (0);
    }

    memset(&ifr, 0, sizeof(ifr));
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl(fd, SIOCGIFADDR, (char*) &ifr) < 0)
    {
        close(fd);
        return(0);
    }
    close(fd);
    return (ntohl(sin->sin_addr.s_addr));
}
