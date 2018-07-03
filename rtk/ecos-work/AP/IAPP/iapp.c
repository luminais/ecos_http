/*
 * User space daemon for IEEE 802.11f Inter-Access Point Protocol (IAPP)
 * Copyright (c) 2003 by Realtek Semiconductor
 * Written by Jimmy Lin <jimmylin@realtek.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* TODO:
 * - add support for MOVE-notify and MOVE-response (this requires support for
 *   finding out IP address for previous AP using RADIUS)
 * - add support for Send- and ACK-Security-Block to speedup IEEE 802.1X during
 *   reassociation to another AP
 * - implement counters etc. for IAPP MIB
 * - verify endianness of fields in IAPP messages; are they big-endian as
 *   used here?
 */
#ifdef __ECOS
#include <network.h>
#include "../apmib/apmib.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifndef __ECOS
#ifdef USE_KERNEL_HEADERS
#include <linux/if_packet.h>
#else /* USE_KERNEL_HEADERS */
#include <netpacket/packet.h>
#endif /* USE_KERNEL_HEADERS */
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#ifndef __ECOS
#include <linux/wireless.h>
#else
#include <net/if.h>			/* for IFNAMSIZ and co... */
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#endif
#include "common.h"
#include "iapp.h"
#include "misc.h"

#ifdef __ECOS
#define CYGNUM_IAPP_THREAD_PRIORITY  16

unsigned char iapp_stack[16*1024];
cyg_handle_t iapp_thread_handle;
cyg_thread iapp_thread_obj;


unsigned char iapp_recv_stack[12*1024];
cyg_handle_t iapp_recv_thread_handle;
cyg_thread iapp_recv_thread_obj;

cyg_mbox iapp_mbox;
cyg_handle_t iapp_mbox_hdl;

static int iapp_run_flag=0;
#ifdef HAVE_SYSTEM_REINIT
	static int iapp_quitting=0;
	static int iapp_recv_quitting=0;
#define tick_to_msec(tick) ((u16_t)((tick)*10+1)) 
#define msec_to_tick(msec) ((cyg_tick_count_t)(msec+9)/10)
#endif
typedef struct _msg_hdr
{
	int msg_len;
	char* msg_buf;
}msg_hdr_t;

void creat_iapp(char *arg);
static int get_iapp_parms(char *arg);
void iapp_recv();
void creat_iapp_recv();
#if !defined(HAVE_NOETH)
extern struct eth_drv_sc rltk819x_sc0;
extern int rltk819x_send_eth(struct eth_drv_sc *sc, unsigned char *data, int size);
#endif
#endif

/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP			0x0001          /* route usable                 */
#define RTF_GATEWAY		0x0002          /* destination is a gateway     */

// local variable
static char *pidfile = "/var/run/iapp.pid";
static char *routefile = "/proc/net/route";
static struct iapp_context iapp_cnt;
static struct iapp_context *hapd = &iapp_cnt;
extern char *fwVersion;

#ifndef __ECOS
static int pidfile_acquire(char *pidfile)
{
	int pid_fd;

	if(pidfile == NULL)
		return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) 
		printf("Unable to open pidfile %s\n", pidfile);
	else 
		lockf(pid_fd, F_LOCK, 0);

	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

static int iapp_init_fifo()
{
/* Here is an assumption that the fifo is created already by iwcontrol
 */
	int flags;
	struct stat status;

	if(stat(DAEMON_FIFO, &status) == 0)
		unlink(DAEMON_FIFO);
	if((mkfifo(DAEMON_FIFO, FILE_MODE) < 0)){
		printf("mkfifo %s fifo error: %s!\n", DAEMON_FIFO, strerror(errno));
		return -1;
	}

	hapd->readfifo = open(DAEMON_FIFO, O_RDONLY, 0);
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "hapd->readfifo = %d\n", hapd->readfifo);

	if ((flags = fcntl(hapd->readfifo, F_GETFL, 0)) < 0) {
		printf("F_GETFL: error\n");
		return -1;
	}
	else {
		flags |= O_NONBLOCK;
		if ((flags = fcntl(hapd->readfifo, F_SETFL, flags)) < 0) {
			printf("F_SETFL: error\n");
			return -1;
		}
	}

	return 0;
}

