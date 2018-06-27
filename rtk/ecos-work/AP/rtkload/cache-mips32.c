/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 - 1999 by Ralf Baechle
 * Modified for R3000 by Paul M. Antoine, 1995, 1996
 * Complete output from die() by Ulf Carlsson, 1998
 * Copyright (C) 1999 Silicon Graphics, Inc.
 */
#include "config.h"
//#include <linux/kconfig.h>
#define CONFIG_PAGE_SIZE_4KB
//#include <linux/linkage.h>
//#include <asm/ptrace.h>
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) //modified by lynn_pu, 2014-10-30
//#include <asm/mipsregs.h>
#else
//#include <asm/rlxregs.h>
#endif
#include <regdef.h>
//#include <asm/addrspace.h>
//#include <asm/system.h>

#define ulong	unsigned long
#define printf	prom_printf

#if 0
#ifdef CONFIG_RTL865XC
/* For Realtek RTL865XC Network platform series */
#define _ICACHE_SIZE		(16 * 1024)		/* 16K bytes */
#define _DCACHE_SIZE		(8 * 1024)		/* 8K bytes */
#define _CACHE_LINE_SIZE	4			/* 4 words */
#endif

void _flush_dcache_()
{
	/* Flush D-Cache using its range */
	unsigned char *p;
	unsigned int size;
	unsigned int flags;
	unsigned int i;

	unsigned int start=KSEG0;
	unsigned int end=KSEG0 + _DCACHE_SIZE;

	size = end - start;

	/* correctness check : flush all if any parameter is illegal */
// david	
//	if (	(size >= dcache_size) ||
	if (	(size >= _DCACHE_SIZE) ||

		(KSEGX(start) != KSEG0)	)
	{
		/*
		 *	ghhguang
		 *		=> For Realtek Lextra CPU,
		 *		   the cache would NOT be flushed only if the Address to-be-flushed
		 *		   is the EXPLICIT address ( which is really stored in that cache line ).
		 *		   For the aliasd addresses, the cache entry would NOT be flushed even
		 *		   it matchs same cache-index.
		 *
		 *		   => This is different from traditional MIPS-based CPU's configuration.
		 *		      So if we want to flush ALL-cache entries, we would need to use "mtc0"
		 *		      instruction instead of simply modifying the "size" to "dcache_size"
		 *		      and "start" to "KSEG0".
		 *
		 */
		__asm__ volatile(
			"mtc0 $0,$20\n\t"
			"nop\n\t"
			"li $8,512\n\t"
			"mtc0 $8,$20\n\t"
			"nop\n\t"
			"nop\n\t"
			"mtc0 $0,$20\n\t"
			"nop"
			: /* no output */
			: /* no input */
				);
	} else
	{
		/* Start to isolate cache space */
		p = (char *)start;

		flags = read_32bit_cp0_register();

		/* isolate cache space */
		write_32bit_cp0_register(CP0_XCONTEXT, (ST0_ISC | flags) &~ ST0_IEC );

		for (i = 0; i < size; i += 0x040)
		{
			asm ( 	
				"cache 0x15, 0x000(%0)\n\t"
			 	"cache 0x15, 0x010(%0)\n\t"
			 	"cache 0x15, 0x020(%0)\n\t"
			 	"cache 0x15, 0x030(%0)\n\t"
				:		/* No output registers */
				:"r"(p)		/* input : 'p' as %0 */
				);
			p += 0x040;
		}

		write_32bit_cp0_register(CP0_XCONTEXT, flags);
	}       
}

void _flush_icache_()
{
	unsigned int start=KSEG0;
	unsigned int end=KSEG0 + _ICACHE_SIZE;

	/*
		Flush data cache at first in write-back platform.

		Ghhuang (2007/3/9):

		RD-Center suggest that we need to flush D-cache entries which
		might match to same address as I-cache ... when we flush
		I-cache.
		( Maybe some data is treated as data/instruction, both. )
	*/
	_flush_dcache_();

	/*Invalidate I-Cache*/
	__asm__ volatile(
		"mtc0 $0,$20\n\t"
		"nop\n\t"
		"li $8,2\n\t"
		"mtc0 $8,$20\n\t"
		"nop\n\t"
		"nop\n\t"
		"mtc0 $0,$20\n\t"
		"nop"
		: /* no output */
		: /* no input */
			);
}


void flush_cache(void)
{
	_flush_dcache_();
	_flush_icache_();	
}

#endif
//==============================================================================

/* Wei add, modify from RDC init_cache.S */

//#include <asm/mipsregs.h> //modified by lynn_pu, 2014-10-30
#include <cacheops.h>



