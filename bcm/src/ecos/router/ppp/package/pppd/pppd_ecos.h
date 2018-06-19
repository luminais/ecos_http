/*
 * Extra ppp struct and defines for ecos-router
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pppd_ecos.h,v 1.2 2010-06-25 18:09:36 Exp $
 */

#ifndef	__PPPD_ECOS_H__
#define	__PPPD_ECOS_H__

#include <sys/syslog.h>//roy add

#ifndef	MAXPATHLEN
#define MAXPATHLEN      128
#endif

#define	PPPNAMSIZ	16

struct ppp_conf {
	char pppname[PPPNAMSIZ];	/* logical ppp interface name */
	char ifname[PPPNAMSIZ];		/* device interface name */
	char user[MAXNAMELEN];		/* User name */
	char passwd[MAXSECRETLEN];	/* Password for PAP */

	int unit;
	int mtu;
	int mru;			/* MTU and MRU that we allow */
	int idle;			/* idle time */
	int defaultroute;		/* Assign default rte through interf */

	u_long netmask;			/* Netmask to use on PPP interface */
	u_long my_ip;			/* My IP address */
	u_long peer_ip;			/* Peer IP address */
	u_long pri_dns;
	u_long snd_dns;
#ifdef MPPE
	int mppe;
#endif
};

/* PPP task variables structure */
struct callout;
struct ppp {
	int unit;
	char pppname[PPPNAMSIZ];	/* logical ppp interface name */
	pid_t pid;			/* pid of pppd task */
	int ppp_dev_fd;
	int ppp_fd;

	/*
	 * configured variables
	 */
	u_long netmask;			/* Netmask to use on PPP interface */
	u_long my_ip;			/* My IP address */
	u_long peer_ip;			/* Peer IP address */
	u_long pri_dns;
	u_long snd_dns;
	int mru;			/* MTU and MRU that we allow */
	int mtu;
	int authfail;
#ifdef MPPE
	int mppe;
#endif

	int defaultroute;		/* Assign default rte through interf */
	struct callout *callout;	/* Callout list */
	u_char inpacket_buf[PPP_MTU + PPP_HDRLEN];	/* Buffer for incoming packet */

	/*
	 * Global variables.
	 */
	char **script_env;
	int s_env_nalloc;

	char ifname[PPPNAMSIZ];		/* Interface name */
	char Hostname[MAXNAMELEN];	/* Our host name */
	u_char Outpacket_buf[PPP_MTU + PPP_HDRLEN];	/* Buffer for outgoing packet */
	int Phase;			/* Where the link is at */
	int Need_holdoff;		/* Need holdoff period after link terminates */
	int Status;			/* exit status for pppd -- volatile int status; */
	int Unsuccess;			/* # unsuccessful connection attempts */
	int Error_count;		/* # of times error() has been called */

	int Listen_time;		/* time to listen first (ms) */

	/*
	 * Variables set by command-line options.
	 */
	int Maxconnect;			/* Maximum connect time (seconds) */
	char User[MAXNAMELEN];		/* User name */
	char Passwd[MAXSECRETLEN];	/* Password for PAP */
	char Our_name[MAXNAMELEN];	/* Authentication host name */
	char Remote_name[MAXNAMELEN];	/* Remote host name */
	bool Explicit_remote;		/* Check if the remote_name is assigned explicitly */
	bool Demand;			/* Do dial-on-demand */
	int Idle_time_limit;		/* Shut down link if idle for this long */
	int Maxfail;			/* Max # of unsuccessful connection attempts */
	bool Multilink;			/* Enable multilink operation */
	bool Noendpoint;		/* don't send/accept endpoint discriminator */

	/* Variables used in auth.c */
	bool Nodetach;			/* Don't detach from controlling tty */
	bool Updetach;			/* Detach once link is up */
	bool Refuse_pap;		/* Don't wanna auth. ourselves with PAP */
	bool Refuse_chap;		/* Don't wanna auth. ourselves with CHAP */
	bool Refuse_eap;		/* Don't wanna auth. ourselves with EAP */
	bool Refuse_mschap;		/* Don't wanna auth. ourselves with MS-CHAP */
	bool Refuse_mschap_v2;		/* Don't wanna auth. ourselves with MS-CHAPv2 */
	char Remote_number[MAXNAMELEN];

	/* Variables for proto options */
	u_char Nak_buffer[PPP_MTU];	/* where we construct a nak packet */
	bool Lax_recv;			/* accept control chars in asyncmap */
	bool Usepeerdns;		/* Use the peer dns */
	int IPCP_is_up;			/* have called np_up() */
	int IPCP_is_open;		/* have called np_up() */
	bool Ask_for_local;		/* request our address from peer */

