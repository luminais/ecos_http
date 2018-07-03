
/*
 * telnetd_monitor.c
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id:
 */

#include <shared.h>
#include <arpa/inet.h>
#include <telnet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <limits.h>
#include <shared.h>
#include <defs.h>


#define MAX_BACKLOG 100
#define MAX_CLIENT_NUM 1

/* Functions prototype declaration */
extern void *telnetd_osl_new_thread(void *entry, void *parg, char *name);
extern void req_handler(struct telnetd_context *t_ctx);
extern void init_loginfo(struct login_info *linfo);
extern void telnetd_shutdown(struct telnetd_context *t_ctx);

static int telnetd_monitor_flag;
static void stop_telnetd_handler(struct telnetd_monitor *t_mtr, struct telnetd_context *t_ctx);

static struct telnetd_context *init_tel(void)
{
	struct telnetd_context *t_cxt;
	struct login_info *linfo;

	t_cxt = (void *)calloc(sizeof(struct telnetd_context), 1);

	if (t_cxt) {
		memset(t_cxt, 0, sizeof(struct telnetd_context));

		linfo = &t_cxt->ln_info;

		init_loginfo(linfo);
	}

	return t_cxt;
}

static void
telnetd_monitor_deinit(struct telnetd_monitor *t_mtr)
{
	free(t_mtr);
}

static int
initialize_listen_socket(u_sockaddr* usaP, char *ipaddr, unsigned short port)
{
	int i;
	struct in_addr addr;
	int listen_fd;
	int ret = 0;

	memset(usaP, 0, sizeof(u_sockaddr));


	if (!ipaddr) {
		TELNETD_PRINT("telnetd:No IP address specified for LAN.\n");
		ret = -1;
		goto out;
	}

	if (!inet_aton(ipaddr, &addr)) {
		TELNETD_PRINT("telnetdd:Invalid IP address specified for LAN.\n");
		ret = -1;
		goto out;
	}

	usaP->sa_in.sin_addr = addr;

	usaP->sa.sa_family = AF_INET;
	usaP->sa_in.sin_port = htons(port);
	listen_fd = socket(usaP->sa.sa_family, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		TELNETD_PRINT("telnetd:socket failed\n");
		ret = -1;
		goto out;
	}

	(void) fcntl(listen_fd, F_SETFD, 1);

	i = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &i,
	sizeof(i)) < 0) {
		TELNETD_PRINT("setsockopt error\n");
		ret = -1;
		goto out;
	}
	if (bind(listen_fd, &usaP->sa, sizeof(struct sockaddr_in)) < 0) {
		TELNETD_PRINT("bind error\n");
		ret = -1;
		goto out;
	}

	if (listen(listen_fd, MAX_BACKLOG) < 0) {
		TELNETD_PRINT("listen error\n");
		ret = -1;
		goto out;
	}

	ret = listen_fd;
out:
	if (ret < 0) {
		if (listen_fd)
			close(listen_fd);
	}

	return ret;
}

void
stop_telnetd_monitor(void)
{
	/* 
	 * Force the state machine to enter shutdown mode, which will trigger 
	 * the cleanup procedure and terminate any associated things.
	 */
	telnetd_monitor_flag = SHUTDOWN_STATE;
}

static void
telnetd_add_to_list(struct telnetd_monitor *t_mtr, struct telnetd_context *t_ctx)
{

	/* Prepend the new ctx to list */
	t_ctx->next = t_mtr->telnetd_list;
	t_mtr->telnetd_list = t_ctx;

}

static void
telnetd_delete_from_list(struct telnetd_monitor *t_mtr, struct telnetd_context *t_ctx)
{
	struct telnetd_context *temp, *prev;

	prev = NULL;

	temp = t_mtr->telnetd_list;

	while (temp != NULL)
	{
		if (temp->telnetd_handle == t_ctx->telnetd_handle) {
			if (temp == t_mtr->telnetd_list) {
				t_mtr->telnetd_list = temp->next;
				goto out;
			}
			else {
				prev->next = temp->next;
				goto out;
			}
		}
		else {
			prev = temp;
			temp = temp->next;
		}

	}
out:
	return;
}

static void
telnetd_handler_main(struct telnetd_context *t_ctx)
{
	t_ctx->running = 1;
	t_ctx->down = 0;

	/* Enter the main function */
	req_handler(t_ctx);

	t_ctx->running = 0;
	t_ctx->down = 1;

}

static void
check_telnetd_running(struct telnetd_monitor *t_mtr)
{
	struct telnetd_context *tel_ctx;

	for (tel_ctx = t_mtr->telnetd_list; tel_ctx != NULL; tel_ctx = tel_ctx->next) {
		if ((tel_ctx->running == 0) && (tel_ctx->down == 1)) {
			stop_telnetd_handler(t_mtr, tel_ctx);
		}
	}
}

