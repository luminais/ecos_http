/*
 * DNSMASQ main state machine.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dnsmasq.c,v 1.6 2010-11-12 15:06:20 Exp $
 */

#include <dnsmasq.h>
#include <unistd.h>
#include <arpa/inet.h>

int dns_query_on_idel_time = 0;

enum {
	INITIATE = 1,
	SHUTDOWN = 2,
	DEBUG = 8
};

int dnsmasq_flag;

static	DNSMASQ_INTERFACE *iflist;
static	DNSMASQ_SERVER *server_list;
static	DNSMASQ_SERVER *last_reply_server;
static	DNSMASQ_HOST *host_list;
static	char localhost[MAXDNAME];
static	DNSMASQ_ENTRY *forward_list;
static	DNSMASQ_ENTRY *cache_list;
static	DNSMASQ_ENTRY *free_pool;
static	int dnsmasq_count;
static	int dnsmasq_uptime;


/* Shutdown dnsmasq */
int
dnsmasq_shutdown(void)
{
	DNSMASQ_INTERFACE	*ifp;
	DNSMASQ_SERVER		*server;
	DNSMASQ_HOST		*host;
	DNSMASQ_ENTRY		*entry;

	/* Free interface list */
	while ((ifp = iflist) != 0) {
		if (ifp->pkt_sock >= 0) {
			close(ifp->pkt_sock);
			ifp->pkt_sock = -1;
		}

		iflist = ifp->next;
		free(ifp);
	}

	/* Free server list */
	while ((server = server_list) != 0) {
		if (server->fd >= 0) {
			close(server->fd);
			server->fd = -1;
		}

		server_list = server->next;
		free(server);
	}

	/* Free host list */
	while ((host = host_list) != 0) {
		host_list = host->next;
		free(host);
	}

	/* Free forward entries */
	while ((entry = forward_list) != 0) {
		forward_list = entry->next;
		free(entry);
	}

	/* Free cache entries */
	while ((entry = cache_list) != 0) {
		cache_list = entry->next;
		free(entry);
	}

	/* Free the freelist */
	while ((entry = free_pool) != 0) {
		free_pool = entry->next;
		free(entry);
	}

	return 0;
}

/* Append a cache into list */
void
dnsmasq_free(DNSMASQ_ENTRY *entry)
{
	if (entry) {
		entry->next = free_pool;
		free_pool = entry;
	}

	return;
}

/* New a cache entry. */
DNSMASQ_ENTRY *
dnsmasq_new()
{
	DNSMASQ_ENTRY *entry;

	/*
	 * Use the spare one in the free list,
	 * if possible.
	 */
	if (free_pool) {
		/* Dequeue the first one */
		entry = free_pool;
		free_pool = entry->next;
	}
	else if (dnsmasq_count < MAXCOUNT) {
		/* Add a new cache to forward list */
		entry = (DNSMASQ_ENTRY *)malloc(sizeof(DNSMASQ_ENTRY));
		if (!entry) {
			DNSMASQ_DBG("%s:FATAL:malloc failed!\n", __func__);
			return NULL;
		}

		dnsmasq_count++;
	}
	else if (cache_list) {
		DNSMASQ_DBG("%s:drain from cache list\n", __func__);
		entry = cache_list;
		cache_list = entry->next;
	}
	else {
		DNSMASQ_DBG("%s:MAXCOUNT(%d) too small\n", __func__, MAXCOUNT);
		return NULL;
	}

	/* Clear entry */
	memset(entry, 0, sizeof(*entry));
	return entry;	
}

void
dnsmasq_forward_del(DNSMASQ_ENTRY *entry)
{
	DNSMASQ_ENTRY *temp;
	DNSMASQ_ENTRY *prev;

	prev = 0;
	temp = forward_list;
	while (temp) {
		/* Remove this one from the cache queue */
		if (temp == entry) {

			if (prev == 0)
				forward_list = entry->next;
			else
				prev->next = entry->next;

			break;
		}

		/* Move to next */
		prev = temp;
		temp = temp->next;
	}

	return;
}

