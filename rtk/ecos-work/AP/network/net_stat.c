
#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>
#include <pkgconf/net.h>
#include <pkgconf/kernel.h>

#include <cyg/infra/cyg_trac.h>        /* tracing macros */
#include <cyg/infra/cyg_ass.h>         /* assertion macros */

#include <cyg/kernel/kapi.h>

#include <unistd.h>
#include <ctype.h>

/* ================================================================= */
/* Include all the necessary network headers by hand. We need to do
 * this so that _KERNEL is correctly defined, or not, for specific
 * headers so we can use both the external API and get at internal
 * kernel structures.
 */

#define _KERNEL
#include <sys/param.h>
#undef _KERNEL
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <netdb.h>
#define _KERNEL

#if defined(CYGPKG_NET_FREEBSD_STACK)
#include <sys/sysctl.h>
#endif
#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/route.h>
#include <net/if_dl.h>

#include <sys/protosw.h>
#include <netinet/in_pcb.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <netinet/ip_var.h>
#include <netinet/icmp_var.h>
#include <netinet/udp_var.h>
#include <netinet/tcp_var.h>
#ifdef CYGPKG_NET_INET6
#include <netinet/ip6.h>
#include <net/if_var.h>
#include <netinet6/ip6_var.h>
#include <netinet6/in6_var.h>
#include <netinet/icmp6.h>
#endif


#include <sys/mbuf.h>

#include <cyg/io/eth/eth_drv_stats.h>

#if defined(CYGPKG_NET_OPENBSD_STACK)
#include <stdio.h>
#endif

/*
extern struct ipstat ipstat;
extern struct icmpstat icmpstat;
extern struct udpstat udpstat;
extern struct tcpstat tcpstat;
extern struct mbstat mbstat;
*/

/*
 * show information about the
 * network interfaces and the protocols.
 */

struct lan_port_status {
    unsigned char link;
    unsigned char speed;
    unsigned char duplex;
    unsigned char nway;    	
}; 

struct port_statistics  {
	unsigned int  rx_bytes;		
 	unsigned int  rx_unipkts;		
       unsigned int  rx_mulpkts;			
	unsigned int  rx_bropkts;		
 	unsigned int  rx_discard;		
       unsigned int  rx_error;			
	unsigned int  tx_bytes;		
 	unsigned int  tx_unipkts;		
       unsigned int  tx_mulpkts;			
	unsigned int  tx_bropkts;		
 	unsigned int  tx_discard;		
       unsigned int  tx_error;			   
};

#define PHY_MODE_HALF	0
#define PHY_MODE_DUPLEX	1

#define PHY_SPEED_10M	0
#define PHY_SPEED_100M	1
#define PHY_SPEED_1000M	2

int get_port_status(unsigned int port_num,struct lan_port_status*port_stat)
{
	int s;
	struct ifreq ifr;
	struct lan_port_status status;
	
	s = socket(AF_INET,SOCK_DGRAM,0);
	if (s < 0) {
        perror("socket");
        return 1;
    }
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data=&status;		
	*((unsigned int *)ifr.ifr_data) = port_num;

    if (ioctl(s, SIOCGIFSTATUS, &ifr)) {
        perror("SIOCGIFSTATUS");
        close( s );
		return 2;
    }
	
	close(s);

	if(port_stat!=NULL){
		memcpy(port_stat,ifr.ifr_data,sizeof(struct lan_port_status));
		return 0;
	}
	else{
		return 3;
	}
	
}

int get_port_statistic(unsigned int port_num,struct port_statistics* port_stat)
{
	int s;
	struct ifreq ifr;
	struct port_statistics statistic;
	
	s = socket(AF_INET,SOCK_DGRAM,0);
	if (s < 0) {
        perror("socket");
        return 1;
    }
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = &statistic;	
	*((unsigned int *)ifr.ifr_data) = port_num;

	if (ioctl(s, SIOCGIFSTATS, &ifr)) {
        perror("SIOCGIFSTATS");
        close( s );
		return 2;
    }
	
	close(s);
	
	if(port_stat!=NULL){
		memcpy(port_stat,ifr.ifr_data,sizeof(struct port_statistics));
		return 0;
	}
	else{
		return 3;
	}
}

