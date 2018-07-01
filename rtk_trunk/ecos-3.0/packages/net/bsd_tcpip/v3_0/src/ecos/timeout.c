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

#ifdef CONFIG_RTL_DRV_DELIVER_INET
#include <net/netisr.h>
#endif
// Timeout support
#define TIMEOUT_DEBUG_RUNTIME

void alarm_timeout_init(void);

#ifndef NTIMEOUTS
//#define NTIMEOUTS 8
#define NTIMEOUTS 128
#endif
static timeout_entry _timeouts[NTIMEOUTS];
static timeout_entry *timeouts = (timeout_entry *)NULL;
static cyg_handle_t timeout_alarm_handle;
static cyg_alarm timeout_alarm;
static cyg_int32 last_delta;
static cyg_tick_count_t last_set_time;

//#define STACK_SIZE CYGNUM_NET_FAST_THREAD_STACKSIZE
#ifdef CONFIG_RTL_DRV_DELIVER_INET
//add by z10312 解决定时器线程栈使用溢出引起线程挂死问题. realtek 定时处理跟bcm 不一样，
//它 所有定时器处理都是在同一线程里处理。16-0124
#define STACK_SIZE (1024*16) 
#else
#define STACK_SIZE (1024*16)
#endif
static char alarm_stack[STACK_SIZE];
static cyg_thread alarm_thread_data;
static cyg_handle_t alarm_thread_handle;

static cyg_flag_t alarm_flag;  
// ------------------------------------------------------------------------
// This routine exists so that this module can synchronize:
extern cyg_uint32 cyg_splinternal(void);

#if defined(TIMEOUT_DEBUG) || defined(TIMEOUT_DEBUG_RUNTIME)

static void show_stacktrace(void *stack_pointer)
{
	long stackdata;
	int i;
	unsigned long *sp = (unsigned long *)stack_pointer;

	diag_printf("Stack :");
	i = 0;
	while ((unsigned long) sp & (4096 - 1)) {
		if (i && ((i % (64 / 8)) == 0)) {
			diag_printf("\n");
			if (i <= 39)
				diag_printf("       ");
		}
		if (i > 39) {
			break;
		}

		stackdata = *sp;
		sp++;

		diag_printf(" %08lx", stackdata);
		i++;
	}
}

void dump_timeout_entry(timeout_entry *pentry)
{
	diag_printf("next:%p prev:%p delta:%d fun:%p arg:%p flags:%x\n",
		pentry->next,pentry->prev,pentry->delta,pentry->fun,pentry->arg,pentry->flags);
}

static void dump_timeouts()
{
	int i;
	for(i=0;i<NTIMEOUTS;i++)
	{
		dump_timeout_entry(&_timeouts[i]);
	}
}
static void
_show_timeouts(void)
{
    timeout_entry *f;
    dump_timeouts();
    for (f = timeouts;  f;  f = f->next) {
        diag_printf("%p: delta: %d, fun: %p, param: %p\n", f, f->delta, f->fun, f->arg);
    }
}

static void  dump_kernel_timer_entry(timeout_entry *ep)
{
	diag_printf("ep %p\n",ep);
	diag_printf("ep flags 0x%x\n",ep->flags);
	diag_printf("ep delta 0x%x\n",ep->delta);	
	diag_printf("ep next %p  prev %p\n",ep->next,ep->prev);
	diag_printf("ep args %p  func %p\n",ep->arg,ep->fun);
}
#endif // TIMEOUT_DEBUG

