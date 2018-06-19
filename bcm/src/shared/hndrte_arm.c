/*
 * hndrte ARM port specific routines.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte_arm.c,v 1.84.8.3 2010-12-02 06:18:22 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <hndrte.h>
#include <hndcpu.h>
#include <sbhndarm.h>
#include <sbchipc.h>
#include <hndchipc.h>
#include <bcmsdpcm.h>


#ifndef HNDRTE_POLLING
static void hndrte_pmu_isr(void *cbdata, uint32 intst);
#endif

#if defined(BCMRECLAIM) && defined(CONFIG_XIP)
#error "Both XIP and RECLAIM defined"
#endif


#ifdef DONGLEBUILD
bool bcmreclaimed = FALSE;

#ifdef BCMRECLAIM
bool r1_reclaimed = FALSE;
#endif
bool r2_reclaimed = FALSE;

void
hndrte_reclaim(void)
{
	uint reclaim_size = 0;
#ifdef BCMRECLAIM
	if (!bcmreclaimed && !r1_reclaimed) {
		extern char _rstart1[], _rend1[];	/* start/end of reclaimable area */
		reclaim_size = (uint)(_rend1 - _rstart1);
		if (reclaim_size) {
			bzero(_rstart1, reclaim_size);	/* blow away the reclaim region */
			hndrte_arena_add((uint32)_rstart1, reclaim_size);
		}

		/* Nightly dongle test searches output for "Returned (.*) bytes to the heap" */
		printf("reclaim section 2: Returned %d bytes to the heap\n", reclaim_size);
		bcmreclaimed = TRUE;
		r1_reclaimed = TRUE;
		return;
	}
#endif /* BCMRECLAIM */
	if (!r2_reclaimed) {
		extern char _rstart2[], _rend2[];	/* start/end of reclaimable area */
		reclaim_size = (uint)(_rend2 - _rstart2);

		if (reclaim_size) {
			bzero(_rstart2, reclaim_size);	/* blow away the reclaim region */
			hndrte_arena_add((uint32)_rstart2, reclaim_size);
		}

		/* Nightly dongle test searches output for "Returned (.*) bytes to the heap" */
		printf("reclaim section 1: Returned %d bytes to the heap\n", reclaim_size);
		bcmreclaimed = FALSE;
		r2_reclaimed = TRUE;

#ifdef HNDRTE_PT_GIANT
		hndrte_append_ptblk();
#endif
#ifdef BCMPKTPOOL
		if (pktpool_shared.inited)
			pktpool_fill(hndrte_osh, &pktpool_shared,
#if defined(WLC_LOW) && !defined(WLC_HIGH)
		/* for bmac, populate all the configured buffers */
			FALSE);
#else
			TRUE);
#endif

#endif /* BCMPKTPOOL */
		return;
	}
}
#endif /* DONGLEBUILD */

/*
 * Timing support:
 *	hndrte_delay(us): Delay us microseconds.
 */

/*
 * c0 ticks per usec - used by hndrte_delay() and is based on 80Mhz clock.
 * We will never expect the busy loop delay to run on any other slow clocks.
 */
static uint32 c0counts_per_us = HT_CLOCK / 1000000;

/* ticks per msec - used by hndrte_update_now() and is based on either 80Mhz
 * clock or 32Khz clock depending on the compile-time decision.
 */
/* ILP clock speed default to 32KHz */
static uint32 pmuticks_per_ms = ILP_CLOCK / 1000;

static uint max_timer_dur = ((1 << 10) - 1) / 32;
/* PmuRev1 has a 24-bit PMU RsrcReq timer. However it pushes all other bits
 * upward. To make the code to run for all revs we use a variable to tell how
 * many bits we need to shift.
 */

#if BCMCHIPID == BCM4328_CHIP_ID
static uint8 flags_shift = 0;
#else
#define flags_shift	14
#endif

/* We'll keep time in ms */
static uint32 lastcount = 0;

