//=================================================================
//
//        traps.cxx
//
//=================================================================
extern "C"
{
#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/mips-regs.h>
#include <cyg/hal/mips_opcode.h>
#include <cyg/hal/basetype.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>           // exception ranges
}
#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.inl>

#ifdef CYGPKG_KERNEL_EXCEPTIONS
#ifdef CYGFUN_KERNEL_API_C

#ifdef CYGPKG_HAL_MIPS_MIPS32
#define ST0_IE			0x00000001
#define ST0_EXL			0x00000002
#define ST0_ERL			0x00000004
#define ST0_KSU			0x00000018
#define KSU_USER		0x00000010
#define KSU_SUPERVISOR		0x00000008
#define KSU_KERNEL		0x00000000
#define ST0_UX			0x00000020
#define ST0_SX			0x00000040
#define ST0_KX 			0x00000080
#define ST0_DE			0x00010000
#define ST0_CE			0x00020000
#else
#define ST0_IEC                 0x00000001
#define ST0_KUC			0x00000002
#define ST0_IEP			0x00000004
#define ST0_KUP			0x00000008
#define ST0_IEO			0x00000010
#define ST0_KUO			0x00000020
#endif

/*
  * Memory segments (32bit kernel mode addresses)
  */
 #define KUSEG                   0x00000000
 #define KSEG0                   0x80000000
 #define KSEG1                   0xa0000000
 #define KSEG2                   0xc0000000
 #define KSEG3                   0xe0000000
 #define K0BASE                 KSEG0
 
 /*
   * Returns the kernel segment base of a given address
   */
 #define KSEGX(a)                 ((((unsigned int)a)) & 0xe0000000)
 #define PHYSADDR(a)          ((((unsigned int)a)) & 0x1fffffff)
 #define KSEG0ADDR(a)        (((((unsigned int)a)) & 0x1fffffff) | KSEG0)
 #define KSEG1ADDR(a)        (((((unsigned int)a)) & 0x1fffffff) | KSEG1)
 #define KSEG2ADDR(a)        (((((unsigned int)a)) & 0x1fffffff) | KSEG2)
 #define KSEG3ADDR(a)        (((((unsigned int)a)) & 0x1fffffff) | KSEG3)

extern "C" void do_reset(int type);
#if defined (CONFIG_RTL_819X)
extern "C" void get_thread_info(void); 
#endif

#define TRACE diag_printf
typedef unsigned int	uint32;
typedef short int16;
typedef char  int8;
externC cyg_uint32 cyg_hal_exception_handler(HAL_SavedRegisters *regs);
externC unsigned int rtl_get_thread_entry();
externC void interrupt_end(
    cyg_uint32          isr_ret,
    cyg_uint32       *intr,
    HAL_SavedRegisters  *regs
    );

static void dumpCallStack(uint32 u32SP, uint32 u32RA, uint32 u32RA4Leaf)
{
	uint32 u32CurSP = u32SP, u32CurRetAddr = u32RA, u32CurrentFuncAddr = 0xFFFFFFFF, *pu32Pos, u32Depth = 0, u32Length, u32TaskEntry;
	int16  i16SPOffset, i16RAOffset;

#define MAX_CALLSTACK_DEPTH 16
#define MAX_CODE_SEARCH_LENGTH 4096

	TRACE("Call stack:\n");
	u32TaskEntry=rtl_get_thread_entry();
	/*u32TaskEntry = oskMyTaskEntry();*/
	/*assume __default_interrupt_vsr's length is 0x208*/
	while ((u32Depth < MAX_CALLSTACK_DEPTH) 
				&& (u32CurrentFuncAddr != (uint32)cyg_hal_exception_handler)
				&& (u32CurrentFuncAddr != (uint32)interrupt_end)
        && !((u32CurrentFuncAddr > (uint32)__default_interrupt_vsr) 
               &&(u32CurrentFuncAddr < (uint32)__default_interrupt_vsr+0x208))
               && (u32CurrentFuncAddr != u32TaskEntry))
	{
		pu32Pos = (uint32 *)u32CurRetAddr;

		/* Search current function start address */
		u32Length = 0;
		i16RAOffset = 0;
		i16SPOffset = 0;
		while ((u32Length < MAX_CODE_SEARCH_LENGTH) 
					&& (NULL != pu32Pos)) /* In case of exception */
		{ 
			if (0 == ((uint32)pu32Pos % 4)) /* MIPS32 */
			{
				if (0x27BD == (*pu32Pos >> 16))
				{
					/* 0x27BDxxxx -> addiu sp, sp, offset */
					i16SPOffset = *pu32Pos & 0xFFFF;
					if ((i16SPOffset < 0) /* found function start address */
						|| ((i16SPOffset > 0) && (0 == u32Depth)) /* maybe we have entered another function */
						)
					{
						i16SPOffset = -i16SPOffset;
						break;
					}
				}
				else if(0x23BD == (*pu32Pos >> 16))
				{
					/*0x23BDxxx -> addi sp,sp,-offset*/
					i16SPOffset = *pu32Pos & 0xFFFF;
					if ((i16SPOffset < 0) /* found function start address */
						|| ((i16SPOffset > 0) && (0 == u32Depth)) /* maybe we have entered another function */
						)
					{
						i16SPOffset = -i16SPOffset;
						break;
					}
				}
				else if (0xAFBF == (*pu32Pos >> 16))
				{
					/* 0xAFBFxxxx -> sw ra, offset(sp) */
					i16RAOffset = *pu32Pos & 0xFFFF;
				}

				u32Length++;
				pu32Pos--;
			}
			else if (1 == ((uint32)pu32Pos & 1))   /*MIPS16 mode*/
			{
				unsigned short instruction=*(unsigned short*)((unsigned int)pu32Pos-1);
				unsigned short extinst=*(unsigned short*)((unsigned int)pu32Pos-1-2);
				int16 i16TmpSPOffset;
				
				if (0x6300 == (instruction & 0xFF00))
				{
					/*case 1: 6300 may just be part of jal/jalx 's address. skip this case*/
					/*case 2: 63fb is part of jal/jal's addresss. but also a start of function
					  * maybe compiler's optimization   . so we only skip it whe RA offset not found.
					800e992e:       1ee0 63fb       jalx    805d8fec <net_clusters_area+0x141c2c>
					800e9930 <cyg_netint>:
					800e9930:       63fb            addiu   sp,-40
					800e9932 <$LCFI51>:
					//int unregister_netisr __P((int));
					static void
					cyg_netint(cyg_addrword_t param)
					{
					800e9932:       6209            sw      ra,36(sp)
					 */
					if(((extinst & 0xFC00) == 0x1C00 || (extinst & 0xFC00) == 0x1800) && (0 == i16RAOffset))
					{
						/*set to zero, let's continue to parse*/
						i16TmpSPOffset = 0;
					}
					/*	0x63xx  -> addiu sp, sp, offset*,  offset*8 */
					else if ((int8)(instruction & 0xFF) < 0)
					{
						/* we should first process normal mip16 instruction when offset is less than zero,
						 * otherwise, some strange code may affect the caculation of sp offset.
						 * eg: there is strange code at the end of function attachAuthTask, which will affect
						 * the function lib1x_nal_uninitialize.
						 */
						i16TmpSPOffset = (int8)(instruction & 0xFF);
						i16TmpSPOffset *= 8;
						//xprintfk("negative tmp sp offset %d\n", i16TmpSPOffset);
					}
					else if (0xF000 == (extinst & 0xF800))
					{
						i16TmpSPOffset = instruction & 0x1F;
						i16TmpSPOffset |= (extinst & 0x7E0);
						i16TmpSPOffset |= ((extinst & 0x1F) << 11);
						//xprintfk("extend inst 0x%04x, tmp sp offset %d\n", extinst, i16TmpSPOffset);
					}
					else
					{
						i16TmpSPOffset = (int8)(instruction & 0xFF);
						i16TmpSPOffset *= 8;
						//xprintfk("positive tmp sp offset %d\n", i16TmpSPOffset);
					}
					
					if ((i16TmpSPOffset > 0) && (0 == u32Depth))
					{
						/* we maybe have entered another function */
						break;
					}

					if (i16TmpSPOffset < 0)
					{
						if (0 == i16RAOffset)
						{
							/* instruction for saving ra is not found yet, so save extended sp offset */
							/* assume that there is no jump between storing ra and changing sp */
							TRACE("i16TmpSPOffset %x pu32Pos %x\n",i16TmpSPOffset,pu32Pos);
							u32CurSP += -i16TmpSPOffset;
							/*assume changing sp is the first command of a function so break.HF*/
							/*it there any exception ? ... */
							break;
						}
						else
						{
							/* Is there only one instruction to modify sp before saving ra? */
							i16SPOffset = -i16TmpSPOffset;
							break;
						}
					}
				}
				else if (0x6200 == (instruction&0xFF00))
				{	
					//we will search ra value
					/*0x62xx  --> sw ra, offset(sp), offset*4*/
					/*offset is unsigned for sw ra. HF*/
					i16RAOffset = (unsigned char)(instruction & 0xFF);
					i16RAOffset *= 4;
					//xprintfk("instruction=0x%04x, raoffset=%d\n", instruction, i16RAOffset);
				}
				
				pu32Pos= (uint32*)((unsigned int)pu32Pos- ((0xF000 == (extinst & 0xF800)) ? 4 : 2));
				u32Length++;
			}
			else /*abnormal case*/
			{
				pu32Pos = NULL;
				break;
			}
		}

		if ((MAX_CODE_SEARCH_LENGTH == u32Length) 
				|| (NULL == pu32Pos)
				)
		{
			/* can't find the start address of this function */
			TRACE("Invalid Address 0x%x.\n", u32CurRetAddr);
			break;
		}
		else
		{
			u32CurrentFuncAddr = (uint32)pu32Pos;
		}

    if ((0 == u32Depth) && (0 == i16RAOffset))
    {
        /* This is a leaf function, it doesn't save ra and doesn't create stack frame. */
        //TRACE("SP: 0x%08x, RA Offset: **, Ret Address: 0x%08x, Func Address: 0x********\n", u32CurSP, u32CurRetAddr);
        /*print the funcaddr .HF*/
        TRACE("SP: 0x%08x, RA Offset: **, Ret Address: 0x%08x, Func Address: 0x%08x\n", u32CurSP, u32CurRetAddr,u32CurrentFuncAddr);
				if (u32RA4Leaf)
				{
        	u32CurRetAddr = u32RA4Leaf;
				}
				else
				{
					break;
				}
    }
    else
    {
    	/* there may be some offset between the real start address of a function 
    	 * and the address of instruction 'addiu sp, sp, offset.
    	 * eg: do_auth_stop
    	 */
			TRACE("SP: 0x%08x, RA Offset: %d, Ret Address: 0x%08x, Func Address: 0x%08x\n", u32CurSP, i16RAOffset, u32CurRetAddr, u32CurrentFuncAddr);
			/* Set new ra and sp */
			u32CurRetAddr = *((unsigned int*)(u32CurSP + i16RAOffset));
			u32CurSP += i16SPOffset;
			//TRACE("New SP: 0x%08x, SP Offset: %d, New Ret Address: 0x%08x\n", u32CurSP, i16SPOffset, u32CurRetAddr);
    }

		u32Depth++;
	}

	return;
}

