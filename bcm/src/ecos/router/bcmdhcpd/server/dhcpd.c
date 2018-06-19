/*
 * DHCP server main state machine.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpd.c,v 1.16 2010-08-10 06:32:22 Exp $
 */
#include <dhcpd.h>
#include <dhcpd_packet.h>


/* Functions prototype declaration */
static int dhcpd_dummy(DHCPD_IFC *);
static int dhcpd_discover(DHCPD_IFC *);
static int dhcpd_request(DHCPD_IFC *);
static int dhcpd_decline(DHCPD_IFC *);
static int dhcpd_release(DHCPD_IFC *);
static int dhcpd_inform(DHCPD_IFC *);

enum {
	INITIATE = 1,
	SHUTDOWN = 2,
	DEBUG = 4
};

static int dhcpd_flag;
static int dhcpd_efd;

DHCPD_IFC *dhcpd_iflist;

typedef int (*state_process_func)(DHCPD_IFC *ifp);
typedef struct dhcpd_state_machine {
	state_process_func func;
	void *arg;
} DHCPD_STATE_MACHINE;

#define NULL_ARG	NULL
static DHCPD_STATE_MACHINE dhcpd_state_machine[DHCP_INFORM+1] = {
	{dhcpd_dummy,		NULL_ARG},	/* 0 */
	{dhcpd_discover,	NULL_ARG},	/* 1:Discover */
	{dhcpd_dummy,		NULL_ARG},	/* 2:Offer */
	{dhcpd_request,		NULL_ARG},	/* 3:Request */
	{dhcpd_decline,		NULL_ARG},	/* 4:Decline */
	{dhcpd_dummy,		NULL_ARG},	/* 5:Ack */
	{dhcpd_dummy,		NULL_ARG},	/* 6:Nack */
	{dhcpd_release,		NULL_ARG},	/* 7:Release */
	{dhcpd_inform,		NULL_ARG}	/* 8:Inform */
};

/* Send server list to option list, for example */
void
dhcpd_add_serverlist_option(struct dhcphdr *packet, unsigned char code, char *buf)
{
	char *name, *p, *next;
	struct in_addr sa;

	for (name = buf, p = name; name && name[0]; name = next, p = 0) {
		strtok_r(p, " ", &next);

		if (inet_aton(name, &sa) != 0)
			dhcp_add_option(packet, code, sizeof(sa), &sa);
	}

	return;
}

/* Add default option to the basic dhcp packet */
void
dhcpd_add_default_options(DHCPD_IFC *ifp)
{
	struct dhcphdr *packet = &ifp->sndpkt;

	unsigned long lease_time;
	char buf[256];

	/*
	 * Set lease time options.
	 * Because dhcp_add_option uses network format,
	 * we have to convert the lease time.
	 */
	if (ifp->cstate != DHCP_INFORM) {
		lease_time = htonl(ifp->clease_time);
		dhcp_add_option(packet, DHCP_LEASE_TIME, sizeof(lease_time), &lease_time);
	}

	/* Add subnet mask */
	dhcp_add_option(packet, DHCP_SUBNET, sizeof(ifp->netmask), &ifp->netmask);

	/* 
	 * Add router
	 * According to the DHCP standard,
	 * the DHCPD_ROUTER should be server list format
	 */
	if (dhcpd_osl_gateway_list(ifp, buf) == 0)
		dhcpd_add_serverlist_option(packet, DHCP_ROUTER, buf);

	/* Add dns server */
	if (dhcpd_osl_dns_list(ifp, buf) == 0) {
		/*
		 * buf contains such as '168.95.1.1 168.95.192.1'
		 * loop all the dns servers and do add option
		 */
		dhcpd_add_serverlist_option(packet, DHCP_DNS_SERVER, buf);
	}

	/* Add domain name */
	if (dhcpd_osl_get_domain(ifp, buf) == 0)
		dhcp_add_option(packet, DHCP_DOMAIN_NAME, strlen(buf), buf);

	return;
}

/*
 * Make a basic dhcp packet, that is common used by
 * DHCPOFFER, DHCPACK and DHCPNACK.
 */
