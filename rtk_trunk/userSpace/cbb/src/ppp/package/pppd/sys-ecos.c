/*
 * sys-ecos.c
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: sys-ecos.c,v 1.5 2010-07-19 08:34:32 Exp $
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/ppp_defs.h>
#include <net/if_ppp.h>
#include "pppd.h"
#include "fsm.h"
#include "lcp.h"
#include "chap-new.h"
#include "upap.h"
#include "ipcp.h"
#include "ccp.h"
#include "magic.h"

#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>

// added for debug, by zhuhuan on 2016.01.12
#define DBG_ZH_OUTPUT(format, string...) //diag_printf(format, ##string)


/*
 * Option variables and default values.
 */
int debug = 0;		/* Debug flag */

#ifndef __CONFIG_LCP_ECHO_INTERVAL__
#define __CONFIG_LCP_ECHO_INTERVAL__		30
#endif
#ifndef __CONFIG_MAX_LCP_ECHO_FAILS__
#define __CONFIG_MAX_LCP_ECHO_FAILS__		5
#endif
#ifndef __CONFIG_LCP_PASSIVE_ECHO_MODE__
#define __CONFIG_LCP_PASSIVE_ECHO_MODE__	0
#endif
#ifndef __CONFIG_LCP_PASSIVE_ECHO_INTERVAL__
#define __CONFIG_LCP_PASSIVE_ECHO_INTERVAL__	30
#endif
#ifndef __CONFIG_LCP_PASSIVE_ECHO_FAILS__
#define __CONFIG_LCP_PASSIVE_ECHO_FAILS__	5
#endif

extern int run_sh(char *cmd, char **argv, char **env);

/* Prototypes for procedures local to this file. */
static int get_flags(int fd);
static void set_flags(int fd, int flags);

/*
 * Functions to read and set the flags value in the device driver
 */
static int
get_flags(int fd)
{
	int flags = 0;

	ioctl(fd, PPPIOCGFLAGS, (caddr_t)&flags);
	return flags;
}

static void
set_flags(int fd, int flags)
{
	ioctl(fd, PPPIOCSFLAGS, (caddr_t)&flags);
}

int
establish_ppp(int unit)
{
	struct ppp *sc = &pppctl[unit];
	int sockfd;
	int fd;
	char name[32];

	sc->ppp_dev_fd = -1;
	sc->ppp_fd = -1;

	/* Open socket for interface ioctl */
	sockfd = sc->ppp_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		syslog(LOG_USER|LOG_INFO, "Cannot open ppp ipc fd");
		return -1;
	}

	/* Open ppp_dev_fd */
	sprintf(name, "/dev/net/ppp/ppp%d", unit);
	fd = sc->ppp_dev_fd = open(name, O_RDWR);
	DBG_ZH_OUTPUT("function: %s; line: %d; msg: name of ppp: %s; fd of ppp_dev---%d\n", __func__, __LINE__, name, fd);
	if (fd < 0) {
		DBG_ZH_OUTPUT("function: %s; line: %d; msg: open %s failed, fd---%d\n", __func__, __LINE__, name, fd);
		syslog(LOG_USER|LOG_INFO, "Cannot open /dev/net/ppp/ppp%d", unit);
		return -1;
	}

	return 0;
}

void
disestablish_ppp(int unit)
{
	struct ppp *sc = &pppctl[unit];
	int sockfd;
	int fd;

	if ((sockfd = sc->ppp_fd) >= 0) {
		close(sockfd);
	}

	if ((fd = sc->ppp_dev_fd) >= 0) {
		/* Close ppp_dev_fd */
		close(fd);
	}

	sc->ppp_fd = -1;
	sc->ppp_dev_fd = -1;
}

/*
 * output - Output PPP packet.
 */
void
output(int unit, u_char *p, int len)
{
	struct ppp *sc = &pppctl[unit];
	int fd = sc->ppp_dev_fd;

	/* Test write */
	write(fd, p, len);
}

