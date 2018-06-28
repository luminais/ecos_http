//=============================================================================
//
//      spi_flash.c
//
//      rtl819x SPI flash driver
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
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>

#include <cyg/io/flash.h>
#include <cyg/io/flash_dev.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_if.h>
#include <string.h>
#include <cyg/io/spi_common.h>
#include <cyg/io/spi_flash.h>

#include CYGHWR_MEMORY_LAYOUT_H

// ----------------------------------------------------------------------------

//typedef cyg_uint16 RTL819X_TYPE;

#if 0
#define RTL819X_INTSCACHE_STATE     int _saved_ints_
#define RTL819X_INTSCACHE_BEGIN()   HAL_DISABLE_INTERRUPTS(_saved_ints_)
#define RTL819X_INTSCACHE_END()     HAL_RESTORE_INTERRUPTS(_saved_ints_)
#else
#define RTL819X_INTSCACHE_STATE     do {} while(0)
#define RTL819X_INTSCACHE_BEGIN()   do {} while(0)
#define RTL819X_INTSCACHE_END()     do {} while(0)
#endif

//#define RTL819X_UNCACHED_ADDRESS(__x) ((RTL819X_TYPE *)(__x))

// ----------------------------------------------------------------------------
// Forward declarations for functions that need to be placed in RAM:

static int rtl819x_spi_flash_hw_erase(cyg_flashaddr_t addr_offset) __attribute__((section (".2ram.rtl819x_spi_flash_hw_erase")));
static int rtl819x_spi_flash_hw_program(unsigned int flash_addr_offset, unsigned char *src, unsigned int len) __attribute__((section (".2ram.rtl819x_spi_flash_hw_program")));

// ----------------------------------------------------------------------------
// Diagnostic routines.

#if 0
#define rtkf_diag( __fmt, ... ) diag_printf("STF: %20s[%3d]: " __fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__ );
#else
#define rtkf_diag( __fmt, ... )
#endif

// ----------------------------------------------------------------------------

#define MTD_SPI_DEBUG 0

#if MTD_SPI_DEBUG
	#define KDEBUG(args...) diag_printf(args)
#else
	#define KDEBUG(args...)
#endif

#define NDEBUG(args...) diag_printf(args)

#define CONFIG_FLASH_NUMBER 0x1

extern struct spi_flash_type	spi_flash_info[];

// SPI Flash Init
/*
void spi_pio_init_8198(void)
{
	KDEBUG("spi_pio_init: rstSPIFlash(0)");
	rstSPIFlash(0);
}
void spi_pio_init(void)
{
	KDEBUG("spi_pio_init: rstSPIFlash(0)");
	rstSPIFlash(0);
}
*/

// SPI Flash Probe
void spi_probe(void)
{
	int i;

	KDEBUG("spi_probe: spi_regist(0, 1)\n");
	for(i=0;i<CONFIG_FLASH_NUMBER;i++)
	{
		spi_regist(i);
	}
}

/*
//SPI Flash Erase Sector
unsigned int spi_sector_erase(unsigned int uiChip, unsigned int uiAddr)
{
	KDEBUG("spi_sector_erase: uiChip=%x; uiAddr=%x\n", uiChip, uiAddr);
	return spi_flash_info[uiChip].pfErase(uiChip, uiAddr);
}

// SPI Flash Erase Block
unsigned int spi_block_erase(unsigned int uiChip, unsigned int uiAddr)
{
	KDEBUG("spi_block_erase: uiChip=%x; uiAddr=%x\n", uiChip, uiAddr);
	unsigned int uiRet;
	uiRet = ComSrlCmd_BE(uiChip, uiAddr);
	return uiRet;
}

// Erase whole chip
unsigned int spi_erase_chip(unsigned int uiChip)
{
	unsigned int uiRet;
	// Spansion
	KDEBUG("spi_erase_chip: uiChip=%x\n", uiChip);
	uiRet = ComSrlCmd_CE(uiChip);
	return uiRet;
}
*/

// print unsigned char
/*
void prnUChar(char* pcName, unsigned char* pucBuffer, unsigned int uiLen)
{
	int i;
	unsigned char* puc;
	puc = pucBuffer;
	NDEBUG("%s", pcName);
	for (i = 0; i< uiLen; i++)
	{
		NDEBUG("%2x ",*puc);
		puc+=1;
	}
	NDEBUG("\n");
}
*/
/************************************ for old interface ************************************/
/*
unsigned int spi_read(unsigned int uiChip, unsigned int uiAddr, unsigned int* puiDataOut)
{
	KDEBUG("spi_read: uiChip=%x; uiAddr=%x; uiLen=4; puiDataOut=%x\n", uiChip, uiAddr, (unsigned long)puiDataOut);
	return spi_flash_info[uiChip].pfRead(uiChip, uiAddr, 4, (unsigned char*)puiDataOut);
}

int flashread (unsigned long dst, unsigned int src, unsigned long length)
{

	KDEBUG("flashread: chip(uiChip)=%d; dst(pucBuffer)=%x; src(uiAddr)=%x; length=%x\n", uiChip, dst, src, length);
	return spi_flash_info[0].pfRead(0, src, length, (unsigned char*)dst);
}

int flashwrite(unsigned long dst, unsigned long src, unsigned long length)
{

	KDEBUG("flashwrite: dst(uiAddr)=%x; src(pucBuffer)=%x; length=%x; \n", dst, src, length);
	return spi_flash_info[0].pfWrite(0, dst, length, (unsigned char*)src);
}

int spi_flw_image(unsigned int chip, unsigned int flash_addr_offset ,unsigned char *image_addr, unsigned int image_size)
{
	KDEBUG("spi_flw_image: chip=%x; flash_addr_offset=%x; image_addr=%x; image_size=%x\n", chip, flash_addr_offset, (unsigned int)image_addr, image_size);
	return spi_flash_info[chip].pfWrite(chip, flash_addr_offset, image_size, image_addr);
}
int spi_flw_image_mio_8198(unsigned int cnt, unsigned int flash_addr_offset , unsigned char *image_addr, unsigned int image_size)
{
	KDEBUG("spi_flw_image_mio_8198: cnt=%x; flash_addr_offset=%x; image_addr=%x; image_size=%x\n", cnt, flash_addr_offset, (unsigned int)image_addr, 	image_size);
	return spi_flash_info[cnt].pfWrite(cnt, flash_addr_offset, image_size, image_addr);
}
*/
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Initialize the flash.

static int
rtl819x_spi_flash_init(struct cyg_flash_dev* dev)
{
    cyg_rtl819x_spi_flash_dev *rtl819x_dev = (cyg_rtl819x_spi_flash_dev *)dev->priv;
    cyg_uint32 flash_size, block_size = 0;
    
    spi_probe();
    // Set up the block info entries.
    dev->block_info = &rtl819x_dev->block_info[0];
    dev->num_block_infos = 1;

    block_size = spi_flash_info[0].sector_size;
    flash_size = spi_flash_info[0].sector_cnt * block_size;
    
    rtl819x_dev->block_info[0].blocks = spi_flash_info[0].sector_cnt;
    rtl819x_dev->block_info[0].block_size = block_size;
    
    // Set end address
    dev->end = dev->start + flash_size - 1;

    rtkf_diag("block_size %d size %08x end %08x\n", block_size, flash_size, dev->end);
    return CYG_FLASH_ERR_OK;
}

// ----------------------------------------------------------------------------

static size_t
rtl819x_spi_flash_query(struct cyg_flash_dev* dev, void* data, size_t len)
{
    static char query[] = "rtl819x spi flash";
    memcpy( data, query, sizeof(query));
    return sizeof(query);
}

// ----------------------------------------------------------------------------
// Get info about the current block, i.e. base and size.
static void
rtl819x_spi_flash_get_block_info(struct cyg_flash_dev* dev, const cyg_flashaddr_t addr, cyg_flashaddr_t* block_start, size_t* block_size)
{
    cyg_uint32      i;
    size_t          offset  = addr - dev->start;
    cyg_flashaddr_t result;

    result  = dev->start;
    
    for (i = 0; i < dev->num_block_infos; i++) {
        if (offset < (dev->block_info[i].blocks * dev->block_info[i].block_size)) {
            offset         -= (offset % dev->block_info[i].block_size);
            *block_start    = result + offset;
            *block_size     = dev->block_info[i].block_size;
            return;
        }
        result  += (dev->block_info[i].blocks * dev->block_info[i].block_size);
        offset  -= (dev->block_info[i].blocks * dev->block_info[i].block_size);
    }
    CYG_FAIL("Address out of range of selected flash device");
}

//-----------------------------------------------------------------------------
// Read back an arbitrary amount of data from flash.
/*
static int 
rtl819x_spi_flash_read(struct cyg_flash_dev *dev, const cyg_flashaddr_t base, void* data, size_t len)
{
	CYG_CHECK_DATA_PTR(dev, "valid flash device pointer required");
	CYG_ASSERT((base >= dev->start) && (base <= dev->end), "flash address out of device range");
	
	flashread((unsigned long)data, (unsigned int)base, (unsigned long)len);
	return FLASH_ERR_OK;
}
*/
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Erase a single sector. There is no API support for chip-erase. The
// generic code operates one sector at a time, invoking the driver for
// each sector, so there is no opportunity inside the driver for
// erasing multiple sectors in a single call. The address argument
// points at the start of the sector.

static int
rtl819x_spi_flash_hw_erase(cyg_flashaddr_t addr_offset)
{
	//rtkf_diag("rtl819x_spi_flash_hw_erase: addr_offset=%x\n", addr_offset);
	spi_flash_info[0].pfErase(0, addr_offset);
	return CYG_FLASH_ERR_OK;
}

// ----------------------------------------------------------------------------
// Write data to flash, using individual word writes. The destination
// address will be aligned in a way suitable for the bus. The source
// address need not be aligned. The count is in RTL819X_TYPE's, not in
// bytes.
static int
rtl819x_spi_flash_hw_program(unsigned int flash_addr_offset, unsigned char *src, unsigned int len)
{
	//rtkf_diag("flashwrite: flash_addr_offset=%x src=%p len=%x\n", flash_addr_offset, src, len);
	spi_flash_info[0].pfWrite(0, flash_addr_offset, len, src, 1); //assume sectors have been erased
	return CYG_FLASH_ERR_OK;
}

// ----------------------------------------------------------------------------
// Erase a single block. The calling code will have supplied a pointer
// aligned to a block boundary.

static int
rtl819x_spi_flash_erase(struct cyg_flash_dev* dev, cyg_flashaddr_t dest)
{
    int                     (*erase_fn)(cyg_uint32);
//    volatile RTL819X_TYPE*    uncached;
    cyg_flashaddr_t         block_start = 0;
    size_t                  block_size;
    int                     result;

    RTL819X_INTSCACHE_STATE;

    rtkf_diag("dest %p\n", dest);
    CYG_CHECK_DATA_PTR(dev, "valid flash device pointer required");
    CYG_ASSERT((dest >= dev->start) && (dest <= dev->end), "flash address out of device range");

    rtl819x_spi_flash_get_block_info(dev, dest, &block_start, &block_size);
    CYG_ASSERT(dest == block_start, "erase address should be the start of a flash block");

//    uncached    = RTL819X_UNCACHED_ADDRESS(dest);
    erase_fn = (int (*)(cyg_uint32)) cyg_flash_anonymizer(&rtl819x_spi_flash_hw_erase);

    RTL819X_INTSCACHE_BEGIN();    
    result = (*erase_fn)(block_start-dev->start);
    RTL819X_INTSCACHE_END();
    
    return result;
}

// ----------------------------------------------------------------------------
// Write some data to the flash. The destination must be aligned to a
// 16 bit boundary. Higher level code guarantees that the data will
// not straddle a block boundary.

int
rtl819x_spi_flash_program(struct cyg_flash_dev* dev, cyg_flashaddr_t dest, const void* src, size_t len)
{
    int (*program_fn)(unsigned int flash_addr_offset, unsigned char *src, unsigned int len);
//    volatile RTL819X_TYPE*    uncached; 
    int result;

    RTL819X_INTSCACHE_STATE;

    rtkf_diag("dest %p src %p len %d\n", dest, src, len);
    CYG_CHECK_DATA_PTR(dev, "valid flash device pointer required");
    CYG_ASSERT((dest >= dev->start) && ((CYG_ADDRESS)dest <= dev->end), "flash address out of device range");

    //destination must be 32-bit aligned.
    if (0 != ((CYG_ADDRESS)dest & 0x3))
        return CYG_FLASH_ERR_INVALID;

//    uncached    = RTL819X_UNCACHED_ADDRESS(dest);
    program_fn = (int (*)(unsigned int flash_addr_offset, unsigned char *src, unsigned int len)) cyg_flash_anonymizer(&rtl819x_spi_flash_hw_program);

    RTL819X_INTSCACHE_BEGIN();
    result = (*program_fn)((dest-dev->start), (unsigned char *)src, len);
    RTL819X_INTSCACHE_END();
    return result;
}

// ----------------------------------------------------------------------------
// Function table

const CYG_FLASH_FUNS(cyg_rtl819x_spi_flash_funs,
                     &rtl819x_spi_flash_init,
                     &rtl819x_spi_flash_query,
                     &rtl819x_spi_flash_erase,
                     &rtl819x_spi_flash_program,
                     NULL, //&rtl819x_spi_flash_read, //read
                     cyg_flash_devfn_lock_nop,
                     cyg_flash_devfn_unlock_nop);

// ----------------------------------------------------------------------------
// End of spi_flash.c