static inline void prepare_frametrace(HAL_SavedRegisters *regs)
{
#if 0
#ifndef CONFIG_KALLSYMS
        /*
         * Remove any garbage that may be in regs (specially func
         * addresses) to avoid show_raw_backtrace() to report them
         */
        memset(regs, 0, sizeof(*regs));
#endif
#endif
        __asm__ __volatile__(
                ".set push\n\t"
                ".set noat\n\t"
                "1: la $1, 1b\n\t"
                "sw $1, %0\n\t"
                "sw $29, %1\n\t"
                "sw $31, %2\n\t"
                ".set pop\n\t"
                : "=m" (regs->pc),
                "=m" (regs->d[29]), "=m" (regs->d[31])
                : : "memory");
}

externC int run_at_stack(unsigned int stack_base, unsigned int  stack)
{
	HAL_SavedRegisters regs;	
	prepare_frametrace(&regs);
	
	//diag_printf("regs.d[29] 0x%x\n",regs.d[29]);
	
	if((regs.d[29] < (unsigned int)stack) && (regs.d[29] >(unsigned int)stack_base))
	{
		return 1;
	}
	return 0;
}

extern unsigned int cyg_interrupt_stack_base;
extern unsigned int cyg_interrupt_stack;

externC int run_at_istack(void)
{
	//diag_printf("cyg_interrupt_stack_base 0x%x cyg_interrupt_stack 0x%x\n",(unsigned int)&cyg_interrupt_stack_base,(unsigned int)&cyg_interrupt_stack);	
	
	return run_at_stack((unsigned int)&cyg_interrupt_stack_base,(unsigned int)&cyg_interrupt_stack);
}

externC void dump_stack(void)
{
	HAL_SavedRegisters regs;
	/* Do NOT remove diag_printf
	  * To Generate
	  *  27bdff40        addiu   sp,sp,-192
         *  afbf00bc        sw      ra,188(sp)
         */
	diag_printf("Dump Stack:\n");
	prepare_frametrace(&regs);
	/*
	  *if mips16 build, the PC is mips32 since asm inline code.
	  * so using ra to dump call trace
	  */
	dumpCallStack(regs.d[29],regs.d[31],0);
}


static void show_stacktrace(HAL_SavedRegisters *regs)
{
	long stackdata;
	int i;
	unsigned long *sp = (unsigned long *)regs->d[29];

	diag_printf("Stack :");
	i = 0;
	while ((unsigned long) sp & (4096 - 1)) {
		if (i && ((i % (64 / 8)) == 0)) {
			diag_printf("\n");
			if (i <= 39)
				diag_printf("       ");
		}
		if (i > 39) {
			break;
		}

		stackdata = *sp;
		sp++;

		diag_printf(" %08lx", stackdata);
		i++;
	}
	/*if valid EPC, dump from pc else from ra*/
	if(KSEGX(regs->pc) == KSEG0)
		dumpCallStack(regs->d[29],regs->pc,regs->d[31]);
	else		
		dumpCallStack(regs->d[29],regs->d[31],0);
}

