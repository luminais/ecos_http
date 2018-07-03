 /******************************************************************************
  *
  * Name: timer.c - 
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

/* System include files */
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>

void dump_timer(struct timer_list *timer)
{
	int count=0;
	timeout_entry *ep,*ep_next, *ep_first;
	ep=&timer->e;
	ep_first=ep;
	diag_printf("timer %p fun:%p\n",timer,timer->function);
	while(ep != NULL ) {
		dump_timer_entry(ep);
		count++;
		ep=ep->next;
		if(ep == ep_first) {
			diag_printf("back\n");
			break;
		}	
	}
	diag_printf("count %d\n",count);
}

void dump_timer_entry(timeout_entry *ep)
{
	diag_printf("ep %p\n",ep);
	diag_printf("ep flags 0x%x\n",ep->flags);
	diag_printf("ep delta 0x%x\n",ep->delta);	
	diag_printf("ep next %p  prev %p\n",ep->next,ep->prev);
	diag_printf("ep args %p  func %p\n",ep->arg,ep->fun);
}

void init_timer(struct timer_list *timer)
{
    callout_init(&timer->e);
}

void mod_timer(struct timer_list *timer, cyg_tick_count_t timeout_ticks)
{
	timeout_ticks -= cyg_current_time();
	if (timeout_ticks < 0)
		timeout_ticks = 2;
	//if (timeout_ticks == 0)
	//	diag_printf("###mod_timer() timer->flag=%d timer->function=%p timeout_ticks=%llu###\n", timer->flag, timer->function, timeout_ticks);
	callout_reset(&timer->e, (cyg_int32)timeout_ticks, timer_func, (void *)timer);
}

void del_timer_sync(struct timer_list *timer)
{
	callout_stop(&timer->e);
}

int timer_pending(struct timer_list *timer)
{
	return callout_pending(&timer->e);
}
