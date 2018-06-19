/*
 * DNSMASQ ecos main entrance.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dnsmasq_ecos.c,v 1.8 2010-08-27 10:23:33 Exp $
 */
#include <dnsmasq.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <iflib.h>
#include <ecos_oslib.h>

#define STACK_SIZE 12*1024

cyg_handle_t dns_thread_h;
cyg_thread dns_thread;
static cyg_uint8 dns_stack[STACK_SIZE];
static int dnsmasq_running = 0;
static int dnsmasq_down = 0;

/*
 * Daemon of DNSMASQ module.
 * This function calls the portable
 * main loop of the dnsmasq functions.
 */
static void
dnsmasq_main(void)
{
	dnsmasq_running = 1;

	/* Enter os independent main loop */
	dnsmasq_mainloop();

	dnsmasq_running = 0;
	dnsmasq_down = 1;
	return;
}

/*
 * Functions to raise the DNSMASQ daemon,
 * called by application main entry and
 * the mointor task.
 */
void
dnsmasq_start(void)
{
	/*
	 * Don't enable DNS masquerate when
	 * router mode is not enabled.
	 */
	if (nvram_match("router_disable", "1")) {
		/* Terminate it anyway */
		dnsmasq_terminate();
		return;
	}

	if (dnsmasq_running == 0) {
		dnsmasq_down = 0;
		cyg_thread_create(
			9,
			(cyg_thread_entry_t *)dnsmasq_main,
			0,
			"DNS daemon",
			dns_stack,
			sizeof(dns_stack),
			&dns_thread_h,
			&dns_thread);
		cyg_thread_resume(dns_thread_h);

		/* Wait until thread scheduled */
		while (!dnsmasq_running && !dnsmasq_down)
			cyg_thread_delay(1);
	}
}

/*
 * Functions to shut down the DNSMASQ daemon,
 * called by the mointor task.
 */
void
dnsmasq_stop(void)
{
	int pid;

	dnsmasq_terminate();

	/* Wait until thread exit */
	pid = oslib_getpidbyname("DNS daemon");
	if (pid) {
		while (oslib_waitpid(pid, NULL) != 0)
		{
			dnsmasq_terminate();//add by ldm
			cyg_thread_delay(1);
		}
	}
	return;
}

void
dnsmasq_restart(void)
{
	dnsmasq_stop();
	dnsmasq_start();
}

/*
 * Functions below are the osl dependent
 * implementation needed by the dnsmasq
 * module.  The prefix of them is
 * 'dnsmasq_osl_' to avoid naming pollution
 */

/* Return a localhosts string */
void
dnsmasq_osl_localhost_init(char *localhost)
{
	char *name, *dn;

	localhost[0] = 0;

	name = nvram_safe_get("wan_hostname");
	dn = nvram_safe_get("wan_domain");
	if ((strlen(name) + strlen(dn)) < 256) {
		if (name == 0) {
			return;
		}
		else if (dn == 0) {
			sprintf(localhost, "%s", name);
		}
		else {
			sprintf(localhost, "%s.%s", name, dn);
		}
	}
}

/* Retur a hosts string */
int
dnsmasq_osl_host_list(char *hostlist)
{
	int i;
	char name[16];
	char *value;
	char buf[256];
	char tmp[256];

	char *next;
	char *domain, *ip, *flag;
	struct in_addr addr;

	hostlist[0] = 0;

	for (i = 0; i < MAXHOSTSENTRY; i++) {
		sprintf(name, "dns_host%d", i);
		value = nvram_get(name);
		if (value == 0)
			continue;

		strcpy(buf, value);

		/* Decompose domain name */
		domain = buf;
		strtok_r(domain, ";", &next);
		if (next == 0)
			continue;

		/* Decompose ip */
		ip = next;
		strtok_r(0, ";", &next);
		if (next == 0)
			continue;

		if (inet_aton(ip, &addr) == 0)
			continue;

		/* Decompose flag */
		flag = next;
		strtok_r(0, ";", &next);
		if (next != 0)
			continue;

		if (atoi(flag) != 1)
			continue;

		/* Print to host list */
		if (hostlist[0] != 0)
			strcat(hostlist, "\n");

		/*
		 * Print hostlist with the format such as
		 * testhost1 192.168.1.100<newline>
		 * testhost2 192.168.1.150<newline>
		 * testhost3 172.27.1.20
		 */
		sprintf(tmp, "%s %s", domain, ip);
		strcat(hostlist, tmp);
	}

	return 0;
}

/* Return the lan ifname string */
int
dnsmasq_osl_ifname_list(char *ifname_list)
{
	char tmp[100];
	char *value;
	int i;
	char lan_ifname[32];

	ifname_list[0] = 0;

	/* Cat all lan interface together */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		char buf[64];
	
		/* Read name name */
		if (i == 0) {
			value = nvram_get("lan_ifname");
		}
		else {
			snprintf(tmp, sizeof(tmp), "lan%x_ifname", i);
			value = nvram_get(tmp);
		}
		if (value == 0)
			continue;

		strcpy(lan_ifname, value);
		if (iflib_getifaddr(lan_ifname, 0, 0) != 0)
			continue;

		/* Cate the ifname list */
		if (ifname_list[0] != 0)
			strcat(ifname_list, " ");

		sprintf(buf, "%d=%s", i, lan_ifname);
		strcat(ifname_list, buf);
	}

	return 0;
}

int
dnsmasq_osl_ifaddr(char *ifname, struct in_addr *ipaddr)
{
	return iflib_getifaddr(ifname, ipaddr, 0);
}

int
dnsmasq_osl_dns_list(char *buf)
{
	if (buf == NULL)
		return -1;

	/* Read dns table */
	strcpy(buf, nvram_safe_get("resolv_conf"));
	return 0;
}

