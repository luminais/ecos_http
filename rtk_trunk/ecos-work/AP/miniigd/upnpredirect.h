/* $Id: upnpredirect.h,v 1.2 2009/07/02 01:26:29 bert Exp $ */
/* (c) 2006 Thomas Bernard */
#ifndef __UPNPREDIRECT_H__
#define __UPNPREDIRECT_H__

/* upnp_redirect() calls OS/fw dependant implementation
 * of the redirection.
 * protocol should be the string "TCP" or "UDP"
 * returns 0 on success.
 */
int
upnp_redirect(unsigned short eport, 
              const char * iaddr, unsigned short iport,
              const char * protocol, const char * desc,int enabled,unsigned short leaseduration);
#ifdef ENABLE_NATPMP
/* upnp_redirect_internal()
 * same as upnp_redirect() without any check */
int
upnp_redirect_internal(unsigned short eport,
                       const char * iaddr, unsigned short iport,
                       int proto, const char * desc,unsigned short timestamp);
#endif

int
upnp_get_redirection_infos(unsigned short eport, const char * protocol,
                           unsigned short * iport, char * iaddr, int iaddrlen,
                           const char * * desc);

int
upnp_get_redirection_infos_by_index(int index,
                                    unsigned short * eport, char * protocol,
                                    unsigned short * iport, 
                                    char * iaddr, int iaddrlen,
                                    const char * * desc);

int
upnp_delete_redirection(unsigned short eport, const char * protocol);

/* _upnp_delete_redir()
 * same as above */
int
_upnp_delete_redir(unsigned short eport, int proto);


#endif


