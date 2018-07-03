//==========================================================================
//
//      var_misc.c
//
//      HAL implementation miscellaneous functions
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
// Contributors: nickg, jlarmour, dmoseley
// Date:         2000-07-14
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_intr.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/var_arch.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_cache.h>
#ifdef CYGPKG_HAL_RLX_PROFILING_SUPPORT
#include <rlx_library.h>
#endif
/*------------------------------------------------------------------------*/
// Array which stores the configured priority levels for the configured
// interrupts.

volatile CYG_BYTE hal_interrupt_level[CYGNUM_HAL_ISR_COUNT];
unsigned long rtk_interrupt_count[CYGNUM_HAL_ISR_COUNT];

/*------------------------------------------------------------------------*/

void hal_variant_init(void)
{
#ifdef CYGPKG_HAL_RLX_PROFILING_SUPPORT
	//extern void rlx_prof_init(void) __attribute__ ((long_call));
	//extern void rlx_gdb_set_param_addr(unsigned int addr) __attribute__ ((long_call));
	extern int __gdb_io_parameter_base_address__;

	rlx_prof_init();
	rlx_gdb_set_param_addr((unsigned int)&__gdb_io_parameter_base_address__);
#endif
}

/*------------------------------------------------------------------------*/

#ifdef CONFIG_CPU_HAS_SYNC
#define __sync()				\
	__asm__ __volatile__(			\
		".set	push\n\t"		\
		".set	noreorder\n\t"		\
		"sync\n\t"			\
		".set	pop"			\
		: /* no output */		\
		: /* no input */		\
		: "memory")
#else
#define __sync()	do { } while(0)
#endif

//wbflush() or fast_iob()
void rlx_flush_write_buffer(void)
{
	__sync();
	/* read a uncacheable address to cause write buffer to be flushed */
	__asm__ __volatile__(
		".set\tpush\n"
		".set\tnoreorder\n"
		"\tlw\t$0, %0\n"
		"\tnop\n"
		".set\tpop\n"
		: /* no output */
		: "m" (*(int *)CYGARC_KSEG_UNCACHED_BASE) /*CYGARC_KSEG_UNCACHED_BASE or 0xbfc00000*/
		: "memory");
}

//-----------------------------------------------------------------------------
// cache-rlx.c: RLX specific mmu/cache code.
//-----------------------------------------------------------------------------
/*
 * Determine whether CPU has CACHE OP
 */
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181) || \
    defined(CONFIG_CPU_RLX4281) || defined(CONFIG_CPU_RLX5281)
  #define CONFIG_CPU_HAS_DCACHE_OP
#else
  #undef CONFIG_CPU_HAS_DCACHE_OP
#endif

/*
#if defined(CONFIG_CPU_RLX4281) || defined(CONFIG_CPU_RLX5281)
  #define CONFIG_CPU_HAS_ICACHE_OP
#else
  #undef CONFIG_CPU_HAS_ICACHE_OP
#endif
*/

/*
 *  CACHE OP
 *   0x10 = IInval
 *   0x11 = DInval
 *   0x15 = DWBInval
 *   0x19 = DWB
 *   0x1b = DWB_IInval
 */
#define CACHE_ICACHE_INVAL	0x10
#define CACHE_DCACHE_INVAL	0x11
#if defined(CONFIG_CPU_HAS_WBC) || defined(CONFIG_CPU_HAS_L2C)
  #define CACHE_DCACHE_FLUSH	0x15
  #define CACHE_DCACHE_WBACK	0x19
#else
  #define CACHE_DCACHE_FLUSH	0x11
  #define CACHE_DCACHE_WBACK	0x11
#endif

#ifdef CONFIG_CPU_HAS_WBIC
  #define CACHE_ICACHE_FLUSH	0x1b
#else
  #define CACHE_ICACHE_FLUSH	0x10
#endif

#define CACHE_OP(op, p)          \
    __asm__ __volatile__ (       \
         ".set  push\n"          \
         ".set  noreorder\n"     \
         "cache %0, 0x000(%1)\n" \
         ".set  pop\n"           \
         : : "i" (op), "r" (p)   \
    )

#define CACHE32_UNROLL4(op, p)   \
    __asm__ __volatile__ (       \
         ".set  push\n"          \
         ".set  noreorder\n"     \
         "cache %0, 0x000(%1)\n" \
         "cache %0, 0x020(%1)\n" \
         "cache %0, 0x040(%1)\n" \
         "cache %0, 0x060(%1)\n" \
         ".set  pop\n"           \
         : : "i" (op), "r" (p)   \
    )