static void show_code(unsigned int *pc)
{
	long i;
	unsigned short *pc16 = NULL;

	diag_printf("Code:");

	if ((unsigned long)pc & 1)
		pc16 = (unsigned short *)((unsigned long)pc & ~1);
	for(i = -3 ; i < 6 ; i++) {
		unsigned int insn;
		
		if (pc16)
			insn = *(pc16 + i);
		else
			insn = *(pc + i);
		
		if (pc16)
			diag_printf("%c%04x%c", (i?' ':'<'), insn, (i?' ':'>'));
		else
			diag_printf("%c%08x%c", (i?' ':'<'), insn, (i?' ':'>'));
	}
}

static void __show_regs(HAL_SavedRegisters *regs)
{
	unsigned int cause = regs->cause;
	int i;

	/*
	 * Saved main processor registers
	 */
	for (i = 0; i < 32; ) {
		if ((i % 4) == 0) {
			if (i==0)
				diag_printf("          zero      at       v0       v1\n");
			else if (i==4)
				diag_printf("           a0       a1       a2       a3\n");
			else if (i==8)
				diag_printf("           t0       t1       t2       t3\n");
			else if (i==12)
				diag_printf("           t4       t5       t6       t7\n");
			else if (i==16)
				diag_printf("           s0       s1       s2       s3\n");
			else if (i==20)
				diag_printf("           s4       s5       s6       s7\n");
			else if (i==24)
				diag_printf("           t8       t9       k0       k1\n");
			else if (i==28)
				diag_printf("           gp       sp       fp       ra\n");
			diag_printf("$%02d   :", i);
		}
		if (i == 0)
			diag_printf(" %08x", 0U);
		else if (i == 26 || i == 27)
			diag_printf(" %8s", "");
		else
			diag_printf(" %08x", regs->d[i]);

		i++;
		if ((i % 4) == 0)
			diag_printf("\n");
	}

	diag_printf("Hi    : %08x\n", regs->hi);
	diag_printf("Lo    : %08x\n", regs->lo);

	/*
	 * Saved cp0 registers
	 */
	diag_printf("epc   : %08x\n", regs->pc);
	diag_printf("ra    : %08x\n", regs->d[31]);

	diag_printf("Status: %08x    ", regs->sr);

#ifdef CYGPKG_HAL_MIPS_MIPS32
	if (regs->sr & ST0_KX)
		diag_printf("KX ");
	if (regs->sr & ST0_SX)
		diag_printf("SX ");
	if (regs->sr & ST0_UX)
		diag_printf("UX ");
	switch (regs->sr & ST0_KSU) {
	case KSU_USER:
		diag_printf("USER ");
		break;
	case KSU_SUPERVISOR:
		diag_printf("SUPERVISOR ");
		break;
	case KSU_KERNEL:
		diag_printf("KERNEL ");
		break;
	default:
		diag_printf("BAD_MODE ");
		break;
	}
	if (regs->sr & ST0_ERL)
		diag_printf("ERL ");
	if (regs->sr & ST0_EXL)
		diag_printf("EXL ");
	if (regs->sr & ST0_IE)
		diag_printf("IE ");
#else
	if (regs->sr & ST0_KUO) diag_printf("KUo ");
	if (regs->sr & ST0_IEO) diag_printf("IEo ");
	if (regs->sr & ST0_KUP) diag_printf("KUp ");
	if (regs->sr & ST0_IEP) diag_printf("IEp ");
	if (regs->sr & ST0_KUC) diag_printf("KUc ");
	if (regs->sr & ST0_IEC) diag_printf("IEc ");
#endif

	diag_printf("\n");

	diag_printf("Cause : %08x\n", cause);

	cause = (cause & CAUSE_EXCMASK) >> CAUSE_EXCSHIFT;
	if (1 <= cause && cause <= 5)
		diag_printf("BadVA : %08x\n", regs->badvr);

	diag_printf("PrId  : %08x\n", regs->prid);
}

void show_exp_thread_info()
{
	Cyg_Thread *thread;
	thread = Cyg_Scheduler::get_current_thread();
	CYG_ADDRESS stack_ptr,stack_base;
	unsigned int stack_size;
	stack_size = thread->get_stack_size();
	stack_base = thread->get_stack_base();
	stack_ptr = thread->stack_ptr;
	diag_printf("--------- [%s] get exception !! ---------\n",thread->get_name());
	diag_printf(" ptr:%p base %p size:%d\n",stack_ptr,stack_base,(unsigned int)stack_size); 	
	if( (stack_ptr <= (stack_base + stack_size)) && (stack_ptr >= stack_base) )
	{
		CYG_ADDRESS ptr = stack_ptr;
		CYG_ADDRESS limit = stack_base + stack_size;
		unsigned char next_line=0;

		diag_printf(" limit:%p\n",limit);
		diag_printf("--------- map symbol only ---------\n");

		
		while(ptr<limit)
		{
			diag_printf(" [<%08x>]",*(CYG_WORD *)ptr);

			if(++next_line & 0x08)
			{
				next_line=0x00;
				diag_printf("\n");
			}
			
			ptr+=sizeof(CYG_WORD);
		}
		diag_printf("\n");
	}
}

static void show_registers(HAL_SavedRegisters *regs)
{
	Cyg_Thread *thread;
	
	__show_regs(regs);
	show_stacktrace(regs);
	show_code((unsigned int *) regs->pc);
	diag_printf("\n");
	show_exp_thread_info();
	thread = Cyg_Scheduler::get_current_thread();
	if( thread == 0 )
		diag_printf("Unknown thread\n");
	else {
		diag_printf("Thread %s (tid: %d), ", thread->get_name(), thread->get_unique_id());
#ifdef CYGIMP_THREAD_PRIORITY
    		diag_printf("CurPri: %d, SetPri: %d\n",thread->get_priority(),	thread->get_current_priority());
#else
    		diag_printf("CurPri: 0, SetPri: 0\n");
#endif
		diag_printf("Stack Base: 0x%x, Size: 0x%x, Used: ",
			thread->get_stack_base(), thread->get_stack_size());
#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
		diag_printf("0x%x\n", thread->measure_stack_usage());
#else
		diag_printf("Unknown\n");
#endif
	}
}

externC void dump_stacktrace(HAL_SavedRegisters *regs)
{
	show_stacktrace(regs);
}	

externC void dump_registers(HAL_SavedRegisters *regs)
{
	show_registers(regs);
}

//-----------------------------------------------------------------------------
#define HAVE_ADE_HANDLER
#ifdef HAVE_ADE_HANDLER
#ifdef CONFIG_MIPS16
enum mips16_major_op {
	mips16_jal_op = 0x3,
	rria_op = 0x8,
	i8_op = 0xc,
	mips16_lb_op = 0x10,
	mips16_lh_op,
	lwsp_op,
	mips16_lw_op,
	mips16_lbu_op,
	mips16_lhu_op,
	lwpc_op,
	mips16_sb_op = 0x18,
	mips16_sh_op,
	swsp_op,
	mips16_sw_op,
	mips16_rrr_op,
	mips16_rr_op,
	mips16_ext_op
};

