/*
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE".  Use of the mrouted program represents acceptance of
 * the terms and conditions listed in that file.
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 *
 * main.c,v 3.8.4.29 1998/03/01 01:49:00 fenner Exp
 */

/*
 * Written by Steve Deering, Stanford University, February 1989.
 *
 * (An earlier version of DVMRP was implemented by David Waitzman of
 *  BBN STC by extending Berkeley's routed program.  Some of Waitzman's
 *  extensions have been incorporated into mrouted, but none of the
 *  original routed code has been adopted.)
 */


#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif

//#include <err.h>
#include "defs.h"
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <fcntl.h>
#include <paths.h>

#ifdef SNMP
#include "snmp.h"
#endif

#ifdef __ECOS
char mrouted_config[320];

int mrouted_state = 0;
//unsigned char mrouted_stack[24*1024];
unsigned char mrouted_stack[8*1024];
cyg_handle_t mrouted_thread_handle;
cyg_thread mrouted_thread_obj;


cyg_flag_t mrouted_flag;
#ifdef HAVE_SYSTEM_REINIT
int mrouted_quitting=0;
static int mrouted_running=0;

#endif

//cyg_handle_t wsc_alarm_hdl;
//cyg_alarm wsc_alarm;
//int wscd_lock_stat;		// for replace ("/tmp/wscd_lock_stat")

char igmp_down_if_name[IFNAMSIZ];
char igmp_down_if_idx;
#ifdef CONFIG_IGMPPROXY_MULTIWAN
char igmp_up_if_name[MAXWAN][IFNAMSIZ];
#else
char igmp_up_if_name[IFNAMSIZ];
#endif
#ifdef CONFIG_IGMPPROXY_MULTIWAN
char igmp_up_if_idx[MAXWAN];
#else
char igmp_up_if_idx;
#endif

#endif

char *configfilename;
char versionstring[100];

static char pidfilename[]  = _PATH_MROUTED_PID;
static char dumpfilename[] = _PATH_MROUTED_DUMP;
static char cachefilename[] = _PATH_MROUTED_CACHE;
static char genidfilename[] = _PATH_MROUTED_GENID;

static int haveterminal = 1;
int did_final_init = 0;

static int sighandled = 0;
#define	GOT_SIGINT	0x01
#define	GOT_SIGHUP	0x02
#define	GOT_SIGUSR1	0x04
#define	GOT_SIGUSR2	0x08




int cache_lifetime 	= DEFAULT_CACHE_LIFETIME;
int prune_lifetime	= AVERAGE_PRUNE_LIFETIME;

int mrouted_debug = 0;
char *progname;
time_t mrouted_init_time;

#define CYGNUM_MROUTED_THREAD_PRIORITY 16

#ifdef SNMP
#define NHANDLERS	34
#else
#define NHANDLERS	2
#endif
//typedef void ihfunc_t;

static struct ihandler {
    int fd;			/* File descriptor		 */
   // ihfunc_t func;		/* Function to call with &fd_set */
} ihandlers[NHANDLERS];
static int nhandlers = 0;

static struct debugname {
    char	*name;
    int		 level;
    int		 nchars;
} debugnames[] = {
    {	"packet",	DEBUG_PKT,	2	},
    {	"pkt",		DEBUG_PKT,	3	},
    {	"pruning",	DEBUG_PRUNE,	1	},
    {	"prunes",	DEBUG_PRUNE,	1	},
    {	"routing",	DEBUG_ROUTE,	1	},
    {	"routes",	DEBUG_ROUTE,	1	},
    {   "route_detail",	DEBUG_RTDETAIL, 6	},
    {   "rtdetail",	DEBUG_RTDETAIL, 2	},
    {	"peers",	DEBUG_PEER,	2	},
    {	"neighbors",	DEBUG_PEER,	1	},
    {	"cache",	DEBUG_CACHE,	1	},
    {	"timeout",	DEBUG_TIMEOUT,	1	},
    {	"callout",	DEBUG_TIMEOUT,	2	},
    {	"interface",	DEBUG_IF,	2	},
    {	"vif",		DEBUG_IF,	1	},
    {	"membership",	DEBUG_MEMBER,	1	},
    {	"groups",	DEBUG_MEMBER,	1	},
    {	"traceroute",	DEBUG_TRACE,	2	},
    {	"mtrace",	DEBUG_TRACE,	2	},
    {	"igmp",		DEBUG_IGMP,	1	},
    {	"icmp",		DEBUG_ICMP,	2	},
    {	"rsrr",		DEBUG_RSRR,	2	},
    {	"3",		0xffffffff,	1	}	/* compat. */
};

