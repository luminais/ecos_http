/*	$KAME: vif.c,v 1.10 2004/07/06 10:22:33 suz Exp $	*/

/*
 * Copyright (c) 1998-2001
 * The University of Southern California/Information Sciences Institute.
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
/*
 * Part of this program has been derived from mrouted.
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE.mrouted".
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 */

#include "defs.h"

/*
 * Exported variables.
 */
struct uvif	pim6dd_uvifs[MAXMIFS] = {0}; /* array of all virtual interfaces          */
vifi_t		pim6dd_numvifs;	/* Number of vifs in use                    */
int             pim6dd_vifs_down;      /* 1=>some interfaces are down              */
int             pim6dd_phys_vif;       /* An enabled vif                           */
int		pim6dd_udp_socket;	/* Since the honkin' kernel doesn't support */
				/* ioctls on raw IP sockets, we need a UDP  */
				/* socket as well as our IGMP (raw) socket. */
				/* How dumb.                                */
int             pim6dd_total_interfaces; /* Number of all interfaces: including the
				   * non-configured, but excluding the
				   * loopback interface and the non-multicast
				   * capable interfaces.
				   */
if_set		pim6dd_if_nullset;	/* an interface face that has all-0 bit
				 * (for comparison)
				 */

/*
 * Forward declarations.
 */
static void pim6dd_start_vif      __P((vifi_t vifi));
static void pim6dd_stop_vif       __P((vifi_t vifi));
static void pim6dd_start_all_vifs __P((void));


void
pim6dd_init_vifs()
{
    vifi_t vifi;
    struct uvif *v;
    int enabled_vifs;
	
    pim6dd_numvifs    = 0;
    pim6dd_vifs_down = FALSE;

    /*
     * Configure the vifs based on the interface configuration of
     * the kernel and the contents of the configuration file.
     * (Open a UDP socket for ioctl use in the config procedures if
     * the kernel can't handle IOCTL's on the MLD socket.)
     */
#ifdef IOCTL_OK_ON_RAW_SOCKET
    pim6dd_udp_socket = pim6dd_mld6_socket;
#else
    if ((pim6dd_udp_socket = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
	pim6dd_log_msg(LOG_ERR, errno, "UDP6 socket");
#endif

    /*
     * Clean up all vifs
     */
    for (vifi = 0, v = pim6dd_uvifs; vifi < MAXMIFS; ++vifi, ++v) {
	memset(v, 0, sizeof(*v));
	v->uv_flags		= 0;
	v->uv_metric		= DEFAULT_METRIC;
	v->uv_admetric		= 0;
	v->uv_rate_limit	= DEFAULT_PHY_RATE_LIMIT;
	strncpy(v->uv_name, "", IFNAMSIZ);
	v->uv_groups		= (struct listaddr *)NULL;
	v->uv_dvmrp_neighbors   = (struct listaddr *)NULL;
	NBRM_CLRALL(v->uv_nbrmap);
	v->uv_querier           = (struct listaddr *)NULL;
	v->uv_prune_lifetime    = 0;
	v->uv_acl               = (struct vif_acl *)NULL;
	v->uv_leaf_timer        = 0;
	v->uv_addrs		= (struct phaddr *)NULL;
	v->uv_filter		= (struct vif_filter *)NULL;
	v->uv_pim_hello_timer   = 0;
	v->uv_gq_timer          = 0;
	v->uv_pim_neighbors	= (struct pim_nbr_entry *)NULL;
	v->uv_local_pref        = pim6dd_default_source_preference;
	v->uv_local_metric      = pim6dd_default_source_metric;
    }

    pim6dd_log_msg(LOG_INFO, 0, "Getting ifs from kernel");
    pim6dd_config_vifs_from_kernel();
    pim6dd_log_msg(LOG_INFO, 0, "Getting ifs from %s", pim6dd_configfilename);
    pim6dd_config_vifs_from_file();

    /*
     * Quit if there are fewer than two enabled vifs or there is a vif
     * which has no link local address.
     */
    enabled_vifs    = 0;
    pim6dd_phys_vif        = -1;
	 
    for (vifi = 0, v = pim6dd_uvifs; vifi < pim6dd_numvifs; ++vifi, ++v) {
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN))
	    continue;
	if (v->uv_linklocal == NULL)
		pim6dd_log_msg(LOG_ERR, 0,
		    "there is no link-local address on vif#%d", vifi);
	if (pim6dd_phys_vif == -1) {
	    struct phaddr *p;

	    /*
	     * If this vif has a global address, set its id
	     * to pim6dd_phys_vif.
	     */
	    for(p = v->uv_addrs; p; p = p->pa_next) {
		if (!IN6_IS_ADDR_LINKLOCAL(&p->pa_addr.sin6_addr) &&
		    !IN6_IS_ADDR_SITELOCAL(&p->pa_addr.sin6_addr)) {
		    pim6dd_phys_vif = vifi;
		    break;
		}
	    }
	}
	enabled_vifs++;
    }

    if (enabled_vifs < 2)
	pim6dd_log_msg(LOG_ERR, 0, "can't forward: %s",
	    enabled_vifs == 0 ? "no enabled ifs" : "only one enabled if");

    memset(&pim6dd_if_nullset, 0, sizeof(pim6dd_if_nullset));
    pim6dd_k_init_pim(pim6dd_mld6_socket);	/* Call to kernel to initiliaze structures */

    pim6dd_start_all_vifs();
}


static void
pim6dd_start_all_vifs()
{
    vifi_t vifi;
    struct uvif *v;

    for (vifi = 0, v = pim6dd_uvifs; vifi < pim6dd_numvifs; ++vifi, ++v) {
	/* Start vif if not DISABLED or DOWN */
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN)) {
	    if (v->uv_flags & VIFF_DISABLED)
		pim6dd_log_msg(LOG_INFO, 0,
		    "%s is DISABLED; if #%u out of service", 
		    v->uv_name, vifi);
	    else
		pim6dd_log_msg(LOG_INFO, 0,
		    "%s is DOWN; if #%u out of service", 
		    v->uv_name, vifi);
	    }
	else
	    pim6dd_start_vif(vifi);
    }
}



/*
 * stop all vifs
 */
void
pim6dd_stop_all_vifs()
{
    vifi_t vifi;
    struct uvif *v;

    for (vifi = 0, v=pim6dd_uvifs; vifi < pim6dd_numvifs; ++vifi, ++v) {
	if (!(v->uv_flags &  VIFF_DOWN)) {
	    pim6dd_stop_vif(vifi);
	}
    }
}


/*
 * Initialize the vif and add to the kernel. The vif can be either
 * physical, tunnel (tunnels will be used in the future
 * when this code becomes PIM multicast boarder router.)
 */
