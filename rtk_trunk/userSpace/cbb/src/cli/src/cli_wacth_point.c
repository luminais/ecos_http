/*************************************************************************
	> Copyright (C) 2017, Tenda Tech. Co., All Rights Reserved.
	> 文件名: cli_watch_point.c
	> 描述: 内存数据断点设置命令(目前不支持8196E)
	> 作者: fh 
	> 版本: 1.0
 ************************************************************************/




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



#define WMPCTL_KERNELENABLE (1<<1)
#define WMPCTL_WATCHMODE (0)
#define WMPCTL_MEMPROTECTIONMODE (1)
#define WMPCTL_ENTRYENABLE_ALL (0xff<<16)
#define WMPCTL_ENTRYENABLE_0 (0x1<<16)
#define WMPCTL_ENTRYENABLE_1 (0x2<<16)

#define WATCHHI_GLOBAL (1<<30)

#define LX0_WMPCTL $5



#define __read_32bit_lxc0_register(source, sel)				\
({ int __res;								\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mflxc0\t%0, " STR(source) "\n\t"			\
			: "=r" (__res));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mflxc0\t%0, " STR(source) ", " #sel "\n\t"		\
			".set\tmips0\n\t"				\
			: "=r" (__res));				\
	__res;								\
})

#define __write_32bit_lxc0_register(register, sel, value)			\
do {									\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mtlxc0\t%z0, " STR(register) "\n\t"			\
			: : "Jr" ((unsigned int)(value)));		\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mtlxc0\t%z0, " STR(register) ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "Jr" ((unsigned int)(value)));		\
} while (0)

#define __read_32bit_c0_register(source, sel)				\
({ int __res;								\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mfc0\t%0, " STR(source) "\n\t"			\
			: "=r" (__res));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mfc0\t%0, " STR(source) ", " #sel "\n\t"		\
			".set\tmips0\n\t"				\
			: "=r" (__res));				\
	__res;								\
})

#define __write_32bit_c0_register(register, sel, value)			\
do {									\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mtc0\t%z0, " STR(register) "\n\t"			\
			: : "Jr" (value));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mtc0\t%z0, " STR(register) ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "Jr" (value));				\
} while (0)


/************************************************************
Function:	 memwatch 		  
Description: 设置内存数据断点 			
Input:								
	需要监测的内存地址vaddr，监测类型flag，3bit表示，I，R，W分别
	表示取指、读、写（1表示写监测，7表示监测所有类型）					
Output: 								  
	无
Return: 								   
	无
Others: 								 
************************************************************/
void memwatch(char* vaddr, unsigned int flag)
{

	vaddr = (char*) (((unsigned int)vaddr) &0xfffffff8);
	flag &=0x7;
	__write_32bit_c0_register(CP0_WATCHLO, 0, (((unsigned int)vaddr)|flag));
	__write_32bit_c0_register(CP0_WATCHHI, 0, WATCHHI_GLOBAL);

	__write_32bit_lxc0_register(LX0_WMPCTL, 0, (WMPCTL_ENTRYENABLE_0| WMPCTL_WATCHMODE| WMPCTL_KERNELENABLE));

	if(__read_32bit_c0_register(CP0_WATCHLO,0) != (((unsigned int)vaddr)|flag))
	{
		printf("set watch point fail\n");
		printf("CP0_WATCHLO: %x\n",__read_32bit_c0_register(CP0_WATCHLO,0));
		printf("CP0_WATCHHI: %x\n",__read_32bit_c0_register(CP0_WATCHHI,0));
		printf("LX0_WMPCTL:  %x\n",__read_32bit_lxc0_register(LX0_WMPCTL,0));
	}

}




void mwatch(int argc, char *argv[])
{ 
	unsigned long src; 
	int type = 0; 
	if(argc == 2)
	{
		type = 0x1; 
	}
	else if(argc == 3)
	{
		type = atoi(argv[2]); 
	}
	else
	{
		printf(" error\n mwatch addr flag\n flag:w:1,r:2,i:4\n");
		return;
	}
	src = strtoul((const char*)(argv[1]), (char **)NULL, 16); 
	memwatch((char*)src,type); 
} 

