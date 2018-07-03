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
#include <stdlib.h>
#include <tz_info.h>
#include <router_net.h>
#include "sys_module.h"
#define  CYGNUM_NET_SNTP_SERVER_MAX 6
#define	NUM_SNTP_SERVER		CYGNUM_NET_SNTP_SERVER_MAX
extern void cyg_sntp_set_servers(struct sockaddr *server_list, cyg_uint32 num_servers);
extern void cyg_sntp_stop(void);

#ifdef __CONFIG_DDNS__
extern void DDNS_corrtime(void);
#endif

#if 1 //add by z10312 解决编译问题，先暂时屏蔽，后续按需打开 16-0120
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
 extern void tenda_arp_set_interval_time();
extern void tenda_arp_update_time();
#endif
#endif
int update_application_flag 	= 0;
int sntp_update_time_success 	= 0;

#ifdef __CONFIG_TENDA_HTTPD_V3__
int wanRunTime = 0;
#endif

#ifdef __CONFIG_ALINKGW__
extern int wan_speed_test(int duration);
#endif

int start_ntpc(void)
{
	struct sockaddr ntp_server[NUM_SNTP_SERVER];
	struct sockaddr_in *sin;
	char server_names[256];
	char *p, *token;
	int i;
	check_time_zone();//检查时区修改
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
	update_application_flag = 0;
	cyg_sntp_set_servers(ntp_server, NUM_SNTP_SERVER);
	return 0;
}

time_t local_to_ntp(time_t local_time)
{
	char tz_index[8] = {0};
	int index,tz_offset;
	strncpy(tz_index, nvram_safe_get("old_time_zone"), sizeof(tz_index)-1);

	index = atoi(tz_index);
	
	if(index >= 0 && index < sizeof(time_zones)/sizeof(time_zones[0]))
		tz_offset = time_zones[index].tz_offset;
	else
		tz_offset = 0;
	
	return (local_time - tz_offset);
}


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
	tenda_arp_set_interval_time();
#endif

	return (sntp_time+tz_offset);
}

void update_application_time(void)
{
	if(update_application_flag == 0){
		
#ifdef __CONFIG_DDNS__
		DDNS_corrtime();
#endif
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		tenda_arp_update_time();
#endif
		update_application_flag = 1;
	}
#ifdef __CONFIG_ALINKGW__
	wan_speed_test(10);
#endif
}

void update_check_time_thread()
{
	msg_send(MODULE_RC, RC_REBOOT_CHECK_MODULE, "op=6");
	msg_send(MODULE_RC, RC_WIFI_SWITCH_SCHED_MODULE, "op=6");
}

void set_sntp_update_time_success( void )
{
	SNTP_DBG("update time success");
	sntp_update_time_success = 1;

}

int get_sntp_update_time_success( void )
{
	return sntp_update_time_success ;

}


int
stop_ntpc(void)
{
	SNTP_DBG("sntp stop");
	cyg_sntp_stop();
	update_application_flag = 0;
	return 0;
}
