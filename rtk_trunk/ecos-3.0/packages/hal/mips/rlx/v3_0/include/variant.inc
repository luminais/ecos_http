#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##	variant.inc
##
##	RLX family assembler header file
##
##=============================================================================
## ####ECOSGPLCOPYRIGHTBEGIN####                                            
## -------------------------------------------                              
## This file is part of eCos, the Embedded Configurable Operating System.   
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under    
## the terms of the GNU General Public License as published by the Free     
## Software Foundation; either version 2 or (at your option) any later      
## version.                                                                 
##
## eCos is distributed in the hope that it will be useful, but WITHOUT      
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
## for more details.                                                        
##
## You should have received a copy of the GNU General Public License        
## along with eCos; if not, write to the Free Software Foundation, Inc.,    
## 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
##
## As a special exception, if other files instantiate templates or use      
## macros or inline functions from this file, or you compile this file      
## and link it with other works to produce a work based on this file,       
## this file does not by itself cause the resulting work to be covered by   
## the GNU General Public License. However the source code for this file    
## must still be made available in accordance with section (3) of the GNU   
## General Public License v2.                                               
##
## This exception does not invalidate any other reasons why a work based    
## on this file might be covered by the GNU General Public License.         
## -------------------------------------------                              
## ####ECOSGPLCOPYRIGHTEND####                                              
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s): 	michael
## Contributors:
## Date:	2009-12-24
## Purpose:	RLX family definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the RLX CPU family.
## Usage:
##		#include <cyg/hal/variant.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/mips.inc>
#include <cyg/hal/platform.inc>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#ifndef __ASSEMBLER__
//#define __ASSEMBLER__
#endif
//#include <cyg/hal/mips-regs.h>
//#include <cyg/hal/var_arch.h>

##-----------------------------------------------------------------------------
## Define CPU variant for architecture HAL.
			
#define	CYG_HAL_MIPS_RLX

//-----------------------------------------------------------------------------
// Load Address and Relocate. This macro is used in code that may be
// linked to execute out of RAM but is actually executed from ROM. The
// code that initializes the memory controller and copies the ROM
// contents to RAM must work in this way, for example. This macro is used
// in place of an "la" macro instruction when loading code and data
// addresses.  There are two versions of the macro here. The first
// assumes that we are executing in the ROM space at 0xbfc00000 and are
// linked to run in the RAM space at 0x80000000.  It simply adds the
// difference between the two to the loaded address.  The second is more
// code, but will execute correctly at either location since it
// calculates the difference at runtime.  The second variant is enabled
// by default.


#ifdef CYG_HAL_STARTUP_ROMRAM

	.macro	lar	reg,addr
.set	noat
	move	$at,ra			# save ra
	la	\reg,\addr		# get address into register
	la	ra,x\@			# get linked address of label
	subu	\reg,\reg,ra		# subtract it from value
	bal	x\@			# branch and link to label
	nop				#  to get current actual address
x\@:
	addu	\reg,\reg,ra		# add actual address
	move	ra,$at			# restore ra
.set	at
	.endm
	
#define	CYGPKG_HAL_MIPS_LAR_DEFINED

#endif


#------------------------------------------------------------------------------
# Cache macros.
        
#ifndef CYGPKG_HAL_MIPS_CACHE_DEFINED

        .macro  hal_cache_init

	# Setup a temporary stack pointer for running C code.
	la	a0,__interrupt_stack
	move	sp,a0
        CYGARC_ADDRESS_REG_UNCACHED(sp)
        
        # Read the CONFIG1 register into a0
        # a0 is not used in RLX/hal_c_cache_init()
        #mfc0	a0, C0_CONFIG, 1
	#nop
	#nop
	#nop

	# Jump to C-code to initialize caches (uncached)
	lar	k0, hal_c_cache_init
	CYGARC_ADDRESS_REG_UNCACHED(k0)
	jalr	k0
	nop
        .endm

#define CYGPKG_HAL_MIPS_CACHE_DEFINED

#endif  


