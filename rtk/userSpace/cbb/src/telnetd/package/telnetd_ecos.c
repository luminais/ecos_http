/*
 * TELNETD ecos main entrance.
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
#include <arpa/inet.h>
#include <shutils.h>

#include <shared.h>
#include <telnetd.h>


#define SERVER_PORT     23

/* Functions prototype declaration */
extern void telnetd_monitor_mainloop(struct telnetd_config *t_config);
extern int cli_main(int fd);
extern void stop_telnetd_monitor(void);


static cyg_handle_t telnetd_daemon_handle;
static char telnetd_daemon_stack[STACK_SIZE];
static cyg_thread telnetd_daemon_thread;
static int telnetd_monitor_running = 0;
static int telnetd_monitor_down = 0;
static int cli_num = 0;
static int bind_port = 6600;
static struct telnetd_config config;

struct thread_priv {
	cyg_handle_t t_handle;
	cyg_thread   t_thread;
	int t_running;
	int t_down;
	char t_stack[STACK_SIZE];
	char t_name[NAME_MAX_LEN];
	void *arg;
	void (*entry)(void *entry_arg);

};
struct  shell_priv {
	int shell_running;
	int shell_down;
	int ipc_skt_fd;
	int ipc_sfd;
	int ipc_cfd;
	int std_fd;
	int ipc_bind_port;
	struct thread_priv th_priv;
	int (*shell_entry)(int arg);
};

static void wait_thread_terminated(struct thread_priv *t_priv);
static void new_thread(struct thread_priv *t_priv);
static int ipc_connect(struct shell_priv *s_priv);
static void kill_cli(int cfd);

static void
stop_ipc(struct shell_priv *s_priv)
{
	if (s_priv) {
		if (s_priv->ipc_sfd) {
			close(s_priv->ipc_sfd);
			s_priv->ipc_sfd = -1;
		}

		if (s_priv->ipc_skt_fd) {
			close(s_priv->ipc_skt_fd);
			s_priv->ipc_skt_fd = -1;
		}
	}
}

static int
start_ipc(struct shell_priv *s_priv)
{
	int  s, ns;
	struct sockaddr_in saddr_in;
	socklen_t sz = sizeof(saddr_in);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		diag_printf("start_ipc: sock error\n");
		return -1;
	}

	memset(&saddr_in, '\0', sizeof(saddr_in));
	saddr_in.sin_family = AF_INET;
	saddr_in.sin_len = sizeof(saddr_in);
	saddr_in.sin_port = htons(s_priv->ipc_bind_port);
	saddr_in.sin_addr.s_addr = INADDR_ANY;


	if (bind(s, (struct sockaddr *) &saddr_in, sizeof(struct sockaddr_in)) < 0) {
		diag_printf("start_ipc: bind error\n");
		close(s);//hqw add for tcp 2014.01.24
		return -1;
	}

	if (listen(s, 1) < 0) {
		diag_printf("start_ipc: listen error\n");
		close(s);//hqw add for tcp 2014.01.24
		return -1;

	}

	if ((ns = accept(s, (struct sockaddr *)&saddr_in, &sz)) < 0) {


		diag_printf("start_ipc: accept error\n");
		close(s);//hqw add for tcp 2014.01.24
		return -1;
	}

	s_priv->ipc_skt_fd = s;
	s_priv->ipc_sfd = ns;
	return ns;
}

/* Initialize internal struct */
void
init_loginfo(struct login_info *linfo)
{
	memset(linfo, 0, sizeof(struct login_info));
	linfo->need_login = FALSE;
	linfo->default_shell = cli_main;
	sprintf(linfo->shell_name, "cli_%d", cli_num++);
}

/* 
 * server can obtain the socket associated to the shell by calling ipc_connect
 */