static void 
pim6dd_start_vif(vifi)
    vifi_t vifi;
{
    struct uvif *v;
    u_long random_delay;

    v		    = &pim6dd_uvifs[vifi];
    /* Initialy no router on any vif */
    v->uv_flags = (v->uv_flags | VIFF_DR | VIFF_NONBRS) & ~VIFF_DOWN;
    v->uv_pim_hello_timer = 1 + RANDOM() % PIM_TIMER_HELLO_PERIOD;
    /* TODO: CHECK THE TIMERS!!!!! Set or reset? */
    v->uv_gq_timer  = 0;
    v->uv_pim_neighbors = (pim_nbr_entry_t *)NULL;
    
    /* Tell kernel to add, i.e. start this vif */
    pim6dd_k_add_vif(pim6dd_mld6_socket, vifi, &pim6dd_uvifs[vifi]);   
    pim6dd_log_msg(LOG_INFO, 0, "%s comes up; if #%u now in service", v->uv_name, vifi);
    
    /*
     * Join the PIM multicast group on the interface.
     */
    pim6dd_k_join(pim6dd_pim6_socket, &pim6dd_allpim6routers_group.sin6_addr, v->uv_ifindex);
	
    /*
     * Join the ALL-ROUTERS multicast group on the interface.
     * This allows mtrace requests to loop back if they are run
     * on the multicast router.
     */
    pim6dd_k_join(pim6dd_mld6_socket, &pim6dd_allrouters_group.sin6_addr, v->uv_ifindex);
	
    /*
     * Until neighbors are discovered, assume responsibility for sending
     * periodic group membership queries to the subnet.  Send the first
     * query.
     */
    #ifdef CONFIG_MLD_PROXY
    //down ifs act as querier, up ifs act as host
    if(strcmp(v->uv_name, down_if) == 0)
    {
    #endif
    v->uv_flags |= VIFF_QUERIER;
    if (!v->uv_querier) {
	v->uv_querier = (struct listaddr *)malloc(sizeof(*v->uv_querier));
	memset(v->uv_querier, 0, sizeof(*v->uv_querier));
    
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif

    }
    v->uv_querier->al_addr = v->uv_linklocal->pa_addr;
    v->uv_querier->al_timer = MLD6_OTHER_QUERIER_PRESENT_INTERVAL;
    time(&v->uv_querier->al_ctime); /* reset timestamp */
    pim6dd_query_groups(v);
    #ifdef CONFIG_MLD_PROXY
    }
    #endif

    /*
     * To avoid synchronization among routers booting simultaneously, set
     * the hello timer to a random value between 1 to PIM_TIMER_HELLO_PERIOD.
     */
    random_delay = 1 + (RANDOM() % (long)(PIM_TIMER_HELLO_PERIOD - 1));
    v->uv_pim_hello_timer = random_delay;
}


/*
 * Stop a vif (either physical interface or tunnel).
 * If we are running only PIM we don't have tunnels.
 */
static void 
pim6dd_stop_vif(vifi)
    vifi_t vifi;
{
    struct uvif *v;
    struct listaddr *a;
    register pim_nbr_entry_t *n, *next;
    struct vif_acl *acl;
    
    /*
     * TODO: make sure that the kernel viftable is 
     * consistent with the daemon table
     */	
    v = &pim6dd_uvifs[vifi];
    pim6dd_k_leave(pim6dd_pim6_socket, &pim6dd_allpim6routers_group.sin6_addr, v->uv_ifindex);
    pim6dd_k_leave(pim6dd_mld6_socket, &pim6dd_allrouters_group.sin6_addr, v->uv_ifindex);
    /*
     * Discard all group addresses.  (No need to tell kernel;
     * the pim6dd_k_del_vif() call will clean up kernel state.)
     */
    while (v->uv_groups != NULL) {
	a = v->uv_groups;
	v->uv_groups = a->al_next;
	free((char *)a);
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
    }
    
    /*
     * TODO: inform (eventually) the neighbors I am going down by sending
     * PIM_HELLO with holdtime=0 so someone else should become a DR.
     */ 

    /* TODO: dummy! Implement it!! Any problems if don't use it? */
    pim6dd_delete_vif_from_mrt(vifi);
    
    /*
     * Delete the interface from the kernel's vif structure.
     */
    pim6dd_k_del_vif(pim6dd_mld6_socket, vifi);
    v->uv_flags     = (v->uv_flags & ~VIFF_DR & ~VIFF_QUERIER & ~VIFF_NONBRS )
	              | VIFF_DOWN;
    v->uv_pim_hello_timer = 0;
    v->uv_gq_timer  = 0;
    for (n = v->uv_pim_neighbors; n != NULL; n = next) {
	next = n->next;	/* Free the space for each neighbour */
	free((char *)n);
    }
    v->uv_pim_neighbors = NULL;

    /* TODO: currently not used */
   /* The Access Control List (list with the scoped addresses) */
    while (v->uv_acl != NULL) {
	acl = v->uv_acl;
	v->uv_acl = acl->acl_next;
	free((char *)acl);
    }

    pim6dd_vifs_down = TRUE;
    pim6dd_log_msg(LOG_INFO, 0,
	"%s goes down; if #%u out of service", v->uv_name, vifi);
}		

