//=================================================================
//
//        softdog.cxx
//
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

#include <network.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>
#include <cyg/io/devtab.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/plf_io.h>
#include <cyg/kernel/kapi.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_arch.h>
#include <ctype.h>
#include <stdlib.h>
#include <cyg/hal/romeperf.h>
#include <cyg/hal/hal_if.h>
#include <time.h>
#include <cyg/io/flash.h>
#include <cyg/io/watchdog.h>


#define SOFTDOG_EXPIRED_TH 	300 /*Seconds*/
static unsigned int softdog_time_passed=0; /*how long idle not schedule*/

extern int cyg_kmem_print_stats(void);
void dump_thread_info(cyg_uint16 pid);
void dump_current_thread();

#ifdef __cplusplus
}
#endif

void softdog_inc_time(void)
{
	softdog_time_passed++;
}

void softdog_dec_time(void)
{
	softdog_time_passed--;
}

void softdog_clear_time(void)
{
	softdog_time_passed=0;
}

externC void softdog_clear_time_c(void)
{	
	softdog_clear_time();
}
static int softdog_expired()
{
	return (softdog_time_passed > SOFTDOG_EXPIRED_TH);
}

void softdog_dump()
{
	struct mallinfo info;
	unsigned long flag;

	/*dump the current as early as can, 
	   since the DSR will be interrupted again,
	   the call trace dump will fail*/
	dump_current_thread();
	
	diag_printf("\nmeminfo:\n");
	cyg_kmem_print_stats();	

	diag_printf("\nmallinfo:\n");
	info = mallinfo();
	diag_printf("arena: %d\n", info.arena);
	diag_printf("ordblks: %d\n", info.ordblks);
	diag_printf("smblks: %d\n", info.smblks);
	diag_printf("hblks: %d\n", info.hblks);
	diag_printf("hblkhd: %d\n", info.hblkhd);
	diag_printf("usmblks: %d\n", info.usmblks);
	diag_printf("fsmblks: %d\n", info.fsmblks);
	diag_printf("uordblks: %d\n", info.uordblks);
	diag_printf("fordblks: %d\n", info.fordblks);
	diag_printf("keepcost: %d\n", info.keepcost);
	diag_printf("maxfree: %d\n", info.maxfree);

	diag_printf("\ndump thread:\n");
	dump_thread_info(0);
}
void softdog_fire()
{
	/*Here may dump all the task info or meminfo*/
	softdog_dump();	
	diag_printf("\n SoftDog triggered! Reboot!\n");
	HAL_PLATFORM_RESET();
}

externC void softdog_check()
{
	//diag_printf("%s %d softdog_time_passed %d\n",__FUNCTION__,__LINE__,softdog_time_passed);
	softdog_inc_time();
	if(softdog_expired())
		softdog_fire();
	return;
}