static int
ipc_connect(struct shell_priv *s_priv)
{
	int size, s, ret = 0;
	struct sockaddr_in scli;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ret = -1;
		goto quit;
	}

	scli.sin_family = AF_INET;
	scli.sin_port = htons(s_priv->ipc_bind_port);
	scli.sin_len = sizeof(scli);
	scli.sin_addr.s_addr = inet_addr("127.0.0.1");  /* Loopback address */
	size = sizeof(struct sockaddr_in);

	if (connect(s, (struct sockaddr *)&scli, size) < 0) {
		ret = -1;
		goto quit;
	}

	ret = s;
	s_priv->ipc_cfd = s;

quit:
	if (ret < 0) {
		if (s >= 0)//hqw add for tcp 2014.01.24
			close(s);
	}

	return ret;
}

/* Clear the rdrfd associated with thread */
static void
clear_rdrfd(cyg_handle_t handle)
{
	cyg_thread_set_rdrfd(handle, -1);
}


/* 
 * Bind the fd with current thread, by which we can redirect stdin/out/err
 * to the desired net fd.
 */
static void
save_rdrfd(int fd)
{
	cyg_handle_t handle;

	handle = cyg_thread_self();

	cyg_thread_set_rdrfd(handle, fd);

}


static void
shell_mainloop(void *entry_data)
{
	struct shell_priv *s_priv;
	struct thread_priv *t_priv;
	int ipc_fd = 0;

	s_priv = (struct shell_priv *)entry_data;

	t_priv = &s_priv->th_priv;

	if (!s_priv->shell_running) {
		s_priv->shell_running = 1;
		s_priv->shell_down = 0;

		/* IPC socket initailization */
		if ((ipc_fd = start_ipc(s_priv)) < 0)
		{
			diag_printf("ipc_connect failed\n");
			goto out;
		}

		/*
		 * Save the rdrfd to process, with it we can identify 
		 * which process cli belongs to  
		 */
		save_rdrfd(ipc_fd);


		/* Enter the shell entry function */
		s_priv->shell_entry(ipc_fd);

		/* Clear the resources after cli exits */
		clear_rdrfd(t_priv->t_handle);
		stop_ipc(s_priv);
		s_priv->std_fd = -1;
	}
out:

	s_priv->shell_running = 0;
	s_priv->shell_down = 1;
}


static char *
get_ipaddr(void)
{
	char *ipaddr = NULL, *proto = NULL;

	ipaddr = nvram_get("lan_ipaddr");
	proto = nvram_get("lan_proto");
	if (!ipaddr && !proto) {
		diag_printf("telnetd:No LAN IP address and protocol specified.\n");
		goto out;
	}

	if (!ipaddr && !strcmp(proto, "static")) {
		diag_printf("telnetd:No static IP address specified for LAN.\n");
		goto out;
	}
out:
	return ipaddr;
}

int
telnetd_osl_is_shell_running(void *handle)
{
	struct shell_priv *s_priv = (struct shell_priv *)handle;

	return s_priv->shell_running;
}

/* 
 * Try to get ipc fd from shell, we can't go further without it. 
 */ 
int
telnetd_osl_getfd_shell(void *shell_handle)
{
	struct shell_priv *s_priv = (struct shell_priv *)shell_handle;

	return ipc_connect(s_priv);
}

/*  
 * Thread-create shell process, which imitates the behavior of 'fork shell' in NetBSD, 
 * although we don't have it actually.  
 */ 
void *
telnetd_osl_exec_shell(int (*shell)(int), char *name)
{
	struct shell_priv *s_priv;
	struct thread_priv *t_priv;

	s_priv  = (void *)calloc(sizeof(struct shell_priv), 1);

	if (s_priv) {
		memset(s_priv, 0, sizeof(struct shell_priv));

		/* Initialize thread_priv struct */
		t_priv = &s_priv->th_priv;
		t_priv->entry = shell_mainloop;
		t_priv->arg = s_priv;

		s_priv->shell_entry = shell;
		s_priv->ipc_bind_port = bind_port++;

		memcpy(t_priv->t_name, name, NAME_MAX_LEN);
		new_thread(t_priv);

		while (!s_priv->shell_running && !s_priv->shell_down)
			cyg_thread_delay(5);
	}

	else {
		diag_printf("alloc s_priv error\n");

	}

	return (void *)s_priv;
}