void
make_dhcpd_packet(DHCPD_IFC *ifp, char type)
{
	struct dhcphdr *packet = &ifp->sndpkt;

	/* Clean up options */
	memset(packet, 0, sizeof(*packet));

	packet->op = DHCP_BOOTREPLY;
	packet->htype = DHCP_HTYPE_ETHERNET;
	packet->hlen = DHCP_HLEN_ETHERNET;
	packet->xid = ifp->cxid;

	/* Fill addresses */
	packet->flags = ifp->cflags;
	packet->ciaddr = ifp->ciaddr;
	packet->giaddr = ifp->cgiaddr;
	packet->yiaddr = ifp->cyiaddr;
	memcpy(packet->chaddr, ifp->chaddr, sizeof(packet->chaddr));

	/* Fill options */
	packet->cookie = htonl(DHCP_MAGIC);

	packet->options[0] = DHCP_END;
	dhcp_add_option(packet, DHCP_MESSAGE_TYPE, sizeof(type), &type);
	dhcp_add_option(packet, DHCP_SERVER_ID, sizeof(ifp->ipaddr), &ifp->ipaddr);
}

/* Send NACK to the client */
int
dhcpd_send_nack(DHCPD_IFC *ifp)
{
	/*
	 * NACK packet -
	 * 1. yiaddr = 0;
	 * 2. MESSAGE_TYPE option = DHCPD_NACK
	 * 3. no default options constructed
	 */
	make_dhcpd_packet(ifp, DHCP_NAK);
	dhcpd_send_packet(ifp, 1);

	DHCPD_LOG("%s: sending NAK", ifp->ifname);
	return 0;
}

/* Send DHCPACK to client */
int
dhcpd_send_ack(DHCPD_IFC *ifp)
{
	/*
	 * ACK packet -
	 * 1. yiaddr = ip afford;
	 * 2. MESSAGE_TYPE option = DHCPD_ACK
	 * 3. All default options are constructed
	 */
	make_dhcpd_packet(ifp, DHCP_ACK);
	dhcpd_add_default_options(ifp);
	//hqw add for ack num 590
	#if 1
	struct dhcphdr *packet = &ifp->sndpkt;
	int i = 307;
	packet->options[i] = DHCP_END;
	i = i-1;
	for(;packet->options[i] != DHCP_END; i--)
		packet->options[i] = 0;
	packet->options[i] = 0;
	#endif	
	//end
	dhcpd_send_packet(ifp, 0);

	DHCPD_LOG("%s: send ACK to %s", ifp->ifname, inet_ntoa(ifp->cyiaddr));
	return 0;
}

int
dhcpd_send_offer(DHCPD_IFC *ifp)
{
	/*
	 * OFFER packet -
	 * 1. yiaddr = ip afford;
	 * 2. MESSAGE_TYPE option = DHCPD_OFFER
	 * 3. All default options are constructed
	 *    except for LEASE_TIME setting to
	 *    configured offer_time
	 */
	make_dhcpd_packet(ifp, DHCP_OFFER);
	dhcpd_add_default_options(ifp);
	//hqw add for ack num 590
	#if 1
	struct dhcphdr *packet = &ifp->sndpkt;
	int i = 307;
	packet->options[i] = DHCP_END;
	i = i-1;
	for(;packet->options[i] != DHCP_END; i--)
		packet->options[i] = 0;
	packet->options[i] = 0;
	#endif	
	//end
	dhcpd_send_packet(ifp, 0);

	DHCPD_LOG("%s: send OFFER of %s", ifp->ifname, inet_ntoa(ifp->cyiaddr));
	return 0;
}

/* dummy function */
static int
dhcpd_dummy(DHCPD_IFC *ifp)
{
	return -1;
}

/* Process INFORM packet and send ACK to the client */
static int
dhcpd_inform(DHCPD_IFC *ifp)
{
	/*
	 * INFORM packet -
	 * 1. yiaddr = 0 -- see dhcpd_parse_packet();
	 * 2. MESSAGE_TYPE option = DHCPD_ACK
	 * 3. All default options are constructed
	 *    except for LEASE_TIME option
	 *	  -- see dhcpd_add_replys
	 */
	make_dhcpd_packet(ifp, DHCP_ACK);
	dhcpd_add_default_options(ifp);
	dhcpd_send_packet(ifp, 0);

	DHCPD_LOG("Receive %s inform", inet_ntoa(ifp->ciaddr));
	return 0;
}