static timeout_entry *e_next_g;
// ------------------------------------------------------------------------
// CALLBACK FUNCTION
// Called from the thread, this runs the alarm callbacks.
// Locking is already in place when this is called.
static void
do_timeout(void)
{
    cyg_int32 min_delta;
    timeout_entry *e, *e_next;
    unsigned long flags;

    CYG_ASSERT( 0 < last_delta, "last_delta underflow" );

    min_delta = last_delta; // local copy
    last_delta = -1; // flag recursive call underway

    e = timeouts;

	timeout_fun *fun_tmp =NULL;
	
    while (e) {
        e_next_g = e->next;  // Because this can change during processing
        if (e->delta) {
#ifdef TIMEOUT_DEBUG
                if ( !(e->delta >= min_delta)) {
                   diag_printf("Bad delta in timeout: %p, delta: %d, min: %d, last: %ld\n", e, e->delta, min_delta, last_set_time);
                   _show_timeouts();
                }                
#endif
	      HAL_DISABLE_INTERRUPTS(flags);
// Note: this _can_ happen if timeouts are scheduled before the clock starts!
//            CYG_ASSERT( e->delta >= min_delta, "e->delta underflow" );
            e->delta -= min_delta;
            if (e->delta <= 0) { // Defensive
                // Time for this item to 'fire'
                timeout_fun *fun = e->fun;
				fun_tmp = fun;
                void *arg = e->arg;
                // Call it *after* cleansing the record
//                diag_printf("%s(%p, %p, %p)\n", __FUNCTION__, e, e->fun, e->arg);
                e->flags &= ~CALLOUT_PENDING;
                e->delta = 0;
                if (e->next) {
                    e->next->prev = e->prev;
                }
                if (e->prev) {
                    e->prev->next = e->next;
                } else {
                    timeouts = e->next;
                }
#ifdef TIMEOUT_DEBUG_RUNTIME				
		   if(!(((unsigned long)fun) & 0x80000000))
		   {
		   	diag_printf("fun:%p e:%p\n",fun,e);
		   	//_show_timeouts();
		   }
#endif	
		   HAL_RESTORE_INTERRUPTS(flags);
                (*fun)(arg);

            }else {
            		HAL_RESTORE_INTERRUPTS(flags);
            }
        }
        e = e_next_g;
	 /*HF. 
	   *     if e_next is deleted since DSR call timer in WIFI,
	   *     just go another round,
	   *     scheduler lock may be used but it will delay DSR.
	   */
	 HAL_DISABLE_INTERRUPTS(flags);   
	 if(e && (e->flags & CALLOUT_PENDING) == 0) {	 	
	 	diag_printf("%s %d timer list warning,e:%p,fun:%p\n",__FUNCTION__,__LINE__,e,e->fun);
		//_show_timeouts();
		HAL_RESTORE_INTERRUPTS(flags);
	 	break;
	 }	 
	 HAL_RESTORE_INTERRUPTS(flags);
    }

    // Now scan for a new timeout *after* running all the callbacks
    // (because they can add timeouts themselves)
    min_delta = 0x7FFFFFFF;  // Maxint
    for (e = timeouts;  e;  e = e->next )
        if (e->delta)
            if (e->delta < min_delta)
                min_delta = e->delta;

    CYG_ASSERT( 0 < min_delta, "min_delta underflow" );

    if (min_delta != 0x7FFFFFFF) {
        // Still something to do, schedule it
        last_set_time = cyg_current_time();
        cyg_alarm_initialize(timeout_alarm_handle, last_set_time+min_delta, 0);
        last_delta = min_delta;
    } else {
        last_delta = 0; // flag no activity
    }
#ifdef TIMEOUT_DEBUG
    diag_printf("Timeout list after %s\n", __FUNCTION__);
    _show_timeouts();
#endif
}

// ------------------------------------------------------------------------
// ALARM EVENT FUNCTION
// This is the DSR for the alarm firing:
static void
do_alarm(cyg_handle_t alarm, cyg_addrword_t data)
{
    cyg_flag_setbits( &alarm_flag, 1 ); 
}

#ifdef CYGPKG_HAL_MIPS_RLX
__attribute__((nomips16)) 
#endif
void ecos_synch_eth_drv_dsr(void)
{
    cyg_flag_setbits( &alarm_flag, 2 );
}

// ------------------------------------------------------------------------
// HANDLER THREAD ENTRY ROUTINE
// This waits on the DSR to tell it to run:
static void
alarm_thread(cyg_addrword_t param)
{
    // This is from the logical ethernet dev; it calls those delivery
    // functions who need attention.
    extern void eth_drv_run_deliveries( void );

    // This is from the logical ethernet dev; it tickles somehow
    // all ethernet devices in case one is wedged.
    extern void eth_drv_tickle_devices( void );

    while ( 1 ) {
        int spl;
        int x;
#ifdef CYGPKG_NET_FAST_THREAD_TICKLE_DEVS
        cyg_tick_count_t later = cyg_current_time();
        later += CYGNUM_NET_FAST_THREAD_TICKLE_DEVS_DELAY;
        x = cyg_flag_timed_wait(
            &alarm_flag,
            -1,
            CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR,
            later );
#else
        x = cyg_flag_wait(
            &alarm_flag,
            -1,
            CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR );

        CYG_ASSERT( 3 & x, "Lost my bits" );
#endif // CYGPKG_NET_FAST_THREAD_TICKLE_DEVS
        CYG_ASSERT( !((~3) & x), "Extra bits" );

        spl = cyg_splinternal();

        CYG_ASSERT( 0 == spl, "spl nonzero" );

        if ( 1 & x )
            do_timeout();

	if ( 2 & x ) {
#ifdef CONFIG_RTL_DRV_DELIVER_INET
		schednetisr(NETISR_RTL_DRV);
#else
		eth_drv_run_deliveries();
#endif
	}
		
#ifdef CYGPKG_NET_FAST_THREAD_TICKLE_DEVS
        // This is in the else clause for "do we deliver" because the
        // network stack might have continuous timing events anyway - so
        // the timeout would not occur, x would be 1 every time.
        else // Tickle the devices...
            eth_drv_tickle_devices();
#endif // CYGPKG_NET_FAST_THREAD_TICKLE_DEVS
        cyg_splx(spl);

#if 0 //def CONFIG_RTL_819X
#ifdef CYGPKG_IO_WATCHDOG
	/*HF. kick watch dog here if net deliver*/
	REG32(BSP_WDTCNR) |= (1 << 23); //WDTCLR
#endif
#endif

    }
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
    // Init the flag of waking up
    cyg_flag_init( &alarm_flag );
    // Create alarm background thread to run the callbacks
    cyg_thread_create(
        CYGPKG_NET_FAST_THREAD_PRIORITY - 1, // Priority
        alarm_thread,                   // entry
        0,                              // entry parameter
        "Network alarm support",        // Name
        &alarm_stack[0],                // Stack
        STACK_SIZE,                     // Size
        &alarm_thread_handle,           // Handle
        &alarm_thread_data              // Thread data structure
        );
    cyg_thread_resume(alarm_thread_handle);    // Start it
    
}