#endif
static void check_multicast_route()
{
	char buff[1024], iface[16];
	char net_addr[128], gate_addr[128], mask_addr[128];
	int num, iflags, refcnt, use, metric, mss, window, irtt;
	FILE *fp = fopen(routefile, "r");
	char *fmt;
	int found = 0;

	if (!fp) {
		printf("Open %s file error.\n", routefile);
		return;
	}

	fmt = "%16s %128s %128s %X %d %d %d %128s %d %d %d";

	while (fgets(buff, 1023, fp)) {
		num = sscanf(buff, fmt, iface, net_addr, gate_addr,
			&iflags, &refcnt, &use, &metric, mask_addr, &mss, &window, &irtt);
		if (num < 10 || !(iflags & RTF_UP) || strcmp(iface, hapd->iapp_iface))
			continue;
		if (!strcmp(net_addr, "E0000000")) {
			found = 1;
			break;
		}
	}

	fclose(fp);
	if (!found) {
		sprintf(buff, "route add -net 224.0.1.0 netmask 255.255.255.0 dev %s", hapd->iapp_iface);
		system(buff);
	}
}


static void iapp_send_add_d3(u8 *sta, u16 id)
{
	char buf[128];
	struct iapp_hdr *hdr;
	struct iapp_add_notify *add;
	struct sockaddr_in addr;
	int n;

	/* Send IAPP-ADD Packet to remove possible association from other APs
	 */

	hdr = (struct iapp_hdr *) buf;
	hdr->version = IAPP_VERSION;
	hdr->command = IAPP_CMD_ADD_notify;
	hdr->identifier = host_to_be16(id);
	hdr->length = host_to_be16(sizeof(*hdr) + sizeof(*add));

	add = (struct iapp_add_notify *) (hdr + 1);
	add->addr_len = ETH_ALEN;
	add->reserved = 0;
	memcpy(add->mac_addr, sta, ETH_ALEN);
	add->seq_num = 0;
	
	/* Send to local subnet address (UDP port IAPP_PORT) */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = hapd->iapp_broadcast.s_addr;
	addr.sin_port = htons(IAPP_PORT_D3);
	if ((n = sendto(hapd->iapp_udp_sock, buf, (char *) (add + 1) - buf, 0,
		   (struct sockaddr *) &addr, sizeof(addr))) < 0)
		perror("sendto[IAPP-ADD]");

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE,
	       "Broadcast %d byte IAPP frame of sta %02x%02x%02x%02x%02x%02x\n",
	       n, sta[0], sta[1], sta[2], sta[3], sta[4], sta[5]);
}


static void iapp_send_add_d5(u8 *sta, u16 id)
{
	char buf[128];
	struct iapp_hdr *hdr;
	struct iapp_add_notify *add;
	struct sockaddr_in addr;
	int n;

	/* Send IAPP-ADD Packet to remove possible association from other APs
	 */

	hdr = (struct iapp_hdr *) buf;
	hdr->version = IAPP_VERSION;
	hdr->command = IAPP_CMD_ADD_notify;
	hdr->identifier = host_to_be16(id);
	hdr->length = host_to_be16(sizeof(*hdr) + sizeof(*add));

	add = (struct iapp_add_notify *) (hdr + 1);
	add->addr_len = ETH_ALEN;
	add->reserved = 0;
	memcpy(add->mac_addr, sta, ETH_ALEN);
	add->seq_num = 0;
	
	/* check routing entry of multicast and set if no */
	//check_multicast_route();
	
	/* Send to local subnet address (UDP port IAPP_PORT) */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (!inet_aton(IAPP_MULTICAST_ADDR, &addr.sin_addr)) {
		perror("inet_aton[IAPP-ADD]");
		return;
	}
	addr.sin_port = htons(IAPP_PORT_D5);
	if ((n = sendto(hapd->iapp_mltcst_sock, buf, (char *) (add + 1) - buf, 0,
		   (struct sockaddr *) &addr, sizeof(addr))) < 0){
		   ;
			//perror("sendto[IAPP-ADD]");
		}

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE,
	       "Multicast %d byte IAPP frame of sta %02x%02x%02x%02x%02x%02x\n",
	       n, sta[0], sta[1], sta[2], sta[3], sta[4], sta[5]);
}


