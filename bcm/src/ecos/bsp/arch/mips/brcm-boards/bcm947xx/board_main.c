/*
 * Board main entry, called by main()
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: board_main.c,v 1.1 2010-04-16 03:42:44 Exp $
 */

#include <typedefs.h>
#include <string.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <osl.h>
#include <siutils.h>
#include <hndchipc.h>
#include <hndmips.h>

extern void pci_init(void);
extern void bcm_netdev_init(void);
extern void board_set_clock(int);
extern void ecos_nvram_init(void);

/*
* eCos porting for start here
*/
int kernel_initial = 0;

int board_main(void)
{
	si_t *sih;

	/*
	 * We invoke si_kattach() before cyg_hal_invoke_constructors() in hal_platform_init()
	 * However when doing si_kattach(), _nvram_init() invokes malloc(), which returns NULL
	 * and invokes printf() before the serial port drivers are ready.
	 * That's why we need the 'kernel_initial' variable to control the nvram driver behaviors.
	 */
	kernel_initial = 1; /* eCos kernel initialized */

	sih = si_kattach(SI_OSH);
	si_mips_init(sih, 0);
	nvram_init((void *)sih);

	/*just let this code in here for BUG:"恢复出厂设置的时候，无线参数与nvram不对"*/
	ecos_nvram_init();
	
	pci_init();

	bcm_netdev_init();

	board_set_clock(0);

	return 0;
}