// ------------------------------------------------------------------------
// EXPORTED API: SET A TIMEOUT
// This can be called from anywhere, including recursively from the timeout
// functions themselves.
cyg_uint32
timeout(timeout_fun *fun, void *arg, cyg_int32 delta)
{
    int i;
    timeout_entry *e;
    cyg_uint32 stamp;

    // this needs to be atomic - recursive calls from the alarm
    // handler thread itself are allowed:
    int spl = cyg_splinternal();

    stamp = 0;  // Assume no slots available
    for (e = _timeouts, i = 0;  i < NTIMEOUTS;  i++, e++) {
        if ((e->flags & CALLOUT_PENDING) == 0) {
            // Free entry
            callout_init(e);
            e->flags = CALLOUT_LOCAL;
            callout_reset(e, delta, fun, arg);
            stamp = (cyg_uint32)e;
            break;
        }
    }
    cyg_splx(spl);
    return stamp;
}

// ------------------------------------------------------------------------
// EXPORTED API: CANCEL A TIMEOUT
// This can be called from anywhere, including recursively from the timeout
// functions themselves.
void
untimeout(timeout_fun *fun, void * arg)
{
    int i;
    timeout_entry *e;
    int spl = cyg_splinternal();

    for (e = _timeouts, i = 0; i < NTIMEOUTS; i++, e++) {
        if (e->delta && (e->fun == fun) && (e->arg == arg)) {
            callout_stop(e);
            break;
        }
    }
    cyg_splx(spl);
}

#ifdef CYGPKG_HAL_MIPS_RLX
__attribute__((nomips16)) 
#endif
void 
callout_init(struct callout *c) 
{
    bzero(c, sizeof(*c));
}

#ifdef CYGPKG_HAL_MIPS_RLX
__attribute__((nomips16)) 
#endif
void 
callout_reset(struct callout *c, int delta, timeout_fun *f, void *p) 
{
    int spl = cyg_splinternal();
    unsigned long flags;
    CYG_ASSERT( 0 < delta, "delta is right now, or even sooner!" );
#ifdef TIMEOUT_DEBUG_RUNTIME
    if(!((unsigned long)f & 0x80000000))
    {
    		diag_printf("%s %d c:%p f:%p\n",__FUNCTION__,__LINE__,c,f);
		//_show_timeouts();
		show_stacktrace((void *)&spl);
		do_reset(0);
    }
#endif
    // Renormalize delta wrt the existing set alarm, if there is one
    if (last_delta > 0) {
#ifdef TIMEOUT_DEBUG
        int _delta = delta;    
        int _time = cyg_current_time();
#endif // TIMEOUT_DEBUG
        // There is an active alarm
        if (last_set_time != 0) {
            // Adjust the delta to be absolute, relative to the alarm
            delta += (cyg_int32)(cyg_current_time() - last_set_time);
        } else {
            // We don't know exactly when the alarm will fire, so just
            // schedule this event for the first time, or sometime after
            ;  // Leaving the value alone won't be "too wrong"
        }
#ifdef TIMEOUT_DEBUG
        diag_printf("delta changed from %d to %d, now: %d, then: %d, last_delta: %d\n", 
                    _delta, delta, _time, (int)last_set_time, last_delta);
        _show_timeouts();
#endif
    }
    // So recorded_delta is set to either:
    // alarm is active:   delta + NOW - THEN
    // alarm is inactive: delta

    // Add this callout/timeout to the list of things to do
    if (c->flags & CALLOUT_PENDING) {
        callout_stop(c);
    }

    struct callout *c_temp;	
    c_temp=timeouts;
    while(c_temp)
    {
    	if(c==c_temp)
    	{
#if 0
    		dump_stack();
    		diag_printf("%s %d\n",__FUNCTION__,__LINE__);
			dump_timer_entry(c_temp);
			if(c_temp->prev)
				dump_timer_entry(c_temp->prev);
			if(c_temp->next)
				dump_timer_entry(c_temp->next);
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);
	    		c_temp=timeouts;
	    		while(c_temp) {
	    			dump_timer_entry(c_temp);
				c_temp=c_temp->next;
	    		}
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);

			/*recovry the system*/
			#ifdef CONFIG_RTL_819X
			#ifdef CYGPKG_IO_WATCHDOG
				REG32(BSP_WDTCNR) = (0); //WDREBOOT
				while(1);
			#endif
			#endif
#endif
			break;
    	}
		c_temp=c_temp->next;
    }

    HAL_DISABLE_INTERRUPTS(flags);

	if(!c_temp)
	{
	    c->prev = (timeout_entry *)NULL;    
	    c->next = timeouts;
	    if (c->next != (timeout_entry *)NULL) {
	        c->next->prev = c;
	    }
	    timeouts = c;
	}
    c->flags |= CALLOUT_PENDING | CALLOUT_ACTIVE;
    c->fun = f;
    c->arg = p;
    c->delta = delta;
    HAL_RESTORE_INTERRUPTS(flags);

