/*
 * DHCP client main entry.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpc_mainloop.c,v 1.11 2010-08-04 10:55:08 Exp $
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <dhcpc.h>
#include <dhcpc_output.h>
#include <kdhcpc.h>

/* Macros */
#define DHCPC_OUT(a)	{cmd = a; goto dhcpc_out;}
#define DEFAULT_IP	"169.254.0.101"

#define	DHCPC_NO_DELAY			0
#define	DHCPC_REINIT_DELAY		15
#define	DHCPC_RELEASE_DELAY		600

#define	DHCPC_MAX_COUNTDOWN		4
#define	DHCPC_INIT_RCV_SEC		4

#define	EVENT_PACKET			1
#define	EVENT_FREE			2
#define	EVENT_RELEASE			4
#define	EVENT_RENEW			8


//第一次用户使用路由器时检测上网环境使用，add by wwk20120929
int recv_dhcp_offer_packet_tag = 0;
int dhcp_offer_tag = 0;
int dhcp_send_discover_num = 0;
extern int dhcp_mode_check_tag;


/* Functions prototype declaration */
extern int CGI_same_net_with_lan(unsigned int wan_ip,unsigned int wan_mask);//roy add
extern int run_sh(char *cmd, char **argv, char **env);
extern unsigned int sysuptime(void);
static int ifup(char *ifname);

static void dhcpc_dummy(struct dhcpc_ifc *difp);
static void dhcpc_state_init(struct dhcpc_ifc *difp);
static void dhcpc_state_selecting(struct dhcpc_ifc *difp);
static void dhcpc_state_requesting(struct dhcpc_ifc *difp);
static void dhcpc_state_bound(struct dhcpc_ifc *difp);
static void dhcpc_state_renewing(struct dhcpc_ifc *difp);
static void dhcpc_state_rebinding(struct dhcpc_ifc *difp);


typedef void (* state_process_func)(struct dhcpc_ifc *);
typedef struct dhcpc_state_machine {
	state_process_func func;
	void *arg;
} DHCPC_STATE_MACHINE;

#define NULL_ARG	NULL
static DHCPC_STATE_MACHINE dhcpc_state_machine[DHCPC_STATE_REBINDING+1] = {
	{dhcpc_dummy, NULL_ARG}, /* 0 */
	{dhcpc_state_init, NULL_ARG}, /* 1:Init */
	{dhcpc_state_selecting, NULL_ARG}, /* 2:Selecting */
	{dhcpc_state_requesting, NULL_ARG}, /* 3:Requesting */
	{dhcpc_state_bound, NULL_ARG}, /* 4:Bound */
	{dhcpc_state_renewing, NULL_ARG}, /* 5:Renewing */
	{dhcpc_state_rebinding, NULL_ARG} /* 6:Rebinding */
};

static void
reset_countdown(struct dhcpc_ifc *difp)
{
	difp->countdown = DHCPC_MAX_COUNTDOWN;
	difp->secs = DHCPC_INIT_RCV_SEC;
	difp->rmsecs = 0;
}

static void
new_xid(struct dhcpc_ifc *difp)
{
	difp->xid = difp->macaddr[5];
	difp->xid |= (difp->macaddr[4]) << 8;
	difp->xid |= (difp->macaddr[3]) << 16;
	difp->xid |= (difp->macaddr[2]) << 24;
	difp->xid ^= (arc4random() & 0xffff0000);
}

static void
no_lease(struct dhcpc_ifc *difp)
{
	struct dhcp_lease *lease = &difp->lease;
	lease->timeout = 0;
}

static void
new_lease(struct dhcpc_ifc *difp) 
{
	struct dhcp_lease *lease = &difp->lease;
    unsigned int tag = 0;

	no_lease(difp);
	lease->curr = 0;
	lease->next = 0;

	lease->time_stamp = time(0);
	lease->time_last = lease->time_stamp;

	/* Get LEASE TIME */
	if (dhcp_get_option(&difp->rxpkt, DHCP_LEASE_TIME, sizeof(tag), &tag) && (tag == 0)) {
		lease->expiry = 0;
		lease->t2 = 0;
		lease->t1 = 0;
		return;
	}

	DHCPC_DBG("get new lease time: %u secs", ntohl(tag));
	lease->expiry = ntohl(tag);

	/* Get T2 */
	tag = 0;
	if (dhcp_get_option(&difp->rxpkt, DHCP_T2, sizeof(tag), &tag) && (tag != 0)) {
		lease->t2 = ntohl(tag);
		DHCPC_DBG("get DHCPC_T2: %d secs", ntohl(tag));
	}
	else {
		lease->t2 = lease->expiry - lease->expiry/8;
	}

	/* Get T1 */
	tag = 0;
	if (dhcp_get_option(&difp->rxpkt, DHCP_T1, sizeof(tag), &tag) && (tag != 0)) {
		lease->t1 = ntohl(tag);
		DHCPC_DBG("get DHCPC_T1: %d secs", ntohl(tag));
	}
	else {
		lease->t1 = lease->expiry/2;
	}

	/* Set first timeout T1 */
	lease->next = DHCPC_LEASE_T1;
	lease->timeout = time(0) + lease->t1;
}