static void iapp_send_layer2_update(u8 *sta)
{
	struct iapp_layer2_update msg;
	int n;

	/* Send Level 2 Update Frame to update forwarding tables in layer 2
	 * bridge devices */

	/* 802.2 Type 1 Logical Link Control (LLC) Exchange Identifier (XID)
	 * Update response frame; IEEE Std 802.2-1998, 5.4.1.2.1 */

	memset(msg.da, 0xff, ETH_ALEN);
	memcpy(msg.sa, sta, ETH_ALEN);
	msg.len = host_to_be16(8);
	msg.dsap = 0;
	msg.ssap = 0;
	msg.control = 0xaf; /* XID response lsb.1111F101.
			     * F=0 (no poll command; unsolicited frame) */
	msg.xid_info[0] = 0x81; /* XID format identifier */
	msg.xid_info[1] = 1; /* LLC types/classes: Type 1 LLC */
	msg.xid_info[2] = 1 << 1; /* XID sender's receive window size (RW)
				   * FIX: what is correct RW with 802.11? */

#ifndef __ECOS
	if ((n = send(hapd->iapp_packet_sock, &msg, sizeof(msg), 0)) < 0)
		perror("send[L2 Update]");
#else
	#if !defined(HAVE_NOETH)
	if(!strcmp(hapd->iapp_iface,"eth0"))
		n = rltk819x_send_eth(&rltk819x_sc0, &msg, sizeof(msg));
	else
	#endif
	{
		diag_printf("Unknown interface name\n");
		return;
	}
#endif
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS,
	       "Send %d byte L2 Update frame of sta %02x%02x%02x%02x%02x%02x\n",
	       n, sta[0], sta[1], sta[2], sta[3], sta[4], sta[5]);
}


static void iapp_new_station(u8 *sta)
{
	if (hapd->spcApply & APPLY_D3)
		iapp_send_add_d3(sta, hapd->iapp_identifier);
	if (hapd->spcApply & APPLY_D5)
		iapp_send_add_d5(sta, hapd->iapp_identifier);
	hapd->iapp_identifier++;
	iapp_send_layer2_update(sta);
}


static void iapp_process_add_notify(struct sockaddr_in *from,
				    struct iapp_hdr *hdr, int len)
{
	struct iapp_add_notify *add = (struct iapp_add_notify *) (hdr + 1);
	unsigned char para[32];
	struct iwreq wrq;
	int i = 0;

	if (len != sizeof(*add)) {
		printf("Invalid IAPP-ADD packet length %d (expected %d)\n",
		       len, sizeof(*add));
		return;
	}

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
	       "Received IAPP-ADD for STA " "%02x%02x%02x%02x%02x%02x" " (seq# %d) from %s:%d\n",
	       add->mac_addr[0], add->mac_addr[1], add->mac_addr[2], add->mac_addr[3], add->mac_addr[4],
	       add->mac_addr[5], be_to_host16(add->seq_num),
	       inet_ntoa(from->sin_addr), ntohs(from->sin_port));

	/* TODO: could use seq_num to try to determine whether last association
	 * to this AP is newer than the one advertised in IAPP-ADD. Although,
	 * this is not really a reliable verification. */

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE,
	       "Removing STA " "%02x%02x%02x%02x%02x%02x" " due to IAPP-ADD notification from "
	       "%s\n", add->mac_addr[0], add->mac_addr[1], add->mac_addr[2], add->mac_addr[3], add->mac_addr[4],
	       add->mac_addr[5], inet_ntoa(from->sin_addr));

	if (hapd->driver_ver == DRIVER_8180)
	{
		memset(para, 0, 32);
		sprintf(para, "delsta=%02x%02x%02x%02x%02x%02x",
			add->mac_addr[0],
			add->mac_addr[1],
			add->mac_addr[2],
			add->mac_addr[3],
			add->mac_addr[4],
			add->mac_addr[5]);
		wrq.u.data.pointer = para;
		wrq.u.data.length = strlen(para);
		strncpy(wrq.ifr_name, hapd->wlan_iface[i], IFNAMSIZ);
		ioctl(hapd->wlan_sock[i], 0x89f6/* RTL8180_IOCTL_SET_WLAN_PARA */, &wrq);
	}
	else
	{
		while(strlen(hapd->wlan_iface[i]) != 0)
		{
			memset(para, 0, 32);
			sprintf(para, "%02x%02x%02x%02x%02x%02xno", add->mac_addr[0], add->mac_addr[1],
			add->mac_addr[2], add->mac_addr[3], add->mac_addr[4], add->mac_addr[5]);
			wrq.u.data.pointer = para;
			wrq.u.data.length = strlen(para);
			strncpy(wrq.ifr_name, hapd->wlan_iface[i], IFNAMSIZ);
			/*Only one socket for ECOS*/
			ioctl(hapd->wlan_sock[0], 0x89f7/* RTL8185_IOCTL_DEL_STA */, &wrq);
			i++;
		}
	}
}