#ifdef TIMEOUT_DEBUG
   diag_printf("%s(%p, %d, %p, %p)\n", __FUNCTION__, c, delta, f, p);
   _show_timeouts();
#endif

    if ((0 == last_delta ||      // alarm was inactive  OR
         delta < last_delta) ) { // alarm was active but later than we need

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
#ifdef TIMEOUT_DEBUG
        if ((int)last_set_time == 0) {
            diag_printf("delta: %d, time: %ld, last_delta: %d\n", delta, last_set_time, last_delta);
        }
#endif
        last_delta = delta;
    }
    // Otherwise, the alarm is active, AND it is set to fire sooner than we
    // require, so when it does, that will sort out calling the item we
    // just added.

#ifdef CYGPKG_INFRA_DEBUG
    // Do some more checking akin to that in the alarm handler:
    if ( last_delta != -1 ) { // not a recursive call
        cyg_tick_count_t now = cyg_current_time();
        timeout_entry *e;

        CYG_ASSERT( last_delta >= 0, "Bad last delta" );
        delta = 0x7fffffff;
        for (e = timeouts;  e;  e = e->next) {
            if (e->delta) {
                CYG_ASSERT( e->delta >= last_delta, "e->delta underflow" );
                // the following triggers if the "next" timeout has not just
                // passed, but passed by 1000 ticks - which with the normal
                // 1 tick = 10ms means 10 seconds - a long time.
                CYG_ASSERT( last_set_time + e->delta + 1000 > now,
                            "Recorded alarm not in the future! Starved network thread?" );
                if ( e->delta < delta )
                    delta = e->delta;
            } else {
                CYG_ASSERT( 0 == e->fun, "Function recorded for 0 delta" );
            }
        }
        if (delta < last_delta) {
            diag_printf("Failed to pick smallest delta - picked: %d, last: %d\n", delta, last_delta);
            for (e = timeouts;  e;  e = e->next) {
                diag_printf("  timeout: %p at %d\n", e->fun, e->delta);
            }
        }
        CYG_ASSERT( delta >= last_delta, "We didn't pick the smallest delta!" );
    }
#endif
    cyg_splx(spl);
}

#ifdef CYGPKG_HAL_MIPS_RLX
__attribute__((nomips16)) 
#endif
void 
callout_stop(struct callout *c) 
{
    int spl = cyg_splinternal();
    unsigned long flags;
#ifdef TIMEOUT_DEBUG
    diag_printf("%s(%p) = %x\n", __FUNCTION__, c, c->flags);
#endif
    if ((c->flags & CALLOUT_PENDING) == 0) {
        c->flags &= ~CALLOUT_ACTIVE;
        cyg_splx(spl);
        return;
    }
    HAL_DISABLE_INTERRUPTS(flags);
    c->flags &= ~(CALLOUT_PENDING | CALLOUT_ACTIVE);
    if (c->next) {
        c->next->prev = c->prev;
    }
    if (c->prev) {
        c->prev->next = c->next;
    } else {
        timeouts = c->next;
    }
	if(c==e_next_g)
		e_next_g=c->next;
    HAL_RESTORE_INTERRUPTS(flags);
    cyg_splx(spl);
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

#ifdef CYGPKG_HAL_MIPS_RLX
__attribute__((nomips16)) 
#endif
int  
callout_pending(struct callout *c) 
{
    return ((c->flags & CALLOUT_PENDING) != 0);
}


// ------------------------------------------------------------------------

// EOF timeout.c