/*
 * Receive a release packet,
 * Expire the lease entry of this client.
 */
static int
dhcpd_release(DHCPD_IFC *ifp)
{
	DHCPD_LEASE *lease;

	lease = dhcpd_lease_match(ifp, ifp->chaddr, NULL);
	if (lease) {
		DHCPD_LOG("%s Released", inet_ntoa(lease->ipaddr));
		dhcpd_lease_expiry_renew(ifp, lease, time(0));
	}
	return 0;
}

/* Receive a decline packet from the client.
 * Flush the lease entry assigned to this client
 * and try to reserve this address if someone occupied.
 */
static int
dhcpd_decline(DHCPD_IFC *ifp)
{
	DHCPD_LEASE *lease;

	lease = dhcpd_lease_match(ifp, ifp->chaddr, NULL);
	if (lease) {
		/* we only clean dynamic leases */
		if ((lease->flag & STATICL) == 0) {
			/* Reserved for a short while with broadcast addr */
			memcpy(lease->mac, "\xff\xff\xff\xff\xff\xff", 6);
			lease->flag |= RESERVED;
			dhcpd_lease_expiry_renew(ifp, lease, time(0) + ifp->container.reserve_time);
		}
	}
	return 0;
}

static struct in_addr *
dhcpd_lookup_chaddr(DHCPD_IFC *ifp, struct in_addr *addr)
{
	DHCPD_LEASE *lease = NULL;
	int i;
	int num = ifp->container.pool_num;
	struct lease_pool *pool = ifp->container.pool;

	lease = dhcpd_lease_match(ifp, ifp->chaddr, NULL);
	if (lease == NULL)
		return NULL;

	if ((lease->flag & STATICL)) {
		if (lease->expiry > (unsigned long)time(0))
			ifp->clease_time = lease->expiry - time(0);

		/* Even for static entries, they have to perform the
		 * DHCP process to ensure we are aware of their existence.
		 */
		*addr = lease->ipaddr;
			return addr;
	}

	/* Check the container pool */
	for (i = 0; i < num; i++, pool++) {
		if (ntohl(lease->ipaddr.s_addr) >= ntohl(pool->start.s_addr) &&
		    ntohl(lease->ipaddr.s_addr) <= ntohl(pool->end.s_addr)) {
			/* Probe who else uses this ip. */
			if (dhcpd_probe(ifp, lease, lease->ipaddr, ifp->chaddr) == 0) {
				if (lease->expiry > (unsigned long)time(0))
					ifp->clease_time = lease->expiry - time(0);

				/*
				 * We don't care the requested ip,
				 * if we can find the MAC address in the lease table,
				 * use the boundle ip address.
				 */
				*addr = lease->ipaddr;
				return addr;
			}
		}
	}

	return NULL;
}

static struct in_addr *
dhcpd_lookup_reqip(DHCPD_IFC *ifp, struct in_addr *addr)
{
	DHCPD_LEASE *lease = NULL;
	int i;
	int  num = ifp->container.pool_num;
	struct lease_pool *pool = ifp->container.pool;

	if (addr == NULL)
		return NULL;

	/*
	 * Although we accept the ip the peer requested,
	 * which might be out of the range we can afford.
	 * Drop it if out of range.
	 */
	for (i = 0; i < num; i++, pool++) {
		if (ntohl(addr->s_addr) >= ntohl(pool->start.s_addr) &&
		    ntohl(addr->s_addr) <= ntohl(pool->end.s_addr)) {
			break;
		}
	}
	if (i == num)
		return NULL;

	DHCPD_LOG("requested IP=%x", ntohl(addr->s_addr));

	/* Another case, the ip is conflicting */
	if ((lease = dhcpd_lease_match(ifp, NULL, addr)) != 0) {
		if ((lease->flag & STATICL) ||
		    lease->expiry > (unsigned long)time(0)) {
			/* This IP was in used by some one */
			return NULL;
		}
	}

	/* Probe who else uses this ip */
	#ifdef __CONFIG_AUTO_CONN__
	if (dhcpd_probe_extend(ifp, lease, *addr, ifp->chaddr) != 0)
	#else
	if (dhcpd_probe(ifp, lease, *addr, ifp->chaddr) != 0) 
	#endif
	{
		/*
		 * Someone occupied the ip in the lease table,
		 * clean this lease.
		 */
		return NULL;
	}

	/* Requested ip is acceptable, use it. */
	return addr;
}

