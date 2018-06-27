/*	$KAME: main.c,v 1.10 2003/09/02 09:57:04 itojun Exp $	*/

/*
 *  Copyright (c) 1998 by the University of Oregon.
 *  All rights reserved.
 *
 *  Permission to use, copy, modify, and distribute this software and
 *  its documentation in source and binary forms for lawful
 *  purposes and without fee is hereby granted, provided
 *  that the above copyright notice appear in all copies and that both
 *  the copyright notice and this permission notice appear in supporting
 *  documentation, and that any documentation, advertising materials,
 *  and other materials related to such distribution and use acknowledge
 *  that the software was developed by the University of Oregon.
 *  The name of the University of Oregon may not be used to endorse or 
 *  promote products derived from this software without specific prior 
 *  written permission.
 *
 *  THE UNIVERSITY OF OREGON DOES NOT MAKE ANY REPRESENTATIONS
 *  ABOUT THE SUITABILITY OF THIS SOFTWARE FOR ANY PURPOSE.  THIS SOFTWARE IS
 *  PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND 
 *  NON-INFRINGEMENT.
 *
 *  IN NO EVENT SHALL UO, OR ANY OTHER CONTRIBUTOR BE LIABLE FOR ANY
 *  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, WHETHER IN CONTRACT,
 *  TORT, OR OTHER FORM OF ACTION, ARISING OUT OF OR IN CONNECTION WITH,
 *  THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *  Other copyrights might apply to parts of this software and are so
 *  noted when applicable.
 */
/*
 *  Questions concerning this software should be directed to 
 *  Kurt Windisch (kurtw@antc.uoregon.edu)
 */
/*
 * Part of this program has been derived from PIM sparse-mode pimd.
 * The pimd program is covered by the license in the accompanying file
 * named "LICENSE.pimd".
 *  
 * The pimd program is COPYRIGHT 1998 by University of Southern California.
 *
 * Part of this program has been derived from mrouted.
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE.mrouted".
 * 
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 */

#include <paths.h>
#include "defs.h"

#ifdef SNMP
#include "snmp.h"
#endif

#ifdef __ECOS
#include <cyg/kernel/kapi.h>           // Kernel API.

#define CYGNUM_NET_PIM6DD_THREADOPT_PRIORITY 16
#define CYGNUM_NET_PIM6DD_THREADOPT_STACKSIZE 12*1024
#define CYG_PIM6DD_DAEMON_STACK_SIZE (CYGNUM_HAL_STACK_SIZE_MINIMUM + \
                                          CYGNUM_NET_PIM6DD_THREADOPT_STACKSIZE)
static cyg_int32 pim6dd_initialized = 0;
cyg_thread   pim6dd_thread_object;
cyg_handle_t pim6dd_thread_handle;
cyg_uint8    pim6dd_thread_stack[CYG_PIM6DD_DAEMON_STACK_SIZE]    
                                       __attribute__((__aligned__ (16)));
#endif /* __ECOS */

char pim6dd_configfilename[256] = _PATH_PIM6D_CONF;
char pim6dd_versionstring[100];

static char pim6dd_pidfilename[]  = _PATH_PIM6D_PID;
/* TODO: not used
static char genidfilename[] = _PATH_PIM6D_GENID;
*/

int pim6dd_haveterminal = 1;
char *pim6dd_progname;

static int pim6dd_sighandled = 0;
#define GOT_SIGINT      0x01
#define GOT_SIGHUP      0x02
#define GOT_SIGUSR1     0x04
#define GOT_SIGUSR2     0x08
#define GOT_SIGALRM     0x10


#ifdef SNMP
#define NHANDLERS       34
#else
#define NHANDLERS       3
#endif

static struct ihandler {
    int fd;			/* File descriptor               */
    ihfunc_t func;		/* Function to call with &fd_set */
} pim6dd_ihandlers[NHANDLERS];
static int pim6dd_nhandlers = 0;

