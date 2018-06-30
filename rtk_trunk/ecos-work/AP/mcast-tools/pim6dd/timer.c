/*	$KAME: timer.c,v 1.9 2004/06/15 07:43:53 itojun Exp $	*/

/*
 *  Copyright (c) 1998 by the University of Oregon.
 *  All rights reserved.
 *
 *  Permission to use, copy, modify, and distribute this software and
 *  its documentation in source and binary forms for lawful
 *  purposes and without fee is hereby granted, provided
 *  that the above copyright notice appear in all copies and that both
 *  the copyright notice and this permission notice appear in supporting
 *  documentation, and that any documentation, advertising materials,
 *  and other materials related to such distribution and use acknowledge
 *  that the software was developed by the University of Oregon.
 *  The name of the University of Oregon may not be used to endorse or 
 *  promote products derived from this software without specific prior 
 *  written permission.
 *
 *  THE UNIVERSITY OF OREGON DOES NOT MAKE ANY REPRESENTATIONS
 *  ABOUT THE SUITABILITY OF THIS SOFTWARE FOR ANY PURPOSE.  THIS SOFTWARE IS
 *  PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND 
 *  NON-INFRINGEMENT.
 *
 *  IN NO EVENT SHALL UO, OR ANY OTHER CONTRIBUTOR BE LIABLE FOR ANY
 *  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, WHETHER IN CONTRACT,
 *  TORT, OR OTHER FORM OF ACTION, ARISING OUT OF OR IN CONNECTION WITH,
 *  THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *  Other copyrights might apply to parts of this software and are so
 *  noted when applicable.
 */
/*
 *  Questions concerning this software should be directed to 
 *  Kurt Windisch (kurtw@antc.uoregon.edu)
 */
/*
 * Part of this program has been derived from PIM sparse-mode pimd.
 * The pimd program is covered by the license in the accompanying file
 * named "LICENSE.pimd".
 *  
 * The pimd program is COPYRIGHT 1998 by University of Southern California.
 *
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
 * Global variables
 */

/*
 * Local functions definitions.
 */


/*
 * Local variables
 */
static u_int16 pim6dd_unicast_routing_timer;    /* Used to check periodically for any
				   * change in the unicast routing.
				   */
static u_int16 pim6dd_unicast_routing_check_interval;
static u_int8  pim6dd_ucast_flag;               /* Used to indicate there was a timeout */


/* to request and compare any route changes */
static srcentry_t pim6dd_srcentry_save;

/*
 * Init some timers
 */
void
pim6dd_init_timers()
{
    pim6dd_unicast_routing_check_interval = UCAST_ROUTING_CHECK_INTERVAL;
    pim6dd_unicast_routing_timer          = pim6dd_unicast_routing_check_interval;

    /* Initialize the srcentry used to save the old routes
     * during unicast routing change discovery process.
     */
    pim6dd_srcentry_save.prev       = (srcentry_t *)NULL;
    pim6dd_srcentry_save.next       = (srcentry_t *)NULL;
    memset(&pim6dd_srcentry_save.address, 0, sizeof(struct sockaddr_in6));
    pim6dd_srcentry_save.address.sin6_len   = sizeof(struct sockaddr_in6);
    pim6dd_srcentry_save.address.sin6_family= AF_INET6;
    pim6dd_srcentry_save.mrtlink    = (mrtentry_t *)NULL;
    pim6dd_srcentry_save.incoming   = NO_VIF;
    pim6dd_srcentry_save.upstream   = (pim_nbr_entry_t *)NULL;
    pim6dd_srcentry_save.metric     = ~0;
    pim6dd_srcentry_save.preference = ~0;
    pim6dd_srcentry_save.timer      = 0;

}

/*
 * On every timer interrupt, advance (i.e. decrease) the timer for each
 * neighbor and group entry for each vif.
 */