void *
telnetd_osl_new_thread(void *entry, void *parg, char *name)
{
	struct thread_priv *t_priv;

	t_priv  = (struct thread_priv *)calloc(sizeof(struct thread_priv), 1);

	if (t_priv) {
		memset(t_priv, 0, sizeof(struct thread_priv));
		memcpy(t_priv->t_name, name, NAME_MAX_LEN);
		t_priv->entry = entry;
		t_priv->arg = parg;

		new_thread(t_priv);
	}

	return (void *)t_priv;
}

void
telnetd_osl_delay(int tick)
{
	cyg_thread_delay(tick);
}

void
telnetd_osl_th_shutdown(void *handle)
{
	struct thread_priv *t_priv = (struct thread_priv *)handle;

	wait_thread_terminated(t_priv);
}

void
telnetd_osl_kill_shell(void *handle)
{
	struct shell_priv *s_priv = (struct shell_priv *)handle;

	if (s_priv)
	{
		if (s_priv->shell_running) {

			/* send kill signal to shell */
			kill_cli(s_priv->ipc_cfd);
			cyg_thread_delay(5);
		}

		/* wait until the shell thread is terminated */
		wait_thread_terminated(&s_priv->th_priv);
	}
}


static void
new_thread(struct thread_priv *t_priv)
{
	cyg_thread_create(
			0,
			(cyg_thread_entry_t *)t_priv->entry,
			(cyg_addrword_t)t_priv->arg,
			t_priv->t_name,
			t_priv->t_stack,
			sizeof(t_priv->t_stack),
			&t_priv->t_handle,
			&t_priv->t_thread);
	cyg_thread_resume(t_priv->t_handle);
}


static void
kill_cli(int cfd)
{
	const char *exit_cmd = "exit\n";

	write(cfd, exit_cmd, strlen(exit_cmd));
}

static void
telnetd_main(void)
{
	telnetd_monitor_running = 1;

	telnetd_monitor_mainloop(&config);

	telnetd_monitor_running = 0;
	telnetd_monitor_down = 1;
	return;
}

/*
 * Functions to raise the telnetd monitor daemon,
 * called by application main entry and
 * the mointor task.
 */
int
telnetd_start(void)
{
	char *addr;

	if (telnetd_monitor_running == 0) {
		telnetd_monitor_down = 0;
		memset(&config, 0, sizeof(struct telnetd_config));
		addr = get_ipaddr();
		strcpy(config.ipaddr, addr);
		config.port = SERVER_PORT;
		cyg_thread_create(
				0,
				(cyg_thread_entry_t *)telnetd_main,
				0,
				"telnetd_main",
				telnetd_daemon_stack,
				sizeof(telnetd_daemon_stack),
				&telnetd_daemon_handle,
				&telnetd_daemon_thread);
		cyg_thread_resume(telnetd_daemon_handle);

		/* Wait until thread scheduled */
		while (!telnetd_monitor_running && !telnetd_monitor_down)
			cyg_thread_delay(1);
	}

	return 0;
}


static void
wait_thread_terminated(struct thread_priv *t_priv)
{
	int pid;
	char *name = t_priv->t_name;


	if (oslib_getpidbyname(name) == 0)
		goto out;

	/* Wait until thread exit */
	pid = oslib_getpidbyname(name);
	if (pid) {
		while (oslib_waitpid(pid, NULL) != 0)
		{
			cyg_thread_delay(5);
		}
	}
out:
	cyg_thread_delete(t_priv->t_handle);
}

/* shutdown telnetd monitor daemon */
void
telnetd_stop(void)
{
	int pid;

	stop_telnetd_monitor();
	if (oslib_getpidbyname("telnetd_main") == 0)
		return;

	/* Wait until thread exit */
	pid = oslib_getpidbyname("telnetd_main");
	if (pid) {
		while (oslib_waitpid(pid, NULL) != 0)
		{
			cyg_thread_delay(5);
		}
	}


	return;
}
