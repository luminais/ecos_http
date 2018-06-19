//==========================================================================
//
//      src/ecos/timeout.c
//
//==========================================================================
// ####BSDCOPYRIGHTBEGIN####                                    
// -------------------------------------------                  
// This file is part of eCos, the Embedded Configurable Operating System.
//
// Portions of this software may have been derived from FreeBSD 
// or other sources, and if so are covered by the appropriate copyright
// and license included herein.                                 
//
// Portions created by the Free Software Foundation are         
// Copyright (C) 2002 Free Software Foundation, Inc.            
// -------------------------------------------                  
// ####BSDCOPYRIGHTEND####                                      
//==========================================================================

//==========================================================================
//
//        lib/timeout.c
//
//        timeout support
//
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     gthomas, hmt
// Contributors:  gthomas, hmt
// Date:          1999-02-05
// Description:   Simple timeout functions
//####DESCRIPTIONEND####

#include <sys/param.h>
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#include <cyg/infra/cyg_ass.h>
#include <net/netisr.h>

// Timeout support

void alarm_timeout_init(void);
#define NTIMEOUTS 32
static timeout_entry _timeouts[NTIMEOUTS];
static timeout_entry *timeouts = (timeout_entry *)NULL;
static timeout_entry *timeouts_freelist = (timeout_entry *)NULL;
static cyg_handle_t timeout_alarm_handle;
static cyg_alarm timeout_alarm;
static cyg_int32 last_delta;
static cyg_tick_count_t last_set_time;
static int in_do_timeout;

static void callout_dequeue(struct callout *c);

// ------------------------------------------------------------------------
// This routine exists so that this module can synchronize:

void
show_timeouts(void)
{
	timeout_entry *f;
	for (f = timeouts;  f;  f = f->next) {
		diag_printf("%p: delta: %d, fun: %p, param: %p\n",
			f, f->delta, f->fun, f->arg);
	}
}

static void
timeouts_init(void)
{
	int i;

	memset(_timeouts, 0, sizeof(_timeouts));
	for (i = 0; i < NTIMEOUTS-1; i++) {
		_timeouts[i].next = &_timeouts[i+1];
	}

	timeouts_freelist = _timeouts;
}
 
// ------------------------------------------------------------------------
// CALLBACK FUNCTION
// Called from the thread, this runs the alarm callbacks.
// Locking is already in place when this is called.
static void
do_timeout(void)
{
	cyg_int32 min_delta;
	timeout_entry *e, *e_next;

	CYG_ASSERT( 0 < last_delta, "last_delta underflow" );

	in_do_timeout = true;

	min_delta = last_delta; // local copy
	last_delta = -1; // flag recursive call underway

	e = timeouts;
	while (e) {
		e_next = e->next;  // Because this can change during processing
		if ((e->flags & CALLOUT_PENDING) && e->delta) {
			e->delta -= min_delta;
			if (e->delta <= 0) { // Defensive
				// Time for this item to 'fire'
				timeout_fun *fun = e->fun;
				void *arg = e->arg;

				e->flags &= ~CALLOUT_PENDING;

				/* Do timeout hook function */
				(*fun)(arg);
			}
		}
		e = e_next;
	}

	// Now scan for a new timeout *after* running all the callbacks
	// (because they can add timeouts themselves)
	min_delta = 0x7FFFFFFF;  // Maxint
	
	e = timeouts;
	while (e) {
		e_next = e->next;
		if ((e->flags & CALLOUT_PENDING) == 0) {
			/* Remove non-pending entries */
			callout_dequeue(e);
		}
		else {
			if (e->delta) {
				if (e->delta < min_delta)
					min_delta = e->delta;
			}
		}
		e = e_next;
	}

	CYG_ASSERT( 0 < min_delta, "min_delta underflow" );

	if (min_delta != 0x7FFFFFFF) {
		// Still something to do, schedule it
		last_set_time = cyg_current_time();
		cyg_alarm_initialize(timeout_alarm_handle, last_set_time+min_delta, 0);
		last_delta = min_delta;
	} else {
		last_delta = 0; // flag no activity
	}

	in_do_timeout = false;
}

// ------------------------------------------------------------------------
// ALARM EVENT FUNCTION
// This is the DSR for the alarm firing:
static void
do_alarm(cyg_handle_t alarm, cyg_addrword_t data)
{
	schednetisr(NETISR_TIMER);
}

// ------------------------------------------------------------------------
// INITIALIZATION FUNCTION
void
cyg_alarm_timeout_init( void )
{
	// Init the alarm object, attached to the real time clock
	cyg_handle_t h;
	cyg_clock_to_counter(cyg_real_time_clock(), &h);
	cyg_alarm_create(h, do_alarm, 0, &timeout_alarm_handle, &timeout_alarm);

	/* Timeout init */
	timeouts_init();
	register_netisr(NETISR_TIMER, do_timeout);
}