static struct debugname {
    char *name;
    int	 level;
    int	 nchars;
} pim6dd_debugnames[] = {
#if 0
    {   "dvmrp_detail",	    DEBUG_DVMRP_DETAIL,   5	    },
    {   "dvmrp_prunes",	    DEBUG_DVMRP_PRUNE,    8	    },
    {   "dvmrp_pruning",    DEBUG_DVMRP_PRUNE,    8	    },
    {   "dvmrp_mrt",        DEBUG_DVMRP_ROUTE,    7	    },
    {   "dvmrp_routes",	    DEBUG_DVMRP_ROUTE,    7	    },
    {   "dvmrp_routing",    DEBUG_DVMRP_ROUTE,    7	    },
    {   "dvmrp_neighbors",  DEBUG_DVMRP_PEER,     7	    },
    {   "dvmrp_peers",	    DEBUG_DVMRP_PEER,     8	    },
    {   "dvmrp_hello",      DEBUG_DVMRP_PEER,     7	    },
    {   "dvmrp_timers",	    DEBUG_DVMRP_TIMER,    7	    },
    {   "dvmrp",	    DEBUG_DVMRP,          1	    },
    {   "igmp_proto",	    DEBUG_IGMP_PROTO,     6	    },
    {   "igmp_timers",	    DEBUG_IGMP_TIMER,     6	    },
    {   "igmp_members",	    DEBUG_IGMP_MEMBER,    6	    },
    {   "groups",	    DEBUG_MEMBER,         1	    },
    {   "membership",       DEBUG_MEMBER,         2	    },
    {   "igmp",	            DEBUG_IGMP, 	  1	    },
#endif
    {   "trace",	    DEBUG_TRACE,          2	    },
    {   "mtrace",	    DEBUG_TRACE,          2	    },
    {   "traceroute",       DEBUG_TRACE,          2	    },
    {   "timeout",	    DEBUG_TIMEOUT,        2	    },
    {   "callout",	    DEBUG_TIMEOUT,        3	    },
    {   "pkt",	            DEBUG_PKT,  	  2	    },
    {   "packets",	    DEBUG_PKT,  	  2	    },
    {   "interfaces",       DEBUG_IF,   	  2	    },
    {   "vif",	            DEBUG_IF,   	  1	    },
    {   "kernel",           DEBUG_KERN,           2	    },
    {   "cache",            DEBUG_MFC,   	  1	    },
    {   "mfc",              DEBUG_MFC,  	  2	    },
    {   "k_cache",          DEBUG_MFC,  	  2	    },
    {   "k_mfc",            DEBUG_MFC,  	  2	    },
    {   "rsrr",	            DEBUG_RSRR, 	  2	    },
    {   "pim_detail",       DEBUG_PIM_DETAIL,     5	    },
    {   "pim_hello",        DEBUG_PIM_HELLO,      5	    },
    {   "pim_neighbors",    DEBUG_PIM_HELLO,      5	    },
    {   "pim_register",     DEBUG_PIM_REGISTER,   5	    },
    {   "registers",        DEBUG_PIM_REGISTER,   2	    },
    {   "pim_join_prune",   DEBUG_PIM_JOIN_PRUNE, 5	    },
    {   "pim_j_p",          DEBUG_PIM_JOIN_PRUNE, 5	    },
    {   "pim_jp",           DEBUG_PIM_JOIN_PRUNE, 5	    },
    {   "pim_graft",        DEBUG_PIM_GRAFT,      5         },
    {   "pim_bootstrap",    DEBUG_PIM_BOOTSTRAP,  5	    },
    {   "pim_bsr",          DEBUG_PIM_BOOTSTRAP,  5	    },
    {   "bsr",	            DEBUG_PIM_BOOTSTRAP,  1	    },
    {   "bootstrap",        DEBUG_PIM_BOOTSTRAP,  1	    },
    {   "pim_asserts",      DEBUG_PIM_ASSERT,     5	    },
    {   "pim_cand_rp",      DEBUG_PIM_CAND_RP,    5	    },
    {   "pim_c_rp",         DEBUG_PIM_CAND_RP,    5	    },
    {   "pim_rp",           DEBUG_PIM_CAND_RP,    6	    },
    {   "rp",	            DEBUG_PIM_CAND_RP,    2	    },
    {   "pim_routes",       DEBUG_PIM_MRT,        6	    },
    {   "pim_routing",      DEBUG_PIM_MRT,        6	    },
    {   "pim_mrt",          DEBUG_PIM_MRT,        5	    },
    {   "pim_timers",       DEBUG_PIM_TIMER,      5	    },
    {   "pim_rpf",          DEBUG_PIM_RPF,        6	    },
    {   "rpf",              DEBUG_RPF,            3	    },
    {   "pim",              DEBUG_PIM,  	  1	    },
    {   "routes",	    DEBUG_MRT,            1	    },
    {   "routing",	    DEBUG_MRT,            1	    },
    {   "mrt",  	    DEBUG_MRT,            1	    },
    {   "routers",          DEBUG_NEIGHBORS,      6	    },
    {   "mrouters",         DEBUG_NEIGHBORS,      7	    },
    {   "neighbors",        DEBUG_NEIGHBORS,      1	    },
    {   "timers",           DEBUG_TIMER,          1	    },
    {   "asserts",          DEBUG_ASSERT,         1	    },
    {   "all",              DEBUG_ALL,            2         },
    {   "3",	            0xffffffff,           1	    }    /* compat. */
};

