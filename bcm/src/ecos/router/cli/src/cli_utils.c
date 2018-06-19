/*
 * CLI command for wireless LAN.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli_utils.c,v 1.6 2010-09-15 11:57:18 Exp $
 */

#include <string.h>
#include <stdio.h>

#include <typedefs.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <cli.h>

extern int wl(int argc, char **argv);
extern int et(int argc, char **argv);

static int
wlconf_cmd(int argc, char* argv[])
{
	extern int wlconf(char *name);
	extern int wlconf_down(char *name);
	extern int wlconf_start(char *name);

	if (argc != 3)
		goto usage;

	if (!strcmp(argv[2], "up"))
		wlconf(argv[1]);
	else if (!strcmp(argv[2], "down"))
		wlconf_down(argv[1]);
	else if (!strcmp(argv[2], "start"))
		wlconf_start(argv[1]);
	else
		goto usage;

	return 0;

usage:
	printf("Usage: wlconf wl0 <up|down|start>\n");
	return -1;
}

/* cli node */
CLI_NODE("wl",		wl,		"wl");
CLI_NODE("wlconf",		wlconf_cmd,	"wlconf <ifname> <up|down|start>");
CLI_NODE("et",		et,		"et");