/*
 * Forward declarations.
 */
static void final_init __P((void *));
static void fasttimer __P((void *));
static void timer __P((void *));
static void timer_routine __P((void *));

static void dump __P((void));
static void dump_version __P((FILE *));
static void fdump __P((void));
static void cdump __P((void));
static void restart __P((void));
static void handler __P((int));
static void cleanup __P((void));
static void resetlogging __P((void *));
static void usage __P((void));
static void cleanup_mrouted();
/* To shut up gcc -Wstrict-prototypes */

int mrouted_main __P((cyg_addrword_t data));


int
register_input_handler(fd, func)
    int fd;
    ihfunc_t func;
{
    if (nhandlers >= NHANDLERS)
	return -1;

    ihandlers[nhandlers].fd = fd;
    //ihandlers[nhandlers++].func = func;

    return 0;
}

int
	
mrouted_main(cyg_addrword_t data)



{
    register int recvlen;
    int dummy;
    FILE *fp;
    struct timeval tv, difftime, curtime, lasttime, *timeout;
    u_int32 prev_genid;
    int vers;
    fd_set rfds, readers;
    int nfds, n, i, secs;
	char *wan_if=NULL;
	char *lan_if=NULL;
    //char todaysversion[];
    struct sigaction sa;
#ifdef SNMP
    struct timeval  timeout, *tvp = &timeout;
    struct timeval  sched, *svp = &sched, now, *nvp = &now;
    int index, block;
#endif
	 
	diag_printf("mrouted start %s %s ...\n", igmp_up_if_name,igmp_down_if_name);
	
	
#ifdef SYSV
    setlinebuf(stderr);
#endif
#if 0
#ifdef LOG_DAEMON
    (void)openlog("mrouted", LOG_PID, LOG_DAEMON);
    (void)setlogmask(LOG_UPTO(LOG_NOTICE));
#else
    (void)openlog("mrouted", LOG_PID);
#endif
#endif

    age_callout_init();
    if(init_igmp()==0)
		return 0;
	
    //init_ipip();
    //init_routes();
    init_ktable();
#ifndef OLD_KERNEL
    /*
     * Unfortunately, you can't k_get_version() unless you've
     * k_init_dvmrp()'d.  Now that we want to move the
     * k_init_dvmrp() to later in the initialization sequence,
     * we have to do the disgusting hack of initializing,
     * getting the version, then stopping the kernel multicast
     * forwarding.
     */
    k_init_dvmrp();
    vers = k_get_version();
    //k_stop_dvmrp();
	//k_reinit_dvmrp();
#endif
#ifdef HAVE_SYSTEM_REINIT	
	mrouted_running	=1;
#endif	
    init_vifs();

    sa.sa_handler = handler;
    sa.sa_flags = 0;	/* Interrupt system calls */
    sigemptyset(&sa.sa_mask);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    FD_ZERO(&readers);
    FD_SET(igmp_socket, &readers);
    nfds = igmp_socket + 1;
    for (i = 0; i < nhandlers; i++) {
	FD_SET(ihandlers[i].fd, &readers);
	if (ihandlers[i].fd >= nfds)
	    nfds = ihandlers[i].fd + 1;
    }
	#if 0
    IF_DEBUG(DEBUG_IF)
	dump_vifs(stderr);
    IF_DEBUG(DEBUG_ROUTE)
	dump_routes(stderr);
	#endif
    /* schedule first timer interrupt */
    //timer_setTimer(1, fasttimer, NULL);
    timer_setTimer(TIMER_INTERVAL, timer, NULL);

    if (mrouted_debug == 0) {
	/*
	 * Detach from the terminal
	 */
	int t;

	haveterminal = 0;
	#if 0
	//if (fork()) exit(0);
	(void)close(0);
	(void)close(1);
	(void)close(2);
	(void)open("/", 0);
	(void)dup2(0, 1);
	(void)dup2(0, 2);
	#endif	
#if defined(SYSV) || defined(linux)
	(void)setpgrp();
#else
#ifdef TIOCNOTTY
	t = open(_PATH_TTY, 2);
	if (t >= 0) {
	    (void)ioctl(t, TIOCNOTTY, (char *)0);
	    (void)close(t);
	}
#else
	//if (setsid() < 0)
	 //   perror("setsid");
#endif
#endif
    }

    fp = fopen(pidfilename, "w");		
    if (fp != NULL) {
	fprintf(fp, "%d\n", (int)getpid());
	(void) fclose(fp);
    }

    /* XXX HACK
     * This will cause black holes for the first few seconds after startup,
     * since we are exchanging routes but not actually forwarding.
     * However, it eliminates much of the startup transient.
     *
     * It's possible that we can set a flag which says not to report any
     * routes (just accept reports) until this timer fires, and then
     * do a report_to_all_neighbors(ALL_ROUTES) immediately before
     * turning on DVMRP.
     */
 	
    timer_setTimer(3, final_init, NULL);
	
    /*
     * Main receive loop.
     */
    dummy = 0;
    difftime.tv_usec = 0;
    gettimeofday(&curtime, NULL);
    lasttime = curtime;

   while(1)
   {
   	
#ifdef HAVE_SYSTEM_REINIT
	if(mrouted_quitting)
	{
		free_mrouted( );
		mrouted_quitting=0;
		break;
	}
#endif
  	if(igmp_socket ==0)
		break;
	bcopy((char *)&readers, (char *)&rfds, sizeof(rfds));
	secs = timer_nextTimer();
#ifndef HAVE_SYSTEM_REINIT	
	if (secs == -1)
	    timeout = NULL;
	else {
	    timeout = &tv;
	    timeout->tv_sec = secs;
	    timeout->tv_usec = 0;
	}
#else
	timeout = &tv;
	timeout->tv_sec	= 1; /* 1 second */
	timeout->tv_usec = 0;
#endif	
	#if 0
	if (sighandled) {
	    if (sighandled & GOT_SIGINT) {
		sighandled &= ~GOT_SIGINT;
		break;
	    }
	    if (sighandled & GOT_SIGHUP) {
		sighandled &= ~GOT_SIGHUP;
		restart();
	    }
	    if (sighandled & GOT_SIGUSR1) {
		sighandled &= ~GOT_SIGUSR1;
		fdump();
	    }
	    if (sighandled & GOT_SIGUSR2) {
		sighandled &= ~GOT_SIGUSR2;
		cdump();
	    }
	}
	#endif
	if ((n = select(nfds, &rfds, NULL, NULL, timeout)) < 0) {
            if (errno != EINTR)
                log(LOG_WARNING, errno, "select failed");
            continue;
        }

	if (n > 0) {
	    if (FD_ISSET(igmp_socket, &rfds)) {
		recvlen = recvfrom(igmp_socket, recv_buf, RECV_BUF_SIZE,
				   0, NULL, &dummy);
		if (recvlen < 0) {
		    if (errno != EINTR) log(LOG_ERR, errno, "recvfrom");
		    continue;
		}
		accept_igmp(recvlen);
	    }

	  
    }
		/*
		 * Handle timeout queue.
		 *
		 * If select + packet processing took more than 1 second,
		 * or if there is a timeout pending, age the timeout queue.
		 *
		 * If not, collect usec in difftime to make sure that the
		 * time doesn't drift too badly.
		 *
		 * If the timeout handlers took more than 1 second,
		 * age the timeout queue again.  XXX This introduces the
		 * potential for infinite loops!
		 */
		do {
		    /*
		     * If the select timed out, then there's no other
		     * activity to account for and we don't need to
		     * call gettimeofday.
		     */
		    if (n == 0) {
			curtime.tv_sec = lasttime.tv_sec + secs;
			curtime.tv_usec = lasttime.tv_usec;
			n = -1;	/* don't do this next time through the loop */
		    } else
			gettimeofday(&curtime, NULL);
		    difftime.tv_sec = curtime.tv_sec - lasttime.tv_sec;
		    difftime.tv_usec += curtime.tv_usec - lasttime.tv_usec;
		    while (difftime.tv_usec >= 1000000) {
			difftime.tv_sec++;
			difftime.tv_usec -= 1000000;
		    }
		    if (difftime.tv_usec < 0) {
			difftime.tv_sec--;
			difftime.tv_usec += 1000000;
		    }
		    lasttime = curtime;
		    if (secs == 0 || difftime.tv_sec > 0)
			age_callout_queue(difftime.tv_sec);
		    secs = -1;
		} while (difftime.tv_sec > 0);
	}
#ifdef HAVE_SYSTEM_REINIT
	mrouted_running=0;
	if(mrouted_quitting)
	{
		free_mrouted( );
		mrouted_quitting=0;
		//diag_printf("-------[%s]:[%d]\n",__FUNCTION__,__LINE__);
	}
#endif

	diag_printf("exiting mrouted\n");
    log(LOG_NOTICE, 0, "%s exiting", versionstring);
#ifndef HAVE_SYSTEM_REINIT	
    cleanup_mrouted();
#endif
    //exit(0);
}

