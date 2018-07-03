#ifndef CYGONCE_DEVS_FLASH_SPI_RTL819X_H
#define CYGONCE_DEVS_FLASH_SPI_RTL819X_H

//=============================================================================
//
//      spi_flash.h
//
//      flash driver definitions
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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
// Author(s):   michael
// Date:        2011-12-07
//
//####DESCRIPTIONEND####
//
//=============================================================================

// The driver-specific data, pointed at by the priv field in a
// a cyg_flash_dev structure.

typedef struct cyg_rtl819x_spi_flash_dev
{
    cyg_flash_block_info_t      block_info[1];
    
} cyg_rtl819x_spi_flash_dev;

//========================================================================*/
// Exported function pointers.

__externC const struct cyg_flash_dev_funs cyg_rtl819x_spi_flash_funs;

//========================================================================*/

#endif // CYGONCE_DEVS_FLASH_SPI_RTL819X_H

