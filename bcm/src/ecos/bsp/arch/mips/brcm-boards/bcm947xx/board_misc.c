/*
 * Misc function of BCM board
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: board_misc.c,v 1.3 2010-05-28 04:12:43 Exp $
 */

#include <string.h>

#include <typedefs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <osl.h>
#include <siutils.h>
#include <hndchipc.h>
#include <hndmips.h>
#include <hndpci.h>
#include <hndcpu.h>
#include <mipsinc.h>
#include <mips33_core.h>
#include <mips74k_core.h>
#include <mipsregs.h>

#include <cyg/hal/hal_intr.h>

extern void hnd_cpu_reset(si_t *sih);

extern void cyg_hal_plf_serial_add(void *regs, uint irq, uint baud_base, uint reg_shift);
extern void pc_serial_info0_add(void *regs,
	cyg_uint32 irq, cyg_uint32 baud_base, cyg_uint32 reg_shift);

/* Global SI handle */
si_t *bcm947xx_sih;

/* Convenience */
#define sih bcm947xx_sih

static int ser_count = 0;
cyg_uint32 g_cpu_clock;

static void
plf_serial_add(void *regs, uint irq, uint baud_base, uint reg_shift)
{
	if (ser_count >= 1)
		return;

	pc_serial_info0_add(regs, irq, baud_base, reg_shift);
	cyg_hal_plf_serial_add(regs, irq, baud_base, reg_shift);
	ser_count++;
	return;
}

void
bcm_sb_init(void)
{
	uint32 coreidx, processor_id;
	void *regs;
	uint32 *intmask;

	sih = si_kattach(SI_OSH);

	/* Use probed value instead of constants */
	g_cpu_clock = si_cpu_clock(sih);
	cyg_hal_clock_period = (g_cpu_clock / 2) / CYGNUM_HAL_RTC_DENOMINATOR;

	si_serial_init(sih, plf_serial_add);

	/* For mips74k, route the timer interrupt */
	processor_id = read_c0_prid();
	if (MIPS74K(processor_id)) {
		coreidx = si_coreidx(sih);
		regs = si_setcore(sih, MIPS74K_CORE_ID, 0);
		si_setcoreidx(sih, coreidx);

		/* Use intmask5 register to route the timer interrupt */
		intmask = (uint32 *)&((mips74kregs_t *)regs)->intmask[5];
		W_REG(NULL, intmask, (1 << 31));
	}
}

void
udelay(int delay)
{
	hal_delay_us(delay);
	return;
}

void
board_reboot(void)
{
	hnd_cpu_reset(sih);
}

void
board_set_clock(int m)
{

}

cyg_uint64
board_current_usec(void)
{
	cyg_uint64 temp;
	cyg_uint32 timer_lo;

	temp = (cyg_uint64)cyg_current_time() * (cyg_uint64)10000UL;
	HAL_CLOCK_READ(&timer_lo);
	temp += (cyg_uint64)((timer_lo*2)/(g_cpu_clock/1000000));
	return (temp);
}

uint32
board_current_msec(void)
{
	cyg_uint64 temp = board_current_usec();

	return (uint32)(temp/1000);
}

void
cpu_idle_sleep(int action)
{
	extern int mips_idle_sleep;

	mips_idle_sleep = action;
}
