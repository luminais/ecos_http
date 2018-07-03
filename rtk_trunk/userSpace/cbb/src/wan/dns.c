/*
 * eCos DNS implementation
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dns.c,v 1.2 2010-05-25 05:57:42 Exp $
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <bcmnvram.h>

extern int cyg_dns_res_init(struct in_addr *dns_server);

int dns_res_init(void)
{
	char dns_list[256];
	char *name, *p, *next;
	struct in_addr dns[__CONFIG_AUTO_DNS_NUM__];
	int i;
	struct in_addr addr;

	memset(dns_list, 0, sizeof(dns_list));
	strncpy(dns_list, nvram_safe_get("resolv_conf"), sizeof(dns_list)-1);

	memset(dns, 0, sizeof(dns));
	for (name = dns_list, p = name, i = 0;
	     name && name[0] && i<__CONFIG_AUTO_DNS_NUM__;
	     name = next, p = 0) {
		strtok_r(p, " ", &next);
		if (inet_aton(name, &addr) == 0)
			continue;
		dns[i] = addr;
		i++;
	}
		 	
	cyg_dns_res_init(dns);
	//2017/2/8 add by lrl更新dns代理中的server_list
	update_server_list(addr,dns_list);
	return 0;
}
