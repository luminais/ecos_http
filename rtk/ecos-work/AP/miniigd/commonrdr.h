/* $Id: commonrdr.h,v 1.1 2009/08/13 05:19:44 bert Exp $ */
/* MiniUPnP project
 * (c) 2006-2007 Thomas Bernard
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#ifndef __COMMONRDR_H__
#define __COMMONRDR_H__

//#include "config.h"

/* init and shutdown functions */
int
init_redirect(void);

void
shutdown_redirect(void);

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

#endif

