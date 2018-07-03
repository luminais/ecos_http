/*
 * shared.h
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

#ifndef _SHARED_H_
#define _SHARED_H_


#include <ecos_oslib.h>
#include <cyg/hal/drv_api.h>


#define NAME_MAX_LEN 64
#define STACK_SIZE 65535

enum {
	INITIATE_STATE = 1,
	SHUTDOWN_STATE = 2,
	FORK_THREAD_DIED_STATE = 3
};

struct login_info {
	void *shell_handle;
	bool need_login;
	int (*default_shell)(int entry_data);
	void (*login_task)(void);
	char shell_name[NAME_MAX_LEN];

};

struct telnetd_context {
	int conn_fd;
	int listen_fd;
	struct login_info ln_info;
	int running;
	int down;
	int status_flag;
	char thread_name[NAME_MAX_LEN];
	char telnetd_stack[STACK_SIZE];
	cyg_handle_t telnetd_handle;
	cyg_thread   telnetd_thread;
	void *thread_handle;
	struct telnetd_context *next;
	struct telnetd_monitor *t_mtr;
};

struct telnetd_config {
	unsigned short port;
	char ipaddr[30];
};

typedef union {
	struct sockaddr sa;
	struct sockaddr_in sa_in;
} u_sockaddr;


struct telnetd_monitor {
	int skt_fd;
	int pnum;
	int client_num;
	int telnetd_counter;
	u_sockaddr usa;
	struct telnetd_context *telnetd_list;
};

#endif /* endif _SHARED_H_ */