static void
add_requests(struct dhcpc_ifc *difp)
{
	struct dhcphdr *txpkt = &difp->txpkt;
	unsigned char cid[7];
	unsigned char param_req[] = {DHCP_SUBNET, DHCP_ROUTER, DHCP_DNS_SERVER, DHCP_DOMAIN_NAME,STATIC_ROUTER_33, STATIC_ROUTER_121, STATIC_ROUTER_249};

	cid[0] = 1;
	memcpy(cid+1, difp->macaddr, 6);
	dhcp_add_option(txpkt, DHCP_CLIENT_ID, 7, cid);

	if (difp->hostname != 0)
		dhcp_add_option(txpkt, DHCP_HOST_NAME, strlen(difp->hostname), difp->hostname);

	dhcp_add_option(txpkt, DHCP_PARAM_REQ, sizeof(param_req), param_req);
}

static void
make_dhcpc_packet(struct dhcpc_ifc *difp, unsigned char type)
{
	struct dhcphdr *txpkt = &difp->txpkt;

	memset(txpkt, 0, sizeof(*txpkt));

	txpkt->op = DHCP_BOOTREQUEST;
	txpkt->htype = DHCP_HTYPE_ETHERNET;
	txpkt->hlen = DHCP_HLEN_ETHERNET;
	txpkt->xid = difp->xid;

	memcpy(txpkt->chaddr, difp->macaddr, DHCP_HLEN_ETHERNET);

	txpkt->cookie = htonl(DHCP_MAGIC);
	txpkt->options[0] = DHCP_END;

	dhcp_add_option(txpkt, DHCP_MESSAGE_TYPE, sizeof(type), &type);
	//add for nex 201210，添加option60选项，解决上级连接cable modem获取不到正常IP问题-------
	if ((type != DHCP_DECLINE) && (type != DHCP_RELEASE))
		dhcp_add_option(txpkt,DHCP_VENDOR,sizeof("MSFT 5.0"),"MSFT 5.0");
	//end add--------------
}

static void
send_discover(struct dhcpc_ifc *difp)
{
	no_lease(difp);
	make_dhcpc_packet(difp, DHCP_DISCOVER);

	//if (difp->req_ip.s_addr) {
		dhcp_add_option(&difp->txpkt, DHCP_REQUESTED_IP,
			sizeof(difp->req_ip), &difp->req_ip);
	//}

	add_requests(difp);
	//hqw add for discover num 590
	#if 1	
	struct dhcphdr *txpkt = &difp->txpkt;
	int i = 307;
	txpkt->options[i] = DHCP_END;
	i = i-1;
	for(;txpkt->options[i] != DHCP_END; i--)
		txpkt->options[i] = 0;
	txpkt->options[i] = 0;
	#endif	
	//end

	dhcpc_write(difp);

	difp->secs += 2 + (arc4random() & 3);
	difp->rmsecs = difp->secs;
	dhcp_send_discover_num ++;
}

static void
send_request(struct dhcpc_ifc *difp)
{
	struct dhcphdr *txpkt = &difp->txpkt;

	make_dhcpc_packet(difp, DHCP_REQUEST);

	if (difp->req_ip.s_addr)
		dhcp_add_option(txpkt, DHCP_REQUESTED_IP, sizeof(difp->req_ip), &difp->req_ip);

	if (difp->server.s_addr)
		dhcp_add_option(txpkt, DHCP_SERVER_ID, sizeof(difp->server), &difp->server);

	add_requests(difp);
	//hqw add for request num 590
	#if 1	
	int i = 307;
	txpkt->options[i] = DHCP_END;
	i = i-1;
	for(;txpkt->options[i] != DHCP_END; i--)
		txpkt->options[i] = 0;
	txpkt->options[i] = 0;
	#endif	
	//end

	dhcpc_write(difp);

	difp->secs += 2 + (arc4random() & 3);
	difp->rmsecs = difp->secs;
}

static void
send_renew_request(struct dhcpc_ifc *difp)
{
	make_dhcpc_packet(difp, DHCP_REQUEST);

	difp->txpkt.ciaddr = difp->req_ip;

	add_requests(difp);

	/* Unicast send, through socket */
	dhcpc_send(difp);

	difp->secs += 2 + (arc4random() & 3);
	difp->rmsecs = difp->secs;
}

static void
send_rebind_request(struct dhcpc_ifc *difp)
{
	make_dhcpc_packet(difp, DHCP_REQUEST);

	difp->txpkt.ciaddr = difp->req_ip;

	add_requests(difp);

	dhcpc_write(difp);

	difp->secs += 2 + (arc4random() & 3);
	difp->rmsecs = difp->secs;
}