static int
dhcpd_set_lease(DHCPD_IFC *ifp, struct in_addr addr,
	unsigned long offer_time, unsigned char *hostname)
{
	/* Add lease. */
	if (dhcpd_lease_add(ifp, ifp->chaddr, addr, offer_time, hostname, 0) == 0) {
		DHCPD_LOG("%s: lease table full!", ifp->ifname);
		return -1;
	}

	/* Offer/Assign address is okay */
	ifp->cyiaddr = addr;
	return 0;
}

/*
 * Matching the config and lease table, and
 * make sure the DHCP request packet is legal
 */
static int
dhcpd_request_client_match(DHCPD_IFC *ifp)
{
	DHCPD_LEASE *lease;
	struct in_addr addr;

	/* Find out the mac address in the lease table */
	lease = dhcpd_lease_match(ifp, ifp->chaddr, NULL);
	if (lease) {
		/*
		 * If the client starts from the INIT-REBOOT state,
		 * it sends a dhcp request packet with REQUESTED IP
		 * saved in its permanet storage before.
		 */
		if (ifp->creqip) {
			/*
			 * Here may be the case, the client got the IP from
			 * us, then, roaming to other network with a new IP
			 * assigned by other dhcp server.  Within the lease
			 * time, it turn off the power, and roaming back into
			 * our IP domain.  In that case, the requested IP will
			 * not be the same as what we saved in the lease table.
			 */
			 #if 0
			/* hqw add, Probe who else uses this ip */
			if (dhcpd_probe(ifp, lease, *(ifp->creqip), ifp->chaddr) != 0) {
				/*
				 * Someone occupied the ip in the lease table,
				 * clean this lease.
				 */
				return -1;
			}
			#endif
			
			if (lease->ipaddr.s_addr != ifp->creqip->s_addr)
				return -1;
		}
		else {
			
			/*
			 * For the RENEWING or REBINDING State,
			 * It is much similar to the INIT-REBOOT situation
			 * explained above. We will send nack and
			 * request the peer to restart from discover state.
			 */
			 #if 0
			/* hqw add, Probe who else uses this ip */
			if (dhcpd_probe(ifp, lease, ifp->ciaddr, ifp->chaddr) != 0) {
				/*
				 * Someone occupied the ip in the lease table,
				 * clean this lease.
				 */
				return -1;
			}
			#endif
			
			if (lease->ipaddr.s_addr != ifp->ciaddr.s_addr)
				return -1;
		}

		addr = lease->ipaddr;
	}
	else {
		/*
		 * Not found in the lease table,
		 * if the client starts from the INIT-REBOOT state,
		 * we accept the requested ip.
		 */
		if (ifp->creqip) {
			addr = *ifp->creqip;
			DHCPD_LOG("%s: no lease, req_ip:%s, accept unconditionally",
				ifp->ifname, inet_ntoa(addr));
		}
		else if (ifp->ciaddr.s_addr) {
			/*
			 * For the RENEWING or REBINDING State,
			 * we accept the ciaddr unconditionally.
			 */
			DHCPD_LOG("%s: no lease, packet->ciaddr:%s, accept unconditionally",
				ifp->ifname, inet_ntoa(ifp->ciaddr));
			addr = ifp->ciaddr;
		}
		else {
			/* Something wrong */
			return -1;
		}

		/* Check conflicting if somebody hold it */
		if (dhcpd_lookup_reqip(ifp, &addr) == NULL)
			return -1;
	}

	/* Set lease */
	return dhcpd_set_lease(ifp, addr, ifp->clease_time, ifp->chostname);
}

