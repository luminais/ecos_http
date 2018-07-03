//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg, jlarmour
// Date:         1999-01-21
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>


#if defined(CYGPKG_IO_PCI)
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>
#endif

#include <cyg/hal/bspchip.h>

/*------------------------------------------------------------------------*/

void hal_platform_init(void)
{
	/* disable ict interrupt */
	REG32(BSP_GIMR) = 0;
	REG32(BSP_GIMR2) = 0;

	/* Set IRR */
	REG32(BSP_IRR0) = BSP_IRR0_SETTING;
	REG32(BSP_IRR1) = BSP_IRR1_SETTING;
	REG32(BSP_IRR2) = BSP_IRR2_SETTING;
	REG32(BSP_IRR3) = BSP_IRR3_SETTING;
	REG32(BSP_IRR4) = BSP_IRR4_SETTING;
	REG32(BSP_IRR5) = BSP_IRR5_SETTING;
	REG32(BSP_IRR6) = BSP_IRR6_SETTING;
	REG32(BSP_IRR7) = BSP_IRR7_SETTING;

	/* enable global interrupt mask */
#ifdef CONFIG_RTL_819X_INTERNAL_TIMER
	//REG32(BSP_GIMR) = BSP_WLAN_MAC_IE | BSP_PCIE_IE | BSP_SW_IE | BSP_UART0_IE;
	//REG32(BSP_GIMR2) = BSP_CPU_SI_TIMER_IE;
#else
	//REG32(BSP_GIMR) = BSP_TC0_IE | BSP_WLAN_MAC_IE | BSP_PCIE_IE | BSP_SW_IE | BSP_UART0_IE;
#ifdef CONFIG_TOCPU_MASK
	REG32(BSP_GIMR) = BSP_TC0_IE | BSP_WLAN_MAC_IE | BSP_SW_IE;
#endif
#endif

	// Set up eCos/ROM interfaces
	hal_if_init();
/*
	HAL_DCACHE_SYNC();
	HAL_ICACHE_INVALIDATE_ALL();
	HAL_ICACHE_ENABLE();
	HAL_DCACHE_INVALIDATE_ALL();
	HAL_DCACHE_ENABLE();
*/
	HAL_CLOCK_INITIALIZE(CYGNUM_HAL_RTC_PERIOD);
}


/*------------------------------------------------------------------------*/
/* Reset support                                                          */

void hal_rtl819x_reset(void)
{
	//local_irq_disable();
	REG32(BSP_WDTCNR) = 0; //enable watch dog
	while (1) ;
}


/*cache related*/

#if 0  /*code from uboot*/
typedef  unsigned long ulong;

#define cache_op(op,addr)						\
	__asm__ __volatile__(						\
	"	.set	push					\n"	\
	"	.set	noreorder				\n"	\
	"	.set	mips3\n\t				\n"	\
	"	cache	%0, %1					\n"	\
	"	.set	pop					\n"	\
	:								\
	: "i" (op), "R" (*(unsigned char *)(addr)))


static inline unsigned long icache_line_size(void)
{
	return 32;
}

static inline unsigned long dcache_line_size(void)
{
	return 32;
}


void flush_cache(ulong start_addr, ulong size)
{
	unsigned long ilsize = icache_line_size();
	unsigned long dlsize = dcache_line_size();
	unsigned long addr, aend;

	HAL_CLEAR_TAGLO();
	HAL_CLEAR_TAGHI();

	/* aend will be miscalculated when size is zero, so we return here */
	if (size == 0)
		return;

	addr = start_addr & ~(dlsize - 1);
	aend = (start_addr + size - 1) & ~(dlsize - 1);

	if (ilsize == dlsize) {
		/* flush I-cache & D-cache simultaneously */
		while (1) {
			cache_op(HIT_WRITEBACK_INV_D, addr);
			cache_op(HIT_INVALIDATE_I, addr);
			if (addr == aend)
				break;
			addr += dlsize;
		}
		return;
	}

	/* flush D-cache */
	while (1) {
		cache_op(HIT_WRITEBACK_INV_D, addr);
		if (addr == aend)
			break;
		addr += dlsize;
	}

	/* flush I-cache */
	addr = start_addr & ~(ilsize - 1);
	aend = (start_addr + size - 1) & ~(ilsize - 1);
	while (1) {
		cache_op(HIT_INVALIDATE_I, addr);
		if (addr == aend)
			break;
		addr += ilsize;
	}
}