static void
send_release(struct dhcpc_ifc *difp)
{
	struct dhcphdr *txpkt = &difp->txpkt;

	make_dhcpc_packet(difp, DHCP_RELEASE);

	txpkt->ciaddr = difp->req_ip;

	dhcp_add_option(txpkt, DHCP_REQUESTED_IP,  sizeof(difp->req_ip), &difp->req_ip);
	dhcp_add_option(txpkt, DHCP_SERVER_ID, sizeof(difp->server), &difp->server);

	/* Unicast send, through socket */
	dhcpc_send(difp);
}

/* Add receiv filter to device */
static void
dhcpc_enable_device(struct dhcpc_ifc *ifp)
{
	if (ifp->dev_fd >= 0)
		ioctl(ifp->dev_fd, DHCPCIOCADDIFFR, 0);
	return;
}

static void
dhcpc_disable_device(struct dhcpc_ifc *ifp)
{
	if (ifp->dev_fd >= 0)
		ioctl(ifp->dev_fd, DHCPCIOCDELIFFR, 0);
	return;
}

static int
dhcpc_read(struct dhcpc_ifc *difp)
{
	char *buf = (char *)&difp->rxpkt;
	int n, datalen = 0;
	struct timeval tv;
	fd_set fds;
	int maxfd;

	/* Set event default to nothing */
	difp->event = 0;

	/* Select every one second */
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(difp->dev_fd, &fds);
	maxfd = difp->dev_fd;

	FD_SET(difp->efd, &fds);
	if (difp->efd > difp->dev_fd)
		maxfd = difp->efd;

	/* Wait for socket events */
	n = select(maxfd+1, &fds, NULL, NULL, &tv);
	if (n <= 0)
		return n;

	/* Check dhcp packet */
	if (FD_ISSET(difp->dev_fd, &fds)) {
		datalen = read(difp->dev_fd, buf, sizeof(difp->rxpkt));
		if (datalen > 0) {
			if (difp->rxpkt.xid   != difp->xid   || 
				    	difp->rxpkt.htype != difp->txpkt.htype || 
				    	difp->rxpkt.hlen  != difp->txpkt.hlen  || 
				    	bcmp( &difp->rxpkt.chaddr, &difp->txpkt.chaddr, difp->txpkt.hlen))
			{
				difp->event = 0;
			}
			else
			{
				difp->event = EVENT_PACKET;
			}
#if 0			
			if (difp->rxpkt.xid == difp->xid ||
			    difp->rxpkt.htype == difp->txpkt.htype ||
			    difp->rxpkt.hlen == difp->txpkt.hlen ||
			    memcmp(&difp->rxpkt.chaddr,
			           &difp->txpkt.chaddr,
			           difp->txpkt.hlen) == 0) {
				/* This is a packet event */
				difp->event = EVENT_PACKET;
			}
#endif			
		}
	}

	/* Check event */
	if (FD_ISSET(difp->efd, &fds)) {
		datalen = read(difp->efd, buf, sizeof(difp->rxpkt));
		if (datalen >= sizeof("FREE") &&
		    memcmp(buf, "FREE", sizeof("FREE")) == 0) {
			difp->event = EVENT_FREE;
		}
		else if (datalen >= sizeof("RENEW") &&
			memcmp(buf, "RENEW", sizeof("RENEW")) == 0) {
			difp->event = EVENT_RENEW;
		}
		else if (datalen >= sizeof("RELEASE") &&
			memcmp(buf, "RELEASE", sizeof("RELEASE")) == 0) {
			difp->event = EVENT_RELEASE;
		}
	}

	return datalen;
}

/*
 * State transfer functions.
 * We have,
 *    enter_init_state,
 *    enter_requesting_state,
 *    enter_bound_state,
 *    enter_renewing_state,
 *    enter_rebinding_state and
 *    leave_bound_state
 */
static void
enter_init_state(struct dhcpc_ifc *difp, int delay)
{
	/* Diable receive */
	dhcpc_disable_device(difp);

	difp->lease.timeout = time(0) + delay;//租期超时
	difp->state = DHCPC_STATE_INIT;

	//DHCPC_DBG("Enter DHCPC_INIT, idle for %d seconds", delay);
}

static void
enter_requesting_state(struct dhcpc_ifc *difp)
{
	/* Up the interface */
	ifup(difp->ifname);
	dhcpc_enable_device(difp);

	reset_countdown(difp);
	new_xid(difp);

	difp->state = DHCPC_STATE_REQUESTING;
	send_request(difp);

	DHCPC_DBG("DHCPC_STATE_REQUESTING init sending");
}

static void
set_state_to_bound(struct dhcpc_ifc *difp)
{
	difp->state = DHCPC_STATE_BOUND;

	/* detach recevie filter */
	dhcpc_disable_device(difp);
}

static char *
get_mask(char *mask,int maskNum)
{
	unsigned long int rel_mask = 0;
	
	if(NULL == mask)
	{
		return NULL;
	}
	
	rel_mask = 0xFFFFFFFF - (0x1<<(32-maskNum)) + 1;
	sprintf(mask,"%d.%d.%d.%d",(rel_mask>>24)&0xff,(rel_mask>>16)&0xff,(rel_mask>>8)&0xff,(rel_mask)&0xff);
	
	return mask;
}

