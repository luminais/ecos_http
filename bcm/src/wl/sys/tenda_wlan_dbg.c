#include <sys/syslog.h>
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

#ifdef TENDA_WLAN_DBG

enum {
    IOV_TD_WL_DBG_VAL,
    IOV_TD_WL_DBG_HELP
};

static const bcm_iovar_t td_wlan_dbg_iovars[] = {
    {"td_dbg", IOV_TD_WL_DBG_VAL, 0 , IOVT_INT32, 0},
    {"td_dbg_help", IOV_TD_WL_DBG_HELP, 0 , IOVT_INT32, 0},
    {NULL, 0, 0, 0, 0}
};

void td_wlan_syslog(int flag,const char *fmt, ...)
{
    char buf[256] = {0};
    if (flag & TD_DBG_SYSLOG_MASK) {
        va_list arg;
        va_start(arg, fmt);
        vsnprintf(buf, 255, fmt, arg);
        va_end (arg);
        syslog(LOG_INFO,buf);
    }
}
void td_wlan_dbg_help(void)
{
    printf("---%s---\n",__func__);

    printf("  TD_DBG_RADIO_CFG  : 0x%x\n",TD_DBG_RADIO_CFG);
    printf("  TD_DBG_BIG_HAMMER : 0x%x\n",TD_DBG_BIG_HAMMER);
    printf("  TD_DBG_EVENT : 0x%x\n",TD_DBG_EVENT);
    printf("  TD_DBG_AUTH  : 0x%x\n",TD_DBG_AUTH);
    printf("  TD_DBG_ASSOC : 0x%x\n",TD_DBG_ASSOC);
    printf("  TD_DBG_DEAUTH: 0x%x\n",TD_DBG_DEAUTH);
    printf("  TD_DBG_EAP   : 0x%x\n",TD_DBG_EAP);
    printf("  TD_DBG_ICMP  : 0x%x\n",TD_DBG_ICMP);
    printf("  TD_DBG_ALL   : 0x%x\n",TD_DBG_ALL);
}
static int td_wlan_dbg_doiovar(void *hdl, const bcm_iovar_t *vi,
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
        case IOV_GVAL(IOV_TD_WL_DBG_VAL):
            *ret_int_ptr = wlc->td_wlan_dbg;
            printf("wlc->td_if_opackets=%d\n",wlc->td_if_opackets);
            break;
        case IOV_SVAL(IOV_TD_WL_DBG_VAL):
            wlc->td_wlan_dbg = int_val;
            break;

        default:
            td_wlan_dbg_help();
            err = BCME_UNSUPPORTED;
    }
    return err;
}

void td_wlan_dbg_init(void *td_wlc)
{
    wlc_info_t *wlc = (wlc_info_t *)td_wlc;

    wlc->td_wlan_dbg = TD_DBG_DEFAULT_LEVEL;
    /* register module */
    if (wlc_module_register(wlc->pub, td_wlan_dbg_iovars, "td_dbg", wlc, td_wlan_dbg_doiovar,
                            NULL, NULL, NULL)) {
        printf("---%s: wlc_module_register err!\n",__func__);
    }
}

void td_wlan_dbg_exit(void *td_wlc)
{
    wlc_info_t *wlc = (wlc_info_t *)td_wlc;
    wlc->td_wlan_dbg = 0;
    wlc_module_unregister(wlc->pub, "td_dbg", wlc);
}
#endif
