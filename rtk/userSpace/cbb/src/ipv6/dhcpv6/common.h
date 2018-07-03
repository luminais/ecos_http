/*	$KAME: common.h,v 1.42 2005/09/16 11:30:13 suz Exp $	*/
/*
 * Copyright (C) 1998 and 1999 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef __KAME__
#define IN6_IFF_INVALID (IN6_IFF_ANYCAST|IN6_IFF_TENTATIVE|\
		IN6_IFF_DUPLICATED|IN6_IFF_DETACHED)
#else
#define IN6_IFF_INVALID (0)
#endif

#ifdef HAVE_ANSI_FUNC
#define FNAME __func__
#elif defined (HAVE_GCC_FUNCTION)
#define FNAME __FUNCTION__
#else
#define FNAME ""
#endif

/* XXX: bsdi4 does not have TAILQ_EMPTY */
#ifndef TAILQ_EMPTY
#define	TAILQ_EMPTY(head) ((head)->tqh_first == NULL)
#endif

/* and linux *_FIRST and *_NEXT */
#ifndef LIST_EMPTY
#define	LIST_EMPTY(head)	((head)->lh_first == NULL)
#endif
#ifndef LIST_FIRST
#define	LIST_FIRST(head)	((head)->lh_first)
#endif
#ifndef LIST_NEXT
#define	LIST_NEXT(elm, field)	((elm)->field.le_next)
#endif
#ifndef LIST_FOREACH
#define	LIST_FOREACH(var, head, field)					\
	for ((var) = LIST_FIRST((head));				\
	    (var);							\
	    (var) = LIST_NEXT((var), field))
#endif
#ifndef TAILQ_FIRST
#define	TAILQ_FIRST(head)	((head)->tqh_first)
#endif
#ifndef TAILQ_LAST
#define	TAILQ_LAST(head, headname)					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))
#endif
#ifndef TAILQ_PREV
#define	TAILQ_PREV(elm, headname, field)				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#endif
#ifndef TAILQ_NEXT
#define	TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)
#endif
#ifndef TAILQ_FOREACH
#define	TAILQ_FOREACH(var, head, field)					\
	for ((var) = TAILQ_FIRST((head));				\
	    (var);							\
	    (var) = TAILQ_NEXT((var), field))
#endif
#ifdef HAVE_TAILQ_FOREACH_REVERSE_OLD
#undef TAILQ_FOREACH_REVERSE
#endif
#ifndef TAILQ_FOREACH_REVERSE
#define	TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	for ((var) = TAILQ_LAST((head), headname);			\
	    (var);							\
	    (var) = TAILQ_PREV((var), headname, field))
#endif


#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

/* s*_len stuff */
static __inline u_int8_t
sysdep_sa_len (const struct sockaddr *sa)
{
#ifndef HAVE_SA_LEN
  switch (sa->sa_family)
    {
    case AF_INET:
      return sizeof (struct sockaddr_in);
    case AF_INET6:
      return sizeof (struct sockaddr_in6);
    }
  return sizeof (struct sockaddr_in);
#else
  return sa->sa_len;
#endif
}

extern int foreground;
extern int debug_thresh;
#ifdef __ECOS
extern char device[];
#else
extern char *device;
#endif

/* search option for dhcp6_find_listval() */
#define MATCHLIST_PREFIXLEN 0x1

/* common.c */
typedef enum { IFADDRCONF_ADD, IFADDRCONF_REMOVE } ifaddrconf_cmd_t;
extern int dhcp6_copy_list6s __P((struct dhcp6_list *, struct dhcp6_list *));
extern void dhcp6_move_lists __P((struct dhcp6_list *, struct dhcp6_list *));
extern void dhcp6_clear_lists __P((struct dhcp6_list *));
extern void dhcp6_clear_listvals __P((struct dhcp6_listval *));
extern struct dhcp6_listval *dhcp6_find_listvals __P((struct dhcp6_list *,
    dhcp6_listval_type_t, void *, int));
extern struct dhcp6_listval *dhcp6_add_listval6s __P((struct dhcp6_list *,
    dhcp6_listval_type_t, void *, struct dhcp6_list *));
extern int dhcp6_vbuf_copy6s __P((struct dhcp6_vbuf *, struct dhcp6_vbuf *));
extern void dhcp6_vbuf_free_6s __P((struct dhcp6_vbuf *));
extern int dhcp6_vbuf_cmps __P((struct dhcp6_vbuf *, struct dhcp6_vbuf *));
extern struct dhcp6_event *dhcp6_create_event6s __P((struct dhcp6_if *, int));
extern void dhcp6_remove_event6s __P((struct dhcp6_event *));
extern void dhcp6_remove_evdatas __P((struct dhcp6_event *));
extern struct authparam *new_authparam_dhcp6s __P((int, int, int));
extern struct authparam *copy_authparams __P((struct authparam *));
extern int dhcp6_auth_replaychecks __P((int, u_int64_t, u_int64_t));
extern int getifaddr6s __P((struct in6_addr *, char *, struct in6_addr *,
			  int, int, int));
extern int getifidfromaddrs __P((struct in6_addr *, unsigned int *));
extern int transmit_sa6s __P((int, struct sockaddr *, char *, size_t));
extern long random_betweens __P((long, long));
extern int prefix6_mask6s __P((struct in6_addr *, int));
extern int sa6_plen2masks __P((struct sockaddr_in6 *, int));
extern char *addr2strs __P((struct sockaddr *));
extern char *in6addr2strs __P((struct in6_addr *, int));
extern int in6_addrscopebyif6s __P((struct in6_addr *, char *));
extern int in6_scopes __P((struct in6_addr *));
extern void setloglevels __P((int));
extern void dprintfs __P((int, const char *, const char *, ...));
extern int get_duid_6s __P((char *, struct duid *, char *));
extern void dhcp6_init_options6s __P((struct dhcp6_optinfo *));
extern void dhcp6_clear_options6 __P((struct dhcp6_optinfo *));
extern int dhcp6_copy_options6s __P((struct dhcp6_optinfo *,
				   struct dhcp6_optinfo *));
extern int dhcp6_get_options6s __P((struct dhcp6opt *, struct dhcp6opt *,
				  struct dhcp6_optinfo *));
extern int dhcp6_set_options6 __P((int, struct dhcp6opt *, struct dhcp6opt *,
				  struct dhcp6_optinfo *));
extern void dhcp6_set_timeoparam6s __P((struct dhcp6_event *));
extern void dhcp6_reset_timer6 __P((struct dhcp6_event *));
extern char *dhcp6optstr6s __P((int));
extern char *dhcp6msgstrs __P((int));
extern char *dhcp6_stcodestrs __P((u_int16_t));
extern char *duidstrs __P((struct duid *));
extern char *dhcp6_event_statestr_6s __P((struct dhcp6_event *));
extern int get_rdvalues __P((int, void *, size_t));
extern int duidcpys __P((struct duid *, struct duid *));
extern int duidcmp6 __P((struct duid *, struct duid *));
extern void duidfree6s __P((struct duid *));
extern int ifaddrconf6s __P((int, char *, char *,int, int, int));
extern int safefiles __P((const char *));

/* missing */
#ifndef HAVE_STRLCAT
extern size_t strlcat __P((char *, const char *, size_t));
#endif
#ifndef HAVE_STRLCPY
extern size_t strlcpy __P((char *, const char *, size_t));
#endif