/* Macro to differenate from non-MIPS16 opcode */
#define MIPS16OP(op)    ((op) | 0x40)
#endif

static unsigned int ade_count = 0;

static int __compute_return_epc(HAL_SavedRegisters *regs)
{
	unsigned int *addr;
	long epc;
	InstFmt insn;
	unsigned int opcode;
#ifdef CONFIG_MIPS16
	union {
		unsigned int *pc;
		unsigned short *mips16pc;
	} pcptr;
	unsigned int target;
	unsigned int mips16 = 0;

	epc = regs->pc;
	if (epc & 1)
		mips16 = 1;
	else if (epc & 2)
		goto unaligned;

	if (!mips16) {
		/*
		 * Read the instruction
		 */
		addr = (unsigned int *) epc;
		insn.word = *addr;
		opcode = insn.IType.op;
	}
	else {
		pcptr.pc = (unsigned int *)((unsigned int)epc & 0xfffffffe);
		/*
		 * This load never faults.
		 */
		insn.halfword[0] = *pcptr.mips16pc;
		opcode = MIPS16OP(insn.mips16_gen_format.op);
	}
#else
	epc = regs->pc;
	if (epc & 3)
		goto unaligned;
	/*
	 * Read the instruction
	 */
	addr = (unsigned int *) epc;
	insn.word = *addr;
	opcode = insn.IType.op;
#endif

	switch (opcode) {
	/*
	 * jr and jalr are in RType format.
	 */
	case OP_SPECIAL:
		switch (insn.RType.func) {
		case OP_JALR:
			regs->d[insn.RType.rd] = epc + 8;
			/* Fall through */
		case OP_JR:
			regs->pc = regs->d[insn.RType.rs];
			break;
		}
		break;

	/*
	 * This group contains:
	 * bltz_op, bgez_op, bltzl_op, bgezl_op,
	 * bltzal_op, bgezal_op, bltzall_op, bgezall_op.
	 */
	case OP_REGIMM:
		switch (insn.IType.rt) {
	 	case OP_BLTZ:
		case OP_BLTZL:
			if ((long)regs->d[insn.IType.rs] < 0)
				epc = epc + 4 + (insn.IType.imm << 2);
			else
				epc += 8;
			regs->pc = epc;
			break;

		case OP_BGEZ:
		case OP_BGEZL:
			if ((long)regs->d[insn.IType.rs] >= 0)
				epc = epc + 4 + (insn.IType.imm << 2);
			else
				epc += 8;
			regs->pc = epc;
			break;

		case OP_BLTZAL:
		case OP_BLTZALL:
			regs->d[31] = epc + 8;
			if ((long)regs->d[insn.IType.rs] < 0)
				epc = epc + 4 + (insn.IType.imm << 2);
			else
				epc += 8;
			regs->pc = epc;
			break;

		case OP_BGEZAL:
		case OP_BGEZALL:
			regs->d[31] = epc + 8;
			if ((long)regs->d[insn.IType.rs] >= 0)
				epc = epc + 4 + (insn.IType.imm << 2);
			else
				epc += 8;
			regs->pc = epc;
			break;
		}
		break;

	/*
	 * These are unconditional and in JType.
	 */
	case OP_JAL:
	case OP_JALX:
		regs->d[31] = regs->pc + 8;
	case OP_J:
		epc += 4;
		epc >>= 28;
		epc <<= 28;
		epc |= (insn.JType.target << 2);
		if (opcode == OP_JALX)
			epc |= 1;
		regs->pc = epc;
		break;

	/*
	 * These are conditional and in IType.
	 */
	case OP_BEQ:
	case OP_BEQL:
		if (regs->d[insn.IType.rs] ==
		    regs->d[insn.IType.rt])
			epc = epc + 4 + (insn.IType.imm << 2);
		else
			epc += 8;
		regs->pc = epc;
		break;

	case OP_BNE:
	case OP_BNEL:
		if (regs->d[insn.IType.rs] !=
		    regs->d[insn.IType.rt])
			epc = epc + 4 + (insn.IType.imm << 2);
		else
			epc += 8;
		regs->pc = epc;
		break;

	case OP_BLEZ: /* not really IType */
	case OP_BLEZL:
		/* rt field assumed to be zero */
		if ((long)regs->d[insn.IType.rs] <= 0)
			epc = epc + 4 + (insn.IType.imm << 2);
		else
			epc += 8;
		regs->pc = epc;
		break;

	case OP_BGTZ:
	case OP_BGTZL:
		/* rt field assumed to be zero */
		if ((long)regs->d[insn.IType.rs] > 0)
			epc = epc + 4 + (insn.IType.imm << 2);
		else
			epc += 8;
		regs->pc = epc;
		break;

#ifdef CONFIG_MIPS16
	/*
	 * These are unconditional MIPS16 JALX, JAL
	 */
	case MIPS16OP(mips16_jal_op): /* for jal(x) */
		insn.halfword[1] = *(pcptr.mips16pc+2);
		target = insn.halfword[1] |
		         (insn.mips16_jalx_format.imm2 << 5) |
		         (insn.mips16_jalx_format.imm1 >> 5);
		regs->d[31] = regs->pc + 6;
		epc += 4;
		epc >>= 28;
		epc <<= 28;
		epc |= (target << 2);
		if (!insn.mips16_jalx_format.x)
			epc |= 1;
		regs->pc = epc;
		break;

	/*
	 * These are unconditional MIPS16 JALR, JR1, JR2
	 */
	case MIPS16OP(mips16_rr_op):
		if (insn.mips16_jalr_format.l) {
			regs->d[31] = epc + 4;
			regs->pc = regs->d[insn.mips16_jalr_format.rx];
		} else {
			if (insn.mips16_jalr_format.ra) {
				regs->pc = regs->d[31];
			} else {
				regs->pc = regs->d[insn.mips16_jalr_format.rx];
			}
		}
		break;
#endif
	}

	return 0;

unaligned:
	diag_printf("unaligned epc\n");
	//SIGBUS
	return -1;
}

static inline int compute_return_epc(HAL_SavedRegisters *regs)
{
	if (!(regs->cause & CAUSE_BD)) {
		if (regs->pc & 0x1)
			regs->pc += 2;
		else
			regs->pc += 4;
		return 0;
	}

	return __compute_return_epc(regs);
}