#define CONFIG_SYS_CACHELINE_SIZE 32
#define _ICACHE_SIZE		(64 * 1024)		
#define _DCACHE_SIZE		(32 * 1024)		
#define _SCACHE_SIZE		(128 * 1024)		


//==============================================================================
#define cache_op(op,addr)						\
	__asm__ __volatile__(						\
	"	.set	push					\n"	\
	"	.set	noreorder				\n"	\
	"	.set	mips3\n\t				\n"	\
	"	cache	%0, %1					\n"	\
	"	nop					\n"	\
	"	.set	pop					\n"	\
	:								\
	: "i" (op), "R" (*(unsigned char *)(addr)))



//==============================================================================
void flush_dcache_range(ulong start_addr, ulong stop)
{
#ifndef CONFIG_NAND_FLASH_BOOTING
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	while (1) 
	{
		cache_op(Hit_Writeback_Inv_D, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
#endif
}
//==============================================================================
//==============================================================================
void flush_scache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	while (1) 
	{
		cache_op(Hit_Writeback_Inv_S, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
}
//==============================================================================
void flush_cache_range(ulong start_addr, ulong size)
{
#ifndef CONFIG_NAND_FLASH_BOOTING
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (start_addr + size - 1) & ~(lsize - 1);

	/* aend will be miscalculated when size is zero, so we return here */
	if (size == 0)
		return;

	while (1) 
	{
		cache_op(Hit_Writeback_Inv_D, addr);
		cache_op(Hit_Invalidate_I, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
#endif
}
void invalidate_scache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	while (1) 
	{
		cache_op(Hit_Invalidate_S, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
}
//==============================================================================
void invalidate_dcache_range(ulong start_addr, ulong stop)
{
#ifndef CONFIG_NAND_FLASH_BOOTING
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	while (1) 
	{
		cache_op(Hit_Invalidate_D, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
#endif
}
//==============================================================================
void invalidate_icache_range(ulong start_addr, ulong stop)  //wei add
{
#ifndef CONFIG_NAND_FLASH_BOOTING
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	while (1) 
	{
		cache_op(Hit_Invalidate_I, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
#endif
}
void flush_cache_sec(void)   //wei add
{
#ifndef CONFIG_NAND_FLASH_BOOTING
	#define START_ADDR 0x80000000
	flush_cache_range(START_ADDR, _DCACHE_SIZE);
	//flush_dcache_range(START_ADDR, START_ADDR+_DCACHE_SIZE);
	flush_scache_range(START_ADDR, START_ADDR+512*1024);
#endif	
	//winfred_wang
	invalidate_cache_sec();
	#ifndef CONFIG_NAND_FLASH_BOOTING
        #define START_ADDR 0x80000000

	//invalidate_scache_range(START_ADDR, 512*1024);
	#endif
}

//==============================================================================
void flush_cache(void)   //wei add
{
#ifndef CONFIG_NAND_FLASH_BOOTING
	#define START_ADDR 0x80000000
	flush_cache_range(START_ADDR, _DCACHE_SIZE);
	//flush_dcache_range(START_ADDR, START_ADDR+_DCACHE_SIZE);
#endif	
	//winfred_wang
	invalidate_cache();
}
//==============================================================================
void invalidate_cache(void)   //wei add
{
#ifndef CONFIG_NAND_FLASH_BOOTING
	#define START_ADDR 0x80000000
	invalidate_dcache_range(START_ADDR, START_ADDR+_DCACHE_SIZE);
	invalidate_icache_range(START_ADDR, START_ADDR+_ICACHE_SIZE);	
#endif
}
void invalidate_cache_sec(void)   //wei add
{
#ifndef CONFIG_NAND_FLASH_BOOTING
	#define START_ADDR 0x80000000
	invalidate_dcache_range(START_ADDR, START_ADDR+_DCACHE_SIZE);
	invalidate_icache_range(START_ADDR, START_ADDR+_ICACHE_SIZE);	
	invalidate_scache_range(START_ADDR,START_ADDR+512*1024);
#endif
}

//==============================================================================
//==============================================================================


#if 0 //wei add, write for 1074K test

/*
 ************************************************************************
 *         C O N F I G 1   R E G I S T E R   ( 1 6, SELECT 1 )          *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |M|  MMU Size |  IS |  IL |  IA |  DS |  DL |  DA |C|M|P|W|C|E|F| Config1
 * | |           |     |     |     |     |     |     |2|D|C|R|A|P|P|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_Config1		$16,1
#define R_C0_Config1		16
#define R_C0_SelConfig1		1

#define S_Config1M		31			/* Additional Config registers present (R) */
#define M_Config1M		(0x1 << S_Config1More)
#define S_Config1More		S_Config1M		/* OBSOLETE */
#define M_Config1More		(0x1 << S_Config1M)
#define S_Config1MMUSize 	25			/* Number of MMU entries - 1 (R) */
#define M_Config1MMUSize 	(0x3f << S_Config1MMUSize)
#define S_Config1IS		22			/* Icache sets per way (R) */
#define M_Config1IS		(0x7 << S_Config1IS)
#define S_Config1IL		19			/* Icache line size (R) */
#define M_Config1IL		(0x7 << S_Config1IL)
#define S_Config1IA		16			/* Icache associativity - 1 (R) */
#define M_Config1IA		(0x7 << S_Config1IA)
#define S_Config1DS		13			/* Dcache sets per way (R) */
#define M_Config1DS		(0x7 << S_Config1DS)
#define S_Config1DL		10			/* Dcache line size (R) */
#define M_Config1DL		(0x7 << S_Config1DL)
#define S_Config1DA		7			/* Dcache associativity (R) */
#define M_Config1DA		(0x7 << S_Config1DA)
#define S_Config1C2		6			/* Coprocessor 2 present (R) */
#define M_Config1C2		(0x1 << S_Config1C2)
#define S_Config1MD		5			/* Denotes MDMX present (R) */
#define M_Config1MD		(0x1 << S_Config1MD)
#define S_Config1PC		4			/* Denotes performance counters present (R) */
#define M_Config1PC		(0x1 << S_Config1PC)
#define S_Config1WR		3			/* Denotes watch registers present (R) */
#define M_Config1WR		(0x1 << S_Config1WR)
#define S_Config1CA		2			/* Denotes MIPS-16 present (R) */
#define M_Config1CA		(0x1 << S_Config1CA)
#define S_Config1EP		1			/* Denotes EJTAG present (R) */
#define M_Config1EP		(0x1 << S_Config1EP)
#define S_Config1FP		0			/* Denotes floating point present (R) */
#define M_Config1FP		(0x1 << S_Config1FP)




#if 1 //wei add
#define read_cp0_register_sel(source, sel)                         \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
	".set push\n\t"					\
	".set reorder\n\t"					\
	".word (0x40000000 | (" STR(%1<<16)") |"STR(source<<11)" |("STR(sel)")  ) \n\t"\
	"or %0,$9, $9 \n\t" 				\	
	".set pop\n\t"						\
        : "=r" (__res)		\
        : "i"(9)		\
        : "$9");          \
        __res;})
#endif

	#define GET_BITVAL(v,bitpos,pat) ((v& (pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf	
	#define RANG5 0x1f

//==============================================================================
int Get_icache_linesize()
{
	int v=read_cp0_register_sel(R_C0_Config1, R_C0_SelConfig1 );
	
	int IL=GET_BITVAL(v, S_Config1IL, RANG3);
	printf("IL=%d\n", IL);
	if(IL==0) return 0;
	
	int linesize= (2<<IL);
	return linesize;
}
//==============================================================================
int Get_icache_lines()
{

	int v=read_cp0_register_sel(R_C0_Config1, R_C0_SelConfig1 );

	int IA=GET_BITVAL(v, S_Config1IA, RANG3);
	printf("IA=%d\n", IA);	
	IA=IA+1;

	int IS=GET_BITVAL(v, S_Config1IS, RANG3);
	printf("IS=%d\n", IS);	
	int lines;
	if(IS==7)
	{	lines=IA*32;
	}
	else
	{	lines=IA*(64<<IS);
	}
	return lines;
}
//==============================================================================
int Get_icache_size()
{
	return Get_icache_linesize() * Get_icache_lines();
}
//==============================================================================
//==============================================================================
int Get_dcache_linesize()
{
#if 0
	int v=read_cp0_register_sel(R_C0_Config1, R_C0_SelConfig1 );
	
	int DL=GET_BITVAL(v, M_Config1DL, RANG3);
	printf("DL=%d\n", DL);	
	if(DL==0) return 0;
	
	int linesize= (2<<DL);
	return linesize;
#else
	return 32;
#endif
}
//==============================================================================
int Get_dcache_lines()
{

	int v=read_cp0_register_sel(R_C0_Config1, R_C0_SelConfig1 );

	int DA=GET_BITVAL(v, S_Config1DA, RANG3);
	printf("DA=%d\n", DA);	
	DA=DA+1;

	int DS=GET_BITVAL(v, S_Config1DS, RANG3);
	printf("DS=%d\n", DS);
	int lines;
	if(DS==7)
	{	lines=DA*32;
	}
	else
	{	lines=DA*(64<<DS);
	}
	return lines;
}
//==============================================================================
int Get_dcache_size()
{
	return Get_dcache_linesize() * Get_dcache_lines();
}
//==============================================================================
#endif


