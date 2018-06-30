/*
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/errno.h>
#include <net/netisr.h>
#include <net/if_types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <arpa/inet.h>

#include <net/ppp_defs.h>
#include <net/if_ppp.h>
#include <net/ppp-comp.h>

 
/* prototypes */
static void ppp_ccp(struct ifppp *sc, struct mbuf *m, int rcvd);
struct mbuf *ppp_dequeue(struct ifppp *ifppp, struct mbuf *m);

#ifdef MPPE
extern struct compressor ppp_mppe;
#endif

static struct compressor *ppp_compressors[8] = 
{
#if DO_BSD_COMPRESS && defined(PPP_BSDCOMP)
    &ppp_bsd_compress,
#endif
#if DO_DEFLATE && defined(PPP_DEFLATE)
    &ppp_deflate,
    &ppp_deflate_draft,
#endif
#ifdef MPPE
	&ppp_mppe,
#endif
    NULL
};

struct mbuf *
ppp_ccp_pktin(sc, m)
    struct ifppp *sc;
    struct mbuf *m;
{
    int proto;
    u_char *cp, adrs, ctrl;
    struct mbuf *dmp = NULL;
    int s;
	
    /* Pull up to take ppp header */
    if (m->m_len < PPP_HDRLEN && (m = m_pullup(m, PPP_HDRLEN)) == 0)
	return 0;
	
    cp = mtod(m, u_char *);
    adrs = cp[0];
    ctrl = cp[1];
    proto = (cp[2] << 8) + cp[3];
	
    /*
     * Decompress this packet if necessary, update the receiver's
     * dictionary, or take appropriate action on a CCP packet.
     */
    if (proto == PPP_COMP && 
	sc->sc_rc_state && 
	(sc->sc_flags & SC_DECOMP_RUN) &&
	!(sc->sc_flags & SC_DC_ERROR) &&
	!(sc->sc_flags & SC_DC_FERROR)) {
	int rv;
		
	/* decompress this packet */
	s = splimp();
	rv = (*sc->sc_rcomp->decompress)(sc->sc_rc_state, m, &dmp);
	splx(s);
		
	if (rv == DECOMP_OK) {
	    m_freem(m);
	    if (dmp == NULL) {
		/* no error, but no decompressed packet produced */
		return 0;
	    }
			
	    m = dmp;
	}
	else {
	    /*
	     * An error has occurred in decompression.
	     * Pass the compressed packet up to pppd, which may take
	     * CCP down or issue a Reset-Req.
	     */
	    m_freem(m);
	    return 0;
	}
    }
    else {
	if (sc->sc_rc_state && (sc->sc_flags & SC_DECOMP_RUN)) {
	    (*sc->sc_rcomp->incomp)(sc->sc_rc_state, m);
	}
		
	if (proto == PPP_CCP) {
	    ppp_ccp(sc, m, 1);
	}
    }
	
    return m;
}

/*
 * Handle a CCP packet.  `rcvd' is 1 if the packet was received,
 * 0 if it is about to be transmitted.
 */
static void
ppp_ccp(sc, m, rcvd)
    struct ifppp *sc;
    struct mbuf *m;
    int rcvd;
{
    u_char *dp, *ep;
    struct mbuf *mp;
    int slen, s;
	
    /*
     * Get a pointer to the data after the PPP header.
     */
    if (m->m_len <= PPP_HDRLEN) {
	mp = m->m_next;
	if (mp == NULL)
	    return;
		
	dp = (mp != NULL)? mtod(mp, u_char *): NULL;
    }
    else {
	mp = m;
	dp = mtod(mp, u_char *) + PPP_HDRLEN;
    }
	
    ep = mtod(mp, u_char *) + mp->m_len;
    if (dp + CCP_HDRLEN > ep)
	return;
	
    slen = CCP_LENGTH(dp);
    if (dp + slen > ep) {
	if (sc->sc_flags & SC_DEBUG)
	    diag_printf("if_ppp/ccp: not enough data in mbuf (%p+%x > %p+%x)\n",
			dp, slen, mtod(mp, u_char *), mp->m_len);
	return;
    }
	
    switch (CCP_CODE(dp)) {
    case CCP_CONFREQ:
    case CCP_TERMREQ:
    case CCP_TERMACK:
	/* CCP must be going down - disable compression */
	if (sc->sc_flags & SC_CCP_UP) {
	    s = splimp();
	    sc->sc_flags &= ~(SC_CCP_UP | SC_COMP_RUN | SC_DECOMP_RUN);
	    splx(s);
	}
	break;
		
    case CCP_CONFACK:
	if (sc->sc_flags & SC_CCP_OPEN && !(sc->sc_flags & SC_CCP_UP)
	    && slen >= CCP_HDRLEN + CCP_OPT_MINLEN
	    && slen >= CCP_OPT_LENGTH(dp + CCP_HDRLEN) + CCP_HDRLEN) {
	    if (!rcvd) {
		/* we're agreeing to send compressed packets. */
		if (sc->sc_xc_state != NULL && 
		    (*sc->sc_xcomp->comp_init)(sc->sc_xc_state,
						dp + CCP_HDRLEN, slen - CCP_HDRLEN,
						sc->sc_unit, 0,
						sc->sc_flags & SC_DEBUG)) {
		    s = splimp();
		    sc->sc_flags |= SC_COMP_RUN;
		    splx(s);
		}
	    }
	    else {
		/* peer is agreeing to send compressed packets. */
		if (sc->sc_rc_state != NULL &&
		    (*sc->sc_rcomp->decomp_init)(sc->sc_rc_state,
						dp + CCP_HDRLEN, slen - CCP_HDRLEN,
						sc->sc_unit, 0, sc->sc_mru, 
						sc->sc_flags & SC_DEBUG)) {
		    s = splimp();
		    sc->sc_flags |= SC_DECOMP_RUN;
		    sc->sc_flags &= ~(SC_DC_ERROR | SC_DC_FERROR);
		    splx(s);
		}
	    }
	}
	break;
		
    case CCP_RESETACK:
	if (sc->sc_flags & SC_CCP_UP) {
	    if (!rcvd) {
		if (sc->sc_xc_state && (sc->sc_flags & SC_COMP_RUN))
		    (*sc->sc_xcomp->comp_reset)(sc->sc_xc_state);
	    }
	    else {
		if (sc->sc_rc_state && (sc->sc_flags & SC_DECOMP_RUN)) {
		    (*sc->sc_rcomp->decomp_reset)(sc->sc_rc_state);
		    s = splimp();
		    sc->sc_flags &= ~SC_DC_ERROR;
		    splx(s);
		}
	    }
	}
	break;
    }
}

