/*
 * interface ioctl library routines
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: iflib.c,v 1.3 2010-06-30 09:36:29 Exp $
 */
#include <iflib.h>

/*
 * Interface ioctls.
 */
int
iflib_ioctl(char *ifname, int cmd, void *data)
{
	int s;
	struct ifreq ifr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return -1;

	strcpy(ifr.ifr_name, ifname);

	switch (cmd) {
	case SIOCGIFFLAGS:
	case SIOCGIFMETRIC:
	case SIOCGIFMTU:
	case SIOCGIFADDR:
	case SIOCGIFNETMASK:
		break;

	case SIOCSIFFLAGS:
		ifr.ifr_flags = (short)(*(int *)data);
		break;

	case SIOCSIFMETRIC:
	case SIOCSIFMTU:
		ifr.ifr_metric = *(int *)data;
		break;

	default:
		ifr.ifr_data = (char *)data;
		break;
	}

	/* Do ioctl */
	if (ioctl(s, cmd, &ifr)) {
		close(s);
		return -1;
	}

	/* copy value to get calls */
	switch (cmd) {
	case SIOCGIFFLAGS:
		*(int *)data = (int)ifr.ifr_flags;
		break;

	case SIOCGIFMETRIC:
	case SIOCGIFMTU:
		*(int *)data = ifr.ifr_metric;
		break;

	case SIOCGIFADDR:
	case SIOCGIFNETMASK:
		bcopy(&ifr.ifr_addr, data, sizeof(struct sockaddr));
		break;

	default:
		break;
	}

	/* Close socket and return */
	close(s);
	return 0;
}

/* UP an interface */
int
iflib_ifup(char *ifname)
{
	int flags;

	if (iflib_ioctl(ifname, SIOCGIFFLAGS, &flags) != 0)
		return -1;

	flags |= IFF_UP;
	return iflib_ioctl(ifname, SIOCSIFFLAGS, &flags);
}

/* Down an interface */
int
iflib_ifdown(char *ifname)
{
	int flags;

	if (iflib_ioctl(ifname, SIOCGIFFLAGS, &flags) != 0)
		return -1;

	flags &= ~IFF_UP;
	return iflib_ioctl(ifname, SIOCSIFFLAGS, &flags);
}

/* Remove all the ip addressses of this interface */
int
iflib_flushifip(char *ifname)
{
	int s;
	struct ifreq ifr;

	s = socket(AF_INET, SOCK_RAW, 0);
	if (s < 0)
		return -1;

	/*
	 * Deleting the interface address is required
	 * to correctly scrub the routing table based
	 * on the current netmask.
	 */
	bzero((char *)&ifr, sizeof(struct ifreq));

	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_len = sizeof(struct sockaddr_in);
	ifr.ifr_addr.sa_family =  AF_INET;

	while (ioctl(s, SIOCGIFADDR, &ifr) == 0) {
		if (ioctl(s, SIOCDIFADDR, &ifr) != 0)
			break;
	}

	close(s);
	return 0;
}

/*
 * Get interface mac
 */
int
iflib_getifhwaddr(char *ifname, unsigned char ifmac[6])
{
	int s;
	struct ifreq ifr;

	/* Check interface address */
	if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFHWADDR, &ifr) != 0 ||
	    memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", 6) == 0) {
		close(s);
		return -1;
	}

	/* Retrieve the ifmac */
	memcpy(ifmac, ifr.ifr_hwaddr.sa_data, 6);

	close(s);
	return 0;
}

/*
 * Set interface mac
 */
int
iflib_setifhwaddr(char *ifname, unsigned char ifmac[6])
{
	int s;
	struct ifreq ifr;

	/* Check interface address */
	if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	memcpy(ifr.ifr_hwaddr.sa_data, ifmac, 6);
	ifr.ifr_hwaddr.sa_family = AF_LINK;

	ioctl(s, SIOCSIFHWADDR, &ifr);

	close(s);
	return 0;
}

#ifdef BCM
/*
 * Set interface ipaddr
 */
int
iflib_getifaddr(char *ifname, struct in_addr *ipaddr, struct in_addr *netmask)
{
	int s;
	struct ifreq ifr;
	int error = -1;

	s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (s < 0)
		return -1;

	/* Retrieve settings */
	memset(&ifr, 0, sizeof(struct ifreq));

	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, ifname);

	/*
	 * Upper layer want to test the interface
	 * status by this ioctl, call anyway.
	 */
	if (ioctl(s, SIOCGIFADDR, &ifr) != 0)
		goto quit;

	/* get ip */
	if (ipaddr)
		*ipaddr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

	/* get mask */
	if (netmask) {
		if (ioctl(s, SIOCGIFNETMASK, &ifr) != 0)
			goto quit;

		*netmask = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	}

	/* Return with no error */
	error = 0;