#ifdef ECOS_DBG_STAT
int dbg_mldproxy_index = 0;
#endif

/*
 * Forward declarations.
 */
static void pim6dd_handler __P((int));
static void pim6dd_timer __P((void *));
static void pim6dd_cleanup __P((void));
static int pim6dd_restart __P((int));
static void pim6dd_resetlogging __P((void *));

#ifdef __ECOS
static void pim6dd_start(void);
static void pim6dd_stop(void);
static int pim6dd_daemon(cyg_addrword_t data);
extern void pim6dd_clean_mld();
extern int pim6dd_clean_pim6();
extern int pim6dd_clean_mrt();
extern int pim6dd_clean_vif();
#else
static int pim6dd_daemon();
#endif /* __ECOS */

/* To shut up gcc -Wstrict-prototypes */
int pim6dd_main __P((int argc, char **argv));

int
pim6dd_register_input_handler(fd, func)
    int fd;
    ihfunc_t func;
{
    if (pim6dd_nhandlers >= NHANDLERS)
	return -1;
    
    pim6dd_ihandlers[pim6dd_nhandlers].fd = fd;
    pim6dd_ihandlers[pim6dd_nhandlers++].func = func;
    
    return 0;
}

int
pim6dd_main(argc, argv)
    int argc;
    char *argv[];
{	
    struct debugname *d;
    char c;
    int tmpd;

#ifndef __ECOS      
    setlinebuf(stderr);
	
    if (geteuid() != 0) {
	fprintf(stderr, "pim6dd: must be root\n");
	exit(1);
    }
#endif /* !__ECOS */

#ifdef  ECOS_DBG_STAT
	dbg_mldproxy_index=dbg_stat_register("mldproxy");
#endif

    
    pim6dd_progname = strrchr(argv[0], '/');
    if (pim6dd_progname)
	pim6dd_progname++;
    else
	pim6dd_progname = argv[0];
    
    argv++;
    argc--;

#ifdef __ECOS      
	if (argc == 1) {
		if (!strcmp(argv[0],"stop")) {
			printf("%s(%d)stop pim6dd\n",__FUNCTION__,__LINE__);
			pim6dd_stop();
			return 0;
		}
	}
#endif

    while (argc > 0 && *argv[0] == '-') {
	if (strcmp(*argv, "-d") == 0) {
	    if (argc > 1 && *(argv + 1)[0] != '-') { 
		char *p,*q;
		int i, len;
		struct debugname *d;
		
		argv++;
		argc--;
		pim6dd_debug = 0;
		p = *argv; q = NULL;
		while (p) {
		    q = strchr(p, ',');
		    if (q)
			*q++ = '\0';
		    len = strlen(p);
		    for (i = 0, d = pim6dd_debugnames;
			 i < sizeof(pim6dd_debugnames) / sizeof(pim6dd_debugnames[0]);
			 i++, d++)
			if (len >= d->nchars && strncmp(d->name, p, len) == 0)
			    break;
		    if (i == sizeof(pim6dd_debugnames) / sizeof(pim6dd_debugnames[0])) {
			int j = 0xffffffff;
			int k = 0;
			fprintf(stderr, "Valid debug levels: ");
			for (i = 0, d = pim6dd_debugnames;
			     i < sizeof(pim6dd_debugnames) / sizeof(pim6dd_debugnames[0]);
			     i++, d++) {
			    if ((j & d->level) == d->level) {
				if (k++)
				    putc(',', stderr);
				fputs(d->name, stderr);
				j &= ~d->level;
			    }
			}
			putc('\n', stderr);
			goto usage;
		    }
		    pim6dd_debug |= d->level;
		    p = q;
		}
	    }
	    else
		pim6dd_debug = DEBUG_DEFAULT;
	}
	else if (strcmp(*argv, "-c") == 0) {
	    if (argc > 1) {
		argv++; argc--;
		strlcpy(pim6dd_configfilename, *argv, sizeof(pim6dd_configfilename));
	    }
	    else
		goto usage;
/* TODO: not implemented */
#ifdef SNMP
	}
	else if (strcmp(*argv, "-P") == 0) {
	    if (argc > 1 && isdigit(*(argv + 1)[0])) {
		argv++, argc--;
		dest_port = atoi(*argv);
	    }
	    else
		dest_port = DEFAULT_PORT;
#endif
	}
	else
	    goto usage;
	argv++; argc--;
    }

    if (argc > 0) {
		usage:
		tmpd = 0xffffffff;
		fprintf(stderr, "usage: pim6dd [-c configfile] [-d [debug_level][,debug_level]]\n");
		
		fprintf(stderr, "debug levels: ");
		c = '(';
		for (d = pim6dd_debugnames; d < pim6dd_debugnames +
			 sizeof(pim6dd_debugnames) / sizeof(pim6dd_debugnames[0]); d++) {
			if ((tmpd & d->level) == d->level) {
				tmpd &= ~d->level;
				fprintf(stderr, "%c%s", c, d->name);
				c = ',';
			}
		}
		fprintf(stderr, ")\n");
#ifdef __ECOS
		return -1;
#else
		exit(1);
#endif
    }	
    
    if (pim6dd_debug != 0) {
	tmpd = pim6dd_debug;
	fprintf(stderr, "debug level 0x%lx ", pim6dd_debug);
	c = '(';
	for (d = pim6dd_debugnames; d < pim6dd_debugnames +
		 sizeof(pim6dd_debugnames) / sizeof(pim6dd_debugnames[0]); d++) {
	    if ((tmpd & d->level) == d->level) {
		tmpd &= ~d->level;
		fprintf(stderr, "%c%s", c, d->name);
		c = ',';
	    }
	}
	fprintf(stderr, ")\n");
    }

	/* Start pim6dd deamon */
#ifdef __ECOS      
	pim6dd_start();
#else
	pim6dd_daemon();
#endif

	return 0;    
}

