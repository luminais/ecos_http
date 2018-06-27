/*
 * CLI command include file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli.h,v 1.3 2010-04-28 03:03:14 Exp $
 */
#ifndef	__CLI_H__
#define __CLI_H__

#include <stdio.h>

#ifndef	__CONFIG_CLI_PROMPT__
#define	CLI_DEFAULT_PROMPT	"CLI>"
#else
#define	CLI_DEFAULT_PROMPT	__CONFIG_CLI_PROMPT__
#endif

#define CLI_MAX_HISTORY		10
#define CLI_MAX_CMD_LEN		511
#define CLI_MAX_ARGV		32

typedef struct {
	int fd;
	FILE *stream;
	char prompt[512];
	char cmd_buf[CLI_MAX_CMD_LEN];
	char history[CLI_MAX_HISTORY][CLI_MAX_CMD_LEN];
	int history_idx;
} cli_context;


#include <cyg/hal/hal_tables.h>
typedef struct cli_node {
	char *name;
	int (*func)(int argc, char **argv);
	char *doc;
} CYG_HAL_TABLE_TYPE cli_node_t;

/* External node */
#define CLI_NODE(name, func, doc)  \
cli_node_t cli_node__##func CYG_HAL_TABLE_ENTRY(clitab) = { \
	name, \
	func, \
	doc \
};

extern cli_node_t __cli_tab__[];
extern cli_node_t __cli_tab_end__;

/* Functions declaration, added by zhuhuan on 2015.12.26 */
extern void tenda_arp_show(int argc, char* argv[]);
extern int time_cmd(int argc, char *argv[]);
extern int syslog_cmd(int argc, char *argv[]);
extern int ifconfig_cmd(int argc, char *argv[]);
extern int mbuf_cmd(int argc, char *argv[]);
extern int thread_cmd(int argc, char* argv[]);
extern int debug_cmd(int argc, char *argv[]);

#endif	/* __CLI_H__ */
