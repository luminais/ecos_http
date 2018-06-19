/*
 * ip_gre.c, GRE protocol switch
 *
 * $ Copyright Broadcom Corporation 2010 $
 * 
 * $Id: ip_gre.c,v 1.1 2010-07-02 03:41:01 simonk Exp $
 */

#include <sys/param.h>
#ifndef __ECOS
#include <sys/systm.h>
#endif
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#ifndef __ECOS
#include <sys/proc.h>

#include <vm/vm.h>
#include <sys/sysctl.h>
#endif

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

struct mbuf *(*ifpptp_inputp) __P((struct mbuf *m, int hlen)) __attribute__((weak));

/*
 * Process a received GRE protocol.
 */
void
gre_input(struct mbuf *m, int hlen)
{
	if (ifpptp_inputp)
		m = ifpptp_inputp(m, hlen);

	if (m != NULL)
		rip_input(m, hlen);

	return;
}
