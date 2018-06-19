/*
 * ecos osl library.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ecos_oslib.c,v 1.4 2010-07-21 03:42:14 Exp $
 */

#include <ecos_oslib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <bcmnvram.h>

char *
oslib_make_nvname(char *prefix, int ifunit, char *name)
{
	static char buf[100];

	if (ifunit)
		snprintf(buf, sizeof(buf), "%s%d_%s", prefix, ifunit, name);
	else
		snprintf(buf, sizeof(buf), "%s_%s", prefix, name);

	return buf;
}

int
oslib_ifname_list(char *ifname_list)
{
	int count;
	char *value;
	char *name;

	ifname_list[0] = 0;

	/* Cat all lan interface together */
	for (count = 0; count < MAX_NO_BRIDGE; count++) {
		char buf[IFNAMSIZ + 8];

		name = oslib_make_nvname("lan", count, "ifname");
		value = nvram_get(name);
		if (value) {
			if (ifname_list[0] != 0)
				strcat(ifname_list, " ");

			/* For example, the ifnames_list will be 0=br0 1=br1 ... */
			sprintf(buf, "%d=%s", count, value);
			strcat(ifname_list, buf);
		}
	}

	return 0;
}

void
oslib_ticks_sleep(int knl_ticks)
{
	cyg_thread_delay(knl_ticks);
}

int
oslib_pid()
{
	cyg_handle_t thread = cyg_thread_self();
	return (pid_t)cyg_thread_get_id(thread);
}

int
oslib_getpidbyname(char *tname)
{
	cyg_handle_t thread = 0;
	cyg_uint16 id = 0;
	cyg_thread_info info;

	/* Loop over the threads, and generate a table row for
	 * each.
	 */
	while (cyg_thread_get_next(&thread, &id))
	{
		if (thread == 0)
			break;

		cyg_thread_get_info(thread, id, &info);

		if (strcmp(info.name, tname) == 0) {
			/* thread in EXIT state. */
			if ((info.state & 0x1b) == 0x10)
				break;

			return id;
		}
	}

	return 0;
}

int
oslib_waitpid(int pid, int *status)
{
	cyg_handle_t thread = 0;
	cyg_uint16 id = 0;
	cyg_thread_info info;

	/* Loop over the threads, and generate a table row for
	 * each.
	 */
	while (cyg_thread_get_next(&thread, &id)) {
		if (thread == 0)
			break;

		if (pid == (int)id) {
			cyg_thread_get_info(thread, id, &info);
			/* thread in EXIT state. */
			if ((info.state & 0x1b) == 0x10) {
				return 0;
			}
			return pid;
		}
	}

	return 0;
}

pid_t
getpid(void)
{
	cyg_handle_t thread = cyg_thread_self();
	return (pid_t)cyg_thread_get_id(thread);
}


int oslib_getnamebypid(pid_t pid, char *name)
{
	cyg_handle_t thread = 0;
	cyg_uint16 id = 0;
	cyg_thread_info info;

	if (name == NULL)
		return 1;
	/* Loop over the threads, and generate a table row for
	 * each.
	 */
	while (cyg_thread_get_next(&thread, &id)) {
		if (thread == 0)
			break;
		if (pid == (int)id) {
			cyg_thread_get_info(thread, id, &info);
			strcpy(name, info.name);
			return 0;
		}
	}
	return 1;
}

int
gethostname(char *name, size_t len)
{
	int i;
	char hostname[256];
	char *value;

	hostname[0] = 0;

	value = nvram_get("wan_hostname");
	if (value) {
		strncpy(hostname, value, sizeof(hostname)-1);
		hostname[sizeof(hostname)-1] = 0;
	}

	i = 1 + strlen(hostname);
	if (i > len)
		i = len;

	memcpy(name, hostname, i);
	return 0;
}
