/*
 * CLI command main entry.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli_main.c,v 1.6 2010-09-14 03:45:27 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cli.h>
#include <cli_io.h>
//#include <shutils.h>
#include <bcmnvram.h>


/*static */int
get_argv(char *string, char *argv[])
{
	int n = 0;
	char *p = string;

	while (*p) {
		argv[n] = p;
		if(*p=='-'&&*(p+1)=='-')
		{
			p=p+3;
			while (*p == ' ' && *p)
				p++;
			argv[n]=p;
			n++;
			break;	
		}
		 /* Search until space */
		while (*p != ' ' && *p) {
			/* Take care of doube quot */
			if (*p == '\"') {
				char *qs, *qe;

				qs = p;
				qe = strchr(p+1, '\"');
				if (qe == NULL)
					return 0; /* Unbalanced */

				/* Null eneded quot string and do shift */
				*qe = '\0';
				memmove(qs, qs+1, (int)(qe-qs));

				p = qe;
				break;
			}

			p++;
		}

		if (*p) {
			*p++ = '\0';

			/* Eat white space */
			while (*p == ' ' && *p)
				p++;
		}

		n++;
		if (n == CLI_MAX_ARGV-1)
			break;
	}

	return n;
}

static int
cli_gets(cli_context *cli)
{
	char c;
	int size = 0;
	int esc_f1 = 0, esc_f2 = 0;
	int idx = cli->history_idx;
	char *buf = cli->cmd_buf;
	FILE *stream = cli->stream;
	int len;

	while (1) {
		c = 0;
		len = cli_io_read(cli->fd, &c);
		if (len < 0)
			return -1;

		switch (c) {
		case 0:
			/* This is caused by introduced by power controller. */
			continue;

		case 0x8:	/* drop through */
		case 0x7f:
			/* erase one character + 'backspace' char */
			size -= 1;
			if (size < 0)
				size = 0;
			else {
				fprintf(stream, "\b \b");
				fflush(stream);
			}
			continue;

		case 0xd:
		case 0xa:
			buf[size] = '\0';
			fprintf(stream, "\r\n");
			fflush(stream);
			return 0;

		case 0x1b:
			esc_f1 = 1;
			continue;

		case 0x5b:
			if (esc_f1) {
				esc_f2 = 1;
				continue;
			}
			esc_f1 = 0;
			break;

		case 0x41:	/* up */
			if (esc_f1 && esc_f2) {
				if (--idx < 0)
					idx = CLI_MAX_HISTORY-1;
			}
			break;

		case 0x42:	/* down */
			if (esc_f1 && esc_f2) {
				if (++idx > CLI_MAX_HISTORY-1)
					idx = 0;
			}
			break;

		default:
			if (esc_f1 && esc_f2) {
				/* Eat all escape sequence */
				esc_f1 = esc_f2 = 0;
				continue;
			}
			break;
		}

		/* Take care of escape */
		if (esc_f1 && esc_f2 && (c == 0x41 || c == 0x42)) {
			size = strlen(cli->history[idx]);
			memcpy(buf, cli->history[idx], size+1);

			fprintf(stream, "\x1b[2K");
			fprintf(stream, "\r%s %s", cli->prompt, buf);
			fflush(stream);
		}
		else if (size < CLI_MAX_CMD_LEN) {
			/* Buffer overflow check */
			buf[size++] = c;

			fputc((int)c, stream);
			fflush(stream);
		}

		esc_f1 = esc_f2 = 0;
	}
}

/* Main loop of cli */
int
cli_main(int fd)
{
	int argc;
	char *argv[CLI_MAX_ARGV+1];
	int retval;
	cli_node_t *cn;
	int count;
	int found;
	int help_mode;

	cli_context *cli;
	char *buf;
	char *prompt;
	char *nv_prompt;
	FILE *stream;

	/* Allocate context */
	cli = (cli_context *)calloc(sizeof(*cli), 1);
	if (!cli)
		return -1;

	/* Init console io */
	cli->fd = fd;
	cli->stream = cli_io_init(fd);
	if (cli->stream == NULL) {
		free(cli);
		return -1;
	}

	/* Set command loop */
	nv_prompt = nvram_get("cli_prompt");
	if (nv_prompt == NULL)
		nv_prompt = CLI_DEFAULT_PROMPT;

	strncpy(cli->prompt, nv_prompt, sizeof(cli->prompt)-1);
	prompt = cli->prompt;
	buf = cli->cmd_buf;
	stream = cli->stream;

	while (1) {
		fprintf(stream, "%s ", prompt);
		fflush(stream);

		memset(buf, 0, CLI_MAX_CMD_LEN);

		if (cli_gets(cli) < 0) {
			/* IO error, quit */
			break;
		}

		/* record into history */
		if (buf[0]) {
			if (cli->history_idx > CLI_MAX_HISTORY-1)
				cli->history_idx = 0;

			memcpy(cli->history[cli->history_idx++], buf, CLI_MAX_CMD_LEN);
		}

		/* Get argc, argv */
		memset(argv, 0, sizeof(argv));
		argc = get_argv(buf, argv);

		if (argc < 1)
			continue;

		/* Add exit command for telnetd */
		if(cli->stream != stdout) {
			if (!strcmp(argv[0], "exit")) {
				printf("exit cli\n");
				break;
			}
		}
		/* Preprocess built-in commands */
		if (!strcmp(argv[0], "prompt")) {
			if (argc == 1) {
				strcpy(prompt, CLI_DEFAULT_PROMPT);
				nvram_unset("cli_prompt");
			}
			else if (argc == 2) {
				strcpy(prompt, argv[1]);
				nvram_set("cli_prompt", argv[1]);
			}
			continue;
		}
		else {
			help_mode = 0;
			if (!strcmp(argv[0], "help") || *(argv[0]) == '?') {
				help_mode = 1;
			}
		}

		/* Search for cli node */
		found = 0;
		count = 0;
		for (cn = __cli_tab__; cn != &__cli_tab_end__; cn++) {
			switch (help_mode) {
			case 1:	/* single help */
				fprintf(stream, "%-12s", cn->name);
				if (++count > 5) {
					fprintf(stream, "\r\n");
					count = 0;
				}
				break;

			default:
				if (!strcmp(argv[0], cn->name)) {
					retval = (*cn->func)(argc, argv);
					found++;
				}
				break;
			}

			if (found)
				break;
		}

		if (!found && help_mode != 1)
			fprintf(stream, "Unknown Command !\r\n");
		else
			fprintf(stream, "\r\n");
	}

	/* Free resource */
	cli_io_free(cli->stream);
	free(cli);
	return 0;
}

/* Entry point of CLI */
static cyg_handle_t cli_thread_handle;
static cyg_thread cli_thread;
static char cli_stack[1024*16];

void
cli_start(void)
{
	cyg_thread_create(
		1,
		(cyg_thread_entry_t *)cli_main,
		-1,
		"cli console",
		(void *)&cli_stack[0],
		sizeof(cli_stack),
		&cli_thread_handle,
		&cli_thread);
	cyg_thread_resume(cli_thread_handle);
}
