/*
 * Main entry
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: main.c,v 1.3 2010-06-03 08:01:34 Exp $
 */
#include <stdlib.h>
#include <stdio.h>

#define	DIR_FOR_DD			"/Dir_For_DD"	/*bonson专有文件夹*/

extern void user_exception_init(void);
extern int board_main(void);
extern int rc(int argc, char *argv[]);
extern void board_reboot(void);

int main(void)
{
	int argc = 1;
	char *argv[2] = {"init", 0};

	/* init user exception handler */
	user_exception_init();

	/* init board */
	board_main();

	/*wwk add*/
	mount( "", "/", "ramfs" );	
	mkdir(DIR_FOR_DD, 0777);
	/*end add*/
	
	/* Entery user main */
	rc(argc, argv);

	/* Reboot */
	board_reboot();
	return 0;
}