static void
usage()
{
	fprintf(stderr,
		"usage: mrouted [-p] [-c configfile] [-d [debug_level]]\n");
	exit(1);
}

static void
final_init(i)
    void *i;
{
    char *s = (char *)i;
	
    log(LOG_NOTICE, 0, "%s%s", versionstring, s ? s : "");
    if (s){
		free(s);
			
#ifdef 	ECOS_DBG_STAT
		dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 0);
#endif
	}
	
    //k_init_dvmrp();		/* enable DVMRP routing in kernel */
	//k_reinit_dvmrp();

    /*
     * Install the vifs in the kernel as late as possible in the
     * initialization sequence.
     */
    //init_installvifs();
	query_groups();
	
    time(&mrouted_init_time);
    did_final_init = 1;
}


/*
 * The 'virtual_time' variable is initialized to a value that will cause the
 * first invocation of timer() to send a probe or route report to all vifs
 * and send group membership queries to all subnets for which this router is
 * querier.  This first invocation occurs approximately TIMER_INTERVAL seconds
 * after the router starts up.   Note that probes for neighbors and queries
 * for group memberships are also sent at start-up time, as part of initial-
 * ization.  This repetition after a short interval is desirable for quickly
 * building up topology and membership information in the presence of possible
 * packet loss.
 *
 * 'virtual_time' advances at a rate that is only a crude approximation of
 * real time, because it does not take into account any time spent processing,
 * and because the timer intervals are sometimes shrunk by a random amount to
 * avoid unwanted synchronization with other routers.
 */

