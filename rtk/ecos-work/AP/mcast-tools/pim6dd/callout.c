/*	$KAME: callout.c,v 1.3 2003/09/02 09:57:04 itojun Exp $	*/

/*
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE.mrouted". Use of the mrouted program represents acceptance
 * of the terms and conditions listed in that file.
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 *
 * callout.c,v 3.8.4.5 1997/05/16 20:18:25 fenner Exp
 */

#include "defs.h"

/* the code below implements a callout queue */
static int pim6dd_id = 0;
static struct timeout_q  *pim6dd_Q = 0; /* pointer to the beginning of timeout queue */

struct timeout_q {
    struct timeout_q *next;		/* next event */
    int        	     id;  
    cfunc_t          func;    	        /* function to call */
    void	     *data;		/* func's data */
    int              time;		/* time offset to next event*/
};

#ifdef CALLOUT_DEBUG
static void pim6dd_print_Q __P((void));
#else
#define	pim6dd_print_Q()	
#endif

void
pim6dd_callout_init()
{
    if (pim6dd_Q) {
	pim6dd_log_msg(LOG_ERR, 0, "timer used before pim6dd_callout_init()");
#ifdef __ECOS
	return;
#else
	exit(1);
#endif
    }
    pim6dd_Q = (struct timeout_q *) 0;
}

void
pim6dd_free_all_callouts()
{
    struct timeout_q *p;
    
    while (pim6dd_Q) {
	p = pim6dd_Q;
	pim6dd_Q = pim6dd_Q->next;
#ifdef HAVE_SYSTEM_REINIT
    if(p->data)
    {
        free(p->data);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
    }
#endif
	free(p);
    
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
    }
#ifdef HAVE_SYSTEM_REINIT
    pim6dd_id = 0;
#endif
}


/*
 * elapsed_time seconds have passed; perform all the events that should
 * happen.
 */
void
pim6dd_age_callout_queue(elapsed_time)
    int elapsed_time;
{
    struct timeout_q *ptr, *expQ;
    
#ifdef CALLOUT_DEBUG
    IF_DEBUG(DEBUG_TIMEOUT)
	pim6dd_log_msg(LOG_DEBUG, 0, "aging queue (elapsed time %d):", elapsed_time);
    pim6dd_print_Q();
#endif

    expQ = pim6dd_Q;
    ptr = NULL;
    
    while (pim6dd_Q) {
	if (pim6dd_Q->time > elapsed_time) {
	    pim6dd_Q->time -= elapsed_time;
	    if (ptr) {
		ptr->next = NULL;
		break;
	    }
	    return;
	} else {
	    elapsed_time -= pim6dd_Q->time;
	    ptr = pim6dd_Q;
	    pim6dd_Q = pim6dd_Q->next;
	}
    }
    
    /* handle queue of expired timers */
    while (expQ) {
	ptr = expQ;
	if (ptr->func)
	    ptr->func(ptr->data);
	expQ = expQ->next;
	free(ptr);

#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
    }
}

/*
 * Return in how many seconds pim6dd_age_callout_queue() would like to be called.
 * Return -1 if there are no events pending.
 */
int
pim6dd_timer_nextTimer()
{
    if (pim6dd_Q) {
	if (pim6dd_Q->time < 0) {
	    pim6dd_log_msg(LOG_WARNING, 0, "pim6dd_timer_nextTimer top of queue says %d", 
		pim6dd_Q->time);
	    return 0;
	}
	return pim6dd_Q->time;
    }
    return -1;
}

/* 
 * sets the timer
 */
int
pim6dd_timer_setTimer(delay, action, data)
    int 	delay;  	/* number of units for timeout */
    cfunc_t	action; 	/* function to be called on timeout */
    void  	*data;  	/* what to call the timeout function with */
{
    struct     timeout_q  *ptr, *node, *prev;
    
#ifdef CALLOUT_DEBUG
    IF_DEBUG(DEBUG_TIMEOUT)
	pim6dd_log_msg(LOG_DEBUG, 0, "setting timer:");
    pim6dd_print_Q();
#endif

    /* create a node */	
    node = (struct timeout_q *)malloc(sizeof(struct timeout_q));
    if (node == 0) {
	pim6dd_log_msg(LOG_WARNING, 0, "Malloc Failed in timer_settimer\n");
	return -1;
    }

    #ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
    #endif
    
    node->func = action; 
    node->data = data;
    node->time = delay; 
    node->next = 0;	
    node->id   = ++pim6dd_id;
    
    prev = ptr = pim6dd_Q;
    
    /* insert node in the queue */
    
    /* if the queue is empty, insert the node and return */
    if (!pim6dd_Q)
	pim6dd_Q = node;
    else {
	/* chase the pointer looking for the right place */
	while (ptr) {
	    
	    if (delay < ptr->time) {
		/* right place */
		
		node->next = ptr;
		if (ptr == pim6dd_Q)
		    pim6dd_Q = node;
		else
		    prev->next = node;
		ptr->time -= node->time;
		return node->id;
	    } else  {
		/* keep moving */
		
		delay -= ptr->time; node->time = delay;
		prev = ptr;
		ptr = ptr->next;
	    }
	}
	prev->next = node;
    }
    return node->id;
}

/* returns the time until the timer is scheduled */
int
pim6dd_timer_leftTimer(timer_id)
    int timer_id;
{
    struct timeout_q *ptr;
    int left = 0;
	
    if (!timer_id)
	return -1;
    
    for (ptr = pim6dd_Q; ptr; ptr = ptr->next) {
	left += ptr->time;
	if (ptr->id == timer_id)
	    return left;
    }
    return -1;
}

/* clears the associated timer */
void
pim6dd_timer_clearTimer(timer_id)
    int  timer_id;
{
    struct timeout_q  *ptr, *prev;
    
    if (!timer_id)
	return;
    
    prev = ptr = pim6dd_Q;
    
    /*
     * find the right node, delete it. the subsequent node's time
     * gets bumped up
     */
    
    while (ptr) {
	if (ptr->id == timer_id) {
	    /* got the right node */
	    
	    /* unlink it from the queue */
	    if (ptr == pim6dd_Q)
		pim6dd_Q = pim6dd_Q->next;
	    else
		prev->next = ptr->next;
	    
	    /* increment next node if any */
	    if (ptr->next != 0)
		(ptr->next)->time += ptr->time;
	    
	    if (ptr->data)
        {  
		    free(ptr->data);
        #ifdef ECOS_DBG_STAT
            dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
        #endif
        }
        
	    free(ptr);
        
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
	    return;
	}
	prev = ptr;
	ptr = ptr->next;
    }
}

#ifdef CALLOUT_DEBUG
/*
 * debugging utility
 */
static void
pim6dd_print_Q()
{
    struct timeout_q  *ptr;
    
    IF_DEBUG(DEBUG_TIMEOUT)
	for (ptr = pim6dd_Q; ptr; ptr = ptr->next)
	    pim6dd_log_msg(LOG_DEBUG, 0, "(%d,%d) ", ptr->id, ptr->time);
}
#endif /* CALLOUT_DEBUG */