#define CACHE16_UNROLL8(op, p)   \
    __asm__ __volatile__ (       \
         ".set  push\n"          \
         ".set  noreorder\n"     \
         "cache %0, 0x000(%1)\n" \
         "cache %0, 0x010(%1)\n" \
         "cache %0, 0x020(%1)\n" \
         "cache %0, 0x030(%1)\n" \
         "cache %0, 0x040(%1)\n" \
         "cache %0, 0x050(%1)\n" \
         "cache %0, 0x060(%1)\n" \
         "cache %0, 0x070(%1)\n" \
         ".set  pop\n"           \
         : : "i" (op), "r" (p)   \
    )

#if (cpu_dcache_line == 32)
  #define DCACHE_OP(op,p)  CACHE32_UNROLL4(op,(p))
#else
  #define DCACHE_OP(op,p)  CACHE16_UNROLL8(op,(p))
#endif

#if (cpu_icache_line == 32)
  #define ICACHE_OP(op,p)  CACHE32_UNROLL4(op,(p))
#else
  #define ICACHE_OP(op,p)  CACHE16_UNROLL8(op,(p))
#endif

/*
 *  CCTL OP
 *   0x1   = DInval
 *   0x2   = IInval
 *   0x100 = DWB
 *   0x200 = DWB_Inval
 */
#define CCTL_DCACHE_WRITE_ALLOC_ON	0x80
#define CCTL_DCACHE_INVAL		0x1
#define CCTL_ICACHE_INVAL		0x2
#define CCTL_ICACHE_FLUSH		0x2
#if defined(CONFIG_CPU_HAS_WBC) || defined(CONFIG_CPU_HAS_L2C)
  #define CCTL_DCACHE_WBACK		0x100
  #define CCTL_DCACHE_FLUSH		0x200
#else
  #define CCTL_DCACHE_WBACK		0x1
  #define CCTL_DCACHE_FLUSH		0x1
#endif

#if defined(CONFIG_CPU_RLX4281) || defined(CONFIG_CPU_RLX5281)
#define CCTL_OP(op)		\
    __asm__ __volatile__(	\
       ".set  push\n"		\
       ".set  noreorder\n"	\
       "mtc0	$0, $20\n"	\
       "li	$8, %0\n"	\
       "mtc0	$8, $20\n"	\
       ".set  pop\n"		\
       : : "i" (op)		\
   )
#else
#define CCTL_OP(op)		\
    __asm__ __volatile__(	\
       ".set  push\n"		\
       ".set  noreorder\n"	\
       "mfc0	$8, $20\n"	\
       "ori	$8, %0\n"	\
       "xori	$9, $8, %0\n"	\
       "mtc0	$9, $20\n"	\
       "mtc0	$8, $20\n"	\
       ".set pop\n"		\
       : : "i" (op)		\
   )
#endif

/*
 * Dummy cache handling routines for machines without boardcaches
 */
//static void cache_noop(void) {}

static inline void rlx_flush_dcache_fast(unsigned long start, unsigned long end)
{
	unsigned long p;

	for (p = start; p < end; p += 0x080) {
		DCACHE_OP(CACHE_DCACHE_FLUSH, p);
	}

	p = p & ~(cpu_dcache_line -1);
	if (p <= end)
		CACHE_OP(CACHE_DCACHE_FLUSH, p);
}

static inline void rlx_wback_dcache_fast(unsigned long start, unsigned long end)
{
	unsigned long p;

	for (p = start; p < end; p += 0x080) {
		DCACHE_OP(CACHE_DCACHE_WBACK, p);
	}

	p = p & ~(cpu_dcache_line -1);
	if (p <= end)
		CACHE_OP(CACHE_DCACHE_WBACK, p);
}

static inline void rlx_inval_dcache_fast(unsigned long start, unsigned long end)
{
	unsigned long p;

	for (p = start; p < end; p += 0x080) {
		DCACHE_OP(CACHE_DCACHE_INVAL, p);
	}

	p = p & ~(cpu_dcache_line -1);
	if (p <= end)
		CACHE_OP(CACHE_DCACHE_INVAL, p);
}

/*
static inline void rlx_flush_icache_fast(unsigned long start, unsigned long end)
{
	unsigned long p;

	for (p = start; p < end; p += 0x080) {
		ICACHE_OP(CACHE_ICACHE_FLUSH, p);
	}

	p = p & ~(cpu_icache_line -1);
	if (p <= end)
		CACHE_OP(CACHE_ICACHE_FLUSH, p);
}
*/

/*
 * DCACHE part
 */
#ifdef CYGSEM_HAL_IMEM_SUPPORT
__attribute__  ((section(".iram-gen")))
#endif
void rlx_flush_dcache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_CPU_HAS_DCACHE_OP
	cyg_uint32 _old;
	if ((end - start) > cpu_dcache_size) {
		CCTL_OP(CCTL_DCACHE_FLUSH);
		return;
	}
	HAL_DISABLE_INTERRUPTS(_old);
	rlx_flush_dcache_fast(start, end);
	HAL_RESTORE_INTERRUPTS(_old);
