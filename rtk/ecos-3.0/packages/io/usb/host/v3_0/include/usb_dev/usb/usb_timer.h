#ifndef _USB_TIMER_H_
#define _USB_TIMER_H_

#include <cyg/kernel/kapi.h>

#define restore_flags(x)			HAL_RESTORE_INTERRUPTS(x)
#define save_and_cli(x)				HAL_DISABLE_INTERRUPTS(x)

#define jiffies	cyg_current_time()
#define del_timer del_timer_sync

enum {
	TIMER_NO_INIT = 0,
	TIMER_INIT = 1,
	TIMER_START = 2,
	TIMER_DISABLE = 3
};

struct timer_list {
	cyg_handle_t alarm_hdl;	
	cyg_alarm alarm_obj;
	int flag;
	unsigned long data;
	void (*function)(cyg_handle_t, cyg_addrword_t);
};

static inline void init_timer(struct timer_list *timer, unsigned int data, 
			 	void (*function)(cyg_handle_t, cyg_addrword_t))
{
	cyg_handle_t sys_clk;
	cyg_handle_t counter_hdl;
	int flags;
	save_and_cli(flags);

	if (timer->flag != TIMER_DISABLE) {
		if (timer->flag == TIMER_NO_INIT) {
			sys_clk = cyg_real_time_clock();
			cyg_clock_to_counter(sys_clk, &counter_hdl);
			cyg_alarm_create(counter_hdl, function, data,
						&timer->alarm_hdl, &timer->alarm_obj);
						
			timer->function = function;
			timer->data = data;
			timer->flag = TIMER_INIT;
		}
		else if (timer->flag == TIMER_START) {
			cyg_alarm_disable(timer->alarm_hdl);
			timer->flag = TIMER_DISABLE;
		}
	}
	restore_flags(flags);	
	
}

static inline void mod_timer(struct timer_list *timer, cyg_tick_count_t timeout_ticks)
{
	int flags;
	save_and_cli(flags);

	if (timer->flag == TIMER_NO_INIT) {
		if (timer->function) {
			cyg_handle_t sys_clk;
			cyg_handle_t counter_hdl;
		
			sys_clk = cyg_real_time_clock();
			cyg_clock_to_counter(sys_clk, &counter_hdl);
			cyg_alarm_create(counter_hdl, (cyg_alarm_t *)timer->function, timer->data, &timer->alarm_hdl, &timer->alarm_obj);
			timer->flag = TIMER_INIT;
		}
		else {
			//diag_printf("###mod_timer() not initilized, timer->flag=%d timer->function=%p timeout_ticks=%llu###\n", timer->flag, timer->function, timeout_ticks);
			restore_flags(flags);
			return;
		}
	}
	else if (timer->flag == TIMER_START) {
		cyg_alarm_disable(timer->alarm_hdl);
	 	timer->flag = TIMER_DISABLE;
	}
	
	if (timeout_ticks < cyg_current_time())
		timeout_ticks = cyg_current_time() + 2;
	//if (timeout_ticks == 0)
	//	diag_printf("###mod_timer() timer->flag=%d timer->function=%p timeout_ticks=%llu###\n", timer->flag, timer->function, timeout_ticks);
	
	cyg_alarm_initialize(timer->alarm_hdl, timeout_ticks, 0);
	
	if (timer->flag == TIMER_DISABLE)
		 cyg_alarm_enable(timer->alarm_hdl);
	timer->flag = TIMER_START;
	restore_flags(flags);	
}
  
static inline int timer_pending (const struct timer_list * timer)
{
	if (timer->alarm_hdl && timer->flag != TIMER_NO_INIT)
		return 1;
	else
		return 0;
}
  
static inline void  del_timer_sync(struct timer_list * timer)
{
	int flags;

	save_and_cli(flags);
	if (timer->alarm_hdl && timer->flag != TIMER_INIT) {
		if (timer->flag == TIMER_START)
			cyg_alarm_disable(timer->alarm_hdl);
		
		cyg_alarm_delete(timer->alarm_hdl);
		timer->flag = TIMER_NO_INIT;
	}
	restore_flags(flags);
}

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

 #endif
