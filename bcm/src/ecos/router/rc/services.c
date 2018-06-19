/*
 * Miscellaneous services
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
 * $Id: services.c,v 1.14 2010-08-11 02:32:10 Exp $
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <rc.h>

extern int g_cur_wl_radio_status;

/* up/down lock */
static int services_lock_init = 0;
static cyg_mutex_t services_lock;

/* External functions */
#ifdef __CONFIG_DHCPD__
int  dhcpd_start(void);
void dhcpd_stop(void);
#endif
#ifdef __CONFIG_HTTPD__
void httpd_start(void);
void httpd_stop(void);
#endif
#ifdef __CONFIG_TENDA_HTTPD__
extern void tenda_httpd_start(void);
extern void tenda_httpd_stop(void);
extern void wps_led_set(void);
#endif/*__CONFIG_TENDA_HTTPD__*/

#ifdef __CONFIG_TELNETD__
extern int telnetd_start(void);
extern void telnetd_stop(void);
#endif

extern void eapd_start(void);
extern void eapd_stop(void);
extern void nasd_start(void);
extern void nasd_stop(void);
#ifdef __CONFIG_IGD__
extern void igd_start(void);
extern void igd_stop(void);
#endif
#ifdef __CONFIG_WPS__
extern void wps_start(void);
extern void wps_stop(void);
#endif
#ifdef __CONFIG_NETBOOT__
extern void netboot_start();
#endif
#ifdef __CONFIG_DNSMASQ__
void dnsmasq_start(void);
void dnsmasq_stop(void);
#endif/*__CONFIG_DNSMASQ__*/
#ifdef __CONFIG_DDNS__
extern void	ddns_start();
extern void	ddns_stop();
#endif/*__CONFIG_DDNS__*/
extern int dns_res_init(void);

void
init_services(void)
{
	if (services_lock_init == 0) {
		cyg_mutex_init(&services_lock);
		services_lock_init = 1;
	}
}

void
start_services(void)
{
#ifdef __CONFIG_TELNETD__
	telnetd_start();
#endif /* __CONFIG_TELNETD__ */

	/* Start dns resolve */
	dns_res_init();

#ifdef __CONFIG_HTTPD__
	httpd_start();
#endif /* __CONFIG_HTTPD__ */
#ifdef __CONFIG_TENDA_HTTPD__
	tenda_httpd_start();
#endif/*__CONFIG_TENDA_HTTPD__*/

	eapd_start();
	nasd_start();
#ifdef __CONFIG_DNSMASQ__
	dnsmasq_start();
#endif /* __CONFIG_DNSMASQ__ */

#ifdef __CONFIG_DDNS__
	ddns_start();
#endif

#ifdef __CONFIG_DHCPD__
	dhcpd_start();
#endif /* __CONFIG_DHCPD__ */
#ifdef __CONFIG_IGD__
	igd_start();
#endif /* __CONFIG_IGD__ */
#ifdef __CONFIG_WPS__
	wps_start();
#endif /* __CONFIG_WPS__ */
#ifdef __CONFIG_NETBOOT__
	netboot_start();
#endif /* __CONFIG_NETBOOT__ */
}

void
stop_services(void)
{
	cyg_mutex_lock(&services_lock);

#ifdef __CONFIG_WPS__
	wps_stop();
#endif /* __CONFIG_WPS__ */
	nasd_stop();
	eapd_stop();
#ifdef __CONFIG_IGD__
	igd_stop();
#endif /* __CONFIG_IGD__ */
#ifdef __CONFIG_HTTPD__
	httpd_stop();
#endif /* __CONFIG_HTTPD__ */

#ifdef __CONFIG_TENDA_HTTPD__
	tenda_httpd_stop();
#endif/*__CONFIG_TENDA_HTTPD__*/

#ifdef __CONFIG_DHCPD__
	dhcpd_stop();
#endif /* __CONFIG_DHCPD__ */
#ifdef __CONFIG_DNSMASQ__
	dnsmasq_stop();
#endif /* __CONFIG_DNSMASQ__ */
#ifdef __CONFIG_TELNETD__
	telnetd_stop();
#endif /* __CONFIG_TELNETD__ */
#ifdef __CONFIG_DDNS__
	ddns_stop();
#endif /* __CONFIG_DNSMASQ__ */

	cyg_mutex_unlock(&services_lock);
}

void
update_services(void)
{
	cyg_mutex_lock(&services_lock);

#ifdef __CONFIG_TELNETD__
	telnetd_start();
#endif /* __CONFIG_TELNETD__ */

	/* Start dns client */
	dns_res_init();

	if (WL_RADIO_ON == g_cur_wl_radio_status)
	{
		eapd_start();
		nasd_start();
	}

	//eapd_start();
	//nasd_start();
#ifdef __CONFIG_IGD__
	igd_start();
#endif /* __CONFIG_IGD__ */
#ifdef __CONFIG_WPS__
	wps_start();
#endif /* __CONFIG_WPS__ */
#ifdef __CONFIG_HTTPD__
	httpd_start();
#endif /* __CONFIG_HTTPD__ */

#ifdef __CONFIG_TENDA_HTTPD__
	tenda_httpd_start();
#endif /* __CONFIG_TENDA_HTTPD__ */

#ifdef __CONFIG_DNSMASQ__
	dnsmasq_start();
#endif /* __CONFIG_DNSMASQ__ */

#ifdef __CONFIG_DDNS__
	ddns_start();
#endif

#ifdef __CONFIG_DHCPD__
	dhcpd_start();
#endif /* __CONFIG_DHCPD__ */

	cyg_mutex_unlock(&services_lock);
}
