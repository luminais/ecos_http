#ifndef CYGONCE_IMP_CACHE_H
#define CYGONCE_IMP_CACHE_H

//=============================================================================
//
//      imp_cache.h
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
// Contributors:        nickg, dmoseley
// Date:        1998-02-17
// Purpose:     Cache control API
// Description: The macros defined here provide the HAL APIs for handling
//              cache control operations.
// Usage:
//              #include <cyg/hal/imp_cache.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/mips-regs.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_cache.h>
#include <cyg/hal/var_arch.h>

//-----------------------------------------------------------------------------
// Global control of data cache

// Invalidate the entire cache
extern void rlx_inval_dcache_all(void);
#define HAL_DCACHE_INVALIDATE_ALL_DEFINED
#define HAL_DCACHE_INVALIDATE_ALL()	rlx_inval_dcache_all()

// Synchronize the contents of the cache with memory.
extern void rlx_flush_dcache_all(void);
#define HAL_DCACHE_SYNC_DEFINED
#define HAL_DCACHE_SYNC()	rlx_flush_dcache_all()

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
extern void rlx_flush_dcache_range(unsigned long start, unsigned long end);
#define HAL_DCACHE_FLUSH_DEFINED
#define HAL_DCACHE_FLUSH( _base_ , _asize_ ) \
	rlx_flush_dcache_range((unsigned long)_base_ , (unsigned long)(_base_+_asize_))

// Write dirty cache lines to memory for the given address range.
extern void rlx_wback_dcache_range(unsigned long start, unsigned long end);
#define HAL_DCACHE_STORE_DEFINED
#define HAL_DCACHE_STORE( _base_ , _asize_ ) \
	rlx_wback_dcache_range((unsigned long)_base_ , (unsigned long)(_base_+_asize_))

// Invalidate cache lines in the given range without writing to memory.
extern void rlx_inval_dcache_range(unsigned long start, unsigned long end);
#define HAL_DCACHE_INVALIDATE_DEFINED
#define HAL_DCACHE_INVALIDATE( _base_ , _asize_ ) \
	rlx_inval_dcache_range((unsigned long)_base_ , (unsigned long)(_base_+_asize_))

//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Invalidate the entire cache
extern void rlx_inval_icache_all(void);
#define HAL_ICACHE_INVALIDATE_ALL_DEFINED
#define HAL_ICACHE_INVALIDATE_ALL()	rlx_inval_icache_all()
    
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
#endif // ifndef CYGONCE_IMP_CACHE_H
// End of imp_cache.h