void
dnsmasq_forward_add(DNSMASQ_ENTRY *entry)
{
	if (entry) {
		entry->next = forward_list;
		forward_list = entry;
	}
}

/* Search a matched forward entry with nid. */
DNSMASQ_ENTRY *
dnsmasq_forward_find_by_nid(unsigned short nid)
{
	DNSMASQ_ENTRY *entry;

	for (entry = forward_list; entry; entry = entry->next) {
		if (entry->nid == nid)
			break;
	}

	return entry;
}

/* Search a matched forward entry with ip and domain name. */
DNSMASQ_ENTRY *
dnsmasq_forward_find_by_addr(struct in_addr addr, char *dnsname,int type)
{
	DNSMASQ_ENTRY *entry;

	/*
	 * Search for an matched dnsname and queryaddr.
	 */
	for (entry = forward_list; entry; entry = entry->next) {
		if (strcasecmp(entry->dnsname, dnsname) == 0 &&
			entry->queryaddr.sin_addr.s_addr == addr.s_addr && 
				entry->flag == type) {
			break;
		}
	}

	return entry;
}

/* Add a cache entry to cache list */
void
dnsmasq_cache_add(DNSMASQ_ENTRY *entry)
{
	if (entry) {
		entry->next = cache_list;
		cache_list = entry;
	}
}

/* Search a matched cache entry. */
DNSMASQ_ENTRY *
dnsmasq_cache_find(char *dnsname)
{
	DNSMASQ_ENTRY *entry;

	/* 
	 * Search a cache entry with matched dnsname
	 */
	for (entry = cache_list; entry; entry = entry->next) {
		if (strcasecmp(entry->dnsname, dnsname) == 0)
			break;
	}

	return entry;
}

char *
dnsmasq_get_localhost()
{
	return localhost;
}

/* Search a matched host name */
DNSMASQ_HOST *
dnsmasq_host_find(char *dnsname)
{
	DNSMASQ_HOST *host;

	for (host = host_list; host; host = host->next) {
		if (strcasecmp(host->name, dnsname) == 0)
			return host;
	}

	return NULL;
}

/* Initialize hosts to a link list */
DNSMASQ_HOST *
dnsmasq_host_init(char *name)
{
	DNSMASQ_HOST *host;
	char *ip;
	struct in_addr addr;

	/* Spilit hostname and address */
	strtok_r(name, " ", &ip);
	if (ip == 0 || inet_aton(ip, &addr) == 0)
		return 0;

	/* Allocate a host entry */
	host = (DNSMASQ_HOST *)malloc(sizeof(DNSMASQ_HOST));
	if (host == 0)
		return 0;

	memset(host, 0, sizeof(DNSMASQ_HOST));
	strcpy(host->name, name);
	host->addr = addr;

	return host;
}

/* Get the last reply server */
DNSMASQ_SERVER *
dnsmasq_get_last_reply_server()
{
	return last_reply_server;
}

/* Set the server that reply the request */
void
dnsmasq_set_last_reply_server(DNSMASQ_SERVER *server)
{
	last_reply_server = server;
}

/* Return the next forword dns server. */
DNSMASQ_SERVER *
dnsmasq_server_next(DNSMASQ_SERVER *current)
{
	DNSMASQ_SERVER *server;

	for (server = server_list;
	     server;
	     server = server->next) {
		if (server == current && server->fd > 0)
			break;
	}
	if (server == NULL || server->next == NULL)
		return server_list;

	return server->next;
}
#define   MXAPORT    6
unsigned short portbuf[MXAPORT] = {0}; //产生的服务器端口至少8个不一样
#define SPORT_MIN	 49152    /* 大端口最小值49152 */
#define SPORT_MAX	 65535	  /* 大端口最大值65535 */

