/* hfload.c
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License.  See the file "COPYING" in the main
 * directory of this archive for more details.
 *
 * Copyright (C) 2000, Jay Carlson
 */

/*
 * Boot loader main program.
 */
#include "config.h"
//#include <linux/autoconf.h>

#include <unistd.h>
//#include <linux/elf.h>
#include "hfload.h"

#define REG32(reg)   (*(volatile unsigned int   *)((unsigned int)reg))

#if defined(CONFIG_RTL_819X)
	#define BASE_ADDR 0xB8000000
	#define set_io_port_base(base)	\
		do { * (unsigned long *) &mips_io_port_base = (base); } while (0)	
	
	const unsigned long mips_io_port_base;
#endif

int file_offset;
unsigned long kernelStartAddr;
/*
int old_stack_pointer;

#define MAX_PHDRS_SIZE 8

Elf32_Ehdr header;
Elf32_Phdr phdrs[MAX_PHDRS_SIZE];
*/
typedef unsigned long  ulg;

extern void flush_cache(void);
extern int prom_printf(const char * fmt, ...);
extern ulg decompress_kernel(ulg output_start, ulg free_mem_ptr_p, ulg free_mem_ptr_end_p, int arch_id);
extern void start_kernel(unsigned long Addr); //just only for compiler happy

/*
void
zero_region(char *start, char *end)
{
	char *addr;
	int count;

	count = end - start;
#ifndef __DO_QUIET__
	printf("zeroing from %08x to to %08x, 0x%x bytes\n", start, end, count);
#endif

#ifndef FAKE_COPYING
	memset(start, 0, count);
#endif
}

void
load_phdr(Elf32_Phdr *phdr)
{
	char *addr, *end;
	
	seek_forward(phdr->p_offset);
	
	addr = (char *)phdr->p_vaddr;
	end = ((char *)addr) + phdr->p_memsz;
	
	copy_to_region(addr, phdr->p_filesz);
	
	addr = ((char *)addr) + phdr->p_filesz;
	
	zero_region(addr, end);
}
*/
// Using register: t6, t7
/*
#define REG32_W(addr,v)	li t6,v;\
			li t7,addr;\
			sw t6, 0(t7);
*/

/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/*
 * Coprocessor 0 register names
 */
#define CP0_INDEX $0
#define CP0_RANDOM $1
#define CP0_ENTRYLO0 $2
#define CP0_ENTRYLO1 $3
#define CP0_CONF $3
#define CP0_CONTEXT $4
#define CP0_PAGEMASK $5
#define CP0_WIRED $6
#define CP0_BADVADDR $8
#define CP0_COUNT $9
#define CP0_ENTRYHI $10
#define CP0_COMPARE $11
#define CP0_STATUS $12
#define CP0_CAUSE $13
#define CP0_EPC $14
#define CP0_PRID $15
#define CP0_CONFIG $16
#define CP0_LLADDR $17
#define CP0_WATCHLO $18
#define CP0_WATCHHI $19
#define CP0_XCONTEXT $20
#define CP0_FRAMEMASK $21
#define CP0_DIAGNOSTIC $22
#define CP0_PERFORMANCE $25
#define CP0_ECC $26
#define CP0_CACHEERR $27
#define CP0_TAGLO $28
#define CP0_TAGHI $29
#define CP0_ERROREPC $30

/*
 * Macros to access the system control coprocessor
 */
#define read_32bit_cp0_register(source)                         \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
	".set\tpush\n\t"					\
	".set\treorder\n\t"					\
        "mfc0\t%0,"STR(source)"\n\t"                            \
	".set\tpop"						\
        : "=r" (__res));                                        \
        __res;})

#define write_32bit_cp0_register(register,value)                \
        __asm__ __volatile__(                                   \
        "mtc0\t%0,"STR(register)"\n\t"				\
	"nop"							\
        : : "r" (value));

