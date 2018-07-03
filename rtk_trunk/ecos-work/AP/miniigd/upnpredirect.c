/* $Id: upnpredirect.c,v 1.3 2009/07/02 01:26:29 bert Exp $ */
/* miniupnp project : http://miniupnp.free.fr/
 * 
 * (c) 2006 Thomas Bernard
 * this software is subject to the conditions detailed in the
 * LICENCE file provided in this distribution
 */
 #ifdef __ECOS
#include <network.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef __ECOS
#include <syslog.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include "upnpredirect.h"
#include "upnpglobalvars.h"
#include "upnpevents.h"

#ifndef __ECOS
#if defined(__linux__)
#include "linux/iptcrdr.h"
#else
#include "openbsd/obsdrdr.h"
#endif
#else
#include "ipfw/ipfwrdr.h"
#endif
#ifdef 	ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif
#ifdef	ECOS_DBG_STAT
extern int	dbg_igd_index;
#endif

static int proto_atoi(const char * protocol)
{
	int proto = IPPROTO_TCP;
	if(strcmp(protocol, "UDP") == 0)
		proto = IPPROTO_UDP;
	return proto;
}

int
upnp_redirect(unsigned short eport, 
              const char * iaddr, unsigned short iport,
              const char * protocol, const char * desc,int enabled,unsigned short leaseduration)
{
	int proto, r;
	char iaddr_old[32];
	unsigned short iport_old;
	unsigned int timestamp;
	char pattern[30],tmp_prt;	
	char command[80];
	proto = proto_atoi(protocol);
	r = get_redirect_rule(ext_if_name, eport, proto,
	                      iaddr_old, sizeof(iaddr_old), &iport_old, 0);
		
	if(r==0 && enabled==0)
	  {
          r = upnp_delete_redirection(eport, protocol);
	  
	  return  0;	  
	  }	  
	if(r==0 && iport!=iport_old)
	{
          r = upnp_delete_redirection(eport, protocol);
	  r=1;
	}
	if(proto==6)
		tmp_prt=1;
	else
		tmp_prt=2;
		
	if(strcmp(iaddr,"255.255.255.255")==0)
	{
	sprintf(pattern,"%d %s %d",tmp_prt,ext_ip_addr,eport);
        sprintf(command,"echo %s > /proc/filter_upnp_br",pattern);	 
	system(command);
	}
		
	if(r==0)
	{
			
//		syslog(LOG_INFO, "port %hu protocol %s allready redirected to %s:%hu",
//	    	             eport, protocol, iaddr_old, iport_old);
		
		if(iport_old==iport && strcmp(iaddr_old,iaddr)!=0)
			return -2;
		return 0;
	}
	else
	{
	//	syslog(LOG_INFO, "redirecting port %hu to %s:%hu protocol %s for: %s",
	  //  	             eport, iaddr, iport, protocol, desc);
	  timestamp = (leaseduration > 0) ? time(NULL) + leaseduration : 3600+time(NULL);
	  
		add_redirect_rule2(ext_if_name, eport, iaddr, iport, proto, desc,timestamp);
//		syslog(LOG_INFO, "create pass rule to %s:%hu protocol %s for: %s",
//	    	             iaddr, iport, protocol, desc);
		//add_filter_rule2(ext_if_name, iaddr, eport, proto, desc);
		//add_filter_rule2(ext_if_name, iaddr, iport, proto, desc); //brad bug fix
	}

	return 0;
}

#ifdef ENABLE_NATPMP
upnp_redirect_internal(unsigned short eport,
                       const char * iaddr, unsigned short iport,
                       int proto, const char * desc,unsigned short timestamp)
{
	/*syslog(LOG_INFO, "redirecting port %hu to %s:%hu protocol %s for: %s",
		eport, iaddr, iport, protocol, desc);			*/
	if(add_redirect_rule2(ext_if_name, eport, iaddr, iport, proto, desc,timestamp) < 0)
	{
		return -1;
	}

#ifdef ENABLE_LEASEFILE
	lease_file_add( eport, iaddr, iport, proto, desc);
#endif
/*	syslog(LOG_INFO, "creating pass rule to %s:%hu protocol %s for: %s",
		iaddr, iport, protocol, desc);*/
	//if(add_filter_rule2(ext_if_name, iaddr, eport, iport, proto, desc) < 0)
#if 0
	if(add_filter_rule2(ext_if_name, iaddr, eport, proto, desc) < 0)
	{
		/* clean up the redirect rule */
#if !defined(__linux__)
		delete_redirect_rule(ext_if_name, eport, proto);
#endif
		return -1;
	}
#endif
#ifdef ENABLE_EVENTS
	upnp_event_var_change_notify(EWanIPC);
#endif
	return 0;
}
#endif


int
upnp_get_redirection_infos(unsigned short eport, const char * protocol,
                           unsigned short * iport,
                           char * iaddr, int iaddrlen,
                           const char * * desc)
{
	int proto, r;
	static char desc_buf[64];
	desc_buf[0] = '\0';
	proto = proto_atoi(protocol);
	r = get_redirect_rule(ext_if_name, eport, proto,
	                      iaddr, iaddrlen, iport, desc_buf);
	*desc = desc_buf;/*"miniupnpd";*/
	return r;
}

int
upnp_get_redirection_infos_by_index(int index,
                                    unsigned short * eport, char * protocol,
                                    unsigned short * iport, 
                                    char * iaddr, int iaddrlen,
                                    const char * * desc)
{
	char ifname[IFNAMSIZ];
	int proto = 0;
	static char desc_buf[64];
	desc_buf[0] = '\0';
	if(get_redirect_rule_by_index(index, ifname, eport, iaddr, iaddrlen,
	                              iport, &proto, desc_buf) < 0)
		return -1;
	else
	{
		if(proto == IPPROTO_TCP)
			memcpy(protocol, "TCP", 4);
		else
			memcpy(protocol, "UDP", 4);
		*desc = desc_buf;/*"miniupnpd";*/
		return 0;
	}
}

#ifdef ENABLE_NATPMP
_upnp_delete_redir(unsigned short eport, int proto)
{
	int r;
#if defined(__linux__)
	r = delete_redirect_and_filter_rules(eport, proto);
#else
	r = delete_redirect_rule(ext_if_name, eport, proto);
	//delete_filter_rule(ext_if_name, eport, proto);
#endif
#ifdef ENABLE_LEASEFILE
	lease_file_remove( eport, proto);
#endif

#ifdef ENABLE_EVENTS
	upnp_event_var_change_notify(EWanIPC);
#endif
	return r;
}
#endif

int
upnp_delete_redirection(unsigned short eport, const char * protocol)
{
	int proto, r;
	proto = proto_atoi(protocol);
//	syslog(LOG_INFO, "removing redirect rule port %hu %s", eport, protocol);
#if defined(__linux__)
	r = delete_redirect_and_filter_rules(eport, proto);
#else
	r = delete_redirect_rule(ext_if_name, eport, proto);
	//delete_filter_rule(ext_if_name, eport, proto);
#endif
	return r;
}

