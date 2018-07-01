/******************************************************************************
  *
  * Name: sys-support.h - System type support for Linux
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

#ifndef __TIMER_H__
#define __TIMER_H__
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <cyg/kernel/kapi.h>
#include <sys/param.h>

#define jiffies	cyg_current_time()
#define del_timer del_timer_sync

struct timer_list {
	timeout_entry e;
	unsigned long data;
	void (*function)(unsigned long);
};

static inline void timer_func(void *timer)
{
	(((struct timer_list *)timer)->function)(((struct timer_list *)timer)->data);
}

extern void init_timer(struct timer_list *timer);
extern void mod_timer(struct timer_list *timer, cyg_tick_count_t timeout_ticks);  
extern void del_timer_sync(struct timer_list *timer);
extern int timer_pending(struct timer_list *timer);

 /*
  *      These inlines deal with timer wrapping correctly. You are 
  *      strongly encouraged to use them
  *      1. Because people otherwise forget
  *      2. Because if the timer wrap changes in future you wont have to
  *         alter your driver code.
  *
  * time_after(a,b) returns true if the time a is after time b.
  *
  * Do this with "<0" and ">=0" to only test the sign of the result. A
  * good compiler would generate better code (and a really good compiler
  * wouldn't care). Gcc is currently neither.
  */
 #define time_after(a,b)		((long)(b) - (long)(a) < 0)
 #define time_before(a,b)		time_after(b,a)
  
 #define time_after_eq(a,b)		((long)(a) - (long)(b) >= 0)
 #define time_before_eq(a,b)	time_after_eq(b,a)
  
#endif /* __TIMER_H__ */
