/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : build_log.c
  版 本 号   : 初稿
  作    者   : 
  生成日期   : 2017年5月19日
  最近修改   :
  功能描述   :

  功能描述   : 实现build命令，向用户提供编译相关的基本信息

  修改历史   :
  1.日    期   : 2017年5月19日
    作    者   : 
    修改内容   : 创建文件

******************************************************************************/

#include "build_log.h"
#include "version.h"


extern void print_build_tag(void);

int show_build_info(int argc, char *argv[])
{
	print_build_tag();
	printf("hard version:%s\n",W311R_ECOS_HV);
	printf("soft version:%s\n",W311R_ECOS_SV);
	printf("language:%s\n",__CONFIG_WEB_VERSION__);
	printf("svn version:%s\n",SVN_VERSION);

	return 0;
}