/*
 * read_packet - get a PPP packet from the serial device.
 */
int
read_packet(int unit, unsigned char *buf)
{
	struct ppp *sc = &pppctl[unit];
	int fd = sc->ppp_dev_fd;
	fd_set fds;
	struct timeval tv = {1, 0};
	int maxfd;
	int n;
	int len;

	/* Set receive select set */
	FD_ZERO(&fds);

	FD_SET(fd, &fds);
	maxfd = fd+1;

	/* Wait for socket events */
	n = select(maxfd+1, &fds, NULL, NULL, &tv);
	if (n <= 0){
		return -1;
	}

	/* process ppp */
	if (!FD_ISSET(fd, &fds))
		return -1;

	len = read(fd, buf, PPP_MTU);
	if (len <= 0){
		return -1;
	}

	return len;
}

/*
 * Initialize options, override options set in lcp_int
 */
void
init_options(int unit)
{
	struct ppp *sc = &pppctl[unit];

	lcp_options *lwo = &lcp_wantoptions[unit];
	lcp_options *lao = &lcp_allowoptions[unit];
	ipcp_options *iwo = &ipcp_wantoptions[unit];
	ipcp_options *iao = &ipcp_allowoptions[unit];
#ifdef	PPP_COMPRESS
	ccp_options *cwo = &ccp_wantoptions[unit];
	ccp_options *cao = &ccp_allowoptions[unit];
#endif

	/* General options */
	peer_mru[sc->unit]  = sc->mru;
	debug = 1;
	sc->Error_count = 0;

	strcpy(sc->Our_name, sc->User);
	sc->Our_name[MAXNAMELEN-1] = 0;
	sc->Hostname[MAXNAMELEN-1] = 0;

	/* LCP options */
	sc->LCP_echo_interval = __CONFIG_LCP_ECHO_INTERVAL__;
	sc->LCP_echo_fails = __CONFIG_MAX_LCP_ECHO_FAILS__;
	sc->LCP_passive_echo_mode = __CONFIG_LCP_PASSIVE_ECHO_MODE__;
	sc->LCP_passive_echo_interval = __CONFIG_LCP_PASSIVE_ECHO_INTERVAL__;
	sc->LCP_passive_echo_fails = __CONFIG_LCP_PASSIVE_ECHO_FAILS__;
	sc->Multilink = 0;
	sc->Noendpoint = 1;
	sc->Nodetach = 1;
	sc->Updetach = 0;

	/* auth.c */
	sc->Refuse_pap = 0;
	sc->Refuse_chap = 0;
	sc->Refuse_eap = 1;	/* Not support eap yet */
#ifdef CHAPMS
	sc->Refuse_mschap = 0;
	sc->Refuse_mschap_v2 = 0;
#else
	sc->Refuse_mschap = 1;
	sc->Refuse_mschap_v2 = 1;
#endif

	/* options override */
	lwo->neg_asyncmap = 0;
	lwo->mru = sc->mru;
	lwo->chap_mdtype = MDTYPE_ALL;
	lwo->neg_eap = 0;
	lwo->neg_pcompression = 0;
	lwo->neg_accompression = 0;
	lwo->neg_endpoint = 0;

	lao->mru = sc->mru;

	lao->chap_mdtype = 0;
	if (!sc->Refuse_chap)
		lao->chap_mdtype |= MDTYPE_MD5;
	if (!sc->Refuse_mschap)
		lao->chap_mdtype |= MDTYPE_MICROSOFT;
	if (!sc->Refuse_mschap_v2)
		lao->chap_mdtype |= MDTYPE_MICROSOFT_V2;

	lao->neg_eap = 0;
	lao->neg_pcompression = 0;
	lao->neg_accompression = 0;
	lao->neg_endpoint = 0;

	bzero((char *)sc->xmit_accm, sizeof(sc->xmit_accm[0]));
	sc->xmit_accm[3] = 0x60000000;

	/*
	 * add the user and password as the secret, and
	 * don't care the peer name by using wildcard "*"
	 */
	/* IPCP options */
	iwo->neg_vj = 0;
	iao->neg_vj = 0;
	iwo->accept_remote = 1;
	iwo->accept_local = 1;
	if (sc->my_ip) {
		iwo->ouraddr = sc->my_ip;
		sc->Ask_for_local = 1;
	}
	if (sc->peer_ip)
		iwo->hisaddr = sc->peer_ip;
	if (sc->pri_dns)
		iwo->dnsaddr[0] = sc->pri_dns;
	if (sc->snd_dns)
		iwo->dnsaddr[1] = sc->snd_dns;

	sc->Usepeerdns = 1;

#ifdef PPP_COMPRESS
	/* CCP options */
	cwo->deflate = 0;
	cwo->deflate_draft = 0;
	cao->deflate = 0;
	cao->deflate_draft = 0;
	cwo->bsd_compress = 0;
	cao->bsd_compress = 0;

	cwo->predictor_1 = 0;
	cao->predictor_1 = 1;

#ifdef MPPE
	if (sc->mppe == 1) {
		cwo->mppe = 1;
		cwo->mppe_stateless = 1;
		cwo->mppe_40 = 1;
		cwo->mppe_56 = 1;
		cwo->mppe_128 = 1;

		cao->mppe_stateless = 1;
		cao->mppe = 1;
		cao->mppe_40 = 1;
		cao->mppe_56 = 1;
		cao->mppe_128 = 1;
	}
	else {
		cwo->mppe = 0;
		cwo->mppe_stateless = 0;
		cwo->mppe_40 = 0;
		cwo->mppe_56 = 0;
		cwo->mppe_128 = 0;

		cao->mppe_stateless = 0;
		cao->mppe = 0;
		cao->mppe_40 = 0;
		cao->mppe_56 = 0;
		cao->mppe_128 = 0;
	}
#endif	/* MPPE */
#endif	/* PPP_COMPRESS */

	/* UPAP option */
	sc->Hide_password = 1;
}