static void iapp_receive_udp(int sockGot)
{
	int len, hlen;
	unsigned char buf[128];
	struct sockaddr_in from;
	socklen_t fromlen;
	struct iapp_hdr *hdr;

	/* Handle incoming IAPP frames (over UDP/IP) */

	fromlen = sizeof(from);
	len = recvfrom(sockGot, buf, sizeof(buf), 0,
		       (struct sockaddr *) &from, &fromlen);
	if (len < 0)
		perror("recvfrom");

	if (from.sin_addr.s_addr == hapd->iapp_own.s_addr)
		return; /* ignore own IAPP messages */

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		      "Received %d byte IAPP frame from %s\n",
		      len, inet_ntoa(from.sin_addr));

	if (len < sizeof(*hdr)) {
		printf("Too short IAPP frame (len=%d)\n", len);
		return;
	}

	hdr = (struct iapp_hdr *) buf;
	hlen = be_to_host16(hdr->length);
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		      "IAPP: version=%d command=%d id=%d len=%d\n",
		      hdr->version, hdr->command,
		      be_to_host16(hdr->identifier), hlen);
	if (hlen > len) {
		printf("Underflow IAPP frame (hlen=%d len=%d)\n", hlen, len);
		return;
	}
	if (hlen < len) {
		printf("Ignoring %d extra bytes from IAPP frame\n",
		       len - hlen);
		len = hlen;
	}

	if (hdr->command == IAPP_CMD_ADD_notify)
		iapp_process_add_notify(&from, hdr, hlen - sizeof(*hdr));
	else
		printf("Unknown IAPP command %d\n", hdr->command);
}


static void do_daemon()
{
	fd_set netFD;
	int selret, nRead, max_sock;

	// while loop to listen socket and check event
#ifdef HAVE_SYSTEM_REINIT
	while(!iapp_quitting)
#else
	while (1)
#endif
	{
		max_sock = 0;
		FD_ZERO(&netFD);
#ifdef HAVE_SYSTEM_REINIT		
		struct timeval	timeofday={1,0};
#endif
		if (hapd->spcApply & APPLY_D3) {
			FD_SET(hapd->iapp_udp_sock, &netFD);
			max_sock = (max_sock > hapd->iapp_udp_sock)? max_sock : hapd->iapp_udp_sock;
		}

		if (hapd->spcApply & APPLY_D5) {
			FD_SET(hapd->iapp_mltcst_sock, &netFD);
			max_sock = (max_sock > hapd->iapp_mltcst_sock)? max_sock : hapd->iapp_mltcst_sock;
		}
#ifndef __ECOS
		FD_SET(hapd->readfifo, &netFD);
		max_sock = (max_sock > hapd->readfifo)? max_sock : hapd->readfifo;
#endif
#ifdef HAVE_SYSTEM_REINIT
		selret = select(max_sock+1, &netFD, NULL, NULL, &timeofday);
#else
		selret = select(max_sock+1, &netFD, NULL, NULL, NULL);
#endif
		if (selret > 0)
		{
			
			if (FD_ISSET(hapd->iapp_udp_sock, &netFD)){
				iapp_receive_udp(hapd->iapp_udp_sock);
			}

			if (FD_ISSET(hapd->iapp_mltcst_sock, &netFD)){
				iapp_receive_udp(hapd->iapp_mltcst_sock);
			}
#ifndef __ECOS
			if (FD_ISSET(hapd->readfifo, &netFD))
			{
				nRead = read(hapd->readfifo, hapd->RecvBuf, MAX_MSG_SIZE);
				if (nRead > 0)
				{
					u8 msg_type = *(hapd->RecvBuf + FIFO_HEADER_LEN);
					HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "Fifo msg type: %d\n", msg_type);

					if (msg_type == DOT11_EVENT_ASSOCIATION_IND) {
						DOT11_ASSOCIATION_IND *msg = (DOT11_ASSOCIATION_IND *)(hapd->RecvBuf + FIFO_HEADER_LEN);
						iapp_new_station(msg->MACAddr);
					}
					else if (msg_type == DOT11_EVENT_REASSOCIATION_IND) {
						DOT11_REASSOCIATION_IND *msg = (DOT11_REASSOCIATION_IND *)(hapd->RecvBuf + FIFO_HEADER_LEN);
						iapp_new_station(msg->MACAddr);
					}
					else {
						// messages that we don't handle and drop
					}
				}
			}
#endif
		}
#ifdef HAVE_SYSTEM_REINIT
		timeofday.tv_sec = 1;
		timeofday.tv_usec = 0;
#endif

	}
}