void
pim6dd_age_vifs()
{
    vifi_t vifi;
    register struct uvif *v;
    register pim_nbr_entry_t *next_nbr, *curr_nbr;

/* XXX: TODO: currently, sending to qe* interface which is DOWN
 * doesn't return error (ENETDOWN) on my Solaris machine,
 * so have to check periodically the 
 * interfaces status. If this is fixed, just remove the defs around
 * the "if (pim6dd_vifs_down)" line.
 */

#if (!((defined SunOS) && (SunOS >= 50)))
    if (pim6dd_vifs_down)
#endif /* Solaris */
	pim6dd_check_vif_state();

    /* Age many things */
    for (vifi = 0, v = pim6dd_uvifs; vifi < pim6dd_numvifs; ++vifi, ++v) {
	if (v->uv_flags & (VIFF_DISABLED | VIFF_DOWN))
	    continue;

	/* Timeout the MLD querier (unless we re the querier) */
	if ((v->uv_flags & VIFF_QUERIER) == 0 &&
	    v->uv_querier) { /* this must be non-NULL, but check for safety. */
	    IF_TIMEOUT(v->uv_querier->al_timer) {
		/* act as a querier by myself */
		v->uv_flags |= VIFF_QUERIER;
		v->uv_querier->al_addr = v->uv_linklocal->pa_addr;
		v->uv_querier->al_timer = MLD6_OTHER_QUERIER_PRESENT_INTERVAL;
		time(&v->uv_querier->al_ctime); /* reset timestamp */
		pim6dd_query_groups(v);
	    }
	}

	/* Timeout neighbors */
	for (curr_nbr = v->uv_pim_neighbors; curr_nbr != NULL;
	     curr_nbr = next_nbr) {
	    next_nbr = curr_nbr->next;
	    /*
	     * Never timeout neighbors with holdtime = 0xffff.
	     * This may be used with ISDN lines to avoid keeping the
	     * link up with periodic Hello messages.
	     */
	    if (PIM_MESSAGE_HELLO_HOLDTIME_FOREVER == curr_nbr->timer)
		continue;
	    IF_NOT_TIMEOUT(curr_nbr->timer)
		continue;

	    pim6dd_delete_pim6_nbr(curr_nbr);
	}
	
	/* PIM_HELLO periodic */
	IF_TIMEOUT(v->uv_pim_hello_timer)
	    pim6dd_send_pim6_hello(v, PIM_TIMER_HELLO_HOLDTIME);

	/* MLD query periodic */
	IF_TIMEOUT(v->uv_gq_timer)
	    pim6dd_query_groups(v);
    }

    IF_DEBUG(DEBUG_IF) {
	pim6dd_dump_vifs(stderr);
	pim6dd_dump_lcl_grp(stderr);
    }
}


/*
 * Scan the whole routing table and timeout a bunch of timers:
 *  - prune timers
 *  - Join/Prune delay timer
 *  - routing entry
 *  - Assert timer
 */
