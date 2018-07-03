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
#include <cyg/kernel/kapi.h>
//#include <shutils.h>
#include <nvram.h>
#include <commands.h>
// Function decration
int run_clicmd(char *command_buf);

/*static */int
get_argv(char *string, char *argv[])
{
	int n = 0;
	char *p = string;

	while (*p) {
		argv[n] = p;

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

static char* insert_char(char *s, int index, char c)
{
	int i;
	int len;
	
	if(!s)
		return s;
	len = strlen(s);
	if(index < -1 || len < index + 1)
		return s;

	for(i = len; i > index; i--)
		s[i+1] = s[i];
	s[index+1] = c;

	return s;
}

static char* del_char(char *s, int index)
{
	int i, len;

	if(!s)
		return s;
	len = strlen(s);
	if(index < 0 || index > len - 1)
		return s;
	for(i = index; i < len; i++)
		s[i] = s[i+1];
	s[len] = '\0';
	
	return s;	
}

static int
cli_gets(cli_context *cli)
{
	char c;
	int size = 0;
	int esc_f1 = 0, esc_f2 = 0;
	int idx = cli->history_idx;
	char *buf = cli->cmd_buf;
	int info_len = strlen(cli->prompt);		//行首提示信息长度
	FILE *stream = cli->stream;
	int len;

	static int insert_mode = 1;	//插入模式
	int input_idx = -1;		//插入的起始下标，实际插入位置 buf[idx+1]
	int  home_key3 = 0, end_key3 = 0, insert_key3 = 0;

	cli_node_t *cn;
	int spare_cmd_cnt = 0;
	char cmd_tmp[64] = {0};
	
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
			if(size == 0 || input_idx < 0){
				continue;
			}
			else{
				del_char(buf, input_idx);
				input_idx--;
				size--;
				fprintf(stream, "\x1b[2K");	//清除本行
				fprintf(stream, "\r%s %s", cli->prompt, buf);	//输出提示符和已输入的命令
				fprintf(stream, "\r\033[%dC", info_len + input_idx + 2);	//输出光标位置
				fflush(stream);
			}
			continue;
		case 0x09:	/*tab*/
			for (cn = __cli_tab__; cn != &__cli_tab_end__; cn++){
				if(!strncmp(cn->name, buf, size)){
					spare_cmd_cnt++;
					if(spare_cmd_cnt == 1){
						strcpy(cmd_tmp, cn->name);
						continue;
					}
					else if(spare_cmd_cnt == 2){
						fprintf(stream, "\n%s\t%s", cmd_tmp, cn->name);
					}
					else{
						fprintf(stream, "\t%s", cn->name);
					}
				}
			}
			if(spare_cmd_cnt==0){
				continue;
			}
			else if(spare_cmd_cnt == 1){
				strcpy(buf, cmd_tmp);
				size = strlen(cmd_tmp);
				input_idx = size - 1;
				fprintf(stream, "\x1b[2K");	//清除本行
				fprintf(stream, "\r%s %s", cli->prompt, buf);
			}
			else{
				//应该输出备选命令的最大公共前缀，这里还没做这种处理
				fprintf(stream, "\n%s %s", cli->prompt, buf);	//输出提示符和已输入的命令
			}
			fprintf(stream, "\r\033[%dC", info_len + input_idx + 2);	//输出光标位置
			fflush(stream);
			spare_cmd_cnt = 0;
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
		
			
		case 0x31:
			if(esc_f1 && esc_f2){
				home_key3 = 1;
				continue;
			}
			break;
			
		case 0x34:
			if(esc_f1 && esc_f2){
				end_key3 = 1;
				continue;
			}
			break;

		case 0x32:
			if(esc_f1 && esc_f2){
				insert_key3 = 1;
				continue;
			}
			break;
		case 0x7e:
			if(esc_f1 && esc_f2){
				if(home_key3){
					input_idx = -1;
					fprintf(stream, "\r\033[%dC", info_len + 1);
					fflush(stream);
				}
				else if(end_key3){
					input_idx = size - 1;
					fprintf(stream, "\r\033[%dC", info_len + 1 + size);
					fflush(stream);
				}
				else if(insert_key3){
					insert_mode = ~insert_mode & 0x1;
				}
				esc_f1 = esc_f2 = home_key3 = end_key3 = insert_key3 = 0;
				continue;
			}
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
			
		case 0x43:	/*right*/
			if (esc_f1 && esc_f2) {
				esc_f1 = esc_f2 = 0;
				if(input_idx + 1 >= size)	//光标移到右边界
					continue;
				input_idx++;
				fprintf(stream, "\033[C");
				fflush(stream);
				continue;
			}
			break;
			
		case 0x44:	/*left*/
			if (esc_f1 && esc_f2) {
				esc_f1 = esc_f2 = 0;
				if(input_idx + 1 <= 0)	//光标移到左边界
					continue;
				input_idx--;
				fprintf(stream, "\033[D");
				fflush(stream);
				continue;
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
			input_idx = size - 1;
			memcpy(buf, cli->history[idx], size+1);

			fprintf(stream, "\x1b[2K");
			fprintf(stream, "\r%s %s", cli->prompt, buf);
			fflush(stream);
		}
		else if (size < CLI_MAX_CMD_LEN) {
			if(insert_mode){		//插入模式
				insert_char(buf, input_idx, c);
				input_idx++;
				size++;
			}
			else{
				if(input_idx + 1 < size)	
					buf[++input_idx] = c;
				else{
					insert_char(buf, input_idx, c);
					input_idx++;
					size++;
				}
			}

			if(input_idx + 1 < size)
			{
				fprintf(stream, "\x1b[2K");	//清除本行
				fprintf(stream, "\r%s %s", cli->prompt, buf);	//输出提示符和已输入的命令
				fprintf(stream, "\r\033[%dC", info_len + input_idx + 2);	//输出光标位置
			}
			else
			{
				fputc((int)c, stream);
			}
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
	// +++++
	char cmdbuf_tmp[CLI_MAX_CMD_LEN];
	// +++++
	
	cyg_thread_delay(50); 	// wait, until nvram initialized
	
	/* Allocate context */
	cli = (cli_context *)calloc(sizeof(*cli), 1);
	if (!cli)
		return -1;

	/* Init console io */
	cli->fd = fd;
	cli->stream = cli_io_init(fd); 	// cli->stream = stdout
	if (cli->stream == NULL) {
		free(cli);
		return -1;
	}

	/* Set command loop */
	nv_prompt = nvram_get("cli_prompt"); 	// nv_prompt=NULL
	if (NULL == nv_prompt)
	{
		nv_prompt = CLI_DEFAULT_PROMPT; 	// nv_prompt="CLI>"
	}

	strncpy(cli->prompt, nv_prompt, sizeof(cli->prompt)-1);
	prompt = cli->prompt;
	buf = cli->cmd_buf;
	stream = cli->stream;

	while (1) {
		fprintf(stream, "%s ", prompt); 	// prompt="CLI>"
		fflush(stream);

		memset(buf, 0, CLI_MAX_CMD_LEN);
		memset(cmdbuf_tmp, 0, CLI_MAX_CMD_LEN);

		if (cli_gets(cli) < 0) {
			/* IO error, quit */
			break;
		}
		strncpy(cmdbuf_tmp, buf, CLI_MAX_CMD_LEN-1);
		cmdbuf_tmp[CLI_MAX_CMD_LEN-1]='\0';
		
		//printf("\n%s_%s:%s\n", __func__, __LINE__, buf);
		
		//printf("cmd:%s\n", cli->cmd_buf);
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
				printf("Realtek's command:\n");
				run_clicmd(cmdbuf_tmp);
				printf("\nTenda's command:\n");
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
		{
			run_clicmd(cmdbuf_tmp);
			// fprintf(stream, "Unknown Command !\r\n");
		}
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
/*bufferL 优先级会导致待机量测试丢包率多0.8%*/
void
cli_start(void)
{
	cyg_thread_create(
		8,
		(cyg_thread_entry_t *)cli_main,
		-1,
		"cli console",
		(void *)&cli_stack[0],
		sizeof(cli_stack),
		&cli_thread_handle,
		&cli_thread);
	cyg_thread_resume(cli_thread_handle);
}

#ifdef CONFIG_TENDA_ATE_REALTEK
int run_tenda_cmd(char *command_buf)
{
    cli_node_t *cn;
    int argc;
    char *argv[CLI_MAX_ARGV+1];

    argc = get_argv(command_buf, argv);
    for (cn = __cli_tab__; cn != &__cli_tab_end__; cn++) {
        if (!strcmp(argv[0], cn->name)) {
            (*cn->func)(argc, argv);
            return 1;
        }
    }
    return 0;
}
#endif