/*
 * CCP is down; free (de)compressor state if necessary.
 */
void
ppp_ccp_closed(struct ifppp *sc)
{
    if (sc->sc_xc_state) {
	(*sc->sc_xcomp->comp_free)(sc->sc_xc_state);
	sc->sc_xc_state = NULL;
    }
    if (sc->sc_rc_state) {
	(*sc->sc_rcomp->decomp_free)(sc->sc_rc_state);
	sc->sc_rc_state = NULL;
    }
}

struct mbuf *ppp_dequeue(struct ifppp *sc, struct mbuf *m)
{
    u_char *cp;
    int address, control, protocol;
    struct mbuf *mp;
    int s;
    
    cp = mtod(m, u_char *);
    address = PPP_ADDRESS(cp);
    control = PPP_CONTROL(cp);
    protocol = PPP_PROTOCOL(cp);
	
    switch (protocol) {
    case PPP_LCP:
    case PPP_IPCP:
	return m;

    case PPP_CCP:
	ppp_ccp(sc, m, 0);
	return m;

    default:
	break;
    }
	
    /* Check xcomp status */
    if (sc->sc_xc_state && (sc->sc_flags & SC_COMP_RUN)) {
	struct mbuf *mcomp = NULL;
	int slen, clen;
		
	slen = 0;
	s = splimp();
	for (mp = m; mp != NULL; mp = mp->m_next)
	    slen += mp->m_len;

	clen = (*sc->sc_xcomp->compress)(sc->sc_xc_state, &mcomp, m, slen, sc->sc_if.if_mtu + PPP_HDRLEN);
	splx(s);
		
	if (mcomp != NULL) {
	    if (sc->sc_flags & SC_CCP_UP) {
		/* Send the compressed packet instead of the original. */
		m_freem(m);
		m = mcomp;
		cp = mtod(m, u_char *);
		protocol = cp[3];
	    }
	    else {
		/* Can't transmit compressed packets until CCP is up. */
		m_freem(mcomp);
	    }
	}
    }

    return m;
}

/*
 * Process an ioctl request to interface.
 */
#ifndef	EINVAL
#define EINVAL -1
#endif
int ppp_ccp_ioctl(ifp, cmd, data)
    struct ifnet *ifp;
    u_long cmd;
    caddr_t data;
{
    struct ifppp *sc;
    struct ppp_option_data *odp;
    u_int nb;
    struct compressor **cp;
    u_char ccp_option[CCP_MAX_OPTION_LENGTH];
    int error = 0;
    int s = splimp();

    sc = (struct ifppp *)ifp;
    switch (cmd) {
    //case PPPIOCSDECOMPRESS:
	//ppp_ccp_closed(sc);
	//break;

    case PPPIOCSCOMPRESS:
	odp = (struct ppp_option_data *)data;
	nb = odp->length;
	if (nb > sizeof(ccp_option))
	    nb = sizeof(ccp_option);
			
	if ((error = copyin(odp->ptr, ccp_option, nb)) != 0) {
	    splx(s);
	    return (error);
	}    
	if (ccp_option[1] < 2) {   
	    splx(s);
	    return (EINVAL);
	}    

	for (cp = ppp_compressors; *cp != NULL; ++cp) {
	    if ((*cp)->compress_proto == (int)(ccp_option[0])) {
		/*
		 * Found a handler for the protocol - try to allocate
		 * a compressor or decompressor.
		 */
		error = 0;
		if (odp->transmit) {
		    if (sc->sc_xc_state != NULL)
			(*sc->sc_xcomp->comp_free)(sc->sc_xc_state);
					
		    sc->sc_xcomp = *cp;
		    sc->sc_xc_state = (*cp)->comp_alloc(ccp_option, nb);
		    if (sc->sc_xc_state == NULL) {
			error = ENOBUFS;
		    }
		    sc->sc_flags &= ~SC_COMP_RUN;
		}
		else {
		    if (sc->sc_rc_state != NULL)
			(*sc->sc_rcomp->decomp_free)(sc->sc_rc_state);
					
		    sc->sc_rcomp = *cp;
		    sc->sc_rc_state = (*cp)->decomp_alloc(ccp_option, nb);
		    if (sc->sc_rc_state == NULL) {
			error = ENOBUFS;
		    }
		    sc->sc_flags &= ~SC_DECOMP_RUN;
		}
				
		splx(s);

		/* Set return code */
		return (error);
	    }
	}
		
	if (sc->sc_flags & SC_DEBUG)
	    diag_printf("ppp: no compressor for [%x %x %x], %x\n",
			ccp_option[0], ccp_option[1], ccp_option[2], nb);

	/* no handler found */
	splx(s);
	return (EINVAL);
	
    default:
	error = EINVAL;
    }

    splx(s);
    return (error);
}