static int emulate_load_store_insn(HAL_SavedRegisters *regs, void *addr, unsigned int *pc)
{
	InstFmt insn;
	unsigned long value;
#ifdef CONFIG_MIPS16
	union {
		unsigned int *pc;
		unsigned short *mips16pc;
	} pcptr;
#endif
	unsigned int opcode;
	unsigned int mips16 = 0;

#ifdef CONFIG_MIPS16
	if (regs->pc & 0x1)
		mips16 = 1;
#endif

	if (!mips16) {
		/*
		 * This load never faults.
		 */
		insn.word = *pc;
		opcode = insn.IType.op;
	}
#ifdef CONFIG_MIPS16
	else {
		pcptr.pc = (unsigned int *)((unsigned int)regs->pc & 0xfffffffe);
		/*
		 * This load never faults.
		 */
		insn.halfword[0] = *pcptr.mips16pc;
		opcode = insn.mips16_gen_format.op;

		if (opcode == mips16_ext_op) {
			insn.halfword[0] = *(pcptr.mips16pc + 1);
			regs->pc += 2;
		}
		else if (opcode == mips16_rr_op)
			insn.halfword[0] = *(pcptr.mips16pc + 1);
		else if (opcode == mips16_jal_op)
			insn.halfword[0] = *(pcptr.mips16pc + 2);

		opcode = MIPS16OP(insn.mips16_gen_format.op);
	}
#endif

	switch (opcode) {
	/*
	 * These are instructions that a compiler doesn't generate.  We
	 * can assume therefore that the code is MIPS-aware and
	 * really buggy.  Emulating these instructions would break the
	 * semantics anyway.
	 */
	case OP_LL:
	case OP_SC:
	case OP_LB:
	case OP_LBU:
	case OP_SB:

	case OP_LWL:
	case OP_LWR:
	case OP_SWL:
	case OP_SWR:
		goto sigbus;

	/*
	 * The remaining opcodes are the ones that are really of interest.
	 */
#ifdef CONFIG_MIPS16
	case MIPS16OP(mips16_lh_op):
#endif
	case OP_LH:
		__asm__ __volatile__ (
			".set\tnoat\n"
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			"lb\t%0, 0(%1)\n"
			"lbu\t$1, 1(%1)\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			"lb\t%0, 1(%1)\n"
			"lbu\t$1, 0(%1)\n\t"
#endif
			"sll\t%0, 0x8\n\t"
			"or\t%0, $1\n\t"
			".set\tat\n\t"
			: "=&r" (value)
			: "r" (addr));

		compute_return_epc(regs);
		if (!mips16) {
			regs->d[insn.IType.rt] = value;
		} else {
			regs->d[insn.mips16_rri_format.ry] = value;
		}
		break;

#ifdef CONFIG_MIPS16
	case MIPS16OP(lwpc_op):
	case MIPS16OP(lwsp_op):
	case MIPS16OP(mips16_lw_op):
#endif
	case OP_LW:
		__asm__ __volatile__ (
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			"lwl\t%0, (%1)\n"
			"lwr\t%0, 3(%1)\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			"lwl\t%0, 3(%1)\n"
			"lwr\t%0, (%1)\n\t"
#endif
			: "=&r" (value)
			: "r" (addr));

		compute_return_epc(regs);
		if (!mips16) {
			regs->d[insn.IType.rt] = value;
		}
#ifdef CONFIG_MIPS16
		else {
			if (opcode == MIPS16OP(lwpc_op) ||
			    opcode == MIPS16OP(lwsp_op))
				regs->d[insn.mips16_ri_format.rx] = value;
			else
				regs->d[insn.mips16_rri_format.ry] = value;
		}
#endif
		break;

#ifdef CONFIG_MIPS16
	case MIPS16OP(mips16_lhu_op):
#endif
	case OP_LHU:
		__asm__ __volatile__ (
			".set\tnoat\n"
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			"lbu\t%0, 0(%1)\n"
			"lbu\t$1, 1(%1)\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			"lbu\t%0, 1(%1)\n"
			"lbu\t$1, 0(%1)\n\t"
#endif
			"sll\t%0, 0x8\n\t"
			"or\t%0, $1\n\t"
			".set\tat\n\t"
			: "=&r" (value)
			: "r" (addr));

		compute_return_epc(regs);
		if (!mips16) {
			regs->d[insn.IType.rt] = value;
		} else {
			regs->d[insn.mips16_rri_format.ry] = value;
		}
		break;

#ifdef CONFIG_MIPS16
	case MIPS16OP(mips16_sh_op):
#endif
	case OP_SH:
		if (!mips16)
			value = regs->d[insn.IType.rt];
		else
			value = regs->d[insn.mips16_rri_format.ry];

		__asm__ __volatile__ (
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			".set\tnoat\n"
			"sb\t%0, 1(%1)\n\t"
			"srl\t$1, %0, 0x8\n"
			"sb\t$1, 0(%1)\n\t"
			".set\tat\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			".set\tnoat\n"
			"sb\t%0, 0(%1)\n\t"
			"srl\t$1, %0, 0x8\n"
			"sb\t$1, 1(%1)\n\t"
			".set\tat\n\t"
#endif
			: 
			: "r" (value), "r" (addr));

		compute_return_epc(regs);
		break;

#ifdef CONFIG_MIPS16
	case MIPS16OP(i8_op):
	case MIPS16OP(swsp_op):
	case MIPS16OP(mips16_sw_op):
#endif
	case OP_SW:
		if (!mips16)
			value = regs->d[insn.IType.rt];
#ifdef CONFIG_MIPS16
		else {
			if (opcode == MIPS16OP(swsp_op)) {
				value = regs->d[insn.mips16_ri_format.rx];
			} else if (opcode == MIPS16OP(i8_op)) {
				value = regs->d[31];
			} else {
				value = regs->d[insn.mips16_rri_format.ry];
			}
		}
#endif

		__asm__ __volatile__ (
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			"swl\t%0,(%1)\n"
			"swr\t%0, 3(%1)\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			"swl\t%0, 3(%1)\n"
			"swr\t%0, (%1)\n\t"
#endif
			: 
			: "r" (value), "r" (addr));

		compute_return_epc(regs);
		break;

	default:
		/*
		 * Pheeee...  We encountered an yet unknown instruction or
		 * cache coherence problem.  Die sucker, die ...
		 */
		goto sigill;
	}
	ade_count++;
	return 1; //handled already

sigbus:
	diag_printf("Unhandled unaligned access op=%d\n", insn.IType.op);
	//SIGBUS
	return 0;

sigill:
	diag_printf("Unhandled unaligned access or invalid instruction\n");
	//SIGILL
	return 0;
}

static int ade_handler(HAL_SavedRegisters *regs)
{
	unsigned int *pc;

	/*
	 * Did we catch a fault trying to load an instruction?
	 */
	if (regs->badvr == regs->pc)
		goto sigbus;

	// compute exception epc
	if (regs->cause & CAUSE_BD)
		pc = (unsigned int *)(regs->pc + 4);
	else
		pc = (unsigned int *)regs->pc;

#ifdef CONFIG_RTL_819X
	/*Only for Ecos*/
	if((KSEGX(regs->badvr) != KSEG0) && (KSEGX(regs->badvr) != KSEG1) )
		show_registers(regs);
#endif
	return emulate_load_store_insn(regs, (void *)regs->badvr, pc);

sigbus:
	diag_printf("unaligned instruction access\n");
	//SIGBUS
	return 0;

	/*
	 * XXX On return from the signal handler we should advance the epc
	 */
}
#endif
#if 0
//在此处定义内存数据断点功能宏，功能默认开启
//#define __CONFIG_WATCH_POINT__
//内存数据断点调试宏，开启后会增加一些打印，目前内存数据断点不支持8196E
//#define WATCH_DEBUG