void show_phy_stats()
{
	
	int i;
	struct lan_port_status port_stat;
	struct port_statistics port_statistic;
	int ret;
	printf("\n");
	for(i=0;i<7;i++){
		printf("Port%d:\n",i);
		ret=get_port_status(i,&port_stat);
		if(!ret){	
			if(port_stat.link){
				printf("\tLink: UP \t");
				switch(port_stat.duplex){
					case PHY_MODE_HALF:
						printf("Duplex:HALF \t");
						break;
					case PHY_MODE_DUPLEX:
						printf("Duplex:FULL \t");
						break;
					default:
						break;
				}

				switch(port_stat.speed){
					case PHY_SPEED_10M:
						printf("Speed:10M\n");
						break;
					case PHY_SPEED_100M:
						printf("Speed:100M\n");
						break;
					case PHY_SPEED_1000M:
						printf("Speed:1000M\n");
						break;
					default:
						break;
				}			
			}	
			else
				printf("\tLINK: DOWN\n");	
		}
				
		ret = get_port_statistic(i,&port_statistic);
		if(!ret){			
        	printf("\tRx ");
			printf("Total bytes:%d,Unicast :%d, Multicast :%d Broadcast :%d ", 
				port_statistic.rx_bytes,port_statistic.rx_unipkts,
				port_statistic.rx_mulpkts,port_statistic.rx_bropkts);
        	printf("Bad:%d Discard:%d\n", port_statistic.rx_error,port_statistic.rx_discard);
			
			printf("\tTx ");
			printf("Total bytes:%d,Unicast :%d, Multicast :%d Broadcast :%d ", 
				port_statistic.tx_bytes,port_statistic.tx_unipkts,
				port_statistic.tx_mulpkts,port_statistic.tx_bropkts);
        	printf("Bad:%d Discard:%d \n", port_statistic.tx_error,	port_statistic.tx_discard);		
		}
		else
			printf("get port statistic error %d\n",ret);
	}
}

/*lqz add for lan link log*/
unsigned char tenda_show_phy_stats(int port)
{
	int ret;
	struct lan_port_status port_stat;

	if(port < 0 || port > 7)
		return 0;
	
	ret = get_port_status(port,&port_stat);
	
	if(!ret)
	{	
		if(port_stat.link)
		{
			return 1;
		}	
		else
		{
			return 0;
		}
	}
	return 0;
}
/*end*/