	/* lcp echo variables */
	int LCP_echos_pending;		/* Number of outstanding echo msgs */
	int LCP_echo_number;		/* ID number of next echo frame */
	int LCP_echo_timer_running;	/* TRUE if a timer running */
	int LCP_echo_interval;		/* Seconds between intervals */
	int LCP_echo_fails;		/* Number of echo failures */

	int LCP_passive_echo_mode;
	int LCP_passive_echos_pending;
	int LCP_passive_echo_timer_running;
	int LCP_passive_echo_interval;
	int LCP_passive_echo_fails;

	/* lcp variables */
	u_long  xmit_accm[8];		/* extended transmit ACCM */

	/* upap varunsuccessiable */
	bool Hide_password;
};

struct ppp *pppsc(void);
extern struct ppp pppctl[];
extern char devnam[MAXPATHLEN];		/* Device name */

/*
 * Global variables.
 */

#define outpacket_buf		(pppsc()->Outpacket_buf)
#define unsuccess		(pppsc()->Unsuccess)
#define error_count		(pppsc()->Error_count)

/*
 * Variables set by command-line options.
 */
/* options.c */
extern int	debug;		/* Debug flag */
#define nodetach		(pppsc()->Nodetach)
#define updetach		(pppsc()->Updetach)
#define idle_time_limit		(pppsc()->Idle_time_limit)

/* auth.c */
#define refuse_pap		(pppsc()->Refuse_pap)
#define refuse_chap		(pppsc()->Refuse_chap)
#define	refuse_eap		(pppsc()->Refuse_eap)
#define	refuse_mschap		(pppsc()->Refuse_mschap)
#define	refuse_mschap_v2	(pppsc()->Refuse_mschap_v2)
#define explicit_remote		(pppsc()->Explicit_remote)
#define remote_name		(pppsc()->Remote_name)
#define remote_number		(pppsc()->Remote_number)

/* upap.c */
#define hide_password	(pppsc()->Hide_password)

#define notify(a,b)
int ppp_send_config __P((int, int, u_int32_t, int, int));
int ppp_recv_config __P((int, int, u_int32_t, int, int));

/*
 * Prototypes.
 */

/* Procedures exported from main.c. */
#define detach()
void die __P((int));		/* Cleanup and exit */
void novm __P((char *));	/* Say we ran out of memory, and die */
void ppp_timeout __P((void (*func)(void *), void *arg, int s, int us));
				/* Call func(arg) after s.us seconds */
void ppp_untimeout __P((void (*func)(void *), void *arg));
				/* Cancel call to func(arg) */
void update_link_stats __P((int)); /* Get stats at link termination */
void script_setenv __P((char *, char *, int));	/* set script env var */
void script_unsetenv __P((char *));		/* unset script env var */
void new_phase __P((int));	/* signal start of new phase */

/* Procedures exported from utils.c. */
int slprintf __P((char *, int, char *, ...));		/* sprintf++ */
int vslprintf __P((char *, int, char *, va_list));	/* vsprintf++ */
size_t strlcpy __P((char *, const char *, size_t));	/* safe strcpy */
size_t strlcat __P((char *, const char *, size_t));	/* safe strncpy */
#ifdef DEBUG
#define warn	ppp_warn
void log_packet __P((u_char *, int, char *, int));
				/* Format a packet and log it with syslog */
void print_string __P((char *, int,  void (*) (void *, char *, ...),
		void *));	/* Format a string for output */
void dbglog __P((char *, ...));	/* log a debug message */
void info __P((char *, ...));	/* log an informational message */
void notice __P((char *, ...));	/* log a notice-level message */
void warn __P((char *, ...));	/* log a warning message */
void error __P((char *, ...));	/* log an error message */
void fatal __P((char *, ...));	/* log an error message and die(1) */
void init_pr_log __P((char *, int));	/* initialize for using pr_log */
void pr_log __P((void *, char *, ...));	/* printer fn, output to syslog */
void end_pr_log __P((void));	/* finish up after using pr_log */
#else
#define log_packet(a,b,c,d)
#define print_string(a,b,c,d)
#define dbglog(a,b...)
#define info(a,b...)
#define notice(a,b...) syslog(LOG_USER|LOG_INFO, a,##b)/*roy add*/
#define warn(a,b...) 
#define error(a,b...) syslog(LOG_USER|LOG_INFO, a,##b)/*roy add*/
#define fatal(a,b...)
#define init_pr_log(a,b)
#define pr_log (NULL)
#define end_pr_log()
#endif	/* DEBUG */