#ifdef __CONFIG_WATCH_POINT__

#ifdef WATCH_DEBUG
#define DEBUG(format, ...) diag_printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif

extern void * memcpy(void * dest,const void *src,size_t count);
char* watch_string[]={"write", "read", "instruction"};
#define LX0_WMPCTL $5
#define LX0_WMPSTATUS $6
#define LX0_WMPVADDR $7
#define STR(x)  __STR(x)
#define __STR(x)  #x

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


static int dowatch_load_store_insn(HAL_SavedRegisters *regs, void *addr, unsigned int *pc)
{
	InstFmt insn;
	unsigned long value;
	union {
		unsigned int *pc;
		unsigned short *mips16pc;
	} pcptr;
	unsigned int opcode;
	unsigned int mips16 = 0;
	if (regs->pc & 0x1)
		mips16 = 1;

	if (!mips16) {
		/*
		 * This load never faults.
		 */
		insn.word = *pc;
		opcode = insn.IType.op;
		DEBUG("not mips16 %d,opcode = %d\n",__LINE__,opcode);
	} else {
		pcptr.pc = (unsigned int *)((unsigned int)regs->pc & 0xfffffffe);
		/*
		 * This load never faults.
		 */
		insn.halfword[0] = *pcptr.mips16pc;
		opcode = insn.mips16_gen_format.op;
		DEBUG("mips16 %d,opcode = %d\n",__LINE__,opcode);
		if (opcode == mips16_ext_op) {
			insn.halfword[0] = *(pcptr.mips16pc + 1);
			regs->pc += 2;
		}
		else if (opcode == mips16_rr_op)
			insn.halfword[0] = *(pcptr.mips16pc + 1);
		else if (opcode == mips16_jal_op)
			insn.halfword[0] = *(pcptr.mips16pc + 2);

		opcode = MIPS16OP(insn.mips16_gen_format.op);
		DEBUG("mips16 %d,opcode = %d\n",__LINE__,opcode);
	}

	switch (opcode) {
	case OP_LBU:
		DEBUG("OP_LBU\n");
	case MIPS16OP(mips16_lbu_op):
		DEBUG("mips16_lbu_op\n");
		value = 0; 
		memcpy(&value,addr,1);
		compute_return_epc(regs);
		value = value >> 24;
		if (!mips16) {
			regs->d[insn.IType.rt] = value;
		} else {
			regs->d[insn.mips16_rri_format.ry] = value;
		}
	break;
	case OP_SB:
		DEBUG("OP_SB\n");
	case MIPS16OP(mips16_sb_op):
		DEBUG("mips16_sb_op\n");
		if (!mips16)
			value = regs->d[insn.IType.rt];
		else {
			if (opcode == MIPS16OP(swsp_op)) {
				value = regs->d[insn.mips16_ri_format.rx];
			} else if (opcode == MIPS16OP(i8_op)) {
				value = regs->d[31];
			} else {
				value = regs->d[insn.mips16_rri_format.ry];
			}
		}
		value = (value << 24) & (0xFF000000);
		memcpy(addr,&value,1);
		compute_return_epc(regs);
	break;
	case MIPS16OP(rria_op):
		DEBUG("rria_op\n");
	break;
	case OP_LB:
		DEBUG("OP_LB\n");
	case MIPS16OP(mips16_lb_op):
		DEBUG("mips16_lb_op\n");
		value = 0; 
		memcpy(&value,addr,1);
		compute_return_epc(regs);
		value = value >> 24;
		if (!mips16) {
			regs->d[insn.IType.rt] = value;
		} else {
			regs->d[insn.mips16_rri_format.ry] = value;
		}
	break;
	case MIPS16OP(mips16_rrr_op):
		DEBUG("mips16_rrr_op\n");
	break;

	/*
	 * These are instructions that a compiler doesn't generate.  We
	 * can assume therefore that the code is MIPS-aware and
	 * really buggy.  Emulating these instructions would break the
	 * semantics anyway.
	 */
	case OP_LL:
	case OP_SC:

	case OP_LWL:
	case OP_LWR:
	case OP_SWL:
	case OP_SWR:
		goto sigbus;

	/*
	 * The remaining opcodes are the ones that are really of interest.
	 */
	case MIPS16OP(mips16_lh_op):
	case OP_LH:
		__asm__ __volatile__ (
			".set\tnoat\n"
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			"lb\t%0, 0(%1)\n"
			"lbu\t$1, 1(%1)\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			"lb\t%0, 1(%1)\n"
			"lbu\t$1, 0(%1)\n\t"
#endif
			"sll\t%0, 0x8\n\t"
			"or\t%0, $1\n\t"
			".set\tat\n\t"
			: "=&r" (value)
			: "r" (addr));

		compute_return_epc(regs);
		if (!mips16) {
			regs->d[insn.IType.rt] = value;
		} else {
			regs->d[insn.mips16_rri_format.ry] = value;
		}
		break;

	case MIPS16OP(lwpc_op):
	case MIPS16OP(lwsp_op):
	case MIPS16OP(mips16_lw_op):
	case OP_LW:
		__asm__ __volatile__ (
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			"lwl\t%0, (%1)\n"
			"lwr\t%0, 3(%1)\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			"lwl\t%0, 3(%1)\n"
			"lwr\t%0, (%1)\n\t"
#endif
			: "=&r" (value)
			: "r" (addr));

		compute_return_epc(regs);
		if (!mips16) {
			regs->d[insn.IType.rt] = value;
		} else {
			if (opcode == MIPS16OP(lwpc_op) ||
			    opcode == MIPS16OP(lwsp_op))
				regs->d[insn.mips16_ri_format.rx] = value;
			else
				regs->d[insn.mips16_rri_format.ry] = value;
		}
		break;

	case MIPS16OP(mips16_lhu_op):
	case OP_LHU:
		__asm__ __volatile__ (
			".set\tnoat\n"
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			"lbu\t%0, 0(%1)\n"
			"lbu\t$1, 1(%1)\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			"lbu\t%0, 1(%1)\n"
			"lbu\t$1, 0(%1)\n\t"
#endif
			"sll\t%0, 0x8\n\t"
			"or\t%0, $1\n\t"
			".set\tat\n\t"
			: "=&r" (value)
			: "r" (addr));

		compute_return_epc(regs);
		if (!mips16) {
			regs->d[insn.IType.rt] = value;
		} else {
			regs->d[insn.mips16_rri_format.ry] = value;
		}
		break;

	case MIPS16OP(mips16_sh_op):
	case OP_SH:
		if (!mips16)
			value = regs->d[insn.IType.rt];
		else
			value = regs->d[insn.mips16_rri_format.ry];

		__asm__ __volatile__ (
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			".set\tnoat\n"
			"sb\t%0, 1(%1)\n\t"
			"srl\t$1, %0, 0x8\n"
			"sb\t$1, 0(%1)\n\t"
			".set\tat\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			".set\tnoat\n"
			"sb\t%0, 0(%1)\n\t"
			"srl\t$1, %0, 0x8\n"
			"sb\t$1, 1(%1)\n\t"
			".set\tat\n\t"
#endif
			: 
			: "r" (value), "r" (addr));
		compute_return_epc(regs);
		break;

	case MIPS16OP(i8_op):
	case MIPS16OP(swsp_op):
	case MIPS16OP(mips16_sw_op):
	case OP_SW:
		if (!mips16)
		{
			DEBUG("rt:%d ",insn.IType.rt);
			value = regs->d[insn.IType.rt];
		}
		else {
			if (opcode == MIPS16OP(swsp_op)) {
				DEBUG("rx:%d ",insn.mips16_ri_format.rx);
				if(insn.mips16_ri_format.rx == 0 || insn.mips16_ri_format.rx ==1)
					value = regs->d[insn.mips16_ri_format.rx+16];
				else
					value = regs->d[insn.mips16_ri_format.rx];
			} else if (opcode == MIPS16OP(i8_op)) {
				value = regs->d[31];
			} else {
				DEBUG("ry:%d  ",insn.mips16_rri_format.ry);
				value = regs->d[insn.mips16_rri_format.ry];
			}
		}
	#if 1
		__asm__ __volatile__ (
#if (CYG_BYTEORDER == CYG_MSBFIRST) //BIG_ENDIAN
			"swl\t%0,(%1)\n"
			"swr\t%0, 3(%1)\n\t"
#endif
#if (CYG_BYTEORDER == CYG_LSBFIRST) //LITTLE_ENDIAN
			"swl\t%0, 3(%1)\n"
			"swr\t%0, (%1)\n\t"
#endif
			: 
			: "r" (value), "r" (addr));
	#else
		memcpy(addr,&value,4);
	#endif
		compute_return_epc(regs);
		break;

	default:
		/*
		 * Pheeee...  We encountered an yet unknown instruction or
		 * cache coherence problem.  Die sucker, die ...
		 */

		goto sigill;
	}


	diag_printf("value: %x\n",value);
	return 1; 

sigbus:
	diag_printf("Unhandled unaligned access op=%d\n", insn.IType.op);
	return 0;

sigill:
	diag_printf("Unhandled unaligned access or invalid instruction\n");
	return 0;
}



