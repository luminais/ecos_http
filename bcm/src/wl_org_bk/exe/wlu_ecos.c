/*
 * ECOS port of wl command line utility
 *
 * Copyright 2002, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id: wlu_ecos.c,v 1.4.90.1 2010-12-21 02:38:56 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include "wlu.h"

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <bcmparams.h>
#include <wlutils.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include <ethtool.h>


/* Search the  wl_cmds table for a matching command name.
 * Return the matching command or NULL if no match found.
 */
static cmd_t*
wl_find_cmd(char* name)
{
	cmd_t *cmd = NULL;

	/* search the wl_cmds for a matching name */
	for (cmd = wl_cmds; cmd->name && strcmp(cmd->name, name); cmd++);

	if (cmd->name == NULL)
		cmd = NULL;

	return cmd;
}

static int wl_default_if(char *buf)
{
	int s, i;
	struct ifreq ifr;
	int rc = -1;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	for (i = 1; i <= DEV_NUMIFS; i ++) {
		/* ignore i/f that is not ethernet */
		ifr.ifr_ifindex = i;
		if (ioctl(s, SIOCGIFNAME, &ifr))
			continue;
		if (ioctl(s, SIOCGIFHWADDR, &ifr))
			continue;
		if (ifr.ifr_hwaddr.sa_family != AF_LINK)
			continue;
		if (!strncmp(ifr.ifr_name, "vlan", 4))
			continue;
		if (wl_probe(ifr.ifr_name) == 0) {
			strcpy(buf, ifr.ifr_name);
			rc = 0;
			break;
		}
	}
	close(s);
	return rc;
}

int
wl(int argc, char **argv)
{
	char if_name[32];
	cmd_t *cmd = NULL;
	int err = 0;
	char *ifname = NULL;
	int help = 0;
	int status;

	wlu_init();

	argv++;

	/* default */
	if (wl_default_if(if_name) != 0)
		return -1;

	/* initialize command processor */
	wl_cmd_init();

	for (; *argv;) {

		/* command line option */
		if ((status = wl_option(&argv, &ifname, &help)) == CMD_OPT) {
			/* print usage */
			if (help)
				break;
			/* select different adapter */
			if (ifname) {
				strncpy(if_name, ifname, IFNAMSIZ);
				if (wl_check((void *)if_name)) {
					printf("wl driver adapter not found\n");
					return -1;
				}
			}
			continue;
		}
		/* parse error */
		else if (status == CMD_ERR)
			break;

		/* search for command */
		for (cmd = wl_cmds; cmd->name && strcmp(cmd->name, *argv); cmd++);

		/* defaults to using the set_var and get_var commands */
		if (cmd->name == NULL)
			cmd = &wl_varcmd;

		/* do command */
		err = (*cmd->func)((void *) if_name, cmd, argv);

		break;
	}

	if (help && *argv) {
		cmd = wl_find_cmd(*argv);
		if (cmd)
			wl_cmd_usage(stdout, cmd);
		else
			printf("%s: Unrecognized command \"%s\", type -h for help\n",
			       wlu_av0, *argv);
	}
	else if (!cmd)
		wl_usage(stdout, NULL);
	else if (err == IOCTL_ERROR)
		wl_printlasterror((void *)if_name);
	else if (err)
		wl_cmd_usage(stderr, cmd);
	return err;
}
