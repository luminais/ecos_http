/*SysRq.cxx */

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
#ifdef CYGPKG_HAL_ROMEPERF_SUPPORT
#include <cyg/hal/romeperf.h>
#endif
#include <cyg/hal/hal_if.h>
#include <time.h>
#ifdef CYGPKG_IO_FLASH
#include <cyg/io/flash.h>
#endif
#include <cyg/io/watchdog.h>

int sysrq_debug_print=0;
extern int cyg_kmem_print_stats(void);
void dump_thread_info(cyg_uint16 pid);
void dump_current_thread();

#ifdef __cplusplus
}
#endif


typedef  void (*FUNC)(HAL_SavedRegisters *regs);

struct sysrq_entry {
	FUNC function;
};


/*callback functions, the function will be called in interrupt context
  *so schedule MUST not be called in those sysrq functions.
  */

void sysrq_d(HAL_SavedRegisters *regs)
{
	sysrq_debug_print ^= 1;
}


/*dump memory*/
void sysrq_m(HAL_SavedRegisters *regs)
{
	struct mallinfo info;
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
}

/*dump thread info*/
void sysrq_t(HAL_SavedRegisters *regs)
{
	dump_thread_info(0);
}

/*dump regs and the interrupted task's context*/
void sysrq_r(HAL_SavedRegisters *regs)
{
	/*In interrupt. interrupted at task or at DSR*/
	/*if at DSR , interrupt_end should be the entry point*/
	dump_registers(regs);
}

/*eth net stats*/
void sysrq_e(HAL_SavedRegisters *regs)
{

}

/*wifi stats*/
void sysrq_w(HAL_SavedRegisters *regs)
{
}

/*usb stats*/
void sysrq_u(HAL_SavedRegisters *regs)
{
}

/*quit-->reboot*/
void sysrq_q(HAL_SavedRegisters *regs)
{
	static int sysrq_q_count=0;
	if(sysrq_q_count++ < 1) {
		diag_printf("\nreboot ?\n");
		return;
	}	
	HAL_PLATFORM_RESET();
}

static struct sysrq_entry sysrq_entry_tbl[] = {
	/*a*/	{NULL},
	/*b*/	{NULL},
	/*c*/	{NULL},
	/*d*/	{sysrq_d},
	/*e*/     {NULL},
	/*f*/	{NULL},
	/*g*/     {NULL},
	/*h*/     {NULL},
	/*i*/     {NULL},
	/*j*/     {NULL},
	/*k*/     {NULL},
	/*l*/     {NULL},
	/*m*/    {sysrq_m},
	/*n*/     {NULL},
	/*o*/     {NULL},
	/*p*/     {NULL},
	/*q*/     {sysrq_q},
	/*r*/     {sysrq_r},
	/*s*/     {NULL},
	/*t*/     {sysrq_t},
	/*u*/     {NULL},
	/*v*/     {NULL},
	/*w*/     {NULL},
	/*x*/     {NULL},
	/*y*/     {NULL},
	/*z*/     {NULL},
};

/*
void sysrq_register(cyg_uint8 letter, FUNC callback)
{
	int index;
	index=get_index(letter)l
	if(sysrq_entry_tbl[index].function == NULL)
		sysrq_entry_tbl[index].function=callback;
	else
		diag_printf("slot %d not empty function %p\n",index,sysrq_entry_tbl[index].function);
}
*/

unsigned char get_index(cyg_uint8  letter)
{
	/*a-z*/
	if(letter >= 0x61 && letter <= 0x7a)
		return (letter-0x61);
	/*A-Z*/
	if(letter >= 0x41 && letter <= 0x5a)
		return (letter-0x41);
}

void rtl819x_handle_sysrq(cyg_uint8  letter, cyg_addrword_t regs)
{
	FUNC callback;
	cyg_uint32 flag;
	callback=sysrq_entry_tbl[get_index(letter)].function;
	
	HAL_DISABLE_INTERRUPTS(flag);
	if(callback)
		(*callback)((HAL_SavedRegisters *)regs);
	
	HAL_RESTORE_INTERRUPTS(flag);
	return;
}

