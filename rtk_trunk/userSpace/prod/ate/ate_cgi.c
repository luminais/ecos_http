/*************************************************************************
	> Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
	> File Name: ate_cgi.c
	> Description:²ú²âCGI 
	> Author: zhuhuan
	> Mail: zhuhuan_IT@outlook.com
	> Version: 1.0
	> Created Time: Fri 29 Jan 2016 09:32:48 AM CST
	> Function List: 

	> History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#include <stdio.h>
#include <webs.h>

extern void ate_start(void);
static void fromSysAted(webs_t wp, char_t *path, char_t *query)
{
	ate_start();
	websWrite(wp,T("load mfg success."));
	websDone(wp,200);
}

void ate_goform_define()
{
	websFormDefine(T("ate"), fromSysAted);
}