void flush_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);
	//diag_printf("flush_dcache_range start_addr 0x%x  stop 0x%x\n",start_addr,stop);

	while (1) {
		cache_op(HIT_WRITEBACK_INV_D, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
	__sync();
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);
	//diag_printf("invalidate_dcache_range start_addr 0x%x  stop 0x%x\n",start_addr,stop);

	while (1) {
		cache_op(HIT_INVALIDATE_D, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}	
	__sync();
}

void invalidate_icache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);
	//diag_printf("invalidate_icache_range start_addr 0x%x  stop 0x%x\n",start_addr,stop);
	while (1) {
		cache_op(HIT_INVALIDATE_I, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
}

void rtl97f_flush_dcache_range(unsigned long start, unsigned long end)
{
	cyg_uint32 _old;	
	HAL_DISABLE_INTERRUPTS(_old);
	flush_dcache_range(start,end);
	HAL_RESTORE_INTERRUPTS(_old);
	return;	
}

void rtl97f_flush_dcache_all(void)
{
	cyg_uint32 _old;
	HAL_DISABLE_INTERRUPTS(_old);
	invalidate_dcache_range(INDEX_BASE,INDEX_BASE+cpu_dcache_size);
	HAL_RESTORE_INTERRUPTS(_old);
	return;
}

void rtl97f_inval_dcache_all(void)
{
	cyg_uint32 _old;
	HAL_DISABLE_INTERRUPTS(_old);
	rtl97f_inval_dcache_range(INDEX_BASE,INDEX_BASE+cpu_dcache_size);	
	HAL_RESTORE_INTERRUPTS(_old);
	return;
}

void rtl97f_inval_dcache_range(unsigned long start, unsigned long end)
{
	cyg_uint32 _old;	
	HAL_DISABLE_INTERRUPTS(_old);
	invalidate_dcache_range(start,end);	
	HAL_RESTORE_INTERRUPTS(_old);
	return;	
}

void rtl97f_inval_icache_range(unsigned long start, unsigned long end)
{
	cyg_uint32 _old;
	HAL_DISABLE_INTERRUPTS(_old);
	invalidate_icache_range(start,end);	
	HAL_RESTORE_INTERRUPTS(_old);
	return;	
}

void rtl97f_inval_icache_all(void)
{	
	cyg_uint32 _old;
	HAL_DISABLE_INTERRUPTS(_old);
	invalidate_icache_range(INDEX_BASE,INDEX_BASE+cpu_icache_size);
	HAL_RESTORE_INTERRUPTS(_old);
	return;
}

#else


void rtl97f_inval_dcache_all(void)
{
	cyg_uint32 _old;
	HAL_DISABLE_INTERRUPTS(_old);
	blast_inv_dcache32();	
	__sync();
	HAL_RESTORE_INTERRUPTS(_old);
	return;
}

void rtl97f_flush_dcache_all(void)
{
	cyg_uint32 _old;
	
	HAL_DISABLE_INTERRUPTS(_old);
	blast_dcache32();	
	__sync();
	HAL_RESTORE_INTERRUPTS(_old);
	return;
}

void rtl97f_flush_dcache_range(unsigned long start, unsigned long end)
{
	cyg_uint32 _old;
	HAL_DISABLE_INTERRUPTS(_old);
	blast_dcache_range(start,end);
	__sync();
	HAL_RESTORE_INTERRUPTS(_old);
	return;	
}

void rtl97f_inval_dcache_range(unsigned long start, unsigned long end)
{
	cyg_uint32 _old;	
	
	HAL_DISABLE_INTERRUPTS(_old);
	blast_inv_dcache_range(start,end);	
	HAL_RESTORE_INTERRUPTS(_old);
	return;	
}

void rtl97f_inval_icache_range(unsigned long start, unsigned long end)
{
	cyg_uint32 _old;
	HAL_DISABLE_INTERRUPTS(_old);
	blast_inv_icache_range(start,end);	
	HAL_RESTORE_INTERRUPTS(_old);
	return;	
}

void rtl97f_inval_icache_all(void)
{	
	cyg_uint32 _old;
	HAL_DISABLE_INTERRUPTS(_old);
	blast_icache32();	
	HAL_RESTORE_INTERRUPTS(_old);
	return;
}
#endif

/*cache code end*/


/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */

//--------------------------------------------------------------------------
#ifdef CYGPKG_IO_FLASH
#ifndef CYG_HAL_STARTUP_ROM
#include <pkgconf/hal.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif

#include <cyg/io/flash.h>
#include <cyg/io/flash_dev.h>
#include <cyg/hal/hal_io.h>
#include <cyg/io/spi_flash.h>

cyg_rtl819x_spi_flash_dev hal_rtl819x_spi_flash_priv;

CYG_FLASH_DRIVER(hal_rtl819x_spi_flash,
                 &cyg_rtl819x_spi_flash_funs,
                 0,
                 0xB0000000,
                 0,
                 0,
                 0,
                 &hal_rtl819x_spi_flash_priv
);
#endif
#endif
