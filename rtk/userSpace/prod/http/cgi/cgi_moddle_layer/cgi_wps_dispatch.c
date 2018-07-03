/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_wps_dispatch.c
  版 本 号   : 初稿
  作    者   : fh
  生成日期   : 2016年10月13日
  最近修改   :
  功能描述   : 

  功能描述   : wps,这一部分UI暂时没作模块化

  修改历史   :
  1.日    期   : 2016年10月13日
    作    者   : fh
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <webs.h>
#include "cgi_common.h"
#include "flash_cgi.h"


/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/


void fromSetWps(webs_t wp, char_t *path, char_t *query)
{
	char_t *security_mode=NULL,  *wpsenable=NULL,*wifienable_2G=NULL,*wifienable_5G=NULL,*action=NULL;
	char_t *wlunit;
	

	action = websGetVar(wp, T("action"), T("pbc"));

	_GET_VALUE("wps_mode",wpsenable);
	_GET_VALUE("wl0_radio",wifienable_2G);
    _GET_VALUE("wl1_radio",wifienable_5G);

	printf("wpsenable=%s,wifienable_2G=%s,wifienable_5G=%s,action=%s,\n",wpsenable,wifienable_2G,wifienable_5G,action);

	if( (0== strcmp(wpsenable, "enabled")) && ((0== strcmp(wifienable_2G, "1")) || (0== strcmp(wifienable_5G, "1"))) && (0== strcmp(action, "pbc")) )
	{	
		printf("start pbc wps.................\n");
		msg_send(MODULE_RC, RC_WPS_MODULE, "string_info=startpbc");
	}
	else
	{
		printf("can't start pbc wps.................\n");
	}

 	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n")); 
	websWrite(wp, T("%s"), "{\"errCode\":\"0\"}"); 
	websDone(wp, 200);
			
	return;
	
}

