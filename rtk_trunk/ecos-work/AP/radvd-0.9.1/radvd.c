/*
 *   $Id: radvd.c,v 1.1 2008-01-11 08:01:30 hf_shi Exp $
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>	 
 *
 *   This software is Copyright 1996-2000 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <pekkas@netcore.fi>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <pathnames.h>
#define PATH_RADVD_CONF "/etc/radvd.conf"
//#define HAVE_PERMISSION	1	// eCos has no permission 
#define HAVE_OVER_PTHREAD	1	// work over pthread  
//#define HAVE_PARSE_ARGS	1	// parse args? 
#define CYGNUM_RADVD_THREAD_PRIORITY 16

struct Interface *IfaceList = NULL;

static char usage_str[] =
	"[-vh] [-d level] [-C config_file] [-m log_method] [-l log_file]\n"
	"\t[-f facility] [-p pid_file] [-u username] [-t chrootdir]";

#ifdef HAVE_GETOPT_LONG
struct option prog_opt[] = {
	{"debug", 1, 0, 'd'},
	{"config", 1, 0, 'C'},
	{"pidfile", 1, 0, 'p'},
	{"logfile", 1, 0, 'l'},
	{"logmethod", 1, 0, 'm'},
	{"facility", 1, 0, 'f'},
	{"username", 1, 0, 'u'},
	{"chrootdir", 1, 0, 't'},
	{"version", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{NULL, 0, 0, 0}
};
#endif

extern FILE *yyin;

char *conf_file = NULL;
static char *pname;
int sock = -1;

volatile int sighup_received = 0;
volatile int sigterm_received = 0;
static volatile int sigint_received = 0;

static void sighup_handler(int sig);
static void sigterm_handler(int sig);
static void sigint_handler(int sig);
static void timer_handler(void *data);
static void config_interface(void);
static void kickoff_adverts(void);
static void stop_adverts(void);
static void reload_config(void);
static void version(void);
static void usage(void);
static int drop_root_privileges(const char *);
static int readin_config(char *);
static int check_conffile_perm(const char *, const char *);

#define exit( n )		return( n )
#define DEFAULT_PNAME	"radvd"

#ifdef HAVE_SYSTEM_REINIT
static int radvd_quitting = 0;
static int radvd_is_running = 0;
static int radvd_reload = 0;
static void clear_interface();
#endif

unsigned char radvd_stack[ 16 * 1024 ];
cyg_handle_t radvd_thread_handle;
cyg_thread radvd_thread_obj;

#ifndef __ECOS
static int main(int argc, char *argv[])
#else
static int radvd_main(cyg_addrword_t data)
#endif
{
	//unsigned char msg[MSG_SIZE];
	unsigned char *msg;
	char pidstr[16];
	int c, log_method;
	char *logfile, *pidfile;
	int facility, fd;
#ifdef HAVE_PERMISSION
	char *username = NULL;
	char *chrootdir = NULL;
#endif
#ifdef HAVE_GETOPT_LONG
	int opt_idx;
#endif

	//pname = ((pname=strrchr(argv[0],'/')) != NULL)?pname+1:argv[0];
	pname = DEFAULT_PNAME;

	srand((unsigned int)time(NULL));

	log_method = L_STDERR_SYSLOG;
	logfile = PATH_RADVD_LOG;
	conf_file = PATH_RADVD_CONF;
	facility = LOG_FACILITY;
#ifndef __ECOS
	pidfile = PATH_RADVD_PID;
#endif

#ifndef __ECOS 
	/* parse args */
#ifdef HAVE_PARSE_ARGS
#ifdef HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, "d:C:l:m:p:t:u:vh", prog_opt, &opt_idx)) > 0)
#else
	while ((c = getopt(argc, argv, "d:C:l:m:p:t:u:vh")) > 0)
