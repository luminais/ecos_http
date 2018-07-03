/* $Id: obsdrdr.h,v 1.1.1.1 2007-08-06 10:04:43 root Exp $ */
/* miniupnp project : http://miniupnp.free.fr/
 *
 * Copyright (c) 2006 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * licence file included within the distribution.
 */
#ifndef __OBSDRDR_H__
#define __OBSDRDR_H__

/* add_redirect_rule2() uses DIOCCHANGERULE ioctl
 * proto can take the values IPPROTO_UDP or IPPROTO_TCP
 */
int
add_redirect_rule2(const char * ifname, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
				   const char * desc);

/* add_filter_rule2() uses DIOCCHANGERULE ioctl
 * proto can take the values IPPROTO_UDP or IPPROTO_TCP
 */
int
add_filter_rule2(const char * ifname, const char * iaddr,
                 unsigned short eport, int proto, const char * desc);
 

/* get_redirect_rule() gets internal IP and port from
 * interface, external port and protocl
 */
int
get_redirect_rule(const char * ifname, unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc);

int
get_redirect_rule_by_index(int index,
                           char * ifname, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc);

/* delete_redirect_rule()
 */
int
delete_redirect_rule(const char * ifname, unsigned short eport, int proto);

/* delete_filter_rule()
 */
int
delete_filter_rule(const char * ifname, unsigned short eport, int proto);

int
clear_redirect_rules(void);

#endif