/*
 * ppp_send_config - configure the transmit characteristics of
 * the ppp interface.
 */
int
ppp_send_config(int unit, int mtu, u_int32_t asyncmap, int pcomp, int accomp)
{
	struct ppp *sc = &pppctl[unit];
	int sockfd = sc->ppp_fd;
	int fd = sc->ppp_dev_fd;
	struct ifreq ifr;
	int x;

	/* Set MTU */
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, sc->pppname);
	ifr.ifr_mtu = mtu;
	ioctl(sockfd, SIOCSIFMTU, &ifr);

	/* Set ASYNC specific ioctl */
	ioctl(fd, PPPIOCSASYNCMAP, &asyncmap);

	/* Set COMP flag */
	x = get_flags(fd);
	x = pcomp? x | SC_COMP_PROT: x &~ SC_COMP_PROT;
	x = accomp? x | SC_COMP_AC: x &~ SC_COMP_AC;
	set_flags(fd, x);
	return 0;
}

/*
 * ppp_recv_config - configure the receive-side characteristics of
 * the ppp interface.
 */
int
ppp_recv_config(int unit, int mru, u_int32_t asyncmap, int pcomp, int accomp)
{
	struct ppp *sc = &pppctl[unit];
	int fd = sc->ppp_dev_fd;
	int x;

	/* set PPPIOCSMRU */
	ioctl(fd, PPPIOCSMRU, &mru);

	/* set PPPIOCSRASYNCMAP */
	ioctl(fd, PPPIOCSRASYNCMAP, &asyncmap);

	/* set PPPIOCGFLAGS */
	x = get_flags(fd);
	x = !accomp? x | SC_REJ_COMP_AC: x &~ SC_REJ_COMP_AC;
	set_flags(fd, x);
	return 0;
}

#ifdef __CONFIG_TCP_AUTO_MTU__
int ip_input_change_mtu(int unit, int mtu)
{
	struct ppp *sc = &pppctl[unit];
	int fd = sc->ppp_dev_fd;

	/* set PPPIOCSMRU */
	ioctl(fd, PPPIOCSMRU, &mtu);
	return 0;
}
#endif