/* Procedures exported from auth.c */
#define	link_required(a)
void link_terminated __P((int));  /* we are finished with the link */
void link_down __P((int));	  /* the LCP layer has left the Opened state */
void link_established __P((int)); /* the link is up; authenticate now */
void start_networks __P((int));  /* start all the network control protos */
void continue_networks __P((int)); /* start network [ip, etc] control protos */
void np_up __P((int, int));	  /* a network protocol has come up */
void np_down __P((int, int));	  /* a network protocol has gone down */
void np_finished __P((int, int)); /* a network protocol no longer needs link */
void auth_peer_fail __P((int, int));
				/* peer failed to authenticate itself */
void auth_peer_success __P((int, int, int, char *, int));
				/* peer successfully authenticated itself */
void auth_withpeer_fail __P((int, int));
				/* we failed to authenticate ourselves */
void auth_withpeer_success __P((int, int, int));
				/* we successfully authenticated ourselves */
#define	auth_check_options()
				/* check authentication options supplied */
void auth_reset __P((int));	/* check what secrets we have */
				/* Check peer-supplied username/password */
#define check_passwd(a,b,c,d,e,f) (UPAP_AUTHNAK)
int  get_secret __P((int, char *, char *, char *, int *, int));
				/* get "secret" for chap */
#define auth_ip_addr(a,b) (1)
				/* check if IP address is authorized */
#define auth_number()	(1)
int  bad_ip_adrs __P((u_int32_t));
				/* check if IP address is unreasonable */

/* Procedures exported from sys-*.c */
int  establish_ppp __P((int));
void disestablish_ppp __P((int));
void output __P((int, u_char *, int));	/* Output a PPP packet */
int  read_packet __P((int, u_char *));	/* Read PPP packet */

int  ccp_test __P((int, u_char *, int, int));
				/* Test support for compression scheme */
void ccp_flags_set __P((int, int, int));
				/* Set kernel CCP state */
int  get_idle_time __P((int, struct ppp_idle *));
				/* Find out how long link has been idle */
int  netif_get_mtu __P((int)); /* Get PPP interface MTU */
void netif_set_mtu __P((int, int)); /* Set PPP interface MTU */
int  sifvjcomp __P((int, int, int, int));
				/* Configure VJ TCP header compression */
int  sifup __P((int));		/* Configure i/f up for one protocol */
int  sifnpmode __P((int u, int proto, enum NPmode mode));
				/* Set mode for handling packets for proto */
int  sifdown __P((int));	/* Configure i/f down for one protocol */
int  sifaddr __P((int, u_int32_t, u_int32_t, u_int32_t));
				/* Configure IPv4 addresses for i/f */
int  cifaddr __P((int, u_int32_t, u_int32_t));
				/* Reset i/f IP addresses */
int  sifdefaultroute __P((int, u_int32_t, u_int32_t));
				/* Create default route through i/f */
int  cifdefaultroute __P((int, u_int32_t, u_int32_t));
				/* Delete default route through i/f */
int  sifproxyarp __P((int, u_int32_t));
				/* Add proxy ARP entry for peer */
int  cifproxyarp __P((int, u_int32_t));
				/* Delete proxy ARP entry for peer */
u_int32_t GetMask __P((u_int32_t)); /* Get appropriate netmask for address */
int  have_route_to __P((u_int32_t)); /* Check if route to addr exists */

/* Procedures exported from options.c */
int  getword __P((FILE *f, char *word, int *newlinep, char *filename));
				/* Read a word from a file */
void init_options __P((int));

/*
 * System dependent definitions for user-level 4.3BSD UNIX implementation.
 */
#undef	TIMEOUT
#undef	UNTIMEOUT
#define TIMEOUT(r, f, t)	ppp_timeout((r), (f), (t), 0)
#define UNTIMEOUT(r, f)		ppp_untimeout((r), (f))

extern double drand48	__P((void));

/*
 * Prototypes.
 */
/* Functions called by lower level */
int ppp_start(struct ppp_conf *conf);
void ppp_stop(char *ppp_name);

#define	srandom(a)	random()
#define	get_host_seed()	0xffffffff
extern	int getpid(void);

extern int setkey(const char *key);
extern int encrypt(char *block, int flag);

#endif	/* __PPPD_ECOS_H__ */
