/*
 * Run rc commands with a new thread
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: run_sh.c,v 1.3 2010-10-27 04:42:15 Exp $
 *
 */

#include <stdio.h>
#include <string.h>

extern int rc(int argc, char **argv);

static int sh_inited = 0;
static cyg_mutex_t sh_lock;

/* sh paramters */
#define NUM_RUN_SH	2
#define SH_STACK_SIZE	16384

#define MAX_ARGV	16
#define	MAX_ENV		64
#define	ARG_BUF_LEN	256
#define ENV_BUF_LEN	2048

struct sh_ctx {
	char *argv[MAX_ARGV];
	char arg_buf[ARG_BUF_LEN];
	char *env[MAX_ENV];
	char env_buf[ENV_BUF_LEN];

	int used;
	char tname[64];
	cyg_handle_t handle;
	cyg_thread thread;
	cyg_uint8 stack[SH_STACK_SIZE];
};

static struct sh_ctx sh_ctx[NUM_RUN_SH];


/* Main entry */
int
sh_main(struct sh_ctx *ctx)
{
	int argc = 0;
	char **argv;
	char **temp;
	char ***env = (char ***)cyg_thread_get_data_ptr(CYGNUM_KERNEL_THREADS_DATA_ENV);

	/* Set env */
	*env = ctx->env;

	/* Setup argc */
	argv = ctx->argv;
	for (temp = argv; *temp; temp++)
		argc++;

	/* Run shell */
	rc(argc, argv);

	/* Free this context */
	cyg_mutex_lock(&sh_lock);
	ctx->used = 0;
	cyg_mutex_unlock(&sh_lock);

	return 0;
}

/* Setup argv */
static int
sh_arg_setup(struct sh_ctx *ctx, char *cmd, char **argv)
{
	char *src;
	char *dst = ctx->arg_buf;
	char **pp = ctx->argv;
	int len;

	/* Do cleanup */
	memset(ctx->arg_buf, 0, sizeof(ctx->arg_buf));
	memset(ctx->argv, 0, sizeof(ctx->argv));

	if (argv == NULL) {
		strncpy(dst, cmd, sizeof(ctx->arg_buf)-1);
		*pp = dst;
		return 0;
	}

	/* Process argv */
	while ((src = *argv++) != NULL) {
		/* Check space left for arg buf */
		len = strlen(src);
		if (dst + len >= ctx->arg_buf + ARG_BUF_LEN) {
			printf("%s::arg_buf too small", __func__);
			break;
		}

		/* Copy to destination */
		memcpy(dst, src, len+1);
		*pp = dst;
		dst += len+1;

		/* Check space for argv */
		if (++pp >= ctx->argv + MAX_ARGV-1) {
			printf("%s::too many argvs", __func__);
			break;
		}
	}

	return 0;
}

/* Setup envp */
static int
sh_env_setup(struct sh_ctx *ctx, char **env)
{
	char *env_src;
	char *env_dst = ctx->env_buf;
	char **env_pp = ctx->env;
	int len;

	/* Do cleanup */
	memset(ctx->env_buf, 0, sizeof(ctx->env_buf));
	//memset(ctx->env_buf, 0, sizeof(ctx->env));
	memset(ctx->env, 0, sizeof(ctx->env));//roy modify,2010/08/26
	
	if (env == NULL)
		return 0;

	while ((env_src = *env++) != NULL) {
		/* Check space left for arg buf */
		len = strlen(env_src);
		if (env_dst + len >= ctx->env_buf + ENV_BUF_LEN) {
			printf("%s::env_buf too small", __func__);
			break;
		}

		/* Copy to destination */
		memcpy(env_dst, env_src, len+1);
		*env_pp = env_dst;
		env_dst += len+1;

		/* Check space for argv */
		if (++env_pp >= ctx->env + MAX_ENV-1) {
			printf("%s::too many envs", __func__);
			break;
		}
	}

	return 0;
}

/* Run a command line script */
int
run_sh(char *cmd, char *argv[], char *envp[])
{
	int i;
	struct sh_ctx *ctx;

	/* Init lock */
	if (sh_inited == 0) {
		cyg_mutex_init(&sh_lock);
		sh_inited = 1;
	}

	/* Allocate context */
	while (1) {
		cyg_mutex_lock(&sh_lock);
		for (i = 0; i < NUM_RUN_SH; i++) {
			if (sh_ctx[i].used == 0) {
				sh_ctx[i].used = 1;
				break;
			}
		}
		cyg_mutex_unlock(&sh_lock);

		if (i != NUM_RUN_SH)
			break;

		/* Wait 100ms */
		cyg_thread_delay(10);
		continue;
	}

	/* Init context */
	ctx = &sh_ctx[i];
	sprintf(ctx->tname, "run_sh%d", i);

	sh_arg_setup(ctx, cmd, argv);
	sh_env_setup(ctx, envp);

	/* Create thread */
	cyg_thread_create(
		6,
		(cyg_thread_entry_t *)sh_main,
		(cyg_addrword_t)ctx,
		ctx->tname,
		ctx->stack,
		sizeof(ctx->stack),
		&ctx->handle,
		&ctx->thread);
	cyg_thread_resume(ctx->handle);

	/* Wait till complete */
	while (1) {
		int used;

		cyg_mutex_lock(&sh_lock);
		used = ctx->used;
		cyg_mutex_unlock(&sh_lock);

		if (!used)
			break;

		/* Wait 100ms */
		cyg_thread_delay(10);
	}

	return 0;
}