/*
 * return the max global Ipv6 address of an UP and ENABLED interface
 * other than the MIFF_REGISTER interface.
*/
struct sockaddr_in6 *
pim6dd_max_global_address()
{
	vifi_t vifi;
	struct uvif *v;
	struct phaddr *p;
	struct phaddr *pmax = NULL;

	for(vifi=0,v=pim6dd_uvifs;vifi< pim6dd_numvifs;++vifi,++v)
	{
		if(v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | MIFF_REGISTER))
			continue;
		/*
		 * take first the max global address of the interface
		 * (without link local) => aliasing
		 */
		for(p=v->uv_addrs;p!=NULL;p=p->pa_next)
		{
			/*
			 * If this is the first global address, take it anyway.
			 */
			if (pmax == NULL) {
				if (!IN6_IS_ADDR_LINKLOCAL(&p->pa_addr.sin6_addr) &&
				    !IN6_IS_ADDR_SITELOCAL(&p->pa_addr.sin6_addr))
					pmax = p;
			}
			else {
				if (pim6dd_inet6_lessthan(&pmax->pa_addr,
						   &p->pa_addr) &&
				    !IN6_IS_ADDR_LINKLOCAL(&p->pa_addr.sin6_addr) &&
				    !IN6_IS_ADDR_SITELOCAL(&p->pa_addr.sin6_addr))
					pmax=p;	
			}
		}
	}

	return(pmax ? &pmax->pa_addr : NULL);
}

struct sockaddr_in6 *
pim6dd_uv_global(vifi)
	vifi_t vifi;
{
	struct uvif *v = &pim6dd_uvifs[vifi];
	struct phaddr *p;

	for (p = v->uv_addrs; p; p = p->pa_next) {
		if (!IN6_IS_ADDR_LINKLOCAL(&p->pa_addr.sin6_addr) &&
		    !IN6_IS_ADDR_SITELOCAL(&p->pa_addr.sin6_addr))
			return(&p->pa_addr);
	}

	return(NULL);
}

/*
 * See if any interfaces have changed from up state to down, or vice versa,
 * including any non-multicast-capable interfaces that are in use as local
 * tunnel end-points.  Ignore interfaces that have been administratively
 * disabled.
 */
void
pim6dd_check_vif_state()
{
    register vifi_t vifi;
    register struct uvif *v;
    struct ifreq ifr;
    static int checking_vifs = 0;

    /*
     * XXX: TODO: True only for DVMRP?? Check.
     * If we get an error while checking, (e.g. two interfaces go down
     * at once, and we decide to send a prune out one of the failed ones)
     * then don't go into an infinite loop!
     */
    if (checking_vifs)
	return;

    pim6dd_vifs_down = FALSE;
    checking_vifs = 1;
    /* TODO: Check all potential interfaces!!! */
    /* Check the physical interfaces only */
    for (vifi = 0, v = pim6dd_uvifs; vifi < pim6dd_numvifs; ++vifi, ++v) {
	if (v->uv_flags & VIFF_DISABLED)
	    continue;
	
	strncpy(ifr.ifr_name, v->uv_name, IFNAMSIZ);
	/* get the interface flags */
	if (ioctl(pim6dd_udp_socket, SIOCGIFFLAGS, (char *)&ifr) < 0)
	    pim6dd_log_msg(LOG_ERR, errno,
		"pim6dd_check_vif_state: ioctl SIOCGIFFLAGS for %s", ifr.ifr_name);

	if (v->uv_flags & VIFF_DOWN) {
	    if (ifr.ifr_flags & IFF_UP) {
		pim6dd_start_vif(vifi);
	    }
	    else pim6dd_vifs_down = TRUE;
	}
	else {
	    if (!(ifr.ifr_flags & IFF_UP)) {
		pim6dd_log_msg(LOG_NOTICE, 0,
		    "%s has gone down; if #%u taken out of service",
		    v->uv_name, vifi);
		pim6dd_stop_vif(vifi);
		pim6dd_vifs_down = TRUE;
	    }
	}
    }
    checking_vifs = 0;
}