#ifdef __ECOS      
static void pim6dd_start(void)
{
    if (pim6dd_initialized)
        return;

    pim6dd_log_msg(LOG_NOTICE, 0, "%s start", pim6dd_versionstring);
    pim6dd_initialized = 1;
    
    cyg_thread_create(CYGNUM_NET_PIM6DD_THREADOPT_PRIORITY,
                      pim6dd_daemon,
                      (cyg_addrword_t)0,
                      "pim6dd Thread",
                      (void *)pim6dd_thread_stack,
                      CYG_PIM6DD_DAEMON_STACK_SIZE,
                      &pim6dd_thread_handle,
                      &pim6dd_thread_object);
    cyg_thread_resume(pim6dd_thread_handle);
}   
#endif /* __ECOS */

#ifdef __ECOS      
static void pim6dd_stop(void)
{
	if (pim6dd_initialized == 0) 
		return;

    cyg_thread_kill(pim6dd_thread_handle);
    cyg_thread_delete(pim6dd_thread_handle);

    pim6dd_log_msg(LOG_NOTICE, 0, "%s stop", pim6dd_versionstring);
	pim6dd_initialized = 0;

    /*
     * reset all the entries
     */

    pim6dd_stop_all_vifs();
    pim6dd_clean_mrt();
    
    pim6dd_cleanup();
    /*
     * TODO: delete?
     * free_all_routes();
     */

    pim6dd_free_all_callouts();
    //shirley pim6dd_stop_all_vifs(); should before pim6dd_cleanup()
	//pim6dd_stop_all_vifs();
    pim6dd_nhandlers=0;
//    pim6dd_k_stop_pim(pim6dd_mld6_socket);
    close(pim6dd_mld6_socket);
    close(pim6dd_pim6_socket);
    close(pim6dd_udp_socket);
#ifdef HAVE_ROUTING_SOCKETS
    close(pim6dd_routing_socket);
#endif
    #if 0
	free(pim6dd_mld6_recv_buf);
	pim6dd_mld6_recv_buf = NULL;
    #endif

    if(pim6dd_mld6_send_buf)
    {
	    free(pim6dd_mld6_send_buf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
	    pim6dd_mld6_send_buf = NULL;
    }

	pim6dd_clean_pim6();
	//shirley, pim6dd_clean_mrt()should before pim6dd_cleanup
	//pim6dd_clean_mrt();
	pim6dd_clean_vif();
    
    pim6dd_clean_mld();

}
#endif /* __ECOS */

#ifdef __ECOS      
static int pim6dd_daemon(cyg_addrword_t data)
#else
static int pim6dd_daemon(void)
#endif
{
    FILE *fp;
    int dummy, dummysigalrm;
    struct timeval tv, difftime, curtime, lasttime, *timeout;
    fd_set rfds, readers;
    int nfds, n, i, secs;
    extern char pim6dd_todaysversion[];
    struct sigaction sa;

#ifndef __ECOS /* Not support in ecos */
#ifdef LOG_DAEMON
    (void)openlog("pim6dd", LOG_PID, LOG_DAEMON);
    (void)setlogmask(LOG_UPTO(LOG_NOTICE));
#else
    (void)openlog_msg("pim6dd", LOG_PID);
#endif /* LOG_DAEMON */
#endif
    snprintf(pim6dd_versionstring, sizeof(pim6dd_versionstring),
	"pim6dd version %s", pim6dd_todaysversion);
    
    pim6dd_log_msg(LOG_DEBUG, 0, "%s starting", pim6dd_versionstring);
    
/* TODO: XXX: use a combination of time and hostid to initialize the random
 * generator.
 */

#ifdef SYSV
    srand48(time(NULL));
#elif defined(__ECOS)
    {
	    struct timeval tm;
	    gettimeofday(&tm, NULL);
	    srand(tm.tv_usec + getpid());
    }
#else
    {
	    struct timeval tm;
	    gettimeofday(&tm, NULL);
	    srandom(tm.tv_usec + gethostid());
    }
#endif
    
    pim6dd_callout_init();

    /* Start up the log rate-limiter */
    pim6dd_resetlogging(NULL);

    if (pim6dd_init_mld6()== -1) {
		printf("%s(%d)pim6dd_init_mld6 fail\n",__FUNCTION__,__LINE__);
    	pim6dd_initialized = 0;
		return -1;
	}

#if 0 
    pim6dd_k_stop_pim(pim6dd_mld6_socket);
    exit(0);			/* XXX */
#endif
    if (pim6dd_init_pim6() == -1) {
		printf("%s(%d)pim6dd_init_pim6 fail\n",__FUNCTION__,__LINE__);
    	pim6dd_initialized = 0;
		return -1;
	}

    pim6dd_init_pim6_mrt();
    pim6dd_init_timers();
    
    /* TODO: check the kernel DVMRP/MROUTED/PIM support version */
    
#ifdef SNMP
    if (i = snmp_init())
	return i;
#endif /* SNMP */
    pim6dd_init_vifs();
    
#ifdef RSRR
    pim6dd_rsrr_init();
#endif /* RSRR */

    sa.sa_handler = pim6dd_handler;
    sa.sa_flags = 0;	/* Interrupt system calls */
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    
    FD_ZERO(&readers);
    FD_SET(pim6dd_mld6_socket, &readers);
    nfds = pim6dd_mld6_socket + 1;
    for (i = 0; i < pim6dd_nhandlers; i++) {
	FD_SET(pim6dd_ihandlers[i].fd, &readers);
	if (pim6dd_ihandlers[i].fd >= nfds)
	    nfds = pim6dd_ihandlers[i].fd + 1;
    }
    
    IF_DEBUG(DEBUG_IF)
	pim6dd_dump_vifs(stderr);
    IF_DEBUG(DEBUG_PIM_MRT)
	pim6dd_dump_pim_mrt(stderr);
    
    /* schedule first pim6dd_timer interrupt */
    pim6dd_timer_setTimer(TIMER_INTERVAL, pim6dd_timer, NULL);
    
    if (pim6dd_debug == 0) {
	/* Detach from the terminal */
#ifdef TIOCNOTTY
      int t;
#endif /* TIOCNOTTY */
      
	pim6dd_haveterminal = 0;
#ifndef __ECOS //not support fork in ecos
	if (fork())
	    exit(0);
	(void)close(0);
	(void)close(1);
	(void)close(2);
	(void)open("/", 0);
	(void)dup2(0, 1);
	(void)dup2(0, 2);
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
	if (setsid() < 0)
	    perror("setsid");
#endif /* TIOCNOTTY */
#endif /* SYSV */
#endif /* !__ECOS */
    } /* End of child process code */

#ifdef HAVE_ROUTING_SOCKETS
    pim6dd_init_routesock();
#endif /* HAVE_ROUTING_SOCKETS */
    
    fp = fopen(pim6dd_pidfilename, "w");
    if (fp != NULL) {
	fprintf(fp, "%d\n", (int)getpid());
	(void) fclose(fp);
    }
    
    /*
     * Main receive loop.
     */
    dummy = 0;
    dummysigalrm = SIGALRM;
    difftime.tv_usec = 0;
    gettimeofday(&curtime, NULL);
    lasttime = curtime;
    for(;;) {
	bcopy((char *)&readers, (char *)&rfds, sizeof(rfds));
	secs = pim6dd_timer_nextTimer();
	if (secs == -1)
	    timeout = NULL;
	else {
	   timeout = &tv;
	   timeout->tv_sec = secs;
	   timeout->tv_usec = 0;
        }
	
	if (pim6dd_sighandled) {
	    if (pim6dd_sighandled & GOT_SIGINT) {
		pim6dd_sighandled &= ~GOT_SIGINT;
		break;
	    }
	    if (pim6dd_sighandled & GOT_SIGHUP) {
		pim6dd_sighandled &= ~GOT_SIGHUP;
		pim6dd_restart(SIGHUP);
	    }
	    if (pim6dd_sighandled & GOT_SIGUSR1) {
		pim6dd_sighandled &= ~GOT_SIGUSR1;
		pim6dd_fdump(SIGUSR1);
	    }
	    if (pim6dd_sighandled & GOT_SIGUSR2) {
		pim6dd_sighandled &= ~GOT_SIGUSR2;
		pim6dd_cdump(SIGUSR2);
	    }
	    if (pim6dd_sighandled & GOT_SIGALRM) {
		pim6dd_sighandled &= ~GOT_SIGALRM;
		pim6dd_timer(&dummysigalrm);
	    }
	}
	if ((n = select(nfds, &rfds, NULL, NULL, timeout)) < 0) {
	    if (errno != EINTR) /* SIGALRM is expected */
		pim6dd_log_msg(LOG_WARNING, errno, "select failed");
	    continue;
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
#ifdef TIMERDEBUG
	    IF_DEBUG(DEBUG_TIMEOUT)
		pim6dd_log_msg(LOG_DEBUG, 0, "TIMEOUT: secs %d, diff secs %d, diff usecs %d", secs, difftime.tv_sec, difftime.tv_usec );
#endif
	    while (difftime.tv_usec >= 1000000) {
		difftime.tv_sec++;
		difftime.tv_usec -= 1000000;
	    }
	    if (difftime.tv_usec < 0) {
		difftime.tv_sec--;
		difftime.tv_usec += 1000000;
	    }
	    lasttime = curtime;
	    if (secs == 0 || difftime.tv_sec > 0) {
#ifdef TIMERDEBUG
		IF_DEBUG(DEBUG_TIMEOUT)
		    pim6dd_log_msg(LOG_DEBUG, 0, "\taging callouts: secs %d, diff secs %d, diff usecs %d", secs, difftime.tv_sec, difftime.tv_usec );
#endif
		pim6dd_age_callout_queue(difftime.tv_sec);
	    }
	    secs = -1;
#ifdef __ECOS      
	} while ((difftime.tv_sec > 0) && (pim6dd_initialized > 0));
#else
	} while (difftime.tv_sec > 0);
#endif

	/* Handle sockets */
	if (n > 0) {
	    /* TODO: shall check first pim6dd_mld6_socket for better performance? */
	    for (i = 0; i < pim6dd_nhandlers; i++) {
		if (FD_ISSET(pim6dd_ihandlers[i].fd, &rfds)) {
		    (*pim6dd_ihandlers[i].func)(pim6dd_ihandlers[i].fd, &rfds);
		}
	    }
	}
    
#ifdef __ECOS
	if (pim6dd_initialized == 0)
		break;
#endif
    } /* Main loop */

    pim6dd_log_msg(LOG_NOTICE, 0, "%s exiting", pim6dd_versionstring);
    pim6dd_cleanup();
#ifdef __ECOS
    return;
#else
	exit(0);
#endif
}