//int main(unsigned long stack_start_addr)
int main(int stack_start_addr, char *argv[]) //for compiler happy
{
	//int i;
	//Elf32_Ehdr *pHdr;
	
	file_offset = 0;

#if defined(CONFIG_RTL_819X)
 	set_io_port_base(BASE_ADDR);
#endif

#ifndef __DO_QUIET__
	printf("decompressing kernel:\n");
#endif

#ifdef CONFIG_RTL8197B_PANA
	extern int is_vmlinux_checksum_ok();
	
	REG32(0xB801900C) =  REG32(0xB801900C) & (~0x0400); //SYSSR, AllSoftwareReady=0
	if (!is_vmlinux_checksum_ok()) {
		printf("Linux image corrupted!\n");
		for (;;);
	}
#endif

#ifndef BZ2_COMPRESS
	decompress_kernel(UNCOMPRESS_OUT, stack_start_addr+4096, FREEMEM_END, 0);
#else
	decompress_kernel(UNCOMPRESS_OUT, stack_start_addr+4096, FREEMEM_END, 0);
#endif

#ifndef __DO_QUIET__
	printf("done decompressing kernel.\n");
#endif

	flush_cache();
	
	REG32(0xb8003000) = 0x0;
	REG32(0xb8003004) = 0xffffffff;
	REG32(0xb8003008) = 0x0;
	REG32(0xb800300c) = 0x0;
#ifndef CONFIG_RTL_8197F
	REG32(0xb8005104) = 0x80000000;
	REG32(0xb8000004) = 0x2;
#endif

/*	{
	unsigned int res;
	res = read_32bit_cp0_register(CP0_STATUS);
	printf("STATUS=%08x\n", res);
	res = read_32bit_cp0_register(CP0_CAUSE);
	printf("CAUSE=%08x\n", res);
	res = read_32bit_cp0_register(CP0_EPC);
	printf("EPC=%08x\n", res);
	res = read_32bit_cp0_register($31);
	printf("ra=%08x\n", res);
	res = read_32bit_cp0_register(CP0_BADVADDR);
	printf("BADVADDR=%08x\n", res);
	}
	
	REG32(0xa0000080) = 0;
	REG32(0xa0000084) = 0;
	REG32(0xa0000088) = 0;
	REG32(0xa000008c) = 0;
	REG32(0xa0000090) = 0;
	REG32(0xa0000094) = 0;
	REG32(0xa0000098) = 0;
	REG32(0xa000009c) = 0;
	REG32(0xa00000a0) = 0;
	REG32(0xa00000a4) = 0;
	REG32(0xa00000a8) = 0;
	REG32(0xa00000ac) = 0;*/
	/*
	REG32(0xb8003110) = 0x0;
	REG32(0xb8003114) = 0x30000000;
	REG32(0xb8010028) = 0x0;
	REG32(0xb801002c) = 0xffffffff;
	REG32(0xb8000010) = 0x245;
	REG32(0xb8000040) = 0x40000;
	REG32(0xb8001008) = 0x6cca0c80;
	REG32(0xb8001004) = 0x52480000;
	printf("0xb8003000=0x%08x\n", REG32(0xb8003000));
	printf("0xb8003004=0x%08x\n", REG32(0xb8003004));
	printf("0xb8003008=0x%08x\n", REG32(0xb8003008));
	printf("0xb800300c=0x%08x\n", REG32(0xb800300c));
	printf("0xb8003110=0x%08x\n", REG32(0xb8003110));
	printf("0xb8003114=0x%08x\n", REG32(0xb8003114));
	printf("0xb8010028=0x%08x\n", REG32(0xb8010028));
	printf("0xb801002c=0x%08x\n", REG32(0xb801002c));
	
	printf("0xb8005104=0x%08x\n", REG32(0xb8005104));
	printf("0xb8000004=0x%08x\n", REG32(0xb8000004));
	printf("0xb8000010=0x%08x\n", REG32(0xb8000010));
	printf("0xb8000040=0x%08x\n", REG32(0xb8000040));
	printf("0xb8001008=0x%08x\n", REG32(0xb8001008));
	printf("0xb8001004=0x%08x\n", REG32(0xb8001004));
	*/
#ifdef CONFIG_RTL_8197F
	kernelStartAddr = (kernelStartAddr & ~0xE0000000) | 0xA0000000;
#endif
	printf("start address: 0x%08x\n", kernelStartAddr);
	start_kernel(kernelStartAddr);
	//start_kernel(0x80000000);
	return 0;
}
