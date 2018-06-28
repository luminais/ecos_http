//==========================================================================
//
//      devs/watchdog/mips/rtl819x/watchdog_rtl819x.cxx
//
//      Watchdog implementation for rtl819x
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
// Author(s):    michael
// Contributors: 
// Date:         2010-07-26
// Purpose:      Watchdog class implementation
// Description:  Contains an implementation of the Watchdog class for use
//               with the rtl819x watchdog timer.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>             // system configuration file
#include <pkgconf/watchdog.h>           // configuration for this package
#include <pkgconf/kernel.h>             // kernel config

#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/kernel/instrmnt.h>        // instrumentation

#include <cyg/hal/hal_io.h>             // IO register access

#include <cyg/io/watchdog.hxx>          // watchdog API
#include <cyg/hal/bspchip.h>
// -------------------------------------------------------------------------
// Constructor

void
Cyg_Watchdog::init_hw(void)
{
    int wdtcnr_ovsel, ovsel_val;
    int ovsel = 8; /* 6~9 */  //8对应着 超期45秒左右
    
    CYG_REPORT_FUNCTION();

#ifdef CONFIG_RTL_819X_INTERNAL_TIMER
    REG32(BSP_CDBR)=(CYGNUM_HAL_RTC_DIV_FACTOR) << BSP_DIVF_OFFSET;
#endif

    if (ovsel == 6) {
    	ovsel_val = 2*1024*1024; //0110: 2**21
    	wdtcnr_ovsel = (0x01 << 17) | (0x02 << 21); //OVSEL[3:2], OVSEL[1:0]
    }
    else if (ovsel == 7) {
    	ovsel_val = 4*1024*1024; //0111: 2**22
    	wdtcnr_ovsel = (0x01 << 17) | (0x03 << 21); //OVSEL[3:2], OVSEL[1:0]
    }
    else if (ovsel == 8) {
    	ovsel_val = 8*1024*1024; //1000: 2**23
    	wdtcnr_ovsel = (0x02 << 17) | (0x00 << 21); //OVSEL[3:2], OVSEL[1:0]
    }
    else {
    	ovsel_val = 16*1024*1024; //1001: 2**24
    	wdtcnr_ovsel = (0x02 << 17) | (0x01 << 21); //OVSEL[3:2], OVSEL[1:0]
    }
    REG32(BSP_WDTCNR) = (0xA5<<24) | (1 << 23) | (1 << 20) | wdtcnr_ovsel;
    //time interval allowed between resets before watchdog triggers, in nanoseconds
    resolution = (1000000000LL/(200000000 / CYGNUM_HAL_RTC_DIV_FACTOR))*ovsel_val;

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Start the watchdog running.

void
Cyg_Watchdog::start()
{
    CYG_REPORT_FUNCTION();

    REG32(BSP_WDTCNR) &= (0x00ffffff);
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Reset watchdog timer. This needs to be called regularly to prevent
// the watchdog firing.

void
Cyg_Watchdog::reset()
{    
    CYG_REPORT_FUNCTION();

    REG32(BSP_WDTCNR) |= (1 << 23); //WDTCLR
    
    CYG_REPORT_RETURN();
}
// -------------------------------------------------------------------------
// EOF watchdog_rtl819x.cxx