void show_network(void)
{
	struct ifaddrs *iflist, *ifp;
	char addr[64];
#if defined(CYGPKG_NET_FREEBSD_STACK)
	u_long *mtypes;
#else
	u_short *mtypes;
#endif
	int i;

	if (getifaddrs(&iflist)!=0)
		return;
        printf("\n");
	ifp = iflist;
	while (ifp != (struct ifaddrs *)NULL) {
		if (ifp->ifa_addr->sa_family != AF_LINK) {
			printf("%s:\n", ifp->ifa_name);
			/* Get the interface's flags and display
			 * the interesting ones.
			 */
			printf("\tFlags ");
			for( i = 0; i < 16; i++ ) {
				switch( ifp->ifa_flags & (1<<i) ) {
				default: break;
				case IFF_UP: printf(" UP"); break;
				case IFF_BROADCAST: printf(" BROADCAST"); break;
				case IFF_DEBUG: printf(" DEBUG"); break;
				case IFF_LOOPBACK: printf(" LOOPBACK"); break;
				case IFF_PROMISC: printf(" PROMISCUOUS"); break;
				case IFF_RUNNING: printf(" RUNNING"); break;
				case IFF_SIMPLEX: printf(" SIMPLEX"); break;
				case IFF_MULTICAST: printf(" MULTICAST"); break;
				}
			}
			printf("\n");
			getnameinfo(ifp->ifa_addr, sizeof(*ifp->ifa_addr),
                                        addr, sizeof(addr), NULL, 0, NI_NUMERICHOST);
			printf("\tAddress %s", addr);
			if (ifp->ifa_netmask) {
				getnameinfo(ifp->ifa_netmask, sizeof(*ifp->ifa_netmask),
                                          addr, sizeof(addr), NULL, 0, NI_NUMERICHOST);
				printf(" Mask %s", addr); 
			}

			if (ifp->ifa_broadaddr) {
				getnameinfo(ifp->ifa_broadaddr, sizeof(*ifp->ifa_broadaddr),
                                          addr, sizeof(addr), NULL, 0, NI_NUMERICHOST);
				printf(" Broadcast %s", addr); 
			}
			printf("\n");
		}
		ifp = ifp->ifa_next;
	}

        /* Now the protocols. For each of the main protocols: IP,
         * ICMP, UDP, TCP print a table of useful information derived
         * from the in-kernel data structures. Note that this only
         * works for the BSD stacks.
         */
	
        printf("IPv4:\n");
        printf("\tRx ");
	printf("Total:%ld ", ipstat.ips_total);
        printf("Bad:%ld ", (ipstat.ips_badsum+ipstat.ips_tooshort+
                             ipstat.ips_toosmall+ipstat.ips_badhlen+
                             ipstat.ips_badlen+ipstat.ips_noproto+
                             ipstat.ips_toolong));
	printf("Reassembled:%ld ", ipstat.ips_reassembled);
	printf("Delivered:%ld\n", ipstat.ips_delivered);

	printf("\tTx ");
	printf("Total:%ld ", ipstat.ips_localout);
	printf("Raw:%ld ", ipstat.ips_rawout);
	printf("Fragmented:%ld\n", ipstat.ips_fragmented);

#ifdef CYGPKG_NET_INET6
        printf("IPv6:\n");
        printf("\tRx ");
	printf("Total: %lld ", ip6stat.ip6s_total);
	printf("Bad:%lld ", (ip6stat.ip6s_tooshort+ip6stat.ip6s_toosmall));
	printf("Reassembled:%lld ", ip6stat.ip6s_reassembled);
	printf("Delivered:%lld ", ip6stat.ip6s_delivered);

	printf("\tTx ");
	printf("Total: %lld ", ip6stat.ip6s_localout);
	printf("Raw:%lld ", ip6stat.ip6s_rawout);
	printf("Fragmented:%lld\n", ip6stat.ip6s_fragmented);
#endif

        printf("ICMPv4:\n");
        printf("\tRx ");
	printf("ECHO:%ld ", icmpstat.icps_inhist[ICMP_ECHO]);
	printf("ECHO REPLY:%ld ", icmpstat.icps_inhist[ICMP_ECHOREPLY]);
	printf("UNREACH:%ld ", icmpstat.icps_inhist[ICMP_UNREACH]);
	printf("REDIRECT:%ld ", icmpstat.icps_inhist[ICMP_REDIRECT]);
	printf("Other:%ld ", (icmpstat.icps_inhist[ICMP_SOURCEQUENCH]+
                             icmpstat.icps_inhist[ICMP_ROUTERADVERT]+
                             icmpstat.icps_inhist[ICMP_ROUTERSOLICIT]+
                             icmpstat.icps_inhist[ICMP_TIMXCEED]+
                             icmpstat.icps_inhist[ICMP_PARAMPROB]+
                             icmpstat.icps_inhist[ICMP_TSTAMP]+
                             icmpstat.icps_inhist[ICMP_TSTAMPREPLY]+
                             icmpstat.icps_inhist[ICMP_IREQ]+
                             icmpstat.icps_inhist[ICMP_IREQREPLY]+
                             icmpstat.icps_inhist[ICMP_MASKREQ]+
                             icmpstat.icps_inhist[ICMP_MASKREPLY]));
	printf("Bad:%ld\n",  (icmpstat.icps_badcode+
                             icmpstat.icps_tooshort+
                             icmpstat.icps_checksum+
                             icmpstat.icps_badlen+
                             icmpstat.icps_bmcastecho));

	printf("\tTx ");
	printf("ECHO:%ld ", icmpstat.icps_outhist[ICMP_ECHO]);
	printf("ECHO REPLY:%ld ", icmpstat.icps_outhist[ICMP_ECHOREPLY]);
	printf("UNREACH:%ld ", icmpstat.icps_outhist[ICMP_UNREACH]);
	printf("REDIRECT:%ld ", icmpstat.icps_outhist[ICMP_REDIRECT]);
	printf("Other:%ld\n", (icmpstat.icps_inhist[ICMP_SOURCEQUENCH]+                             
                             icmpstat.icps_outhist[ICMP_ROUTERADVERT]+
                             icmpstat.icps_outhist[ICMP_ROUTERSOLICIT]+
                             icmpstat.icps_outhist[ICMP_TIMXCEED]+
                             icmpstat.icps_outhist[ICMP_PARAMPROB]+
                             icmpstat.icps_outhist[ICMP_TSTAMP]+
                             icmpstat.icps_outhist[ICMP_TSTAMPREPLY]+
                             icmpstat.icps_outhist[ICMP_IREQ]+
                             icmpstat.icps_outhist[ICMP_IREQREPLY]+
                             icmpstat.icps_outhist[ICMP_MASKREQ]+
                             icmpstat.icps_outhist[ICMP_MASKREPLY]));

#ifdef CYGPKG_NET_INET6
        printf("ICMPv6:\n");
	printf("\tRx ");
	printf("ECHO:%lld ", icmp6stat.icp6s_inhist[ICMP_ECHO]);
	printf("ECHO REPLY:%lld ", icmp6stat.icp6s_inhist[ICMP_ECHOREPLY]);
	printf("UNREACH:%lld ", icmp6stat.icp6s_inhist[ICMP_UNREACH]);
	printf("REDIRECT:%lld ", icmp6stat.icp6s_inhist[ICMP_REDIRECT]);
	printf("Other:%lld ",(icmp6stat.icp6s_inhist[ICMP_SOURCEQUENCH]+
                             icmp6stat.icp6s_inhist[ICMP_ROUTERADVERT]+
                             icmp6stat.icp6s_inhist[ICMP_ROUTERSOLICIT]+
                             icmp6stat.icp6s_inhist[ICMP_TIMXCEED]+
                             icmp6stat.icp6s_inhist[ICMP_PARAMPROB]+
                             icmp6stat.icp6s_inhist[ICMP_TSTAMP]+
                             icmp6stat.icp6s_inhist[ICMP_TSTAMPREPLY]+
                             icmp6stat.icp6s_inhist[ICMP_IREQ]+
                             icmp6stat.icp6s_inhist[ICMP_IREQREPLY]+
                             icmp6stat.icp6s_inhist[ICMP_MASKREQ]+
                             icmp6stat.icp6s_inhist[ICMP_MASKREPLY]));
	printf("Bad:%lld\n", (icmp6stat.icp6s_badcode+
                             icmp6stat.icp6s_tooshort+
                             icmp6stat.icp6s_checksum+
                             icmp6stat.icp6s_badlen));

	printf("\tTx ");
	printf("ECHO:%lld ", icmp6stat.icp6s_outhist[ICMP_ECHO]);
	printf("ECHO REPLY:%lld ", icmp6stat.icp6s_outhist[ICMP_ECHOREPLY]);
	printf("UNREACH:%lld ", icmp6stat.icp6s_outhist[ICMP_UNREACH]);
	printf("REDIRECT:%lld ", icmp6stat.icp6s_outhist[ICMP_REDIRECT]);
	printf("Other:%lld\n", (icmp6stat.icp6s_inhist[ICMP_SOURCEQUENCH]+                             
                             icmp6stat.icp6s_outhist[ICMP_ROUTERADVERT]+
                             icmp6stat.icp6s_outhist[ICMP_ROUTERSOLICIT]+
                             icmp6stat.icp6s_outhist[ICMP_TIMXCEED]+
                             icmp6stat.icp6s_outhist[ICMP_PARAMPROB]+
                             icmp6stat.icp6s_outhist[ICMP_TSTAMP]+
                             icmp6stat.icp6s_outhist[ICMP_TSTAMPREPLY]+
                             icmp6stat.icp6s_outhist[ICMP_IREQ]+
                             icmp6stat.icp6s_outhist[ICMP_IREQREPLY]+
                             icmp6stat.icp6s_outhist[ICMP_MASKREQ]+
                             icmp6stat.icp6s_outhist[ICMP_MASKREPLY]));
#endif

	printf("UDP:\n");
	printf("\tRx ");
	printf("Total:%ld ", udpstat.udps_ipackets);
	printf("Bad:%ld\n",  (udpstat.udps_hdrops+
                             udpstat.udps_badsum+
                             udpstat.udps_badlen+
                             udpstat.udps_noport+
                             udpstat.udps_noportbcast+
                             udpstat.udps_fullsock));
	printf("\tTx ");
	printf("Total:%ld\n", udpstat.udps_opackets);

        printf("TCP:\n");
	printf("\tRx ");
#if defined(CYGPKG_NET_FREEBSD_STACK)
	printf("Packets:%ld ", tcpstat.tcps_rcvtotal);
	printf("Data Packets:%ld ", tcpstat.tcps_rcvpack);
	printf("Bytes:%ld\n", tcpstat.tcps_rcvbyte);
#else
	printf("Packets:%d ", tcpstat.tcps_rcvtotal);
	printf("Data Packets:%d ", tcpstat.tcps_rcvpack);
	printf("Bytes:%lld\n", tcpstat.tcps_rcvbyte);
#endif                    
	printf("\tTx ");
#if defined(CYGPKG_NET_FREEBSD_STACK)
	printf("Packets:%ld ", tcpstat.tcps_sndtotal);
	printf("Data Packets:%ld ", tcpstat.tcps_sndpack);
	printf("Bytes:%ld\n", tcpstat.tcps_sndbyte);
#else
	printf("Packets:%d ", tcpstat.tcps_sndtotal);
	printf("Data Packets:%d ", tcpstat.tcps_sndpack);
	printf("Bytes:%lld\n", tcpstat.tcps_sndbyte);
#endif
	printf("\tConnections:\n");
#if defined(CYGPKG_NET_FREEBSD_STACK)
	printf("\tInitiated:%ld ", tcpstat.tcps_connattempt);
	printf("Accepted:%ld ", tcpstat.tcps_accepts);
	printf("Established:%ld ", tcpstat.tcps_connects);
	printf("Closed:%ld\n", tcpstat.tcps_closed);
#else
	printf("\tInitiated:%d ", tcpstat.tcps_connattempt);
	printf("Accepted:%d ", tcpstat.tcps_accepts);
	printf("Established:%d ", tcpstat.tcps_connects);
	printf("Closed:%d\n", tcpstat.tcps_closed);
#endif

	printf("Mbufs:\n");
	printf("\tSummary:\n");
	printf("\tMbufs:%ld ", mbstat.m_mbufs);
	printf("Clusters:%ld ", mbstat.m_clusters);
	printf("Free Clusters:%ld ", mbstat.m_clfree);
	printf("Drops:%ld ", mbstat.m_drops);
	printf("Waits:%ld ", mbstat.m_wait);
	printf("Drains:%ld ", mbstat.m_drain);
#if defined(CYGPKG_NET_FREEBSD_STACK)
	printf("Copy Fails:%ld ", mbstat.m_mcfail);
	printf("Pullup Fails:%ld", mbstat.m_mpfail);
#endif                    

	printf("\n\tTypes:\n");

#if defined(CYGPKG_NET_FREEBSD_STACK)
	mtypes = mbtypes;
	printf("\tFREE:%ld ", mtypes[MT_FREE]);
	printf("DATA:%ld ", mtypes[MT_DATA]);
	printf("HEADER:%ld ", mtypes[MT_HEADER]);
	printf("SONAME:%ld ", mtypes[MT_SONAME]);
	printf("FTABLE:%ld\n", mtypes[MT_FTABLE]);
#else
	mtypes = mbstat.m_mtypes;
	printf("\tFREE:%d ", mtypes[MT_FREE]);
	printf("DATA:%d ", mtypes[MT_DATA]);
	printf("HEADER:%d ", mtypes[MT_HEADER]);
	printf("SOCKET:%d ", mtypes[MT_SOCKET]);
	printf("PCB:%d ", mtypes[MT_PCB]);
	printf("RTABLE:%d ", mtypes[MT_RTABLE]);
	printf("HTABLE:%d ", mtypes[MT_HTABLE]);
	printf("ATABLE:%d ", mtypes[MT_ATABLE]);
	printf("SONAME:%d ", mtypes[MT_SONAME]);
	printf("SOOPTS:%d ", mtypes[MT_SOOPTS]);
	printf("FTABLE:%d\n", mtypes[MT_FTABLE]);
#endif
	/* Ignore the rest for now... */
                    
	freeifaddrs(iflist);
}
