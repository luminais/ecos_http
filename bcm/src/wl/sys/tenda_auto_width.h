/*****************************************************************************
Copyright (C), 吉祥腾达，保留所有版权
File name ： tenda_auto_width.c
Description : 20/40自动频宽调整
Author ：jack deng
Version ：V1.0
Date ：2015-6-10
Others ：
History ：
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期： 修改者：
   内容：
2）...
 
*****************************************************************************/
#ifndef __TENDA_AUTO_WIDTH_H__
#define __TENDA_AUTO_WIDTH_H__

#if defined(WLC_HIGH) && !defined(WLC_LOW)
#undef TENDA_AUTO_WIDTH
#endif

#define TD_WIDTH_ADJUST_ONCE
#define TD_WIDTH_DEFAULT_DEBUG  0
#define TD_AUTO_WIDTH_TO_20M   1
#define TD_AUTO_WIDTH_TO_40M   0

#define TD_DEF_WATCHDOG_TIME        60 /* 1 min */
#define TD_INTER_WATCHDOG_TIME      600 /* 10 min */
#define TD_MAX_WATCHDOG_TIME        3600 /* 1 hour */

#define TD_ITFR_EVAL_GOOD_TH         1500
#define TD_ITFR_EVAL_BAD_TH          2800
#define TD_ITFR_GOOD_PRE_TH     80
#define TD_ITFR_BAD_PRE_TH      40

struct td_width_status {
    unsigned int itfr_good;
    unsigned int itfr_medium;
    unsigned int itfr_bad;
};

struct td_width_info {
    int             debug;
    int             enable;
    unsigned short  watchdog_timer;

    unsigned int itfr_eval_good_th;
    unsigned int itfr_eval_bad_th;
    unsigned char itfr_bad_pre_th;
    unsigned char itfr_good_pre_th;
    struct td_width_status status;
};

void td_auto_width_eval_itfr(void *td_wlc,unsigned int itfr);
void td_auto_width_cfg(void *td_wlc,int setting);
void td_auto_width_watchdog(void *td_wlc);
void td_auto_width_init(void *td_wlc);
void td_auto_width_exit(void *td_wlc);
#endif