u_long virtual_time = 0;


/*
 * Timer routine.  Performs periodic neighbor probing, route reporting, and
 * group querying duties, and drives various timers in routing entries and
 * virtual interface data structures.
 */
static void
timer(i)
    void *i;
{
	//diag_printf("virtual_time:%x,[%s]:[%d].\n",virtual_time,__FUNCTION__,__LINE__);
    //age_routes();	/* Advance the timers in the route entries     */
   // age_vifs();		/* Advance the timers for neighbors */
   // age_table_entry();	/* Advance the timers for the cache entries */

    if (virtual_time % IGMP_QUERY_INTERVAL == 0) {
	/*
	 * Time to query the local group memberships on all subnets
	 * for which this router is the elected querier.
	 */
	//diag_printf("time to send query!\n");
	query_groups();
    }

  
    /*
     * Advance virtual time
     */
    virtual_time += TIMER_INTERVAL;
    //timer_setTimer(TIMER_INTERVAL, timer_routine, NULL);
    timer_setTimer(TIMER_INTERVAL, timer, NULL);
}


static void
cleanup_mrouted()
{
    static in_cleanup = 0;

    if (!in_cleanup) {
	in_cleanup++;
#ifdef RSRR
	rsrr_clean();
#endif /* RSRR */
	//expire_all_routes();
	//report_to_all_neighbors(ALL_ROUTES);
	if (did_final_init){
		
	    k_stop_dvmrp();
	}
    }
}