/*
 * The 'virtual_time' variable is initialized to a value that will cause the
 * first invocation of pim6dd_timer() to send a probe or route report to all vifs
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

u_long pim6dd_pim6_virtual_time = 0;

/*
 * Timer routine. Performs all perodic functions:
 * aging interfaces, quering neighbors and members, etc... The granularity
 * is equal to TIMER_INTERVAL.
 */
static void 
pim6dd_timer(i)
    void *i;
{
    pim6dd_age_vifs();	        /* Timeout neighbors and groups         */
    pim6dd_age_routes();  	/* Timeout routing entries              */
    
    pim6dd_pim6_virtual_time += TIMER_INTERVAL;
    pim6dd_timer_setTimer(TIMER_INTERVAL, pim6dd_timer, NULL);
}	

/*
 * Performs all necessary functions to quit gracefully
 */
/* TODO: implement all necessary stuff */
static void
pim6dd_cleanup()
{

#ifdef RSRR
    pim6dd_rsrr_clean();
#endif /* RSRR */

    pim6dd_k_stop_pim(pim6dd_mld6_socket);

    /* TODO: XXX (not in the spec)
     */
}


/*
 * Signal handler.  Take note of the fact that the signal arrived
 * so that the main loop can take care of it.
 */
static void
pim6dd_handler(sig)
    int sig;
{
    switch (sig) {
    case SIGALRM:
	pim6dd_sighandled |= GOT_SIGALRM;
    case SIGINT:
    case SIGTERM:
	pim6dd_sighandled |= GOT_SIGINT;
	break;
	
    case SIGHUP:
	pim6dd_sighandled |= GOT_SIGHUP;
	break;
	
    case SIGUSR1:
	pim6dd_sighandled |= GOT_SIGUSR1;
	break;
	
    case SIGUSR2:
	pim6dd_sighandled |= GOT_SIGUSR2;
	break;
    }
}