uint32
hndrte_update_now(void)
{
	uint32 count, diff;

	count = R_REG(hndrte_osh, &hndrte_ccr->pmutimer);

	diff = count - lastcount;
	if (diff >= pmuticks_per_ms) {
		lastcount = count;
		return diff / pmuticks_per_ms;
	}

	return 0;
}

#ifdef BCMDBG_SD_LATENCY
static uint32 lastcount_us = 0;

uint32
hndrte_update_now_us(void)
{
	uint32 count, diff;

	count = get_arm_cyclecount();

	diff = count - lastcount_us;
	if (diff >= c0counts_per_us) {
		lastcount_us = count;
		return diff / c0counts_per_us;
	}

	return 0;
}
#endif /* BCMDBG_SD_LATENCY */

void
hndrte_delay(uint32 us)
{
	uint32 curr, c0_wait;

	curr = get_arm_cyclecount();
	c0_wait = us * c0counts_per_us;

	while ((get_arm_cyclecount() - curr) < c0_wait)
		;
}

#ifdef BCMDBG_LOADAVG
/* Note: load average mechanism also requires BCMDBG_FORCEHT to be defined */

#define LOADAVG_HZ		100		/* Sample Hz */
#define LOADAVG_ALPHA		0.999		/* Decay per sample */
#define LOADAVG_PRT_IVAL	300		/* Print interval, in units of Hz */

#define LOADAVG_A		((int)(LOADAVG_ALPHA * 256))

static unsigned int loadavg;
static unsigned int loadavg_prt;

#endif /* BCMDBG_LOADAVG */

/*
 * hndrte.
 *
 * Initialize and background:
 *	hndrte_cpu_init: Initialize the world.
 */

void
BCMATTACHFN(hndrte_cpu_init)(si_t *sih)
{
	si_arm_init(sih);

#ifdef EXT_CBALL
	{
	uint32 *v = (uint32 *)0;
	extern char __traps[], _mainCRTStartup[];

	/*
	 * Write new exception vectors.
	 * EXT_CBALL currently does not link with 'startup' at address 0.
	 */

	v[ 0] = 0xea000000 | ((uint32)_mainCRTStartup / 4 - 2);	/* 0000: b <reset> */
	v[ 1] = 0xe59ff014;				/* 0004: ldr pc, [pc, #20] */
	v[ 2] = 0xe59ff014;				/* 0008: ldr pc, [pc, #20] */
	v[ 3] = 0xe59ff014;				/* 000c: ldr pc, [pc, #20] */
	v[ 4] = 0xe59ff014;				/* 0010: ldr pc, [pc, #20] */
	v[ 5] = 0xe59ff014;				/* 0014: ldr pc, [pc, #20] */
	v[ 6] = 0xe59ff014;				/* 0018: ldr pc, [pc, #20] */
	v[ 7] = 0xe59ff014;				/* 001c: ldr pc, [pc, #20] */
	v[ 8] = (uint32)__traps + 0x00;			/* 0020: =tr_und */
	v[ 9] = (uint32)__traps + 0x10;			/* 0024: =tr_swi */
	v[10] = (uint32)__traps + 0x20;			/* 0028: =tr_iab */
	v[11] = (uint32)__traps + 0x30;			/* 002c: =tr_dab */
	v[12] = (uint32)__traps + 0x40;			/* 0030: =tr_bad */
	v[13] = (uint32)__traps + 0x50;			/* 0034: =tr_irq */
	v[14] = (uint32)__traps + 0x60;			/* 0038: =tr_fiq */
	}
#endif /* EXT_CBALL */

	/* Set trap handler */
	hndrte_set_trap((uint32)hndrte_trap_handler);

	/* Initialize timers */
	c0counts_per_us = (si_cpu_clock(sih) + 999999) / 1000000;
	pmuticks_per_ms = (si_ilp_clock(sih) + 999) / 1000;
	if (sih->pmurev >= 1) {
		max_timer_dur = ((1 << 24) - 1);
#if BCMCHIPID == BCM4328_CHIP_ID
		flags_shift = 14;
#endif
	} else {
		max_timer_dur = ((1 << 10) - 1);
	}
	max_timer_dur /= pmuticks_per_ms;

#ifndef HNDRTE_POLLING
	/* Register the timer interrupt handler */
	si_cc_register_isr(sih, hndrte_pmu_isr, CI_PMU, NULL);
#endif
}

