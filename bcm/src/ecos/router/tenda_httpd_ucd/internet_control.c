#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <typedefs.h>
#include <proto/ethernet.h>
#include <bcmparams.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <netconf.h>
#include <nvparse.h>
#include <wlutils.h>


#include "route_cfg.h"
#include "flash_cgi.h"
#include "webs.h"
#include "uemf.h"
#include "../tc/tc.h"

void get_wl_list_station(char *name, struct ether_addr *ea,char *mac, char *ip)//获取无线客户端
{
	char buf[sizeof(sta_info_t)];
	char macbuf[32];
	char ipbuf[32] = {0};
	int j = 0;
	struct in_addr a;
	int mactoip[6]={0};
	strcpy(buf, "sta_info");
	memcpy(buf + strlen(buf) + 1, (unsigned char *)ea, ETHER_ADDR_LEN);
	if (!wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf))) {
		sta_info_t *sta = (sta_info_t *)buf;
		memset(macbuf,0,sizeof(macbuf));
		sprintf(macbuf,"%02X:%02X:%02X:%02X:%02X:%02X",
				sta->ea.octet[0],  sta->ea.octet[1],
				sta->ea.octet[2],  sta->ea.octet[3],
				sta->ea.octet[4],  sta->ea.octet[5]);
		if(!strcmp(ipbuf, "0.0.0.0")){
			return;
		}
		for(j=0;j<strlen(macbuf);j++)
		{
			if(macbuf[j] >= 'A' && macbuf[j] <= 'Z')
				macbuf[j] += 32;
		}
		//根据mac找IP
		sscanf(macbuf,"%02x:%02x:%02x:%02x:%02x:%02x",mactoip,mactoip+1,mactoip+2,mactoip+3,mactoip+4,mactoip+5);
		a.s_addr = lookip(mactoip);
		strcpy(ip, inet_ntoa(a));
		for(j=0; j<strlen(macbuf); j++)
		{
			if(macbuf[j] >= 'a' && macbuf[j] <= 'z')
				macbuf[j] -= 32;
		}
		strcpy(mac, macbuf);
	}
	
	return;	
}

int isWifiClient(char *mac, char *ip)
{
	char tmp[512] = {0}, prefix[20] = "wl0";
	char *name=NULL;
	struct maclist *mac_list;
	int mac_list_size;
	int i;
	char *wl_unit;
	char mid_mac[20] = "00:00:00:00:00:00";
	char mid_ip[16] = "255.255.255.255";
	if(mac == NULL || ip == NULL)
		return 0;

	char *wl_mode_tag;

	_GET_VALUE("wl0_mode",wl_mode_tag);
	if(strcmp(wl_mode_tag,"ap") == 0)
	{
		wl_unit = "0";
	}
	else
	{
		wl_unit = "0.1";
	}
	//_GET_VALUE(WLN0_UNIT, wl_unit);
	snprintf(prefix, sizeof(prefix), "wl%s_", wl_unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
	mac_list_size = sizeof(mac_list->count) + 256 * sizeof(struct ether_addr);
	mac_list = malloc(mac_list_size);
	
	if (!mac_list){
		return 0;
	}
	#ifndef __NetBSD__
		/*strcpy((char*)mac_list, "authe_sta_list");
		if (wl_ioctl(name, WLC_GET_VAR, mac_list, mac_list_size)) {
			free(mac_list);
			return 0;
		}*/
		strcpy((char*)mac_list, "assoclist");
		if (wl_ioctl(name, WLC_GET_ASSOCLIST, mac_list, mac_list_size)) {
			free(mac_list);
			return 0;
		}
	#else
	/* NetBSD TBD... */
		mac_list->count=0;
	#endif
	
	/* query sta_info for each STA and output one table row each */
		for (i = 0; i < mac_list->count; i++) {
			memset(mid_mac,0,strlen(mid_mac));
			memset(mid_mac,0,strlen(mid_ip));
			get_wl_list_station(name, &mac_list->ea[i], mid_mac, mid_ip);				
			if(strcmp(mac,mid_mac) == 0 && strcmp(ip,mid_ip) == 0)
			{
				free(mac_list);
				return 1;
			}
		}
		
		free(mac_list);
		return 0;	
}