#endif
	{
		switch (c) {
		case 'C':
			conf_file = optarg;
			break;
		case 'd':
			set_debuglevel(atoi(optarg));
			break;
		case 'f':
			facility = atoi(optarg);
			break;
		case 'l':
			logfile = optarg;
			break;
		case 'p':
			pidfile = optarg;
			break;
		case 'm':
			if (!strcmp(optarg, "syslog"))
			{
				log_method = L_SYSLOG;
			}
			else if (!strcmp(optarg, "stderr_syslog"))
			{
				log_method = L_STDERR_SYSLOG;
			}
			else if (!strcmp(optarg, "stderr"))
			{
				log_method = L_STDERR;
			}
			else if (!strcmp(optarg, "logfile"))
			{
				log_method = L_LOGFILE;
			}
			else if (!strcmp(optarg, "none"))
			{
				log_method = L_NONE;
			}
			else
			{
				fprintf(stderr, "%s: unknown log method: %s\n", pname, optarg);
				exit(1);
			}
			break;
#ifdef HAVE_PERMISSION
		case 't':
			chrootdir = strdup(optarg);
			break;
		case 'u':
			username = strdup(optarg);
			break;
#endif
		case 'v':
			version();
			break;
		case 'h':
			usage();
#ifdef HAVE_GETOPT_LONG
		case ':':
			fprintf(stderr, "%s: option %s: parameter expected\n", pname,
				prog_opt[opt_idx].name);
			exit(1);
#endif
		case '?':
			exit(1);
		}
	}
#endif
#endif // __ECOS

#ifdef HAVE_PERMISSION
	if (chrootdir) {
		if (!username) {
			fprintf(stderr, "Chroot as root is not safe, exiting\n");
			exit(1);
		}
		
		if (chroot(chrootdir) == -1) {
			perror("chroot");
			exit (1);
		}
		
		if (chdir("/") == -1) {
			perror("chdir");
			exit (1);
		}
		/* username will be switched later */
	}
#endif
	
	if (log_open(log_method, pname, logfile, facility) < 0)
		exit(1);

	flog(LOG_INFO, "version %s started", VERSION);

	/* get a raw socket for sending and receiving ICMPv6 messages */
	if(sock == -1)
	{
		sock = open_icmpv6_socket();
		if (sock < 0)
			exit(1);
	}

#ifdef HAVE_PERMISSION	
	/* drop root privileges if requested. */
	if (username) {
		if (drop_root_privileges(username) < 0)
			exit(1);
	}

	/* check that 'other' cannot write the file
         * for non-root, also that self/own group can't either
         */
	if (check_conffile_perm(username, conf_file) < 0) {
		if (get_debuglevel() == 0)
			exit(1);
		else
			flog(LOG_WARNING, "Insecure file permissions, but continuing anyway");
	}
#endif // HAVE_PERMISSION
	
	/* if we know how to do it, check whether forwarding is enabled */
	if (check_ip6_forwarding()) {
		if (get_debuglevel() == 0) {
			flog(LOG_ERR, "IPv6 forwarding seems to be disabled, exiting");
			exit(1);
		}
		else
			flog(LOG_WARNING, "IPv6 forwarding seems to be disabled, but continuing anyway.");
	}

	/* parse config file */
	if (readin_config(conf_file) < 0)
		;//exit(1);	// continue to run, because it stands as a daemon 

#ifndef __ECOS
	/* FIXME: not atomic if pidfile is on an NFS mounted volume */	
	if ((fd = open(pidfile, O_CREAT|O_EXCL|O_WRONLY, 0644)) < 0)
	{
		flog(LOG_ERR, "another radvd seems to be already running, terminating");
		exit(1);
	}
	
	/*
	 * okay, config file is read in, socket and stuff is setup, so
	 * lets fork now...
	 */
#ifndef HAVE_OVER_PTHREAD
	if (get_debuglevel() == 0) {

		/* Detach from controlling terminal */
		if (daemon(0, 0) < 0)
			perror("daemon");

		/*
		 * reopen logfile, so that we get the process id right in the syslog
		 */
		if (log_reopen() < 0)
			exit(1);

	}
#endif
#endif
	
	/*
	 *	config signal handlers
	 */
	signal(SIGHUP, sighup_handler);
	signal(SIGTERM, sigterm_handler);
	signal(SIGINT, sigint_handler);

	if((msg = malloc(MSG_SIZE)) == NULL)
	{
		fprintf(stderr, "%s %d: malloc error!\n");
		return -1;
	}

#ifndef __ECOS
	snprintf(pidstr, sizeof(pidstr), "%d\n", getpid());
	
	write(fd, pidstr, strlen(pidstr));
	
	close(fd);
#endif
#ifdef HAVE_SYSTEM_REINIT
	radvd_is_running = 1;
#endif
	config_interface();
	sleep(3);
	kickoff_adverts();

	/* enter loop */
