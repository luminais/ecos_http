#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#ifdef CYGPKG_NET_FREEBSD_STACK
#include <net/if_var.h>
#endif
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <net/if.h>
#include <net/if_dl.h>

int is_interface_up(const char *ifname)
{
	int s;
	struct ifreq ifr;
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return 0;
	}
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(s, SIOCGIFFLAGS, &ifr)) {
		perror("SIOCGIFFLAGS");
		close(s);
		return 0;
	}
	close(s);
	//printf("ifr.ifr_flags=%x\n", ifr.ifr_flags);
	return ((ifr.ifr_flags&IFF_UP) ? 1 : 0);
}

int interface_up(const char *intf)
{
	int s;
	struct ifreq ifr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}

	strcpy(ifr.ifr_name, intf);
	ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
	if (ioctl(s, SIOCSIFFLAGS, &ifr)) {
		perror("SIOCSIFFLAGS");
	}
	close(s);
	return 0;
}

int interface_down(const char *intf)
{
	int s;
	struct ifreq ifr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}
	strcpy(ifr.ifr_name, intf);
	if (ioctl(s, SIOCGIFFLAGS, &ifr)) {
		perror("SIOCGIFFLAGS");
		close(s);
		return -1;
	}

	strcpy(ifr.ifr_name, intf);
	if(ifr.ifr_flags & IFF_UP)
	{
		//diag_printf("%s %d [%d]\n", ifr.ifr_name, ifr.ifr_addr.sa_family, AF_LINK);
		ifr.ifr_flags &= (~IFF_UP);
		if (ioctl(s, SIOCSIFFLAGS, &ifr)) {
			perror("SIOCSIFFLAGS");
			close(s);
			return -1;
		}
	}
	close(s);
	return 0;
}

int interface_config(const char *intf, char *addr, char *netmask)
{
	struct ifreq ifr;
	struct in_aliasreq addreq;
	int s;
		
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}
	strcpy(ifr.ifr_name, intf);
	if (ioctl(s, SIOCDIFADDR, &ifr) < 0) {
		if (errno == EADDRNOTAVAIL) {
			/* means no previous address for interface */
		} else {
			perror("SIOCDIFADDR");
			close(s);
			return -1;
		}
	}

	strcpy(addreq.ifra_name, intf);
	addreq.ifra_addr.sin_family = AF_INET;
	addreq.ifra_addr.sin_len = sizeof(struct sockaddr_in);
	if (inet_aton(addr, &addreq.ifra_addr.sin_addr) == 0) {
		printf("bad value: %s", addr);
		close(s);
		return -1;
	}
	addreq.ifra_mask.sin_family = AF_INET;
	addreq.ifra_mask.sin_len = sizeof(struct sockaddr_in);
	if (inet_aton(netmask, &addreq.ifra_mask.sin_addr) == 0) {
		printf("bad value: %s", netmask);
		close(s);
		return -1;
	}
	addreq.ifra_broadaddr.sin_family = AF_INET;
	addreq.ifra_broadaddr.sin_len = sizeof(struct sockaddr_in);
	addreq.ifra_broadaddr.sin_addr.s_addr = (addreq.ifra_addr.sin_addr.s_addr & addreq.ifra_mask.sin_addr.s_addr)
						 | (~addreq.ifra_mask.sin_addr.s_addr);
	/*printf("###%x %x %x###\n", addreq.ifra_addr.sin_addr.s_addr,
				   addreq.ifra_mask.sin_addr.s_addr,
				   addreq.ifra_broadaddr.sin_addr.s_addr);*/
	if (ioctl(s, SIOCAIFADDR, &addreq) < 0)
		perror("SIOCAIFADDR");
	close(s);
	return 0;
}

int shutdown_all_interfaces(void)
{
	char *inbuf = NULL;
	struct ifconf ifc;
	struct ifreq *ifrp, ifreq;
	int len = 8192, i;
	int s;
	extern void force_stop_wlan_hw(void);
		
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}
	
	while (1) {
		ifc.ifc_len = len;
		ifc.ifc_buf = inbuf = realloc(inbuf, len);
		if (inbuf == NULL) {
			close(s);
			perror("malloc");
			return -1;
		}
		if (ioctl(s, SIOCGIFCONF, &ifc) < 0) {
			close(s);
			perror("ioctl(SIOCGIFCONF)");
			return -1;
		}
		if (ifc.ifc_len + sizeof(struct ifreq) < len)
			break;
		len *= 2;
	}
	ifrp = ifc.ifc_req;
	ifreq.ifr_name[0] = '\0';
	for (i = 0; i < ifc.ifc_len; ) {
		//diag_printf("Search next interface name ...\n");
		ifrp = (struct ifreq *)((caddr_t)ifc.ifc_req + i);
		i += sizeof(ifrp->ifr_name) +
		    (ifrp->ifr_addr.sa_len > sizeof(struct sockaddr) ?
		    ifrp->ifr_addr.sa_len : sizeof(struct sockaddr));
		if (ifrp->ifr_addr.sa_family != AF_LINK)
			continue;
		if ((strcmp(ifrp->ifr_name, "lo0")==0) ||
		    (strcmp(ifrp->ifr_name, "bridge0")==0))
		    continue;
		diag_printf("%s %d [%d]\n", ifrp->ifr_name, ifrp->ifr_addr.sa_family, AF_LINK);
		interface_down(ifrp->ifr_name);
	}
	close(s);
	free(ifc.ifc_buf);
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
	force_stop_wlan_hw();