/*
 * If the source is directly connected to us, find the vif number for
 * the corresponding physical interface (tunnels excluded).
 * Local addresses are excluded.
 * Return the vif number or NO_VIF if not found.
 */
vifi_t 
pim6dd_find_vif_direct(src)
    struct sockaddr_in6 *src;
{
    vifi_t vifi;
    register struct uvif *v;
    register struct phaddr *p;
	
    for (vifi = 0, v = pim6dd_uvifs; vifi < pim6dd_numvifs; ++vifi, ++v) {
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_TUNNEL))
	    continue;
	for (p = v->uv_addrs; p; p = p->pa_next) {
		if (pim6dd_inet6_equal(src, &p->pa_addr))
			return(NO_VIF);

		if (v->uv_flags & VIFF_POINT_TO_POINT)
			if (pim6dd_inet6_equal(src, &p->pa_rmt_addr))
				return(vifi);
		if (pim6dd_inet6_match_prefix(src, &p->pa_prefix, &p->pa_subnetmask))
			return(vifi);
	}
    }
    return (NO_VIF);
} 


/*
 * Checks if src is local address. If "yes" return the vif index,
 * otherwise return value is NO_VIF.
 */
vifi_t
pim6dd_local_address(src)
    struct sockaddr_in6 *src;
{
    vifi_t vifi;
    register struct uvif *v;
    register struct phaddr *p;
    
    for (vifi = 0, v = pim6dd_uvifs; vifi < pim6dd_numvifs; ++vifi, ++v) {
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN))
	    continue;
	for (p = v->uv_addrs; p; p = p->pa_next) {
		if (pim6dd_inet6_equal(src, &p->pa_addr))
			return(vifi);
	}
    }   
    /* Returning NO_VIF means not a local address */
    return (NO_VIF);	
}

/*
 * If the source is directly connected, or is local address,
 * find the vif number for the corresponding physical interface
 * (tunnels excluded).
 * Return the vif number or NO_VIF if not found.
 */
vifi_t 
pim6dd_find_vif_direct_local(src)
    struct sockaddr_in6 *src;
{
    vifi_t vifi;
    register struct uvif *v;
    register struct phaddr *p;
	
    for (vifi = 0, v = pim6dd_uvifs; vifi < pim6dd_numvifs; ++vifi, ++v) {
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN | VIFF_TUNNEL))
	    continue;
	for (p = v->uv_addrs; p; p = p->pa_next) {
		if (pim6dd_inet6_equal(src, &p->pa_addr) ||
		    pim6dd_inet6_match_prefix(src, &p->pa_prefix, &p->pa_subnetmask))
			return(vifi);

		if (v->uv_flags & VIFF_POINT_TO_POINT)
			if (pim6dd_inet6_equal(src, &p->pa_rmt_addr))
				return(vifi);
	}
    }
    return (NO_VIF);
} 

int pim6dd_clean_vif()
{
    vifi_t vifi;
	int i;
    register struct uvif *v;

    for (i = 0; i < MAXMIFS; i++) {
		v = &pim6dd_uvifs[i]; 

		if (v->uv_linklocal && v->uv_linklocal != v->uv_addrs) {
			free(v->uv_linklocal);
#ifdef ECOS_DBG_STAT
            dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
			v->uv_linklocal = (struct phaddr *)NULL;
		}

		if (v->uv_addrs) {
			free(v->uv_addrs);
#ifdef ECOS_DBG_STAT
            dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
			v->uv_addrs = (struct phaddr *)NULL;
		}
		if (v->uv_querier) {
			free(v->uv_querier);
#ifdef ECOS_DBG_STAT
            dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
			v->uv_querier = (struct listaddr *)NULL;
		}
	}
}