/*
 * Signal handler.  Take note of the fact that the signal arrived
 * so that the main loop can take care of it.
 */
static void
handler(sig)
    int sig;
{
    switch (sig) {
	case SIGINT:
	case SIGTERM:
	    sighandled |= GOT_SIGINT;
	    break;

	case SIGHUP:
	    sighandled |= GOT_SIGHUP;
	    break;

	case SIGUSR1:
	    sighandled |= GOT_SIGUSR1;
	    break;

	case SIGUSR2:
	    sighandled |= GOT_SIGUSR2;
	    break;
    }
}

/*
 * Dump internal data structures to stderr.
 */
static void
dump()
{
    dump_vifs(stderr);
    dump_routes(stderr);
}

static void
dump_version(fp)
    FILE *fp;
{
    time_t t;

    time(&t);
    fprintf(fp, "%s ", versionstring);
    if (did_final_init)
	    fprintf(fp, "up %s",
		    scaletime(t - mrouted_init_time));
    else
	    fprintf(fp, "(not yet initialized)");
    fprintf(fp, " %s\n", ctime(&t));
}

/*
 * Dump internal data structures to a file.
 */
static void
fdump()
{
    FILE *fp;

    fp = fopen(dumpfilename, "w");
    if (fp != NULL) {
	dump_version(fp);
	dump_vifs(fp);
	dump_routes(fp);
	(void) fclose(fp);
    }
}


/*
 * Dump local cache contents to a file.
 */
static void
cdump()
{
    FILE *fp;

    fp = fopen(cachefilename, "w");
    if (fp != NULL) {
	dump_version(fp);
	dump_cache(fp); 
	(void) fclose(fp);
    }
}


/*
 * Restart mrouted
 */
static void
restart_mrouted()
{
    char *s;

    s = (char *)malloc(sizeof(" restart"));
    if (s == NULL)
		log(LOG_ERR, 0, "out of memory");
	else{
#ifdef 	ECOS_DBG_STAT
		dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif

 		strcpy(s, " restart");
	}
	//diag_printf("free callouts![%s]:[%d].\n",__FUNCTION__,__LINE__);
    /*
     * reset all the entries
     */
    //free_all_prunes();
   // free_all_routes();
    free_all_callouts();
	//diag_printf("stop vif![%s]:[%d].\n",__FUNCTION__,__LINE__);
	
    stop_all_vifs();
	//diag_printf("exit ktable![%s]:[%d].\n",__FUNCTION__,__LINE__);
	exit_ktable();
	//exit_igmp();
   //diag_printf("stop dvmrp![%s]:[%d].\n",__FUNCTION__,__LINE__);
    k_stop_dvmrp();
   // close(igmp_socket);
   	//diag_printf("close udp socket![%s]:[%d].\n",__FUNCTION__,__LINE__);
    close(udp_socket);
   
#ifdef 	ECOS_DBG_STAT
	dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
	//igmp_socket = 0;
	udp_socket =0;
    did_final_init = 0;

    /*
     * start processing again
     */
    dvmrp_genid++;
	#if 0
    	if(init_igmp()==0)
	{
		return;
	}	
	#endif
    //init_routes();
    
	//diag_printf("init_ktable![%s]:[%d].\n",__FUNCTION__,__LINE__);
    init_ktable();
	//diag_printf("init_dvmrp![%s]:[%d].\n",__FUNCTION__,__LINE__);
	k_init_dvmrp();
	//diag_printf("init_vifs![%s]:[%d].\n",__FUNCTION__,__LINE__);
    init_vifs();
	
    /*XXX Schedule final_init() as main does? */
	//diag_printf("final_init![%s]:[%d].\n",__FUNCTION__,__LINE__);
    final_init(s);

    /* schedule timer interrupts */
    //timer_setTimer(1, fasttimer, NULL);
    
	//diag_printf("query_groups![%s]:[%d].\n",__FUNCTION__,__LINE__);
    query_groups();
    virtual_time=0;
	//diag_printf("setTimer![%s]:[%d].\n",__FUNCTION__,__LINE__);
    timer_setTimer(TIMER_INTERVAL, timer, NULL);
	diag_printf("restart mrouted %d......\n",dvmrp_genid);

}