void *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
hndrte_dma_alloc_consistent(uint size, uint16 align_bits, uint *alloced, void *pap,
	char *file, int line)
#else
hndrte_dma_alloc_consistent(uint size, uint16 align_bits, uint *alloced, void *pap)
#endif
{
	void *buf;
	uint16 align;

	align = (1 << align_bits);

	/* align on a OSL defined boundary */
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!(buf = hndrte_malloc_align(size, align_bits, file, line)))
#else
	if (!(buf = hndrte_malloc_align(size, align_bits)))
#endif
		return NULL;
	ASSERT(ISALIGNED((uintptr)buf, align));
	*alloced = size;

#ifdef CONFIG_XIP
	/* 
	 * arm bootloader memory is remapped but backplane addressing is
	 * 0-based
	 *
	 * Background: since the mask rom bootloader code executes in a
	 * read-only memory space apart from SoC RAM, data addresses
	 * specified by bootloader code must be decoded differently from
	 * text addresses. Consequently, the processor must run in a
	 * "memory remap" mode whereby data addresses containing the
	 * MEMORY_REMAP bit are decoded as residing in SoC RAM. However,
	 * backplane agents, e.g., the dma engines, always use 0-based
	 * addresses for SoC RAM, regardless of processor mode.
	 * Consequently it is necessary to strip the MEMORY_REMAP bit
	 * from addresses programmed into backplane agents.
	 */
	*(ulong *)pap = (ulong)buf & ~MEMORY_REMAP;
#else
	*(ulong *)pap = (ulong)buf;
#endif /* CONFIG_XIP */

	return (buf);
}

void
hndrte_dma_free_consistent(void *va)
{
	hndrte_free(va);
}

/* Two consecutive res_req_timer writes must be 2 ILP + 2 OCP clocks apart */
#if BCMCHIPID == BCM4328_CHIP_ID
static void
pr41608_war(void)
{
	static volatile uint32 hndrte_pmutmr_time = 0;
	uint32 expect;

	if (hndrte_sih->pmurev > 0)
		return;

	expect = hndrte_pmutmr_time + 3;
	if (expect < hndrte_pmutmr_time)
		while (R_REG(hndrte_osh, &hndrte_ccr->pmutimer) >= hndrte_pmutmr_time);
	while (R_REG(hndrte_osh, &hndrte_ccr->pmutimer) <= expect)
		;
	hndrte_pmutmr_time = R_REG(hndrte_osh, &hndrte_ccr->pmutimer);
}
#else
#define pr41608_war()
#endif /* BCMCHIPID == BCM4328_CHIP_ID */

static uint32 timer_extra_ms = 0;
static uint32 timer_extra_ms_start;

void
hndrte_set_irq_timer(uint ms)
{
	uint32 req;
	uint ticks;

	if (ms > max_timer_dur) {
		/* don't req HT if we are breaking a large timer to multiple max h/w duration */
		timer_extra_ms = ms - max_timer_dur;
		timer_extra_ms_start = hndrte_time();
		ms = max_timer_dur;
		req = (PRRT_ALP_REQ | PRRT_INTEN) << flags_shift;
	} else {
		req = (PRRT_HT_REQ | PRRT_INTEN) << flags_shift;
		timer_extra_ms = 0;
	}

	ticks = ms ? ms * pmuticks_per_ms : 2;

	pr41608_war();

	W_REG(hndrte_osh, &hndrte_ccr->res_req_timer, req | ticks);
	(void)R_REG(hndrte_osh, &hndrte_ccr->res_req_timer);
}