#if 1
	for (;;)
	{
		int len, hoplimit;
		struct sockaddr_in6 rcv_addr;
		struct in6_pktinfo *pkt_info = NULL;
		
		len = recv_rs_ra(sock, msg, &rcv_addr, &pkt_info, &hoplimit);
		if (len > 0)
			process(sock, IfaceList, msg, len, 
				&rcv_addr, pkt_info, hoplimit);

		{
			// tick timer alarm after timeout 
			extern void radvd_tick_alarm_handler( void );
			radvd_tick_alarm_handler();
		}
		
		if (sigterm_received || sigint_received) {
			printf( "radvd stop & exit!\n" );
			stop_adverts();
			sigterm_received=0;
			break;
		}
		
#ifndef HAVE_SYSTEM_REINIT
		if (sighup_received)
		{
			printf( "radvd reload.%d!\n", sighup_received );
			if( sighup_received == 2 )	// If one uncheck enable option, 
				stop_adverts();			// send RA with zero Router Lifetime
			reload_config();		
			sighup_received = 0;
		}
#else
		if (sighup_received == 2)
		{
			stop_adverts();			// send RA with zero Router Lifetime
			break;
		}

		if(radvd_reload == 1)
		{
			stop_adverts();
			reload_config();
			radvd_reload = 0;
		}
#endif
	}
#endif
	
//	unlink(pidfile);
//	exit(0);
	if(msg != NULL)
		free(msg);
#ifdef HAVE_SYSTEM_REINIT
	radvd_quitting = 0;
	radvd_is_running = 0;
	clear_interface();
#endif
	return 0;
}

#ifdef HAVE_OVER_PTHREAD
#if 0
static void *
__radvd_main_thread( void *arg )
{
	__radvd_main( ( ( mainarg_t * )arg ) ->argc, ( ( mainarg_t * )arg ) ->argv );
	
	return NULL;
}

#include <pthread.h>
#endif

void create_radvd(void)
{
#ifdef HAVE_SYSTEM_REINIT
	if(radvd_is_running == 1)
	{
		radvd_reload = 1;
		return;
	}
#endif
	
	/* Create the thread */
	cyg_thread_create(CYGNUM_RADVD_THREAD_PRIORITY,
		      radvd_main,
		      0,
		      "radvd",
		      &radvd_stack,
		      sizeof(radvd_stack),
		      &radvd_thread_handle,
		      &radvd_thread_obj);
	/* Let the thread run when the scheduler starts */
	cyg_thread_resume(radvd_thread_handle);

#if 0
	// RADVD must run under POSIX thread 
	pthread_t thread;
	mainarg_t mainarg;
	int ret;
	// Create the main thread
	pthread_attr_t attr;
	struct sched_param schedparam;
	
	schedparam.sched_priority = CYGNUM_POSIX_MAIN_DEFAULT_PRIORITY;
	
	pthread_attr_init( &attr );
	pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setstackaddr( &attr, &radvd_stack[sizeof(radvd_stack)] );
	pthread_attr_setstacksize( &attr, sizeof(radvd_stack) );
	pthread_attr_setschedpolicy( &attr, SCHED_RR );
	pthread_attr_setschedparam( &attr, &schedparam );
	
	//diag_printf( "%s: %d thread=%d info=%p\n", __FUNCTION__, __LINE__, thread, pthread_info_id( thread ) );
	
	// fill arg for main() 
	mainarg.argc = argc;
	mainarg.argv = argv;
	
	// create thread 
	ret = pthread_create( &thread, &attr, __radvd_main_thread, ( void* )&mainarg );

	// wait for complete 
	//pthread_join( thread, NULL );
#endif
#ifdef HAVE_PARSE_ARGS
	sleep( 1 );		// wait for child thread to acquire arguments 
#endif
}

#ifdef HAVE_SYSTEM_REINIT
void kill_radvd()
{
	if(radvd_is_running == 0)
		return;

	sigint_received = 1;
	radvd_quitting = 1;
	while(radvd_quitting){
		cyg_thread_delay(200);
	}
	radvd_is_running = 0;
	cyg_thread_kill(radvd_thread_handle);
	cyg_thread_delete(radvd_thread_handle);
}

