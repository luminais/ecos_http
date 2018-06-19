/*
 * User exception handler
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: exception.c,v 1.3 2010-11-04 04:46:45 Exp $
 */

#include <cyg/kernel/kapi.h>
#include <osl.h>

extern void board_reboot(void);

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

/* User exception handler */
void
user_exception_handler(cyg_addrword_t data, cyg_code_t exception_number, cyg_addrword_t info)
{
	int i;
	HAL_SavedRegisters *regs;

	regs = (HAL_SavedRegisters *) info;

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

#if defined(BCMDBG)
	while (1);
#else
	board_reboot();
#endif
}

/* Hook the user exception to eCos kernel */
void
user_exception_init(void)
{
	cyg_addrword_t old_data;
	cyg_exception_handler_t *old_handler;

	/* user_exception_handler will handler all exception. */
	cyg_exception_set_handler(CYGNUM_HAL_EXCEPTION_MAX,
		user_exception_handler, 0, &old_handler, &old_data);
}


/* dereference and address that may cause a bus exception */
static int bus_error = 0;

void
read_dbe_handler(cyg_addrword_t data, cyg_code_t exception_number, cyg_addrword_t info)
{
	HAL_SavedRegisters *regs;

	regs = (HAL_SavedRegisters *) info;
	/* bump epc so we don't re-excecute */
	regs->pc += CYG_HAL_MIPS_REG_SIZE;
	bus_error = -1;
}

int
read_dbe(char *addr, char *value, int size)
{
	cyg_addrword_t old_data;
	cyg_exception_handler_t *old_handler;
	unsigned long __state;

	switch (size)
	{
	case 1:
		break;
	case 2:
		if ((uint32)addr & 1)
			return -1;
		break;
	case 4:
		if ((uint32)addr & 3)
			return -1;
		break;
	default:
		break;
	}

	if (!(IS_KSEG0((uint32)addr) || IS_KSEG1((uint32)addr)))
		return -1;

	/* s = splimp(); */

	HAL_DISABLE_INTERRUPTS(__state);

	bus_error = 0;

	cyg_exception_set_handler(CYGNUM_HAL_VECTOR_DBE,
		read_dbe_handler, 0, &old_handler, &old_data);

	switch (size)
	{
	case 1:
		*(uint8 *)value = *(uint8 *)addr;
		break;
	case 2:
		*(uint16 *)value = *(uint16 *)addr;
		break;
	case 4:
		*(uint32 *)value = *(uint32 *)addr;
		break;
	default:
		break;
	}

	cyg_exception_set_handler(CYGNUM_HAL_VECTOR_DBE, old_handler, old_data, NULL, NULL);

	HAL_RESTORE_INTERRUPTS(__state);

	/* splx(s); */

	return bus_error;
}
