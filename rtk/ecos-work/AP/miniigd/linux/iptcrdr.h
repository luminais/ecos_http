/* $Id: iptcrdr.h,v 1.1.1.1 2007-08-06 10:04:43 root Exp $ */
/* miniupnp project
 * (c) Thomas Bernard
 * http://miniupnp.free.fr/
 * This software is subject to the conditions detailed in the
 * LICENCE file provided within the distribution. */
#ifndef __IPTCRDR_H__
#define __IPTCRDR_H__

int
add_redirect_rule2(const char * ifname, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
				   const char * desc);

int
add_filter_rule2(const char * ifname, const char * iaddr,
                 unsigned short eport, int proto, const char * desc);

int
get_redirect_rule(const char * ifname, unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc);

int
get_redirect_rule_by_index(int index,
                           char * ifname, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc);

/*int
delete_redirect_rule(const char * ifname, unsigned short eport, int proto);

int
delete_filter_rule(const char * ifname, unsigned short eport, int proto);
*/
int
delete_redirect_and_filter_rules(unsigned short eport, int proto);

/* for debug */
int list_redirect_rule(const char * ifname);

int addnatrule(int proto, unsigned short eport,
               const char * iaddr, unsigned short iport);

int add_filter_rule(int proto, const char * iaddr, unsigned short iport);

#endif

