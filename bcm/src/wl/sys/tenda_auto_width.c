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
#include <wlc_cfg.h>
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <proto/802.11.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>
#include <bcmutils.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>

#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_scb.h>

#include <wlc_rm.h>
#include "wlc_cac.h"
#include <wlc_ap.h>

#include <bcmotp.h>

#if defined(TENDA_AUTO_WIDTH)  && defined(TENDA_PHY_STATUS)

#define TD_PRINT   printf

#define TD_WIDTH_DBG(_info,_fmt, ...)  \
    do {                        \
        if (_info->debug) {    \
            printf(_fmt, ##__VA_ARGS__);    \
        }                       \
    } while(0)

enum {
    IOV_TD_WIDTH_TEST,
    IOV_TD_WIDTH_ENABLE,
    IOV_TD_WIDTH_DBG,
    IOV_TD_WIDTH_WATCHDOG_TIME,
    IOV_TD_WIDTH_ITFR_GOOD_PRE,
    IOV_TD_WIDTH_ITFR_BAD_PRE,
    IOV_TD_WIDTH_ITFR_EVAL_GOOD_TH,
    IOV_TD_WIDTH_ITFR_EVAL_BAD_TH,
    IOV_TD_WIDTH_SHOW,
    IOV_TD_WIDTH_HLEP,
};

static unsigned short width_timer_counts = 0;

static const bcm_iovar_t td_auto_width_iovars[] = {
    {"td_width_test", IOV_TD_WIDTH_TEST, 0 , IOVT_INT32, 0},
    {"td_width_enable", IOV_TD_WIDTH_ENABLE, 0 , IOVT_INT32, 0},
    {"td_width_dbg", IOV_TD_WIDTH_DBG, 0 , IOVT_INT32, 0},
    {"td_width_watch_time", IOV_TD_WIDTH_WATCHDOG_TIME, 0 , IOVT_INT32, 0},

    {"td_width_itfr_good_pre", IOV_TD_WIDTH_ITFR_GOOD_PRE, 0 , IOVT_INT32, 0},
    {"td_width_itfr_bad_pre", IOV_TD_WIDTH_ITFR_BAD_PRE, 0 , IOVT_INT32, 0},
    {"td_width_itfr_eval_good_th", IOV_TD_WIDTH_ITFR_EVAL_GOOD_TH, 0 , IOVT_INT32, 0},
    {"td_width_itfr_eval_bad_th", IOV_TD_WIDTH_ITFR_EVAL_BAD_TH, 0 , IOVT_INT32, 0},

    {"td_width_show", IOV_TD_WIDTH_SHOW, 0 , IOVT_INT32, 0},
    {"td_width_help", IOV_TD_WIDTH_HLEP, 0 , IOVT_INT32, 0},
    {NULL, 0, 0, 0, 0}
};

static void td_auto_width_help(wlc_info_t *wlc)
{
    int i;

    TD_PRINT("=== tenda auto_width cmd ===\n");
    for (i =0; ; i++) {
        if (td_auto_width_iovars[i].name == NULL)
            break;
        TD_PRINT("  %s\n",td_auto_width_iovars[i].name);
    }
}

void td_auto_width_eval_itfr(void *td_wlc,unsigned int itfr)
{
    wlc_info_t *wlc = (wlc_info_t *)td_wlc;
    struct td_width_status *status = &wlc->td_width.status;

    /* debug print */
    if (wlc->td_width.debug) {
        TD_PRINT("%s:itfr=%d good_eval_th=%d bad_evel_th=%d\n",
            __func__,itfr,wlc->td_width.itfr_eval_good_th,wlc->td_width.itfr_eval_bad_th);
    }

    if (itfr > wlc->td_width.itfr_eval_bad_th) {
        status->itfr_bad++;
    } else if (itfr < wlc->td_width.itfr_eval_good_th){
        status->itfr_good++;
    } else {
        status->itfr_medium++;
    }
}

static void td_auto_width_status_show(wlc_info_t *wlc)
{
    struct td_width_status *status = &wlc->td_width.status;
    struct td_width_info *info = &wlc->td_width;

    TD_PRINT("====%s====\n",__func__);
    TD_PRINT("Enable:%d Debug:%d\n",info->enable,info->debug);
    TD_PRINT("watchdog:%ds\n",info->watchdog_timer);
    TD_PRINT("itfr_eval_good_th:%d itfr_eval_bad_th:%d\n",
        info->itfr_eval_good_th,info->itfr_eval_bad_th);
    TD_PRINT("itfr good_pre_th:%d bad_pre_th:%d\n",info->itfr_good_pre_th,info->itfr_bad_pre_th);

    TD_PRINT("-------status-------\n");
    TD_PRINT("itfr_bad   :%d\n",status->itfr_bad);
    TD_PRINT("itfr_good  :%d\n",status->itfr_good);
    TD_PRINT("itfr_medium:%d\n",status->itfr_medium);
}

static void td_auto_width_status_clear(wlc_info_t *wlc)
{
    struct td_width_status *status = &wlc->td_width.status;

    memset(status,0,sizeof(struct td_width_status));    
}

static void td_change_width(wlc_info_t *wlc,int type)
{
	int idx;
	//wlc_bsscfg_t *cfg;
    
    TD_PRINT("--%s--type:%d--\n",__func__,type);
#ifdef TENDA_WLAN_DBG
    TD_DBG(wlc,TD_DBG_RADIO_CFG,"wl%d:autowidth cur(%s) to (%s)\n",
        wlc->pub->unit,CHSPEC_IS40(wlc->chanspec) ? "40M":"20M",
        (type == TD_AUTO_WIDTH_TO_20M) ? "20M":"40M");
#endif
	wlc_ht_coex_switch_bw(wlc, type);
    //FOREACH_UP_AP(wlc, idx, cfg) {
    //    wlc_ht_coex_switch_bw(cfg, type);
    //}
}

int is_scb_assoc(wlc_info_t *wlc)
{
    struct scb *scb;
    struct scb_iter scbiter;
    int ret = 0;

    FOREACHSCB(wlc->scbstate, &scbiter, scb) {
        if (SCB_ASSOCIATED(scb)) {
            ret = 1;
            break;
        }
    }
    return ret;
}

int td_auto_width_itfr_handle(wlc_info_t *wlc)
{
    struct td_width_info *info = &wlc->td_width;
    struct td_width_status *status = &wlc->td_width.status;
    int itfr_bad_pre = 0, itfr_good_pre = 0;
    int itfr_all_cnt = status->itfr_bad + status->itfr_good + status->itfr_medium;
    int ret = 0;

    if (itfr_all_cnt == 0) {
        return ret;
    }

    if (is_scb_assoc(wlc)) {
        TD_WIDTH_DBG(info,"%s:there are somme scb assoc,so return\n",__func__);
        return ret;
    }

    itfr_bad_pre = (status->itfr_bad * 100)/itfr_all_cnt;
    itfr_good_pre = (status->itfr_good * 100)/itfr_all_cnt;

    TD_WIDTH_DBG(info,"%s:Width(%s) bad_pre=%d th=%d, good_pre=%d th=%d\n",
        __func__,CHSPEC_IS40(wlc->chanspec) ? "40M":"20M",
        itfr_bad_pre,info->itfr_bad_pre_th,itfr_good_pre,info->itfr_good_pre_th);
    if (CHSPEC_IS40(wlc->chanspec)) {
        if (itfr_bad_pre > info->itfr_bad_pre_th) {
            td_change_width(wlc, TD_AUTO_WIDTH_TO_20M);
            ret = 1;
        }
    } else {
        if (itfr_good_pre > info->itfr_good_pre_th) {
            td_change_width(wlc, TD_AUTO_WIDTH_TO_40M);
            ret = 1;
        }
    }
    return ret;
}


void td_auto_width_watchdog(void *td_wlc)
{
    wlc_info_t *wlc = (wlc_info_t *)td_wlc;
    struct td_width_info *info = &wlc->td_width;

    if (info->enable == 0) {
        return;
    }

    if (wlc->wet || (APSTA_ENAB(wlc->pub))) {
        TD_WIDTH_DBG(info,"%s:it's wet or wisp mode,return!\n",__func__);
        return;
    }

    if (width_timer_counts > info->watchdog_timer) {
        width_timer_counts = 0;
        if(td_auto_width_itfr_handle(wlc)) {
            info->watchdog_timer += TD_INTER_WATCHDOG_TIME;
            if (info->watchdog_timer > TD_MAX_WATCHDOG_TIME) {
                info->watchdog_timer = TD_MAX_WATCHDOG_TIME;
            }
#ifdef TD_WIDTH_ADJUST_ONCE
            info->enable = 1;
#endif
        }
        td_auto_width_status_clear(wlc);
        return;
    }

    width_timer_counts++;
}

void td_auto_width_cfg(void *td_wlc,int setting)
{
    wlc_info_t *wlc = (wlc_info_t *)td_wlc;
    struct td_width_info *info = &wlc->td_width;
    
    info->enable            = setting;
    info->watchdog_timer    = TD_DEF_WATCHDOG_TIME;

    info->itfr_bad_pre_th   = TD_ITFR_BAD_PRE_TH;
    info->itfr_good_pre_th  = TD_ITFR_GOOD_PRE_TH;
    info->itfr_eval_good_th = TD_ITFR_EVAL_GOOD_TH;
    info->itfr_eval_bad_th  = TD_ITFR_EVAL_BAD_TH;
    info->debug             = TD_WIDTH_DEFAULT_DEBUG;
    width_timer_counts = 0;
    td_auto_width_status_clear(wlc);
}
static int td_auto_width_doiovar(void *hdl, const bcm_iovar_t *vi,
                                        uint32 actionid, const char *name,
                                        void *p, uint plen, void *a, int alen, 
                                        int val_size, struct wlc_if *wlcif)
{
    wlc_info_t *wlc = (wlc_info_t *)hdl;
    int32 int_val = 0;
    int32 *ret_int_ptr;
    int err = 0;
    ret_int_ptr = (int32 *)a;

    bcopy(p, &int_val, sizeof(int_val));
    switch (actionid) {
        case IOV_GVAL(IOV_TD_WIDTH_TEST):
            if (CHSPEC_IS40(wlc->default_bss->chanspec)) {
                TD_PRINT("cur width is 40M\n");
            } else {
                TD_PRINT("cur width is 20M\n");
            }
            *ret_int_ptr = 0;
            break;
        case IOV_SVAL(IOV_TD_WIDTH_TEST):
            td_change_width(wlc,int_val);
            break;

        case IOV_GVAL(IOV_TD_WIDTH_ENABLE):
            *ret_int_ptr = wlc->td_width.enable;
            break;
        case IOV_SVAL(IOV_TD_WIDTH_ENABLE):
            wlc->td_width.enable = int_val;
            td_auto_width_cfg(wlc,int_val);
            break;

        case IOV_GVAL(IOV_TD_WIDTH_SHOW):
            td_auto_width_status_show(wlc);
            *ret_int_ptr = 0;
            break;

        case IOV_GVAL(IOV_TD_WIDTH_DBG):
            *ret_int_ptr = wlc->td_width.debug;
            break;
        case IOV_SVAL(IOV_TD_WIDTH_DBG):
            wlc->td_width.debug = int_val;
            break;

        case IOV_GVAL(IOV_TD_WIDTH_WATCHDOG_TIME):
            *ret_int_ptr = wlc->td_width.watchdog_timer;
            break;
        case IOV_SVAL(IOV_TD_WIDTH_WATCHDOG_TIME):
            wlc->td_width.watchdog_timer = int_val;
            break;

/* add for itfr */
        case IOV_GVAL(IOV_TD_WIDTH_ITFR_GOOD_PRE):
            *ret_int_ptr = wlc->td_width.itfr_good_pre_th;
            break;
        case IOV_SVAL(IOV_TD_WIDTH_ITFR_GOOD_PRE):
            wlc->td_width.itfr_good_pre_th = int_val;
            break;
        case IOV_GVAL(IOV_TD_WIDTH_ITFR_BAD_PRE):
            *ret_int_ptr = wlc->td_width.itfr_bad_pre_th;
            break;
        case IOV_SVAL(IOV_TD_WIDTH_ITFR_BAD_PRE):
            wlc->td_width.itfr_bad_pre_th = int_val;
            break;
        case IOV_GVAL(IOV_TD_WIDTH_ITFR_EVAL_GOOD_TH):
            *ret_int_ptr = wlc->td_width.itfr_eval_good_th;
            break;
        case IOV_SVAL(IOV_TD_WIDTH_ITFR_EVAL_GOOD_TH):
            wlc->td_width.itfr_eval_good_th = int_val;
            break;
        case IOV_GVAL(IOV_TD_WIDTH_ITFR_EVAL_BAD_TH):
            *ret_int_ptr = wlc->td_width.itfr_eval_bad_th;
            break;
        case IOV_SVAL(IOV_TD_WIDTH_ITFR_EVAL_BAD_TH):
            wlc->td_width.itfr_eval_bad_th = int_val;
            break;
/* end for itfr */
        default:
            td_auto_width_help(wlc);
            err = BCME_UNSUPPORTED;
    }
    return err;
}

void td_auto_width_init(void *td_wlc)
{
    wlc_info_t *wlc = (wlc_info_t *)td_wlc;

    td_auto_width_cfg(wlc,0);
    /* register module */
    if (wlc_module_register(wlc->pub, td_auto_width_iovars, "td_width", wlc, td_auto_width_doiovar,
                            NULL, NULL, NULL)) {
        printf("---%s: wlc_module_register err!\n",__func__);
    }
}

void td_auto_width_exit(void *td_wlc)
{
    wlc_info_t *wlc = (wlc_info_t *)td_wlc;
    wlc_module_unregister(wlc->pub, "td_width", wlc);
}

#endif