static unsigned short
dnsmasq_random_port()
{
    static unsigned char num = 0;
    unsigned short lastport = 0;
    unsigned char  i,flag = 0;
    while(!flag)
    {
        flag = 1;
        srand((unsigned short)time(NULL)+num);
        lastport = SPORT_MAX - (rand() % (SPORT_MAX - SPORT_MIN));
        for(i = 0;i < MXAPORT;i++)
        {
            if(lastport == portbuf[i])
            {
                
                flag = 0;
                break;
            }            
        }
        if(!flag) continue;        
        portbuf[num++] = lastport;
        if(num >= MXAPORT) num = 0;
        break;
    }
    return lastport;
}

/* Initialize dns servers to a link list */
DNSMASQ_SERVER
*dnsmasq_server_init(struct in_addr addr)
{
	DNSMASQ_SERVER *server;
	unsigned short sport = 0;
	sport = dnsmasq_random_port();
	server = (DNSMASQ_SERVER *)malloc(sizeof(DNSMASQ_SERVER));
	if (server == 0)
		return 0;
	memset(server, 0, sizeof(DNSMASQ_SERVER));
    
	server->fd = -1;
	server->addr = addr;
	if ((server->fd = open_udp_socket("0.0.0.0", sport)) < 0) {
		DNSMASQ_DBG("FATAL: couldn't create listen socket for %s", inet_ntoa(addr));
		goto init_err;
	}
	return server;

init_err:
	if (server != 0)
		free(server);
	return 0;
}

/* Initialize interface to a link list */
DNSMASQ_INTERFACE
*dnsmasq_ifinit(char *idxname)
{
	DNSMASQ_INTERFACE *ifp;
	char *name;
	char tmp[32] = {0};
	char ipaddr[32] = {0};

	ifp = (DNSMASQ_INTERFACE *)malloc(sizeof(DNSMASQ_INTERFACE));
	if (ifp == 0)
		return 0;

	memset(ifp, 0, sizeof(DNSMASQ_INTERFACE));

	/* 
	 * Separate the "instance+name" string to
	 * instance and interface name, and save
	 * in the interface structure.
	 */
	strtok_r(idxname, "=", &name);

	strcpy(ifp->ifname, name);

	ifp->unit = atoi(idxname);
	ifp->pkt_sock = -1;

	if(ifp->unit == 0)
	{
		if (dnsmasq_osl_ifaddr(ifp->ifname, &ifp->ipaddr) != 0) {
			DNSMASQ_DBG("Interface %s can't get ip addr.\n", ifp->ifname);
			goto init_err;
		}
	}else
	{
		snprintf(tmp, sizeof(tmp), "lan%d_ipaddr", ifp->unit);
		inet_aton(nvram_get(tmp),&(ifp->ipaddr));		
	}

	/* Open dnsmasq server socket */
	strcpy(ipaddr, inet_ntoa(ifp->ipaddr));
	if ((ifp->pkt_sock = open_udp_socket(ipaddr, NAMESERVER_PORT)) < 0) {
		DNSMASQ_DBG("FATAL: couldn't create listen socket for %s", name);
		goto init_err;
	}

	DNSMASQ_DBG("ADD dnsmasq%d on %s", ifp->unit, ifp->ifname);
	return ifp;

init_err:
	if (ifp != 0)
		free(ifp);

	return 0;
}

/*
 * Initialize dnsmasq, include interfaces,
 * dns servers, hosts and cache entry.
 */