#ifdef __ECOS
int iapp_main(cyg_addrword_t data)
#else
int main(int argc, char *argv[])
#endif
{
	iapp_run_flag=1;
	struct ifreq ifr;
#ifndef __ECOS
	struct sockaddr_ll addr;
	int ifindex;
#endif
	int one;
	struct sockaddr_in *paddr, uaddr;
	int opt, ttl = 64, loop = 0;
	struct ip_mreq multiaddr;
	struct in_addr adtp;
	struct iwreq wrq;
	char tmpbuf[32];
	int i;
	int on = 1;
	
#ifndef __ECOS
	// destroy old process and create a PID file
	{
		int pid_fd;
		FILE *fp;
		char line[20];
		pid_t pid;

		if ((fp = fopen(pidfile, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if (sscanf(line, "%d", &pid)) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
		}
		pid_fd = pidfile_acquire(pidfile);
		if (pid_fd < 0)
			return 0;

		if (daemon(0,1) == -1) {		
			printf("fork iapp error!\n");
			exit(1);
		}
		pidfile_write_release(pid_fd);
	}
#endif


// parse arguments
	memset(hapd->iapp_iface, 0, sizeof(hapd->iapp_iface));
	memset(hapd->wlan_iface, 0, sizeof(hapd->wlan_iface));
	hapd->spcApply = APPLY_D3 | APPLY_D5;
	hapd->debug = HOSTAPD_DEBUG_NO;
#ifndef __ECOS	

	while ((opt = getopt(argc, argv, "-d:n:")) > 0) {
		switch (opt) {
			case 1:
				if (!strncmp(optarg, "br", 2) ||
					!strncmp(optarg, "eth", 3) ||
					!strncmp(optarg, "wlan", 4)) {
					if (strlen(hapd->iapp_iface) == 0)
						strcpy(hapd->iapp_iface, optarg);
					else {
						i = 0;
						while(strlen(hapd->wlan_iface[i]) != 0)
							i++;
						strcpy(hapd->wlan_iface[i], optarg);
					}
				}
				else {
					printf("Wrong interface\n");
					return -1;
				}
				break;
			case 'd':
				hapd->debug = atoi(optarg);
				if (hapd->debug > HOSTAPD_DEBUG_MSGDUMPS)
					hapd->debug = HOSTAPD_DEBUG_MSGDUMPS;
				break;
			case 'n':
				if (!strcmp(optarg, "d3"))
					hapd->spcApply &= ~APPLY_D3;
				else if (!strcmp(optarg, "d5"))
					hapd->spcApply &= ~APPLY_D5;
				else {
					printf("Usage: iapp interface [-d debug_level] [-n d3|d5] [wlan0 ...]\n");
					return -1;
				}
				break;
			default:
				printf("Usage: iapp interface [-d debug_level] [-n d3|d5] [wlan0 ...]\n");
				return -1;
		}
	}
#else
	//hapd->debug = 100;
	if(get_iapp_parms((char *)data) < 0){
		diag_printf("IAPP: parmeters is invalid!\n");
		return -1;
	} 
#endif

	if (strlen(hapd->iapp_iface) == 0) {
		printf("IAPP:invalid iapp interface\n");
		//printf("Usage: iapp interface [-d debug_level] [-n d3|d5] [wlan0 ...]\n");
		return -1;
	}

	// at least one wlan interface
	if (strlen(hapd->wlan_iface[0]) == 0)
		strcpy(hapd->wlan_iface[0], "wlan0");

#ifndef __ECOS // message box replace it 
	// fifo initialization
	if (iapp_init_fifo() < 0) {
		printf("Init fifo fail.\n");
		return -1;
	}
#endif
	/* TODO:
	 * open socket for sending and receiving IAPP frames over TCP
	 */

	/*
	 * create broadcast socket
	 */
	hapd->iapp_udp_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (hapd->iapp_udp_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		return -1;
	}
	/*Get Lan address*/
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, hapd->iapp_iface, sizeof(ifr.ifr_name));
#ifndef __ECOS
	while (ioctl(hapd->iapp_udp_sock, SIOCGIFINDEX, &ifr) != 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		       "iapp: ioctl(SIOCGIFINDEX) failed!\n");
		sleep(1);
	}
	ifindex = ifr.ifr_ifindex;
#endif
	while (ioctl(hapd->iapp_udp_sock, SIOCGIFADDR, &ifr) != 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		       "iapp: ioctl(SIOCGIFADDR) failed!\n");
		sleep(1);
	}
	paddr = (struct sockaddr_in *) &ifr.ifr_addr;
	if (paddr->sin_family != AF_INET) {
		printf("Invalid address family %i (SIOCGIFADDR)\n",
		       paddr->sin_family);
		goto iapp_err;
	}
	hapd->iapp_own.s_addr = paddr->sin_addr.s_addr;
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "Got ip addr %s\n", inet_ntoa(hapd->iapp_own));

	if (ioctl(hapd->iapp_udp_sock, SIOCGIFBRDADDR, &ifr) != 0) {
		perror("ioctl(SIOCGIFBRDADDR)");
		goto iapp_err;
	}
	paddr = (struct sockaddr_in *) &ifr.ifr_addr;
	if (paddr->sin_family != AF_INET) {
		printf("Invalid address family %i (SIOCGIFBRDADDR)\n",
		       paddr->sin_family);
		goto iapp_err;
	}
	hapd->iapp_broadcast.s_addr = paddr->sin_addr.s_addr;
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "Got bdcst addr %s\n", inet_ntoa(hapd->iapp_broadcast));

	one = 1;
	if (setsockopt(hapd->iapp_udp_sock, SOL_SOCKET, SO_BROADCAST,
		       (char *) &one, sizeof(one)) < 0) {
		perror("setsockopt[SOL_SOCKET,SO_BROADCAST]");
		goto iapp_err;
	}

	memset(&uaddr, 0, sizeof(uaddr));
	uaddr.sin_family = AF_INET;
	uaddr.sin_port = htons(IAPP_PORT_D3);
	if (bind(hapd->iapp_udp_sock, (struct sockaddr *) &uaddr,
		 sizeof(uaddr)) < 0) {
		perror("bind[UDP]");
		goto iapp_err;
	}

	/*
	 * create multicast socket
	 */
	hapd->iapp_mltcst_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (hapd->iapp_mltcst_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		goto iapp_err;
	}
