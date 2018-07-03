/*
* Copyright (c) 2010, Realtek Semiconductor Corp.
*
* rlx_prof_int.c:
* Enable/Disable interrupts
*
*/
#include <pkgconf/hal.h>
#include <cyg/hal/hal_intr.h>
#include <rlx_library.h>

//int diag_printf(const char *, ...) __attribute__((long_call));

cyg_uint32 rlx_prof_old;

 __attribute__ ((section(".rlxprof_text"))) void rlx_prof_disable_int(void)
{
	/* disable interrupt here */
	HAL_DISABLE_INTERRUPTS(rlx_prof_old);
	//diag_printf("rlx_prof_disable_int()\n");
}

 __attribute__ ((section(".rlxprof_text"))) void rlx_prof_enable_int(void)
{
	/* enable interrupt here */
	HAL_RESTORE_INTERRUPTS(rlx_prof_old);
	//diag_printf("rlx_prof_enable_int()\n");
}