int
dnsmasq_init(void)
{
	char ifname_list[256];
	char dns_list[256];
	char *host_table;

	char *name, *p, *next;
	DNSMASQ_INTERFACE *ifp = 0;
	DNSMASQ_SERVER *server = 0;
	DNSMASQ_HOST *host = 0;


	dnsmasq_flag = 0;
	dnsmasq_uptime = time(0);
	dnsmasq_count = 0;

	iflist = NULL;
	server_list = NULL;
	host_list = NULL;

	forward_list = NULL;
	cache_list = NULL;
	free_pool = NULL;

	/*
	 * Do interface initialization firstly.
	 * The is because the dns servers might
	 * be incorrectly configured to be as
	 * same as one of the interface ip.
	 *
	 * Enumerate all interfaces name,
	 * and do interface intialization for
	 * each one.
	 */
	memset(ifname_list, 0, sizeof(ifname_list));
	dnsmasq_osl_ifname_list(ifname_list);

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

		ifp = dnsmasq_ifinit(name);

		/* Do prepend */
		if (ifp) {
			ifp->next = iflist;
			iflist = ifp;
		}
	}

	if (iflist == 0)
		goto error_out;

	/* Do host intialization */
	host_table = (char *)malloc(256 * MAXHOSTSENTRY);
	if (host_table == NULL)
		goto error_out;

	memset(host_table, 0, 256 * MAXHOSTSENTRY);
	dnsmasq_osl_host_list(host_table);

	for (name = host_table, p = name;
	     name && name[0];
	     name = next, p = 0) {
		strtok_r(p, "\n\r", &next);

		host = dnsmasq_host_init(name);
		if (host) {
			host->next = host_list;
			host_list = host;
		}
	}

	free(host_table);

	/* Initialize local host */
	dnsmasq_osl_localhost_init(localhost);

	/* 
	 * Enumerate all the dns server names,
	 * and do server intialization one by one.
	 */
	if (dnsmasq_osl_dns_list(dns_list) != 0) {
		DNSMASQ_DBG("Server init fail.");
		goto error_out;
	}

	for (name = dns_list, p = name;
	     name && name[0];
	     name = next, p = 0) {
		struct in_addr addr;

		strtok_r(p, " ", &next);

		if (inet_aton(name, &addr) == 0)
			continue;

		/*
		 * Should not be as same as one of the interface's
		 * ip address.
		 */
		for (ifp = iflist; ifp; ifp = ifp->next) {
			if (addr.s_addr == ifp->ipaddr.s_addr)
				break;
		}
		if (ifp != 0)
			continue;

		/* Do server intialization */
		server = dnsmasq_server_init(addr);
		if (server) {
			server->next = server_list;
			server_list = server;
		}
	}

	/* Set active dns server */
	last_reply_server = server_list;

	DNSMASQ_DBG("DNSMASQ daemon is ready to run.");
	return 0;

error_out:
	dnsmasq_flag = SHUTDOWN;
	return -1;
}

void
dnsmasq_timeout(void)
{
	DNSMASQ_ENTRY *entry;
	DNSMASQ_ENTRY *prev, *next;
	time_t now = time(0);

	/* At least one second passed */
	if ((now - dnsmasq_uptime) <= 0)
		return;

	dnsmasq_uptime = now;

	/* Aging the cache list */
	prev = 0;
	entry = cache_list;
	while (entry) {
		next = entry->next;

		if ((entry->expire) < now) {
			/* Remove from forward list */
			if (prev == 0)
				cache_list = entry->next;
			else
				prev->next = entry->next;

			/* Free this entry */
			dnsmasq_free(entry);
		}
		else {
			/* Save the previous one */
			prev = entry;
		}

		entry = next;
	}

	/* Aging the forward list */
	prev = 0;
	entry = forward_list;
	while (entry) {
		next = entry->next;

		if ((entry->expire) < now) {
			/* Remove from forward list */
			if (prev == 0)
				forward_list = entry->next;
			else
				prev->next = entry->next;

			/* Free this entry */
			dnsmasq_free(entry);
		}
		else {
			/* Save the previous one */
			prev = entry;
		}

		entry = next;
	}
}

/* 
 * Process dnsmasq messages, include messages from interface,
 * and messages from dns servers.
 */
static char packet[PACKETSZ+MAXDNAME+RRFIXEDSZ];