static int
start_telnetd_handler(struct telnetd_monitor *t_mtr, int c_fd)
{
	int ret = 0;
	struct telnetd_context *tel_ctx;

	/* Initialize the telnetd context */
	if (!(tel_ctx = (struct telnetd_context *)init_tel())) {
		TELNETD_PRINT("init_tel error\n");
		ret = -1;
		goto quit;
	}

	tel_ctx->t_mtr = t_mtr;
	tel_ctx->conn_fd = c_fd;

	sprintf(tel_ctx->thread_name, "telnetd_%d", t_mtr->telnetd_counter++);

	/* Do the creation of request handler to handle requests from client */
	tel_ctx->thread_handle = telnetd_osl_new_thread((void *)telnetd_handler_main,
	(void *)tel_ctx, tel_ctx->thread_name);

	if (!tel_ctx->thread_handle) {
		TELNETD_PRINT("create new thread error\n");
		ret = -1;
		goto quit;
	}

	telnetd_add_to_list(t_mtr, tel_ctx);

	t_mtr->pnum++;


quit:
	if (ret < 0) {
		if (tel_ctx)
			free(tel_ctx);
	}

	return ret;
}

const char *err_msg = "Error! The number of connections execeeds maximum loading, reject!\n";

static void
output_msg(int cfd, const char *str)
{
	if (cfd >= 0) {
		write(cfd, err_msg, strlen(str));
	}
}

static void
telnetd_monitor_dispatch(struct telnetd_monitor *t_mtr)
{
	int conn_fd, maxfd;
	struct timeval tval = {1, 0};
	socklen_t sz = sizeof(t_mtr->usa);
	fd_set active_fd;

	maxfd = t_mtr->skt_fd;
	FD_ZERO(&active_fd);
	FD_SET(t_mtr->skt_fd, &active_fd);

	if (select (maxfd+1, &active_fd, NULL, NULL, &tval) > 0) {
		if (FD_ISSET(t_mtr->skt_fd, &active_fd)) {
			if ((conn_fd = accept(t_mtr->skt_fd, &(t_mtr->usa.sa), &sz)) < 0) {
				TELNETD_PRINT("accept error\n");
				return;
			}

			TELNETD_PRINT("new client arrive, fd:%d\n", conn_fd);

			if (t_mtr->pnum >= MAX_CLIENT_NUM) {

				TELNETD_PRINT("The number of connection exceeds %d\n",
				MAX_CLIENT_NUM);
				output_msg(conn_fd, err_msg);
				close(conn_fd);
				return;
			}

			/* spawn new thread to process the request */
			if (start_telnetd_handler(t_mtr, conn_fd) < 0) {
				TELNETD_PRINT("start telnetd handler failed\n");
				close(conn_fd);
			}

		}
	}

} /* end of telnetd_monitor_dispatch */

static void
stop_telnetd_handler(struct telnetd_monitor *t_mtr, struct telnetd_context *t_ctx)
{
	/* set the shutdown flag */
	telnetd_shutdown(t_ctx);

	if (t_ctx->conn_fd >= 0)
		close(t_ctx->conn_fd);

	telnetd_delete_from_list(t_mtr, t_ctx);
	free(t_ctx->thread_handle);
	free(t_ctx);
	t_mtr->pnum--;
}


static void
telnetd_monitor_shutdown(struct telnetd_monitor *t_mtr)
{
	struct  telnetd_context *t_ctx;

	TELNETD_PRINT("telnetd_monitor_shutdown\n");

	/* wait all threads being terminated */
	for (t_ctx = t_mtr->telnetd_list; t_ctx != NULL; t_ctx = t_ctx->next) {
		stop_telnetd_handler(t_mtr, t_ctx);
	}

	if (t_mtr->skt_fd >= 0) {
		close(t_mtr->skt_fd);
		t_mtr->skt_fd = -1;
	}
}

static struct telnetd_monitor *telnetd_monitor_init(char *ip_addr, unsigned short port)
{
	struct telnetd_monitor *t_mtr;

	telnetd_monitor_flag = INITIATE_STATE;

	t_mtr = (void *)calloc(sizeof(struct telnetd_monitor), 1);

	if (t_mtr) {
		memset(t_mtr, 0, sizeof(struct telnetd_monitor));
		t_mtr->telnetd_list = 0;
	}
	else {
		TELNETD_PRINT("alloc telnetd monitor context error\n");
		goto err_out;
	}

	if ((t_mtr->skt_fd = initialize_listen_socket(&t_mtr->usa, ip_addr, port)) < 0) {
		TELNETD_PRINT("can't bind to any address\n");
		goto err_out;
	}

	return t_mtr;

err_out:
	if (t_mtr)
		free(t_mtr);


	return NULL;
}

void
telnetd_monitor_mainloop(struct telnetd_config *config)
{
	struct telnetd_monitor *t_mtr;

	TELNETD_PRINT("telnetd_monitor_mainloop\n");

	/* Initailize telnetd monitor */
	if (!(t_mtr = telnetd_monitor_init(config->ipaddr, config->port)))
		return;

	/* Loop forever handling requests */
	while (1) {
		switch (telnetd_monitor_flag) {
		case SHUTDOWN_STATE:
			telnetd_monitor_shutdown(t_mtr);
			goto out;
		default:
			telnetd_monitor_dispatch(t_mtr);
			check_telnetd_running(t_mtr);
			break;
		}
	} /* end of while */
out:
	telnetd_monitor_deinit(t_mtr);
	return;
}