void
hndrte_ack_irq_timer(void)
{
	pr41608_war();

	W_REG(hndrte_osh, &hndrte_ccr->res_req_timer, 0);
	(void)R_REG(hndrte_osh, &hndrte_ccr->res_req_timer);
}

void
hndrte_wait_irq(si_t *sih)
{
	hnd_cpu_wait(sih);
}

void
hndrte_enable_interrupts(void)
{
	enable_arm_ints(PS_I);
}

void
hndrte_disable_interrupts(void)
{
	disable_arm_ints(PS_I);
}

/* Enable/Disable h/w HT request */
#if defined(__ARM_ARCH_4T__)
#if BCMCHIPID == BCM4328_CHIP_ID || BCMCHIPID == BCM4325_CHIP_ID
#define HW_HTREQ_ON() \
	do { \
		if (hndarm_rev < 2) \
			AND_REG(hndrte_osh, ARMREG(hndarm_armr, clk_ctl_st), ~CCS_FORCEHWREQOFF); \
	} while (0)
#define HW_HTREQ_OFF() \
	do { \
		if (hndarm_rev < 2) \
			OR_REG(hndrte_osh, ARMREG(hndarm_armr, clk_ctl_st), CCS_FORCEHWREQOFF); \
	} while (0)
#else	/* BCMCHIPID != BCM4328_CHIP_ID && BCMCHIPID != BCM4325_CHIP_ID */
#define HW_HTREQ_ON()
#define HW_HTREQ_OFF()
#endif	/* BCMCHIPID != BCM4328_CHIP_ID && BCMCHIPID != BCM4325_CHIP_ID */
#elif defined(__ARM_ARCH_7M__)
#define HW_HTREQ_ON()
#define HW_HTREQ_OFF()
#endif	/* __ARM_ARCH_7M__ */

void
hndrte_idle_init(si_t *sih)
{
#ifndef HNDRTE_POLLING
	HW_HTREQ_OFF();
#endif
}

#ifndef HNDRTE_POLLING
static void
hndrte_pmu_isr(void *cbdata, uint32 ccintst)
{
	/* Handle pmu timer interrupt here */
	if (
#if BCMCHIPID == BCM4328_CHIP_ID
	    ccintst == 0 ||
#endif
	    (R_REG(hndrte_osh, &hndrte_ccr->res_req_timer) & (PRRT_REQ_ACTIVE << flags_shift))) {
		/* Clear the pmustatus.intpend bit */
		W_REG(hndrte_osh, &hndrte_ccr->pmustatus, PST_INTPEND);
		(void)R_REG(hndrte_osh, &hndrte_ccr->pmustatus);
		hndrte_ack_irq_timer();
		if (timer_extra_ms) {
			uint32 current = hndrte_time();
			uint32 delta =  current > timer_extra_ms_start ?
			        current - timer_extra_ms_start :
			        (uint32)~0 - timer_extra_ms_start + 1 + current;
			if (timer_extra_ms > delta) {
				hndrte_set_irq_timer(timer_extra_ms - delta);
				return;
			}
		}
		/* Run timeouts */
		hndrte_timer_isr();
	}
}
#endif	/* !HNDRTE_POLLING */