static void clear_interface()
{
	struct Interface *iface;

	/* disable timers, free interface and prefix structures */
	for(iface=IfaceList; iface; iface=iface->next)
	{
		/* check that iface->tm was set in the first place */
		if (iface->tm.next && iface->tm.prev)
		{
			dlog(LOG_DEBUG, 4, "disabling timer for %s", iface->Name);
			clear_timer(&iface->tm);
		}
	}

	iface=IfaceList; 
	while(iface)
	{
		struct Interface *next_iface = iface->next;
		struct AdvPrefix *prefix;
		struct AdvRoute *route;
#ifdef SUPPORT_RDNSS_OPTION
		struct AdvRDNSS *rdnss;
#endif
#ifdef SUPPORT_DNSSL_OPTION
		struct AdvDNSSL *dnssl;
#endif

		dlog(LOG_DEBUG, 4, "freeing interface %s", iface->Name);
		
		set_ipv6_mreq_allrouters( sock, iface->Name, IPV6_LEAVE_GROUP );
		
		prefix = iface->AdvPrefixList;
		while (prefix)
		{
			struct AdvPrefix *next_prefix = prefix->next;
			
			free(prefix);
			prefix = next_prefix;
		}
		
		route = iface->AdvRouteList;
		while (route)
		{
			struct AdvRoute *next_route = route->next;

			free(route);
			route = next_route;
		}  
#ifdef SUPPORT_RDNSS_OPTION
		rdnss = iface->AdvRDNSSList;
		while (rdnss)
		{
			struct AdvRDNSS *next_rdnss = rdnss->next;

			free(rdnss);
			rdnss = next_rdnss;
		}
#endif
#ifdef SUPPORT_DNSSL_OPTION
		dnssl = iface->AdvDNSSLList;
		while (dnssl)
		{
			struct AdvDNSSL *next_dnssl = dnssl->next;
			int i;

			for (i = 0; i < dnssl->AdvDNSSLNumber; i++)
				free(dnssl->AdvDNSSLSuffixes[i]);
			free(dnssl->AdvDNSSLSuffixes);
			free(dnssl);

			dnssl = next_dnssl;
		}
#endif
		free(iface);
		iface = next_iface;
	}

	IfaceList = NULL;
}
#endif
#endif // HAVE_OVER_PTHREAD

static void
timer_handler(void *data)
{
	struct Interface *iface = (struct Interface *) data;
	double next;

	dlog(LOG_DEBUG, 4, "timer_handler called for %s", iface->Name);

	send_ra(sock, iface, NULL);
	iface->init_racount++;
	/*add to support RFC 4861 section 6.2.4*/
	next = rand_between(iface->MinRtrAdvInterval, iface->MaxRtrAdvInterval); 
	if (iface->init_racount < MAX_INITIAL_RTR_ADVERTISEMENTS)
	{
		next = min(MAX_INITIAL_RTR_ADVERT_INTERVAL, next);
	}
	set_timer(&iface->tm, next);
}

static void
config_interface(void)
{
	struct Interface *iface;
	for(iface=IfaceList; iface; iface=iface->next)
	{
		if (iface->AdvLinkMTU)
			set_interface_linkmtu(iface->Name, iface->AdvLinkMTU);
		if (iface->AdvCurHopLimit)
			set_interface_curhlim(iface->Name, iface->AdvCurHopLimit);
		if (iface->AdvReachableTime)
			set_interface_reachtime(iface->Name, iface->AdvReachableTime);
		if (iface->AdvRetransTimer)
			set_interface_retranstimer(iface->Name, iface->AdvRetransTimer);
		
		set_ipv6_mreq_allrouters( sock, iface->Name, IPV6_JOIN_GROUP );
	}
}

static void
kickoff_adverts(void)
{
	struct Interface *iface;

	/*
	 *	send initial advertisement and set timers
	 */

	for(iface=IfaceList; iface; iface=iface->next)
	{
		if( ! iface->UnicastOnly )
		{
			init_timer(&iface->tm, timer_handler, (void *) iface);
			if (iface->AdvSendAdvert)
			{
				/* send an initial advertisement */
				send_ra(sock, iface, NULL);
				/*add to support RFC 4861 section 6.2.4*/
				iface->init_racount++;
				set_timer(&iface->tm,min(MAX_INITIAL_RTR_ADVERT_INTERVAL
					,iface->MaxRtrAdvInterval));
			}
		}
	}
}

