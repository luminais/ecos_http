#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kautoconf.h>
#include "flash_cgi.h"
#include "webs.h"
#include "uemf.h"
#include <bcmwifi.h>
#include "cJSON.h"

#include "pi_common.h"
#include "sys_module.h"
#include "rc_module.h"
#include <wl_utility_rltk.h>//Realtek无线驱动获取信息使用

#include "cgi_common.h"
#include <wan.h>
#include <wifi.h>


#ifdef __CONFIG_A9__
extern void extend_status_set(int page_status_flag,int get_ip_flag);
extern int get_ip_flag_get();
#endif


void formGetStatusBeforeBridge(webs_t wp, char_t *path, char_t *query)
{
	int len = 0;
	cJSON *root = NULL;
	PI8 *out  = NULL;
	PI8 extend_status[8] = {0};
	WANSTATUS wan_status = WAN_DISCONNECTED;
	WIFISTASTATUS wifi_status = WIFI_OK;

	gpi_wifi_get_status_info(&wan_status,&wifi_status);

	/*extended：0桥接错误，1桥接成功，2桥接中*/
	extern void tpi_apclient_dhcpc_set_web_wait(PIU8 set_wait,PIU8 get_return);
	if(WIFI_OK == wifi_status)
	{
		if(WAN_CONNECTED == wan_status)
		{
			strcpy(extend_status, "1");
		}
		else
		{
			strcpy(extend_status, "2");
		}
	}
	else if(WIFI_AUTHENTICATED_FAIL == wifi_status || WIFI_AUTH_FAIL == wifi_status)
	{
		strcpy(extend_status, "0");
	}
	else
	{
		strcpy(extend_status, "2");
	}

	root =  cJSON_CreateObject();
	cJSON_AddStringToObject(root, "extended", extend_status);

	out = cJSON_Print(root);
	cJSON_Delete(root);

	len = strlen(out);
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n"));
	websWrite(wp, T("Content-length: %d\r\n\r\n"), len);

	websWrite(wp, out);
	websDone(wp, 200);

	free(out);

	if(0 == strcmp(extend_status,"1"))
	{
		printf("###############[%s.%d]:get connected suc!\n",__func__,__LINE__);
		tpi_apclient_dhcpc_set_web_wait(-1,1);
	}
	else
	{
		tpi_apclient_dhcpc_set_web_wait(-1,0);
	}

	return;
}


void formGetRebootStatus(webs_t wp, char_t *path, char_t *query)
{
	PI8 *callback_str = NULL;
	PI8 ret_str[16] = "success";
	char result[64] = {0};
	cJSON *root = NULL;
	char *out  = NULL;

	callback_str = websGetVar(wp, T("callback"), T(""));

	if(strcmp(nvram_safe_get("wl0_ssid"), nvram_safe_get("wl0.1_ssid")) != 0)
	{
		sprintf(ret_str, "%s", "fail");
	}

	root =  cJSON_CreateObject();
	cJSON_AddStringToObject(root, "status", ret_str);

	out = cJSON_Print(root);
	cJSON_Delete(root);

	sprintf(result, "%s(%s)\n", callback_str, out);
	free(out);

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: application/x-javascript; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n"));
	websWrite(wp, T("Content-length: %d\r\n\r\n"), strlen(result));

	websWrite(wp, result);
	websDone(wp, 200);

	return;
}