/* Trap initialization */
void
BCMATTACHFN(hndrte_trap_init)(void)
{
#if defined(BCMDBG_TRAP) && !defined(_HNDRTE_SIM_)
	/*
	 * When BCMDBG_TRAP is true, set up CPU to trap on as many errors as possible.
	 * Capabilities vary by CPU.
	 */

#if defined(__ARM_ARCH_7M__)
	/* Enable traps for detecting unaligned accesses and divide by zero */
	wreg32(CM3_CFGCTRL, rreg32(CM3_CFGCTRL) | 0x18);	/* DIV_0_TRP | UNALIGN_TRP */

	/* The rest of this is to trap on read/write to the low 128 bytes of memory.
	 * Set TRCENA (1 << 24) in the Debug Exception and Monitor Control Register to enable
	 * DWT (data watchpoint and tracing) functionality, and turn on MON_EN (1 << 16) to
	 * enable Monitor Mode.
	 */
	wreg32(0xe000edfc, rreg32(0xe000edfc) | (1U << 24) | (1U << 16));

	/* Set address mask DWT_MASK0 to ignore the bottom 7 bits and match a 128-byte block */
	wreg32(0xe0001024, 7);

	/* Set comparator value DWT_COMP0 to 0 to match the block at address 0x00000000. */
	wreg32(0xe0001020, 0x00000000);

	/* Read DWT_FUNCTION0 to clear the MATCH bit in case it is already set */
	(void)rreg32(0xe0001028);

	/* Set DWT_FUNCTION0 to 6 (Watchpoint on Write) or 7 (Watchpoint on Read or Write) */
	wreg32(0xe0001028, 7);

	/* The priority of all exceptions and interrupts defaults to 0 (highest).  The data
	 * watchpoint exception will not occur while handling an interrupt if they're the same
	 * priority, and most of our code executes at interrupt level, so set the priority of
	 * interrupts to the next highest priority, which is 0x20 since priority is configured
	 * for 3 bits and is held in the upper 3 bits of an 8-bit value.
	 */
	wreg32(0xe000e400, 1U << 5);
#endif /* __ARM_ARCH_7M__ */
#endif /* BCMDBG_TRAP && !_HNDRTE_SIM_ */

#ifdef BCMDBG_LOADAVG
	/* Set up SysTick interrupt for profiling */
	wreg32(0xe000e014, HT_CLOCK / LOADAVG_HZ);
	wreg32(0xe000e010, 7);	/* ENABLE | INTR | CORE CLOCK */
#endif /* BCMDBG_LOADAVG */
}