void
dnsmasq_dispatch(void)
{
	DNSMASQ_INTERFACE *ifp;
	DNSMASQ_SERVER *server;
	DNSHEADER *dnsheader = (DNSHEADER *)packet;
	int len	= 0;

	fd_set fds;
	int maxfd = 0;
	struct timeval tv = {1, 0};
	int n;

	/* Set receive select set */
	FD_ZERO(&fds);

	for (server = server_list; server; server = server->next) {
		FD_SET(server->fd, &fds);
		if (server->fd > maxfd)
			maxfd = server->fd;
	}

	for (ifp = iflist; ifp; ifp = ifp->next) {
		/* Set the packet socket bit */
		FD_SET(ifp->pkt_sock, &fds);
		if (ifp->pkt_sock >= maxfd)
			maxfd = ifp->pkt_sock;
	}

	/* Wait for socket events */
	n = select(maxfd+1, &fds, NULL, NULL, &tv);
	if (n > 0) {
		/* check dns query from querist */
		for (ifp = iflist; ifp; ifp = ifp->next) {
			if (FD_ISSET(ifp->pkt_sock, &fds)) {
				/* request packet, deal with query */
				struct sockaddr_in queryaddr;
				unsigned int queryaddrlen = sizeof(queryaddr);
				DNSMASQ_DBG("ipf->pkt_sock's select is weak, will recv packet");
				len = recvfrom(ifp->pkt_sock, dnsheader, PACKETSZ, 0,
					(struct sockaddr *)&queryaddr, &queryaddrlen);
				if (len >= sizeof(DNSHEADER)) {
					/*
					 * Process the request, even if no server, in which case
					 * we will send no error message to accelarate the timeout
					 * of DNS requesting.
					 */
					dnsmasq_process_query(dnsheader, len, ifp->pkt_sock,
						ifp->ipaddr, &queryaddr);
					
					dns_query_on_idel_time++;
				}
			}
		}
		/* 
		 * pxy discribe 2013.06.06
		 * once wan get ip , then run the following code
		 */ 
		/* check dns reply from dns server */
		for (server = server_list; server; server = server->next) {
			if (FD_ISSET(server->fd, &fds)) {
				len = recvfrom(server->fd, packet, PACKETSZ, 0, NULL, NULL);
				if (len >= sizeof(DNSHEADER)) {
					/*
					 * Process the resply from the dns server,
					 */
					dnsmasq_process_reply(dnsheader, len, server->fd);
				}
			}
		}
		
	}

	/* Do time out here */
	dnsmasq_timeout();

	return;
}

/* 
 * The main entrance of dnsmasq,
 * loop continues until shutdown happens
 */
void
dnsmasq_mainloop(void)
{
	/* Do init */
	dnsmasq_init();

	while (1) {
		switch (dnsmasq_flag) {
		case SHUTDOWN:
			dnsmasq_shutdown();
			DNSMASQ_DBG("DNSMASQ shutdown\n");
			return;

		case 0:
		default:
			dnsmasq_dispatch();
			break;
		}
	}
}

/* Set the flag to terminate dnsmasq. */
void
dnsmasq_terminate(void)
{
	dnsmasq_flag = SHUTDOWN;
}
/*2017/2/8 add by lrl*/
void update_server_list(struct in_addr addr,char *dns_list)
{
	char *name, *p, *next;
	DNSMASQ_INTERFACE *ifp = 0;
	DNSMASQ_SERVER *server = 0;
	
    DNSMASQ_SERVER *server_list_temp = NULL;
	DNSMASQ_SERVER *server_temp = 0;

	server_temp = server_list;

	for (name = dns_list, p = name;
		 name && name[0];
		 name = next, p = 0) {
		struct in_addr addr;

		strtok_r(p, " ", &next);

		if (inet_aton(name, &addr) == 0)
			continue;

		/*
		 * Should not be as same as one of the interface's
		 * ip address.
		 */
		for (ifp = iflist; ifp; ifp = ifp->next) {
			if (addr.s_addr == ifp->ipaddr.s_addr)
				break;
		}
		if (ifp != 0)
			continue;

		/* Do server intialization */
		server = dnsmasq_server_init(addr);
		if (server) {
			server->next = server_list_temp;
			server_list_temp = server;
		}
	}

    server_list = server_list_temp;
	
	/* Set active dns server */
	last_reply_server = server_list;

	/* Free server list */
	while ((server = server_temp) != 0) 
	{
			if (server->fd >= 0) {
				close(server->fd);
				server->fd = -1;
			}
	
			server_temp = server->next;
			free(server);
	}

	return;
}

