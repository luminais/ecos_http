/*
 * Functions to handle leases of DHCP server module.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpd_lease.c,v 1.7 2010-11-10 05:02:58 Exp $
 */

#include <dhcpd.h>
#include <stdio.h>
#include <string.h>


char *inet_mactoa(unsigned char *mac){
	static	char tmp[20];
    strcpy(tmp,"00:00:00:00:00:00");
	sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	return tmp;
}

void
dhcpd_lease_free(DHCPD_IFC *ifp, DHCPD_LEASE *lease)
{
	int index = ntohl(lease->ipaddr.s_addr) & 0xff;
	struct lease_head *inuse = &ifp->container.inuse;

	/* Free hostname */
	if (lease->hostname) {
		free(lease->hostname);
		lease->hostname = NULL;
	}

	/* Probe could add a lease with the same ipaddr as
	 * ours.  So, confirm it is ours before updating
	 */
	if (ifp->container.lineup[index] == lease) {
		/* lineup set to 0 */
		ifp->container.lineup[index] = 0;
	}

	/* Make sure remove from inused queue */
	if (!CIRCLEQ_EMPTY(inuse))
		CIRCLEQ_REMOVE(inuse, lease, lease_queue);

	CIRCLEQ_INSERT_HEAD(&ifp->container.free, lease, lease_queue);
	return;
}

void
dhcpd_lease_init(DHCPD_IFC *ifp)
{
	DHCPD_LEASE *lease;
	DHCPD_LEASE *end;
	int i;
	unsigned int num = 0;
	struct lease_pool *pool = ifp->container.pool;

	CIRCLEQ_INIT(&ifp->container.inuse);
	CIRCLEQ_INIT(&ifp->container.free);

	/* Allocate requested lease pool */
	for (i = 0; i < ifp->container.pool_num; i++, pool++) {
		num += ntohl(pool->end.s_addr) - ntohl(pool->start.s_addr) + 1;
	}
	#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	num+=DHCPD_STATIC_LEASE_NU+1;
	#endif
	ifp->container.lease_array = calloc(num, sizeof(*lease));
	if (ifp->container.lease_array == NULL) {
		DHCPD_DBG("can not allocate %d pool entries\n", num);
		return;
	}

	memset(ifp->container.lineup, 0, sizeof(ifp->container.lineup));

	lease = ifp->container.lease_array;
	end = lease + num;
	for (; lease < end; lease++) {
		dhcpd_lease_free(ifp, lease);
	}
}

void
dhcpd_lease_deinit(DHCPD_IFC *ifp)
{
	struct lease_head *inuse = &ifp->container.inuse;
	DHCPD_LEASE *lease;

	if (ifp->container.lease_array == NULL)
		return;

	while (!CIRCLEQ_EMPTY(inuse)) {
		/* Move from inuse queue */
		lease = CIRCLEQ_FIRST(inuse);

		/* Free to pool */
		dhcpd_lease_free(ifp, lease);
	}

	/* Free pool */
	free(ifp->container.lease_array);
	ifp->container.lease_array = NULL;

	/* Cleanup for safety */
	CIRCLEQ_INIT(&ifp->container.free);
	return;
}

void
dhcpd_lease_expiry_renew(DHCPD_IFC *ifp, DHCPD_LEASE *lease, unsigned int new_expiry)
{
	struct lease_head *inuse = &ifp->container.inuse;
	DHCPD_LEASE *iterate;

	/* Remove from queue */
	CIRCLEQ_REMOVE(inuse, lease, lease_queue);

	/* Set new lease */
	lease->expiry = new_expiry;

	CIRCLEQ_FOREACH_REVERSE(iterate, inuse, lease_queue) {
		if (new_expiry >= iterate->expiry) {
			CIRCLEQ_INSERT_AFTER(inuse, iterate, lease, lease_queue);
			return;
		}
	}

	/* oldest, insert to head */
	CIRCLEQ_INSERT_HEAD(inuse, lease, lease_queue);
}

