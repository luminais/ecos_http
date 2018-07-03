/*************************************************************************
  > Copyright (C) 1998-2016, Tenda Tech. Co., All Rights Reserved.
  > File Name: userSpace/prod/http/cgi/cgi_moddle_layer/cgi_goform_define.c
  > Description: 
  > Author: ZhuHuan
  > Mail: zhuhuan_IT@outlook.com
  > Version: 1.0
  > Created Time: Wednesday 2016-07-27 16:48 CST
  > Function List: 

  > History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#include "webs.h"
#include "uemf.h"
#include "cgi_common.h"
/***************************extern funs*********************************/


/*wifi relay*/
extern void formGetWifiScan(webs_t wp, char_t *path, char_t *query);


#ifdef __CONFIG_MOBILE_WEB__
extern void formGetRebootStatus(webs_t wp, char_t *path, char_t *query);
extern void formGetStatusBeforeBridge(webs_t wp, char_t *path, char_t *query);
#endif

#ifdef __CONFIG_BRIDGE_AP__
extern void fromGetHomePageselect(webs_t wp, char_t *path, char_t *query);
#endif

#ifdef __CONFIG_WPS_RTK__
void fromSetWps(webs_t wp, char_t *path, char_t *query);
#endif
extern void fromLoginOut (webs_t wp, char_t *path, char_t *query);

/*****************************end*************************************/

void goform_define()
{
	__asm__ __volatile__("nop");
	websFormDefine(T("setWAN"), formSet);
	websFormDefine(T("getWAN"), formGet);
	
	/*wifi set*/
	websFormDefine(T("setWifi"), formSet);
	websFormDefine(T("getWifi"), formGet);

	/*wifi relay*/
	websFormDefine(T("getWifiRelay"), formGet);
	websFormDefine(T("getWifiScan"), formGetWifiScan);
	websFormDefine(T("setWifiRelay"), formSet);
	
	/*Êñ∞Âä†Êé•Âè£*/
	websFormDefine(T("getWizard"), formGet);
	websFormDefine(T("setWizard"), formSet);

	websFormDefine(T("getNAT"), formGet);
	websFormDefine(T("setNAT"), formSet);
	websFormDefine(T("setSysTools"), formSet);
	websFormDefine(T("getSysTools"), formGet);
	websFormDefine(T("sysReboot"), formSet);
	websFormDefine(T("sysRestore"), formSet);
    websFormDefine(T("setMacClone"), formSet);
#ifdef __CONFIG_TENDA_APP__
	/*‘›≤ª…˝º∂*/
	websFormDefine(T("setHomePageInfo"), formSet);
#endif
    /* system status info */
	websFormDefine(T("getStatus"), formGet);
    /* traffic control */
#ifdef __CONFIG_TC__
	websFormDefine(T("getQos"), formGet);
	websFormDefine(T("setQos"), formSet);
#endif
	/*ÊâãÊú∫È°µÈù¢Êé•Âè£*/
#ifdef __CONFIG_MOBILE_WEB__
	websFormDefine(T("getRebootStatus"), formGetRebootStatus);
	websFormDefine(T("getStatusBeforeBridge"), formGetStatusBeforeBridge);
#endif

	websFormDefine(T("setParentControl"), formSet);
	websFormDefine(T("getParentControl"), formGet);
#ifdef __CONFIG_BRIDGE_AP__
	websFormDefine(T("getHomePageInfo"), formGet);
#endif
#ifdef __CONFIG_LED__
	websFormDefine(T("setPowerSave"), formSet);
	websFormDefine(T("getPowerSave"), formGet);
#endif

#ifdef __CONFIG_WPS_RTK__
	websFormDefine(T("setWifiWps"), fromSetWps);
#endif
	websFormDefine(T("loginOut"), fromLoginOut);
#ifdef __CONFIG_WEB_TO_APP__
	
	websFormDefine(T("setApp"), form_appSet);
	websFormDefine(T("getApp"), form_appGet);
	
#endif

}