quit:
	if (error) {
		if (ipaddr)
			ipaddr->s_addr = 0;
		if (netmask)
			netmask->s_addr = 0;
	}

	close(s);
	return error;
}
#else
#ifdef REALTEK
int
iflib_getifaddr(char *ifname, struct in_addr *ipaddr, struct in_addr *netmask)
{
	int s;
	struct ifreq ifr;
	int error = -1;

	s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (s < 0)
		return -1;

	/* Retrieve settings */
	memset(&ifr, 0, sizeof(struct ifreq));

	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, ifname);

	/*
	 * Upper layer want to test the interface
	 * status by this ioctl, call anyway.
	 */
	if (ioctl(s, SIOCGIFADDR, &ifr) != 0)
		goto quit;

	if(ipaddr || netmask)
	{
		/* llm add, realtek获取到空的接口的地址时，删掉该接口地址 */
		while(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr == 0)
		{
			if (ioctl(s, SIOCDIFADDR, &ifr) != 0)
				break;
			if(ioctl(s, SIOCGIFADDR, &ifr) != 0)
				break;
		}
	}
	/* get ip */
	if (ipaddr)
	{
		*ipaddr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	}
	/* get mask */
	if (netmask) {
		if (ioctl(s, SIOCGIFNETMASK, &ifr) != 0)
			goto quit;

		*netmask = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	}

	/* Return with no error */
	error = 0;
quit:
	if (error) {
		if (ipaddr)
			ipaddr->s_addr = 0;
		if (netmask)
			netmask->s_addr = 0;
	}

	close(s);
	return error;
}

#endif
#endif
/*
 * Set interface ipaddr
 */
int
iflib_setifaddr(char *ifname, struct in_addr ipaddr, struct in_addr netmask)
{
	int s;
	struct ifreq ifr;
	struct sockaddr_in *sin;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, ifname);

	/* Set netmask */
	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	sin->sin_len = sizeof(*sin);
	sin->sin_family = AF_INET;
	sin->sin_addr = netmask;
	ioctl(s, SIOCSIFNETMASK, &ifr);

	/* Set address */
	sin->sin_addr = ipaddr;
	ioctl(s, SIOCSIFADDR, &ifr);

	close(s);
	return 0;
}

/*
 * Set alias ipaddr
 */
int
iflib_setaliasaddr(char *ifname, struct in_addr ipaddr, struct in_addr netmask)
{
	int s;
	int rc = 0;
	struct ifaliasreq ifa;
	struct sockaddr_in *sin;

	s = socket(AF_INET, SOCK_RAW, 0);
	if (s < 0)
		return -1;

	memset(&ifa, 0, sizeof(ifa));
	strcpy(ifa.ifra_name, ifname);

	sin = (struct sockaddr_in *)&ifa.ifra_addr;
	sin->sin_len = sizeof(*sin);
	sin->sin_family = AF_INET;
	sin->sin_addr = ipaddr;

	sin = (struct sockaddr_in *)&ifa.ifra_mask;
	sin->sin_len = sizeof(*sin);
	sin->sin_family = AF_INET;
	sin->sin_addr = netmask;

	if (ioctl(s, SIOCAIFADDR, &ifa) != 0) {
		if (errno != EEXIST)
			rc = -1;
	}

	close(s);
	return rc;
}

//roy+++
//moved from ralink sdk
//------------------------------------------------------------------------------
// FUNCTION
//		void buid_ip_set(struct ip_set *setp,
//                 const char *if_name,
//                 unsigned int addrs_ip,
//                 unsigned int addrs_netmask,
//                 unsigned int addrs_broadcast,
//                 unsigned int addrs_gateway,
//                 unsigned int addrs_server,
//                 unsigned char	mode,
//                 unsigned char	*data	)
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
void buid_ip_set(struct ip_set *setp,
                 const char *if_name,
                 unsigned int addrs_ip,
                 unsigned int addrs_netmask,
                 unsigned int addrs_broadcast,
                 unsigned int addrs_gateway,
                 unsigned int addrs_server,
                 unsigned int mtu,
                 unsigned char	mode,
                  const char *domain,
                 unsigned char	*data	)
{
	if(if_name ==  NULL)
		return;
	memset(setp, 0, sizeof(struct ip_set)); // clear
	strcpy(setp->ifname, if_name);
	setp->ip = addrs_ip;
	if(addrs_netmask || (setp->ip == 0))
    	setp->netmask = addrs_netmask;
    else
    {	// if no netmask, set class netmask 
    	if (IN_CLASSA(setp->ip))
			setp->netmask = IN_CLASSA_HOST;
		else if (IN_CLASSB(setp->ip))
			setp->netmask = IN_CLASSB_HOST;
		else
			setp->netmask = IN_CLASSC_HOST;
    }
    
    setp->server_ip = addrs_server;
   
   	setp->gw_ip = addrs_gateway;
    if(addrs_broadcast)
    	setp->broad_ip = addrs_broadcast;
   	else 
		setp->broad_ip=((setp->ip)&(setp->netmask))|~(setp->netmask);
		
    setp->mtu = mtu;
    
    setp->mode = mode;
    if(domain)
    	strcpy(setp->domain, domain);
	setp->data = data;
}