static void
stop_adverts(void)
{
	struct Interface *iface;

	/*
	 *	send final RA (a SHOULD in RFC2461 section 6.2.5)
	 */

	for (iface=IfaceList; iface; iface=iface->next) {
		if( ! iface->UnicastOnly ) {
			if (iface->AdvSendAdvert) {
				/* send a final advertisement with zero Router Lifetime */
				iface->AdvDefaultLifetime = 0;
				send_ra(sock, iface, NULL);
			}
		}
	}
}

static void reload_config(void)
{
	struct Interface *iface;

	flog(LOG_INFO, "attempting to reread config file");

	dlog(LOG_DEBUG, 4, "reopening log");
	if (log_reopen() < 0)
		exit(1);

	/* disable timers, free interface and prefix structures */
	for(iface=IfaceList; iface; iface=iface->next)
	{
		/* check that iface->tm was set in the first place */
		if (iface->tm.next && iface->tm.prev)
		{
			dlog(LOG_DEBUG, 4, "disabling timer for %s", iface->Name);
			clear_timer(&iface->tm);
		}
	}

	iface=IfaceList; 
	while(iface)
	{
		struct Interface *next_iface = iface->next;
		struct AdvPrefix *prefix;
		struct AdvRoute *route;
#ifdef SUPPORT_RDNSS_OPTION
		struct AdvRDNSS *rdnss;
#endif
#ifdef SUPPORT_DNSSL_OPTION
		struct AdvDNSSL *dnssl;
#endif

		dlog(LOG_DEBUG, 4, "freeing interface %s", iface->Name);
		
		set_ipv6_mreq_allrouters( sock, iface->Name, IPV6_LEAVE_GROUP );
		
		prefix = iface->AdvPrefixList;
		while (prefix)
		{
			struct AdvPrefix *next_prefix = prefix->next;
			
			free(prefix);
			prefix = next_prefix;
		}
		
		route = iface->AdvRouteList;
		while (route)
		{
			struct AdvRoute *next_route = route->next;

			free(route);
			route = next_route;
		}  
#ifdef SUPPORT_RDNSS_OPTION
		rdnss = iface->AdvRDNSSList;
		while (rdnss)
		{
			struct AdvRDNSS *next_rdnss = rdnss->next;

			free(rdnss);
			rdnss = next_rdnss;
		}
#endif
#ifdef SUPPORT_DNSSL_OPTION
		dnssl = iface->AdvDNSSLList;
		while (dnssl)
		{
			struct AdvDNSSL *next_dnssl = dnssl->next;
			int i;

			for (i = 0; i < dnssl->AdvDNSSLNumber; i++)
				free(dnssl->AdvDNSSLSuffixes[i]);
			free(dnssl->AdvDNSSLSuffixes);
			free(dnssl);

			dnssl = next_dnssl;
		}
#endif
		free(iface);
		iface = next_iface;
	}

	IfaceList = NULL;

	/* reread config file */
	if (readin_config(conf_file) < 0)
		exit(1);

	config_interface();
	kickoff_adverts();

	flog(LOG_INFO, "resuming normal operation");
}

void request_radvd_reload( int enable )
{
	// web page check enable option or not? 
	sighup_received = ( enable ? 1 : 2 );
#ifdef HAVE_SYSTEM_REINIT
	if(sighup_received == 1)
	{
		create_radvd();
	}
	else if(sighup_received == 2 && radvd_is_running == 1)
	{
		radvd_quitting = 1;
		while(radvd_quitting){
			cyg_thread_delay(200);
		}
		radvd_is_running = 0;
		cyg_thread_kill(radvd_thread_handle);
		cyg_thread_delete(radvd_thread_handle);
	}
#endif
}

static void
sighup_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGHUP, sighup_handler);

	dlog(LOG_DEBUG, 4, "sighup_handler called");

	sighup_received = 1;
}

static void
sigterm_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGTERM, sigterm_handler);

	dlog(LOG_DEBUG, 4, "sigterm_handler called");

	sigterm_received = 1;
}

static void
sigint_handler(int sig)
{
	/* Linux has "one-shot" signals, reinstall the signal handler */
	signal(SIGINT, sigint_handler);

	dlog(LOG_DEBUG, 4, "sigint_handler called");

	sigint_received = 1;
}