/* Add a lease into the table, clearing out any old ones */
DHCPD_LEASE *
dhcpd_lease_add
(
	DHCPD_IFC *ifp,
	unsigned char *mac,
	struct in_addr ipaddr,
	unsigned long lease_time,
	unsigned char *hostname,
	int flag
)
{
	DHCPD_LEASE *lease;
	int len;
	int old_flag = 0;
	int index = ntohl(ipaddr.s_addr) & 0xff;
	unsigned long expiry;

	lease = dhcpd_lease_match(ifp, mac, NULL);
	if (lease) {
		/*
		 * If the lease found is a static entry,
		 * we will ignore the flag value input.
		 */
		if (lease->flag == STATICL)
			old_flag = lease->flag;
	}
	else if (!CIRCLEQ_EMPTY(&ifp->container.free)) {
		/* Move from inuse queue */
		lease = CIRCLEQ_FIRST(&ifp->container.free);
		CIRCLEQ_REMOVE(&ifp->container.free, lease, lease_queue);

		/* Argust, Insert temproary */
		CIRCLEQ_INSERT_HEAD(&ifp->container.inuse, lease, lease_queue);
	}
	else {
		struct lease_head *inuse = &ifp->container.inuse;
		DHCPD_LEASE *iterate;

		/* Is there any chance to be here? */
		CIRCLEQ_FOREACH(iterate, inuse, lease_queue) {
			if (iterate->expiry >= time(0)) {
				/* No expired entry */
				break;
			}
			if ((iterate->flag & STATICL) == 0 && iterate->expiry < time(0)) {
				/* The first one is the oldest one */
				lease = iterate;
				break;
			}
		}
	}

	/* clean out any old ones */
	if (lease) {
		/* Update mac and ipaddr */
		memcpy(lease->mac, mac, 6);
		lease->ipaddr = ipaddr;

		/* add hostname */
		if (lease->hostname) {
			free(lease->hostname);
			lease->hostname = 0;
		}
		if (hostname && (len = strlen((char *)hostname)) != 0) {
			/* Allocate memory to hold hostname */
			lease->hostname = malloc(len+1);
			if (lease->hostname)
				memcpy(lease->hostname, hostname, len+1);
		}

		/* Set flag */
		if (old_flag)
			lease->flag = old_flag;
		else
			lease->flag = flag;

		/* Renew lease expiry time */
		if (lease_time)
			expiry = time(0) + lease_time;
		else
			expiry = 0xffffffff;	/* Never expire */

		dhcpd_lease_expiry_renew(ifp, lease, expiry);

		/* Setup index for matching */
		ifp->container.lineup[index] = lease;
	}

	return lease;
}

DHCPD_LEASE *
dhcpd_lease_match(DHCPD_IFC *ifp, unsigned char *mac, struct in_addr *ipaddr)
{
	struct lease_head *inuse = &ifp->container.inuse;
	DHCPD_LEASE *lease;

	/* request type check */
	if (!mac && !ipaddr)
		return NULL;

	CIRCLEQ_FOREACH(lease, inuse, lease_queue) {
		if (mac && ipaddr) {
			if (memcmp(lease->mac, mac, 6) == 0 &&
			    lease->ipaddr.s_addr == ipaddr->s_addr) {
				/* best match */
				return lease;
			}
		}
		else if (mac && memcmp(mac, lease->mac, 6) == 0) {
			/* mac address match */
			return lease;
		}
		else if (ipaddr && ipaddr->s_addr == lease->ipaddr.s_addr) {
			/* IP address match */
			return lease;
		}
	}

	return NULL;
}