/*
 * sifvjcomp - config tcp header compression
 */
int
sifvjcomp(int unit, int vjcomp, int cidcomp, int maxcid)
{
	struct ppp *sc = &pppctl[unit];
	int fd = sc->ppp_dev_fd;
	int x;

	/* set PPPIOCGFLAGS */
	x = get_flags(fd);
	x = vjcomp ? x | SC_COMP_TCP: x &~ SC_COMP_TCP;
	x = cidcomp? x & ~SC_NO_TCP_CCID: x | SC_NO_TCP_CCID;
	set_flags(fd, x);

	/* set PPPIOCSMAXCID */
	ioctl(fd, PPPIOCSMAXCID, &maxcid);
	return 1;
}

/*
 * sifup - Config the interface up and enable IP packets to pass.
 */
int
sifup(int unit)
{
	struct ppp *sc = &pppctl[unit];
	struct ifreq ifr;
	int sockfd = sc->ppp_fd;

	/* PPP ifup */	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, sc->pppname);
	DBG_ZH_OUTPUT("function:%s; line:%d; ppp name: %s\n", __func__, __LINE__, sc->pppname);
	ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	DBG_ZH_OUTPUT("function:%s; line:%d; flags: %d\n", __func__, __LINE__, ifr.ifr_flags);

	ifr.ifr_flags |= IFF_UP;
	ioctl(sockfd, SIOCSIFFLAGS, &ifr);

	ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	DBG_ZH_OUTPUT("function:%s; line:%d; flags: %d\n", __func__, __LINE__, ifr.ifr_flags);
	return 1;
}

/*
 * sifdown - Config the interface down and disable IP.
 */
int
sifdown(int unit)
{
	struct ppp *sc = &pppctl[unit];
	struct ifreq ifr;
	int sockfd = sc->ppp_fd;

	/* PPP ifdown */	
	if (sockfd >= 0) {
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, sc->pppname);
		ioctl(sockfd, SIOCGIFFLAGS, &ifr);

		ifr.ifr_flags &= ~IFF_UP;
		ioctl(sockfd, SIOCSIFFLAGS, &ifr);
		return 1;
	}

	return 0;
}

/*
 * sifaddr - Config the interface IP addresses and netmask.
 */

/* Why ? */
u_int32_t
GetMask(u_int32_t addr)
{
	return 0xffffffff;
}

int
have_route_to(u_int32_t addr)
{
	return 0;
}

/*
 * sifaddr - Config the interface IP addresses and netmask.
 */
int
sifaddr(int unit, u_int32_t our_adr, u_int32_t his_adr, u_int32_t net_mask)
{
	return 1;
}

/*
 * cifaddr - Clear the interface IP addresses, and delete routes
 * through the interface if possible.
 */
int
cifaddr(int unit, u_int32_t our_adr, u_int32_t his_adr)
{
	struct ppp *sc = &pppctl[unit];
	int sockfd = sc->ppp_fd;
	int status;
	struct ifreq ifr;

	/*
	 * Deleting the interface address is required to
	 * correctly scrub the routing table based on
	 * the current netmask.
	 */
	bzero((char *)&ifr, sizeof(struct ifreq));

	strcpy(ifr.ifr_name, sc->pppname);
	ifr.ifr_addr.sa_len = sizeof(struct sockaddr_in);
	((struct sockaddr_in *)&ifr.ifr_addr)->sin_family = AF_INET;

	while (ioctl(sockfd, SIOCGIFADDR, (int)&ifr) == 0) {
		status = ioctl(sockfd, SIOCDIFADDR, (int)&ifr);
		if (status < 0) {
			/*
			 * Sometimes no address has been
			 * set, so ignore that error.
			 */
			if (errno != EADDRNOTAVAIL) {
				return 0;
			}
		}
	}

	return 1;
}