#if 1
	// Join a multicast group IAPP_MULTICAST_ADDR on this interface
	inet_aton(IAPP_MULTICAST_ADDR, &adtp);
	multiaddr.imr_multiaddr.s_addr = adtp.s_addr;
	multiaddr.imr_interface.s_addr = hapd->iapp_own.s_addr;//htonl(INADDR_ANY);
#endif
	while(1) {
#ifndef __ECOS // Need to  check it with huxin
		char cmd[128];
		sprintf(cmd, "route delete -net 224.0.0.0 netmask 240.0.0.0 dev %s", hapd->iapp_iface);
		system(cmd);
		sprintf(cmd, "route add -net 224.0.0.0 netmask 240.0.0.0 dev %s", hapd->iapp_iface);
		system(cmd);
#endif
		if (setsockopt(hapd->iapp_mltcst_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
				&multiaddr, sizeof(struct ip_mreq)) >= 0)
			break;
		sleep(1);
	}
	
	if(setsockopt(hapd->iapp_mltcst_sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0)
	{
		goto iapp_err;
	}	
	if( setsockopt( hapd->iapp_mltcst_sock, SOL_SOCKET, SO_REUSEADDR,(char *)&on, sizeof(on) ) != 0 ) {

        goto iapp_err;
	}
	#if 0
	if(setsockopt(hapd->iapp_mltcst_sock, SOL_SOCKET, SO_BINDTODEVICE, "eth0", strlen("eth0")) < 0)
	{
	
		 goto iapp_err;
	}
	#endif
	// don't loopback
	if (setsockopt(hapd->iapp_mltcst_sock, IPPROTO_IP, IP_MULTICAST_LOOP,
		       &loop, sizeof(loop)) < 0) {
		perror("setsockopt[IPPROTO_IP, IP_MULTICAST_LOOP]");
		goto iapp_err;
	}

	// set TTL to 64
	if (setsockopt(hapd->iapp_mltcst_sock, IPPROTO_IP, IP_MULTICAST_TTL,
		       &ttl, sizeof(ttl)) < 0) {
		perror("setsockopt[IPPROTO_IP, IP_MULTICAST_TTL]");
		goto iapp_err;
	}

	memset(&uaddr, 0, sizeof(uaddr));
	uaddr.sin_family = AF_INET;
	uaddr.sin_port = htons(IAPP_PORT_D5);
	uaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(hapd->iapp_mltcst_sock, (struct sockaddr *) &uaddr,
		 sizeof(uaddr)) < 0) {
		perror("bind[UDP]");
		goto iapp_err;
	}
	