/* Find an available address */
struct in_addr *
dhcpd_addr_alloc(DHCPD_IFC *ifp, struct in_addr *addr)
{
	struct lease_head *inuse = &ifp->container.inuse;
	DHCPD_LEASE *iterate;
	int i, j;
	int num = ifp->container.pool_num;
	struct lease_pool *pool = ifp->container.pool;

	/* Search in configure range at first */
	for (i = 0; i < num; i++, pool++) {
		for (j = ntohl(pool->start.s_addr);
		     j <= ntohl(pool->end.s_addr);
		     j++) {
			/* Skip interface ip */
			if (j == ntohl(ifp->ipaddr.s_addr))
				continue;

			/* Get lease with lineup */
			addr->s_addr = htonl(j);
			iterate = ifp->container.lineup[j & 0xff];
			if (iterate == NULL) {
				/* This one is not in the inuse list */
				if (dhcpd_probe(ifp, NULL, *addr, ifp->chaddr) == 0) {
					/* Okay, no one use it */
					return addr;
				}
			}
		}
	}

reiterate:
	/* No free one, pick the oldest expired one in table */
	CIRCLEQ_FOREACH(iterate, inuse, lease_queue) {
		if (iterate->expiry >= time(0)) {
			/* No expired entry */
			break;
		}
		if ((iterate->flag & STATICL) == 0 && iterate->expiry < time(0)) {
			/* Is this ip occupied by others */
			if (dhcpd_probe(ifp, iterate, iterate->ipaddr, ifp->chaddr) == 0) {
				/* Safe to use */
				addr->s_addr = iterate->ipaddr.s_addr;
				return addr;
			}

			/* Circular queue changed, starting from head again */
			goto reiterate;
		}
	}

	/* Can't find in the inuse list */
	return NULL;
}

/*
 * Check if an ip used by someone other then the given ipaddr and mac.
 * If no one use this ip or it is exact the given one, return 0.
 * Otherwise, save the reserved mac address and return -1;
 */
int
dhcpd_probe(DHCPD_IFC *ifp, DHCPD_LEASE *lease, struct in_addr ipaddr, unsigned char *chaddr)
{
	unsigned char testchaddr[6] = {0};
	unsigned char zerochaddr[6] = {0};

	int icmp_ping(unsigned int dst, int seq, int timeout);

	/*
	 * In order to send the ARP request,
	 * delete ARP entry before pinging
	 */
	dhcpd_osl_arpdel(ipaddr);

	/* Ping this host */
	icmp_ping(ntohl(ipaddr.s_addr), 0, 1000);

	/* Read the ARP again */
	dhcpd_osl_arpget(ipaddr, testchaddr);

	/* Check if someone else used this address */
	if (memcmp(testchaddr, zerochaddr, 6) != 0 &&
	    memcmp(testchaddr, chaddr, 6) != 0) {
		/* Free the lease anyway */
		if (lease)
			dhcpd_lease_free(ifp, lease);

		/* Reserved this entry until conflict time passed */
		dhcpd_lease_add(ifp, testchaddr, ipaddr,
			ifp->container.reserve_time, NULL, RESERVED);
		return -1;
	}

	/* Good.  No one use it, we can free the orginal owner now,
	 * if it is not our target
	 */
	if (lease) {
		if (memcmp(lease->mac, chaddr, 6) != 0)
			dhcpd_lease_free(ifp, lease);
	}

	/* Delete this ARP entry again */
	dhcpd_osl_arpdel(ipaddr);
	return 0;
}

#ifdef __CONFIG_AUTO_CONN__
int
dhcpd_probe_extend(DHCPD_IFC *ifp, DHCPD_LEASE *lease, struct in_addr ipaddr, unsigned char *chaddr)
{
	unsigned char testchaddr[6] = {0};
	unsigned char zerochaddr[6] = {0};

	int icmp_ping(unsigned int dst, int seq, int timeout);

	/*
	 * In order to send the ARP request,
	 * delete ARP entry before pinging
	 */
	dhcpd_osl_arpdel(ipaddr);

	/* Ping this host */
	icmp_ping(ntohl(ipaddr.s_addr), 0, 1000);

	/* Read the ARP again */
	dhcpd_osl_arpget(ipaddr, testchaddr);

	/* Check if someone else used this address */
	if (memcmp(testchaddr, zerochaddr, 6) != 0 &&
	    memcmp(testchaddr, chaddr, 6) != 0) {
		/* Free the lease anyway */
		if (lease)
			dhcpd_lease_free(ifp, lease);

		if(dhcpd_lease_match(ifp, testchaddr, NULL) != NULL)
			return -1;

		/* Reserved this entry until conflict time passed */
		dhcpd_lease_add(ifp, testchaddr, ipaddr,
			ifp->container.reserve_time, NULL, RESERVED);
		return -1;
	}

	/* Good.  No one use it, we can free the orginal owner now,
	 * if it is not our target
	 */
	if (lease) {
		if (memcmp(lease->mac, chaddr, 6) != 0)
			dhcpd_lease_free(ifp, lease);
	}

	/* Delete this ARP entry again */
	dhcpd_osl_arpdel(ipaddr);
	return 0;
}

