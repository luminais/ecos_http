/*
 * Network protocol CLI route debug commands.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli_arp.c,v 1.2 2010/07/26 09:07:51 Exp $
 *
 */

 //roy +++2010/10/08

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iflib.h>
#include <ecos_oslib.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/sockio.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/if_ether.h>
#include <net/if_dl.h>
#include <net/radix.h>
#include <net/route.h>
#include <net/if_var.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <bcmnvram.h>
#include <netdb.h>
#include <cli.h>

extern int envram_init(int argc, char* argv[]);
extern int envram_get(int argc, char* argv[]);
extern int envram_set(int argc, char* argv[]);
extern int envram_commit(int argc, char* argv[]);
extern int envram_show(int argc, char* argv[]);

static int envram_cmd(int argc, char* argv[])
{
	int rc = -1;

	if (argc < 2)
		goto usage;

	if (!strcmp(argv[1], "init")) {
		rc = envram_init(argc, argv);
	}
	else if (!strcmp(argv[1], "show")) {
		rc = envram_show(argc, argv);
	}
	else if (!strcmp(argv[1], "get")) {
		rc = envram_get(argc, argv);
	}
	else if (!strcmp(argv[1], "set")) {
		rc = envram_set(argc, argv);
	}
	else if (!strcmp(argv[1], "commit")) {
		rc = envram_commit(argc, argv);
	}

	if (rc == 0)
		return 0;

usage:
	printf("envram [init] [get name] [set name=value] [commit] [show]...\n");
	return rc;
}

/* cli node */
CLI_NODE("envram",	envram_cmd,	"nvram command");