#ifndef __ECOS
	/*
	 * create layer 2 socket
	 */
	hapd->iapp_packet_sock = socket(PF_PACKET, SOCK_RAW, 0);
	if (hapd->iapp_packet_sock < 0) {
		perror("socket[PF_PACKET,SOCK_RAW]");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifindex;
	if (bind(hapd->iapp_packet_sock, (struct sockaddr *) &addr,
		 sizeof(addr)) < 0) {
		perror("bind[PACKET]");
		return -1;
	}
#endif
	/*
	 * create wlan sockets
	 */
	 /*So many socket, ECOS just one??*/
#ifndef __ECOS
	i = 0;
	while(strlen(hapd->wlan_iface[i]) != 0)
	{
		hapd->wlan_sock[i] = socket(PF_INET, SOCK_DGRAM, 0);
		if (hapd->wlan_sock[i] < 0) {
			perror("socket[PF_INET,SOCK_DGRAM]");
			goto iapp_err;
		}
		strncpy(wrq.ifr_name, hapd->wlan_iface[i], IFNAMSIZ);
		if (ioctl(hapd->wlan_sock[i], SIOCGIWNAME, &wrq) < 0) {
			sprintf(tmpbuf, "ioctl[%s,SIOCGIWNAME]", hapd->wlan_iface[i]);
			perror(tmpbuf);
			goto iapp_err;
		}
		i++;
	}
#else
	hapd->wlan_sock[0] = socket(PF_INET, SOCK_DGRAM, 0);
	if (hapd->wlan_sock[0] < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		goto iapp_err;
	}
#endif
#ifndef __ECOS
	// determine which driver used
	{
		FILE *fp;
		if((fp = fopen("/proc/rtl8180/status", "r")) != NULL) {
			fclose(fp);
			hapd->driver_ver = DRIVER_8180;
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "The WLAN driver is RTK_8180\n");
		}
		else {
			hapd->driver_ver = DRIVER_8185;
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "The WLAN driver is RTK_8185\n");
		}
	}
#else
		hapd->driver_ver = DRIVER_8185;
#endif
	// Show message on console
	printf("IEEE 802.11f (IAPP) using interface %s (%s)\n",
		   hapd->iapp_iface, fwVersion);

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE, "using wlan interface:");
	i = 0;
	while(strlen(hapd->wlan_iface[i]) != 0)
	{
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE, " %s", hapd->wlan_iface[i]);
		i++;
	}
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE, "\n");
#ifdef __ECOS
	creat_iapp_recv();
#endif
	do_daemon();
#ifdef HAVE_SYSTEM_REINIT
	iapp_cleanup();
	iapp_quitting = 0;
#endif
	return 0;
iapp_err:
	if(hapd ->iapp_mltcst_sock >= 0)
		close(hapd ->iapp_mltcst_sock);
	if(hapd ->iapp_udp_sock >= 0)
		close(hapd ->iapp_udp_sock);
	if(hapd ->wlan_sock[0] >= 0)
		close(hapd ->wlan_sock[0]);
	return -1;
}
static void check_lan_ip_state()
{
	struct ifreq ifr;
	struct sockaddr_in *paddr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, hapd->iapp_iface, sizeof(ifr.ifr_name));
	if (ioctl(hapd->iapp_udp_sock, SIOCGIFADDR, &ifr) != 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		       "iapp: ioctl(SIOCGIFADDR) failed!\n");
		return;
	}
	paddr = (struct sockaddr_in *) &ifr.ifr_addr;
	if (paddr->sin_family != AF_INET) {
		printf("Invalid address family %i (SIOCGIFADDR)\n",
		       paddr->sin_family);
		return ;
	}
	hapd->iapp_own.s_addr = paddr->sin_addr.s_addr;
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "Got ip addr %s\n", inet_ntoa(hapd->iapp_own));

	if (ioctl(hapd->iapp_udp_sock, SIOCGIFBRDADDR, &ifr) != 0) {
		perror("ioctl(SIOCGIFBRDADDR)");
		return ;
	}
	paddr = (struct sockaddr_in *) &ifr.ifr_addr;
	if (paddr->sin_family != AF_INET) {
		printf("Invalid address family %i (SIOCGIFBRDADDR)\n",
		       paddr->sin_family);
		return ;
	}
	hapd->iapp_broadcast.s_addr = paddr->sin_addr.s_addr;
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "Got bdcst addr %s\n", inet_ntoa(hapd->iapp_broadcast));
}

void iapp_recv()
{
	msg_hdr_t *msg_hdr;
	u8 msg_type;
#ifdef HAVE_SYSTEM_REINIT
	while(!iapp_recv_quitting)
#else
	while(1)
#endif
	{
#ifdef HAVE_SYSTEM_REINIT
		msg_hdr = cyg_mbox_timed_get(iapp_mbox_hdl, cyg_current_time() + msec_to_tick(500));
#else
		msg_hdr = (msg_hdr_t *)cyg_mbox_get(iapp_mbox_hdl);
#endif
		if (msg_hdr)
		{
			memcpy(hapd->RecvBuf,msg_hdr->msg_buf,msg_hdr->msg_len);
				free(msg_hdr);
			msg_type = *(hapd->RecvBuf + FIFO_HEADER_LEN);
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "Fifo msg type: %d\n", msg_type);

			if (msg_type == DOT11_EVENT_ASSOCIATION_IND) {
				DOT11_ASSOCIATION_IND *msg = (DOT11_ASSOCIATION_IND *)(hapd->RecvBuf + FIFO_HEADER_LEN);
				check_lan_ip_state();
				iapp_new_station(msg->MACAddr);
			}
			else if (msg_type == DOT11_EVENT_REASSOCIATION_IND) {
				DOT11_REASSOCIATION_IND *msg = (DOT11_REASSOCIATION_IND *)(hapd->RecvBuf + FIFO_HEADER_LEN);
				check_lan_ip_state();
				iapp_new_station(msg->MACAddr);
			}
			else {
				// messages that we don't handle and drop
			}
		}

	}