#endif

/* Dump lease with flat format */
struct lease_t *
dhcpd_lease_copy(char *ifname)
{
	DHCPD_IFC *ifp;
	struct lease_head *inuse;
	DHCPD_LEASE *iterate;
	int count;
	struct lease_t *dump, *lease;

	/* Locate interface */
	for (ifp = dhcpd_iflist; ifp; ifp = ifp->next) {
		if (strcmp(ifp->ifname, ifname) == 0)
			break;
	}
	if (ifp == NULL)
		return 0;

	/* Estmate lease size */
	count = 0;
	inuse = &ifp->container.inuse;
	CIRCLEQ_FOREACH(iterate, inuse, lease_queue)
		count++;

	if (count == 0)
		return NULL;

	/* Allocate memory to store */
	dump = (struct lease_t *)calloc(count, sizeof(struct lease_t));
	if (dump == NULL)
		return 0;

	/* Copy data */
	lease = dump;
	CIRCLEQ_FOREACH(iterate, inuse, lease_queue) {
		lease->last = 0;
		lease->flag = iterate->flag;
		memcpy(lease->mac, iterate->mac, 6);
		lease->ipaddr = iterate->ipaddr;
		lease->expiry = iterate->expiry;		
		if (iterate->hostname) {
			strncpy((char *)lease->hostname, (char *)iterate->hostname,
				sizeof(lease->hostname)-1);
		}

		/* Advance to next */
		lease++;
	}

	(lease-1)->last = 1;
	return dump;
}

/* pxy w316r_vsl01 2013.2.21 */
/*
struct detec{
        char inuse;
        char auth;
        unsigned char mac[6];
        in_addr_t ip;
		char hostname[30];
		int flag;
};
extern	struct detec det[255];
*/


void 
copyHostname2TendaArp(char *ifname)
{
	DHCPD_IFC *ifp;
	struct lease_head *inuse;
	DHCPD_LEASE *iterate;

    struct machost_t *machost, *new_node;

	machost = new_node = NULL;

	/* Check thread status */
	if (oslib_getpidbyname("DHCP_server") == 0)
		return;
	
	/* Locate interface */
	for (ifp = dhcpd_iflist; ifp; ifp = ifp->next) {
		if (strcmp(ifp->ifname, ifname) == 0)
			break;
	}
	if (ifp == NULL)
		return;

	/* Copy data */
	inuse = &ifp->container.inuse;

	CIRCLEQ_FOREACH(iterate, inuse, lease_queue) {
		if(iterate->hostname){
			new_node = (struct machost_t *)malloc(sizeof(struct machost_t));
			if(NULL == new_node)
			{
				diag_printf("func=%s; line=%d; msg: No buf!\n", __func__, __LINE__);
				goto out;
			}
			
			// get value
			memset((char *)new_node, 0, sizeof(struct machost_t));
			new_node->ip = iterate->ipaddr.s_addr;
			memcpy(new_node->mac, iterate->mac, 6);
			memcpy(new_node->hostname, (char *)iterate->hostname, 63);
			new_node->next = NULL;

			// chain the list
			new_node->next = machost;
			machost = new_node;
		}
	}
	
	tenda_arp_set_hostname(machost);

out:	
	// free the list.
	while(new_node = machost)
	{
		machost = machost->next;
		free(new_node);
	}
	
	return;
}
/**/