void
pim6dd_age_routes()
{
    mrtentry_t *mrtentry_ptr, *mrtentry_next;
    grpentry_t *grpentry_ptr, *grpentry_next;
    vifi_t  vifi;
    int change_flag, state_change;
    int update_src_iif;
    u_long curr_bytecnt;

    /*
     * Timing out of the global `pim6dd_unicast_routing_timer` and data rate timer
     */
    IF_TIMEOUT(pim6dd_unicast_routing_timer) {
	pim6dd_ucast_flag = TRUE;
	pim6dd_unicast_routing_timer = pim6dd_unicast_routing_check_interval;
    }
    else {
	pim6dd_ucast_flag = FALSE;
    }

    /* Walk the (S,G) entries */
    if(pim6dd_grplist == (grpentry_t *)NULL) 
	return;
    for(grpentry_ptr = pim6dd_grplist;
	grpentry_ptr != (grpentry_t *)NULL; 
	grpentry_ptr = grpentry_next) {
	grpentry_next = grpentry_ptr->next;

	for(mrtentry_ptr = grpentry_ptr->mrtlink; 
	    mrtentry_ptr != (mrtentry_t *)NULL; 
	    mrtentry_ptr = mrtentry_next) {
	    mrtentry_next = mrtentry_ptr->grpnext;

	    /* Refresh entry timer if data forwarded */
	    curr_bytecnt = mrtentry_ptr->sg_count.bytecnt;
	    if (pim6dd_k_get_sg_cnt(pim6dd_udp_socket,
			     &mrtentry_ptr->source->address,
			     &mrtentry_ptr->group->group,
			     &mrtentry_ptr->sg_count)) {
		/* No such routing entry in kernel */
		pim6dd_delete_mrtentry(mrtentry_ptr);
		continue;
	    }
	    if(!(IF_ISEMPTY(&mrtentry_ptr->oifs)) && 
	       curr_bytecnt != mrtentry_ptr->sg_count.bytecnt) {
		/* Packets have been forwarded - refresh timer
		 * Note that these counters count packets received, 
		 * not packets forwarded.  So only refresh if packets
		 * received and non-null oiflist.
		 */
		IF_DEBUG(DEBUG_MFC)
		    pim6dd_log_msg(LOG_DEBUG, 0, 
			"Refreshing src %s, dst %s after %ld bytes forwarded",
			pim6dd_inet6_fmt(&mrtentry_ptr->source->address.sin6_addr),
			pim6dd_inet6_fmt(&mrtentry_ptr->group->group.sin6_addr),
			mrtentry_ptr->sg_count.bytecnt); 
		SET_TIMER(mrtentry_ptr->timer, PIM_DATA_TIMEOUT);
	    }	    

	    /* Time out the entry */
	    IF_TIMEOUT(mrtentry_ptr->timer) {
		pim6dd_delete_mrtentry(mrtentry_ptr);
		continue;
	    }

	    /* Time out asserts */
	    if(mrtentry_ptr->flags & MRTF_ASSERTED) 
		IF_TIMEOUT(mrtentry_ptr->assert_timer) {
		    mrtentry_ptr->flags &= ~MRTF_ASSERTED;
		    mrtentry_ptr->upstream = mrtentry_ptr->source->upstream;
		    mrtentry_ptr->metric   = mrtentry_ptr->source->metric;
		    mrtentry_ptr->preference = mrtentry_ptr->source->preference;
	        }

	    /* Time out Pruned interfaces */
	    change_flag = FALSE;
	    for (vifi = 0; vifi < pim6dd_numvifs; vifi++) {
		if (IF_ISSET(vifi, &mrtentry_ptr->pruned_oifs))
		    IF_TIMEOUT(mrtentry_ptr->prune_timers[vifi]) {
		        IF_CLR(vifi, &mrtentry_ptr->pruned_oifs);
			SET_TIMER(mrtentry_ptr->prune_timers[vifi], 0);
		        change_flag = TRUE;
		    }
	    }
	    
	    /* Unicast Route changes */
	    update_src_iif = FALSE;
	    if (pim6dd_ucast_flag == TRUE) {
		/* iif toward the source */
		pim6dd_srcentry_save.incoming = mrtentry_ptr->source->incoming;
		pim6dd_srcentry_save.upstream = mrtentry_ptr->source->upstream;
		pim6dd_srcentry_save.preference = mrtentry_ptr->source->preference;
		pim6dd_srcentry_save.metric = mrtentry_ptr->source->metric;
		
		if (pim6dd_set_incoming(mrtentry_ptr->source,
				 PIM_IIF_SOURCE) != TRUE) {
		    /*
		     * XXX: not in the spec!
		     * Cannot find route toward that source.
		     * This is bad. Delete the entry.
		     */
		    pim6dd_delete_mrtentry(mrtentry_ptr);
		    continue;
		}
		else {
		    /* iif info found */
		    if (!(mrtentry_ptr->flags & MRTF_ASSERTED) && 
			((pim6dd_srcentry_save.incoming !=
			  mrtentry_ptr->incoming)
			 || (pim6dd_srcentry_save.upstream !=
			     mrtentry_ptr->upstream))) {
			/* Route change has occur */
			update_src_iif = TRUE;
			mrtentry_ptr->incoming =
			    mrtentry_ptr->source->incoming;
			mrtentry_ptr->upstream =
			    mrtentry_ptr->source->upstream;
			/* mrtentry should have pref/metric of upstream
			 * assert winner, but we dont have that info,
			 * so use the source pref/metric, which will be
			 * larger and thus the correct assert winner
			 * from upstream will be chosen.
			 */
			mrtentry_ptr->preference = 
			    mrtentry_ptr->source->preference;
			mrtentry_ptr->metric = 
			    mrtentry_ptr->source->metric;
		    }
		}
	    }

	    if ((change_flag == TRUE) || (update_src_iif == TRUE)) {
		    /* Flush the changes */
		    state_change = 
			    pim6dd_change_interfaces(mrtentry_ptr,
					      mrtentry_ptr->incoming,
					      &mrtentry_ptr->pruned_oifs,
					      &mrtentry_ptr->leaves,
					      &mrtentry_ptr->asserted_oifs);
		    if(state_change == -1)
			    pim6dd_trigger_prune_alert(mrtentry_ptr);
	    }
	}
    }

    IF_DEBUG(DEBUG_PIM_MRT)
	pim6dd_dump_pim_mrt(stderr);
    return;
}