void
hndrte_trap_handler(trap_t *tr)
{
#ifndef HNDRTE_POLLING
#if defined(__ARM_ARCH_4T__)
	if (tr->type == TR_IRQ) {
#if BCMCHIPID == BCM4328_CHIP_ID
		/* Check PMU res_req_timer interrupt apart from others because
		 * this may be an "intermediate" timer due to the h/w max timer
		 * length limitation. We request ALP with these intermediate
		 * timers to save power therefore don't request HT.
		 */
		if (hndrte_sih->ccrev < 21 &&
		    (R_REG(hndrte_osh, &hndrte_ccr->res_req_timer) & PRRT_REQ_ACTIVE)) {
			hndrte_pmu_isr(NULL, 0);
			return;
		}
#endif /* BCMCHIPID == BCM4328_CHIP_ID */
		HW_HTREQ_ON();
		hndrte_isr();
		HW_HTREQ_OFF();
		return;
	}
#elif defined(__ARM_ARCH_7M__)
	if (tr->type >= TR_ISR && tr->type < TR_ISR + ARMCM3_NUMINTS) {
		HW_HTREQ_ON();
		hndrte_isr();
		HW_HTREQ_OFF();
		return;
	}

#ifdef BCMDBG_LOADAVG
	if (tr->type == TR_SYSTICK) {
		/* CPU was idle if it broke out of a wfi instruction. */
		unsigned int idle = (*(uint16 *)(tr->epc - 2) == 0xbf30);
		unsigned int load = (idle ? 0 : 100) * 256;
		loadavg = (loadavg * LOADAVG_A + load * (256 - LOADAVG_A)) >> 8;
		if (++loadavg_prt >= LOADAVG_PRT_IVAL) {
			printf("Load: %d.%02d (pc=0x%x)\n",
			       loadavg >> 8, (100 * (loadavg & 255)) >> 8,
			       tr->epc);
			loadavg_prt = 0;
		}
		return;
	}
#endif	/* BCMDBG_LOADAVG */
#endif	/* __ARM_ARCH_7M__ */
#endif	/* !HNDRTE_POLLING */

#ifdef TR_ARMRST
	if (tr->type == TR_ARMRST) {
		printf("ARM reset: SErr=%d, Rstlg=0x%x\n",
		       hndrte_ccsbr->sbtmstatehigh & SBTMH_SERR,
		       R_REG(hndrte_osh, ARMREG(hndarm_armr, resetlog)));
	}
#endif

#if defined(__ARM_ARCH_7M__)
	 if ((tr->type == TR_FAULT) && (tr->pc  == ((uint32)&osl_busprobe & ~1))) {
		 /*	LDR and ADR instruction always sets the least significant bit 
		  *	of program address to 1 to indicate thumb mode 
		  *	LDR R0, =osl_busprobe ; R0 = osl_busprobe + 1
		  */
		/* printf("busprobe failed for addr 0x%08x\n", tr->pc); */  
			*((volatile uint32 *)(tr->r13 + CM3_TROFF_PC)) = tr->pc + 6;
			*((volatile uint32 *)(tr->r13 + CM3_TROFF_R0)) = ~0;
			return;
		}
#endif /* defined(__ARM_ARCH_7M__) */		

	/*
	 * ARM7TDMI trap types:
	 *	0=RST, 1=UND, 2=SWI, 3=IAB, 4=DAB, 5=BAD, 6=IRQ, 7=FIQ
	 *
	 * ARM CM3 trap types:
	 *	1=RST, 2=NMI, 3=FAULT, 4=MM, 5=BUS, 6=USAGE, 11=SVC,
	 *	12=DMON, 14=PENDSV, 15=SYSTICK, 16+=ISR
	 */

	/* Note that UTF parses the first line, so the format should not be changed. */
	printf("\nTRAP %x(%x): pc %x, lr %x, sp %x, cpsr %x, spsr %x\n",
	       tr->type, (uint32)tr, tr->pc, tr->r14, tr->r13, tr->cpsr, tr->spsr);

#if defined(BCMSPACE) || defined(HNDRTE_CONSOLE)
	printf("  r0 %x, r1 %x, r2 %x, r3 %x, r4 %x, r5 %x, r6 %x\n",
	       tr->r0, tr->r1, tr->r2, tr->r3, tr->r4, tr->r5, tr->r6);
	printf("  r7 %x, r8 %x, r9 %x, r10 %x, r11 %x, r12 %x\n",
	       tr->r7, tr->r8, tr->r9, tr->r10, tr->r11, tr->r12);
#endif /* BCMSPACE */


	hndrte_die(__LINE__);
}

#ifdef DONGLEOVERLAYS
#if defined(__ARM_ARCH_7M__)
uint32
hndrte_overlayerrfn_branch_instr(int32 from, int32 to)
{
	/* template is for 32-bit encoded thumb-2 branch (not bl) instruction */
	/* template allows for a negative 22-bit/4MB branch (S, J1 & J2 values hard coded) */
	/* bits 10:0 of template_hw2 are bits 11:1 of offset */
	/* bits 9:0 of template_hw1 are bits 21:12 of offset */
	/* the rest of offset bits are assumed to be 1's for a negative, sign-extended val */
	/* the arm shifts this value left by one bit, yielding a 22-bit signed offset value */
	uint16 template_hw1 = 0xf400;
	uint16 template_hw2 = 0xb800;
	uint16 bits_hw1;
	uint16 bits_hw2;
	int32 offset;

	/* this routine only works for a branch to a negative offset */
	ASSERT(from > to);

	/* calculate the negative offset to the error routine from the end of region */
	/* ("from" is assumed to be 4 bytes beyond the instruction being created) */
	offset = -(from - to);
	offset = offset >> 1;
	bits_hw1 = (uint16) (template_hw1 | ((uint32)offset >> 12 & 0x3ff));
	bits_hw2 = (uint16) (template_hw2 | ((uint32)offset & 0x7ff));
	return (uint32) ((bits_hw2 << 16) | bits_hw1);
}
#else
#error "hndrte_overlayerrfn_branch_instr needs to be defined for arm7 !"
#endif /* __ARM_ARCH_7M__ */
#endif /* DONGLEOVERLAYS */