static char *
get_static_router(char *dest,unsigned char *src)
{
	unsigned int pos = 0, netmask = 0,first_time = 1;
	char mask[20] = {0},destnet[20] = {0},thegw[20] = {0},value[100];
	
	if(NULL == dest || NULL == src)
	{
		return NULL;
	}

	while( src[pos] != '\0' ) 
	{
		memset(mask, 0, sizeof(mask));
		memset(destnet, 0, sizeof(destnet));
		memset(thegw, 0, sizeof(thegw));
		memset(value, 0, sizeof(value));
		netmask = src[pos++];
		
		if(NULL == get_mask(mask,netmask))
		{
			return NULL;
		}
		
		if( netmask <= 8 ) 
		{
			sprintf(destnet, "%u.0.0.0", src[pos++]);
		}
		else if( netmask <= 16 ) 
		{
			sprintf(destnet, "%u.%u.0.0", src[pos++],src[pos++]);
		}
		else if( netmask <= 24 ) 
		{
			sprintf(destnet, "%u.%u.%u.0", src[pos++],src[pos++],src[pos++]);
		}
		else
		{
			sprintf(destnet, "%u.%u.%u.%u", src[pos++],src[pos++],src[pos++],src[pos++]);
		}
		sprintf(thegw, "%u.%u.%u.%u", src[pos++],src[pos++],src[pos++],src[pos++]);
		/* dest_ip/dest_gw/net_mask*/
		if(first_time)
			sprintf(value,"%s/%s/%s",destnet,thegw,mask);
		else
			sprintf(value," %s/%s/%s",destnet,thegw,mask);
		
		first_time = 0;
		strncat(dest,value,strlen(value));		
	}

	return dest;
}

static char *
get_33_static_router(char *dest,unsigned char *src)
{
	unsigned int pos = 0,first_time = 1;
	char mask[20] = {0},destnet[20] = {0},thegw[20] = {0},value[100];
	
	if(NULL == dest || NULL == src)
	{
		return NULL;
	}
	
	while( src[pos] != '\0' ) 
	{
		memset(mask, 0, sizeof(mask));
		memset(destnet, 0, sizeof(destnet));
		memset(thegw, 0, sizeof(thegw));
		memset(value, 0, sizeof(value));
		
		if ((src[pos+3] & 0xff) != 0)		
			sprintf(mask,"%s","255.255.255.255");	
		else if ((src[pos+3] & 0xff) == 0 && (src[pos+2] & 0xff) != 0)		
			sprintf(mask,"%s","255.255.255.0");
		else if ((src[pos+3] & 0xff) == 0 && (src[pos+2] & 0xff) == 0 && (src[pos+1] & 0xff) != 0)		
			sprintf(mask,"%s","255.255.0.0");	
		else if ((src[pos+3]& 0xff) == 0 && (src[pos+2] & 0xff) == 0 && (src[pos+1] & 0xff) == 0 && (src[pos+0] & 0xff) != 0)		
			sprintf(mask,"%s","255.0.0.0");
		else
			sprintf(mask,"%s","0.0.0.0");			
		
		sprintf(destnet, "%u.%u.%u.%u", src[pos++],src[pos++],src[pos++],src[pos++]);
		sprintf(thegw, "%u.%u.%u.%u", src[pos++],src[pos++],src[pos++],src[pos++]);
		/* dest_ip/dest_gw/net_mask*/
		if(first_time)
			sprintf(value,"%s/%s/%s",destnet,thegw,mask);
		else
			sprintf(value," %s/%s/%s",destnet,thegw,mask);
		
		first_time = 0;
		strncat(dest,value,strlen(value));	
	}

	return dest;
}


