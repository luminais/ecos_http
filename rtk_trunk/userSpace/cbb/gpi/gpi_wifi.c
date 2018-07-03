#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wifi.h"

SYS_WORK_MODE gpi_wifi_get_mode()
{
	return tpi_wifi_get_sys_work_mode();
}

#ifdef __CONFIG_GUEST__
RET_INFO gpi_wifi_check_ip_confilct_by_wan(unsigned int wan_ip,unsigned int wan_mask)
{
	return tpi_wifi_check_ip_confilct_by_wan(wan_ip,wan_mask);
}
#endif
RET_INFO gpi_wifi_get_status_info(WANSTATUS *wan_status,WIFISTASTATUS *wifi_status)
{
	if(NULL == wan_status || NULL == wifi_status)
	{
		return RET_ERR;
	}
	return tpi_wifi_get_wifi_status(wan_status,wifi_status);
}

RET_INFO gpi_get_wifi_scan_result(void *scan_list,char	 *ifname)
{
	return tpi_get_wifi_scan_result(scan_list,ifname);
}


P_WIFI_INFO_STRUCT gpi_wifi_get_info(WLAN_RATE_TYPE wl_rate)
{
	return tpi_wifi_get_info(wl_rate);
}

void gpi_wifi_guest_get_info(P_WIFI_SSID_CFG cfg,char *ifname)
{
	if(NULL == cfg || NULL == ifname)
	{
		return;
	}

	if(0 == strncmp(ifname,TENDA_WLAN24_GUEST_IFNAME,strlen(TENDA_WLAN24_GUEST_IFNAME)))
	{
		tpi_wifi_update_basic_info(cfg,WL_24G_GUEST);
	}
	else if(0 == strncmp(ifname,TENDA_WLAN5_GUEST_IFNAME,strlen(TENDA_WLAN5_GUEST_IFNAME)))
	{
		tpi_wifi_update_basic_info(cfg,WL_5G_GUEST);
	}
    else
	{
		tpi_wifi_update_basic_info(cfg,WL_24G_GUEST);
	}
	
	return;
}

P_WIFI_CURRCET_INFO_STRUCT gpi_wifi_get_curret_info(WLAN_RATE_TYPE wl_rate,P_WIFI_CURRCET_INFO_STRUCT cfg)
{
	if(NULL != cfg && ( WLAN_RATE_24G == wl_rate || WLAN_RATE_5G == wl_rate))
	{
		cfg = tpi_wifi_get_curret_info(wl_rate,cfg);
	}
	return cfg;
}

RET_INFO gpi_wifi_channles_in_country(WLAN_RATE_TYPE wl_rate,int channels)
{
	return  tpi_wifi_channles_in_country(wl_rate,channels);
}

PI32 gpi_wifi_get_channels(WLAN_RATE_TYPE wl_rate, PI32 bandwidth, PI8 *country, PIU16 *list, PI32 len)
{
	return tpi_wifi_get_channels(wl_rate,bandwidth,country,list,len);
}

void gpi_wifi_get_power_default(WLAN_RATE_TYPE wl_rate,PI8 *power)
{	
    if(NULL != power || WLAN_RATE_24G != wl_rate || WLAN_RATE_5G != wl_rate)
    {
		tpi_wifi_get_power_default(wl_rate,power);
    }

    return ;
}

void gpi_wifi_get_country_default(PI8 *country)
{
    if(NULL != country)
    {
		tpi_wifi_get_country_default(country);
    }

    return;
}

void gpi_wifi_get_bridge_rssi(WLAN_RATE_TYPE wl_rate,P_WIFI_BRIDGE_INFO_STRUCT bridge_info)
{
	if(NULL != bridge_info || WLAN_RATE_24G != wl_rate || WLAN_RATE_5G != wl_rate)
	{
		tpi_wifi_get_bridge_rssi(wl_rate,bridge_info);
	}

	return;
}