#define LOG_MAX_MSGS	20	/* if > 20/minute then shut up for a while */
#define LOG_SHUT_UP	600	/* shut up for 10 minutes */
static int log_nmsgs = 0;

static void
resetlogging(arg)
    void *arg;
{
    int nxttime = 60;
    void *narg = NULL;

    if (arg == NULL && log_nmsgs > LOG_MAX_MSGS) {
	nxttime = LOG_SHUT_UP;
	narg = (void *)&log_nmsgs;	/* just need some valid void * */
	syslog(LOG_WARNING, "logging too fast, shutting up for %d minutes",
			LOG_SHUT_UP / 60);
    } else {
	log_nmsgs = 0;
    }

    timer_setTimer(nxttime, resetlogging, narg);
}

char *
scaletime(t)
    u_long t;
{
#define SCALETIMEBUFLEN 20
    static char buf1[20];
    static char buf2[20];
    static char *buf = buf1;
    char *p;

    p = buf;
    if (buf == buf1)
	buf = buf2;
    else
	buf = buf1;

    /* XXX snprintf */
    sprintf(p, "%2ld:%02ld:%02ld", t / 3600, (t % 3600) / 60, t % 60);
    p[SCALETIMEBUFLEN - 1] = '\0';
    return p;
}

#ifdef RINGBUFFER
#define NLOGMSGS 10000
#define LOGMSGSIZE 200
char *logmsg[NLOGMSGS];
static int logmsgno = 0;

void
printringbuf()
{
    FILE *f;
    int i;

    f = fopen("/var/tmp/mrouted.log", "a");
    if (f == NULL) {
	log(LOG_ERR, errno, "can't open /var/tmp/mrouted.log");
	/*NOTREACHED*/
    }
    fprintf(f, "--------------------------------------------\n");

    i = (logmsgno + 1) % NLOGMSGS;

    while (i != logmsgno) {
	if (*logmsg[i]) {
	    fprintf(f, "%s\n", logmsg[i]);
	    *logmsg[i] = '\0';
	}
	i = (i + 1) % NLOGMSGS;
    }

    fclose(f);
}
#endif

#if 0
/*
 * Log errors and other messages to the system log daemon and to stderr,
 * according to the severity of the message and the current debug level.
 * For errors of severity LOG_ERR or worse, terminate the program.
 */
