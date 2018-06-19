#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include <wlutils.h>
#include "pi_wifi.h"

WIFISTASTATUS tpi_get_sta_info(char *ifname)
{
    int ret;
    char buf[1526] = {0};
    sta_info_t *sta;
    struct ether_addr ea;
    int off;
    int sta_flag;
    int *wpa;

	if(NULL == ifname)
	{
		return WIFI_INIT_FAIL;
	}

    ret = wl_ioctl(ifname, WLC_GET_BSSID, &ea, 6);
    if(ret != 0)
    {
        //PI_PRINTF(TPI,"Get BSSID info Failed\n");
        return WIFI_INIT_FAIL;
    }

    strcpy(buf, "sta_info");
    off = strlen(buf)+1;
    memcpy(buf+off, &ea, 6);
    ret = wl_ioctl(ifname, WLC_GET_VAR, buf, 1526);
    if(ret !=0)
    {
        //PI_PRINTF(TPI,"Get sta info Failed\n");
        return WIFI_INIT_FAIL;
    }
    sta = (sta_info_t *)buf;
    sta_flag = sta->flags;
    strcpy(buf, "wpa_auth");
    ret = wl_ioctl(ifname, WLC_GET_VAR, buf, 1526);
    if(ret != 0)
    {
        //PI_PRINTF(TPI,"Get Auth info Failed\n");
        return WIFI_INIT_FAIL;
    }
    wpa =(int*) buf;
    if(*wpa == WPA_AUTH_DISABLED)
    {
        goto assoc;
    }

    if(!(sta_flag & WL_STA_AUTHO))
    {
        //PI_PRINTF(TPI,"STA auth failed\n");//WPA加密认证失败
        return WIFI_AUTH_FAIL;
    }
assoc:
    if(!(sta_flag & WL_STA_ASSOC))
    {
        //PI_PRINTF(TPI,"STA associate failed\n");//关联错误
        return WIFI_ASSOCIATED_FAIL;
    }
authe:
    if(!(sta_flag & WL_STA_AUTHE))
    {
        //PI_PRINTF(TPI,"authenticate failed\n");//WEP加密失败
        return WIFI_AUTHENTICATED_FAIL;
    }
    //PI_PRINTF(TPI,"Sta Connect OK\n");
    return WIFI_OK;
}

WLMODE tpi_get_wl_mode()
{
    WLMODE ret = WL_AP_MODE;
    char wl_mode[16] = {0};

    get_wl0_mode(wl_mode);

    if(0 == strcmp(wl_mode,"sta"))
    {
        ret = WL_WISP_MODE;
    }
    else if(0 == strcmp(wl_mode,"wet"))
    {
        ret = WL_APCLIENT_MODE;
    }
    else if(0 == strcmp(wl_mode,"wds"))
    {
        ret = WL_WDS_MODE;
    }
    else
    {
        ret = WL_AP_MODE;
    }

    return ret;
}


PI_INFO tpi_get_wifi_status(WANSTATUS *wan_status,WIFISTASTATUS *wifi_status)
{
    PI_INFO ret = PI_SUC;
	WLMODE mode = WL_AP_MODE;
    int conStat = 0;
    char wan_name[32] = "eth1";
    struct ether_addr bssid = {{0}};

	if(NULL == wan_status || NULL == wifi_status)
	{
		return PI_ERR;
	}

	mode = tpi_get_wl_mode();

	switch(mode)
	{
		case WL_WISP_MODE:
			conStat = get_wan_connstatus();
			if(0 == conStat)
	        {
	            (*wan_status) = WAN_DISCONNECTED;
	        }
	        else if(1 == conStat)
	        {
	            (*wan_status) = WAN_CONNECTING;
	        }
	        else if(2 == conStat)
	        {
	            (*wan_status) = WAN_CONNECTED;
	        }
			(*wifi_status) = tpi_get_sta_info(wan_name);
			//PI_PRINTF(TPI,"WISP:%d,%d\n",(*wan_status),(*wifi_status));
			break;
		case WL_APCLIENT_MODE:
			(*wifi_status) = tpi_get_sta_info(wan_name);
	        if(WIFI_INIT_FAIL == wifi_status)
	        {
	            (*wan_status) = WAN_DISCONNECTED;
	        }
	        else if(WIFI_OK == wifi_status)
	        {
#ifdef __CONFIG_APCLIENT_DHCPC__
				char ip[17] = {0},mask[17] = {0};
				gpi_get_apclient_dhcpc_addr(ip,mask);
				if(0 == strcmp(ip,"") || 0 == strcmp(ip,""))
				{
					(*wan_status) = WAN_CONNECTING;
				}
				else
#endif	        	
				{
	            	(*wan_status) = WAN_CONNECTED;
				}
	        }
	        else
	        {
	            (*wan_status) = WAN_CONNECTING;
	        }
			//PI_PRINTF(TPI,"APCLIENT:%d,%d\n",(*wan_status),(*wifi_status));
			break;
		case WL_AP_MODE:
		default:
			ret = PI_ERR;
			break;
	}
	
    return ret;
}
