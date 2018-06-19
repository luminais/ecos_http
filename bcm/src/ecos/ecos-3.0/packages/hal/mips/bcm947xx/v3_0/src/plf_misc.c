//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg, jlarmour, dmoseley, jskov
// Date:         2001-03-20
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>

#include CYGBLD_HAL_PLATFORM_H
#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>






/*------------------------------------------------------------------------*/
// Initialize the caches

int hal_icache_size(void)
{
	int icache_linesize, icache_assoc, icache_sets;
	volatile unsigned config1_val;
	
	asm volatile("mfc0 %0,$16,1;" : "=r"(config1_val));
	
	switch (config1_val & CONFIG1_IL)
	{
	case CONFIG1_ICACHE_LINE_SIZE_16_BYTES: icache_linesize = 16;      break;
	case CONFIG1_ICACHE_NOT_PRESET:         return -1;                 break;
	default:      /* Error */               return -1;                 break;
	}

	switch (config1_val & CONFIG1_IA)
	{
	case CONFIG1_ICACHE_DIRECT_MAPPED:      icache_assoc = 1;          break;
	case CONFIG1_ICACHE_2_WAY:              icache_assoc = 2;          break;
	case CONFIG1_ICACHE_3_WAY:              icache_assoc = 3;          break;
	case CONFIG1_ICACHE_4_WAY:              icache_assoc = 4;          break;
	default:      /* Error */               return -1;                 break;
	}

	switch (config1_val & CONFIG1_IS)
	{
	case CONFIG1_ICACHE_64_SETS_PER_WAY:    icache_sets = 64;          break;
	case CONFIG1_ICACHE_128_SETS_PER_WAY:   icache_sets = 128;         break;
	case CONFIG1_ICACHE_256_SETS_PER_WAY:   icache_sets = 256;         break;
	case CONFIG1_ICACHE_512_SETS_PER_WAY:   icache_sets = 512;         break;
	default:      /* Error */               return -1;                 break;
	}

	return (icache_sets * icache_assoc * icache_linesize);
}

int hal_dcache_size(void)
{
	int dcache_linesize, dcache_assoc, dcache_sets;
	volatile unsigned config1_val;
	
	asm volatile("mfc0 %0,$16,1;" : "=r"(config1_val));
	
	switch (config1_val & CONFIG1_DL)
	{
	case CONFIG1_DCACHE_LINE_SIZE_16_BYTES: dcache_linesize = 16;      break;
	case CONFIG1_DCACHE_NOT_PRESET:         return -1;                 break;
	default:      /* Error */               return -1;                 break;
	}
	
	switch (config1_val & CONFIG1_DA)
	{
	case CONFIG1_DCACHE_DIRECT_MAPPED:      dcache_assoc = 1;          break;
	case CONFIG1_DCACHE_2_WAY:              dcache_assoc = 2;          break;
	case CONFIG1_DCACHE_3_WAY:              dcache_assoc = 3;          break;
	case CONFIG1_DCACHE_4_WAY:              dcache_assoc = 4;          break;
	default:      /* Error */               return -1;                 break;
	}
	
	switch (config1_val & CONFIG1_DS)
	{
	case CONFIG1_DCACHE_64_SETS_PER_WAY:    dcache_sets = 64;          break;
	case CONFIG1_DCACHE_128_SETS_PER_WAY:   dcache_sets = 128;         break;
	case CONFIG1_DCACHE_256_SETS_PER_WAY:   dcache_sets = 256;         break;
	case CONFIG1_DCACHE_512_SETS_PER_WAY:   dcache_sets = 512;         break;
	default:      /* Error */               return -1;                 break;
	}

	return (dcache_sets * dcache_assoc * dcache_linesize);
}

void hal_c_cache_init(unsigned long config1_val)
{
	volatile unsigned val;
	int icache_size, dcache_size;
	
	icache_size = hal_icache_size();
	HAL_ICACHE_INVALIDATE_ALL(icache_size);
	
	dcache_size = hal_dcache_size();
	HAL_DCACHE_INVALIDATE_ALL(dcache_size);
	
	// enable (write-back) cached KSEG0
	asm volatile("mfc0 %0,$16;" : "=r"(val));
	val |= 3;
	asm volatile("mtc0 %0,$16;nop;nop;sll $0,$0,3;" : : "r"(val));
}

void hal_icache_sync(void)
{
	int icache_size;
	
	icache_size = hal_icache_size();
	HAL_ICACHE_INVALIDATE_ALL(icache_size);
}

void hal_dcache_sync(void)
{
	int dcache_size;
	
	dcache_size = hal_dcache_size();
	HAL_DCACHE_INVALIDATE_ALL(dcache_size);
}

void bcm_sb_init(void) CYGBLD_ATTRIB_WEAK; 


void bcm_sb_init(void) 
{

#define UART_REGS 0xb8000300

#define UART_BAUD_BASE 66000000

#define UART_IRQ 1

#define UART_REG_SHIFT 0

	void *regs;
	int irq, baud_base, reg_shift;


	regs = UART_REGS;
	irq = UART_IRQ;
	baud_base = UART_BAUD_BASE;
	reg_shift = UART_REG_SHIFT ;
	
        pc_serial_info0_add(regs, irq, baud_base, reg_shift);
        cyg_hal_plf_serial_add(regs, irq, baud_base, reg_shift);
        return;                 
}       

/*------------------------------------------------------------------------*/
    
/* this is called from the kernel */
void hal_platform_init(void)
{
    bcm_sb_init();

    hal_if_init();

    //HAL_ICACHE_INVALIDATE_ALL(hal_icache_size);
    //HAL_ICACHE_ENABLE();
    //HAL_DCACHE_INVALIDATE_ALL(hal_dcache_size);
    //HAL_DCACHE_ENABLE();
}

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */

