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
// Date:        2010-05-21
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
static int rtl819x_spi_flash_hw_program(unsigned int flash_addr_offset , unsigned char *image_addr, unsigned int image_size) __attribute__((section (".2ram.rtl819x_spi_flash_hw_program")));

// ----------------------------------------------------------------------------
// Diagnostic routines.

#if 0
#define rtkf_diag( __fmt, ... ) diag_printf("STF: %20s[%3d]: " __fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__ );
#define rtkf_dump_buf( __addr, __size ) diag_dump_buf( __addr, __size )
#else
#define rtkf_diag( __fmt, ... )
#define rtkf_dump_buf( __addr, __size )
#endif

extern char _bootimg_start, _bootimg_end;

#define LENGTH(i)       SPI_LENGTH(i)
#define CS(i)           SPI_CS(i)
#define CS_8198(i)           SPI_CS_8198(i)
#define RD_ORDER(i)     SPI_RD_ORDER(i)
#define WR_ORDER(i)     SPI_WR_ORDER(i)
#define READY(i)        SPI_READY(i)
#define CLK_DIV(i)      SPI_CLK_DIV(i)
#define RD_MODE(i)      SPI_RD_MODE(i)
#define SFSIZE(i)       SPI_SFSIZE(i)
#define TCS(i)          SPI_TCS(i)
#define RD_OPT(i)       SPI_RD_OPT(i)
#define prom_printf diag_printf
#define dprintf diag_printf
#define printf dprintf
#define SPI_DUMMY 0xff

unsigned int get_ctimestamp( void );
void check_spi_clk_div(void);
extern int check_dram_freq_reg(void) ;
void set_spi_clk_div(unsigned int spi_clock_div_num);

                 //end of SST's SPI
void spi_sector_erase(unsigned int cnt,unsigned int i);
void EWSR(unsigned short cnt);
void WRSR(unsigned short cnt);

                 /*JSW@20091008: Add for 8196C/8198 New SPI Memory Controller*/
#ifdef SUPPORT_SPI_MIO_8198_8196C
void Set_QE_bit(unsigned short QE_bit, unsigned short cnt, unsigned int  SPI_ID);
unsigned long __spi_flash_preinit_8198(unsigned int read_data1 , unsigned int  cnt);
void spi_sector_erase_8198(unsigned int cnt, unsigned short i);
void auto_spi_memtest(unsigned long DRAM_starting_addr, unsigned int spi_clock_div_num);
void auto_spi_memtest_8198(unsigned long DRAM_starting_addr, unsigned int spi_clock_div_num);
void spi_pio_init_8198(void);
                 /*JSW@20100929: Add SST SPI Flash support for 8196C and 8198*/
void EWSR_8198(unsigned short cnt);
void WRSR_8198(unsigned short cnt);
#endif

void sst_spi_write(unsigned int cnt, unsigned int address, unsigned char data_in);

extern unsigned int rand2(void);

#if 0   //michael
int flashread (unsigned long dst, unsigned int src, unsigned long length)
{
    Int16 tmp=0;
    if (length==0)
        return 1;
                 /* It is the first byte              */
    if (src & 0x01)                                  /* FLASH should be 16Bit alignment   */
    {
                 /* FLASH 16bit alignment             */
        tmp = rtl_inw(FLASH_BASE + src - 1);         /* FLASH 16bit access                */
        tmp = tmp & 0x00ff;
        *(Int8*)(dst) = (Int8)tmp;                   /* Byte access SDRAM                 */

                 //dprintf("%X,%X,%X,%X\n",dst,src,length,*(Int8*)(dst));

        dst=dst+1;                                   /* Finish the First Byte             */
        src=src+1;                                   /* Finish the First Byte             */
        length=length-1;                             /* Finish the First Byte             */
    }

    while(length)
    {
        if(length == 1)
        {
            tmp = rtl_inw(FLASH_BASE + src);
            *(Int8*)(dst)=(Int8)( ((tmp >> 8) & 0x00ff) );
                 //  dprintf("%X,%X,%X,%X\n",dst,src,length,*(Int8*)(dst));
            break;
        }

        tmp=rtl_inw(FLASH_BASE + src);               // From now on, src 16Bit aligned
                 // FLASH 16bit access
                 //if(src&0x01)
                 //dprintf("error");
                 //if(length<2)
                 //dprintf("error");
        memcpy((Int8*)dst,&tmp,2);                   //use memcpy, avoid SDRAM alignement

                 //dprintf("%X,%X,%X,%X\n",dst,src,length,*(unsigned short*)(dst));

        dst=dst+2;
        src=src+2;
        length=length-2;
    }
    return 1;
}
#endif

                 /*
                  *  :SPI Flash Info  @20080613
                  */
                 /*
                  * JSW :SPI Flash Info  @20080805
                  * Note1:SST just only has single-byte program,but SST25VF064C(8MB/80MHZ) gets page-program just like MXIC
                  */
const struct spi_flash_db   spi_flash_known[] =
{
                 /*  List of supported SingleI/O chip    */
                 /*  Verified OK*/
    {                                                /* MXIC(50MHZ): MX25L3235D/MX25L3205D(4MB),MX25L1635D/MX25L1605D(2MB) */
        0xC2, 0x20,   0
    },
    {                                                /* SST(50MHZ): SST25VF016B(2MB)/SST25VF032B(4MB)/SST25VF064C(8MB/80MHZ) */
        0xBF, 0x25,   0
    },
    {                                                /* Spansion(104MHZ):S25FL064P(8MB)*/
        0x01, 0x02,   1
    },
    {                                                /* Spansion(104MHZ):S25FL128P(16MB)*/
        0x01, 0x20,   0
    },

                 /*  List of supported Multi I/O chip    */
                 /*  Verified OK*/
    {                                                /* MXIC(104MHZ):MX25L3235D(4MB) */
        0xC2, 0x5e,   0
    },
    {                                                /* MXIC(104MHZ):MX25L1635D(2MB) */
        0xC2, 0x24,   0
    },
    {                                                /* MXIC(133MHZ):MX25L1636E(2MB) */
        0xC2, 0x25,   0
    },
    {                                                /* WindBond(80MHZ):W25Q16(2MB)/W25Q32(4MB)/W25Q64(8MB)*/
        0xEF, 0x40,   0
    },
    {                                                /*  EON(100MHZ/Sector4KB) :EN25F32(4MB)/:EN25F16(2MB) */
        0x1C, 0x31,   0
    },
                 /*  Multi I/O chip    */
                 /*  Not Verified yet*/
    {                                                /*  SST*/
        0xBF, 0x26,   0
    },
    {                                                /*  EON(100MHZ/Sector64KB) :EN25P64(8MB) */
        0x1C, 0x20,   0
    },
    {                                                /*  EON(100MHZ/Sector64KB) :EN25P64(8MB) */
        0x1F, 0x47,   0
    },

};

//#if 1
//static unsigned int spi_flash_total_size;
//static unsigned int spi_flash_num_of_chips = 0;
//#endif

                 /*
                  * SPI Flash Info  //3
                  */
struct spi_flash_type   spi_flash_info[5];           //3

                 /*
                  * SPI Flash APIs
                  */

                 /*
                  * This function shall be called when switching from MMIO to PIO mode
                  */
void spi_pio_init(void)
{
    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}


                 //unsigned short FAST_READ=0; //1:enable,0:disable ,global variable for spi_read command
#define SPI_DUMMY 0xff

#if 0   //michael
                 //JSW add :Sector Erase
void spi_sector_erase(unsigned int cnt,unsigned int i)
{

                 /* WREN Command */
    spi_ready();
                 //*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x06 << 24;
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

                 /* SE Command */
    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);

                 /*JSW@20090714 Note:
                  Instruction code :
                 =======================
                 For Old 50MHZ SPI:
                 Sector Erase(20h)~
                 1.MXIC: 4KB
                 2.Spansion : Not support
                 3.SST: 4KB

                 SE(D8h)~
                 1.MXIC: 64KB
                 2.Spansion : 64KB
                 3.SST:  64KB

                 ========================
                 For New 104MHZ SPI:
                 Sector Erase(20h)~
                 1.MXIC: 4KB
                 2.Spansion : 4KB
                 3.SST:  4KB

                 SE(D8h)~
                 1.MXIC: 64KB
                 2.Spansion : 64KB
                 3.SST: 64KB

                 */
#ifdef Set_SECTOR_ERASE_64KB
    *(volatile unsigned int *) SFDR = ( 0xD8 << 24) | (i * 65536);
#endif

#ifdef Set_SECTOR_ERASE_4KB
    *(volatile unsigned int *) SFDR = ( 0x20 << 24) | (i * 4096);
#endif

    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

                 /* RDSR Command */
    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x05 << 24;

    while (1)
    {
                 /* RDSR Command */
        if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
            break;
    }

    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

#if 1
#ifdef Set_SECTOR_ERASE_64KB
    printf("Erase Sector:%d->%d,Addr:0x%x->0x%x\n" ,i,\
        i+1,(0xbd000000+(i*0x10000)),(0xbd000000+(((i+1)*0x10000))-1));
#endif

#ifdef Set_SECTOR_ERASE_4KB
    printf("Erase Sector:%d->%d,Addr:0x%x->0x%x\n" ,i,\
        i+1,(0xbd000000+(i*0x1000)),(0xbd000000+(((i+1)*0x1000))-1));
#endif
#else
    prom_printf(".");
#endif
}
#endif

void spi_ready(void)
{
    while (1)
    {
        if ( (*(volatile unsigned int *) SFCSR) & READY(1))
            break;
    }
}


unsigned short SST_1Byte_SPI_Flash=0;                //20090903 added:"1"=SST single-byte,"0"=SST 4-byte style SPI
void spi_probe(void)
{
    unsigned int cnt, i;
    //unsigned int temp;

    //dprintf("%s:0xb8001204=%x\n", __FUNCTION__, READ_MEM32(0xb8001204) );
    for (cnt = 0; cnt < 1; cnt++) 
    {

#ifdef SUPPORT_SPI_MIO_8198_8196C

        CHECK_READY;
        SPI_REG(SFCSR_8198) = SPI_CS_INIT;           //deactive CS0, CS1
        CHECK_READY;
        SPI_REG(SFCSR_8198) = 0;                     //active CS0,CS1
        CHECK_READY;
        SPI_REG(SFCSR_8198) = SPI_CS_INIT;           //deactive CS0, CS1
        CHECK_READY;
        unsigned int read_data;
#if 1
                 // Read Flash ID, JSW@20090304: only for 8196C/8198 new design
        SPI_REG(SFCSR_8198) = (CS_8198(cnt) | SFCSR_LEN(3) | SFCSR_CMD_BYTE(0x9f));
        read_data = (SPI_REG(SFDR2_8198) >> 8);
        *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);
                 // *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) ;

                 dprintf("RDID data =%x\n",  read_data );

#else
                 /* JSW@20100528:New safe RDID Command for probe some low-speed SPI RDID  */
        *(volatile unsigned int *) SFCSR_8198 = SFCSR_LEN(3) | CS_8198(cnt) | READY(1);
        *(volatile unsigned int *) SFDR_8198 = 0x9F << 24;
        read_data = (SPI_REG(SFDR_8198) >> 8);

                 //dprintf("RDID data =%x\n",  read_data );
#endif

                 //JSW:Pre-init 8198 SPI-controller for Set Quad Mode and QE bit
        __spi_flash_preinit_8198(read_data, cnt);

        spi_flash_info[cnt].maker_id = (read_data >> 16) & 0xFF;
        spi_flash_info[cnt].type_id = (read_data >> 8) & 0xFF;
        spi_flash_info[cnt].capacity_id = (read_data) & 0xFF;
        CHECK_READY;

#else                                        //JSW:for old 8196B SPI

                 /* Here set the default setting */
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1 + cnt) | READY(1);

                 /* One More Toggle (Necessary) */
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
                 /*20081002: JSW  For spi_probe OK, add spi_ready() */
        spi_ready();
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1 + cnt) | READY(1);
        spi_ready();

                 /* RDID Command */
        *(volatile unsigned int *) SFDR = 0x9F << 24;
        *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1 + cnt) | READY(1);

#if 1//SPI_DBG_MESSAGE
        dprintf("\n===========(cnt=%d)===========\n", cnt);
        dprintf("8196_SFCR(b8001200) =%x\n", *(volatile unsigned int *) SFCR);
        dprintf("8196_SFCSR(b8001204) =%x\n", *(volatile unsigned int *) SFCSR);
#endif

        temp = *(volatile unsigned int *) SFDR;

        dprintf("temp =%x\n", temp);

        spi_flash_info[cnt].maker_id = (temp >> 24) & 0xFF;
        spi_flash_info[cnt].type_id = (temp >> 16) & 0xFF;
        spi_flash_info[cnt].capacity_id = (temp >> 8) & 0xFF;
        spi_ready();
        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
#endif

                 /* Iterate Each Maker ID/Type ID Pair */
        for (i = 0; i < sizeof(spi_flash_known) / sizeof(struct spi_flash_db); i++)
        {

            if ((spi_flash_info[cnt].maker_id == spi_flash_known[i].maker_id) &&
                (spi_flash_info[cnt].type_id == spi_flash_known[i].type_id))
            {
#ifdef SPI_DBG_MESSAGE
                dprintf("\n=============================\n");
                dprintf("<Probe SPI #%d >\n", cnt);
                dprintf("1.Manufacture ID=0x%x\n2.Device ID=0x%x\n3.Capacity ID=0x%x\n", spi_flash_info[cnt].maker_id, spi_flash_info[cnt].type_id, spi_flash_info[cnt].capacity_id);
#endif

                switch (spi_flash_info[cnt].capacity_id)
                {

                    case 0x8d:
#ifdef SPI_DBG_MESSAGE
                        dprintf("4.SST SPI (0.5 MByte)!\n");
#endif
                 //JSW: Modify SST size_shift for calculation
                        spi_flash_info[cnt].capacity_id -= 122;

                 //All SST product need to do this first : Free lock-area in advance
                 /*JSW@20100929: Add SST SPI Flash support for 8196C and 8198*/
#ifdef SUPPORT_SPI_MIO_8198_8196C
                        EWSR_8198(cnt);
                        WRSR_8198(cnt);
#else
                        EWSR(cnt);
                        WRSR(cnt);
#endif
			   SST_1Byte_SPI_Flash=1;       //It's 1-Byte page program style SPI
                 //RDSR(cnt);
                        break;

                    case 0x8e:
#ifdef SPI_DBG_MESSAGE
                        dprintf("4.SST SPI (1 MByte)!\n");
#endif
                 //JSW: Modify SST size_shift for convenience of calculation later
                        spi_flash_info[cnt].capacity_id -= 122;

                 //All SST product need to do this first : Free lock-area in advance
                 /*JSW@20100929: Add SST SPI Flash support for 8196C and 8198*/
#ifdef SUPPORT_SPI_MIO_8198_8196C
                        EWSR_8198(cnt);
                        WRSR_8198(cnt);
#else
                        EWSR(cnt);
                        WRSR(cnt);
#endif
                 // RDSR(cnt);
                  SST_1Byte_SPI_Flash=1;       //It's 1-Byte page program style SPI
                        break;

                    case 0x4b:
#ifdef SPI_DBG_MESSAGE
                        dprintf("4.SST SPI (8 MByte)!\n");
#endif

                        spi_flash_info[cnt].capacity_id -= 52;
                 /*JSW@20100929: Add SST SPI Flash support for 8196C and 8198*/
#ifdef SUPPORT_SPI_MIO_8198_8196C
                        EWSR_8198(cnt);
                        WRSR_8198(cnt);
#else
                        EWSR(cnt);
                        WRSR(cnt);
#endif
                        SST_1Byte_SPI_Flash=0;       //It's 4-Byte page program style SPI
                 //RDSR(cnt);
                        break;

                    case 0x4a:
#ifdef SPI_DBG_MESSAGE
                        dprintf("4.SST SPI (4 MByte)!\n");
#endif

                        spi_flash_info[cnt].capacity_id -= 52;

#ifdef SUPPORT_SPI_MIO_8198_8196C
                        EWSR_8198(cnt);
                        WRSR_8198(cnt);
#else
                        EWSR(cnt);
                        WRSR(cnt);
#endif
 			   SST_1Byte_SPI_Flash=1;       //It's 1-Byte page program style SPI
                 //RDSR(cnt);
                        break;

                    case 0x41:
#ifdef SPI_DBG_MESSAGE
                        dprintf("4.SST SPI (2 MByte)!\n");
#endif
                 //JSW: Modify  SST size_shift for convenience of calculation later
                        spi_flash_info[cnt].capacity_id -= 44;

                 //All SST product need to do this first : Free lock-area in advance
                 /*JSW@20100929: Add SST SPI Flash support for 8196C and 8198*/
#ifdef SUPPORT_SPI_MIO_8198_8196C
                        EWSR_8198(cnt);
                        WRSR_8198(cnt);
#else
                        EWSR(cnt);
                        WRSR(cnt);
#endif
			   SST_1Byte_SPI_Flash=1;       //It's 1-Byte page program style SPI
                 //RDSR(cnt);
                        break;

                    case 0x20:
#ifdef SPI_DBG_MESSAGE
                        dprintf("4.MXIC SPI (4 MByte)!\n");
#endif
                        break;

                    case 0x18:
#ifdef SPI_DBG_MESSAGE
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            dprintf("4.MXIC SPI (16 MByte)!\n");
                        }
                        if (spi_flash_info[cnt].maker_id == 0x01)
                        {
                            dprintf("4.Spansion SPI (16 MByte)!\n");
                        }
#endif
                        break;

                    case 0x17:
#ifdef SPI_DBG_MESSAGE
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            dprintf("4.MXIC SPI (8 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0xef)
                        {
                            dprintf("4.Winbond SPI (8 MByte)!\n");
                        }
                 //EON
                        else if (spi_flash_info[cnt].maker_id == 0x1c)
                        {
                            dprintf("4.EON SPI (8 MByte)!\n");
                        }
#endif
                        break;

                    case 0x16:
#ifdef SPI_DBG_MESSAGE
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            dprintf("4.MXIC SPI (4 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0x01)
                        {
                            dprintf("4.Spansion SPI (8 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0xef)
                        {
                            dprintf("4.Winbond SPI (4 MByte)!\n");
                        }
                 //EON
                        else if (spi_flash_info[cnt].maker_id == 0x1c)
                        {
                            dprintf("4.EON SPI (4 MByte)!\n");
                        }
#endif
                        break;

                    case 0x15:
#ifdef SPI_DBG_MESSAGE
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            dprintf("4.MXIC SPI (2 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0x01)
                        {
                            dprintf("4.Spansion SPI (4 MByte)!\n");
                        }
                        else if (spi_flash_info[cnt].maker_id == 0xef)
                        {
                            dprintf("4.Winbond SPI (2 MByte)!\n");
                        }
                 //EON
                        else if (spi_flash_info[cnt].maker_id == 0x1c)
                        {
                            dprintf("4.EON SPI (2 MByte)!\n");
                        }
#endif

                        break;

                    case 0x14:
#ifdef SPI_DBG_MESSAGE
                        if (spi_flash_info[cnt].maker_id == 0xc2)
                        {
                            dprintf("4.MXIC SPI (1 MByte)!\n");
                        }
                        else
                        {
                            dprintf("4.Spansion SPI (2 MByte)!\n");
                        }
#endif

                        break;

                    case 0x13:
#ifdef SPI_DBG_MESSAGE
                        dprintf("4.Spansion SPI (1 MByte)!\n");
#endif

                        break;

                    case 0x12:
#ifdef SPI_DBG_MESSAGE
                        dprintf("4.Spansion SPI (0.5 MByte)!\n");
#endif

                        break;

                    case 0x01:
#ifdef SPI_DBG_MESSAGE
                        dprintf("4.ATMEL SPI (4 MByte)!\n");
#endif

                        break;

                    default :
                        dprintf("4.Unknow type/size!\n");
                        break;
                }

                spi_flash_info[cnt].device_size = (unsigned char)((signed char)spi_flash_info[cnt].capacity_id + spi_flash_known[i].size_shift);

#ifdef Set_SECTOR_ERASE_4KB
                spi_flash_info[cnt].sector_cnt = (1 << (spi_flash_info[cnt].device_size - 16)) * 16;
#ifdef SPI_DBG_MESSAGE
                dprintf("5.Total sector-counts = %d(sector=4KB)\n", spi_flash_info[cnt].sector_cnt);
#endif
#else
                spi_flash_info[cnt].sector_cnt = (1 << (spi_flash_info[cnt].device_size - 16));
#ifdef SPI_DBG_MESSAGE
                dprintf("5.Total sector-counts = %d(sector=64KB)\n", spi_flash_info[cnt].sector_cnt);
#endif
#endif

#ifdef SPI_DBG_MESSAGE
                dprintf("6.spi_flash_info[%d].device_size = %d\n", cnt, spi_flash_info[cnt].device_size);
#endif

                 //Set 8196C and 8198 SPI flash size and parameter
#if defined  (SUPPORT_SPI_MIO_8198_8196C)
                 //Set SPI Flash size
                 //REG32(SFCR2_8198) &= 0xFF0FFFFF; //RD_OPT=0
                REG32(SFCR2_8198) &= 0xFF1FFFFF;     //RD_OPT=1
                REG32(SFCR2_8198) |= SPI_SFSIZE_8198(spi_flash_info[0].device_size - 17) ;

                 /*
                  * SPI_CLK_DIV(0) =DRAM clock/2
                  * SPI_CLK_DIV(1) =DRAM clock/4
                  * SPI_CLK_DIV(2) =DRAM clock/6
                  * ...
                  */

                 /*When using older SPI flash that < 40MHZ ,please define "CONFIG_AUTO_PROBE_LIMITED_SPI_CLK_UNDER_40MHZ" in menuconfig*/

#if defined (CONFIG_BOOT_SIO_8198 )
                 //JSW:DDR 200/2=100MHZ will over SPI Spec,so we divide by 4 here
	#if defined (CONFIG_DDR_SDRAM) || defined (CONFIG_DDR2_SDRAM)

	                WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(2) | SPI_TCS_8198(31));

	#elif defined (CONFIG_SDRAM)         //Set SPI clock divide Mem_clock by 4 for SDRAM
	                WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(2) | SPI_TCS_8198(31));
#endif

                 /*Auto Probe Low-speed SPI Flash and Set clock < 40MHZ 	*/

                 /*List table for MXIC Low-speed SPI flash:Add your older SPI Flash ID below*/
#ifdef CONFIG_AUTO_PROBE_LIMITED_SPI_CLK_UNDER_40MHZ
                 //#define MX25L0805D  0x00C22014   /*MXIC 1MB*/
                 //#define MX25L1605D  0x00C22015   /*MXIC 2MB*/
                 //#define MX25L1636E  0x00C22515   /*MXIC 2MB*/
                 //#define MX25L3205D  0x00C22016   /*MXIC 4MB*/
                 //#define MX25L3206E  0x00C22016   /*MXIC 4MB*/
                 //#define MX25L6405D  0x00C22017   /*MXIC 8MB*/
                 //#define MX25L12805D  0x00C22018   /*MXIC 16MB*/
                 

                if (((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x14)
//                    || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x15) //michael removed
//                    || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x16) //michael removed
                    || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x17)
                    || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x18)
                    || (spi_flash_info[0].type_id==0x25 )) //Using Single-Byte Burning for All SST SPI Flash 
                {
#ifdef SPI_DBG_MESSAGE
                    dprintf("spi_flash.c: Set SPI clock < 40MHZ for low-speed SPI Flash\n");
#endif
                    WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(7) | SPI_TCS_8198(31));

#ifdef SPI_DBG_MESSAGE
                    dprintf("spi_flash.c: Set No dummy cycle\n");
#endif
                    REG32(SFCR2_8198)=READ_MEM32(SFCR2_8198)&0xFFFF1FFF;
		}


		

		

                 //#define W25Q64      0x00EF4017  /*Winbond 8MB*/
                if (((spi_flash_info[0].maker_id == 0xef) && (spi_flash_info[0].type_id == 0x40)&&(spi_flash_info[0].capacity_id) == 0x17))
                {
#ifdef SPI_DBG_MESSAGE
                    dprintf("spi_flash.c: Set SPI clock = 80MHZ for Winbond 8MB SPI Flash\n");
#endif
                    WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(0) | SPI_TCS_8198(31));

                }
#endif        //end of CONFIG_AUTO_PROBE_LIMITED_SPI_CLK_UNDER_40MHZ

                 /*For DIO and QIO*/
#elif defined(CONFIG_BOOT_DIO_8198) ||defined(CONFIG_BOOT_QIO_8198)
                 //Set SPI clock divide Mem_clock by 4
               // WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(2) | SPI_TCS_8198(31));
		//JSW@20110401 , Set Quad IO Read clock close to 75MHZ
	  	   WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) | SPI_TCS_8198(31));
#endif

#else
	                 /*For old 8196 memory-controller*/
	#ifdef CONFIG_LIMITED_SPI_40MHZ      //SPI clock divide Mem_clock by 6 for low-speed SPI flash
	                 /*Auto Probe Low-speed SPI Flash and Set clock < 40MHZ */
	                if ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x14)
	                {
	                    dprintf("spi_flash.c: Set 8196 SPI clock < 40MHZ for low-speed SPI Flash\n");
	                    WRITE_MEM32(SFCR, SPI_SFSIZE(spi_flash_info[0].device_size - 17) \
	                        | SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) \
	                        | SPI_TCS(15) | SPI_RD_MODE(0) | SPI_RD_OPT(0));
	                }

	#else                                //SPI clock divide Mem_clock by 2 for high speed SPI flash
	                WRITE_MEM32(SFCR, SPI_SFSIZE(spi_flash_info[0].device_size - 17) \
	                    | SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(0) \
	                    | SPI_TCS(15) | SPI_RD_MODE(0) | SPI_RD_OPT(0));
	#endif
#endif

            }
        }

    }
    //dprintf("%s:0xb8001204=%x\n", __FUNCTION__, READ_MEM32(0xb8001204) );
}


                 //#ifdef SUPPORT_SST_SPI
#if 0   //michael
void EWSR(unsigned short cnt)
{
                 //printf("\nEWSR\n ");

    spi_ready();
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
                 //	Send_Byte(0x50);		/* enable writing to the status register */
    *(volatile unsigned int *) SFDR = 0x50 << 24;    //swap

                 //	CE_High();			/* disable device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

}


                 /* SST WRSR */
void WRSR(unsigned short cnt)
{
                 //printf("\nWRSR\n");
    spi_ready();

                 // CE_Low();			/* enable device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

                 //Send_Byte(0x01);		/* select write to status register */
    *(volatile unsigned int *) SFDR = (0x01 << 24);

                 //Send_Byte(byte);		/* data that will change the status of BPx  or BPL (only bits 2,3,4,5,7 can be written) */
    *(volatile unsigned int *) SFDR = (0x0<<24);     //swap ,still 0

                 //CE_High();			/* disable the device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

}
#endif

void EWSR_8198(unsigned short cnt)
{
                 // printf("\n8198 EWSR\n ");

    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1+cnt) | READY(1);
                 //	Send_Byte(0x50);		/* enable writing to the status register */
                                                     //swap
    *(volatile unsigned int *) SFDR_8198 = 0x50 << 24;

                 //	CE_High();			/* disable device */
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

}


                 /* SST WRSR */
void WRSR_8198(unsigned short cnt)
{
                // printf("\n8198 WRSR\n");
    CHECK_READY;

                 // CE_Low();			/* enable device */
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1+cnt) | READY(1);

                 //Send_Byte(0x01);		/* select write to status register */
    *(volatile unsigned int *) SFDR_8198 = (0x01 << 24);

                 //Send_Byte(byte);		/* data that will change the status of BPx  or BPL (only bits 2,3,4,5,7 can be written) */
    *(volatile unsigned int *) SFDR_8198 = (0x0<<24);//swap ,still 0

                 //CE_High();			/* disable the device */
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

}

#if 0   //michael
void RDSR(unsigned short cnt)
{
                 //printf("\nRDSR\n ");
    unsigned char byte ;
    spi_ready();

                 // CE_Low();			/* enable device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

                 //Send_Byte(0x05);		/* select write to status register */
    *(volatile unsigned int *) SFDR = (0x05 << 24);

    while (1)
    {
                 /* RDSR Command */
        if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
        {
            break;
        }
    }

    byte = *(volatile unsigned int *) SFDR;
                 //printf("\nSTATUS(char)=%x\n", byte);

                 //CE_High();			/* disable the device */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

}
#endif

#if 0   //michael
void sst_spi_write(unsigned int cnt, unsigned int address, unsigned char data_in)
{
                 /* RDSR Command */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x05 << 24;

    while (1)
    {
                 /* RDSR Command */
        if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x0)
        {
            break;
        }
    }

                 //1.release CS
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

                 /* WREN Command */
    spi_ready();                                     //2.waiting release OK
                 //3.CS low
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x06 << 24;    //4.instr code
                 //1.release CS
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

                 /* BP Command */
    spi_ready();                                     //2.waiting release OK
                 //3.CS low
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);

                 //JSW: for SST Byte Program,be aware of LENGTH @2007/9/26
                 //4.instr code
    *(volatile unsigned int *) SFDR = (0x02 << 24) | (address & 0xFFFFFF);
                 //  *(volatile unsigned int *) SFDR = (0x02<<24 ) ;  //4.instr code

                 //3.CS low
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = (data_in<<24);
                 //5.CS HIGH
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

}
#endif

#ifdef SUPPORT_SPI_MIO_8198_8196C                                               /*JSW@20100929: Add SST SPI Flash support for 8196C and 8198*/
void sst_spi_write_8198(unsigned int cnt, unsigned int address, unsigned char data_in)
{
 #if 1
    spi_pio_init_8198();

                 //No dummy cycle
    *(volatile unsigned int *) SFCR2_8198 &= 0xFFFF1BFF;


                 /* RDSR Command */
    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1+cnt) ;
    *(volatile unsigned int *) SFDR_8198  = 0x05 << 24;

    while (1)
    {
        if ( ((*(volatile unsigned int *) SFDR_8198 ) & 0x01000000) == 0x00000000)
        {
            break;
        }
    }
#endif

                 //1.release CS
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 /* WREN Command */
    CHECK_READY;                                     //2.waiting release OK
                 //3.CS low
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1+cnt) | READY(1);
                                                     //4.instr code
    *(volatile unsigned int *) SFDR_8198 = 0x06 << 24;
                 //1.release CS
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 /* BP Command */
    CHECK_READY;                                     //2.waiting release OK
                 //3.CS low
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(1+cnt) | READY(1);

                 //JSW: for SST Byte Program,be aware of LENGTH @2007/9/26
                 //4.instr code
    *(volatile unsigned int *) SFDR_8198 = (0x02 << 24) | (address & 0xFFFFFF);
                 //  *(volatile unsigned int *) SFDR = (0x02<<24 ) ;  //4.instr code

                 //3.CS low
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR_8198 = (data_in<<24);
                 //5.CS HIGH
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

}
#endif

                 //#endif
                 //end of SUPPORT_SST_SPI

                 // 20070827 auto-test-porgram for SPI and SDRAM

#define MIN(i, j)                ((i) < (j) ? (i) : (j))
#define MAX(i, j)                ((i) > (j) ? (i) : (j))

                 //void auto_memtest(unsigned char *start,unsigned char *end)
#if 0   //michael
int spi_flw_image(unsigned int chip, unsigned int flash_addr_offset ,unsigned char *image_addr, unsigned int image_size)
{
#if 0
#ifdef Set_SECTOR_ERASE_4KB
    dprintf("\nSingle Sector : 4KB \n");
#else
    dprintf("\nSingle Sector : 64KB \n");
#endif
#endif
    unsigned int temp;
    unsigned short i, j, k;
    unsigned char *cur_addr;
    unsigned int cur_size;
    unsigned int SST_Single_Byte_Data,SST_Flash_Offset;
    short shift_cnt8;
    unsigned int byte_cnt=0;

    cur_addr = image_addr;
    cur_size = image_size;

#ifdef Set_SECTOR_ERASE_64KB
                 //Sector:64KB
    unsigned int sector_start_cnt = flash_addr_offset / 65535;

    unsigned int sector_end_cnt = sector_start_cnt + (image_size / 0x10000);
#endif

#ifdef Set_SECTOR_ERASE_4KB
                 //Sector:4KB
    unsigned int sector_start_cnt = flash_addr_offset / 4095;

    unsigned int sector_end_cnt = sector_start_cnt + (image_size / 0x1000);
#endif

                 /* Iterate Each Sector */
    for (i = sector_start_cnt; i <= sector_end_cnt; i++)
    {
                 //unsigned int spi_data;
        spi_pio_init();
        spi_sector_erase(chip,i);

        *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

                 /* Iterate Each Page */
#ifdef Set_SECTOR_ERASE_64KB
        for (j = 0; j < 256; j++)                    //64KB(Sector)/256B(PageProgram)=256
#endif
#ifdef Set_SECTOR_ERASE_4KB
            for (j = 0; j < 16; j++)                 //4KB(Sector)/256B(PageProgram)=16
#endif
        {
            if (cur_size == 0)
                break;

            spi_ready();
            *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
            *(volatile unsigned int *) SFDR = 0x05 << 24;

            while (1)
            {

                if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
                {
                    break;
                }
            }
            *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

                 /* WREN Command */
            spi_ready();
            *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
            *(volatile unsigned int *) SFDR = 0x06 << 24;
            *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

                 /* PP Command */
            spi_ready();
            *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
#ifdef Set_SECTOR_ERASE_64KB
                 //Sector:64KB
            *(volatile unsigned int *) SFDR_8198 = (0x02 << 24) | (i * 65536) | (j * 256);
#endif

#ifdef Set_SECTOR_ERASE_4KB
            if ((spi_flash_info[0].type_id==0x25 ))
                {}
                else
            {
                 //Sector:4KB , j:page Program size
                *(volatile unsigned int *) SFDR = (0x02 << 24) | (i * 4096) | (j * 256);
            }
#endif

            for (k = 0; k < 64; k++)                 //k:write 4Byte once
            {
                temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));

                spi_ready();
                if (cur_size >= 4)
                {
                    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
                    cur_size -= 4;
                }
                else
                {

                    *(volatile unsigned int *) SFCSR = LENGTH(cur_size-1) | CS(1+chip) | READY(1);
                    cur_size = 0;
                }

                 //for SST SPI Type:Single-Byte Burning
                if ((spi_flash_info[0].type_id==0x25 ))
                {
                    for(shift_cnt8=24;shift_cnt8>=0;shift_cnt8-=8)
                    {
                        byte_cnt%=4;
                        SST_Single_Byte_Data=(temp>>shift_cnt8)&0xff;
                        SST_Flash_Offset=(i*4096)+(j*256)+(k*4)+byte_cnt;
                 //Write 1 byte each time

#if 1
                 /* RDSR Command */
                        spi_ready();
                        *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
                        *(volatile unsigned int *) SFDR = 0x05 << 24;

                        while (1)
                        {
                            if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
                            {
                                break;
                            }
                        }
#endif

                        sst_spi_write(chip,SST_Flash_Offset,SST_Single_Byte_Data);
                        byte_cnt+=1;

                    }
                }
                else                                 //for MXIC and Spansion 4 byte  burning
                {
                    *(volatile unsigned int *) SFDR = temp;
                }

                cur_addr += 4;

                if (cur_size == 0)
                    break;
            }

            *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

                 /* RDSR Command */
            spi_ready();
            *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
            *(volatile unsigned int *) SFDR = 0x05 << 24;

            while (1)
            {
                unsigned int status = *(volatile unsigned int *) SFDR;

                 /* RDSR Command */
                if ((status & 0x01000000) == 0x00000000)
                {
                    break;
                }
#if 0                                //JSW@20090714 :Delete for code size
                unsigned int cnt=0;
                if (cnt > (1000*1000*200))
                {

                    busy:
#ifdef Set_SECTOR_ERASE_64KB
                    dprintf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*65536+j*256);
#endif

#ifdef Set_SECTOR_ERASE_4KB
                    dprintf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*4096+j*256);
#endif
                    goto busy;
                }
                cnt++;
#endif
            }

            *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

        }

        if (cur_size == 0)
            break;
    }                                                /* Iterate Each Sector */
    return 1;

}
#endif

                 /*JSW@20091007: For RTL8196C/8198 New SPI architecture*/
#ifdef SUPPORT_SPI_MIO_8198_8196C


                 //JSW add :Sector Erase
void spi_sector_erase_8198(unsigned int cnt, unsigned short i)
{
   spi_pio_init_8198();   //michael
                 /* WREN Command */
    CHECK_READY;
                 //*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
    *(volatile unsigned int *) SFDR_8198 = 0x06 << 24;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 /* SE Command */
    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(1 + cnt) | READY(1);
     CHECK_READY;
#ifdef Set_SECTOR_ERASE_64KB
    *(volatile unsigned int *) SFDR_8198 = (0xD8 << 24) | (i * 65536);
#endif

#ifdef Set_SECTOR_ERASE_4KB
    *(volatile unsigned int *) SFDR_8198 = (0x20 << 24) | (i * 4096);
#endif
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

                 /* RDSR Command */
    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
    *(volatile unsigned int *) SFDR_8198 = 0x05 << 24;

    while (1)
    {
                 /* RDSR Command */
        if (((*(volatile unsigned int *) SFDR_8198) & 0x01000000) == 0x00000000)
            break;
    }

    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

#if 0
                 //dprintf("Erase Sector: %d\n", i);
#else
    //prom_printf(".");
#endif

}


void Set_QE_bit(unsigned short QE_bit, unsigned short cnt, uint32 SPI_ID)
{
                 //1.release CS
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 /* WREN Command */
    CHECK_READY;
                 //3.CS low
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
                 //4.instr code
    *(volatile unsigned int *) SFDR_8198 = 0x06 << 24;
                 //1.release CS;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 //WRSR
    SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x01));

    if (QE_bit)                                      //QE=1
    {
#ifdef MXIC_SPI_QPP_Instruction_code
        SPI_REG(SFDR2_8198) = 0x40000000;            //Enable QE Bit for MXIC
#endif                                       //end of MXIC_SPI_QPP_Instruction_code

#ifdef Spansion_SPI_QPP_Instruction_code
        SPI_REG(SFDR2_8198) = 0x00020000;            //Enable QE Bit for Spansion
#endif                                       //end of Spansion_SPI_QPP_Instruction_code
    }
    else                                             //QE=0
    {
        SPI_REG(SFDR2_8198) = 0x0;
    }

}


void spi_pio_init_8198(void)
{
    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(0) | READY(1);

    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(0) | READY(1);

    CHECK_READY;
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);
}


                 /*JSW @20090416 :Pre-init 8198 SPI-controller for Set Quad Mode and QE bit
                    SFCR2_CMDIO(?) depends on Vendor's Spec

                 */
unsigned long __spi_flash_preinit_8198(unsigned int read_data, unsigned int cnt)
{
    unsigned int RDSR_data;

    switch (read_data&0x00FFFFFF)
    {
        case MX25L1635D:
        case MX25L3235D:
        case MX25L6445E:
        case MX25L12845E:

#if 0
                 // Read EQ bit
                 //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
            RDSR_data = SPI_REG(SFDR2_8198);
            dprintf("Before WRSR, RDSR_data =%x\n", RDSR_data);

                 //WRSR
                 //WRSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x01));
            SPI_REG(SFDR2_8198) = 0x40000000;

                 // Read EQ bit
                 //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
                 //SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(3)|SFCSR_CMD_BYTE(0x05));//RDSR
            RDSR_data = SPI_REG(SFDR2_8198);
            dprintf("After WRSR, RDSR_data =%x\n", RDSR_data);

            if (RDSR_data&0x40000000)                // MXIC Quad Enable Bit
            {
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xeb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(3) | SFCR2_DATAIO(2));
                 //dprintf(" RDSR_data =%x\n",RDSR_data);
            }
            else
            {
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));
                 //dprintf("RDSR_data =%x\n",RDSR_data);
            }
#endif

#if 1
#ifdef CONFIG_BOOT_SIO_8198
                 //0x0b,FAST READ
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));

                 //Unset QUAD Enable Bit
            Set_QE_bit(0, cnt, read_data);
                 //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
            RDSR_data = SPI_REG(SFDR2_8198);
#ifdef SPI_DBG_MESSAGE
            dprintf("\nSIO RDSR_data =%x\n", RDSR_data);
            dprintf("MXIC: Set Single I/O Fast Read\n");
#endif
#endif

#ifdef CONFIG_BOOT_DIO_8198
                 //0xbb,Dual READ
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xbb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(1) | SFCR2_DUMMYCYCLE(2) | SFCR2_DATAIO(1));
                 //Unset QUAD Enable Bit
            Set_QE_bit(0, cnt, read_data);
                 //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
            RDSR_data = SPI_REG(SFDR2_8198);
#ifdef SPI_DBG_MESSAGE
            dprintf("DIO RDSR_data =%x\n", RDSR_data);
            dprintf("MXIC: Set DUAL I/O Read\n");
#endif
#endif

#ifdef CONFIG_BOOT_QIO_8198
                 //Set QUAD Enable Bit
            Set_QE_bit(1, cnt, read_data);
                 //RDSR
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
            RDSR_data = SPI_REG(SFDR2_8198);
#ifdef SPI_DBG_MESSAGE
            dprintf("QIO RDSR_data =%x\n", RDSR_data);
#endif

            if (RDSR_data&0x40000000)                // MXIC Quad Enable Bit
            {
                CHECK_READY;
                 //default OK
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xeb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(3) | SFCR2_DATAIO(2));
                CHECK_READY;
#ifdef SPI_DBG_MESSAGE
                dprintf("MXIC: Set QUAD I/O Read\n");
#endif
            }
            else
            {
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));
#ifdef SPI_DBG_MESSAGE
                 //dprintf("RDSR_data =%x\n",RDSR_data);
                dprintf("\n\nMXIC: Set QUAD I/O Fail!!\n\n");
#endif
            }
#endif
#endif
            break;                                   //End of MXIC

                 /*JSW@20100929: Add SST SPI Flash support for 8196C and 8198*/
        case SST26VF016:
        case SST26VF032:
        case SST25VF016B:
        case SST25VF032B:
	 case SST25VF064C:           
	           #if 1		
				   dprintf("SST: Set Single I/O mode \n");
			        //0x0b,FAST READ
	            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));

	                 //Unset QUAD Enable Bit
	            Set_QE_bit(0, cnt, read_data);
	                 //RDSR
	            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x05));
	            RDSR_data = SPI_REG(SFDR2_8198);
	            //dprintf("\nSIO RDSR_data =%x\n", RDSR_data);
	           
		    #endif
            break;
                 //End of SST

        case W25Q80:
        case W25Q16:
        case W25Q32:
        case W25Q64:                                 /*JSW: 20100610 Winbond 8MB*/
                 // Read EQ bit
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x35));
            read_data = SPI_REG(SFDR2_8198);

#ifdef CONFIG_BOOT_QIO_8198
            if (read_data&0x02000000)                // MXIC Quad Enable Bit
            {
                dprintf("=>Winbond:Set Quad READ I/O SPI ,Enter High Performane Mode\n");
                 // Enter High Performane Mode
                SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(3) | SFCSR_CMD_BYTE(0xA3));
                SPI_REG(SFDR2_8198) = 0x00000000;
                                                     //Quad IO
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xeb) | SFCR2_RDOPT | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(3) | SFCR2_DATAIO(2));
            }
            else
            {
                dprintf("=>Winbond:Set Quad READ I/O Fail\n");
            }
#endif

#ifdef CONFIG_BOOT_DIO_8198
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(3) | SFCSR_CMD_BYTE(0xA3));
            SPI_REG(SFDR2_8198) = 0x00000000;
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xbb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_CMDIO(0) | SFCR2_ADDRIO(1) | SFCR2_DUMMYCYCLE(2) | SFCR2_DATAIO(1));
#ifdef SPI_DBG_MESSAGE
            dprintf("=>Winbond:Set Dual READ I/O SPI \n");
#endif
#endif

#ifdef CONFIG_BOOT_SIO_8198
#ifdef SPI_DBG_MESSAGE
            dprintf("=>Winbond:Set Single IO mode \n");
#endif
            WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(3) | SPI_TCS_8198(31));

                 //SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(3)|SFCSR_CMD_BYTE(0xA3));
                 //SPI_REG(SFDR2) = 0x00000000;
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));
                 //SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b)|SFCR2_SIZE(7)|SFCR2_RDOPT|SFCR2_ADDRIO(0)|SFCR2_DUMMYCYCLE(4)|SFCR2_DATAIO(0)|SFCR2_HOLD_TILL_SFDR2);
#endif

            break;                                   //End of Winbond

        case S25FL032P:
        case S25FL128P:
#ifdef CONFIG_BOOT_QIO_8198
            Set_QE_bit(1, cnt, read_data);
                 //Read Configuration Register
                 //Spansion RCR(0x35)
            SPI_REG(SFCSR_8198) = (SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SFCSR_LEN(1) | SFCSR_CMD_BYTE(0x35));
            read_data = SPI_REG(SFDR2_8198);
#ifdef SPI_DBG_MESSAGE
            dprintf("Spansion RCR =%x\n", read_data);
#endif
            if (read_data&0x02000000)                //If Quad enable bit set.
            {
                SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xeb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(2) | SFCR2_DUMMYCYCLE(3) | SFCR2_DATAIO(2));
#ifdef SPI_DBG_MESSAGE
                dprintf("=>Spansion:Set QUAD READ I/O SPI \n");
#endif
            }
#endif

#ifdef CONFIG_BOOT_DIO_8198
            Set_QE_bit(0, cnt, read_data);
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0xbb) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(1) | SFCR2_DUMMYCYCLE(2) | SFCR2_DATAIO(1));
#ifdef SPI_DBG_MESSAGE
            dprintf("=>Spansion:Set Dual READ I/O SPI \n");
#endif
#endif

#ifdef CONFIG_BOOT_SIO_8198
            Set_QE_bit(0, cnt, read_data);
            SPI_REG(SFCR2_8198) = (SFCR2_SFCMD(0x0b) | SFCR2_SIZE(7) | SFCR2_RDOPT | SFCR2_ADDRIO(0) | SFCR2_DUMMYCYCLE(4) | SFCR2_DATAIO(0));
#ifdef SPI_DBG_MESSAGE
            dprintf("=>Spansion:Set Single READ I/O SPI \n");
#endif
#endif

            break;                                   //End of Spansion

        default:
            break;
    }
    read_data = SPI_REG(SFCR2_8198) & (~SFCR2_HOLD_TILL_SFDR2);
    SPI_REG(SFCR2_8198) = read_data;
    return 0;
}

#endif                                               //end of SUPPORT_SPI_MIO_8198_8196C


// ----------------------------------------------------------------------------
// Initialize the flash.


static int
rtl819x_spi_flash_init(struct cyg_flash_dev* dev)
{
    cyg_rtl819x_spi_flash_dev *rtl819x_dev = (cyg_rtl819x_spi_flash_dev *)dev->priv;
    cyg_uint32 flash_size, block_size = 0;
    
    //diag_printf("###spi_probe start###\n");
    spi_probe();
    //diag_printf("###spi_probe end###\n");
    // Set up the block info entries.
    dev->block_info                             = &rtl819x_dev->block_info[0];
    dev->num_block_infos                        = 1;

    block_size = 4096;
    flash_size = spi_flash_info[0].sector_cnt * block_size;
    
    rtl819x_dev->block_info[0].blocks             = spi_flash_info[0].sector_cnt;
    rtl819x_dev->block_info[0].block_size         = block_size;
    
    // Set end address
    dev->end                                    = dev->start+flash_size-1;

    rtkf_diag("block_size %d size %08x end %08x\n", block_size, flash_size, dev->end );    

    
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
	int temp;
#ifdef Set_SECTOR_ERASE_64KB
	unsigned int sector_cnt = addr_offset / 0x10000; //Sector:64KB
#endif
#ifdef Set_SECTOR_ERASE_4KB
	unsigned int sector_cnt = addr_offset / 0x1000; //Sector:4KB
#endif

	spi_pio_init_8198();
	//No dummy cycle
	temp = *(volatile unsigned int *) SFCR2_8198;
	*(volatile unsigned int *) SFCR2_8198 = temp & 0xFFFF1BFF;
	//1.release CS
	*(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

	spi_sector_erase_8198(0, sector_cnt);

	*(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);
/*
#ifdef Set_SECTOR_ERASE_64KB
	printf("Erase Sector:%d,Addr:0x%x->0x%x\n" , sector_cnt, \
            (0xbd000000 + (sector_cnt*0x10000)), (0xbd000000 + (((sector_cnt + 1)*0x10000)) - 1));
#endif
#ifdef Set_SECTOR_ERASE_4KB
	printf("Erase Sector:%d,Addr:0x%x->0x%x\n" , sector_cnt, \
            (0xbd000000 + (sector_cnt*0x1000)), (0xbd000000 + (((sector_cnt + 1)*0x1000)) - 1));
#endif
*/
	*(volatile unsigned int *) SFCR2_8198 = temp;
	return CYG_FLASH_ERR_OK;
}

// ----------------------------------------------------------------------------
// Write data to flash, using individual word writes. The destination
// address will be aligned in a way suitable for the bus. The source
// address need not be aligned. The count is in RTL819X_TYPE's, not in
// bytes.
static int
rtl819x_spi_flash_hw_program(unsigned int flash_addr_offset , unsigned char *image_addr, unsigned int image_size)
{
    unsigned int temp;
    unsigned int i, j, k;
    unsigned char *cur_addr;
    unsigned int cur_size;
    //unsigned int RDSR_data;

                 /*JSW@20100929: Add SST SPI Flash support for 8196C and 8198*/
    unsigned int SST_Single_Byte_Data,SST_Flash_Offset;
    short shift_cnt8;
    unsigned int byte_cnt=0;
	unsigned int cnt = 0;   //michael

    cur_addr = image_addr;
    cur_size = image_size;

#ifdef Set_SECTOR_ERASE_64KB
                 //Sector:64KB
    unsigned int sector_start_cnt = flash_addr_offset / 0x10000;
    //michael
	unsigned int sector_end_cnt =  (flash_addr_offset + image_size - 1) / 0x10000;
	unsigned int first_page = (flash_addr_offset % 0x10000) / 256;
#endif

#ifdef Set_SECTOR_ERASE_4KB
                 //Sector:4KB
    unsigned int sector_start_cnt = flash_addr_offset / 0x1000;
    //michael
	unsigned int sector_end_cnt =  (flash_addr_offset + image_size - 1) / 0x1000;
	unsigned int first_page = (flash_addr_offset % 0x1000) / 256;
#endif
	unsigned int first_4byte = (flash_addr_offset % 256) / 4;

                 //dprintf("sector_start_cnt =%d\n", sector_start_cnt);
                 //dprintf("sector_end_cnt =%d\n", sector_end_cnt);
                 //dprintf("first_page=%d\n", first_page);
                 //dprintf("first_4byte=%d\n", first_4byte);

    spi_pio_init_8198();
                 //No dummy cycle
    *(volatile unsigned int *) SFCR2_8198 &= 0xFFFF1BFF;
                 //1.release CS
    *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 /* Iterate Each Sector */
                 //i:sector
    for (i = sector_start_cnt; i <= sector_end_cnt; i++)
    {

	

        spi_pio_init_8198();
        //spi_sector_erase_8198(cnt,i); //do not erase here, michael

        *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 /* Iterate Each Page */
		//michael
		if (i == sector_start_cnt)
			j = first_page;
		else
			j = 0;

#ifdef Set_SECTOR_ERASE_64KB
		for (; j<256; j++) //64KB(Sector)/256B(PageProgram)=256
#endif
#ifdef Set_SECTOR_ERASE_4KB
		for (; j<16; j++)  //4KB(Sector)/256B(PageProgram)=16
#endif
        {
            if (cur_size == 0)
                break;

            CHECK_READY;
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
            *(volatile unsigned int *) SFDR_8198 = 0x05 << 24;
            while (1)
            {
                 //write in progress loop
                if (((*(volatile unsigned int *) SFDR_8198) & 0x01000000) == 0x00000000)
                {
                    break;
                }
            }

                 /*JSW @ 20090414: For 8198 SPI QIO Write*/
#ifdef CONFIG_BOOT_QIO_8198

            CHECK_READY;
            SPI_REG(SFCSR_8198) = SPI_CS_INIT;       //deactive CS0, CS1
            CHECK_READY;
            SPI_REG(SFCSR_8198) = 0;                 //active CS0,CS1
            CHECK_READY;
            SPI_REG(SFCSR_8198) = SPI_CS_INIT;       //deactive CS0, CS1
            CHECK_READY;

                 //No dummy cycle
            *(volatile unsigned int *) SFCR2_8198 &= 0xFFFF1BFF;
                 //1.release CS
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 /* WREN Command */
            CHECK_READY;
                 //3.CS low
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
                 //4.instr code
            *(volatile unsigned int *) SFDR_8198 = 0x06 << 24;
                 //1.release CS
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

            CHECK_READY;
                 //3.CS low and IO_WIDTH must =0
            *(volatile unsigned int *) SFCSR_8198 = (LENGTH(0) | CS(1 + cnt) | READY(1) | SFCSR_IO_WIDTH(0));

                 //4.QPP instr code for MXIC
#ifdef MXIC_SPI_QPP_Instruction_code
	//JSW@20110401 , Set Quad IO QPP clock close to 20 MHZ
	 WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(3) | SPI_TCS_8198(31));

			*(volatile unsigned int *) SFDR_8198 = 0x38 << 24;

            CHECK_READY;
                 //for 3 Byte address and IO_WIDTH must =2
            *(volatile unsigned int *) SFCSR_8198 = (LENGTH(2) | CS(1 + cnt) | SFCSR_IO_WIDTH(2));
#endif

                 //4.QPP instr code Spansion
#ifdef Spansion_SPI_QPP_Instruction_code
            *(volatile unsigned int *) SFDR_8198 = 0x32 << 24;

                 //for 3 Byte address and IO_WIDTH must = 0
            *(volatile unsigned int *) SFCSR_8198 = (LENGTH(2) | CS(1 + cnt) | SFCSR_IO_WIDTH(0));
#endif

#ifdef Set_SECTOR_ERASE_64KB
                 // 3 Byte address to SFDR
            *(volatile unsigned int *) SFDR_8198 = ((((i * 65535) | (j * 256)) & 0xFFFFFF) << 8);
#endif

#ifdef Set_SECTOR_ERASE_4KB
                 // 3 Byte address to SFDR
            *(volatile unsigned int *) SFDR_8198 = ((((i * 4096) | (j * 256)) & 0xFFFFFF) << 8);
#endif

            CHECK_READY;
                 //4Byte Data
            *(volatile unsigned int *) SFCSR_8198 = (LENGTH(3) | CS(1 + cnt) | SFCSR_IO_WIDTH(2));

#else                                    /*JSW @ 20090414: For 8198 SPI SIO or DIO Write*/

            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 /* WREN Command */
            CHECK_READY;
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
            *(volatile unsigned int *) SFDR_8198 = 0x06 << 24;
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);

                 /* PP Command (0x02)*/
            CHECK_READY;
                 //default OK
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(1 + cnt) | READY(1);

#ifdef Set_SECTOR_ERASE_64KB
                 //Sector:64KB
            *(volatile unsigned int *) SFDR_8198 = (0x02 << 24) | (i * 65536) | (j * 256);
#endif

#ifdef Set_SECTOR_ERASE_4KB
                 //Sector:4KB , j:page Program size
            *(volatile unsigned int *) SFDR_8198 = (0x02 << 24) | (i * 4096) | (j * 256);
#endif
#endif                                   //end of CONFIG_BOOT_QIO_8198

			//michael
			if (i == sector_start_cnt)
				k = first_4byte;
			else
				k = 0;
			for (; k<64; k++)                 //k:write 4Byte once
            {
                temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));

                CHECK_READY;
                if (cur_size >= 4)
                {
#ifdef CONFIG_BOOT_QIO_8198
                 //4Byte Data
                    *(volatile unsigned int *) SFCSR_8198 = (LENGTH(3) | CS(1 + cnt) | SFCSR_IO_WIDTH(2));
#else
                    *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(1 + cnt) ;
#endif
                    cur_size -= 4;
                }
                else
                {
#ifdef CONFIG_BOOT_QIO_8198
                    *(volatile unsigned int *) SFCSR_8198 = LENGTH(cur_size - 1) | CS(1 + cnt) | SFCSR_IO_WIDTH(2);
#else
                    *(volatile unsigned int *) SFCSR_8198 = LENGTH(cur_size - 1) | CS(1 + cnt) ;
#endif
                    cur_size = 0;
                }

#if 1                                /*JSW@20100929: Add SST SPI Flash support for 8196C and 8198*/
                 /*for SST SPI Type:Single-Byte Burning*/
               // if ((spi_flash_info[0].type_id==0x25 )) //Using Single-Byte Burning for All SST SPI Flash 
                if ( SST_1Byte_SPI_Flash==1 )  //Using 4-Byte Burning only for Speed-up SST 8MB SPI Flash                
                {                
                    for(shift_cnt8=24;shift_cnt8>=0;shift_cnt8-=8)
                    {

                        byte_cnt%=4;

                        SST_Single_Byte_Data=(temp>>shift_cnt8)&0xff;
                        SST_Flash_Offset=(i*4096)+(j*256)+(k*4)+byte_cnt;

#if 0
                        dprintf("\nbyte_cnt: %d\n", byte_cnt);
                        dprintf("\ntemp: %x\n", temp);
                        dprintf("\nshift_cnt8: %x\n", shift_cnt8);
                        dprintf("\n SST_Single_Byte_Data: %x\n\n",  SST_Single_Byte_Data);
#endif
                        CHECK_READY;
                 //Write 1 byte each time

                        sst_spi_write_8198(cnt,SST_Flash_Offset,SST_Single_Byte_Data);
                        byte_cnt+=1;

                    }
                }
                else                                 //for MXIC and Spansion 4 byte  burning
                {
                    *(volatile unsigned int *) SFDR_8198  = temp;
                }
#endif                               //20100929 :Add  SST SPI Flash for 8196C

                cur_addr += 4;

                if (cur_size == 0)
                    break;
            }                                        //end of k-loop

            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

                 /* RDSR Command */
            CHECK_READY;
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(1 + cnt) | READY(1);
            *(volatile unsigned int *) SFDR_8198 = 0x05 << 24;

            unsigned int timeout = 0;
            while (1)
            {
                unsigned int status = *(volatile unsigned int *) SFDR_8198;

                 /* RDSR Command */
                if ((status & 0x01000000) == 0x00000000)
                {
                    break;
                }

                if (timeout  > (1000*1000*200))
                {

                    busy:
#ifdef Set_SECTOR_ERASE_64KB
                    dprintf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*65536 + j*256);
#endif
#ifdef Set_SECTOR_ERASE_4KB
                    dprintf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*4096 + j*256);
#endif
                    goto busy;
                }
                timeout ++;
            }

            CHECK_READY;
#ifdef CONFIG_BOOT_SIO_8198
                 //CS high
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

                 //Recovery READ mode dummy cycle for SIO that SPI clcok > 40MHZ
            *(volatile unsigned int *) SFCR2_8198 |= SFCR2_DUMMYCYCLE(4);

                 /*Auto Probe Low-speed SPI Flash and Set clock < 40MHZ 	*/
                 //Limited SPI clock <=40MHZ for older SPI flash
#ifdef CONFIG_AUTO_PROBE_LIMITED_SPI_CLK_UNDER_40MHZ
                 //#define MX25L0805D  0x00C22014   /*MXIC 1MB*/
                 //#define MX25L1605D  0x00C22015   /*MXIC 2MB*/
                 //#define MX25L3205D  0x00C22016   /*MXIC 4MB*/
                 //#define MX25L3206E  0x00C22016   /*MXIC 4MB*/
                 //#define MX25L6405D  0x00C22017   /*MXIC 8MB*/
                 //#define MX25L12805D  0x00C22018   /*MXIC 16MB*/
            if (((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x14)
//                || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x15) //michael removed
//                || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x16) //michael removed
                || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x17)
                || ((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x20)&&(spi_flash_info[0].capacity_id) == 0x18))
            {
                 //dprintf("spi_flash.c: Set SPI clock < 40MHZ for low-speed SPI Flash\n");
                WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(2) | SPI_TCS_8198(31));

                 //dprintf("spi_flash.c: Set No dummy cycle\n");
                REG32(SFCR2_8198)=READ_MEM32(SFCR2_8198)&0xFFFF1FFF;
            }
#endif
#endif

#ifdef CONFIG_BOOT_DIO_8198
                 //CS high
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);
                 //dummy_cycle(0)
            *(volatile unsigned int *) SFCR2_8198 &= 0xFFFF1FFF;
                 //Recovery READ mode 4 dummy cycle for DIO
                 //4
            *(volatile unsigned int *) SFCR2_8198 |= SFCR2_DUMMYCYCLE(2);
#endif

#ifdef CONFIG_BOOT_QIO_8198
                 //CS high
            *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);
                 //*(volatile unsigned int *) SFCR2_8198 &=0xFFF31FFF;//dummy_cycle(0) and CMD_IO(0)
                 //*(volatile unsigned int *) SFCR2_8198 = (SFCR2_DUMMYCYCLE(3)|SFCR2_CMDIO(2));//Recovery READ mode dummy cycle for QIO
                 //dummy_cycle(0)
            *(volatile unsigned int *) SFCR2_8198 &= 0xFFFF1FFF;
                 //Recovery READ mode 6 dummy cycle for QIO
            *(volatile unsigned int *) SFCR2_8198 |= SFCR2_DUMMYCYCLE(3);

		

	       if (((spi_flash_info[0].maker_id == 0xc2) && (spi_flash_info[0].type_id == 0x25)&&(spi_flash_info[0].capacity_id) == 0x15)) // MXIC MX25L1636E 133MHZ 
                {
                    //dprintf("spi_flash.c: Set SPI clock=DRAM clock/2  for high-speed SPI Flash\n");
                    WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(0) | SPI_TCS_8198(31));

                }
		 else
		 {
		 	//JSW@20110401 , Set Quad IO Read clock close to 75MHZ
	     		WRITE_MEM32(SFCR_8198, SPI_RD_ORDER(1) | SPI_WR_ORDER(1) | SPI_CLK_DIV(1) | SPI_TCS_8198(31));			
		 }
	
#endif
            CHECK_READY;

        }                                            //end of j-loop

        if (cur_size == 0)
        {
                 // dprintf("i =%d\n",i);
            break;
        }
    }                                                /* Iterate Each Sector */
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
    erase_fn    = (int (*)(cyg_uint32)) cyg_flash_anonymizer( & rtl819x_spi_flash_hw_erase );

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
    int (*program_fn)(unsigned int flash_addr_offset , unsigned char *image_addr, unsigned int image_size);
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
    program_fn  = (int (*)(unsigned int flash_addr_offset , unsigned char *image_addr, unsigned int image_size)) cyg_flash_anonymizer( & rtl819x_spi_flash_hw_program );

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