static void
enter_bound_state(struct dhcpc_ifc *difp)
{
	int i, j;
	char *ptr;
	char *argv[] = {difp->script, "bound", NULL};
	//char *env[12] = {0};
	//char env_buf[1024];
	char *env[16] = {0};
	char env_buf[2048];

	struct in_addr mask;
	struct in_addr gw;
	struct in_addr addr;
	int num, num1;
	char *dns, *wins;
	char domain[256] = {0};
	char static_router[256] = {0};
	unsigned int static_router_tag = 0;
	char static_router_33[256] = {0};
	char static_router_121[256] = {0};
	char static_router_249[256] = {0};
	char wan_check_val[4];


	/* Set state back to bound */
	set_state_to_bound(difp);

	/* Reset lease and IP */
	new_lease(difp);

	/* Build sh command */
	i = 0;
	ptr = env_buf;

	/* interface */
	env[i++] = ptr;
	ptr += sprintf(ptr, "interface=%s", difp->ifname) + 1;

	/* ip */
	env[i++] = ptr;
	ptr += sprintf(ptr, "ip=%s", inet_ntoa(difp->rxpkt.yiaddr)) + 1;

	/* subnet */
	if (dhcp_get_option(&difp->rxpkt, DHCP_SUBNET, sizeof(mask), &mask)) {
		env[i++] = ptr;
		ptr += sprintf(ptr, "subnet=%s", inet_ntoa(mask)) + 1;
	}

	/* router */
	if (dhcp_get_option(&difp->rxpkt, DHCP_ROUTER, sizeof(gw), &gw)) {
		env[i++] = ptr;
		ptr += sprintf(ptr, "router=%s", inet_ntoa(gw)) + 1;
	}

	/* DNS */
	dns = (char *)dhcp_get_option(&difp->rxpkt, DHCP_DNS_SERVER, 0, NULL);
	if (dns) {
		num = (int)*(dns - 1);	/* TLV length */
		if ((num % 4) == 0 && (num = num/4) != 0) {
			char *tmp = ptr;
			for (j = 0; j < num; j++, dns += 4) {
				memcpy(&addr.s_addr, dns, 4);	/* Alignment to 4 */
				if (j == 0)
					tmp += sprintf(tmp, "dns=%s", inet_ntoa(addr));
				else
					tmp += sprintf(tmp, " %s", inet_ntoa(addr));
			}

			env[i++] = ptr;
			ptr += strlen(ptr) + 1;
		}
	}

	/* WINS */
	wins = (char *)dhcp_get_option(&difp->rxpkt, DHCP_WINS_SERVER, 0, NULL);
	if (wins) {
		num = (int)*(wins - 1);	/* TLV length */
		if ((num % 4) == 0 && (num = num/4) != 0) {
			char *tmp = ptr;
			for (j = 0; j < num; j++, wins += 4) {
				memcpy(&addr.s_addr, wins, 4);	/* Alignment to 4 */
				if (j == 0)
					tmp += sprintf(tmp, "wins=%s", inet_ntoa(addr));
				else
					tmp += sprintf(tmp, " %s", inet_ntoa(addr));
			}

			env[i++] = ptr;
			ptr += strlen(ptr) + 1;
		}
	}

	/* domain */
	dhcp_get_option(&difp->rxpkt, DHCP_DOMAIN_NAME, sizeof(domain), domain);
	if (domain[0] != 0) {
		env[i++] = ptr;
		ptr += sprintf(ptr, "domain=%s", domain) + 1;
	}

	/* Lease */
	env[i++] = ptr;
	ptr += sprintf(ptr, "expiry=%u", time(0) + difp->lease.expiry) + 1;

	env[i++] = ptr;
	ptr += sprintf(ptr, "lease=%u", difp->lease.expiry) + 1;

	env[i++] = ptr;
	ptr += sprintf(ptr, "t1=%u", difp->lease.t1) + 1;

	env[i++] = ptr;
	ptr += sprintf(ptr, "t2=%u", difp->lease.t2) + 1;

//hqw add
	dhcp_get_option(&difp->rxpkt, STATIC_ROUTER_121, sizeof(static_router_121), static_router_121);
	dhcp_get_option(&difp->rxpkt, STATIC_ROUTER_249, sizeof(static_router_249), static_router_249);
	dhcp_get_option(&difp->rxpkt, STATIC_ROUTER_33, sizeof(static_router_33), static_router_33);
	
	if (static_router_121[0] != 0) 
	{
		static_router_tag = STATIC_ROUTER_121;
		get_static_router(static_router,static_router_121);
	}
	else if(static_router_249[0] != 0)
	{
		static_router_tag = STATIC_ROUTER_249;
		get_static_router(static_router,static_router_249);
	}
	else if(static_router_33[0] != 0)
	{
		static_router_tag = STATIC_ROUTER_33;
		get_33_static_router(static_router,static_router_33);
	}

	if(static_router[0] != 0)
	{
		env[i++] = ptr;
		ptr += sprintf(ptr, "static_route=%s", static_router) + 1;
	}
//end

	DHCPC_DBG("DHCPC_BOUND get ip success");
#if 0 //pxy rm 2013.06.03
	/*add by ldm for stop dhcp -> pppoe,20130304*/
	nvram_set("wan0_check","0");
	/*end add*/
#endif	
	/* Run script */
	run_sh(argv[0], argv, env);
}

static void
do_deconfig(struct dhcpc_ifc *difp, int delay)
{
	char interface[64];
	char *argv[3] = {difp->script, "deconfig", NULL};
	char *env[2] = {interface, NULL};

	enter_init_state(difp, delay);

	/* rc deconfig */
	sprintf(env[0], "interface=%s", difp->ifname);

	run_sh(argv[0], argv, env);
	return;
}

static void
leave_bound_state(struct dhcpc_ifc *difp)
{
	/* Starting from init */
	DHCPC_DBG("IP from DHCP was expired!!\n");

	do_deconfig(difp, DHCPC_NO_DELAY);
}

static void
enter_renewing_state(struct dhcpc_ifc *difp)
{
	dhcpc_enable_device(difp);

	reset_countdown(difp);
	new_xid(difp);

	difp->state = DHCPC_STATE_RENEWING;
	send_renew_request(difp);

	DHCPC_DBG("DHCPC_STATE_RENEWING sending");
}

static void
enter_rebinding_state(struct dhcpc_ifc *difp)
{
	dhcpc_enable_device(difp);

	reset_countdown(difp);
	new_xid(difp);

	difp->state = DHCPC_STATE_REBINDING;
	send_rebind_request(difp);

	DHCPC_DBG("DHCPC_STATE_REBINDING sending");
}

