/*************************************************************************
	> File Name: biz_typedef.h
	> Author: 
	> Mail: 
	> Created Time: Wed Sep 16 21:54:42 2015
 ************************************************************************/

#ifndef _BIZ_TYPEDEF_H
#define _BIZ_TYPEDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>

#define BIZ_RED         "\033[0;32;31m"
#define BIZ_LIGHT_RED   "\033[1;31m"
#define BIZ_GREEN       "\033[0;32;32m"
#define BIZ_LIGHT_GREEN "\033[1;32m"
#define BIZ_BLUE        "\033[0;32;34m"
#define BIZ_LIGHT_BLUE  "\033[1;34m"
#define BIZ_DARY_GRAY   "\033[1;30m"
#define BIZ_CYAN        "\033[0;36m"
#define BIZ_LIGHT_CYAN  "\033[1;36m"
#define BIZ_PURPLE      "\033[0;35m"
#define BIZ_LIGHT_PURPLE "\033[1;35m"
#define BIZ_BROWN       "\033[0;33m"
#define BIZ_YELLOW      "\033[1;33m"
#define BIZ_LIGHT_GRAY  "\033[0;37m"
#define BIZ_WHITE       "\033[1;37m"
#define BIZ_BLK         "\033[5m"
#define BIZ_FMT_NONE    "\033[m"

#define BIZ_DEBUG_SWITCH
#ifdef BIZ_DEBUG_SWITCH
#define BIZ_DEBUG(fmt, arg...) fprintf(stdout, "[ %s ] [ %d ] [ %s ]:" \
                                        BIZ_LIGHT_BLUE fmt BIZ_FMT_NONE, __FILE__, __LINE__, __FUNCTION__, ##arg);

#define BIZ_INFO(fmt, arg...) fprintf(stdout, "[ %s ] [ %d ] [ %s ]:" \
                                        BIZ_LIGHT_GREEN fmt BIZ_FMT_NONE, __FILE__, __LINE__, __FUNCTION__, ##arg);

#define BIZ_ERROR(fmt, arg...) fprintf(stdout, "[ %s ] [ %d ] [ %s ]:" \
                                        BIZ_LIGHT_RED fmt BIZ_FMT_NONE, __FILE__, __LINE__, __FUNCTION__, ##arg);
#else
#define BIZ_DEBUG(fmt, arg...)
#define BIZ_ERROR(fmt, arg...)
#define BIZ_INFO(fmt, arg...)
#endif

enum {
    BIZ_RET_FAILURE = -1,
    BIZ_RET_SUCCESS = 0,
    BIZ_RET_INTERNAL_FAILURE,
    //WAN 模块
    BIZ_RET_WAN_LAN_IP_CONFLICT = 11,	//WAN与LAN IP冲突
    //WIFI模块
    BIZ_RET_ERR_MODE_UNKOWN =30,		//wifi的信号强度的模式未知
    BIZ_RET_MAX,
};

#endif

