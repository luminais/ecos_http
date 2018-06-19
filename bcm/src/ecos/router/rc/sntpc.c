/*
 * sntpc wrapper
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: sntpc.c,v 1.1 2010-05-25 05:35:32 Exp $
 *
 */
#include <pkgconf/system.h>
#include <pkgconf/net_sntp.h>
#include <network.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/sntp/sntp.h>

#include <time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <arpa/inet.h>
#include <bcmnvram.h>
//roy+++
#include <stdlib.h>//for atoi
#include <tz_info.h>
#include <router_net.h>
//

#define	NUM_SNTP_SERVER		CYGNUM_NET_SNTP_SERVER_MAX
extern void cyg_sntp_set_servers(struct sockaddr *server_list, cyg_uint32 num_servers);
extern void cyg_sntp_stop(void);
#ifdef __CONFIG_DDNS__
extern void DDNS_corrtime(void);
#endif
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
 extern void setArpStaIntervalTime();
extern void updateArpStaTime();
#endif
int update_application_flag = 0;
int sntp_update_time_success = 0;
#ifdef __CONFIG_TENDA_HTTPD_V3__
int wanRunTime = 0;
#endif
#ifdef __CONFIG_ALINKGW__
extern int wan_speed_test(int duration);
extern int g_alinkgw_enable; 
#endif
int
start_ntpc(void)
{
	struct sockaddr ntp_server[NUM_SNTP_SERVER];
	struct sockaddr_in *sin;
	char server_names[256];
	char *p, *token;
	int i;
	/* Init to zero */
	strncpy(server_names, nvram_safe_get("ntp_server"), sizeof(server_names)-1);
	p = server_names;

	memset(ntp_server, 0, sizeof(ntp_server));
	for (i = 0; i < NUM_SNTP_SERVER; i++) {
		token = strsep(&p, " ");
		if (token == NULL)
			break;

		sin = (struct sockaddr_in *)&ntp_server[i];
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(*sin);
		sin->sin_port = htons(123);
		inet_aton(token, &sin->sin_addr);
	}
	
	/* pxy rm, 2014.01.23 */
//	SNTP_DBG("sntp start");

	update_application_flag = 0;
	cyg_sntp_set_servers(ntp_server, 4);
	return 0;
}
//roy +++2010/09/07
time_t ntp_to_local2(time_t sntp_time)
{
	char tz_index[8] = {0};
	int index,tz_offset;
	strncpy(tz_index, nvram_safe_get("time_zone"), sizeof(tz_index)-1);

	index = atoi(tz_index);
	
	if(index >= 0 && index < sizeof(time_zones)/sizeof(time_zones[0]))
		tz_offset = time_zones[index].tz_offset;
	else
		tz_offset = 0;
#ifdef __CONFIG_TENDA_HTTPD_V3__		
	wanRunTime = time(0) - primary_wan_ip_set.conn_time;
#endif
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	setArpStaIntervalTime();
#endif
	SNTP_DBG("update time success");
	
	return (sntp_time+tz_offset);
}

void update_application_time(void)
{
	if(update_application_flag == 0){
#ifdef __CONFIG_DDNS__
		DDNS_corrtime();
#endif
		primary_wan_ip_set.conn_time = time(0);
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		updateArpStaTime();
#endif
		update_application_flag = 1;
	}
#ifdef __CONFIG_ALINKGW__
	if( g_alinkgw_enable )
		wan_speed_test(10);
#endif
}
void set_sntp_update_time_success( void )
{
	sntp_update_time_success = 1;

}

int get_sntp_update_time_success( void )
{
	return sntp_update_time_success ;

}

//roy+++
int
stop_ntpc(void)
{
	SNTP_DBG("sntp stop");
	cyg_sntp_stop();
	update_application_flag = 0;
	return 0;
}