/* Create default route through i/f */
int
sifdefaultroute(int unit, u_int32_t our_adr, u_int32_t his_adr)
{
	return 1;
}

/* Delete default route through i/f */
int
cifdefaultroute(int unit, u_int32_t our_adr, u_int32_t his_adr)
{
	return 1;
}

/* Add proxy ARP entry for peer */
int
sifproxyarp(int unit, u_int32_t his_adr)
{
	return 1;
}

/* Delete proxy ARP entry for peer */
int
cifproxyarp(int unit, u_int32_t his_adr)
{
	return 1;
}

#ifdef	PPP_COMPRESS
/* CCP supported function */
int
ccp_test(int unit, u_char *opt_ptr, int opt_len, int for_transmit)
{
	struct ppp *sc = &pppctl[unit];
	int fd = sc->ppp_dev_fd;
	struct ppp_option_data data;

	memset(&data, '\0', sizeof(data));
	data.ptr = opt_ptr;
	data.length = opt_len;
	data.transmit = for_transmit;

	/* Test the installed compression method */
	if (ioctl(fd, PPPIOCSCOMPRESS, &data) == 0 ||
		errno == ENOBUFS) {
		return 1;
	}

	return -1;
}

void
ccp_flags_set(int unit, int isopen, int isup)
{
	struct ppp *sc = &pppctl[unit];
	int fd = sc->ppp_dev_fd;
	int x;

	/* Read sc_flags */
	x = get_flags(fd);
	x = isopen? x | SC_CCP_OPEN : x &~ SC_CCP_OPEN;
	x = isup?   x | SC_CCP_UP   : x &~ SC_CCP_UP;
	set_flags(fd, x);
}
#endif	/* PPP_COMPRESS */

/*
 * sifnpmode - Set the mode for handling packets for a given NP.
 */
int
sifnpmode(int u, int proto, enum NPmode mode)
{
	struct ppp *sc = pppsc();
	int fd = sc->ppp_dev_fd;
	struct npioctl npi;

	npi.protocol = proto;
	npi.mode = mode;
	if (ioctl(fd, PPPIOCSNPMODE, &npi) < 0)
		return 0;

	return 1;
}

/* Find out how long link has been idle */
int
get_idle_time(int u, struct ppp_idle *ip)
{
	struct ppp *sc = pppsc();
	int fd = sc->ppp_dev_fd;

	/* Test the installed compression method */
	if (ioctl(fd, PPPIOCGIDLE, (caddr_t)ip) == 0) {
		return 1;
	}

	return 0;
}

/*
 * netif_get_mtu - get the MTU on the PPP network interface.
 */
int
netif_get_mtu(int unit)
{
	struct ppp *sc = &pppctl[unit];
	int sockfd = sc->ppp_fd;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, sc->pppname);

	ioctl(sockfd, SIOCGIFMTU, &ifr);
	return ifr.ifr_mtu;
}

/*
 * netif_set_mtu - set the MTU on the PPP network interface.
 */
void
netif_set_mtu(int unit, int mtu)
{
	struct ppp *sc = &pppctl[unit];
	int sockfd = sc->ppp_fd;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, sc->pppname);
	ifr.ifr_mtu = mtu;

	ioctl(sockfd, SIOCSIFMTU, &ifr);
}

/* ppp thread stack */
#include <ecos_oslib.h>

#define STACKSIZE			8192
static char statck[NUM_PPP][STACKSIZE];
static char taskname[NUM_PPP][50];
static cyg_thread thread_obj[NUM_PPP];
static cyg_handle_t thread_handle[NUM_PPP];

struct ppp pppctl[NUM_PPP];
static int ppp_used[NUM_PPP];

extern void ppp_mainloop(struct ppp *sc);

/* Get control sc of this ppp thread */
struct ppp *
pppsc(void)
{
	int i;
	pid_t pid = getpid();

	for (i = 0; i < NUM_PPP; i++) {
		if (ppp_used[i] && pppctl[i].pid == pid) {
			return &pppctl[i];
		}
	}

	/*
	 * Should return 0, but for safety
	 * reason, return the first entry.
	 */
	return &pppctl[0];
}

