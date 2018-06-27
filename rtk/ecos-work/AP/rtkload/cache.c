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
//#include <linux/autoconf.h>
//#include <linux/linkage.h>
//#include <asm/ptrace.h>
//#include <asm/mipsregs.h>
//#include <asm/rlxregs.h>
#include <regdef.h>
//#include <asm/addrspace.h>
//#include <asm/system.h>

//#include <cyg/hal/bspcpu.h>

/*
 * Memory segments (32bit kernel mode addresses)
 */
#define KUSEG                   0x00000000
#define KSEG0                   0x80000000
#define KSEG1                   0xa0000000
#define KSEG2                   0xc0000000
#define KSEG3                   0xe0000000

#define KSEGX(a)                ((a) & 0xe0000000)

#define ST0_IEC                 0x00000001

#if 1 //defined(CONFIG_RTL_819XD)
#define _ICACHE_SIZE		cpu_icache_size
#define _DCACHE_SIZE		cpu_dcache_size
#else
/* For Realtek RTL865XC Network platform series */
#define _ICACHE_SIZE		(16 * 1024)		/* 16K bytes */
#define _DCACHE_SIZE		(8 * 1024)		/* 8K bytes */
#endif

#define PROM_DEBUG

#ifdef PROM_DEBUG
extern int prom_printf(char *, ...);
#endif

/*Cyrus Tsai*/
void flush_cache(void);
void flush_icache(unsigned int start, unsigned int end);
void flush_dcache(unsigned int start, unsigned int end);
/*Cyrus Tsai*/

#define read_c0_status()	__read_32bit_c0_register($12, 0)
#define write_c0_status(val)	__write_32bit_c0_register($12, 0, val)

#define __read_32bit_c0_register(source, sel)				\
({ int __res;								\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mfc0\t%0, " #source "\n\t"			\
			: "=r" (__res));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mfc0\t%0, " #source ", " #sel "\n\t"		\
			".set\tmips0\n\t"				\
			: "=r" (__res));				\
	__res;								\
})

#define __write_32bit_c0_register(register, sel, value)			\
do {									\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mtc0\t%z0, " #register "\n\t"			\
			: : "Jr" (value));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mtc0\t%z0, " #register ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "Jr" (value));				\
} while (0)

void flush_cache(void)
{
#ifdef 	CONFIG_RTL_EB8186
	flush_dcache(0,0);		
	flush_icache(0,0);	
#endif	
#if defined(CONFIG_RTL_819X)
	flush_dcache(KSEG0, KSEG0+_DCACHE_SIZE);		
	flush_icache(KSEG0, KSEG0+_ICACHE_SIZE);	
#endif
}


/*Cyrus Tsai*/
void flush_icache(unsigned int start, unsigned int end)
{
#if defined(CONFIG_RTL_819X)
	/*
		Flush data cache at first in write-back platform.

		Ghhuang (2007/3/9):

		RD-Center suggest that we need to flush D-cache entries which
		might match to same address as I-cache ... when we flush
		I-cache.
		( Maybe some data is treated as data/instruction, both. )
	*/
	flush_dcache(start, end);

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

#endif

#ifdef CONFIG_RTL_EB8186
   unsigned long flags;
   volatile unsigned int reg;
    save_flags(flags);cli();
   reg=read_32bit_cp0_register(CP0_XCONTEXT);
   __asm__ volatile("nop");   
    __asm__ volatile("nop");
   write_32bit_cp0_register(CP0_XCONTEXT, (reg &(~0x2))); //write '0' to bit 0,1
   __asm__ volatile("nop");
   __asm__ volatile("nop");    
   write_32bit_cp0_register(CP0_XCONTEXT, (reg | 0x2)); //wirte '1' to bit 0, 1
   __asm__ volatile("nop");  
    __asm__ volatile("nop");
    restore_flags(flags);
#endif
}

void flush_dcache(unsigned int start, unsigned int end)
{
#ifdef CONFIG_RTL_EB8186
   unsigned long flags;
   volatile unsigned int reg;
    save_flags(flags);cli();
    reg=read_32bit_cp0_register(CP0_XCONTEXT);
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_32bit_cp0_register(CP0_XCONTEXT, (reg & (~0x1))); //write '0' to bit 0,1
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_32bit_cp0_register(CP0_XCONTEXT, (reg | 0x1)); //wirte '1' to bit 0, 1
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    restore_flags(flags);
#endif    
    
#if defined(CONFIG_RTL_819X)
	/* Flush D-Cache using its range */
	unsigned char *p;
	unsigned int size;
	unsigned int flags;
	unsigned int i;
   
	size = end - start;
   
	/* correctness check : flush all if any parameter is illegal */
// david	
//	if (	(size >= dcache_size) ||
	if ((size >= _DCACHE_SIZE) ||	(KSEGX(start) != KSEG0)	)
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
	} 
#if 1	
	else
	{
		/* Start to isolate cache space */
		p = (unsigned char *)start;
   
		flags = read_c0_status();
   
		/* isolate cache space */
		//write_c0_status( (ST0_ISC | flags) &~ ST0_IEC );
		write_c0_status( flags & ~ST0_IEC );
   
		for (i = 0; i < size; i += 0x040)
		{
			asm ( 	
			#ifdef OPEN_RSDK_RTL865x
				".word 0xbc750000\n\t"
				".word 0xbc750010\n\t"
				".word 0xbc750020\n\t"
				".word 0xbc750030\n\t"
			#endif
				"cache 0x15, 0x000(%0)\n\t"
			 	"cache 0x15, 0x010(%0)\n\t"
			 	"cache 0x15, 0x020(%0)\n\t"
			 	"cache 0x15, 0x030(%0)\n\t"
				:		/* No output registers */
				:"r"(p)		/* input : 'p' as %0 */
				);
			p += 0x040;
		}
   
		write_c0_status(flags);
	}    
#endif
#endif    
   
}
/*Cyrus Tsai*/
