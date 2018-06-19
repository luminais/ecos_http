#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "webs.h"
#include "flash_cgi.h"
#include "route_cfg.h"
#include "wsIntrn.h"

void formWifiPowerInit(webs_t wp, char_t *path, char_t *query)
{
	char *tmp_value;
	char sigle_mode[8] = {0};
	
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, "{");

	_GET_VALUE(WLN0_POWER, tmp_value);

	if (strlen(tmp_value)>2)
	{
		strcpy(sigle_mode,tmp_value);
	}
	else
	{	
		strcpy(sigle_mode,"middle");
	}
	websWrite(wp, "\"signal-mode\":\"%s\"", sigle_mode);	
	
	websWrite(wp, "}");
		
	websDone(wp, 200);

	return;
}

#ifdef __CONFIG_ALINKGW__
extern int report_wifipamode_state(void) ;
extern int g_alinkgw_enable; 
#endif


void formWifiPowerSet(webs_t wp, char_t *path, char_t *query)
{
	char *tmp_value;
	char sigle_mode[8] = {0};
	
	tmp_value = websGetVar(wp, T("signal-mode"), T("high"));

	strcpy(sigle_mode,tmp_value);

	_SET_VALUE(WLN0_POWER, sigle_mode);

	/*for NH325: high-19db,middle-17.5db*/
	if(0 == strcmp(sigle_mode, "high"))
	{
		_SET_VALUE(WLN0_PWR_PERCENT, "100");
	}
	else
	{
		_SET_VALUE(WLN0_PWR_PERCENT, "92");
	}
	
	_COMMIT();

#ifdef __CONFIG_ALINKGW__
	 if( g_alinkgw_enable )
		 report_wifipamode_state() ;
#endif
	websWrite(wp, "1");
	
	websDone(wp, 200);
	
	_SET_WL_PWR_PENCENT();

}

void wirelessPwrAspDefine()
{

}

void wirelessPwrGoformDefine()
{
	websFormDefine(T("GetPowerControl"),formWifiPowerInit);	
	websFormDefine(T("SetPowerControl"), formWifiPowerSet);
}

void wirelessPwrAspGoformDefine()
{
	wirelessPwrAspDefine();
	
	wirelessPwrGoformDefine();
}