/* Process dhcpd request packets */
static int
dhcpd_request(DHCPD_IFC *ifp)
{
	DHCPD_LOG("%s: Receive REQUEST", ifp->ifname);

	/*
	 * Some clients send dhcp request with
	 * broadcast IP address. So, we have to check the
	 * the server id, if that is not ours, drop it.
	 */
	if (ifp->cserver && ifp->cserver->s_addr != ifp->ipaddr.s_addr) {
		DHCPD_LOG("Not my server ID");
		return -1;
	}
	/*
	 * Matching the client in the lease table,
	 * if found, send ack and complete the process,
	 * otherwise, nack the peer.
	 */
	if (dhcpd_request_client_match(ifp) == 0) {
		dhcpd_send_ack(ifp);
	}
	else {
		dhcpd_send_nack(ifp);
	}
	return 0;
}

/* Process dhcpd discover packet */
static int
dhcpd_discover(DHCPD_IFC *ifp)
{
	struct in_addr addr;

	/* block all 0 and all 0xff mac address */
	if (memcmp(ifp->chaddr, "\x00\x00\x00\x00\x00\x00", 6) == 0 ||
	    memcmp(ifp->chaddr, "\xff\xff\xff\xff\xff\xff", 6) == 0) {
		DHCPD_LOG("Block all 0 and all 0xff mac address");
		return -1;
	}

	/* 
	 * Step 1:
	 * Search the MAC in the lease table, if found,
	 * we consider it as the one wants to renew it's lease time.
	 * Step 2:
	 * The incoming MAC is not in the lease table,
	 * or the ip in the lease table is used by someone,
	 * let's try the client's REQUEST_IP if any.
	 * Step 3:
	 * The REQUEST_IP does not work.
	 * We will give the client a valid one in the lease table.
	 */
	if (dhcpd_lookup_chaddr(ifp, &addr))
		goto found;

	if (ifp->creqip) {
		addr = *ifp->creqip;
		if (dhcpd_lookup_reqip(ifp, &addr))
			goto found;
	}

	/* Allocate a free one */
	if (dhcpd_addr_alloc(ifp, &addr) == NULL)
		return -1;

found:
	/* Set lease */
	if (dhcpd_set_lease(ifp, addr, ifp->container.offer_time, NULL) != 0)
		return -1;

	/* Send offer here */
	dhcpd_send_offer(ifp);
	return 0;
}


int
dhcpd_check_vendor_options(struct dhcpd_ifc *ifp, struct dhcphdr *packet)
{
	unsigned char vendor[256] = {0};
	
	if (dhcp_get_option(packet, DHCP_VENDOR, sizeof(vendor), vendor) == 0)
		return 0;		

	/* Special patch for MSFT */
	if (memcmp(vendor, "MSFT ", 5) == 0) {
		ifp->cflags = htons(BROADCAST_FLAG);
	}

	return 0;
}

int
dhcpd_parse_packet(DHCPD_IFC *ifp, struct dhcphdr *packet)
{
	unsigned char *ptr = ifp->coptions;
	unsigned char *cstate = ptr;
	unsigned long clease_time;

	/* Ingore checking non-request type message */
	if (packet->op != DHCP_BOOTREQUEST)
		return 0;

	/* DHCP magic number checking */
	if (ntohl(packet->cookie) != DHCP_MAGIC)
		return 0;

	/* Save the address information */
	ifp->cflags = packet->flags;
	ifp->cxid = packet->xid;
	ifp->ciaddr = packet->ciaddr;
	ifp->cyiaddr.s_addr = 0;
	ifp->cgiaddr = packet->giaddr;
	memcpy(ifp->chaddr, packet->chaddr, sizeof(packet->chaddr));

	/* Do cleanup */
	memset(ifp->coptions, 0, sizeof(ifp->coptions));
	ifp->cserver = NULL;
	ifp->creqip = NULL;
	ifp->chostname = NULL;

	/* Pickup options we are intrested */
	dhcp_get_option(packet, DHCP_MESSAGE_TYPE, sizeof(*cstate), cstate);
	if (*cstate < DHCP_DISCOVER || *cstate > DHCP_INFORM)
		return 0;

	ptr += 4;
	if (dhcp_get_option(packet, DHCP_SERVER_ID, sizeof(*ifp->cserver), ptr))
		ifp->cserver = (struct in_addr *)ptr;

	ptr += sizeof(*ifp->cserver);
	if (dhcp_get_option(packet, DHCP_REQUESTED_IP, sizeof(*ifp->creqip), ptr))
		ifp->creqip = (struct in_addr *)ptr;

	ptr += sizeof(*ifp->creqip);
	if (dhcp_get_option(packet, DHCP_HOST_NAME, 255, ptr))
		ifp->chostname = ptr;

	/* Special handle the lease_time to host format */
	if (dhcp_get_option(packet, DHCP_LEASE_TIME, sizeof(clease_time), &clease_time)) {
		clease_time = ntohl(clease_time);
		if (clease_time > ifp->container.lease_time ||
		    clease_time < ifp->container.lease_min) {
			/* Reset to configured lease time */
			clease_time = ifp->container.lease_time;
		}
	}
	else {
		clease_time = ifp->container.lease_time;
	}

	ifp->clease_time = clease_time;

	/* Check vendor options */
	dhcpd_check_vendor_options(ifp, packet);	

	return (int)(*cstate);
}