int do_watch(HAL_SavedRegisters *regs)
{
	int addr, status, tmp,watch;
	char *print_string;
	InstFmt insn;
	unsigned long value;
	unsigned int *pc;

	addr = __read_32bit_lxc0_register( LX0_WMPVADDR, 0);
	status = __read_32bit_lxc0_register( LX0_WMPSTATUS, 0);

	tmp = __read_32bit_lxc0_register(LX0_WMPCTL, 0);
	watch = tmp;

	tmp = tmp & (~(status&0xff0000));
	__write_32bit_lxc0_register(LX0_WMPCTL, 0, tmp);

	
	pc = (unsigned int *)( regs->pc + ((regs->cause & CAUSE_BD) ? 4 : 0));

	diag_printf("watch ADDR:%x, ENTRY:%x ", addr, (status&0xff0000)>>16, status&0x7);
	

	if (status&1) print_string=watch_string[0];
	if (status&2) print_string=watch_string[1];
	if (status&4) print_string=watch_string[2];
	diag_printf("pc: %x cause by: %s ", pc,print_string);
	DEBUG("\n");

	dowatch_load_store_insn(regs, (void *)addr, pc);

	return 0;
}

#endif// __CONFIG_WATCH_POINT__
#endif
//-----------------------------------------------------------------------------

static void exc_handler0(cyg_addrword_t data, cyg_code_t number, cyg_addrword_t info)
{
#ifdef HAVE_ADE_HANDLER
	if ((number==4) || (number==5)) {
		//diag_printf("ExcNum=%d\n", number);
		
		//show_registers((HAL_SavedRegisters *)info);
		
		// handle adel and ades
		if (ade_handler((HAL_SavedRegisters *)info)) {
			return;
		}
	}
#endif

	diag_printf("\nExcCode: %d ", number);
	switch (number)
	{
	case 0:
		diag_printf("Int\n");
		break;
	case 1:
		diag_printf("TLBMOD\n");
		break;
	case 2:
		diag_printf("TLBL\n");
		break;
	case 3:
		diag_printf("TLBS\n");
		break;
	case 4:
		diag_printf("AdEL\n");
		break;
	case 5:
		diag_printf("AdES\n");
		break;
#ifdef CYGPKG_HAL_MIPS_MIPS32
	case 6:
		diag_printf("IBE\n");
		break;
	case 7:
		diag_printf("DBE\n");
		break;
#endif
	case 8:
		diag_printf("Sys\n");
		break;
	case 9:
		diag_printf("Bp\n");
		break;
	case 10:
		diag_printf("RI\n");
		break;
	case 11:
		diag_printf("CpU\n");
		break;
	case 12:
		diag_printf("Ov\n");
		break;
#ifdef CYGPKG_HAL_MIPS_MIPS32
	case 13:
		diag_printf("TRAP\n");
		break;
	case 14:
		diag_printf("Div_by_0\n"); //??
		break;
	case 15:
		diag_printf("FPE\n");
		break;
	case 18:
		diag_printf("C2E\n");
		break;
	case 22:
		diag_printf("MDMX:\n");
		break;
	case 23:
		diag_printf("Watch\n");
		break;
	case 24:
		diag_printf("MCheck\n");
		break;
	case 25:
		diag_printf("Thread\n");
		break;
	case 26:
		diag_printf("DSP\n");
		break;
	case 30:
		diag_printf("CacheErr\n");
		break;
#endif
	default:
		diag_printf("Unknown\n");
		break;
	}
	show_registers((HAL_SavedRegisters *)info);
	#if defined (CONFIG_RTL_819X)
	get_thread_info(); 
	#endif
	//reboot immediately
	do_reset(0);
	while (1);
}

//-----------------------------------------------------------------------------
#if 0
// The following function attempts to cause an exception in various
// hacky ways.  It is machine dependent what exception is generated.
// It does reads rather than writes hoping not to corrupt anything
// important.
static int
cause_fpe(int num)
{
    double a;

    a = 1.0/num;                        // Depending on FPU emulation and/or
                                        // the FPU architecture, this may
                                        // cause an exception.
                                        // (float division by zero)

    return ((int)a)/num;                // This may cause an exception if
                                        // the architecture supports it.
                                        // (integer division by zero).
} // cause_fpe()
#endif

