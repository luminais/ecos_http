/*
 * Firewall services
 *
 * Copyright (C) 2010, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: firewall.c,v 1.7 2010-07-25 14:49:32 Exp $
 */
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <iflib.h>
#include <rc.h>
#include <shutils.h>
#include <netconf.h>
#include <nvparse.h>
#include <bcmnvram.h>

#ifdef __CONFIG_NAT__
void
start_firewall(void)
{
	if (nvram_match("fw_disable", "1"))
		return;
	firewall_init();
	ipnat_init();
}

void
stop_firewall(void)
{
	ipnat_deinit();
	firewall_flush();
}

void
update_firewall(void)
{
	stop_firewall();
	start_firewall();
}

#if defined(__CONFIG_PPTP__) || defined(__CONFIG_L2TP__) || defined(__CONFIG_PPPOE2__) 
void
update_tunnel_firewall(char *pppname)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *tunnel_ifname;

	unit = ppp_ifunit(pppname);
	if (unit < 0)
		return;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	/* Get bound ifname */
	tunnel_ifname = nvram_get(strcat_r(prefix, "bind_ifname", tmp));
	if (tunnel_ifname) {
	 	/* Set tunnel firewall and nat rules */
		firewall_basic_rule(tunnel_ifname);
		ipnat_napt_init(tunnel_ifname);
	}
}
#endif	/* __CONFIG_PPTP__ || __CONFIG_L2TP__ */
#endif /* __CONFIG_NAT__ */