// ------------------------------------------------------------------------
// EXPORTED API: SET A TIMEOUT
// This can be called from anywhere, including recursively from the timeout
// functions themselves.
cyg_uint32
timeout(timeout_fun *fun, void *arg, cyg_int32 delta)
{
	timeout_entry *e;

	if (delta == 0)
		delta = 1;

	// this needs to be atomic - recursive calls from the alarm
	// handler thread itself are allowed:
	int spl = splinternal();

	/* Search for timeouts */
	e = timeouts;
	while (e) {
		if (((cyg_int32)e >= (cyg_int32)_timeouts) &&
		    ((cyg_int32)e < (cyg_int32)&_timeouts[NTIMEOUTS]) &&
		    (e->fun == fun) && (e->arg == arg)) {
			break;
		}
		e = e->next;
	}

	/* Try to get one from free list */
	if (e == NULL) {
		e = timeouts_freelist;
		if (e) {
			timeouts_freelist = timeouts_freelist->next;
			callout_init(e);
		}
	}

	/* Reset this callout */
	if (e)
		callout_reset(e, delta, fun, arg);

	splx(spl);

	return (cyg_uint32)e;
}

// ------------------------------------------------------------------------
// EXPORTED API: CANCEL A TIMEOUT
// This can be called from anywhere, including recursively from the timeout
// functions themselves.
void
untimeout(timeout_fun *fun, void * arg)
{
	timeout_entry *e;
	int spl = splinternal();

	e = timeouts;
	while (e) {
		if (((cyg_int32)e >= (cyg_int32)_timeouts) &&
		    ((cyg_int32)e < (cyg_int32)&_timeouts[NTIMEOUTS]) &&
		    (e->fun == fun) &&
		    (e->arg == arg)) {
			if (!in_do_timeout) {
				/* Remove from list */
				callout_dequeue(e);
			}
			else {
				e->flags &= ~CALLOUT_PENDING;
			}
			break;
		}
		e = e->next;
	}

	splx(spl);
}

static void
callout_dequeue(struct callout *c) 
{
	if (c->next) {
		c->next->prev = c->prev;
	}

	if (c->prev) {
		c->prev->next = c->next;
	}
	else {
		timeouts = c->next;
	}

	c->flags &= ~CALLOUT_PENDING;

	/* Return back to timeout_freelist if it belongs */
	if ((cyg_int32)c >= (cyg_int32)_timeouts &&
	    (cyg_int32)c < (cyg_int32)&_timeouts[NTIMEOUTS]) {
		/* Free back */
		c->next = timeouts_freelist;
		timeouts_freelist = c;
	}
	return;
}

void 
callout_init(struct callout *c) 
{
	bzero(c, sizeof(*c));
}

void 
callout_reset(struct callout *c, int delta, timeout_fun *f, void *p) 
{
	timeout_entry *e;
	int spl = splinternal();

	CYG_ASSERT( 0 < delta, "delta is right now, or even sooner!" );

	// Renormalize delta wrt the existing set alarm, if there is one
	if (last_delta > 0) {
		// There is an active alarm
		if (last_set_time != 0) {
			// Adjust the delta to be absolute, relative to the alarm
			delta += (cyg_int32)(cyg_current_time() - last_set_time);
		} else {
			// We don't know exactly when the alarm will fire, so just
			// schedule this event for the first time, or sometime after
			;  // Leaving the value alone won't be "too wrong"
		}
	}
	// So recorded_delta is set to either:
	// alarm is active:   delta + NOW - THEN
	// alarm is inactive: delta

	/* Make sure call out is in the queue */
	e = timeouts;
	while (e) {
		if (e == c)
			break;
		e = e->next;
	}

	/* Add to list */
	if (!e) {
		c->prev = (timeout_entry *)NULL;
		c->next = timeouts;
		if (c->next != (timeout_entry *)NULL) {
			c->next->prev = c;
		}
		timeouts = c;
	}

	c->flags = (CALLOUT_PENDING | CALLOUT_ACTIVE);
	c->fun = f;
	c->arg = p;
	c->delta = delta;

	// alarm was inactive  OR
	// alarm was active but later than we need
	if ((0 == last_delta ||
	    delta < last_delta) ) {

		// (if last_delta is -1, this call is recursive from the handler so
		//  also do nothing in that case)

		// Here, we know the new item added is sooner than that which was
		// most recently set, if any, so we can just go and set it up.
		if ( 0 == last_delta )
			last_set_time = cyg_current_time();

		// So we use, to set the alarm either:
		// alarm is active:   (delta + NOW - THEN) + THEN
		// alarm is inactive:  delta + NOW
		// and in either case it is true that
		//  (recorded_delta + last_set_time) == (delta + NOW)
		cyg_alarm_initialize(timeout_alarm_handle, last_set_time+delta, 0);
		last_delta = delta;
	}
	// Otherwise, the alarm is active, AND it is set to fire sooner than we
	// require, so when it does, that will sort out calling the item we
	// just added.

	splx(spl);
}

void 
callout_stop(struct callout *c) 
{
	struct callout *e;
	int spl = splinternal();

	/* Make sure it is in the timeouts */
	if (!in_do_timeout) {
		e = timeouts;
		while (e) {
			if (e == c) {
				callout_dequeue(e);
				break;
			}
			e = e->next;
		}
	}

	c->flags &= ~(CALLOUT_PENDING | CALLOUT_ACTIVE);
	splx(spl);
}

int  
callout_active(struct callout *c) 
{
	return ((c->flags & CALLOUT_ACTIVE) != 0);
}

void 
callout_deactivate(struct callout *c) 
{
	c->flags &= ~CALLOUT_ACTIVE;
}

int  
callout_pending(struct callout *c) 
{
	return ((c->flags & CALLOUT_PENDING) != 0);
}


// ------------------------------------------------------------------------

// EOF timeout.c