externC void cause_exception(int type)
{
#ifdef HAVE_ADE_HANDLER
	if (0==type) {
		diag_printf("ade_count=%u\n", ade_count);
		return;
	}
#endif

#if 0
	{
	char m[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	int *w = (int *) (m + 1);
	short *h = (short *) (m + 5);
	int i;

	diag_printf("w=%p *w=%08x\n", w, *w);
	*w = 0x0a0b0c0d;
	for(i = 0; i <= 7; i++)
		diag_printf("%02x ", m[i]);
	diag_printf("\n");

	diag_printf("h=%p *h=%04x\n", h, *h);
	*h = 0x0809;
	for(i = 0; i <= 7; i++)
		diag_printf("%02x ", m[i]);
	diag_printf("\n");
	}
#endif

#if 0
	{
	int x;
	unsigned int p=0;
	//unsigned int p=0x80703000;
	//unsigned int p=0x80703001;

	// First try for an address exception (unaligned access exception
	// or SEGV/BUS exceptions)
	do {
		x=*(volatile int *)(p-1);
		p+=0x100000;
	} while(p != 0);

	// Next try an integer or floating point divide-by-zero exception.
	cause_fpe(0);
	}
#endif
}

//-----------------------------------------------------------------------------
externC void install_exception_handler(void)
{
	cyg_exception_handler_t *old_handler;
	cyg_addrword_t old_data;

	cyg_exception_set_handler(
		CYGNUM_HAL_EXCEPTION_MAX, 
		&exc_handler0,
		0,
		&old_handler,
		&old_data);
}		
#if 0
//add by z10312 移植应用程序中断异常 处理
static char *exception_string[CYGNUM_HAL_EXCEPTION_MAX+1] = {
	"External interrupt",
	"TLB modification exception",
	"TLB miss (Load or IFetch)",
	"TLB miss (Store)",
	"Address error (Load or Ifetch)",
	"Address error (store)",
	"Bus error (Ifetch)",
	"Bus error (data load or store)",
	"System call",
	"Break point",
	"Reserved instruction",
	"Coprocessor unusable",
	"Arithmetic overflow",
	"Reserved (13)",
	"Division-by-zero [reserved vector]"
};

//add by zzh 20140416 used to print tracing stack info
void show_exception_info(HAL_SavedRegisters *exception_info)
{
#if 1 //show thread exception info
			Cyg_Thread *thread;
			thread = Cyg_Scheduler::get_current_thread();
			CYG_ADDRESS stack_ptr,stack_base;
			unsigned int stack_size;
			stack_size = thread->get_stack_size();
			stack_base = thread->get_stack_base();
			stack_ptr = thread->stack_ptr;
			diag_printf("--------- [%s] get exception !! ---------\n",thread->get_name());
			diag_printf(" ptr:%p base %p size:%d\n",stack_ptr,stack_base,(unsigned int)stack_size); 	
			if( (stack_ptr <= (stack_base + stack_size)) && (stack_ptr >= stack_base) )
			{
#if 1 //print useful symbol only...
				//extern char CYG_LABEL_NAME (start) [];
				//extern char CYG_LABEL_NAME (__bss_end) [];
	
				CYG_ADDRESS ptr = stack_ptr;
				CYG_ADDRESS limit = stack_base + stack_size;
				unsigned char next_line=0;
	
				diag_printf(" limit:%p\n",limit);
				diag_printf("--------- map symbol only ---------\n");
		
				//get stack ptr when exception occur...
				if(exception_info)
				{
					CYG_ADDRESS newptr;
					newptr = ((HAL_SavedRegisters*)exception_info)->d[29];	//R29 stack ptr
					if(newptr<=limit && newptr >= stack_base)
					{
						ptr=newptr; 				
						diag_printf(" updated stack ptr from R29 :%p\n",newptr);
					}else
					{ //for handle over use stack limit
						if(newptr < stack_base)
							diag_printf("over use stack limit ? stack ptr %p\n",newptr);
					}
				}
	
				//get PC and RA for last two funtion call from register
				if(exception_info)
				{
					HAL_SavedRegisters *regs;
					regs = (HAL_SavedRegisters *) exception_info;
					diag_printf(" [<%08x>]",regs->pc);					
					diag_printf(" [<%08x>]",regs->d[31]);
					next_line=2;
				}
				
				while(ptr<limit)
				{
					//if( (unsigned int)(*(CYG_WORD *)ptr) >= (unsigned int)(CYG_LABEL_NAME (start)) && (unsigned int)(*(CYG_WORD *)ptr) < (unsigned int)(CYG_LABEL_NAME (__bss_end)) )
					{
						diag_printf(" [<%08x>]",*(CYG_WORD *)ptr);
	
						if(++next_line & 0x08)
						{
							next_line=0x00;
							diag_printf("\n");
						}
					}
					ptr+=sizeof(CYG_WORD);
				}
				diag_printf("\n");
#endif
			}
#endif
}
//end by zzh

externC void tapf_board_reboot(void);


/* User exception handler */
void
user_exception_handler(cyg_addrword_t data, cyg_code_t exception_number, cyg_addrword_t info)
{	
	int i;
	HAL_SavedRegisters *regs;
	
	regs = (HAL_SavedRegisters *) info;

	
#ifdef HAVE_ADE_HANDLER
		if ((exception_number==4) || (exception_number==5)) {
			//diag_printf("ExcNum=%d\n", number);
			
			//show_registers((HAL_SavedRegisters *)info);
			
			// handle adel and ades
			if (ade_handler((HAL_SavedRegisters *)info)) {
				return;
			}
		}
#endif

//处理内存数据断点
#ifdef __CONFIG_WATCH_POINT__
	if(exception_number == 23)
	{
		if(!do_watch((HAL_SavedRegisters *)info))
		return;
	}
#endif

	show_exception_info((HAL_SavedRegisters *)info);

	diag_printf("Exception --------------------------------------------------------------\n");
	if (exception_number >= 0 && exception_number <= CYGNUM_HAL_EXCEPTION_MAX)
		diag_printf("  Type: %s\n", exception_string[exception_number]);
	
	/* dump register information. */
	diag_printf("  Data Regs:\n");
	for (i = 0; i < 8; i++) {
		diag_printf("    R%-2d   %08X    R%-2d   %08X    R%-2d   %08X    R%-2d   %08X\n",
			i, regs->d[i], i+8, regs->d[i+8],
			i+16, regs->d[i+16], i+24, regs->d[i+24]);
	}
	diag_printf("\n");

	diag_printf("    HI    %08X    LO    %08X    SR    %08X    PC    %08X\n",
		regs->hi, regs->lo, regs->sr, regs->pc);
	diag_printf("                      CAUSE %08X    PRID  %08X    BADVR %08X\n",
		regs->cache, regs->prid, regs->badvr);
	diag_printf("------------------------------------------------------------------------\n");
	
	show_registers(regs);
	tapf_board_reboot();
}


/* Hook the user exception to eCos kernel */
externC void
user_exception_init(void)
{
	
	cyg_addrword_t old_data;
	cyg_exception_handler_t *old_handler;
	
#ifdef __CONFIG_WATCH_POINT__
	//设置23号内存数据断点异常向量
	hal_vsr_table[23] = (volatile CYG_ADDRESS)__default_exception_vsr;
#endif
	/* user_exception_handler will handler all exception. */
	cyg_exception_set_handler(CYGNUM_HAL_EXCEPTION_MAX,
		user_exception_handler, 0, &old_handler, &old_data);
}



#endif
#endif	
#endif	