/* Process the incoming dhcpd packet */
void
dhcpd_process(DHCPD_IFC *ifp)
{
	struct dhcphdr packet;

	int bytes;
	int cstate;

	/* Read packet from listen socket */
	if ((bytes = dhcpd_read_packet(ifp, &packet)) < 0) {
		return;
	}
	/*
	 * Retrieve all the necessary information
	 * to the context for further process.
	 */
	cstate = dhcpd_parse_packet(ifp, &packet);
	if (cstate == 0) {
		DHCPD_DBG("couldn't get message type option from packet, ignoring");
		return;
	}

	/*
	 * Process different type of dhcp packets
	 */
	ifp->cstate = cstate;
	dhcpd_state_machine[cstate].func(ifp);
	return;
}

extern int
dhcpd_osl_renew_lease(DHCPD_IFC *ifp);

/* Process the incoming dhcpd packet */
static int
dhcpd_event(void)
{
	char buf[256] = {0};
	char *ifname;
	DHCPD_IFC *ifp;

	struct sockaddr_in addr = {0};
	socklen_t addr_len = sizeof(addr);

	if (recvfrom(dhcpd_efd, buf, sizeof(buf)-1, 0,
		(struct sockaddr *)&addr, &addr_len) <= 0) {
		/* Can't read */
		return -1;
	}

	/* Event switch here */
	if (memcmp(buf, "LEASE DUMP ", sizeof("LEASE DUMP ")-1) == 0) {
		struct lease_t *dump;

		/* Locate interface */
		ifname = buf + sizeof("LEASE DUMP ")-1;
		for (ifp = dhcpd_iflist; ifp; ifp = ifp->next) {
			if (strcmp(ifp->ifname, ifname) == 0)
				break;
		}
		if (ifp == 0)
			return 0;

		/* Get lease */
		dump = dhcpd_lease_copy(ifp->ifname);
		if (dump) {
			struct lease_t *lease = dump;
			int len = 0;
			int last = 0;

			while (last == 0) {
				last = lease->last;
				lease++;

				len += sizeof(struct lease_t);
			};

			sendto(dhcpd_efd, dump, len, 0, (struct sockaddr *)&addr, sizeof(addr));
			free(dump);
		}
	}
	else if (strcmp(buf, "STOP") == 0) {
		dhcpd_flag = SHUTDOWN;
	}else if(memcmp(buf, "RENEW_LEASE ", sizeof("RENEW_LEASE ")-1) == 0){

		ifname = buf + sizeof("RENEW_LEASE ")-1;
		
		for (ifp = dhcpd_iflist; ifp; ifp = ifp->next) {
			if (strcmp(ifp->ifname, ifname) == 0)
				break;
		}
		if (ifp == 0)
			return 0;

		dhcpd_osl_renew_lease(ifp);
	}

	return 0;
}