static int
drop_root_privileges(const char *username)
{
	struct passwd *pw = NULL;
	pw = getpwnam(username);
	if (pw) {
		if (initgroups(username, pw->pw_gid) != 0 || setgid(pw->pw_gid) != 0 || setuid(pw->pw_uid) != 0) {
			flog(LOG_ERR, "Couldn't change to '%.32s' uid=%d gid=%d\n", 
					username, pw->pw_uid, pw->pw_gid);
			return (-1);
		}
	}
	else {
		flog(LOG_ERR, "Couldn't find user '%.32s'\n", username);
		return (-1);
	}
	return 0;
}

static int
check_conffile_perm(const char *username, const char *conf_file)
{
	struct stat *st = NULL;
	struct passwd *pw = NULL;
	FILE *fp = fopen(conf_file, "r");

	if (fp == NULL) {
		flog(LOG_ERR, "can't open %s: %s", conf_file, strerror(errno));
		return (-1);
	}
	fclose(fp);

	st = malloc(sizeof(struct stat));
	if (st == NULL)
		goto errorout;

	if (!username)
		username = "root";
	
	pw = getpwnam(username);

	if (stat(conf_file, st) || pw == NULL)
		goto errorout;

	if (st->st_mode & S_IWOTH) {
                flog(LOG_ERR, "Insecure file permissions (writable by others): %s", conf_file);
		goto errorout;
        }

	/* for non-root: must not be writable by self/own group */
	if (strncmp(username, "root", 5) != 0 &&
	    ((st->st_mode & S_IWGRP && pw->pw_gid == st->st_gid) ||
	     (st->st_mode & S_IWUSR && pw->pw_uid == st->st_uid))) {
                flog(LOG_ERR, "Insecure file permissions (writable by self/group): %s", conf_file);
		goto errorout;
        }

	free(st);
        return 0;

errorout:
	free(st);
	return(-1);
}

int
check_ip6_forwarding(void)
{
	int forw_sysctl[] = { SYSCTL_IP6_FORWARDING };
	int value;
	size_t size = sizeof(value);
	FILE *fp = NULL;

#ifdef __linux__
	fp = fopen(PROC_SYS_IP6_FORWARDING, "r");
	if (fp) {
		fscanf(fp, "%d", &value);
		fclose(fp);
	}
	else
		flog(LOG_DEBUG, "Correct IPv6 forwarding procfs entry not found, "
	                       "perhaps the procfs is disabled, "
	                        "or the kernel interface has changed?");
#endif /* __linux__ */

	if (!fp && sysctl(forw_sysctl, sizeof(forw_sysctl)/sizeof(forw_sysctl[0]),
	    &value, &size, NULL, 0) < 0) {
		flog(LOG_DEBUG, "Correct IPv6 forwarding sysctl branch not found, "
			"perhaps the kernel interface has changed?");
		return(0);	/* this is of advisory value only */
	}
	
	if (value != 1) {
		flog(LOG_DEBUG, "IPv6 forwarding setting is: %u, should be 1", value);
		return(-1);
	}
		
	return(0);
}

static int
readin_config(char *fname)
{
	int ret = 0;
	
	if ((yyin = fopen(fname, "r")) == NULL)
	{
		flog(LOG_ERR, "can't open %s: %s", fname, strerror(errno));
		return (-1);
	}

	if (yyparse() != 0)
	{
		flog(LOG_ERR, "error parsing or activating the config file: %s", fname);
		ret = -1;	// make file close safely (empty file will cause it error)
	}
	
	fclose(yyin);
	return ret;
}

static void
version(void)
{
	fprintf(stderr, "Version: %s\n\n", VERSION);
	fprintf(stderr, "Compiled in settings:\n");
	fprintf(stderr, "  default config file		\"%s\"\n", PATH_RADVD_CONF);
	fprintf(stderr, "  default pidfile		\"%s\"\n", PATH_RADVD_PID);
	fprintf(stderr, "  default logfile		\"%s\"\n", PATH_RADVD_LOG);
	fprintf(stderr, "  default syslog facililty	%d\n", LOG_FACILITY);
	fprintf(stderr, "Please send bug reports or suggestions to %s.\n",
		CONTACT_EMAIL);

	exit(1);	
}

static
void
usage(void)
{
	fprintf(stderr, "usage: %s %s\n", pname, usage_str);
	exit(1);	
}