/*
 * Allocate a pppoe_softc from the pool
 */
struct ppp *
ppp_osl_sc_alloc(char *pppname)
{
	int i;
	char buf[32];

	for (i = 0; i < NUM_PPP; i++) {
		sprintf(buf, "ppp%d", i);
		if (strcmp(buf, pppname) == 0 && ppp_used[i] == 0) {
			/* Allocate this one */
			ppp_used[i] = 1;
			return &pppctl[i];
		}
	}
	return 0;
}

void
ppp_osl_sc_free(struct ppp *sc)
{
	int i;

	for (i = 0; i < NUM_PPP; i++) {
		if (sc == &pppctl[i]) {
			/* Free this one */
			ppp_used[i] = 0;
		}
	}
}

/*
 * Daemon of PPP module.
 * This function calls the portable
 * main loop of the ppp functions.
 */
static void
PPP_main(struct ppp *sc)
{
	/* Enter os independent main loop */
	ppp_mainloop(sc);

	/* Return resource */
	ppp_osl_sc_free(sc);
	return;
}

static void
ppp_if_up(void)
{
	struct ppp *sc = pppsc();
	char *argv[] = {"ip-up", NULL};
	nvram_commit();
	run_sh(argv[0], argv, sc->script_env);
}

static void
ppp_if_down(void)
{
	struct ppp *sc = pppsc();
	char *argv[] = {"ip-down", NULL};

	run_sh(argv[0], argv, sc->script_env);
}

/* Spaw the ppp daemon */
int ppp_start(struct ppp_conf *conf)
{
	int i;
	struct ppp *sc;

	if (ip_up_hook == NULL)
		ip_up_hook = ppp_if_up;

	if (ip_down_hook == NULL)
		ip_down_hook = ppp_if_down;

	sc = ppp_osl_sc_alloc(conf->pppname);
	if (sc == 0)
		return 0;

	memset(sc, 0, sizeof(*sc));

	/* Setup ppp structure */
	strcpy(sc->pppname, conf->pppname);
	strcpy(sc->ifname, conf->ifname);
	strcpy(sc->User, conf->user);
	strcpy(sc->Passwd, conf->passwd);

	sc->unit = conf->unit;
	sc->mtu = conf->mtu;
	sc->mru = conf->mru;
	sc->Idle_time_limit = conf->idle;
	sc->defaultroute = conf->defaultroute;
	sc->netmask = conf->netmask;
	sc->my_ip = conf->my_ip;
	sc->peer_ip = conf->peer_ip;
	sc->pri_dns = conf->pri_dns;
	sc->snd_dns = conf->snd_dns;
#ifdef MPPE
	sc->mppe = conf->mppe;
#endif
    /*lq pppoe向ppp传递信息*/
	#ifdef __CONFIG_PPPOE_SERVER__
	if(sc->unit == 1)
	{
		sc->synchro_type = conf->synchro_type;
		sc->start_time = conf->start_time;
		sc->time_out = conf->time_out;
		strcpy(sc->service_name,conf->service_name);
	}
	#endif
	/* Setup daemon date */
	i = sc->unit;
	sprintf(taskname[i], "pppd%d", sc->unit);

	cyg_thread_create(
		8,
		(cyg_thread_entry_t *)PPP_main,
		(cyg_addrword_t)sc,
		taskname[i],
		&statck[i],
		STACKSIZE,
		&thread_handle[i],
		&thread_obj[i]);

	sc->pid = oslib_getpidbyname(taskname[i]);

	cyg_thread_resume(thread_handle[i]);

	return (int)sc->pid;
}

void ppp_stop(char *ppp_name)
{
	int unit;

	/* Decompose unit */
	if (memcmp(ppp_name, "ppp", 3) != 0)
		return;

	unit = atoi(&ppp_name[3]);
	die(unit);
}