/* dhcpd packet dispatching */
void
dhcpd_dispatch(void)
{
	DHCPD_IFC *ifp;
	fd_set fds;
	int maxfd;
	struct timeval tval = {1, 0};
	int n;

	maxfd = 0;
	FD_ZERO(&fds);

	/* Set event fd */
	FD_SET(dhcpd_efd, &fds);
	maxfd = dhcpd_efd;

	for (ifp = dhcpd_iflist; ifp; ifp = ifp->next) {
		/* Set the device file bit */
		FD_SET(ifp->fd, &fds);
		if (ifp->fd >= maxfd)
			maxfd = ifp->fd;
	}

	n = select(maxfd+1, &fds, NULL, NULL, &tval);
	if (n > 0) {
		if (FD_ISSET(dhcpd_efd, &fds))
			dhcpd_event();

		for (ifp = dhcpd_iflist; ifp; ifp = ifp->next) {
			/* process dhcpd */
			if (FD_ISSET(ifp->fd, &fds)) {
				/* Process the dhcpd incoming packet */
				dhcpd_process(ifp);
			}
		}
	}
	return;
}

static void
dhcpd_ifdeinit(DHCPD_IFC *ifp)
{
	/* Close dhcp server device file */
	if (ifp->fd >= 0) {
		close(ifp->fd);
		ifp->fd = -1;
	}

	/* Free lease pool */
	dhcpd_lease_deinit(ifp);

	/* Free the allocated interface structure. */
	free(ifp);
	return;
}

/* Shutdown the dhcp server */
int
dhcpd_shutdown(void)
{
	DHCPD_IFC *ifp;

	if (dhcpd_efd >= 0)
		close(dhcpd_efd);

	while ((ifp = dhcpd_iflist) != 0) {

		/* Detach from the link list */
		dhcpd_iflist = ifp->next;

		dhcpd_ifdeinit(ifp);
	}

	return 0;
}

/* Initialize dhcpd interface initialization. */
DHCPD_IFC
*dhcpd_ifinit(char *idxname)
{
	DHCPD_IFC *ifp;
	char *name;
	char devname[64];

	ifp = (DHCPD_IFC *)malloc(sizeof(*ifp));
	if (ifp == 0)
		return 0;

	memset(ifp, 0, sizeof(*ifp));

	/* 
	 * Separate the "instance+name" string to
	 * instance and interface name, and save
	 * in the interface structure.
	 */
	strtok_r(idxname, "=", &name);

	strcpy(ifp->ifname, name);
	ifp->unit = atoi(idxname);
	ifp->fd = -1;

	/* 
	 * Initialize interface configuration.
	 *
	 * Note: For different OS platform, the configuration
	 * should be read from a permanent storage, such as
	 * a file or an NVRAM, and convert to the input interface
	 * struct.
	 */
	if (dhcpd_osl_init_config(ifp) != 0)
		goto init_err;

	/* Initialize Lease pool, allocate pool_buf */
	dhcpd_lease_init(ifp);

	/* Open dhcp server device */
	sprintf(devname, "%s/%s", DHCPD_DEVNAME, name);
	if ((ifp->fd = open(devname, O_RDWR)) < 0) {
		DHCPD_DBG("FATAL: couldn't create device for %s", name);
		goto init_err;
	}

	/* Do prepend */
	ifp->next = dhcpd_iflist;
	dhcpd_iflist = ifp;

	DHCPD_DBG("ADD dhcpd%d on %s", ifp->unit, ifp->ifname);
	return ifp;

init_err:
	if (ifp)
		dhcpd_ifdeinit(ifp);

 	return 0;
}
//roy +++2010/09/03 add for static lease
extern int str2arglist(char *, char **, char, int);
void dhcpd_LoadStaticLeases(DHCPD_IFC *ifp)
{
	int i=0;
	char lease_str[100];
	char *arglists[5];
	struct in_addr sa,lan_sa;
	struct ether_addr mac;//in ethernet.h
	unsigned int flag;

	char lan_ip[20],ret;

	ret = dhcpd_osl_lan_ip(ifp, lan_ip);
	if(ret){
		inet_aton(lan_ip, &lan_sa);
	}

	//i>=1 && i<=16
	while(i++ <= DHCPD_STATIC_LEASE_NU+1)
	{	
		dhcpd_osl_static_lease(ifp,lease_str,i);

		if(lease_str[0] == 0)//no more can read
		{
			//printf("%s,%d: i=%d, break\n",__FUNCTION__,__LINE__,i);	
			break;
		}	
		if (str2arglist(lease_str, arglists, ';', 5) != 5)
			continue;
			
		inet_aton(arglists[1], &sa);
		ether_aton_r(arglists[2], &mac);
		flag = atoi(arglists[3]);

		if (flag & DISABLED)
			continue;

		if(ret && lan_sa.s_addr == sa.s_addr)
			continue;

		/* Add lease. */
		
		//printf("%s,%d: ip=%s, mac=%s\n",__FUNCTION__,__LINE__,inet_ntoa(sa), ether_ntoa(&mac)); 
		if (dhcpd_lease_add(ifp, mac.octet, sa,atoi(arglists[4]), arglists[0],flag) == 0) {
			DHCPD_LOG("%s: lease table full!", ifp->ifname);
			return;
		}
	
	}
	
	return;
}
//roy+++