/* TODO: not verified */
/* PIMDM TODO */
/*
 * Restart the daemon
 */
static int
pim6dd_restart(i)
    int i;
{
#ifdef SNMP
    int s;
#endif /* SNMP */
    
    pim6dd_log_msg(LOG_NOTICE, 0, "%s restart", pim6dd_versionstring);
    
    /*
     * reset all the entries
     */
    /*
     * TODO: delete?
     * free_all_routes();
     */
    pim6dd_free_all_callouts();
    pim6dd_stop_all_vifs();
    pim6dd_nhandlers=0;
    pim6dd_k_stop_pim(pim6dd_mld6_socket);
    close(pim6dd_mld6_socket);
    close(pim6dd_pim6_socket);
    close(pim6dd_udp_socket);
#ifdef HAVE_ROUTING_SOCKETS
    close(pim6dd_routing_socket);
#endif
    
    /*
     * start processing again
     */
    if (pim6dd_init_mld6()== -1) {
		printf("%s(%d)pim6dd_init_mld6 fail\n",__FUNCTION__,__LINE__);
		return -1;
	}

    if (pim6dd_init_pim6() == -1) {
		printf("%s(%d)pim6dd_init_pim6 fail\n",__FUNCTION__,__LINE__);
		return -1;
	}
#ifdef HAVE_ROUTING_SOCKETS
    pim6dd_init_routesock();
#endif /* HAVE_ROUTING_SOCKETS */
    pim6dd_init_pim6_mrt();
#ifdef SNMP
    if ( s = snmp_init())
#ifdef __ECOS
	return;
#else
	exit(s);
#endif /* __ECOS */
#endif /* SNMP */
    pim6dd_init_vifs();

#ifdef RSRR
    pim6dd_rsrr_init();
#endif /* RSRR */

    /* schedule timer interrupts */
    pim6dd_timer_setTimer(TIMER_INTERVAL, pim6dd_timer, NULL);
}


static void
pim6dd_resetlogging(arg)
    void *arg;
{
    int nxttime = 60;
    void *narg = NULL;
    
    if (arg == NULL && pim6dd_log_nmsgs > LOG_MAX_MSGS) {
	nxttime = LOG_SHUT_UP;
	narg = (void *)&pim6dd_log_nmsgs;	/* just need some valid void * */
	#ifndef __ECOS 
	syslog(LOG_WARNING, "logging too fast, shutting up for %d minutes",
	       LOG_SHUT_UP / 60);
	#else
	printf("logging too fast, shutting up for %d minutes\n",
	       LOG_SHUT_UP / 60);
	#endif /* !__ECOS */
    } else {
	pim6dd_log_nmsgs = 0;
    }
    
    pim6dd_timer_setTimer(nxttime, pim6dd_resetlogging, narg);
}