#ifdef __STDC__
void
log(int severity, int syserr, char *format, ...)
{
    va_list ap;
    static char fmt[211] = "warning - ";
    char *msg;
    struct timeval now;
    time_t now_sec;
    struct tm *thyme;
#ifdef RINGBUFFER
    static int ringbufinit = 0;
#endif

    va_start(ap, format);
#else
/*VARARGS3*/
void
log(severity, syserr, format, va_alist)
    int severity, syserr;
    char *format;
    va_dcl
{
    va_list ap;
    static char fmt[311] = "warning - ";
    char *msg;
    char tbuf[20];
    struct timeval now;
    time_t now_sec;
    struct tm *thyme;
#ifdef RINGBUFFER
    static int ringbufinit = 0;
#endif

    va_start(ap);
#endif
    vsnprintf(&fmt[10], sizeof(fmt) - 10, format, ap);
    va_end(ap);
    msg = (severity == LOG_WARNING) ? fmt : &fmt[10];

#ifdef RINGBUFFER
    if (!ringbufinit) {
	int i;

	for (i = 0; i < NLOGMSGS; i++) {
	    logmsg[i] = malloc(LOGMSGSIZE);
	    if (logmsg[i] == 0) {
		syslog(LOG_ERR, "out of memory");
		exit(-1);
	    }
	    *logmsg[i] = 0;
	}
	ringbufinit = 1;
    }
    gettimeofday(&now,NULL);
    now_sec = now.tv_sec;
    thyme = localtime(&now_sec);
    snprintf(logmsg[logmsgno++], LOGMSGSIZE, "%02d:%02d:%02d.%03ld %s err %d",
		    thyme->tm_hour, thyme->tm_min, thyme->tm_sec,
		    now.tv_usec / 1000, msg, syserr);
    logmsgno %= NLOGMSGS;
    if (severity <= LOG_NOTICE)
#endif
    /*
     * Log to stderr if we haven't forked yet and it's a warning or worse,
     * or if we're debugging.
     */
    if (haveterminal && (mrouted_debug || severity <= LOG_WARNING)) {
	gettimeofday(&now,NULL);
	now_sec = now.tv_sec;
	thyme = localtime(&now_sec);
	if (!mrouted_debug)
	    fprintf(stderr, "%s: ", progname);
	fprintf(stderr, "%02d:%02d:%02d.%03ld %s", thyme->tm_hour,
		    thyme->tm_min, thyme->tm_sec, now.tv_usec / 1000, msg);
	#if 0
	if (syserr == 0)
	    fprintf(stderr, "\n");
	else if (syserr < sys_nerr)
	    fprintf(stderr, ": %s\n", sys_errlist[syserr]);
	else
	    fprintf(stderr, ": errno %d\n", syserr);
	#endif
    }

    /*
     * Always log things that are worse than warnings, no matter what
     * the log_nmsgs rate limiter says.
     * Only count things worse than debugging in the rate limiter
     * (since if you put daemon.debug in syslog.conf you probably
     * actually want to log the debugging messages so they shouldn't
     * be rate-limited)
     */
    if ((severity < LOG_WARNING) || (log_nmsgs < LOG_MAX_MSGS)) {
	if (severity < LOG_DEBUG)
	    log_nmsgs++;
	if (syserr != 0) {
	    errno = syserr;
	    syslog(severity, "%s: %m", msg);
	} else
	    syslog(severity, "%s", msg);
    }

    if (severity <= LOG_ERR) exit(-1);
}
#endif
#ifdef DEBUG_MFC
void
md_log(what, origin, mcastgrp)
    int what;
    u_int32 origin, mcastgrp;
{
    static FILE *f = NULL;
    struct timeval tv;
    u_int32 buf[4];

    if (!f) {
	if ((f = fopen("/tmp/mrouted.clog", "w")) == NULL) {
	    log(LOG_ERR, errno, "open /tmp/mrouted.clog");
	}
    }

    gettimeofday(&tv, NULL);
    buf[0] = tv.tv_sec;
    buf[1] = what;
    buf[2] = origin;
    buf[3] = mcastgrp;

    fwrite(buf, sizeof(u_int32), 4, f);
}
#endif

int mcastReservedEnabled=1;

void rtl_set_mrouteReserveEnable(unsigned int enabled)
{	
	mcastReservedEnabled =enabled;
	
	//printf("reserved:%d[%s][%d].\n",mcastReservedEnabled,__FUNCTION__,__LINE__);
	return;
}
unsigned int rtl_get_mrouteReserveEnable(void)
{	

	return mcastReservedEnabled;
}

int cyg_igmpproxy_init =0;

void create_mrouted(char * wan_if, char * lan_if)
{
	int flag=0;
	diag_printf("\ncreat mrouted %s %s.\n",wan_if,lan_if);
	
#ifdef HAVE_SYSTEM_REINIT
	if(mrouted_quitting){
		int cnt;
		while(mrouted_quitting){
			if(cnt++ >5){
				diag_printf("mrouted error:[%s] wait cleanup failed\n",__FUNCTION__);
				return 0;
			}
			sleep(1);
		}
		if(cyg_igmpproxy_init){
			diag_printf("!![%s] [%d]\n",__FUNCTION__,__LINE__);
			cyg_thread_kill(mrouted_thread_handle);
			cyg_thread_delete(mrouted_thread_handle);
		}
	}
#endif
	strcpy(igmp_down_if_name, lan_if);
	strcpy(igmp_up_if_name, wan_if);

	if (cyg_igmpproxy_init
	#ifdef HAVE_SYSTEM_REINIT	
	&&(mrouted_quitting==0)
	#endif
		)
	{
		diag_printf("\nrestart mrouted %s %s.\n",wan_if,lan_if);
		restart_mrouted();
		return ;
	}
	
	cyg_igmpproxy_init=1;
	//diag_printf("\ncyg_igmpproxy_init:%d,wan_if:%s,lan_if%s,flag:%d,[%s]:[%d].\n",cyg_igmpproxy_init,wan_if,lan_if,flag,__FUNCTION__,__LINE__);
	{
	/* Create the thread */
	cyg_thread_create(CYGNUM_MROUTED_THREAD_PRIORITY,
		      mrouted_main,
		      flag,
		      "mrouted thread",
		      &mrouted_stack,
		      sizeof(mrouted_stack),
		      &mrouted_thread_handle,
		      &mrouted_thread_obj);
	/* Let the thread run when the scheduler starts */
	cyg_thread_resume(mrouted_thread_handle);
	}
	

	#ifdef  ECOS_DBG_STAT
	dbg_igmpproxy_index=dbg_stat_register("igmpproxy");
  	  #endif

}

#ifdef HAVE_SYSTEM_REINIT
#define MROUTED_MAX_WAIT_CNT	300
void free_mrouted( )
{
	//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
	free_all_callouts();
	//diag_printf("stop vif![%s]:[%d].\n",__FUNCTION__,__LINE__);
	stop_all_vifs();
	//diag_printf("exit ktable![%s]:[%d].\n",__FUNCTION__,__LINE__);
	exit_ktable();
	exit_igmp();
	//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
	k_stop_dvmrp();

	 if(igmp_socket)
 	{
 		//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
		close(igmp_socket);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		igmp_socket=0;
 	}
   	//diag_printf("close udp socket![%s]:[%d].\n",__FUNCTION__,__LINE__);
	if(udp_socket)
	{
  		 close(udp_socket);
		 
#ifdef ECOS_DBG_STAT
		 dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		 udp_socket=0;
	}	
	 return;
}

void clean_mrouted( )
{
	int cnt=0;
	if(cyg_igmpproxy_init)
	{
		diag_printf("kill mrouted %d!\n",mrouted_running);
		if(mrouted_running)
			mrouted_quitting = 1;
		
		while(mrouted_quitting){
			cyg_thread_delay(10); //10*0.01s
			cnt++;
			if(cnt> MROUTED_MAX_WAIT_CNT){
				diag_printf("cnt:%d  [%s][%d]!\n",cnt,__FUNCTION__,__LINE__);
				break;
			}	
		}	
		
	#if 0 //luminais mark
		cyg_thread_kill(mrouted_thread_handle);
		cyg_thread_delete(mrouted_thread_handle);
	#endif
		mrouted_running =0;
		cyg_igmpproxy_init =0;
		
	}
	return ;
}
#endif