#endif
	return (0);
}

// ------------------------------------------------------------------------

static int route_add_s( 
						int action,
						struct sockaddr_in *target,
                        struct sockaddr_in *gw,
                        struct sockaddr_in *mask,
                        int metric,
                        char *dev)
{
    struct ecos_rtentry route;
   // struct sockaddr_in mask;

    int s;
	#if 0
    memcpy( &mask, gw, sizeof(*gw) );	
    maskbits--;
    mask.sin_addr.s_addr = htonl( (0xfffffffful ^ ((0x80000000ul >> maskbits)-1)) );
	#endif
    memset( &route, 0, sizeof(route) );

    memcpy( &route.rt_dst    ,  target, sizeof(*target) );
    memcpy( &route.rt_gateway,  gw    , sizeof(*gw) );
    memcpy( &route.rt_genmask,  mask  , sizeof(*mask) ); 

    if((target->sin_addr.s_addr & mask->sin_addr.s_addr) == target->sin_addr.s_addr)
    		route.rt_flags = RTF_UP|RTF_GATEWAY;
    else
		route.rt_flags = RTF_UP|RTF_HOST;
    route.rt_metric = metric;

    route.rt_dev = dev;
/*
    diag_printf("INFO:<Route - dst: %s",
                inet_ntoa(((struct sockaddr_in *)&route.rt_dst)->sin_addr));
    diag_printf(", mask: %s",
                inet_ntoa(((struct sockaddr_in *)&route.rt_genmask)->sin_addr));
    diag_printf(", gateway: %s>\n",
                inet_ntoa(((struct sockaddr_in *)&route.rt_gateway)->sin_addr));
*/
    s = socket( AF_INET, SOCK_DGRAM, 0 );
    if (s < 0) {
        perror( "socket" );
        return false;
    }
	if (ioctl(s, action, &route)) {
			if(action==SIOCADDRT)
        		perror("SIOCADDRT");
			else if(action==SIOCDELRT)
				perror("SIOCDELRT");
        	close(s);
        	return false;
    }
 //   diag_printf( "PASS:<Route added OK>\n" );
    close(s);
    return true;
}

static void
usage(cp)
	const char *cp;
{
	if (cp)
		warnx("bad keyword: %s", cp);
	diag_printf("usage: route add/delete -net/-host [] -gateway [] -netmask [] -metric [] -interface []\n");
}

#define K_METRIC	44
#define	K_GATEWAY	9
#define	K_NETMASK	27
#define	K_NET	26
#define	K_INTERFACE	15
#define 	K_HOST	12

int route_add(int argc, char **argv)
{
	char *target=NULL;
    char *gw=NULL;
   	char *maskbits=NULL;
    int metric=0;
    char *dev=NULL;
	int key;
	int action;
	
    struct sockaddr_in t_s, gw_s,mask_s;
	
	memset( &t_s,  0, sizeof(t_s)  );
    memset( &gw_s, 0, sizeof(gw_s) );
	memset( &mask_s, 0, sizeof(mask_s) );
	
	if(**argv=='a')
		action=SIOCADDRT;
	else if(**argv=='d')
		action=SIOCDELRT;
	else{
		usage((char *)NULL);
		return -1;
	}		
		
	while (--argc > 0) {
		if (**(++argv)== '-') {
			switch (key = keyword(1 + *argv)) {
				case K_GATEWAY:
					if (!--argc){
						usage((char *)NULL);
						return -1;
					}
					gw=*++argv;
					break;					
				case K_HOST:
					if (!--argc){
						usage((char *)NULL);
						return -1;
					}
					target=*++argv;
					break;
				case K_NETMASK:
					if (!--argc){
						usage((char *)NULL);
						return -1;
					}
					maskbits=*++argv;
					break;
				case K_NET:	
					if (!--argc){
						usage((char *)NULL);
						return -1;
					}
					target = *++argv;
					break;
				case K_INTERFACE:
					if (!--argc){
						usage((char *)NULL);
						return -1;
					}
					dev = *++argv;
					break;
				case K_METRIC:
					if (!--argc){
						usage((char *)NULL);
						return -1;
					}
					metric=atoi(*++argv);
					break;
				default:
					usage((char *)NULL);
					return -1;					
			}
		}
		else{
			usage((char *)NULL);
			return -1;	
		}
	}
   	if(target)
		t_s.sin_addr.s_addr = inet_addr(target);
	t_s.sin_len = sizeof(t_s);
	t_s.sin_family = AF_INET;
	if(gw)
		gw_s.sin_addr.s_addr= inet_addr(gw);
	gw_s.sin_len = sizeof(gw_s);
	gw_s.sin_family = AF_INET;
	if(maskbits)
		mask_s.sin_addr.s_addr= inet_addr(maskbits);
	mask_s.sin_len = sizeof(mask_s);
	mask_s.sin_family = AF_INET;
	
#if 0
	diag_printf("net:%x\n",t_s.sin_addr.s_addr);
	diag_printf("gw:%x\n",gw_s.sin_addr.s_addr);
	diag_printf("netmask:%x\n",mask_s.sin_addr.s_addr);
	diag_printf("metric:%d\n",metric);
	if(dev)
		diag_printf("dev:%s\n",dev);
#endif
	
    return route_add_s( action,&t_s, &gw_s, &mask_s, metric,dev);
}

// ------------------------------------------------------------------------
