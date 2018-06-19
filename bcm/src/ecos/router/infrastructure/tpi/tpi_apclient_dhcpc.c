#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include "pi_apclient_dhcpc.h"
#include <net/if.h>
#include "wlioctl.h"
#include <wlutils.h>
#include "rc.h"

static APCLIENT_DHCPC_INFO_STRUCT ctx;
static P_APCLIENT_DHCPC_INFO_STRUCT pctx = &ctx;

extern char *get_wl0_mode(char *value);
extern PI_INFO gpi_get_wifi_status_info(WANSTATUS * wan_status,WIFISTASTATUS * wifi_status);
extern void dhcpc_start(char *ifname, char *script, char *hostname);
extern void dhcpc_stop(char *ifname);
extern void dhcpd_stop(void);
extern void dhcpd_start(void);
extern void et_restart_lan_port(int lan_port);
extern struct ether_addr *ether_aton(const char *asc);

extern login_ip_time loginUserInfo[MAX_USER_NUM];

int tpi_get_apclient_dhcpc_enable()
{
	return pctx->enable;
}

PI_INFO tpi_get_apclient_dhcpc_info(char *ip,char *mask)
{
	PI_INFO ret = PI_SUC;
	
	if(NULL == ip || NULL == mask)
		return PI_ERR;
	
	strncpy(ip,pctx->ipaddr,strlen(pctx->ipaddr));
	strncpy(mask,pctx->mask,strlen(pctx->ipaddr));
	
	return ret;
}

PI_INFO tpi_set_apclient_dhcpc_info(char *ip,char *mask)
{
	PI_INFO ret = PI_SUC;
	
	if(NULL == ip || NULL == mask)
		return PI_ERR;
	
	strncpy(pctx->ipaddr,ip,strlen(ip));
	strncpy(pctx->mask,mask,strlen(mask));
	
	return ret;
}

PI_INFO init_apclient_dhcpc_info()
{
	PI_INFO ret = PI_SUC;
	char wl_mode[16] = {0};
	
	memset(pctx,0x0,sizeof(APCLIENT_DHCPC_INFO_STRUCT));

	get_wl0_mode(wl_mode);
	if(0 != strcmp(wl_mode,"wet") || 1 != nvram_match("router_disable", "1"))
	{
		return PI_ERR;
	}

	pctx->enable = 1;	
	sprintf(pctx->lan_ifnames,"%s",nvram_safe_get("lan_ifname"));
	strcpy(pctx->ipaddr,"");
	strcpy(pctx->mask,"");
	pctx->now_status = WAN_DISCONNECTED;
	pctx->before_status = WAN_DISCONNECTED;
	pctx->connected_count = 0;	
	pctx->disconnected_count = 0;	
	
	return ret;
}

PI_INFO tpi_apclient_dhcpc_restart_phy(char *ifnames)
{
	PI_INFO ret = PI_SUC;
	int i = 0;
	 
	for(i = PORT_LAN1;i <= PORT_LAN4;i++)
	{
		et_restart_lan_port(i);
	}

	scb_val_t scbval;
	struct ether_addr *hwaddr;
	if(NULL == ifnames)
		return ret;
	
	scbval.val = 0;
	hwaddr = ether_aton(nvram_safe_get("wl0.1_hwaddr"));
	 
	if(hwaddr)
	{
		memcpy(&scbval.ea, hwaddr, ETHER_ADDR_LEN);
		wl_ioctl(ifnames, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scbval, sizeof(scbval));
	}
	return ret;
}

PI_INFO tpi_apclient_clear_web_servers_info()
{
	PI_INFO ret = PI_SUC;
	int i = 0;
	
	for (i=0; i<MAX_USER_NUM; i++)
	{	
		memset(loginUserInfo[i].ip, 0, IP_SIZE);				
		loginUserInfo[i].time = 0;
	}

	return ret;
}

PI_INFO tpi_apclient_lan_dhcp_action(PI_ACTION action)
{
	PI_INFO ret = PI_SUC;
	
	switch(action)
	{
		case PI_STOP:
			nvram_set("lan_proto", "static");
			dhcpd_stop();
			dhcpd_start();
			tpi_apclient_dhcpc_restart_phy("wl0.1");
			tpi_apclient_clear_web_servers_info();
			break;
		case PI_START:
			nvram_set("lan_proto", "dhcp");
			dhcpd_stop();
			dhcpd_start();
			tpi_apclient_dhcpc_restart_phy("wl0.1");
			tpi_apclient_clear_web_servers_info();
			break;
		case PI_RESTART:
		default:
			PI_PRINTF(TPI,"action is %d\n",action);
			break;
	}
	
	return ret;
}

PI_INFO tpi_apclient_dhcpc_action(PI_ACTION action)
{
	PI_INFO ret = PI_SUC;

	switch(action)
	{
		case PI_STOP:
			dhcpc_stop(pctx->lan_ifnames);
			ifconfig(pctx->lan_ifnames, IFUP,nvram_safe_get("lan_ipaddr"),nvram_safe_get("lan_netmask"));
			tpi_apclient_lan_dhcp_action(PI_START);
			break;
		case PI_START:
			nvram_unset("resolv_conf");
			dhcpc_start(pctx->lan_ifnames, "landhcpc", "");
			break;
		case PI_RESTART:
		default:
			PI_PRINTF(TPI,"action is %d\n",action);
			break;
	}
	
	return ret;
}

void tpi_apclient_main()
{
	int first_time = 1,dhcp_start = 1,wifiStat = 0;
	while(pctx->enable)
	{
		gpi_get_wifi_status_info(&(pctx->now_status),&wifiStat);
		
		if(WIFI_OK == wifiStat && WAN_CONNECTING == pctx->now_status)
		{
			pctx->now_status = WAN_CONNECTED;
		}
		
		switch(pctx->now_status)
		{
			case WAN_CONNECTED:
				if(1 == first_time)
				{
					pctx->connected_count++;
					pctx->before_status = WAN_CONNECTED;
					tpi_apclient_dhcpc_action(PI_START);
					first_time = 0;
				}
				else
				{
					if((pctx->connected_count++) >= CONNECTED_COUNT_MAX && WAN_DISCONNECTED == pctx->before_status)
					{
						pctx->before_status = WAN_CONNECTED;
						tpi_apclient_dhcpc_action(PI_START);
					}
				}
				dhcp_start = 0;
				pctx->disconnected_count = 0;
				break;
			default:
				if(1 == dhcp_start)
				{
					tpi_apclient_lan_dhcp_action(PI_START);
					dhcp_start = 0;
				}
				if((pctx->disconnected_count++) >= DISCONNECTED_COUNT_MAX && WAN_CONNECTED == pctx->before_status)
				{
					pctx->before_status = WAN_DISCONNECTED;					
					strcpy(pctx->ipaddr,"");
					strcpy(pctx->mask,"");
					tpi_apclient_dhcpc_action(PI_STOP);
				}
				pctx->connected_count = 0;
				break;
		}		
		cyg_thread_delay(APCLINET_DHCPC_SLEEP);
	}
	return;
}