#else
	CCTL_OP(CCTL_DCACHE_FLUSH);
#endif
	//rlx_flush_write_buffer();
}

void rlx_wback_dcache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_CPU_HAS_DCACHE_OP
	cyg_uint32 _old;
	if ((end - start) > cpu_dcache_size) {
		CCTL_OP(CCTL_DCACHE_WBACK);
		return;
	}
	HAL_DISABLE_INTERRUPTS(_old);
	rlx_wback_dcache_fast(start, end);
	HAL_RESTORE_INTERRUPTS(_old);
#else
	CCTL_OP(CCTL_DCACHE_WBACK);
#endif
	//rlx_flush_write_buffer();
}

void rlx_inval_dcache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_CPU_HAS_DCACHE_OP
	cyg_uint32 _old;
	if ((end - start) > cpu_dcache_size) {
		CCTL_OP(CCTL_DCACHE_INVAL);
		return;
	}
	HAL_DISABLE_INTERRUPTS(_old);
	rlx_inval_dcache_fast(start, end);
	HAL_RESTORE_INTERRUPTS(_old);
#else
	CCTL_OP(CCTL_DCACHE_INVAL);
#endif
}

/*
 * ICACHE part
 */
/*
static inline void local_rlx_flush_icache_range(unsigned long start, unsigned long end)
{
#ifdef CONFIG_CPU_HAS_ICACHE_OP
	unsigned long size;

  #if !defined(CONFIG_CPU_HAS_WBIC) \
	&& (defined(CONFIG_CPU_HAS_WBC) \
		|| defined(CONFIG_CPU_HAS_L2C))
	rlx_flush_dcache_range(start, end);
  #endif

	size = end - start;
	if (size > cpu_icache_size) {
  #ifdef CONFIG_CPU_HAS_WBIC
		if (size > cpu_dcache_size) {
			CCTL_OP(CCTL_ICACHE_FLUSH | CCTL_DCACHE_FLUSH);
			return;
		} else
			rlx_flush_dcache_fast(start, end);
  #endif
		CCTL_OP(CCTL_ICACHE_FLUSH);
		return;
	}

	rlx_flush_icache_fast(start, end);
#else
	rlx_flush_dcache_range(start, end);
	CCTL_OP(CCTL_ICACHE_FLUSH);
#endif
}

static void rlx_flush_icache_range(unsigned long start, unsigned long end)
{
	preempt_disable();
	local_rlx_flush_icache_range(start, end);
	preempt_enable();
}
*/

void inline rlx_flush_dcache_all(void)
{
	CCTL_OP(CCTL_DCACHE_FLUSH);
	//rlx_flush_write_buffer();
}

/*
void inline rlx_wback_dcache_all(void)
{
	CCTL_OP(CCTL_DCACHE_WBACK);
	//rlx_flush_write_buffer();
}
*/

void inline rlx_inval_dcache_all(void)
{
	CCTL_OP(CCTL_DCACHE_INVAL);
}

void inline rlx_inval_icache_all(void)
{
	CCTL_OP(CCTL_ICACHE_INVAL);
}
/*------------------------------------------------------------------------*/
// Initialize the caches

int hal_init_icache(void)
{
	//rlx_inval_icache_all();
	return HAL_ICACHE_SIZE;
}

int hal_init_dcache(void)
{
#ifdef CYGPKG_HAL_DWALLOC_SUPPORT
	CCTL_OP(CCTL_DCACHE_WRITE_ALLOC_ON);
#endif
	//rlx_inval_dcache_all();
	return HAL_DCACHE_SIZE;
}

void hal_c_cache_init(unsigned long config1_val)
{
	if (hal_init_icache() == -1)
	{
		/* Error */
		;
	}

	if (hal_init_dcache() == -1)
	{
		/* Error */
		;
	}
}

//==========================================================================
// When compiling C++ code with static objects the compiler
// inserts a call to __cxa_atexit() with __dso_handle as one of the
// arguments. __cxa_atexit() would normally be provided by glibc, and
// __dso_handle is part of crtstuff.c. eCos applications
// are linked rather differently, so either a differently-configured
// compiler is needed or dummy versions of these symbols should be
// provided. If these symbols are not actually used then providing
// them is still harmless, linker garbage collection will remove them.
void
__cxa_atexit(void (*arg1)(void*), void* arg2, void* arg3)
{
}

void*   __dso_handle = (void*) &__dso_handle;
/*------------------------------------------------------------------------*/
/* End of var_misc.c                                                      */