/*
 * DHCPC state machine.
 * Based on the RFC, we have
 *     INIT,
 *     SELECTING,
 *     REQUESTING,
 *     BOUND,
 *     RENEWING and
 *     REBOUNDING
 */
static void
dhcpc_dummy(struct dhcpc_ifc *difp)
{
}

static void
dhcpc_state_init(struct dhcpc_ifc *difp)
{
	/* Patch here */ /*补丁这里*/
	if (difp->lease.timeout > time(0))
		return;

	/* Up the interface *//*起接口*/
	ifup(difp->ifname);
	dhcpc_enable_device(difp);

	reset_countdown(difp);
	new_xid(difp); /*生成随机数，用来标示消息传递*/

	difp->state = DHCPC_STATE_SELECTING;

	/* Send discover */
	send_discover(difp);
	DHCPC_DBG("DHCPC_DISCOVER sending");
}

static void
dhcpc_state_selecting(struct dhcpc_ifc *difp)
{
	unsigned char *msgtype;
	unsigned char *server_ip;

	if (difp->event != EVENT_PACKET) {
		/* Continue if not timed-out */  //如果没有超时
		if (--difp->rmsecs != 0)
			return;

		/* Check if countdown completed */ //检查倒计时完成
		if (--difp->countdown == 0) {
			enter_init_state(difp, DHCPC_REINIT_DELAY);//重初始化延迟
			return;
		}

		/* Send new discover packet */ //发送新的数据包
		DHCPC_DBG("DHCPC_DISCOVER sending");
		send_discover(difp);
		return;
	}

	DHCPC_DBG("DHCPC_OFFER received");
	recv_dhcp_offer_packet_tag = 1;
	dhcp_offer_tag = 1;
	if(dhcp_mode_check_tag == 1)
	{
		dhcp_mode_check_tag = 0;
		return;
	}
	if ((msgtype = dhcp_get_option(&difp->rxpkt, DHCP_MESSAGE_TYPE, 0, NULL))) {
		if (DHCP_OFFER == *msgtype) {
			if ((server_ip = dhcp_get_option(&difp->rxpkt, DHCP_SERVER_ID, 0, NULL)))
				memcpy(&difp->server, server_ip, 4);

			difp->req_ip = difp->rxpkt.yiaddr;
//add by roy+++
#ifdef __CONFIG_APCLIENT_DHCPC__
			if( 0 != strncmp(difp->ifname, "br",strlen("br")))
#endif
			{//only for wan port
				unsigned int wan_ip,wan_mask;
				wan_ip = (unsigned int)difp->rxpkt.yiaddr.s_addr;
				dhcp_get_option(&difp->rxpkt, DHCP_SUBNET, sizeof(wan_mask), &wan_mask);
				if(CGI_same_net_with_lan(wan_ip,wan_mask)){
					DHCPC_DBG("ERROR: WAN NET is same as LAN\n");
					nvram_set("err_check","11");//add by ldm
					enter_init_state(difp, DHCPC_NO_DELAY);
					return;
				}
			}		
//end---		
			enter_requesting_state(difp);
			//rec DHCP_OFFER packet tag,add by wwk20120929
			//recv_dhcp_offer_packet_tag = 1;
			
			return;
		}
	}

	return;
}

static void
dhcpc_state_requesting(struct dhcpc_ifc *difp)
{
	unsigned char *msgtype;
	unsigned char *server_ip;

	if (difp->event != EVENT_PACKET) {
		if (--difp->rmsecs != 0)
			return;

		/* Check if countdown completed */
		if (--difp->countdown == 0) {
			enter_init_state(difp, DHCPC_REINIT_DELAY);
			return;
		}

		/* Send request packet */
		DHCPC_DBG("DHCPC_STATE_REQUESTING sending");
		send_request(difp);
		return;
	}

	DHCPC_DBG("DHCPC_STATE_REQUESTING received");

	server_ip = dhcp_get_option(&difp->rxpkt, DHCP_SERVER_ID, 0, NULL);
	if ((msgtype = dhcp_get_option(&difp->rxpkt, DHCP_MESSAGE_TYPE, 0, NULL))) {
		if (DHCP_ACK == *msgtype &&
		    (difp->server.s_addr == 0 ||
		     !memcmp(&difp->server, server_ip, sizeof(difp->server)))) {
			if (difp->req_ip.s_addr == difp->rxpkt.yiaddr.s_addr) {
				unsigned int tag = 0;
				if (dhcp_get_option(&difp->rxpkt, DHCP_LEASE_TIME,
				    sizeof(tag), &tag)) {
					tag = ntohl(tag);

					DHCPC_DBG("DHCPC_STATE_REQUESTING lease = %u", tag);
					if (tag <= 25) {
						enter_init_state(difp, DHCPC_NO_DELAY);
						return;
					}
				}

				memcpy(&difp->server, server_ip, 4);
				enter_bound_state(difp);
				return;
			}
			else {
				DHCPC_DBG("assiged IP was not request IP");
			}
		}

		if (DHCP_NAK == *msgtype)
			DHCPC_DBG("NAK received");

		enter_init_state(difp, DHCPC_NO_DELAY);
		return;
	}

	return;
}

static void
dhcpc_state_bound(struct dhcpc_ifc *difp)
{
	/* Do nothing here */
}

static void
dhcpc_state_renewing(struct dhcpc_ifc *difp)
{
	unsigned char *msgtype;
	unsigned char *server_ip;

	if (difp->event != EVENT_PACKET) {
		if (--difp->rmsecs != 0)
			return;

		if (--difp->countdown == 0) {
			set_state_to_bound(difp);
			return;
		}

		/* Send renew request */
		DHCPC_DBG("DHCPC_STATE_RENEWING sending");
		send_renew_request(difp);
		return;
	}

	DHCPC_DBG("DHCPC_STATE_RENEWING received");

	if ((msgtype = dhcp_get_option(&difp->rxpkt, DHCP_MESSAGE_TYPE, 0, NULL))) {
		if (DHCP_ACK == *msgtype && difp->rxpkt.yiaddr.s_addr == difp->req_ip.s_addr) {
			if ((server_ip = dhcp_get_option(&difp->rxpkt, DHCP_SERVER_ID, 0, NULL)))
				memcpy(&difp->server, server_ip, 4);

			enter_bound_state(difp);
			return;
		}
		if (DHCP_NAK == *msgtype) {
			leave_bound_state(difp);
			return;
		}
	}
	return;
}

static void
dhcpc_state_rebinding(struct dhcpc_ifc *difp)
{
	unsigned char *msgtype;
	unsigned char *server_ip;

	if (difp->event != EVENT_PACKET) {
		if (--difp->rmsecs != 0)
			return;

		if (--difp->countdown == 0) {
			set_state_to_bound(difp);
			return;
		}

		/* Send rebound request */
		DHCPC_DBG("DHCPC_STATE_REBINDING sending");
		send_rebind_request(difp);
		return;
	}

	DHCPC_DBG("DHCPC_STATE_REBINDING received");

	if ((msgtype = dhcp_get_option(&difp->rxpkt, DHCP_MESSAGE_TYPE, 0, NULL))) {
		if (DHCP_ACK == *msgtype && difp->rxpkt.yiaddr.s_addr == difp->req_ip.s_addr) {
			if ((server_ip = dhcp_get_option(&difp->rxpkt, DHCP_SERVER_ID, 0, NULL)))
				memcpy(&difp->server, server_ip, 4);

			enter_bound_state(difp);
			return;
		}
		else if (DHCP_NAK == *msgtype) {
			leave_bound_state(difp);
			return;
		}
	}
	return;
}

/* Timeout function */
static void
do_timeout(struct dhcpc_ifc *difp)
{
	unsigned int now = (unsigned int)time(0);
	struct dhcp_lease *lease = &difp->lease;

	if (lease->timeout <= 0)
		return;

	if (difp->state < DHCPC_STATE_BOUND)
		return;

	/* Special patch for NTP *//*时间服务器*/
	if ((now - lease->time_last) > 631123200) {
		unsigned int delta = lease->time_last - lease->time_stamp;
		unsigned int gap = lease->timeout - lease->time_stamp;

		char interface[64];
		char expiry[64];
		char *env[] = {interface, expiry, NULL};
		char *argv[] = {difp->script, "lease", NULL};

		lease->time_stamp = now - delta;
		lease->timeout = lease->time_stamp + gap;

		/* Update to expiry */
		sprintf(env[0], "interface=%s", difp->ifname);
		sprintf(env[1], "expiry=%u", lease->time_stamp + lease->expiry);

		run_sh(argv[0], argv, env);
	}

	lease->time_last = now;
	if (lease->timeout > now)
		return;

	/*
	 * The following state change is only for
	 * BOUND, RENEWING and REBINDING
	 */
	lease->curr = lease->next;
	no_lease(difp);

	/* Set next timeout, the order is T1->T2->EX */
	if (lease->curr & DHCPC_LEASE_EX) {
		leave_bound_state(difp);
		return;
	}
	else if (lease->curr & DHCPC_LEASE_T2) {
		lease->next = DHCPC_LEASE_EX;
		lease->timeout = time(0) + (lease->expiry - lease->t2);
	}
	else if (lease->curr & DHCPC_LEASE_T1) {
		lease->next = DHCPC_LEASE_T2;
		lease->timeout = time(0) + (lease->t2 - lease->t1);
	}

	/* Set next state */
	if (lease->curr & DHCPC_LEASE_T2)
		enter_rebinding_state(difp);
	else if (lease->curr & DHCPC_LEASE_T1)
		enter_renewing_state(difp);
}

static void
do_release(struct dhcpc_ifc *difp)
{
	send_release(difp);

	do_deconfig(difp, DHCPC_RELEASE_DELAY);
}

