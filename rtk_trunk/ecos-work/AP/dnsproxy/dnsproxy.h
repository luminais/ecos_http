/* $Id: dnsproxy.h,v 1.9 2010/01/11 15:02:00 armin Exp $ */
/*
 * Copyright (c) 2003,2004,2005,2010 Armin Wolfermann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _DNSPROXY_H_
#define _DNSPROXY_H_

/* LONGLONG */
#include <sys/types.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#if HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#include <stdarg.h>

//#include <event.h>

#ifdef DEBUG
#define DPRINTF(x) do { printf x ; } while (0)
#else
#define DPRINTF(x)
#endif

#ifdef GLOBALS
#define GLOBAL(a) a
#define GLOBAL_INIT(a,b) a = b
#else
#define GLOBAL(a) extern a
#define GLOBAL_INIT(a,b) extern a
#endif

#define CYGNUM_DNS_PROXY_THREADOPT_PRIORITY 14
//#define CYGNUM_DNS_PROXY_THREADOPT_STACKSIZE 0x00008000
#define CYGNUM_DNS_PROXY_THREADOPT_STACKSIZE 0x00002000


struct request {
	unsigned short		id;

	struct sockaddr_in	client;
	unsigned short		clientid;
	unsigned char		recursion;

	//struct event		timeout;
	int	timeout;
	struct request		**prev;
	struct request		*next;
};
#if 0
typedef void (timeout_fun)(void *);
typedef struct callout {
    struct callout *next, *prev;
    unsigned int     delta;  // Number of "ticks" in the future for this timeout
    timeout_fun  *fun;    // Function to execute when it expires
    void         *arg;    // Argument to pass when it does
    int           flags;  // Info about this item
} timeout_entry;

#define CALLOUT_LOCAL    0x0001
#define CALLOUT_ACTIVE   0x0002
#define CALLOUT_PENDING  0x0004
#endif
#define HASHSIZE 8
#define HASH(id) (id & ((1 << HASHSIZE) - 1))
extern struct request *request_hash[1 << HASHSIZE];

extern unsigned int authoritative_port;
extern unsigned int authoritative_timeout;
extern unsigned int recursive_port;
extern unsigned int recursive_timeout;
extern unsigned int stats_timeout;
extern unsigned int port;

extern char *authoritative;
extern char *chrootdir;
extern char *listenat;
extern char *recursive;
extern char *user;

extern unsigned long active_queries;
extern unsigned long all_queries;
extern unsigned long authoritative_queries;
extern unsigned long recursive_queries;
extern unsigned long removed_queries;
extern unsigned long dropped_queries;
extern unsigned long answered_queries;
extern unsigned long dropped_answers;
extern unsigned long late_answers;
extern unsigned long hash_collisions;

/* dnsproxy.c */
void signal_handler(int, short, void *);

/* daemon.c */
//int daemon(int, int);

/* hash.c */
void hash_add_request(struct request *);
void hash_remove_request(struct request *);
struct request *hash_find_request(unsigned short);

/* internal.c */
int add_internal(char *);
int is_internal(struct in_addr);

/* log.c */
void log_syslog(const char *);
void info(const char *, ...);
void error(const char *, ...);
void fatal(const char *, ...);

/* parse.c */
int parse(const char *);

struct dnssrv_t 
{
    int                    sock;      /* for communication with server */
    struct sockaddr_in     addr;      /* IP address of server */
    char*                  domain;    /* optional domain to match.  Set to
					 zero for a default server */
};

#endif /* _DNSPROXY_H_ */
