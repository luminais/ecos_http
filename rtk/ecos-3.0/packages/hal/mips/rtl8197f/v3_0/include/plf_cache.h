#ifndef CYGONCE_PLF_CACHE_H
#define CYGONCE_PLF_CACHE_H

//=============================================================================
//
//      plf_cache.h
//
//      HAL cache control API
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-02-17
// Purpose:     Cache control API
// Description: The macros defined here provide the HAL APIs for handling
//              cache control operations.
// Usage:
//              #include <cyg/hal/plf_cache.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/bspcpu.h>
#include <cyg/hal/plf_cache.h>

//=============================================================================

// Nothing here at present.

//-----------------------------------------------------------------------------
// Cache dimensions

// Data cache
#define HAL_DCACHE_SIZE             (32 << 10)	// Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE    32			// Size of a data cache line
#define HAL_DCACHE_WAYS           4			// Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE             (64 << 10)	// Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE    32			// Size of a cache line
#define HAL_ICACHE_WAYS           4			// Associativity of the cache

#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

#define HAL_DCACHE_WRITETHRU_MODE       0 //0
#define HAL_DCACHE_WRITEBACK_MODE       1 //1

#define CYGPKG_HAL_MIPS_CACHE_DIMENSIONS_DEFINED


// Add by HF. 
/*
 *  * Cache Operations available on all MIPS processors with R4000-style caches
 */
#define Index_Invalidate_I	0x00
#define Index_Writeback_Inv_D	0x01
#define Index_Load_Tag_I	0x04
#define Index_Load_Tag_D	0x05
#define Index_Store_Tag_I	0x08
#define Index_Store_Tag_D	0x09
#define Hit_Invalidate_I	0x10
#define Hit_Invalidate_D	0x11
#define Hit_Writeback_Inv_D	0x15

#if 0
#define INDEX_INVALIDATE_I      0x00
#define INDEX_WRITEBACK_INV_D   0x01
#define INDEX_LOAD_TAG_I    0x04
#define INDEX_LOAD_TAG_D    0x05
#define INDEX_STORE_TAG_I   0x08
#define INDEX_STORE_TAG_D   0x09
#define HIT_INVALIDATE_I    0x10
#define HIT_INVALIDATE_D    0x11
#define HIT_WRITEBACK_INV_D 0x15
#endif

#define INDEX_BASE CYGARC_KSEG_CACHED_BASE

#define cache_op(op,addr)						\
	__asm__ __volatile__(						\
	"	.set	push					\n"	\
	"	.set	noreorder				\n"	\
	"	.set	mips3\n\t				\n"	\
	"	cache	%0, %1					\n"	\
	"	.set	pop					\n"	\
	:								\
	: "i" (op), "R" (*(unsigned char *)(addr)))

#define cache32_unroll32(base,op)					\
	__asm__ __volatile__(						\
	"	.set push					\n"	\
	"	.set noreorder					\n"	\
	"	.set mips3					\n"	\
	"	cache %1, 0x000(%0); cache %1, 0x020(%0)	\n"	\
	"	cache %1, 0x040(%0); cache %1, 0x060(%0)	\n"	\
	"	cache %1, 0x080(%0); cache %1, 0x0a0(%0)	\n"	\
	"	cache %1, 0x0c0(%0); cache %1, 0x0e0(%0)	\n"	\
	"	cache %1, 0x100(%0); cache %1, 0x120(%0)	\n"	\
	"	cache %1, 0x140(%0); cache %1, 0x160(%0)	\n"	\
	"	cache %1, 0x180(%0); cache %1, 0x1a0(%0)	\n"	\
	"	cache %1, 0x1c0(%0); cache %1, 0x1e0(%0)	\n"	\
	"	cache %1, 0x200(%0); cache %1, 0x220(%0)	\n"	\
	"	cache %1, 0x240(%0); cache %1, 0x260(%0)	\n"	\
	"	cache %1, 0x280(%0); cache %1, 0x2a0(%0)	\n"	\
	"	cache %1, 0x2c0(%0); cache %1, 0x2e0(%0)	\n"	\
	"	cache %1, 0x300(%0); cache %1, 0x320(%0)	\n"	\
	"	cache %1, 0x340(%0); cache %1, 0x360(%0)	\n"	\
	"	cache %1, 0x380(%0); cache %1, 0x3a0(%0)	\n"	\
	"	cache %1, 0x3c0(%0); cache %1, 0x3e0(%0)	\n"	\
	"	.set pop					\n"	\
	:							\
	: "r" (base),						\
	"i" (op));