#ifdef HAVE_SYSTEM_REINIT
	while(msg_hdr = (msg_hdr_t *)cyg_mbox_tryget(iapp_mbox_hdl)){
		free(msg_hdr);
	}
	iapp_recv_quitting = 0;
#endif

}

static int get_iapp_parms(char *arg)
{
	char *s,*p;
	int i = 0;
	s = arg;
	/*lan interface*/
	while(1){
		if(*s != ' ' || *s == NULL )
			break;
		s++;
	}
	if(*s == NULL)
		return -1;
	p = strpbrk(s," ");
	if(p == NULL){
		p = s;
			while(1){
				if(*s == NULL)
					break;
				s++;
			}
			memcpy(hapd->iapp_iface,p,s-p);
		return 0;
	}
	memcpy(hapd->iapp_iface,s,p-s);
	hapd->iapp_iface[p-s]=0;
	s = p;
	
	/*wlan interface*/
	while(1){
		while(1){
			if(*s != ' ' || *s == NULL)
				break;
			s++;
		}
		if(*s == NULL )
				break;
		p = strpbrk(s," ");
		if(p == NULL){
			p = s;
			while(1){
				if(*s == NULL)
					break;
				s++;
			}
			memcpy(hapd->wlan_iface[i],p,s-p);
			break;
		}
		else{
			memcpy(hapd->wlan_iface[i],s,p-s);
			s = p;
			i++;
		}
	}
	return 0;
} 
char iapp_parm[150];
void creat_iapp(char *arg)
{
	if(iapp_run_flag)
		return;
	
	/*Creat messag box*/
	strcpy(iapp_parm,arg);
	cyg_mbox_create(&iapp_mbox_hdl,&iapp_mbox);
	/* Create the thread */
	cyg_thread_create(CYGNUM_IAPP_THREAD_PRIORITY,
			      iapp_main,
			      iapp_parm,
			      "iapp",
			      &iapp_stack,
			      sizeof(iapp_stack),
			      &iapp_thread_handle,
			      &iapp_thread_obj );
		/* Let the thread run when the scheduler starts */
	cyg_thread_resume(iapp_thread_handle);
	
}
void creat_iapp_recv()
{
	

	/* Create the thread */
	
	cyg_thread_create(CYGNUM_IAPP_THREAD_PRIORITY+1,
			      iapp_recv,
			      0,
			      "iapp recv",
			      &iapp_recv_stack,
			      sizeof(iapp_recv_stack),
			      &iapp_recv_thread_handle,
			      &iapp_recv_thread_obj );
		/* Let the thread run when the scheduler starts */
	cyg_thread_resume(iapp_recv_thread_handle);
	
}
#ifdef HAVE_SYSTEM_REINIT
void kill_iapp()
{
	if(iapp_run_flag){
		iapp_quitting = 1;
		while(iapp_quitting){
			cyg_thread_delay(20);
		}
		iapp_exit();
		iapp_run_flag = 0;
	}
	
}
void iapp_cleanup()
{
	cyg_thread_info tinfo;
	if(get_thread_info_by_name("iapp recv", &tinfo)){
		iapp_recv_quitting = 1;
		while(iapp_recv_quitting){
			cyg_thread_delay(10);
		}
		cyg_thread_kill(iapp_recv_thread_handle);
		cyg_thread_delete(iapp_recv_thread_handle);
	}
	cyg_mbox_delete(iapp_mbox_hdl);
	/*clean up socket*/
	if(hapd ->iapp_mltcst_sock >= 0)
		close(hapd ->iapp_mltcst_sock);
	if(hapd ->iapp_udp_sock >= 0)
		close(hapd ->iapp_udp_sock);
	if(hapd ->wlan_sock[0] >= 0)
		close(hapd ->wlan_sock[0]);
	
}
void iapp_exit()
{
	cyg_thread_kill(iapp_thread_handle);
	cyg_thread_delete(iapp_thread_handle);
}
#endif