# The other_vector trampoline is already installed as part of
# the executable image. However, the TLB exception is, in RAM
# in the analogous place to the reset vector in ROM. In a
# ROM or ROMRAM startup we need to copy it into place.

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) || \
    (defined(CYG_HAL_STARTUP_RAM) && !defined(CYGSEM_HAL_USE_ROM_MONITOR))
	.macro	hal_mon_copy_trampoline
	la	a0,utlb_vector
	la	a1,utlb_vector_end
	la	t0,0x80000000
1:
	lw	v0,0(a0)
	nop
	sw	v0,0(t0)
	addiu	a0,a0,4
	bne	a0,a1,1b
	addiu	t0,t0,4
#if defined(CYG_HAL_STARTUP_ROM) || \
    (defined(CYG_HAL_STARTUP_RAM) && !defined(CYGSEM_HAL_USE_ROM_MONITOR))
	la	a0,other_vector
	la	a1,other_vector_end
	la	t0,0x80000080
1:
	lw	v0,0(a0)
	nop
	sw	v0,0(t0)
	addiu	a0,a0,4
	bne	a0,a1,1b
	addiu	t0,t0,4
#endif
	.endm
#else
	.macro	hal_mon_copy_trampoline
	.endm
#endif
	
#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_MIPS_MON_DEFINED

#if	defined(CYG_HAL_STARTUP_ROM) ||			\
	defined(CYG_HAL_STARTUP_ROMRAM) ||		\
	(	defined(CYG_HAL_STARTUP_RAM) &&		\
		!defined(CYGSEM_HAL_USE_ROM_MONITOR))
	# If we are starting up from ROM, or we are starting in
	# RAM and NOT using a ROM monitor, initialize the VSR table.

	.macro	hal_mon_init
	hal_mon_copy_trampoline
        # Set default exception VSR for all vectors
        ori     a0,zero,16  # CYGNUM_HAL_VSR_COUNT
	la	a1,__default_exception_vsr
	la	a2,hal_vsr_table
1:      sw      a1,0(a2)
        addi    a2,a2,4
        addi    a0,a0,-1
        bne     a0,zero,1b
         nop

        # Now set special VSRs
       	la	a0,hal_vsr_table
        # Set interrupt VSR
	la	a1,__default_interrupt_vsr
        sw      a1,0*4(a0)  # CYGNUM_HAL_VECTOR_INTERRUPT
        # Add special handler on breakpoint vector to allow GDB and
        # GCC to both use 'break' without conflicts.
	la	a1,__break_vsr_springboard
	sw	a1,9*4(a0)  # CYGNUM_HAL_VECTOR_BREAKPOINT
        # Set exception handler on special vectors
        # FIXME: Should use proper definitions
	la	a1,__default_exception_vsr
	sw	a1,32*4(a0)             # debug
	sw	a1,33*4(a0)             # utlb
	sw	a1,34*4(a0)             # nmi
	flush_dcache_invalidate_icache
	.endm
	
#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

	# Initialize the VSR table entries
	# We only take control of the interrupt vector,
	# the rest are left to the ROM for now...

	.macro	hal_mon_init
	hal_mon_copy_trampoline
	la	a0,__default_interrupt_vsr
	la	a3,hal_vsr_table
	sw	a0,0(a3)
	flush_dcache_invalidate_icache
	.endm

#else

	.macro	hal_mon_init
	hal_mon_copy_trampoline
	flush_dcache_invalidate_icache
	.endm

#endif
			

#define CYGPKG_HAL_MIPS_MON_DEFINED

#endif

#------------------------------------------------------------------------------
# Decide whether the VSR table is defined externally, or is to be defined
# here.

#if defined(CYGPKG_HAL_MIPS_SIM) ||        \
    ( defined(CYGPKG_HAL_MIPS_ATLAS) &&    \
      defined(CYG_HAL_STARTUP_RAM) &&      \
      !defined(CYGSEM_HAL_USE_ROM_MONITOR) \
    )

## VSR table defined in linker script

#else

#define CYG_HAL_MIPS_VSR_TABLE_DEFINED

#endif

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