/* build blast_xxx, blast_xxx_page, blast_xxx_page_indexed */
#define __BUILD_BLAST_CACHE(pfx, desc, indexop, hitop, lsize) \
static inline void blast_##pfx##cache##lsize(void)			\
{									\
	unsigned long start = INDEX_BASE;				\
	unsigned long end = start + ((cpu_##desc##_size)/4);	\
	unsigned long ws_inc = ((cpu_##desc##_size)/4);	\
	unsigned long ws_end = (cpu_##desc##_size);   \
	unsigned long ws, addr;						\
												\
												\
	for (ws = 0; ws < ws_end; ws += ws_inc)				\
		for (addr = start; addr < end; addr += lsize * 32)	\
			cache##lsize##_unroll32(addr|ws, indexop);	\
														\
}

__BUILD_BLAST_CACHE(d, dcache, Index_Writeback_Inv_D, Hit_Writeback_Inv_D, 32)
__BUILD_BLAST_CACHE(i, icache, Index_Invalidate_I, Hit_Invalidate_I, 32)
__BUILD_BLAST_CACHE(inv_d, dcache, Index_Writeback_Inv_D, Hit_Invalidate_D, 32)


/* build blast_xxx_range, protected_blast_xxx_range */
#define __BUILD_BLAST_CACHE_RANGE(pfx, desc, hitop, prot) \
static inline void prot##blast_##pfx##cache##_range(unsigned long start, \
						    unsigned long end)	\
{									\
		unsigned long lsize = cpu_##desc##_line;			\
		unsigned long addr = start & ~(lsize - 1);			\
		unsigned long aend = (end - 1) & ~(lsize - 1);			\
																\
																\
		while (1) {							\
			prot##cache_op(hitop, addr);				\
			if (addr == aend)					\
				break;						\
			addr += lsize;						\
		}								\
										\
}

__BUILD_BLAST_CACHE_RANGE(d, dcache, Hit_Writeback_Inv_D, )
__BUILD_BLAST_CACHE_RANGE(inv_d, dcache, Hit_Invalidate_D, )
__BUILD_BLAST_CACHE_RANGE(inv_i, dcache, Hit_Invalidate_I, )


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Global control of data cache

#if 1
#define __sync()                \
    __asm__ __volatile__(           \
        ".set   push\n\t"       \
        ".set   noreorder\n\t"      \
        ".set   mips2\n\t"      \
        "sync\n\t"          \
        ".set   pop"            \
        : /* no output */       \
        : /* no input */        \
        : "memory")
#else
#define __sync()  do {} while(0)
#endif

//-----------------------------------------------------------------------------
// General cache defines.
#define HAL_CLEAR_TAGLO()  asm volatile (" mtc0 $0, $28;" \
                                             " nop;"      \
                                             " nop;"      \
                                             " nop;")
#define HAL_CLEAR_TAGHI()  asm volatile (" mtc0 $0, $29;" \
                                             " nop;"      \
                                             " nop;"      \
                                             " nop;")

// Invalidate the entire cache
extern void rtl97f_inval_dcache_all(void);
#define HAL_DCACHE_INVALIDATE_ALL_DEFINED
#define HAL_DCACHE_INVALIDATE_ALL()	rtl97f_inval_dcache_all()


// Synchronize the contents of the cache with memory.
extern void rtl97f_flush_dcache_all(void);
#define HAL_DCACHE_SYNC_DEFINED
#define HAL_DCACHE_SYNC()	rtl97f_flush_dcache_all()

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_asize_)

// Set the data cache write mode
//#define HAL_DCACHE_WRITE_MODE( _mode_ )

//-----------------------------------------------------------------------------
// The RLX has no cache locking facility so we define the guard macros
// to disable the definitions in hal_arch.h.
#define HAL_DCACHE_LOCK_DEFINED        // Ensure no default definition
#define HAL_DCACHE_UNLOCK_DEFINED      // Ensure no default definition
#define HAL_DCACHE_UNLOCK_ALL_DEFINED  // Ensure no default definition

//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _asize_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
extern void rtl97f_flush_dcache_range(unsigned long start, unsigned long end);
#define HAL_DCACHE_FLUSH_DEFINED
#define HAL_DCACHE_FLUSH( _base_ , _asize_ ) \
	rtl97f_flush_dcache_range((unsigned long)_base_ , (unsigned long)(_base_+_asize_))

// Write dirty cache lines to memory for the given address range.
//extern void rlx_wback_dcache_range(unsigned long start, unsigned long end);
#define HAL_DCACHE_STORE_DEFINED
#define HAL_DCACHE_STORE( _base_ , _asize_ ) \
	rtl97f_flush_dcache_range((unsigned long)_base_ , (unsigned long)(_base_+_asize_))

// Invalidate cache lines in the given range without writing to memory.
extern void rtl97f_inval_dcache_range(unsigned long start, unsigned long end);
#define HAL_DCACHE_INVALIDATE_DEFINED
#define HAL_DCACHE_INVALIDATE( _base_ , _asize_ ) \
	rtl97f_inval_dcache_range((unsigned long)_base_ , (unsigned long)(_base_+_asize_))

//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Invalidate the entire cache
extern void rtl97f_inval_icache_all(void);
#define HAL_ICACHE_INVALIDATE_ALL_DEFINED
#define HAL_ICACHE_INVALIDATE_ALL()	rtl97f_inval_icache_all()
    
// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC_DEFINED
#define HAL_ICACHE_SYNC()					\
    CYG_MACRO_START						\
    HAL_DCACHE_SYNC();						\
    HAL_ICACHE_INVALIDATE_ALL();				\
    CYG_MACRO_END

// Set the instruction cache refill burst size
//#define HAL_ICACHE_BURST_SIZE(_asize_)

//-----------------------------------------------------------------------------
// The RLX has no cache locking facility so we define the guard macros
// to disable the definitions in hal_arch.h.
#define HAL_ICACHE_LOCK_DEFINED        // Ensure no default definition
#define HAL_ICACHE_UNLOCK_DEFINED      // Ensure no default definition
#define HAL_ICACHE_UNLOCK_ALL_DEFINED  // Ensure no default definition

//-----------------------------------------------------------------------------
// Instruction cache line control

// Invalidate cache lines in the given range without writing to memory.
#define HAL_ICACHE_INVALIDATE_DEFINED
#define HAL_ICACHE_INVALIDATE( _base_ , _asize_ )               \
    CYG_MACRO_START                                             \
    HAL_ICACHE_INVALIDATE_ALL();				\
    CYG_MACRO_END

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_PLF_CACHE_H
// End of plf_cache.h