static int
do_event(struct dhcpc_ifc *difp)
{
	switch (difp->event) {
	case EVENT_PACKET:
		/* Keep difp->event and let do_dhcpc handle it */
		return 0;

	case EVENT_FREE:
		do_release(difp);
		return -1;	/* End of program */

	case EVENT_RELEASE:
		DHCPC_DBG("DHCPC_RELEASE sending");
		do_release(difp);
		break;

	case EVENT_RENEW:
		if (difp->state == DHCPC_STATE_BOUND) {
			enter_renewing_state(difp);
		}
		else {
#ifndef __CONFIG_DHCPC_INIT_REBOOT_STATE_DISABLE__
			if (difp->req_ip.s_addr && (difp->state != DHCPC_STATE_INIT)){
				enter_requesting_state(difp);
			}
			else
#endif	/* __CONFIG_DHCPC_INIT_REBOOT_STATE_DISABLE__ */
			{

				enter_init_state(difp, DHCPC_NO_DELAY);
			}
		}
		no_lease(difp);
		break;

	default:
		break;
	}

	/* Clear event */
	difp->event = 0;
	return 0;
}

/* Shutdown this dhcpc interface */
static void
dhcpc_shutdown(struct dhcpc_ifc *difp)
{
	DHCPC_DBG("interface %s shutdown", difp->ifname);

	/* Close device */
	if (difp->dev_fd >= 0)
		close(difp->dev_fd);

	if (difp->efd >= 0)
		close(difp->efd);

	if (difp->hostname)
		free(difp->hostname);

	free(difp);
	return;
}

/*
 * Get interface mac
 */
static int
get_ifmac(char *ifname, char ifmac[6])
{
	int s;
	struct ifreq ifr;
	int rc = -1;

	/* Check interface address */
	if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFHWADDR, &ifr) != 0 ||
	    memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", 6) == 0) {
		goto errout;
	}

	/* Retrieve the ifmac */
	memcpy(ifmac, ifr.ifr_hwaddr.sa_data, 6);
	rc = 0;

errout:
	close(s);
	return 0;
}

static int
ifup(char *ifname)
{
	int s;
	struct ifreq ifr;
	int rc = -1;

	/* Check interface address */
	if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFFLAGS, &ifr) != 0)
		goto errout;

	if ((ifr.ifr_flags & IFF_UP) == 0) {
		ifr.ifr_flags |= IFF_UP;
		if (ioctl(s, SIOCSIFFLAGS, &ifr) != 0)
			goto errout;
	}

	/* OK, it's fine */
	rc = 0;
errout:
	close(s);
	return rc;
}

/* Initialize this dhcpc interface */
static int
dhcpc_init(struct dhcpc_ifc *difp, struct dhcpc_config *config)
{
	char *ifname = config->ifname;
	char name[64];
	int len;
	int ifidx;

	DHCPC_DBG("interface %s init", ifname);

	memset(difp, 0, sizeof(*difp));
	difp->dev_fd = -1;
	difp->efd = -1;

	strcpy(difp->ifname, config->ifname);
	strcpy(difp->script, config->script);

	/* Get physical MAC addr */
	if (get_ifmac(ifname, difp->macaddr) != 0)
		return -1;

	/* Open dhcpc device fd */
	ifidx = if_nametoindex(difp->ifname);
	if (ifidx == 0)
		return -1;

	difp->efd = open_udp_socket("127.0.0.1", DHCPC_IPC_PORT+ifidx);
	if (difp->efd < 0)
		return -1;

	sprintf(name, "%s/%s", DHCPC_DEVNAME, ifname);
	difp->dev_fd = open(name, O_RDWR);
	if (difp->dev_fd < 0) {
		close(difp->efd);
		difp->efd = -1;
		return -1;
	}

	/* Do initialization */
	len = strlen(config->hostname);
	if (len) {
		difp->hostname = (char *)malloc(len+1);
		if (difp->hostname)
			strcpy(difp->hostname, config->hostname);
	}
	difp->mtu = config->mtu;
	difp->req_ip = config->req_ip;
	difp->server.s_addr = 0;
	no_lease(difp);

	/* Default set to INIT state */
	enter_init_state(difp, DHCPC_NO_DELAY);
	return 0;
}

/*
 * Main loop of dhcpc state machine
 */
void
dhcpc_mainloop(struct dhcpc_config *config)
{
	/* Do init */
	struct dhcpc_ifc *difp;

	difp = (struct dhcpc_ifc *)malloc(sizeof(*difp));
	if (difp == NULL) {
		DHCPC_DBG("interface %s malloc failed!", config->ifname);
		return;
	}

	if (dhcpc_init(difp, config) != 0) {
		DHCPC_DBG("interface %s init failed!", config->ifname);
		goto errout;
	}

	/* Main loop */
	while (1) {
		/* Read packet or message */
		dhcpc_read(difp);

		/* lease time out machine */
		do_timeout(difp);

		/* Process event */
		if (do_event(difp) != 0)
			break;

		/* Do dhcpc state machine */
		dhcpc_state_machine[difp->state].func(difp);
	}

errout:
	dhcpc_shutdown(difp);
	return;
}
