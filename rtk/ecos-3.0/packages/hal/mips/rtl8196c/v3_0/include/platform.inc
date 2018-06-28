#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	rtl819x board assembler header file
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
## Author(s): 	dmoseley
## Contributors:	dmoseley
## Date:	2000-06-06
## Purpose:	rtl819x board definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the rtl819x board.
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/mips.inc>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/bspchip.h>
#include <cyg/hal/bspinit.h>

#define CONFIG_D16_16

#------------------------------------------------------------------------------
// Using register: t6, t7
#define REG32_W(addr,v)	li t6,v;\
			li t7,addr;\
			sw t6, 0(t7);

	.macro flush_dcache_invalidate_icache
	# force write back and invalidate dcache
	mfc0	t0, $20
	ori	t0, 0x200
	xori	t1, t0, 0x200
	mtc0	t1, $20
	mtc0	t0, $20
	
	# flush write buffer
	la	t0, 0xA0000000
	lw	$0, 0(t0)
	nop
	# invalidate icache
	mfc0	t0, $20
	ori	t0, 0x2
	xori	t1, t0, 0x2
	mtc0	t1, $20
	mtc0	t0, $20
	.endm
				
#------------------------------------------------------------------------------

#define CYGPKG_HAL_RESET_VECTOR_FIRST_CODE
	.macro hal_reset_vector_first_code
 	#move 	t0, zero
	#mtc0 	t0, status
	#nop
	.endm

#define CYGPKG_HAL_EARLY_INIT
	.macro	hal_early_init
	# Do these initializations early (rather than in hal_memc_init) so
	# we have access to the LEDs on the board for debugging purposes.
#if 1
	#REG32_W(0xb8000010, 0x279);	 //96C, Rev. A
	REG32_W(0xb8000010, 0x245);	 //96C, Rev. B
	REG32_W(0xb8000040, 0x40000);    //FCS1=CKE

        # DTR
        REG32_W(0xb8001008, 0x6cca0c80); //96C SDR 156MHZ
        
        # DCR
#if defined(CONFIG_D8_16)
	REG32_W(0xb8001004, 0x52080000);
#elif defined(CONFIG_D16_16)
	REG32_W(0xb8001004, 0x52480000);
#elif defined(CONFIG_D32_16)
	REG32_W(0xb8001004, 0x54480000);
#elif defined(CONFIG_D64_16)
	REG32_W(0xb8001004, 0x54880000);
#endif
#endif
	.endm

#------------------------------------------------------------------------------
# Interrupt decode macros

# Only INTN[0] is connected on the rtl819x board. We need to access the
# interrupt controller to get the actual vector number.

#ifndef CYGPKG_HAL_MIPS_INTC_INIT_DEFINED

	.macro	hal_intc_init
	mfc0	v0,status
	nop
	lui	v1,0xFFFF
	ori	v1,v1,0x04FF
	and	v0,v0,v1	# Clear the IntMask bits except IM[0]
	mtc0	v0,status
	nop
	nop
	nop
	.endm

#define CYGPKG_HAL_MIPS_INTC_INIT_DEFINED

#endif


#ifndef CYGPKG_HAL_MIPS_INTC_DECODE_DEFINED
	.macro	hal_intc_decode vnum

	mfc0	v1,status		# get status register (interrupt mask)
	mfc0	v0,cause		# get cause register
	and	v0,v0,v1		# apply interrupt mask
	andi	v1,v0,0xf800		# test INTN[1-5]
	bnez	v1,0f
	nop
	
	la	t1,0xb8003000		# GIMR
	lw	v1,0(t1)
	la	t0,0xb8003004		# GISR
	lw	v0,0(t0)
	nop
	and	v0,v0,v1
	la	v1,BSP_IRQ_ICTL_MASK
	and	v0,v0,v1
	la 	t0, 32
    2:
    	and	v1,v0,0x80000000
	sll	v0,v0,1
	bnez	v1,3f
	nop
	sub	t0, t0, 1
	bnez	t0,2b
	nop
	# unfound, spurious interrupt
#if 1
        la 	t0, 0x25            # ISR number 37, spurious interrupt
#else
    7:
        li      t0, 0xb8002000      # UART_THR
        li      t1,'e'		    #it shows e, that is not reasonable to come here.
        nop
        sb      t1, 0(t0)
     	nop
     	nop
	 		            #just hang here.
      	b	7b 
        nop
        nop
#endif
        
    3:	
	la	v0,0			# v0 = 0
	b	1f
	sub	\vnum,t0,v0		# vnum = t0 - 0 == vector number	
    0:
	srl	v0,v0,10		# shift interrupt bits down
	andi	v0,v0,0x3f		# isolate 6 interrupt bits
	la	v1,hal_intc_translation_table
	add	v0,v0,v1		# index into table
	lb	\vnum,0(v0)		# pick up vector number
	nop
    1:
	.endm

#ifndef CYGPKG_HAL_MIPS_INTC_TRANSLATE_DEFINED
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
	.macro	hal_intc_translate inum,vnum
	move	\vnum,zero			# Just vector zero is supported
	.endm
#else			
	.macro	hal_intc_translate inum,vnum
	move	\vnum,\inum			# Vector == interrupt number
	.endm
#endif
#endif

	.macro	hal_intc_decode_data
hal_intc_translation_table:
	.byte	32, 32, 32, 32
	.byte	33, 33, 33, 33
	.byte	34, 34, 34, 34
	.byte	34, 34, 34, 34
	.byte	35, 35, 35, 35
	.byte	35, 35, 35, 35
	.byte	35, 35, 35, 35
	.byte	35, 35, 35, 35
	.byte	36, 36, 36, 36
	.byte	36, 36, 36, 36
	.byte	36, 36, 36, 36
	.byte	36, 36, 36, 36
	.byte	36, 36, 36, 36
	.byte	36, 36, 36, 36
	.byte	36, 36, 36, 36
	.byte	36, 36, 36, 36
	.endm

#define CYGPKG_HAL_MIPS_INTC_DECODE_DEFINED
#define CYGPKG_HAL_MIPS_INTC_DEFINED

// We also define our own interrupt tables in platform.S...
#define CYG_HAL_MIPS_ISR_TABLES_DEFINED
			
#endif

#ifndef CYGPKG_HAL_MIPS_MEMC_DEFINED
	.macro	hal_memc_init
#if defined(CYG_HAL_STARTUP_RAM)
	_imem_dmem_init
#endif
#if defined(CYG_HAL_STARTUP_ROMRAM)
	# Having got the RAM working, we must now relocate the Entire
	# ROM into it and then continue execution from RAM.

	la	t0,0x80000000		# dest addr
	la	t1,0xbfc00000		# source addr
	la	t3,__ram_data_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	addi	t1,t1,4
	sw	v0,0(t0)		# write word
	addi	t0,t0,4
	bne	t0,t3,1b
	nop

	flush_dcache_invalidate_icache

	la	v0,2f			# RAM address to go to
	jr	v0
	nop
2:	
	# We are now executing out of RAM!
#endif
	.endm

#define CYGPKG_HAL_MIPS_MEMC_DEFINED
		
#endif

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