/* add by ldm */
void
add_dhcpd_static_lease(void)
{
	DHCPD_IFC *ifp;

	for (ifp = dhcpd_iflist; ifp; ifp = ifp->next) {

		struct lease_head *inuse = &ifp->container.inuse;
		DHCPD_LEASE *lease;
		
		CIRCLEQ_FOREACH(lease, inuse, lease_queue) {
			lease->flag = DYNAMICL;
		}
		dhcpd_LoadStaticLeases(ifp);
	}

	return;
}
void
update_dhcp_client_lease(time_t new_time2)
{
	DHCPD_IFC *ifp;

	for (ifp = dhcpd_iflist; ifp; ifp = ifp->next) {
		struct lease_head *inuse = &ifp->container.inuse;
		DHCPD_LEASE *lease;
		
		CIRCLEQ_FOREACH(lease, inuse, lease_queue) {
			lease->expiry = new_time2 + (lease->expiry - time(0));
		}
	}

	return;
}
//end by ldm
/* Initialize the dhcpd context. */
int
dhcpd_init(void)
{
	DHCPD_IFC *ifp = 0;

	char ifname_list[256];
	char *name, *p, *next;

	/* Clean up */
	dhcpd_flag = 0;
	dhcpd_efd = -1;
	dhcpd_iflist = 0;

	/* enumerate all interfaces name */
	memset(ifname_list, 0, sizeof(ifname_list));

	dhcpd_osl_ifname_list(ifname_list);

	/* loop all the interfaces to do interface initialization */
	for (name = ifname_list, p = name;
	     name && name[0];
	     name = next, p = 0) {

		/*
		 * Separate "instance+name" for each interface in
		 * the ifname list string, and do interface
		 * initialization with the "instance+name" string.
		 */
		strtok_r(p, " ", &next);
		ifp = dhcpd_ifinit(name);
		if (ifp == 0){
			DHCPD_DBG("dhcpd_request_init::%s init error!", name+2);
		}
//roy +++2010/09/03 add for static lease
		else{
			dhcpd_LoadStaticLeases(ifp);
		}	
//roy+++
	}

	/*
	 * The iflist is the interface link.  Not every interface
	 * enables the dhcp server, but no one enables the dhcp
	 * server, the iflist will be null.  In this case,
	 * the dhcp server should go down.
	 */
	if (dhcpd_iflist == 0) {
		DHCPD_DBG("No dhcpd interface specified, bye!");
		goto error_out;
	}

	/* Open ipc fd now */
	if ((dhcpd_efd = open_udp_socket("127.0.0.1", DHCPD_IPC_PORT)) < 0) {
		DHCPD_DBG("Event IPC open failed!");
		goto error_out;
	}

	DHCPD_DBG("DHCPD daemon is ready to run");
	return 0;

error_out:
	dhcpd_flag = SHUTDOWN;
	return -1;
}

/* Portable main loop of dhcp server */
void
dhcpd_mainloop(void)
{
	/* initialize dhcpd */
	dhcpd_init();

	/* main loop */
	while (1) {
		switch (dhcpd_flag) {
		case SHUTDOWN:
			DHCPD_DBG("DHCPD shutdown!");
			dhcpd_shutdown();
			return;

		default:
			dhcpd_dispatch();
			break;
		}
	}

	return;
}
